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
#include <graphene/chain/protocol/worker.hpp>

namespace graphene { namespace chain {

void worker_create_operation::validate() const
{
   FC_ASSERT(fee.amount >= 0);
   FC_ASSERT(work_end_date >= work_begin_date + 24*3600);
   FC_ASSERT(work_end_date <= work_begin_date + 24*3600 *365);
   FC_ASSERT(daily_pay > 0);
   FC_ASSERT(daily_pay < GRAPHENE_MAX_SHARE_SUPPLY);
   FC_ASSERT(name.size() < GRAPHENE_MAX_WORKER_NAME_LENGTH, "max name length ${x}", ("x",GRAPHENE_MAX_WORKER_NAME_LENGTH));
   FC_ASSERT(url.size() < GRAPHENE_MAX_URL_LENGTH );
   FC_ASSERT(memo.size() < GRAPHENE_MAX_MEMO_LENGTH );
   FC_ASSERT(is_safe_string(url),"${x}",("x",url));
   FC_ASSERT(is_safe_string(memo),"${x}",("x",memo));
} 
share_type worker_create_operation::calculate_fee( const fee_parameters_type& k )const
{
   auto core_fee_required = k.fee;     
   core_fee_required += calculate_data_fee( fc::raw::pack_size(this), k.price_per_kbyte );   
   return core_fee_required;
}

} }
