/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell * copies of the Software, and to permit persons to whom the Software is * furnished to do so, subject to the following conditions: *  * The above copyright notice and this permission notice shall be included in * all copies or substantial portions of the Software. *  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN * THE SOFTWARE. */
#pragma once

#include <graphene/app/plugin.hpp>
#include <graphene/business/locktoken_object.hpp>
#include <graphene/chain/database.hpp>

#include <fc/thread/future.hpp>
#include <fc/uint128.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace graphene {namespace locktoken_history {
using namespace chain; 
 
struct  account_locktoken_item {
  uint32_t                      type = 0;
  uint32_t                      count = 0;
  asset_precision               total;
  flat_map<asset_id_type,asset_precision> interest;

  void add_interest(const asset_precision &other) {
    auto itr_interest = interest.find(other.asset_id);
    if(itr_interest == interest.end())
    {
      interest[other.asset_id] = other;
    }
    else
    {
      itr_interest->second.precision = other.precision;
      itr_interest->second.symbol = other.symbol;
      itr_interest->second.amount += other.amount;
    }
  }
};
struct account_locktoken_his_object : public abstract_object<account_locktoken_his_object>
{
   static const uint8_t space_id = implementation_ids;
   static const uint8_t type_id  = impl_locktoken_history_object_type;

   account_id_type issuer;
   vector<flat_map<asset_id_type, account_locktoken_item>>   locktoken;    
};
struct by_locktoken_account; 
typedef multi_index_container < 
   account_locktoken_his_object, 
   indexed_by < 
     ordered_unique < tag < by_id > , member < object, object_id_type,  & object::id >> , 
     ordered_unique < tag < by_locktoken_account > , member < account_locktoken_his_object, account_id_type,  & account_locktoken_his_object::issuer >>  
     >>  account_locktoken_his_object_multi_index_type; 

typedef generic_index < account_locktoken_his_object,account_locktoken_his_object_multi_index_type > account_locktoken_his_index; 

namespace detail {
class locktoken_history_plugin_impl; 
}

class locktoken_history_plugin : public graphene::app::plugin
{
public:
  locktoken_history_plugin();
  virtual ~locktoken_history_plugin();

  std::string plugin_name() const override;
  virtual void plugin_set_program_options(
      boost::program_options::options_description &cli,
      boost::program_options::options_description &cfg) override;
  virtual void plugin_initialize(
      const boost::program_options::variables_map &options) override;
  virtual void plugin_startup() override;
  virtual void get_info(fc::mutable_variant_object &result) override; 
public:
  uint64_t get_locktoken_history_count(account_id_type account, asset_id_type asset_id,  uint32_t utype) const;
  vector<locktoken_object> get_locktoken_history(account_id_type account,  asset_id_type asset_id, uint32_t utype ,uint64_t ustart, uint64_t ulimit) const;

private:
  friend class detail::locktoken_history_plugin_impl;
  std::unique_ptr<detail::locktoken_history_plugin_impl> my;
};
}}//graphene::locktoken_history

FC_REFLECT(graphene::locktoken_history::account_locktoken_item,  
    (type)
    (count)
    (total)
    (interest)
    )
FC_REFLECT_DERIVED(graphene::locktoken_history::account_locktoken_his_object, (graphene::db::object),  
   (issuer)
   (locktoken)   
   )

 
