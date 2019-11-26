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
#include <graphene/chain/budget_member_evaluator.hpp>
#include <graphene/chain/budget_member_object.hpp>
#include <graphene/chain/worker_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <graphene/chain/protocol/vote.hpp> 
#include <graphene/chain/transaction_evaluation_state.hpp>
#include <graphene/chain/hardfork.hpp>
#include <fc/smart_ref_impl.hpp>

namespace graphene { namespace chain {


void verify_budget_votes( const database& db, flat_set<vote_id_type> &votes)
{      
      vector<vote_id_type> remove_id;
      const auto &approve_worker_idx = db.get_index_type<worker_index>().indices().get<by_vote_for>();
      const auto& against_worker_idx = db.get_index_type<worker_index>().indices().get<by_vote_against>(); 
      for ( auto id : votes ) 
      {
         if(id.type() ==vote_id_type::worker ) //有的worker自动过期删除了
          {                      
            if(approve_worker_idx.find( id ) == approve_worker_idx.end() &&  against_worker_idx.find( id ) == against_worker_idx.end())
            {
              remove_id.push_back(id);
            }
         }
      }
      for ( auto id : remove_id )       
        votes.erase(id);      
}
void_result budget_member_create_evaluator::do_evaluate( const budget_member_create_operation& op )
{ try {
   FC_ASSERT(db().get(op.budget_member_account).is_lifetime_member());
   FC_ASSERT((op.budget_member_account(db()).uaccount_property & account_budget) == 0);
   if(op.fee.amount >1)
   {
      lock_fees = db().get_fee(op,1);
     FC_ASSERT(db().get_balance(op.budget_member_account,op.fee.asset_id)>= lock_fees,"lock fee  ${ptype}", ("ptype", lock_fees) );   
   }
   FC_ASSERT(!op.budget_member_account(db()).is_lock_balance());   
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type budget_member_create_evaluator::do_apply( const budget_member_create_operation& op )
{ try {
   vote_id_type vote_id;
   db().modify(db().get_global_properties(), [&vote_id](global_property_object& p) {
      vote_id = get_next_vote_id(p, vote_id_type::budget);
   });
   auto &account = op.budget_member_account(db());
   const auto& new_del_object = db().create<budget_member_object>( [&]( budget_member_object& obj ){
         obj.name                    = account.name;
         obj.budget_member_account   = op.budget_member_account;
         obj.vote_id            = vote_id;
         obj.url                = op.url;
         obj.memo               = op.memo;
         obj.lock_asset         = lock_fees;
   });

   
   db().modify(account, [&](account_object& p) {
      p.uaccount_property |= account_budget;
   });

   if(lock_fees.amount >0)
   {
    db().adjust_balance(40,"",op.budget_member_account,- lock_fees);
    auto account_statistics = account.statistics(db()); 
    db().modify(account_statistics, [&](account_statistics_object& p) {      
       p.lock_asset += lock_fees;
     });
   }  

   return new_del_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result budget_member_update_evaluator::do_evaluate( const budget_member_update_operation& op )
{ try {
   FC_ASSERT(db().get(op.budget_member).budget_member_account == op.budget_member_account);
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result budget_member_update_evaluator::do_apply( const budget_member_update_operation& op )
{ try {
   database& _db = db();
   _db.modify(
      _db.get(op.budget_member),
      [&]( budget_member_object& com )
      {
         if( op.new_url.valid() )
            com.url = *op.new_url;
         if( op.new_memo.valid() )
            com.memo = *op.new_memo;   
         if(op.votes.valid())
         {
            com.votes = *op.votes;
            verify_budget_votes(_db,com.votes);
         }
      });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }
 


} } // graphene::chain
