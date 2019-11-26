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
#include <graphene/chain/carrier_evaluator.hpp>
#include <graphene/chain/carrier_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/protocol/vote.hpp>

namespace graphene { namespace chain {

void_result carrier_create_evaluator::do_evaluate( const carrier_create_operation& op )
{ try {
      const auto &account = op.carrier_account(db());
      FC_ASSERT(account.is_lifetime_member());
      FC_ASSERT((account.uaccount_property & account_carrier) == 0,"${x}",("x",account.uaccount_property));
      if (op.fee.amount > 1)
      {
         lock_fees = db().get_fee(op, 1);
         FC_ASSERT(db().get_balance(op.carrier_account, op.fee.asset_id) >= lock_fees + op.fee, "fee  ${ptype}", ("ptype", lock_fees + op.fee));    
      }
      FC_ASSERT(!account.is_lock_balance());   
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type carrier_create_evaluator::do_apply( const carrier_create_operation& op )
{ try {
   vote_id_type vote_id;
   db().modify(db().get_global_properties(), [&vote_id](global_property_object& p) {
      vote_id = get_next_vote_id(p, vote_id_type::carrier);
   });

   auto &account = op.carrier_account(db());

   const auto& new_carrier_dynamic = db().create<carrier_dynamic_object>( [&]( carrier_dynamic_object& obj ){
    });
   const auto &new_carrier_object = db().create<carrier_object>([&](carrier_object &obj) {
      obj.name = account.name;
      obj.carrier_account = op.carrier_account;
      obj.need_auth = op.need_auth;
      obj.vote_id = vote_id;
      obj.url = op.url;
      obj.memo = op.memo;
      if (op.config.valid())
         obj.config = *op.config;
      obj.lock_asset = lock_fees;
      obj.dynamic_id = new_carrier_dynamic.id;
      obj.op_type    = carrier_op_invest | carrier_op_loan;
   });

   
   db().modify(account, [&](account_object& p) {
      p.uaccount_property |= account_carrier;
   });

   if(lock_fees.amount >0)
   {
    db().adjust_balance(41,"",op.carrier_account,- lock_fees);
    auto account_statistics = account.statistics(db()); 
    db().modify(account_statistics, [&](account_statistics_object& p) {      
       p.lock_asset += lock_fees;
     });
   }  

   return new_carrier_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result carrier_update_evaluator::do_evaluate( const carrier_update_operation& op )
{ try {
   FC_ASSERT(db().get(op.carrier).carrier_account == op.carrier_account);
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result carrier_update_evaluator::do_apply( const carrier_update_operation& op )
{ try {
   database& _db = db();
   _db.modify(
      _db.get(op.carrier),
      [&]( carrier_object& wit )
      {
         if( op.need_auth.valid() )
            wit.need_auth = *op.need_auth;
         if( op.new_url.valid() )
            wit.url = *op.new_url;        
         if( op.new_memo.valid() )
            wit.memo = *op.new_memo;    
          if( op.new_config.valid() )
            wit.config = *op.new_config;    
      });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

 
} } // graphene::chain
