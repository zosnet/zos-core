/*
 * Copyright (c) 2017 Cryptonomex, Inc., and contributors.
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

#include <graphene/app/full_account.hpp>

#include <graphene/chain/protocol/types.hpp>

#include <graphene/chain/database.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/business/bitlender_option_object.hpp>
#include <graphene/chain/balance_object.hpp>
#include <graphene/chain/chain_property_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/budget_member_object.hpp>
#include <graphene/business/confidential_object.hpp>
#include <graphene/business/market_object.hpp>
#include <graphene/business/bitlender_object.hpp>
#include <graphene/business/finance_object.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/chain/worker_object.hpp>
#include <graphene/business/locktoken_object.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/carrier_object.hpp>
#include <graphene/chain/author_object.hpp>
#include <graphene/chain/gateway_object.hpp>
#include <graphene/chain/budget_record_object.hpp>
#include <graphene/market_history/market_history_plugin.hpp>
#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/bitlender_history/bitlender_history_plugin.hpp>
#include <graphene/locktoken_history/locktoken_history_plugin.hpp>
#include <graphene/finance_history/finance_history_plugin.hpp>


#include <fc/api.hpp>
#include <fc/optional.hpp>
#include <fc/variant_object.hpp>

#include <fc/network/ip.hpp>

#include <boost/container/flat_set.hpp>

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace graphene { namespace app {

using namespace graphene::chain;
using namespace graphene::market_history;
using namespace graphene::bitlender_history;
using namespace graphene::locktoken_history;
using namespace std;

class database_api_impl;

struct order
{
   string                     price;
   string                     quote;
   string                     base;
};

struct order_book
{
  string                      base;
  string                      quote;
  vector< order >             bids;
  vector< order >             asks;
};

struct market_ticker
{
   time_point_sec             time;
   string                     base;
   string                     quote;
   string                     latest;
   string                     lowest_ask;
   string                     highest_bid;
   string                     percent_change;
   string                     base_volume;
   string                     quote_volume;
};

struct market_volume
{
   time_point_sec             time;
   string                     base;
   string                     quote;
   string                     base_volume;
   string                     quote_volume;
};

struct market_trade
{
   int64_t                    sequence = 0;
   fc::time_point_sec         date;
   string                     price;
   string                     amount;
   string                     value;
   account_id_type            side1_account_id = GRAPHENE_NULL_ACCOUNT;
   account_id_type            side2_account_id = GRAPHENE_NULL_ACCOUNT;
};

/**
 * @brief The database_api class implements the RPC API for the chain database.
 *
 * This API exposes accessors on the database which query state tracked by a blockchain validating node. This API is
 * read-only; all modifications to the database must be performed via transactions. Transactions are broadcast via
 * the @ref network_broadcast_api.
 */
class database_api
{
   public:
      database_api(graphene::chain::database& db,application& app);
      ~database_api();

      /////////////
      // Objects //
      /////////////

      /**
       * @brief Get the objects corresponding to the provided IDs
       * @param ids IDs of the objects to retrieve
       * @return The objects retrieved, in the order they are mentioned in ids
       *
       * If any of the provided IDs does not map to an object, a null variant is returned in its position.
       */
      fc::variants get_objects(const vector<object_id_type>& ids)const;

      ///////////////////
      // Subscriptions //
      ///////////////////

      /**
       * @brief Register a callback handle which then can be used to subscribe to object database changes
       * @param cb The callback handle to register
       * @param nofity_remove_create Whether subscribe to universal object creation and removal events.
       *        If this is set to true, the API server will notify all newly created objects and ID of all
       *        newly removed objects to the client, no matter whether client subscribed to the objects.
       *        By default, API servers don't allow subscribing to universal events, which can be changed
       *        on server startup.
       */
      void set_subscribe_callback( std::function<void(const variant&)> cb, bool notify_remove_create );
      /**
       * @brief Register a callback handle which will get notified when a transaction is pushed to database
       * @param cb The callback handle to register
       *
       * Note: a transaction can be pushed to database and be popped from database several times while
       *   processing, before and after included in a block. Everytime when a push is done, the client will
       *   be notified.
       */
      void set_pending_transaction_callback( std::function<void(const variant& signed_transaction_object)> cb );
      /**
       * @brief Register a callback handle which will get notified when a block is pushed to database
       * @param cb The callback handle to register
       */
      void set_block_applied_callback( std::function<void(const variant& block_id)> cb );
      /**
       * @brief Stop receiving any notifications
       *
       * This unsubscribes from all subscribed markets and objects.
       */
      void cancel_all_subscriptions();

      /////////////////////////////
      // Blocks and transactions //
      /////////////////////////////

      /**
       * @brief Retrieve a block header
       * @param block_num Height of the block whose header should be returned
       * @return header of the referenced block, or null if no matching block was found
       */
      optional<block_header> get_block_header(uint64_t block_num)const;

      /**
      * @brief Retrieve multiple block header by block numbers
      * @param block_num vector containing heights of the block whose header should be returned
      * @return array of headers of the referenced blocks, or null if no matching block was found
      */
      map<uint64_t, optional<block_header>> get_block_header_batch(const vector<uint64_t> block_nums)const;


      /**
       * @brief Retrieve a full, signed block
       * @param block_num Height of the block to be returned
       * @return the referenced block, or null if no matching block was found
       */
      optional<signed_block> get_block(uint64_t block_num)const;
      optional<signed_block_info> get_block_ids(uint64_t block_num) const;

      optional<signed_block_info> get_block_with_id(uint64_t block_num, transaction_id_type trx_id)const;
      /**
       * @brief used to fetch an individual transaction.
       */
      processed_transaction get_transaction( uint64_t block_num, uint32_t trx_in_block )const;

      optional<processed_transaction> get_transaction_by_id( uint64_t block_num, transaction_id_type trx_id)const;
      /**
       * If the transaction has not expired, this method will return the transaction for the given ID or
       * it will return NULL if it is not known.  Just because it is not known does not mean it wasn't
       * included in the blockchain.
       */
      optional<signed_transaction> get_recent_transaction_by_id( const transaction_id_type& id )const;

      /////////////
      // Globals //
      /////////////

      /**
       * @brief Retrieve the @ref chain_property_object associated with the chain
       */
      chain_property_object get_chain_properties()const;

      /**
       * @brief Retrieve the current @ref global_property_object
       */
      global_property_object     get_global_properties()const;

      bitlender_paramers_object     get_bitlender_paramers()const;
      bitlender_paramers_object_key  get_bitlender_paramers_key(optional<bitlender_key> key)const;

      finance_paramers_object    get_finance_paramers()const;
      /**
       * @brief Retrieve compile-time constants
       */
      fc::variant_object get_config()const;

      /**
       * @brief Get the chain ID
       */
      chain_id_type get_chain_id()const;

      uint32_t              get_budget_pre_block( fc::time_point_sec now)const;
      
      /**
       * @brief Retrieve the current @ref dynamic_global_property_object
       */
      dynamic_global_property_object get_dynamic_global_properties()const;

      variant  get_block_summary();

      variant get_info() const;

      bool    is_plugin_enable(const std::string &plugin) const ;
      //////////
      // Keys //
      //////////

      vector<vector<account_id_type>> get_key_references( vector<public_key_type> key )const;

     /**
      * Determine whether a textual representation of a public key
      * (in Base-58 format) is *currently* linked
      * to any *registered* (i.e. non-stealth) account on the blockchain
      * @param public_key Public key
      * @return Whether a public key is known
      */
     bool is_public_key_registered(string public_key) const;

      //////////////
      // Accounts //
      //////////////

      /**
       * @brief Get a list of accounts by ID
       * @param account_ids IDs of the accounts to retrieve
       * @return The accounts corresponding to the provided IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<account_object>> get_accounts(const vector<account_id_type>& account_ids)const;
      vector<optional<account_statistics_object>> get_accounts_statistics(const vector<account_id_type> &account_ids) const;
      /**
       * @brief Fetch all objects relevant to the specified accounts and subscribe to updates
       * @param callback Function to call with updates
       * @param names_or_ids Each item must be the name or ID of an account to retrieve
       * @return Map of string from @ref names_or_ids to the corresponding account
       *
       * This function fetches all relevant objects for the given accounts, and subscribes to updates to the given
       * accounts. If any of the strings in @ref names_or_ids cannot be tied to an account, that input will be
       * ignored. All other accounts will be retrieved and subscribed.
       *
       */
      std::map<string,full_account> get_full_accounts( const vector<string>& names_or_ids, bool subscribe );

      optional<account_object> get_account_by_name( string name )const;
      vector<account_info> get_account_author( account_id_type account_id )const;
      /**
       *  @return all accounts that referr to the key or account id in their owner or active authorities.
       */
      vector<account_id_type> get_account_references( account_id_type account_id )const;
      vector<string>    get_account_config(account_id_type account_id ,vector<string> sindex)const;

      vector<operation_history_object>  get_account_history(account_id_type account,vector<int64_t> optypes,  int64_t start, int64_t offset) const;
      /**
       * @brief Get a list of accounts by name
       * @param account_names Names of the accounts to retrieve
       * @return The accounts holding the provided names
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<account_object>> lookup_account_names(const vector<string>& account_names)const;

      /**
       * @brief Get names and IDs for registered accounts
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @return Map of account names to corresponding IDs
       */
      map<string,account_id_type> lookup_accounts(const string& lower_bound_name, uint32_t limit)const;
      /**
       * @brief Get names and IDs for registered accounts
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @param utype
       * @return Map of account names to corresponding IDs
       */
      map<string, account_id_type> lookup_accounts_by_type(const string &lower_bound_name, uint32_t utype, uint32_t limit) const;
      vector<account_info>         lookup_accounts_by_property(uint32_t utype, uint32_t limit)const;
      vector<carrier_id_type>      lookup_carrier(const string& url)const;

      //////////////
      // Balances //
      //////////////

      /**
       * @brief Get an account's balances in various assets
       * @param id ID of the account to get balances for
       * @param assets IDs of the assets to get balances of; if empty, get all assets account has a balance in
       * @return Balances of the account
       */
      vector<asset_summary> get_account_balances(account_id_type id, const flat_set<asset_id_type>& assets)const;
      /// Semantically equivalent to @ref get_account_balances, but takes a name instead of an ID.
      vector<asset_summary> get_named_account_balances(const std::string& name, const flat_set<asset_id_type>& assets)const;

      vector<account_balance_object_ex> get_sort_balances(asset_id_type ass_id,share_type amount, uint32_t limit );

      vector<asset_summary> get_account_balances_summary(account_id_type id,uint32_t utype) const;

      vector<asset> get_account_lock_balances(account_id_type id, uint32_t utype) const;
    
     
      /** @return all unclaimed balance objects for a set of addresses */
      vector<balance_object> get_balance_objects( const vector<address>& addrs )const;
      

      vector<asset> get_vested_balances( const vector<balance_id_type>& objs )const;

      vector<vesting_balance_object> get_vesting_balances( account_id_type account_id )const;
      vector<vesting_balance_data> get_vesting_balances_data(account_id_type account_id) const;
      /**
       * @brief Get the total number of accounts registered with the blockchain
       */
      uint64_t get_account_count()const;

      uint64_t get_object_count(object_id_type id,bool bmaxid)const;

      ////////////
      // Assets //
      ////////////

      /**
       * @brief Get a list of assets by ID
       * @param asset_ids IDs of the assets to retrieve
       * @return The assets corresponding to the provided IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<asset_object>> get_assets(const vector<asset_id_type>& asset_ids)const;

      /**
       * @brief Get assets alphabetically by symbol name
       * @param lower_bound_symbol Lower bound of symbol names to retrieve
       * @param limit Maximum number of assets to fetch (must not exceed 100)
       * @return The assets found
       */
      vector<asset_object> list_assets(const string& lower_bound_symbol, uint32_t limit)const;

      /**
       * @brief Get a list of assets by symbol
       * @param asset_symbols Symbols or stringified IDs of the assets to retrieve
       * @return The assets corresponding to the provided symbols or IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<asset_object>> lookup_asset_symbols(const vector<string>& symbols_or_ids)const;

      /////////////////////
      // Markets / feeds //
      /////////////////////

      /**
       * @brief Get limit orders in a given market
       * @param a ID of asset being sold
       * @param b ID of asset being purchased
       * @param limit Maximum number of orders to retrieve
       * @return The limit orders, ordered from least price to greatest
       */
      vector<limit_order_object> get_limit_orders(asset_id_type a, asset_id_type b, uint32_t limit)const;
      vector<object_id_type>     get_account_limit_orders(account_id_type acc, asset_id_type a, asset_id_type b)const;

      /**
       * @brief Get call orders in a given asset
       * @param a ID of asset being called
       * @param limit Maximum number of orders to retrieve
       * @return The call orders, ordered from earliest to be called to latest
       */
      vector<call_order_object> get_call_orders(asset_id_type a, uint32_t limit)const;

      /**
       * @brief Get forced settlement orders in a given asset
       * @param a ID of asset being settled
       * @param limit Maximum number of orders to retrieve
       * @return The settle orders, ordered from earliest settlement date to latest
       */
      vector<force_settlement_object> get_settle_orders(asset_id_type a, uint32_t limit)const;

      /**
       * @brief Get collateral_bid_objects for a given asset
       * @param a ID of asset
       * @param limit Maximum number of objects to retrieve
       * @param start skip that many results
       * @return The settle orders, ordered from earliest settlement date to latest
       */
      vector<collateral_bid_object> get_collateral_bids(const asset_id_type asset, uint32_t limit, uint32_t start)const;

       /**
       * @brief get_loan_orders
       * @param status
       * @param lstart
       * @param limit
       * @return The settle orders, ordered from earliest settlement date to latest
       */

      vector<bitlender_order_info>      get_loan_orders(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const vector<uint32_t> &status,uint32_t start, uint32_t limit)const;
     
      uint32_t                          get_loan_counts(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const vector<uint32_t> &status)const;

      vector<bitlender_order_info>      get_notify_orders(uint32_t status, uint32_t start, uint32_t limit) const;
      vector<bitlender_order_info>      get_account_notify_orders(const account_id_type& id,uint32_t status, uint32_t start, uint32_t limit) const;
      /**
       * @brief get_invest_orders
       * @param status
       * @param lstart
       * @param limit
       * @return The settle orders, ordered from earliest settlement date to latest
       */

      vector<bitlender_invest_info>     get_invest_orders(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const vector<uint32_t> &status, uint32_t start, uint32_t limit)const;
      uint32_t                          get_invest_counts(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const vector<uint32_t> &status)const;



      /**
       *  @return all open margin positions for a given account id.
       */
      vector<call_order_object> get_margin_positions( const account_id_type& id )const;

      /**
       * @brief Request notification when the active orders in the market between two assets changes
       * @param callback Callback method which is called when the market changes
       * @param a First asset ID
       * @param b Second asset ID
       *
       * Callback will be passed a variant containing a vector<pair<operation, operation_result>>. The vector will
       * contain, in order, the operations which changed the market, and their results.
       */
      void subscribe_to_market(std::function<void(const variant&)> callback,
                   asset_id_type a, asset_id_type b);

      /**
       * @brief Unsubscribe from updates to a given market
       * @param a First asset ID
       * @param b Second asset ID
       */
      void unsubscribe_from_market( asset_id_type a, asset_id_type b );


      /**
       * @brief Request notification when the active orders in the market between two assets changes
       * @param callback Callback method which is called when the market changes
       * @param a First asset ID
       * @param b Second asset ID
       *
       * Callback will be passed a variant containing a vector<pair<operation, operation_result>>. The vector will
       * contain, in order, the operations which changed the market, and their results.
       */
      void subscribe_to_bitlender(std::function<void(const variant&)> callback,
                   asset_id_type a, asset_id_type b);

      /**
       * @brief Unsubscribe from updates to a given market
       * @param a First asset ID
       * @param b Second asset ID
       */
      void unsubscribe_from_bitlender( asset_id_type a, asset_id_type b );


      /**
       * @brief Returns the ticker for the market assetA:assetB
       * @param a String name of the first asset
       * @param b String name of the second asset
       * @return The market ticker for the past 24 hours.
       */
      market_ticker get_ticker( const string& base, const string& quote )const;
      market_ticker get_finance_ticker( const string& base, const string& quote )const;

      /**
       * @brief Returns the 24 hour volume for the market assetA:assetB
       * @param a String name of the first asset
       * @param b String name of the second asset
       * @return The market volume over the past 24 hours
       */
      market_volume get_24_volume( const string& base, const string& quote )const;

      /**
       * @brief Returns the order book for the market base:quote
       * @param base String name of the first asset
       * @param quote String name of the second asset
       * @param depth of the order book. Up to depth of each asks and bids, capped at 50. Prioritizes most moderate of each
       * @return Order book of the market
       */
      order_book get_order_book( const string& base, const string& quote, unsigned limit = 50 )const;

      /**
       * @brief Returns vector of 24 hour volume markets sorted by reverse base_volume
       * Note: this API is experimental and subject to change in next releases
       * @param limit Max number of results
       * @return Desc Sorted volume vector
       */
      vector<market_volume> get_top_markets(uint32_t limit)const;

      /**
       * @brief Returns recent trades for the market assetA:assetB, ordered by time, most recent first. The range is [stop, start)
       * Note: Currently, timezone offsets are not supported. The time must be UTC.
       * @param a String name of the first asset
       * @param b String name of the second asset
       * @param stop Stop time as a UNIX timestamp, the earliest trade to retrieve
       * @param limit Number of trasactions to retrieve, capped at 100
       * @param start Start time as a UNIX timestamp, the latest trade to retrieve
       * @return Recent transactions in the market
       */
      vector<market_trade> get_trade_history( const string& base, const string& quote, fc::time_point_sec start, fc::time_point_sec stop, unsigned limit = 100 )const;

      /**
       * @brief Returns trades for the market assetA:assetB, ordered by time, most recent first. The range is [stop, start)
       * Note: Currently, timezone offsets are not supported. The time must be UTC.
       * @param a String name of the first asset
       * @param b String name of the second asset
       * @param stop Stop time as a UNIX timestamp, the earliest trade to retrieve
       * @param limit Number of trasactions to retrieve, capped at 100
       * @param start Start sequence as an Integer, the latest trade to retrieve
       * @return Transactions in the market
       */
      vector<market_trade> get_trade_history_by_sequence( const string& base, const string& quote, int64_t start, fc::time_point_sec stop, unsigned limit = 100 )const;



      ///////////////
      // Witnesses //
      ///////////////

      /**
       * @brief Get a list of witnesses by ID
       * @param witness_ids IDs of the witnesses to retrieve
       * @return The witnesses corresponding to the provided IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<witness_object>> get_witnesses(const vector<witness_id_type>& witness_ids)const;

      /**
       * @brief Get the witness owned by a given account
       * @param account The ID of the account whose witness should be retrieved
       * @return The witness object, or null if the account does not have a witness
       */
      fc::optional<witness_object> get_witness_by_account(account_id_type account)const;

      /**
       * @brief Get names and IDs for registered witnesses
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @return Map of witness names to corresponding IDs
       */
      map<string, witness_id_type> lookup_witness_accounts(const string& lower_bound_name, uint32_t limit)const;

      /**
       * @brief Get the total number of witnesses registered with the blockchain
       */
      uint64_t get_witness_count()const;

      ///////////////
      // gateways //
      ///////////////

      /**
       * @brief Get a list of gatewayes by ID
       * @param gateway_ids IDs of the gatewayes to retrieve
       * @return The gatewayes corresponding to the provided IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<gateway_object>> get_gatewayes(const vector<gateway_id_type>& gateway_ids)const;
      optional<gateway_cfg> get_gateway_cfg(const gateway_id_type gateway_id,const asset_id_type asset_id)const;
      fc::variant_object get_account_attachinfo(const account_id_type acc, uint32_t utype) const;
      /**
       * @brief Get the gateway owned by a given account
       * @param account The ID of the account whose gateway should be retrieved
       * @return The gateway object, or null if the account does not have a gateway
       */
      fc::optional<gateway_object> get_gateway_by_account(account_id_type account)const;

      /**
       * @brief Get names and IDs for registered gatewayes
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @return Map of gateway names to corresponding IDs
       */
      map<string, gateway_id_type> lookup_gateway_accounts(const string& lower_bound_name, uint32_t limit)const;

      /**
       * @brief Get the total number of gatewayes registered with the blockchain
       */
      uint64_t get_gateway_count()const;

      ///////////////
      // carriers //
      ///////////////

      /**
       * @brief Get a list of carrieres by ID
       * @param carrier_ids IDs of the carrieres to retrieve
       * @return The carrieres corresponding to the provided IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<carrier_object>> get_carrieres(const vector<carrier_id_type>& carrier_ids)const;

      /**
       * @brief Get the carrier owned by a given account
       * @param account The ID of the account whose carrier should be retrieved
       * @return The carrier object, or null if the account does not have a carrier
       */
      fc::optional<carrier_object> get_carrier_by_account(account_id_type account)const;

      /**
       * @brief Get names and IDs for registered carrieres
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @return Map of carrier names to corresponding IDs
       */
      map<string, carrier_id_type> lookup_carrier_accounts(const string& lower_bound_name, uint32_t limit)const;

      /**
       * @brief Get the total number of carrieres registered with the blockchain
       */
      uint64_t get_carrier_count()const;
      asset   get_allowed_withdraw(const object_id_type id, const fc::time_point_sec &now) const;


       ///////////////
      // authors //
      ///////////////

      /**
       * @brief Get a list of authores by ID
       * @param author_ids IDs of the authores to retrieve
       * @return The authores corresponding to the provided IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<author_object>> get_authors(const vector<author_id_type>& author_ids)const;

      /**
       * @brief Get the author owned by a given account
       * @param account The ID of the account whose author should be retrieved
       * @return The author object, or null if the account does not have a author
       */
      fc::optional<author_object> get_author_by_account(account_id_type account)const;
      optional<carrier_cfg> get_carrier_cfg(const carrier_id_type carrier_id, const asset_id_type asset_id) const;


      fc::optional<author_object> get_author_by_trust(object_id_type object_id,asset_id_type asset_id)const;

      /**
       * @brief Get names and IDs for registered authores
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @return Map of author names to corresponding IDs
       */
      map<string, author_id_type> lookup_author_accounts(const string& lower_bound_name, uint32_t limit)const;

      /**
       * @brief Get the total number of authores registered with the blockchain
       */
      uint64_t get_author_count()const;



      ///////////////////////
      // Committee members //
      ///////////////////////

      /**
       * @brief Get a list of committee_members by ID
       * @param committee_member_ids IDs of the committee_members to retrieve
       * @return The committee_members corresponding to the provided IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<committee_member_object>> get_committee_members(const vector<committee_member_id_type>& committee_member_ids)const;
   
      /**
       * @brief Get the committee_member owned by a given account
       * @param account The ID of the account whose committee_member should be retrieved
       * @return The committee_member object, or null if the account does not have a committee_member
       */
      fc::optional<committee_member_object> get_committee_member_by_account(account_id_type account)const;
  
      /**
       * @brief Get names and IDs for registered committee_members
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @return Map of committee_member names to corresponding IDs
       */
      map<string, committee_member_id_type> lookup_committee_member_accounts(const string& lower_bound_name, uint32_t limit)const;
     

      /**
       * @brief Get the total number of committee registered with the blockchain
      */
      uint64_t get_committee_count()const;


      ///////////////////////
      // Budget members //
      ///////////////////////

      /**
       * @brief Get a list of budget_members by ID
       * @param budget_member_ids IDs of the budget_members to retrieve
       * @return The budget_members corresponding to the provided IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<budget_member_object>> get_budget_members(const vector<budget_member_id_type>& budget_member_ids)const;
   
      /**
       * @brief Get the budget_member owned by a given account
       * @param account The ID of the account whose budget_member should be retrieved
       * @return The budget_member object, or null if the account does not have a budget_member
       */
      fc::optional<budget_member_object> get_budget_member_by_account(account_id_type account)const;
  
      /**
       * @brief Get names and IDs for registered budget_members
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @return Map of budget_member names to corresponding IDs
       */
      map<string, budget_member_id_type> lookup_budget_member_accounts(const string& lower_bound_name, uint32_t limit)const;
     

      /**
       * @brief Get the total number of budget registered with the blockchain
      */
      uint64_t get_budget_count()const;


      ///////////////////////
      // Worker proposals  //
      ///////////////////////

      /**
       * @brief Get all workers
       * @return All the workers
       *
      */
      vector<worker_object> get_all_workers()const;

      /**
       * @brief Get the workers owned by a given account
       * @param account The ID of the account whose worker should be retrieved
       * @return The worker object, or null if the account does not have a worker
       */
      vector<optional<worker_object>> get_workers_by_account(account_id_type account)const;

      /**
       * @brief Get the workers owned by a given account
       * @param account The ID of the account whose worker should be retrieved
       * @return The worker object, or null if the account does not have a worker
       */
      vector<optional<worker_object>> get_workers_by_name(string name)const;


      /**
       * @brief Get the total number of workers registered with the blockchain
      */
      uint64_t get_worker_count()const;


      vector<locktoken_id_type> get_locktokens(account_id_type account, asset_id_type asset_id,uint32_t type)const;
      fc::variant_object get_locktoken_sum(account_id_type account, asset_id_type asset_id, uint32_t utype)const;
      fc::optional<locktoken_option_object> get_locktoken_option(asset_id_type asset_id, uint32_t lock_type , uint32_t checkasset)const;
      vector<locktoken_option_object> get_locktoken_options(uint32_t lock_type, uint32_t checkasset) const;
      ///////////
      // Votes //
      ///////////

      /**
       *  @brief Given a set of votes, return the objects they are voting for.
       *
       *  This will be a mixture of committee_member_object, witness_objects, and worker_objects
       *
       *  The results will be in the same order as the votes.  Null will be returned for
       *  any vote ids that are not found.
       */
      vector<variant> lookup_vote_ids( const vector<vote_id_type>& votes )const;

      ////////////////////////////
      // Authority / validation //
      ////////////////////////////

      /// @brief Get a hexdump of the serialized binary form of a transaction
      std::string get_transaction_hex(const signed_transaction& trx)const;

      /**
       *  This API will take a partially signed transaction and a set of public keys that the owner has the ability to sign for
       *  and return the minimal subset of public keys that should add signatures to the transaction.
       */
      set<public_key_type> get_required_signatures( const signed_transaction& trx, const flat_set<public_key_type>& available_keys )const;

      /**
       *  This method will return the set of all public keys that could possibly sign for a given transaction.  This call can
       *  be used by wallets to filter their set of public keys to just the relevant subset prior to calling @ref get_required_signatures
       *  to get the minimum subset.
       */
      set<public_key_type> get_potential_signatures( const signed_transaction& trx )const;
      set<address> get_potential_address_signatures( const signed_transaction& trx )const;

      /**
       * @return true of the @ref trx has all of the required signatures, otherwise throws an exception
       */
      bool           verify_authority( const signed_transaction& trx )const;

      bool           is_notify_info(const account_id_type account) const;
      int32_t        is_authenticator(const account_id_type account,const uint32_t type,const asset_id_type asset_id,const account_id_type author) const;
      vector<asset_id_type> get_auth_asset(const account_id_type account,const uint32_t type) const;

      bool           is_cheap_name(const std::string &name) const;
      bool           is_object(object_id_type id) const;
      /**
       * @return true if the signers have enough authority to authorize an account
       */
      bool           verify_account_authority( const string& name_or_id, const flat_set<public_key_type>& signers )const;

      /**
       *  Validates a transaction against the current state without broadcasting it on the network.
       */
      processed_transaction    validate_transaction( const signed_transaction& trx )const;
      void                     validate_opertation(const operation &op,bool is_proposed_trx)const;
      void                     validate_proposal(const signed_transaction &trx) const;
      vector<account_id_type>  vertify_transaction(const signed_transaction &trx);
      vector<account_id_type>  vertify_account_login(const signed_transaction &trx);
      /**
       *  For each operation calculate the required fee in the specified asset type.  If the asset type does
       *  not have a valid core_exchange_rate
       */
      vector< fc::variant > get_required_fees( const vector<operation>& ops, asset_id_type id )const;

      ///////////////////////////
      // Proposed transactions //
      ///////////////////////////

      /**
       *  @return the set of proposed transactions relevant to the specified account id.
       */
      vector<proposal_object> get_proposed_transactions( account_id_type id)const;

      flat_map<public_key_type,proposal_object> get_proposed_key_transactions( account_id_type id )const;
       /**
       *  @return the set of proposed transactions relevant to the specified account id.
       */
      vector<proposal_object> get_my_proposed_transactions( account_id_type id)const;

   
      //////////////////////
      // Blinded balances //
      //////////////////////

      /**
       *  @return the set of blinded balance objects by commitment ID
       */
      vector<blinded_balance_object> get_blinded_balances( const flat_set<commitment_type>& commitments )const;

      /////////////////
      // Withdrawals //
      /////////////////

      /**
       *  @brief Get non expired withdraw permission objects for a giver(ex:recurring customer)
       *  @param account Account to get objects from
       *  @param start Withdraw permission objects(1.12.X) before this ID will be skipped in results. Pagination purposes.
       *  @param limit Maximum number of objects to retrieve
       *  @return Withdraw permission objects for the account
       */
      vector<withdraw_permission_object> get_withdraw_permissions_by_giver(account_id_type account, withdraw_permission_id_type start, uint32_t limit)const;

      /**
       *  @brief Get non expired withdraw permission objects for a recipient(ex:service provider)
       *  @param account Account to get objects from
       *  @param start Withdraw permission objects(1.12.X) before this ID will be skipped in results. Pagination purposes.
       *  @param limit Maximum number of objects to retrieve
       *  @return Withdraw permission objects for the account
       */
      vector<withdraw_permission_object> get_withdraw_permissions_by_recipient(account_id_type account, withdraw_permission_id_type start, uint32_t limit)const;

       /**
       *  @brief Get non expired withdraw permission objects for a recipient(ex:service provider)
       *  @param asset_id Asset to get objects from       
       *  @return bitlender_option_object
       */
      fc::optional<bitlender_option_object> get_bitlender_option(asset_id_type asset_id)const;
      fc::optional<bitlender_option_object_key> get_bitlender_option_key(asset_id_type asset_id,optional<bitlender_key> key)const;
       /**
       *  @brief Get non expired withdraw permission objects for a recipient(ex:service provider)
       *  @param asset_id Asset to get objects from       
       *  @return bitlender_option_object
       */
      fc::optional<issue_fundraise_object> get_issue_fundraise(asset_id_type issue_id,asset_id_type buy_id)const;
 
      fc::optional<asset_exchange_feed_object>   get_asset_exchange_feed(asset_id_type asset_base,asset_id_type asset_quote,uint32_t utype)const;

      std::string dec_message(string prikey_wifi, public_key_type pubkey, uint64_t nonce,vector<char> message);
      memo_data enc_message(string prikey_wifi, public_key_type pubkey, uint64_t nonce, std::string message);

   private:
      std::shared_ptr< database_api_impl > my;
      application& _app;
};

} }

FC_REFLECT( graphene::app::order, (price)(quote)(base) );
FC_REFLECT( graphene::app::order_book, (base)(quote)(bids)(asks) );
FC_REFLECT( graphene::app::market_ticker,
            (time)(base)(quote)(latest)(lowest_ask)(highest_bid)(percent_change)(base_volume)(quote_volume) );
FC_REFLECT( graphene::app::market_volume, (time)(base)(quote)(base_volume)(quote_volume) );
FC_REFLECT( graphene::app::market_trade, (sequence)(date)(price)(amount)(value)(side1_account_id)(side2_account_id) );

FC_API(graphene::app::database_api,
   // Objects
   (get_objects)

   // Subscriptions
   (set_subscribe_callback)
   (set_pending_transaction_callback)
   (set_block_applied_callback)
   (cancel_all_subscriptions)

   // Blocks and transactions
   (get_block_header)
   (get_block_header_batch)
   (get_block)
   (get_block_ids)
   (get_block_with_id)
   (get_transaction)
   (get_transaction_by_id)
   (get_recent_transaction_by_id)

   // Globals
   (get_chain_properties)
   (get_global_properties) 
   (get_config)
   (get_info)
   (get_bitlender_paramers)
   (get_bitlender_paramers_key)
   (get_finance_paramers)
   (is_plugin_enable)
   (get_chain_id)
   (get_dynamic_global_properties)
   (get_block_summary)

   // Keys
   (get_key_references)
   (is_public_key_registered)

   // Accounts
   (get_accounts)
   (get_accounts_statistics)
   (get_full_accounts)
   (get_account_by_name)
   (get_account_author)
   (get_account_history)
   (get_account_config)
   (lookup_account_names)
   (lookup_accounts)
   (lookup_accounts_by_type)
   (lookup_accounts_by_property)
   (get_account_count)
   (get_object_count)

   // Balances
   (get_account_balances)
   (get_sort_balances)
   (get_named_account_balances)
   (get_account_lock_balances)
   (get_account_balances_summary)
   (get_balance_objects) 
   (get_vested_balances)
   (get_vesting_balances)
   (get_vesting_balances_data)
   (get_allowed_withdraw)

   // Assets
   (get_assets)
   (list_assets)
   (lookup_asset_symbols)


   // Markets / feeds
   (get_order_book)
   (get_limit_orders)
   (get_account_limit_orders)
   (get_call_orders)
   (get_settle_orders)
   (get_margin_positions)
   (get_collateral_bids)
   (get_loan_orders)
   (get_invest_orders)   
   (get_loan_counts)   
   (get_invest_counts)
   (get_notify_orders)
   (get_account_notify_orders)
   (subscribe_to_market)
   (unsubscribe_from_market)
   (subscribe_to_bitlender)
   (unsubscribe_from_bitlender)
   (get_ticker)
   (get_finance_ticker)
   (get_24_volume)
   (get_top_markets)
   (get_trade_history)
   (get_trade_history_by_sequence)

   // Witnesses
   (get_witnesses)
   (get_witness_by_account)
   (lookup_witness_accounts)
   (get_witness_count)

// gatewayes
   (get_gatewayes)
   (get_gateway_cfg)
   (get_account_attachinfo)
   (get_gateway_by_account)
   (lookup_gateway_accounts)
   (get_gateway_count)

   // carrieres
   (get_carrieres)
   (get_carrier_by_account)
   (lookup_carrier_accounts)
   (get_carrier_count)

 // authores
   (get_authors)
   (get_author_by_account)
   (get_carrier_cfg)
   (lookup_author_accounts)
   (get_author_count)
   (get_author_by_trust)



   // Committee members
   (get_committee_members)
   (get_committee_member_by_account)
   (lookup_committee_member_accounts)
   (get_committee_count)      

  // Budget members
   (get_budget_members)
   (get_budget_member_by_account)
   (lookup_budget_member_accounts)
   (get_budget_count)     

   // workers
   (get_all_workers)
   (get_workers_by_account)
   (get_workers_by_name)
   (get_worker_count)

   //locktoken
   (get_locktokens)
   (get_locktoken_sum)      
   (get_locktoken_option)
   (get_locktoken_options)

   // Votes
   (lookup_vote_ids)

   // Authority / validation
   (get_transaction_hex)
   (get_required_signatures)
   (get_potential_signatures)
   (get_potential_address_signatures)
   (verify_authority)
   (verify_account_authority)
   (validate_transaction)
   (validate_opertation)
   (validate_proposal)
   (vertify_transaction)
   (vertify_account_login)
   (is_authenticator)
   (is_notify_info)
   (get_auth_asset)
   (is_cheap_name)
   (is_object)
  
   (get_required_fees)

   // Proposed transactions
   (get_proposed_transactions)
   (get_proposed_key_transactions)
   (get_my_proposed_transactions)
  

   // Blinded balances
   (get_blinded_balances)

   // Withdrawals
   (get_withdraw_permissions_by_giver)
   (get_withdraw_permissions_by_recipient)

   // bitrender
   (get_bitlender_option)
   (get_bitlender_option_key)
   (get_issue_fundraise)
   (get_asset_exchange_feed)
   (get_budget_pre_block)
   (dec_message)
   (enc_message)    
)
