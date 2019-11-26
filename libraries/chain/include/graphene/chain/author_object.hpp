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

   class author_object;
   
   class author_object : public abstract_object<author_object>
   {
      public:
         static const uint8_t space_id = protocol_ids;
         static const uint8_t type_id = author_object_type;

         string                  name; 
         account_id_type         author_account;
         uint32_t                auth_type = 0;         
         optional<vesting_balance_id_type> pay_vb;
         vector<asset_id_type>   allow_asset;
         vote_id_type            vote_id;
         uint64_t                total_votes = 0;
         string                  url;    
         string                  memo;    
         string                  config;    
         asset                   lock_asset;
         share_type              pay_for_account = 0;
         share_type              pay_for_referrer = 0;
         identity_type           enable = identity_enable;

         bool              is_enable() const { return enable ==identity_enable   ; }   
         author_object() : vote_id(vote_id_type::author) {}      
         
   };

   struct by_account;
   struct by_vote_id;
   struct by_last_block;
   using author_multi_index_type = multi_index_container<
      author_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member<object, object_id_type, &object::id>
         >,
         ordered_unique< tag<by_account>,
            member<author_object, account_id_type, &author_object::author_account>
         >,
         ordered_unique< tag<by_vote_id>,
            member<author_object, vote_id_type, &author_object::vote_id>
         >
      >
   >;
   using author_index = generic_index<author_object, author_multi_index_type>; 
} } // graphene::chain

FC_REFLECT_DERIVED( graphene::chain::author_object, (graphene::db::object),
                    (name)
                    (author_account)             
                    (auth_type)                    
                    (pay_vb)
                    (allow_asset)
                    (vote_id)
                    (total_votes)
                    (url)   
                    (memo)   
                    (config)   
                    (lock_asset)
                    (pay_for_account)
                    (pay_for_referrer)
                    (enable)                             
                  )             
 