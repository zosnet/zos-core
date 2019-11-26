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

#include <graphene/chain/operation_history_object.hpp>

namespace graphene { namespace store_db {

namespace detail
{
 
void store_db_plugin_impl::sql_query( const std::string& b )
{
   if(_store_db_callback)
      _store_db_callback(b);
} 
           /* limit_order_create_operation,   //1 挂单交易
            limit_order_cancel_operation,   //2 取消挂单
            call_order_update_operation,    //3 抵押发币
            fill_order_operation,           //4 VIRTUAL
            account_whitelist_operation,   //7 
            account_transfer_operation,   //9  
            asset_issue_operation,   //14
            asset_reserve_operation,   //15
            asset_fund_fee_pool_operation,   //16
            asset_settle_operation,   //17
            asset_global_settle_operation,   //18        
            withdraw_permission_create_operation,   //25
            withdraw_permission_update_operation,   //26
            withdraw_permission_claim_operation,   //27
            withdraw_permission_delete_operation,   //28
            custom_operation
            assert_operation     
            override_transfer_operation,   //38
            transfer_to_blind_operation,   //39 正常账户转帐到隐私账户
            blind_transfer_operation,   //40  隐私之间转账
            transfer_from_blind_operation,   //41 从隐私账户到正常账户
            asset_settle_cancel_operation,  //42 VIRTUAL
            fba_distribute_operation
            bid_collateral_operation
            execute_bid_operation
            asset_reserve_fees_operation                
            fill_object_history_operation,          //66 VIRTUAL 
            issue_fundraise_create_operation,        //67 创建筹资参数            
            issue_fundraise_update_operation,        //68 修改筹资参数     
            buy_fundraise_create_operation,               //69 筹资 
            buy_fundraise_enable_operation,               //70 筹资有效 
            issue_fundraise_publish_feed_operation,        //90 众筹喂价            
            finance_paramers_update_operation,        //91 众筹系统参数     
            sell_exchange_create_operation,        //92 发布兑换   
            sell_exchange_update_operation,        //93 更新兑换
            sell_exchange_remove_operation,        //94 删除兑换
            buy_exchange_create_operation,        //95  发布购买 
            buy_exchange_update_operation,        //96 更新购买
            buy_exchange_remove_operation,        //97  删除购买 
            issue_fundraise_remove_operation,     //98  删除筹资     
            bitlender_option_fee_mode_operation,   //99 修改法币参数        
            account_authenticate_operation,      //100 认证信息
            author_create_operation,            //101 创建认证人
            author_update_operation,            //102 更新认证人
            bitlender_recycle_interest_operation, //103 利息逾期
            account_config_operation,      //104 用户配置
            asset_update_gateway_operation,   //105 更新资产网关     
            committee_member_update_zos_parameters_operation,   //106 修改ZOS系统参数     
            locktoken_create_operation,   //107 创建锁仓
            locktoken_update_operation,   //108 修改锁仓
            locktoken_remove_operation,   //109  取消锁仓  
            locktoken_node_operation,   //110 收益锁仓
            locktoken_option_operation   //111 锁仓参数
            */

    
         //  3
    /*  force_settlement_object_type,//4      
      limit_order_object_type,//7
      call_order_object_type,//8
      custom_object_type,//9      
      operation_history_object_type,//11
      withdraw_permission_object_type,//12   
      issue_fundraise_object_type,//18
      buy_fundraise_object_type,//19     
      sell_exchange_object_type,//23
      buy_exchange_object_type,//24
      author_object_type,//25
      locktoken_object_type,//26
      locktoken_options_object_type,//27     
      */
      
 

 

  /*    impl_reserved0_object_type,                //2 formerly index_meta_object_type, TODO: delete me         
      impl_transaction_object_type,//7
      impl_account_transaction_history_object_type,//9
      impl_blinded_balance_object_type,//10
      impl_chain_property_object_type,//11      
      impl_buyback_object_type,//15
      impl_fba_accumulator_object_type,//16
      impl_collateral_bid_object_type,//17      
      impl_account_balance_history_object_type,//19     
      impl_finance_paramers_object_type,//22
      impl_locktoken_history_object_type,//25
      */





void  store_db_plugin_impl::TruncateDB()
{
      ilog( "start truncate DBBlock");

      std::string sql = "";
      sql = "TRUNCATE `transfer_operation`;";  //1
      _store_db_callback(sql);
      sql = "TRUNCATE `account_create_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `limit_order_create_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `limit_order_cancel_operation`;";
      _store_db_callback(sql); 
      sql = "TRUNCATE `call_order_update_operation`;";
      _store_db_callback(sql);     
      sql = "TRUNCATE `fill_order_operation`;";
      _store_db_callback(sql);  
      sql = "TRUNCATE `bitlender_option_update_operation`;";//2
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_option_fee_mode_operation`;";//3
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_option_create_operation`;";//4
      _store_db_callback(sql);
      sql = "TRUNCATE `gateway_deposit_operation`;";//5
      _store_db_callback(sql);
      sql = "TRUNCATE `gateway_withdraw_operation`;";//6
      _store_db_callback(sql);
      sql = "TRUNCATE `gateway_issue_currency_operation`;";//7
      _store_db_callback(sql);
      sql = "TRUNCATE `proposal_create_operation`;";//8
      _store_db_callback(sql);
      sql = "TRUNCATE `proposal_update_operation`;";//9
      _store_db_callback(sql);
      sql = "TRUNCATE `proposal_delete_operation`;";//10
      _store_db_callback(sql);
      sql = "TRUNCATE `withdraw_permission_create_operation`;";//10
      _store_db_callback(sql);
      sql = "TRUNCATE `withdraw_permission_update_operation`;";//10
      _store_db_callback(sql);
      sql = "TRUNCATE `withdraw_permission_claim_operation`;";//10
      _store_db_callback(sql);
      sql = "TRUNCATE `withdraw_permission_delete_operation`;";//10
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_option_author_operation`;";//12
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_option_stop_operation`;";//13
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_rate_update_operation`;";//14
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_loan_operation`;";//15
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_invest_operation`;";//16
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_repay_interest_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_overdue_interest_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_repay_principal_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_overdue_repay_principal_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_add_collateral_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_recycle_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_setautorepayer_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `fill_object_history_operation`;";
      _store_db_callback(sql);     
      sql = "TRUNCATE `bitlender_autorepayment_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_remove_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_squeeze_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_publish_feed_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_update_feed_producers_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_test_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_paramers_update_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `revoke_vesting_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `transfer_vesting_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `budget_member_update_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `budget_member_create_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `gateway_update_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `carrier_update_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `witness_update_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `committee_member_update_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `carrier_create_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `gateway_create_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `committee_member_create_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `witness_create_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `withdraw_exchange_fee_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `change_identity_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `issue_fundraise_create_operation`;";
      _store_db_callback(sql);
       sql = "TRUNCATE `issue_fundraise_update_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `buy_fundraise_create_operation`;";
      _store_db_callback(sql); 
       sql = "TRUNCATE `issue_fundraise_publish_feed_operation`;";
      _store_db_callback(sql);
       sql = "TRUNCATE `finance_paramers_update_operation`;";
      _store_db_callback(sql);
       sql = "TRUNCATE `sell_exchange_create_operation`;";
      _store_db_callback(sql);
       sql = "TRUNCATE `sell_exchange_update_operation`;";
      _store_db_callback(sql);
       sql = "TRUNCATE `sell_exchange_remove_operation`;";
      _store_db_callback(sql);
       sql = "TRUNCATE `buy_exchange_create_operation`;";
      _store_db_callback(sql);
       sql = "TRUNCATE `buy_exchange_update_operation`;";
      _store_db_callback(sql);
       sql = "TRUNCATE `buy_exchange_remove_operation`;";
      _store_db_callback(sql);
       sql = "TRUNCATE `issue_fundraise_remove_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `account_coupon_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_property_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_claim_fees_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `fba_distribute_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bid_collateral_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `execute_bid_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_reserve_fees_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `custom_operation`;";
      _store_db_callback(sql);
       sql = "TRUNCATE `assert_operation`;";
      _store_db_callback(sql);  
      sql = "TRUNCATE `balance_claim_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `override_transfer_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `transfer_to_blind_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `blind_transfer_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `transfer_from_blind_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_settle_cancel_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `worker_create_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `vesting_balance_withdraw_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `vesting_balance_create_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `committee_member_update_global_parameters_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `account_update_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `account_whitelist_operation`;";
      _store_db_callback(sql);      
      sql = "TRUNCATE `account_upgrade_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `account_transfer_operation`;";
      _store_db_callback(sql);      
      sql = "TRUNCATE `asset_create_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_update_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_update_bitasset_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_update_feed_producers_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_issue_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_reserve_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_fund_fee_pool_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_settle_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_global_settle_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_publish_feed_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_recycle_interest_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `account_authenticate_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `author_create_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `author_update_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `account_config_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_update_gateway_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `committee_member_update_zos_parameters_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `locktoken_create_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `locktoken_create_operation`;";
      _store_db_callback(sql);
       sql = "TRUNCATE `locktoken_update_operation`;";
      _store_db_callback(sql);
       sql = "TRUNCATE `locktoken_remove_operation`;";
      _store_db_callback(sql);
       sql = "TRUNCATE `locktoken_node_operation`;";
      _store_db_callback(sql);
       sql = "TRUNCATE `locktoken_option_operation`;";
      _store_db_callback(sql);
      
 

      sql = "TRUNCATE `account_balance_object`;";
      _store_db_callback(sql);      
      sql = "TRUNCATE `balance_history`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `block_operation`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `proposal_object`;";//11
      _store_db_callback(sql);
      sql = "TRUNCATE `block_transition`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `block_summary_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `account_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `account_statistics_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `balance_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_dynamic_data_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_bitasset_data_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_option_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_order_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_invest_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `committee_member_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `budget_member_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `witness_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `gateway_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `gateway_dynamic_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `carrier_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `carrier_dynamic_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `worker_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `bitlender_paramers_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `witness_schedule_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `special_authority_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `account_transaction_history_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `budget_record_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `vesting_balance_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `dynamic_global_property_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `global_property_object`;";
      _store_db_callback(sql);
      sql = "TRUNCATE `asset_exchange_feed_object`;";
      _store_db_callback(sql);             
      sql = "TRUNCATE `force_settlement_object`;";
      _store_db_callback(sql);         
      sql = "TRUNCATE `limit_order_object`;";
      _store_db_callback(sql);    
      sql = "TRUNCATE `call_order_object`;";
      _store_db_callback(sql);    
      sql = "TRUNCATE `custom_object`;";
      _store_db_callback(sql);    
      sql = "TRUNCATE `operation_history_object`;";
      _store_db_callback(sql);    
      sql = "TRUNCATE `withdraw_permission_object`;";
      _store_db_callback(sql);    
      sql = "TRUNCATE `issue_fundraise_object`;";
      _store_db_callback(sql);    
      sql = "TRUNCATE `buy_fundraise_object`;";
      _store_db_callback(sql);   
      sql = "TRUNCATE `sell_exchange_object`;";
      _store_db_callback(sql);   
      sql = "TRUNCATE `buy_exchange_object`;";
      _store_db_callback(sql);   
      sql = "TRUNCATE `author_object`;";
      _store_db_callback(sql);    
      sql = "TRUNCATE `locktoken_object`;";
      _store_db_callback(sql);    
      sql = "TRUNCATE `locktoken_options_object`;";
      _store_db_callback(sql);    
      sql = "TRUNCATE `transaction_object`;";
      _store_db_callback(sql);    
       sql = "TRUNCATE `blinded_balance_object`;";
      _store_db_callback(sql);  
       sql = "TRUNCATE `chain_property_object`;";
      _store_db_callback(sql);  
       sql = "TRUNCATE `buyback_object`;";
      _store_db_callback(sql);  
       sql = "TRUNCATE `fba_accumulator_object`;";
      _store_db_callback(sql);  
       sql = "TRUNCATE `collateral_bid_object`;";
      _store_db_callback(sql);         
       sql = "TRUNCATE `finance_paramers_object`;";
      _store_db_callback(sql);  

      ilog( "end truncate DBBlock");    
}
void  store_db_plugin_impl::RemveDBBlockNum(uint64_t block_num)
{
      ilog( "start RemveDBBlock #${block}", ("block",block_num) );

      std::string num = fc::to_string(block_num);
       std::string sql = "DELETE  FROM `account_balance_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);
        sql = "DELETE  FROM `block_summary_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);
        sql = "DELETE  FROM `account_object` WHERE `insert_num` >= " + num + "  ;";
        _store_db_callback(sql);
        sql = "DELETE  FROM `account_statistics_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);
        sql = "DELETE  FROM `balance_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_object` WHERE `insert_num` >= " + num + "   ;";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_dynamic_data_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_bitasset_data_object` WHERE `insert_num` >= " + num + "   ;";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_option_object` WHERE `insert_num` >= " + num + "   ;";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_order_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_invest_object` WHERE `insert_num` >= " + num + " ;";        
        _store_db_callback(sql);
        sql = "DELETE  FROM `proposal_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);
         sql = "DELETE  FROM `committee_member_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);
         sql = "DELETE  FROM `budget_member_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);
         sql = "DELETE  FROM `witness_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);
         sql = "DELETE  FROM `gateway_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);
        sql = "DELETE  FROM `gateway_dynamic_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);
         sql = "DELETE  FROM `carrier_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);
        sql = "DELETE  FROM `carrier_dynamic_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);
         sql = "DELETE  FROM `worker_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);    
        sql = "DELETE  FROM `bitlender_paramers_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
          sql = "DELETE  FROM `witness_schedule_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);    
        sql = "DELETE  FROM `special_authority_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `budget_record_object` WHERE `insert_num` >= " + num + " ;";        
        _store_db_callback(sql);   
        sql = "DELETE  FROM `vesting_balance_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `dynamic_global_property_object` WHERE `insert_num` >= " + num + " ;";        
        _store_db_callback(sql);   
        sql = "DELETE  FROM `global_property_object` WHERE `insert_num` >= " + num + " ;";        
        _store_db_callback(sql);      
        sql = "DELETE  FROM `asset_exchange_feed_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);      
        sql = "DELETE  FROM `force_settlement_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `limit_order_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `call_order_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);           
       sql = "DELETE  FROM `custom_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `operation_history_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `withdraw_permission_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `issue_fundraise_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `buy_fundraise_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `sell_exchange_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `buy_exchange_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `author_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `locktoken_options_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `locktoken_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `transaction_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `account_transaction_history_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);   
         sql = "DELETE  FROM `blinded_balance_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);  
        sql = "DELETE  FROM `chain_property_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);          
        sql = "DELETE  FROM `buyback_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);  
        sql = "DELETE  FROM `fba_accumulator_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);          
        sql = "DELETE  FROM `collateral_bid_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);         
        sql = "DELETE  FROM `finance_paramers_object` WHERE `insert_num` >= " + num + " ;";
        _store_db_callback(sql);       


        sql = "DELETE  FROM `account_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `limit_order_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `limit_order_cancel_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `call_order_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql); 
        sql = "DELETE  FROM `fill_order_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);   
        sql = "DELETE  FROM `balance_history` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `block_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `block_transition` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `transfer_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_option_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_option_fee_mode_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql); 
        sql = "DELETE  FROM `bitlender_option_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `gateway_deposit_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `gateway_withdraw_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `gateway_issue_currency_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `proposal_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `proposal_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `proposal_delete_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `withdraw_permission_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `withdraw_permission_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `withdraw_permission_claim_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
         sql = "DELETE  FROM `withdraw_permission_delete_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);      
        sql = "DELETE  FROM `bitlender_option_author_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_option_stop_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
         sql = "DELETE  FROM `bitlender_rate_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_loan_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_invest_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_repay_interest_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_overdue_interest_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_repay_principal_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_overdue_repay_principal_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_add_collateral_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_recycle_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_setautorepayer_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `fill_object_history_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);     
        sql = "DELETE  FROM `bitlender_autorepayment_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_remove_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_squeeze_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_publish_feed_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_update_feed_producers_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_test_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_paramers_update_operation` WHERE `block_num` >= " + num + ";";       
        _store_db_callback(sql);
        sql = "DELETE  FROM `revoke_vesting_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `transfer_vesting_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `budget_member_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `budget_member_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);      
        sql = "DELETE  FROM `gateway_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `carrier_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `witness_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `committee_member_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `carrier_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `gateway_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `committee_member_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `witness_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `withdraw_exchange_fee_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `change_identity_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `account_coupon_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `issue_fundraise_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `issue_fundraise_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `buy_fundraise_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `issue_fundraise_publish_feed_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql); 
        sql = "DELETE  FROM `finance_paramers_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql); 
        sql = "DELETE  FROM `sell_exchange_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql); 
        sql = "DELETE  FROM `sell_exchange_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql); 
        sql = "DELETE  FROM `sell_exchange_remove_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql); 
        sql = "DELETE  FROM `buy_exchange_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql); 
        sql = "DELETE  FROM `buy_exchange_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql); 
        sql = "DELETE  FROM `buy_exchange_remove_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql); 
        sql = "DELETE  FROM `issue_fundraise_remove_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);  
        sql = "DELETE  FROM `asset_property_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_claim_fees_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `fba_distribute_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bid_collateral_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `execute_bid_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_reserve_fees_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql); 
        sql = "DELETE  FROM `custom_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `assert_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql); 
        sql = "DELETE  FROM `balance_claim_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `override_transfer_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `transfer_to_blind_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `blind_transfer_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
         sql = "DELETE  FROM `transfer_from_blind_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_settle_cancel_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `worker_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `vesting_balance_withdraw_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `vesting_balance_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `committee_member_update_global_parameters_operation` WHERE `block_num` >= " + num + ";";        
        _store_db_callback(sql);
        sql = "DELETE  FROM `account_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `account_whitelist_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql); 
        sql = "DELETE  FROM `account_transfer_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);  
        sql = "DELETE  FROM `account_upgrade_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);        
        sql = "DELETE  FROM `asset_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_update_bitasset_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_update_feed_producers_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_issue_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_reserve_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_fund_fee_pool_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_settle_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_global_settle_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_publish_feed_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `bitlender_recycle_interest_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `account_authenticate_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `author_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `author_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `account_config_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `asset_update_gateway_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `committee_member_update_zos_parameters_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `locktoken_create_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);
        sql = "DELETE  FROM `locktoken_update_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);     
       sql = "DELETE  FROM `locktoken_remove_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);       
       sql = "DELETE  FROM `locktoken_node_operation` WHERE `block_num` >= " + num + ";";
        _store_db_callback(sql);     
       sql = "DELETE  FROM `locktoken_option_operation` WHERE `block_num` >= " + num + ";";  

       ilog( "end RemveDBBlock");

}
void store_db_plugin_impl::PrepareDB()
{
      graphene::chain::database &db = _self.database();
      if (block_start > 2)
      {
            RemveDBBlockNum(block_start - 1);
            block_start = block_start - 2;
      }
      else
      {
            block_start = 0;
            TruncateDB();

            const auto dy = db.get_dynamic_global_properties();
            {
                  const auto &idxs = db.get_index_type<account_index>().indices().get<by_id>();
                  for (const auto &a : idxs)
                        Prepare_account_object(a, dy.initial_time, 0, true);
            }
            {
                  const auto &idxs = db.get_index_type<account_balance_index>().indices().get<by_id>();
                  for (const auto &a : idxs)
                        Prepare_account_balance_object(a, dy.initial_time, 0, true);
            }
            {
                  const simple_index<account_statistics_object> &idxs = db.get_index_type<simple_index<account_statistics_object>>();
                  for (const auto &a : idxs)
                        Prepare_account_statistics_object(a, dy.initial_time, 0, true);
            }
            {
                  const auto &idxs = db.get_index_type<vesting_balance_index>().indices().get<by_id>();
                  for (const auto &a : idxs)
                        Prepare_vesting_balance_object(a, dy.initial_time, 0, true);
            }
            {
                  const auto &idxs = db.get_index_type<balance_index>().indices().get<by_id>();
                  for (const auto &a : idxs)
                        Prepare_balance_object(a, dy.initial_time, 0, true);
            }
            {
                  const simple_index<asset_dynamic_data_object> &idxs = db.get_index_type<simple_index<asset_dynamic_data_object>>();
                  for (const auto &a : idxs)
                        Prepare_asset_dynamic_data_object(a, dy.initial_time, 0, true);
            }
            {
                  const auto &idxs = db.get_index_type<asset_bitasset_data_index>().indices().get<by_id>();
                  for (const auto &a : idxs)
                        Prepare_asset_bitasset_data_object(a, dy.initial_time, 0, true);
            }
            {
                  const auto &idxs = db.get_index_type<asset_index>().indices().get<by_id>();
                  for (const auto &a : idxs)
                        Prepare_asset_object(a, dy.initial_time, 0, true);
            }
            {
                  const simple_index<global_property_object> &idxs = db.get_index_type<simple_index<global_property_object>>();
                  for (const auto &a : idxs)
                        Prepare_global_property_object(a, dy.initial_time, 0, true);
            }
            {
                  const simple_index<dynamic_global_property_object> &idxs = db.get_index_type<simple_index<dynamic_global_property_object>>();
                  for (const auto &a : idxs)
                        Prepare_dynamic_global_property_object(a, dy.initial_time, 0, true);
            } 
            {
                  const auto &idxs = db.get_index_type<finance_paramers_index>().indices().get<by_id>();
                  for (const auto &a : idxs)
                        Prepare_finance_paramers_object(a, dy.initial_time, 0, true);
            }
             {
                  const auto &idxs = db.get_index_type<carrier_dynamic_index>().indices().get<by_id>();
                  for (const auto &a : idxs)
                        Prepare_carrier_dynamic_object(a, dy.initial_time, 0, true);
            }
             {
                  const auto &idxs = db.get_index_type<gateway_dynamic_index>().indices().get<by_id>();
                  for (const auto &a : idxs)
                        Prepare_gateway_dynamic_object(a, dy.initial_time, 0, true);
            }
            {
                  const auto &idxs = db.get_index_type<bitlender_paramers_index>().indices().get<by_id>();
                  for (const auto &a : idxs)
                        Prepare_bitlender_paramers_object(a, dy.initial_time, 0, true);
            }

      }
}      
 
 void store_db_plugin_impl::updateBlockOperation(const operation &o_op, const operation_result &op_r,transaction_id_type id,uint64_t num_trx,const uint64_t &num_op,const fc::time_point_sec &time_block, const uint64_t &num_block)
 {       
       int64_t which_op = o_op.which();
       if (which_op == operation::tag<transfer_operation>::value)
             Prepare_transfer_operation(o_op.get<transfer_operation>(), op_r, id, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<limit_order_create_operation>::value)
             Prepare_limit_order_create_operation(o_op.get<limit_order_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<limit_order_cancel_operation>::value)
             Prepare_limit_order_cancel_operation(o_op.get<limit_order_cancel_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<call_order_update_operation>::value)
             Prepare_call_order_update_operation(o_op.get<call_order_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<fill_order_operation>::value)
             Prepare_fill_order_operation(o_op.get<fill_order_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<account_create_operation>::value)
             Prepare_account_create_operation(o_op.get<account_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<account_update_operation>::value)
             Prepare_account_update_operation(o_op.get<account_update_operation>(), op_r, num_trx, num_op, time_block, num_block);       
       else if (which_op == operation::tag<account_config_operation>::value)
             Prepare_account_config_operation(o_op.get<account_config_operation>(), op_r, num_trx, num_op, time_block, num_block);                          
       else if (which_op == operation::tag<account_whitelist_operation>::value)
             Prepare_account_whitelist_operation(o_op.get<account_whitelist_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<account_upgrade_operation>::value)
             Prepare_account_upgrade_operation(o_op.get<account_upgrade_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<account_transfer_operation>::value)
             Prepare_account_transfer_operation(o_op.get<account_transfer_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_create_operation>::value)
             Prepare_asset_create_operation(o_op.get<asset_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_update_operation>::value)
             Prepare_asset_update_operation(o_op.get<asset_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_update_bitasset_operation>::value)
             Prepare_asset_update_bitasset_operation(o_op.get<asset_update_bitasset_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_update_gateway_operation>::value)
             Prepare_asset_update_gateway_operation(o_op.get<asset_update_gateway_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_update_feed_producers_operation>::value)
             Prepare_asset_update_feed_producers_operation(o_op.get<asset_update_feed_producers_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_issue_operation>::value)
             Prepare_asset_issue_operation(o_op.get<asset_issue_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_reserve_operation>::value)
             Prepare_asset_reserve_operation(o_op.get<asset_reserve_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_fund_fee_pool_operation>::value)
             Prepare_asset_fund_fee_pool_operation(o_op.get<asset_fund_fee_pool_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_settle_operation>::value)
             Prepare_asset_settle_operation(o_op.get<asset_settle_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_global_settle_operation>::value)
             Prepare_asset_global_settle_operation(o_op.get<asset_global_settle_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_publish_feed_operation>::value)
             Prepare_asset_publish_feed_operation(o_op.get<asset_publish_feed_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<witness_create_operation>::value)
             Prepare_witness_create_operation(o_op.get<witness_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<witness_update_operation>::value)
             Prepare_witness_update_operation(o_op.get<witness_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<proposal_create_operation>::value)
             Prepare_proposal_create_operation(o_op.get<proposal_create_operation>(), op_r, id,num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<proposal_update_operation>::value)
             Prepare_proposal_update_operation(o_op.get<proposal_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<proposal_delete_operation>::value)
             Prepare_proposal_delete_operation(o_op.get<proposal_delete_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<withdraw_permission_create_operation>::value)
             Prepare_withdraw_permission_create_operation(o_op.get<withdraw_permission_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<withdraw_permission_update_operation>::value)
             Prepare_withdraw_permission_update_operation(o_op.get<withdraw_permission_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<withdraw_permission_claim_operation>::value)
             Prepare_withdraw_permission_claim_operation(o_op.get<withdraw_permission_claim_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<withdraw_permission_delete_operation>::value)
             Prepare_withdraw_permission_delete_operation(o_op.get<withdraw_permission_delete_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<committee_member_create_operation>::value)
             Prepare_committee_member_create_operation(o_op.get<committee_member_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<committee_member_update_operation>::value)
             Prepare_committee_member_update_operation(o_op.get<committee_member_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<committee_member_update_global_parameters_operation>::value)
             Prepare_committee_member_update_global_parameters_operation(o_op.get<committee_member_update_global_parameters_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<vesting_balance_create_operation>::value)
             Prepare_vesting_balance_create_operation(o_op.get<vesting_balance_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<vesting_balance_withdraw_operation>::value)
             Prepare_vesting_balance_withdraw_operation(o_op.get<vesting_balance_withdraw_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<worker_create_operation>::value)
             Prepare_worker_create_operation(o_op.get<worker_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<custom_operation>::value)
             Prepare_custom_operation(o_op.get<custom_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<assert_operation>::value)
             Prepare_assert_operation(o_op.get<assert_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<balance_claim_operation>::value)
             Prepare_balance_claim_operation(o_op.get<balance_claim_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<override_transfer_operation>::value)
             Prepare_override_transfer_operation(o_op.get<override_transfer_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<transfer_to_blind_operation>::value)
             Prepare_transfer_to_blind_operation(o_op.get<transfer_to_blind_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<blind_transfer_operation>::value)
             Prepare_blind_transfer_operation(o_op.get<blind_transfer_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<transfer_from_blind_operation>::value)
             Prepare_transfer_from_blind_operation(o_op.get<transfer_from_blind_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_settle_cancel_operation>::value)
             Prepare_asset_settle_cancel_operation(o_op.get<asset_settle_cancel_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_claim_fees_operation>::value)
             Prepare_asset_claim_fees_operation(o_op.get<asset_claim_fees_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<fba_distribute_operation>::value)
             Prepare_fba_distribute_operation(o_op.get<fba_distribute_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bid_collateral_operation>::value)
             Prepare_bid_collateral_operation(o_op.get<bid_collateral_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<execute_bid_operation>::value)
             Prepare_execute_bid_operation(o_op.get<execute_bid_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_reserve_fees_operation>::value)
             Prepare_asset_reserve_fees_operation(o_op.get<asset_reserve_fees_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<gateway_withdraw_operation>::value)
             Prepare_gateway_withdraw_operation(o_op.get<gateway_withdraw_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<gateway_deposit_operation>::value)
             Prepare_gateway_deposit_operation(o_op.get<gateway_deposit_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<gateway_issue_currency_operation>::value)
             Prepare_gateway_issue_currency_operation(o_op.get<gateway_issue_currency_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_option_stop_operation>::value)
             Prepare_bitlender_option_stop_operation(o_op.get<bitlender_option_stop_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_rate_update_operation>::value)
             Prepare_bitlender_rate_update_operation(o_op.get<bitlender_rate_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<asset_property_operation>::value)
             Prepare_asset_property_operation(o_op.get<asset_property_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_loan_operation>::value)
             Prepare_bitlender_loan_operation(o_op.get<bitlender_loan_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_option_create_operation>::value)
             Prepare_bitlender_option_create_operation(o_op.get<bitlender_option_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_option_author_operation>::value)
             Prepare_bitlender_option_author_operation(o_op.get<bitlender_option_author_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_option_update_operation>::value)
             Prepare_bitlender_option_update_operation(o_op.get<bitlender_option_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
        else if (which_op == operation::tag<bitlender_option_fee_mode_operation>::value)
             Prepare_bitlender_option_fee_mode_operation(o_op.get<bitlender_option_fee_mode_operation>(), op_r, num_trx, num_op, time_block, num_block);      
       else if (which_op == operation::tag<bitlender_invest_operation>::value)
             Prepare_bitlender_invest_operation(o_op.get<bitlender_invest_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_repay_interest_operation>::value)
             Prepare_bitlender_repay_interest_operation(o_op.get<bitlender_repay_interest_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_overdue_interest_operation>::value)
             Prepare_bitlender_overdue_interest_operation(o_op.get<bitlender_overdue_interest_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_recycle_interest_operation>::value)
             Prepare_bitlender_recycle_interest_operation(o_op.get<bitlender_recycle_interest_operation>(), op_r, num_trx, num_op, time_block, num_block);             
       else if (which_op == operation::tag<bitlender_repay_principal_operation>::value)
             Prepare_bitlender_repay_principal_operation(o_op.get<bitlender_repay_principal_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_pre_repay_principal_operation>::value)
             Prepare_bitlender_pre_repay_principal_operation(o_op.get<bitlender_pre_repay_principal_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_overdue_repay_principal_operation>::value)
             Prepare_bitlender_overdue_repay_principal_operation(o_op.get<bitlender_overdue_repay_principal_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_add_collateral_operation>::value)
             Prepare_bitlender_add_collateral_operation(o_op.get<bitlender_add_collateral_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_recycle_operation>::value)
             Prepare_bitlender_recycle_operation(o_op.get<bitlender_recycle_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_setautorepayer_operation>::value)
             Prepare_bitlender_setautorepayer_operation(o_op.get<bitlender_setautorepayer_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<fill_object_history_operation>::value)
             Prepare_fill_object_history_operation(o_op.get<fill_object_history_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag< issue_fundraise_create_operation>::value)
             Prepare_finance_option_create_operation(o_op.get< issue_fundraise_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag< issue_fundraise_update_operation>::value)
             Prepare_issue_fundraise_update_operation(o_op.get< issue_fundraise_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
      else if (which_op == operation::tag< issue_fundraise_remove_operation>::value)
             Prepare_issue_fundraise_remove_operation(o_op.get< issue_fundraise_remove_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag< issue_fundraise_publish_feed_operation>::value)
             Prepare_fundraise_publish_feed_operation(o_op.get< issue_fundraise_publish_feed_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag< finance_paramers_update_operation>::value)
             Prepare_finance_paramers_update_operation(o_op.get< finance_paramers_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
      else if (which_op == operation::tag<sell_exchange_create_operation>::value)
             Prepare_sell_exchange_create_operation(o_op.get< sell_exchange_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
      else if (which_op == operation::tag<sell_exchange_update_operation>::value)
             Prepare_sell_exchange_update_operation(o_op.get< sell_exchange_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
      else if (which_op == operation::tag<sell_exchange_remove_operation>::value)
             Prepare_sell_exchange_remove_operation(o_op.get< sell_exchange_remove_operation>(), op_r, num_trx, num_op, time_block, num_block);
      else if (which_op == operation::tag<buy_exchange_create_operation>::value)
             Prepare_buy_exchange_create_operation(o_op.get< buy_exchange_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
      else if (which_op == operation::tag<buy_exchange_update_operation>::value)
             Prepare_buy_exchange_update_operation(o_op.get< buy_exchange_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
      else if (which_op == operation::tag<buy_exchange_remove_operation>::value)
             Prepare_buy_exchange_remove_operation(o_op.get< buy_exchange_remove_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<buy_fundraise_create_operation>::value)
             Prepare_fundraise_create_operation(o_op.get<buy_fundraise_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<buy_fundraise_enable_operation>::value)
             Prepare_buy_fundraise_enable_operation(o_op.get<buy_fundraise_enable_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<account_coupon_operation>::value)
             Prepare_account_coupon_operation(o_op.get<account_coupon_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<change_identity_operation>::value)
             Prepare_change_identity_operation(o_op.get<change_identity_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_autorepayment_operation>::value)
             Prepare_bitlender_autorepayment_operation(o_op.get<bitlender_autorepayment_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<withdraw_exchange_fee_operation>::value)
             Prepare_withdraw_exchange_fee_operation(o_op.get<withdraw_exchange_fee_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_paramers_update_operation>::value)
             Prepare_bitlender_paramers_update_operation(o_op.get<bitlender_paramers_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<gateway_create_operation>::value)
             Prepare_gateway_create_operation(o_op.get<gateway_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<gateway_update_operation>::value)
             Prepare_gateway_update_operation(o_op.get<gateway_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<carrier_create_operation>::value)
             Prepare_carrier_create_operation(o_op.get<carrier_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<carrier_update_operation>::value)
             Prepare_carrier_update_operation(o_op.get<carrier_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<budget_member_create_operation>::value)
             Prepare_budget_member_create_operation(o_op.get<budget_member_create_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<budget_member_update_operation>::value)
             Prepare_budget_member_update_operation(o_op.get<budget_member_update_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<transfer_vesting_operation>::value)
             Prepare_transfer_vesting_operation(o_op.get<transfer_vesting_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<revoke_vesting_operation>::value)
             Prepare_revoke_vesting_operation(o_op.get<revoke_vesting_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_remove_operation>::value)
             Prepare_bitlender_remove_operation(o_op.get<bitlender_remove_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_squeeze_operation>::value)
             Prepare_bitlender_squeeze_operation(o_op.get<bitlender_squeeze_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_publish_feed_operation>::value)
             Prepare_bitlender_publish_feed_operation(o_op.get<bitlender_publish_feed_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_update_feed_producers_operation>::value)
             Prepare_bitlender_update_feed_producers_operation(o_op.get<bitlender_update_feed_producers_operation>(), op_r, num_trx, num_op, time_block, num_block);
       else if (which_op == operation::tag<bitlender_test_operation>::value)
             Prepare_bitlender_test_operation(o_op.get<bitlender_test_operation>(), op_r, num_trx, num_op, time_block, num_block);    
       else if (which_op == operation::tag<account_authenticate_operation>::value)
            Prepare_account_authenticate_operation(o_op.get<account_authenticate_operation>(), op_r, num_trx, num_op, time_block, num_block);  
       else if (which_op == operation::tag<author_create_operation>::value)
            Prepare_author_create_operation(o_op.get<author_create_operation>(), op_r, num_trx, num_op, time_block, num_block);  
       else if (which_op == operation::tag<author_update_operation>::value)
            Prepare_author_update_operation(o_op.get<author_update_operation>(), op_r, num_trx, num_op, time_block, num_block);            
       else if (which_op == operation::tag<committee_member_update_zos_parameters_operation>::value)
            Prepare_committee_member_update_zos_parameters_operation(o_op.get<committee_member_update_zos_parameters_operation>(), op_r, num_trx, num_op, time_block, num_block);              
       else if (which_op == operation::tag<locktoken_create_operation>::value)
            Prepare_locktoken_create_operation(o_op.get<locktoken_create_operation>(), op_r, num_trx, num_op, time_block, num_block);             
       else if (which_op == operation::tag<locktoken_update_operation>::value)
            Prepare_locktoken_update_operation(o_op.get<locktoken_update_operation>(), op_r, num_trx, num_op, time_block, num_block);              
       else if (which_op == operation::tag<locktoken_remove_operation>::value)
            Prepare_locktoken_remove_operation(o_op.get<locktoken_remove_operation>(), op_r, num_trx, num_op, time_block, num_block);              
       else if (which_op == operation::tag<locktoken_node_operation>::value)
           Prepare_locktoken_node_operation(o_op.get<locktoken_node_operation>(), op_r, num_trx, num_op, time_block, num_block);             
       else if (which_op == operation::tag<locktoken_option_operation>::value)
           Prepare_locktoken_option_operation(o_op.get<locktoken_option_operation>(), op_r, num_trx, num_op, time_block, num_block);             
       else
       {
             elog("updateBlockOperation : ${x}  ", ("x", o_op.which()));
             assert(0);
       }        
 }

//保存操作记录
void store_db_plugin_impl::updateBlockOperation(const signed_block &b)
{
      try
      {
            graphene::chain::database &db = _self.database();
            uint64_t num_block = db.head_block_num();
            fc::time_point_sec time_block = db.head_block_time();

            if (block_start >= num_block)
                  return;
            uint64_t num_trx = 0;

            if(block_apply >= num_block)
                  RemveDBBlockNum(num_block);
            block_apply = num_block;

            for (const auto &o_tr : b.transactions)
            {
                  uint64_t num_op = 0;
                  transaction_id_type trx_id = o_tr.id();
                  Prepare_base_transation(o_tr,trx_id, num_trx, time_block, num_block);                 
                  for (const auto &o_op : o_tr.operations)
                  {
                     Prepare_base_operation(o_op, o_tr.operation_results[num_op],trx_id, num_trx, num_op, time_block, num_block);
                     updateBlockOperation(o_op, o_tr.operation_results[num_op],trx_id, num_trx, num_op, time_block, num_block);
                     num_op++;
                  }
                  num_trx++;
            }
      }
      catch (const fc::exception &e)
      {
            elog(e.to_string());
      }
}

//保存用户账户资产变动历史记录
void  store_db_plugin_impl::updateAccountBalanceHistory( const signed_block& b )
{
    try{
    graphene::chain::database &db = _self.database(); 
    uint64_t num_block = db.head_block_num();
    fc::time_point_sec time_block = db.head_block_time();

    if(block_start>=num_block)
     return;

    const vector<balance_history_> &hist = db.get_applied_balances();
    for( const auto&o_op : hist )
    {  
      std::string info = o_op.info.substr(0,199);      
      std::string sinsert =  "INSERT INTO `balance_history` (\
      `account_id`, \
      `asset_id`,\
      `utype`,\
      `info`,\
      `amount`,\
      `balance`,\
      `block_num`,\
      `block_time` ) \
       VALUES ("\
      + fc::to_string(o_op.issuer.get_instance())+"," \
      + fc::to_string(o_op.asset_op.asset_id.get_instance())+"," \
      + fc::to_string(o_op.utype)+",'" \
      + info+"'," \
      + fc::to_string(o_op.asset_op.amount.value)+"," \
      + fc::to_string(o_op.balance.value)+"," \
      + fc::to_string(num_block)+",'" \
      + time_block.to_db_string()+"');";

       
      _store_db_callback(sinsert);
    }    
   }catch(const fc::exception& e) 
  {
    elog(e.to_string());
  } 
}
//保存内存状态
void store_db_plugin_impl::updateObjectDel(const vector<object_id_type> &ids)
{
      graphene::chain::database &db = _self.database();

      const fc::time_point_sec block_time = db.head_block_time();
      const uint64_t block_number = db.head_block_num();

      if (block_start >= block_number)
            return;

      try
      {
            for (auto const &value : ids)
            {
                  if (value.space() == protocol_ids && value.type() == balance_object_type)
                  {
                        balance_object p;
                        p.id = value;
                        Prepare_balance_object(p, block_time, block_number, 2);
                  }
                  else if (value.space() == implementation_ids && value.type() == impl_transaction_object_type)
                  {
                       transaction_object p;
                       p.id = value;
                       Prepare_transaction_object(p, block_time, block_number, 2);
                  }
                  else if (value.space() == protocol_ids && value.type() == proposal_object_type)
                  {
                        proposal_object p;
                        p.id = value;
                        Prepare_proposal_object(p, block_time, block_number, 2);
                  }
                  else if (value.space() == protocol_ids && value.type() == worker_object_type)
                  {
                        worker_object p;
                        p.id = value;
                        Prepare_worker_object(p, block_time, block_number, 2);
                  }
                  else if (value.space() == protocol_ids && value.type() == withdraw_permission_object_type)
                  {
                        withdraw_permission_object p;
                        p.id = value;
                        Prepare_withdraw_permission_object(p, block_time, block_number, 2);
                  }
                  
                  else if (value.space() == protocol_ids && value.type() == sell_exchange_object_type)
                  {
                        sell_exchange_object p;
                        p.id = value;
                        Prepare_sell_exchange_object(p, block_time, block_number, 2);
                  }
                  else if (value.space() == protocol_ids && value.type() == buy_exchange_object_type)
                  {
                        buy_exchange_object p;
                        p.id = value;
                        Prepare_buy_exchange_object(p, block_time, block_number, 2);
                  }       
                  else if (value.space() == protocol_ids && value.type() == limit_order_object_type)
                  {
                        limit_order_object p;
                        p.id = value;
                        Prepare_limit_order_object(p, block_time, block_number, 2);
                  }           
                  
                  else if (value.space() == protocol_ids && value.type() == call_order_object_type)
                  {
                        call_order_object p;
                        p.id = value;
                        Prepare_call_order_object(p, block_time, block_number, 2);
                  }   
                  else if (value.space() == protocol_ids && value.type() == bitlender_order_object_type)
                  {
                        bitlender_order_object p;
                        p.id = value;
                        Prepare_bitlender_order_object(p, block_time, block_number, 2);
                  }                 
                  else if (value.space() == implementation_ids && value.type() == impl_bitlender_invest_object_type)
                  {
                        bitlender_invest_object p;
                        p.id = value;
                        Prepare_bitlender_invest_object(p, block_time, block_number, 2);
                  }
                   else if (value.space() == protocol_ids && value.type() == vesting_balance_object_type)
                  {
                        vesting_balance_object p;
                        p.id = value;
                        Prepare_vesting_balance_object(p, block_time, block_number, 2);
                  }     
                   else if (value.space() == protocol_ids && value.type() == issue_fundraise_object_type)
                  {
                        issue_fundraise_object p;
                        p.id = value;
                        Prepare_issue_fundraise_object(p, block_time, block_number, 2);
                  } 
                  else if (value.space() == protocol_ids && value.type() == buy_fundraise_object_type)
                  {
                        buy_fundraise_object p;
                        p.id = value;
                        Prepare_buy_fundraise_object(p, block_time, block_number, 2);
                  } 
                  else if (value.space() == protocol_ids && value.type() == locktoken_object_type)
                  {
                        locktoken_object p;
                        p.id = value;
                        Prepare_locktoken_object(p, block_time, block_number, 2);
                  } 
                  else  
                  { 
                     elog("updateObjectDel : ${x} ${y} ",("x", value.space())("y", value.type()));
                     assert(0);
                  }

                  
            }
      }
      catch (const fc::exception &e)
      {
            elog(e.to_string());
      }
}
void store_db_plugin_impl::updateObjectChange(const vector<object_id_type> &ids, bool isNew) 
{
      graphene::chain::database &db = _self.database();

      const fc::time_point_sec block_time = db.head_block_time();
      const uint64_t block_number = db.head_block_num();

      if (block_start >= block_number)
            return;

      try
      {

            for (auto  &value : ids)
            {
                  if (value.is<account_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const account_object *>(obj);
                        if (p != nullptr)
                              Prepare_account_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<asset_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const asset_object *>(obj);
                        if (p != nullptr)
                              Prepare_asset_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<bitlender_option_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const bitlender_option_object *>(obj);
                        if (p != nullptr)
                              Prepare_bitlender_option_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<issue_fundraise_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const issue_fundraise_object *>(obj);
                        if (p != nullptr)
                              Prepare_issue_fundraise_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<force_settlement_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const force_settlement_object *>(obj);
                        if (p != nullptr)
                              Prepare_force_settlement_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<committee_member_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const committee_member_object *>(obj);
                        if (p != nullptr)
                              Prepare_committee_member_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<budget_member_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const budget_member_object *>(obj);
                        if (p != nullptr)
                              Prepare_budget_member_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<witness_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const witness_object *>(obj);
                        if (p != nullptr)
                              Prepare_witness_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<gateway_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const gateway_object *>(obj);
                        if (p != nullptr)
                              Prepare_gateway_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<gateway_dynamic_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const gateway_dynamic_object *>(obj);
                        if (p != nullptr)
                              Prepare_gateway_dynamic_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<carrier_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const carrier_object *>(obj);
                        if (p != nullptr)
                              Prepare_carrier_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<limit_order_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const limit_order_object *>(obj);
                        if (p != nullptr)
                              Prepare_limit_order_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<call_order_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const call_order_object *>(obj);
                        if (p != nullptr)
                              Prepare_call_order_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<bitlender_order_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const bitlender_order_object *>(obj);
                        if (p != nullptr)
                              Prepare_bitlender_order_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<buy_fundraise_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const buy_fundraise_object *>(obj);
                        if (p != nullptr)
                              Prepare_buy_fundraise_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<sell_exchange_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const sell_exchange_object *>(obj);
                        if (p != nullptr)
                              Prepare_sell_exchange_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<buy_exchange_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const buy_exchange_object *>(obj);
                        if (p != nullptr)
                              Prepare_buy_exchange_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }

                  else if (value.is<proposal_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const proposal_object *>(obj);
                        if (p != nullptr)
                              Prepare_proposal_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<operation_history_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const operation_history_object *>(obj);
                        if (p != nullptr)
                              Prepare_operation_history_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<withdraw_permission_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const withdraw_permission_object *>(obj);
                        if (p != nullptr)
                              Prepare_withdraw_permission_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<vesting_balance_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const vesting_balance_object *>(obj);
                        if (p != nullptr)
                              Prepare_vesting_balance_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<worker_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const worker_object *>(obj);
                        if (p != nullptr)
                              Prepare_worker_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<balance_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const balance_object *>(obj);
                        if (p != nullptr)
                              Prepare_balance_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<account_balance_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const account_balance_object *>(obj);
                        if (p != nullptr)
                              Prepare_account_balance_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  
                  else if (value.is<global_property_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const global_property_object *>(obj);
                        if (p != nullptr)
                              Prepare_global_property_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<dynamic_global_property_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const dynamic_global_property_object *>(obj);
                        if (p != nullptr)
                              Prepare_dynamic_global_property_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<asset_dynamic_data_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const asset_dynamic_data_object *>(obj);
                        if (p != nullptr)
                              Prepare_asset_dynamic_data_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<asset_bitasset_data_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const asset_bitasset_data_object *>(obj);
                        if (p != nullptr)
                              Prepare_asset_bitasset_data_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<asset_exchange_feed_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const asset_exchange_feed_object *>(obj);
                        if (p != nullptr)
                              Prepare_asset_exchange_feed_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<account_statistics_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const account_statistics_object *>(obj);
                        if (p != nullptr)
                              Prepare_account_statistics_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<transaction_object>())
                  {
                       /* auto obj = db.find_object(value);
                        auto p = static_cast<const transaction_object *>(obj);
                        if (p != nullptr)
                              Prepare_transaction_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);*/
                  }
                  else if (value.is<block_summary_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const block_summary_object *>(obj);
                        if (p != nullptr)
                              Prepare_block_summary_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  // else if (value.is<account_transaction_history_object>())
                  // {
                  //       auto obj = db.find_object(value);
                  //       auto p = static_cast<const account_transaction_history_object *>(obj);
                  //       if (p != nullptr)
                  //             Prepare_account_transaction_history_object(*p, block_time, block_number, isNew);
                  //       else
                  //             assert(0);
                  // }
                  /*else if (value.is<balance_history_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const balance_history_object *>(obj);
                        if (p != nullptr)
                              Prepare_balance_history_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }*/
                  else if (value.is<chain_property_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const chain_property_object *>(obj);
                        if (p != nullptr)
                              Prepare_chain_property_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<witness_schedule_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const witness_schedule_object *>(obj);
                        if (p != nullptr)
                              Prepare_witness_schedule_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<budget_record_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const budget_record_object *>(obj);
                        if (p != nullptr)
                              Prepare_budget_record_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<blinded_balance_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const blinded_balance_object *>(obj);
                        if (p != nullptr)
                              Prepare_blinded_balance_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<special_authority_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const special_authority_object *>(obj);
                        if (p != nullptr)
                              Prepare_special_authority_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<buyback_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const buyback_object *>(obj);
                        if (p != nullptr)
                              Prepare_buyback_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<fba_accumulator_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const fba_accumulator_object *>(obj);
                        if (p != nullptr)
                              Prepare_fba_accumulator_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<collateral_bid_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const collateral_bid_object *>(obj);
                        if (p != nullptr)
                              Prepare_collateral_bid_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<bitlender_invest_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const bitlender_invest_object *>(obj);
                        if (p != nullptr)
                              Prepare_bitlender_invest_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<bitlender_paramers_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const bitlender_paramers_object *>(obj);
                        if (p != nullptr)
                              Prepare_bitlender_paramers_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<carrier_dynamic_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const carrier_dynamic_object *>(obj);
                        if (p != nullptr)
                              Prepare_carrier_dynamic_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<author_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const author_object *>(obj);
                        if (p != nullptr)
                              Prepare_author_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<locktoken_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const locktoken_object *>(obj);
                        if (p != nullptr)
                              Prepare_locktoken_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<locktoken_option_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const locktoken_option_object *>(obj);
                        if (p != nullptr)
                              Prepare_locktoken_options_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }                                             
                  else if (value.is<finance_paramers_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const finance_paramers_object *>(obj);
                        if (p != nullptr)
                              Prepare_finance_paramers_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  // else if (value.is<bitlender_history_object>())
                  // {
                  //       auto obj = db.find_object(value);
                  //       auto p = static_cast<const bitlender_history_object *>(obj);
                  //       if (p != nullptr)
                  //             Prepare_bitlender_history_object(*p, block_time, block_number, isNew);
                  //       else
                  //             assert(0);
                  // }
                  // else if (value.is<bucket_object>())
                  // {
                  //       auto obj = db.find_object(value);
                  //       auto p = static_cast<const bucket_object *>(obj);
                  //       if (p != nullptr)
                  //             Prepare_bucket_object(*p, block_time, block_number, isNew);
                  //       else
                  //             assert(0);
                  // }

                  // else if (value.is<market_ticker_meta_object>())
                  // {
                  //       auto obj = db.find_object(value);
                  //       auto p = static_cast<const market_ticker_meta_object *>(obj);
                  //       if (p != nullptr)
                  //             Prepare_market_ticker_meta_object(*p, block_time, block_number, isNew);
                  //       else
                  //             assert(0);
                  // }

                  /*else if (value.is<order_history_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const order_history_object *>(obj);
                        if (p != nullptr)
                              Prepare_order_history_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }
                  else if (value.is<market_ticker_object>())
                  {
                        auto obj = db.find_object(value);
                        auto p = static_cast<const market_ticker_object *>(obj);
                        if (p != nullptr)
                              Prepare_market_ticker_object(*p, block_time, block_number, isNew);
                        else
                              assert(0);
                  }*/
                  else
                  { 
                        elog("updateObjectChange : ${x} ${y} ",("x", value.space())("y", value.type()));
                        assert(0);
                  }
            }
      }
      catch (const fc::exception &e)
      {
            elog(e.to_string());
      }
}

store_db_plugin_impl::~store_db_plugin_impl()
{
   return;
}


} // end namespace detail

store_db_plugin::store_db_plugin() :
   my( new detail::store_db_plugin_impl(*this) )
{
}

store_db_plugin::~store_db_plugin()
{
}

std::string store_db_plugin::plugin_name()const
{
   return "store_db";
}
std::string store_db_plugin::plugin_description()const
{
   return "Stores blockchain  in   database.";
}

void store_db_plugin::plugin_set_program_options(
   boost::program_options::options_description& cli,
   boost::program_options::options_description& cfg
   )
{
   cli.add_options()         
         ("store_db_trx", boost::program_options::value<bool>()->implicit_value(false), "store_db trx flag")
         ("store_db_object", boost::program_options::value<bool>()->implicit_value(false), "store_db object flag")  
         ;
   cfg.add(cli);
} 
void store_db_plugin::get_info(fc::mutable_variant_object &result) 
{
      result["store-db-url"] = my->db_url;
      result["store-db-user"] = my->db_user;
      result["store-db-pwd"] = my->db_pwd;
      result["store-db-name"] = my->db_name;
      result["store-db-port"] = my->db_port;
      result["store_db_trx"] = my->store_db_trx;
      result["store_db_object"] = my->store_db_object;
}
void store_db_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{  
   my->_store_db_callback = database()._store_db_callback;
   my->_store_db_start    = database()._store_db_start;   
   FC_ASSERT(my->_store_db_callback, "no store db callback");
   if(my->_store_db_start)
     my->block_start = my->_store_db_start();

   if (options.count("store-db-url"))
   {
         my->db_url = options["store-db-url"].as<std::string>();
   }
   if (options.count("store-db-user")) {
      my->db_user = options["store-db-user"].as<std::string>();
   }
   if (options.count("store-db-pwd")) {
      my->db_pwd = options["store-db-pwd"].as<std::string>();
   }
   if (options.count("store-db-port")) {
      my->db_port = options["store-db-port"].as<int>();
   }
   if (options.count("store-db-name")) {       
      my->db_name = options["store-db-name"].as<std::string>();
   }  
   if (options.count("store_db_trx")) {
      my->store_db_trx = options["store_db_trx"].as<bool>();
   }
   if (options.count("store_db_object")) {
      my->store_db_object = options["store_db_object"].as<bool>();
   }
   if(my->store_db_object)
   {
    database().removed_objects.connect([&](const vector<object_id_type>& ids, const vector<const object*>& ob, const flat_set<account_id_type>& impacted_accounts){ my->updateObjectDel(ids); });         
    database().new_objects.connect([&]( const vector<object_id_type>& ids, const flat_set<account_id_type>& impacted_accounts ){ my->updateObjectChange(ids, 1); });
    database().changed_objects.connect([&]( const vector<object_id_type>& ids, const flat_set<account_id_type>& impacted_accounts ){ my->updateObjectChange(ids, 0); });    
    database().applied_block.connect([this](const signed_block &b) { my->updateAccountBalanceHistory(b); });
   }
   if(my->store_db_trx)
   {
     database().applied_block.connect([this](const signed_block &b) { my->updateBlockOperation(b); });
   }
   database()._app_status.has_store_history_plugin = true;
}

void store_db_plugin::plugin_startup()
{
   ilog("store_db : plugin_startup() begin");

   my->PrepareDB();
}

} }