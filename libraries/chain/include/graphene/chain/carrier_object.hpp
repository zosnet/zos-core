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
#include <graphene/chain/protocol/asset.hpp>
#include <graphene/db/object.hpp>
#include <graphene/db/generic_index.hpp>



namespace graphene { namespace chain {
   using namespace graphene::db;

   class carrier_object;
   
   class carrier_object : public abstract_object<carrier_object>
   {
      public:
         static const uint8_t space_id = protocol_ids;
         static const uint8_t type_id = carrier_object_type;

         string                                  name;
         account_id_type                         carrier_account;
         uint32_t                                need_auth = 0;
         flat_map<asset_id_type,account_id_type> trust_auth;
         optional<author_id_type>                def_auth;
         carrier_dynamic_id_type                 dynamic_id;
         optional<vesting_balance_id_type>       pay_vb;
         vote_id_type                            vote_id;
         uint64_t                                total_votes = 0;
         string                                  url;    
         string                                  memo;    
         string                                  config;    
         asset                                   lock_asset;
         identity_type                           enable = identity_enable;
         uint32_t                                op_type = 0;

         bool              is_enable() const { return enable ==identity_enable   ; }
         carrier_object() : vote_id(vote_id_type::carrier) {}

         optional<author_id_type> get_author(const database &d,const asset_id_type loan) const;
         
   };

   struct by_account;
   struct by_vote_id;
   struct by_last_block;
   using carrier_multi_index_type = multi_index_container<
      carrier_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member<object, object_id_type, &object::id>
         >,
         ordered_unique< tag<by_account>,
            member<carrier_object, account_id_type, &carrier_object::carrier_account>
         >,
         ordered_unique< tag<by_vote_id>,
            member<carrier_object, vote_id_type, &carrier_object::vote_id>
         >
      >
   >;
   using carrier_index = generic_index<carrier_object, carrier_multi_index_type>;



   class carrier_dynamic_object : public abstract_object<carrier_dynamic_object>
   {
      public:
         static const uint8_t space_id = implementation_ids;
         static const uint8_t type_id  = impl_carrier_dynamic_object_type;

         flat_map<asset_id_type, share_type>  loan;
         flat_map<asset_id_type, share_type>  invest;
         flat_map<asset_id_type, share_type>  collateralize_risk;         
         flat_map<asset_id_type, share_type>  fee;
         flat_map<asset_id_type, share_type>  recycle_collateralize;
         flat_map<asset_id_type, share_type>  recycle_loan;

         void add_loan(const asset &a)
         {           
            loan[a.asset_id] += a.amount;
         }
         void add_invest(const asset &a)
         {
            invest[a.asset_id] += a.amount;
         }
          void add_collateralize_risk(const asset &a)
         {
            collateralize_risk[a.asset_id] += a.amount;
         }
         void add_fee(const asset &a)
         {
             fee[a.asset_id] += a.amount;
         }
         void add_recycle_collateralize(const asset &a)
         {
            recycle_collateralize[a.asset_id] += a.amount;
         }
         void add_recycle_loan(const asset &a)
         {
            recycle_loan[a.asset_id] += a.amount;
         }
        
   };

    using carrier_dynamic_multi_index_type = multi_index_container<
      carrier_dynamic_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member<object, object_id_type, &object::id>
         > 
      >
   >;
   using carrier_dynamic_index = generic_index<carrier_dynamic_object, carrier_dynamic_multi_index_type>;
} } // graphene::chain

FC_REFLECT_DERIVED( graphene::chain::carrier_object, (graphene::db::object),
                    (name)
                    (carrier_account)             
                    (need_auth)
                    (trust_auth)
                    (def_auth)
                    (dynamic_id)       
                    (pay_vb)
                    (vote_id)
                    (total_votes)
                    (url)   
                    (memo)   
                    (config)   
                    (lock_asset)
                    (enable)    
                    (op_type)       
                  )
FC_REFLECT_DERIVED( graphene::chain::carrier_dynamic_object, (graphene::db::object),                              
                    (loan)
                    (invest)
                    (collateralize_risk)
                    (fee)
                    (recycle_collateralize)
                    (recycle_loan)                              
                  )             
 