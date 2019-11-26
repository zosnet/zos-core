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
#include <graphene/chain/protocol/vote.hpp>

namespace graphene { namespace chain { 

   /**
    * @brief Create a budget_member object, as a bid to hold a budget_member seat on the network.
    * @ingroup operations
    *
    * Accounts which wish to become budget_members may use this operation to create a budget_member object which stakeholders may
    * vote on to approve its position as a budget_member.
    */
   struct budget_member_create_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = 100 * GRAPHENE_BLOCKCHAIN_PRECISION; uint64_t lock_fee = 0 * GRAPHENE_BLOCKCHAIN_PRECISION;};

      asset                                 fee;
      /// The account which owns the budget_member. This account pays the fee for this operation.
      account_id_type                       budget_member_account;
      string                                url;
      string                                memo;

      account_id_type fee_payer()const { return budget_member_account; }
      void            validate()const;
      share_type      get_fee(const fee_parameters_type& p,uint32_t k)const {return p.lock_fee;}
   };

   /**
    * @brief Update a budget_member object.
    * @ingroup operations
    *
    * Currently the only field which can be updated is the `url`
    * field.
    */
   struct budget_member_update_operation : public base_operation
   {
       struct fee_parameters_type
      {
          
         share_type fee             = 10000;
         uint32_t   price_per_kbyte = 1000;
      };

      asset                                 fee;
      /// The budget member to update.
      budget_member_id_type              budget_member;
      /// The account which owns the budget_member. This account pays the fee for this operation.
      account_id_type                       budget_member_account;
      optional< string >                    new_url;
      optional< string >                    new_memo;
      optional<flat_set<vote_id_type> >     votes;

      account_id_type fee_payer()const { return budget_member_account; }
      void            validate()const;
      share_type      calculate_fee( const fee_parameters_type& k )const;
   };
 

   /// TODO: budget_member_resign_operation : public base_operation

} } // graphene::chain
FC_REFLECT( graphene::chain::budget_member_create_operation::fee_parameters_type, (fee)(lock_fee) )
FC_REFLECT( graphene::chain::budget_member_update_operation::fee_parameters_type,  (fee)(price_per_kbyte) )
 
FC_REFLECT( graphene::chain::budget_member_create_operation,
            (fee)(budget_member_account)(url)(memo) )
FC_REFLECT( graphene::chain::budget_member_update_operation,
            (fee)(budget_member)(budget_member_account)(new_url)(new_memo)(votes))
 

