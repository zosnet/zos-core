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
#include <graphene/chain/protocol/operations.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/database.hpp>

namespace graphene { namespace chain {

   class bitlender_option_create_evaluator : public evaluator<bitlender_option_create_evaluator>
   {
      public:
         typedef bitlender_option_create_operation operation_type;

         void_result do_evaluate( const bitlender_option_create_operation& o );
         object_id_type do_apply( const bitlender_option_create_operation& o );

         /** override the default behavior defined by generic_evalautor which is to
          * post the fee to fee_paying_account_stats.pending_fees */
      private:   
         flat_set<gateway_id_type>  gateways;       
   };
    class bitlender_option_author_evaluator : public evaluator<bitlender_option_author_evaluator>
   {
      public:
         typedef bitlender_option_author_operation operation_type;

         void_result do_evaluate( const bitlender_option_author_operation& o );
         void_result do_apply( const bitlender_option_author_operation& o );

         /** override the default behavior defined by generic_evalautor which is to
          * post the fee to fee_paying_account_stats.pending_fees */
      
   };

   class bitlender_option_update_evaluator : public evaluator<bitlender_option_update_evaluator>
   {
      public:
         typedef bitlender_option_update_operation operation_type;

         void_result do_evaluate( const bitlender_option_update_operation& o );
         void_result do_apply( const bitlender_option_update_operation& o );

      private:   
         flat_set<gateway_id_type>  gateways;             
   };
      class bitlender_option_stop_evaluator : public evaluator<bitlender_option_stop_evaluator>
   {
      public:
         typedef bitlender_option_stop_operation operation_type;

         void_result do_evaluate( const bitlender_option_stop_operation& o );
         void_result do_apply( const bitlender_option_stop_operation& o );

      private:   
         flat_set<gateway_id_type>  gateways;             
   };
     class bitlender_option_fee_mode_evaluator : public evaluator<bitlender_option_fee_mode_evaluator>
   {
      public:
         typedef bitlender_option_fee_mode_operation operation_type;

         void_result do_evaluate( const bitlender_option_fee_mode_operation& o );
         void_result do_apply( const bitlender_option_fee_mode_operation& o );                  
   };

   class bitlender_paramers_update_evaluator : public evaluator<bitlender_paramers_update_evaluator>
   {
      public:
         typedef bitlender_paramers_update_operation operation_type;

         void_result do_evaluate( const bitlender_paramers_update_operation& o );
         void_result do_apply( const bitlender_paramers_update_operation& o );

      private:   
         flat_set<gateway_id_type>  gateways;             
   };
   class bitlender_rate_update_evaluator : public evaluator<bitlender_rate_update_evaluator>
   {
      public:
         typedef bitlender_rate_update_operation operation_type;

         void_result do_evaluate( const bitlender_rate_update_operation& o );
         void_result do_apply( const bitlender_rate_update_operation& o );   
      private:
         bitlender_key bt_key;       
   };

   class  bitlender_update_feed_producers_evaluator : public evaluator<bitlender_update_feed_producers_evaluator>
   {
      public:
         typedef bitlender_update_feed_producers_operation operation_type;

         void_result do_evaluate( const operation_type& o );
         void_result do_apply( const operation_type& o );       
 
   };


   class bitlender_publish_feeds_evaluator : public evaluator<bitlender_publish_feeds_evaluator>
   {
      public:
         typedef bitlender_publish_feed_operation operation_type;

         void_result do_evaluate( const bitlender_publish_feed_operation& o );
         void_result do_apply( const bitlender_publish_feed_operation& o ); 

         const asset_exchange_feed_object* feed_update = nullptr;

         
   };



} } // graphene::chain
