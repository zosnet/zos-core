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
#include <graphene/chain/protocol/account.hpp>
#include <graphene/chain/hardfork.hpp>
#include <boost/algorithm/string.hpp>

namespace graphene { namespace chain {

/**
 * Names must comply with the following grammar (RFC 1035):
 * <domain> ::= <subdomain> | " "
 * <subdomain> ::= <label> | <subdomain> "." <label>
 * <label> ::= <letter> [ [ <ldh-str> ] <let-dig> ]
 * <ldh-str> ::= <let-dig-hyp> | <let-dig-hyp> <ldh-str>
 * <let-dig-hyp> ::= <let-dig> | "-"
 * <let-dig> ::= <letter> | <digit>
 *
 * Which is equivalent to the following:
 *
 * <domain> ::= <subdomain> | " "
 * <subdomain> ::= <label> ("." <label>)*
 * <label> ::= <letter> [ [ <let-dig-hyp>+ ] <let-dig> ]
 * <let-dig-hyp> ::= <let-dig> | "-"
 * <let-dig> ::= <letter> | <digit>
 *
 * I.e. a valid name consists of a dot-separated sequence
 * of one or more labels consisting of the following rules:
 *
 * - Each label is three characters or more
 * - Each label begins with a letter
 * - Each label ends with a letter or digit
 * - Each label contains only letters, digits or hyphens
 *
 * In addition we require the following:
 *
 * - All letters are lowercase
 * - Length is between (inclusive) GRAPHENE_MIN_ACCOUNT_NAME_LENGTH and GRAPHENE_MAX_ACCOUNT_NAME_LENGTH
 */
bool is_valid_password( const string& password) {
   return true;
}
bool is_valid_name( const string& name_s )
{ try {
    string name = name_s;
    boost::algorithm::to_lower(name);
    
    const size_t len = name.size();

    if( len < GRAPHENE_MIN_ACCOUNT_NAME_LENGTH )
    {
          ilog( ".");
        return false;
    }

    if( len > GRAPHENE_MAX_ACCOUNT_NAME_LENGTH )
    {
          ilog( ".");
        return false;
    }

    size_t begin = 0;
    while( true )
    {
       size_t end = name.find_first_of( '.', begin );
       if( end == std::string::npos )
          end = len;
       if( (end - begin) < GRAPHENE_MIN_ACCOUNT_NAME_LENGTH )
       {
          idump( (name) (end)(len)(begin)(GRAPHENE_MAX_ACCOUNT_NAME_LENGTH) );
          return false;
       }
       switch( name[begin] )
       {
          case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
          case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
          case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
          case 'y': case 'z':
             break;
          default:
          ilog( ".");
             return false;
       }
       switch( name[end-1] )
       {
          case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
          case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
          case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
          case 'y': case 'z':
          case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
          case '8': case '9':
             break;
          default:
          ilog( ".");
             return false;
       }
       for( size_t i=begin+1; i<end-1; i++ )
       {
          switch( name[i] )
          {
             case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
             case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
             case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
             case 'y': case 'z':
             case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
             case '8': case '9':
             case '-':
                break;
             default:
          ilog( ".");
                return false;
          }
       }
       if( end == len )
          break;
       begin = end+1;
    }
    return true;
} FC_CAPTURE_AND_RETHROW( (name_s) ) }

bool is_cheap_name( const string& n )
{
   bool v = false;
   
   if(n.size()>8)  return true;
   if(n.size()<=2) return false;

   for (auto c : n)
   {
      if( c >= '0' && c <= '9' ) return true;
      if( c == '.' || c == '-' || c == '/' ) return true;
      switch( c )
      {
         case 'a':
         case 'e':
         case 'i':
         case 'o':
         case 'u':
         case 'y':
            v = true;
      }
   }
   if( !v )
      return true;
   return false;
}
bool  account_options::is_samevotes(flat_set<vote_id_type> &votes1)
{
   if(votes.size()!=votes1.size())
      return false;
   for(auto &id1 : votes)
   {
      if(votes1.find(id1) == votes1.end())
         return false;
   }
   return true;
}

void account_options::validate() const
{
   auto needed_witnesses = num_witness;
   auto needed_committee = num_committee;
   auto needed_budget    = num_budget;
   

   for( vote_id_type id : votes )
      if( id.type() == vote_id_type::witness && needed_witnesses )
         --needed_witnesses;
   else if ( id.type() == vote_id_type::committee && needed_committee )
         --needed_committee;
   else if ( id.type() == vote_id_type::budget && needed_budget )
         --needed_budget;      
     

   FC_ASSERT( needed_witnesses == 0 && needed_committee == 0 && needed_budget == 0 ,
              "May not specify fewer witnesses or committee    members of budget    members than the number voted for.");
}

share_type account_create_operation::calculate_fee( const fee_parameters_type& k )const
{
   auto core_fee_required = k.basic_fee;

   if( !is_cheap_name(name) )
   {  
      uint32_t char_len[10] = {0,8,7,6,5,4,3,2,1,0};
      core_fee_required = k.premium_fee;
      if(name.size()<=8 && name.size()>0)
         core_fee_required += char_len[name.size()] * k.char_fee;
   }

   // Authorities and vote lists can be arbitrarily large, so charge a data fee for big ones
   auto data_fee =  calculate_data_fee( fc::raw::pack_size(*this), k.price_per_kbyte ); 
   core_fee_required += data_fee;

   return core_fee_required;
}

uint16_t       account_create_operation::get_referrer_percent(fc::time_point_sec now) const
{
   uint16_t referrer_ret = referrer_percent;
   bool has_small_percent = (
      (now <= HARDFORK_200101_TIME)
      && (referrer != registrar  )
      && (referrer_percent != 0    )
      && (referrer_percent <= 0x100)
      );

    if( has_small_percent )
    {
      if( referrer_ret >= 100 )
      {
         wlog( "between 100% and 0x100%:  ${o}", ("o", *this) );
      }
      referrer_ret = referrer_ret*100;
      if( referrer_ret > GRAPHENE_100_PERCENT )
         referrer_ret = GRAPHENE_100_PERCENT;
    }
    return referrer_ret;
}



void account_create_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT( is_valid_name( name ) );
   FC_ASSERT( is_safe_string(std::string(name)),"${x}",("x",name));
   FC_ASSERT( referrer_percent <= GRAPHENE_100_PERCENT );
   FC_ASSERT( owner.num_auths() != 0 );
   FC_ASSERT( owner.address_auths.size() == 0 );
   FC_ASSERT( active.num_auths() != 0 );
   FC_ASSERT( active.address_auths.size() == 0 );
   FC_ASSERT( !owner.is_impossible(), "cannot create an account with an imposible owner authority threshold" );
   FC_ASSERT( !active.is_impossible(), "cannot create an account with an imposible active authority threshold" );
   options.validate();
   if(  owner_special_authority.valid() )
      validate_special_authority( * owner_special_authority );
   if(  active_special_authority.valid() )
      validate_special_authority( * active_special_authority );
   if(  buyback_options.valid() )
   {
      FC_ASSERT( !( owner_special_authority.valid()) );
      FC_ASSERT( !( active_special_authority.valid()) );
      FC_ASSERT( owner == authority::null_authority() );
      FC_ASSERT( active == authority::null_authority() );
      size_t n_markets =  buyback_options->markets.size();
      FC_ASSERT( n_markets > 0 );
      for( const asset_id_type m :  buyback_options->markets )
      {
         FC_ASSERT( m !=  buyback_options->asset_to_buy );
      }
   }
   
}

share_type account_update_operation::calculate_fee( const fee_parameters_type& k )const
{
   auto core_fee_required = k.fee;  
   if( new_options  )
      core_fee_required += calculate_data_fee( fc::raw::pack_size(new_options->votes), k.price_per_kbyte );   
   return core_fee_required;
}

void account_update_operation::validate()const
{
   FC_ASSERT( account != GRAPHENE_TEMP_ACCOUNT );
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT( !account.is_null());

   bool has_action = (owner.valid() || active.valid() || limitactive.valid() || new_options.valid() || owner_special_authority.valid() || active_special_authority.valid() );

   FC_ASSERT( has_action );

   if( owner )
   {
      FC_ASSERT( owner->num_auths() != 0 );
      FC_ASSERT( owner->address_auths.size() == 0 );
      FC_ASSERT( !owner->is_impossible(), "cannot update an account with an imposible owner authority threshold" );
      
      
   }
   if( active )
   {
      FC_ASSERT( active->num_auths() != 0 );
      FC_ASSERT( active->address_auths.size() == 0 );
      FC_ASSERT( !active->is_impossible(), "cannot update an account with an imposible active authority threshold" );      
   }

   if( new_options )
      new_options->validate();
   if( owner_special_authority.valid() )
      validate_special_authority( *owner_special_authority );
   if( active_special_authority.valid() )
      validate_special_authority( *active_special_authority ); 
}
 
share_type account_authenticate_operation::calculate_fee(const fee_parameters_type& k) const
{
    auto core_fee_required = k.fee;
   if(auth_data.valid() )
   {
      if(auth_data->key.valid() )
        core_fee_required += calculate_data_fee( fc::raw::pack_size(*(auth_data->key)), k.price_per_kbyte );
     if(auth_data->info.valid() )
        core_fee_required += calculate_data_fee( fc::raw::pack_size(*(auth_data->info)), k.price_per_kbyte );     
   }
    return core_fee_required;
} 

share_type account_config_operation::calculate_fee(const fee_parameters_type& k) const
{
    auto core_fee_required = k.fee;   
    core_fee_required += calculate_data_fee( fc::raw::pack_size(this), k.price_per_kbyte );     
    return core_fee_required;
} 
void account_config_operation::validate()const
{
   for(auto &a : config)
   {
     FC_ASSERT(a.first.size()  > 0);
     FC_ASSERT(a.first.size()  <100);
     FC_ASSERT(a.second.size() <10000);
   }
}
void account_authenticate_operation::validate()const
{   
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT( !issuer.is_null());
     
   if (op_type == 1) //修改靓号提案人身份
   {
      FC_ASSERT(flags.valid());
   }
   else if(op_type == 2) //取消
   {
      FC_ASSERT(0);     
   }
   else if(op_type == 3) //锁定账户
   {
      FC_ASSERT(lock_balance.valid());
   }
   else if(op_type == 4) //修改用户验证公钥
   {
      FC_ASSERT(auth_key.valid());
   }
   else if(op_type == 5) //修改用户数据
   {
      FC_ASSERT(auth_data.valid());
      FC_ASSERT(auth_data->key.valid() || auth_data->info.valid());
      if (auth_data->key.valid())
      {
        FC_ASSERT(auth_data->key->from != public_key_type());
        FC_ASSERT(auth_data->key->message.size() < 20000);
      }
      if(auth_data->info.valid())
      {
        FC_ASSERT(auth_data->info->from != public_key_type());
        FC_ASSERT(auth_data->info->message.size() < 20000);
      }
   }
   else if(op_type == 6) //验证用户 
   {
      FC_ASSERT(auth_account.valid());    
      FC_ASSERT(auth_account->expiration<  365 * 24 *3600 *10);
      FC_ASSERT((auth_account->state & 0xFFFF0000) == 0);
   }
   else if(op_type == 7 || op_type == 8) //网关，运营商 认证人信息
   {
      FC_ASSERT(need_auth.valid() || trust_auth.valid() || auth_data.valid());
      if(trust_auth.valid())
        FC_ASSERT(trust_auth->size()<100);
      if(auth_data.valid())
      {
         FC_ASSERT(auth_data->authenticator.valid());
      }  
   }
   else if(op_type == 9) //绑定手机号邮箱
   {
      FC_ASSERT(auth_data.valid());
      FC_ASSERT(auth_data->info.valid());
      FC_ASSERT(auth_data->info->message.size() < 20000);
   }
   else if (op_type == 10) // 运营商是否支持 投资，借款
   {
      FC_ASSERT(flags.valid());     
   }
   else if (op_type == 11)  
   {
      FC_ASSERT(flags.valid());      
      FC_ASSERT(need_auth.valid());  
   }
   else
      FC_ASSERT(false); 
}

share_type account_upgrade_operation::calculate_fee(const fee_parameters_type& k) const
{
   if( upgrade_to_lifetime_member )
      return k.membership_lifetime_fee;
   else 
      return k.membership_annual_fee;
}


void account_upgrade_operation::validate() const
{
   FC_ASSERT( fee.amount >= 0 );
}

void account_transfer_operation::validate()const
{
   FC_ASSERT(false);   
   FC_ASSERT( fee.amount >= 0 );
   
}


void account_coupon_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
}
 
void withdraw_exchange_fee_operation::validate()const
{
   FC_ASSERT( fee.amount >=0 );
   FC_ASSERT( core_amount > 0 );
}

} } // graphene::chain
