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
#include <graphene/business/protocol/bitlender_option_ops.hpp>
#include <string>
#include <vector>
 


namespace graphene { namespace chain {



//发标
struct bitlender_loan_operation : public base_operation
{
  struct fee_parameters_type
  {

    uint64_t fee = 10000;
    uint32_t price_per_kbyte = 1000;
  };

  asset fee;
  account_id_type issuer;      //借款人
  account_id_type carrier;     //运营商
  std::string order;           //借款单号
  asset amount_to_loan;        //法币
  uint32_t loan_period;        //借款周期
  uint32_t interest_rate = 0;  //借款利息
  uint32_t repayment_type = 1; //还款方式

  asset amount_to_collateralize; //抵押币
  uint32_t collateral_rate = 0;  //抵押率 GRAPHENE_COLLATERAL_RATIO_DENOM
  uint32_t bid_period = 0;       //挂单秒数
  optional<std::string> memo;
  

  extensions_type extensions;

  account_id_type fee_payer() const { return issuer; }
  void validate() const;
  share_type calculate_fee(const fee_parameters_type &k) const;
   };


   //应标
   struct bitlender_invest_operation : public base_operation
   {
      struct fee_parameters_type  {   uint64_t fee = 10000; };

      asset                    fee;
      account_id_type          issuer;            //投资人
      account_id_type          loan_id;           //借款人
      account_id_type          carrier;           //运营商
      bitlender_order_id_type  order_id;
      asset                    amount_to_invest;  //法币 

      extensions_type          extensions;  
      
      account_id_type          fee_payer()const { return issuer; }
      void                     validate()const;
      share_type               calculate_fee(const fee_parameters_type& k) const;
   };
   //还息
   struct bitlender_repay_interest_operation : public base_operation
   {
      
      struct fee_parameters_type {  uint64_t fee = 10000; };
      
      asset                    fee;
      account_id_type          issuer;
      bitlender_order_id_type  order_id;
      uint32_t                 repay_period;    

       extensions_type          extensions;  

      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type& k) const  { return k.fee; };
    };

   //逾期还息
   struct bitlender_overdue_interest_operation : public base_operation
   {      
      struct fee_parameters_type {  uint64_t fee = 10000; };
      
      asset                    fee;
      account_id_type          issuer;
      bitlender_order_id_type  order_id;
      uint32_t                 repay_period;

       extensions_type          extensions;  

      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type& k) const  { return k.fee; };
    };
    //处理逾期利息
    struct bitlender_recycle_interest_operation : public base_operation
    {
      struct fee_parameters_type {  uint64_t fee = 10000;;uint32_t price_per_kbyte = 1000; };
 
      asset                    fee;
      account_id_type          issuer;
      bitlender_order_id_type  order_id;
      uint32_t                 repay_period;
      optional<string>         memo;

      extensions_type          extensions;  

      account_id_type fee_payer()const { return issuer; }
      void            validate() const;
      share_type      calculate_fee(const fee_parameters_type& k) const;
    };
    


      //还本
    struct bitlender_repay_principal_operation : public base_operation
    {
      struct fee_parameters_type {   uint64_t fee = 10000;      };

      asset                    fee;
      account_id_type          issuer;
      bitlender_order_id_type  order_id;

       extensions_type          extensions;  
      
      account_id_type fee_payer()const { return issuer; }
      void            validate()const {};
      share_type      calculate_fee(const fee_parameters_type& k) const  { return k.fee; };
   };
   //提前还款
    struct bitlender_pre_repay_principal_operation : public base_operation
    {
      struct fee_parameters_type {   uint64_t fee = 10000;      };

      asset                    fee;
      account_id_type          issuer;
      bitlender_order_id_type  order_id;

       extensions_type          extensions;  
      
      account_id_type fee_payer()const { return issuer; }
      void            validate()const {};
      share_type      calculate_fee(const fee_parameters_type& k) const  { return k.fee; };
   };
   //逾期还款
   struct bitlender_overdue_repay_principal_operation : public base_operation
   {
      
      struct fee_parameters_type {  uint64_t fee = 10000; };

      asset                    fee;
      account_id_type          issuer;
      bitlender_order_id_type  order_id;
      
       extensions_type          extensions;  

      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type& k) const  { return k.fee; };
   };
   //补仓
   struct bitlender_add_collateral_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 10000;};
 
      asset                    fee;
      account_id_type          issuer;
      bitlender_order_id_type  order_id;
      asset                    collateral;
      uint32_t                 collateral_rate = 0;  

      extensions_type          extensions;  

      account_id_type fee_payer()const { return issuer; }
      void            validate() const;
      share_type      calculate_fee(const fee_parameters_type& k)const { return k.fee; }
   };
   //处理不良资产
   struct bitlender_recycle_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 10000;uint32_t price_per_kbyte = 1000;  };
 
      asset                    fee;
      account_id_type          issuer;
      bitlender_order_id_type  order_id;
      share_type               asset_pay;
      optional<string>         memo;

      extensions_type          extensions;  

      account_id_type fee_payer()const { return issuer; }
      void            validate() const;
      share_type      calculate_fee(const fee_parameters_type& k) const;
   };

   struct bitlender_test_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 10000;};
 
      asset                    fee;
      account_id_type          issuer;
      object_id_type           order_id;
      uint32_t                 ntype;
      uint32_t                 nvalue;

       extensions_type          extensions;  

      account_id_type fee_payer()const { return issuer; }
      void            validate() const;
      share_type      calculate_fee(const fee_parameters_type& k)const { return k.fee; }
   };
   enum fill_object_operation_type
   {
     fill_object_finish,
     fill_object_order,
     fill_object_invest,
     fill_object_issue_fundraise,
     fill_object_buy_fundraise,
     fill_object_sell_exchange,
     fill_object_buy_exchange,
     fill_object_worker,
     fill_object_proposal,
     fill_object_withdraw_permission,
     fill_object_locktoken,
     fill_object_limitorder,
   };
   struct fill_object_history_operation : public base_operation
   {      
     struct fee_parameters_type { };
    
     asset                    fee;
     object_id_type           id;       
     account_id_type          issuer;
     int                      itype = 0;
     uint32_t                 idatatype = 0;
     object_id_type           src_id;  
     object_id_type           dest_id;  
     vector<char>             data;

     account_id_type fee_payer()const { return account_id_type(); }     
     share_type      calculate_fee(const fee_parameters_type& k)const { return 0; }
     share_type      get_fee(const fee_parameters_type& ,uint32_t )const {return 0;}
   };
    struct bitlender_setautorepayer_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 10000;};
 
      asset                    fee;
      account_id_type          issuer;
      bitlender_order_id_type  order_id;  
      bool                     bset;  

       extensions_type          extensions;  

      account_id_type fee_payer()const { return issuer; }
      void            validate() const;
      share_type      calculate_fee(const fee_parameters_type& k)const { return k.fee; }
   };

   struct bitlender_autorepayment_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 10000;};
 
      asset                    fee;     
      account_id_type          issuer;
      bitlender_order_id_type  order_id;
      uint32_t                 repay_period;

      extensions_type          extensions;  
   
      account_id_type fee_payer()const { return issuer; }
      void            validate() const;
      share_type      calculate_fee(const fee_parameters_type& k)const { return k.fee; }
   };
   struct bitlender_remove_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 10000;};
 
      asset                    fee;     
      account_id_type          issuer;
      bitlender_order_id_type  order_id;      
      extensions_type extensions;

      account_id_type fee_payer()const { return issuer; }
      void            validate() const;
      share_type      calculate_fee(const fee_parameters_type& k)const { return k.fee; }
   };

   struct bitlender_squeeze_operation : public base_operation
   {
     struct fee_parameters_type {   uint64_t fee = 0; };

      asset                    fee;
      account_id_type          issuer;
      bitlender_order_id_type  order_id;
     bool                     squeeze_state = 0;
      account_id_type fee_payer()const { return issuer; }
   };

} } // graphene::chain


FC_REFLECT( graphene::chain::bitlender_loan_operation::fee_parameters_type,  (fee)(price_per_kbyte))
FC_REFLECT( graphene::chain::bitlender_invest_operation::fee_parameters_type, (fee))
FC_REFLECT( graphene::chain::bitlender_repay_interest_operation::fee_parameters_type,  (fee) )
FC_REFLECT( graphene::chain::bitlender_overdue_interest_operation::fee_parameters_type,  (fee) )
FC_REFLECT( graphene::chain::bitlender_recycle_interest_operation::fee_parameters_type,  (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::bitlender_repay_principal_operation::fee_parameters_type,  (fee) )
FC_REFLECT( graphene::chain::bitlender_pre_repay_principal_operation::fee_parameters_type,  (fee) )
FC_REFLECT( graphene::chain::bitlender_overdue_repay_principal_operation::fee_parameters_type, (fee) )  
FC_REFLECT( graphene::chain::bitlender_add_collateral_operation::fee_parameters_type,  (fee) )  
FC_REFLECT( graphene::chain::bitlender_recycle_operation::fee_parameters_type,  (fee)(price_per_kbyte) )  
FC_REFLECT( graphene::chain::bitlender_setautorepayer_operation::fee_parameters_type,  (fee) ) 
FC_REFLECT( graphene::chain::bitlender_autorepayment_operation::fee_parameters_type, (fee) ) 
FC_REFLECT( graphene::chain::bitlender_remove_operation::fee_parameters_type,  (fee) ) 
FC_REFLECT( graphene::chain::bitlender_squeeze_operation::fee_parameters_type,  (fee)  ) 
FC_REFLECT( graphene::chain::bitlender_test_operation::fee_parameters_type,  (fee) )
FC_REFLECT( graphene::chain::fill_object_history_operation::fee_parameters_type,  ) // VIRTUAL


FC_REFLECT( graphene::chain::bitlender_loan_operation,
                          (fee) 
                          (issuer) 
                          (carrier)
                          (order)                           
                          (amount_to_loan)                          
                          (loan_period)          
                          (interest_rate)                 
                          (repayment_type) 
                          (amount_to_collateralize)                                                     
                          (collateral_rate) 
                          (bid_period)                           
                          (memo)                          
                          (extensions) 
                          )  
FC_REFLECT( graphene::chain::bitlender_invest_operation,
                         (fee)
                         (issuer)
                         (loan_id)
                         (carrier)
                         (order_id)
                         (amount_to_invest)                         
                         (extensions) 
                         )
FC_REFLECT( graphene::chain::bitlender_repay_interest_operation, 
                         (fee)
                         (issuer)
                         (order_id)                        
                         (repay_period)
                         (extensions)
                         )
FC_REFLECT( graphene::chain::bitlender_overdue_interest_operation, 
                         (fee)
                         (issuer)
                         (order_id)
                         (repay_period)
                         (extensions)
                         )                        
FC_REFLECT( graphene::chain::bitlender_recycle_interest_operation, 
                         (fee)
                         (issuer)
                         (order_id)
                         (repay_period)
                         (memo)
                         (extensions)
                         )    

FC_REFLECT( graphene::chain::bitlender_repay_principal_operation, 
                         (fee)
                         (issuer)
                         (order_id)
                         (extensions)
                        )
FC_REFLECT( graphene::chain::bitlender_pre_repay_principal_operation, 
                         (fee)
                         (issuer)
                         (order_id)
                         (extensions)
                        )     
FC_REFLECT( graphene::chain::bitlender_overdue_repay_principal_operation, 
                         (fee)
                         (issuer)
                         (order_id)
                         (extensions)
                         )
FC_REFLECT( graphene::chain::bitlender_add_collateral_operation, 
                         (fee)
                         (issuer)
                         (order_id)
                         (collateral) 
                         (collateral_rate)
                         (extensions)
                         ) 
FC_REFLECT( graphene::chain::bitlender_recycle_operation, 
                         (fee)
                         (issuer)
                         (order_id)
                         (asset_pay)
                         (memo)
                         (extensions)
                         ) 
                         
FC_REFLECT( graphene::chain::bitlender_test_operation, 
                         (fee)
                         (issuer)
                         (order_id)
                         (ntype)
                         (nvalue)
                         (extensions)
                         ) 
                         
FC_REFLECT( graphene::chain::fill_object_history_operation, 
                         (fee)     
                         (id)     
                         (issuer)
                         (itype)
                         (idatatype)
                         (src_id)
                         (dest_id)
                         (data)                                              
                         ) 
FC_REFLECT( graphene::chain::bitlender_setautorepayer_operation, 
                         (fee)
                         (issuer)
                         (order_id)        
                         (bset)     
                         (extensions)            
                         ) 


FC_REFLECT( graphene::chain::bitlender_autorepayment_operation, 
                         (fee)
                         (issuer)
                         (order_id)        
                         (repay_period)   
                         (extensions)              
                         )      
FC_REFLECT( graphene::chain::bitlender_remove_operation, 
                         (fee)
                         (issuer)
                         (order_id)                                               
                         (extensions)  
                         )            
FC_REFLECT( graphene::chain::bitlender_squeeze_operation,  
                         (fee)                     
                         (issuer)
                         (order_id)      
                         (squeeze_state)  
                         )                                   
   
