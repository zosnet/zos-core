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
#include <graphene/chain/database.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/is_authorized_asset.hpp>
#include <graphene/chain/transaction_evaluation_state.hpp>

#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/fba_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/business/market_evaluator.hpp>
#include <graphene/business/bitlender_evaluator.hpp>
#include <graphene/business/finance_evaluator.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>

#include <fc/uint128.hpp>

namespace graphene { namespace chain {
database& generic_evaluator::db()const { return trx_state->db(); }

   operation_result generic_evaluator::start_evaluate( transaction_evaluation_state& eval_state, const operation& op, bool apply )
   { try {
      trx_state   = &eval_state;
      //check_required_authorities(op);
      auto result = evaluate( op );

      if( apply ) result = this->apply( op );
      return result;
   } FC_CAPTURE_AND_RETHROW() }

   void generic_evaluator::prepare_fee(account_id_type account_id, asset fee)
   {
      const database& d = db();
      fee_from_account = fee;
      FC_ASSERT( fee.amount >= 0 );
      fee_paying_account = &account_id(d);
      fee_paying_account_statistics = &fee_paying_account->statistics(d);
      is_coupon_fee = false;
      is_pay_fee    = false;
      fee_asset = &fee.asset_id(d);
      fee_asset_dyn_data = &fee_asset->dynamic_asset_data_id(d);

      if(true)// d.head_block_time() > HZARDFORK_419_TIME )
      {
         FC_ASSERT( is_authorized_asset( d, *fee_paying_account, *fee_asset ), "Account ${acct} '${name}' attempted to pay fee by using asset ${a} '${sym}', which is unauthorized due to whitelist / blacklist",
            ("acct", fee_paying_account->id)("name", fee_paying_account->name)("a", fee_asset->id)("sym", fee_asset->symbol) );
      }

      if( fee_from_account.asset_id == GRAPHENE_CORE_ASSET )
         core_fee_paid = fee_from_account.amount;
      else
      {
        FC_ASSERT(false);//eric
         asset fee_from_pool = fee_from_account * fee_asset->options.core_exchange_rate;
         FC_ASSERT( fee_from_pool.asset_id == GRAPHENE_CORE_ASSET );
         core_fee_paid = fee_from_pool.amount;
         FC_ASSERT( core_fee_paid <= fee_asset_dyn_data->fee_pool, "Fee pool balance of '${b}' is less than the ${r} required to convert ${c}",
                    ("r", db().to_pretty_string( fee_from_pool))("b",db().to_pretty_string(asset(fee_asset_dyn_data->fee_pool,fee_from_pool.asset_id)))("c",db().to_pretty_string(fee)) );
      }
   }

   void generic_evaluator::convert_fee()
   {
      if( !trx_state->skip_fee ) {
         if( fee_asset->get_id() != GRAPHENE_CORE_ASSET )
         {
            db().modify(*fee_asset_dyn_data, [this](asset_dynamic_data_object& d) {
               d.accumulated_fees += fee_from_account.amount;
               d.fee_pool -= core_fee_paid;
            });
         }
      }
   }
 void generic_evaluator::rollback_fee()
   { try {
      if( !trx_state->skip_fee ) {
         database& d = db();
         is_pay_fee = false;
         d.modify(*fee_paying_account_statistics, [&](account_statistics_object& s)
         {                        
            if (is_coupon_fee)
            {             
               s.coupon_fees   -= core_fee_paid;          
            }
            else 
            {              
              s.pay_fee(GRAPHENE_CORE_ASSET,-core_fee_paid, d.get_global_properties().parameters.cashback_vesting_threshold);              
            }
         });
      }
   } FC_CAPTURE_AND_RETHROW() }
   void generic_evaluator::pay_fee()
   { try {
      if( !trx_state->skip_fee ) {
         database& d = db();
         is_pay_fee = true;
         d.modify(*fee_paying_account_statistics, [&](account_statistics_object& s)
         {
            s.activate_time = d.head_block_time();
            s.activate_count++;
            bool _set_coupon = get_type() == operation::tag<account_coupon_operation>::value;
            if (fee_from_account.asset_id == GRAPHENE_CORE_ASSET && (_set_coupon || s.amount_coupon >= core_fee_paid))
            {             
               s.coupon_fees   += core_fee_paid;
               is_coupon_fee = true;
            }
            else 
            {
              is_coupon_fee = false;
              s.pay_fee(GRAPHENE_CORE_ASSET,core_fee_paid, d.get_global_properties().parameters.cashback_vesting_threshold);              
            }
         });
      }
   } FC_CAPTURE_AND_RETHROW() }

   void generic_evaluator::pay_fba_fee( uint64_t fba_id )
   {
      FC_ASSERT(false);//erci
      database& d = db();
      const fba_accumulator_object& fba = d.get< fba_accumulator_object >( fba_accumulator_id_type( fba_id ) );    
      if (!fba.is_configured(d))
      {
         generic_evaluator::pay_fee();
         return;
      }
      d.modify( fba, [&]( fba_accumulator_object& _fba )
      {
         _fba.accumulated_fba_fees += core_fee_paid;
      } );
   }

   share_type generic_evaluator::calculate_fee_for_operation(const operation& op) const
   {
     return db().current_fee_schedule().calculate_fee( op ).amount;
   }
   void generic_evaluator::db_adjust_balance(const account_id_type& fee_payer, asset fee_from_account)
   {
     if(is_pay_fee)
     {
       if(is_coupon_fee)
       {
         db().modify(*fee_paying_account_statistics, [&](account_statistics_object& s)
         {           
           s.amount_coupon += fee_from_account.amount;               
         });
       }
       else 
        db().adjust_balance(60,"",fee_payer, fee_from_account);
     }
   }

} }
