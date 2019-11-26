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
#include <graphene/chain/witness_evaluator.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/gateway_object.hpp>
#include <graphene/chain/carrier_object.hpp>
#include <graphene/chain/author_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/budget_member_object.hpp>
#include <graphene/chain/balance_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/protocol/vote.hpp>

namespace graphene { namespace chain {

void_result witness_create_evaluator::do_evaluate( const witness_create_operation& op )
{ try {
   FC_ASSERT(db().get(op.witness_account).is_lifetime_member());
   FC_ASSERT((op.witness_account(db()).uaccount_property & account_witness) == 0);
    if( op.fee.amount >1)
   {
      lock_fees = db().get_fee(op,1);
     FC_ASSERT(db().get_balance(op.witness_account,op.fee.asset_id)>= lock_fees,"lock fee  ${ptype}", ("ptype", lock_fees) );  
   }
     FC_ASSERT(!op.witness_account(db()).is_lock_balance());  
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type witness_create_evaluator::do_apply( const witness_create_operation& op )
{ try {
   vote_id_type vote_id;
   db().modify(db().get_global_properties(), [&vote_id](global_property_object& p) {
      vote_id = get_next_vote_id(p, vote_id_type::witness);
   });
   auto &account = op.witness_account(db());
   const auto &new_witness_object = db().create<witness_object>([&](witness_object &obj) {
      obj.name = account.name;
      obj.witness_account = op.witness_account;
      obj.signing_key = op.block_signing_key;
      obj.vote_id = vote_id;
      obj.url = op.url;
      obj.memo = op.memo;
      obj.lock_asset = lock_fees;
   });

   db().modify(account, [&](account_object& p) {
      p.uaccount_property |= account_witness;      
   });
 

   if(lock_fees.amount >0)
   {
    db().adjust_balance(78,"",op.witness_account,- lock_fees);
    auto account_statistics = account.statistics(db()); 
    db().modify(account_statistics, [&](account_statistics_object& p) {      
       p.lock_asset += lock_fees;
     });
   }  
     

   return new_witness_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result witness_update_evaluator::do_evaluate( const witness_update_operation& op )
{ try {
   FC_ASSERT(db().get(op.witness).witness_account == op.witness_account);
    FC_ASSERT(!op.witness_account(db()).is_lock_balance()); 
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result witness_update_evaluator::do_apply( const witness_update_operation& op )
{ try {
   database& _db = db();
   _db.modify(
      _db.get(op.witness),
      [&]( witness_object& wit )
      {
         if( op.new_url.valid() )
            wit.url = *op.new_url;
          if( op.new_memo.valid() )
            wit.memo = *op.new_memo;
         if( op.new_signing_key.valid() )
            wit.signing_key = *op.new_signing_key;
      });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }


void_result change_identity_evaluator::do_evaluate( const change_identity_operation& op )
{ try {
    database& _db = db();  
    switch(op.object_id.type())
    {
     case committee_member_object_type:   
     { 
        committee_member_id_type id = op.object_id;
        const committee_member_object &mem = id(_db);   
        enable = mem.enable;
        account_id = mem.committee_member_account;
        committee_member_create_operation op;
        lock_fees = _db.get_fee(op,1);
     }         
     break;
     case gateway_object_type:
     { 
        gateway_id_type id = op.object_id;
        const gateway_object &mem = id(_db);   
        enable = mem.enable;
        account_id = mem.gateway_account;
        gateway_create_operation op;
        lock_fees = _db.get_fee(op,1);
     }         
     break;
     case witness_object_type:
     { 
        witness_id_type id = op.object_id;
        const witness_object &mem = id(_db);   
        enable = mem.enable;
        account_id = mem.witness_account;
        witness_create_operation op;
        lock_fees = _db.get_fee(op,1);
     }         
     break;   
     case carrier_object_type:
      { 
        carrier_id_type id = op.object_id;
        const carrier_object &mem = id(_db);   
        enable = mem.enable;
        account_id = mem.carrier_account;
        carrier_create_operation op;
        lock_fees = _db.get_fee(op,1);
     }         
     break;
     case author_object_type:
      { 
        author_id_type id = op.object_id;
        const author_object &mem = id(_db);   
        enable = mem.enable;
        account_id = mem.author_account;
        author_create_operation op;
        lock_fees = _db.get_fee(op,1);
     }         
     break;
     case budget_member_object_type:
      { 
        budget_member_id_type id = op.object_id;
        const budget_member_object &mem = id(_db);   
        enable = mem.enable;
        account_id = mem.budget_member_account;
        budget_member_create_operation op;
        lock_fees = _db.get_fee(op,1);
     }         
     break;     
     default:
      FC_ASSERT(false,"unknow object id ${object}",("object",op.object_id));
      break;
    }    
    FC_ASSERT(account_id == op.issuer);
    if(op.enable)    
      FC_ASSERT(enable == identity_disable || enable == identity_enable_lost,"same statue ${enable}",("enable",enable));
    else
      FC_ASSERT(enable == identity_enable || enable == identity_enable_lost,"same statue ${enable}",("enable",enable));
    //检查费用是否合适  
    if(op.enable && enable == identity_disable )      
    {
        FC_ASSERT(db().get_balance(account_id,lock_fees.asset_id)>= lock_fees,"lock fee  ${ptype}", ("ptype", lock_fees) );           
    }    
    return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }
template <typename T>
void  change_identity_evaluator::adjust_identify(const T &i,database& _db, const change_identity_operation& op ) 
{
  if(op.enable)
  {    
    if(i.enable == identity_disable && lock_fees.amount >0)
    {
      _db.adjust_balance(79,"",account_id,-lock_fees);
      auto &account = account_id(_db);
      auto account_statistics = account.statistics(_db); 
      _db.modify(account_statistics, [&](account_statistics_object& p) {      
         p.lock_asset += lock_fees;
        });
    }
    _db.modify(i, [&](T& p) {
      if(p.enable == identity_disable && lock_fees.amount >0)
         p.lock_asset = lock_fees;
      p.enable = identity_enable;
   });
  }
  else
  {
    lock_fees = i.lock_asset;
     _db.modify(i, [&](T& p) {     
      p.lock_asset.amount.value  = 0;
      p.enable = identity_disable;
    });
    if(lock_fees.amount >0)
    {
       auto &account = account_id(_db);
       auto account_statistics = account.statistics(_db); 
       _db.modify(account_statistics, [&](account_statistics_object& p) {      
         p.lock_asset -= lock_fees;
        });


       const auto& global_properties = db().get_global_properties();
       if(global_properties.parameters.identify_lock_vesting_seconds >3600)
       {         
         _db.create<vesting_balance_object>([&](vesting_balance_object& b) {
         b.owner = this->account_id;
         b.balance = this->lock_fees;
         b.bautoremove = true;

         linear_vesting_policy policy;
         policy.begin_timestamp = _db.head_block_time();
         policy.vesting_cliff_seconds = global_properties.parameters.identify_lock_vesting_seconds;
         policy.vesting_duration_seconds = 1;
         policy.begin_balance = b.balance.amount;         

         b.policy = std::move(policy);
         });
       }
       else
         _db.adjust_balance(80,"",account_id,lock_fees);
    }
  }
}
void_result change_identity_evaluator::do_apply( const change_identity_operation& op )
{ try {
    database& _db = db();    
    switch(op.object_id.type())
    {
     case committee_member_object_type:   
     { 
        committee_member_id_type id = op.object_id;
        const committee_member_object &mem = id(_db);   
        adjust_identify(mem,_db,op);         
     }         
     break;
     case gateway_object_type:
     { 
        gateway_id_type id = op.object_id;
        const gateway_object &mem = id(_db);   
         adjust_identify(mem,_db,op); 
     }         
     break;
     case witness_object_type:
     { 
        witness_id_type id = op.object_id;
        const witness_object &mem = id(_db);   
         adjust_identify(mem,_db,op); 
     }         
     break;   
     case carrier_object_type:
      { 
        carrier_id_type id = op.object_id;
        const carrier_object &mem = id(_db);   
        adjust_identify(mem,_db,op); 
     }         
     break;
     case author_object_type:
      { 
        author_id_type id = op.object_id;
        const author_object &mem = id(_db);   
        adjust_identify(mem,_db,op); 
     }         
     break;
     case budget_member_object_type:
      { 
        budget_member_id_type id = op.object_id;
        const budget_member_object &mem = id(_db);   
         adjust_identify(mem,_db,op); 
     }         
     break;     
     default:
      FC_ASSERT(false,"unknow object id ${object}",("object",op.object_id));
      break;
    } 
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }


} } // graphene::chain
