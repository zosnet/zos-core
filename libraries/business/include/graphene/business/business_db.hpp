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
#pragma once

#include <graphene/chain/global_property_object.hpp>
#include <graphene/chain/node_property_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/fork_database.hpp>
#include <graphene/chain/block_database.hpp>
#include <graphene/chain/genesis_state.hpp>
#include <graphene/chain/evaluator.hpp>

#include <graphene/db/object_database.hpp>
#include <graphene/db/object.hpp>
#include <graphene/db/simple_index.hpp>
#include <fc/signals.hpp>

#include <graphene/chain/protocol/protocol.hpp>

#include <fc/log/logger.hpp>

#include <map>
 

namespace graphene { namespace chain {
   using graphene::db::abstract_object;
   using graphene::db::object;   
   class op_evaluator;
   class database;
   class transaction_evaluation_state;

   class business_db  
   {
      public:
         business_db(database *db): _db(db) {}
         ~business_db() {}    

      public: 

         void     pay_locktoken(const asset &rec);
         asset    get_locktoken_budget();

        //block update 
         void clear_expired_orders();
         void check_exchange_orders();
         void clear_expired_exchange();
         void check_hour_task();
         void clear_expired_bitlender();
         void clear_expired_fundraise();         
         void clear_expired_locktoken();
         void check_dy_locktoken();


         void update_bitlender_expired_feeds();
        //////////////////// db_market.cpp ////////////////////

         /// @{ @group Market Helpers
         void globally_settle_asset( const asset_object& bitasset, const price& settle_price );
         void cancel_settle_order(const force_settlement_object& order, bool create_virtual_op = true);
         void cancel_limit_order(const limit_order_object& order, bool create_virtual_op, bool skip_cancel_fee);
         void revive_bitasset( const asset_object& bitasset );
         void cancel_bid(const collateral_bid_object& bid, bool create_virtual_op = true);
         void execute_bid( const collateral_bid_object& bid, share_type debt_covered, share_type collateral_from_fund, const price_feed& current_feed );
         void cancel_loan(const bitlender_order_id_type& orderid);
         void remove_loan(const bitlender_order_id_type& orderid);
         void remove_locktoken(const locktoken_id_type& orderid);
         void remove_limit_order(const limit_order_id_type &orderid);
         void recycle_loan(const bitlender_order_object& order);
         void overdue_loan(const bitlender_order_object& order);
         void remove_buy_fundraise(const buy_fundraise_id_type& id);
         void remove_issue_fundraise(const issue_fundraise_id_type& id);
         void enable_buy_fundraise(const buy_fundraise_id_type& orderid);
         void remove_buy_exchange(const buy_exchange_id_type& id);
         void remove_sell_exchange(const sell_exchange_id_type& id);
         void auto_repayment(const bitlender_order_object& order,int iperiod, transaction_evaluation_state &state);
         bool check_for_blackswan( const asset_object& mia, bool enable_black_swan = true );
         /**
          * @brief Process a new limit order through the markets
          * @param order The new order to process
          * @return true if order was completely filled; false otherwise
          *
          * This function takes a new limit order, and runs the markets attempting to match it with existing orders
          * already on the books.
          */
         bool apply_order(const limit_order_object& new_order_object);
         bool apply_order_full(const limit_order_object& new_order_object);

         /**
          * Matches the two orders,
          *
          * @return a bit field indicating which orders were filled (and thus removed)
          *
          * 0 - no orders were matched
          * 1 - bid was filled
          * 2 - ask was filled
          * 3 - both were filled
          */
         ///@{
         int match( const limit_order_object& taker, const limit_order_object& maker, const price& trade_price );
         int match( const limit_order_object& taker, const call_order_object& maker, const price& trade_price,
                    const price& feed_price, const uint16_t maintenance_collateral_ratio,
                    const optional<price>& maintenance_collateralization );
         ///@}

         /// Matches the two orders, the first parameter is taker, the second is maker.
         /// @return the amount of asset settled
         asset match(const call_order_object& call,
                   const force_settlement_object& settle,
                   const price& match_price,
                   asset max_settlement,
                   const price& fill_price);

         /**
          * @return true if the order was completely filled and thus freed.
          */
         bool fill_limit_order( const limit_order_object& order, const asset& pays, const asset& receives, bool cull_if_small,
                                const price& fill_price, const bool is_maker );
         bool fill_call_order( const call_order_object& order, const asset& pays, const asset& receives,
                               const price& fill_price, const bool is_maker );
         bool fill_settle_order( const force_settlement_object& settle, const asset& pays, const asset& receives,
                                 const price& fill_price, const bool is_maker );

         bool check_call_orders( const asset_object& mia, bool enable_black_swan = true, bool for_new_limit_order = false );

         void check_bitlender_orders(const asset_id_type &base_check,const asset_id_type &quote_check);
         void check_exchange_orders(const asset_id_type &base_check,const asset_id_type &quote_check);

         // helpers to fill_order
         void pay_order( const account_object& receiver, const asset& receives,const asset& fee, const asset& pays,const limit_order_object& order);

         asset calculate_market_fee(const asset_object& recv_asset, const asset& trade_amount);
         asset pay_market_fees( const asset_object& recv_asset, const asset& receives );
         asset pay_market_fees( const account_object& seller, const asset_object& recv_asset, const asset& receives );
         ///@}

         void  _cancel_bids_and_revive_mpa( const asset_object& bitasset, const asset_bitasset_data_object& bad );



         void  process_bids( const asset_bitasset_data_object& bad );
      private:
        database *_db;
   };

 
} }
 