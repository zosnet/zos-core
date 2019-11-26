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

#define auth_gateway 0x0010000 
#define auth_carrier 0x0020000
#define auth_author  0x0040000

#define auth_nosetting 2000

#define author_mask         (auth_gateway | auth_carrier | auth_author)
#define author_xor_mask    (~(auth_gateway | auth_carrier | auth_author))

namespace graphene { namespace chain { 

  /**
    * @brief Create a author object, as a bid to hold a author position on the network.
    * @ingroup operations
    *
    * Accounts which wish to become authores may use this operation to create a author object which stakeholders may
    * vote on to approve its position as a author.
    */
   struct author_create_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 100 * GRAPHENE_BLOCKCHAIN_PRECISION; uint64_t lock_fee = 0 * GRAPHENE_BLOCKCHAIN_PRECISION;};

      asset             fee;
      /// The account which owns the author. This account pays the fee for this operation.
      account_id_type   author_account;      
      uint32_t          auth_type = 0;//
      vector<asset_id_type> allow_asset;
      string url;
      string            memo;
      optional<string>  config; 

      account_id_type fee_payer()const { return author_account; }
      void            validate()const;
      share_type      get_fee(const fee_parameters_type& p,uint32_t k)const {return p.lock_fee;}
      void            get_required_active_authorities( flat_set<account_id_type>& a )const 
      {      
        a.clear(); 
        a.insert(GRAPHENE_COMMITTEE_ACCOUNT);
      }      
   };

  /**
    * @brief Update a author object's URL and block signing key.
    * @ingroup operations
    */
   struct author_update_operation : public base_operation
   {
      struct fee_parameters_type
      {
         
         share_type fee = 10 * GRAPHENE_BLOCKCHAIN_PRECISION;
         uint32_t price_per_kbyte = 1000; 
      };

      asset                           fee;
      /// The author object to update.
      author_id_type                  author;
      /// The account which owns the author. This account pays the fee for this operation.
      account_id_type                 author_account;
      
      optional<uint32_t>              auth_type ;// 
      /// The new URL.
      optional< string >              new_url;
      optional< string >              new_memo;
      optional< string >              new_config;
      optional<vector<asset_id_type>> new_allow_asset;
      /// The new block signing key.
      

      account_id_type fee_payer()const { return author_account; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type &k) const;
      void            get_required_active_authorities( flat_set<account_id_type>& a )const 
      {      
        if(new_url.valid())
        {
          a.clear(); 
          a.insert(GRAPHENE_COMMITTEE_ACCOUNT);
        }
      }   
   };
 
   /// TODO: author_resign_operation : public base_operation

} } // graphene::chain

FC_REFLECT( graphene::chain::author_create_operation::fee_parameters_type,  (fee)(lock_fee) )
FC_REFLECT( graphene::chain::author_create_operation, 
     (fee)
     (author_account)
     (auth_type) 
     (allow_asset)
     (url)
     (memo)
     (config) 
     )

FC_REFLECT( graphene::chain::author_update_operation::fee_parameters_type,  (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::author_update_operation, 
     (fee)
     (author)
     (author_account)     
     (auth_type)
     (new_url)
     (new_memo)
     (new_config) 
     (new_allow_asset)
     )
