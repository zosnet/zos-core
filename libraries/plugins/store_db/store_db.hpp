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

#include <graphene/store_db/store_db_plugin.hpp>
#include <fc/smart_ref_impl.hpp>

#include <graphene/chain/block_summary_object.hpp>
#include <graphene/chain/vesting_balance_object.hpp>
#include <graphene/chain/balance_object.hpp>
#include <graphene/business/bitlender_option_object.hpp>
#include <graphene/business/market_object.hpp>
#include <graphene/chain/budget_member_object.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/fba_object.hpp>
#include <graphene/chain/budget_record_object.hpp>
#include <graphene/business/confidential_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/gateway_object.hpp>
#include <graphene/chain/carrier_object.hpp>
#include <graphene/business/finance_object.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/chain/withdraw_permission_object.hpp>
#include <graphene/chain/worker_object.hpp>
#include <graphene/chain/transaction_object.hpp>
#include <graphene/chain/chain_property_object.hpp>
#include <graphene/chain/witness_schedule_object.hpp>
#include <graphene/chain/special_authority_object.hpp>
#include <graphene/chain/buyback_object.hpp>
#include <graphene/business/bitlender_object.hpp>
#include <graphene/business/locktoken_object.hpp>
#include <graphene/chain/author_object.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/chain/buyback_object.hpp> 


namespace graphene { namespace store_db {
 
namespace detail
{

#define    to_optional_string(x)  (x.valid() ? to_db_string(*x) : "")
#define    to_optional_time(x)    (x.valid() ? x->to_db_string() : "1970-01-01 00:00:00")
#define    to_optional_id(x)      (x.valid() ? fc::to_string(x->get_instance()) : "0")


inline std::string  chang_db_string(const std::string &b1)
{
  std::string b = b1;
  for (size_t l = 0; l < b.size(); l++)
  {
    if (b[l] == '\'' || b[l] == '`' ||  b[l] == '=' ||  b[l] == '>' || b[l] == '<')
      b[l] = '?';
    else if (b[l] < ' ')
      b[l] = ' ';
   }
   return b;
}

template<typename T>
inline std::string to_db_string(const T& o)
{
  fc::variant vresult;
  fc::to_variant( o, vresult, GRAPHENE_NET_MAX_NESTED_OBJECTS );
  return  chang_db_string(fc::json::to_string(vresult));
}


class store_db_plugin_impl
{
   public:
      store_db_plugin_impl(store_db_plugin& _plugin)
         : _self( _plugin )
      {   }
      virtual ~store_db_plugin_impl();

      void updateObjectChange( const vector<object_id_type>& ids , bool isNew) ;
      void updateObjectDel( const vector<object_id_type>& ids  );
      void updateBlockOperation(const signed_block &b);   
      void updateBlockOperation(const operation &p,const operation_result &r, transaction_id_type id, uint64_t num_trx,const uint64_t &num_op,const fc::time_point_sec &block_time, const uint64_t &block_number);
      void updateAccountBalanceHistory(const signed_block &b);

      void sql_query(const std::string &b);    

      store_db_plugin &_self;
      std::string db_url = "http://localhost:3060/";
      std::string db_user ="";
      std::string db_pwd  = "";
      std::string db_name  = "";
      int db_port = 3306;      
      bool store_db_trx = true;
      bool store_db_object = true;


      uint64_t block_start = 0;
      uint64_t block_apply = 0;
      std::function<void(const std::string&)> _store_db_callback;
      std::function<uint64_t()>               _store_db_start;
      void PrepareDB();
      void TruncateDB();
      void RemveDBBlockNum(uint64_t block_num);
      void Prepare_base_transation(const processed_transaction trx , const transaction_id_type id,const uint64_t &num_trx, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_base_operation(const operation &p, const operation_result &result, const transaction_id_type id,const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_transfer_operation(const transfer_operation &p, const operation_result &result, const transaction_id_type id,const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_limit_order_create_operation(const limit_order_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_limit_order_cancel_operation(const limit_order_cancel_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_call_order_update_operation(const call_order_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_fill_order_operation(const fill_order_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_account_create_operation(const account_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_account_update_operation(const account_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);     
      void Prepare_account_authenticate_operation(const account_authenticate_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_account_config_operation(const account_config_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_account_whitelist_operation(const account_whitelist_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_account_upgrade_operation(const account_upgrade_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_account_transfer_operation(const account_transfer_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_create_operation(const asset_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_update_operation(const asset_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_update_bitasset_operation(const asset_update_bitasset_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_update_gateway_operation(const asset_update_gateway_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_update_feed_producers_operation(const asset_update_feed_producers_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_issue_operation(const asset_issue_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_reserve_operation(const asset_reserve_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_fund_fee_pool_operation(const asset_fund_fee_pool_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_settle_operation(const asset_settle_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_global_settle_operation(const asset_global_settle_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_publish_feed_operation(const asset_publish_feed_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_witness_create_operation(const witness_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_witness_update_operation(const witness_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_proposal_create_operation(const proposal_create_operation &p, const operation_result &result,const transaction_id_type id, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_proposal_update_operation(const proposal_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_proposal_delete_operation(const proposal_delete_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_withdraw_permission_create_operation(const withdraw_permission_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_withdraw_permission_update_operation(const withdraw_permission_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_withdraw_permission_claim_operation(const withdraw_permission_claim_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_withdraw_permission_delete_operation(const withdraw_permission_delete_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_committee_member_create_operation(const committee_member_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_committee_member_update_operation(const committee_member_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_committee_member_update_global_parameters_operation(const committee_member_update_global_parameters_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_vesting_balance_create_operation(const vesting_balance_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_vesting_balance_withdraw_operation(const vesting_balance_withdraw_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_worker_create_operation(const worker_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_custom_operation(const custom_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_assert_operation(const assert_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_balance_claim_operation(const balance_claim_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_override_transfer_operation(const override_transfer_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_transfer_to_blind_operation(const transfer_to_blind_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_blind_transfer_operation(const blind_transfer_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_transfer_from_blind_operation(const transfer_from_blind_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_settle_cancel_operation(const asset_settle_cancel_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_claim_fees_operation(const asset_claim_fees_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_fba_distribute_operation(const fba_distribute_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bid_collateral_operation(const bid_collateral_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_execute_bid_operation(const execute_bid_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_reserve_fees_operation(const asset_reserve_fees_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_gateway_withdraw_operation(const gateway_withdraw_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_gateway_deposit_operation(const gateway_deposit_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_gateway_issue_currency_operation(const gateway_issue_currency_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_option_create_operation(const bitlender_option_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_option_author_operation(const bitlender_option_author_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_option_update_operation(const bitlender_option_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_option_fee_mode_operation(const bitlender_option_fee_mode_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_rate_update_operation(const bitlender_rate_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_option_stop_operation(const bitlender_option_stop_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_asset_property_operation(const asset_property_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_loan_operation(const bitlender_loan_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_invest_operation(const bitlender_invest_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_repay_interest_operation(const bitlender_repay_interest_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_overdue_interest_operation(const bitlender_overdue_interest_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_recycle_interest_operation(const bitlender_recycle_interest_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number); 
      void Prepare_bitlender_repay_principal_operation(const bitlender_repay_principal_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_pre_repay_principal_operation(const bitlender_pre_repay_principal_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_overdue_repay_principal_operation(const bitlender_overdue_repay_principal_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_add_collateral_operation(const bitlender_add_collateral_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_recycle_operation(const bitlender_recycle_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_setautorepayer_operation(const bitlender_setautorepayer_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_fill_object_history_operation(const fill_object_history_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_finance_option_create_operation(const  issue_fundraise_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_issue_fundraise_update_operation(const  issue_fundraise_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_issue_fundraise_remove_operation(const  issue_fundraise_remove_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_fundraise_publish_feed_operation(const  issue_fundraise_publish_feed_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_finance_paramers_update_operation(const  finance_paramers_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_fundraise_create_operation(const buy_fundraise_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_buy_fundraise_enable_operation(const buy_fundraise_enable_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_sell_exchange_create_operation(const sell_exchange_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_sell_exchange_update_operation(const sell_exchange_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_sell_exchange_remove_operation(const sell_exchange_remove_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_buy_exchange_create_operation(const buy_exchange_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_buy_exchange_update_operation(const buy_exchange_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_buy_exchange_remove_operation(const buy_exchange_remove_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_account_coupon_operation(const account_coupon_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_change_identity_operation(const change_identity_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_autorepayment_operation(const bitlender_autorepayment_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_withdraw_exchange_fee_operation(const withdraw_exchange_fee_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_paramers_update_operation(const bitlender_paramers_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_gateway_create_operation(const gateway_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_gateway_update_operation(const gateway_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_carrier_create_operation(const carrier_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_carrier_update_operation(const carrier_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_budget_member_create_operation(const budget_member_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_budget_member_update_operation(const budget_member_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_transfer_vesting_operation(const transfer_vesting_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_revoke_vesting_operation(const revoke_vesting_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_remove_operation(const bitlender_remove_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_squeeze_operation(const bitlender_squeeze_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_publish_feed_operation(const bitlender_publish_feed_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_update_feed_producers_operation(const bitlender_update_feed_producers_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_bitlender_test_operation(const bitlender_test_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      
      void Prepare_author_create_operation(const author_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_author_update_operation(const author_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_committee_member_update_zos_parameters_operation(const committee_member_update_zos_parameters_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_locktoken_create_operation(const locktoken_create_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_locktoken_update_operation(const locktoken_update_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_locktoken_remove_operation(const locktoken_remove_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_locktoken_node_operation(const locktoken_node_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
      void Prepare_locktoken_option_operation(const locktoken_option_operation &p, const operation_result &result, const uint64_t &num_trx, const uint64_t &num_op, const fc::time_point_sec &block_time, const uint64_t &block_number);
 

      void Prepare_account_object(const account_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_asset_object(const asset_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_bitlender_option_object(const bitlender_option_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_issue_fundraise_object(const issue_fundraise_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_force_settlement_object(const force_settlement_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_committee_member_object(const committee_member_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_budget_member_object(const budget_member_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_witness_object(const witness_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_gateway_object(const gateway_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_gateway_dynamic_object(const gateway_dynamic_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_carrier_object(const carrier_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_limit_order_object(const limit_order_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_call_order_object(const call_order_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_bitlender_order_object(const bitlender_order_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_buy_fundraise_object(const buy_fundraise_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);    
      void Prepare_buy_exchange_object(const buy_exchange_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);    
      void Prepare_sell_exchange_object(const sell_exchange_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);    
      void Prepare_proposal_object(const proposal_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_operation_history_object(const operation_history_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_withdraw_permission_object(const withdraw_permission_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_vesting_balance_object(const vesting_balance_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_worker_object(const worker_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_balance_object(const balance_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);      
      void Prepare_account_balance_object(const account_balance_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_global_property_object(const global_property_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_dynamic_global_property_object(const dynamic_global_property_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_asset_dynamic_data_object(const asset_dynamic_data_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_asset_bitasset_data_object(const asset_bitasset_data_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_asset_exchange_feed_object(const asset_exchange_feed_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);

      void Prepare_account_statistics_object(const account_statistics_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_transaction_object(const transaction_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_block_summary_object(const block_summary_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      //void Prepare_account_transaction_history_object(const account_transaction_history_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);

      //void Prepare_balance_history_object(const balance_history_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_chain_property_object(const chain_property_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_witness_schedule_object(const witness_schedule_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_budget_record_object(const budget_record_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_blinded_balance_object(const blinded_balance_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_special_authority_object(const special_authority_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_buyback_object(const buyback_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_fba_accumulator_object(const fba_accumulator_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_collateral_bid_object(const collateral_bid_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_bitlender_invest_object(const bitlender_invest_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_bitlender_paramers_object(const bitlender_paramers_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_finance_paramers_object(const finance_paramers_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_carrier_dynamic_object(const carrier_dynamic_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
       //void Prepare_bitlender_history_object(const bitlender_history_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_author_object(const author_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      void Prepare_locktoken_object(const locktoken_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew); 
      void Prepare_locktoken_options_object(const locktoken_option_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
     
 



      //void Prepare_bucket_object(const bucket_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);         
      //void Prepare_market_ticker_meta_object(const market_ticker_meta_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);    
      //void Prepare_order_history_object(const order_history_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
      //void Prepare_market_ticker_object(const market_ticker_object &p, const fc::time_point_sec &block_time, const uint64_t &block_number, const int isNew);
}; 
}

} }