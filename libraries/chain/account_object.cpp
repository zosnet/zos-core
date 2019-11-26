/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/gateway_object.hpp>
#include <graphene/chain/carrier_object.hpp>
#include <graphene/business/bitlender_option_object.hpp>
#include <graphene/chain/author_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/hardfork.hpp>
#include <fc/uint128.hpp>



namespace graphene { namespace chain {

share_type cut_fee(share_type a, uint16_t p)
{
   if( a == 0 || p == 0 )
      return 0;
   if( p == GRAPHENE_100_PERCENT )
      return a;

   fc::uint128 r(a.value);
   r *= p;
   r /= GRAPHENE_100_PERCENT;
   return r.to_uint64();
}

void account_balance_object::adjust_balance(const asset& delta)
{
   assert(delta.asset_id == asset_type);
   balance += delta.amount;
} 
void account_statistics_object::set_coupon(database& d,share_type fee,bool bset)
{
    if(bset)
    {
       const auto& dynamic_properties = d.get_dynamic_global_properties();
       const auto& chain_parameters   = d.get_global_properties().parameters;
       if((d.head_block_time()< dynamic_properties.initial_time  + chain_parameters.coupon_expire_time) && coupon_month != fc::get_month_flag(d.head_block_time()) && amount_coupon < chain_parameters.coupon_per_month)
       {                    
         share_type core_add     = chain_parameters.coupon_per_month - amount_coupon + fee;
         if(core_add >0 )
         {
             amount_coupon += core_add;
             d.modify( dynamic_properties, [&]( dynamic_global_property_object& obj ){
                 obj.coupon_supply += core_add;      
             });
             coupon_month = fc::get_month_flag(d.head_block_time());
         }         
       }  
    }
    else if(!bset && amount_coupon >0)
    {                 
        const auto& dynamic_properties = d.get_dynamic_global_properties();
        if (owner.get_instance() > 100)
        {
           d.modify(dynamic_properties, [&](dynamic_global_property_object &obj) {
              obj.coupon_supply -= amount_coupon;
           });
        }
        amount_coupon = 0;
        coupon_month = 0;
    }
}
void account_statistics_object::process_fees(const account_object& a, database& d) const
{    
    if( pending_fees > 0 || pending_vested_fees > 0 )
   {
      auto pay_out_fees = [&](const account_object& account, share_type core_fee_total, bool require_vesting)
      {
         // Check the referrer -- if he's no longer a member, pay to the lifetime referrer instead.
         // No need to check the registrar; registrars are required to be lifetime members.
         /*if(account.referrer != account.lifetime_referrer && account.referrer(d).is_basic_account(d.head_block_time()) )
            d.modify(account, [](account_object& a) {
               a.referrer = a.lifetime_referrer;
            });*///// annual member
           
         asset_id_type core_id = GRAPHENE_CORE_ASSET;

         share_type network_cut = cut_fee(core_fee_total, account.network_fee_percentage);
         assert( network_cut <= core_fee_total );

#ifndef NDEBUG
         const auto& props = d.get_global_properties();

         share_type reserveed = cut_fee(network_cut, props.parameters.reserve_percent_of_fee);
         share_type accumulated = network_cut - reserveed;
         assert( accumulated + reserveed == network_cut );
#endif
         share_type lifetime_cut = cut_fee(core_fee_total, account.lifetime_referrer_fee_percentage);
         share_type referral = core_fee_total - network_cut - lifetime_cut;

         d.modify(core_id(d).dynamic_data(d), [network_cut](asset_dynamic_data_object& d) {
            d.accumulated_fees += network_cut;
         });

         // Potential optimization: Skip some of this math and object lookups by special casing on the account type.
         // For example, if the account is a lifetime member, we can skip all this and just deposit the referral to
         // it directly.
         share_type referrer_cut = cut_fee(referral, account.referrer_rewards_percentage);
         share_type registrar_cut = referral - referrer_cut;

         d.deposit_cashback(d.get(account.lifetime_referrer),core_id, lifetime_cut, require_vesting);
         d.deposit_cashback(d.get(account.referrer),core_id, referrer_cut, require_vesting);
         d.deposit_cashback(d.get(account.registrar), core_id,registrar_cut, require_vesting);

         assert( referrer_cut + registrar_cut + accumulated + reserveed + lifetime_cut == core_fee_total );
      };

      pay_out_fees(a, pending_fees, true);
      pay_out_fees(a, pending_vested_fees, false);

      d.modify(*this, [&](account_statistics_object& s) {
         s.lifetime_fees_paid += pending_fees + pending_vested_fees;
         s.pending_fees = 0;
         s.pending_vested_fees = 0;
      });
   }
   if(coupon_fees >0)
   {
      const auto& dynamic_properties = d.get_dynamic_global_properties();         
     d.modify( dynamic_properties, [&]( dynamic_global_property_object& obj ){
         obj.coupon_supply -= coupon_fees;      
     });
     d.modify(*this, [&](account_statistics_object& s) {         
         s.coupon_fees = 0;
      });
   }
   if(amount_coupon >0)
   {
    const auto& dynamic_properties = d.get_dynamic_global_properties();
    const auto& chain_parameters   = d.get_global_properties().parameters;
    if(d.head_block_time() >= dynamic_properties.initial_time + chain_parameters.coupon_expire_time)
    {
        d.modify(*this, [&](account_statistics_object& s) {         
            s.set_coupon(d,0,false);
      });
    }
   }
}

void account_statistics_object::pay_fee(asset_id_type core_id, share_type core_fee, share_type cashback_vesting_threshold )
{
   if( core_fee > cashback_vesting_threshold )
      pending_fees += core_fee;
   else
      pending_vested_fees += core_fee;    
}

set<account_id_type> account_member_index::get_account_members(const account_object& a)const
{
   set<account_id_type> result;
   for( auto auth : a.owner.account_auths )
      result.insert(auth.first);
   for( auto auth : a.active.account_auths )
      result.insert(auth.first);
   return result;
}
set<public_key_type> account_member_index::get_key_members(const account_object& a)const
{
   set<public_key_type> result;
   for( auto auth : a.owner.key_auths )
      result.insert(auth.first);
   for( auto auth : a.active.key_auths )
      result.insert(auth.first);
   result.insert( a.options.memo_key );
   return result;
}
set<address> account_member_index::get_address_members(const account_object& a)const
{
   set<address> result;
   for( auto auth : a.owner.address_auths )
      result.insert(auth.first);
   for( auto auth : a.active.address_auths )
      result.insert(auth.first);
   result.insert( a.options.memo_key );
   return result;
}

void account_member_index::object_inserted(const object& obj)
{
    assert( dynamic_cast<const account_object*>(&obj) ); // for debug only
    const account_object& a = static_cast<const account_object&>(obj);

    auto account_members = get_account_members(a);
    for( auto item : account_members )
       account_to_account_memberships[item].insert(obj.id);

    auto key_members = get_key_members(a);
    for( auto item : key_members )
       account_to_key_memberships[item].insert(obj.id);

    auto address_members = get_address_members(a);
    for( auto item : address_members )
       account_to_address_memberships[item].insert(obj.id);
}

void account_member_index::object_removed(const object& obj)
{
    assert( dynamic_cast<const account_object*>(&obj) ); // for debug only
    const account_object& a = static_cast<const account_object&>(obj);

    auto key_members = get_key_members(a);
    for( auto item : key_members )
       account_to_key_memberships[item].erase( obj.id );

    auto address_members = get_address_members(a);
    for( auto item : address_members )
       account_to_address_memberships[item].erase( obj.id );

    auto account_members = get_account_members(a);
    for( auto item : account_members )
       account_to_account_memberships[item].erase( obj.id );
}

void account_member_index::about_to_modify(const object& before)
{
   before_key_members.clear();
   before_account_members.clear();
   assert( dynamic_cast<const account_object*>(&before) ); // for debug only
   const account_object& a = static_cast<const account_object&>(before);
   before_key_members     = get_key_members(a);
   before_address_members = get_address_members(a);
   before_account_members = get_account_members(a);
}

void account_member_index::object_modified(const object& after)
{
    assert( dynamic_cast<const account_object*>(&after) ); // for debug only
    const account_object& a = static_cast<const account_object&>(after);

    {
       set<account_id_type> after_account_members = get_account_members(a);
       vector<account_id_type> removed; removed.reserve(before_account_members.size());
       std::set_difference(before_account_members.begin(), before_account_members.end(),
                           after_account_members.begin(), after_account_members.end(),
                           std::inserter(removed, removed.end()));

       for( auto itr = removed.begin(); itr != removed.end(); ++itr )
          account_to_account_memberships[*itr].erase(after.id);

       vector<object_id_type> added; added.reserve(after_account_members.size());
       std::set_difference(after_account_members.begin(), after_account_members.end(),
                           before_account_members.begin(), before_account_members.end(),
                           std::inserter(added, added.end()));

       for( auto itr = added.begin(); itr != added.end(); ++itr )
          account_to_account_memberships[*itr].insert(after.id);
    }


    {
       set<public_key_type> after_key_members = get_key_members(a);

       vector<public_key_type> removed; removed.reserve(before_key_members.size());
       std::set_difference(before_key_members.begin(), before_key_members.end(),
                           after_key_members.begin(), after_key_members.end(),
                           std::inserter(removed, removed.end()));

       for( auto itr = removed.begin(); itr != removed.end(); ++itr )
          account_to_key_memberships[*itr].erase(after.id);

       vector<public_key_type> added; added.reserve(after_key_members.size());
       std::set_difference(after_key_members.begin(), after_key_members.end(),
                           before_key_members.begin(), before_key_members.end(),
                           std::inserter(added, added.end()));

       for( auto itr = added.begin(); itr != added.end(); ++itr )
          account_to_key_memberships[*itr].insert(after.id);
    }

    {
       set<address> after_address_members = get_address_members(a);

       vector<address> removed; removed.reserve(before_address_members.size());
       std::set_difference(before_address_members.begin(), before_address_members.end(),
                           after_address_members.begin(), after_address_members.end(),
                           std::inserter(removed, removed.end()));

       for( auto itr = removed.begin(); itr != removed.end(); ++itr )
          account_to_address_memberships[*itr].erase(after.id);

       vector<address> added; added.reserve(after_address_members.size());
       std::set_difference(after_address_members.begin(), after_address_members.end(),
                           before_address_members.begin(), before_address_members.end(),
                           std::inserter(added, added.end()));

       for( auto itr = added.begin(); itr != added.end(); ++itr )
          account_to_address_memberships[*itr].insert(after.id);
    }

}

void account_referrer_index::object_inserted( const object& obj )
{
}
void account_referrer_index::object_removed( const object& obj )
{
}
void account_referrer_index::about_to_modify( const object& before )
{
}
void account_referrer_index::object_modified( const object& after  )
{
} 
 vector<asset_id_type>  account_object::get_auth_asset(const database& db,uint32_t type) const
 {
    vector<asset_id_type> re;
    if((type &0x01) && (uaccount_property & account_authenticator))
    {
         const auto& idx = db.get_index_type<author_index>().indices().get<by_account>();
         auto itr = idx.find(id);
         if(itr!=idx.end())
         {
            for(const auto &a : itr->allow_asset)
               re.push_back(a);
         }
    }
    if((type &0x02) && (uaccount_property & account_gateway))
    {
         const auto& idx = db.get_index_type<gateway_index>().indices().get<by_account>();
         auto itr = idx.find(id);
         if(itr!=idx.end())
         {
            for(const auto &a : itr->allowed_assets)
               re.push_back(a);
         }
    }
    if((type &0x04) && (uaccount_property & account_carrier))
    {
         const auto& idx = db.get_index_type<bitlender_option_index>().indices().get<by_id>();
         for( const bitlender_option_object& option : idx)
         {               
            for( const auto & op : option.options)
            {
               if(op.second.options.carriers.count(id))
                 re.push_back(option.asset_id);
            }
         }      
    }
    return re;
 }
 const bool  account_object::is_auth_asset(const database& db,asset_id_type id,uint32_t type) const
 {
    vector<asset_id_type> re = get_auth_asset(db,type);
    return std::find(re.begin(), re.end(), id) != re.end();    
 }
const bool account_object::is_gateway_enable(const database& db,asset_id_type ass_id) const
{
   if((uaccount_property & account_gateway) <=0) return false;
   const auto& idx = db.get_index_type<gateway_index>().indices().get<by_account>();
   auto itr = idx.find(id);
   if( itr != idx.end() ) 
   {
      if(!itr->is_enable())
         return false;
      const auto &ass = ass_id(db);        
      return ass.whitelist_gateways.find(itr->gateway_account)  != ass.whitelist_gateways.end();
    }
    else              
      return false;
}
 const bool account_object::is_gateway_enable(const database& db) const
{
   if((uaccount_property & account_gateway) <=0) return false;
   const auto& idx = db.get_index_type<gateway_index>().indices().get<by_account>();
    auto itr = idx.find(id);
   if( itr != idx.end() )  return itr->is_enable();   
    else                   return false;
}
const bool account_object::is_carrier_enable(const database& db) const
{
    if((uaccount_property & account_carrier) <=0) return false; 
    const auto& idx = db.get_index_type<carrier_index>().indices().get<by_account>();
    auto itr = idx.find(id);
    if( itr != idx.end() )  return itr->is_enable();   
    else                    return false;
}
const carrier_id_type account_object::get_carrier_id(const database& db) const
{
    const auto& idx = db.get_index_type<carrier_index>().indices().get<by_account>();
    auto itr = idx.find(id);
    if( itr != idx.end() )  return itr->id;   
    else                    
    {
       FC_ASSERT(false,"get_carrier_id ${x}",("x",name));
       return carrier_id_type(0);
    }    
}
 const gateway_id_type  account_object::get_gateway_id(const database &db) const
 {
    const auto& idx = db.get_index_type<gateway_index>().indices().get<by_account>();
    auto itr = idx.find(id);
    if( itr != idx.end() )  return itr->id;   
    else                    
    {
       FC_ASSERT(false,"get_gateway_id ${x}",("x",name));
       return gateway_id_type(0);
    }    
 }

const bool account_object::is_author_enable(const database& db) const
{
   if((uaccount_property & account_authenticator) <=0) return false;
   const auto& idx = db.get_index_type<author_index>().indices().get<by_account>();
   auto itr = idx.find(id);
   if( itr != idx.end() )  return itr->is_enable();   
   else                    return false;
}
const author_id_type account_object::get_author_id(const database& db) const
{
    const auto& idx = db.get_index_type<author_index>().indices().get<by_account>();
    auto itr = idx.find(id);
    if( itr != idx.end() )  return itr->id;   
    else                    
    {
       FC_ASSERT(false,"get_author_id ${x}",("x",name));
       return author_id_type(0);
    }    
}

} } // graphene::chain
