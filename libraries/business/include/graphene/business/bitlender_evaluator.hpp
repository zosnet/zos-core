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
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/protocol/types.hpp>

namespace graphene { namespace chain {

   class  account_object;
   class  carrier_object;
   class  asset_object;
   class  asset_exchange_feed_object;
   class  asset_bitasset_data_object;
   struct bitlender_loan_operation;
   struct bitlender_squeeze_operation;
   struct bitlender_remove_operation;
   struct bitlender_invest_operation;
   struct bitlender_repay_interest_operation;
   struct bitlender_overdue_interest_operation;
   struct bitlender_recycle_interest_operation;
   struct bitlender_repay_principal_operation;
   struct bitlender_pre_repay_principal_operation;
   struct bitlender_setautorepayer_operation;
   struct bitlender_autorepayment_operation;
   struct bitlender_overdue_repay_principal_operation;
   struct bitlender_overdue_repay_principal_operation;

   struct bitlender_test_operation;

 


   class bitlender_calc  
   {
      public:
         //取得参数列表
        static const bitlender_option_object &get_option(const database &d, const asset_id_type asset) ;
        //取得喂价
        static const asset_exchange_feed_object      &get_feeds(const database &d, const asset_id_type base, const asset_id_type quote,const bool bcheck);
        //计算某月利息
        static asset                   get_repay_interest_rate(const database &d, const bitlender_order_object &order, uint32_t loan_period);
        //计算抵押的风险保证金
        static asset                   get_risk_collateralize_by_loan(const database &d, const asset &loan, const asset &collateralize,const fee_mode_type fee_mode,const bitlender_key &type);
        //计算抵押的总费用
        static asset                   get_fee_collateralize_by_loan(const database &d, const asset &loan, const asset &collateralize,const fee_mode_type fee_mode,const bitlender_key &type);
        //根据借款，计算最小抵押币
        static asset                   get_min_collateralize_by_loan(const database &d, const asset &loan, const asset &collateralize);
        static asset                   get_user_collateralize_by_loan(const database &d, const asset &loan, const asset &collateralize, uint16_t collateral_ratio);
        //计算运营商法币费用
        static asset                   get_carrier_fee(const database &d, const asset &loan, const asset &collateralize,const bitlender_key &type);
        //根据借款，计算强制抵押币
        static asset                   get_short_squeeze_by_loan(const database &d, const asset &loan, const asset &collateralize);        
        //重新激活进入平仓的单子
        static void                    revive_squeeze_order(database &d, const bitlender_order_object &order);
        //检查所有本资产的单子是否到达强制平仓线
        static void                    check_bitlender_orders(database &d,const asset_id_type &base_check,const asset_id_type &quote_check);
        //检查某个单子是否达到平仓线
        static void                    check_bitlender_order(database &d, const bitlender_order_object &order, vector<bitlender_order_id_type> &aid, vector<bitlender_order_id_type> &arm);

        static void                    check_bitlender_order(database &d, const bitlender_order_object &order);
        //单子是否达到平仓
        static bool                    is_short_squeeze(const database &d, const bitlender_order_object &order,const bool bcheck);
        //单子是否达到最小抵押
        static bool                    is_min_collateralize(const database &d, const bitlender_order_object &order,const bool bcheck);
        static bool                    is_user_collateralize(const database &d,const bitlender_order_object &order);
        //根据投资总额
        static asset                   get_invest_total(const database &d, const bitlender_order_object &order);
        //提前还款费用
        static asset                   get_pre_repay_principal_fee(const database &d, const bitlender_order_object &order);
        //逾期还款费用
        static asset                   get_overdue_repay_principal_fee(const database &d, const bitlender_order_object &order);
        //逾期利息罚款
        static asset                   get_overdue_interest_fee( const database& d,const bitlender_order_object &order, uint32_t loan_period);
        //取得最后还利息的期数
        static void                     get_notify_check_time(  database& d, bitlender_order_object &order);
        //是否回收
        static bool                     is_recycle_principal(const database &d, const bitlender_order_object &order);
        //取得利息逾期次数
        static uint32_t                 get_interest_overdue_periods( const database& d,const bitlender_order_object &order);
        //计算订单的所有利息
        static void                     get_repay_interest_data(const bitlender_option & option,bitlender_order_object &order,time_point_sec &invest_finish_time);
        //通知历史记录统计
        static void                     fill_loan_operation_finish( database &d, const bitlender_order_object &order);
        //计算是否最后一期
        static bool                     get_is_last_period(const database &d, const bitlender_order_object &order);
        //计算是否最后一月
        static bool                     get_is_last_month(const database &d, const bitlender_order_object &order);
        //填充还利息数据
        static void                     fill_invester( database &d, const bitlender_order_object &order, share_type irate, share_type ifee, uint32_t loan_period,account_id_type pay_id,bool bcheck);
        //检查是否利息还完成了
        static void                     fill_invester_finish(database &d, const bitlender_order_object &order);
        //设置本金结束
        static void                     fill_repay_principal_finish(database &d, const bitlender_order_object &order, share_type ifee, uint32_t ufininshstatue);
        //提前欢利息
        static asset                    fill_pre_interest( database& d, time_point_sec now,const bitlender_order_object &order,bool do_balance = true);
        //判断前面的利息是否还了
        static bool                     get_pre_inverest_finish(const database &d, const bitlender_order_object &order, uint32_t uperiod);
        //取得自动还款期数
        static int                      get_autointerest_period(fc::time_point_sec &now,const bitlender_order_object &order);
        //计算分配金额
        static void                     get_precent_value(share_type amount, vector<share_type> &base, vector<share_type> &ret);
        static asset                    get_recycle_collateralize_by_loan(const database &d, const asset &loan, const asset &collateralize);
        static asset                    get_recycle_loan_by_collateralize(const database &d, const asset &loan, const asset &collateralize);

        static int32_t                 get_can_loan(const database &d,const carrier_object & ca ,const account_object &a,const asset_id_type &aid);
        static int32_t                 get_can_invest(const database &d,const carrier_object & ca ,const account_object &a,const asset_id_type &aid);
        static int32_t                 get_can_author(const database &d, const carrier_object &ca, const account_object &a, const asset_id_type &aid);
        static int32_t                 get_can_node(const database &d,const carrier_object & ca ,const account_object &a,const asset_id_type &aid);
        static int32_t                 get_can_dyfixed(const database &d,const carrier_object & ca ,const account_object &a,const asset_id_type &aid);


        static void check_interest_overdue(database &d, bitlender_order_object &order);
   };

   class bitlender_loan_evaluator : public evaluator<bitlender_loan_evaluator>
   {
      public:
         typedef bitlender_loan_operation operation_type;

         void_result do_evaluate( const bitlender_loan_operation& o );
         object_id_type do_apply( const bitlender_loan_operation& o );
      private:
         carrier_id_type  carrier_id;
         bitlender_key    key;
   };

   class bitlender_invest_evaluator : public evaluator<bitlender_invest_evaluator>
   {
      public:
         typedef bitlender_invest_operation operation_type;

         void_result do_evaluate( const bitlender_invest_operation& o );
         object_id_type do_apply( const bitlender_invest_operation& o );

      private:
        fc::time_point _timer;   
        carrier_id_type  carrier_id;

      
 
   };

   class bitlender_repay_interest_evaluator : public evaluator<bitlender_repay_interest_evaluator>
   {
      public:
         typedef bitlender_repay_interest_operation operation_type;

         void_result do_evaluate( const bitlender_repay_interest_operation& o );
         void_result do_apply( const bitlender_repay_interest_operation& o );

   
         
   };

   class bitlender_overdue_interest_evaluator : public evaluator<bitlender_overdue_interest_evaluator>
   {
      public:
         typedef bitlender_overdue_interest_operation operation_type;

         void_result do_evaluate( const bitlender_overdue_interest_operation& o );
         void_result do_apply( const bitlender_overdue_interest_operation& o );    
      
     

   };
   class bitlender_recycle_interest_evaluator : public evaluator<bitlender_recycle_interest_evaluator>
   {
      public:
         typedef bitlender_recycle_interest_operation operation_type;

         void_result do_evaluate( const bitlender_recycle_interest_operation& o );
         void_result do_apply( const bitlender_recycle_interest_operation& o );    
     
      private:
         carrier_id_type recycle_id;
         account_id_type recycle;
   };



    

   class bitlender_repay_principal_evaluator : public evaluator<bitlender_repay_principal_evaluator>
   {
      public:
         typedef bitlender_repay_principal_operation operation_type;

         void_result do_evaluate( const bitlender_repay_principal_operation& o );
         void_result do_apply( const bitlender_repay_principal_operation& o );

         
         
   };

   class bitlender_pre_repay_principal_evaluator : public evaluator<bitlender_pre_repay_principal_evaluator>
   {
      public:
         typedef bitlender_pre_repay_principal_operation operation_type;

         void_result do_evaluate( const bitlender_pre_repay_principal_operation& o );
         void_result do_apply( const bitlender_pre_repay_principal_operation& o );

     
         
   };



   class bitlender_overdue_repay_principal_evaluator : public evaluator<bitlender_overdue_repay_principal_evaluator>
   {
      public:
         typedef bitlender_overdue_repay_principal_operation operation_type;

         void_result do_evaluate( const bitlender_overdue_repay_principal_operation& o );
         void_result do_apply( const bitlender_overdue_repay_principal_operation& o );

      
   };
   class bitlender_add_collateral_evaluator : public evaluator<bitlender_add_collateral_evaluator>
   {
      public:
         typedef bitlender_add_collateral_operation operation_type;

         void_result do_evaluate( const bitlender_add_collateral_operation& o );
         void_result do_apply( const bitlender_add_collateral_operation& o );      

         
   };
   class bitlender_recycle_evaluator : public evaluator<bitlender_recycle_evaluator>
   {
      public:
         typedef bitlender_recycle_operation operation_type;

         void_result do_evaluate( const bitlender_recycle_operation& o );
         void_result do_apply( const bitlender_recycle_operation& o );

      private:
         carrier_id_type recycle_id;
         account_id_type recycle;

         asset           amount_to_collateralize;
         asset           amount_to_loan;
         asset           pay;
   };

   class bitlender_setautorepayer_evaluator : public evaluator<bitlender_setautorepayer_evaluator>
   {
      public:
         typedef bitlender_setautorepayer_operation operation_type;

         void_result do_evaluate( const bitlender_setautorepayer_operation& o );
         void_result do_apply( const bitlender_setautorepayer_operation& o );         
         
   };  
  class bitlender_autorepayment_evaluator : public evaluator<bitlender_autorepayment_evaluator>
   {
      public:
         typedef bitlender_autorepayment_operation operation_type;

         void_result do_evaluate( const bitlender_autorepayment_operation& o );
         void_result do_apply( const bitlender_autorepayment_operation& o );

         
         
   };  

   class bitlender_remove_evaluator : public evaluator<bitlender_remove_evaluator>
   {
      public:
         typedef bitlender_remove_operation operation_type;

         void_result do_evaluate( const bitlender_remove_operation& o );
         void_result do_apply( const bitlender_remove_operation& o );

          
   };  


   class bitlender_test_evaluator : public evaluator<bitlender_test_evaluator>
   {
      public:
         typedef bitlender_test_operation operation_type;

         void_result do_evaluate( const bitlender_test_operation& o );
         void_result do_apply( const bitlender_test_operation& o );


       
   };

} } // graphene::chain
