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
#include <graphene/chain/gateway_evaluator.hpp>
#include <graphene/chain/gateway_object.hpp>
#include <graphene/business/bitlender_option_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/protocol/vote.hpp>

namespace graphene { namespace chain {
 
 void do_allowed_asset(database &db,gateway_object &gateway,const flat_set<asset_id_type> &allowed_new)
 {
   flat_set<asset_id_type> do_asset;
   for (auto &a : gateway.allowed_assets)
   {
     const auto &ass = a(db);
     db.modify(ass, [&](asset_object& p) {
      p.whitelist_gateways.erase(gateway.gateway_account);
     });
     do_asset.insert(a);
   }
   gateway.allowed_assets.clear();
   for(auto &a : allowed_new)
   {
     const auto &ass = a(db);
     db.modify(ass, [&](asset_object& p) 
     { 
      if(!p.whitelist_gateways.count(gateway.gateway_account))
          p.whitelist_gateways.insert(gateway.gateway_account);
     });
     gateway.allowed_assets.insert(a);
     do_asset.insert(a);
   }   
   const auto& bitlender_options = db.get_index_type<bitlender_option_index>().indices().get<by_id>();
   for( const bitlender_option_object& option : bitlender_options)
   {      
      if(do_asset.find(option.asset_id )!=do_asset.end())
      {
         const auto &ass = option.asset_id (db);
         db.modify(option, [&](bitlender_option_object& p) 
         {
           p.gateways = ass.whitelist_gateways;            
         });
      }
   }
 }
void_result gateway_create_evaluator::do_evaluate( const gateway_create_operation& op )
{ try {
      const auto account = op.gateway_account(db());
      FC_ASSERT(account.is_lifetime_member());
      FC_ASSERT((account.uaccount_property & account_gateway) == 0,"${x}",("x",account.uaccount_property));
      if (op.fee.amount > 1)
      {
         lock_fees = db().get_fee(op, 1);
         FC_ASSERT(db().get_balance(op.gateway_account, op.fee.asset_id) >= lock_fees + op.fee, "fee  ${ptype}", ("ptype", lock_fees + op.fee));   
   }      
   for(auto &ass: op.allowed_asset)
     db().get(ass);

   FC_ASSERT(!account.is_lock_balance());  
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type gateway_create_evaluator::do_apply( const gateway_create_operation& op )
{ try {
   vote_id_type vote_id;
   db().modify(db().get_global_properties(), [&vote_id](global_property_object& p) {
      vote_id = get_next_vote_id(p, vote_id_type::gateway);
   });

   auto &account = op.gateway_account(db());

   const auto& new_gateway_dynamic = db().create<gateway_dynamic_object>( [&]( gateway_dynamic_object& obj ){
   });   
   const auto& new_gateway_object = db().create<gateway_object>( [&]( gateway_object& obj ){
         obj.name             = account.name;
         obj.gateway_account  = op.gateway_account;       
         obj.need_auth        = op.need_auth;
         obj.vote_id          = vote_id;
         obj.url              = op.url;
         obj.memo             = op.memo;
         obj.lock_asset       = lock_fees;
         obj.dynamic_id = new_gateway_dynamic.id;
         do_allowed_asset(db(), obj, op.allowed_asset);
   });  
   
   db().modify(account, [&](account_object& p) {
      p.uaccount_property |= account_gateway;
   });

   if(lock_fees.amount >0)
   {
    db().adjust_balance(64,"",op.gateway_account,- lock_fees);
    auto account_statistics = account.statistics(db()); 
    db().modify(account_statistics, [&](account_statistics_object& p) {      
       p.lock_asset += lock_fees;
     });
   }  

   return new_gateway_object.id;

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result gateway_update_evaluator::do_evaluate( const gateway_update_operation& op )
{ try {
   FC_ASSERT(db().get(op.gateway).gateway_account == op.gateway_account);
   if( op.allowed_asset.valid())
   {   
      for(auto &ass: *op.allowed_asset)
        db().get(ass);
   }
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result gateway_update_evaluator::do_apply( const gateway_update_operation& op )
{ try {
   database& _db = db();
   _db.modify(_db.get(op.gateway),[&]( gateway_object& wit )
  {
         if(op.need_auth.valid())
            wit.need_auth = *op.need_auth;
         if( op.new_url.valid() )
            wit.url = *op.new_url;        
          if( op.new_memo.valid() )
            wit.memo = *op.new_memo; 
         if( op.allowed_asset.valid() )
            do_allowed_asset( db(),wit,*op.allowed_asset);
      });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

 


} } // graphene::chain
