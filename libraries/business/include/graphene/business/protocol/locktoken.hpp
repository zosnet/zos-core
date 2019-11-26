#pragma once
#include <graphene/chain/protocol/base.hpp>

enum locktoken_type
{
   locktoken_dy = 0,
   locktoken_fixed = 1,
   locktoken_node = 2,
   locktoken_awardnode = 3,
   locktoken_buynode = 4,
   locktoken_typemax,
};
enum locktoken_mode
{
   locktoken_tofixed = 1,
   locktoken_tody = 2,
   locktoken_remove = 3,
   locktoken_modemax,
};
enum locktoken_paymode
{
   locktoken_none = 0,
   locktoken_core = 1,
   locktoken_self = 2
};
#define locktoken_options_version 9

namespace graphene { namespace chain {


   struct locktoken_options {
      uint32_t                                           version = 0;   //版本号
      locktoken_option_id_type                           op_id;
      uint32_t                                           lock_mode = 0; //锁仓模式

      flat_map<uint32_t,flat_map<share_type,uint32_t>>  locktoken_rates;                       //利息  ，10000为单位
      share_type                                        max_rate = 100000000000000;            //最大利息
      uint32_t                                          vesting_seconds = (60 * 60 * 24 * 20); //利息或者退款锁定时间

      share_type                                        locktoken_min = 1;               //最小值
      share_type                                        locktoken_max = 100000000000000; //最大值      
     
      asset_id_type                                     pay_asset;

      account_id_type                                   payer;           //付款人
      account_id_type                                   carrier;         //运营商  

      share_type                                        max_supply = GRAPHENE_MAX_SHARE_SUPPLY;            //最大发行量

      vector<asset_id_type>                             buy_asset_whitelist;// 购买节点允许资产

      void validate() const;
   };

   struct locktoken_option_operation : public base_operation
   {
      struct fee_parameters_type {         
         uint64_t fee = 1 * GRAPHENE_BLOCKCHAIN_PRECISION; 
      };

      asset            fee;
      account_id_type  issuer;       //必须和资产创建者相同
      asset_id_type    asset_lock;   //锁定资产
      uint32_t         op_type = 0;  //1 新建 0 修改 2 付款人同意

      fc::string       sParam;       //参数

      extensions_type  extensions;

      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type& k)const;  
   };

   
   struct locktoken_create_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 1*GRAPHENE_BLOCKCHAIN_PRECISION;};

      asset                fee;
      account_id_type      issuer;  //锁仓人
      asset                locked;  //锁仓金额
      account_id_type      to;      //受益人，或者发起运营商id
      uint32_t             period;  //锁仓期限，月为单位
      uint32_t             type = 0;       //锁仓类型 0 活期 1 定期 2 节点
      uint32_t             autolock = 0;   //定期到后，是否自动转，1 自动转定期，2 自动转活期，3 自动取消

      extensions_type      extensions;

      account_id_type   fee_payer()const { return issuer; }
      void validate() const;
      share_type calculate_fee(const fee_parameters_type &k) const;
   };
   struct locktoken_update_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 1*GRAPHENE_BLOCKCHAIN_PRECISION;};

      asset                        fee;
      account_id_type              issuer;       //操作人
      locktoken_id_type            locktoken_id; //单子
      uint32_t                     op_type = 0; // 1 定期活期互转，3 补仓 4 减仓 5 修改定期期限
      optional<asset>              locked;   //补减仓金额  
      optional<account_id_type>    to;       //受益人，暂时不用    
      optional<uint32_t>           type;     //锁仓类型 0 活期 1 定期    
      optional<uint32_t>           period;   //锁仓期限，月为单位
      optional<uint32_t>           autolock; //定期到后，是否自动转，1 自动转定期，2 自动转活期，3 自动取消
      
      extensions_type              extensions;

      account_id_type   fee_payer()const { return issuer; }
      void validate() const;
      share_type calculate_fee(const fee_parameters_type &k) const;
   };
   struct locktoken_node_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 1*GRAPHENE_BLOCKCHAIN_PRECISION;};

      asset                        fee;
      account_id_type              issuer;  //用户
      locktoken_id_type            locktoken_id; //单子
      uint32_t                     op_type = 0; //  
      account_id_type              to;
      asset                        to_amount;
      uint32_t                     period;  

      extensions_type              extensions;

      account_id_type   fee_payer()const { return issuer; }
      void validate() const;
      share_type calculate_fee(const fee_parameters_type &k) const;
   };
   struct locktoken_remove_operation : public base_operation
   {
      struct fee_parameters_type {  uint64_t fee = 1*GRAPHENE_BLOCKCHAIN_PRECISION;};

      asset                     fee;
      account_id_type           issuer;   
      locktoken_id_type         locktoken_id;   

      extensions_type           extensions;

      account_id_type   fee_payer()const { return issuer; }
      void validate() const;
      share_type calculate_fee(const fee_parameters_type &k) const;
   };
} } 



FC_REFLECT( graphene::chain::locktoken_options,
            (version)
            (op_id)
            (lock_mode)
            (locktoken_rates)
            (max_rate)
            (vesting_seconds)
            (locktoken_min)
            (locktoken_max)
            (pay_asset)
            (payer)
            (carrier)
            (max_supply)
            (buy_asset_whitelist)
          )

FC_REFLECT( graphene::chain::locktoken_option_operation::fee_parameters_type,  (fee))
FC_REFLECT( graphene::chain::locktoken_option_operation,
            (fee)
            (issuer)
            (asset_lock)
            (op_type)
            (sParam)
            (extensions) 
            )

FC_REFLECT( graphene::chain::locktoken_create_operation::fee_parameters_type,  (fee))
FC_REFLECT( graphene::chain::locktoken_create_operation,
            (fee)
            (issuer)
            (locked)
            (to)
            (period)
            (type)
            (autolock)
            (extensions)
            )

FC_REFLECT( graphene::chain::locktoken_update_operation::fee_parameters_type,  (fee))
FC_REFLECT( graphene::chain::locktoken_update_operation,
            (fee)
            (issuer)
            (locktoken_id)
            (op_type)
            (locked)
            (to)
            (type)
            (period)
            (autolock)
            (extensions)
            )
FC_REFLECT( graphene::chain::locktoken_node_operation::fee_parameters_type,  (fee))
FC_REFLECT( graphene::chain::locktoken_node_operation,
            (fee)
            (issuer)
            (locktoken_id)
            (op_type)
            (to)
            (to_amount)
            (period)
            (extensions)  
            )      
FC_REFLECT( graphene::chain::locktoken_remove_operation::fee_parameters_type,  (fee))
FC_REFLECT( graphene::chain::locktoken_remove_operation,
            (fee)
            (issuer)
            (locktoken_id)
            (extensions)
            )                        

