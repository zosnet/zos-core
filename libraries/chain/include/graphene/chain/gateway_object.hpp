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

   class gateway_object;

   class gateway_object : public abstract_object<gateway_object>
   {
      public:
         static const uint8_t space_id = protocol_ids;
         static const uint8_t type_id = gateway_object_type;

         string                                  name;
         account_id_type                         gateway_account;
         uint32_t                                need_auth = 0;
         flat_map<asset_id_type,account_id_type> trust_auth;
         optional<author_id_type>                def_auth;
         gateway_dynamic_id_type                 dynamic_id;
         optional< vesting_balance_id_type >     pay_vb;
         vote_id_type                            vote_id;
         uint64_t                                total_votes = 0;
         string                                  url;     
         string                                  memo;    
         asset                                   lock_asset;
         flat_set<asset_id_type>                 allowed_assets;         

         identity_type                           enable = identity_enable;         

         bool             is_enable() const { return enable ==identity_enable   ; }
         gateway_object() : vote_id(vote_id_type::gateway) {}    

         optional<author_id_type> get_author(const database &d,const asset_id_type loan) const;    
   };

   struct by_account;
   struct by_vote_id;
   struct by_last_block;
   using gateway_multi_index_type = multi_index_container<
      gateway_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member<object, object_id_type, &object::id>
         >,
         ordered_unique< tag<by_account>,
            member<gateway_object, account_id_type, &gateway_object::gateway_account>
         >,
         ordered_unique< tag<by_vote_id>,
            member<gateway_object, vote_id_type, &gateway_object::vote_id>
         >
      >
   >;
   using gateway_index = generic_index<gateway_object, gateway_multi_index_type>;


   class gateway_dynamic_object : public abstract_object<gateway_dynamic_object>
   {
      public:
         static const uint8_t space_id = implementation_ids;
         static const uint8_t type_id  = impl_gateway_dynamic_object_type;
         

         flat_map<asset_id_type,share_type> deposit_amount;
         flat_map<asset_id_type,share_type> withdraw_amount;
         flat_map<asset_id_type,uint64_t>   deposit_count;
         flat_map<asset_id_type,uint64_t>   withdraw_count;
         flat_map<asset_id_type,share_type> issue_amount;     
   };

   
   using gateway_dynamic_multi_index_type = multi_index_container<
      gateway_dynamic_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member<object, object_id_type, &object::id>
         > 
      >
   >;
   using gateway_dynamic_index = generic_index<gateway_dynamic_object, gateway_dynamic_multi_index_type>;
} } // graphene::chain

FC_REFLECT_DERIVED( graphene::chain::gateway_object, (graphene::db::object),
                    (name)
                    (gateway_account)       
                    (need_auth)        
                    (trust_auth)    
                    (def_auth)
                    (dynamic_id)     
                    (pay_vb)
                    (vote_id)
                    (total_votes)
                    (url)                                       
                    (memo)    
                    (lock_asset)
                    (allowed_assets) 
                    (enable)    
                  )
FC_REFLECT_DERIVED( graphene::chain::gateway_dynamic_object, (graphene::db::object),                                                 
                    (deposit_amount)
                    (withdraw_amount)
                    (deposit_count)
                    (withdraw_count)
                    (issue_amount)                    
                  )                  
