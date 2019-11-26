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
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/memo.hpp>

#include <string>
#include <vector>



namespace graphene { namespace chain { 

  class  sell_exchange_object;
  class  issue_fundraise_object;
  class  database;
 

  class finance_paramers
  {
     public:
        uint32_t            fundraise_fees_rate = 0;                    //费用比例  10000单位, 扣除费用，为 销售币        
        uint32_t            fundraise_fees_distribution = 50;           //分配比例  100 单位 买方， 买方各自的费用分配比例 
        uint32_t            exchange_fees_rate = 0;                     //费用比例  10000单位, 扣除费用，为 销售币        
        uint32_t            exchange_fees_distribution = 50;            //分配比例  100 单位 买方， 买方各自的费用分配比例 
        uint64_t            sell_exchange_overdue_time = 3600*24*365;   //购买挂单时间
        uint64_t            buy_exchange_overdue_time  = 3600*24*30*3;  //购买挂单时间

        extensions_type     extensions;
     void validate() const;
  };

  
 
struct fundraise_option
{
  
      uint32_t               period = 0;                    //期数
      account_id_type        feed_publiser;                 //价格修改人
      share_type             max_supply = 0;                //最大筹集量
      share_type             min_fundraise  = 10000000;     //单笔最小，以换算后的 zos      
      share_type             max_fundraise  = 1000000000;   //单笔最大，以换算后的 zos 
      share_type             step_fundraise = 1;            //增长最小，以换算后的 zos 
      time_point_sec         start_time;                    //开始时间
      time_point_sec         end_time;                      //结束时间
      price                  fundraise_price;               //筹款价格     
      
      uint32_t               time_zone    = 0;              //时区
      extensions_type        extensions;

      void validate() const;
      
};



   struct buy_fundraise_create_operation : public base_operation
   {
      struct fee_parameters_type 
      {         
        uint64_t fee = 5 * GRAPHENE_BLOCKCHAIN_PRECISION;       
      };

      asset                         fee;              //费用
      account_id_type               issuer;           //购买人
      issue_fundraise_id_type       fundraise_id;     //众筹单号
      uint32_t                      period;           //期数 
      asset                         amount;           //筹资数量
      uint32_t                      bimmediately = 1; //立刻生效    
      optional<memo_data>           memo;      //  
      
      extensions_type          extensions;     
     
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;   
      share_type      calculate_fee(const fee_parameters_type& k) const;
   };
   struct buy_fundraise_enable_operation : public base_operation
   {
      struct fee_parameters_type 
      {
         
        uint64_t fee = 50000;            
      };

      asset                    fee;
      account_id_type          issuer;            //购买人  
      buy_fundraise_id_type    fundraise_id;      //购买单号
      uint32_t                 benable;           //是否生效

      extensions_type          extensions;      
     
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;   
      share_type      calculate_fee(const fee_parameters_type& k) const;
   }; 

   struct fundraise_feed_type :public fundraise_option
   {      
      void  validate(const asset_id_type &sell,const asset_id_type &buy) const;
      void  copy(const fundraise_option &other) { *(fundraise_option *)this = other; }
      void  validate_db(const database &db,const asset_id_type &sell,const asset_id_type &buy) const;
      
   }; 
   struct fundraise_bancor_type
   {
     share_type total_bancor = 0;   
     vesting_balance_id_type balance;     
     void  validate(const asset_id_type &sell,const asset_id_type &buy) const;     
     void  validate_db(const database &db,const asset_id_type &sell,const asset_id_type &buy) const;
      
   }; 
  struct fundraise_market_type
  {    
     share_type total_market = 0;     
     void  validate(const asset_id_type &sell,const asset_id_type &buy) const;  
     void  validate_db(const database &db,const asset_id_type &sell,const asset_id_type &buy) const;
       
  };
  typedef static_variant<
   fundraise_feed_type,
   fundraise_bancor_type,
   fundraise_market_type
   > fundraise_type;

  struct fundraise_validate_visitor
  { 
   private:
     const asset_id_type *p_sell;
     const asset_id_type *p_buy;
   public:
      fundraise_validate_visitor(const asset_id_type *op_sell,const asset_id_type *op_buy) : p_sell(op_sell),p_buy(op_buy) {}
      typedef void result_type;
      template<typename W>
      void operator()(W& fundraise)const
      {
         fundraise.validate(*p_sell,*p_buy);
      }
  };


  struct fundraise_validate_db_visitor
  { 
   private:
     const database      *p_db;     
     const asset_id_type *p_sell;
     const asset_id_type *p_buy;
   public:
      fundraise_validate_db_visitor(const database *_db,const asset_id_type *op_sell,const asset_id_type *op_buy) : p_db(_db),p_sell(op_sell),p_buy(op_buy){}
      typedef void result_type;
      template<typename W>
      void operator()(W& fundraise)const
      {
         fundraise.validate_db(*p_db,*p_sell,*p_buy);
      }
  };

struct issue_fundraise_create_operation : public base_operation
{
  struct fee_parameters_type
  {     
      uint64_t fee = 5 * GRAPHENE_BLOCKCHAIN_PRECISION;
  };
  asset                    fee;  
  account_id_type          issuer;              //创建人
  uint32_t                 need_auth = 0;
  vector<account_id_type>  trust_auth;
  asset_id_type            issue_asset_id;      //发行币  
  account_id_type          issue_asset_owner;   //发行币的创建者
  asset_id_type            buy_asset_id;        //购买币
  account_id_type          fundraise_owner;     //筹集收款人 
  string                   url;                 //描述
  uint32_t                 overdue_time = 0;   //每笔临时单子的逾期时间
  fundraise_type           initializer;  //参数
  
  extensions_type          extensions;   
  account_id_type fee_payer() const { return issuer; }
  void validate() const;
  
  void get_required_active_authorities(flat_set<account_id_type> &a) const
  {  
    a.clear();
    if(issue_asset_owner == GRAPHENE_NULL_ACCOUNT)
        a.insert(GRAPHENE_COMMITTEE_ACCOUNT);
    else 
        a.insert(issue_asset_owner);
  }
 };   
      
   struct issue_fundraise_update_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee             = 50000; };

      asset                             fee;       
      account_id_type                   issuer;            //修改人
      optional<uint32_t>                need_auth;
      optional<vector<account_id_type>> trust_auth;
      issue_fundraise_id_type           fundraise_id;      //筹款单号  
      asset_id_type                     issue_asset_id;      //发行币  
      account_id_type                   issue_asset_owner; //发行币的创建者
      asset_id_type                     buy_asset_id;        //购买币    
      uint32_t                          period;            //期数
      optional<fundraise_type>          update_op;    //参数
      optional<string>                  url;               //描述
      optional<uint32_t>                overdue_time;   //每笔临时单子的逾期时间
      uint32_t                          op_type = 0;       //0 增加或者修改 1 删除 2 停运 3 启运 

      extensions_type                extensions;

      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
 
      void            get_required_active_authorities( flat_set<account_id_type>& a )const 
      {
        a.clear();       
        if(issue_asset_owner == GRAPHENE_NULL_ACCOUNT)
          a.insert(GRAPHENE_COMMITTEE_ACCOUNT);
        else 
          a.insert(issue_asset_owner);
      }      
   };

   struct issue_fundraise_remove_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee             = 50000; };

      asset                          fee;       
      account_id_type                issuer;            //修改人
      issue_fundraise_id_type        fundraise_id;      //筹款单号
      account_id_type                issue_asset_owner; //发行币的创建者
      
      extensions_type                extensions;

      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
 
      void            get_required_active_authorities( flat_set<account_id_type>& a )const 
      {
        a.clear();       
        if(issue_asset_owner == GRAPHENE_NULL_ACCOUNT)
          a.insert(GRAPHENE_COMMITTEE_ACCOUNT);
        else 
          a.insert(issue_asset_owner);
      }      
   };

   struct issue_fundraise_publish_feed_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee             = 50000; };

      asset                          fee;       
      account_id_type                issuer; 
      issue_fundraise_id_type        fundraise_id;     
      uint32_t                       period;               //期数     
      price                          fundraise_price;  //筹款价格
      extensions_type                extensions;

      account_id_type fee_payer()const { return issuer; }
      void            validate()const; 
      void            get_required_active_authorities( flat_set<account_id_type>& a )const 
      {
        a.clear();       
        a.insert(issuer);
      }      
   };


   struct finance_paramers_update_operation : public base_operation
   {
      struct fee_parameters_type {   uint64_t fee             = 10000;     };

      asset                    fee;       
      account_id_type          issuer;    
      finance_paramers         options;    

      extensions_type          extensions;
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;     
      void            get_required_active_authorities( flat_set<account_id_type>& a )const 
      {
        a.clear();
        a.insert(GRAPHENE_COMMITTEE_ACCOUNT);
      }      
   };
   struct exchange_feed_type
   {    
      price      ex_price;            //价格      
      share_type min_amount  = 10000000;     //单笔最小，以换算后的 zos      
      share_type max_amount  = 1000000000;   //单笔最大，以换算后的 zos 
      share_type step_amount = 1;            //增长最小，以换算后的 zos    
      void  validate(const asset &sell,const asset &buy) const;    
   }; 
   struct exchange_bancor_type
   {
     share_type total_bancor = 0;   
     vesting_balance_id_type balance;     
     void  validate(const asset &sell,const asset &buy) const;
     
   }; 
  struct exchange_market_type
  {    
     share_type total_market = 0;     
     void  validate(const asset &sell,const asset &buy) const;     
  };
  typedef static_variant<
   exchange_feed_type,
   exchange_bancor_type,
   exchange_market_type
   > exchange_type;


  struct exchange_validate_visitor
  { 
   private:
     const asset *p_sell;
     const asset *p_buy;
   public:
      exchange_validate_visitor(const asset *op_sell,const asset *op_buy) : p_sell(op_sell),p_buy(op_buy) {}
      typedef void result_type;
      template<typename W>
      void operator()(W& exchange)const
      {
         exchange.validate(*p_sell,*p_buy);
      }
  };

   struct sell_exchange_create_operation : public base_operation
   {
      struct fee_parameters_type {   uint64_t fee             =  5 * GRAPHENE_BLOCKCHAIN_PRECISION;     };

      asset                    fee;          //费用
      account_id_type          issuer;       //发布人 
      uint32_t                 need_auth = 0;
      vector<account_id_type>  trust_auth;
      asset                    sell;         //销售的币
      asset                    buy;          //购买的币
      string                   url;          //描述      
      exchange_type            initializer;  //参数

      extensions_type          extensions;
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;     
   };
   struct sell_exchange_update_operation : public base_operation
   {
      struct fee_parameters_type {   uint64_t fee             = 50000;     };

      asset                             fee;       
      account_id_type                   issuer;    
      sell_exchange_id_type             sell_id;
      optional<uint32_t>                need_auth;
      optional<vector<account_id_type>> trust_auth;
      optional<asset>                   sell;         //销售的币
      optional<asset>                   buy;          //购买的币
      optional<string>                  url;          //描述      
      uint32_t                          op_type;      // 0 更新数据 1 停运  2 启动
      optional<exchange_type>           update_op;    //参数
      
      extensions_type                   extensions;
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;     
   };
   struct sell_exchange_remove_operation : public base_operation
   {
      struct fee_parameters_type {   uint64_t fee             = 50000;     };

      asset                    fee;       
      account_id_type          issuer;
      sell_exchange_id_type    sell_id;

      extensions_type          extensions;
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;     
   };
   struct buy_exchange_create_operation : public base_operation
   {
      struct fee_parameters_type {   uint64_t fee             =  5 * GRAPHENE_BLOCKCHAIN_PRECISION;     };

      asset                           fee;       
      account_id_type                 issuer;    //购买者
      asset                           buy;       //购买金额
      optional<sell_exchange_id_type> sell_id;   //销售 id
      optional<price>                 min_price; //最低价格
      optional<memo_data>             memo;      //

      extensions_type                 extensions;
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;     
   };
   struct buy_exchange_update_operation : public base_operation
   {
      struct fee_parameters_type {   uint64_t fee             = 50000;     };

      asset                    fee;       
      account_id_type          issuer;    
      buy_exchange_id_type     buy_id;
      asset                    buy;       //购买金额      
      optional<price>          min_price; //最低价格
      optional<memo_data>      memo;      //
      extensions_type          extensions;
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;     
   };
   struct buy_exchange_remove_operation : public base_operation
   {
      struct fee_parameters_type {   uint64_t fee             = 50000;     };

      asset                    fee;       
      account_id_type          issuer;
      buy_exchange_id_type     buy_id;

      extensions_type          extensions;
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;     
   };

 
} } // graphene::chain


FC_REFLECT( graphene::chain::issue_fundraise_create_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::issue_fundraise_update_operation::fee_parameters_type,  (fee) ) 
FC_REFLECT( graphene::chain::issue_fundraise_remove_operation::fee_parameters_type,  (fee) ) 

FC_REFLECT( graphene::chain::issue_fundraise_publish_feed_operation::fee_parameters_type,  (fee) )
FC_REFLECT( graphene::chain::finance_paramers_update_operation::fee_parameters_type,  (fee) )
  
FC_REFLECT( graphene::chain::buy_fundraise_create_operation::fee_parameters_type, (fee))
FC_REFLECT( graphene::chain::buy_fundraise_enable_operation::fee_parameters_type, (fee))

FC_REFLECT( graphene::chain::sell_exchange_create_operation::fee_parameters_type, (fee))
FC_REFLECT( graphene::chain::sell_exchange_update_operation::fee_parameters_type, (fee))
FC_REFLECT( graphene::chain::sell_exchange_remove_operation::fee_parameters_type, (fee))

FC_REFLECT( graphene::chain::buy_exchange_create_operation::fee_parameters_type, (fee))
FC_REFLECT( graphene::chain::buy_exchange_update_operation::fee_parameters_type, (fee))
FC_REFLECT( graphene::chain::buy_exchange_remove_operation::fee_parameters_type, (fee))

 FC_REFLECT(graphene::chain::finance_paramers,            
            (fundraise_fees_rate)            
            (fundraise_fees_distribution)
            (exchange_fees_rate)            
            (exchange_fees_distribution)
            (sell_exchange_overdue_time)
            (buy_exchange_overdue_time)
            (extensions)
           ) 
           
 FC_REFLECT(graphene::chain::fundraise_option,            
            (period)
            (feed_publiser)
            (max_supply)     
            (min_fundraise)
            (max_fundraise)     
            (step_fundraise)   
            (start_time)
            (end_time)
            (fundraise_price)                        
            (time_zone)
            (extensions)
            )
FC_REFLECT_DERIVED( graphene::chain::fundraise_feed_type,(graphene::chain::fundraise_option),)
FC_REFLECT( graphene::chain::fundraise_bancor_type, (balance)(total_bancor))
FC_REFLECT( graphene::chain::fundraise_market_type, (total_market) )
FC_REFLECT_TYPENAME( graphene::chain::fundraise_type)
 FC_REFLECT( graphene::chain::issue_fundraise_create_operation,   
            (fee)                     
            (issuer) 
            (need_auth)
            (trust_auth)
            (issue_asset_id) 
            (issue_asset_owner) 
            (buy_asset_id)
            (fundraise_owner)            
            (url)
            (overdue_time)
            (initializer) 
            (extensions) 
          )
  FC_REFLECT( graphene::chain::issue_fundraise_update_operation,     
            (fee)                            
            (issuer)        
            (need_auth) 
            (trust_auth)
            (fundraise_id)
            (issue_asset_id) 
            (issue_asset_owner) 
            (buy_asset_id)
            (period)            
            (update_op)
            (url)
            (overdue_time)
            (op_type)        
            (extensions)
          )  
FC_REFLECT( graphene::chain::issue_fundraise_remove_operation,     
            (fee)                            
            (issuer)         
            (fundraise_id)
            (issue_asset_owner)            
            (extensions)
          )  
          
  FC_REFLECT( graphene::chain::issue_fundraise_publish_feed_operation,     
            (fee)                            
            (issuer)         
            (fundraise_id)
            (period)          
            (fundraise_price)        
            (extensions)
          )  
   FC_REFLECT( graphene::chain::finance_paramers_update_operation,     
            (fee)                            
            (issuer)         
            (options)                   
            (extensions)
          )  

          
FC_REFLECT( graphene::chain::buy_fundraise_create_operation,
                          (fee) 
                          (issuer) 
                          (fundraise_id)
                          (period)
                          (amount)                          
                          (bimmediately)                  
                          (memo)         
                          (extensions)                                                   
                          )   
FC_REFLECT( graphene::chain::buy_fundraise_enable_operation,
                          (fee) 
                          (issuer) 
                          (fundraise_id)
                          (benable)          
                          (extensions) 
                          )    

FC_REFLECT( graphene::chain::exchange_feed_type,
              (ex_price) 
              (min_amount) 
              (max_amount) 
              (step_amount) 
              )
FC_REFLECT( graphene::chain::exchange_bancor_type, (balance)(total_bancor))
FC_REFLECT( graphene::chain::exchange_market_type, (total_market) )
FC_REFLECT_TYPENAME( graphene::chain::exchange_type )
FC_REFLECT( graphene::chain::sell_exchange_create_operation,
                          (fee) 
                          (issuer) 
                          (need_auth)
                          (trust_auth)
                          (sell) 
                          (buy) 
                          (url)                           
                          (initializer)
                          (extensions) 
                          )    
FC_REFLECT( graphene::chain::sell_exchange_update_operation,
                          (fee) 
                          (need_auth)
                          (trust_auth)
                          (issuer) 
                          (sell_id) 
                          (sell) 
                          (buy) 
                          (url)                           
                          (op_type) 
                          (update_op) 
                          (extensions) 
                          )    
FC_REFLECT( graphene::chain::sell_exchange_remove_operation,
                          (fee) 
                          (issuer) 
                          (sell_id)
                          (extensions) 
                          )    
FC_REFLECT( graphene::chain::buy_exchange_create_operation,
                          (fee) 
                          (issuer) 
                          (buy) 
                          (sell_id) 
                          (min_price)  
                          (memo)         
                          (extensions) 
                          )    
FC_REFLECT( graphene::chain::buy_exchange_update_operation,
                          (fee) 
                          (issuer) 
                          (buy_id) 
                          (buy) 
                          (min_price)  
                          (memo)         
                          (extensions) 
                          )    
FC_REFLECT( graphene::chain::buy_exchange_remove_operation,
                          (fee) 
                          (issuer) 
                          (buy_id)
                          (extensions) 
                          )    







