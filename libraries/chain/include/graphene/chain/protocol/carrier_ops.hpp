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

#define carrier_cash        0x00000001  
#define carrier_bit         0x00000002
#define carrier_loan        0x00000004
#define carrier_invest      0x00000008
#define carrier_hash        0x00000020
#define carrier_dyfixed     0x00000040
#define carrier_locknode    0x00000080

#define carrier_mask      (carrier_cash | carrier_bit | carrier_loan |  carrier_invest |  carrier_hash | carrier_dyfixed | carrier_locknode)
#define carrier_xor_mask  (~carrier_mask)
 
#define carrier_op_invest  0x00000001
#define carrier_op_loan    0x00000002

#define carrier_op_mask      (carrier_op_invest | carrier_op_loan)
#define carrier_op_xor_mask  (~carrier_op_mask)

namespace graphene { namespace chain { 

  /**
    * @brief Create a carrier object, as a bid to hold a carrier position on the network.
    * @ingroup operations
    *
    * Accounts which wish to become carrieres may use this operation to create a carrier object which stakeholders may
    * vote on to approve its position as a carrier.
    */
   struct carrier_create_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 100 * GRAPHENE_BLOCKCHAIN_PRECISION; uint64_t lock_fee = 1000 * GRAPHENE_BLOCKCHAIN_PRECISION;};

      asset             fee;
      /// The account which owns the carrier. This account pays the fee for this operation.
      account_id_type   carrier_account;
      uint32_t          need_auth = 0;// 
      string            url;
      string            memo;
      optional<string>  config;
 

      account_id_type fee_payer()const { return carrier_account; }
      void            validate()const;
      share_type      get_fee(const fee_parameters_type& p,uint32_t k)const {return p.lock_fee;}
      void            get_required_active_authorities( flat_set<account_id_type>& a )const 
      {      
        a.clear(); 
        a.insert(GRAPHENE_COMMITTEE_ACCOUNT);
      }      
   };

  /**
    * @brief Update a carrier object's URL and block signing key.
    * @ingroup operations
    */
   struct carrier_update_operation : public base_operation
   {
      struct fee_parameters_type
      {
         
         share_type fee = 10 * GRAPHENE_BLOCKCHAIN_PRECISION;
         uint32_t price_per_kbyte = 1000; 
      };

      asset             fee;
      /// The carrier object to update.
      carrier_id_type   carrier;
      /// The account which owns the carrier. This account pays the fee for this operation.
      account_id_type   carrier_account;
      optional<uint32_t> need_auth ;// 
      /// The new URL.
      optional< string > new_url;
      optional< string > new_memo;
      optional< string > new_config;
      /// The new block signing key.
      

      account_id_type fee_payer()const { return carrier_account; }
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
 
   /// TODO: carrier_resign_operation : public base_operation

} } // graphene::chain

FC_REFLECT( graphene::chain::carrier_create_operation::fee_parameters_type,  (fee)(lock_fee) )
FC_REFLECT( graphene::chain::carrier_create_operation, 
     (fee)
     (carrier_account)
     (need_auth) 
     (url)
     (memo)
     (config) 
     )

FC_REFLECT( graphene::chain::carrier_update_operation::fee_parameters_type,  (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::carrier_update_operation, 
     (fee)
     (carrier)
     (carrier_account)
     (need_auth)
     (new_url)
     (new_memo)
     (new_config) 
     )
