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

#include <graphene/chain/database.hpp>
#include <graphene/chain/db_with.hpp>

#include <graphene/chain/asset_object.hpp>
 

#include <fc/uint128.hpp>

namespace graphene { namespace chain {



void  database::remove_proposal(const proposal_object &_object)
{
   if(_app_status.has_object_history_plugin)
   {  
        fill_object_history_operation op;
        op.itype  = fill_object_proposal;
        op.id     = _object.id;
        op.issuer = _object.proposer;
        op.data   = fc::raw::pack(_object);
        push_applied_operation(op);
   }
   remove(_object);  
}
void  database::remove_worker(const worker_id_type& orderid)
{
   auto &worker = get(orderid);
   if(_app_status.has_object_history_plugin)
   {  
      fill_object_history_operation op;
      op.itype  = fill_object_worker;
      op.id     = worker.id;
      op.issuer = worker.worker_account;
      op.data   = fc::raw::pack(worker);
      push_applied_operation(op);
   }
   remove(worker);
}
 

} }
