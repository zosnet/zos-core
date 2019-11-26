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
#include <graphene/chain/db_with.hpp>

#include <graphene/chain/asset_object.hpp>
#include <graphene/business/bitlender_option_object.hpp>
#include <graphene/chain/global_property_object.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/business/market_object.hpp>
#include <graphene/business/bitlender_object.hpp>
#include <graphene/business/finance_object.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/chain/worker_object.hpp>
#include <graphene/business/locktoken_object.hpp>
#include <graphene/chain/transaction_object.hpp>
#include <graphene/chain/withdraw_permission_object.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/carrier_object.hpp>
#include <graphene/chain/gateway_object.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>

#include <fc/uint128.hpp>

namespace graphene { namespace chain {

void database::update_global_dynamic_data( const signed_block& b )
{
   const dynamic_global_property_object& _dgp =
      dynamic_global_property_id_type(0)(*this);

   uint32_t missed_blocks = get_slot_at_time( b.timestamp );
   assert( missed_blocks != 0 );
   missed_blocks--;
   for( uint32_t i = 0; i < missed_blocks; ++i ) {
      const auto& witness_missed = get_scheduled_witness( i+1 )(*this);
      if(  witness_missed.id != b.witness ) {
         /*
         const auto& witness_account = witness_missed.witness_account(*this);
         if( (fc::time_point::now() - b.timestamp) < fc::seconds(30) )
            wlog( "Witness ${name} missed block ${n} around ${t}", ("name",witness_account.name)("n",b.block_num())("t",b.timestamp) );
            */
         if((i%100000 == 0) && (i > 0) )
             std::cerr << " missed " << i << "\n";   

         modify(witness_missed, [&](witness_object &w) {
           w.total_missed++;
           w.recent_missed++;
         });
      } 
   }

   // dynamic global properties updating
   modify( _dgp, [&]( dynamic_global_property_object& dgp ){
      if( BOOST_UNLIKELY( b.block_num() == 1 ) )
         dgp.recently_missed_count = 0;
      else if( _checkpoints.size() && _checkpoints.rbegin()->first >= b.block_num() )
         dgp.recently_missed_count = 0;
      else if( missed_blocks )
         dgp.recently_missed_count += GRAPHENE_RECENTLY_MISSED_COUNT_INCREMENT*missed_blocks;
      else if( dgp.recently_missed_count > GRAPHENE_RECENTLY_MISSED_COUNT_INCREMENT )
         dgp.recently_missed_count -= GRAPHENE_RECENTLY_MISSED_COUNT_DECREMENT;
      else if( dgp.recently_missed_count > 0 )
         dgp.recently_missed_count--;

      dgp.head_block_number = b.block_num();
      dgp.head_block_id = b.id();
      dgp.time = b.timestamp;
      dgp.current_witness = b.witness;
      dgp.recent_slots_filled = (
           (dgp.recent_slots_filled << 1)
           + 1) << missed_blocks;
      dgp.current_aslot += missed_blocks+1;
   });
   //idump(( _dgp.head_block_number));
   //idump(( _dgp.last_irreversible_block_num )  );
   if( !(get_node_properties().skip_flags & skip_undo_history_check) )
   {
      GRAPHENE_ASSERT( _dgp.head_block_number - _dgp.last_irreversible_block_num  < GRAPHENE_MAX_UNDO_HISTORY, undo_database_exception,
                 "The database does not have enough undo history to support a blockchain with so many missed blocks. "
                 "Please add a checkpoint if you would like to continue applying blocks beyond this point.",
                 ("last_irreversible_block_num",_dgp.last_irreversible_block_num)("head", _dgp.head_block_number)("head_id", _dgp.head_block_id)
                 ("recently_missed",_dgp.recently_missed_count)("max_undo",GRAPHENE_MAX_UNDO_HISTORY) );
   }

   _undo_db.set_max_size( _dgp.head_block_number - _dgp.last_irreversible_block_num + 1 );
   _fork_db.set_max_size( _dgp.head_block_number - _dgp.last_irreversible_block_num + 1 );
}

void database::update_signing_witness(const witness_object& signing_witness, const signed_block& new_block)
{
   const global_property_object& gpo = get_global_properties();
   const dynamic_global_property_object& dpo = get_dynamic_global_properties();
   uint64_t new_block_aslot = dpo.current_aslot + get_slot_at_time( new_block.timestamp );

   share_type witness_pay = std::min( gpo.parameters.witness_pay_per_block, dpo.miners_budget[miner_witness] );

   modify( dpo, [&]( dynamic_global_property_object& _dpo )
   {
      _dpo.miners_budget[miner_witness] -= witness_pay;
      _dpo.miners_payed[miner_witness]  += witness_pay;
   } );

   deposit_witness_pay( signing_witness, witness_pay );

   modify( signing_witness, [&]( witness_object& _wit )
   {
      _wit.last_aslot = new_block_aslot;
      _wit.last_confirmed_block_num = new_block.block_num();
   } );
}

void database::update_last_irreversible_block()
{
   const global_property_object& gpo = get_global_properties();
   const dynamic_global_property_object& dpo = get_dynamic_global_properties();

   vector< const witness_object* > wit_objs;
   wit_objs.reserve( gpo.active_witnesses.size() );
   for( const witness_id_type& wid : gpo.active_witnesses )
      wit_objs.push_back( &(wid(*this)) );

   static_assert( GRAPHENE_IRREVERSIBLE_THRESHOLD > 0 && GRAPHENE_IRREVERSIBLE_THRESHOLD<=GRAPHENE_100_PERCENT, "irreversible threshold must be nonzero" );

   // 1 1 1 2 2 2 2 2 2 2 -> 2     .7*10 = 7
   // 1 1 1 1 1 1 1 2 2 2 -> 1
   // 3 3 3 3 3 3 3 3 3 3 -> 3

   size_t offset = ((GRAPHENE_100_PERCENT - GRAPHENE_IRREVERSIBLE_THRESHOLD) * wit_objs.size() / GRAPHENE_100_PERCENT);

   std::nth_element( wit_objs.begin(), wit_objs.begin() + offset, wit_objs.end(),
      []( const witness_object* a, const witness_object* b )
      {
         return a->last_confirmed_block_num < b->last_confirmed_block_num;
      } );

   uint32_t new_last_irreversible_block_num = wit_objs[offset]->last_confirmed_block_num;

   if( new_last_irreversible_block_num > dpo.last_irreversible_block_num )
   {
      modify( dpo, [&]( dynamic_global_property_object& _dpo )
      {
         _dpo.last_irreversible_block_num = new_last_irreversible_block_num;
      } );
   }
}
void database::clear_expired_transactions()
{ try {
   //Look for expired transactions in the deduplication list, and remove them.
   //Transactions must have expired by at least two forking windows in order to be removed.
   auto& transaction_idx = static_cast<transaction_index&>(get_mutable_index(implementation_ids, impl_transaction_object_type));
   const auto& dedupe_index = transaction_idx.indices().get<by_expiration>();   
   while ((!dedupe_index.empty()) && (head_block_time() > dedupe_index.begin()->trx.expiration))  
     transaction_idx.remove(*dedupe_index.begin());
         
} FC_CAPTURE_AND_RETHROW() }

void database::clear_expired_proposals()
{
   const auto& proposal_expiration_index = get_index_type<proposal_index>().indices().get<by_expiration>();
   while( !proposal_expiration_index.empty() && proposal_expiration_index.begin()->expiration_time <= head_block_time() )
   {
      const proposal_object& proposal = *proposal_expiration_index.begin();
      processed_transaction result;
      try {
         if( proposal.is_authorized_to_execute(*this) )
         {
            result = push_proposal(proposal);
            //TODO: Do something with result so plugins can process it.
            continue;
         }
         else
         {
             modify(proposal, [&](proposal_object &p) {
               p.status    = 4; //过期
               p.errorinfo = "";
             });         
         }
      } catch( const fc::exception& e ) {
         elog("Failed to apply proposed transaction on its expiration. Deleting it.\n${proposal}\n${error}",
              ("proposal", proposal)("error", e.to_detail_string()));  
         modify(proposal, [&e](proposal_object &p) {
               p.status    = 2; //执行失败
               p.errorinfo = e.to_detail_string();
         });                
      }
      remove_proposal(proposal);
   }
}

/**
 *  let HB = the highest bid for the collateral  (aka who will pay the most DEBT for the least collateral)
 *  let SP = current median feed's Settlement Price 
 *  let LC = the least collateralized call order's swan price (debt/collateral)
 *
 *  If there is no valid price feed or no bids then there is no black swan.
 *
 *  A black swan occurs if MAX(HB,SP) <= LC
 */
void database::check_hour_task()
{
   _business_db->check_hour_task();
}

void database::update_expired_feeds()
{
   auto& asset_idx = get_index_type<asset_index>().indices().get<by_type>();
   auto itr = asset_idx.lower_bound( true /** market issued */ );
   while( itr != asset_idx.end() )
   {
      const asset_object& a = *itr;
      ++itr;
      FC_ASSERT(!a.is_core());        

      assert(a.is_market_issued());

      bool update_cer = false; 
      const asset_bitasset_data_object& b = a.bitasset_data(*this);
      bool feed_is_expired = b.feed_is_expired( head_block_time() );
      if( feed_is_expired )
      {
        const price_feed cur_feed = b.current_feed;
         modify(b, [this,&update_cer](asset_bitasset_data_object& a) 
         {
            a.update_median_feeds(head_block_time());
            if( a.need_to_update_cer() )
            {
               update_cer = true;
               a.asset_cer_updated = false;
               a.feed_cer_updated = false;
            }
         });
         if ( !b.current_feed.settlement_price.is_null() && !(cur_feed == b.current_feed))         
           _business_db->check_call_orders(b.current_feed.settlement_price.base.asset_id(*this));                
      }
      if( update_cer )
       {
         if( !b.current_feed.core_exchange_rate.is_null() &&  a.options.core_exchange_rate != b.current_feed.core_exchange_rate )
         modify(a, [&b](asset_object& a) {
            a.options.core_exchange_rate = b.current_feed.core_exchange_rate;
         });
       }
   }
}

void database::update_core_exchange_rates()
{
   const auto& idx = get_index_type<asset_bitasset_data_index>().indices().get<by_cer_update>();
   if( idx.begin() != idx.end() )
   {
      for( auto itr = idx.rbegin(); itr->need_to_update_cer(); itr = idx.rbegin() )
      {
         const asset_bitasset_data_object& b = *itr;
         const asset_object& a = b.asset_id( *this );
         if( a.options.core_exchange_rate != b.current_feed.core_exchange_rate )
         {
            modify( a, [&b]( asset_object& ao )
            {
               ao.options.core_exchange_rate = b.current_feed.core_exchange_rate;
            });
         }
         modify( b, []( asset_bitasset_data_object& abdo )
         {
            abdo.asset_cer_updated = false;
            abdo.feed_cer_updated = false;
         });
      }
   }
}

void database::update_maintenance_flag( bool new_maintenance_flag )
{
   modify( get_dynamic_global_properties(), [&]( dynamic_global_property_object& dpo )
   {
      auto maintenance_flag = dynamic_global_property_object::maintenance_flag;
      dpo.dynamic_flags =
           (dpo.dynamic_flags & ~maintenance_flag)
         | (new_maintenance_flag ? maintenance_flag : 0);
   } );
   return;
}
void  database::remove_withdraw_permission(const withdraw_permission_id_type &id)
{
   auto &withdraw = get(id);
   if(_app_status.has_object_history_plugin)
   {  
      fill_object_history_operation op;
      op.itype  = fill_object_withdraw_permission;
      op.id     = withdraw.id;
      op.issuer = withdraw.withdraw_from_account;
      op.data   = fc::raw::pack(withdraw);
      push_applied_operation(op);
   }
   remove(withdraw);
}
void database::update_withdraw_permissions()
{
  auto &permit_index = get_index_type<withdraw_permission_index>().indices().get<by_expiration>();
  vector<withdraw_permission_id_type> re;
  fc::time_point_sec now = head_block_time();
  auto itr = permit_index.begin();
  while (itr != permit_index.end())
  {
     if(itr->expiration <= now)
      re.push_back(itr->id);
     itr++;
  }
  for(const auto &id : re)
     remove_withdraw_permission(id);
  re.clear();
}

} }
