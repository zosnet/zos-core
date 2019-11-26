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
#include <graphene/business/protocol/finance_ops.hpp>

namespace graphene { namespace chain {

using namespace graphene::db; 


  class fundraise_option_data 
  {
      public:
      fundraise_option     option;
      share_type           supply = 0;
      bool                 enable = true;
  };

   struct fundraise_info
   {
      uint8_t        type;
      uint32_t       period;
      share_type     supply;
      share_type     max_supply;
      time_point_sec start_time; //开始时间
      time_point_sec end_time; //结束时间
      price          fundraise_price;//筹款价格
   };

   struct fundraise_feed_data
   {
      asset sell;
      asset buy;            
      flat_map<uint32_t, fundraise_option_data> options;
      void     get_info(const uint32_t period,fundraise_info &info) const;
      uint32_t get_period(const fc::time_point_sec now) const;
      bool     is_enable(const uint32_t period) const;
      fundraise_feed_type get_fundraise_type(uint32_t period) const;
      void  create(const issue_fundraise_object &sell,const fundraise_type *pdata) ; 
      void  can_update(const database &db,const issue_fundraise_object &sell_obj,const issue_fundraise_update_operation &pdata) const;
      void  update(const issue_fundraise_object &sell_obj, const fundraise_type *pdata, const uint32_t period ,const uint32_t op_type); 
      void  can_feed(const database &db, const issue_fundraise_publish_feed_operation &pdata) const; 
      void  publish_feed(const database &db, const issue_fundraise_publish_feed_operation &pdata); 
      price canbuy(const database &db,  const buy_fundraise_create_operation &op) const; 
      void  buysync(const database &db,   const buy_fundraise_object &op,const bool enable); 
   }; 
   struct fundraise_bancor_data
   {
      asset sell;
      void     get_info(const uint32_t period,fundraise_info &info) const;
      uint32_t get_period(const fc::time_point_sec now) const { return 1; };
      bool     is_enable(const uint32_t period) const;
      fundraise_bancor_type get_fundraise_type(uint32_t period) const;  
      void  create(const issue_fundraise_object &sell,const fundraise_type *pdata) ; 
      void  can_update(const database &db,const issue_fundraise_object &sell_obj,const issue_fundraise_update_operation &pdata) const;
      void  update(const issue_fundraise_object &sell_obj, const fundraise_type *pdata,const uint32_t period ,const uint32_t op_type); 
      void  can_feed(const database &db, const issue_fundraise_publish_feed_operation &pdata) const;  
      void  publish_feed(const database &db, const issue_fundraise_publish_feed_operation &pdata); 
      price canbuy(const database &db,   const buy_fundraise_create_operation &op) const; 
      void  buysync(const database &db,   const buy_fundraise_object &op,const bool enable); 
   }; 
  struct fundraise_market_data
  {   
      asset sell;       
      void     get_info(const uint32_t period,fundraise_info &info) const;
      uint32_t get_period(const fc::time_point_sec now) const { return 1; };
      bool     is_enable(const uint32_t period) const;
      fundraise_market_type get_fundraise_type(uint32_t period) const; 
      void  create(const issue_fundraise_object &sell,const fundraise_type *pdata) ;
      void  can_update(const database &db,const issue_fundraise_object &sell_obj,const issue_fundraise_update_operation &pdata) const;
      void  update(const issue_fundraise_object &sell_obj, const fundraise_type *pdata,const uint32_t period ,const uint32_t op_type);
      void  can_feed(const database &db, const issue_fundraise_publish_feed_operation &pdata) const; 
      void  publish_feed(const database &db, const issue_fundraise_publish_feed_operation &pdata); 
      price canbuy(const database &db,  const buy_fundraise_create_operation &op) const; 
      void  buysync(const database &db,   const buy_fundraise_object &op,const bool enable); 
  };
  typedef static_variant<
   fundraise_feed_data,
   fundraise_bancor_data,
   fundraise_market_data
   > fundraise_data;



   class issue_fundraise_object :  public abstract_object<issue_fundraise_object>
   {
      public:
         static const uint8_t space_id = protocol_ids;
         static const uint8_t type_id  = issue_fundraise_object_type;
         
         account_id_type          issuer;
         uint32_t                 need_auth = 0;
         vector<account_id_type>  trust_auth;
         asset_id_type            issue_id;         //筹集那个货币
         asset_id_type            buy_id;        //返回那个货币   
         account_id_type          fundraise_owner;//筹集归属人
         string                   url;            //描述
         uint32_t                 overdue_time = 0;   //每笔临时单子的逾期时间
         fc::time_point_sec       finance_time;        //投资时间
         //flat_map<uint32_t, fundraise_option_data> options;
         fundraise_data           fundraise_op; //配置参数

         
         void validate() const;
         uint32_t get_period(const fc::time_point_sec now) const;
         bool     is_enable(const uint32_t period) const;
         void     get_info(const uint32_t period, fundraise_info &info) const;
   };

   struct by_id;
   struct by_asset_id;
   typedef multi_index_container<
      issue_fundraise_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > > ,
         ordered_unique<
           tag<by_asset_id>,
           composite_key<
            issue_fundraise_object,
            member<issue_fundraise_object, asset_id_type, &issue_fundraise_object::issue_id>,
            member<issue_fundraise_object, asset_id_type, &issue_fundraise_object::buy_id>> >                   
      >
   > issue_fundraise_object_multi_index_type;
   typedef generic_index<issue_fundraise_object, issue_fundraise_object_multi_index_type> issue_fundraise_index; 



   class finance_paramers_object :  public abstract_object<finance_paramers_object>
   {
      public:
         static const uint8_t space_id = implementation_ids;
         static const uint8_t type_id  = impl_finance_paramers_object_type;      

         finance_paramers options;//基础参数 
         optional<finance_paramers> pending_options;     
   
   };
       
 
   typedef multi_index_container<
      finance_paramers_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >       
      >
   > finance_paramers_object_multi_index_type;
   typedef generic_index<finance_paramers_object, finance_paramers_object_multi_index_type> finance_paramers_index;


#define    finance_normal  1
#define    finance_wait    2
#define    finance_expire  3
#define    finance_finish  4

class buy_fundraise_object : public abstract_object<buy_fundraise_object>
{
   public:
      static const uint8_t space_id = protocol_ids;
      static const uint8_t type_id  = buy_fundraise_object_type;
    
      account_id_type           issuer;              //购买人      
      account_id_type           fundraise_owner;     //款项接收人
      issue_fundraise_id_type   fundraise_id;        //众筹单号
      uint32_t                  period;              //期数
      uint32_t                  ustate;              //状态
      asset                     issue_asset;         //发行的币
      asset                     issue_fee;           //发行方费用,发行币为单位
      asset                     buy_asset;           //购买币
      asset                     buy_fee;             //购买人费用，发行币为单位
      price                     fundraise_price;     //价格
      fc::time_point_sec        finance_time;        //投资时间
      fc::time_point_sec        overdue_time;        //自动生效时间
      fc::time_point_sec        finish_time;         //生效时间     
      optional<memo_data>       memo;      //

      uint32_t                  offset_time = 0; //测试逾期时间偏移量
};

struct by_id;
   typedef multi_index_container<
      buy_fundraise_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >                 
      >
   > buy_fundraise_object_multi_index_type;
   typedef generic_index<buy_fundraise_object, buy_fundraise_object_multi_index_type> buy_fundraise_index; 

 
   struct exchange_feed_data
   {
      asset sell;
      asset buy;    
      price ex_price;            //价格
      share_type min_amount = 0; //最小购买
      share_type max_amount = 0;  //最大购买
      share_type step_amount = 0; //购买步长
      price get_price(const asset &buy) const;
      void  validate(const sell_exchange_object &obj,const asset &sell,const asset &buy) const;   
      void  update(const sell_exchange_object &sell, const exchange_type *pdata);
      void  create(const sell_exchange_object &sell, const exchange_type *pdata);
      bool  canbuy(const asset &buy) const;
      void  sync(database &d, sell_exchange_object &sell);
      exchange_feed_type get_exchange_type() const;
   }; 
   struct exchange_bancor_data
   {
      asset sell;
      asset buy;     
      share_type total_bancor = 0;   
      vesting_balance_id_type balance;
      price get_price(const asset &buy) const;     
      void  validate(const sell_exchange_object &obj,const asset &sell,const asset &buy) const;   
      void  update(const sell_exchange_object &sell,const exchange_type *pdata) ; 
      void  create(const sell_exchange_object &sell,const exchange_type *pdata) ;  
      bool  canbuy(const asset &buy) const; 
      void  sync(database &d, sell_exchange_object &sell);
      exchange_bancor_type get_exchange_type() const;   
   }; 
  struct exchange_market_data
  {   
      asset sell;
      asset buy;      
      share_type total_market = 0;
      price get_price(const asset &buy) const;     
      void  validate(const sell_exchange_object &obj,const asset &sell,const asset &buy) const;   
      void  update(const sell_exchange_object &sell,const exchange_type *pdata) ; 
      void  create(const sell_exchange_object &sell,const exchange_type *pdata) ;  
      bool  canbuy(const asset &buy) const;
      void  sync(database &d, sell_exchange_object &sell);    
      exchange_market_type get_exchange_type() const; 
  };
  typedef static_variant<
   exchange_feed_data,
   exchange_bancor_data,
   exchange_market_data
   > exchange_data;


class sell_exchange_object : public graphene::db::abstract_object<sell_exchange_object>
{
   public:
      static const uint8_t space_id = protocol_ids;
      static const uint8_t type_id =  sell_exchange_object_type;

      account_id_type         issuer;
      uint32_t                need_auth = 0;
      vector<account_id_type> trust_auth;
      asset                   sell_init;   //销售的币 初始化
      asset                   buy_init;         //购买的币 初始化
      asset                   sell;        //销售的币     
      asset                   buy;         //购买的币
      string                  url;         //描述
      share_type              sell_supply;
      share_type              buy_supply;
      bool                    exchange;

      exchange_data           exchange_op; //配置参数
      bool                    enable = true;


      fc::time_point_sec  create_time;  //创建时间
      fc::time_point_sec  overdue_time;
      fc::time_point_sec  last_time; //最后交易时间


      uint32_t             offset_time = 0; //测试逾期时间偏移量

      price get_price() const;
      void sync(database *db);
      bool canbuy(const asset *p) const;
};

typedef multi_index_container<
   sell_exchange_object,
   indexed_by<
      ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >      
   >
> sell_exchange_object_multi_index_type;

using sell_exchange_index = generic_index<sell_exchange_object,sell_exchange_object_multi_index_type>;

class buy_exchange_object : public graphene::db::abstract_object<buy_exchange_object>
{
   public:
      static const uint8_t space_id = protocol_ids;
      static const uint8_t type_id =  buy_exchange_object_type;
      
      account_id_type       issuer;
      sell_exchange_id_type sell_id;      
      asset                 sell_asset;          //发行的币     
      asset                 sell_fee;            //发行方费用,发行币为单位
      asset                 buy_asset;           //购买币
      asset                 buy_fee;             //购买人费用，发行币为单位
      price                 min_price;           //价格
      price                 finish_price;        //成交价格
      uint32_t              ustate;
      uint32_t              op_type;             // 0 指定id  1 撮合
      fc::time_point_sec    finance_time;        //投资时间
      fc::time_point_sec    overdue_time;        //自动生效时间
      fc::time_point_sec    finish_time;         //生效时间     
      optional<memo_data>   memo;      //  

      uint32_t             offset_time = 0; //测试逾期时间偏移量    
};

typedef multi_index_container<
   buy_exchange_object,
   indexed_by<
      ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >      
   >
> buy_exchange_object_multi_index_type;

using buy_exchange_index = generic_index<buy_exchange_object,buy_exchange_object_multi_index_type>;
  
} } // graphene::chain 



FC_REFLECT( graphene::chain::fundraise_option_data, 
            (option)
            (supply)     
            (enable)
          )  


FC_REFLECT( graphene::chain::fundraise_feed_data,
              (sell)            
              (buy)            
              (options)   
              )
FC_REFLECT( graphene::chain::fundraise_bancor_data,
              (sell)              
              )
FC_REFLECT( graphene::chain::fundraise_market_data,
              (sell)             
              )
FC_REFLECT_TYPENAME( graphene::chain::fundraise_data )


FC_REFLECT_DERIVED( graphene::chain::issue_fundraise_object, (graphene::db::object),            
            (issuer)        
            (need_auth)    
            (trust_auth)    
            (issue_id)            
            (buy_id)  
            (fundraise_owner)  
            (url)              
            (overdue_time)
            (finance_time)
            //(options)   
            (fundraise_op)
          )        
FC_REFLECT_DERIVED( graphene::chain::finance_paramers_object, (graphene::db::object),
            (options)     
            (pending_options)
          )  



FC_REFLECT_DERIVED( graphene::chain::buy_fundraise_object, (graphene::db::object),                                        
                    (issuer)
                    (fundraise_owner)
                    (fundraise_id)
                    (period)                    
                    (ustate) 
                    (issue_asset)   
                    (issue_fee)
                    (buy_asset)
                    (buy_fee)
                    (fundraise_price)
                    (finance_time)
                    (overdue_time)
                    (finish_time)                
                    (memo)   
                  )                         
        
 

FC_REFLECT( graphene::chain::exchange_feed_data,
              (sell)
              (buy)              
              (ex_price) 
              (min_amount) 
              (max_amount) 
              (step_amount)  
              )
FC_REFLECT( graphene::chain::exchange_bancor_data,
              (sell)
              (buy)              
              (balance)
              (total_bancor)
              )
FC_REFLECT( graphene::chain::exchange_market_data,
              (sell)
              (buy)               
              (total_market) 
              )

FC_REFLECT_TYPENAME( graphene::chain::exchange_data )
FC_REFLECT_DERIVED( graphene::chain::sell_exchange_object, (graphene::db::object),  
                    (issuer)
                    (need_auth)
                    (trust_auth)    
                    (sell) 
                    (sell_init)
                    (buy) 
                    (buy_init)
                    (url)           
                    (sell_supply)
                    (buy_supply)           
                    (exchange)                      
                    (exchange_op)      
                    (enable)    
                    (create_time)      
                    (overdue_time)
                    (last_time)      
                    )
FC_REFLECT_DERIVED( graphene::chain::buy_exchange_object, (graphene::db::object),  
                    (issuer)
                    (sell_id)
                    (sell_asset)
                    (sell_fee)
                    (buy_asset)
                    (buy_fee)
                    (min_price)
                    (finish_price)
                    (ustate)
                    (op_type)
                    (finance_time)
                    (overdue_time)
                    (finish_time)
                    (memo)   
                     )
                  