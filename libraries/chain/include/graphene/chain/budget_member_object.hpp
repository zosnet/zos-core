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
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/object.hpp>
#include <graphene/db/generic_index.hpp>

namespace graphene { namespace chain {
   using namespace graphene::db;

   class account_object;

   /**
    *  @brief tracks information about a budget_member account.
    *  @ingroup object
    *
    *  A budget_member is responsible for setting blockchain parameters and has
    *  dynamic multi-sig control over the budget account.  The current set of
    *  active budget_members has control.
    *
    *  budget_members were separated into a separate object to make iterating over
    *  the set of budget_member easy.
    */
   class budget_member_object : public abstract_object<budget_member_object>
   {
      public:
         static const uint8_t space_id = protocol_ids;
         static const uint8_t type_id  = budget_member_object_type;

         string                 name;
         account_id_type        budget_member_account;
         vote_id_type           vote_id;
         uint64_t               total_votes = 0;
         string                 url;
         string                 memo;
         flat_set<vote_id_type> votes;              
         asset                  lock_asset;
         identity_type          enable = identity_enable;
        
         
         bool                   is_enable() const { return enable ==identity_enable   ; }
         bool                   is_samevotes(flat_set<vote_id_type> &votes1)
         {
            if(votes.size()!=votes1.size())
            return false;
            for(auto &id1 : votes)
            {
               if(votes1.find(id1) == votes1.end())
                  return false;
            }
            return true;
          }         
   };

    

   struct by_account;
   struct by_vote_id;
   using budget_member_multi_index_type = multi_index_container<
      budget_member_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member<object, object_id_type, &object::id>
         >,
         ordered_unique< tag<by_account>,
            member<budget_member_object, account_id_type, &budget_member_object::budget_member_account>
         >,
         ordered_unique< tag<by_vote_id>,
            member<budget_member_object, vote_id_type, &budget_member_object::vote_id>
         >
      >
   >;
   using budget_member_index = generic_index<budget_member_object, budget_member_multi_index_type>;
} } // graphene::chain

FC_REFLECT_DERIVED( graphene::chain::budget_member_object, (graphene::db::object),
                    (name)(budget_member_account)(vote_id)(total_votes)(url)(memo)(votes)(lock_asset)(enable))
 
