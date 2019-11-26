/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 
 */
#pragma once

#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/memo.hpp>

#define btlender_rate_type 1000

namespace graphene { namespace chain {

class asset_exchange_feed_object;
bool is_valid_product(const string &product);
/**
    * @ingroup operations
    */

 
 enum   fee_mode_type  
 {
   fee_invest,
   fee_loan 
 };
 enum   loan_period_type  
 {
   loan_null,
   loan_month, //月贷
   loan_year,//年贷
   loan_day,//日贷
 };
 enum   loan_mode_type  
 {   
   loan_bitcash, 
   loan_direct,   
 };
 enum   repayment_mode  
 {   
   repayment_null, 
   repayment_first_interest, //先息后本
   repayment_total_payment, //一次性还本付息
   repayment_equal_total_payment , //等额本息
   repayment_equal_principal, //等额本金

 };

 
struct bitlender_rate
{
  uint32_t interest_rate = 1; //利率1(利率基数10000)
};
#define  loan_samecarrier             0x01  
#define  loan_onecarrier              0x02  

#define  loan_mode_all                (loan_samecarrier | loan_onecarrier)  

#define  lender_desc                  "LENDER"    

 
struct bitlender_key
{
   bitlender_key(): repayment_period_uint(loan_month), repayment_type(repayment_first_interest),loan_mode(0) {}
   bitlender_key(uint32_t u,uint32_t t,uint32_t l=0,uint32_t o=0) : repayment_period_uint(u), repayment_type(t),loan_mode(l),op_type(o) {}
   bitlender_key(uint32_t u) { set_type(u); }
   uint8_t   repayment_period_uint   = loan_month;//期限单位 1 月 2  年    3 日   
   uint8_t   repayment_type          = repayment_first_interest;//还款方式
   uint8_t   loan_mode               = 0;//借款模式
   uint8_t   op_type                 = 0;
 
   void            validate() const;   
   void            is_option() const;
   void            set_type(uint32_t u);
   uint32_t        get_type() const;
   void            get_force_loan_mode();
   string          get_index() const;
   vector<string>  get_index(const optional<bitlender_key> keyin) const;

   const bool can_loan() const;
   const bool can_invest() const;
   const bool can_repay_interest() const;
   const bool can_overdue_interest() const;
   const bool can_recycle_interest() const;
   const bool can_repay_principal() const;
   const bool can_pre_repay_principal() const;
   const bool can_overdue_repay_principal() const;
   const bool can_add_collateral() const;
   const bool can_recycle() const;
   const bool can_setautorepayer() const;
   const bool can_autorepayment() const;
   const bool can_remove() const;
   const bool can_squeeze() const;
}; 

struct bitlender_index
{
   uint8_t   repayment_period_uint   = loan_month;//期限单位 1 月 2  年    3 日   
   uint8_t   repayment_type          = repayment_first_interest;//还款方式
   bitlender_index()
   {
     repayment_period_uint   = loan_month;//期限单位 1 月 2  年    3 日   
     repayment_type          = repayment_first_interest;//还款方式
   }
   bitlender_index(const bitlender_key &other)
   {
      repayment_period_uint = other.repayment_period_uint;
      repayment_type        = other.repayment_type;      
   }
};
inline bool operator < (const bitlender_index &a, const bitlender_index &b) 
{
   return std::tie( a.repayment_type, a.repayment_period_uint ) <  std::tie( b.repayment_type, b.repayment_period_uint);
}
inline bool operator == (const bitlender_index &a,const bitlender_index &b) 
{
   return std::tie( a.repayment_type, a.repayment_period_uint ) ==  std::tie( b.repayment_type, b.repayment_period_uint);
}

class bitlender_paramers
{
   public:
       uint32_t               max_rate_month                 =   36;      //最大投资周期
       uint32_t               max_platform_service_rate      =   500;     //最大服务费率(基数1000) 
       uint32_t               max_carrier_service_rate       =   500;     //最大运营商服务费率(基数1000) 
       uint32_t               max_risk_margin_rate           =   100;     //最高风险准备金率(基数1000)
       uint32_t               min_platform_service_rate      =   16;      //最小平台费用比例(基数100)
       uint32_t               max_repayment_rate             =   3600/12; //最大利率(利率基数10000)
       extensions_type        extensions;

       void validate(const bitlender_key &etype) const;
       void set_type(const bitlender_key &etype) ;
};

//说明，各个体系独立喂价。喂价函数在各个体系里面

class bitlender_feed_option
{
 public:
   uint32_t flags = 1; // 0 feeder 1 author 2 witness 3 commit
   /// Time before a price feed expires
   uint32_t feed_lifetime_sec = GRAPHENE_DEFAULT_PRICE_FEED_LIFETIME;
   /// Minimum number of unexpired feeds required to extract a median feed from
   uint8_t minimum_feeds = GRAPHENE_DEFAULT_MINIMUM_FEEDS;
   /// This is the delay between the time a long requests settlement and the chain evaluates the settlement
   uint32_t force_settlement_delay_sec = GRAPHENE_DEFAULT_FORCE_SETTLEMENT_DELAY;
   /// This is the percent to adjust the feed price in the short's favor in the event of a forced settlement
   uint16_t force_settlement_offset_percent = GRAPHENE_DEFAULT_FORCE_SETTLEMENT_OFFSET;
   /// Force settlement volume can be limited such that only a certain percentage of the total existing supply
   /// of the asset may be force-settled within any given chain maintenance interval. This field stores the
   /// percentage of the current supply which may be force settled within the current maintenance interval. If
   /// force settlements come due in an interval in which the maximum volume has already been settled, the new
   /// settlements will be enqueued and processed at the beginning of the next maintenance interval.
   uint16_t maximum_force_settlement_volume = GRAPHENE_DEFAULT_FORCE_SETTLEMENT_MAX_VOLUME;

   flat_set<account_id_type> feeders;

   bitlender_feed_option &operator=(const bitlender_feed_option &o)
   {
      flags = o.flags;
      feed_lifetime_sec = o.feed_lifetime_sec;
      minimum_feeds = o.minimum_feeds;
      force_settlement_delay_sec = o.force_settlement_delay_sec;
      force_settlement_offset_percent = o.force_settlement_offset_percent;
      maximum_force_settlement_volume = o.maximum_force_settlement_volume;
      feeders = o.feeders;
      return *this;
   }
   void copy_to(asset_exchange_feed_object &o) const;

   void validate() const;
   };

  
   class bitlender_option
   {
    public:
      account_id_type issuer_rate;//修改利率人

      uint32_t   max_repayment_period    = 6;//最大借款期限    
      uint32_t   repayment_period_uint  = 1;//参数类型
      uint32_t   max_bid_time            = 7*24*3600;//最大投标期限    
      uint32_t   max_overdue_period      = 2;//最大利息逾期期数，超过这个期数，就是本金逾期了
      uint32_t   notify_overdue_time     = 7*24*3600;  //逾期提醒时间    
      uint32_t   overdue_expiration_time = 7*24*3600; //逾期时间         
      uint32_t   repayment_type          = 1;//还款方式         
      uint32_t   repayment_date          = 0;//还款日期     
      share_type min_invest_amount       = 10000;//最小投资金额
      share_type min_loan_amount         = 100000;//最小借款金额
      share_type min_invest_increase_range = 10000;//最小投资增加额
      share_type min_loan_increase_range   = 10000;//最小借款增加额
      uint32_t   max_repayment_rate = 3600/12;        //最大利率
      share_type max_interest_rate  = 500000;         //利息封顶值

      uint32_t   risk_margin_rate                = 2;      //风险准备金率(基数1000)
      share_type max_risk_margin                 = 50000;  //风险准备金封顶

      uint32_t   carrier_service_charge_rate     = 0;     //运营商服务费费率(基数1000)     
      share_type max_carrier_service_charge      = 50000; //运营商服务费封顶   
      uint32_t   carrier_service_loan_carrier    = 50;    //借方运营商 (基数100)
      uint32_t   carrier_service_invest_carrier  = 50;    //投资方运营商

      uint32_t   platform_service_charge_rate     = 20;    //服务费费率(基数1000)    
      share_type max_platform_service_charge_rate = 50000; //服务费封顶

      uint32_t   platform_service_loan_carrier   = 28;  //借方运营商 (基数100)
      uint32_t   platform_service_invest_carrier = 28;  //投资方运营商
      uint32_t   platform_service_loan_refer     = 10;  //借方推荐人
      uint32_t   platform_service_invest_refer   = 10;  //投资方推荐人
      uint32_t   platform_service_gateway        = 8;   //网关
      uint32_t   platform_service_platform       = 16;  //平台



      uint32_t   overdue_liquidation_rate              = 5;//本金逾期违约金率(基数100)
      uint32_t   overdue_penalty_interest_rate         = 4;//利息逾期违约金率(基数100)
      uint32_t   early_repayment_liquidation_rate      = 50;//提前还款违约金率(基数100)      
      uint32_t   overdue_penalty_date_interest_rate    = 4; //利息逾期按天违约金率(基数10000) 
      

      uint32_t   principal_guarantee_rate             = 90; //处理不良资产，或者平仓时候，投资者保本例

      account_id_type            platform_id;         //平台     
      flat_set<account_id_type>  carriers;            //运营商 
      flat_set<asset_id_type>    allowed_collateralize; //允许抵押的币种

      uint32_t                   ahead_autopay_time = 2 * 24 * 3600; //提前还款天数

      uint32_t                   time_zone = 0;      //时区   

      extensions_type            extensions;      
      
      void validate(const bitlender_paramers &paramers) const;
      void validate_precision(uint32_t precision)  const;
      void set_type(const bitlender_key &etype);
      void copy(const bitlender_option &obj);
      
   };
   class bitlender_option_1903 : public bitlender_option
   {
     public: 
      flat_set<account_id_type>          loan_carriers;
      flat_set<account_id_type>          invest_carriers;
   };
   struct bitlender_paramers_update_operation : public base_operation
   {
      struct fee_parameters_type {   uint64_t fee             = 10000;     };

      asset                    fee;       
      account_id_type          issuer;    
      bitlender_paramers       options;    
      uint32_t                 repayment_type = 1;//还款方式    

      extensions_type          extensions;
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type& k) const;
      void            get_required_active_authorities( flat_set<account_id_type>& a )const 
      {
        a.clear();
        a.insert(GRAPHENE_COMMITTEE_ACCOUNT);
      }
      
   };

   struct bitlender_option_create_operation : public base_operation
   {
      struct fee_parameters_type { 
         
         uint64_t fee             = 10000;
         uint32_t price_per_kbyte = 100; /// only required for large memos.
      };
      asset           fee;
      asset_id_type   asset_id;  
      account_id_type issuer; 
      string          sproduct;     

      bitlender_option options;
      uint32_t         fee_mode = 0;  

       extensions_type   extensions;

      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      share_type      calculate_fee( const fee_parameters_type& k ) const;
      void get_required_active_authorities( flat_set<account_id_type>& a )const 
      {   
        a.clear();     
         a.insert(GRAPHENE_COMMITTEE_ACCOUNT);
      }
   }; 
   struct bitlender_option_author_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee             = 10000;    };

      asset                      fee;      
      account_id_type            issuer; 
      bitlender_option_id_type   option_id;
      flat_set<account_id_type>  authors;
      uint32_t                   weight_threshold;       
      
      extensions_type   extensions;
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      share_type      calculate_fee( const fee_parameters_type& k ) const {return k.fee;}
      void get_required_active_authorities( flat_set<account_id_type>& a )const 
      {    
        a.clear();    
         a.insert(GRAPHENE_COMMITTEE_ACCOUNT);
      }
   }; 
   
      
   struct bitlender_option_update_operation : public base_operation
   {
      struct fee_parameters_type { 
         
         uint64_t fee             = 10000;
         uint32_t price_per_kbyte = 100;
      };

      asset                    fee;       
      account_id_type          issuer; 
      bitlender_option_id_type option_id;
      account_id_type          author;    
     
      bitlender_option         options;    

      extensions_type           extensions;
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type& k) const;
      void            get_required_active_authorities( flat_set<account_id_type>& a )const 
      {
        a.clear();
        a.insert(author);
      }
      
   };
    
   //因为费用模式，需要理事会批准，所以单独拿出来
      struct bitlender_option_fee_mode_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee             = 10000;   };

      asset                    fee;       
      account_id_type          issuer;       
      bitlender_option_id_type option_id;
      uint32_t                 fee_mode;      

      extensions_type          extensions;
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;      
      void            get_required_active_authorities( flat_set<account_id_type>& a )const 
      {
        a.clear();
        a.insert(GRAPHENE_COMMITTEE_ACCOUNT);
      }
      
   };
   struct bitlender_option_stop_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee             = 10000;   };

      asset                    fee;       
      account_id_type          issuer;       
      bitlender_option_id_type option_id;
      account_id_type          author;
      bool                     stop;

      extensions_type           extensions;
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;      
      void            get_required_active_authorities( flat_set<account_id_type>& a )const 
      {
        a.clear();
        a.insert(author);
      }
      
   };
   
   struct bitlender_rate_update_operation : public base_operation
   {
      struct fee_parameters_type { 
         
         uint64_t fee             = 10000;
         uint32_t price_per_kbyte = 1000;
      };

      asset                             fee;
      account_id_type                   issuer;    
      bitlender_option_id_type          option_id;      

      flat_map<uint32_t,bitlender_rate> interest_rate_add;
      vector<uint32_t>                  interest_rate_remove;
      //bitlender_key                     repayment_type;
       extensions_type                  extensions;
      account_id_type fee_payer()const { return issuer; }
      void validate() const;
      share_type calculate_fee(const fee_parameters_type &k) const;    
   };
  
 




   struct bitlender_update_feed_producers_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 20 * GRAPHENE_BLOCKCHAIN_PRECISION; };

      asset           fee;
      account_id_type issuer;
      bitlender_option_id_type option_id;
      account_id_type author;
      

      bitlender_feed_option     new_feed_option;
      extensions_type           extensions;

      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      void            get_required_active_authorities( flat_set<account_id_type>& a )const 
      {
        a.clear();
        a.insert(author);
      }
   };

    
   struct bitlender_publish_feed_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 10; };

      asset                  fee; ///< paid for by publisher
      account_id_type        publisher;  
      asset_id_type          asset_id; ///< asset for which the feed is published    
      price_feed             feed;

      extensions_type        extensions;

      account_id_type fee_payer()const { return publisher; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type &k) const;   
   };



} } // graphene::chain          

 
FC_REFLECT( graphene::chain::bitlender_rate,
            (interest_rate)
           )

FC_REFLECT_ENUM( graphene::chain::fee_mode_type,
                (fee_invest)
                (fee_loan)
              )   
FC_REFLECT_ENUM( graphene::chain::loan_period_type,
                (loan_null)
                (loan_month)
                (loan_year)
                (loan_day)
              )     
FC_REFLECT_ENUM( graphene::chain::repayment_mode,
                (repayment_null)
                (repayment_first_interest)              
                (repayment_total_payment)              
                (repayment_equal_total_payment )              
                (repayment_equal_principal)               
              )      
FC_REFLECT_ENUM( graphene::chain::loan_mode_type,
                (loan_bitcash)
                (loan_direct)
              )   
FC_REFLECT( graphene::chain::bitlender_index,
          (repayment_period_uint)
          (repayment_type)          
           )              
FC_REFLECT( graphene::chain::bitlender_key,
          (repayment_period_uint)
          (repayment_type)
          (loan_mode)
          (op_type)
           )
FC_REFLECT( graphene::chain::bitlender_paramers,
          (max_rate_month)
          (max_platform_service_rate)
          (max_carrier_service_rate)
          (max_risk_margin_rate)
          (min_platform_service_rate)
          (max_repayment_rate)
          (extensions)
           )
    
 

FC_REFLECT( graphene::chain::bitlender_feed_option,
            (flags)
            (feed_lifetime_sec)
            (minimum_feeds)
            (force_settlement_delay_sec)
            (force_settlement_offset_percent)         
            (maximum_force_settlement_volume)    
            (feeders)     
          )             

FC_REFLECT( graphene::chain::bitlender_option,
            (issuer_rate)
            (max_repayment_period)
            (repayment_period_uint)
            (max_bid_time)          
            (max_overdue_period)
            (notify_overdue_time)
            (overdue_expiration_time)            
            (repayment_type)   
            (repayment_date)         
            (min_invest_amount)                    
            (min_loan_amount)
            (min_invest_increase_range)
            (min_loan_increase_range)
            (max_repayment_rate)
            (max_interest_rate)
            (risk_margin_rate)     
            (max_risk_margin)              
            (carrier_service_charge_rate)         
            (max_carrier_service_charge)    
            (carrier_service_loan_carrier)     
            (carrier_service_invest_carrier)   
            (platform_service_charge_rate)     
            (max_platform_service_charge_rate) 
            (platform_service_loan_carrier)  
            (platform_service_invest_carrier) 
            (platform_service_loan_refer)    
            (platform_service_invest_refer)   
            (platform_service_gateway)        
            (platform_service_platform)       

            (overdue_liquidation_rate)
            (overdue_penalty_interest_rate)
            (early_repayment_liquidation_rate)            
            (overdue_penalty_date_interest_rate)
            (principal_guarantee_rate)
            (platform_id)      
          //  (gateways)
            (carriers)
            (allowed_collateralize)
            (ahead_autopay_time)
            (time_zone)   
            (extensions)
            )  

FC_REFLECT_DERIVED( graphene::chain::bitlender_option_1903, (graphene::chain::bitlender_option),
            (loan_carriers)          
            (invest_carriers)
            )  

FC_REFLECT( graphene::chain::bitlender_option_create_operation::fee_parameters_type,  (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::bitlender_option_author_operation::fee_parameters_type,  (fee) )
FC_REFLECT( graphene::chain::bitlender_paramers_update_operation::fee_parameters_type,  (fee) )


FC_REFLECT( graphene::chain::bitlender_option_update_operation::fee_parameters_type,  (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::bitlender_option_stop_operation::fee_parameters_type,  (fee) )
FC_REFLECT( graphene::chain::bitlender_rate_update_operation::fee_parameters_type,  (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::bitlender_update_feed_producers_operation::fee_parameters_type,  (fee) )
FC_REFLECT( graphene::chain::bitlender_publish_feed_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::bitlender_option_fee_mode_operation::fee_parameters_type, (fee) )




FC_REFLECT( graphene::chain::bitlender_option_create_operation,   
            (fee)         
            (asset_id)                 
            (issuer)     
            (sproduct)     
            (options)
            (fee_mode)
            (extensions)
          )
 

FC_REFLECT( graphene::chain::bitlender_option_author_operation,   
            (fee)         
            (issuer)  
            (option_id)               
            (authors)     
            (weight_threshold)     
            (extensions)          
          )     

FC_REFLECT( graphene::chain::bitlender_option_update_operation,     
            (fee)                            
            (issuer)  
            (option_id)     
            (author)                        
            (options)          
            (extensions)
          )

FC_REFLECT( graphene::chain::bitlender_option_stop_operation,     
            (fee)                            
            (issuer)  
            (option_id)     
            (author)       
            (stop)                             
            (extensions)
          )

FC_REFLECT( graphene::chain::bitlender_option_fee_mode_operation,     
            (fee)                            
            (issuer)  
            (option_id)     
            (fee_mode)                 
            (extensions)
          )


FC_REFLECT( graphene::chain::bitlender_paramers_update_operation,     
            (fee)                            
            (issuer)                                  
            (options)           
            (repayment_type)
            (extensions)
          )            

FC_REFLECT( graphene::chain::bitlender_rate_update_operation,     
            (fee)                            
            (issuer) 
            (option_id)                     
            (interest_rate_add)      
            (interest_rate_remove)
            //(repayment_type)
            (extensions)
            )   
 


FC_REFLECT( graphene::chain::bitlender_update_feed_producers_operation,     
            (fee)                            
            (issuer) 
            (option_id)
            (author)            
            (new_feed_option)                  
            (extensions)
          )   

FC_REFLECT( graphene::chain::bitlender_publish_feed_operation,     
            (fee)                            
            (publisher)                  
            (asset_id)               
            (feed)          
            (extensions)
          )    
