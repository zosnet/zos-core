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
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/gateway_object.hpp>
#include <graphene/chain/carrier_object.hpp>
#include <graphene/business/bitlender_option_object.hpp>
#include <graphene/chain/author_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/hardfork.hpp>
#include <fc/uint128.hpp>



namespace graphene { namespace chain {
 

optional<author_id_type> carrier_object::get_author(const database &d,const asset_id_type loan) const
{  
  const auto iter = trust_auth.find(loan);
  if (iter != trust_auth.end())
  {
    auto author_id = iter->second;
    auto ac        = author_id(d);
    if(ac.uaccount_property & account_authenticator)
      return ac.get_author_id(d);
  }
  if(def_auth.valid())
    return *def_auth;
  auto ac        = carrier_account(d);  
  if(ac.uaccount_property & account_authenticator)
    return ac.get_author_id(d);
  return optional<author_id_type>();
}
optional<author_id_type> gateway_object::get_author(const database &d,const asset_id_type loan) const
{  
  const auto iter = trust_auth.find(loan);
  if (iter != trust_auth.end())
  {
    auto author_id = iter->second;
    auto ac        = author_id(d);
    if(ac.uaccount_property & account_authenticator)
      return ac.get_author_id(d);
  }
  if(def_auth.valid())
    return *def_auth;
  auto ac        = gateway_account(d);  
  if(ac.uaccount_property & account_authenticator)
    return ac.get_author_id(d);
  return optional<author_id_type>();
}


} } // graphene::chain
 