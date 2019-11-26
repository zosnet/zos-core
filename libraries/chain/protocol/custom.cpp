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
#include <graphene/chain/protocol/custom.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>

namespace graphene { namespace chain {

database *g_pdb = nullptr;
void custom_operation::setDB(database *pDB)
{
   g_pdb = pDB;
}
void custom_operation::get_required_active_authorities(flat_set<account_id_type> &a) const
{     
   if(id == 6)
   {    
      fc::string name;      
      name.insert(name.begin(), data.begin(), data.end());
      const auto &accounts_by_name = g_pdb->get_index_type<account_index>().indices().get<by_name>();
      auto itr = accounts_by_name.find(name);
      if (itr != accounts_by_name.end())
         a.insert(itr->id);
      else
         a.insert(payer);
   }
   else
   {
      a.insert(payer);
   }
}

void custom_operation::validate()const
{
   FC_ASSERT( fee.amount > 0 );
}
share_type custom_operation::calculate_fee(const fee_parameters_type& k)const
{
   return k.fee + calculate_data_fee( fc::raw::pack_size(*this), k.price_per_kbyte );
}

} }
