#pragma once

#include <graphene/chain/protocol/types.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/db/object.hpp>
#include <graphene/chain/protocol/asset.hpp>
#include <graphene/business/protocol/locktoken.hpp>

namespace graphene { namespace chain {

using namespace graphene::db;

//节点奖励详情
struct locktoken_reward {
   locktoken_id_type from_id;     //奖励单号
   account_id_type   from_account;//奖励人
   uint32_t          type;        //奖励类型 1级奖励 2级奖励
   asset             amount;      //奖励金额
};

class locktoken_object : public graphene::db::abstract_object<locktoken_object>
{
   public:
      static const uint8_t space_id = protocol_ids;
      static const uint8_t type_id =  locktoken_object_type;

      locktoken_option_id_type  option_id;//参数  
      account_id_type           issuer;    //锁仓人
      asset_precision           locked;    //锁仓金额
      
      account_id_type      to;        //受益人， 锁仓为控制人
      uint32_t             period;    //锁仓期限，月为单位
      uint32_t             type = 0;  //锁仓类型 0 活期 1 定期 2 锁仓
      uint32_t             autolock = 0;    //定期到后，是否自动转，1 自动转定期，2 自动转活期，3 自动取消      
      fc::time_point_sec   init_time;     //创建时间
      fc::time_point_sec   create_time;     //创建时间
      uint32_t             rate;            //利率
      fc::time_point_sec   remove_time;     //删除时间
      fc::time_point_sec   except_time;     //到期时间

      asset_id_type        pay_asset;        //利息模式
      account_id_type      payer;           //利息支付人
      account_id_type      carrier;         //运营商
      price                pay_price;       //支付利息的价格
      share_type           max_rate;
      uint32_t             vesting_seconds; //退款或者利息锁定时间

      asset                buy_asset;        //购买资产

      asset                dy_interest;     //活期每天奖励总和，每月清理一次
      uint32_t             dy_dayflag;      // 
       flat_map<fc::time_point_sec, share_type> dy_list;     //收益列表  测试专用

      asset_precision      interest;                              //本单子总收益          
      flat_map<fc::time_point_sec, share_type> interest_list;     //收益列表
      flat_map<fc::time_point_sec, locktoken_reward> reward_list; //奖励列表

      uint32_t ustate = 0;  //状态

      uint64_t offset_time;

      void get_info() const;
};

typedef multi_index_container<
   locktoken_object,
   indexed_by<
      ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >   
   >
> locktoken_object_multi_index_type;

using locktoken_index = generic_index<locktoken_object, locktoken_object_multi_index_type>;

class locktoken_option_object : public graphene::db::abstract_object<locktoken_option_object>
{
   public:
      static const uint8_t space_id = protocol_ids;
      static const uint8_t type_id =  locktoken_options_object_type;

      
      account_id_type                                   issuer;                                //创建人
      fc::string                                        name;                                  //名称
      asset_id_type                                     asset_id;                              //对应资产
      uint8_t                                           asset_precision;                       //资产精度
      fc::string                                        asset_name;                            //资产名称
      uint8_t                                           interest_precision;                       //资产精度
      fc::string                                        interest_name;                            //资产名称
      uint32_t                                          lock_mode;                             //锁仓模式
      bool                                              payer_enable = false;
      locktoken_options                                 lockoptions; //参数
};

struct by_asset_mode;
typedef multi_index_container<
   locktoken_option_object,
   indexed_by<
      ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
      ordered_unique<
         tag<by_asset_mode>,
         composite_key<
            locktoken_option_object,
            member<locktoken_option_object, asset_id_type, &locktoken_option_object::asset_id>,            
            member<locktoken_option_object, uint32_t, &locktoken_option_object::lock_mode> > >   
   >
> locktoken_options_object_multi_index_type;

using locktoken_option_index = generic_index<locktoken_option_object, locktoken_options_object_multi_index_type>;

} } // graphene::chain
 
FC_REFLECT( graphene::chain::locktoken_reward,    
    (from_id)
    (from_account)
    (type)
    (amount)
    )

FC_REFLECT_DERIVED( graphene::chain::locktoken_option_object, (graphene::db::object), 
                  (issuer)             
                  (name)    
                  (asset_id)
                  (asset_precision)
                  (asset_name)
                  (interest_precision)
                  (interest_name) 
                  (lock_mode)           
                  (payer_enable)       
                  (lockoptions)              
                  )
                   
FC_REFLECT_DERIVED( graphene::chain::locktoken_object, (graphene::db::object), 
                  (option_id)
                  (issuer)
                  (locked)
                  (to)
                  (period)
                  (type)
                  (autolock)
                  (init_time)
                  (create_time)
                  (rate)
                  (remove_time)
                  (except_time)
                  (pay_asset)
                  (payer)
                  (carrier)
                  (pay_price)
                  (vesting_seconds)
                  (buy_asset)
                  (max_rate)
                  (dy_interest)
                  (dy_dayflag)
                  (dy_list)
                  (interest)
                  (interest_list)
                  (reward_list)
                  (ustate)
                  (offset_time)
                  )
 