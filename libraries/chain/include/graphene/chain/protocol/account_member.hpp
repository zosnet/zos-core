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

#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/chain_parameters.hpp>
#include <graphene/chain/protocol/memo.hpp>
 

namespace graphene { namespace chain { 

   
   //给网关发行货币
   struct gateway_issue_currency_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 10000;   };

      asset                                 fee;
      account_id_type                       issuer;
      account_id_type                       account_to;
      asset                                 issue_currency;
      bool                                  revoke = false;

      optional<memo_data>                   memo;
      extensions_type                       extensions;

      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type& k)const {return k.fee;}
       void get_required_active_authorities( flat_set<account_id_type>& a )const 
       {    
         a.clear();    
         a.insert(GRAPHENE_COMMITTEE_ACCOUNT);
       }
   };
    //充值 这个地方应该带个  memo_data
    struct gateway_withdraw_operation : public base_operation
   {
      struct fee_parameters_type {   uint64_t fee = 10000; };

      asset                                 fee;      
      account_id_type                       from;
      account_id_type                       to;
      asset                                 withdraw;

       
      extensions_type   extensions;

      account_id_type fee_payer()const { return from; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type& k)const {return k.fee;}
   };
   //提现
   struct gateway_deposit_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 1000;  };

      asset                                 fee;      
      account_id_type                       from;
      account_id_type                       to;
      asset                                 deposit;

      extensions_type   extensions;

      account_id_type fee_payer()const { return from; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type& k)const {return k.fee;}
   }; 

} } // graphene::chain

FC_REFLECT( graphene::chain::gateway_issue_currency_operation::fee_parameters_type,  (fee) )
FC_REFLECT( graphene::chain::gateway_withdraw_operation::fee_parameters_type,  (fee) )
FC_REFLECT( graphene::chain::gateway_deposit_operation::fee_parameters_type,  (fee)  )
 

 
FC_REFLECT( graphene::chain::gateway_issue_currency_operation,
            (fee)
            (issuer)
            (account_to)
            (issue_currency)       
            (revoke)
            (memo)
            (extensions)      
            )
 


FC_REFLECT( graphene::chain::gateway_withdraw_operation,
            (fee)            
            (from)
            (to)
            (withdraw) 
            (extensions)     
            ) 

FC_REFLECT( graphene::chain::gateway_deposit_operation,
            (fee)            
            (from)
            (to)
            (deposit) 
            (extensions)     
            )                        