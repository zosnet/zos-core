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

#include <graphene/chain/protocol/asset.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/db/object.hpp>

#include <boost/multi_index/composite_key.hpp>
 

namespace graphene { namespace chain {

using namespace graphene::db;

#define INTEREST_NORMAL         1  //未还息
#define INTEREST_FINISH         2  //已还息
#define INTEREST_OVERDUE        3  //逾期
#define INTEREST_OVERDUEFINISH  4  //逾期还息
#define INTEREST_PREFINISH      5  //提前还本
#define INTEREST_RECYCLE_FINISH 6  //处理不良资产
//还息记录
struct bitlender_repay_interest
{  
 public: 
  time_point_sec  finish_time     = time_point_sec::maximum();             //还息时间 
  asset           amount_repay_interest;   //实际利息
  asset           fines_repay_interest;    //实际罚款
};
//预计还息记录
struct bitlender_order_interest  
{  
public:
  account_id_type pay_id;                    //还息人
  uint32_t        interest_state  = 0;       //状态
  time_point_sec  expect_time     = time_point_sec::maximum();            //还息预计时间      
  time_point_sec  finish_time     = time_point_sec::maximum();            //实际还息时间 
  asset           expect_repay_interest;    //预计利息
  asset           amount_repay_interest;    //实际利息
  asset           fines_repay_interest;     //实际罚款
  bitlender_rate  interest_rate;            //利率
  uint32_t        ustate         = 0;       //状态
public:
  bool is_normal()  const {return  interest_state == INTEREST_NORMAL;} 
  bool is_finish()  const {return  interest_state == INTEREST_FINISH || interest_state == INTEREST_OVERDUEFINISH || interest_state ==INTEREST_PREFINISH;}
  bool is_overdue() const {return  interest_state == INTEREST_OVERDUE;}
  asset get_overdue_fee(const fc::time_point_sec &now, const bitlender_option_object &option,const bitlender_key &type) const ;  
  asset get_pre_interest_fee(const fc::time_point_sec &now, const bitlender_option_object &option,const bitlender_key &type) const ;
};
 
struct  bitlender_invest_order
{  
 public: 

  account_id_type account;
  share_type      invest;
  account_id_type carrier;
  fc::time_point  invest_time;
};
struct  bitlender_invest_order_
{  
 public:
   string          name;
   account_id_type account;
   share_type      invest;
   string          carrier_name;
   account_id_type carrier;
   fc::time_point invest_time;
};

#define  bitlender_invest_autopay  0x00000001
 



struct bitlender_order_info
{    

   bitlender_order_id_type id;

   account_info     issuer;

   account_id_type  carrier;          //运营商账户ID
   carrier_id_type  carrier_id;       //运营商ID   

   asset            amount_to_loan;   
   asset_info       asset_to_loan;

   asset            amount_to_collateralize;     
   asset_info       asset_to_collateralize;     

   flat_map<uint32_t,bitlender_order_interest> repay_interest; //还利息记录

   vector<bitlender_invest_id_type> interest_book;

   bitlender_key   repayment_type;       //借款类型 

   uint32_t        loan_period;             //借款周期

   time_point_sec  loan_time;
   time_point_sec  expiration_time;
   time_point_sec  invest_finish_time;
   time_point_sec  repay_principal_time;
   time_point_sec  expect_principal_time;
   time_point_sec notify_time;


   asset           collateralize_fee;        //抵押币费用 
   asset           collateralize_risk;       //抵押币风险保证金
   asset           carrier_fee;              //运营商费用 
   asset           repay_principal_fee;   //提前还款费用    


   bitlender_rate  interest_rate;          
   uint32_t        collateral_rate         = 0;
   uint32_t        collateral_rate_carrier = 0;   //运营商增加的抵押率百分比  1000 为单位   

   uint32_t        order_state = 0;          //当前状态
   uint32_t        recycle_pre_state = 0;    //不良资产前状态
   uint32_t        history_pre_state = 0;    //进入历史之前的状态   

   price_feed      current_feed;

   asset           current_invest;

   uint8_t         need_add_collateralize = 0;

   uint32_t        offset_time = 0;               //测试逾期时间偏移量
};

struct bitlender_invest_info
{

   bitlender_invest_id_type      id;   
   bitlender_order_id_type       order_id;           //标单id
   account_info                  issuer;
   account_info                  carrier;
   asset                         amount_to_loan;   
   asset_info                    asset_to_loan;

   asset                         amount_to_collateralize;     
   asset_info                    asset_to_collateralize;   

   asset                          amount_to_invest;     
   asset_info                     asset_to_invest; 

   asset                          amount_to_recycle;        //回收的抵押币  

   asset                          repay_principal_fee;      //提前还款费用
   time_point_sec                 invest_time;         
   time_point_sec                 repay_principal_time;

   uint32_t                       state = 0;      

   price_feed                     current_feed;

   optional<bitlender_order_info> order_info;
};

class bitlender_invest_object : public abstract_object<bitlender_invest_object>
{
   public:
      static const uint8_t space_id = implementation_ids;
      static const uint8_t type_id  = impl_bitlender_invest_object_type;     
    
      account_id_type           issuer;             //应标人     
      account_id_type           carrier;            //运营商账户ID
      carrier_id_type           carrier_id;         //运营商ID
      account_id_type           referrer;           //推荐人
      bitlender_order_id_type   order_id;           //标单id
      bitlender_option_id_type  option_id;          //参数id 
      asset                     amount_to_loan;     //借款币  
      asset                     amount_to_invest;         //法币
      asset                     amount_to_collateralize;  //抵押币    
      asset                     amount_to_recycle;        //回收的抵押币  
      asset                     amount_to_indemnify;      //回收的补偿币      
      asset                     repay_principal_fee;      //提前还款费用
      bitlender_rate            interest_rate;            //借款利率
      time_point_sec            invest_time;              //应标时间
      time_point_sec            repay_principal_time;     //还本时间

      flat_map<uint32_t,bitlender_repay_interest> repay_interest;//还利息记录

      bitlender_key            repayment_type;       //借款类型 
      asset                    lock_invest;          //锁定的法币
      uint32_t                 state = 0;      

      pair<asset_id_type,asset_id_type> get_market() const
      {
         auto tmp = std::make_pair( amount_to_invest.asset_id, amount_to_collateralize.asset_id );
         if( tmp.first > tmp.second ) std::swap( tmp.first, tmp.second );
         return tmp;
      }

      void  get_info(const database &db, bitlender_invest_info &info,bool borderinfo = true) const;
};
struct by_time;
struct by_id;
typedef multi_index_container<
   bitlender_invest_object,
   indexed_by<
      ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > > ,
      ordered_non_unique< tag<by_time>, member<bitlender_invest_object, time_point_sec, &bitlender_invest_object::invest_time> >     
   >
> bitlender_invest_multi_index_type;

typedef generic_index<bitlender_invest_object, bitlender_invest_multi_index_type> bitlender_invest_index;

#define ORDER_NORMAL                   1  //发标状态
#define ORDER_EXPIRATION               2  //订单逾期
#define ORDER_INVEST_ING               3  //正在应标中
#define ORDER_INVEST_FINISH            4  //满标
#define ORDER_REPAY_ING                5  //开始付息
#define ORDER_REPAY_FINISH             6  //还息结束
#define ORDER_PRINCIPAL_FINISH         7  //还本结束
#define ORDER_PRE_PRINCIPAL_FINISH     8  //提前还本结束
#define ORDER_OVERDUE_PRINCIPAL_FINISH 9  //逾期还本结束
#define ORDER_INTEREST_OVERDUE         10 //利息逾期
#define ORDER_PRINCIPAL_OVERDUE        11 //本金逾期
#define ORDER_RECYCLE                  12 //不良资产
#define ORDER_RECYCLE_FINISH           13 //不良资产处理完成
#define ORDER_NOTIFY_ING               14 //逾期通知
#define ORDER_SQUEEZE                  15 //强制平仓
#define ORDER_NOTIFY_SQUEEZE           16 //强制平仓通知
#define ORDER_CANCEL                   17 //流标
#define ORDER_SQUEEZE_CANCEL           18 //流标平仓
#define ORDER_REMOVE                   19 //删除
#define ORDER_NOTIFY_NEEDCOLLATE       20 //需要补仓的单子
#define ORDER_RECYCLE_FINISHING        21 //不良资产正在处理中
#define ORDER_RECYCLE_SQEEZEING        22 //平仓正在处理中

#define TIME_NORMAL                    0  //正常状态
#define TIME_INTEREST_NOTIFY           1  //利息提醒
#define TIME_INTEREST_OVERDUE          2  //利息逾期

#define TIME_PRINCIPAL_NOTIFY          3  //本金提醒
#define TIME_PRINCIPAL_OVERDUE         4  //本金逾期
#define TIME_PRINCIPAL_RECYCLE         5  //本金回收
#define TIME_PRINCIPAL_RECYCLEFINISH   6  


class bitlender_order_object : public abstract_object<bitlender_order_object>
{
   public:
      static const uint8_t space_id = protocol_ids;
      static const uint8_t type_id  = bitlender_order_object_type;

      account_id_type          issuer;           //借款人      
      account_id_type          carrier;          //运营商账户ID
      carrier_id_type          carrier_id;       //运营商ID      
      account_id_type          referrer;         //推荐人
      account_id_type          autorepaymenter;  //自动还款账户
      bitlender_option_id_type option_id;        //参数id 
      asset_feeds_id_type      feed_id;          //喂价交易对
      std::string              order;            //借款单号      
      asset                    amount_to_loan;   //法币      
      bitlender_key            repayment_type;   //借款类型 

      uint32_t                loan_period;             //借款周期
      bitlender_rate          interest_rate;           //借款利率
      uint32_t                repayment_date;          //还款日期
      

      asset           amount_to_collateralize;  //抵押币        
      asset           collateralize_fee;        //抵押币费用 
      asset           collateralize_risk;       //抵押币风险保证金
      asset           carrier_fee;              //运营商费用 
 
      uint32_t        collateral_rate = 0;         //抵押率,发单或补仓 运营商最小抵押倍数    GRAPHENE_COLLATERAL_RATIO_DENOM
      uint32_t        collateral_rate_carrier = 0; //运营商增加的抵押率百分比  1000 为单位
      uint32_t        bid_period = 0;           //挂单秒数    
      fee_mode_type   fee_mode ;                //费用模式
       
      time_point_sec  loan_time              = time_point_sec::maximum(); //挂单开始时间  
      time_point_sec  expiration_time        = time_point_sec::maximum(); //挂单结束时间         
      time_point_sec  invest_finish_time     = time_point_sec::maximum(); //满标时间         
      time_point_sec  repay_principal_time   = time_point_sec::maximum(); //还款时间       
      time_point_sec  expect_principal_time  = time_point_sec::maximum(); //预计还款时间   
      //各种逾期时间      
      uint32_t        time_state             = 0;                     //各种逾期状态，内部使用，外部不能使用 
      time_point_sec  notify_time            = time_point_sec::max(); //各种逾期时间，内部使用，外部不能使用       

      string          memo; //订单备注

      asset           lock_collateralize;    //锁定的抵押币            
      asset           repay_principal_fee;   //提前还款费用      

      flat_map<bitlender_invest_id_type,bitlender_invest_order>    interest_book; //投资人列表
      flat_map<uint32_t,bitlender_order_interest>                  repay_interest; //还利息记录
      vector<share_type>                                           distribution_fees;//费用分配记录

      uint32_t        order_state = 0;          //当前状态
      uint32_t        order_flag  = 0;          //当前操作标志
   
      price_feed      price_settlement;         //喂价   

      uint32_t        overdue_expiration_time;  //逾期时间
      uint32_t        max_overdue_period;       //最大逾期月份，超过这个期数为本金逾期
      uint32_t        notify_overdue_time;      //提前通知时间
      uint32_t        ahead_autopay_time;       //提前还款时间
      string          info;                     //各种信息
      uint32_t        recycle_pre_state = 0;    //不良资产前状态
      uint32_t        history_pre_state = 0;    //进入历史之前的状态   
      

      uint32_t        overdue_interest_periods    =0;//逾期期数，不存盘
      uint32_t        offset_time = 0;               //测试逾期时间偏移量


    public:
      bool is_loan()             const { return  order_state == ORDER_NORMAL;  }      //刚投标
      bool is_investing()        const { return  order_state == ORDER_INVEST_ING;}    //已经开始应标
      bool is_investfinish()     const { return  order_state == ORDER_INVEST_FINISH;} //应标结束
      bool is_interest()         const { return  order_state == ORDER_REPAY_ING;}     //开始还息
      bool is_interestfinish()   const { return  order_state == ORDER_REPAY_FINISH;}  //还息完成     
      bool is_principalfinish()  const { return  order_state == ORDER_PRINCIPAL_FINISH||order_state == ORDER_PRE_PRINCIPAL_FINISH||order_state == ORDER_OVERDUE_PRINCIPAL_FINISH;}  //还本完成
      bool is_principaloverdue() const { return  order_state == ORDER_PRINCIPAL_OVERDUE;}  //本金逾期    
      bool is_recycle()          const { return  order_state == ORDER_RECYCLE || recycle_pre_state == ORDER_RECYCLE;} //不良资产
      bool is_recycleex()        const { return  order_state == ORDER_RECYCLE;}                                        //不良资产
      bool is_squeeze()          const { return  order_state == ORDER_SQUEEZE;} //强制平仓      
      bool is_recyclefinish()    const { return  order_state == ORDER_RECYCLE_FINISH;} //不良资产完成
      bool is_recyclefinishing() const { return  order_state == ORDER_RECYCLE_FINISHING || order_state == ORDER_RECYCLE_SQEEZEING;} //不良资产正在处理中
      bool is_normal()           const { return  order_state == ORDER_NORMAL || order_state == ORDER_INVEST_ING || order_state == ORDER_INVEST_FINISH || order_state == ORDER_REPAY_ING ||  order_state == ORDER_PRINCIPAL_OVERDUE;}  


      pair<asset_id_type,asset_id_type> get_market() const
      {
         auto tmp = std::make_pair( amount_to_loan.asset_id, amount_to_collateralize.asset_id );
         if( tmp.first > tmp.second ) std::swap( tmp.first, tmp.second );
         return tmp;
      }
      asset get_overdue_fee(const fc::time_point_sec &now, const bitlender_option_object &option) const ;
      asset get_overdue_fee(const asset pay,const fc::time_point_sec &now, const bitlender_option_object &option) const ;      
      bool  is_notify(database &d) const ;
      bool  is_needaddcoll(database &d) const ;
      bool  is_recycle_issuer(const database &db,const account_id_type &d) const ;
      bool  can_interest(const database &d ,const account_id_type &id) const;
      void  set_recycle( database &d) ;
      void  set_overdue( database &d) ;
      void  get_info(const database &db, bitlender_order_info &info) const;
};

struct by_id;
struct by_time;
struct by_loan;
typedef multi_index_container<
   bitlender_order_object,
   indexed_by<
      ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >    ,
      ordered_non_unique< tag<by_time>, member<bitlender_order_object, time_point_sec, &bitlender_order_object::loan_time> >   
   >
> bitlender_order_multi_index_type;

typedef generic_index<bitlender_order_object, bitlender_order_multi_index_type> bitlender_order_index;

 class bitlender_order_data : public  bitlender_order_object 
 {
public: 
     account_info     issuer_info; 
     asset_info       asset_to_loan; 
     asset_info       asset_to_collateralize;   
     price_feed       current_feed;                //当前喂价 
     asset            current_invest;              //当前投资总量
     uint8_t          need_add_collateralize = 0;  //是否需要增加抵押物

     //flat_map<bitlender_invest_id_type,account_info>    interest_account; //配合显示投资人信息
     flat_map<bitlender_invest_id_type,bitlender_invest_order_>    interest_book; //投资人列表

     uint32_t         offset_time = 0;               //测试逾期时间偏移量

     void get_object(const database &db,const bitlender_order_object &object);

     virtual variant to_variant()const { return variant( static_cast<const bitlender_order_data&>(*this), MAX_NESTING ); }
 };
 class bitlender_invest_data : public  bitlender_invest_object 
 {
public: 
     account_info     issuer_info; 
     asset_info       asset_to_loan; 
     asset_info       asset_to_collateralize;   
     account_info     carrier_info;

     price_feed       current_feed;   //当前喂价

     optional<bitlender_order_data> order_info;  //订单信息
     
     void get_object(const database &db,const bitlender_invest_object &object,bool borderinfo = true);
   
     virtual variant to_variant() const { return variant(static_cast<const bitlender_invest_data &>(*this), MAX_NESTING); }
 };

  

  

} } // graphene::chain
 
FC_REFLECT( graphene::chain::bitlender_repay_interest,
                    (finish_time)
                    (amount_repay_interest)
                    (fines_repay_interest)                   
                    ) 

FC_REFLECT( graphene::chain::bitlender_invest_order,
                    (account)
                    (invest)  
                    (carrier)
                    (invest_time)      
                    )      
FC_REFLECT( graphene::chain::bitlender_invest_order_,
                    (name)
                    (account)
                    (invest)  
                    (carrier_name)
                    (carrier)
                    (invest_time)      
                    )                            
FC_REFLECT( graphene::chain::bitlender_order_interest,  
                    (pay_id)
                    (interest_state)
                    (expect_time)                    
                    (finish_time)
                    (expect_repay_interest)
                    (amount_repay_interest)
                    (fines_repay_interest)
                    (interest_rate)    
                    (ustate)
                    )        
 
 FC_REFLECT( graphene::chain::bitlender_invest_info,
                    (id)
                    (issuer)
                    (carrier)
                    (amount_to_loan)                   
                    (asset_to_loan)
                    (amount_to_collateralize)
                    (asset_to_collateralize)                   
                    (amount_to_invest)
                    (asset_to_invest)
                    (amount_to_recycle)
                    (repay_principal_fee)
                    (invest_time)                   
                    (repay_principal_time)
                    (state)
                    (current_feed)                  
                    (order_info)  
                    ) 
FC_REFLECT_DERIVED( graphene::chain::bitlender_invest_object, (graphene::db::object),                                 
                    (issuer)                    
                    (carrier)
                    (carrier_id)                    
                    (referrer)
                    (order_id)
                    (option_id)
                    (amount_to_loan)                      
                    (amount_to_invest)
                    (amount_to_collateralize)                    
                    (amount_to_recycle)
                    (amount_to_indemnify)
                    (repay_principal_fee)             
                    (interest_rate)
                    (invest_time)
                    (repay_principal_time)
                    (repay_interest)
                    (repayment_type)
                    (lock_invest)                      
                    (state)
                    ) 

FC_REFLECT( graphene::chain::bitlender_order_info,
                    (id)
                    (issuer)
                    (carrier)
                    (carrier_id) 
                    (amount_to_loan)
                    (asset_to_loan)                   
                    (amount_to_collateralize)
                    (asset_to_collateralize)                    
                    (repay_interest)       
                    (interest_book)
                    (repayment_type)
                    (loan_period)                            
                    (loan_time)                   
                    (expiration_time)
                    (invest_finish_time)
                    (repay_principal_time)                   
                    (expect_principal_time)
                    (notify_time)
                    (collateralize_fee)
                    (collateralize_risk)
                    (carrier_fee)                   
                    (repay_principal_fee) 
                    (interest_rate)
                    (collateral_rate)
                    (collateral_rate_carrier)
                    (order_state)
                    (recycle_pre_state)
                    (history_pre_state)
                    (current_feed)
                    (current_invest)   
                    (need_add_collateralize)
                    (offset_time)
                    ) 
FC_REFLECT_DERIVED( graphene::chain::bitlender_order_object, (graphene::db::object),                                                           
                    (issuer)                    
                    (carrier)
                    (carrier_id)                    
                    (referrer)
                    (autorepaymenter)
                    (option_id)
                    (feed_id)
                    (order)
                    (amount_to_loan)                             
                    (repayment_type)   
                    (loan_period)
                    (interest_rate)
                    (repayment_date)                    
                    (amount_to_collateralize)                              
                    (collateralize_fee)      
                    (collateralize_risk)
                    (carrier_fee)
                    (collateral_rate)
                    (collateral_rate_carrier)
                    (bid_period)    
                    (fee_mode)
                    (loan_time)
                    (expiration_time)
                    (invest_finish_time)
                    (repay_principal_time)
                    (expect_principal_time)  
                    (time_state)  
                    (notify_time)  
                    (memo)
                    (lock_collateralize)
                    (repay_principal_fee)                           
                    (interest_book)                    
                    (repay_interest) 
                    (distribution_fees)
                    (order_state)      
                    (order_flag)
                    (price_settlement)                                     
                    (overdue_expiration_time)     
                    (max_overdue_period)
                    (notify_overdue_time)
                    (ahead_autopay_time)
                    (info)          
                    (recycle_pre_state)
                    (history_pre_state) 
                  )   

FC_REFLECT_DERIVED( graphene::chain::bitlender_order_data, (graphene::chain::bitlender_order_object),      
                   (issuer_info)
                   (asset_to_loan)
                   (asset_to_collateralize)
                   (current_feed)
                   (current_invest)          
                   (need_add_collateralize)         
                   (interest_book)
                   (offset_time)
                   ) 

FC_REFLECT_DERIVED( graphene::chain::bitlender_invest_data, (graphene::chain::bitlender_invest_object),      
                   (issuer_info)
                   (asset_to_loan)
                   (asset_to_collateralize)
                   (carrier_info)
                   (current_feed)             
                   (order_info)                
                   ) 

 
       