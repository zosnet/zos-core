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
#include <graphene/chain/committee_member_evaluator.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <graphene/chain/protocol/vote.hpp>
#include <graphene/chain/transaction_evaluation_state.hpp>

#include <fc/smart_ref_impl.hpp>

namespace graphene { namespace chain {

void_result committee_member_create_evaluator::do_evaluate( const committee_member_create_operation& op )
{ try {
   FC_ASSERT(db().get(op.committee_member_account).is_lifetime_member());
   FC_ASSERT((op.committee_member_account(db()).uaccount_property & account_committe) == 0);
   if(op.fee.amount >1)
   {
      lock_fees = db().get_fee(op,1);
     FC_ASSERT(db().get_balance(op.committee_member_account,op.fee.asset_id)>= lock_fees,"lock fee  ${ptype}", ("ptype", lock_fees) );    
   }
    FC_ASSERT(!op.committee_member_account(db()).is_lock_balance()); 
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type committee_member_create_evaluator::do_apply( const committee_member_create_operation& op )
{ try {
   vote_id_type vote_id;
   db().modify(db().get_global_properties(), [&vote_id](global_property_object& p) {
      vote_id = get_next_vote_id(p, vote_id_type::committee);
   });
   auto &account = op.committee_member_account(db());
   const auto& new_del_object = db().create<committee_member_object>( [&]( committee_member_object& obj ){
         obj.name               = account.name;
         obj.committee_member_account   = op.committee_member_account;
         obj.vote_id            = vote_id;
         obj.url                = op.url;
         obj.memo               = op.memo;
         obj.lock_asset         = lock_fees;
   });   
   db().modify(account, [&](account_object& p) {
      p.uaccount_property |= account_committe;
   });

   if(lock_fees.amount >0)
   {
    db().adjust_balance(42,"",op.committee_member_account,- lock_fees);
    auto account_statistics = account.statistics(db()); 
    db().modify(account_statistics, [&](account_statistics_object& p) {      
       p.lock_asset += lock_fees;
     });
   }  

   return new_del_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result committee_member_update_evaluator::do_evaluate( const committee_member_update_operation& op )
{ try {
   FC_ASSERT(db().get(op.committee_member).committee_member_account == op.committee_member_account);
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result committee_member_update_evaluator::do_apply( const committee_member_update_operation& op )
{ try {
   database& _db = db();
   _db.modify(
      _db.get(op.committee_member),
      [&]( committee_member_object& com )
      {
         if( op.new_url.valid() )
            com.url = *op.new_url;
         if( op.new_memo.valid() )
            com.memo = *op.new_memo;   
      });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result committee_member_update_global_parameters_evaluator::do_evaluate(const committee_member_update_global_parameters_operation& o)
{ try {
   FC_ASSERT(trx_state->_is_proposed_trx);
   
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result committee_member_update_global_parameters_evaluator::do_apply(const committee_member_update_global_parameters_operation& o)
{ try {
   db().modify(db().get_global_properties(), [&o](global_property_object& p) {
      p.pending_parameters = o.new_parameters;
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result committee_member_update_zos_parameters_evaluator::do_evaluate(const committee_member_update_zos_parameters_operation& o)
{ try {
   FC_ASSERT(trx_state->_is_proposed_trx);
   variant obj = fc::json::from_string(o.new_parameters);       
   from_variant( obj, _parameters, GRAPHENE_MAX_NESTED_OBJECTS );
   _parameters.validate();
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result committee_member_update_zos_parameters_evaluator::do_apply(const committee_member_update_zos_parameters_operation& o)
{ try {
   db().modify(db().get_global_properties(), [this](global_property_object& p) {
      p.pending_zosparameters = this->_parameters;
   });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }


 



} } // graphene::chain
