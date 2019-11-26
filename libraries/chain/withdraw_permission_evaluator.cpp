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
#include <graphene/chain/withdraw_permission_evaluator.hpp>
#include <graphene/chain/withdraw_permission_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/is_authorized_asset.hpp>
#include <graphene/chain/account_member_evaluator.hpp>
#include <graphene/chain/gateway_object.hpp>
#include <graphene/chain/carrier_object.hpp>

namespace graphene { namespace chain {

void_result withdraw_permission_create_evaluator::do_evaluate(const operation_type& op)
{ try {
   database& d = db();

   auto &t = op.authorized_account(d);  
   auto &f = op.withdraw_from_account(d);  
   auto &a = op.withdrawal_limit.asset_id(d);
   
   FC_ASSERT(d.find_object(op.withdraw_from_account));
   FC_ASSERT(d.find_object(op.authorized_account));
   FC_ASSERT(d.find_object(op.withdrawal_limit.asset_id));
   FC_ASSERT(op.period_start_time > d.head_block_time());
   FC_ASSERT(op.period_start_time + op.periods_until_expiration * op.withdrawal_period_sec > d.head_block_time());
   FC_ASSERT(op.withdrawal_period_sec >= d.get_global_properties().parameters.block_interval);
   FC_ASSERT(!op.withdraw_from_account(d).is_lock_balance());   

   op.validate();

   if(op.authorized_identify & account_gateway) 
   {
      const auto& idx = d.get_index_type<gateway_index>().indices().get<by_account>();
      auto itr = idx.find(op.authorized_account);
      FC_ASSERT(itr != idx.end());

      FC_ASSERT(a.is_loan_issued() || a.is_lender_issued(), " asset '${id1}' must be loan or lender  ", ("id1", op.withdrawal_limit));
      FC_ASSERT(t.is_gateway_member(), " account '${id1}' must be gateway member  ", ("id1", op.authorized_account));
      //检查网关是否有抵押币

      FC_ASSERT(get_can_withdraw(d,t.get_gateway_id(d)(d), f,op.withdrawal_limit.asset_id) > 0);
      FC_ASSERT(t.is_gateway_enable(d, op.withdrawal_limit.asset_id), "gateway '${id1}' is not authority deposit ", ("id1", t.id));
    }
    if(op.authorized_identify & account_carrier) 
    {
      const auto& idx = d.get_index_type<carrier_index>().indices().get<by_account>();
      auto itr = idx.find(op.authorized_account);
      FC_ASSERT(itr != idx.end());

      FC_ASSERT(a.is_loan_issued() || a.is_lender_issued(), " asset '${id1}' must be loan or lender  ", ("id1", op.withdrawal_limit));
      FC_ASSERT(t.is_carrier_member(), " account '${id1}' must be carrier member  ", ("id1", op.authorized_account));
      //检查网关是否有抵押币     
      FC_ASSERT(t.is_carrier_enable(d), "gateway '${id1}' is not authority deposit ", ("id1", t.id));     
    }

    share_type ubalance = d.get_balance(op.withdraw_from_account, op.withdrawal_limit.asset_id).amount;
    FC_ASSERT(ubalance >= op.withdrawal_limit.amount, "insufficient_balance '${id1}' < '${id2}'", ("id1", ubalance)("id2", op.withdrawal_limit.amount));

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type withdraw_permission_create_evaluator::do_apply(const operation_type& op)
{ try {
   return db().create<withdraw_permission_object>([&op](withdraw_permission_object& p) {
      p.withdraw_from_account = op.withdraw_from_account;
      p.authorized_account = op.authorized_account;
      p.withdrawal_limit = op.withdrawal_limit;
      p.withdrawal_period_sec = op.withdrawal_period_sec;
      p.expiration = op.period_start_time + op.periods_until_expiration * op.withdrawal_period_sec;
      p.period_start_time = op.period_start_time;
      p.authorized_identify = op.authorized_identify;
   }).id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result withdraw_permission_claim_evaluator::do_evaluate(const withdraw_permission_claim_evaluator::operation_type& op)
{ try {
   const database& d = db();
   time_point_sec head_block_time = d.head_block_time();   
   const withdraw_permission_object& permit = op.withdraw_permission(d);
   FC_ASSERT(permit.expiration > head_block_time);
   FC_ASSERT(permit.authorized_account == op.withdraw_to_account);
   FC_ASSERT(permit.withdraw_from_account == op.withdraw_from_account);
   FC_ASSERT(permit.withdrawal_limit.asset_id == op.amount_to_withdraw.asset_id);   
   FC_ASSERT(permit.period_start_time <= head_block_time);   
   FC_ASSERT(op.amount_to_withdraw <= permit.available_this_period( head_block_time ) );
   FC_ASSERT(d.get_balance(op.withdraw_from_account, op.amount_to_withdraw.asset_id) >= op.amount_to_withdraw);
   const asset_object& _asset = op.amount_to_withdraw.asset_id(d);
   if( _asset.is_transfer_restricted() ) FC_ASSERT( _asset.issuer == permit.authorized_account || _asset.issuer == permit.withdraw_from_account );
   const account_object& from  = op.withdraw_to_account(d);
   const account_object& to    = permit.authorized_account(d);
   FC_ASSERT(!from.is_lock_balance()); 
   FC_ASSERT(!to.is_lock_balance()); 
   
   FC_ASSERT( is_authorized_asset( d, to, _asset ) );
   FC_ASSERT( is_authorized_asset( d, from, _asset ) );
   if(_asset.is_cash_issued()) {
      FC_ASSERT(to.is_gateway_member(), "must be gateway member");
   }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result withdraw_permission_claim_evaluator::do_apply(const withdraw_permission_claim_evaluator::operation_type& op)
{ try {
   database& d = db();

   const withdraw_permission_object& permit = d.get(op.withdraw_permission);
   d.modify(permit, [&](withdraw_permission_object& p) {
      auto periods = (d.head_block_time() - p.period_start_time).to_seconds() / p.withdrawal_period_sec;
      p.period_start_time += periods * p.withdrawal_period_sec;      
      p.claimed_total += op.amount_to_withdraw.amount;
      if( periods == 0 )
         p.claimed_this_period += op.amount_to_withdraw.amount;
      else
         p.claimed_this_period = op.amount_to_withdraw.amount;
   });

   d.adjust_balance(76,"",op.withdraw_from_account, -op.amount_to_withdraw);
   d.adjust_balance(77,"",op.withdraw_to_account, op.amount_to_withdraw);

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result withdraw_permission_update_evaluator::do_evaluate(const withdraw_permission_update_evaluator::operation_type& op)
{ try {
   database& d = db();   
   const withdraw_permission_object& permit = op.permission_to_update(d);
   FC_ASSERT(permit.authorized_account == op.authorized_account);
   FC_ASSERT(permit.withdraw_from_account == op.withdraw_from_account);
   FC_ASSERT(permit.withdrawal_limit.asset_id == op.withdrawal_limit.asset_id);
   FC_ASSERT(d.find_object(op.withdrawal_limit.asset_id));
   FC_ASSERT(op.period_start_time >= d.head_block_time());
   FC_ASSERT(op.period_start_time + op.periods_until_expiration * op.withdrawal_period_sec > d.head_block_time());
   FC_ASSERT(op.withdrawal_period_sec >= d.get_global_properties().parameters.block_interval);
   FC_ASSERT(!op.withdraw_from_account(d).is_lock_balance()); 

    share_type ubalance = d.get_balance(op.withdraw_from_account, op.withdrawal_limit.asset_id).amount;
    FC_ASSERT(ubalance >= op.withdrawal_limit.amount, "insufficient_balance '${id1}' < '${id2}'", ("id1", ubalance)("id2", op.withdrawal_limit.amount));

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result withdraw_permission_update_evaluator::do_apply(const withdraw_permission_update_evaluator::operation_type& op)
{ try {
   database& d = db();

   d.modify(op.permission_to_update(d), [&op](withdraw_permission_object& p) {
      p.period_start_time = op.period_start_time;
      p.expiration = op.period_start_time + op.periods_until_expiration * op.withdrawal_period_sec;
      p.withdrawal_limit = op.withdrawal_limit;
      p.withdrawal_period_sec = op.withdrawal_period_sec;
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result withdraw_permission_delete_evaluator::do_evaluate(const withdraw_permission_delete_evaluator::operation_type& op)
{ try {
   database& d = db();   
   const withdraw_permission_object& permit = op.withdraw_permission(d);
   FC_ASSERT(permit.authorized_account == op.authorized_account);
   FC_ASSERT(permit.withdraw_from_account == op.withdraw_from_account);   
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result withdraw_permission_delete_evaluator::do_apply(const withdraw_permission_delete_evaluator::operation_type& op)
{ try {
   db().remove_withdraw_permission(op.withdraw_permission);
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // graphene::chain
