/*
 * Copyright (c) 2018 oxarbitrage, and contributors.
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

#include "store_db.hpp"


namespace graphene { namespace store_db {

namespace detail
{
   


void store_db_plugin_impl::Prepare_base_operation(const operation &o, const operation_result &result,const transaction_id_type id, const uint64_t & num_trx,const uint64_t & num_op,const fc::time_point_sec &block_time, const uint64_t &block_number)
{

   std::string sinsert = " INSERT INTO `block_operation` ( \
   `block_num`,\
   `block_time`,\
   `trxid`,\
   `trx_num`,\
   `op_num`,\
   `operation` )\
   VALUES ( "\
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"\
   + id.str()+"',"\
   + fc::to_string(num_trx)+"," \
   + fc::to_string(num_op)+"," \
   + fc::to_string(o.which())+");";   
   _store_db_callback(sinsert);
} 
void  store_db_plugin_impl::Prepare_base_transation(const processed_transaction trx , const transaction_id_type id,const uint64_t &num_trx, const fc::time_point_sec &block_time, const uint64_t &block_number)
{
     std::string sinsert = " INSERT INTO `block_transition` ( \
   `block_num`,\
   `block_time`,\
   `trxid`,\
   `trx_num`,\
   `json` )\
   VALUES ( "\
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"\
   + id.str()+"',"\
   + fc::to_string(num_trx)+",'" \
   + to_db_string(trx)+"');";      
   _store_db_callback(sinsert);

}
void store_db_plugin_impl::Prepare_transfer_operation(const transfer_operation &o, const operation_result &result,const transaction_id_type id, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{

   fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
  std::string json3 = "";
  if(o.memo.valid())
  {
    fc::to_variant( *o.memo, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
    json3 = fc::json::to_string(vresult3);
  }
 

  std::string sinsert =  " INSERT INTO `transfer_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `trxid`,\
   `from`,\
   `to`,\
   `amount_id`,\
   `amount_amount`,\
   `memo`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+",'" \
    + id.str() +"'," \
    + fc::to_string(o.from.get_instance())+"," \
    + fc::to_string(o.to.get_instance())+"," \
    + fc::to_string(o.amount.asset_id.get_instance())+"," \
    + fc::to_string(o.amount.amount.value)+",'" \
    + json3+"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   
}
void store_db_plugin_impl::Prepare_limit_order_create_operation(const limit_order_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
  std::string sinsert =  " INSERT INTO `limit_order_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `seller`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.seller.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_limit_order_cancel_operation(const limit_order_cancel_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
     std::string sinsert =  " INSERT INTO `limit_order_cancel_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `fee_paying_account`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.fee_paying_account.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_call_order_update_operation(const call_order_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
     std::string sinsert =  " INSERT INTO `call_order_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `funding_account`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.funding_account.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_fill_order_operation(const fill_order_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
 
   
    std::string sinsert =  " INSERT INTO `fill_order_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `account_id`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.account_id.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_account_create_operation(const account_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{  

  fc::variant vresult1,vresult2;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);

  std::string sinsert =  " INSERT INTO `account_create_operation` ( \
  `fee_asset_id`,\
  `fee_asset_amount`,\
  `registar_id`,\
  `referrer_id`,\
  `referrer_percent`,\
  `name`,\
  `block_num`,\
  `block_time`,\
  `trx_num`,\
  `op_num`,\
  `jason`, \
  `result` )\
  VALUES ( "\
  + fc::to_string(o.fee.asset_id.get_instance())+"," \
  + fc::to_string(o.fee.amount.value)+"," \
  + fc::to_string(o.registrar.get_instance())+"," \
  + fc::to_string(o.referrer.get_instance())+"," \
  + fc::to_string(o.referrer_percent)+",'" \
  + (o.name)+"'," \
  + fc::to_string(block_number)+",'" \
  + block_time.to_db_string()+"',"\
   + fc::to_string(num_trx)+"," \
   + fc::to_string(num_op)+",'" \
   + json1+"','" \
   + json2+"');";

   _store_db_callback(sinsert);   
 

}
 
void store_db_plugin_impl::Prepare_account_authenticate_operation(const account_authenticate_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{std::string sinsert =  " INSERT INTO `account_authenticate_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_account_config_operation(const account_config_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
 std::string sinsert =  " INSERT INTO `account_config_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_account_update_operation(const account_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{   

std::string sinsert =  " INSERT INTO `account_update_operation` ( \
  `fee_asset_id`,\
  `fee_asset_amount`,\
  `account`,\
  `owner`,\
  `active`,\
  `limitactive`,\
  `new_options`,\
  `owner_special_authority`,\
  `active_special_authority`,\
  `block_num`,\
  `block_time`,\
  `trx_num`,\
  `op_num`,\
  `jason`, \
  `result` )\
  VALUES ( "\
  + fc::to_string(o.fee.asset_id.get_instance())+"," \
  + fc::to_string(o.fee.amount.value)+"," \
  + fc::to_string(o.account.get_instance())+",'" \
  + to_optional_string(o. owner)+"','" \
  + to_optional_string(o. active)+"','" \
  + to_optional_string(o.limitactive )+"','" \
  + to_optional_string(o.new_options )+"','" \
  + to_optional_string(o. owner_special_authority)+"','" \
  + to_optional_string(o. active_special_authority)+"'," \
  + fc::to_string(block_number)+",'" \
  + block_time.to_db_string()+"',"\
   + fc::to_string(num_trx)+"," \
   + fc::to_string(num_op)+",'" \
   + to_db_string(o)+"','" \
   + to_db_string(result)+"');";
   _store_db_callback(sinsert);   
}
void store_db_plugin_impl::Prepare_account_whitelist_operation(const account_whitelist_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
  
 
   
    std::string sinsert =  " INSERT INTO `account_whitelist_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `authorizing_account`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.authorizing_account.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  

}
void store_db_plugin_impl::Prepare_account_upgrade_operation(const account_upgrade_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
  std::string sinsert =  " INSERT INTO `account_upgrade_operation` ( \
  `fee_asset_id`,\
  `fee_asset_amount`,\
  `account_to_upgrade`,\
  `upgrade_to_lifetime_member`,\
  `block_num`,\
  `block_time`,\
  `trx_num`,\
  `op_num`,\
  `jason`, \
  `result` )\
  VALUES ( "\
  + fc::to_string(o.fee.asset_id.get_instance())+"," \
  + fc::to_string(o.fee.amount.value)+"," \
  + fc::to_string(o.account_to_upgrade.get_instance())+"," \
  + fc::to_string(o. upgrade_to_lifetime_member)+"," \
  + fc::to_string(block_number)+",'" \
  + block_time.to_db_string()+"',"\
   + fc::to_string(num_trx)+"," \
   + fc::to_string(num_op)+",'" \
   + to_db_string(o)+"','" \
   + to_db_string(result)+"');";
   _store_db_callback(sinsert);   
}
void store_db_plugin_impl::Prepare_account_transfer_operation(const account_transfer_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
      std::string sinsert =  " INSERT INTO `account_transfer_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `account_id`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.account_id.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_asset_create_operation(const asset_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{

std::string sinsert =  " INSERT INTO `asset_create_operation` ( \
  `fee_asset_id`,\
  `fee_asset_amount`,\
  `issuer`,\
  `symbol`,\
  `precision`,\
   `max_supply`,\
  `market_fee_percent`,\
  `max_market_fee`,\
  `issuer_permissions`,\
  `flags`,\
  `core_exchange_rate`,\
  `whitelist_authorities`,\
  `blacklist_authorities`,\
  `whitelist_markets`,\
  `blacklist_markets`,\
  `description`,\
  `bitasset_opts`,\
  `is_prediction_market`,\
  `block_num`,\
  `block_time`,\
  `trx_num`,\
  `op_num`,\
  `jason`, \
  `result` )\
  VALUES ( "\
  + fc::to_string(o.fee.asset_id.get_instance())+"," \
  + fc::to_string(o.fee.amount.value)+"," \
  + fc::to_string(o.issuer.get_instance())+",'" \
  + o. symbol+"'," \
  + fc::to_string(o. precision)+"," \
  +fc::to_string(o.common_options.max_supply.value)+"," \
  + fc::to_string(o.common_options.market_fee_percent)+"," \
  + fc::to_string(o.common_options.max_market_fee.value)+"," \
  + fc::to_string(o.common_options.issuer_permissions)+"," \
  + fc::to_string(o.common_options.flags)+",'" \
  + to_db_string(o.common_options.core_exchange_rate)+"','" \
  + to_db_string(o.common_options.whitelist_authorities)+"','" \
  + to_db_string(o.common_options.blacklist_authorities)+"','" \
  + to_db_string(o.common_options.whitelist_markets)+"','" \
  + to_db_string(o.common_options.blacklist_markets)+"','" \
  + o.common_options.description+"','" \
  + to_optional_string(o. bitasset_opts)+"'," \
  + fc::to_string(o. is_prediction_market)+"," \
  + fc::to_string(block_number)+",'" \
  + block_time.to_db_string()+"',"\
  + fc::to_string(num_trx)+"," \
  + fc::to_string(num_op)+",'" \
  + to_db_string(o)+"','" \
  + to_db_string(result)+"');";
  _store_db_callback(sinsert);   

}
void store_db_plugin_impl::Prepare_asset_update_operation(const asset_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
std::string sinsert =  " INSERT INTO `asset_update_operation` ( \
  `fee_asset_id`,\
  `fee_asset_amount`,\
  `issuer`,\
  `asset_to_update`,\
  `new_issuer`,\
   `max_supply`,\
  `market_fee_percent`,\
  `max_market_fee`,\
  `issuer_permissions`,\
  `flags`,\
  `core_exchange_rate`,\
  `whitelist_authorities`,\
  `blacklist_authorities`,\
  `whitelist_markets`,\
  `blacklist_markets`,\
  `description`,\
  `block_num`,\
  `block_time`,\
  `trx_num`,\
  `op_num`,\
  `jason`, \
  `result` )\
  VALUES ( "\
  + fc::to_string(o.fee.asset_id.get_instance())+"," \
  + fc::to_string(o.fee.amount.value)+"," \
  + fc::to_string(o.issuer.get_instance())+"," \
  + fc::to_string(o.asset_to_update.get_instance())+"," \
  + to_optional_id(o.new_issuer)+"," \
  + fc::to_string(o.new_options.max_supply.value)+"," \
  + fc::to_string(o.new_options.market_fee_percent)+"," \
  + fc::to_string(o.new_options.max_market_fee.value)+"," \
  + fc::to_string(o.new_options.issuer_permissions)+"," \
  + fc::to_string(o.new_options.flags)+",'" \
  + to_db_string(o.new_options.core_exchange_rate)+"','" \
  + to_db_string(o.new_options.whitelist_authorities)+"','" \
  + to_db_string(o.new_options.blacklist_authorities)+"','" \
  + to_db_string(o.new_options.whitelist_markets)+"','" \
  + to_db_string(o.new_options.blacklist_markets)+"','" \
  + o.new_options.description+"'," \
  + fc::to_string(block_number)+",'" \
  + block_time.to_db_string()+"',"\
   + fc::to_string(num_trx)+"," \
   + fc::to_string(num_op)+",'" \
   + to_db_string(o)+"','" \
   + to_db_string(result)+"');";
   _store_db_callback(sinsert);   
}
 
void store_db_plugin_impl::Prepare_asset_update_bitasset_operation(const asset_update_bitasset_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
std::string sinsert =  " INSERT INTO `asset_update_bitasset_operation` ( \
  `fee_asset_id`,\
  `fee_asset_amount`,\
  `issuer`,\
  `asset_to_update`,\
  `feed_lifetime_sec`,\
  `minimum_feeds`,\
  `force_settlement_delay_sec`,\
  `force_settlement_offset_percent`,\
  `maximum_force_settlement_volume`,\
  `short_backing_asset`,\
  `block_num`,\
  `block_time`,\
  `trx_num`,\
  `op_num`,\
  `jason`, \
  `result` )\
  VALUES ( "\
  + fc::to_string(o.fee.asset_id.get_instance())+"," \
  + fc::to_string(o.fee.amount.value)+"," \
  + fc::to_string(o.issuer.get_instance())+"," \
  + fc::to_string(o.asset_to_update.get_instance())+"," \
  + fc::to_string(o.new_options.feed_lifetime_sec)+"," \
  + fc::to_string(o.new_options.minimum_feeds)+"," \
  + fc::to_string(o.new_options.force_settlement_delay_sec)+"," \
  + fc::to_string(o.new_options.maximum_force_settlement_volume)+"," \
  + fc::to_string(o.new_options.short_backing_asset.get_instance())+"," \
  + fc::to_string(block_number)+",'" \
  + block_time.to_db_string()+"',"\
   + fc::to_string(num_trx)+"," \
   + fc::to_string(num_op)+",'" \
   + to_db_string(o)+"','" \
   + to_db_string(result)+"');";
   _store_db_callback(sinsert);   

}
void store_db_plugin_impl::Prepare_asset_update_feed_producers_operation(const asset_update_feed_producers_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{   
  std::string sinsert =  " INSERT INTO `asset_update_feed_producers_operation` ( \
  `fee_asset_id`,\
  `fee_asset_amount`,\
  `issuer`,\
  `asset_to_update`,\
  `new_feed_producers`,\
  `block_num`,\
  `block_time`,\
  `trx_num`,\
  `op_num`,\
  `jason`, \
  `result` )\
  VALUES ( "\
  + fc::to_string(o.fee.asset_id.get_instance())+"," \
  + fc::to_string(o.fee.amount.value)+"," \
  + fc::to_string(o.issuer.get_instance())+"," \
  + fc::to_string(o.asset_to_update.get_instance())+",'" \
  + to_db_string(o.new_feed_producers)+"'," \
  + fc::to_string(block_number)+",'" \
  + block_time.to_db_string()+"',"\
   + fc::to_string(num_trx)+"," \
   + fc::to_string(num_op)+",'" \
   + to_db_string(o)+"','" \
   + to_db_string(result)+"');";
   _store_db_callback(sinsert);   
}
void store_db_plugin_impl::Prepare_asset_issue_operation(const asset_issue_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ 
   
    std::string sinsert =  " INSERT INTO `asset_issue_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_asset_reserve_operation(const asset_reserve_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{    std::string sinsert =  " INSERT INTO `asset_reserve_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `payer`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.payer.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_asset_fund_fee_pool_operation(const asset_fund_fee_pool_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{    std::string sinsert =  " INSERT INTO `asset_fund_fee_pool_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `from_account`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.from_account.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_asset_settle_operation(const asset_settle_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{    std::string sinsert =  " INSERT INTO `asset_settle_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `account`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.account.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_asset_global_settle_operation(const asset_global_settle_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
    std::string sinsert =  " INSERT INTO `asset_global_settle_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_asset_publish_feed_operation(const asset_publish_feed_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
   fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
 
   
    std::string sinsert =  " INSERT INTO `asset_publish_feed_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `publisher`,\
   `asset_id`,\
   `settlement_base_id`,\
   `settlement_base_amount`,\
   `settlement_quote_id`,\
   `settlement_quote_amount`,\
   `maintenance_collateral_ratio`,\
   `maximum_short_squeeze_ratio`,\
   `core_base_id`,\
   `core_base_amount`,\
   `core_quote_id`,\
   `core_quote_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.publisher.get_instance())+"," \
    + fc::to_string(o.asset_id.get_instance())+"," \
    + fc::to_string(o.feed.settlement_price.base.asset_id.get_instance())+"," \
    + fc::to_string(o.feed.settlement_price.base.amount.value)+"," \
    + fc::to_string(o.feed.settlement_price.quote.asset_id.get_instance())+"," \
    + fc::to_string(o.feed.settlement_price.quote.amount.value)+"," \
    + fc::to_string(o.feed.maintenance_collateral_ratio)+"," \
    + fc::to_string(o.feed.maximum_short_squeeze_ratio)+"," \
    + fc::to_string(o.feed.core_exchange_rate.base.asset_id.get_instance())+"," \
    + fc::to_string(o.feed.core_exchange_rate.base.amount.value)+"," \
    + fc::to_string(o.feed.core_exchange_rate.quote.asset_id.get_instance())+"," \
    + fc::to_string(o.feed.core_exchange_rate.quote.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);

}  

void store_db_plugin_impl::Prepare_witness_create_operation(const witness_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{
 
  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2); 
   
    std::string sinsert =  " INSERT INTO `witness_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `witness_account`,\
   `url`,\
   `memo`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.witness_account.get_instance())+",'" \
    + chang_db_string(o.url)  +"','" \
    + chang_db_string(o.memo) +"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  

}
void store_db_plugin_impl::Prepare_witness_update_operation(const witness_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
 
 fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2); 
   std::string json3 = "";
  if(o.new_signing_key.valid())
  {
    fc::to_variant( *o.new_signing_key, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
    json3 = fc::json::to_string(vresult3);
  }

    std::string sinsert =  " INSERT INTO `witness_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `witness`,\
   `witness_account`,\
   `new_url`,\
   `new_memo`,\
   `new_signing_key`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.witness.get_instance())+"," \
    + fc::to_string(o.witness_account.get_instance())+",'" \
    + (o.new_url.valid() ? *o.new_url : "") +"','" \
    + (o.new_memo.valid() ? *o.new_memo : "") +"','" \
    + json3+"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  

}

void store_db_plugin_impl::Prepare_proposal_create_operation(const proposal_create_operation &o, const operation_result &result,const transaction_id_type id, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{

  fc::variant vresult1,vresult2,vresult3,vresult4;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
   std::string json3 = "";
  if(o.memo.valid())
  {
    fc::to_variant( *o.memo, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
    json3 = fc::json::to_string(vresult3);
  }
  fc::to_variant( o.proposed_ops, vresult4, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json4 = fc::json::to_string(vresult4);

  std::string sinsert =  " INSERT INTO `proposal_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `fee_paying_account`,\
   `expiration_time`,\
   `proposed_ops`,\
   `review_period_seconds`,\
   `memo`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.fee_paying_account.get_instance())+",'" \
    + o.expiration_time.to_db_string()+"','" \
    + json4+"'," \
    + fc::to_string(o.review_period_seconds.valid() ?*o.review_period_seconds:0)+",'" \
    + json3+"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);

   operation_result op_result;
   uint64_t num_opex = num_op *1000;
   for (const auto &o_op : o.proposed_ops)
   {
     //Prepare_base_operation(o_op, o_tr.operation_results[num_op], num_trx, num_op, time_block, num_block);
     updateBlockOperation(o_op.op, op_result, id,num_trx, num_opex, block_time, block_number);
     num_opex++;
   }

}
void store_db_plugin_impl::Prepare_proposal_update_operation(const proposal_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ 
  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);

  fc::variant vresult_0,vresult_1,vresult_2,vresult_3,vresult_4,vresult_5;
  fc::to_variant( o.active_approvals_to_add, vresult_0, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json_0 = fc::json::to_string(vresult_0);
  fc::to_variant( o.active_approvals_to_remove, vresult_1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json_1 = fc::json::to_string(vresult_1);
  fc::to_variant( o.owner_approvals_to_add, vresult_2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json_2 = fc::json::to_string(vresult_2);
  fc::to_variant( o.owner_approvals_to_remove, vresult_3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json_3 = fc::json::to_string(vresult_3);
  fc::to_variant( o.key_approvals_to_add, vresult_4, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json_4 = fc::json::to_string(vresult_4);
  fc::to_variant( o.key_approvals_to_remove, vresult_5, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json_5 = fc::json::to_string(vresult_5);
   

  std::string sinsert =  " INSERT INTO `proposal_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `fee_paying_account`,\
   `proposal`,\
   `active_approvals_to_add`,\
   `active_approvals_to_remove`,\
   `owner_approvals_to_add`,\
   `owner_approvals_to_remove`,\
   `key_approvals_to_add`,\
   `key_approvals_to_remove`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.fee_paying_account.get_instance())+"," \
    + fc::to_string(o.proposal.get_instance())+",'" \
    + json_0+"','" \
    + json_1+"','" \
    + json_2+"','" \
    + json_3+"','" \
    + json_4+"','" \
    + json_5+"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   


}
void store_db_plugin_impl::Prepare_proposal_delete_operation(const proposal_delete_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);

  std::string sinsert =  " INSERT INTO `proposal_delete_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `fee_paying_account`,\
   `using_owner_authority`,\
   `proposal`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.fee_paying_account.get_instance())+"," \
    + fc::to_string(o.using_owner_authority)+"," \
    + fc::to_string(o.proposal.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   
}
void store_db_plugin_impl::Prepare_withdraw_permission_create_operation(const withdraw_permission_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ std::string sinsert =  " INSERT INTO `withdraw_permission_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `withdraw_from_account`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.withdraw_from_account.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_withdraw_permission_update_operation(const withdraw_permission_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
      std::string sinsert =  " INSERT INTO `withdraw_permission_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `withdraw_from_account`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.withdraw_from_account.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_withdraw_permission_claim_operation(const withdraw_permission_claim_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
      std::string sinsert =  " INSERT INTO `withdraw_permission_claim_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `withdraw_from_account`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.withdraw_from_account.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_withdraw_permission_delete_operation(const withdraw_permission_delete_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{   
  std::string sinsert =  " INSERT INTO `withdraw_permission_delete_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `withdraw_from_account`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.withdraw_from_account.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
  }
void store_db_plugin_impl::Prepare_committee_member_create_operation(const committee_member_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
 {

  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2); 
   
    std::string sinsert =  " INSERT INTO `committee_member_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `committee_member_account`,\
   `url`,\
   `memo`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.committee_member_account.get_instance())+",'" \
    + chang_db_string(o.url)  +"','" \
    + chang_db_string(o.memo) +"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  



 }
void store_db_plugin_impl::Prepare_committee_member_update_operation(const committee_member_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
  
 fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2); 
  

    std::string sinsert =  " INSERT INTO `committee_member_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `committee_member`,\
   `committee_member_account`,\
   `new_url`,\
   `new_memo`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.committee_member.get_instance())+"," \
    + fc::to_string(o.committee_member_account.get_instance())+",'" \
    + (o.new_url.valid() ? *o.new_url : "") +"','" \
    + (o.new_memo.valid() ? *o.new_memo : "") +"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  


}
void store_db_plugin_impl::Prepare_committee_member_update_global_parameters_operation(const committee_member_update_global_parameters_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ 
   fc::variant vresult1,vresult2,vresult3;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2);
   fc::to_variant( o.new_parameters, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json3 = fc::json::to_string(vresult3);
 

   std::string sinsert =  " INSERT INTO `committee_member_update_global_parameters_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `new_parameters`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
    VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+",'" \
    + json3+"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert); 

}
void store_db_plugin_impl::Prepare_vesting_balance_create_operation(const vesting_balance_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
   fc::variant vresult1,vresult2,vresult3;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2);
   fc::to_variant( o.policy, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json3 = fc::json::to_string(vresult3);
 

   std::string sinsert =  " INSERT INTO `vesting_balance_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `creator`,\
   `owner`,\
   `amount_id`,\
   `amount_amount`,\
   `policy`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
    VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.creator.get_instance())+"," \
    + fc::to_string(o.owner.get_instance())+"," \
    + fc::to_string(o.amount.asset_id.get_instance())+"," \
    + fc::to_string(o.amount.amount.value)+",'" \
    + json3+"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert); 

}
void store_db_plugin_impl::Prepare_vesting_balance_withdraw_operation(const vesting_balance_withdraw_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
 
 

       fc::variant vresult1,vresult2,vresult3;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2);
 

   std::string sinsert =  " INSERT INTO `vesting_balance_withdraw_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `vesting_balance`,\
   `owner`,\
   `amount_id`,\
   `amount_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
    VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.vesting_balance.get_instance())+"," \
    + fc::to_string(o.owner.get_instance())+"," \
    + fc::to_string(o.amount.asset_id.get_instance())+"," \
    + fc::to_string(o.amount.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert); 

}
void store_db_plugin_impl::Prepare_worker_create_operation(const worker_create_operation &o ,const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ 
   fc::variant vresult1,vresult2,vresult3;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2);
   fc::to_variant( o.initializer, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json3 = fc::json::to_string(vresult3); 

   std::string sinsert =  " INSERT INTO `worker_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `owner`,\
   `work_begin_date`,\
   `work_end_date`,\
   `daily_pay`,\
   `name`,\
   `url`,\
   `memo`,\
   `initializer`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
    VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.owner.get_instance())+",'" \
    + o.work_begin_date.to_db_string()+"','"\
    + o.work_end_date.to_db_string()+"',"\
    + fc::to_string(o.daily_pay.value)+",'" \
    + chang_db_string(o.name) +  "','" \
    + chang_db_string(o.url)+  "','" \
    + chang_db_string(o.memo)+  "','" \
    + json3+  "'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert); 


}
void store_db_plugin_impl::Prepare_custom_operation(const custom_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{   
  std::string sinsert =  " INSERT INTO `custom_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `payer`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.payer.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_assert_operation(const assert_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ 
      std::string sinsert =  " INSERT INTO `assert_operation ` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `fee_paying_account`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.fee_paying_account.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_balance_claim_operation(const balance_claim_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
 
   fc::variant vresult1,vresult2,vresult3;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2);
   fc::to_variant( o.balance_owner_key, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json3 = fc::json::to_string(vresult3); 

   std::string sinsert =  " INSERT INTO `balance_claim_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `deposit_to_account`,\
   `balance_to_claim`,\
   `balance_owner_id`,\
   `balance_owner_key`,\
   `total_claimed_id`,\
   `total_claimed_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
    VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.deposit_to_account.get_instance())+"," \
    + fc::to_string(o.balance_to_claim.get_instance())+"," \
    + fc::to_string(o.balance_owner_id.get_instance())+",'" \
    + json3+  "'," \
    + fc::to_string(o.total_claimed.asset_id.get_instance())+"," \
    + fc::to_string(o.total_claimed.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert); 


}
void store_db_plugin_impl::Prepare_override_transfer_operation(const override_transfer_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
     std::string sinsert =  " INSERT INTO `override_transfer_operation ` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_transfer_to_blind_operation(const transfer_to_blind_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
     std::string sinsert =  " INSERT INTO `transfer_to_blind_operation ` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `from`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.from.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_blind_transfer_operation(const blind_transfer_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
      std::string sinsert =  " INSERT INTO `blind_transfer_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_transfer_from_blind_operation(const transfer_from_blind_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
      std::string sinsert =  " INSERT INTO `transfer_from_blind_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `to`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.to.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_asset_settle_cancel_operation(const asset_settle_cancel_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
  std::string sinsert =  " INSERT INTO `asset_settle_cancel_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_asset_claim_fees_operation(const asset_claim_fees_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{

   fc::variant vresult1,vresult2,vresult3;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2);
 

   std::string sinsert =  " INSERT INTO `asset_claim_fees_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `amount_to_claim_id`,\
   `amount_to_claim_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
    VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.amount_to_claim.asset_id.get_instance())+"," \
    + fc::to_string(o.amount_to_claim.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert); 
}
void store_db_plugin_impl::Prepare_fba_distribute_operation(const fba_distribute_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
      std::string sinsert =  " INSERT INTO `fba_distribute_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_bid_collateral_operation(const bid_collateral_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{   std::string sinsert =  " INSERT INTO `bid_collateral_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_execute_bid_operation(const execute_bid_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ std::string sinsert =  " INSERT INTO `execute_bid_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_asset_reserve_fees_operation(const asset_reserve_fees_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
   std::string sinsert =  " INSERT INTO `asset_reserve_fees_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_asset_property_operation(const asset_property_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
   

   fc::variant vresult1,vresult2,vresult3;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2);
 

   std::string sinsert =  " INSERT INTO `asset_property_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `asset_id`,\
   `uasset_property`,\
   `uasset_mask`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
    VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.asset_id.get_instance())+"," \
    + fc::to_string(o.uasset_property)+"," \
    + fc::to_string(o.uasset_mask)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert); 


}
void store_db_plugin_impl::Prepare_gateway_withdraw_operation(const gateway_withdraw_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{  
   fc::variant vresult1,vresult2,vresult3;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2);
 

   std::string sinsert =  " INSERT INTO `gateway_withdraw_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `from`,\
   `to`,\
   `withdraw_id`,\
   `withdraw_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
    VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.from.get_instance())+"," \
    + fc::to_string(o.to.get_instance())+"," \
    + fc::to_string(o.withdraw.asset_id.get_instance())+"," \
    + fc::to_string(o.withdraw.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert); 
}
void store_db_plugin_impl::Prepare_gateway_deposit_operation(const gateway_deposit_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ 
   fc::variant vresult1,vresult2,vresult3;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2);
 

   std::string sinsert =  " INSERT INTO `gateway_deposit_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `from`,\
   `to`,\
   `deposit_id`,\
   `deposit_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
    VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.from.get_instance())+"," \
    + fc::to_string(o.to.get_instance())+"," \
    + fc::to_string(o.deposit.asset_id.get_instance())+"," \
    + fc::to_string(o.deposit.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert); 

}
void store_db_plugin_impl::Prepare_gateway_issue_currency_operation(const gateway_issue_currency_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
  
   fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
  std::string json3 = "";
  if(o.memo.valid())
  {
    fc::to_variant( *o.memo, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
    json3 = fc::json::to_string(vresult3);
  }
 

  std::string sinsert =  " INSERT INTO `gateway_issue_currency_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `account_to`,\
   `issue_currency_id`,\
   `issue_currency_amount`,\
   `revoke`,\
   `memo`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.account_to.get_instance())+"," \
    + fc::to_string(o.issue_currency.asset_id.get_instance())+"," \
    + fc::to_string(o.issue_currency.amount.value)+"," \
    + fc::to_string(o.revoke)+",'" \
    + json3+"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   

}
void store_db_plugin_impl::Prepare_bitlender_option_create_operation(const bitlender_option_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
   

   fc::variant vresult1,vresult2;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2);
 
  fc::variant vresult3;
  fc::to_variant( o.options.carriers, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json3 = fc::json::to_string(vresult3);

  fc::variant vresult4;
  fc::to_variant( o.options.allowed_collateralize, vresult4, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json4 = fc::json::to_string(vresult4);

   std::string sinsert =  " INSERT INTO `bitlender_option_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `asset_id`,\
   `issuer`,\
   `sproduct`,\
   `issuer_rate`,\
   `max_repayment_period`,\
   `repayment_period_uint`,\
   `max_bid_time`,\
   `max_overdue_period`,\
   `notify_overdue_time`,\
   `overdue_expiration_time`,\
   `repayment_type`,\
   `repayment_date`,\
   `min_invest_amount`,\
   `min_loan_amount`,\
   `min_invest_increase_range`,\
   `min_loan_increase_range`,\
   `max_repayment_rate`,\
   `max_interest_rate`,\
   `risk_margin_rate`,\
   `max_risk_margin`,\
   `carrier_service_charge_rate`,\
   `max_carrier_service_charge`,\
   `carrier_service_loan_carrier`,\
   `carrier_service_invest_carrier`,\
   `platform_service_charge_rate`,\
   `max_platform_service_charge_rate`,\
   `platform_service_loan_carrier`,\
   `platform_service_invest_carrier`,\
   `platform_service_loan_refer`,\
   `platform_service_invest_refer`,\
   `platform_service_gateway`,\
   `platform_service_platform`,\
   `overdue_liquidation_rate`,\
   `overdue_penalty_interest_rate`,\
   `early_repayment_liquidation_rate`,\
   `overdue_penalty_date_interest_rate`,\
   `principal_guarantee_rate`,\
   `platform_id`,\
   `carriers`,\
   `allowed_collateralize`,\
   `time_zone`,\
   `fee_mode`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
    VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.asset_id.get_instance())+"," \
    + fc::to_string(o.issuer.get_instance())+",'" \
    + o.sproduct+"'," \
    + fc::to_string(o.options.issuer_rate.get_instance()) +","\
    + fc::to_string(o.options.max_repayment_period) +","\
    + fc::to_string(o.options.repayment_period_uint) +","\
    + fc::to_string(o.options.max_bid_time) +","\
    + fc::to_string(o.options.max_overdue_period) +","\
    + fc::to_string(o.options.notify_overdue_time) +","\
    + fc::to_string(o.options.overdue_expiration_time) +","\
    + fc::to_string(o.options.repayment_type) +","\
    + fc::to_string(o.options.repayment_date) +","\
    + fc::to_string(o.options.min_invest_amount.value) +","\
    + fc::to_string(o.options.min_loan_amount.value) +","\
    + fc::to_string(o.options.min_invest_increase_range.value) +","\
    + fc::to_string(o.options.min_loan_increase_range.value) +","\
    + fc::to_string(o.options.max_repayment_rate) +","\
    + fc::to_string(o.options.max_interest_rate.value) +","\
    + fc::to_string(o.options.risk_margin_rate) +","\
    + fc::to_string(o.options.max_risk_margin.value) +","\
    + fc::to_string(o.options.carrier_service_charge_rate) +","\
    + fc::to_string(o.options.max_carrier_service_charge.value) +","\
    + fc::to_string(o.options.carrier_service_loan_carrier) +","\
    + fc::to_string(o.options.carrier_service_invest_carrier) +","\
    + fc::to_string(o.options.platform_service_charge_rate) +","\
    + fc::to_string(o.options.max_platform_service_charge_rate.value) +","\
    + fc::to_string(o.options.platform_service_loan_carrier) +","\
    + fc::to_string(o.options.platform_service_invest_carrier) +","\
    + fc::to_string(o.options.platform_service_loan_refer) +","\
    + fc::to_string(o.options.platform_service_invest_refer) +","\
    + fc::to_string(o.options.platform_service_gateway) +","\
    + fc::to_string(o.options.platform_service_platform) +","\
    + fc::to_string(o.options.overdue_liquidation_rate) +","\
    + fc::to_string(o.options.overdue_penalty_interest_rate) +","\
    + fc::to_string(o.options.early_repayment_liquidation_rate) +","\
    + fc::to_string(o.options.overdue_penalty_date_interest_rate) +","\
    + fc::to_string(o.options.principal_guarantee_rate) +","\
    + fc::to_string(o.options.platform_id.get_instance()) +",'"\
    + json3 +"','"\
    + json4  +"',"\
    + fc::to_string(o.options.time_zone) +","\
    + fc::to_string(o.fee_mode) +","\
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_bitlender_option_author_operation(const bitlender_option_author_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
 
  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
  fc::to_variant( o.authors, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json3 = fc::json::to_string(vresult3);

  std::string sinsert =  " INSERT INTO `bitlender_option_author_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `option_id`,\
   `authors`,\
   `weight_threshold`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.option_id.get_instance())+",'" \
    + json3 +"'," \
    + fc::to_string(o.weight_threshold)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);     

}
void store_db_plugin_impl::Prepare_bitlender_option_fee_mode_operation(const bitlender_option_fee_mode_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
   fc::variant vresult1,vresult2;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2);

   std::string sinsert =  " INSERT INTO `bitlender_option_fee_mode_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `option_id`,\
   `fee_mode`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.option_id.get_instance())+"," \
    + fc::to_string(o.fee_mode)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   
}
void store_db_plugin_impl::Prepare_bitlender_option_update_operation(const bitlender_option_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{

   fc::variant vresult1,vresult2;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2);
 
  fc::variant vresult3;
  fc::to_variant( o.options.carriers, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json3 = fc::json::to_string(vresult3);

  fc::variant vresult4;
  fc::to_variant( o.options.allowed_collateralize, vresult4, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json4 = fc::json::to_string(vresult4);

   std::string sinsert =  " INSERT INTO `bitlender_option_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `option_id`,\
   `author`,\
   `issuer_rate`,\
   `max_repayment_period`,\
   `repayment_period_uint`,\
   `max_bid_time`,\
   `max_overdue_period`,\
   `notify_overdue_time`,\
   `overdue_expiration_time`,\
   `repayment_type`,\
   `repayment_date`,\
   `min_invest_amount`,\
   `min_loan_amount`,\
   `min_invest_increase_range`,\
   `min_loan_increase_range`,\
   `max_repayment_rate`,\
   `max_interest_rate`,\
   `risk_margin_rate`,\
   `max_risk_margin`,\
   `carrier_service_charge_rate`,\
   `max_carrier_service_charge`,\
   `carrier_service_loan_carrier`,\
   `carrier_service_invest_carrier`,\
   `platform_service_charge_rate`,\
   `max_platform_service_charge_rate`,\
   `platform_service_loan_carrier`,\
   `platform_service_invest_carrier`,\
   `platform_service_loan_refer`,\
   `platform_service_invest_refer`,\
   `platform_service_gateway`,\
   `platform_service_platform`,\
   `overdue_liquidation_rate`,\
   `overdue_penalty_interest_rate`,\
   `early_repayment_liquidation_rate`,\
   `overdue_penalty_date_interest_rate`,\
   `principal_guarantee_rate`,\
   `platform_id`,\
   `carriers`,\
   `allowed_collateralize`,\
   `time_zone`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
    VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.option_id.get_instance())+"," \
    + fc::to_string(o.author.get_instance())+"," \
    + fc::to_string(o.options.issuer_rate.get_instance()) +","\
    + fc::to_string(o.options.max_repayment_period) +","\
    + fc::to_string(o.options.repayment_period_uint) +","\
    + fc::to_string(o.options.max_bid_time) +","\
    + fc::to_string(o.options.max_overdue_period) +","\
    + fc::to_string(o.options.notify_overdue_time) +","\
    + fc::to_string(o.options.overdue_expiration_time) +","\
    + fc::to_string(o.options.repayment_type) +","\
    + fc::to_string(o.options.repayment_date) +","\
    + fc::to_string(o.options.min_invest_amount.value) +","\
    + fc::to_string(o.options.min_loan_amount.value) +","\
    + fc::to_string(o.options.min_invest_increase_range.value) +","\
    + fc::to_string(o.options.min_loan_increase_range.value) +","\
    + fc::to_string(o.options.max_repayment_rate) +","\
    + fc::to_string(o.options.max_interest_rate.value) +","\
    + fc::to_string(o.options.risk_margin_rate) +","\
    + fc::to_string(o.options.max_risk_margin.value) +","\
    + fc::to_string(o.options.carrier_service_charge_rate) +","\
    + fc::to_string(o.options.max_carrier_service_charge.value) +","\
    + fc::to_string(o.options.carrier_service_loan_carrier) +","\
    + fc::to_string(o.options.carrier_service_invest_carrier) +","\
    + fc::to_string(o.options.platform_service_charge_rate) +","\
    + fc::to_string(o.options.max_platform_service_charge_rate.value) +","\
    + fc::to_string(o.options.platform_service_loan_carrier) +","\
    + fc::to_string(o.options.platform_service_invest_carrier) +","\
    + fc::to_string(o.options.platform_service_loan_refer) +","\
    + fc::to_string(o.options.platform_service_invest_refer) +","\
    + fc::to_string(o.options.platform_service_gateway) +","\
    + fc::to_string(o.options.platform_service_platform) +","\
    + fc::to_string(o.options.overdue_liquidation_rate) +","\
    + fc::to_string(o.options.overdue_penalty_interest_rate) +","\
    + fc::to_string(o.options.early_repayment_liquidation_rate) +","\
    + fc::to_string(o.options.overdue_penalty_date_interest_rate) +","\
    + fc::to_string(o.options.principal_guarantee_rate) +","\
    + fc::to_string(o.options.platform_id.get_instance()) +",'"\
    + json3 +"','"\
    + json4  +"',"\
    + fc::to_string(o.options.time_zone) +","\
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert); 
}
void store_db_plugin_impl::Prepare_bitlender_option_stop_operation(const bitlender_option_stop_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{
  
 fc::variant vresult1,vresult2;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2);

  std::string sinsert =  " INSERT INTO `bitlender_option_stop_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `option_id`,\
   `author`,\
   `stop`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.option_id.get_instance())+"," \
    + fc::to_string(o.author.get_instance()) +"," \
    + fc::to_string(o.stop)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   
}
void store_db_plugin_impl::Prepare_bitlender_rate_update_operation(const bitlender_rate_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ 

   fc::variant vresult1,vresult2;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2);

   fc::variant vresult3,vresult4;
   fc::to_variant( o.interest_rate_add, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json3 = fc::json::to_string(vresult3);
   fc::to_variant( o.interest_rate_remove, vresult4, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json4 = fc::json::to_string(vresult4);

  std::string sinsert =  " INSERT INTO `bitlender_rate_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `option_id`,\
   `interest_rate_add`,\
   `interest_rate_remove`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.option_id.get_instance())+",'" \
    + json3 +"','" \
    + json4 +"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   

}
void store_db_plugin_impl::Prepare_bitlender_loan_operation(const bitlender_loan_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{   

   fc::variant vresult1,vresult2;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2); 

  std::string sinsert =  " INSERT INTO `bitlender_loan_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `carrier`,\
   `order`,\
   `amount_to_loan_id`,\
   `amount_to_loan_amount`,\
   `loan_period`,\
   `interest_rate`,\
   `repayment_type`,\
   `amount_to_collateralize_id`,\
   `amount_to_collateralize_amount`,\
   `collateral_rate`,\
   `bid_period`,\
   `memo`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.carrier.get_instance())+",'" \
    + chang_db_string(o.order) +"'," \
    + fc::to_string(o.amount_to_loan.asset_id.get_instance())+"," \
    + fc::to_string(o.amount_to_loan.amount.value)+"," \
    + fc::to_string(o.loan_period)+"," \
    + fc::to_string(o.interest_rate)+"," \
    + fc::to_string(o.repayment_type)+"," \
    + fc::to_string(o.amount_to_collateralize.asset_id.get_instance())+"," \
    + fc::to_string(o.amount_to_collateralize.amount.value)+"," \
    + fc::to_string(o.collateral_rate)+"," \
    + fc::to_string(o.bid_period)+",'" \
    + (o.memo.valid() ? *o.memo : " ")+"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   


}
void store_db_plugin_impl::Prepare_bitlender_invest_operation(const bitlender_invest_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{   
     fc::variant vresult1,vresult2;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2); 

  std::string sinsert =  " INSERT INTO `bitlender_invest_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `carrier`,\
   `loan_id`,\
   `order_id`,\
   `amount_to_invest_id`,\
   `amount_to_invest_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.carrier.get_instance())+"," \
    + fc::to_string(o.loan_id.get_instance())+"," \
    + fc::to_string(o.order_id.get_instance())+"," \
    + fc::to_string(o.amount_to_invest.asset_id.get_instance())+"," \
    + fc::to_string(o.amount_to_invest.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   

}
void store_db_plugin_impl::Prepare_bitlender_repay_interest_operation(const bitlender_repay_interest_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{ 
   fc::variant vresult1,vresult2;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2); 
   
  std::string sinsert =  " INSERT INTO `bitlender_repay_interest_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `order_id`,\
   `repay_period`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.order_id.get_instance())+"," \
    + fc::to_string(o.repay_period)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   

}


void store_db_plugin_impl::Prepare_bitlender_recycle_interest_operation(const bitlender_recycle_interest_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{
     fc::variant vresult1,vresult2;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2); 
   
  std::string sinsert =  " INSERT INTO `bitlender_recycle_interest_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `order_id`,\
   `repay_period`,\
   `memo`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.order_id.get_instance())+"," \
    + fc::to_string(o.repay_period)+",'" \
    + to_optional_string(o.memo)+"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   

}

void store_db_plugin_impl::Prepare_bitlender_overdue_interest_operation(const bitlender_overdue_interest_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{
     fc::variant vresult1,vresult2;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2); 
   
  std::string sinsert =  " INSERT INTO `bitlender_overdue_interest_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `order_id`,\
   `repay_period`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.order_id.get_instance())+"," \
    + fc::to_string(o.repay_period)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   

}
void store_db_plugin_impl::Prepare_bitlender_repay_principal_operation(const bitlender_repay_principal_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{
     fc::variant vresult1,vresult2;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2); 
   
  std::string sinsert =  " INSERT INTO `bitlender_repay_principal_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `order_id`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.order_id.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   

 

}
void store_db_plugin_impl::Prepare_bitlender_pre_repay_principal_operation(const bitlender_pre_repay_principal_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{
       fc::variant vresult1,vresult2;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2); 
   
  std::string sinsert =  " INSERT INTO `bitlender_pre_repay_principal_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `order_id`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.order_id.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);     


}
void store_db_plugin_impl::Prepare_bitlender_overdue_repay_principal_operation(const bitlender_overdue_repay_principal_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{
         fc::variant vresult1,vresult2;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2); 
   
  std::string sinsert =  " INSERT INTO `bitlender_overdue_repay_principal_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `order_id`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.order_id.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   
}
void store_db_plugin_impl::Prepare_bitlender_add_collateral_operation(const bitlender_add_collateral_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ 
    fc::variant vresult1,vresult2;
   fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json1 = fc::json::to_string(vresult1);
   fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json2 = fc::json::to_string(vresult2); 
   
  std::string sinsert =  " INSERT INTO `bitlender_add_collateral_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `order_id`,\
   `collateral_id`,\
   `collateral_amount`,\
   `collateral_rate`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.order_id.get_instance())+"," \
    + fc::to_string(o.collateral.asset_id.get_instance())+"," \
    + fc::to_string(o.collateral.amount.value)+"," \
    + fc::to_string(o.collateral_rate)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   
}
void store_db_plugin_impl::Prepare_bitlender_recycle_operation(const bitlender_recycle_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{
  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
  std::string json3 = "";
  if(o.memo.valid())
  {
    fc::to_variant( *o.memo, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
    json3 = fc::json::to_string(vresult3);
  }

    std::string sinsert =  " INSERT INTO `bitlender_recycle_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `order_id`,\
   `asset_pay`,\
   `memo`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.order_id.get_instance())+"," \
    + fc::to_string(o.asset_pay.value)+",'" \
    + json3+"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);   


}
void store_db_plugin_impl::Prepare_bitlender_setautorepayer_operation(const bitlender_setautorepayer_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{

  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
 

    std::string sinsert =  " INSERT INTO `bitlender_setautorepayer_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `order_id`,\
   `bset`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.order_id.get_instance())+"," \
    + fc::to_string(o.bset)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);
  
}
void store_db_plugin_impl::Prepare_bitlender_autorepayment_operation(const bitlender_autorepayment_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
 {

  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
 

    std::string sinsert =  " INSERT INTO `bitlender_autorepayment_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `order_id`,\
   `bset`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.order_id.get_instance())+"," \
    + fc::to_string(o.repay_period)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);
  
 }
 void store_db_plugin_impl::Prepare_bitlender_paramers_update_operation(const bitlender_paramers_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
 {
 

  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
 

    std::string sinsert =  " INSERT INTO `bitlender_paramers_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `max_rate_month`,\
   `max_platform_service_rate`,\
   `max_carrier_service_rate`,\
   `max_risk_margin_rate`,\
   `min_platform_service_rate`,\
   `max_repayment_rate`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.options.max_rate_month )+"," \
    + fc::to_string(o.options.max_platform_service_rate )+"," \
    + fc::to_string(o.options.max_carrier_service_rate )+"," \
    + fc::to_string(o.options.max_risk_margin_rate )+"," \
    + fc::to_string(o.options.min_platform_service_rate )+"," \
    + fc::to_string(o.options.max_repayment_rate )+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);




 }
 void store_db_plugin_impl::Prepare_bitlender_remove_operation(const bitlender_remove_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
 {
    fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
 

    std::string sinsert =  " INSERT INTO `bitlender_remove_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `order_id`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.order_id.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);
 }
void store_db_plugin_impl::Prepare_bitlender_squeeze_operation(const bitlender_squeeze_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
      fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
 

    std::string sinsert =  " INSERT INTO `bitlender_squeeze_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `order_id`,\
   `bset`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.order_id.get_instance())+"," \
    + fc::to_string(o.squeeze_state)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);
}
void store_db_plugin_impl::Prepare_bitlender_publish_feed_operation(const bitlender_publish_feed_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
 {
  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
 
    
    std::string sinsert =  " INSERT INTO `bitlender_publish_feed_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `publisher`,\
   `asset_id`,\
   `settlement_base_id`,\
   `settlement_base_amount`,\
   `settlement_quote_id`,\
   `settlement_quote_amount`,\
   `maintenance_collateral_ratio`,\
   `maximum_short_squeeze_ratio`,\
   `core_base_id`,\
   `core_base_amount`,\
   `core_quote_id`,\
   `core_quote_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.publisher.get_instance())+"," \
    + fc::to_string(o.asset_id.get_instance())+"," \
    + fc::to_string(o.feed.settlement_price.base.asset_id.get_instance())+"," \
    + fc::to_string(o.feed.settlement_price.base.amount.value)+"," \
    + fc::to_string(o.feed.settlement_price.quote.asset_id.get_instance())+"," \
    + fc::to_string(o.feed.settlement_price.quote.amount.value)+"," \
    + fc::to_string(o.feed.maintenance_collateral_ratio)+"," \
    + fc::to_string(o.feed.maximum_short_squeeze_ratio)+"," \
    + fc::to_string(o.feed.core_exchange_rate.base.asset_id.get_instance())+"," \
    + fc::to_string(o.feed.core_exchange_rate.base.amount.value)+"," \
    + fc::to_string(o.feed.core_exchange_rate.quote.asset_id.get_instance())+"," \
    + fc::to_string(o.feed.core_exchange_rate.quote.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);       

 }
void store_db_plugin_impl::Prepare_bitlender_update_feed_producers_operation(const bitlender_update_feed_producers_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
 {

  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
  fc::to_variant( o.new_feed_option, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json3 = fc::json::to_string(vresult3);
 

    std::string sinsert =  " INSERT INTO `bitlender_update_feed_producers_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `author`,\
   `option_id`,\
   `new_feed_option`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.author.get_instance())+"," \
     + fc::to_string(o.option_id.get_instance())+",'" \
    + json3+"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";
 

 }
void store_db_plugin_impl::Prepare_bitlender_test_operation(const bitlender_test_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
 {

  fc::variant vresult1,vresult2;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
   

    std::string sinsert =  " INSERT INTO `bitlender_test_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `order_id`,\
   `ntype`,\
   `nvalue`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.order_id.instance())+"," \
    + fc::to_string(o.ntype)+"," \
    + fc::to_string(o.nvalue)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";
 
 }

 
void  store_db_plugin_impl::Prepare_author_create_operation(const author_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{ std::string sinsert =  " INSERT INTO `author_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void  store_db_plugin_impl::Prepare_author_update_operation(const author_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{ std::string sinsert =  " INSERT INTO `author_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void  store_db_plugin_impl::Prepare_asset_update_gateway_operation(const asset_update_gateway_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{ std::string sinsert =  " INSERT INTO `asset_update_gateway_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
 }
void  store_db_plugin_impl::Prepare_committee_member_update_zos_parameters_operation(const committee_member_update_zos_parameters_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{ std::string sinsert =  " INSERT INTO `committee_member_update_zos_parameters_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void  store_db_plugin_impl::Prepare_locktoken_create_operation(const locktoken_create_operation &o ,const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{ std::string sinsert =  " INSERT INTO `locktoken_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void  store_db_plugin_impl::Prepare_locktoken_update_operation(const locktoken_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{ std::string sinsert =  " INSERT INTO `locktoken_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void  store_db_plugin_impl::Prepare_locktoken_remove_operation(const locktoken_remove_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{ std::string sinsert =  " INSERT INTO `locktoken_remove_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);   
}
void  store_db_plugin_impl::Prepare_locktoken_node_operation(const locktoken_node_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{ std::string sinsert =  " INSERT INTO `asset_reserve_fees_olocktoken_node_operationperation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);   
}
void  store_db_plugin_impl::Prepare_locktoken_option_operation(const locktoken_option_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{ std::string sinsert =  " INSERT INTO `locktoken_option_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);   
}
 

void store_db_plugin_impl::Prepare_fill_object_history_operation(const fill_object_history_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
  std::string sinsert =  " INSERT INTO `fill_object_history_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_finance_option_create_operation(const  issue_fundraise_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ std::string sinsert =  " INSERT INTO `issue_fundraise_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_issue_fundraise_update_operation(const  issue_fundraise_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{ std::string sinsert =  " INSERT INTO `issue_fundraise_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_issue_fundraise_remove_operation(const  issue_fundraise_remove_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{ std::string sinsert =  " INSERT INTO `issue_fundraise_remove_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_fundraise_publish_feed_operation(const  issue_fundraise_publish_feed_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ std::string sinsert =  " INSERT INTO `issue_fundraise_publish_feed_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_finance_paramers_update_operation(const  finance_paramers_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{ std::string sinsert =  " INSERT INTO `finance_paramers_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_fundraise_create_operation(const buy_fundraise_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ std::string sinsert =  " INSERT INTO `buy_fundraise_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_buy_fundraise_enable_operation(const buy_fundraise_enable_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ std::string sinsert =  " INSERT INTO `buy_fundraise_enable_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_sell_exchange_create_operation(const sell_exchange_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ std::string sinsert =  " INSERT INTO `sell_exchange_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_sell_exchange_update_operation(const sell_exchange_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ std::string sinsert =  " INSERT INTO `sell_exchange_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_sell_exchange_remove_operation(const sell_exchange_remove_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ std::string sinsert =  " INSERT INTO `sell_exchange_remove_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_buy_exchange_create_operation(const buy_exchange_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ std::string sinsert =  " INSERT INTO `buy_exchange_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_buy_exchange_update_operation(const buy_exchange_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ std::string sinsert =  " INSERT INTO `buy_exchange_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_buy_exchange_remove_operation(const buy_exchange_remove_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ std::string sinsert =  " INSERT INTO `buy_exchange_remove_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + to_db_string(o)+"','" \
    + to_db_string(result)+"');";

   _store_db_callback(sinsert);  
}


void store_db_plugin_impl::Prepare_account_coupon_operation(const account_coupon_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
 fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2); 
   
    std::string sinsert =  " INSERT INTO `account_coupon_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  


}
void store_db_plugin_impl::Prepare_change_identity_operation(const change_identity_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{

  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2); 
   
    std::string sinsert =  " INSERT INTO `change_identity_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `object_id`,\
   `enable`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.object_id.instance())+"," \
    + fc::to_string(o.enable)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  


}
void store_db_plugin_impl::Prepare_withdraw_exchange_fee_operation(const withdraw_exchange_fee_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ 
  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2); 
   
    std::string sinsert =  " INSERT INTO `withdraw_exchange_fee_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
  `exchange_fees_asset_id`,\
   `exchange_fees_asset_amount`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.withdraw_asset_id.get_instance())+"," \
    + fc::to_string(o.core_amount.value)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  

}
void store_db_plugin_impl::Prepare_gateway_create_operation(const gateway_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{

  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2); 
   
    std::string sinsert =  " INSERT INTO `gateway_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `gateway_account`,\
   `url`,\
   `memo`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.gateway_account.get_instance())+",'" \
    + chang_db_string(o.url)  +"','" \
    + chang_db_string(o.memo) +"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  

}
void store_db_plugin_impl::Prepare_gateway_update_operation(const gateway_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
   fc::variant vresult1,vresult2;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
   

    std::string sinsert =  " INSERT INTO `gateway_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `gateway`,\
   `gateway_account`,\
   `new_url`,\
   `new_memo`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.gateway.get_instance())+"," \
    + fc::to_string(o.gateway_account.get_instance())+",'" \
    + (o.new_url.valid() ? *o.new_url : "") +"','" \
    + (o.new_memo.valid() ? *o.new_memo : "") +"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  
 
    
}
void store_db_plugin_impl::Prepare_carrier_create_operation(const carrier_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
 

fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2); 
   
    std::string sinsert =  " INSERT INTO `carrier_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `carrier_account`,\
   `url`,\
   `memo`,\
   `config`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.carrier_account.get_instance())+",'" \
    + chang_db_string(o.url)  +"','" \
    + chang_db_string(o.memo) +"','" \
    + to_optional_string(o.config) +"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  

}
void store_db_plugin_impl::Prepare_carrier_update_operation(const carrier_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
 
fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2); 
 
 

    std::string sinsert =  " INSERT INTO `carrier_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `carrier`,\
   `carrier_account`,\
   `new_url`,\
   `new_memo`,\
   `new_config`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.carrier.get_instance())+"," \
    + fc::to_string(o.carrier_account.get_instance())+",'" \
    + (o.new_url.valid() ? *o.new_url : "") +"','" \
    + (o.new_memo.valid() ? *o.new_memo : "") +"','" \
    +  to_optional_string(o.new_config) +"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  

}
void store_db_plugin_impl::Prepare_budget_member_create_operation(const budget_member_create_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number)
{
   fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2); 
   
    std::string sinsert =  " INSERT INTO `budget_member_create_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `budget_member_account`,\
   `url`,\
   `memo`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.budget_member_account.get_instance())+",'" \
    + chang_db_string(o.url)  +"','" \
    + chang_db_string(o.memo) +"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_budget_member_update_operation(const budget_member_update_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{ 
  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2); 
   std::string json3 = "";
  if(o.votes.valid())
  {
    fc::to_variant( *o.votes, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
    json3 = fc::json::to_string(vresult3);
  }

    std::string sinsert =  " INSERT INTO `budget_member_update_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `budget_member`,\
   `budget_member_account`,\
   `new_url`,\
   `new_memo`,\
   `votes`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.budget_member.get_instance())+"," \
    + fc::to_string(o.budget_member_account.get_instance())+",'" \
    + (o.new_url.valid() ? *o.new_url : "") +"','" \
    + (o.new_memo.valid() ? *o.new_memo : "") +"','" \
    + json3+"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  

}
void store_db_plugin_impl::Prepare_transfer_vesting_operation(const transfer_vesting_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
   fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2); 
   std::string json3 = "";
  if(o.memo.valid())
  {
    fc::to_variant( *o.memo, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
    json3 = fc::json::to_string(vresult3);
  }

    std::string sinsert =  " INSERT INTO `transfer_vesting_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `from`,\
   `to`,\
   `amount_id`,\
   `amount_amount`,\
   `vesting_cliff_seconds`,\
   `vesting_duration_seconds`,\
   `memo`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.from.get_instance())+"," \
    + fc::to_string(o.to.get_instance())+"," \
    + fc::to_string(o.amount.asset_id.get_instance())+"," \
    + fc::to_string(o.amount.amount.value)+"," \
    + fc::to_string(o.vesting_cliff_seconds)+"," \
    + fc::to_string(o.vesting_duration_seconds)+",'" \
    + json3+"'," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert);  
}
void store_db_plugin_impl::Prepare_revoke_vesting_operation(const revoke_vesting_operation &o, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number) 
{
   fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( o, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( result, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2); 

    std::string sinsert =  " INSERT INTO `revoke_vesting_operation` ( \
   `fee_asset_id`,\
   `fee_asset_amount`,\
   `issuer`,\
   `vesting_id`,\
   `block_num`,\
   `block_time`,\
   `trx_num`,\
   `op_num`,\
   `jason`, \
   `result` )\
   VALUES ( "\
    + fc::to_string(o.fee.asset_id.get_instance())+"," \
    + fc::to_string(o.fee.amount.value)+"," \
    + fc::to_string(o.issuer.get_instance())+"," \
    + fc::to_string(o.vesting_id.get_instance())+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"\
    + fc::to_string(num_trx)+"," \
    + fc::to_string(num_op)+",'" \
    + json1+"','" \
    + json2+"');";

   _store_db_callback(sinsert); 
} 

} // namespace detail

} // namespace store_db
} // namespace graphene


 