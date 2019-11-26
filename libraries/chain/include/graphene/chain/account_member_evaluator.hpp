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
#include <graphene/chain//protocol/account_member.hpp>

namespace graphene { namespace chain {


   class gateway_issue_currency_evaluator : public evaluator<gateway_issue_currency_evaluator>
   {
      public:
         typedef gateway_issue_currency_operation operation_type;

         void_result do_evaluate( const gateway_issue_currency_operation& o );
         void_result do_apply( const gateway_issue_currency_operation& o );
   };

   class gateway_withdraw_evaluator : public evaluator<gateway_withdraw_evaluator>
   {
      public:
         typedef gateway_withdraw_operation operation_type;

         void_result do_evaluate( const gateway_withdraw_operation& o );
         void_result do_apply( const gateway_withdraw_operation& o );
   };
 
   class gateway_deposit_evaluator : public evaluator<gateway_deposit_evaluator>
   {
      public:
         typedef gateway_deposit_operation operation_type;

         void_result do_evaluate( const gateway_deposit_operation& o );
         void_result do_apply( const gateway_deposit_operation& o );
   };

  
  int32_t get_can_withdraw(const database &d,const gateway_object & ca ,const account_object &a,const asset_id_type &aid);
  int32_t get_can_deposit(const database &d,const gateway_object & ca ,const account_object &a,const asset_id_type &aid);

} } // graphene::chain
