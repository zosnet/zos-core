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

struct budget_record
{   
    budget_record()
    {
      for(int l=0;l<miner_max;l++)
      {
         miner_budget.push_back(0);       
         from_unused_miner_budget.push_back(0);
      }
    }


   uint64_t time_since_last_budget = 0;

   // sources of budget
   share_type                     from_initial_reserve = 0;
   share_type                     from_accumulated_fees = 0;
   share_type                     from_platform_fees = 0;
   vector<share_type>             from_unused_miner_budget; 

   // sinks of budget, should sum up to total_budget
   vector<share_type>   miner_budget;
   share_type           worker_budget = 0;     
};

class budget_record_object;

class budget_record_object : public graphene::db::abstract_object<budget_record_object>
{
   public:
      static const uint8_t space_id = implementation_ids;
      static const uint8_t type_id = impl_budget_record_object_type;

      fc::time_point_sec time;
      budget_record record;
};

} }

FC_REFLECT(
   graphene::chain::budget_record,
   (time_since_last_budget)
   (from_initial_reserve)
   (from_accumulated_fees)
   (from_platform_fees)
   (from_unused_miner_budget)
   (miner_budget)
   (worker_budget)     
)

FC_REFLECT_DERIVED(
   graphene::chain::budget_record_object,
   (graphene::db::object),
   (time)
   (record)
)
