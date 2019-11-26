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
#include <graphene/business/finance_object.hpp>
//#include <graphene/chain/database.hpp>
//#include <graphene/chain/operation_history_object.hpp>

#include <fc/thread/future.hpp>
#include <fc/uint128.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace graphene {namespace finance_history {
using namespace chain; 
 



enum finance_history_object_type
{
   finance_finance_order_history_object_type = 0,
   finance_finance_bucket_object_type = 1,
   finance_ticker_object_type = 2,
   finance_ticker_meta_object_type = 3
};

struct finance_bucket_key
{
   finance_bucket_key( asset_id_type a, asset_id_type b, uint32_t s, fc::time_point_sec o )
   :base(a),quote(b),seconds(s),open(o){}
   finance_bucket_key(){}

   asset_id_type      base;
   asset_id_type      quote;
   uint32_t           seconds = 0;
   fc::time_point_sec open;

   friend bool operator < ( const finance_bucket_key& a, const finance_bucket_key& b )
   {
      return std::tie( a.base, a.quote, a.seconds, a.open ) < std::tie( b.base, b.quote, b.seconds, b.open );
   }
   friend bool operator == ( const finance_bucket_key& a, const finance_bucket_key& b )
   {
      return std::tie( a.base, a.quote, b.seconds, a.open ) == std::tie( b.base, b.quote, b.seconds, b.open );
   }
};

struct finance_bucket_object : public abstract_object<finance_bucket_object>
{
   static const uint8_t space_id = finance_history_ids;
   static const uint8_t type_id  = finance_finance_bucket_object_type;

   price high()const { return asset( high_base, key.base ) / asset( high_quote, key.quote ); }
   price low()const { return asset( low_base, key.base ) / asset( low_quote, key.quote ); }

   finance_bucket_key          key;
   share_type          high_base;
   share_type          high_quote;
   share_type          low_base;
   share_type          low_quote;
   share_type          open_base;
   share_type          open_quote;
   share_type          close_base;
   share_type          close_quote;
   share_type          base_volume;
   share_type          quote_volume;
};

struct finance_history_key {
  asset_id_type        base;
  asset_id_type        quote;
  int64_t              sequence = 0;

  friend bool operator < ( const finance_history_key& a, const finance_history_key& b ) {
    return std::tie( a.base, a.quote, a.sequence ) < std::tie( b.base, b.quote, b.sequence );
  }
  friend bool operator == ( const finance_history_key& a, const finance_history_key& b ) {
    return std::tie( a.base, a.quote, a.sequence ) == std::tie( b.base, b.quote, b.sequence );
  }
};
struct finance_order_history_object : public abstract_object<finance_order_history_object>
{
   static const uint8_t space_id = finance_history_ids;
   static const uint8_t type_id  = finance_finance_order_history_object_type;

   finance_history_key          key;
   fc::time_point_sec   time;
   fill_order_operation op;
};
struct finance_finance_order_history_object_key_base_extractor
{
   typedef asset_id_type result_type;
   result_type operator()(const finance_order_history_object& o)const { return o.key.base; }
};
struct finance_finance_order_history_object_key_quote_extractor
{
   typedef asset_id_type result_type;
   result_type operator()(const finance_order_history_object& o)const { return o.key.quote; }
};
struct finance_finance_order_history_object_key_sequence_extractor
{
   typedef int64_t result_type;
   result_type operator()(const finance_order_history_object& o)const { return o.key.sequence; }
};

struct finance_ticker_object : public abstract_object<finance_ticker_object>
{
   static const uint8_t space_id = finance_history_ids;
   static const uint8_t type_id  = finance_ticker_object_type;

   asset_id_type       base;
   asset_id_type       quote;
   share_type          last_day_base;
   share_type          last_day_quote;
   share_type          latest_base;
   share_type          latest_quote;
   fc::uint128         base_volume;
   fc::uint128         quote_volume;
};

struct finance_ticker_meta_object : public abstract_object<finance_ticker_meta_object>
{
   static const uint8_t space_id = finance_history_ids;
   static const uint8_t type_id  = finance_ticker_meta_object_type;

   object_id_type      rolling_min_order_his_id;
   bool                skip_min_order_his_id = false;
};

struct by_finance_key;
typedef multi_index_container<
   finance_bucket_object,
   indexed_by<
      ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
      ordered_unique< tag<by_finance_key>, member< finance_bucket_object, finance_bucket_key, &finance_bucket_object::key > >
   >
> finance_bucket_object_multi_index_type;

struct by_finance_time;
typedef multi_index_container<
   finance_order_history_object,
   indexed_by<
      ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
      ordered_unique< tag<by_finance_key>, member< finance_order_history_object, finance_history_key, &finance_order_history_object::key > >,
      ordered_unique<
         tag<by_finance_time>,
         composite_key<
            finance_order_history_object,
            finance_finance_order_history_object_key_base_extractor,
            finance_finance_order_history_object_key_quote_extractor,
            member<finance_order_history_object, time_point_sec, &finance_order_history_object::time>,
            finance_finance_order_history_object_key_sequence_extractor
         >,
         composite_key_compare<
            std::less< asset_id_type >,
            std::less< asset_id_type >,
            std::greater< time_point_sec >,
            std::less< int64_t >
         >
      >
   >
> finance_order_history_multi_index_type;

struct by_finance;
struct by_fvolume;
typedef multi_index_container<
   finance_ticker_object,
   indexed_by<
      ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
      ordered_non_unique< tag<by_fvolume>, member< finance_ticker_object, fc::uint128, &finance_ticker_object::base_volume > >,
      ordered_unique<
         tag<by_finance>,
         composite_key<
            finance_ticker_object,
            member<finance_ticker_object, asset_id_type, &finance_ticker_object::base>,
            member<finance_ticker_object, asset_id_type, &finance_ticker_object::quote>
         >
      >
   >
> finance_ticker_object_multi_index_type;

typedef generic_index<finance_bucket_object, finance_bucket_object_multi_index_type>         finance_bucket_index;
typedef generic_index<finance_order_history_object, finance_order_history_multi_index_type>  finance_history_index;
typedef generic_index<finance_ticker_object, finance_ticker_object_multi_index_type>         finance_ticker_index;



namespace detail {
class finance_history_plugin_impl; 
}


class finance_history_plugin:public graphene::app::plugin {
public:
finance_history_plugin(); 
virtual ~finance_history_plugin(); 

std::string plugin_name()const override; 
virtual void plugin_set_program_options(
boost::program_options::options_description & cli, 
boost::program_options::options_description & cfg)override; 
virtual void plugin_initialize(
const boost::program_options::variables_map & options)override; 
virtual void plugin_startup()override; 
virtual void get_info(fc::mutable_variant_object &result) override;
 

vector < issue_fundraise_object >   get_account_issue_fundraise_history(account_id_type a, fc::time_point_sec start, fc::time_point_sec end)const; 
vector < buy_fundraise_object >     get_account_buy_fundraise_history(account_id_type a, fc::time_point_sec start, fc::time_point_sec end)const; 
vector < sell_exchange_object >     get_account_sell_exchange_history(account_id_type a, fc::time_point_sec start, fc::time_point_sec end)const; 
vector < buy_exchange_object >      get_account_buy_exchange_history(account_id_type a, fc::time_point_sec start, fc::time_point_sec end)const; 
vector<buy_fundraise_object>        list_buy_fundraise_by_id(issue_fundraise_id_type sel_id,uint64_t start,uint64_t limit) const;
vector<buy_exchange_object>         list_buy_exchange_by_id(sell_exchange_id_type id,  uint64_t start,uint64_t limit) const;

uint32_t                    max_history()const;
const flat_set<uint32_t>&   tracked_buckets()const;
uint32_t                    max_order_his_records_per_market()const;
uint32_t                    max_order_his_seconds_per_market()const;

private:
friend class detail::finance_history_plugin_impl; 
std::unique_ptr < detail::finance_history_plugin_impl > my; 
}; 

}}//graphene::finance_history
  


FC_REFLECT( graphene::finance_history::finance_history_key, (base)(quote)(sequence) )
FC_REFLECT_DERIVED( graphene::finance_history::finance_order_history_object, (graphene::db::object), (key)(time)(op) )
FC_REFLECT( graphene::finance_history::finance_bucket_key, (base)(quote)(seconds)(open) )
FC_REFLECT_DERIVED( graphene::finance_history::finance_bucket_object, (graphene::db::object),
                    (key)
                    (high_base)(high_quote)
                    (low_base)(low_quote)
                    (open_base)(open_quote)
                    (close_base)(close_quote)
                    (base_volume)(quote_volume) )
FC_REFLECT_DERIVED( graphene::finance_history::finance_ticker_object, (graphene::db::object),
                    (base)(quote)
                    (last_day_base)(last_day_quote)
                    (latest_base)(latest_quote)
                    (base_volume)(quote_volume) )
FC_REFLECT_DERIVED( graphene::finance_history::finance_ticker_meta_object, (graphene::db::object),
                    (rolling_min_order_his_id)(skip_min_order_his_id) )
