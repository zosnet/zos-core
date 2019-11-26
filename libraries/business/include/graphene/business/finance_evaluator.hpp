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
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/protocol/types.hpp>

namespace graphene { namespace chain {

   class  account_object;
   class  asset_object;
   class  asset_bitasset_data_object;


   class finance_calc  
   {
      public:       
        //处理订单
        static const void enable_buy_fundraise( database &d, const buy_fundraise_object &ob, uint32_t benable);
        static const void enable_buy_exchange (database &d, const buy_exchange_object &ob, const sell_exchange_id_type sell_id,bool enable);
        static const void enable_sell_exchange(database &d, const sell_exchange_object &ob, bool enable);
   };
   class issue_fundraise_create_evaluator : public evaluator<issue_fundraise_create_evaluator>
   {
      public:
         typedef  issue_fundraise_create_operation operation_type;

         void_result do_evaluate( const  issue_fundraise_create_operation& o );
         object_id_type do_apply( const  issue_fundraise_create_operation& o );      

         /** override the default behavior defined by generic_evalautor which is to
          * post the fee to fee_paying_account_stats.pending_fees */
      
   };     

   class issue_fundraise_update_evaluator : public evaluator<issue_fundraise_update_evaluator>
   {
      public:
         typedef  issue_fundraise_update_operation operation_type;

         void_result do_evaluate( const  issue_fundraise_update_operation& o );
         void_result do_apply( const  issue_fundraise_update_operation& o );
       
   };

      class issue_fundraise_remove_evaluator : public evaluator<issue_fundraise_remove_evaluator>
   {
      public:
         typedef  issue_fundraise_remove_operation operation_type;

         void_result do_evaluate( const  issue_fundraise_remove_operation& o );
         void_result do_apply( const  issue_fundraise_remove_operation& o );

       
   };

     class issue_fundraise_publish_feed_evaluator : public evaluator<issue_fundraise_publish_feed_evaluator>
   {
      public:
         typedef issue_fundraise_publish_feed_operation operation_type;

         void_result do_evaluate( const issue_fundraise_publish_feed_operation& o );
         void_result do_apply( const issue_fundraise_publish_feed_operation& o ); 
      
   };


   class finance_paramers_update_evaluator : public evaluator<finance_paramers_update_evaluator>
   {
      public:
         typedef finance_paramers_update_operation operation_type;

         void_result do_evaluate( const finance_paramers_update_operation& o );
         void_result do_apply( const finance_paramers_update_operation& o );                  
   }; 
   
   class buy_fundraise_create_evaluator : public evaluator<buy_fundraise_create_evaluator>
   {
      public:
         typedef buy_fundraise_create_operation operation_type;

         void_result do_evaluate( const buy_fundraise_create_operation& o );
         object_id_type do_apply( const buy_fundraise_create_operation& o );
      private:
        price buy_price;
   };
   class buy_fundraise_enable_evaluator : public evaluator<buy_fundraise_enable_evaluator>
   {
      public:
         typedef buy_fundraise_enable_operation operation_type;

         void_result do_evaluate( const buy_fundraise_enable_operation& o );
         void_result do_apply( const buy_fundraise_enable_operation& o );

       
   }; 

   class sell_exchange_create_evaluator : public evaluator<sell_exchange_create_evaluator>
   {
      public:
         typedef  sell_exchange_create_operation operation_type;

         void_result do_evaluate( const  sell_exchange_create_operation& o );
         object_id_type do_apply( const  sell_exchange_create_operation& o );
      private:
         bool exchange = false;
   };
   class sell_exchange_update_evaluator : public evaluator<sell_exchange_update_evaluator>
   {
      public:
         typedef  sell_exchange_update_operation operation_type;

         void_result do_evaluate( const  sell_exchange_update_operation& o );
         void_result do_apply( const  sell_exchange_update_operation& o );
   };
   class sell_exchange_remove_evaluator : public evaluator<sell_exchange_remove_evaluator>
   {
      public:
         typedef  sell_exchange_remove_operation operation_type;

         void_result do_evaluate( const  sell_exchange_remove_operation& o );
         void_result do_apply( const  sell_exchange_remove_operation& o );
   };
   class buy_exchange_create_evaluator : public evaluator<buy_exchange_create_evaluator>
   {
      public:
         typedef  buy_exchange_create_operation operation_type;

         void_result do_evaluate( const  buy_exchange_create_operation& o );
         object_id_type do_apply( const  buy_exchange_create_operation& o );
   };
   class buy_exchange_update_evaluator : public evaluator<buy_exchange_update_evaluator>
   {
      public:
         typedef  buy_exchange_update_operation operation_type;

         void_result do_evaluate( const  buy_exchange_update_operation& o );
         void_result do_apply( const  buy_exchange_update_operation& o );
   };
   class buy_exchange_remove_evaluator : public evaluator<buy_exchange_remove_evaluator>
   {
      public:
         typedef  buy_exchange_remove_operation operation_type;

         void_result do_evaluate( const  buy_exchange_remove_operation& o );
         void_result do_apply( const  buy_exchange_remove_operation& o );
   };

} } // graphene::chain
