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

 

void store_db_plugin_impl::Prepare_account_object(const account_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{ 
  if(isNew )
  {    
    std::string sinsert = " INSERT INTO `account_object`  ( \
    `id`,\
    `membership_expiration_date`,\
    `register_date`,\
    `registrar`,\
    `referrer`,\
    `lifetime_referrer`,\
    `network_fee_percentage`,\
    `lifetime_referrer_fee_percentage`,\
    `referrer_rewards_percentage`,\
    `name`,\
    `statistics_id`,\
    `vesting_balance_id`,\
    `top_n_control_flags`,\
    `uaccount_property`,\
    `options`,\
    `auth_data`,\
    `state`,\
    `insert_num`,\
    `block_num`,\
    `block_time`,\
    `json`)\
    VALUES ( "\
    + fc::to_string(p.id.instance())+",'" \
    + p.membership_expiration_date.to_db_string()+"','"\
    + p.register_date.to_db_string()+"',"\
    + fc::to_string(p.registrar.get_instance())+"," \
    + fc::to_string(p.referrer.get_instance())+"," \
    + fc::to_string(p.lifetime_referrer.get_instance())+"," \
    + fc::to_string(p.network_fee_percentage)+"," \
    + fc::to_string(p.lifetime_referrer_fee_percentage)+"," \
    + fc::to_string(p.referrer_rewards_percentage)+",'" \
    + p.name+"'," \
    + fc::to_string(p.statistics.get_instance())+"," \
    + to_optional_id(p.cashback_vb)+"," \
    + fc::to_string(p.top_n_control_flags)+"," \
    + fc::to_string(p.uaccount_property)+",'" \
    + to_db_string(p.options)+"','" \
    + to_optional_string(p.auth_data)+"'," \
    + fc::to_string(0)+"," \
    + fc::to_string(block_number)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"','"
    + to_db_string(p)+"');"; 
    _store_db_callback(sinsert);
  }
  else 
  {
    std::string sinsert = "UPDATE `account_object`  SET  \
    `membership_expiration_date` = '"+p.membership_expiration_date.to_db_string()+ "'," \
   "`network_fee_percentage` = "+ fc::to_string(p.network_fee_percentage) + ","\
   "`lifetime_referrer_fee_percentage` = "+ fc::to_string(p.lifetime_referrer_fee_percentage)+ ","\
   "`referrer_rewards_percentage` = "+fc::to_string(p.referrer_rewards_percentage)+ ","\
   "`vesting_balance_id` = "+ to_optional_id(p.cashback_vb) + ","\
   "`top_n_control_flags` = "+ fc::to_string(p.top_n_control_flags) + ","\
   "`uaccount_property` = "+ fc::to_string(p.uaccount_property)+ ","\
   "`options` = '"+ to_db_string(p.options)+ "',"\
   "`auth_data` = '"+ to_optional_string(p.auth_data)+ "',"\
   "`state` = "+ fc::to_string(1)+ ","\
   "`block_num` = "+ fc::to_string(block_number) + ","\
   "`block_time` = '"+block_time.to_db_string() + "',"\
   "`json` = '"+ to_db_string(p) + "' "\
   "WHERE  `id` = "+fc::to_string(p.id.instance())+";";  
    _store_db_callback(sinsert);
  }
}
void store_db_plugin_impl::Prepare_asset_object(const asset_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
  fc::variant vresult;
  fc::to_variant( p, vresult, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json = fc::json::to_string(vresult);

  fc::variant vresult1;
  fc::to_variant( p.options.core_exchange_rate, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
 

  if(isNew)
  {  
  std::string sinsert = " INSERT INTO `asset_object` (   \
  `id`,\
  `symbol`,\
  `precision`,\
  `issuer`,\
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
  `dynamic_asset_data_id`,\
  `bitasset_data_id`,\
  `uasset_property`,\
  `buyback_account`,\
  `state`,\
  `insert_num`,\
  `block_num`,\
  `block_time`,\
  `json`)\
   VALUES ( "\
   + fc::to_string(p.id.instance())+",'" \
   + p.symbol+"'," \
   + fc::to_string(p.precision)+"," \
   + fc::to_string(p.issuer.get_instance())+"," \
   + fc::to_string(p.options.max_supply.value)+"," \
   + fc::to_string(p.options.market_fee_percent)+"," \
   + fc::to_string(p.options.max_market_fee.value)+"," \
   + fc::to_string(p.options.issuer_permissions)+"," \
   + fc::to_string(p.options.flags)+",'" \
   + json1+"','" \
   + to_db_string(p.options.whitelist_authorities)+"','" \
   + to_db_string(p.options.blacklist_authorities)+"','" \
   + to_db_string(p.options.whitelist_markets)+"','" \
   + to_db_string(p.options.blacklist_markets)+"','" \
   + p.options.description+"'," \
   + fc::to_string(p.dynamic_asset_data_id.get_instance())+"," \
   + to_optional_id(p.bitasset_data_id)+"," \
   + fc::to_string(p.uasset_property)+"," \
   + to_optional_id(p.buyback_account)+"," \
   + fc::to_string(0)+"," \
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + json+"');"; 
   _store_db_callback(sinsert);
  
  }
  else 
  {
   std::string sinsert = "  UPDATE `asset_object` SET  \
   `symbol` = '"+ p.symbol + "',"\
   "`precision` = "+ fc::to_string(p.precision) + ","\
   "`issuer` = "+ fc::to_string(p.issuer.get_instance()) + ","\
   "`max_supply` = "+ fc::to_string(p.options.max_supply.value) + ","\
   "`market_fee_percent` = "+ fc::to_string(p.options.market_fee_percent) + ","\
   "`max_market_fee` = "+ fc::to_string(p.options.max_market_fee.value) + ","\
   "`issuer_permissions` = "+ fc::to_string(p.options.issuer_permissions) + ","\
   "`flags` = "+ fc::to_string(p.options.flags) + ","\
   "`core_exchange_rate` = '"+ json1 + "',"\
   "`whitelist_authorities` = '"+ to_db_string(p.options.whitelist_authorities) + "',"\
   "`blacklist_authorities` = '"+ to_db_string(p.options.blacklist_authorities)+ "',"\
   "`whitelist_markets` = '"+ to_db_string(p.options.whitelist_markets) + "',"\
   "`blacklist_markets` = '"+ to_db_string(p.options.blacklist_markets)+ "',"\
   "`description` = '"+ p.options.description + "',"\
   "`dynamic_asset_data_id` = "+ fc::to_string(p.dynamic_asset_data_id.get_instance()) + ","\
   "`bitasset_data_id` = "+ fc::to_string(p.bitasset_data_id.valid() ? p.bitasset_data_id->get_instance():0) + ","\
   "`uasset_property` = "+ fc::to_string(p.uasset_property) + ","\
   "`buyback_account` = "+ fc::to_string(p.buyback_account.valid() ? p.buyback_account->get_instance():0) + ","\
   "`state` = "+ fc::to_string(1) + ","\
   "`block_time` = '"+ block_time.to_db_string()  + "',"\
   "`block_num` = "+fc::to_string(block_number) + ","\
   "`json` = '"+ json + "'  "\
   "WHERE  `id` = "+fc::to_string(p.id.instance())+";";  
    _store_db_callback(sinsert);
  }  
}
void store_db_plugin_impl::Prepare_bitlender_option_object(const bitlender_option_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
 
   


  if (isNew == 1)
  {
   std::string sinsert = " INSERT INTO `bitlender_option_object`  ( \
  `id`,\
  `asset_id`,\
  `sproduct`,\
  `issuer`,\
  `author`,\
  `options`,\
  `fee_mode`,\
  `feed_option`,\
  `gateways`,\
  `stop`,\
  `state`,\
  `insert_num`,\
  `block_num`,\
  `block_time`,\
  `json`)\
  VALUES (" \
  + fc::to_string(p.id.instance()) +","\
  + fc::to_string(p.asset_id.get_instance()) +",'"\
  + p.sproduct +"',"\
  + fc::to_string(p.issuer.get_instance()) +","\
  + fc::to_string(p.author.get_instance()) +",'"\
  +  to_db_string(p.options) +"',"\
  + fc::to_string(p.fee_mode) +",'"\
  +  to_db_string(p.feed_option) +"','"\
  +  to_db_string(p.gateways) +"',"\
  + fc::to_string(p.stop) +","\
  + fc::to_string(0) +","\
  + fc::to_string(block_number)+"," \
  + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   _store_db_callback(sinsert);  
  }
  else if (isNew == 0)
  {
    std::string sinsert = "   UPDATE `bitlender_option_object`  SET  \
    `options` = '"+ to_db_string(p.options) + "',"\
   "`author` = "+ fc::to_string(p.author.get_instance()) + ","\
   "`fee_mode` = "+ fc::to_string(p.fee_mode) + ","\
   "`feed_option` = '"+ to_db_string(p.feed_option) + "',"\
   "`gateways` = '"+ to_db_string(p.gateways) + "',"\
   "`stop` = "+ fc::to_string(p.stop) + ","\
   "`state` = "+ fc::to_string(1) + ","\
   "`block_time` = '"+ block_time.to_db_string()  + "',"\
   "`block_num` = "+fc::to_string(block_number) + ","\
   "`json` = '"+ to_db_string(p) + "'  "\
   "WHERE  `id` = "+fc::to_string(p.id.instance())+";";  
    _store_db_callback(sinsert);
  }
}
void store_db_plugin_impl::Prepare_issue_fundraise_object(const issue_fundraise_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
  if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `issue_fundraise_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `issue_fundraise_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `issue_fundraise_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_force_settlement_object(const force_settlement_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
   if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `force_settlement_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `force_settlement_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `force_settlement_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_committee_member_object(const committee_member_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{ 
 if(isNew)
  {  
  std::string sinsert = " INSERT INTO `committee_member_object` (   \
  `id`,\
  `committee_member_account`,\
  `vote_id`,\
  `total_votes`,\
  `url`,\
  `memo`,\
  `lock_asset_id`,\
  `lock_asset_amount`,\
  `enable`,\
  `state`,\
  `insert_num`,\
  `block_num`,\
  `block_time`,\
  `json`)\
  VALUES  ( "\
  + fc::to_string(p.id.instance())+"," \
  + fc::to_string(p.committee_member_account.get_instance())+"," \
  + fc::to_string(p.vote_id.instance())+"," \
  + fc::to_string(p.total_votes)+",'" \
  + chang_db_string(p.url)+"','" \
  + chang_db_string(p.memo)+"'," \
  + fc::to_string(p.lock_asset.asset_id.get_instance())+"," \
  + fc::to_string(p.lock_asset.amount.value)+"," \
  + fc::to_string(p.enable)+"," \
  + fc::to_string(0)+"," \
  + fc::to_string(block_number)+"," \
  + fc::to_string(block_number)+",'" \
  + block_time.to_db_string()+"','"
  + to_db_string(p)+"');"; 
   _store_db_callback(sinsert);  
  }
  else 
  {
   std::string sinsert = "  UPDATE `committee_member_object` SET  \
   `committee_member_account` = "+   fc::to_string(p.committee_member_account.get_instance())+ ","\
   "`vote_id` = "+  fc::to_string(p.vote_id.instance()) + ","\
   "`total_votes` = "+ fc::to_string(p.total_votes)  + ","\
   "`url` ='"+  chang_db_string(p.url) + "',"\
   "`memo` = '"+ chang_db_string(p.memo)  + "',"\
   "`lock_asset_id` = "+ fc::to_string(p.lock_asset.asset_id.get_instance())  + ","\
   "`lock_asset_amount` ="+ fc::to_string(p.lock_asset.amount.value)  + ","\
   "`enable` = "+   fc::to_string(p.enable)+ ","\
   "`state` = "+ fc::to_string(1) + ","\
   "`block_time` = '"+ block_time.to_db_string()  + "',"\
   "`block_num` = "+fc::to_string(block_number) + ","\
   "`json` = '"+ to_db_string(p) + "'  "\
   "WHERE  `id` = "+fc::to_string(p.id.instance())+";";  
    _store_db_callback(sinsert);  
  }
}
void store_db_plugin_impl::Prepare_budget_member_object(const budget_member_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
 if(isNew)
  {  
   std::string sinsert = " INSERT INTO `budget_member_object` (   \
   `id`,\
   `budget_member_account`,\
   `vote_id`,\
   `total_votes`,\
   `url`,\
   `memo`,\
   `votes`,\
   `lock_asset_id`,\
   `lock_asset_amount`,\
   `enable`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
    VALUES ( "\
    + fc::to_string(p.id.instance())+"," \
    + fc::to_string(p.budget_member_account.get_instance())+"," \
    + fc::to_string(p.vote_id.instance())+"," \
    + fc::to_string(p.total_votes)+",'" \
    + chang_db_string(p.url)+"','" \
    + chang_db_string(p.memo)+"','" \
    + to_db_string(p.votes)+"'," \
    + fc::to_string(p.lock_asset.asset_id.get_instance())+"," \
    + fc::to_string(p.lock_asset.amount.value)+"," \
    + fc::to_string(p.enable)+"," \
    + fc::to_string(0)+"," \
    + fc::to_string(block_number)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"','"
    + to_db_string(p)+"');"; 
    _store_db_callback(sinsert);  
   } 
 else 
  {
   std::string sinsert = "  UPDATE `budget_member_object` SET  \
   `budget_member_account` = "+ fc::to_string(p.budget_member_account.get_instance())+ ","\
  "`vote_id` = "+ fc::to_string(p.vote_id.instance()) + ","\
  "`total_votes` ="+fc::to_string(p.total_votes)+ ","\
  "`url` = '"+ chang_db_string(p.url) + "',"\
  "`memo` = '"+chang_db_string(p.memo)+ "',"\
  "`votes` = '"+ to_db_string(p.votes)+ "',"\
  "`lock_asset_id` = "+ fc::to_string(p.lock_asset.asset_id.get_instance()) + ","\
  "`lock_asset_amount` ="+ fc::to_string(p.lock_asset.amount.value) + ","\
  "`enable` ="+ fc::to_string(p.enable) + ","\
  "`state` = "+ fc::to_string(1) + ","\
  "`block_time` = '"+ block_time.to_db_string()  + "',"\
  "`block_num` = "+fc::to_string(block_number) + ","\
  "`json` = '"+ to_db_string(p) + "'  "\
  "WHERE  `id` = "+fc::to_string(p.id.instance())+";";  
    _store_db_callback(sinsert);  
  }
}
void store_db_plugin_impl::Prepare_witness_object(const witness_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{

if(isNew)
  {  
   std::string sinsert = " INSERT INTO `witness_object` (   \
   `id`,\
   `witness_account`,\
   `last_aslot`,\
   `pay_vb`,\
   `vote_id`,\
   `total_votes`,\
   `url`,\
   `memo`,\
   `total_missed`,\
   `recent_missed`,\
   `last_confirmed_block_num`,\
   `lock_asset_id`,\
   `lock_asset_amount`,\
   `enable`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
    VALUES ( "\
    + fc::to_string(p.id.instance())+"," \
    + fc::to_string(p.witness_account.get_instance())+"," \
    + fc::to_string(p.last_aslot)+"," \
    + to_optional_id(p.pay_vb)+"," \
    + fc::to_string(p.vote_id.instance())+"," \
    + fc::to_string(p.total_votes)+",'" \
    + chang_db_string(p.url)+"','" \
    + chang_db_string(p.memo)+"'," \
    + fc::to_string(p.total_missed)+"," \
    + fc::to_string(p.recent_missed)+"," \
    + fc::to_string(p.last_confirmed_block_num)+"," \
    + fc::to_string(p.lock_asset.asset_id.get_instance())+"," \
    + fc::to_string(p.lock_asset.amount.value)+"," \
    + fc::to_string(p.enable)+"," \
    + fc::to_string(0)+"," \
    + fc::to_string(block_number)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"','"
    + to_db_string(p)+"');"; 
    _store_db_callback(sinsert);  
   } 
 else 
  { 

   std::string sinsert = "  UPDATE `witness_object` SET  \
   `witness_account` = "+ fc::to_string(p.witness_account.get_instance())+ ","\
  "`last_aslot` = "+ fc::to_string(p.last_aslot)+ ","\
  "`pay_vb` = "+ to_optional_id(p.pay_vb)+ ","\
  "`vote_id` = "+ fc::to_string(p.vote_id.instance()) + ","\
  "`total_votes` ="+fc::to_string(p.total_votes)+ ","\
  "`url` = '"+ chang_db_string(p.url) + "',"\
  "`memo` = '"+chang_db_string(p.memo)+ "',"\
  "`total_missed` = "+ fc::to_string(p.total_missed)+ ","\
  "`recent_missed` = "+ fc::to_string(p.recent_missed)+ ","\
  "`last_confirmed_block_num` = "+ fc::to_string(p.last_confirmed_block_num)+ ","\
  "`lock_asset_id` = "+ fc::to_string(p.lock_asset.asset_id.get_instance()) + ","\
  "`lock_asset_amount` ="+ fc::to_string(p.lock_asset.amount.value) + ","\
  "`enable` ="+ fc::to_string(p.enable) + ","\
  "`state` = "+ fc::to_string(1) + ","\
  "`block_time` = '"+ block_time.to_db_string()  + "',"\
  "`block_num` = "+fc::to_string(block_number) + ","\
  "`json` = '"+ to_db_string(p) + "'  "\
  "WHERE  `id` = "+fc::to_string(p.id.instance())+";";  
    _store_db_callback(sinsert);  
  }
}
void store_db_plugin_impl::Prepare_gateway_dynamic_object(const gateway_dynamic_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{ 

if(isNew)
  {  
   std::string sinsert = " INSERT INTO `gateway_dynamic_object` (   \
   `id`,\
   `deposit_amount`,\
   `withdraw_amount`,\
   `deposit_count`,\
   `withdraw_count`,\
   `issue_amount`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
    VALUES ( "\
    + fc::to_string(p.id.instance())+",'"\
    + to_db_string(p.deposit_amount)+"','" \
    + to_db_string(p.withdraw_amount)+"','" \
    + to_db_string(p.deposit_count)+"','" \
    + to_db_string(p.withdraw_count)+"','" \
    + to_db_string(p.issue_amount)+"'," \
    + fc::to_string(0)+"," \
    + fc::to_string(block_number)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"','"
    + to_db_string(p)+"');"; 
    _store_db_callback(sinsert);  
   } 
 else 
  { 
   std::string sinsert = "  UPDATE `gateway_dynamic_object` SET  \
   `deposit_amount` = '"+ to_db_string(p.deposit_amount)+ "',"\
  "`withdraw_amount` = '"+ to_db_string(p.withdraw_amount)+ "',"\
  "`deposit_count` = '"+ to_db_string(p.deposit_count)+ "',"\
  "`withdraw_count` = '"+ to_db_string(p.withdraw_count)+ "',"\
  "`issue_amount` = '"+ to_db_string(p.issue_amount)+ "',"\
  "`state` = "+ fc::to_string(1) + ","\
  "`block_time` = '"+ block_time.to_db_string()  + "',"\
  "`block_num` = "+fc::to_string(block_number) + ","\
  "`json` = '"+ to_db_string(p) + "'  "\
  "WHERE  `id` = "+fc::to_string(p.id.instance())+";";  
    _store_db_callback(sinsert);  
  } 

}
void store_db_plugin_impl::Prepare_gateway_object(const gateway_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{  

if(isNew)
  {  
   std::string sinsert = " INSERT INTO `gateway_object` (   \
   `id`,\
   `gateway_account`,\
   `dynamic_id`,\
   `pay_vb`,\
   `vote_id`,\
   `total_votes`,\
   `url`,\
   `memo`,\
   `lock_asset_id`,\
   `lock_asset_amount`,\
   `enable`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
    VALUES ( "\
    + fc::to_string(p.id.instance())+"," \
    + fc::to_string(p.gateway_account.get_instance())+"," \
    + fc::to_string(p.dynamic_id.get_instance())+"," \
    + to_optional_id(p.pay_vb)+"," \
    + fc::to_string(p.vote_id.instance())+"," \
    + fc::to_string(p.total_votes)+",'" \
    + chang_db_string(p.url)+"','" \
    + chang_db_string(p.memo)+"'," \
    + fc::to_string(p.lock_asset.asset_id.get_instance())+"," \
    + fc::to_string(p.lock_asset.amount.value)+"," \
    + fc::to_string(p.enable)+"," \
    + fc::to_string(0)+"," \
    + fc::to_string(block_number)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"','"\
    + to_db_string(p)+"');"; 
    _store_db_callback(sinsert);  
   } 
 else 
  { 

   std::string sinsert = "  UPDATE `gateway_object` SET  \
   `gateway_account` = "+ fc::to_string(p.gateway_account.get_instance())+ ","\
  "`dynamic_id` = "+ fc::to_string(p.dynamic_id.get_instance())+ ","\
  "`pay_vb` = "+ to_optional_id(p.pay_vb)+ ","\
  "`vote_id` = "+ fc::to_string(p.vote_id.instance()) + ","\
  "`total_votes` ="+fc::to_string(p.total_votes)+ ","\
  "`url` = '"+ chang_db_string(p.url) + "',"\
  "`memo` = '"+chang_db_string(p.memo)+ "',"\
  "`lock_asset_id` = "+ fc::to_string(p.lock_asset.asset_id.get_instance()) + ","\
  "`lock_asset_amount` ="+ fc::to_string(p.lock_asset.amount.value) + ","\
  "`enable` ="+ fc::to_string(p.enable) + ","\
  "`state` = "+ fc::to_string(1) + ","\
  "`block_time` = '"+ block_time.to_db_string()  + "',"\
  "`block_num` = "+fc::to_string(block_number) + ","\
  "`json` = '"+ to_db_string(p) + "'  "\
  "WHERE  `id` = "+fc::to_string(p.id.instance())+";";  
    _store_db_callback(sinsert);  
  } 

}
void store_db_plugin_impl::Prepare_carrier_dynamic_object(const carrier_dynamic_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{ 
  
if(isNew)
  {  
   std::string sinsert = " INSERT INTO `carrier_dynamic_object` (   \
   `id`,\
   `loan`,\
   `invest`,\
   `collateralize_risk`,\
   `fee`,\
   `recycle_collateralize`,\
   `recycle_loan`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
    VALUES ( "\
    + fc::to_string(p.id.instance())+",'" \
    + to_db_string(p.loan)+"','" \
    + to_db_string(p.invest)+"','" \
    + to_db_string(p.collateralize_risk)+"','" \
    + to_db_string(p.fee)+"','" \
    + to_db_string(p.recycle_collateralize)+"','" \
    + to_db_string(p.recycle_loan)+"'," \
    + fc::to_string(0)+"," \
    + fc::to_string(block_number)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"','"\
    + to_db_string(p)+"');"; 
    _store_db_callback(sinsert);  
   } 
 else 
  { 

   std::string sinsert = "  UPDATE `carrier_dynamic_object` SET  \
   `loan` = '"+ to_db_string(p.loan)+ "',"\
  "`invest` = '"+ to_db_string(p.invest)+ "',"\
  "`collateralize_risk` = '"+ to_db_string(p.collateralize_risk) + "',"\
  "`fee` = '"+to_db_string(p.fee)+ "',"\
  "`recycle_collateralize` = '"+ to_db_string(p.recycle_collateralize)+ "',"\
  "`recycle_loan` = '"+to_db_string(p.recycle_loan)+ "',"\
  "`state` = "+ fc::to_string(1) + ","\
  "`block_time` = '"+ block_time.to_db_string()  + "',"\
  "`block_num` = "+fc::to_string(block_number) + ","\
  "`json` = '"+ to_db_string(p) + "'  "\
  "WHERE  `id` = "+fc::to_string(p.id.instance())+";";  
    _store_db_callback(sinsert);  
  }
}

void store_db_plugin_impl::Prepare_carrier_object(const carrier_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{ 

if(isNew)
  {  
   std::string sinsert = " INSERT INTO `carrier_object` (   \
   `id`,\
   `carrier_account`,\
   `pay_vb`,\
   `vote_id`,\
   `total_votes`,\
   `url`,\
   `memo`,\
   `config`,\
   `lock_asset_id`,\
   `lock_asset_amount`,\
   `enable`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
    VALUES ( "\
    + fc::to_string(p.id.instance())+"," \
    + fc::to_string(p.carrier_account.get_instance())+"," \
    + to_optional_id(p.pay_vb)+"," \
    + fc::to_string(p.vote_id.instance())+"," \
    + fc::to_string(p.total_votes)+",'" \
    + chang_db_string(p.url)+"','" \
    + chang_db_string(p.memo)+"','" \
    + chang_db_string(p.config)+"'," \
    + fc::to_string(p.lock_asset.asset_id.get_instance())+"," \
    + fc::to_string(p.lock_asset.amount.value)+"," \
    + fc::to_string(p.enable)+"," \
    + fc::to_string(0)+"," \
    + fc::to_string(block_number)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"','"
    + to_db_string(p)+"');"; 
    _store_db_callback(sinsert);  
   } 
 else 
  { 

   std::string sinsert = "  UPDATE `carrier_object` SET  \
   `carrier_account` = "+ fc::to_string(p.carrier_account.get_instance())+ ","\
  "`pay_vb` = "+ to_optional_id(p.pay_vb)+ ","\
  "`vote_id` = "+ fc::to_string(p.vote_id.instance()) + ","\
  "`total_votes` ="+fc::to_string(p.total_votes)+ ","\
  "`url` = '"+ chang_db_string(p.url)+ "',"\
  "`memo` = '"+chang_db_string(p.memo)+ "',"\
  "`config` = '"+chang_db_string(p.config)+ "',"\
  "`lock_asset_id` = "+ fc::to_string(p.lock_asset.asset_id.get_instance()) + ","\
  "`lock_asset_amount` ="+ fc::to_string(p.lock_asset.amount.value) + ","\
  "`enable` ="+ fc::to_string(p.enable) + ","\
  "`state` = "+ fc::to_string(1) + ","\
  "`block_time` = '"+ block_time.to_db_string()  + "',"\
  "`block_num` = "+fc::to_string(block_number) + ","\
  "`json` = '"+ to_db_string(p) + "'  "\
  "WHERE  `id` = "+fc::to_string(p.id.instance())+";";  
    _store_db_callback(sinsert);  
  }

}
void store_db_plugin_impl::Prepare_worker_object(const worker_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{//2


if(isNew ==1)
  {  
   std::string sinsert = " INSERT INTO `worker_object` (   \
   `id`,\
   `worker_account`,\
   `work_begin_date`,\
   `work_end_date`,\
   `daily_pay`,\
   `worker`,\
   `name`,\
   `url`,\
   `memo`,\
   `vote_for`,\
   `vote_against`,\
   `total_votes_for`,\
   `total_votes_against`,\
   `need_pay`,\
   `total_pay`,\
   `last_pay`,\
   `last_budget_time`,\
   `is_approved`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
    VALUES ( "\
    + fc::to_string(p.id.instance())+"," \
    + fc::to_string(p.worker_account.get_instance())+",'" \
     + p.work_begin_date.to_db_string()+"','"
    + p.work_end_date.to_db_string()+"',"
    + fc::to_string(p.daily_pay.value)+"," \
    + fc::to_string(p.worker.which())+",'" \
    + chang_db_string(p.name)+"','" \
    + chang_db_string(p.url)+"','" \
    + chang_db_string(p.memo)+"'," \
    + fc::to_string(p.vote_for)+"," \
    + fc::to_string(p.vote_against)+"," \
    + fc::to_string(p.total_votes_for)+"," \
    + fc::to_string(p.total_votes_against)+"," \
    + fc::to_string(p.need_pay.value)+"," \
    + fc::to_string(p.total_pay.value)+"," \
    + fc::to_string(p.last_pay.value)+",'" \
    + p.last_budget_time.to_db_string()+"'," \
    + fc::to_string(p.is_approved)+"," \
    + fc::to_string(0)+"," \
    + fc::to_string(block_number)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"','"
    + to_db_string(p)+"');"; 
    _store_db_callback(sinsert);  
   } 
 else   if (isNew ==0)
  { 

   std::string sinsert = "  UPDATE `worker_object` SET  \
   `worker_account` = "+ fc::to_string(p.worker_account.get_instance())+ ","\
  "`work_begin_date` = '"+ p.work_begin_date.to_db_string()+ "',"\
  "`work_end_date` = '"+ p.work_end_date.to_db_string() + "',"\
  "`daily_pay` ="+fc::to_string(p.daily_pay.value)+ ","\
  "`worker` ="+fc::to_string(p.worker.which())+ ","\
  "`name` ='"+chang_db_string(p.name)+ "',"\
  "`url` = '"+ chang_db_string(p.url) + "',"\
  "`memo` = '"+chang_db_string(p.memo)+ "',"\
  "`vote_for` = "+ fc::to_string(p.vote_for) + ","\
  "`vote_against` ="+ fc::to_string(p.vote_for) + ","\
  "`total_votes_for` ="+ fc::to_string(p.total_votes_for) + ","\
  "`total_votes_against` = "+ fc::to_string(p.total_votes_against) + ","\
  "`need_pay` ="+ fc::to_string(p.need_pay.value) + ","\
  "`total_pay` ="+ fc::to_string(p.total_pay.value) + ","\
  "`last_pay` = "+ fc::to_string(p.last_pay.value) + ","\
  "`last_budget_time` ='"+ p.last_budget_time.to_db_string() + "',"\
  "`is_approved` ="+ fc::to_string(p.is_approved) + ","\
  "`state` = "+ fc::to_string(1) + ","\
  "`block_time` = '"+ block_time.to_db_string()  + "',"\
  "`block_num` = "+fc::to_string(block_number) + ","\
  "`json` = '"+ to_db_string(p) + "'  "\
  "WHERE  `id` = "+fc::to_string(p.id.instance())+";";  
    _store_db_callback(sinsert);  
  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `worker_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);

}
void store_db_plugin_impl::Prepare_limit_order_object(const limit_order_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{    if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `limit_order_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `limit_order_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `limit_order_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_call_order_object(const call_order_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
    if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `call_order_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `call_order_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `call_order_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_bitlender_order_object(const bitlender_order_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{//2
fc::variant vresult;
  fc::to_variant( p, vresult, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json = fc::json::to_string(vresult);
 
 fc::variant vresult0;
  fc::to_variant( p.interest_rate, vresult0, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json0 = fc::json::to_string(vresult0);

 fc::variant vresult1;
  fc::to_variant( p.interest_book, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);

  fc::variant vresult2;
  fc::to_variant(p.interest_book, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS);
  std::string json2 = fc::json::to_string(vresult2);

  fc::variant vresult3;
  fc::to_variant(p.distribution_fees, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS);
  std::string json3 = fc::json::to_string(vresult3);

  fc::variant vresult4;
  fc::to_variant(p.price_settlement, vresult4, GRAPHENE_NET_MAX_NESTED_OBJECTS);
  std::string json4 = fc::json::to_string(vresult4);



 if (isNew ==1)
  {
   std::string sinsert = " INSERT INTO `bitlender_order_object` (  \
   `id`,\
`issuer`,\
`carrier`,\
`carrier_id`,\
`referrer`,\
`option_id`,\
`feed_id`,\
`order`,\
`amount_to_loan_id`,\
`amount_to_loan_amount`,\
`interest_rate`,\
`amount_to_collateralize_id`,\
`amount_to_collateralize_amount`,\
`collateralize_fee_id`,\
`collateralize_fee_amount`,\
`collateralize_risk_id`,\
`collateralize_risk_amount`,\
`carrier_fee_id`,\
`carrier_fee_amount`,\
`collateral_rate`,\
`bid_period`,\
`fee_mode`,\
`loan_time`,\
`expiration_time`,\
`invest_finish_time`,\
`repay_principal_time`,\
`expect_principal_time`,\
`time_state`,\
`notify_time`,\
`memo`,\
`lock_collateralize_id`,\
`lock_collateralize_amount`,\
`repay_principal_fee_id`,\
`repay_principal_fee_amount`,\
`interest_book`,\
`distribution_fees`,\
`order_state`,\
`order_flag`,\
`price_settlement`,\
`overdue_expiration_time`,\
`info`,\
`recycle_pre_state`,\
`history_pre_state`,\
`insert_num`,\
`block_num`,\
`block_time`,\
`state`,\
`json`)\
 VALUES  ("\
 + fc::to_string(p.id.instance()) +","\
 + fc::to_string(p.issuer.get_instance()) +","\
 + fc::to_string(p.carrier.get_instance()) +","\
 + fc::to_string(p.carrier_id.get_instance()) +","\
 + fc::to_string(p.referrer.get_instance()) +","\
 + fc::to_string(p.option_id.get_instance()) +","\
 + fc::to_string(p.feed_id.get_instance()) +",'"\
 + chang_db_string(p.order) +"',"\
 + fc::to_string(p.amount_to_loan.asset_id.get_instance()) +","\
 + fc::to_string(p.amount_to_loan.amount.value) +",'"\
 + json0+"',"\
 + fc::to_string(p.amount_to_collateralize.asset_id.get_instance()) +","\
 + fc::to_string(p.amount_to_collateralize.amount.value) +","\
 + fc::to_string(p.collateralize_fee.asset_id.get_instance()) +","\
 + fc::to_string(p.collateralize_fee.amount.value) +","\
 + fc::to_string(p.collateralize_risk.asset_id.get_instance()) +","\
 + fc::to_string(p.collateralize_risk.amount.value) +","\
 + fc::to_string(p.carrier_fee.asset_id.get_instance()) +","\
 + fc::to_string(p.carrier_fee.amount.value) +","\
 + fc::to_string(p.collateral_rate) +","\
 + fc::to_string(p.bid_period) +","\
 + fc::to_string(p.fee_mode) +",'"\
 + p.loan_time.to_db_string()+"','"
 + p.expiration_time.to_db_string()+"','"
 + p.invest_finish_time.to_db_string()+"','"
 + p.repay_principal_time.to_db_string()+"','"
 + p.expect_principal_time.to_db_string()+"',"
 + fc::to_string(p.time_state) +",'"\
 + p.notify_time.to_db_string()+"','"
 + p.memo +"',"\
 + fc::to_string(p.lock_collateralize.asset_id.get_instance()) +","\
 + fc::to_string(p.lock_collateralize.amount.value) +","\
 + fc::to_string(p.repay_principal_fee.asset_id.get_instance()) +","\
 + fc::to_string(p.repay_principal_fee.amount.value) +",'"\
 + json2 +"','"\
 + json3+"',"\
 + fc::to_string(p.order_state) +","\
 + fc::to_string(p.order_flag) +",'"\
 + json4+"',"\
 + fc::to_string(p.overdue_expiration_time)+",'"
 + p.info +"',"\
 + fc::to_string(p.recycle_pre_state) +","\
 + fc::to_string(p.history_pre_state) +","\
 + fc::to_string(block_number)+"," \
 + fc::to_string(block_number)+",'" \
 + block_time.to_db_string()+"',"
  + fc::to_string(0)+",'" \
 + json+"');"; 
   _store_db_callback(sinsert);  
  }
  else  if (isNew ==0)
  {
    std::string sinsert = " UPDATE `bitlender_order_object` SET   \
    `carrier` = "+ fc::to_string(p.carrier.get_instance()) + "," \
   "`carrier_id` = "+ fc::to_string(p.carrier_id.get_instance()) + "," \
   "`referrer` = "+ fc::to_string(p.referrer.get_instance()) + "," \
   "`option_id` = "+ fc::to_string(p.option_id.get_instance()) + "," \
   "`feed_id` = "+ fc::to_string(p.feed_id.get_instance()) + "," \
   "`order` = '"+  chang_db_string(p.order)+ "'," \
   "`amount_to_loan_id` = "+ fc::to_string(p.amount_to_loan.asset_id.get_instance()) + "," \
   "`amount_to_loan_amount` = "+ fc::to_string(p.amount_to_loan.amount.value)+ "," \
   "`interest_rate` = '"+ json0 + "'," \
   "`amount_to_collateralize_id` = "+ fc::to_string(p.amount_to_collateralize.asset_id.get_instance())+ "," \
   "`amount_to_collateralize_amount` = "+ fc::to_string(p.amount_to_collateralize.amount.value)+ "," \
   "`collateralize_fee_id` = "+ fc::to_string(p.collateralize_fee.asset_id.get_instance())+ "," \
   "`collateralize_fee_amount` = "+ fc::to_string(p.collateralize_fee.amount.value)+ "," \
   "`collateralize_risk_id` = "+ fc::to_string(p.collateralize_risk.asset_id.get_instance())+ "," \
   "`collateralize_risk_amount` = "+fc::to_string(p.collateralize_risk.amount.value)  + "," \
   "`carrier_fee_id` = "+ fc::to_string(p.carrier_fee.asset_id.get_instance())+ "," \
   "`carrier_fee_amount` = "+ fc::to_string(p.carrier_fee.amount.value) + "," \
   "`collateral_rate` = "+fc::to_string(p.collateral_rate) + "," \
   "`bid_period` = "+ fc::to_string(p.bid_period) + "," \
   "`fee_mode` = "+ fc::to_string(p.fee_mode) + "," \
   "`loan_time` = '"+ p.loan_time.to_db_string() + "'," \
   "`expiration_time` = '"+p.expiration_time.to_db_string() + "'," \
   "`invest_finish_time` = '"+ p.invest_finish_time.to_db_string() + "'," \
   "`repay_principal_time` = '"+  p.repay_principal_time.to_db_string()+ "'," \
   "`expect_principal_time` = '"+ p.expect_principal_time.to_db_string()+ "'," \
   "`time_state` = "+ fc::to_string(p.time_state) + "," \
   "`notify_time` = '"+ p.notify_time.to_db_string()+ "'," \
   "`memo` = '"+ p.memo  + "'," \
   "`lock_collateralize_id` = "+ fc::to_string(p.lock_collateralize.asset_id.get_instance()) + "," \
   "`lock_collateralize_amount` ="+ fc::to_string(p.lock_collateralize.amount.value)+ "," \
   "`repay_principal_fee_id` ="+ fc::to_string(p.repay_principal_fee.asset_id.get_instance())+ "," \
   "`repay_principal_fee_amount` = "+ fc::to_string(p.repay_principal_fee.amount.value) + "," \
   "`interest_book` = '"+json2+ "'," \
   "`distribution_fees` = '"+ json3+ "'," \
   "`order_state` = "+ fc::to_string(p.order_state)+ "," \
   "`order_flag` ="+ fc::to_string(p.order_flag)+ "," \
   "`price_settlement` = '"+ json4+ "'," \
   "`overdue_expiration_time` = "+ fc::to_string(p.overdue_expiration_time)+ "," \
   "`info` = '"+ p.info+ "'," \
   "`recycle_pre_state` = "+ fc::to_string(p.recycle_pre_state)+ "," \
   "`history_pre_state` ="+ fc::to_string(p.history_pre_state)+ "," \
   "`block_time` = '"+ block_time.to_db_string()  + "',"\
   "`block_num` = "+fc::to_string(block_number) + ","\
   "`state` = "+fc::to_string(1) + ","\
   "`json` = '"+ json + "'  "\
   "WHERE  `id` = "+fc::to_string(p.id.instance())+";";  
    _store_db_callback(sinsert);   
  }  else  if(isNew ==2)
  {
      std::string sinsert = " UPDATE  `bitlender_order_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}

void store_db_plugin_impl::Prepare_bitlender_invest_object(const bitlender_invest_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{//2
  
  fc::variant vresult;
  fc::to_variant( p, vresult, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json = fc::json::to_string(vresult);

  fc::variant vresult1;
  fc::to_variant( p.interest_rate, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);

  fc::variant vresult2;
  fc::to_variant( p.repay_interest, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);

  


  if(isNew ==1)
  {
   std::string sinsert = "  INSERT INTO `bitlender_invest_object` (   \
   `id`,\
   `issuer`,\
   `carrier`,\
   `carrier_id`,\
   `referrer`,\
   `order_id`,\
   `option_id`,\
   `amount_to_loan_id`,\
   `amount_to_loan_amount`,\
   `amount_to_invest_id`,\
   `amount_to_invest_amount`,\
   `amount_to_collateralize_id`,\
   `amount_to_collateralize_amount`,\
   `repay_principal_fee_id`,\
   `repay_principal_fee_amount`,\
   `interest_rate`,\
   `invest_time`,\
   `repay_principal_time`,\
   `repay_interest`,\
   `lock_invest_id`,\
   `lock_invest_amount`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `state`,\
   `json`)\
   VALUES   (  "\
   + fc::to_string(p.id.instance()) +","\
   + fc::to_string(p.issuer.get_instance()) +","\
   + fc::to_string(p.carrier.get_instance()) +","\
   + fc::to_string(p.carrier_id.get_instance()) +","\
   + fc::to_string(p.referrer.get_instance()) +","\
   + fc::to_string(p.order_id.get_instance()) +","\
   + fc::to_string(p.option_id.get_instance()) +","\
   + fc::to_string(p.amount_to_loan.asset_id.get_instance()) +","\
   + fc::to_string(p.amount_to_loan.amount.value) +","\
   + fc::to_string(p.amount_to_invest.asset_id.get_instance()) +","\
   + fc::to_string(p.amount_to_invest.amount.value) +","\
   + fc::to_string(p.amount_to_collateralize.asset_id.get_instance()) +","\
   + fc::to_string(p.amount_to_collateralize.amount.value) +","\
   + fc::to_string(p.repay_principal_fee.asset_id.get_instance()) +","\
   + fc::to_string(p.repay_principal_fee.amount.value) +",'"\
   + json1 +"','"\
   + p.invest_time.to_db_string()  +"','"\
   + p.repay_principal_time.to_db_string()  +"','"\
   + json2 +"',"\
   + fc::to_string(p.lock_invest.asset_id.get_instance()) +","\
   + fc::to_string(p.lock_invest.amount.value) +","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"',"
   + fc::to_string(0)+",'" \
   + json+"');"; 
    _store_db_callback(sinsert);  
  }
  else  if (isNew ==0)
  {
    std::string sinsert = " UPDATE `bitlender_invest_object` SET   \
    `amount_to_loan_id` ="+ fc::to_string(p.amount_to_loan.asset_id.get_instance())  + "," \
   "`amount_to_loan_amount` ="+fc::to_string(p.amount_to_loan.amount.value)   + "," \
   "`amount_to_invest_id` = "+  fc::to_string(p.amount_to_invest.asset_id.get_instance()) + "," \
   "`amount_to_invest_amount` ="+  fc::to_string(p.amount_to_invest.amount.value) + "," \
   "`amount_to_collateralize_id` = "+   fc::to_string(p.amount_to_collateralize.asset_id.get_instance()) + "," \
   "`amount_to_collateralize_amount` = "+fc::to_string(p.amount_to_collateralize.amount.value)   + "," \
   "`repay_principal_fee_id` = "+   fc::to_string(p.repay_principal_fee.asset_id.get_instance())  + "," \
   "`repay_principal_fee_amount` = "+ fc::to_string(p.repay_principal_fee.amount.value)  + "," \
   "`interest_rate` = '"+  json1 + "'," \
   "`invest_time` = '"+   p.invest_time.to_db_string() + "'," \
   "`repay_principal_time` = '"+ p.repay_principal_time.to_db_string()  + "'," \
   "`repay_interest` ='"+  json2 + "'," \
   "`lock_invest_id` = "+  fc::to_string(p.lock_invest.asset_id.get_instance()) + "," \
   "`lock_invest_amount` = "+ fc::to_string(p.lock_invest.amount.value)  + "," \
   "`block_time` = '"+ block_time.to_db_string()  + "',"\
   "`block_num` = "+fc::to_string(block_number) + ","\
   "`state` = "+fc::to_string(1) + ","\
   "`json` = '"+ json + "'  "\
   "WHERE  `id` = "+fc::to_string(p.id.instance())+";";  
    _store_db_callback(sinsert);
  } else  if(isNew ==2)
  {
      std::string sinsert = " UPDATE  `bitlender_invest_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_author_object(const author_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{     if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `author_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `author_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `author_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_locktoken_object(const locktoken_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{     if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `locktoken_object` (  \
   `id`,\
   `option_id`,\
   `issuer`,\
   `locked_amount`,\
   `locked_asset`,\
   `locked_precison`,\
   `to`,\
   `period`,\
   `type`,\
   `autolock`,\
   `init_time`,\
   `create_time`,\
   `rate`,\
   `remove_time`,\
   `except_time`,\
   `pay_asset`,\
   `payer`,\
   `carrier`,\
   `pay_price`,\
   `max_rate`,\
   `vesting_seconds`,\
   `buy_asset_amount`,\
   `buy_asset_asset`,\
   `dy_interest_amount`,\
   `dy_interest_asset`,\
   `dy_list`,\
   `interest_amount`,\
   `interest_asset`,\
   `interest_precision`,\
   `interest_list`,\
   `reward_list`,\
   `ustate`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(p.option_id.get_instance())+","\
   +fc::to_string(p.issuer.get_instance())+","\
   + fc::to_string(p.locked.amount.value)+"," \
   + fc::to_string(p.locked.asset_id.get_instance())+"," \
   + fc::to_string(p.locked.precision)+"," \
   +fc::to_string(p.to.get_instance())+","\
   +fc::to_string(p.period)+"," \
   +fc::to_string(p.type)+"," \
   +fc::to_string(p.autolock)+",'" \
   +p.init_time.to_db_string()+"','" \
   +p.create_time.to_db_string()+"'," \
   +fc::to_string(p.rate)+",'" \
   +p.remove_time.to_db_string()+"','"\
   +p.except_time.to_db_string()+"'," \
   +fc::to_string(p.pay_asset.get_instance())+","\
   +fc::to_string(p.payer.get_instance())+","\
   +fc::to_string(p.carrier.get_instance())+",'"\
   + to_db_string(p.pay_price)+"',"\
   +fc::to_string(p.max_rate.value)+"," \
   +fc::to_string(p.vesting_seconds)+"," \
   +fc::to_string(p.buy_asset.amount.value)+"," \
   +fc::to_string(p.buy_asset.asset_id.get_instance())+"," \
   +fc::to_string(p.dy_interest.amount.value)+"," \
   +fc::to_string(p.dy_interest.asset_id.get_instance())+",'" \
   +to_db_string(p.dy_list)+"'," \
   +fc::to_string(p.interest.amount.value)+"," \
   +fc::to_string(p.interest.asset_id.get_instance())+"," \
   +fc::to_string(p.interest.precision)+",'" \
   +to_db_string(p.interest_list)+"','" \
   +to_db_string(p.reward_list)+"'," \
   +fc::to_string(p.ustate)+"," \
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
 


  }
  else  if(isNew == 0)
  { 
    std::string sinsert = " UPDATE `locktoken_object` SET   \
   `locked_amount` = "+ fc::to_string(p.locked.amount.value)+ "," \
   "`locked_asset` = "+ fc::to_string(p.locked.asset_id.get_instance())+ "," \
   "`locked_precison` = "+ fc::to_string(p.locked.precision)+ "," \
   "`to` = "+ fc::to_string(p.to.get_instance())+ "," \
   "`period` = "+ fc::to_string(p.period)+ "," \
   "`type` = "+ fc::to_string(p.type)+ "," \
   "`autolock` = "+ fc::to_string(p.autolock)+ "," \
   "`init_time` = '"+ p.init_time.to_db_string() + "'," \
   "`create_time` = '"+ p.create_time.to_db_string()+ "'," \
   "`rate` = "+ fc::to_string(p.rate)+ "," \
   "`remove_time` = '"+ p.remove_time.to_db_string()+ "'," \
   "`except_time` = '"+ p.except_time.to_db_string()+ "'," \
   "`pay_asset` = "+ fc::to_string(p.pay_asset.get_instance())+ "," \
   "`payer` = "+ fc::to_string(p.payer.get_instance())+ "," \
   "`carrier` = "+ fc::to_string(p.carrier.get_instance())+ "," \
   "`pay_price` = '"+ to_db_string(p.pay_price) + "'," \
   "`max_rate` = "+ fc::to_string(p.max_rate.value) + "," \
   "`vesting_seconds` = "+ fc::to_string(p.vesting_seconds) + "," \
   "`buy_asset_amount` = "+ fc::to_string(p.buy_asset.amount.value)+ "," \
   "`buy_asset_asset` = "+ fc::to_string(p.buy_asset.asset_id.get_instance()) + "," \
   "`dy_interest_amount` = "+ fc::to_string(p.dy_interest.amount.value) + "," \
   "`dy_interest_asset` = "+ fc::to_string(p.dy_interest.asset_id.get_instance()) + "," \
    "`dy_list` = '"+ to_db_string(p.dy_list) + "'," \
   "`interest_amount` = "+ fc::to_string(p.interest.amount.value) + "," \
   "`interest_asset` = "+ fc::to_string(p.interest.asset_id.get_instance()) + "," \
   "`interest_precision` = "+ fc::to_string(p.interest.precision)+ "," \
   "`interest_list` = '"+ to_db_string(p.interest_list)+ "'," \
   "`reward_list` = '"+ to_db_string(p.reward_list) + "'," \
   "`ustate` = "+ fc::to_string(p.ustate) + "," \
   "`state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `locktoken_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_locktoken_options_object(const locktoken_option_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{     if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `locktoken_option_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `locktoken_option_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `locktoken_option_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_buy_fundraise_object(const buy_fundraise_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{     if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `buy_fundraise_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `buy_fundraise_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `buy_fundraise_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_buy_exchange_object(const buy_exchange_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{    if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `buy_exchange_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `buy_exchange_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `buy_exchange_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_sell_exchange_object(const sell_exchange_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{    if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `sell_exchange_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `sell_exchange_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `sell_exchange_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_proposal_object(const proposal_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{ //2

  fc::variant vresult;
  fc::to_variant( p, vresult, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json = fc::json::to_string(vresult);

  fc::variant vresult1;
  fc::to_variant( p.proposed_transaction, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);

  
  fc::variant vresult_0,vresult_1,vresult_2,vresult_3,vresult_4,vresult_5;
  fc::to_variant( p.required_active_approvals, vresult_0, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json_0 = fc::json::to_string(vresult_0);
  fc::to_variant( p.available_active_approvals, vresult_1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json_1 = fc::json::to_string(vresult_1);
  fc::to_variant( p.required_owner_approvals, vresult_2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json_2 = fc::json::to_string(vresult_2);
  fc::to_variant( p.available_owner_approvals, vresult_3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json_3 = fc::json::to_string(vresult_3);
  fc::to_variant( p.available_key_approvals, vresult_4, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json_4 = fc::json::to_string(vresult_4);
 
  if(isNew ==1)
  {
   std::string sinsert = "  INSERT INTO `proposal_object` (   \
   `id`,\
   `expiration_time`,\
   `review_period_time`,\
   `proposed_transaction`,\
   `memo`,\
   `required_active_approvals`,\
   `available_active_approvals`,\
   `required_owner_approvals`,\
   `available_owner_approvals`,\
   `available_key_approvals`,\
   `proposer`,\
   `errorinfo`,\
   `status`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `state`,\
   `json`)\
    VALUES   (  "\
   + fc::to_string(p.id.instance()) +",'"\
   + p.expiration_time.to_db_string() +"','"\
   + (p.review_period_time.valid() ? p.expiration_time.to_db_string() : "1970-01-01 00:00:00") +"','"\
   + json1 +"','"\
   + p.memo +"','"\
   + json_0+"','" \
   + json_1+"','" \
   + json_2+"','" \
   + json_3+"','" \
   + json_4+"'," \
   + fc::to_string(p.proposer.get_instance()) +",'"\
   + chang_db_string(p.errorinfo)+"',"\
   + fc::to_string(p.status) +","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"',"
   + fc::to_string(0)+",'" \
   + json+"');"; 
    _store_db_callback(sinsert);  
  }
  else  if (isNew ==0)
  {
    std::string sinsert = " UPDATE `proposal_object` SET   \
    `expiration_time` ='"+   p.expiration_time.to_db_string()+ "'," \
   "`review_period_time` ='"+  (p.review_period_time.valid() ? p.expiration_time.to_db_string() : "1970-01-01 00:00:00") + "'," \
    "`proposed_transaction` ='"+  json1 + "'," \
    "`memo` ='"+  p.memo + "'," \
    "`required_active_approvals` ='"+json_0   + "'," \
    "`available_active_approvals` ='"+  json_1 + "'," \
    "`required_owner_approvals` ='"+  json_2 + "'," \
    "`available_owner_approvals` ='"+ json_3  + "'," \
    "`available_key_approvals` ='"+  json_4 + "'," \
    "`proposer` ="+ fc::to_string(p.proposer.get_instance())  + "," \
    "`errorinfo` ='"+   chang_db_string(p.errorinfo) + "'," \
    "`status` ="+   fc::to_string(p.status)+ "," \
    "`block_time` = '"+ block_time.to_db_string()  + "',"\
    "`block_num` = "+fc::to_string(block_number) + ","\
    "`state` = "+fc::to_string(1) + ","\
    "`json` = '"+ json + "'  "\
    "WHERE  `id` = "+fc::to_string(p.id.instance())+";";  
     _store_db_callback(sinsert);
  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `proposal_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0); 

}

void store_db_plugin_impl::Prepare_operation_history_object(const operation_history_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
   if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `operation_history_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `operation_history_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `operation_history_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
  
}
void store_db_plugin_impl::Prepare_withdraw_permission_object(const withdraw_permission_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{  if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `withdraw_permission_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `withdraw_permission_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `withdraw_permission_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_vesting_balance_object(const vesting_balance_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{//2
  if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `vesting_balance_object` (  \
   `id`,\
   `owner`,\
   `from`,\
   `balance_id`,\
   `balance_amount`,\
   `policy`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(p.owner.get_instance())+","\
   +fc::to_string(p.from.get_instance())+","\
   +fc::to_string(p.balance.asset_id.get_instance())+","\
   +fc::to_string(p.balance.amount.value)+",'"\
   +to_db_string(p.policy)+"',"\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `vesting_balance_object` SET   \
    `owner` =  "+ fc::to_string(p.owner.get_instance())+ "," \
   "`from` =  "+ fc::to_string(p.from.get_instance())+ "," \
   "`balance_id` =  "+ fc::to_string(p.balance.asset_id.get_instance())+ "," \
   "`balance_amount` =  "+ fc::to_string(p.balance.amount.value)+ "," \
   "`policy` =  '"+ to_db_string(p.policy)+ "'," \
   "`state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `vesting_balance_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);


}

void store_db_plugin_impl::Prepare_balance_object(const balance_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
  linear_vesting_policy py;
  if(p.vesting_policy.valid())
    py = *p.vesting_policy;

  fc::variant vresult1,vresult2,vresult3;
  fc::to_variant( p, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::to_variant( py, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);  
  fc::to_variant( p.owner, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json3 = fc::json::to_string(vresult3);

  asset allowed_withdraw = p.available(block_time);

  if(isNew ==1)
  {
    std::string sinsert = " INSERT INTO  `balance_object` ( \
    `id`,\
    `owner`,\
    `balance_asset_id`,\
    `balance_asset_amount`,\
    `vesting_policy-type`,\
    `vesting_policy`,\
    `last_claim_date`,\
    `sync_balance`,\
    `allowed_withdraw`,\
    `allowed_withdraw_time`,\
    `insert_num`,\
    `block_num`,\
    `block_time`,\
    `state`,\
    `json`)\
    VALUES ( "\
    +fc::to_string(p.id.instance())+",'"\
    +json3+"',"\
    +fc::to_string(p.balance.asset_id.get_instance())+","\
    +fc::to_string(p.balance.amount.value)+","\
    +fc::to_string(0)+",'"\
    +json2+"','"\
    +p.last_claim_date.to_db_string() +"',"\
    +fc::to_string(p.sync_balance.value)+","\
    +fc::to_string(allowed_withdraw.amount.value)+",'"\
    +block_time.to_db_string() +"',"\
    + fc::to_string(block_number)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"',"
    +fc::to_string(0)+",'"\
    + json1+"');"; 
   
   _store_db_callback(sinsert);

  
  }
  else  if(isNew ==0)
  {
    std::string sinsert = " UPDATE  `balance_object` SET \
    `balance_asset_id` =  "+fc::to_string(p.balance.asset_id.get_instance()) + "," \
   "`balance_asset_amount` =  "+fc::to_string(p.balance.amount.value) + "," \
   "`vesting_policy-type` =  "+fc::to_string(0) + "," \
   "`vesting_policy` = '"+json2 + "'," \
   "`last_claim_date` =  '"+p.last_claim_date.to_db_string() + "'," \
   "`sync_balance` =  "+fc::to_string(p.sync_balance.value) + "," \
   "`allowed_withdraw` =  "+fc::to_string(allowed_withdraw.amount.value) + "," \
   "`allowed_withdraw_time` =  '"+block_time.to_db_string() + "'," \
   "`block_num` =  "+fc::to_string(block_number)  + "," \
   "`block_time` =  '"+block_time.to_db_string()  + "'," \
   "`state` =  "+fc::to_string(1) + "," \
   "`json` =  '"+json1 + "' " \
   "WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";

    _store_db_callback(sinsert);
  }
  else  if(isNew ==2)
  {
      std::string sinsert = " UPDATE  `balance_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_account_balance_object(const account_balance_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
  if(isNew )
  {
     std::string sinsert = " \
        INSERT INTO `account_balance_object` \
        (`id`,\
         `asset_id`,\
         `account_id`,\
         `insert_num`,\
         `block_num`,\
         `state`,\
         `balance`,\
         `block_time`)\
          VALUES\
        ("+fc::to_string(p.id.instance())+","\
          +fc::to_string(p.asset_type.get_instance())+","\
         +fc::to_string(p.owner.get_instance())+","\
         +fc::to_string(block_number)+","\
         +fc::to_string(block_number)+","\
         +fc::to_string(0)+","\
         +fc::to_string(p.balance.value)+",'"\
         +block_time.to_db_string()+"');";    
     _store_db_callback(sinsert);
  }      
  else
  {
      std::string sinsert = \
      "UPDATE `account_balance_object` SET   `block_num` = "+fc::to_string(block_number)+", "\
      +"`balance` = "+fc::to_string(p.balance.value)+", "\
      +"`state` = "+fc::to_string(1)+", "\
      +"`block_time` = '"+block_time.to_db_string()+"'  "\
      +"WHERE `id` =  "+fc::to_string(p.id.instance()) +";";
       _store_db_callback(sinsert);
  }
}

void store_db_plugin_impl::Prepare_global_property_object(const global_property_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{

if(isNew)
  {
   std::string sinsert = " INSERT INTO `global_property_object` (  \
   `id`,\
   `parameters`,\
   `pending_parameters`,\
   `next_available_vote_id`,\
   `active_committee_members`,\
   `active_budget_members`,\
   `active_witnesses`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+",'"\
   +to_db_string(p.parameters)  +"','"\
   +to_optional_string(p.pending_parameters)+"',"\
   +fc::to_string(p.next_available_vote_id)+",'"\
   +to_db_string(p.active_committee_members)+"','"\
   +to_db_string(p.active_budget_members)+"','"\
   +to_db_string(p.active_witnesses)+"',"\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);

   Prepare_global_property_object(p, block_time, block_number, 0); //参数太多了，所以只弄一次，不要删除
    }
  else 
  {
    std::string sinsert = " UPDATE `global_property_object` SET   \
    `parameters` =  '"+  to_db_string(p.parameters) + "'," \
   "`pending_parameters` =  '"+  to_optional_string(p.pending_parameters) + "'," \
   "`current_fees` =  '"+ to_db_string(p.parameters.current_fees) + "'," \
   "`block_interval` =  "+ fc::to_string(p.parameters.block_interval) + "," \
   "`maintenance_interval` =  "+ fc::to_string(p.parameters.maintenance_interval) + "," \
   "`maintenance_skip_slots` =  "+ fc::to_string(p.parameters.maintenance_skip_slots) + "," \
   "`committee_proposal_review_period` =  "+ fc::to_string(p.parameters.committee_proposal_review_period) + "," \
   "`maximum_transaction_size` =  "+ fc::to_string(p.parameters.maximum_transaction_size) + "," \
   "`maximum_block_size` =  "+ fc::to_string(p.parameters.maximum_block_size) + "," \
   "`maximum_time_until_expiration` =  "+ fc::to_string(p.parameters.maximum_time_until_expiration) + "," \
   "`maximum_proposal_lifetime` =  "+ fc::to_string(p.parameters.maximum_proposal_lifetime) + "," \
   "`maximum_asset_whitelist_authorities` =  "+ fc::to_string(p.parameters.maximum_asset_whitelist_authorities) + "," \
   "`maximum_asset_feed_publishers` =  "+ fc::to_string(p.parameters.maximum_asset_feed_publishers) + "," \
   "`maximum_witness_count` =  "+ fc::to_string(p.parameters.maximum_witness_count) + "," \
   "`maximum_committee_count` =  "+ fc::to_string(p.parameters.maximum_committee_count) + "," \
   "`maximum_budget_count` =  "+ fc::to_string(p.parameters.maximum_budget_count) + "," \
   "`maximum_authority_membership` =  "+ fc::to_string(p.parameters.maximum_authority_membership) + "," \
   "`reserve_percent_of_fee` =  "+ fc::to_string(p.parameters.reserve_percent_of_fee) + "," \
   "`network_percent_of_fee` =  "+ fc::to_string(p.parameters.network_percent_of_fee) + "," \
   "`lifetime_referrer_percent_of_fee` =  "+ fc::to_string(p.parameters.lifetime_referrer_percent_of_fee) + "," \
   "`cashback_vesting_period_seconds` =  "+ fc::to_string(p.parameters.cashback_vesting_period_seconds) + "," \
   "`cashback_vesting_threshold` =  "+ fc::to_string(p.parameters.cashback_vesting_threshold.value) + "," \
   "`count_non_member_votes` =  "+ fc::to_string(p.parameters.count_non_member_votes) + "," \
   "`allow_non_member_whitelists` =  "+ fc::to_string(p.parameters.allow_non_member_whitelists) + "," \
   "`witness_pay_per_block` =  "+ fc::to_string(p.parameters.witness_pay_per_block.value) + "," \
   "`witness_pay_vesting_seconds` =  "+ fc::to_string(p.parameters.witness_pay_vesting_seconds) + "," \
   "`max_worker_budget_per_day` =  "+ fc::to_string(p.parameters.max_worker_budget_per_day.value) + "," \
   "`identify_lock_vesting_seconds` =  "+ fc::to_string(p.parameters.identify_lock_vesting_seconds) + "," \
   "`max_predicate_opcode` =  "+ fc::to_string(p.parameters.max_predicate_opcode) + "," \
   "`fee_liquidation_threshold` =  "+ fc::to_string(p.parameters.fee_liquidation_threshold.value) + "," \
   "`accounts_per_fee_scale` =  "+ fc::to_string(p.parameters.accounts_per_fee_scale) + "," \
   "`account_fee_scale_bitshifts` =  "+ fc::to_string(p.parameters.account_fee_scale_bitshifts) + "," \
   "`max_authority_depth` =  "+ fc::to_string(p.parameters.max_authority_depth) + "," \
   "`coupon_per_month` =  "+ fc::to_string(p.parameters.coupon_per_month.value) + "," \
   "`coupon_expire_time` =  "+ fc::to_string(p.parameters.coupon_expire_time) + "," \
   "`next_available_vote_id` =  "+ fc::to_string(p.next_available_vote_id) + "," \
   "`active_committee_members` =  '"+ to_db_string(p.active_committee_members) + "'," \
   "`active_budget_members` =  '"+ to_db_string(p.active_budget_members) + "'," \
   "`active_witnesses` = '"+to_db_string(p.active_witnesses) + "'," \
   "`state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);
  }

}
void store_db_plugin_impl::Prepare_dynamic_global_property_object(const dynamic_global_property_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
  if(isNew )
  {
     std::string sinsert = " INSERT INTO `dynamic_global_property_object`  ( \
     `id`,\
     `head_block_number`,\
     `head_block_id`,\
     `time`,\
     `initial_time`,\
     `current_witness`,\
     `next_maintenance_time`,\
     `last_budget_time`,\
     `accounts_registered_this_interval`,\
     `recently_missed_count`,\
     `current_aslot`,\
     `miner_budget`,\
     `worker_budget`,\
     `coupon_supply`,\
     `miner_supply`,\
     `miner_fee_used`,\
     `budget_supply`,\
     `budget_fees_used`,\
     `recent_slots_filled`,\
     `dynamic_flags`,\
     `last_irreversible_block_num`,\
     `state`,\
     `insert_num`,\
     `block_num`,\
     `block_time`,\
     `json`)\
     VALUES ( "\
     +fc::to_string(p.id.instance())+","\
     +fc::to_string(p.head_block_number)+",'"\
     +to_db_string(p.head_block_id)+"','"\
     +p.time.to_db_string()+"','"\
     +p.initial_time.to_db_string()+"',"\
     +fc::to_string(p.current_witness.get_instance())+",'"\
     +p.next_maintenance_time.to_db_string()+"','"\
     +p.last_budget_time.to_db_string()+"',"\
     +fc::to_string(p.accounts_registered_this_interval)+","\
     +fc::to_string(p.recently_missed_count)+","\
     +fc::to_string(p.current_aslot)+",'"\
     +to_db_string(p.miners_budget)+"',"\
     +"0" +","\
     +fc::to_string(p.coupon_supply.value)+","\
     +fc::to_string(p.miner_supply.value)+","\
     +fc::to_string(p.miner_fee_used.value)+","\
     +fc::to_string(p.budget_supply.value)+","\
     +fc::to_string(p.budget_fees_used.value)+",'"\
     +to_db_string(p.recent_slots_filled)+"',"\
     +fc::to_string(p.dynamic_flags)+","\
     +fc::to_string(p.last_irreversible_block_num)+","\
     +fc::to_string(0)+","\
     + fc::to_string(block_number)+"," \
     + fc::to_string(block_number)+",'" \
     + block_time.to_db_string()+"','"
     + to_db_string(p)+"');";    
   _store_db_callback(sinsert);
  }
  else
  {
 std::string sinsert = " UPDATE `dynamic_global_property_object` SET   \
  `head_block_number` = "+ fc::to_string(p.head_block_number) + "," \
 "`head_block_id` =  '"+  to_db_string(p.head_block_id)+ "'," \
 "`time` =  '"+ p.time.to_db_string()+ "'," \
 "`initial_time` =  '"+ p.initial_time.to_db_string() + "'," \
 "`current_witness` = "+  fc::to_string(p.current_witness.get_instance())+ "," \
 "`next_maintenance_time` =  '"+ p.next_maintenance_time.to_db_string() + "'," \
 "`last_budget_time` =  '"+  p.last_budget_time.to_db_string()+ "'," \
 "`accounts_registered_this_interval` =  "+ fc::to_string(p.accounts_registered_this_interval) + "," \
 "`recently_missed_count` =  "+  fc::to_string(p.recently_missed_count)+ "," \
 "`current_aslot` = "+ fc::to_string(p.current_aslot) + "," \
 "`miner_budget` =  '"+ to_db_string(p.miners_budget) + "'," \
 "`worker_budget` =  "+  "0"+ "," \
 "`coupon_supply` = "+ fc::to_string(p.coupon_supply.value) + "," \
 "`miner_supply` = "+ fc::to_string(p.miner_supply.value) + "," \
 "`miner_fee_used` =  "+fc::to_string(p.miner_fee_used.value)  + "," \
 "`budget_supply` =  "+fc::to_string(p.budget_supply.value)  + "," \
 "`budget_fees_used` =  "+fc::to_string(p.budget_fees_used.value)  + "," \
 "`recent_slots_filled` =  '"+  to_db_string(p.recent_slots_filled)+ "'," \
 "`dynamic_flags` = "+ fc::to_string(p.dynamic_flags) + "," \
 "`last_irreversible_block_num` =  "+ fc::to_string(p.last_irreversible_block_num) + "," \
 "`state` = "+fc::to_string(1) + "," \
 "`block_num` = "+ fc::to_string(block_number)+ "," \
 "`block_time` = '"+ block_time.to_db_string()+ "'," \
 "`json` = '"+to_db_string(p) + "' " \
 "WHERE `id` = "+fc::to_string(p.id.instance())+";";

   _store_db_callback(sinsert);
  }

}
void store_db_plugin_impl::Prepare_asset_dynamic_data_object(const asset_dynamic_data_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
   fc::variant vresult;
  fc::to_variant( p, vresult, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json = fc::json::to_string(vresult);
  if(isNew)
  {
   std::string sinsert = " INSERT INTO `asset_dynamic_data_object` (  \
   `id`,\
   `current_supply`,\
   `confidential_supply`,\
   `accumulated_fees`,\
   `fee_pool`,\
   `sync_supply`,\
   `fundraise_supply`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(p.current_supply.value)+","\
   +fc::to_string(p.confidential_supply.value)+","\
   +fc::to_string(p.accumulated_fees.value)+","\
   +fc::to_string(p.fee_pool.value)+","\
   +fc::to_string(p.sync_supply.value)+","\
   +fc::to_string(p.fundraise_supply.value)+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + json+"');"; 
   
   _store_db_callback(sinsert);

  
  }
  else 
  {
    std::string sinsert = " UPDATE `asset_dynamic_data_object` SET   \
    `current_supply` =  "+fc::to_string(p.current_supply.value) + "," \
   "`confidential_supply` =  "+fc::to_string(p.confidential_supply.value) + "," \
   "`accumulated_fees` =  "+fc::to_string(p.accumulated_fees.value) + "," \
   "`fee_pool` =  "+fc::to_string(p.fee_pool.value) + "," \
   "`sync_supply` =  "+fc::to_string(p.sync_supply.value) + "," \
   "`fundraise_supply` = "+fc::to_string(p.fundraise_supply.value) + "," \
   "`state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+json + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
}
void store_db_plugin_impl::Prepare_asset_bitasset_data_object(const asset_bitasset_data_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
   fc::variant vresult;
  fc::to_variant( p, vresult, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json = fc::json::to_string(vresult);
  fc::variant vresult1;
  fc::to_variant( p.feeds, vresult1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json1 = fc::json::to_string(vresult1);
  fc::variant vresult2;
  fc::to_variant( p.current_feed, vresult2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json2 = fc::json::to_string(vresult2);
  fc::variant vresult3;
  fc::to_variant( p.settlement_price, vresult3, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json3 = fc::json::to_string(vresult3);
  if(isNew)
  {
   std::string sinsert = "  INSERT INTO `asset_bitasset_data_object` (  \
   `id`,\
   `feed_lifetime_sec`,\
   `minimum_feeds`,\
   `force_settlement_delay_sec`,\
   `force_settlement_offset_percent`,\
   `maximum_force_settlement_volume`,\
   `short_backing_asset`,\
   `feeds`,\
   `current_feed`,\
   `current_feed_publication_time`,\
   `is_prediction_market`,\
   `force_settled_volume`,\
   `settlement_price`,\
   `settlement_fund`,\
   `flags`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
    VALUES(  "\
    +fc::to_string(p.id.instance())+","\
    +fc::to_string(p.options.feed_lifetime_sec)+","\
    +fc::to_string(p.options.minimum_feeds)+","\
    +fc::to_string(p.options.force_settlement_delay_sec)+","\
    +fc::to_string(p.options.force_settlement_offset_percent)+","\
    +fc::to_string(p.options.maximum_force_settlement_volume)+","\
    +fc::to_string(p.options.short_backing_asset.get_instance())+",'"\
    +json1+"','"\
    +json2+"','"\
    +p.current_feed_publication_time.to_db_string()+"',"\
    +fc::to_string(p.is_prediction_market)+","\
    +fc::to_string(p.force_settled_volume.value)+",'"\
    +json3+"',"\
    +fc::to_string(p.settlement_fund.value)+","\
    +fc::to_string(p.flags)+","\
    +fc::to_string(0)+","\
    +fc::to_string(block_number)+"," \
    +fc::to_string(block_number)+",'" \
    +block_time.to_db_string()+"','"
    +json+"');"; 

   _store_db_callback(sinsert);


  }
  else
  {
    std::string sinsert = "  UPDATE `asset_bitasset_data_object` SET \
    `feed_lifetime_sec` = "+ fc::to_string(p.options.feed_lifetime_sec)+ "," \
    "`minimum_feeds` = "+ fc::to_string(p.options.minimum_feeds)+ "," \
    "`force_settlement_delay_sec` = "+ fc::to_string(p.options.force_settlement_delay_sec)+ "," \
    "`force_settlement_offset_percent` = "+ fc::to_string(p.options.force_settlement_offset_percent)+ "," \
    "`maximum_force_settlement_volume` = "+ fc::to_string(p.options.maximum_force_settlement_volume)+ "," \
    "`short_backing_asset` = "+ fc::to_string(p.options.short_backing_asset.get_instance())+ "," \
    "`feeds` = '"+ json1 + "'," \
    "`current_feed` = '"+ json2+ "'," \
    "`current_feed_publication_time` = '"+ p.current_feed_publication_time.to_db_string()+ "'," \
    "`is_prediction_market` = "+ fc::to_string(p.is_prediction_market)+ "," \
    "`force_settled_volume` = "+ fc::to_string(p.force_settled_volume.value)+ "," \
    "`settlement_price` = '"+ json3+ "'," \
    "`settlement_fund` = "+ fc::to_string(p.settlement_fund.value)+ "," \
    "`flags` = "+ fc::to_string(p.flags)+ "," \
    "`state` = "+ fc::to_string(1)+ "," \
    "`block_num` = "+ fc::to_string(block_number)+ "," \
    "`block_time` = '"+ block_time.to_db_string()+ "'," \
    "`json` = '"+json + "' " \
    "WHERE `id` = "+fc::to_string(p.id.instance())+";";

    _store_db_callback(sinsert);

  }
}
void store_db_plugin_impl::Prepare_asset_exchange_feed_object(const asset_exchange_feed_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
         
  if(isNew)
  {
   std::string sinsert = " INSERT INTO `asset_exchange_feed_object` (  \
   `id`,\
   `base`,\
   `quote`,\
   `type`,\
   `options`,\
   `feeds`,\
   `current_feed`,\
   `current_feed_publication_time`,\
   `force_settled_volume`,\
   `settlement_price`,\
   `settlement_fund`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(p.base.get_instance())+","\
   +fc::to_string(p.quote.get_instance())+","\
   +fc::to_string(p.type)+",'"\
   +to_db_string(p.options)+"','"\
   +to_db_string(p.feeds)+"','"\
   +to_db_string(p.current_feed)+"','"\
   +p.current_feed_publication_time.to_db_string()+"',"\
   +fc::to_string(p.force_settled_volume.value)+",'"\
   +to_db_string(p.settlement_price)+"',"\
   +fc::to_string(p.settlement_fund.value)+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else 
  {
    std::string sinsert = " UPDATE `asset_exchange_feed_object` SET   \
    `base` = "+  fc::to_string(p.base.get_instance())+ "," \
   "`quote` = "+ fc::to_string(p.quote.get_instance()) + "," \
   "`type` = "+ fc::to_string(p.type) + "," \
   "`options` = '"+ to_db_string(p.options) + "'," \
   "`feeds` = '"+ to_db_string(p.feeds) + "'," \
   "`current_feed` = '"+  to_db_string(p.current_feed)+ "'," \
   "`current_feed_publication_time` = '"+ p.current_feed_publication_time.to_db_string() + "'," \
   "`force_settled_volume` ="+ fc::to_string(p.force_settled_volume.value) + "," \
   "`settlement_price` =' "+ to_db_string(p.settlement_price) + "'," \
   "`settlement_fund` ="+  fc::to_string(p.settlement_fund.value)+ "," \
   "`state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);
  }
}
void store_db_plugin_impl::Prepare_account_statistics_object(const account_statistics_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
  fc::variant vresult;
  fc::to_variant( p, vresult, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  std::string json = fc::json::to_string(vresult);
  if(isNew)
  {
   std::string sinsert = " INSERT INTO `account_statistics_object` ( \
   `id`,\
   `account_id`,\
   `total_core_in_orders`,\
   `lifetime_fees_paid`,\
   `pending_fees`,\
   `pending_vested_fees`,\
   `lock_asset_id`,\
   `lock_asset_amount`,\
   `exchange_fees`,\
   `coupon_fees`,\
   `amount_coupon`,\
   `coupon_month`,\
   `activate_time`,\
   `activate_count`,\
   `statistics_count`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(p.owner.get_instance())+","\
   +fc::to_string(p.total_core_in_orders.value)+","\
   +fc::to_string(p.lifetime_fees_paid.value)+","\
   +fc::to_string(p.pending_fees.value)+","\
   +fc::to_string(p.pending_vested_fees.value)+","\
   +fc::to_string(p.lock_asset.asset_id.get_instance())+","\
   +fc::to_string(p.lock_asset.amount.value)+","\
   +fc::to_string(p.exchange_fees.value)+","\
   +fc::to_string(p.coupon_fees.value)+","\
   +fc::to_string(p.amount_coupon.value)+","\
   +fc::to_string(p.coupon_month)+",'"\
   +p.activate_time.to_db_string() +"',"\
   +fc::to_string(p.activate_count)+","\
   +fc::to_string(p.statistics_count)+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + json+"');"; 
   
   _store_db_callback(sinsert);
  }
  else 
  {
   std::string sinsert = " UPDATE `account_statistics_object`  SET \
    `total_core_in_orders` = "+fc::to_string(p.total_core_in_orders.value) + "," \
   "`lifetime_fees_paid` = "+fc::to_string(p.lifetime_fees_paid.value) + "," \
   "`pending_fees` = "+ fc::to_string(p.pending_fees.value)+ "," \
   "`pending_vested_fees` = "+fc::to_string(p.pending_vested_fees.value) + "," \
   "`lock_asset_id` = "+ fc::to_string(p.lock_asset.asset_id.get_instance())+ "," \
   "`lock_asset_amount` = "+ fc::to_string(p.lock_asset.amount.value)+ "," \
   "`exchange_fees` = "+ fc::to_string(p.exchange_fees.value)+ "," \
   "`coupon_fees` = "+ fc::to_string(p.coupon_fees.value)+ "," \
   "`amount_coupon` = "+fc::to_string(p.amount_coupon.value) + "," \
   "`coupon_month` = "+ fc::to_string(p.coupon_month)+ "," \
   "`activate_time` = '"+ p.activate_time.to_db_string()+ "'," \
   "`activate_count` = "+ fc::to_string(p.activate_count)+ "," \
   "`statistics_count` = "+ fc::to_string(p.statistics_count)+ "," \
   "`state` = "+ fc::to_string(1)+ "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+json + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";

   _store_db_callback(sinsert);
  }
}

void store_db_plugin_impl::Prepare_transaction_object(const transaction_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{ 
  if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `transaction_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `transaction_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `transaction_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
  
}
void store_db_plugin_impl::Prepare_block_summary_object(const block_summary_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{   
   graphene::chain::database &db = _self.database();
   optional<signed_block> singblock = db.fetch_block_by_number(block_number);
   assert(singblock.valid());

   signed_block_header header = *singblock;
   fc::variant vresult;
   fc::to_variant( header, vresult, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   std::string json = fc::json::to_string(vresult);

  
    std::string sinsert = " INSERT INTO `block_summary_object` \
    (`block_num`,\
    `version`,\
     `block_id`,\
    `block_time`,\
    `trans_count`,\
    `witness_id`,\
    `confrim_time`,\
    `insert_num`,\
    `state`,\
    `json`)\
    VALUES( " \
    +fc::to_string(block_header::num_from_id(p.block_id))+","\
    +fc::to_string(p.version)+",'"\
    +fc::json::to_string(p.block_id)+"','"\
    +p.block_time.to_db_string()+"',"\
    +fc::to_string(p.trans_count)+","\
    +fc::to_string(p.witness_id.get_instance())+","\
    +fc::to_string(p.confrim_time)+","\
    +fc::to_string(block_header::num_from_id(p.block_id))+","\
    +fc::to_string(1)+",'"\
    +json+"');";    
    
    _store_db_callback(sinsert);

    
}
 
void store_db_plugin_impl::Prepare_chain_property_object(const chain_property_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
if(isNew)
  {
   std::string sinsert = " INSERT INTO `chain_property_object` ( \
   `id`,\
   `chain_id`,\
   `min_committee_member_count`,\
   `min_budget_member_count`,\
   `min_witness_count`,\
   `num_special_accounts`,\
   `num_special_assets`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
    VALUES ( "\
    +fc::to_string(p.id.instance())+",'"\
    +to_db_string(p.chain_id)+"',"\
    +fc::to_string(p.immutable_parameters.min_committee_member_count)+","\
    +fc::to_string(p.immutable_parameters.min_budget_member_count)+","\
    +fc::to_string(p.immutable_parameters.min_witness_count)+","\
    +fc::to_string(p.immutable_parameters.num_special_accounts)+","\
    +fc::to_string(p.immutable_parameters.num_special_assets)+","\
    +fc::to_string(0)+","\
    + fc::to_string(block_number)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"','"
    + to_db_string(p)+"');"; 
   }
  else
  {
    std::string sinsert = "  UPDATE `chain_property_object` SET \
    `chain_id` = '"+ to_db_string(p.chain_id) + "'," \
    "`min_committee_member_count` = "+ fc::to_string(p.immutable_parameters.min_committee_member_count)+ "," \
    "`min_budget_member_count` = "+ fc::to_string(p.immutable_parameters.min_budget_member_count)+ "," \
    "`min_witness_count` = "+ fc::to_string(p.immutable_parameters.min_witness_count)+ "," \
    "`num_special_accounts` = "+ fc::to_string(p.immutable_parameters.num_special_accounts)+ "," \
    "`num_special_assets` = "+ fc::to_string(p.immutable_parameters.num_special_assets)+ "," \
    "`state` = "+ fc::to_string(1)+ "," \
    "`block_num` = "+ fc::to_string(block_number)+ "," \
    "`block_time` = '"+ block_time.to_db_string()+ "'," \
    "`json` = '"+to_db_string(p) + "' " \
    "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);
  }

}
void store_db_plugin_impl::Prepare_witness_schedule_object(const witness_schedule_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{   
if(isNew)
  {
   std::string sinsert = " INSERT INTO `witness_schedule_object` ( \
   `id`,\
   `current_shuffled_witnesses`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
    VALUES ( "\
    +fc::to_string(p.id.instance())+",'"\
    +to_db_string(p.current_shuffled_witnesses)+"',"\
    +fc::to_string(0)+","\
    + fc::to_string(block_number)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"','"
    + to_db_string(p)+"');"; 
   }
  else
  {
    std::string sinsert = "  UPDATE `witness_schedule_object` SET \
    `current_shuffled_witnesses` = '"+ to_db_string(p.current_shuffled_witnesses) + "'," \
    "`state` = "+ fc::to_string(1)+ "," \
    "`block_num` = "+ fc::to_string(block_number)+ "," \
    "`block_time` = '"+ block_time.to_db_string()+ "'," \
    "`json` = '"+to_db_string(p) + "' " \
    "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);
  }


}
void store_db_plugin_impl::Prepare_budget_record_object(const budget_record_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{

  if(isNew)
  {
   std::string sinsert = " INSERT INTO `budget_record_object` ( \
   `id`,\
   `time`,\
   `time_since_last_budget`,\
   `from_initial_reserve`,\
   `from_accumulated_fees`,\
   `from_platform_fees`,\
   `from_unused_miner_budget`,\
   `miner_budget`,\
   `worker_budget`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
    VALUES ( "\
    +fc::to_string(p.id.instance())+",'"\
   + p.time.to_db_string()+"',"\
    +fc::to_string(p.record.time_since_last_budget)+","\
    +fc::to_string(p.record.from_initial_reserve.value)+","\
    +fc::to_string(p.record.from_accumulated_fees.value)+","\
    +fc::to_string(p.record.from_platform_fees.value)+",'"\
    +to_db_string(p.record.from_unused_miner_budget)+"','"\
    +to_db_string(p.record.miner_budget)+"',"\
    +fc::to_string(p.record.worker_budget.value)+","\
    +fc::to_string(0)+","\
    + fc::to_string(block_number)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"','"\
    + to_db_string(p)+"');"; 

      _store_db_callback(sinsert);
   }
  else
  {
    std::string sinsert = "  UPDATE `budget_record_object` SET \
    `time` = '"+  p.time.to_db_string()+ "'," \
    "`time_since_last_budget` = "+ fc::to_string(p.record.time_since_last_budget) + "," \
    "`from_initial_reserve` = "+ fc::to_string(p.record.time_since_last_budget) + "," \
    "`from_accumulated_fees` = "+  fc::to_string(p.record.from_initial_reserve.value)+ "," \
    "`from_platform_fees` = "+  fc::to_string(p.record.from_accumulated_fees.value)+ "," \
    "`from_unused_miner_budget` = '"+ to_db_string(p.record.from_unused_miner_budget)+ "'," \
    "`miner_budget` = '"+ to_db_string(p.record.miner_budget) + "'," \
    "`worker_budget` = "+ fc::to_string(p.record.worker_budget.value) + "," \
    "`state` = "+ fc::to_string(1)+ "," \
    "`block_num` = "+ fc::to_string(block_number)+ "," \
    "`block_time` = '"+ block_time.to_db_string()+ "'," \
    "`json` = '"+to_db_string(p) + "' " \
    "WHERE `id` = "+fc::to_string(p.id.instance())+";";

    _store_db_callback(sinsert);

  }


}
void store_db_plugin_impl::Prepare_blinded_balance_object(const blinded_balance_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
    if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `blinded_balance_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `blinded_balance_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `blinded_balance_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_special_authority_object(const special_authority_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
 if(isNew)
  {
   std::string sinsert = " INSERT INTO `special_authority_object` ( \
   `id`,\
   `account`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
    VALUES ( "\
    +fc::to_string(p.id.instance())+","\
    +fc::to_string(p.account.get_instance())+","\
    +fc::to_string(0)+","\
    + fc::to_string(block_number)+"," \
    + fc::to_string(block_number)+",'" \
    + block_time.to_db_string()+"','"
    + to_db_string(p)+"');"; 
   }
  else
  {
    std::string sinsert = "  UPDATE `special_authority_object` SET \
    `account` = "+ fc::to_string(p.account.get_instance())+ "," \
    "`state` = "+ fc::to_string(1)+ "," \
    "`block_num` = "+ fc::to_string(block_number)+ "," \
    "`block_time` = '"+ block_time.to_db_string()+ "'," \
    "`json` = '"+to_db_string(p) + "' " \
    "WHERE `id` = "+fc::to_string(p.id.instance())+";";

    _store_db_callback(sinsert);

  }


}
void store_db_plugin_impl::Prepare_buyback_object(const buyback_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
      if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `buyback_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `buyback_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `buyback_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_fba_accumulator_object(const fba_accumulator_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
      if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `fba_accumulator_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `fba_accumulator_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `fba_accumulator_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_collateral_bid_object(const collateral_bid_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{
      if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `collateral_bid_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `collateral_bid_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `collateral_bid_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}
void store_db_plugin_impl::Prepare_finance_paramers_object(const finance_paramers_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{     if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `finance_paramers_object` (  \
   `id`,\
   `state`,\
   `insert_num`,\
   `block_num`,\
   `block_time`,\
   `json`)\
   VALUES ( "\
   +fc::to_string(p.id.instance())+","\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   
   _store_db_callback(sinsert);  
  }
  else  if(isNew == 0)
  {
    std::string sinsert = " UPDATE `finance_paramers_object` SET   \
    `state` = "+fc::to_string(1) + "," \
   "`block_num` = "+ fc::to_string(block_number)+ "," \
   "`block_time` = '"+ block_time.to_db_string()+ "'," \
   "`json` = '"+to_db_string(p) + "' " \
   "WHERE `id` = "+fc::to_string(p.id.instance())+";";
    _store_db_callback(sinsert);

  }
  else  if (isNew ==2)
  {
       std::string sinsert = " UPDATE  `finance_paramers_object` SET  `state` =  "+fc::to_string(2) + ","\
        "`block_num` =  "+fc::to_string(block_number)  + ",`block_time` =  '"+block_time.to_db_string()  + "' " + \
        "  WHERE `id` =  "+fc::to_string(p.id.instance()) + ";";
        _store_db_callback(sinsert);
  }
  else
    assert(0);
}

void store_db_plugin_impl::Prepare_bitlender_paramers_object(const bitlender_paramers_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew)
{   
  if(isNew == 1)
  {
   std::string sinsert = " INSERT INTO `bitlender_paramers_object` ( \
   `id`,\
   `options`,\
   `pending_options`,\
   `state`,\
  `insert_num`,\
  `block_num`,\
  `block_time`,\
  `json`)\
   VALUES ( "\
    +fc::to_string(p.id.instance())+",'"\
   +to_db_string(p.options)+"','"\
   +to_db_string(p.pending_options)+"',"\
   +fc::to_string(0)+","\
   + fc::to_string(block_number)+"," \
   + fc::to_string(block_number)+",'" \
   + block_time.to_db_string()+"','"
   + to_db_string(p)+"');"; 
   }    
  else if(isNew == 0)
  {
    std::string sinsert = "  UPDATE `bitlender_paramers_object` SET \
    `options` = '"+ to_db_string(p.options)+ "'," \
    "`pending_options` = '"+ to_db_string(p.pending_options) + "'," \
    "`state` = "+ fc::to_string(1)+ "," \
    "`block_num` = "+ fc::to_string(block_number)+ "," \
    "`block_time` = '"+ block_time.to_db_string()+ "'," \
    "`json` = '"+to_db_string(p) + "' " \
    "WHERE `id` = "+fc::to_string(p.id.instance())+";";

    _store_db_callback(sinsert);

  }
}
     
} } }



 