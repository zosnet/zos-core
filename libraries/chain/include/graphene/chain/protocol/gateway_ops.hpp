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

#define gateway_cash      0x00000001
#define gateway_bit       0x00000002
#define gateway_dispost   0x00000004
#define gateway_withdray  0x00000008
#define gateway_hash      0x00000020
#define gateway_dyfixed   0x00000040
#define gateway_locknode  0x00000080

#define gateway_mask            (gateway_cash | gateway_bit | gateway_dispost | gateway_withdray |  gateway_hash | gateway_dyfixed | gateway_locknode)
#define gateway_xor_mask        (~gateway_mask)

namespace graphene { namespace chain { 

  /**
    * @brief Create a gateway object, as a bid to hold a gateway position on the network.
    * @ingroup operations
    *
    * Accounts which wish to become gatewayes may use this operation to create a gateway object which stakeholders may
    * vote on to approve its position as a gateway.
    */
   struct gateway_create_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 100 * GRAPHENE_BLOCKCHAIN_PRECISION; uint64_t lock_fee = 1000 * GRAPHENE_BLOCKCHAIN_PRECISION;};

      asset                   fee;
      /// The account which owns the gateway. This account pays the fee for this operation.
      account_id_type         gateway_account;
      uint32_t                need_auth = 0;
      string                  url;
      string                  memo;
      flat_set<asset_id_type> allowed_asset;
      

      account_id_type fee_payer()const { return gateway_account; }
      void            validate()const;
      share_type      get_fee(const fee_parameters_type& p,uint32_t k)const {return p.lock_fee;}
      void            get_required_active_authorities( flat_set<account_id_type>& a )const 
      {       
        a.clear();
        a.insert(GRAPHENE_COMMITTEE_ACCOUNT);
      }      
   };

  /**
    * @brief Update a gateway object's URL and block signing key.
    * @ingroup operations
    */
   struct gateway_update_operation : public base_operation
   {
      struct fee_parameters_type
      {
         
         share_type fee = 10 * GRAPHENE_BLOCKCHAIN_PRECISION;
          uint32_t price_per_kbyte = 1000; 
      };

      asset                             fee;
      /// The gateway object to update.
      gateway_id_type                   gateway;
      /// The account which owns the gateway. This account pays the fee for this operation.
      account_id_type                   gateway_account;
      optional<uint32_t>                need_auth;
      /// The new URL.
      optional< string >                new_url;
      optional< string >                new_memo;
      optional<flat_set<asset_id_type>> allowed_asset;
      /// The new block signing key.
    

      account_id_type fee_payer()const { return gateway_account; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type &k) const;
      void            get_required_active_authorities(flat_set<account_id_type> &a) const
      {       
        a.clear();
        a.insert(GRAPHENE_COMMITTEE_ACCOUNT);
      }     
   };
 
   /// TODO: gateway_resign_operation : public base_operation

} } // graphene::chain

FC_REFLECT( graphene::chain::gateway_create_operation::fee_parameters_type, (fee)(lock_fee) )
FC_REFLECT( graphene::chain::gateway_create_operation,
     (fee)
     (gateway_account)
     (need_auth)
     (url)
     (memo)
     (allowed_asset)
      )

FC_REFLECT( graphene::chain::gateway_update_operation::fee_parameters_type,  (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::gateway_update_operation,
    (fee)
    (gateway)
    (gateway_account)
    (need_auth) 
    (new_url)
    (new_memo)
    (allowed_asset) 
    )

 