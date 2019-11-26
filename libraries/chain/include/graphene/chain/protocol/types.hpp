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
#include <fc/container/flat_fwd.hpp>
#include <fc/io/varint.hpp>
#include <fc/io/enum_type.hpp>
#include <fc/crypto/sha224.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/optional.hpp>
#include <fc/safe.hpp>
#include <fc/container/flat.hpp>
#include <fc/string.hpp>

#include <graphene/chain/protocol/ext.hpp>

#include <fc/io/raw.hpp>
#include <fc/uint128.hpp>
#include <fc/static_variant.hpp>
#include <fc/smart_ref_fwd.hpp>

#include <memory>
#include <vector>
#include <deque>
#include <cstdint>
#include <graphene/chain/protocol/address.hpp>
#include <graphene/db/object_id.hpp>
#include <graphene/chain/protocol/config.hpp>

#include <boost/rational.hpp>

namespace graphene { namespace chain {
   using namespace graphene::db;

   using                               std::map;
   using                               std::vector;
   using                               std::unordered_map;
   using                               std::string;
   using                               std::deque;
   using                               std::shared_ptr;
   using                               std::weak_ptr;
   using                               std::unique_ptr;
   using                               std::set;
   using                               std::pair;
   using                               std::enable_shared_from_this;
   using                               std::tie;
   using                               std::make_pair;

   using                               fc::smart_ref;
   using                               fc::variant_object;
   using                               fc::variant;
   using                               fc::enum_type;
   using                               fc::optional;
   using                               fc::unsigned_int;
   using                               fc::unsigned_int64;
   using                               fc::signed_int;
   using                               fc::time_point_sec;
   using                               fc::time_point;
   using                               fc::safe;
   using                               fc::flat_map;
   using                               fc::flat_set;
   using                               fc::static_variant;
   using                               fc::ecc::range_proof_type;
   using                               fc::ecc::range_proof_info;
   using                               fc::ecc::commitment_type;
   struct void_t{};

   typedef fc::ecc::private_key        private_key_type;
   typedef fc::sha256 chain_id_type;

   typedef boost::rational< int32_t >   ratio_type;

   #define miner_max  5
  

   enum miner_type
   {
     miner_witness,     
     miner_locktoken, 
     miner_worker, 
   };
   enum identity_type
   {
     identity_disable,
     identity_enable,    
     identity_enable_lost, 
   }; 
  
   enum asset_issuer_permission_flags
   {
      charge_market_fee    = 0x01, /**< an issuer-specified percentage of all market trades in this asset is paid to the issuer */
      white_list           = 0x02, /**< accounts must be whitelisted in order to hold this asset */
      override_authority   = 0x04, /**< issuer may transfer asset back to himself */
      transfer_restricted  = 0x08, /**< require the issuer to be one party to every transfer */
      disable_force_settle = 0x10, /**< disable force settling */
      global_settle        = 0x20, /**< allow the bitasset issuer to force a global settling -- this may be set in permissions, but not flags */
      disable_confidential = 0x40, /**< allow the asset to be used with confidential transactions */
      witness_fed_asset    = 0x80, /**< allow the asset to be fed by witnesses */
      committee_fed_asset  = 0x100 /**< allow the asset to be fed by the committee */
   };
   const static uint32_t ASSET_ISSUER_PERMISSION_MASK = charge_market_fee|white_list|override_authority|transfer_restricted|disable_force_settle|global_settle|disable_confidential
      |witness_fed_asset|committee_fed_asset;
   const static uint32_t UIA_ASSET_ISSUER_PERMISSION_MASK = charge_market_fee|white_list|override_authority|transfer_restricted|disable_confidential;

   enum reserved_spaces
   {
      relative_protocol_ids = 0,
      protocol_ids          = 1,
      implementation_ids    = 2,
      marke_history_ids     = 3,
      bitlender_history_ids = 4,
      finance_history_ids   = 5
   };

   inline bool is_relative( object_id_type o ){ return o.space() == 0; }
   inline bool is_safe_string(const std::string & str )
   {
      return (str.find("\"") == std::string::npos) && (str.find("<") == std::string::npos) && (str.find(">") == std::string::npos)  && (str.find("\'")==std::string::npos) ; 
   }
   inline bool is_safe_js_string(const std::string & str )
   {
      return (str.find("<") == std::string::npos) && (str.find(">") == std::string::npos); 
   }

   /**
    *  List all object types from all namespaces here so they can
    *  be easily reflected and displayed in debug output.  If a 3rd party
    *  wants to extend the core code then they will have to change the
    *  packed_object::type field from enum_type to uint16 to avoid
    *  warnings when converting packed_objects to/from json.
    */
   enum object_type
   {
      null_object_type,//0
      base_object_type,//1
      account_object_type,//2
      asset_object_type,    //  3
      force_settlement_object_type,//4
      committee_member_object_type,  //5
      witness_object_type,//6
      limit_order_object_type,//7
      call_order_object_type,//8
      custom_object_type,//9
      proposal_object_type,//10
      operation_history_object_type,//11
      withdraw_permission_object_type,//12
      vesting_balance_object_type,//13
      worker_object_type,//14
      balance_object_type, //15     
      bitlender_option_object_type,//16
      bitlender_order_object_type,//17
      issue_fundraise_object_type,//18
      buy_fundraise_object_type,//19
      gateway_object_type,//20
      carrier_object_type,//21
      budget_member_object_type,   //22
      sell_exchange_object_type,//23
      buy_exchange_object_type,//24
      author_object_type,//25
      locktoken_object_type,//26
      locktoken_options_object_type,//27      
      OBJECT_TYPE_COUNT //28 /< Sentry value which contains the number of different object types
   };

   enum impl_object_type
   {
      impl_global_property_object_type,          //0
      impl_dynamic_global_property_object_type,  //1
      impl_reserved0_object_type,                //2 formerly index_meta_object_type, TODO: delete me
      impl_asset_dynamic_data_type,//3
      impl_asset_bitasset_data_type,//4
      impl_account_balance_object_type,//5
      impl_account_statistics_object_type,//6
      impl_transaction_object_type,//7
      impl_block_summary_object_type,//8
      impl_account_transaction_history_object_type,//9
      impl_blinded_balance_object_type,//10
      impl_chain_property_object_type,//11
      impl_witness_schedule_object_type,//12
      impl_budget_record_object_type,//13
      impl_special_authority_object_type,//14
      impl_buyback_object_type,//15
      impl_fba_accumulator_object_type,//16
      impl_collateral_bid_object_type,//17
      
      impl_bitlender_invest_object_type,//18
      impl_account_balance_history_object_type,//19
      impl_bitlender_paramers_object_type,//20
      impl_asset_exchange_feed_type,//21
      impl_finance_paramers_object_type,//22
      impl_carrier_dynamic_object_type,//23
      impl_gateway_dynamic_object_type,//24
      impl_locktoken_history_object_type,//25
   };

   //typedef fc::unsigned_int            object_id_type;
   //typedef uint64_t                    object_id_type;
   class account_object;
   class committee_member_object;
   class budget_member_object;
   class witness_object;
   class carrier_object;
   class author_object;
   class gateway_object;
   class asset_object;
   class asset_exchange_feed_object;
   class asset_bitasset_data_object;
   class bitlender_option_object;
   class bitlender_paramers_object;
   class finance_paramers_object;
   class gateway_dynamic_object;
   class carrier_dynamic_object;
   class issue_fundraise_object;
   class force_settlement_object;
   class limit_order_object;
   class bitlender_order_object;   
   class bitlender_invest_object;
   class buy_fundraise_object;
   class call_order_object;
   class custom_object;
   class proposal_object;
   class operation_history_object;
   class balance_history_;
   class withdraw_permission_object;
   class vesting_balance_object;
   class worker_object;
   class locktoken_object;
   class locktoken_option_object;
   class sell_exchange_object;
   class buy_exchange_object;
   class balance_object;
   class blinded_balance_object;

   typedef object_id< protocol_ids, account_object_type,                account_object>                  account_id_type;
   typedef object_id< protocol_ids, asset_object_type,                  asset_object>                    asset_id_type;
   typedef object_id< protocol_ids, bitlender_option_object_type,       bitlender_option_object>         bitlender_option_id_type;
   typedef object_id< protocol_ids, issue_fundraise_object_type,        issue_fundraise_object>          issue_fundraise_id_type;
   typedef object_id< protocol_ids, force_settlement_object_type,       force_settlement_object>         force_settlement_id_type;
   typedef object_id< protocol_ids, committee_member_object_type,       committee_member_object>         committee_member_id_type;
   typedef object_id< protocol_ids, budget_member_object_type,          budget_member_object>            budget_member_id_type;
   typedef object_id< protocol_ids, witness_object_type,                witness_object>                  witness_id_type;
   typedef object_id< protocol_ids, gateway_object_type,                gateway_object>                  gateway_id_type;
   typedef object_id< protocol_ids, carrier_object_type,                carrier_object>                  carrier_id_type;
   typedef object_id< protocol_ids, author_object_type,                 author_object>                   author_id_type;
   typedef object_id< protocol_ids, limit_order_object_type,            limit_order_object>              limit_order_id_type;
   typedef object_id< protocol_ids, call_order_object_type,             call_order_object>               call_order_id_type;
   typedef object_id< protocol_ids, bitlender_order_object_type,        bitlender_order_object>          bitlender_order_id_type;   
   typedef object_id< protocol_ids, buy_fundraise_object_type,          buy_fundraise_object>            buy_fundraise_id_type;   
   typedef object_id< protocol_ids, custom_object_type,                 custom_object>                   custom_id_type;
   typedef object_id< protocol_ids, proposal_object_type,               proposal_object>                 proposal_id_type;
   typedef object_id< protocol_ids, operation_history_object_type,      operation_history_object>        operation_history_id_type;
   typedef object_id< protocol_ids, withdraw_permission_object_type,    withdraw_permission_object>      withdraw_permission_id_type;
   typedef object_id< protocol_ids, vesting_balance_object_type,        vesting_balance_object>          vesting_balance_id_type;
   typedef object_id< protocol_ids, worker_object_type,                 worker_object>                   worker_id_type;
   typedef object_id< protocol_ids, locktoken_object_type,              locktoken_object>                locktoken_id_type;
   typedef object_id< protocol_ids, locktoken_options_object_type,      locktoken_option_object>         locktoken_option_id_type;
   typedef object_id< protocol_ids, balance_object_type,                balance_object>                  balance_id_type;
   typedef object_id< protocol_ids, sell_exchange_object_type,          sell_exchange_object>            sell_exchange_id_type;
   typedef object_id< protocol_ids, buy_exchange_object_type,           buy_exchange_object>             buy_exchange_id_type;

   // implementation types
   class global_property_object;
   class dynamic_global_property_object;
   class asset_dynamic_data_object;
   class asset_bitasset_data_object;
   class asset_exchange_feed_object;
   class account_balance_object;
   class account_statistics_object;
   class transaction_object;
   class block_summary_object;
   class account_transaction_history_object;
   class chain_property_object;
   class witness_schedule_object;
   class budget_record_object;
   class special_authority_object;
   class buyback_object;
   class fba_accumulator_object;
   class collateral_bid_object;
   class balance_history_object;
   
   typedef object_id<implementation_ids, impl_global_property_object_type, global_property_object> global_property_id_type;
   typedef object_id< implementation_ids, impl_dynamic_global_property_object_type,  dynamic_global_property_object>    dynamic_global_property_id_type;
   typedef object_id< implementation_ids, impl_asset_dynamic_data_type,      asset_dynamic_data_object>                 asset_dynamic_data_id_type;
   typedef object_id< implementation_ids, impl_asset_bitasset_data_type,     asset_bitasset_data_object>                asset_bitasset_data_id_type;
   typedef object_id< implementation_ids, impl_asset_exchange_feed_type,             asset_exchange_feed_object>                        asset_feeds_id_type;


   typedef object_id< implementation_ids, impl_account_balance_object_type,  account_balance_object>                    account_balance_id_type;
   typedef object_id< implementation_ids, impl_account_statistics_object_type,account_statistics_object>                account_statistics_id_type;
   typedef object_id< implementation_ids, impl_transaction_object_type,      transaction_object>                        transaction_obj_id_type;
   typedef object_id< implementation_ids, impl_block_summary_object_type,    block_summary_object>                      block_summary_id_type;

   typedef object_id< implementation_ids, impl_account_transaction_history_object_type,account_transaction_history_object>       account_transaction_history_id_type;
   typedef object_id< implementation_ids, impl_account_balance_history_object_type,balance_history_object>                       account_balance_history_id_type;

   typedef object_id< implementation_ids, impl_chain_property_object_type,   chain_property_object>                     chain_property_id_type;
   typedef object_id< implementation_ids, impl_witness_schedule_object_type, witness_schedule_object>                   witness_schedule_id_type;
   typedef object_id< implementation_ids, impl_budget_record_object_type, budget_record_object >                        budget_record_id_type;
   typedef object_id< implementation_ids, impl_blinded_balance_object_type, blinded_balance_object >                    blinded_balance_id_type;
   typedef object_id< implementation_ids, impl_special_authority_object_type, special_authority_object >                special_authority_id_type;
   typedef object_id< implementation_ids, impl_buyback_object_type, buyback_object >                                    buyback_id_type;
   typedef object_id< implementation_ids, impl_fba_accumulator_object_type, fba_accumulator_object >                    fba_accumulator_id_type;
   typedef object_id< implementation_ids, impl_collateral_bid_object_type, collateral_bid_object >                      collateral_bid_id_type;
   typedef object_id< implementation_ids, impl_bitlender_invest_object_type, bitlender_invest_object >                  bitlender_invest_id_type;
   typedef object_id< implementation_ids, impl_bitlender_paramers_object_type, bitlender_paramers_object >              bitlender_paramers_id_type;
   typedef object_id< implementation_ids, impl_finance_paramers_object_type,   finance_paramers_object >                finance_paramers_id_type;
   typedef object_id< implementation_ids, impl_carrier_dynamic_object_type,   carrier_dynamic_object >                  carrier_dynamic_id_type;
   typedef object_id< implementation_ids, impl_gateway_dynamic_object_type,   gateway_dynamic_object >                  gateway_dynamic_id_type; 

   typedef fc::array<char, GRAPHENE_MAX_ASSET_SYMBOL_LENGTH>    symbol_type;
   typedef fc::ripemd160                                        block_id_type;
   typedef fc::ripemd160                                        checksum_type;
   typedef fc::ripemd160                                        transaction_id_type;
   typedef fc::sha256                                           digest_type;
   typedef fc::ecc::compact_signature                           signature_type;
   typedef safe<int64_t>                                        share_type;

   typedef uint16_t                                             weight_type;

   struct public_key_type
   {
       struct binary_key
       {
          binary_key() {}
          uint32_t                 check = 0;
          fc::ecc::public_key_data data;
       };
       fc::ecc::public_key_data key_data;
       public_key_type();
       public_key_type( const fc::ecc::public_key_data& data );
       public_key_type( const fc::ecc::public_key& pubkey );
       explicit public_key_type( const std::string& base58str );
       operator fc::ecc::public_key_data() const;
       operator fc::ecc::public_key() const;
       explicit operator std::string() const;
       friend bool operator == ( const public_key_type& p1, const fc::ecc::public_key& p2);
       friend bool operator == ( const public_key_type& p1, const public_key_type& p2);
       friend bool operator != ( const public_key_type& p1, const public_key_type& p2);
   };

   struct extended_public_key_type
   {
      struct binary_key
      {
         binary_key() {}
         uint32_t                   check = 0;
         fc::ecc::extended_key_data data;
      };
      
      fc::ecc::extended_key_data key_data;
       
      extended_public_key_type();
      extended_public_key_type( const fc::ecc::extended_key_data& data );
      extended_public_key_type( const fc::ecc::extended_public_key& extpubkey );
      explicit extended_public_key_type( const std::string& base58str );
      operator fc::ecc::extended_public_key() const;
      explicit operator std::string() const;
      friend bool operator == ( const extended_public_key_type& p1, const fc::ecc::extended_public_key& p2);
      friend bool operator == ( const extended_public_key_type& p1, const extended_public_key_type& p2);
      friend bool operator != ( const extended_public_key_type& p1, const extended_public_key_type& p2);
   };
   
   struct extended_private_key_type
   {
      struct binary_key
      {
         binary_key() {}
         uint32_t                   check = 0;
         fc::ecc::extended_key_data data;
      };
      
      fc::ecc::extended_key_data key_data;
       
      extended_private_key_type();
      extended_private_key_type( const fc::ecc::extended_key_data& data );
      extended_private_key_type( const fc::ecc::extended_private_key& extprivkey );
      explicit extended_private_key_type( const std::string& base58str );
      operator fc::ecc::extended_private_key() const;
      explicit operator std::string() const;
      friend bool operator == ( const extended_private_key_type& p1, const fc::ecc::extended_private_key& p2);
      friend bool operator == ( const extended_private_key_type& p1, const extended_private_key_type& p2);
      friend bool operator != ( const extended_private_key_type& p1, const extended_private_key_type& p2);
   };
} }  // graphene::chain

namespace fc
{
    void to_variant( const graphene::chain::public_key_type& var,  fc::variant& vo, uint32_t max_depth = 2 );
    void from_variant( const fc::variant& var,  graphene::chain::public_key_type& vo, uint32_t max_depth = 2 );
    void to_variant( const graphene::chain::extended_public_key_type& var, fc::variant& vo, uint32_t max_depth = 2 );
    void from_variant( const fc::variant& var, graphene::chain::extended_public_key_type& vo, uint32_t max_depth = 2 );
    void to_variant( const graphene::chain::extended_private_key_type& var, fc::variant& vo, uint32_t max_depth = 2 );
    void from_variant( const fc::variant& var, graphene::chain::extended_private_key_type& vo, uint32_t max_depth = 2 );
}

FC_REFLECT( graphene::chain::public_key_type, (key_data) )
FC_REFLECT( graphene::chain::public_key_type::binary_key, (data)(check) )
FC_REFLECT( graphene::chain::extended_public_key_type, (key_data) )
FC_REFLECT( graphene::chain::extended_public_key_type::binary_key, (check)(data) )
FC_REFLECT( graphene::chain::extended_private_key_type, (key_data) )
FC_REFLECT( graphene::chain::extended_private_key_type::binary_key, (check)(data) )

FC_REFLECT_ENUM( graphene::chain::object_type,
                 (null_object_type)
                 (base_object_type)
                 (account_object_type)
                 (force_settlement_object_type)
                 (asset_object_type)                 
                 (committee_member_object_type)             
                 (witness_object_type)
                 (limit_order_object_type)
                 (call_order_object_type)
                 (custom_object_type)
                 (proposal_object_type)
                 (operation_history_object_type)
                 (withdraw_permission_object_type)
                 (vesting_balance_object_type)
                 (worker_object_type)
                 (balance_object_type)                             
                 (bitlender_option_object_type)
                 (bitlender_order_object_type)
                 (issue_fundraise_object_type)
                 (buy_fundraise_object_type)
                 (gateway_object_type)
                 (carrier_object_type)               
                 (budget_member_object_type)   
                 (sell_exchange_object_type)
                 (buy_exchange_object_type)
                 (author_object_type)       
                 (locktoken_object_type)      
                 (locktoken_options_object_type)   
                 (OBJECT_TYPE_COUNT)
               )
FC_REFLECT_ENUM( graphene::chain::impl_object_type,
                 (impl_global_property_object_type)
                 (impl_dynamic_global_property_object_type)
                 (impl_reserved0_object_type)
                 (impl_asset_dynamic_data_type)
                 (impl_asset_bitasset_data_type)
                 (impl_account_balance_object_type)
                 (impl_account_statistics_object_type)
                 (impl_transaction_object_type)
                 (impl_block_summary_object_type)
                 (impl_account_transaction_history_object_type)
                 (impl_blinded_balance_object_type)
                 (impl_chain_property_object_type)
                 (impl_witness_schedule_object_type)
                 (impl_budget_record_object_type)
                 (impl_special_authority_object_type)
                 (impl_buyback_object_type)
                 (impl_fba_accumulator_object_type)
                 (impl_collateral_bid_object_type)
                 (impl_bitlender_invest_object_type)
                 (impl_account_balance_history_object_type)
                 (impl_bitlender_paramers_object_type)
                 (impl_asset_exchange_feed_type)
                 (impl_finance_paramers_object_type)
                 (impl_carrier_dynamic_object_type)
                 (impl_gateway_dynamic_object_type) 
                 (impl_locktoken_history_object_type)
               )

FC_REFLECT_TYPENAME( graphene::chain::share_type )

FC_REFLECT_TYPENAME( graphene::chain::account_id_type )
FC_REFLECT_TYPENAME( graphene::chain::asset_id_type )
FC_REFLECT_TYPENAME( graphene::chain::force_settlement_id_type )
FC_REFLECT_TYPENAME( graphene::chain::committee_member_id_type )
FC_REFLECT_TYPENAME( graphene::chain::budget_member_id_type )
FC_REFLECT_TYPENAME( graphene::chain::witness_id_type )
FC_REFLECT_TYPENAME( graphene::chain::limit_order_id_type )
FC_REFLECT_TYPENAME( graphene::chain::call_order_id_type )
FC_REFLECT_TYPENAME( graphene::chain::bitlender_order_id_type )
FC_REFLECT_TYPENAME( graphene::chain::custom_id_type )
FC_REFLECT_TYPENAME( graphene::chain::proposal_id_type )
FC_REFLECT_TYPENAME( graphene::chain::operation_history_id_type )
FC_REFLECT_TYPENAME( graphene::chain::withdraw_permission_id_type )
FC_REFLECT_TYPENAME( graphene::chain::vesting_balance_id_type )
FC_REFLECT_TYPENAME( graphene::chain::worker_id_type )
FC_REFLECT_TYPENAME( graphene::chain::locktoken_id_type)
FC_REFLECT_TYPENAME( graphene::chain::balance_id_type )
FC_REFLECT_TYPENAME( graphene::chain::global_property_id_type )
FC_REFLECT_TYPENAME( graphene::chain::dynamic_global_property_id_type )
FC_REFLECT_TYPENAME( graphene::chain::asset_dynamic_data_id_type )
FC_REFLECT_TYPENAME( graphene::chain::asset_bitasset_data_id_type )
FC_REFLECT_TYPENAME( graphene::chain::account_balance_id_type )
FC_REFLECT_TYPENAME( graphene::chain::account_statistics_id_type )
FC_REFLECT_TYPENAME( graphene::chain::transaction_obj_id_type )
FC_REFLECT_TYPENAME( graphene::chain::block_summary_id_type )
FC_REFLECT_TYPENAME( graphene::chain::account_transaction_history_id_type )
FC_REFLECT_TYPENAME( graphene::chain::budget_record_id_type )
FC_REFLECT_TYPENAME( graphene::chain::special_authority_id_type )
FC_REFLECT_TYPENAME( graphene::chain::buyback_id_type )
FC_REFLECT_TYPENAME( graphene::chain::fba_accumulator_id_type )
FC_REFLECT_TYPENAME( graphene::chain::collateral_bid_id_type )
FC_REFLECT_TYPENAME( graphene::chain::bitlender_invest_id_type )
FC_REFLECT_TYPENAME( graphene::chain::bitlender_paramers_id_type )

FC_REFLECT( graphene::chain::void_t, )

FC_REFLECT_ENUM( graphene::chain::asset_issuer_permission_flags,
   (charge_market_fee)
   (white_list)
   (transfer_restricted)
   (override_authority)
   (disable_force_settle)
   (global_settle)
   (disable_confidential)
   (witness_fed_asset)
   (committee_fed_asset)
   )

FC_REFLECT_ENUM( graphene::chain::miner_type,
   (miner_witness)    
   (miner_locktoken)    
   (miner_worker)
   )
   
FC_REFLECT_ENUM( graphene::chain::identity_type,
   (identity_disable)
   (identity_enable)  
   (identity_enable_lost)   
   ) 
 
