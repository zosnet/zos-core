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
#include <graphene/chain/protocol/buyback.hpp>
#include <graphene/chain/protocol/ext.hpp>
#include <graphene/chain/protocol/special_authority.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/protocol/vote.hpp>
#include <graphene/chain/protocol/memo.hpp>

 


namespace graphene { namespace chain {

   bool is_valid_name( const string& s );
   bool is_cheap_name( const string& n );
   bool is_valid_password( const string& s);

   struct  account_auth
   {      
      optional<account_id_type>     authenticator; 
      optional<memo_data>           info;      
      optional<memo_data>           key;   
   };
   struct  account_author
   {
      uint32_t            state;  
      uint32_t            expiration = 365 * 24 *3600 *2;  
      account_id_type     account;      
      uint64_t            hash64;      
   };
 
   struct carrier_cfg
   {
      asset_id_type             assetid;
      fc::string                symbol;
      uint64_t                  uasset_property;
      carrier_id_type           lendCarrier;      
      carrier_id_type           investCarrier;
      fc::string                lendname;
      fc::string                investname;
      fc::string                lendUrl;
      fc::string                investUrl;
      account_id_type           lendaccount;
      account_id_type           investaccount;
      optional<author_id_type>  lendauthor;
      optional<account_id_type> lendauthor_account;
      optional<string>          lendauthor_url;
      optional<string>          lendauthor_name;
      optional<author_id_type>  investauthor;
      optional<account_id_type> investauthor_account;
      optional<string>          investauthor_url;
      optional<string>          investauthor_name;
      bool                      lendenable = true;
      bool                      investenable = true;
      bool                      lendvalidate = true;
      bool                      investvalidate = true;
   };
   struct gateway_cfg
   {
      asset_id_type             asset_id;
      string                    symbol;
      string                    real_symbol;
      string                    real_asset_sub;
      uint32_t                  uasset_property;
      gateway_id_type           gateway_id;
      account_id_type           account_id;
      string                    account_name;
      string                    gateway_url;
      uint8_t                   precision;
      share_type                deposit_amount;
      share_type                withdraw_amount;
      optional<author_id_type>  author_id;
      optional<account_id_type> author_account_id;
      optional<string>          author_url;    
      optional<string>          author_name;
      bool                      enable; 

   };
   struct collateral_cfg
   {
      asset_id_type loan;
      asset_id_type invest;
      uint32_t      rate; // 抵押率 ，GRAPHENE_COLLATERAL_RATIO_DENOM
      uint32_t      interest;// 利息 ，GRAPHENE_COLLATERAL_RATIO_DENOM
      double        setting;
   };
   struct asset_cfg
   {
      asset_id_type             id;
      string                    symbol;
      share_type                max_supply = 0;
      uint8_t                   precision = 0;
      uint32_t                  uasset_property = 0;
      share_type                total_core_in_lending = 0;
   };

   struct lock_recommend
   {
      asset_id_type asset_id;
      uint32_t lockmode;
      uint32_t period;
      uint32_t automode;
      vector<uint32_t> lable;
      uint32_t des;
   };

   struct sell_pair
   {
      asset_id_type first_id;
      string first_symbol;
      asset_id_type second_id;
      string second_symbol;
      uint32_t need_author;
   };  

   //反向排序
   inline bool
   operator<(const asset_cfg &a, const asset_cfg &b)
   {
    if(a.total_core_in_lending ==  b.total_core_in_lending)
     return a.id <  b.id;
   else
      return a.total_core_in_lending > b.total_core_in_lending;
   }
   inline bool operator == (const asset_cfg &a,const asset_cfg &b) 
   {
     return a.id ==  b.id;
   }

 
   /// These are the fields which can be updated by the active authority.
   struct account_options
   {
      /// The memo key is the key this account will typically use to encrypt/sign transaction memos and other non-
      /// validated account activities. This field is here to prevent confusion if the active authority has zero or
      /// multiple keys in it.
      public_key_type memo_key;

      public_key_type auth_key;
      /// If this field is set to an account ID other than GRAPHENE_PROXY_TO_SELF_ACCOUNT,
      /// then this account's votes will be ignored; its stake
      /// will be counted as voting for the referenced account's selected votes instead.
      account_id_type voting_account = GRAPHENE_PROXY_TO_SELF_ACCOUNT;

      /// The number of active witnesses this account votes the blockchain should appoint
      /// Must not exceed the actual number of witnesses voted for in @ref votes
      uint16_t num_witness = 0;
      /// The number of active committee members this account votes the blockchain should appoint
      /// Must not exceed the actual number of committee members voted for in @ref votes
      uint16_t num_committee = 0;

      /// The number of active budget members this account votes the blockchain should appoint
      /// Must not exceed the actual number of budget members voted for in @ref votes
      uint16_t num_budget = 0;

      /// This is the list of vote IDs this account votes for. The weight of these votes is determined by this
      /// account's balance of core asset
      flat_set<vote_id_type> votes;
      extensions_type extensions;

      void validate() const;
      bool is_samevotes(flat_set<vote_id_type> &votes);
      };

      /**
    *  @ingroup operations
    */
      struct account_create_operation : public base_operation
      {

         struct fee_parameters_type
         {

            uint64_t basic_fee = 10000;                                  ///< the cost to register the cheapest non-free account
            uint64_t premium_fee = 1000 * GRAPHENE_BLOCKCHAIN_PRECISION; ///< the cost to register the cheapest non-free account
            uint32_t price_per_kbyte = 1000;
            uint64_t char_fee = 10 * GRAPHENE_BLOCKCHAIN_PRECISION;
         };

         asset fee;
         /// This account pays the fee. Must be a lifetime member.
         account_id_type registrar;

         /// This account receives a portion of the fee split between registrar and referrer. Must be a member.
         account_id_type referrer;
         /// Of the fee split between registrar and referrer, this percentage goes to the referrer. The rest goes to the
         /// registrar.
         uint16_t referrer_percent = 0;

         string name;
         authority owner;
         authority active;
         authority limitactive;

         account_options options;

         optional<special_authority> owner_special_authority;
         optional<special_authority> active_special_authority;
         optional<buyback_account_options> buyback_options;

         extensions_type extensions;

         account_id_type fee_payer() const { return registrar; }
         void validate() const;
         share_type calculate_fee(const fee_parameters_type &) const;
         share_type get_fee(const fee_parameters_type &, uint32_t) const { return 0; }
         uint16_t get_referrer_percent(fc::time_point_sec now) const;

         void get_required_active_authorities(flat_set<account_id_type> &a) const
         {
            // registrar should be required anyway as it is the fee_payer(), but we insert it here just to be sure
            a.insert(registrar);
            if (buyback_options.valid())
               a.insert(buyback_options->asset_to_buy_issuer);
         }
      };

      /**
    * @ingroup operations
    * @brief Update an existing account
    *
    * This operation is used to update an existing account. It can be used to update the authorities, or adjust the options on the account.
    * See @ref account_object::options_type for the options which may be updated.
    */
      struct account_update_operation : public base_operation
      {

         struct fee_parameters_type
         {

            share_type fee = 10000;
            uint32_t price_per_kbyte = 1000;
         };

         asset fee;
         /// The account to update
         account_id_type account;

         /// New owner authority. If set, this operation requires owner authority to execute.
         optional<authority> owner;
         /// New active authority. This can be updated by the current active authority.
         optional<authority> active;

         optional<authority> limitactive;

         /// New account options
         optional<account_options> new_options;

         optional<special_authority> owner_special_authority;
         optional<special_authority> active_special_authority;

         extensions_type extensions;

         account_id_type fee_payer() const { return account; }
         void validate() const;
         share_type calculate_fee(const fee_parameters_type &k) const;
         share_type get_fee(const fee_parameters_type &, uint32_t) const { return 0; }
         bool is_owner_update() const
         {
            return owner || owner_special_authority.valid();
         }

         void get_required_owner_authorities(flat_set<account_id_type> &a) const
         {
            if (is_owner_update())
               a.insert(account);
         }

         void get_required_active_authorities(flat_set<account_id_type> &a) const
         {
            if (!is_owner_update())
               a.insert(account);
         }
      };

      struct account_config_operation : public base_operation
      {
         struct fee_parameters_type
         {
            share_type fee = 1000;
            uint32_t price_per_kbyte = 100;
         };

         asset fee;
         /// The account to update
         account_id_type issuer;
         flat_map<string, string> config;
         uint32_t op_type = 0;

         extensions_type extensions;

         account_id_type fee_payer() const { return issuer; }
         void validate() const;
         share_type calculate_fee(const fee_parameters_type &k) const;
         share_type get_fee(const fee_parameters_type &, uint32_t) const { return 0; }
      };

      struct account_authenticate_operation : public base_operation
      {
         struct fee_parameters_type
         {
            share_type fee = 1000;
            uint32_t price_per_kbyte = 100;
         };

         asset                     fee;
         /// The account to update
         account_id_type               issuer;
         uint32_t                      op_type;            //1 修改靓号提案人     3 锁定账户   4 修改用户验证公钥 5 修改用户数据 6 验证用户    7  修改网关验证信息 8  修改运营商验证信息 9 绑定手机号邮箱 10 运营商是否支持 投资，借款 11 认证人奖励
         optional<account_id_type>     auth_flag;          //7  网关，8 运营商  缺省认证人 
         optional<bool>                lock_balance;       //3
         optional<public_key_type>     auth_key;           //4
         optional<account_author>      auth_account;       //6
         optional<account_auth>        auth_data;          //5  9 
         optional<uint32_t>            flags;              //1  10 运营商是否支持 投资，借款
         optional<uint32_t>            need_auth;          //7  网关，8 运营商 认证配置
         optional<flat_map<asset_id_type,account_id_type>>  trust_auth;         //7  网关，8 运营商 信任认证人               

         extensions_type extensions;

         account_id_type fee_payer() const { return issuer; }
         void validate() const;
         share_type calculate_fee(const fee_parameters_type &k) const;
         share_type get_fee(const fee_parameters_type &, uint32_t) const { return 0; }
         void get_required_owner_authorities(flat_set<account_id_type> &a) const
         {
            if (op_type == 3)
               a.insert(issuer);
         }          
      };

      /**
    * @brief This operation is used to whitelist and blacklist accounts, primarily for transacting in whitelisted assets
    * @ingroup operations
    *
    * Accounts can freely specify opinions about other accounts, in the form of either whitelisting or blacklisting
    * them. This information is used in chain validation only to determine whether an account is authorized to transact
    * in an asset type which enforces a whitelist, but third parties can use this information for other uses as well,
    * as long as it does not conflict with the use of whitelisted assets.
    *
    * An asset which enforces a whitelist specifies a list of accounts to maintain its whitelist, and a list of
    * accounts to maintain its blacklist. In order for a given account A to hold and transact in a whitelisted asset S,
    * A must be whitelisted by at least one of S's whitelist_authorities and blacklisted by none of S's
    * blacklist_authorities. If A receives a balance of S, and is later removed from the whitelist(s) which allowed it
    * to hold S, or added to any blacklist S specifies as authoritative, A's balance of S will be frozen until A's
    * authorization is reinstated.
    *
    * This operation requires authorizing_account's signature, but not account_to_list's. The fee is paid by
    * authorizing_account.
    */
      struct account_whitelist_operation : public base_operation
      {
         struct fee_parameters_type
         {
            share_type fee = 10000;
         };
         enum account_listing
         {
            no_listing = 0x0,                                    ///< No opinion is specified about this account
            white_listed = 0x1,                                  ///< This account is whitelisted, but not blacklisted
            black_listed = 0x2,                                  ///< This account is blacklisted, but not whitelisted
            white_and_black_listed = white_listed | black_listed ///< This account is both whitelisted and blacklisted
         };

         /// Paid by authorizing_account
         asset fee;
         /// The account which is specifying an opinion of another account
         account_id_type authorizing_account;
         /// The account being opined about
         account_id_type account_to_list;
         /// The new white and blacklist status of account_to_list, as determined by authorizing_account
         /// This is a bitfield using values defined in the account_listing enum
         uint8_t new_listing = no_listing;

         extensions_type extensions;

         account_id_type fee_payer() const { return authorizing_account; }
         void validate() const
         {
            FC_ASSERT(fee.amount >= 0);
            FC_ASSERT(new_listing < 0x4);
         }
      };

      /**
    * @brief Manage an account's membership status
    * @ingroup operations
    *
    * This operation is used to upgrade an account to a member, or renew its subscription. If an account which is an
    * unexpired annual subscription member publishes this operation with @ref upgrade_to_lifetime_member set to false,
    * the account's membership expiration date will be pushed backward one year. If a basic account publishes it with
    * @ref upgrade_to_lifetime_member set to false, the account will be upgraded to a subscription member with an
    * expiration date one year after the processing time of this operation.
    *
    * Any account may use this operation to become a lifetime member by setting @ref upgrade_to_lifetime_member to
    * true. Once an account has become a lifetime member, it may not use this operation anymore.
    */
      struct account_upgrade_operation : public base_operation
      {
         struct fee_parameters_type
         {

            uint64_t membership_annual_fee = 5000 * GRAPHENE_BLOCKCHAIN_PRECISION;
            uint64_t membership_lifetime_fee = 1000 * GRAPHENE_BLOCKCHAIN_PRECISION; ///< the cost to upgrade to a lifetime member
         };

         asset fee;
         /// The account to upgrade; must not already be a lifetime member
         account_id_type account_to_upgrade;
         /// If true, the account will be upgraded to a lifetime member; otherwise, it will add a year to the subscription
         bool upgrade_to_lifetime_member = false;
         extensions_type extensions;

         account_id_type fee_payer() const { return account_to_upgrade; }
         void validate() const;
         share_type calculate_fee(const fee_parameters_type &k) const;
         share_type get_fee(const fee_parameters_type &, uint32_t) const { return 0; }
      };

      /**
    * @brief transfers the account to another account while clearing the white list
    * @ingroup operations
    *
    * In theory an account can be transferred by simply updating the authorities, but that kind
    * of transfer lacks semantic meaning and is more often done to rotate keys without transferring
    * ownership.   This operation is used to indicate the legal transfer of title to this account and
    * a break in the operation history.
    *
    * The account_id's owner/active/voting/memo authority should be set to new_owner
    *
    * This operation will clear the account's whitelist statuses, but not the blacklist statuses.
    */
      struct account_transfer_operation : public base_operation
      {
         struct fee_parameters_type
         {
            uint64_t fee = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
         };

         asset fee;
         account_id_type account_id;
         account_id_type new_owner;
         extensions_type extensions;

         account_id_type fee_payer() const { return account_id; }
         void validate() const;
      };

      struct account_coupon_operation : public base_operation
      {
         struct fee_parameters_type
         {
            uint64_t fee = 1 * GRAPHENE_BLOCKCHAIN_PRECISION;
         };

         asset fee;
         account_id_type issuer;
         extensions_type extensions;
         account_id_type fee_payer() const { return issuer; }
         share_type calculate_fee(const fee_parameters_type &k) const { return k.fee; }
         void validate() const;
      };

      struct withdraw_exchange_fee_operation : public base_operation
      {
         struct fee_parameters_type
         {
            uint64_t fee = 1 * GRAPHENE_BLOCKCHAIN_PRECISION;
         };

         asset fee;
         account_id_type issuer;
         share_type core_amount;          //ZOS  数量，这个将从worker费用中扣除
         asset_id_type withdraw_asset_id; //目标币的种类，需要兑换成什么币

         extensions_type extensions;
         account_id_type fee_payer() const { return issuer; }
         share_type calculate_fee(const fee_parameters_type &k) const { return k.fee; }
         void validate() const;
      };

} } // graphene::chain

FC_REFLECT( graphene::chain:: account_auth,
            (authenticator)(info)(key))
FC_REFLECT( graphene::chain:: account_author,
            (state)(expiration)(account)(hash64))   

FC_REFLECT( graphene::chain::lock_recommend,
            (asset_id)
            (lockmode)
            (period)
            (automode)
            (lable)
            (automode)
            (des)
          )        

FC_REFLECT( graphene::chain::sell_pair,
            (first_id)
            (first_symbol)
            (second_id)
            (second_symbol)
            (need_author)
          )        


            
FC_REFLECT(graphene::chain::account_options, (memo_key)(auth_key)(voting_account)(num_witness)(num_committee)(num_budget)(votes)(extensions))
FC_REFLECT_ENUM( graphene::chain::account_whitelist_operation::account_listing,
                (no_listing)(white_listed)(black_listed)(white_and_black_listed))

//FC_REFLECT(graphene::chain::account_create_operation::ext, (null_ext)(owner_special_authority)(active_special_authority)(buyback_options) )
//FC_REFLECT(graphene::chain::account_create_operation::ext, (null_ext))
FC_REFLECT( graphene::chain::account_create_operation,
            (fee)(registrar)
            (referrer)(referrer_percent)
            (name)(owner)(active)(limitactive)(options)                    
            (owner_special_authority)
            (active_special_authority)
            (buyback_options)
            (extensions)
          )

//FC_REFLECT(graphene::chain::account_update_operation::ext, (null_ext)(owner_special_authority)(active_special_authority) )
FC_REFLECT( graphene::chain::account_update_operation,
            (fee)(account)(owner)(active)(limitactive)(new_options)            
            (owner_special_authority)
            (active_special_authority)
            (extensions)
          ) 
FC_REFLECT(graphene::chain::carrier_cfg,
            (assetid)
            (symbol)
            (uasset_property) 
            (lendCarrier)
            (investCarrier)
            (lendname)
            (investname)
            (lendUrl)
            (investUrl) 
            (lendaccount)
            (investaccount)
            (lendauthor)
            (lendauthor_account)
            (lendauthor_url)
            (lendauthor_name)
            (investauthor)
            (investauthor_account)
            (investauthor_url)
            (investauthor_name)
            (lendenable)
            (investenable)
            (lendvalidate)
            (investvalidate)
            )  
FC_REFLECT(graphene::chain::collateral_cfg,
            (loan)
            (invest)
            (rate)
            (interest)
            (setting)
            )  
FC_REFLECT(graphene::chain::asset_cfg,
            (id)
            (symbol)         
            (max_supply)
            (uasset_property)
            (precision)        
            )      
  
FC_REFLECT(graphene::chain::gateway_cfg,
            (asset_id)
            (symbol)
            (real_symbol)
            (real_asset_sub)
            (uasset_property)
            (gateway_id)
            (account_id)
            (account_name)
            (gateway_url)
            (precision)
            (deposit_amount)
            (withdraw_amount)
            (account_id) 
            (author_id)
            (author_account_id)
            (author_url)
            (author_name)
            (enable)
            )  
 


FC_REFLECT( graphene::chain::account_authenticate_operation,
            (fee)
            (issuer)
            (op_type)
            (auth_flag)
            (lock_balance)
            (auth_key)
            (auth_account)
            (auth_data)
            (flags)
            (need_auth)
            (trust_auth)   
            (extensions)
          ) 

FC_REFLECT( graphene::chain::account_config_operation,
            (fee)
            (issuer)     
            (config)
            (op_type)
            (extensions)
          ) 


FC_REFLECT( graphene::chain::account_upgrade_operation,
            (fee)(account_to_upgrade)(upgrade_to_lifetime_member)(extensions) )

FC_REFLECT( graphene::chain::account_whitelist_operation, (fee)(authorizing_account)(account_to_list)(new_listing)(extensions))

FC_REFLECT( graphene::chain::account_create_operation::fee_parameters_type, (basic_fee)(premium_fee)(price_per_kbyte)(char_fee))
FC_REFLECT( graphene::chain::account_whitelist_operation::fee_parameters_type,  (fee) )
FC_REFLECT( graphene::chain::account_update_operation::fee_parameters_type,   (fee)(price_per_kbyte))
FC_REFLECT( graphene::chain::account_authenticate_operation::fee_parameters_type,    (fee)(price_per_kbyte))
FC_REFLECT( graphene::chain::account_config_operation::fee_parameters_type,    (fee)(price_per_kbyte))
FC_REFLECT( graphene::chain::account_upgrade_operation::fee_parameters_type,   (membership_annual_fee)(membership_lifetime_fee))
FC_REFLECT( graphene::chain::account_transfer_operation::fee_parameters_type,  (fee) )
FC_REFLECT( graphene::chain::account_coupon_operation::fee_parameters_type,  (fee) )
FC_REFLECT( graphene::chain::withdraw_exchange_fee_operation::fee_parameters_type,  (fee))


FC_REFLECT( graphene::chain::account_transfer_operation, (fee)(account_id)(new_owner)(extensions) )
FC_REFLECT( graphene::chain::account_coupon_operation, (fee)(issuer) (extensions))
FC_REFLECT( graphene::chain::withdraw_exchange_fee_operation, (fee)(issuer)(core_amount)(withdraw_asset_id)(extensions))
   