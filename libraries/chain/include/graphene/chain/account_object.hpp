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
#include <graphene/chain/protocol/operations.hpp>
#include <graphene/db/generic_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <graphene/chain/protocol/memo.hpp>

namespace graphene { namespace chain {
   class database;

   /**
    * @class account_statistics_object
    * @ingroup object
    * @ingroup implementation
    *
    * This object contains regularly updated statistical data about an account. It is provided for the purpose of
    * separating the account data that changes frequently from the account data that is mostly static, which will
    * minimize the amount of data that must be backed up as part of the undo history everytime a transfer is made.
    */
   class account_statistics_object : public graphene::db::abstract_object<account_statistics_object>
   {
      public:
         static const uint8_t space_id = implementation_ids;
         static const uint8_t type_id  = impl_account_statistics_object_type;

         account_id_type  owner;
         /**
          * Keep the most recent operation as a root pointer to a linked list of the transaction history.
          */
         account_transaction_history_id_type most_recent_op;
         /** Total operations related to this account. */
         uint32_t                            total_ops = 0;
         /** Total operations related to this account that has been removed from the database. */
         uint32_t                            removed_ops = 0;

         /**
          * When calculating votes it is necessary to know how much is stored in orders (and thus unavailable for
          * transfers). Rather than maintaining an index of [asset,owner,order_id] we will simply maintain the running
          * total here and update it every time an order is created or modified.
          */
         share_type total_core_in_orders;
         /**
          * Tracks the total fees paid by this account for the purpose of calculating bulk discounts.
          */
         share_type lifetime_fees_paid;

         /**
          * Tracks the fees paid by this account which have not been disseminated to the various parties that receive
          * them yet (registrar, referrer, lifetime referrer, network, etc). This is used as an optimization to avoid
          * doing massive amounts of uint128 arithmetic on each and every operation.
          *
          * These fees will be paid out as vesting cash-back, and this counter will reset during the maintenance
          * interval.
          */
         //锁定费用
         share_type pending_fees;
         /**
          * Same as @ref pending_fees, except these fees will be paid out as pre-vested cash-back (immediately
          * available for withdrawal) rather than requiring the normal vesting period.
          */
         //非锁定费用
         share_type pending_vested_fees;
         //身份锁定的总和，要计算在计票中
         asset                              lock_asset;

         share_type                         exchange_fees = 0;  //可提取并转换的费用
         
         //优惠卷费用, 燃烧币，最后直接回收，不计入分配
         share_type                         coupon_fees = 0;    
         //优惠卷，剩余优惠卷
         share_type                         amount_coupon = 0;  
         //已经领取优惠卷的月份
         uint32_t                           coupon_month = 0;  

         //锁仓总和,要计算在计票中
         share_type                         lock_token; 

         fc::time_point_sec                 activate_time = fc::time_point_sec::min();
         uint64_t                           activate_count = 0;
         uint64_t                           statistics_count = 0;

         flat_map<asset_id_type,vesting_balance_id_type> locktoken_vb;

         /// @brief Split up and pay out @ref pending_fees and @ref pending_vested_fees
         void process_fees(const account_object& a, database& d) const;

         /**
          * Core fees are paid into the account_statistics_object by this method
          */
         void pay_fee(asset_id_type core_id, share_type core_fee, share_type cashback_vesting_threshold );

         void set_coupon(database& d,share_type fee,bool bset);
   };

   /**
    * @brief Tracks the balance of a single account/asset pair
    * @ingroup object
    *
    * This object is indexed on owner and asset_type so that black swan
    * events in asset_type can be processed quickly.
    */
   class account_balance_object : public abstract_object<account_balance_object>
   {
      public:
         static const uint8_t space_id = implementation_ids;
         static const uint8_t type_id  = impl_account_balance_object_type;

         account_id_type   owner;
         asset_id_type     asset_type;
         share_type        balance;

         asset get_balance()const { return asset(balance, asset_type); }
         void  adjust_balance(const asset& delta);
   };

   class account_balance_object_ex : public account_balance_object {
     public: 
      fc::string  name;
      fc::string  symbol;
      uint8_t     precision;
   };

 

   /**
    * @brief This class represents an account on the object graph
    * @ingroup object
    * @ingroup protocol
    *
    * Accounts are the primary unit of authority on the graphene system. Users must have an account in order to use
    * assets, trade in the markets, vote for committee_members, etc.
    */
   #define  account_gateway          0x00000001
   #define  account_carrier          0x00000002
   #define  account_genesis          0x00000004 
   #define  account_platform         0x00000008 
   #define  account_witness          0x00000010
   #define  account_committe         0x00000020 //
   #define  account_budget           0x00000040 // 
   #define  account_sub_committe     0x00000080 //小理事会成员
   #define  account_lock_balance     0x00000100 
   #define  account_sys_feeder       0x00000200
   #define  account_sys_fuacet       0x00000400
   #define  account_propose_register 0x01000000
   #define  account_authenticator    0x02000000
   #define  account_flag_mask        (account_propose_register)
   #define  account_mask             (account_lock_balance)
   #define  can_auth_mask            (account_authenticator|account_carrier|account_gateway)



   struct  account_auth_item
   {
      uint32_t            state;
      uint32_t            expiration = 0;
      uint64_t            hash64;               // data 的 hash 值，防止用户修改数据 (const char* buf, size_t len);    hash64
      fc::time_point_sec  auth_time;            //验证时间      
   };
   struct account_auth_data
   {      
      flat_map<account_id_type, account_auth_item> auth_state;    
      uint64_t                                     hash64;
      optional<memo_data>                          info;      
      memo_data                                    key;  
   };

   
   #define  account_type_lifemember     0x00000001  //搜索条件
   #define  account_type_comparename    0x00000002

   struct account_info
   {
      account_id_type id;
      string          name;
      string          info;
   };
   class account_object : public graphene::db::abstract_object<account_object>
   {
      public:
         static const uint8_t space_id = protocol_ids;
         static const uint8_t type_id  = account_object_type;      
         /**
          * The time at which this account's membership expires.
          * If set to any time in the past, the account is a basic account.
          * If set to time_point_sec::maximum(), the account is a lifetime member.
          * If set to any time not in the past less than time_point_sec::maximum(), the account is an annual member.
          *
          * See @ref is_lifetime_member, @ref is_basic_account, @ref is_annual_member, and @ref is_member
          */
         time_point_sec membership_expiration_date;
         time_point_sec register_date;

         ///The account that paid the fee to register this account. Receives a percentage of referral rewards.
         account_id_type registrar;
         /// The account credited as referring this account. Receives a percentage of referral rewards.
         account_id_type referrer;
         /// The lifetime member at the top of the referral tree. Receives a percentage of referral rewards.
         account_id_type lifetime_referrer;

         /// Percentage of fee which should go to network.
         uint16_t network_fee_percentage = GRAPHENE_DEFAULT_NETWORK_PERCENT_OF_FEE;
         /// Percentage of fee which should go to lifetime referrer.
         uint16_t lifetime_referrer_fee_percentage = 0;
         /// Percentage of referral rewards (leftover fee after paying network and lifetime referrer) which should go
         /// to referrer. The remainder of referral rewards goes to the registrar.
         uint16_t referrer_rewards_percentage = 0;

         /// The account's name. This name must be unique among all account names on the graph. May not be empty.
         string name;

         /**
          * The owner authority represents absolute control over the account. Usually the keys in this authority will
          * be kept in cold storage, as they should not be needed very often and compromise of these keys constitutes
          * complete and irrevocable loss of the account. Generally the only time the owner authority is required is to
          * update the active authority.
          */
         authority owner;
         /// The owner authority contains the hot keys of the account. This authority has control over nearly all
         /// operations the account may perform.
         authority active;

         authority limitactive;

         optional< account_auth_data>   auth_data;
         optional<memo_data>            user_info;    //9  用户手机信息
         typedef account_options        options_type;
         account_options options;

         /// The reference implementation records the account's statistics in a separate object. This field contains the
         /// ID of that object.
         account_statistics_id_type statistics;

         /**
          * This is a set of all accounts which have 'whitelisted' this account. Whitelisting is only used in core
          * validation for the purpose of authorizing accounts to hold and transact in whitelisted assets. This
          * account cannot update this set, except by transferring ownership of the account, which will clear it. Other
          * accounts may add or remove their IDs from this set.
          */
         flat_set<account_id_type> whitelisting_accounts;

         /**
          * Optionally track all of the accounts this account has whitelisted or blacklisted, these should
          * be made Immutable so that when the account object is cloned no deep copy is required.  This state is
          * tracked for GUI display purposes.
          *
          * TODO: move white list tracking to its own multi-index container rather than having 4 fields on an
          * account.   This will scale better because under the current design if you whitelist 2000 accounts,
          * then every time someone fetches this account object they will get the full list of 2000 accounts.
          */
         ///@{
         set<account_id_type> whitelisted_accounts;
         set<account_id_type> blacklisted_accounts;
         ///@}


         /**
          * This is a set of all accounts which have 'blacklisted' this account. Blacklisting is only used in core
          * validation for the purpose of forbidding accounts from holding and transacting in whitelisted assets. This
          * account cannot update this set, and it will be preserved even if the account is transferred. Other accounts
          * may add or remove their IDs from this set.
          */
         flat_set<account_id_type> blacklisting_accounts;

         /**
          * Vesting balance which receives cashback_reward deposits.
          */
         optional<vesting_balance_id_type> cashback_vb;
         optional<vesting_balance_id_type> locktoken_vb;

         special_authority owner_special_authority = no_special_authority();
         special_authority active_special_authority = no_special_authority();

         /**
          * This flag is set when the top_n logic sets both authorities,
          * and gets reset when authority or special_authority is set.
          */
         uint8_t top_n_control_flags = 0;
         static const uint8_t top_n_control_owner  = 1;
         static const uint8_t top_n_control_active = 2;

         
         uint32_t uaccount_property = 0;

         /**
          * This is a set of assets which the account is allowed to have.
          * This is utilized to restrict buyback accounts to the assets that trade in their markets.
          * In the future we may expand this to allow accounts to e.g. voluntarily restrict incoming transfers.
          */
         optional< flat_set<asset_id_type> > allowed_assets;

         flat_map<string, string> configs;

         bool has_special_authority()const
         {
            return (owner_special_authority.which() != special_authority::tag< no_special_authority >::value)
                || (active_special_authority.which() != special_authority::tag< no_special_authority >::value);
         }       
         vector<asset_id_type> get_auth_asset(const database& db,uint32_t type = 0x77) const;
         const bool is_auth_asset(const database& db,asset_id_type id,uint32_t type = 0x77) const;
         const bool is_gateway_enable(const database& d,asset_id_type ass_id) const;
         const bool is_gateway_enable(const database &db) const;
         const bool is_carrier_enable(const database& d) const;
         const bool is_author_enable(const database& d) const;
         const carrier_id_type get_carrier_id(const database &db) const;
         const gateway_id_type get_gateway_id(const database &db) const;
         const author_id_type  get_author_id(const database &db) const;

         /// @return true if this is a lifetime member account; false otherwise.
         bool is_lifetime_member()const
         {
            return membership_expiration_date >= time_point_sec::maximum();
         }
         bool is_gateway_member()const
         {
            return (uaccount_property &  account_gateway) == account_gateway;
         }
          bool is_carrier_member()const
         {
            return (uaccount_property &  account_carrier) == account_carrier;
         }
         bool is_committee_member()const
         {
            return (uaccount_property &  account_committe) == account_committe;
         }
           bool is_witness()const
         {
            return (uaccount_property &  account_witness) == account_witness;
         }
           bool is_budget_member()const
         {
            return (uaccount_property &  account_budget) == account_budget;
         }
         bool is_lock_balance()const
         {
            return (uaccount_property &  account_lock_balance) == account_lock_balance;
         }
         /// @return true if this is a basic account; false otherwise.
         bool is_basic_account(time_point_sec now)const
         {
            return now > membership_expiration_date;
         }
         /// @return true if the account is an unexpired annual member; false otherwise.
         /// @note This method will return false for lifetime members.
         bool is_annual_member(time_point_sec now)const
         {
            return !is_lifetime_member() && !is_basic_account(now);
         }
         /// @return true if the account is an annual or lifetime member; false otherwise.
         bool is_member(time_point_sec now)const
         {
            return !is_basic_account(now);
         }
         bool is_valid()const
         {
            return type_id>0;
         }
         void get_info(account_info &info) const
         {
            info.id = id;
            info.name = name;
         }

         account_id_type get_id()const { return id; }
   };

   /**
    *  @brief This secondary index will allow a reverse lookup of all accounts that a particular key or account
    *  is an potential signing authority.
    */
   class account_member_index : public secondary_index
   {
      public:
         virtual void object_inserted( const object& obj ) override;
         virtual void object_removed( const object& obj ) override;
         virtual void about_to_modify( const object& before ) override;
         virtual void object_modified( const object& after  ) override;


         /** given an account or key, map it to the set of accounts that reference it in an active or owner authority */
         map< account_id_type, set<account_id_type> > account_to_account_memberships;
         map< public_key_type, set<account_id_type> > account_to_key_memberships;
         /** some accounts use address authorities in the genesis block */
         map< address, set<account_id_type> >         account_to_address_memberships;


      protected:
         set<account_id_type>  get_account_members( const account_object& a )const;
         set<public_key_type>  get_key_members( const account_object& a )const;
         set<address>          get_address_members( const account_object& a )const;

         set<account_id_type>  before_account_members;
         set<public_key_type>  before_key_members;
         set<address>          before_address_members;
   };


   /**
    *  @brief This secondary index will allow a reverse lookup of all accounts that have been referred by
    *  a particular account.
    */
   class account_referrer_index : public secondary_index
   {
      public:
         virtual void object_inserted( const object& obj ) override;
         virtual void object_removed( const object& obj ) override;
         virtual void about_to_modify( const object& before ) override;
         virtual void object_modified( const object& after  ) override;

         /** maps the referrer to the set of accounts that they have referred */
         map< account_id_type, set<account_id_type> > referred_by;
   };

   struct by_account_asset;
   struct by_asset_balance;
   struct by_balances;
   /**
    * @ingroup object_index
    */
   typedef multi_index_container<
      account_balance_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
         ordered_non_unique< tag<by_balances>, member< account_balance_object, share_type, &account_balance_object::balance > >,
         ordered_unique< tag<by_account_asset>,
            composite_key<
               account_balance_object,
               member<account_balance_object, account_id_type, &account_balance_object::owner>,
               member<account_balance_object, asset_id_type, &account_balance_object::asset_type>
            >
         >,
         ordered_unique< tag<by_asset_balance>,
            composite_key<
               account_balance_object,
               member<account_balance_object, asset_id_type, &account_balance_object::asset_type>,
               member<account_balance_object, share_type, &account_balance_object::balance>,
               member<account_balance_object, account_id_type, &account_balance_object::owner>
            >,
            composite_key_compare<
               std::less< asset_id_type >,
               std::greater< share_type >,
               std::less< account_id_type >
            >
         >
      >
   > account_balance_object_multi_index_type;

   /**
    * @ingroup object_index
    */
   typedef generic_index<account_balance_object, account_balance_object_multi_index_type> account_balance_index;

   struct by_name{};

   /**
    * @ingroup object_index
    */
   typedef multi_index_container<
      account_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
         ordered_unique< tag<by_name>, member<account_object, string, &account_object::name> >
      >
   > account_multi_index_type;
   /**
    * @ingroup object_index
    */
   typedef generic_index<account_object, account_multi_index_type> account_index;

}}

FC_REFLECT( graphene::chain:: account_auth_item,
            (state)(expiration)(hash64)(auth_time)
            )

 FC_REFLECT( graphene::chain::account_info,
                  (id)
                  (name)                
                  (info)    
                  )       

FC_REFLECT( graphene::chain:: account_auth_data,            
            (auth_state)            
            (hash64)
            (info)
            (key)
            )

FC_REFLECT_DERIVED( graphene::chain::account_object,
                    (graphene::db::object),
                    (membership_expiration_date)(register_date)(registrar)(referrer)(lifetime_referrer)
                    (network_fee_percentage)(lifetime_referrer_fee_percentage)(referrer_rewards_percentage)
                    (name)(owner)(active)(limitactive)(auth_data)(user_info)(options)(statistics)(whitelisting_accounts)(blacklisting_accounts)
                    (whitelisted_accounts)(blacklisted_accounts)
                    (cashback_vb)
                    (locktoken_vb)
                    (owner_special_authority)(active_special_authority)
                    (top_n_control_flags)                    
                    (uaccount_property)              
                    (allowed_assets)
                    (configs)
                    )
                    
FC_REFLECT_DERIVED( graphene::chain::account_balance_object,
                    (graphene::db::object),
                    (owner)(asset_type)(balance) )

FC_REFLECT_DERIVED( graphene::chain::account_balance_object_ex,
                    (graphene::chain::account_balance_object),
                    (name)(symbol)(precision) ) 

FC_REFLECT_DERIVED( graphene::chain::account_statistics_object,
                    (graphene::chain::object),
                    (owner)
                    (most_recent_op)
                    (total_ops)(removed_ops)
                    (total_core_in_orders)
                    (lifetime_fees_paid)
                    (pending_vested_fees)
                    (pending_fees)
                    (lock_asset)
                    (exchange_fees)
                    (coupon_fees)
                    (amount_coupon)
                    (coupon_month) 
                    (lock_token)
                    (activate_time)
                    (activate_count)
                    (statistics_count)       
                    (locktoken_vb)             
                  )

