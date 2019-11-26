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

#include <graphene/app/api.hpp>
#include <graphene/utilities/key_conversion.hpp>

using namespace graphene::app;
using namespace graphene::chain;
using namespace graphene::utilities;
using namespace std;

namespace fc
{
   void to_variant( const account_multi_index_type& accts, variant& vo, uint32_t max_depth );
   void from_variant( const variant &var, account_multi_index_type &vo, uint32_t max_depth );
}

namespace graphene { namespace wallet {

typedef uint16_t transaction_handle_type;

/**
 * This class takes a variant and turns it into an object
 * of the given type, with the new operator.
 */

object* create_object( const variant& v );

struct plain_keys
{
   map<public_key_type, string>  keys;
   fc::sha512                    checksum;
};

struct brain_key_info
{
   string brain_priv_key;
   string wif_priv_key;
   public_key_type pub_key;
   address add_ress;
};


  


/**
 *  Contains the confirmation receipt the sender must give the receiver and
 *  the meta data about the receipt that helps the sender identify which receipt is
 *  for the receiver and which is for the change address.
 */
struct blind_confirmation 
{
   struct output
   {
      string                          label;
      public_key_type                 pub_key;
      stealth_confirmation::memo_data decrypted_memo;
      stealth_confirmation            confirmation;
      authority                       auth;
      string                          confirmation_receipt;
   };

   signed_transaction     trx;
   vector<output>         outputs;
};
struct account_key
{
  string                wif_key;
  public_key_type       pub_key;
  address               add_ress;
 
};
struct account_key_info
{
  account_id_type id;
  string          name;
  vector<account_key> active;
  vector<account_key> owner;
  account_key         memo;
  account_key         auth;
  account_key         witness;
};

struct blind_balance
{
   asset                     amount;
   public_key_type           from; ///< the account this balance came from
   public_key_type           to; ///< the account this balance is logically associated with
   public_key_type           one_time_key; ///< used to derive the authority key and blinding factor
   fc::sha256                blinding_factor;
   fc::ecc::commitment_type  commitment;
   bool                      used = false;
};

struct blind_receipt
{
   std::pair<public_key_type,fc::time_point>        from_date()const { return std::make_pair(from_key,date); }
   std::pair<public_key_type,fc::time_point>        to_date()const   { return std::make_pair(to_key,date);   }
   std::tuple<public_key_type,asset_id_type,bool>   to_asset_used()const   { return std::make_tuple(to_key,amount.asset_id,used);   }
   const commitment_type& commitment()const        { return data.commitment; }

   fc::time_point                  date;
   public_key_type                 from_key;
   string                          from_label;
   public_key_type                 to_key;
   string                          to_label;
   asset                           amount;
   string                          memo;
   authority                       control_authority;
   stealth_confirmation::memo_data data;
   bool                            used = false;
   stealth_confirmation            conf;
};

struct by_from;
struct by_to;
struct by_to_asset_used;
struct by_commitment;

typedef multi_index_container< blind_receipt,
   indexed_by<
      ordered_unique< tag<by_commitment>, const_mem_fun< blind_receipt, const commitment_type&, &blind_receipt::commitment > >,
      ordered_unique< tag<by_to>, const_mem_fun< blind_receipt, std::pair<public_key_type,fc::time_point>, &blind_receipt::to_date > >,
      ordered_non_unique< tag<by_to_asset_used>, const_mem_fun< blind_receipt, std::tuple<public_key_type,asset_id_type,bool>, &blind_receipt::to_asset_used > >,
      ordered_unique< tag<by_from>, const_mem_fun< blind_receipt, std::pair<public_key_type,fc::time_point>, &blind_receipt::from_date > >
   >
> blind_receipt_index_type;


struct key_label
{
   string          label;
   public_key_type key;
};


struct by_label;
struct by_key;
typedef multi_index_container<
   key_label,
   indexed_by<
      ordered_unique< tag<by_label>, member< key_label, string, &key_label::label > >,
      ordered_unique< tag<by_key>, member< key_label, public_key_type, &key_label::key > >
   >
> key_label_index_type;


struct wallet_data
{
   /** Chain ID this wallet is used with */
   chain_id_type chain_id;
   account_multi_index_type my_accounts;
   /// @return IDs of all accounts in @ref my_accounts
   vector<object_id_type> my_account_ids()const
   {
      vector<object_id_type> ids;
      ids.reserve(my_accounts.size());
      std::transform(my_accounts.begin(), my_accounts.end(), std::back_inserter(ids),
                     [](const account_object& ao) { return ao.id; });
      return ids;
   }
   /// Add acct to @ref my_accounts, or update it if it is already in @ref my_accounts
   /// @return true if the account was newly inserted; false if it was only updated
   bool update_account(const account_object& acct)
   {
      auto& idx = my_accounts.get<by_id>();
      auto itr = idx.find(acct.get_id());
      if( itr != idx.end() )
      {
         idx.replace(itr, acct);
         return false;
      } else {
         idx.insert(acct);
         return true;
      }
   }

   /** encrypted keys */
   vector<char>              cipher_keys;

   /** map an account to a set of extra keys that have been imported for that account */
   map<account_id_type, set<public_key_type> >  extra_keys;

   // map of account_name -> base58_private_key for
   //    incomplete account regs
   map<string, vector<string> > pending_account_registrations;
   map<string, string> pending_witness_registrations;

   key_label_index_type                                              labeled_keys;
   blind_receipt_index_type                                          blind_receipts;

   string                    ws_server = "ws://localhost:8090";
   string                    ws_user;
   string                    ws_password;
};

struct exported_account_keys
{
    string account_name;
    vector<vector<char>> encrypted_private_keys;
    vector<public_key_type> public_keys;
};

struct exported_keys
{
    fc::sha512 password_checksum;
    vector<exported_account_keys> account_keys;
};

struct approval_delta
{
   vector<string> active_approvals_to_add;
   vector<string> active_approvals_to_remove;
   vector<string> owner_approvals_to_add;
   vector<string> owner_approvals_to_remove;
   vector<string> key_approvals_to_add;
   vector<string> key_approvals_to_remove;
};

struct worker_vote_delta
{
   flat_set<worker_id_type> vote_for;
   flat_set<worker_id_type> vote_against;
   flat_set<worker_id_type> vote_abstain;
};

struct account_info
{
  string zone;
  string mobile;
  string mail;   
};
 

struct signed_block_with_info : public signed_block
{
   signed_block_with_info( const signed_block& block );
   signed_block_with_info( const signed_block_with_info& block ) = default;

   block_id_type block_id;
   public_key_type signing_key;
   vector< transaction_id_type > transaction_ids;
};

struct vesting_balance_object_with_info : public vesting_balance_object
{
   vesting_balance_object_with_info( const vesting_balance_object& vbo, fc::time_point_sec now );
   vesting_balance_object_with_info( const vesting_balance_object_with_info& vbo ) = default;

   /**
    * How much is allowed to be withdrawn.
    */
   asset allowed_withdraw;
   /**
    * The time at which allowed_withdrawal was calculated.
    */
   fc::time_point_sec allowed_withdraw_time;
};

namespace detail {
class wallet_api_impl;
}

/***
 * A utility class for performing various state-less actions that are related to wallets
 */
class utility {
   public:
      /**
       * Derive any number of *possible* owner keys from a given brain key.
       *
       * NOTE: These keys may or may not match with the owner keys of any account.
       * This function is merely intended to assist with account or key recovery.
       *
       * @see suggest_brain_key()
       *
       * @param brain_key    Brain key
       * @param number_of_desired_keys  Number of desired keys
       * @return A list of keys that are deterministically derived from the brainkey
       */
      static vector<brain_key_info> derive_owner_keys_from_brain_key(string brain_key, int number_of_desired_keys = 1);
};

struct operation_detail {
   string                   memo;
   string                   description;
   operation_history_object op;
};

struct operation_detail_ex {
    string                   memo;
    string                   description;
    operation_history_object op;
    transaction_id_type      transaction_id;
};

struct account_history_operation_detail {
   uint32_t                     total_count = 0;
   uint32_t                     result_count = 0;
   vector<operation_detail_ex>  details;
};

/**
 * This wallet assumes it is connected to the database server with a high-bandwidth, low-latency connection and
 * performs minimal caching. This API could be provided locally to be used by a web interface.
 */
class wallet_api
{
   public:
      wallet_api( const wallet_data& initial_data, fc::api<login_api> rapi );
      wallet_api(const wallet_data &initial_data);
      virtual ~wallet_api();

      bool copy_wallet_file( string destination_filename );

      fc::ecc::private_key derive_private_key(const std::string& prefix_string, int sequence_number) const;

      variant                           info();
      variant                           node_info();
      /** Returns info such as client version, git version of graphene/fc, version of boost, openssl.
       * @returns compile time info and client and dependencies versions
       */
      variant_object                      about() const;
      optional<signed_block_with_info>    get_block( uint64_t num );
      optional<signed_block_with_info>    get_block_ids( uint64_t num );
      processed_transaction               get_transaction( uint64_t block_num, uint32_t trx_in_block )const;
      /** Returns the number of accounts registered on the blockchain
       * @returns the number of registered accounts
       */
      uint64_t                          get_account_count()const;
      uint64_t                          get_object_count(object_id_type id, bool bmaxid = false) const;

      /** Lists all accounts controlled by this wallet.
       * This returns a list of the full account objects for all accounts whose private keys 
       * we possess.
       * @returns a list of account objects
       */
      vector<account_object>            list_my_accounts();
      /** Lists all accounts registered in the blockchain.
       * This returns a list of all account names and their account ids, sorted by account name.
       *
       * Use the \c lowerbound and limit parameters to page through the list.  To retrieve all accounts,
       * start by setting \c lowerbound to the empty string \c "", and then each iteration, pass
       * the last account name returned as the \c lowerbound for the next \c list_accounts() call.
       *
       * @param lowerbound the name of the first account to return.  If the named account does not exist, 
       *                   the list will start at the account that comes after \c lowerbound
       * @param limit the maximum number of accounts to return (max: 1000)
       * @returns a list of accounts mapping account names to account ids
       */
      map<string,account_id_type>       list_accounts(const string& lowerbound, uint32_t limit);
      /** List the balances of an account.
       * Each account can have multiple balances, one for each type of asset owned by that 
       * account.  The returned list will only contain assets for which the account has a
       * nonzero balance
       * @param id the name or id of the account whose balances you want
       * @returns a list of the given account's balances
       */
      vector<asset_summary>             list_account_balances(const string& id);

      asset_summary                     get_account_balance(const string& id,const string &asset_id);

      vector<asset_summary>             get_account_balances_summary(const string& id);
     

      void                              list_all_balances(const string &symbol);

      void                              sort_balances(const string& symbol, uint32_t limit);

      /** Lists all assets registered on the blockchain.
       * 
       * To list all assets, pass the empty string \c "" for the lowerbound to start
       * at the beginning of the list, and iterate as necessary.
       *
       * @param lowerbound  the symbol of the first asset to include in the list.
       * @param limit the maximum number of assets to return (max: 100)
       * @returns the list of asset objects, ordered by symbol
       */
      vector<asset_object>              list_assets(const string& lowerbound, uint32_t limit)const;
      
      /** Returns the most recent operations on the named account.
       *
       * This returns a list of operation history objects, which describe activity on the account.
       *
       * @param name the name or id of the account
       * @param limit the number of entries to return (starting from the most recent)
       * @returns a list of \c operation_history_objects
       */
      vector<operation_detail>  get_account_history(string name, int limit)const;

      /** Returns the relative operations on the named account from start number.
       *
       * @param name the name or id of the account
       * @param stop Sequence number of earliest operation.
       * @param limit the number of entries to return
       * @param start  the sequence number where to start looping back throw the history
       * @returns a list of \c operation_history_objects
       */
     vector<operation_detail>  get_relative_account_history(string name, uint32_t stop, int limit, uint32_t start)const;
     vector<balance_history>   get_balance_history(string account, vector<string> asset_name, uint32_t type,uint64_t start , uint64_t nlimit);
     vector<balance_history>   get_balance_history_object(string account, vector<string> asset_name,uint32_t type,uint64_t ustart, uint64_t nlimit);
      vector<bucket_object>                   get_market_history(string symbol, string symbol2, uint32_t bucket, fc::time_point_sec start, fc::time_point_sec end)const;
      vector<bitlender_history_object>        get_bitlender_loan_history(string base, string quote, uint32_t type)const;      
      vector<limit_order_object>              get_limit_orders(string a, string b, uint32_t limit)const;
      vector<call_order_object>               get_call_orders(string a, uint32_t limit)const;
      vector<force_settlement_object>         get_settle_orders(string a, uint32_t limit)const;

      vector<bitlender_order_info>      get_notify_orders(uint32_t status,uint32_t start, uint32_t limit)const;
      vector<bitlender_order_info>      get_loan_orders(string a,uint32_t status, uint32_t start, uint32_t limit)const;
      vector<bitlender_invest_info>     get_invest_orders(string a,uint32_t status, uint32_t start, uint32_t limit)const;

      market_ticker                     get_ticker( const string& base, const string& quote, bool ismarket = false )const;

      /** Returns the collateral_bid object for the given MPA
       *
       * @param account the name or id of the asset
       * @param order the number of entries to return       
       * @returns a list of  bitlender_invest_object
       */
      vector<operation_history_object> get_account_bitlender_history( string account,bitlender_order_id_type order ) const;
       /** Returns the collateral_bid object for the given MPA
       *
       * @param account the name or id of the asset
       * @param start time  , notify it mush use as "2017-05-01T01:00:00"  must use quotation marks
       * @param end time ,notify it mush use as  "2018-05-01T01:00:00" must use quotation marks
       * @returns a list of  account_loan
       */
      vector<bitlender_order_info>   get_account_loan_history( string account, fc::time_point_sec start, fc::time_point_sec end )const;
      /** Returns the collateral_bid object for the given MPA
       *
       * @param account the name or id of the asset
       * @param start the number of entries to return
       * @param end the sequence number where to start looping back throw the history
       * @returns a list of  bitlender_invest_object
       */
      vector<bitlender_invest_info>   get_account_invest_history( string account, fc::time_point_sec start, fc::time_point_sec end )const;

      vector<issue_fundraise_object>   get_account_issue_fundraise_history( string account, fc::time_point_sec start, fc::time_point_sec end )const;
      vector<buy_fundraise_object>     get_account_buy_fundraise_history( string account, fc::time_point_sec start, fc::time_point_sec end )const;
      vector<sell_exchange_object>     get_account_sell_exchange_history( string account, fc::time_point_sec start, fc::time_point_sec end )const;
      vector<buy_exchange_object>      get_account_buy_exchange_history( string account, fc::time_point_sec start, fc::time_point_sec end )const;

      fc::variant                      get_object_history(object_id_type id) const;

      /** Returns the collateral_bid object for the given MPA
       *
       * @param asset the name or id of the asset
       * @param limit the number of entries to return
       * @param start the sequence number where to start looping back throw the history
       * @returns a list of \c collateral_bid_objects
       */
      vector<collateral_bid_object> get_collateral_bids(string asset, uint32_t limit = 100, uint32_t start = 0) const;
      
      /** Returns the block chain's slowly-changing settings.
       * This object contains all of the properties of the blockchain that are fixed
       * or that change only once per maintenance interval (daily) such as the
       * current list of witnesses, committee_members, block interval, etc.
       * @see \c get_dynamic_global_properties() for frequently changing properties
       * @returns the global properties
       */
      global_property_object            get_global_properties() const;

      /**
       * Get operations relevant to the specified account filtering by operation type, with transaction id
       *
       * @param name the name or id of the account, whose history shoulde be queried
       * @param operation_types The IDs of the operation we want to get operations in the account( 0 = transfer , 1 = limit order create, ...)
       * @param start the sequence number where to start looping back throw the history
       * @param limit the max number of entries to return (from start number)
       * @returns account_history_operation_detail
       */
      account_history_operation_detail get_account_history_by_operations(string name, vector<uint16_t> operation_types, uint32_t start, int limit);

      /** Returns the block chain's rapidly-changing properties.
       * The returned object contains information that changes every block interval
       * such as the head block number, the next witness, etc.
       * @see \c get_global_properties() for less-frequently changing properties
       * @returns the dynamic global properties
       */
      dynamic_global_property_object    get_dynamic_global_properties() const;

      bitlender_paramers_object         get_bitlender_paramers() const;
      bitlender_paramers_object_key     get_bitlender_paramers_key(optional<bitlender_key> key) const;

      finance_paramers_object         get_finance_paramers() const;

      /** Returns information about the given account.
       *
       * @param account_name_or_id the name or id of the account to provide information about
       * @returns the public account data stored in the blockchain
       */
      account_object                    get_account(string account_name_or_id) const;

      /** Returns information about the given asset.
       * @param asset_name_or_id the symbol or id of the asset in question
       * @returns the information about the asset stored in the block chain
       */
      asset_object                      get_asset(string asset_name_or_id) const;

      /** Returns the BitAsset-specific data for a given asset.
       * Market-issued assets's behavior are determined both by their "BitAsset Data" and
       * their basic asset data, as returned by \c get_asset().
       * @param asset_name_or_id the symbol or id of the BitAsset in question
       * @returns the BitAsset-specific data for this asset
       */
      asset_bitasset_data_object        get_bitasset_data(string asset_name_or_id)const;

      fc::optional<asset_exchange_feed_object>  get_asset_exchange_feed(string asset_base,string asset_quote,uint32_t utype)const;

      asset_dynamic_data_object get_dynamic_data_object(string asset_name_or_id) const;

      /** Lookup the id of a named account.
       * @param account_name_or_id the name of the account to look up
       * @returns the id of the named account
       */
      account_id_type                   get_account_id(string account_name_or_id) const;

      /**
       * Lookup the id of a named asset.
       * @param asset_name_or_id the symbol of an asset to look up
       * @returns the id of the given asset
       */
      asset_id_type                     get_asset_id(string asset_name_or_id) const;

      /**
       * Returns the blockchain object corresponding to the given id.
       *
       * This generic function can be used to retrieve any object from the blockchain
       * that is assigned an ID.  Certain types of objects have specialized convenience 
       * functions to return their objects -- e.g., assets have \c get_asset(), accounts 
       * have \c get_account(), but this function will work for any object.
       *
       * @param id the id of the object to return
       * @returns the requested object
       */
      variant                           get_object(object_id_type id) const;

       /**
       * Returns the proposed transaction object corresponding to the given id.
       *
       * @param id the id of the object to return       
       * @returns the requested object
       */
      
      vector<proposal_object>           get_proposed_transaction( account_id_type id )const;
       /**
       * Returns the proposed transaction object created by the given id.
       *
       * @param id the id of the object to return       
       * @returns the requested object
       */
      
      vector<proposal_object>           get_my_proposed_transaction( account_id_type id )const;
      /** Returns the current wallet filename.  
       *
       * This is the filename that will be used when automatically saving the wallet.
       *
       * @see set_wallet_filename()
       * @return the wallet filename
       */
      string                            get_wallet_filename() const;

      /**
       * Get the WIF private key corresponding to a public key.  The
       * private key must already be in the wallet.
       */
      string                            get_private_key( public_key_type pubkey )const;

      /**
       * @ingroup Transaction Builder API
       */
      transaction_handle_type begin_builder_transaction();
      /**
       * @ingroup Transaction Builder API
       */
      void add_operation_to_builder_transaction(transaction_handle_type transaction_handle, const operation& op);
      /**
       * @ingroup Transaction Builder API
       */
      void replace_operation_in_builder_transaction(transaction_handle_type handle,
                                                    unsigned operation_index,
                                                    const operation& new_op);
      /**
       * @ingroup Transaction Builder API
       */
      asset set_fees_on_builder_transaction(transaction_handle_type handle, string fee_asset = GRAPHENE_SYMBOL);
      /**
       * @ingroup Transaction Builder API
       */
      transaction preview_builder_transaction(transaction_handle_type handle);
      /**
       * @ingroup Transaction Builder API
       */
      signed_transaction sign_builder_transaction(transaction_handle_type transaction_handle, bool broadcast = true);

      /** Broadcast signed transaction
       * @param tx signed transaction
       * @returns the transaction ID along with the signed transaction.
       */
      pair<transaction_id_type,signed_transaction> broadcast_transaction(signed_transaction tx);
      pair<transaction_id_type,signed_transaction> broadcast_build_transaction(transaction_handle_type transaction_handle);

      /**
       * @ingroup Transaction Builder API
       */
     

      signed_transaction propose_builder_transaction(
         transaction_handle_type handle,
         string account_name_or_id,
         time_point_sec expiration = time_point::now() + fc::minutes(1),
         uint32_t review_period_seconds = 0,
         bool broadcast = true
        );

      /**
       * @ingroup Transaction Builder API
       */
      void remove_builder_transaction(transaction_handle_type handle);

      /** Checks whether the wallet has just been created and has not yet had a password set.
       *
       * Calling \c set_password will transition the wallet to the locked state.
       * @return true if the wallet is new
       * @ingroup Wallet Management
       */
      bool    is_new()const;
      
      /** Checks whether the wallet is locked (is unable to use its private keys).  
       *
       * This state can be changed by calling \c lock() or \c unlock().
       * @return true if the wallet is locked
       * @ingroup Wallet Management
       */
      bool    is_locked()const;
      
      /** Locks the wallet immediately.
       * @ingroup Wallet Management
       */
      void    lock();
      
      /** Unlocks the wallet.  
       *
       * The wallet remain unlocked until the \c lock is called
       * or the program exits.
       * @param password the password previously set with \c set_password()
       * @ingroup Wallet Management
       */
      void    unlock(string password);
      
      /** Sets a new password on the wallet.
       *
       * The wallet must be either 'new' or 'unlocked' to
       * execute this command.
       * @ingroup Wallet Management
       */
      void    set_password(string password);

      /** Dumps all private keys owned by the wallet.
       *
       * The keys are printed in WIF format.  You can import these keys into another wallet
       * using \c import_key()
       * @returns a map containing the private keys, indexed by their public key 
       */
      map<public_key_type, string> dump_private_keys();
      account_key_info             dump_account_keys(string account);
      
      /** Returns a list of all commands supported by the wallet API.
       *
       * This lists each command, along with its arguments and return types.
       * For more detailed help on a single command, use \c get_help()
       *
       * @returns a multi-line string suitable for displaying on a terminal
       */
      string  help()const;

      /** Returns detailed help on a single API command.
       * @param method the name of the API command you want help with
       * @returns a multi-line string suitable for displaying on a terminal
       */
      string  gethelp(const string& method)const;

      /** Loads a specified Graphene wallet.
       *
       * The current wallet is closed before the new wallet is loaded.
       *
       * @warning This does not change the filename that will be used for future
       * wallet writes, so this may cause you to overwrite your original
       * wallet unless you also call \c set_wallet_filename()
       *
       * @param wallet_filename the filename of the wallet JSON file to load.
       *                        If \c wallet_filename is empty, it reloads the
       *                        existing wallet file
       * @returns true if the specified wallet is loaded
       */
      bool    load_wallet_file(string wallet_filename = "");

      /** Saves the current wallet to the given filename.
       * 
       * @warning This does not change the wallet filename that will be used for future
       * writes, so think of this function as 'Save a Copy As...' instead of
       * 'Save As...'.  Use \c set_wallet_filename() to make the filename
       * persist.
       * @param wallet_filename the filename of the new wallet JSON file to create
       *                        or overwrite.  If \c wallet_filename is empty,
       *                        save to the current filename.
       */
      void    save_wallet_file(string wallet_filename = "");

      /** Sets the wallet filename used for future writes.
       *
       * This does not trigger a save, it only changes the default filename
       * that will be used the next time a save is triggered.
       * 
       * @param wallet_filename the new filename to use for future saves
       * @param bautosave the new filename to use for future saves
       */
      void    set_wallet_filename(string wallet_filename,bool bautosave = true);
      

      /** Suggests a safe brain key to use for creating your account.
       * \c create_account_with_brain_key() requires you to specify a 'brain key',
       * a long passphrase that provides enough entropy to generate cyrptographic
       * keys.  This function will suggest a suitably random string that should
       * be easy to write down (and, with effort, memorize).
       * @returns a suggested brain_key
       */
      brain_key_info suggest_brain_key()const;

      vector<account_key> suggest_account_key(string name, string password) const;
      /**
      * Derive any number of *possible* owner keys from a given brain key.
      *
      * NOTE: These keys may or may not match with the owner keys of any account.
      * This function is merely intended to assist with account or key recovery.
      *
      * @see suggest_brain_key()
      *
      * @param brain_key    Brain key
      * @param number_of_desired_keys  Number of desired keys
      * @return A list of keys that are deterministically derived from the brainkey
      */
      vector<brain_key_info> derive_owner_keys_from_brain_key(string brain_key, int number_of_desired_keys = 1) const;

      /**
      * Determine whether a textual representation of a public key
      * (in Base-58 format) is *currently* linked
      * to any *registered* (i.e. non-stealth) account on the blockchain
      * @param public_key Public key
      * @return Whether a public key is known
      */
      bool is_public_key_registered(string public_key) const;

      /** Converts a signed_transaction in JSON form to its binary representation.
       *
       * @param tx the transaction to serialize
       * @returns the binary form of the transaction.  It will not be hex encoded, 
       *          this returns a raw string that may have null characters embedded 
       *          in it
       */
      string serialize_transaction(signed_transaction tx) const;

      /** Imports the private key for an existing account.
       *
       * The private key must match either an owner key or an active key for the
       * named account.  
       *
       * @see dump_private_keys()
       *
       * @param account_name_or_id the account owning the key
       * @param wif_key the private key in WIF format
       * @returns true if the key was imported
       */
      bool import_key(string account_name_or_id, string wif_key);
      void remove_key(public_key_type pub_key);
      bool import_password(string account_name_or_id, string password);

      signed_transaction change_password(string account_name_or_id, string password, bool broadcast );

      map<string, bool> import_accounts( string filename, string password );

      bool import_account_keys( string filename, string password, string src_account_name, string dest_account_name );

      signed_transaction set_propose_register(string account_name_or_id, bool bset, bool broadcast);
      signed_transaction set_account_config(string account_name_or_id, string index, string config, bool broadcast);
      /**
       * This call will construct transaction(s) that will claim all balances controled
       * by wif_keys and deposit them into the given account.
       */
      vector< signed_transaction > import_balance( string account_name_or_id, const vector<string>& wif_keys, bool broadcast );
   
      /** Transforms a brain key to reduce the chance of errors when re-entering the key from memory.
       *
       * This takes a user-supplied brain key and normalizes it into the form used
       * for generating private keys.  In particular, this upper-cases all ASCII characters
       * and collapses multiple spaces into one.
       * @param s the brain key as supplied by the user
       * @returns the brain key in its normalized form
       */
      string normalize_brain_key(string s) const;

      /** Registers a third party's account on the blockckain.
       *
       * This function is used to register an account for which you do not own the private keys.
       * When acting as a registrar, an end user will generate their own private keys and send
       * you the public keys.  The registrar will use this function to register the account
       * on behalf of the end user.
       *
       * @see create_account_with_brain_key()
       *
       * @param name the name of the account, must be unique on the blockchain.  Shorter names
       *             are more expensive to register; the rules are still in flux, but in general
       *             names of more than 8 characters with at least one digit will be cheap.
       * @param owner the owner key for the new account
       * @param active the active key for the new account
       * @param memo the memo key for the new account
       * @param author the author key for the new account
       * @param registrar_account the account which will pay the fee to register the user
       * @param referrer_account the account who is acting as a referrer, and may receive a
       *                         portion of the user's transaction fees.  This can be the
       *                         same as the registrar_account if there is no referrer.
       * @param referrer_percent the percentage (0 - 100) of the new user's transaction fees
       *                         not claimed by the blockchain that will be distributed to the
       *                         referrer; the rest will be sent to the registrar.  Will be
       *                         multiplied by GRAPHENE_1_PERCENT when constructing the transaction.
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering the account
       */
      signed_transaction register_account(string name,
                                          public_key_type owner,
                                          public_key_type active,
                                          public_key_type memo,
                                          public_key_type author,
                                          string  registrar_account,
                                          string  referrer_account,
                                          uint32_t referrer_percent,
                                          bool broadcast = false);

      signed_transaction propose_register_account(string issuer, string name,
                                          public_key_type owner,
                                          public_key_type active,
                                          public_key_type memo,
                                          public_key_type author,
                                          string  registrar_account,
                                          string  referrer_account,
                                          uint32_t referrer_percent,
                                          bool broadcast = false);

      signed_transaction create_account(string name,   string password,                                          
                                        string  registrar_account,
                                        string  referrer_account,
                                        uint32_t referrer_percent,
                                        bool broadcast = false);

      /** Returns the number of accounts registered on the blockchain
       * @param account the name of the first account to return.  If the named account does not exist,        
       * @returns the number of registered accounts
       */
      vector<string>          get_account_auth(string account) const;

      /** Returns the number of accounts set_gateway_need_auth
       * @param issuer the name of the first account to return.  If the named account does not exist, 
       * @param need_auth the name of the first account to return.  If the named account does not exist, 
       * @param trust_auth the name of the first account to return.  If the named account does not exist, 
       * @param def_author the name of the first account to return.  If the named account does not exist, 
       * @param broadcast the name of the first account to return.  If the named account does not exist,  
       * @returns the number of registered accounts
       */
      signed_transaction        set_gateway_need_auth(string issuer,optional<uint32_t> need_auth,optional<vector<vector<string>>> trust_auth,optional<string> def_author,bool broadcast  = false);
      signed_transaction        set_carrier_need_auth(string issuer,optional<uint32_t> need_auth,optional<vector<vector<string>>> trust_auth,optional<string> def_author,bool broadcast  = false);
      signed_transaction        set_carrier_can_invest(string issuer,bool bcan,bool broadcast  = false);

      signed_transaction        set_account_auth(string account,string to,string info,string key,bool broadcast  = false);       
      signed_transaction        set_account_pubkey(string issuer,public_key_type ownerkey,public_key_type actkey,public_key_type memokey,public_key_type authkey,bool broadcast  = false);
      signed_transaction        set_auth_state(string issuer,string account,uint32_t state,bool broadcast  = false);
      signed_transaction        set_auth_payfor(string issuer, string foracc, string forrefer, bool broadcast = false);
      int32_t is_authenticator(const string account, const uint32_t type, const string symbol,const string author) const;

      signed_transaction        set_account_info(string account, variant_object info, bool broadcast);
      optional<account_info>    get_account_info(string account); 
      vector<string>            get_account_config(string account);
      void                      get_admin_config(string admin_name, string loan_symbol, string invest_symbol);
      /**
       *  Upgrades an account to prime status.
       *  This makes the account holder a 'lifetime member'.
       *
       *  @todo there is no option for annual membership
       *  @param name the name or id of the account to upgrade
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction upgrading the account
       */
      signed_transaction upgrade_account(string name, bool broadcast);
          /**
       *  Upgrades an account to prime status.
       *  This makes the account holder a 'lifetime member'.
       *
       *  @todo there is no option for annual membership
       *  @param name the name or id of the account to upgrade
       *  @param lock true to broadcast the transaction on the network 
       *  @param broadcast true to broadcast the transaction on the network
       *  @returns the signed transaction upgrading the account
       */
      signed_transaction set_account_lock_balance(string name,bool lock, bool broadcast);

      /** Creates a new account and registers it on the blockchain.
       *
       * @todo why no referrer_percent here?
       *
       * @see suggest_brain_key()
       * @see register_account()
       *
       * @param brain_key the brain key used for generating the account's private keys
       * @param account_name  the name of the account, must be unique on the blockchain.  Shorter names
       *                     are more expensive to register; the rules are still in flux, but in general
       *                     names of more than 8 characters with at least one digit will be cheap.
       * @param registrar_account the account which will pay the fee to register the user
       * @param referrer_account the account who is acting as a referrer, and may receive a
       *                         portion of the user's transaction fees.  This can be the
       *                         same as the registrar_account if there is no referrer.
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering the account
       */
      signed_transaction create_account_with_brain_key(string brain_key,
                                                       string account_name,
                                                       string registrar_account,
                                                       string referrer_account,
                                                       bool broadcast = false);

      /**account_coupon it on the blockchain.       
      
       * @param account_name the name of the account, must be unique on the blockchain.  Shorter names
       *                     are more expensive to register; the rules are still in flux, but in general
       *                     names of more than 8 characters with at least one digit will be cheap.       
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering the account
       */
      signed_transaction account_coupon(string account_name, bool broadcast = false);
       /**account_coupon it on the blockchain.       
      
       * @param account_name the name of the account, must be unique on the blockchain.  Shorter names
       *                     are more expensive to register; the rules are still in flux, but in general
       *                     names of more than 8 characters with at least one digit will be cheap.   
       * @param core_fee CORE asset amount
       * @param dest_symbol the name of asset which you want to exchange    
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering the account
       */

      signed_transaction withdraw_exchange_fee(string account_name,string core_fee,string dest_symbol, bool broadcast=true);

      /** Transfer an amount from one account to another.
       * @param from the name or id of the account sending the funds
       * @param to the name or id of the account receiving the funds
       * @param amount the amount to send (in nominal units -- to send half of a ZOS, specify 0.5)
       * @param asset_symbol the symbol or id of the asset to send
       * @param memo a memo to attach to the transaction.  The memo will be encrypted in the 
       *             transaction and readable for the receiver.  There is no length limit
       *             other than the limit imposed by maximum transaction size, but transaction
       *             increase with transaction size
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction transferring funds
       */
      signed_transaction transfer(string from,
                                  string to,
                                  string amount,
                                  string asset_symbol,
                                  string memo,
                                  bool broadcast = false);

      signed_transaction transfer_noencmsg(string from,
                                  string to,
                                  string amount,
                                  string asset_symbol,
                                  string memo,
                                  bool broadcast = false);

       signed_transaction propose_transfer(string from,
                                  string to,
                                  string amount,
                                  string asset_symbol,
                                  string memo,
                                  bool broadcast = false);

       /** Transfer an amount from one account to another.
       * @param from the name or id of the account sending the funds
       * @param to the name or id of the account receiving the funds
       * @param amount the amount to send (in nominal units -- to send half of a ZOS, specify 0.5)
       * @param asset_symbol the symbol or id of the asset to send
       * @param memo a memo to attach to the transaction.  The memo will be encrypted in the 
       *             transaction and readable for the receiver.  There is no length limit
       *             other than the limit imposed by maximum transaction size, but transaction
       *             increase with transaction size
       * @param vesting_cliff_seconds the amount to send (in nominal units -- to send half of a ZOS, specify 0.5)
       * @param vesting_duration_seconds the symbol or id of the asset to send
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction transferring funds
       */
      signed_transaction transfer_vesting(string from,
                                  string to,
                                  string amount,
                                  string asset_symbol,
                                  string memo,
                                  uint32_t vesting_cliff_seconds,
                                  uint32_t vesting_duration_seconds,
                                  bool broadcast = false);


      /**
       *  This method works just like transfer, except it always broadcasts and
       *  returns the transaction ID along with the signed transaction.
       */
      pair<transaction_id_type,signed_transaction> transfer2(string from,
                                                             string to,
                                                             string amount,
                                                             string asset_symbol,
                                                             string memo ) {
         auto trx = transfer( from, to, amount, asset_symbol, memo, true );
         return std::make_pair(trx.id(),trx);
      }


      /**
       *  This method is used to convert a JSON transaction to its transactin ID.
       */
      transaction_id_type get_transaction_id( const signed_transaction& trx )const { return trx.id(); }


      /** Sign a memo message.
       *
       * @param from the name or id of signing account; or a public key.
       * @param to the name or id of receiving account; or a public key.
       * @param memo text to sign.
       */
      memo_data sign_memo(string from, string to, string memo);

      /** Read a memo.
       *
       * @param memo JSON-enconded memo.
       * @returns string with decrypted message..
       */
      string read_memo(const memo_data& memo);


      /** These methods are used for stealth transfers */
      ///@{
      /**
       *  This method can be used to set the label for a public key
       *
       *  @note No two keys can have the same label.
       *
       *  @return true if the label was set, otherwise false
       */
      bool                        set_key_label( public_key_type, string label );
      string                      get_key_label( public_key_type )const;

      /**
       *  Generates a new blind account for the given brain key and assigns it the given label.
       */
      public_key_type             create_blind_account( string label, string brain_key  );

      /**
       * @return the total balance of all blinded commitments that can be claimed by the
       * given account key or label
       */
      vector<asset>                get_blind_balances( string key_or_label );

     

       
      /** @return all blind accounts for which this wallet has the private key */
      map<string,public_key_type> get_blind_accounts()const;
      /** @return the public key associated with the given label */
      public_key_type             get_public_key( string label )const;
      ///@}

      /**
       * @return all blind receipts to/form a particular account
       */
      vector<blind_receipt> blind_history( string key_or_account );

      /**
       *  Given a confirmation receipt, this method will parse it for a blinded balance and confirm
       *  that it exists in the blockchain.  If it exists then it will report the amount received and
       *  who sent it.
       *
       *  @param confirmation_receipt - if not empty and the sender is a unknown public key, then the unknown public key will be given the label opt_from
       *  @param opt_from - if not empty and the sender is a unknown public key, then the unknown public key will be given the label opt_from
       *  @param opt_memo - if not empty and the sender is a unknown public key, then the unknown public key will be given the label opt_from
       *  @param confirmation_receipt - a base58 encoded stealth confirmation 
       */
      blind_receipt receive_blind_transfer( string confirmation_receipt, string opt_from, string opt_memo );

     
      /**
       *  Transfers a public balance from  to one or more blinded balances using a
       *  stealth transfer.
       * 
       *  @param from_account_id_or_name -  1
       *  @param asset_symbol - 2
       *  @param to_amounts - 3
       *  @param broadcast -  4
       */
      blind_confirmation transfer_to_blind( string from_account_id_or_name, 
                                            string asset_symbol,                                           
                                            vector<pair<string, string>> to_amounts, 
                                            bool broadcast = false );

      /**
       * Transfers funds from a set of blinded balances to a public account balance.
       */
      blind_confirmation transfer_from_blind( 
                                            string from_blind_account_key_or_label,
                                            string to_account_id_or_name, 
                                            string amount,
                                            string asset_symbol,
                                            bool broadcast = false );

      /**
       *  Used to transfer from one set of blinded balances to another
       */
      blind_confirmation blind_transfer( string from_key_or_label,
                                         string to_key_or_label,
                                         string amount,
                                         string symbol,
                                         bool broadcast = false );

      /** Place a limit order attempting to sell one asset for another.
       *
       * Buying and selling are the same operation on Graphene; if you want to buy ZOS 
       * with USD, you should sell USD for ZOS.
       *
       * The blockchain will attempt to sell the \c symbol_to_sell for as
       * much \c symbol_to_receive as possible, as long as the price is at 
       * least \c min_to_receive / \c amount_to_sell.   
       *
       * In addition to the transaction fees, market fees will apply as specified 
       * by the issuer of both the selling asset and the receiving asset as
       * a percentage of the amount exchanged.
       *
       * If either the selling asset or the receiving asset is whitelist
       * restricted, the order will only be created if the seller is on
       * the whitelist of the restricted asset type.
       *
       * Market orders are matched in the order they are included
       * in the block chain.
       *
       * @todo Allow order expiration to be set here.  Document default/max expiration time
       *
       * @param seller_account the account providing the asset being sold, and which will 
       *                       receive the proceeds of the sale.
       * @param amount_to_sell the amount of the asset being sold to sell (in nominal units)
       * @param symbol_to_sell the name or id of the asset to sell
       * @param min_to_receive the minimum amount you are willing to receive in return for
       *                       selling the entire amount_to_sell
       * @param symbol_to_receive the name or id of the asset you wish to receive
       * @param expiration if the order does not fill immediately, this is the length of 
       *                    time the order will remain on the order books before it is 
       *                    cancelled and the un-spent funds are returned to the seller's 
       *                    account
       * @param mode if true, the order will only be included in the blockchain
       *                     if it is filled immediately; if false, an open order will be
       *                     left on the books to fill any amount that cannot be filled
       *                     immediately.
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction selling the funds
       */
      signed_transaction sell_asset(string seller_account,
                                    string amount_to_sell,
                                    string   symbol_to_sell,
                                    string min_to_receive,
                                    string   symbol_to_receive,
                                    uint32_t expiration = 0,
                                    uint16_t  mode = 0,
                                    bool     broadcast = false);

      signed_transaction buy_asset(string seller_account,
                                    string amount_to_sell,
                                    string   symbol_to_sell,
                                    string min_to_receive,
                                    string   symbol_to_receive,
                                    uint32_t expiration = 0,
                                    uint16_t  mode = 0,
                                    bool     broadcast = false);
      /** Borrow an asset or update the debt/collateral ratio for the loan.
       *
       * This is the first step in shorting an asset.  Call \c sell_asset() to complete the short.
       *
       * @param borrower_name the name or id of the account associated with the transaction.
       * @param amount_to_borrow the amount of the asset being borrowed.  Make this value
       *                         negative to pay back debt.
       * @param asset_symbol the symbol or id of the asset being borrowed.
       * @param amount_of_collateral the amount of the backing asset to add to your collateral
       *        position.  Make this negative to claim back some of your collateral.
       *        The backing asset is defined in the \c bitasset_options for the asset being borrowed.
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction borrowing the asset
       */
      signed_transaction borrow_asset(string borrower_name, string amount_to_borrow, string asset_symbol,
                                      string amount_of_collateral, bool broadcast = false);

      /** Cancel an existing order
       *
       * @param order_id the id of order to be cancelled
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction canceling the order
       */
      signed_transaction cancel_order(object_id_type order_id, bool broadcast = false);

      /** Creates a new user-issued or market-issued asset.
       *
       * Many options can be changed later using \c update_asset()
       *
       * Right now this function is difficult to use because you must provide raw JSON data
       * structures for the options objects, and those include prices and asset ids.
       *
       * @param issuer the name or id of the account who will pay the fee and become the 
       *               issuer of the new asset.  This can be updated later
       *  @param asset_issuer
       * @param symbol the ticker symbol of the new asset
       * @param precision the number of digits of precision to the right of the decimal point,
       *                  must be less than or equal to 12
       * @param common asset options required for all new assets.
       *               Note that core_exchange_rate technically needs to store the asset ID of 
       *               this new asset. Since this ID is not known at the time this operation is 
       *               created, create this price as though the new asset has instance ID 1, and
       *               the chain will overwrite it with the new asset's ID.
       * @param bitasset_opts options specific to BitAssets.  This may be null unless the
       *               \c market_issued flag is set in common.flags 
       *       
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new asset
       */
      signed_transaction create_asset(string issuer,
                                      string asset_issuer,
                                      string symbol,
                                      uint8_t precision,
                                      asset_options common,
                                      fc::optional<bitasset_options> bitasset_opts,                                                                        
                                      bool broadcast = false);
                        

      signed_transaction asset_reserve_fees(string issuer,
                                            string amount,
                                            string symbol,
                                            bool broadcast);

      signed_transaction asset_claim_fees(string issuer,
                                          string amount,
                                          string symbol,
                                          bool broadcast);

      /** Creates a new user-issued or market-issued asset.
       *
       * Many options can be changed later using \c create_bitlender_option()
       *
      
       * @param issuer the name or id of the account who will pay the fee and become the 
       *               issuer of the new asset.  This can be updated later
       * @param symbol the asset of the option     
       * @param key    the asset of the option     
       * @param option_values the asset of the option      
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */
      signed_transaction create_bitlender_option(string issuer,
                                      string  symbol,              
                                      const   optional<variant_object> key,                               
                                      const   variant_object& option_values,                                                                                              
                                      bool    broadcast = false);
       /** Creates a new user-issued or market-issued asset.
       *
       * Many options can be changed later using \c create_bitlender_option()
       *
      
       * @param issuer the name or id of the account who will pay the fee and become the 
       *               issuer of the new asset.  This can be updated later
       * @param symbol       
       * @param authors the asset of the option     
       * @param weight_threshold the asset of the option      
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

      signed_transaction update_bitlender_option_author(string    issuer,    
                                                        string    symbol,   
                                                       
                                                        const  vector<string> authors, 
                                                        const     uint32_t     weight_threshold,                                                                                                                                                         
                                                        bool      broadcast);


       /** Creates a new user-issued or market-issued asset.      
       * @param issuer the name or id of the account who will pay the fee and become the 
       *               issuer of the new asset.  This can be updated later  
       * @param carrier the name or id of the account who will pay the fee and become the        
       * @param key the name or id of the account who will pay the fee and become the        
       * @param option_values the asset of the option      
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

      signed_transaction bitlender_lend_order(string issuer,   string carrier,  
                                      const optional<variant_object> key,                                           
                                      const   variant_object& option_values,                                                                                              
                                      bool    broadcast = false);

       /** Creates a new user-issued or market-issued asset.             
       * @param option_values the asset of the option             
       * @returns the signed transaction creating a new cash option
       */

      fc::variant bitlender_loan_info(const variant_object &option_values);

      /** Creates a new user-issued or market-issued asset.      
       * @param issuer the name or id of the account who will pay the fee and become the 
       *               issuer of the new asset.  This can be updated later       
       * @param carrier the name or id of the account who will pay the fee and become the 
       * @param option_values the asset of the option      
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

      signed_transaction bitlender_invest_order(string issuer,       string carrier,                                           
                                      const   variant_object& option_values,                                                                                              
                                      bool    broadcast = false);


       /** Creates a new user-issued or market-issued asset.      
       * @param issuer the name or id of the account who will pay the fee and become the        
       * @param id the name or id of the account who will pay the fee and become the 
       * @param uperiod the asset of the option      
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

      signed_transaction bitlender_repay_interest(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                              
                                      const   uint32_t uperiod,
                                      bool    broadcast = false);

      /*      
       * @param issuer the name or id of the account who will pay the fee and become the        
       * @param id the name or id of the account who will pay the fee and become the        
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

      signed_transaction bitlender_overdue_interest(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                              
                                      const   uint32_t uperiod,
                                      bool    broadcast = false);

      /*      
       * @param issuer the name or id of the account who will pay the fee and become the        
       * @param id the name or id of the account who will pay the fee and become the        
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

      signed_transaction bitlender_recycle_interest(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                              
                                      const   uint32_t uperiod,
                                      bool    broadcast = false);

      /** Creates a new user-issued or market-issued asset.      
       * @param issuer the name or id of the account who will pay the fee and become the        
       * @param id the name or id of the account who will pay the fee and become the        
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */
      signed_transaction bitlender_repay_principal(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                                                                    
                                      bool    broadcast = false);
       /** Creates a new user-issued or market-issued asset.      
       * @param issuer the name or id of the account who will pay the fee and become the        
       * @param id the name or id of the account who will pay the fee and become the        
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

      signed_transaction bitlender_pre_repay_principal(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                                                                    
                                      bool    broadcast = false);
       /** Creates a new user-issued or market-issued asset.      
       * @param issuer the name or id of the account who will pay the fee and become the        
       * @param id the name or id of the account who will pay the fee and become the        
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

      signed_transaction bitlender_overdue_repay_principal(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                                                                    
                                      bool    broadcast = false);

      /** Creates a new user-issued or market-issued asset.      
       * @param issuer the name or id of the account who will pay the fee and become the        
       * @param id the name or id of the account who will pay the fee and become the        
       * @param pay the name or id of the account who will pay the fee and become the        
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

      signed_transaction bitlender_recycle_principal(string issuer,                                           
                                      const   bitlender_order_id_type id, 
                                      const   string                pay,                                                                                                                                        
                                      bool    broadcast = false);

       /** Creates a new user-issued or market-issued asset.      
       * @param issuer the name or id of the account who will pay the fee and become the        
       * @param id the name or id of the account who will pay the fee and become the                   
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

      signed_transaction bitlender_remove_order(string issuer,
                                                const bitlender_order_id_type id,
                                                bool broadcast);

      /** Creates a new user-issued or market-issued asset.      
       * @param issuer the name or id of the account who will pay the fee and become the        
       * @param id the name or id of the account who will pay the fee and become the      
       * @param bset the name or id of the account who will pay the fee and become the        
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

      signed_transaction bitlender_set_auto_repayment(string issuer,                                           
                                      const   bitlender_order_id_type id,  
                                      const   bool    bset  ,                                                                                                                                   
                                      bool    broadcast = false); 

 
      /** Creates a new user-issued or market-issued asset.      
       * @param issuer the name or id of the account who will pay the fee and become the 
       *               issuer of the new asset.  This can be updated later       
       * @param option_values the asset of the option      
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

      signed_transaction bitlender_add_collateral(string issuer,                                              
                                      const   variant_object& option_values,                                                                                              
                                      bool    broadcast = false);

   
     vector<bitlender_order_id_type> list_bitlender_order(string loan, string lender, uint32_t period, uint32_t ufiletermask, uint32_t usort);

     /** Creates a new user-issued or market-issued asset.      
       * @param issuer the name or id of the account who will pay the fee and become the 
       *               issuer of the new asset.  This can be updated later       
       * @param order_id the asset of the option   
       * @param utype the asset of the option      
       * @param uvalue the asset of the option         
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

     signed_transaction bitlender_test_principal(string issuer,
                                                 const object_id_type order_id,
                                                 uint32_t utype,
                                                 uint32_t uvalue,
                                                 bool broadcast = false);

     /** Issue new shares of an asset.
       *
       * @param to_account the name or id of the account to receive the new shares
       * @param amount the amount to issue, in nominal units
       * @param symbol the ticker symbol of the asset to issue
       * @param memo a memo to include in the transaction, readable by the recipient
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction issuing the new shares
       */
     signed_transaction issue_asset(string to_account, string amount,
                                    string symbol,
                                    string memo,
                                    bool broadcast = false);

     /** Update the core options on an asset.
       * There are a number of options which all assets in the network use. These options are 
       * enumerated in the asset_object::asset_options struct. This command is used to update 
       * these options for an existing asset.
       *
       * @note This operation cannot be used to update BitAsset-specific options. For these options,
       * \c update_bitasset() instead.
       * @param issuer 
       * @param symbol the name or id of the asset to update
       * @param new_issuer if changing the asset's issuer, the name or id of the new issuer.
       *                   null if you wish to remain the issuer of the asset
       * @param option_values the new asset_options object, which will entirely replace the existing
       *                    options.
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction updating the asset
       */
     signed_transaction update_asset(string issuer,string symbol,
                                     optional<string> new_issuer,
                                     const  variant_object& option_values,  
                                     bool broadcast = false);


    

     /** Update the cash options
       
       * @param issuer the name or id of the asset to update
       * @param symbol if changing the asset's issuer, the name or id of the new issuer.
       *                       null if you wish to remain the issuer of the asset    
       * @param key   
       * @param option_values        the new asset_options object, which will entirely replace the existing
       *                       options.       
       * @param broadcast      true to broadcast the transaction on the network
       * @returns the signed transaction updating the asset
       */
     signed_transaction update_bitlender_option(string issuer,
                                                string symbol,  
                                                const  optional<variant_object> key,                                                                                        
                                                const variant_object &option_values,
                                                bool broadcast = false);

     signed_transaction stop_bitlender_option(string issuer,
                                              string symbol,
                                              const bool stop,
                                              bool broadcast = false);

     signed_transaction fee_mode_bitlender_option(string issuer,
                                                  string symbol,
                                                  const uint32_t mode,
                                                  bool broadcast = false);


     /** Update the cash options
       
       * @param issuer the name or id of the asset to update      
       * @param key the name or id of the asset to update      
       * @param option_values        the new asset_options object, which will entirely replace the existing
       *                       options.              
       * @param broadcast      true to broadcast the transaction on the network
       * @returns the signed transaction updating the asset
       */
     signed_transaction update_bitlender_paramers(string issuer,                   
                                                const optional<variant_object> key,                                
                                                const variant_object &option_values,                                                
                                                bool broadcast); 

     /** Update the cash options
       
       * @param issuer the name or id of the asset to update      
       * @param option_values        the new asset_options object, which will entirely replace the existing
       *                       options.       
       * @param broadcast      true to broadcast the transaction on the network
       * @returns the signed transaction updating the asset
       */
      signed_transaction update_finance_paramers(string issuer,                                                
                                                const variant_object &option_values,
                                                bool broadcast); 

      /** Creates a new user-issued or market-issued asset.
       *
       * Many options can be changed later using \c create_bitlender_option()
       *
      
       * @param issuer the name or id of the account who will pay the fee and become the 
       *               issuer of the new asset.  This can be updated later
       * @param issue_asset the asset of the option      
       * @param buy_asset the asset of the option 
       * @param fundraise_owner the asset of the option 
       * @param type
       * @param option_values the asset of the option             
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */
      signed_transaction create_issue_fundraise(string issuer,                                              
                                               string  issue_asset,  
                                               string  buy_asset, 
                                               string  fundraise_owner,    
                                               string  type,      
                                               const   variant_object& option_values,                                                                                           
                                               bool    broadcast = false);

                                                      /** Update the cash options
       
       * @param issuer the name or id of the asset to update
       * @param issue_asset if changing the asset's issuer, the name or id of the new issuer.
       * @param buy_asset if changing the asset's issuer, the name or id of the new issuer. 
       * @param period if changing the asset's issuer, the name or id of the new issuer. 
       * @param exchange_settings        the new asset_options object, which will entirely replace the existing
       * @param op_type if changing the asset's issuer, the name or id of the new issuer.       
       * @param broadcast      true to broadcast the transaction on the network
       * @returns the signed transaction updating the asset
       */
     signed_transaction update_issue_fundraise(string    issuer,
                                                    string    issue_asset,  
                                                    string    buy_asset, 
                                                    uint32_t  period,
                                                    const     optional<variant_object> exchange_settings,
                                                    uint32_t  op_type,                                                  
                                                    bool broadcast = false);

      /** Update the cash options
         * @param issuer the name or id of the asset to update
       * @param issue_asset if changing the asset's issuer, the name or id of the new issuer.
       * @param buy_asset if changing the asset's issuer, the name or id of the new issuer.           
       * @param broadcast      true to broadcast the transaction on the network
       * @returns the signed transaction updating the asset
       */
     signed_transaction remove_issue_fundraise(string    issuer,
                                                    string    issue_asset,  
                                                    string    buy_asset,                               
                                                    bool broadcast = false);

      

        signed_transaction issue_fundraise_publish_feed(string issuer,
                                         string   issue,
                                         string   buy, 
                                         uint32_t period, 
                                         price    feed,
                                         bool broadcast = false);

      /*
       * Many options can be changed later using \c create_buy_fundraise()
       *
      
       * @param issuer the name or id of the account who will pay the fee and become the 
       *               issuer of the new asset.  This can be updated later
       * @param issue the asset of the option
       * @param buy the asset of the option
       * @param amount the asset of the option     
       * @param bimmediately the asset of the option      
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */
      signed_transaction create_buy_fundraise(string  issuer,
                                        string   issue,
                                        string   buy,  
                                        string   amount,                                            
                                        const   uint32_t bimmediately,                                                                                              
                                        bool    broadcast = false);

      /*
       * Many options can be changed later using \c enable_buy_fundraise()
       *
      
       * @param issuer the name or id of the account who will pay the fee and become the 
       *               issuer of the new asset.  This can be updated later
       * @param finance_id the asset of the option         
       * @param benable the asset of the option      
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */
      signed_transaction enable_buy_fundraise(string  issuer,
                                        buy_fundraise_id_type finance_id,                                        
                                        const   bool benable,                                                                                              
                                        bool    broadcast = false);   

       /*
       * Many options can be changed later using \c create_sell_exchange()
       *
      
       * @param issuer the name or id of the account who will pay the fee and become the 
       *               issuer of the new asset.  This can be updated later
       * @param sell_asset the asset of the option         
       * @param sell_amount the asset of the option         
       * @param buy_asset the asset of the option        
       * @param buy_amount the asset of the option  
       * @param type the asset of the option   
       * @param exchange_settings the asset of the option      
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

     signed_transaction create_sell_exchange(
                                  string      issuer,
                                  string      sell_asset,
                                  string      sell_amount,                                  
                                  string      buy_asset,   
                                  string      buy_amount, 
                                  string      type,
                                  variant     exchange_settings,
                                  bool        broadcast = false);  

       /*
       * Many options can be changed later using \c update_sell_exchange()
       *
      
       * @param issuer the name or id of the account who will pay the fee and become the 
       *               issuer of the new asset.  This can be updated later       
       * @param sell_id the asset of the option  
       * @param sell_amount the asset of the option         
       * @param buy_amount the asset of the option  
       * @param op_type the asset of the option  
       * @param exchange_settings the asset of the option      
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new cash option
       */

     signed_transaction update_sell_exchange(
         string issuer,
         sell_exchange_id_type sell_id,
         string sell_amount,
         string buy_amount,
         uint32_t op_type,
         optional<variant_object> exchange_settings,
         bool broadcast);

     signed_transaction remove_sell_exchange(
         string issuer,
         sell_exchange_id_type sell_id,
         bool broadcast = false);

     signed_transaction update_buy_exchange(
         string issuer,
         string amount,
         string asset,
         buy_exchange_id_type buy_id,
         optional<price> price,
         bool broadcast = false);

     signed_transaction create_buy_exchange(
         string issuer,
         string amount,
         string asset,
         optional<sell_exchange_id_type> sell_id,
         optional<price> price,
         bool broadcast = false);

     signed_transaction remove_buy_exchange(
         string issuer,
         buy_exchange_id_type buy_id,
         bool broadcast = false);

     /** Update the cash rate
       
       * @param issuer the name or id of the asset to update
       * @param symbol if changing the asset's issuer, the name or id of the new issuer.
       *                       null if you wish to remain the issuer of the asset      
       * @param key_in 
       * @param changed_rate_add        the new asset_options object, which will entirely replace the existing
       * @param changed_rate_remove     true to broadcast the transaction on the network
       * @param broadcast     true to broadcast the transaction on the network
       * @returns the signed transaction updating the asset
       */

     signed_transaction update_bitlender_rate(string issuer,
                                              string symbol,
                                              const  optional<variant_object> key_in,   
                                              const variant_object &changed_rate_add,
                                              vector<uint32_t> changed_rate_remove,
                                              bool broadcast = false);

                          


     /* get_bitlender_option.
       * @param symbol      true to broadcast the transaction on the network
       * @returns the signed transaction updating the asset
       */

     bitlender_option_object    get_bitlender_option(string symbol);
     bitlender_option_object_key get_bitlender_option_key(string symbol,optional<bitlender_key> key);
     /* get_bitlender_option.
       * @param symbol      true to broadcast the transaction on the network
       * @param pay      true to broadcast the transaction on the network
       * @returns the signed transaction updating the asset
       */

     issue_fundraise_object get_issue_fundraise(string issue,string buy = "ZOS");

     vector<issue_fundraise_id_type>  list_issue_fundraise(string  issue, string buy, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit);
     vector<issue_fundraise_id_type>  list_my_issue_fundraise(string account, bool bcreate,uint64_t start,uint64_t limit);

     vector<buy_fundraise_id_type>    list_buy_fundraise(string  issue, string buy, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit);
     vector<buy_fundraise_id_type>    list_my_buy_fundraise(string account, bool bowner,uint64_t start,uint64_t limit);
     vector<buy_fundraise_object>     list_buy_fundraise_by_id(issue_fundraise_id_type id,uint64_t start,uint64_t limit);


     vector<sell_exchange_id_type>  list_sell_exchange(string  issue, string buy, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit);
     vector<sell_exchange_id_type>  list_my_sell_exchange(string account, uint64_t start,uint64_t limit);

     vector<buy_exchange_id_type>    list_buy_exchange(string  issue, string buy, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit);
     vector<buy_exchange_id_type>    list_my_buy_exchange(string account, uint64_t start,uint64_t limit);
     vector<buy_exchange_object>     list_buy_exchange_by_id(sell_exchange_id_type id,uint64_t start,uint64_t limit);



     /** Update the options specific to a BitAsset.
       *
       * BitAssets have some options which are not relevant to other asset types. This operation is used to update those
       * options an an existing BitAsset.
       *
       * @see update_asset()
       * @param issuer 
       * @param symbol the name or id of the asset to update, which must be a market-issued asset
       * @param option_values the new bitasset_options object, which will entirely replace the existing   options.     
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction updating the bitasset
       */
     signed_transaction update_bitasset(string    issuer,string symbol,
                                        const  variant_object& option_values,  
                                        bool broadcast = false);


      signed_transaction update_asset_gateway(string issuer,string symbol,
                                               const  vector<string> & gateways,                                                 
                                               bool   broadcast)    ;                                    
   
       /** Update the set of feed-producing accounts for a BitAsset.
       *
       * BitAssets have price feeds selected by taking the median values of recommendations from a set of feed producers.
       * This command is used to specify which accounts may produce feeds for a given BitAsset.
       * @param issuer
       * @param symbol the name or id of the asset to update
       * @param new_feed_producers a list of account names or ids which are authorized to produce feeds for the asset.
       *                           this list will completely replace the existing list
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction updating the bitasset's feed producers
       */
     signed_transaction update_asset_feed_producers(string    issuer,string symbol,
                                                    flat_set<string> new_feed_producers,
                                                    bool broadcast = false);
 
    /** Update the set of feed-producing accounts for a BitAsset.
       *
       * BitAssets have price feeds selected by taking the median values of recommendations from a set of feed producers.
       * This command is used to specify which accounts may produce feeds for a given BitAsset.
       * @param saccount the name or id of the asset to update
       * @param symbol the name or id of the asset to update
       * @param option
       * @param new_feed_producers a list of account names or ids which are authorized to produce feeds for the asset.
       *                           this list will completely replace the existing list
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction updating the bitasset's feed producers
       */
     signed_transaction propose_bitlender_feed_producers(string saccount, string symbol,
                                                         const variant_object &option,
                                                         flat_set<string> new_feed_producers,
                                                          bool broadcast /* = false */);

     /** Publishes a price feed for the named asset.
       *
       * Price feed providers use this command to publish their price feeds for market-issued assets. A price feed is
       * used to tune the market for a particular market-issued asset. For each value in the feed, the median across all
       * committee_member feeds for that asset is calculated and the market for the asset is configured with the median of that
       * value.
       *
       * The feed object in this command contains three prices: a call price limit, a short price limit, and a settlement price.
       * The call limit price is structured as (collateral asset) / (debt asset) and the short limit price is structured
       * as (asset for sale) / (collateral asset). Note that the asset IDs are opposite to eachother, so if we're
       * publishing a feed for USD, the call limit price will be CORE/USD and the short limit price will be USD/CORE. The
       * settlement price may be flipped either direction, as long as it is a ratio between the market-issued asset and
       * its collateral.
       *
       * @param publishing_account the account publishing the price feed
       * @param symbol the name or id of the asset whose feed we're publishing
       * @param feed the price_feed object containing the three prices making up the feed
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction updating the price feed for the given asset
       */
     signed_transaction publish_asset_feed(string publishing_account,
                                           string symbol,
                                           price_feed feed,
                                           bool broadcast = false);

       /** Publishes a price feed for the named asset.                                           
      *
       * @param publishing_account the account publishing the price feed       
       * @param symbol the price_feed object containing the three prices making up the feed
       * @param feed the price_feed object containing the three prices making up the feed
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction updating the price feed for the given asset
       */

     signed_transaction publish_bitlender_feed(string publishing_account,   
                                           string symbol,
                                           price_feed feed,
                                           bool broadcast = false);                                           

     /** Pay into the fee pool for the given asset.
       *
       * User-issued assets can optionally have a pool of the core asset which is 
       * automatically used to pay transaction fees for any transaction using that
       * asset (using the asset's core exchange rate).
       *
       * This command allows anyone to deposit the core asset into this fee pool.
       *
       * @param from the name or id of the account sending the core asset
       * @param symbol the name or id of the asset whose fee pool you wish to fund
       * @param amount the amount of the core asset to deposit
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction funding the fee pool
       */
     signed_transaction fund_asset_fee_pool(string from,
                                            string symbol,
                                            string amount,
                                            bool broadcast = false);

     /** Burns the given user-issued asset.
       *
       * This command burns the user-issued asset to reduce the amount in circulation.
       * @note you cannot burn market-issued assets.
       * @param from the account containing the asset you wish to burn
       * 
       * @param amount the amount to burn, in nominal units
       * @param symbol the name or id of the asset to burn
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction burning the asset
       */
     signed_transaction reserve_asset(string from,
                                      string amount,
                                      string symbol,
                                      bool broadcast = false);

     /** Forces a global settling of the given asset (black swan or prediction markets).
       *
       * In order to use this operation, asset_to_settle must have the global_settle flag set
       *
       * When this operation is executed all balances are converted into the backing asset at the
       * settle_price and all open margin positions are called at the settle price.  If this asset is
       * used as backing for other bitassets, those bitassets will be force settled at their current
       * feed price.
       *
       * @note this operation is used only by the asset issuer, \c settle_asset() may be used by 
       *       any user owning the asset
       *
       * @param symbol the name or id of the asset to force settlement on
       * @param settle_price the price at which to settle
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction settling the named asset
       */
     signed_transaction global_settle_asset(string symbol,
                                            price settle_price,
                                            bool broadcast = false);

     /** Schedules a market-issued asset for automatic settlement.
       *
       * Holders of market-issued assests may request a forced settlement for some amount of their asset. This means that
       * the specified sum will be locked by the chain and held for the settlement period, after which time the chain will
       * choose a margin posision holder and buy the settled asset using the margin's collateral. The price of this sale
       * will be based on the feed price for the market-issued asset being settled. The exact settlement price will be the
       * feed price at the time of settlement with an offset in favor of the margin position, where the offset is a
       * blockchain parameter set in the global_property_object.
       *
       * @param account_to_settle the name or id of the account owning the asset
       * @param amount_to_settle the amount of the named asset to schedule for settlement
       * @param symbol the name or id of the asset to settlement on
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction settling the named asset
       */
     signed_transaction settle_asset(string account_to_settle,
                                     string amount_to_settle,
                                     string symbol,
                                     bool broadcast = false);

     /** Creates or updates a bid on an MPA after global settlement.
       *
       * In order to revive a market-pegged asset after global settlement (aka
       * black swan), investors can bid collateral in order to take over part of
       * the debt and the settlement fund, see BSIP-0018. Updating an existing
       * bid to cover 0 debt will delete the bid.
       *
       * @param bidder_name the name or id of the account making the bid
       * @param debt_amount the amount of debt of the named asset to bid for
       * @param debt_symbol the name or id of the MPA to bid for
       * @param additional_collateral the amount of additional collateral to bid
       *        for taking over debt_amount. The asset type of this amount is
       *        determined automatically from debt_symbol.
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating/updating the bid
       */
     signed_transaction bid_collateral(string bidder_name, string debt_amount, string debt_symbol,
                                       string additional_collateral, bool broadcast = false);

     /** Whitelist and blacklist accounts, primarily for transacting in whitelisted assets.
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
       * @param authorizing_account the account who is doing the whitelisting
       * @param account_to_list the account being whitelisted
       * @param new_listing_status the new whitelisting status
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction changing the whitelisting status
       */
     signed_transaction whitelist_account(string authorizing_account,
                                          string account_to_list,
                                          account_whitelist_operation::account_listing new_listing_status,
                                          bool broadcast = false);

     /** Creates a committee_member object owned by the given account.
       *
       * An account can have at most one committee_member object.
       *
       * @param owner_account the name or id of the account which is creating the committee_member
       * @param url a URL to include in the committee_member record in the blockchain.  Clients may
       *            display this when showing a list of committee_members.  May be blank.
       * @param memo 
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering a committee_member
       */
     signed_transaction create_committee_member(string owner_account,
                                                string url,
                                                string memo,
                                                bool broadcast = false);

       /* Update a carrier object owned by the given account.
       *
       * @param carrier_name The name of the carrier's owner account.  Also accepts the ID of the owner account or the ID of the carrier.
       * @param url Same as for create_carrier.  The empty string makes it remain the same.      
       * @param broadcast true if you wish to broadcast the transaction.
       */
      signed_transaction update_committee_member(string carrier_name,
                                       string url,            
                                       optional<string> memo,                         
                                       bool broadcast = false);



     /** Creates a budget_member object owned by the given account.
       *
       * An account can have at most one budget_member object.
       *
       * @param owner_account the name or id of the account which is creating the budget_member
       * @param url a URL to include in the budget_member record in the blockchain.  Clients may
       *            display this when showing a list of budget_members.  May be blank.
       * @param memo 
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering a budget_member
       */
     signed_transaction create_budget_member(string owner_account,
                                                string url,
                                                string memo,
                                                bool broadcast = false);

     /* Update a carrier object owned by the given account.
       *
       * @param carrier_name The name of the carrier's owner account.  Also accepts the ID of the owner account or the ID of the carrier.
       * @param url Same as for create_carrier.  The empty string makes it remain the same.      
       * @param broadcast true if you wish to broadcast the transaction.
       */
      signed_transaction update_budget_member(string budget_name,
                                       string url,                                     
                                       optional<string> memo,         
                                       bool broadcast = false);


     /** Creates a gateway_member object owned by the given account.
       *
       * An account can have at most one gateway_member object.
       *
       * @param owner_account the name or id of the account which is creating the gatway_member
       * @param url a URL to include in the gatway_member record in the blockchain.  Clients may   
       * @param memo  
       * @param a_asset a URL to include in the gatway_member record in the blockchain.  Clients may    *            
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering a gatway_member
       */
     signed_transaction create_gateway(string owner_account,
                                              string url,
                                              string memo,
                                              vector<string> a_asset,
                                              bool broadcast = false);

     gateway_object      get_gateway(string owner_account);
     /**
       * Update a gateway object owned by the given account.
       *
       * @param gateway_name The name of the gateway's owner account.  Also accepts the ID of the owner account or the ID of the gateway.
       * @param url Same as for create_gateway.  The empty string makes it remain the same.  
       * @param memo 
       * @param a_asset Same as for create_gateway.  The empty string makes it remain the same.      
       * @param broadcast true if you wish to broadcast the transaction.
       */
     signed_transaction update_gateway(string gateway_name,
                                       string url,  
                                       optional<string> memo,
                                       optional<vector<string>> a_asset,                                   
                                       bool broadcast = false);

     /** Creates a gateway_member object owned by the given account.
       *
       * An account can have at most one gateway_member object.
       *
       * @param owner_account the name or id of the account which is creating the gatway_member
       * @param url a URL to include in the gatway_member record in the blockchain.  Clients may    
       * @param memo    
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering a gatway_member
       */
     signed_transaction create_carrier(string owner_account,
                                              string url,
                                              string memo,
                                              bool broadcast = false);


     carrier_object get_carrier(string owner_account);
     /* Update a carrier object owned by the given account.
       *
       * @param carrier_name The name of the carrier's owner account.  Also accepts the ID of the owner account or the ID of the carrier.
       * @param url Same as for create_carrier.  The empty string makes it remain the same.
       * @param block_signing_key The new block signing public key.  The empty string makes it remain the same.
       * @param broadcast true if you wish to broadcast the transaction.
       */
     signed_transaction update_carrier(string carrier_name,
                                       string url,
                                       optional<string> memo,
                                       bool broadcast = false);


     signed_transaction create_author(string owner_account,
                                              string url,
                                              string memo,
                                               vector<string> asset_symbol,
                                              bool broadcast = false);


     author_object get_author(string owner_account);
     /* Update a author object owned by the given account.
       *
       * @param author_name The name of the author's owner account.  Also accepts the ID of the owner account or the ID of the author.
       * @param url Same as for create_author.  The empty string makes it remain the same.
       * @param memo The new block signing public key.  The empty string makes it remain the same.
       * @param asset_id The new block signing public key.  The empty string makes it remain the same.
       * @param broadcast true if you wish to broadcast the transaction.
       */
     signed_transaction update_author(string author_name,
                                       string url,
                                       optional<string> memo,
                                       optional<vector<string>> asset_symbol,  
                                       bool broadcast = false);


     /* An account can have at most one gateway_member object.
       *
       * @param owner_account the name or id of the account which is creating the gatway_member
       * @param gateway_account a URL to include in the gatway_member record in the blockchain.  Clients may
       * @param usupply a URL to include in the gatway_member record in the blockchain.  Clients may
       * @param symbol a URL to include in the gatway_member record in the blockchain.  Clients may
       * @param revoke a URL to include in the gatway_member record in the blockchain.  Clients may
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering a gatway_member
       */

     signed_transaction gateway_issue_currency(string owner_account, string gateway_account, string usupply, string symbol, bool send , bool broadcast );

     /* An account can have at most one gateway_member object.
       *
       * @param from the name or id of the account which is creating the gatway_member
       * @param to a URL to include in the gatway_member record in the blockchain.  Clients may
       * @param uwithdraw a URL to include in the gatway_member record in the blockchain.  Clients may
       * @param symbol a URL to include in the gatway_member record in the blockchain.  Clients may
       
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering a gatway_member
       */

     signed_transaction gateway_withdraw(string from, string to, string uwithdraw, string symbol, bool broadcast = true);

     signed_transaction gateway_create_withdraw(string from, string to, string amount, string symbol,uint32_t withdrawal_period_sec,uint32_t periods_until_expiration ,uint32_t identify, bool broadcast);
     signed_transaction gateway_update_withdraw(withdraw_permission_id_type id, string amount, string symbol,uint32_t withdrawal_period_sec,uint32_t periods_until_expiration , bool broadcast);
     signed_transaction gateway_remove_withdraw(withdraw_permission_id_type id,bool broadcast);
     signed_transaction gateway_claim_withdraw(withdraw_permission_id_type id, string amount, string symbol,  bool broadcast);
     /* An account can have at most one gateway_member object.
       *
       * @param from the name or id of the account which is creating the gatway_member
       * @param to a URL to include in the gatway_member record in the blockchain.  Clients may
       * @param deposit a URL to include in the gatway_member record in the blockchain.  Clients may
       * @param symbol a URL to include in the gatway_member record in the blockchain.  Clients may
       
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering a gatway_member
       */

     signed_transaction gateway_deposit(string from, string to, string deposit, string symbol, bool broadcast = true);

     /* An account can have at most one gateway_member object.
       *
       * @param account the name or id of the account which is creating the gatway_member 
       * @param from the name or id of the account which is creating the gatway_member
       * @param to a URL to include in the gatway_member record in the blockchain.  Clients may
       * @param deposit a URL to include in the gatway_member record in the blockchain.  Clients may
       * @param symbol a URL to include in the gatway_member record in the blockchain.  Clients may
       * @param memo a URL to include in the gatway_member record in the blockchain.  Clients may
       
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering a gatway_member
       */


     signed_transaction gateway_propose_deposit(string account,string from, string to, string deposit, string symbol,string memo, bool broadcast = true);

     /** Lists all witnesses registered in the blockchain.
       * This returns a list of all account names that own witnesses, and the associated witness id,
       * sorted by name.  This lists witnesses whether they are currently voted in or not.
       *
       * Use the \c lowerbound and limit parameters to page through the list.  To retrieve all witnesss,
       * start by setting \c lowerbound to the empty string \c "", and then each iteration, pass
       * the last witness name returned as the \c lowerbound for the next \c list_witnesss() call.
       *
       * @param lowerbound the name of the first witness to return.  If the named witness does not exist, 
       *                   the list will start at the witness that comes after \c lowerbound
       * @param limit the maximum number of witnesss to return (max: 1000)
       * @returns a list of witnesss mapping witness names to witness ids
       */
     map<string, witness_id_type> list_witnesses(const string &lowerbound, uint32_t limit);

     /** Lists all committee_members registered in the blockchain.
       * This returns a list of all account names that own committee_members, and the associated committee_member id,
       * sorted by name.  This lists committee_members whether they are currently voted in or not.
       *
       * Use the \c lowerbound and limit parameters to page through the list.  To retrieve all committee_members,
       * start by setting \c lowerbound to the empty string \c "", and then each iteration, pass
       * the last committee_member name returned as the \c lowerbound for the next \c list_committee_members() call.
       *
       * @param lowerbound the name of the first committee_member to return.  If the named committee_member does not exist, 
       *                   the list will start at the committee_member that comes after \c lowerbound
       * @param limit the maximum number of committee_members to return (max: 1000)
       * @returns a list of committee_members mapping committee_member names to committee_member ids
       */
     map<string, committee_member_id_type> list_committee_members(const string &lowerbound, uint32_t limit);

     /** Lists all budget_members registered in the blockchain.
       * This returns a list of all account names that own budget_members, and the associated budget_member id,
       * sorted by name.  This lists budget_members whether they are currently voted in or not.
       *
       * Use the \c lowerbound and limit parameters to page through the list.  To retrieve all budget_members,
       * start by setting \c lowerbound to the empty string \c "", and then each iteration, pass
       * the last budget_member name returned as the \c lowerbound for the next \c list_budget_members() call.
       *
       * @param lowerbound the name of the first budget_member to return.  If the named budget_member does not exist, 
       *                   the list will start at the budget_member that comes after \c lowerbound
       * @param limit the maximum number of budget_members to return (max: 1000)
       * @returns a list of budget_members mapping budget_member names to budget_member ids
       */
     map<string, budget_member_id_type> list_budget_members(const string &lowerbound, uint32_t limit);

     /** Lists all gatway_members registered in the blockchain.
       * This returns a list of all account names that own gatway_members, and the associated committee_member id,
       * sorted by name.  This lists gatway_members whether they are currently voted in or not.
       *
       * Use the \c lowerbound and limit parameters to page through the list.  To retrieve all gatway_members,
       * start by setting \c lowerbound to the empty string \c "", and then each iteration, pass
       * the last committee_member name returned as the \c lowerbound for the next \c list_committee_members() call.
       *
       * @param lowerbound the name of the first committee_member to return.  If the named committee_member does not exist, 
       *                   the list will start at the committee_member that comes after \c lowerbound
       * @param limit the maximum number of gatway_members to return (max: 1000)
       * @returns a list of gatway_members mapping committee_member names to committee_member ids
       */
     map<string, gateway_id_type> list_gateways(const string &lowerbound, uint32_t limit);

     /** Lists all carrier registered in the blockchain.
       * This returns a list of all account names that own gatway_members, and the associated committee_member id,
       * sorted by name.  This lists gatway_members whether they are currently voted in or not.
       *
       * Use the \c lowerbound and limit parameters to page through the list.  To retrieve all gatway_members,
       * start by setting \c lowerbound to the empty string \c "", and then each iteration, pass
       * the last committee_member name returned as the \c lowerbound for the next \c list_committee_members() call.
       *
       * @param lowerbound the name of the first committee_member to return.  If the named committee_member does not exist, 
       *                   the list will start at the committee_member that comes after \c lowerbound
       * @param limit the maximum number of gatway_members to return (max: 1000)
       * @returns a list of gatway_members mapping committee_member names to committee_member ids
       */
     map<string, carrier_id_type> list_carriers(const string &lowerbound, uint32_t limit);
     
     map<string, author_id_type> list_authors(const string &lowerbound, uint32_t limit);

     vector<worker_object>        list_workers( );

      /** Returns information about the given witness.
       * @param start the name or id of the witness account owner, or the id of the witness
       * @param limit the name or id of the witness account owner, or the id of the witness
       * @returns the information about the witness stored in the block chain
       */

     vector<worker_object>        list_workers_history(worker_id_type start, uint64_t limit );
      /** Returns information about the given witness.
       * @param owner_account the name or id of the witness account owner, or the id of the witness
       * @param start the name or id of the witness account owner, or the id of the witness
       * @param limit the name or id of the witness account owner, or the id of the witness
       * @returns the information about the witness stored in the block chain
       */

     vector<proposal_object>        list_proposals_history(string owner_account,proposal_id_type start, uint64_t limit );

 



     /** Returns information about the given witness.
       * @param owner_account the name or id of the witness account owner, or the id of the witness
       * @returns the information about the witness stored in the block chain
       */
     witness_object get_witness(string owner_account);

     /** Returns information about the given committee_member.
       * @param owner_account the name or id of the committee_member account owner, or the id of the committee_member
       * @returns the information about the committee_member stored in the block chain
       */
     committee_member_object get_committee_member(string owner_account);

     /** Returns information about the given budget_member.
       * @param owner_account the name or id of the budget_member account owner, or the id of the budget_member
       * @returns the information about the budget_member stored in the block chain
       */
     budget_member_object get_budget_member(string owner_account);

     /** Creates a witness object owned by the given account.
       *
       * An account can have at most one witness object.
       *
       * @param owner_account the name or id of the account which is creating the witness
       * @param url a URL to include in the witness record in the blockchain.  Clients may
       *            display this when showing a list of witnesses.  May be blank.
       * @param memo  
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering a witness
       */
     signed_transaction create_witness(string owner_account,
                                       string url,
                                       string memo,
                                       bool broadcast = false);

     /**
       * Update a witness object owned by the given account.
       *
       * @param witness_name The name of the witness's owner account.  Also accepts the ID of the owner account or the ID of the witness.
       * @param url Same as for create_witness.  The empty string makes it remain the same.
       * @param memo 
       * @param block_signing_key The new block signing public key.  The empty string makes it remain the same.
       * @param broadcast true if you wish to broadcast the transaction.
       */
       


     signed_transaction update_witness(string witness_name,
                                       string url,
                                       optional<string> memo,
                                       optional<string> block_signing_key,
                                       bool broadcast = false);
    /**
       * Update a witness object owned by the given account.       
       * @param issuer_name
       * @param object_id The name of the witness's owner account.  Also accepts the ID of the owner account or the ID of the witness.
       * @param benable Same as for create_witness.  The empty string makes it remain the same.       
       * @param broadcast true if you wish to broadcast the transaction.
       */                                   
     signed_transaction change_identity( string issuer_name,
                                         object_id_type object_id,      
                                         bool benable,
                                         bool broadcast = false);

     signed_transaction enable_witness(string witness_name, bool broadcast);
     /**
       * Create a worker object.
       *
       * @param owner_account The account which owns the worker and will be paid
       * @param work_begin_date When the work begins
       * @param work_end_date When the work ends
       * @param daily_pay Amount of pay per day (NOT per maint interval)
       * @param name Any text
       * @param url Any text
       * @param memo
       * @param worker_settings {"type" : "burn"|"refund"|"vesting", "pay_vesting_period_days" : x}
       * @param broadcast true if you wish to broadcast the transaction.
       */
     signed_transaction create_worker(
         string owner_account,
         time_point_sec work_begin_date,
         time_point_sec work_end_date,
         string daily_pay,
         string name,
         string url,
         string memo,
         variant worker_settings,
         bool broadcast = false);

     vector<optional<worker_object>> get_workers(string account);
     vector<optional<worker_object>> get_workers_by_name(string worker);
 


     /**
       * Update your votes for a worker
       *
       * @param account The account which will pay the fee and update votes.
       * @param delta 
       * @param broadcast true if you wish to broadcast the transaction.
       */
     signed_transaction vote_for_worker(
         string account,
         worker_vote_delta delta,
         bool broadcast);

     /**
       * Get information about a vesting balance object.
       *
       * @param account_name An account name, account ID, or vesting balance object ID.
       */
     vector<vesting_balance_object_with_info> get_vesting_balances(string account_name);

     /**
       * Withdraw a vesting balance.
       *
       * @param witness_name The account name of the witness, also accepts account ID or vesting balance ID type.
       * @param amount The amount to withdraw.
       * @param asset_symbol The symbol of the asset to withdraw.
       * @param broadcast true if you wish to broadcast the transaction
       */
     signed_transaction withdraw_vesting(
         string witness_name,
         string amount,
         string asset_symbol,
         bool broadcast = false);

     signed_transaction revoke_vesting(
         string account_name,
         vesting_balance_id_type object_id,         
         bool broadcast = false);

     /** Vote for a given committee_member.
       *
       * An account can publish a list of all committee_memberes they approve of.  This 
       * command allows you to add or remove committee_memberes from this list.
       * Each account's vote is weighted according to the number of shares of the
       * core asset owned by that account at the time the votes are tallied.
       *
       * @note you cannot vote against a committee_member, you can only vote for the committee_member
       *       or not vote for the committee_member.
       *
       * @param voting_account the name or id of the account who is voting with their shares
       * @param committee_member the name or id of the committee_member' owner account
       * @param approve true if you wish to vote in favor of that committee_member, false to 
       *                remove your vote in favor of that committee_member
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed transaction changing your vote for the given committee_member
       */
     signed_transaction vote_for_committee_member(string voting_account,
                                                  string committee_member,
                                                  bool approve,
                                                  bool broadcast  );

      /** Vote for a given budget_member.
       *
       * An account can publish a list of all budget_memberes they approve of.  This 
       * command allows you to add or remove budget_memberes from this list.
       * Each account's vote is weighted according to the number of shares of the
       * core asset owned by that account at the time the votes are tallied.
       *
       * @note you cannot vote against a budget_member, you can only vote for the budget_member
       *       or not vote for the budget_member.
       *
       * @param voting_account the name or id of the account who is voting with their shares
       * @param budget_member the name or id of the budget_member' owner account
       * @param approve true if you wish to vote in favor of that budget_member, false to 
       *                remove your vote in favor of that budget_member
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed transaction changing your vote for the given budget_member
       */
     signed_transaction vote_for_budget_member(string voting_account,
                                                  string budget_member,
                                                  bool approve,
                                                  bool broadcast );


     /** Vote for a given witness.
       *
       * An account can publish a list of all witnesses they approve of.  This 
       * command allows you to add or remove witnesses from this list.
       * Each account's vote is weighted according to the number of shares of the
       * core asset owned by that account at the time the votes are tallied.
       *
       * @note you cannot vote against a witness, you can only vote for the witness
       *       or not vote for the witness.
       *
       * @param voting_account the name or id of the account who is voting with their shares
       * @param witness the name or id of the witness' owner account
       * @param approve true if you wish to vote in favor of that witness, false to 
       *                remove your vote in favor of that witness
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed transaction changing your vote for the given witness
       */
     signed_transaction vote_for_witness(string voting_account,
                                         string witness,
                                         bool approve,
                                         bool broadcast );

     /** Set the voting proxy for an account.
       *
       * If a user does not wish to take an active part in voting, they can choose
       * to allow another account to vote their stake.
       *
       * Setting a vote proxy does not remove your previous votes from the blockchain,
       * they remain there but are ignored.  If you later null out your vote proxy,
       * your previous votes will take effect again.
       *
       * This setting can be changed at any time.
       *
       * @param account_to_modify the name or id of the account to update
       * @param voting_account the name or id of an account authorized to vote account_to_modify's shares,
       *                       or null to vote your own shares
       *
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed transaction changing your vote proxy settings
       */
     signed_transaction set_voting_proxy(string account_to_modify,
                                         optional<string> voting_account,
                                         bool broadcast = false);

     /** Set your vote for the number of witnesses and budget_members in the system.
       *
       * Each account can voice their opinion on how many committee_members and how many 
       * witnesses there should be in the active committee_member/active witness list.  These
       * are independent of each other.  You must vote your approval of at least as many
       * committee_members or witnesses as you claim there should be (you can't say that there should
       * be 20 committee_members but only vote for 10). 
       *
       * There are maximum values for each set in the blockchain parameters (currently 
       * defaulting to 1001).
       *
       * This setting can be changed at any time.  If your account has a voting proxy
       * set, your preferences will be ignored.
       *
       * @param account_to_modify the name or id of the account to update
       * @param desired_number_of_witnesses the number 
       * @param desired_number_of_committee_members the number 
       * @param desired_number_of_budget_members the number        
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed transaction changing your vote proxy settings
       */
     signed_transaction set_desired_member_count(string account_to_modify,
                                                                       uint16_t desired_number_of_witnesses,
                                                                       uint16_t desired_number_of_committee_members,
                                                                       uint16_t desired_number_of_budget_members,
                                                                       bool broadcast = false);

     /** Signs a transaction.
       *
       * Given a fully-formed transaction that is only lacking signatures, this signs
       * the transaction with the necessary keys and optionally broadcasts the transaction
       * @param tx the unsigned transaction
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed version of the transaction
       */
     signed_transaction sign_transaction(signed_transaction tx, bool broadcast = false);

     /** Returns an uninitialized object representing a given blockchain operation.
       *
       * This returns a default-initialized object of the given type; it can be used 
       * during early development of the wallet when we don't yet have custom commands for
       * creating all of the operations the blockchain supports.  
       *
       * Any operation the blockchain supports can be created using the transaction builder's
       * \c add_operation_to_builder_transaction() , but to do that from the CLI you need to 
       * know what the JSON form of the operation looks like.  This will give you a template
       * you can fill in.  It's better than nothing.
       * 
       * @param operation_type the type of operation to return, must be one of the 
       *                       operations defined in `graphene/chain/operations.hpp`
       *                       (e.g., "global_parameters_update_operation")
       * @return a default-constructed operation of the given type
       */
     operation get_prototype_operation(string operation_type);

     /** Returns an uninitialized object representing a given blockchain operation.
       *
       * This returns a default-initialized object of the given type; it can be used 
       * during early development of the wallet when we don't yet have custom commands for
       * creating all of the operations the blockchain supports.  
       *
       * Any operation the blockchain supports can be created using the transaction builder's
       * \c add_operation_to_builder_transaction() , but to do that from the CLI you need to 
       * know what the JSON form of the operation looks like.  This will give you a template
       * you can fill in.  It's better than nothing.
      
       * @return a default-constructed operation of the given type
       */

     flat_map<string, operation> get_prototype_operations();

     /** Creates a transaction to propose a parameter change.
       *
       * Multiple parameters can be specified if an atomic change is
       * desired.
       *
       * @param proposing_account The account paying the fee to propose the tx
       * @param expiration_time Timestamp specifying when the proposal will either take effect or expire.
       * @param changed_values The values to change; all other chain parameters are filled in with default values
       * @param memo true if you wish to broadcast the transaction 
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed version of the transaction
       */
     signed_transaction propose_parameter_change(
         const string &proposing_account,
         fc::time_point_sec expiration_time,
         const variant_object &changed_values,
         optional<string> memo,
         bool broadcast = false);

     signed_transaction propose_zosparameter_change(
         const string &proposing_account,
         fc::time_point_sec expiration_time,
         const variant_object &changed_values,
         optional<string> memo,
         bool broadcast = false);


     /** Propose a fee change.
       * propose_fee_change nathan "2018-08-24T:10:00:00"  {"carrier_create":{"fee": 2100000,"lock_fee": 1100000}} true
       * @param proposing_account The account paying the fee to propose the tx
       * @param expiration_time Timestamp specifying when the proposal will either take effect or expire.
       * @param changed_values Map of operation type to new fee.  Operations may be specified by name or ID.
       *    The "scale" key changes the scale.  All other operations will maintain current values.
       * @param memo true if you wish to broadcast the transaction
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed version of the transaction
       * 
       */
     signed_transaction propose_fee_change(
         const string &proposing_account,
         fc::time_point_sec expiration_time,
         const variant_object &changed_values,
         optional<string> memo,
         bool broadcast = false);

     /** Propose a fee change.
       * 
       * @param proposing_account The account paying the fee to propose the tx
       * @param symbol The account paying the fee to propose the tx
       * @param bcash Timestamp specifying when the proposal will either take effect or expire.     
       * @param broadcast Map of operation type to new fee.  Operations may be specified by name or ID.       
       * @return the signed version of the transaction
       */
      signed_transaction propose_asset_to_cash(
         const string& proposing_account,
         const string& symbol,
         bool  bcash,         
         bool broadcast = false);

      signed_transaction locktoken_options_param(
          const string &issuer,
          const string &asset_lock,
          const uint32_t& lock_mode,              
          const variant_object &changed_values,
          bool broadcast = false);

      signed_transaction locktoken_options_enablepayer(
          const string &issuer,
          const string &asset_lock,
          const uint32_t &lock_mode ,bool broadcast = false);

      signed_transaction locktoken_create(
        const string   issuer,      
        const string   to,      
        const string   amount,
        const string   asset_symbol,        
        const uint32_t type,
        const uint32_t period,        
        const uint32_t autolock, 
        bool broadcast = false);

      signed_transaction locktoken_update(
          const string issuer,
          const locktoken_id_type locktoken_id,
          const uint32_t op_type,
          const string amount,
          const string asset_symbol,
          const uint32_t type,
          const uint32_t period,
          const uint32_t autolock,
          bool broadcast = false);

      signed_transaction locktoken_remove(
          const string issuer,
          const locktoken_id_type id,
          bool broadcast = false);

      fc::variant_object get_locktoken_sum(string account, string symbol, uint32_t utype);

      fc::optional<locktoken_option_object> get_locktoken_option(string symbol, uint32_t lock_type , bool checkasset)const;

      vector<locktoken_id_type> get_locktokens(string account, string symbol,uint32_t type) const;

      vector<locktoken_object> get_locktoken_history(string account, string symbol, uint32_t utype, uint64_t ustart, uint64_t ulimit) const;
      /** Propose a fee change.
       * 
       * @param proposing_account The account paying the fee to propose the tx
       * @param symbol The account paying the fee to propose the tx
       * @param ubitproperty Timestamp specifying when the proposal will either take effect or expire.
       * @param umask Timestamp specifying when the proposal will either take effect or expire.
       * @param broadcast Map of operation type to new fee.  Operations may be specified by name or ID.       
       * @return the signed version of the transaction
       */
      signed_transaction propose_asset_bitlender_property(
         const string& proposing_account,
         const string& symbol,
         uint32_t      ubitproperty,  
         uint32_t      umask,          
         bool broadcast = false);
 



      /** Approve or disapprove a proposal.
       *
       * @param fee_paying_account The account paying the fee for the op.
       * @param proposal_id The proposal to modify.
       * @param delta Members contain approvals to create or remove.  In JSON you can leave empty members undefined.
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed version of the transaction
       */
      signed_transaction approve_proposal(
         const string& fee_paying_account,
         const string& proposal_id,
         const approval_delta& delta,
         bool broadcast /* = false */
         );
   
      /** Approve or disapprove a proposal.
       *
       * @param fee_paying_account The account paying the fee for the op.
       * @param proposal_id The proposal to modify.
       * @param using_owner_authority Members contain approvals to create or remove.  In JSON you can leave empty members undefined.
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed version of the transaction
       */
      signed_transaction remove_proposal(
         const string& fee_paying_account,
         const string& proposal_id,
         bool  using_owner_authority , 
         bool broadcast /* = false */
         );


      order_book get_order_book( const string& base, const string& quote, unsigned limit = 50);
      void       get_gas_info();
      void dbg_make_uia(string creator, string symbol);
      void dbg_make_mia(string creator, string symbol);
      void dbg_push_blocks( std::string src_filename, uint32_t count );
      void dbg_generate_blocks( std::string debug_wif_key, uint32_t count );
      void dbg_stream_json_objects( const std::string& filename );
      void dbg_update_object( fc::variant_object update );

      void flood_network(string prefix, uint32_t number_of_transactions);

      void network_add_nodes( const vector<string>& nodes );
      vector< variant > network_get_connected_peers();
      vector< variant > network_get_potential_peers();
      fc::variant_object network_get_advanced_node_parameters() const;
      /**
       * @brief Set advanced node parameters, such as desired and max
       *        number of connections
       * @param params a JSON object containing the name/value pairs for the parameters to set
      */
      void network_set_advanced_node_parameters(const fc::variant_object& params);
      /**
       *  Used to transfer from one set of blinded balances to another
       */
      blind_confirmation blind_transfer_help( string from_key_or_label,
                                         string to_key_or_label,
                                         string amount,
                                         string symbol,
                                         bool broadcast = false,
                                         bool to_temp = false );


      std::map<string,std::function<string(fc::variant,const fc::variants&)>> get_result_formatters() const;

      fc::signal<void(bool)> lock_changed;
      std::shared_ptr<detail::wallet_api_impl> my;
      void encrypt_keys();

      void test(uint64_t start);
};

} }

 
FC_REFLECT( graphene::wallet::key_label, (label)(key) )
FC_REFLECT( graphene::wallet::blind_balance, (amount)(from)(to)(one_time_key)(blinding_factor)(commitment)(used) )
FC_REFLECT( graphene::wallet::blind_confirmation::output, (label)(pub_key)(decrypted_memo)(confirmation)(auth)(confirmation_receipt) )
FC_REFLECT( graphene::wallet::blind_confirmation, (trx)(outputs) )

FC_REFLECT( graphene::wallet::plain_keys, (keys)(checksum) )

FC_REFLECT( graphene::wallet::wallet_data,
            (chain_id)
            (my_accounts)
            (cipher_keys)
            (extra_keys)
            (pending_account_registrations)(pending_witness_registrations)
            (labeled_keys)
            (blind_receipts)
            (ws_server)
            (ws_user)
            (ws_password)
          )

FC_REFLECT( graphene::wallet::brain_key_info,
            (brain_priv_key)
            (wif_priv_key)
            (pub_key)
            (add_ress)
          )

  

FC_REFLECT( graphene::wallet::exported_account_keys, (account_name)(encrypted_private_keys)(public_keys) )

FC_REFLECT( graphene::wallet::exported_keys, (password_checksum)(account_keys) )

FC_REFLECT( graphene::wallet::blind_receipt,
            (date)(from_key)(from_label)(to_key)(to_label)(amount)(memo)(control_authority)(data)(used)(conf) )

FC_REFLECT( graphene::wallet::approval_delta,
   (active_approvals_to_add)
   (active_approvals_to_remove)
   (owner_approvals_to_add)
   (owner_approvals_to_remove)
   (key_approvals_to_add)
   (key_approvals_to_remove)
)

FC_REFLECT( graphene::wallet::worker_vote_delta,
   (vote_for)
   (vote_against)
   (vote_abstain)
)

FC_REFLECT( graphene::wallet::account_info,
   (zone)
   (mobile)
   (mail)   
)


FC_REFLECT_DERIVED( graphene::wallet::signed_block_with_info, (graphene::chain::signed_block),
   (block_id)(signing_key)(transaction_ids) )

FC_REFLECT_DERIVED( graphene::wallet::vesting_balance_object_with_info, (graphene::chain::vesting_balance_object),
   (allowed_withdraw)(allowed_withdraw_time) )

FC_REFLECT( graphene::wallet::operation_detail, 
            (memo)(description)(op) )

FC_REFLECT(graphene::wallet::operation_detail_ex,
            (memo)(description)(op)(transaction_id))

FC_REFLECT( graphene::wallet::account_history_operation_detail,
        (total_count)(result_count)(details))
FC_REFLECT( graphene::wallet::account_key,
        (wif_key)(pub_key)(add_ress)) 

FC_REFLECT( graphene::wallet::account_key_info,
        (id)(name)(active)(owner)(memo)(auth)(witness)) 

      

FC_API( graphene::wallet::wallet_api,
        (help)
        (gethelp)
        (info)
        (node_info)
        (about)        
        (begin_builder_transaction)
        (add_operation_to_builder_transaction)
        (replace_operation_in_builder_transaction)
        (set_fees_on_builder_transaction)
        (preview_builder_transaction)
        (sign_builder_transaction)
        (broadcast_transaction) 
        (broadcast_build_transaction)           
        (propose_builder_transaction)
        (remove_builder_transaction)        
        (load_wallet_file)
        (save_wallet_file)
        (serialize_transaction)
        (sign_transaction)
        (is_new)
        (is_locked)
        (lock)
        (unlock)
        (set_password)
        (dump_private_keys)
        (dump_account_keys)
        (get_account_balance)
        (get_account_balances_summary)
        (list_my_accounts)
        (list_accounts)
        (list_account_balances)
        (list_all_balances)
        (sort_balances)
        (list_assets)
        (import_key)
        (import_password)
        (remove_key)
        (change_password)  
        (import_accounts)
        (import_account_keys)
        (import_balance)      
        (get_vesting_balances)
        (withdraw_vesting)
        (revoke_vesting)
        (withdraw_exchange_fee)
        (suggest_brain_key)
        (suggest_account_key)
       // (derive_owner_keys_from_brain_key)
        (register_account)
        (propose_register_account)
        (create_account)
        (get_account_auth)
        (set_account_auth)    
        (set_account_pubkey) 
        (set_auth_state)     
        (set_account_info)     
        (get_account_info)   
        (get_account_config)    
        // (get_admin_config)          
        (upgrade_account)
        (set_account_lock_balance)
        (set_propose_register)    
        (set_account_config)   
        (create_account_with_brain_key)
        (account_coupon)
        (set_auth_payfor)
        (is_authenticator)
        (set_gateway_need_auth)
        (set_carrier_need_auth)
        (set_carrier_can_invest)        
        (sell_asset)
        (buy_asset)
       // (borrow_asset)
       // (cancel_order)
        (transfer)
        (transfer_noencmsg)
        (transfer_vesting)
        //(transfer2)
        (propose_transfer)
        (get_transaction_id)
        (asset_reserve_fees)
        (asset_claim_fees)
        (create_asset)        
        (update_asset)    
        (update_asset_gateway)    
        (update_bitasset)                
        (update_asset_feed_producers)        
        (propose_bitlender_feed_producers)
        (publish_asset_feed)
        (locktoken_options_param)
        (locktoken_options_enablepayer)
        (locktoken_create)
        (locktoken_update)
        (locktoken_remove)
        (get_locktoken_sum)
        (get_locktoken_option)
        (get_locktokens)
        (get_locktoken_history)        
        (publish_bitlender_feed)
        (create_bitlender_option)
        (update_bitlender_option_author)
        (update_bitlender_option)
        (stop_bitlender_option)
        (fee_mode_bitlender_option)
        (update_bitlender_paramers)
        (update_bitlender_rate)
        (get_bitlender_option)
        (get_bitlender_option_key)
        (get_bitlender_paramers)
        //(get_bitlender_paramers_key)

        //(create_issue_fundraise)
        //(update_issue_fundraise)
        //(issue_fundraise_publish_feed)  
        //(remove_issue_fundraise)  
        //(create_buy_fundraise)
        //(enable_buy_fundraise)        
        //(get_issue_fundraise)
        //(list_issue_fundraise)        
        //(list_my_issue_fundraise)
        //(list_buy_fundraise)        
        //(list_buy_fundraise_by_id)
        //(list_my_buy_fundraise)
        //(list_sell_exchange)        
        //(list_my_sell_exchange)
        //(list_buy_exchange)        
        //(list_buy_exchange_by_id)
        //(list_my_buy_exchange) 
        //(update_finance_paramers)        
        //(get_finance_paramers)
        //(create_sell_exchange)
        //(update_sell_exchange)
        //(remove_sell_exchange)
        //(create_buy_exchange)
        //(update_buy_exchange)
        //(remove_buy_exchange)  


        (bitlender_lend_order)
        (bitlender_loan_info)
        (bitlender_invest_order)
        (bitlender_repay_interest)
        (bitlender_overdue_interest)
        (bitlender_recycle_interest)
        (bitlender_repay_principal)
        (bitlender_pre_repay_principal)                                      
        (bitlender_overdue_repay_principal)
        (bitlender_recycle_principal)
        (bitlender_remove_order)
        (bitlender_set_auto_repayment)
        (bitlender_add_collateral)    
        (list_bitlender_order)
        (bitlender_test_principal)
        (issue_asset)
        (get_asset)
        (get_bitasset_data)
        (get_asset_exchange_feed)
        (get_dynamic_data_object)
        //(fund_asset_fee_pool)
        (reserve_asset)
        //(global_settle_asset)
       // (settle_asset)
       // (bid_collateral)
       // (whitelist_account)
        (create_committee_member)
        (update_committee_member)  
        (create_budget_member)
        (update_budget_member)        
        (create_gateway)
        (update_gateway)
        (get_gateway)
        (create_carrier)
        (get_carrier)
        (update_carrier)
        (create_author)
        (get_author)
        (update_author)
        (gateway_issue_currency)
        (gateway_deposit)
        (gateway_propose_deposit)        
        (gateway_withdraw)
        (get_witness)        
        (get_committee_member)   
        (get_budget_member)   
        (list_witnesses)
        (list_committee_members)
        (list_budget_members)
        (list_gateways)
        (list_carriers)
        (list_authors)
        (list_workers)
        (list_workers_history)
        (list_proposals_history)
        (create_witness)
        (update_witness)
        (change_identity)
        (enable_witness)
        (create_worker)
        (get_workers_by_name)
        (get_workers)
        (vote_for_worker)               
        (vote_for_committee_member)
        (vote_for_budget_member)
        (vote_for_witness)
        (set_voting_proxy)
        (set_desired_member_count)
        (get_account)
        (get_account_id)
        (get_block)
        (get_block_ids)
        (get_transaction)
        (get_account_count)
        (get_object_count)
        (get_account_history)
        (get_relative_account_history)
        (get_balance_history)    
        (get_balance_history_object)    
        (get_account_history_by_operations)
       // (get_collateral_bids)
        (is_public_key_registered)
       // (get_market_history)
        (get_bitlender_loan_history)        
        (get_account_bitlender_history)
        (get_account_loan_history)
        (get_account_invest_history)
        //(get_account_issue_fundraise_history)
        //(get_account_buy_fundraise_history)
        //(get_account_sell_exchange_history)
        //(get_account_buy_exchange_history)
        (get_object_history)        
        (get_global_properties)
        (get_dynamic_global_properties)
        (get_object)
        (get_proposed_transaction)     
        (get_my_proposed_transaction)    
        (get_private_key)      
        (normalize_brain_key)
       // (get_limit_orders)
       // (get_call_orders)
        (get_loan_orders)
        (get_notify_orders)
        (get_invest_orders)         
       // (get_settle_orders)
        (get_ticker)       
        (get_prototype_operation)       
        (propose_parameter_change)      
        (propose_zosparameter_change)   
        (propose_fee_change)        
        (propose_asset_bitlender_property)
        (approve_proposal)        
        (remove_proposal)     
       // (flood_network)
 
        (network_add_nodes)
        (network_get_connected_peers)
        (network_get_potential_peers)
        (network_get_advanced_node_parameters)
        (network_set_advanced_node_parameters)

        //(sign_memo)
        //(read_memo)
        //(set_key_label)
        //(get_key_label)
        //(get_public_key)
        //(get_blind_accounts)    
        //(blind_history)   
        //(create_blind_account)
        //(get_blind_balances)              
        //(transfer_to_blind)
        //(transfer_from_blind)
        //(blind_transfer)       
        //(receive_blind_transfer)
        //(get_order_book)
      //  (get_gas_info)
        (gateway_create_withdraw)
        (gateway_update_withdraw)
        (gateway_remove_withdraw)
        (gateway_claim_withdraw)  
        (test)      
      )
  // (dbg_make_uia)
      //  (dbg_make_mia)
       // (dbg_push_blocks)
       // (dbg_generate_blocks)
       // (dbg_stream_json_objects)
       // (dbg_update_object)