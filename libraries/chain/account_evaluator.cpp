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

#include <fc/smart_ref_impl.hpp>

#include <graphene/chain/account_evaluator.hpp>
#include <graphene/chain/buyback.hpp>
#include <graphene/chain/buyback_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/budget_member_object.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/internal_exceptions.hpp>
#include <graphene/chain/special_authority.hpp>
#include <graphene/chain/special_authority_object.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/gateway_object.hpp>
#include <graphene/chain/carrier_object.hpp>
#include <graphene/chain/author_object.hpp>

#include <algorithm>

namespace graphene { namespace chain {

void verify_authority_accounts( const database& db, const authority& a )
{
   const auto& chain_params = db.get_global_properties().parameters;
   GRAPHENE_ASSERT(
      a.num_auths() <= chain_params.maximum_authority_membership,
      internal_verify_auth_max_auth_exceeded,
      "Maximum authority membership exceeded" );
   for( const auto& acnt : a.account_auths )
   {
      GRAPHENE_ASSERT( db.find_object( acnt.first ) != nullptr,
         internal_verify_auth_account_not_found,
         "Account ${a} specified in authority does not exist",
         ("a", acnt.first) );
   }
}
bool verify_account_auth(database &db,const authority *pauthor, account_id_type author_id,uint32_t &dep)
{
     dep --;
     if(dep<=0)
      return false;
      if (pauthor-> account_auths.find(author_id) != pauthor-> account_auths.end())
       return true;
     for(auto &a_id : pauthor-> account_auths)
     {  
       const auto &account = a_id.first(db);
       if(verify_account_auth(db,&account.owner, author_id,dep))
           return true;
       if(verify_account_auth(db,&account.active, author_id,dep))
           return true;
       if(verify_account_auth(db,&account.limitactive, author_id,dep))
           return true;     
     }
     return false;
}

void verify_account_votes( const database& db, const account_options& options )
{
   // ensure account's votes satisfy requirements
   // NB only the part of vote checking that requires chain state is here,
   // the rest occurs in account_options::validate()

   const auto& gpo = db.get_global_properties();
   const auto& chain_params = gpo.parameters;

   FC_ASSERT( options.num_witness <= chain_params.maximum_witness_count,
              "Voted for more witnesses than currently allowed (${c})", ("c", chain_params.maximum_witness_count) );
   FC_ASSERT( options.num_committee <= chain_params.maximum_committee_count,
              "Voted for more committee members than currently allowed (${c})", ("c", chain_params.maximum_committee_count) );
    FC_ASSERT( options.num_budget <= chain_params.maximum_budget_count,
             "Voted for more budget members than currently allowed (${c})", ("c", chain_params.maximum_budget_count) );
 
   {
    //  const auto& approve_worker_idx = db.get_index_type<worker_index>().indices().get<by_vote_for>(); 用户不给worker投票，预算委员会投票
      const auto& committee_idx = db.get_index_type<committee_member_index>().indices().get<by_vote_id>();      
      const auto& budget_idx    = db.get_index_type<budget_member_index>().indices().get<by_vote_id>();      
      const auto& witness_idx = db.get_index_type<witness_index>().indices().get<by_vote_id>();
      const auto& gateway_idx = db.get_index_type<gateway_index>().indices().get<by_vote_id>();
      const auto& carrier_idx = db.get_index_type<carrier_index>().indices().get<by_vote_id>();
      for ( auto id : options.votes ) {
         switch ( id.type() ) {
            case vote_id_type::committee:
               FC_ASSERT( committee_idx.find(id) != committee_idx.end(),
                          "Can not vote for ${id} which does not exist.", ("id",id) );
               break;
            case vote_id_type::budget:
               FC_ASSERT( budget_idx.find(id) != budget_idx.end(),
                          "Can not vote for ${id} which does not exist.", ("id",id) );
               break;
            case vote_id_type::witness:
               FC_ASSERT( witness_idx.find(id) != witness_idx.end(),
                          "Can not vote for ${id} which does not exist.", ("id",id) );
               break;
            case vote_id_type::gateway:
               FC_ASSERT(gateway_idx.find(id) != gateway_idx.end(),
                          "Can not vote for ${id} which does not exist.", ("id",id) );
               break;
            case vote_id_type::carrier:
               FC_ASSERT( carrier_idx.find(id) != carrier_idx.end(),
                          "Can not vote for ${id} which does not exist.", ("id",id) );
               break;
            /*case vote_id_type::worker:
               FC_ASSERT( approve_worker_idx.find( id ) != approve_worker_idx.end(),
                          "Can not vote for ${id} which does not exist.", ("id",id) );
               break;*/
            default:
               FC_THROW( "Invalid Vote Type: ${id}", ("id", id) );
               break;
         }
      }
   }
}


void_result account_create_evaluator::do_evaluate( const account_create_operation& op )
{ try {
   database& d = db();
 
    

   FC_ASSERT( d.find_object(op.options.voting_account), "Invalid proxy account specified." );
   FC_ASSERT( fee_paying_account->is_lifetime_member() || !is_cheap_name(op.name), "Only Lifetime members may register an account." );
   FC_ASSERT(!fee_paying_account->is_lock_balance());
   //推荐人可以是普通用户  annual member
   //FC_ASSERT( op.referrer(d).is_member(d.head_block_time()), "The referrer must be either a lifetime or annual subscriber." );

   try
   {
      verify_authority_accounts( d, op.owner );
      verify_authority_accounts( d, op.active );
      verify_authority_accounts( d, op.limitactive );
   }
   GRAPHENE_RECODE_EXC( internal_verify_auth_max_auth_exceeded, account_create_max_auth_exceeded )
   GRAPHENE_RECODE_EXC( internal_verify_auth_account_not_found, account_create_auth_account_not_found )

   if( op. owner_special_authority.valid() )
      evaluate_special_authority( d, *op. owner_special_authority );
   if( op. active_special_authority.valid() )
      evaluate_special_authority( d, *op. active_special_authority );
   if( op. buyback_options.valid() )
      evaluate_buyback_account_options( d, *op. buyback_options );
   verify_account_votes( d, op.options );

   auto& acnt_indx = d.get_index_type<account_index>();
   if( op.name.size() )
   {
      auto current_account_itr = acnt_indx.indices().get<by_name>().find( op.name );
      FC_ASSERT( current_account_itr == acnt_indx.indices().get<by_name>().end(),
                 "Account '${a}' already exists.", ("a",op.name) );
   }
   uint16_t referrer_percent = op.get_referrer_percent(db().head_block_time());   
   FC_ASSERT(referrer_percent   <= GRAPHENE_100_PERCENT);


   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type account_create_evaluator::do_apply( const account_create_operation& o )
{ try {

   database& d = db();
   uint16_t referrer_percent = o.get_referrer_percent(db().head_block_time());   

   const auto &new_acnt_object = db().create<account_object>([&](account_object &obj) {
         obj.membership_expiration_date = time_point_sec::min();
         obj.register_date = d.head_block_time();
         obj.registrar = o.registrar;
         obj.referrer = o.referrer;
         obj.lifetime_referrer = o.referrer(db()).lifetime_referrer;

         auto& params = db().get_global_properties().parameters;
         obj.network_fee_percentage = params.network_percent_of_fee;
         obj.lifetime_referrer_fee_percentage = params.lifetime_referrer_percent_of_fee;
         obj.referrer_rewards_percentage = referrer_percent;

         obj.name             = o.name;
         obj.owner            = o.owner;
         obj.active           = o.active;
         obj.limitactive      = o.limitactive;
         obj.options          = o.options;
         obj.statistics = db().create<account_statistics_object>([&](account_statistics_object& s)
         {
            s.owner = obj.id;
            s.coupon_month = 0;
            s.set_coupon(db(),0,true);
         }).id;

         if( o. owner_special_authority.valid() )
               obj.owner_special_authority = *(o. owner_special_authority);
         if( o. active_special_authority.valid() )
               obj.active_special_authority = *(o. active_special_authority);
         if( o. buyback_options.valid() )
         {
               obj.allowed_assets = o. buyback_options->markets;
               obj.allowed_assets->emplace(o. buyback_options->asset_to_buy);
         }
   });

   /*
   if( has_small_percent )
   {
      wlog( "Account affected by #453 registered in block ${n}:  ${na} reg=${reg} ref=${ref}:${refp} ltr=${ltr}:${ltrp}",
         ("n", db().head_block_num()) ("na", new_acnt_object.id)
         ("reg", o.registrar) ("ref", o.referrer) ("ltr", new_acnt_object.lifetime_referrer)
         ("refp", new_acnt_object.referrer_rewards_percentage) ("ltrp", new_acnt_object.lifetime_referrer_fee_percentage) );
      wlog( "Affected account object is ${o}", ("o", new_acnt_object) );
   }
   */

   const auto& dynamic_properties = db().get_dynamic_global_properties();
   db().modify(dynamic_properties, [](dynamic_global_property_object& p) {
      ++p.accounts_registered_this_interval;
   });

   const auto& global_properties = db().get_global_properties();
   if( dynamic_properties.accounts_registered_this_interval %
       global_properties.parameters.accounts_per_fee_scale == 0 )
      db().modify(global_properties, [&](global_property_object& p) {
         p.parameters.current_fees->get<account_create_operation>().basic_fee <<= p.parameters.account_fee_scale_bitshifts;
      });

   if(    o. owner_special_authority.valid() || o. active_special_authority.valid() )
   {
      db().create< special_authority_object >( [&]( special_authority_object& sa )
      {
         sa.account = new_acnt_object.id;
      } );
   }

   if( o. buyback_options.valid() )
   {
      asset_id_type asset_to_buy = o. buyback_options->asset_to_buy;
      d.create< buyback_object >( [&]( buyback_object& bo )
      {
         bo.asset_to_buy = asset_to_buy;
      } );
      d.modify( asset_to_buy(d), [&]( asset_object& a )
      {
         a.buyback_account = new_acnt_object.id;
      } );
   }

   return new_acnt_object.id;
} FC_CAPTURE_AND_RETHROW((o)) }


void_result account_update_evaluator::do_evaluate( const account_update_operation& o )
{ try {
   database& d = db();
 
   

   try
   {
      if( o.owner )      
      {
          uint32_t udep  = d.get_global_properties().parameters.maximum_authority_membership+1;
          FC_ASSERT(!verify_account_auth(d, &(*o.owner),o.account, udep));
          verify_authority_accounts(d, *o.owner);
      } 
      if( o.active )    
      {
          uint32_t udep  = d.get_global_properties().parameters.maximum_authority_membership+1;
          FC_ASSERT(!verify_account_auth(d, &(*o.active),o.account, udep));
          verify_authority_accounts( d, *o.active );
      } 
      if( o.limitactive )
      {
          uint32_t udep  = d.get_global_properties().parameters.maximum_authority_membership+1;
          FC_ASSERT(!verify_account_auth(d, &(*o.limitactive),o.account, udep));
          verify_authority_accounts( d, *o.limitactive ); 
      }           
   }
   GRAPHENE_RECODE_EXC( internal_verify_auth_max_auth_exceeded, account_update_max_auth_exceeded )
   GRAPHENE_RECODE_EXC( internal_verify_auth_account_not_found, account_update_auth_account_not_found )

   if( o.owner_special_authority.valid() )
      evaluate_special_authority( d, *o.owner_special_authority );
   if( o.active_special_authority.valid() )
      evaluate_special_authority( d, *o.active_special_authority );

   acnt = &o.account(d);

   //小理事会不能修改属性
   FC_ASSERT((acnt->uaccount_property & account_sub_committe) == 0);
   FC_ASSERT(!acnt->is_lock_balance());    

   if( o.new_options.valid() )
      verify_account_votes( d, *o.new_options );

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_update_evaluator::do_apply( const account_update_operation& o )
{ try {
   database& d = db();
   bool sa_before, sa_after;
   d.modify( *acnt, [&](account_object& a){
      if( o.owner )
      {
         a.owner = *o.owner;
         a.top_n_control_flags = 0;
      }
      if( o.active )
      {
         a.active = *o.active;
         a.top_n_control_flags = 0;
      }
      if( o.limitactive )
      {
         a.limitactive = *o.limitactive;
         a.top_n_control_flags = 0;
      }
      if( o.new_options )
      {
         //防止用户不小心修改了，必须通过  account_authenticate_operation 修改
         //20191201去掉，以后authkey和memokey独立分开，修改密码时候，自动对info修改加密
         //public_key_type auth_key = a.options.auth_key;
         a.options = *o.new_options;         
         // a.options.auth_key = auth_key;         
      }
       
      sa_before = a.has_special_authority();
      if( o.owner_special_authority.valid() )
      {
         a.owner_special_authority = *(o.owner_special_authority);
         a.top_n_control_flags = 0;
      }
      if( o.active_special_authority.valid() )
      {
         a.active_special_authority = *(o.active_special_authority);
         a.top_n_control_flags = 0;
      }      
      sa_after = a.has_special_authority();
   });

   if( sa_before && (!sa_after) )
   {
      const auto& sa_idx = d.get_index_type< special_authority_index >().indices().get<by_account>();
      auto sa_it = sa_idx.find( o.account );
      assert( sa_it != sa_idx.end() );
      d.remove( *sa_it );
   }
   else if( (!sa_before) && sa_after )
   {
      d.create< special_authority_object >( [&]( special_authority_object& sa )
      {
         sa.account = o.account;
      } );
   }
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }
 
void_result account_authenticate_evaluator::do_evaluate( const account_authenticate_operation& o )
{ try { 
   database& d = db(); 
   const auto &acnt = o.issuer(d);

   if(o.op_type == 1) //修改靓号提案人
   {
      FC_ASSERT(o.flags.valid());
      FC_ASSERT(((*o.flags) & (~account_flag_mask)) == 0);
      FC_ASSERT( (*o.flags) != (acnt.uaccount_property & account_flag_mask) );
   }
   else if(o.op_type == 2) //取消
   {
      FC_ASSERT(false);
   }
   else if(o.op_type == 3) //锁定账户
   {
      bool flag = (acnt.uaccount_property & account_lock_balance) > 0 ? true : false;
      FC_ASSERT(o.lock_balance.valid());
      FC_ASSERT( (*o.lock_balance) != flag);
   }
   else if(o.op_type == 4) //修改用户验证公钥
   {
      FC_ASSERT(o.auth_key.valid());
      FC_ASSERT( (*o.auth_key) != acnt.options.auth_key);
   }
   else if(o.op_type == 5) //修改用户数据
   {
      FC_ASSERT(o.auth_data.valid());   
      if(o.auth_data->authenticator.valid())  
      {         
         const auto &auth = (*(o.auth_data->authenticator))(d);
         //用户必须是认证人
         FC_ASSERT((auth.uaccount_property & account_authenticator) >0);   
         if(o.auth_data->key.valid())
            FC_ASSERT(auth.options.auth_key == o.auth_data->key->to);           
      }
      if(o.auth_key.valid()) {
         FC_ASSERT( (*o.auth_key) != acnt.options.auth_key);
      }
       if(o.auth_data->info.valid()) {
         FC_ASSERT(o.auth_data->info->from == o.auth_data->info->to);  
       }
   }   
   else if(o.op_type == 6)  //验证用户 
   {        
      FC_ASSERT(o.auth_account.valid());
      FC_ASSERT(o.auth_account->account != o.issuer);
      const auto &auth = o.auth_account->account(d);
      const auto &issuer_obj = o.issuer(d);
      issuer_obj.get_author_id(d);
      auth.referrer(d);
      FC_ASSERT((issuer_obj.uaccount_property & can_auth_mask) >0);   
      if(issuer_obj.uaccount_property & account_authenticator)
        FC_ASSERT(issuer_obj.is_author_enable(d));
      else if(issuer_obj.uaccount_property & account_carrier)
        FC_ASSERT(issuer_obj.is_carrier_enable(d));
      else if(issuer_obj.uaccount_property & account_gateway)
        FC_ASSERT(issuer_obj.is_gateway_enable(d));
      if(!o.auth_account->state && auth.auth_data.valid())
         FC_ASSERT(auth.auth_data->auth_state.find(o.issuer) != auth.auth_data->auth_state.end());
      FC_ASSERT((o.auth_account->state & author_mask) == 0);
      FC_ASSERT((o.auth_account->state & author_mask) < auth_nosetting);
   }
   else if(o.op_type == 7) //修改网关认证人信息
   {
     acnt.get_gateway_id(d)(d);     
     if (o.need_auth.valid())
     {
     }        
     if(o.auth_data.valid())
     {
       const auto &ca = (*(o.auth_data->authenticator))(d);       
       ca.get_author_id(d);
     }    
     if(o.trust_auth.valid())
     {
       for(auto & a : *o.trust_auth)
       {
          a.first(d);
          const auto &ca = a.second(d);
          FC_ASSERT((ca.uaccount_property & can_auth_mask) >0);
          FC_ASSERT(ca.is_auth_asset(d, a.first));
       }
     }
   }
   else if(o.op_type == 8)//修改运营商认证人信息
   {
      acnt.get_carrier_id(d)(d);
      if(o.need_auth.valid())       
      {
      }
      if(o.auth_data.valid())
      {
       const auto &ca = (*(o.auth_data->authenticator))(d);
       ca.get_author_id(d);
      }         
      if(o.trust_auth.valid())
      {
       for(auto & a : *o.trust_auth)
       {
          a.first(d);
          const auto &ca = a.second(d);
          FC_ASSERT((ca.uaccount_property & can_auth_mask) >0);
          FC_ASSERT(ca.is_auth_asset(d, a.first));
       }
     }
   }
   else if(o.op_type == 9) //绑定手机号邮箱
   {
      FC_ASSERT(o.auth_data.valid());      
      FC_ASSERT(o.auth_data->info.valid());      
      const auto& accounts_index = d.get_index_type<account_index>().indices().get<by_name>();
      auto iter = accounts_index.find("admin-notify");      
      FC_ASSERT(iter!=accounts_index.end(), "no admin notify");        
      FC_ASSERT(o.auth_data->info->to == iter->options.auth_key);
      FC_ASSERT(o.auth_data->info->message.size() <= 1000);
   }
   else if(o.op_type == 10) // 运营商是否支持 投资，借款
   {
     FC_ASSERT(o.flags.valid());      
     FC_ASSERT(((*o.flags) & carrier_op_xor_mask) == 0);
   }
   else if(o.op_type == 11) // 认证人奖励
   {
     const auto &acnt = o.issuer(d);
     acnt.get_author_id(d)(d);
     FC_ASSERT(o.flags.valid());      
     FC_ASSERT(o.need_auth.valid());  
   }
   else
      FC_ASSERT(false);

   return void_result();

    //if(o.flags.valid())  
//     
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_authenticate_evaluator::do_apply( const account_authenticate_operation& o )
{
  try {
    database &d = db();
    if (o.op_type == 6)  //验证用户 
    { 
      const auto &acnt = o.auth_account->account(d);
      d.modify(acnt, [&o, &d](account_object &a) {
         fc::time_point_sec now = d.head_block_time();
         if (!a.auth_data.valid()) 
         {
           account_auth_data add;            
           a.auth_data = add;
         }
         if (o.auth_account->state)
         {
            account_auth_item add;
            add.state = o.auth_account->state;

            if (a.uaccount_property & account_authenticator)
               add.state |= a.get_author_id(d)(d).auth_type | auth_author;
            if (a.uaccount_property & account_carrier)
               add.state |= auth_carrier;
            if (a.uaccount_property & account_gateway)
               add.state |= auth_gateway;

            add.expiration = o.auth_account->expiration;
            add.auth_time = now;
            if (o.auth_account->hash64 == 0)
               add.hash64 = a.auth_data->hash64;
            else
               add.hash64 = o.auth_account->hash64;
            a.auth_data->auth_state[o.issuer] = add;
         }
         else
         {
            a.auth_data->auth_state.erase(o.issuer);
         }
         //过期
         vector<account_id_type> erase_id;
         for (const auto &aa : a.auth_data->auth_state)
         {
            if (aa.second.auth_time + aa.second.expiration < now)
               erase_id.push_back(aa.first);
         }
         for (const auto &aa : erase_id)
            a.auth_data->auth_state.erase(aa);         
      });
      //处理奖励
      const auto &issuer_obj = o.issuer(d);
      const auto &au_obj = issuer_obj.get_author_id(d)(d);   
      if(au_obj.pay_for_account + au_obj.pay_for_referrer > 0)
      {
         asset all = d.get_balance(o.issuer, GRAPHENE_CORE_ASSET);
         if(all.amount >= au_obj.pay_for_account + au_obj.pay_for_referrer)
         {
            d.adjust_balance(109, "to " + acnt.name, o.issuer, -asset(au_obj.pay_for_account + au_obj.pay_for_referrer, all.asset_id));
            if(au_obj.pay_for_account>0)  d.adjust_balance(110, "from " + issuer_obj.name, acnt.id, asset(au_obj.pay_for_account, all.asset_id));
            if(au_obj.pay_for_referrer>0) d.adjust_balance(111, "from " + issuer_obj.name, acnt.referrer, asset(au_obj.pay_for_referrer, all.asset_id));
         }
      }
    } 
    else if(o.op_type == 7)//网关，认证人信息
    {
     const auto &acnt = o.issuer(d);
     const auto &auth = acnt.get_gateway_id(d)(d);
     d.modify(auth, [&](gateway_object &a) {
        if (o.need_auth.valid())        
           a.need_auth = *o.need_auth;
         if (o.trust_auth.valid())
         {
           a.trust_auth.clear();
           for(auto aa: *o.trust_auth) 
             a.trust_auth[aa.first] = aa.second;
         }
        if(o.auth_data.valid())
        {
          const auto &ca = (*(o.auth_data->authenticator))(d);
          a.def_auth = ca.get_author_id(d);
        }       
     });
    }
    else if(o.op_type == 8)//运营商 认证人信息
    {
      const auto &acnt = o.issuer(d);
      const auto &auth = acnt.get_carrier_id(d)(d);
      d.modify(auth, [&](carrier_object &a) {
        if (o.need_auth.valid())        
           a.need_auth = *o.need_auth;
         if (o.trust_auth.valid())
         {
           a.trust_auth.clear();
           for(auto aa: *o.trust_auth) 
             a.trust_auth[aa.first] = aa.second;
         }
         if(o.auth_data.valid())
         {
          const auto &ca = (*(o.auth_data->authenticator))(d);
          a.def_auth = ca.get_author_id(d);
         }    
      });  
    }
    else if(o.op_type == 9)
    {
      const auto &acnt = o.issuer(d);
      d.modify(acnt, [&](account_object &a) {
         a.user_info = *(o.auth_data->info);
      });
    }
    else if(o.op_type == 10)
    {
      const auto &acnt = o.issuer(d);
      const auto &auth = acnt.get_carrier_id(d)(d);
      d.modify(auth, [&](carrier_object &a) {
         a.op_type = (*o.flags) & carrier_op_mask;
      });
    }
    else if(o.op_type == 11)
    {
      const auto &acnt = o.issuer(d);
      const auto &auth = acnt.get_author_id(d)(d);
      d.modify(auth, [&](author_object &a) {
         a.pay_for_account = (*o.flags);
         a.pay_for_referrer = (*o.need_auth);
      });
    }
    else {
      const auto &acnt = o.issuer(d);
      d.modify(acnt, [&](account_object &a) {
        if (o.op_type == 1)  //修改靓号提案人身份
        {
          a.uaccount_property &= ~account_flag_mask;
          a.uaccount_property |= (*o.flags) & account_flag_mask;
        }
        else if (o.op_type == 2) //取消
        {          
        }
        else if (o.op_type == 3) //锁定账户
        {
          if (*o.lock_balance)
            a.uaccount_property |= account_lock_balance;
          else
            a.uaccount_property &= ~account_lock_balance;
        }
        else if (o.op_type == 4) //修改用户验证公钥
        {
          a.options.auth_key = *o.auth_key;
        }
        else if (o.op_type == 5) //修改用户数据
        {
          if (!a.auth_data.valid()) 
          {
            account_auth_data add;            
            a.auth_data = add;
          }
          if(o.auth_key.valid()) {
            a.options.auth_key = *o.auth_key;
          }
          if(o.auth_data->key.valid())
          {
            a.auth_data->key    = *o.auth_data->key;
            a.auth_data->hash64 = fc::hash64((const char *)&a.auth_data->key, sizeof(a.auth_data->key));            
          }
          if(o.auth_data->info.valid())
             a.auth_data->info = *o.auth_data->info;
        }
      });
    }

    return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

 

void_result account_config_evaluator::do_evaluate( const account_config_operation& o )
{ try { 
   database& d = db(); 
   const auto &acnt = o.issuer(d);
   FC_ASSERT(acnt.uaccount_property >= 0);

   for(auto &b : o.config)  {
      if(b.first == "locktoken_recommend") {
         vector<lock_recommend> recommend;                          
         recommend = fc::json::from_string(b.second).as<vector<lock_recommend>> (4);
         _config[b.first] =   fc::json::to_string(recommend);
      } if(b.first == "sellPair") {
         vector<sell_pair> sellpari;                          
         sellpari = fc::json::from_string(b.second).as<vector<sell_pair>> (4);
         _config[b.first] =  fc::json::to_string(sellpari);
      } else {
         _config[b.first] = b.second;
      }
   }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_config_evaluator::do_apply( const account_config_operation& o )
{
  try {
    database &d = db();
    const auto &acnt = o.issuer(d);
    d.modify(acnt, [&](account_object &a) {        
       if(o.op_type == 1)
       {
         for(auto &b : _config)   
            a.configs.erase(b.first);
       }
       else
       {
          for(auto &b : _config) 
            a.configs[b.first] = b.second;
       }
    });
    return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }


void_result account_whitelist_evaluator::do_evaluate(const account_whitelist_operation& o)
{ try {
   database& d = db();
   FC_ASSERT(false,"3");
   
   listed_account = &o.account_to_list(d);
   FC_ASSERT(!listed_account->is_lock_balance());

   if( !d.get_global_properties().parameters.allow_non_member_whitelists )
      FC_ASSERT( o.authorizing_account(d).is_lifetime_member(), "The authorizing account must be a lifetime member." );

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_whitelist_evaluator::do_apply(const account_whitelist_operation& o)
{ try {
   database& d = db();

   d.modify(*listed_account, [&o](account_object& a) {
      if( o.new_listing & o.white_listed )
         a.whitelisting_accounts.insert(o.authorizing_account);
      else
         a.whitelisting_accounts.erase(o.authorizing_account);

      if( o.new_listing & o.black_listed )
         a.blacklisting_accounts.insert(o.authorizing_account);
      else
         a.blacklisting_accounts.erase(o.authorizing_account);
   });

   /** for tracking purposes only, this state is not needed to evaluate */
   d.modify( o.authorizing_account(d), [&]( account_object& a ) {
     if( o.new_listing & o.white_listed )
        a.whitelisted_accounts.insert( o.account_to_list );
     else
        a.whitelisted_accounts.erase( o.account_to_list );

     if( o.new_listing & o.black_listed )
        a.blacklisted_accounts.insert( o.account_to_list );
     else
        a.blacklisted_accounts.erase( o.account_to_list );
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_upgrade_evaluator::do_evaluate(const account_upgrade_evaluator::operation_type& o)
{ try {
   database& d = db();

   account = &d.get(o.account_to_upgrade);
   FC_ASSERT(!account->is_lifetime_member());
   FC_ASSERT(!account->is_lock_balance());

   return {};
//} FC_CAPTURE_AND_RETHROW( (o) ) }
} FC_RETHROW_EXCEPTIONS( error, "Unable to upgrade account '${a}'", ("a",o.account_to_upgrade(db()).name) ) }

void_result account_upgrade_evaluator::do_apply(const account_upgrade_evaluator::operation_type& o)
{ try {
   database& d = db();

   d.modify(*account, [&](account_object& a) {
      if( o.upgrade_to_lifetime_member )
      {
         // Upgrade to lifetime member. I don't care what the account was before.
         a.statistics(d).process_fees(a, d);
         a.membership_expiration_date = time_point_sec::maximum();
         a.referrer = a.registrar = a.lifetime_referrer = a.get_id();
         a.lifetime_referrer_fee_percentage = GRAPHENE_100_PERCENT - a.network_fee_percentage;
      } else if( a.is_annual_member(d.head_block_time()) ) {
         FC_ASSERT(false, "can not support annual member"); // annual member eric
         // Renew an annual subscription that's still in effect.
         //FC_ASSERT( d.head_block_time() <= HZARDFORK_613_TIME );
         FC_ASSERT(a.membership_expiration_date - d.head_block_time() < fc::days(3650),
                   "May not extend annual membership more than a decade into the future.");
         a.membership_expiration_date += fc::days(365);
      } else {
         // Upgrade from basic account.
         FC_ASSERT(false, "can not support annual member");// annual member eric
         ///FC_ASSERT(d.head_block_time() <= HZARDFORK_613_TIME);
         a.statistics(d).process_fees(a, d);
         assert(a.is_basic_account(d.head_block_time()));
         a.referrer = a.get_id();
         a.membership_expiration_date = d.head_block_time() + fc::days(365);
      }
   });

   return {};
} FC_RETHROW_EXCEPTIONS( error, "Unable to upgrade account '${a}'", ("a",o.account_to_upgrade(db()).name) ) }

void_result account_coupon_evaluator::do_evaluate(const account_coupon_evaluator::operation_type& o)
{ try {
      database& d = db();
      const auto& dynamic_properties = d.get_dynamic_global_properties();
      const auto& chain_parameters   = d.get_global_properties().parameters;
      const auto& account            = o.issuer(d);
      const auto& statistics         = account.statistics(d);

      FC_ASSERT(d.head_block_time() < dynamic_properties.initial_time + chain_parameters.coupon_expire_time, "coupon_expire_time '${a}'", ("a", (dynamic_properties.initial_time + chain_parameters.coupon_expire_time).to_iso_string()));
      FC_ASSERT(statistics.coupon_month != fc::get_month_flag(d.head_block_time()), "coupon_month '${a}'", ("a", statistics.coupon_month));
      FC_ASSERT(statistics.amount_coupon != chain_parameters.coupon_per_month, "amount_coupon '${a}' < '${b}' ", ("a", statistics.amount_coupon) ("b", chain_parameters.coupon_per_month));
      FC_ASSERT(!account.is_lock_balance()); 
      return {}; 
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_coupon_evaluator::do_apply(const account_coupon_evaluator::operation_type& o)
{ try {
   database& d = db();  
   const auto& account            = o.issuer(d);
   const auto& statistics         = account.statistics(d);
   d.modify( statistics, [&]( account_statistics_object& obj ){
      obj.set_coupon(d, o.fee.amount,true);   
    });         
    return {}; 
} FC_CAPTURE_AND_RETHROW( (o) ) }
 


void_result withdraw_exchange_fee_evaluator::do_evaluate(const withdraw_exchange_fee_evaluator::operation_type& o)
{ try {
    const auto& d                  = db();  
     const auto& account            = o.issuer(d);
     const auto& statistics         = account.statistics(d);   
     const auto& asset_fees         = o.withdraw_asset_id(d); 

     FC_ASSERT(!account.is_lock_balance());     
     FC_ASSERT(statistics.exchange_fees >= o.core_amount,"all_fees '${a}' exchange_fees  '${b}' ",("a",statistics.exchange_fees)("b", o.core_amount));
     if(o.withdraw_asset_id!=GRAPHENE_CORE_ASSET)
     {
        FC_ASSERT(asset_fees.is_bitasset_issued());
        const auto& price_fees         = asset_fees.bitasset_data(d);      
        price_fees.current_feed.core_exchange_rate.validate();
        price_fees.current_feed.core_exchange_rate.is_core();
        asset get = asset(o.core_amount, GRAPHENE_CORE_ASSET) * price_fees.current_feed.core_exchange_rate;
        asset all = d.get_balance(GRAPHENE_PLATFORM_ACCOUNT, o.withdraw_asset_id);
        FC_ASSERT(get.amount > 0);
        FC_ASSERT(all >= get,"plantform_id all_fees '${a}' exchange_fees  '${b}' ",("a",all)("b",get));
     }

     return {}; 
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result withdraw_exchange_fee_evaluator::do_apply(const withdraw_exchange_fee_evaluator::operation_type& o)
{ try {
     auto& d                        = db();  
     const auto& account            = o.issuer(d);
     const auto& statistics         = account.statistics(d);    
     const auto& asset_fees         = o.withdraw_asset_id(d);     
     if(o.withdraw_asset_id!=GRAPHENE_CORE_ASSET)
     {
        const auto& price_fees         = asset_fees.bitasset_data(d);     
        asset get = asset(o.core_amount, GRAPHENE_CORE_ASSET)*price_fees.current_feed.core_exchange_rate;  
        d.adjust_balance(0,"",GRAPHENE_PLATFORM_ACCOUNT,-get);        
        d.adjust_balance(0,"",GRAPHENE_PLATFORM_ACCOUNT,asset(o.core_amount, GRAPHENE_CORE_ASSET));        
        d.adjust_balance(1,"",o.issuer,get);        
     }
     else
     {
        asset get = asset(o.core_amount, GRAPHENE_CORE_ASSET);
        d.adjust_balance(1,"",o.issuer,get);        
     }
     d.modify( statistics, [&]( account_statistics_object& obj ){
      obj.exchange_fees -=  o.core_amount;   
    });       
    
     return {}; 
} FC_CAPTURE_AND_RETHROW( (o) ) }


} } // graphene::chain
