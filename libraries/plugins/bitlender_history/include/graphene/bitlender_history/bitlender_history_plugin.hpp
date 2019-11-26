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
#include <graphene/business/bitlender_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/operation_history_object.hpp>

#include <fc/thread/future.hpp>
#include <fc/uint128.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace graphene {namespace bitlender_history {
using namespace chain; 

enum bitlender_history_object_type {
bitlender_history_object_type = 1, 
bitlender_invest_bucket_object_type = 2, 
}; 

struct bitlender_bucket_key {
bitlender_bucket_key(asset_id_type a, asset_id_type b, uint32_t s, fc::time_point_sec o):base(a), quote(b) {}
bitlender_bucket_key() {}

asset_id_type base; 
asset_id_type quote; 

friend bool operator < (const bitlender_bucket_key & a, const bitlender_bucket_key & b) {
return std::tie(a.base, a.quote) < std::tie(b.base, b.quote); 
}
friend bool operator == (const bitlender_bucket_key & a, const bitlender_bucket_key & b) {
return std::tie(a.base, a.quote) == std::tie(b.base, b.quote); 
}
}; 
struct bitlender_day_detail
{
  uint32_t      day;
  fc::time_point_sec day_tm;
  share_type    total_loan = 0;
  share_type    total_collateralize = 0;
  uint64_t      total_count = 0;
  share_type    collateral_loan = 0;
  share_type    collateral_collateralize = 0;
  uint64_t      collateral_count = 0;
  share_type    squeeze_loan = 0;
  share_type    squeeze_collateralize = 0;
  uint64_t      squeeze_count = 0;

  void operator += (bitlender_day_detail &other)
  {
      total_loan += other. total_loan;
      total_collateralize += other. total_collateralize;
      total_count += other.total_count ;
      collateral_loan += other. collateral_loan;
      collateral_collateralize += other. collateral_collateralize;
      collateral_count += other.collateral_count ;
      squeeze_loan += other.squeeze_loan ;
      squeeze_collateralize += other.squeeze_collateralize ;
      squeeze_count += other. squeeze_count;    
  }
};



struct bitlender_history_object:public abstract_object < bitlender_history_object >  {
static const uint8_t space_id = bitlender_history_ids; 
static const uint8_t type_id = bitlender_history_object_type; 

bitlender_bucket_key key;
string     base_symbol;
uint8_t    base_precision;
string     quote_symbol;
uint8_t    quote_precision;

uint32_t   fee_mode = 0;                //费用模式
share_type total_laon_amount  = 0;    //总借款量
share_type total_collateralize_amount  = 0; //低于最小抵押线
share_type total_earnings_amount  = 0; //低于平仓线
share_type total_loan_collateralize_fee  = 0; //
share_type total_loan_collateralize_risk  = 0; //
share_type total_loan_carrier_fee  = 0; //
share_type total_invest_collateralize_fee  = 0; //
share_type total_invest_collateralize_risk  = 0; //
share_type total_invest_carrier_fee  = 0; //
uint32_t   total_max_ratio = 0; //
uint32_t   total_min_ratio = 0x7fff; //
share_type total_laon_count  = 0; //总借次数
share_type total_invest_count  = 0; //总投资次数
share_type total_recycle_count  = 0;//不良资产的次数

share_type today_laon_amount  = 0; 
share_type today_collateralize_amount  = 0; 
share_type today_earnings_amount  = 0; 
share_type today_loan_collateralize_fee  = 0; 
share_type today_loan_collateralize_risk  = 0; 
share_type today_loan_carrier_fee  = 0; 
share_type today_invest_collateralize_fee  = 0; 
share_type today_invest_collateralize_risk  = 0; 
share_type today_invest_carrier_fee  = 0; 
uint32_t   today_max_ratio = 0; 
uint32_t   today_min_ratio = 0x7fff; 
share_type today_laon_count  = 0; 
share_type today_invest_count  = 0; 

vector<bitlender_day_detail> day_detail;
vector<bitlender_day_detail> week_detail;
vector<bitlender_day_detail> month_detail;

void reset_today_flag()
{
    today_laon_amount = 0;
    today_collateralize_amount = 0;
    today_earnings_amount = 0;
    today_loan_collateralize_fee = 0;
    today_loan_collateralize_risk = 0;
    today_loan_carrier_fee = 0;    
    today_invest_collateralize_fee = 0;
    today_invest_collateralize_risk = 0;
    today_invest_carrier_fee = 0;
    today_max_ratio = 0;
    today_min_ratio = 0x7fff;
    today_laon_count = 0;
    today_invest_count = 0;
}

}; 

struct by_bitlender_key; 
typedef multi_index_container < 
bitlender_history_object, 
indexed_by < 
ordered_unique < tag < by_id > , member < object, object_id_type,  & object::id >> , 
ordered_unique < tag < by_bitlender_key > , member < bitlender_history_object, bitlender_bucket_key,  & bitlender_history_object::key >>  >> 
bitlender_history_object_multi_index_type; 

typedef generic_index < bitlender_history_object, 
bitlender_history_object_multi_index_type > 
bitlender_history_index; 

namespace detail {
class bitlender_history_plugin_impl; 
}

/**
 *  The market history plugin can be configured to track any number of intervals via its configuration.  Once per block it
 *  will scan the virtual operations and look for fill_order_operations and then adjust the appropriate bucket objects for
 *  each fill order.
 */
class bitlender_history_plugin : public graphene::app::plugin
{
public:
  bitlender_history_plugin();
  virtual ~bitlender_history_plugin();

  std::string plugin_name() const override;
  virtual void plugin_set_program_options(
      boost::program_options::options_description &cli,
      boost::program_options::options_description &cfg) override;
  virtual void plugin_initialize(
      const boost::program_options::variables_map &options) override;
  virtual void plugin_startup() override;
  virtual void get_info(fc::mutable_variant_object &result) override;

  const flat_set<uint32_t> &tracked_buckets() const;

public:
  vector<operation_history_object> get_account_bitlender_history(account_id_type account, bitlender_order_id_type order) const;
  vector<bitlender_order_info> get_account_loan_history(account_id_type a, fc::time_point_sec start, fc::time_point_sec end, uint64_t ustart, uint64_t ulimit) const;
  vector<bitlender_invest_info> get_account_invest_history(account_id_type a, fc::time_point_sec start, fc::time_point_sec end, uint64_t ustart, uint64_t ulimit) const;

  uint64_t get_account_loan_history_count(account_id_type a, fc::time_point_sec start, fc::time_point_sec end) const;
  uint64_t get_account_invest_history_count(account_id_type a, fc::time_point_sec start, fc::time_point_sec end) const;

  void fill_day_detail(bitlender_history_object &a, const bitlender_order_object &b);
  void fill_calc_detail(bitlender_history_object &a, uint16_t type);

private:
  optional<bitlender_order_object> get_loan_history(bitlender_order_id_type order) const;  
private:
  friend class detail::bitlender_history_plugin_impl;
  std::unique_ptr<detail::bitlender_history_plugin_impl> my;
};
}}//graphene::bitlender_history


FC_REFLECT(graphene::bitlender_history::bitlender_day_detail,
        (day)
        (day_tm)
        (total_loan)
        (total_collateralize)
        (total_count)
        (collateral_loan)
        (collateral_collateralize)
        (collateral_count)
        (squeeze_loan)
        (squeeze_collateralize)
        (squeeze_count)
        ) 

FC_REFLECT(graphene::bitlender_history::bitlender_bucket_key, (base)(quote))
FC_REFLECT_DERIVED(graphene::bitlender_history::bitlender_history_object, (graphene::db::object), 
(key)
(base_symbol)
(base_precision)
(quote_symbol)
(quote_precision) 
(fee_mode)
(total_laon_amount)
(total_collateralize_amount)
(total_earnings_amount)
(total_loan_collateralize_fee)
(total_loan_collateralize_risk)
(total_loan_carrier_fee)
(total_invest_collateralize_fee)
(total_invest_collateralize_risk)
(total_invest_carrier_fee)
(total_max_ratio)
(total_min_ratio)
(total_laon_count)
(total_invest_count)
(total_recycle_count)
(today_laon_amount)
(today_collateralize_amount)
(today_earnings_amount)
(today_loan_collateralize_fee)
(today_loan_collateralize_risk)
(today_loan_carrier_fee)
(today_invest_collateralize_fee)
(today_invest_collateralize_risk)
(today_invest_carrier_fee)
(today_max_ratio)
(today_min_ratio)
(today_laon_count)
(today_invest_count)
(day_detail)
(week_detail)
(month_detail)
)
