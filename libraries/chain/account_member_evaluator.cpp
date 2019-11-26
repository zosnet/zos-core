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
#include <graphene/chain/account_member_evaluator.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/author_object.hpp>
#include <graphene/chain/carrier_object.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <graphene/chain/protocol/vote.hpp>
#include <graphene/chain/transaction_evaluation_state.hpp>
#include <graphene/chain/gateway_object.hpp>

#include <fc/smart_ref_impl.hpp>

namespace graphene { namespace chain {

int32_t get_can_author(const database &d,const gateway_object & ca ,const account_object &a,const asset_id_type &aid)
{
  //法币需要校验
  const auto &as = aid(d);
  fc::time_point_sec now = d.head_block_time();
  //是否校验法币
  if (as.is_cash_issued())
  {
    if ((ca.need_auth & gateway_cash) <= 0)
      return auth_nosetting;
  }
  else //是否校验数字货币
  {
    if ((ca.need_auth & gateway_bit) <= 0)
      return auth_nosetting;
  }  

  account_id_type author_id; //认证人
  //根据币种，选择信任的认证人
  const auto iter = ca.trust_auth.find(aid);
  if (iter != ca.trust_auth.end())
    author_id = iter->second;
  //否则为缺省认证人  
  else if (ca.def_auth.valid())
  {
    const auto &ass = (*ca.def_auth)(d);
    author_id = ass.author_account;
  }
  //否则为自己
  else
    author_id = ca.gateway_account;
  //检查认证人权限
  if (!author_id(d).is_auth_asset(d, aid))
    return -5;

  if (!a.auth_data.valid())
    return -1;  

  const auto iter_auth = a.auth_data->auth_state.find(author_id);
  //没认证
  if (iter_auth == a.auth_data->auth_state.end())
    return -2;
  //检查 hash  
  if (ca.need_auth & gateway_hash)
  {
    if (iter_auth->second.hash64 != a.auth_data->hash64)
      return -3;
  }
  //检查过期
  if (iter_auth->second.auth_time + iter_auth->second.expiration < now)
  {
    return -4;
  }  
  return (iter_auth->second.state & author_xor_mask);  
}
int32_t get_can_withdraw(const database &d,const gateway_object & ca ,const account_object &a,const asset_id_type &aid)
{  
    //借没有校验标志
  if((ca.need_auth & gateway_withdray) <=0)
    return auth_nosetting;
  return get_can_author(d,ca,a,aid);
}
int32_t get_can_deposit(const database &d,const gateway_object & ca ,const account_object &a,const asset_id_type &aid)
{
     //借没有校验标志 
  if ((ca.need_auth & gateway_dispost) <= 0)
    return auth_nosetting;
  return get_can_author(d,ca,a,aid);
}

void_result gateway_issue_currency_evaluator::do_evaluate( const gateway_issue_currency_operation& op )
{ try {
     database& d = db();
     auto &a = op.issue_currency.asset_id(d);
     auto &c = op.account_to(d);     
     auto &ad = a.dynamic_asset_data_id(d);    

    FC_ASSERT(!c.is_lock_balance());  
    op.validate();
    FC_ASSERT(a.is_bitasset_issued() || a.id == GRAPHENE_CORE_ASSET , "asset must be smart coin.  '${id1}'", ("id1", a.id));
    FC_ASSERT(c.is_gateway_member(), "account must be a gateway member.  '${id1}'", ("id1", c.id));
    if (op.revoke)
    {
        const auto& idx = d.get_index_type<gateway_index>().indices().get<by_account>();
        auto itr = idx.find(op.account_to);
        FC_ASSERT(itr != idx.end());
        auto itr_issue = itr->dynamic_id(d).issue_amount.find(op.issue_currency.asset_id);
        FC_ASSERT(itr_issue != itr->dynamic_id(d).issue_amount.end());
        asset ubalance = d.get_balance(op.account_to, op.issue_currency.asset_id);
        FC_ASSERT(op.issue_currency.amount <= ubalance.amount, "asset  '${id1}' must less '${id2}'", ("id1", op.issue_currency)("id2", ubalance));
        FC_ASSERT(op.issue_currency.amount <= ad.current_supply, "asset  '${id1}' must less '${id2}'", ("id1", op.issue_currency)("id2", ad.current_supply));         
        FC_ASSERT(op.issue_currency.amount <= itr_issue->second, "asset  '${id1}' must less '${id2}'", ("id1", op.issue_currency)("id2", itr_issue->second));         
     }
     else
     {
       //检查网关是否有抵押币
       FC_ASSERT(c.is_gateway_enable(d,op.issue_currency.asset_id),"gateway '${id1}' is not authority deposit ", ("id1", c.id));  
       FC_ASSERT(a.options.max_supply >= ad.current_supply);
       asset umax = asset(a.options.max_supply - ad.current_supply,op.issue_currency.asset_id);
       FC_ASSERT(op.issue_currency.amount<=umax.amount , "asset  '${id1}' must less '${id2}'", ("id1", op.issue_currency)("id2",umax));
       //检查供应量 ZOS 从预算中出
       if(a.id == GRAPHENE_CORE_ASSET) {
         const dynamic_global_property_object& dpo = d.get_dynamic_global_properties();
         FC_ASSERT(op.issue_currency.amount<= GRAPHENE_MAX_SHARE_BUDGET - dpo.budget_supply , "asset  '${id1}' must less '${id2}'", ("id1", op.issue_currency)("id2",GRAPHENE_MAX_SHARE_BUDGET - dpo.budget_supply));         
       }
     }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result gateway_issue_currency_evaluator::do_apply( const gateway_issue_currency_operation& op )
{ try {
     database& d = db();
     auto &a = op.issue_currency.asset_id(d);      
     auto &ad = a.dynamic_asset_data_id(d);
     const auto& idx = d.get_index_type<gateway_index>().indices().get<by_account>();
     auto itr = idx.find(op.account_to);     
     const dynamic_global_property_object& dpo = d.get_dynamic_global_properties();

     if(op.revoke)
     {
        d.adjust_balance(88,"",op.account_to, -op.issue_currency);
        d.modify(ad, [&](asset_dynamic_data_object &data) {
         data.current_supply -= op.issue_currency.amount;
        });
        d.modify(itr->dynamic_id(d), [&](gateway_dynamic_object &data) {
          data.issue_amount[op.issue_currency.asset_id] -= op.issue_currency.amount;
        });
        //ZOS 从预算中出
        if(a.id == GRAPHENE_CORE_ASSET) {
          d.modify(dpo, [&](dynamic_global_property_object &data) {
            data.budget_supply -= op.issue_currency.amount;
          });
        }
     }
     else
     {
        d.adjust_balance(3,"",op.account_to, op.issue_currency);
        d.modify(ad, [&](asset_dynamic_data_object &data) {
         data.current_supply += op.issue_currency.amount;
        });
        d.modify(itr->dynamic_id(d), [&](gateway_dynamic_object &data) {
         data.issue_amount[op.issue_currency.asset_id] += op.issue_currency.amount;
        });
        //ZOS 从预算中出
        if(a.id == GRAPHENE_CORE_ASSET) {
          d.modify(dpo, [&](dynamic_global_property_object &data) {
            data.budget_supply += op.issue_currency.amount;
          });
        }
     }

     return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }


//提现,给网关
void_result gateway_withdraw_evaluator::do_evaluate( const gateway_withdraw_operation& op )
{ try {
    database& d = db();
    auto &t = op.to(d);  
    auto &f = op.from(d);  
    auto &a = op.withdraw.asset_id(d);
     
    FC_ASSERT(!f.is_lock_balance());     
    FC_ASSERT(!t.is_lock_balance());     

    const auto& idx = d.get_index_type<gateway_index>().indices().get<by_account>();
    auto itr = idx.find(op.to);
    FC_ASSERT(itr != idx.end());

    op.validate();

    FC_ASSERT(a.is_loan_issued() || a.is_lender_issued(), " asset '${id1}' must be loan or lender  ", ("id1", op.withdraw));
    FC_ASSERT(t.is_gateway_member(), " account '${id1}' must be gateway member  ", ("id1", op.to));
     //检查网关是否有抵押币

    FC_ASSERT(get_can_withdraw(d,t.get_gateway_id(d)(d), f,op.withdraw.asset_id));
    FC_ASSERT(t.is_gateway_enable(d, op.withdraw.asset_id), "gateway '${id1}' is not authority deposit ", ("id1", t.id));

    share_type ubalance = d.get_balance(op.from, op.withdraw.asset_id).amount;
    FC_ASSERT(ubalance >= op.withdraw.amount, "insufficient_balance '${id1}' < '${id2}'", ("id1", ubalance)("id2", op.withdraw.amount));

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result gateway_withdraw_evaluator::do_apply( const gateway_withdraw_operation& op )
{ try {
   database& d = db();
   const auto& idx = d.get_index_type<gateway_index>().indices().get<by_account>();
   auto itr = idx.find(op.to);   

   d.adjust_balance(4,"",op.to, op.withdraw);
   d.adjust_balance(5,"",op.from, -op.withdraw);
   d.modify(itr->dynamic_id(d), [&](gateway_dynamic_object &data) {
       data.withdraw_amount[op.withdraw.asset_id] += op.withdraw.amount;
       data.withdraw_count[op.withdraw.asset_id]++;
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) } 

//充值
void_result gateway_deposit_evaluator::do_evaluate( const gateway_deposit_operation& op )
{ try {

    database& d = db();
    auto &t = op.to(d);  
    auto &f = op.from(d);  
    auto &a = op.deposit.asset_id(d);

    FC_ASSERT(!f.is_lock_balance());      
    FC_ASSERT(!t.is_lock_balance());      

    const auto& idx = d.get_index_type<gateway_index>().indices().get<by_account>();
    auto itr = idx.find(op.from);
    FC_ASSERT(itr != idx.end());


    op.validate();

    FC_ASSERT(a.is_loan_issued() || a.is_lender_issued(), " asset '${id1}' must be loan or bitlender  ", ("id1", op.deposit));
    FC_ASSERT(f.is_gateway_member(), " account '${id1}' must be gateway member  ", ("id1", op.to));
    //检查网关是否有抵押币
    FC_ASSERT(f.is_gateway_enable(d, op.deposit.asset_id),"gateway '${id1}' is not authority deposit ", ("id1", f.id)); 
    FC_ASSERT(get_can_deposit(d,f.get_gateway_id(d)(d), t,op.deposit.asset_id) > 0);

    share_type ubalance = d.get_balance(op.from, op.deposit.asset_id).amount;
    FC_ASSERT(ubalance >= op.deposit.amount, "insufficient_balance '${id1}' < '${id2}'", ("id1", ubalance)("id2", op.deposit.amount));

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result gateway_deposit_evaluator::do_apply( const gateway_deposit_operation& op )
{ try {
    database& d = db();

    const auto& idx = d.get_index_type<gateway_index>().indices().get<by_account>();
    auto itr = idx.find(op.from);

    d.adjust_balance(6,"",op.to, op.deposit);
    d.adjust_balance(7,"",op.from, -op.deposit);

    d.modify(itr->dynamic_id(d), [&](gateway_dynamic_object &data) {
       data.deposit_amount[op.deposit.asset_id] += op.deposit.amount;
       data.deposit_count[op.deposit.asset_id]++;
   });

    return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // graphene::chain
