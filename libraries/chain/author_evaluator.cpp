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
#include <graphene/chain/author_evaluator.hpp>
#include <graphene/chain/author_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/protocol/vote.hpp>

namespace graphene { namespace chain {

void_result author_create_evaluator::do_evaluate( const author_create_operation& op )
{ try {
      const auto &account = op.author_account(db());
      FC_ASSERT(account.is_lifetime_member());
      FC_ASSERT((account.uaccount_property & account_authenticator) == 0,"${x}",("x",account.uaccount_property));
      if (op.fee.amount > 1)
      {
         lock_fees = db().get_fee(op, 1);
         FC_ASSERT(db().get_balance(op.author_account, op.fee.asset_id) >= lock_fees + op.fee, "fee  ${ptype}", ("ptype", lock_fees + op.fee));    
      }
      FC_ASSERT(!account.is_lock_balance());   
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type author_create_evaluator::do_apply( const author_create_operation& op )
{ try {
   vote_id_type vote_id;
   db().modify(db().get_global_properties(), [&vote_id](global_property_object& p) {
      vote_id = get_next_vote_id(p, vote_id_type::author);
   });
   
   auto &account = op.author_account(db());
   
   const auto &new_author_object = db().create<author_object>([&](author_object &obj) {
      obj.name           = account.name;
      obj.author_account = op.author_account;
      obj.auth_type = op.auth_type & author_mask;
      obj.allow_asset = op.allow_asset;
      obj.vote_id = vote_id;
      obj.url = op.url;
      obj.memo = op.memo;      
      if (op.config.valid())
         obj.config = *op.config;
      obj.lock_asset = lock_fees;      
   });

   db().modify(account, [&](account_object& p) {
      p.uaccount_property |= account_authenticator;
   });

   if(lock_fees.amount >0)
   {
    db().adjust_balance(97,"",op.author_account,- lock_fees);
    auto account_statistics = account.statistics(db()); 
    db().modify(account_statistics, [&](account_statistics_object& p) {      
       p.lock_asset += lock_fees;
     });
   }  

   return new_author_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result author_update_evaluator::do_evaluate( const author_update_operation& op )
{ try {
   FC_ASSERT(db().get(op.author).author_account == op.author_account);
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result author_update_evaluator::do_apply( const author_update_operation& op )
{ try {
   database& _db = db();
   _db.modify(
      _db.get(op.author),
      [&]( author_object& wit )
      {
         if( op.auth_type.valid() )
            wit.auth_type = (*op.auth_type) & author_mask;
         if( op.new_url.valid() )
            wit.url = *op.new_url;        
         if( op.new_memo.valid() )
            wit.memo = *op.new_memo;    
          if( op.new_config.valid() )
            wit.config = *op.new_config;    
         if( op.new_allow_asset.valid() )
            wit.allow_asset = *op.new_allow_asset;                
      });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

 
} } // graphene::chain
