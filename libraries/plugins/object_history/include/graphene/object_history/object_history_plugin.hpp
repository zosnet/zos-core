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

#include <graphene/app/plugin.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/business/bitlender_object.hpp> 
#include <graphene/business/finance_object.hpp> 
#include <graphene/chain/worker_object.hpp> 
#include <graphene/business/locktoken_object.hpp> 
#include <graphene/chain/proposal_object.hpp>
#include <graphene/business/market_object.hpp>
#include <graphene/chain/withdraw_permission_object.hpp>

#include <fc/thread/future.hpp>
#include <fc/uint128.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace graphene { namespace object_history  {
using namespace chain;

 
 

namespace detail
{
    class object_history_plugin_impl;
}
 
class object_history_plugin : public graphene::app::plugin
{
   public:
      object_history_plugin();
      virtual ~object_history_plugin();

      std::string plugin_name()const override;
      virtual void plugin_set_program_options(
         boost::program_options::options_description& cli,
         boost::program_options::options_description& cfg) override;
      virtual void plugin_initialize(
         const boost::program_options::variables_map& options) override;
      virtual void plugin_startup() override;
      virtual void get_info(fc::mutable_variant_object &result) override;
      const flat_set<uint32_t> &tracked_buckets() const;

    public:     
      bitlender_order_data             get_loan_object_history( object_id_type id) const;
      bitlender_invest_data            get_invest_object_history(object_id_type id) const;
      issue_fundraise_object           get_issue_fundraise_object_history(   object_id_type id  )const;
      buy_fundraise_object             get_buy_fundraise_object_history(   object_id_type id  )const;
      sell_exchange_object             get_sell_exchange_object_history(   object_id_type id  )const;
      buy_exchange_object              get_buy_exchange_object_history(   object_id_type id  )const;
      worker_object                    get_worker_object_history(   object_id_type id  )const;
      locktoken_object                 get_locktoken_object_history(   object_id_type id  )const;
      withdraw_permission_object       get_withdraw_permission_object_history(   object_id_type id  )const;      
      proposal_object                  get_proposal_object_history(   object_id_type id  )const;
      vector<worker_object>            get_workers_history(worker_id_type start, uint64_t limit ) const;
      limit_order_object               get_limit_order_object_history(   object_id_type id  )const;
      vector<proposal_object>          get_proposals_history(account_id_type account,proposal_id_type start, uint64_t limit )const;
      vector<limit_order_object>       get_account_limit_history(vector<limit_order_id_type> ids) const;
      vector<limit_order_id_type>      get_account_limit_history_count(account_id_type a, asset_id_type base, asset_id_type quote, uint64_t ulimit) const;

    private:
      friend class detail::object_history_plugin_impl;
      std::unique_ptr<detail::object_history_plugin_impl> my;
};
 
} } //graphene::object_history 
 