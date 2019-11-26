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

namespace graphene { namespace chain {
   class fclocktoken {
    public:
       static const locktoken_option_object &get_option(const database &d, const asset_id_type asset, const uint32_t lock_mode);
       static void locktoken_paybacklocked(database &d, const account_id_type &act, const asset &locked,  const locktoken_object &obj);
       static void locktoken_paybackinterest(database &d, const account_id_type &act, const asset &locked, const locktoken_object &obj, bool doPayer);
       static void locktoken_changelockedto(database &d, const account_id_type &act, const asset &locked);
       static void locktoken_vesting(database &d, const account_id_type &act, const asset &locked, const uint32_t period);
       static price locktoken_getprice(const database &d, const asset_object &locked,  const asset_id_type &payasset);       
       static asset locktoken_getpay(const database &d, const asset &locked,  const asset_id_type &payasset, price &getprice);       
       static uint32_t locktoken_getrate(const locktoken_option_object &ass, const uint32_t period, const share_type amount);
       static bool get_canchange(database &d, locktoken_object &obj,uint32_t type, bool bsetdata) ;
       static uint32_t get_node_level(const locktoken_option_object &option, const asset &locked);
   };

   class locktoken_option_evaluator : public evaluator<locktoken_option_evaluator>
   {
      public:
         typedef locktoken_option_operation operation_type;
         void_result do_evaluate( const locktoken_option_operation& o );
         void_result do_apply( const locktoken_option_operation& o ); 

      private:
         locktoken_options _parameters;
         locktoken_option_id_type _option_id;
   };

   class locktoken_create_evaluator : public evaluator<locktoken_create_evaluator>
   {
      public:
         typedef locktoken_create_operation operation_type;

         void_result do_evaluate( const operation_type& o );
         object_id_type do_apply( const operation_type& o );
      private:
         uint32_t                 _rate = 0;   
         asset_id_type            _pay_asset;
         account_id_type          _payer;
         account_id_type          _carrier;
         locktoken_option_id_type _option_id;
         price                    _price;
         uint32_t                 _vesting_seconds;
         share_type               _max_rate;
         asset                    _buy_asset;
         uint32_t                 _level; 

   };
   class locktoken_update_evaluator : public evaluator<locktoken_update_evaluator>
   {
      public:
         typedef locktoken_update_operation operation_type;

         void_result do_evaluate( const operation_type& o );
         void_result do_apply( const operation_type& o );
      private:
         uint32_t _rate = 0;  
         locktoken_option_id_type _option_id; 
   };
   class locktoken_node_evaluator : public evaluator<locktoken_node_evaluator>
   {
      public:
         typedef locktoken_node_operation operation_type;

         void_result do_evaluate( const operation_type& o );
         void_result do_apply( const operation_type& o );
       private:
         locktoken_id_type _to_obj;  
   };
   class locktoken_remove_evaluator : public evaluator<locktoken_remove_evaluator>
   {
      public:
         typedef locktoken_remove_operation operation_type;

         void_result do_evaluate( const operation_type& o );
         void_result do_apply( const operation_type& o );
   };

} } // graphene::chain
