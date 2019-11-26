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

#include <graphene/app/database_api.hpp>
#include <graphene/app/util.hpp>
#include <graphene/chain/get_config.hpp>
#include <graphene/chain/block_summary_object.hpp>
#include <graphene/business/bitlender_evaluator.hpp>
#include <graphene/chain/account_member_evaluator.hpp>

#include <fc/bloom_filter.hpp>
#include <fc/smart_ref_impl.hpp>

#include <fc/crypto/hex.hpp>
#include <fc/uint128.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/rational.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <graphene/utilities/key_conversion.hpp>

#include <cctype>

#include <cfenv>
#include <iostream>

#define GET_REQUIRED_FEES_MAX_RECURSION 4

typedef std::map< std::pair<graphene::chain::asset_id_type, graphene::chain::asset_id_type>, std::vector<fc::variant> > market_queue_type;
typedef market_queue_type  bitlender_queue_type;

namespace graphene { namespace app {

 
class database_api_impl : public std::enable_shared_from_this<database_api_impl>
{
   public:
      explicit database_api_impl( graphene::chain::database& db, application& app);
      ~database_api_impl();


      // Objects
      fc::variants get_objects(const vector<object_id_type>& ids)const;

      // Subscriptions
      void set_subscribe_callback( std::function<void(const variant&)> cb, bool notify_remove_create );
      void set_pending_transaction_callback( std::function<void(const variant&)> cb );
      void set_block_applied_callback( std::function<void(const variant& block_id)> cb );
      void cancel_all_subscriptions();

      // Blocks and transactions
      optional<block_header> get_block_header(uint64_t block_num)const;
      map<uint64_t, optional<block_header>> get_block_header_batch(const vector<uint64_t> block_nums)const;
      optional<signed_block> get_block(uint64_t block_num)const;
      optional<signed_block_info> get_block_ids(uint64_t block_num) const;
      optional<signed_block_info> get_block_with_id(uint64_t block_num, transaction_id_type trx_id) const;
      processed_transaction get_transaction( uint64_t block_num, uint32_t trx_in_block )const;
      optional<processed_transaction> get_transaction_by_id( uint64_t block_num, transaction_id_type trx_id)const;
      // Globals
      chain_property_object          get_chain_properties()const;
      global_property_object         get_global_properties()const;
      bitlender_paramers_object      get_bitlender_paramers()const;
      bitlender_paramers_object_key  get_bitlender_paramers_key(optional<bitlender_key> key)const;
      finance_paramers_object        get_finance_paramers()const;
      fc::variant_object get_config()const;
      chain_id_type get_chain_id()const;
      dynamic_global_property_object get_dynamic_global_properties()const;
   

      // Keys
      vector<vector<account_id_type>> get_key_references( vector<public_key_type> key )const;
      bool is_public_key_registered(string public_key) const;

      // Accounts
      vector<optional<account_object>> get_accounts(const vector<account_id_type>& account_ids)const;
      vector<optional<account_statistics_object>> get_accounts_statistics(const vector<account_id_type> &account_ids) const;
      std::map<string, full_account> get_full_accounts(const vector<string> &names_or_ids, bool subscribe);
      optional<account_object> get_account_by_name( string name )const;
      vector<account_info> get_account_author( account_id_type account_id )const;
      vector<account_id_type> get_account_references( account_id_type account_id )const;
      vector<operation_history_object>  get_account_history(account_id_type account, vector<int64_t> optypes, int64_t start, int64_t offset) const;
      vector<optional<account_object>> lookup_account_names(const vector<string>& account_names)const;
      map<string,account_id_type> lookup_accounts(const string& lower_bound_name, uint32_t limit)const;
      map<string,account_id_type> lookup_accounts_by_type(const string& lower_bound_name, uint32_t utype, uint32_t limit)const;
      vector<account_info>         lookup_accounts_by_property(uint32_t utype, uint32_t limit)const;
      vector<carrier_id_type>  lookup_carrier(const string& url)const;
      uint64_t get_account_count()const;
      uint64_t get_object_count(object_id_type id,bool bmaxid)const;
      vector<string>    get_account_config(account_id_type account_id ,vector<string> sindex)const;

      // Balances
      vector<asset_summary> get_account_balances(account_id_type id, const flat_set<asset_id_type>& assets)const;
      vector<asset_summary> get_named_account_balances(const std::string& name, const flat_set<asset_id_type>& assets)const;    vector<asset_summary> get_account_balances_summary(account_id_type id,uint32_t utype) const;
      vector<asset> get_account_lock_balances(account_id_type id, uint32_t utype) const;
      vector<balance_object> get_balance_objects( const vector<address>& addrs )const;
      vector<asset> get_vested_balances( const vector<balance_id_type>& objs )const;
      vector<vesting_balance_object> get_vesting_balances( account_id_type account_id )const;
      vector<vesting_balance_data> get_vesting_balances_data( account_id_type account_id )const;

      // Assets
      vector<optional<asset_object>> get_assets(const vector<asset_id_type>& asset_ids)const;
      vector<asset_object>           list_assets(const string& lower_bound_symbol, uint32_t limit)const;
      vector<optional<asset_object>> lookup_asset_symbols(const vector<string>& symbols_or_ids)const;

      // Markets / feeds
      vector<limit_order_object>         get_limit_orders(asset_id_type a, asset_id_type b, uint32_t limit)const;
      vector<object_id_type>             get_account_limit_orders(account_id_type acc, asset_id_type a, asset_id_type b)const;
      vector<call_order_object>          get_call_orders(asset_id_type a, uint32_t limit)const;
      vector<force_settlement_object>    get_settle_orders(asset_id_type a, uint32_t limit)const;
      vector<call_order_object>          get_margin_positions( const account_id_type& id )const;
      vector<collateral_bid_object>      get_collateral_bids(const asset_id_type asset, uint32_t limit, uint32_t start)const;
      vector<bitlender_order_info>       get_loan_orders(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const vector<uint32_t> &status, uint32_t start, uint32_t limit)const;
      vector<bitlender_order_info>       get_notify_orders(const account_id_type& id,uint32_t status, uint32_t start, uint32_t limit) const;      
      vector<bitlender_invest_info>      get_invest_orders(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const vector<uint32_t> &status, uint32_t start, uint32_t limit)const;
      uint32_t                           get_invest_counts(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const vector<uint32_t> &status)const;
      uint32_t                           get_loan_counts(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const vector<uint32_t> &status)const;

      void subscribe_to_market(std::function<void(const variant&)> callback, asset_id_type a, asset_id_type b);
      void unsubscribe_from_market(asset_id_type a, asset_id_type b);
      market_ticker                      get_ticker( const string& base, const string& quote, bool skip_order_book = false )const;
      market_ticker                      get_finance_ticker( const string& base, const string& quote, bool skip_order_book = false )const;
      market_volume                      get_24_volume( const string& base, const string& quote )const;
      order_book                         get_order_book( const string& base, const string& quote, unsigned limit = 50 )const;
      vector<market_volume>              get_top_markets(uint32_t limit)const;
      vector<market_trade>               get_trade_history( const string& base, const string& quote, fc::time_point_sec start, fc::time_point_sec stop, unsigned limit = 100 )const;
      vector<market_trade>               get_trade_history_by_sequence( const string& base, const string& quote, int64_t start, fc::time_point_sec stop, unsigned limit = 100 )const;

      void subscribe_to_bitlender(std::function<void(const variant&)> callback, asset_id_type a, asset_id_type b);
      void unsubscribe_from_bitlender(asset_id_type a, asset_id_type b);

      // Witnesses
      vector<optional<witness_object>> get_witnesses(const vector<witness_id_type>& witness_ids)const;
      fc::optional<witness_object> get_witness_by_account(account_id_type account)const;
      map<string, witness_id_type> lookup_witness_accounts(const string& lower_bound_name, uint32_t limit)const;
      uint64_t get_witness_count()const;

      // Committee members
      vector<optional<committee_member_object>> get_committee_members(const vector<committee_member_id_type>& committee_member_ids)const;
      fc::optional<committee_member_object> get_committee_member_by_account(account_id_type account)const;
      map<string, committee_member_id_type> lookup_committee_member_accounts(const string& lower_bound_name, uint32_t limit)const;          
      uint64_t get_committee_count()const;

       // Budget members
      vector<optional<budget_member_object>> get_budget_members(const vector<budget_member_id_type>& budget_member_ids)const;
      fc::optional<budget_member_object> get_budget_member_by_account(account_id_type account)const;
      map<string, budget_member_id_type> lookup_budget_member_accounts(const string& lower_bound_name, uint32_t limit)const;          
      uint64_t get_budget_count()const;


      // gatewayes
      fc::variant_object get_account_attachinfo(const account_id_type acc, uint32_t utype) const;
      optional<gateway_cfg> get_gateway_cfg(const gateway_id_type gateway_id, const asset_id_type asset_id) const;
      vector<optional<gateway_object>> get_gatewayes(const vector<gateway_id_type>& gateway_ids)const;
      fc::optional<gateway_object> get_gateway_by_account(account_id_type account)const;
      map<string, gateway_id_type> lookup_gateway_accounts(const string& lower_bound_name, uint32_t limit)const;
      uint64_t get_gateway_count()const;

        // carrieres
      vector<optional<carrier_object>> get_carrieres(const vector<carrier_id_type>& carrier_ids)const;
      fc::optional<carrier_object> get_carrier_by_account(account_id_type account)const;      
      map<string, carrier_id_type> lookup_carrier_accounts(const string& lower_bound_name, uint32_t limit)const;
      uint64_t get_carrier_count()const;
      asset get_allowed_withdraw(const object_id_type id, const fc::time_point_sec &now) const;

      // authors
      vector<optional<author_object>> get_authors(const vector<author_id_type>& author_ids)const;
      fc::optional<author_object> get_author_by_account(account_id_type account)const;
      optional<carrier_cfg> get_carrier_cfg(const carrier_id_type carrier_id, const asset_id_type asset_id) const;
      map<string, author_id_type> lookup_author_accounts(const string& lower_bound_name, uint32_t limit)const;
      fc::optional<author_object> get_author_by_trust(object_id_type object_id,asset_id_type asset_id)const;
      uint64_t get_author_count()const;


      // Workers
      vector<worker_object> get_all_workers()const;
      vector<optional<worker_object>> get_workers_by_account(account_id_type account)const;
      vector<optional<worker_object>> get_workers_by_name(string name)const;
      uint64_t get_worker_count()const;

      vector<locktoken_id_type> get_locktokens(account_id_type account,asset_id_type asset_id,uint32_t type)const;
      fc::variant_object get_locktoken_sum(account_id_type account, asset_id_type asset_id, uint32_t utype)const;
      fc::optional<locktoken_option_object> get_locktoken_option(asset_id_type asset_id, uint32_t lock_type , uint32_t checkasset)const;
      vector<locktoken_option_object> get_locktoken_options(uint32_t lock_type,uint32_t checkasset )const;

      // Votes
      vector<variant> lookup_vote_ids( const vector<vote_id_type>& votes )const;

      // Authority / validation
      std::string get_transaction_hex(const signed_transaction& trx)const;
      set<public_key_type> get_required_signatures( const signed_transaction& trx, const flat_set<public_key_type>& available_keys )const;
      set<public_key_type> get_potential_signatures( const signed_transaction& trx )const;
      set<address> get_potential_address_signatures( const signed_transaction& trx )const;
      bool       verify_authority( const signed_transaction& trx )const;
      int32_t    is_authenticator(const account_id_type account,const uint32_t type,const asset_id_type asset_id,const account_id_type author) const;
      bool       is_notify_info(const account_id_type account) const;
      vector<asset_id_type>    get_auth_asset(const account_id_type account,const uint32_t type) const;


      bool verify_account_authority( const string& name_or_id, const flat_set<public_key_type>& signers )const;
      processed_transaction validate_transaction( const signed_transaction& trx )const;
      void                  validate_opertation( const operation &op,bool is_proposed_trx )const;
      void                  validate_proposal( const signed_transaction& trx )const;

      vector< fc::variant > get_required_fees( const vector<operation>& ops, asset_id_type id )const;

      // Proposed transactions
      vector<proposal_object> get_proposed_transactions( account_id_type id )const;

      flat_map<public_key_type,proposal_object> get_proposed_key_transactions( account_id_type id )const;

      // Proposed transactions
      vector<proposal_object> get_my_proposed_transactions( account_id_type id)const;
      
      // Blinded balances
      vector<blinded_balance_object> get_blinded_balances( const flat_set<commitment_type>& commitments )const;

      // Withdrawals
      vector<withdraw_permission_object> get_withdraw_permissions_by_giver(account_id_type account, withdraw_permission_id_type start, uint32_t limit)const;
      vector<withdraw_permission_object> get_withdraw_permissions_by_recipient(account_id_type account, withdraw_permission_id_type start, uint32_t limit)const;

      fc::optional<bitlender_option_object> get_bitlender_option(asset_id_type asset_id)const;
      fc::optional<bitlender_option_object_key> get_bitlender_option_key(asset_id_type asset_id,optional<bitlender_key> key)const;
      fc::optional<issue_fundraise_object>   get_issue_fundraise(asset_id_type asset_id,asset_id_type pay_id)const;
      fc::optional<asset_exchange_feed_object>      get_asset_exchange_feed(asset_id_type asset_base,asset_id_type asset_quote,uint32_t utype)const;

      uint32_t                              get_budget_pre_block( fc::time_point_sec now)const;
   
      
   //private:
      static string price_to_string( const price& _price, const asset_object& _base, const asset_object& _quote );

      template<typename T>
      void subscribe_to_item( const T& i )const
      {
         auto vec = fc::raw::pack(i);
         if( !_subscribe_callback )
            return;

         if( !is_subscribed_to_item(i) )
            _subscribe_filter.insert( vec.data(), vec.size() );
      }

      template<typename T>
      bool is_subscribed_to_item( const T& i )const
      {
         if( !_subscribe_callback )
            return false;

         return _subscribe_filter.contains( i );
      }

      bool is_impacted_account( const flat_set<account_id_type>& accounts)
      {
         if( !_subscribed_accounts.size() || !accounts.size() )
            return false;

         return std::any_of(accounts.begin(), accounts.end(), [this](const account_id_type& account) {
            return _subscribed_accounts.find(account) != _subscribed_accounts.end();
         });
      }

      const std::pair<asset_id_type,asset_id_type> get_order_market( const force_settlement_object& order )
      {
         // TODO cache the result to avoid repeatly fetching from db
         asset_id_type backing_id = order.balance.asset_id( _db ).bitasset_data( _db ).options.short_backing_asset;
         auto tmp = std::make_pair( order.balance.asset_id, backing_id );
         if( tmp.first > tmp.second ) std::swap( tmp.first, tmp.second );
         return tmp;
      }

      template<typename T>
      const std::pair<asset_id_type,asset_id_type> get_order_market( const T& order )
      {
         return order.get_market();
      }

      template<typename T>
      void enqueue_if_subscribed_to_market(const object* obj, market_queue_type& queue, bool full_object=true)
      {
         const T* order = dynamic_cast<const T*>(obj);
         FC_ASSERT( order != nullptr);

         const auto& market = get_order_market( *order );

         auto sub = _market_subscriptions.find( market );
         if( sub != _market_subscriptions.end() ) {
            queue[market].emplace_back( full_object ? obj->to_variant() : fc::variant(obj->id, 1) );
         }
      }
     template<typename T>
      void enqueue_if_subscribed_to_bitlender(const object* obj, market_queue_type& queue, bool full_object=true)
      {
         const T* order = dynamic_cast<const T*>(obj);
         FC_ASSERT( order != nullptr);

         const auto& market = get_order_market( *order );

         auto sub = _bitlender_subscriptions.find( market );
         if( sub != _bitlender_subscriptions.end() ) {
            queue[market].emplace_back( full_object ? obj->to_variant() : fc::variant(obj->id, 1) );
         }
      }

      void broadcast_updates( const vector<variant>& updates );
      void broadcast_market_updates( const market_queue_type& queue);
      void broadcast_bitlender_updates( const market_queue_type& queue);
      void handle_object_changed(bool force_notify, bool full_object, const vector<object_id_type>& ids, const flat_set<account_id_type>& impacted_accounts, std::function<const object*(object_id_type id)> find_object);

      /** called every time a block is applied to report the objects that were changed */
      void on_objects_new(const vector<object_id_type>& ids, const flat_set<account_id_type>& impacted_accounts);
      void on_objects_changed(const vector<object_id_type>& ids, const flat_set<account_id_type>& impacted_accounts);
      void on_objects_removed(const vector<object_id_type>& ids, const vector<const object*>& objs, const flat_set<account_id_type>& impacted_accounts);
      void on_applied_block();

      bool is_proposal_author(database &db,const vector<account_id_type> &account_id, account_id_type &author_id,uint32_t &dep ) const;
      void is_proposal_key(database &db,const vector<account_id_type> &account_id, account_id_type &author_id,const proposal_object &p,flat_map<public_key_type,proposal_object> &keys, uint32_t &dep) const;

      bool _notify_remove_create = false;
      mutable fc::bloom_filter _subscribe_filter;
      std::set<account_id_type> _subscribed_accounts;
      std::function<void(const fc::variant&)> _subscribe_callback;
      std::function<void(const fc::variant&)> _pending_trx_callback;
      std::function<void(const fc::variant&)> _block_applied_callback;

      boost::signals2::scoped_connection                                                                                           _new_connection;
      boost::signals2::scoped_connection                                                                                           _change_connection;
      boost::signals2::scoped_connection                                                                                           _removed_connection;
      boost::signals2::scoped_connection                                                                                           _applied_block_connection;
      boost::signals2::scoped_connection                                                                                           _pending_trx_connection;
      map< pair<asset_id_type,asset_id_type>, std::function<void(const variant&)> >      _market_subscriptions;
      map< pair<asset_id_type,asset_id_type>, std::function<void(const variant&)> >      _bitlender_subscriptions;
      graphene::chain::database&                                                         _db;
      application&                                                                       _app;
       
};

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Constructors                                                     //
//                                                                  //
//////////////////////////////////////////////////////////////////////

database_api::database_api( graphene::chain::database& db ,application& app )
   : my( new database_api_impl( db,app ) ),_app(app) {}

database_api::~database_api() {}

database_api_impl::database_api_impl( graphene::chain::database& db , application& app )
:_db(db) ,_app(app)
{
   wlog("creating database api ${x}", ("x",int64_t(this)) );
   _new_connection = _db.new_objects.connect([this](const vector<object_id_type>& ids, const flat_set<account_id_type>& impacted_accounts) {
                                on_objects_new(ids, impacted_accounts);
                                });
   _change_connection = _db.changed_objects.connect([this](const vector<object_id_type>& ids, const flat_set<account_id_type>& impacted_accounts) {
                                on_objects_changed(ids, impacted_accounts);
                                });
   _removed_connection = _db.removed_objects.connect([this](const vector<object_id_type>& ids, const vector<const object*>& objs, const flat_set<account_id_type>& impacted_accounts) {
                                on_objects_removed(ids, objs, impacted_accounts);
                                });
   _applied_block_connection = _db.applied_block.connect([this](const signed_block&){ on_applied_block(); });

   _pending_trx_connection = _db.on_pending_transaction.connect([this](const signed_transaction& trx ){
                         if( _pending_trx_callback ) _pending_trx_callback( fc::variant(trx, GRAPHENE_MAX_NESTED_OBJECTS) );
                      });
}

database_api_impl::~database_api_impl()
{
   elog("freeing database api ${x}", ("x",int64_t(this)) );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Objects                                                          //
//                                                                  //
//////////////////////////////////////////////////////////////////////

uint32_t  database_api::get_budget_pre_block( fc::time_point_sec now)const
{
 return my->get_budget_pre_block( now );
}

uint32_t  database_api_impl::get_budget_pre_block( fc::time_point_sec now)const
{
     return _db.get_budget_pre_block(now);
}
fc::variants database_api::get_objects(const vector<object_id_type>& ids)const
{
   //idump((ids))
   return my->get_objects( ids );
}

fc::variants database_api_impl::get_objects(const vector<object_id_type>& ids)const
{ try {
   if( _subscribe_callback )  {
      for( auto id : ids )
      {
         if( id.type() == operation_history_object_type && id.space() == protocol_ids ) continue;
         if( (id.type() == impl_account_transaction_history_object_type   ||  id.type() ==impl_account_balance_history_object_type) && 
              id.space() == implementation_ids ) continue;

         this->subscribe_to_item( id );
      }
   }

   fc::variants result;
   result.reserve(ids.size());

   std::transform(ids.begin(), ids.end(), std::back_inserter(result),
                  [this](object_id_type id) -> fc::variant {
      if(auto obj = _db.find_object(id))
         return obj->to_variant();
      return {};
   });

   return result;
} FC_CAPTURE_AND_RETHROW( (ids) ) } 

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Subscriptions                                                    //
//                                                                  //
//////////////////////////////////////////////////////////////////////

void database_api::set_subscribe_callback( std::function<void(const variant&)> cb, bool notify_remove_create )
{
   my->set_subscribe_callback( cb, notify_remove_create );
}

void database_api_impl::set_subscribe_callback( std::function<void(const variant&)> cb, bool notify_remove_create )
{
   if( notify_remove_create )
   {
      FC_ASSERT(_db._app_status.enable_subscribe_to_all,
                 "Subscribing to universal object creation and removal is disallowed in this server." );
   }

   _subscribe_callback = cb;
   _notify_remove_create = notify_remove_create;
   _subscribed_accounts.clear();

   static fc::bloom_parameters param;
   param.projected_element_count    = 10000;
   param.false_positive_probability = 1.0/100;
   param.maximum_size = 1024*8*8*2;
   param.compute_optimal_parameters();
   _subscribe_filter = fc::bloom_filter(param);
}

void database_api::set_pending_transaction_callback( std::function<void(const variant&)> cb )
{
   my->set_pending_transaction_callback( cb );
}

void database_api_impl::set_pending_transaction_callback( std::function<void(const variant&)> cb )
{
   _pending_trx_callback = cb;
}

void database_api::set_block_applied_callback( std::function<void(const variant& block_id)> cb )
{
   my->set_block_applied_callback( cb );
}

void database_api_impl::set_block_applied_callback( std::function<void(const variant& block_id)> cb )
{
   _block_applied_callback = cb;
}

void database_api::cancel_all_subscriptions()
{
   my->cancel_all_subscriptions();
}

void database_api_impl::cancel_all_subscriptions()
{
   set_subscribe_callback( std::function<void(const fc::variant&)>(), true);
   _market_subscriptions.clear();
   _bitlender_subscriptions.clear();
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Blocks and transactions                                          //
//                                                                  //
//////////////////////////////////////////////////////////////////////

optional<block_header> database_api::get_block_header(uint64_t block_num)const
{
   return my->get_block_header( block_num );
}

optional<block_header> database_api_impl::get_block_header(uint64_t block_num) const
{
      try {
   if(block_num>_db.head_block_num())    
     return optional<block_header>();
   auto result = _db.fetch_block_by_number(block_num);
   if(result)
      return *result;
   return {};
      }catch(...)
      {
          return optional<block_header>();
      }
}
map<uint64_t, optional<block_header>> database_api::get_block_header_batch(const vector<uint64_t> block_nums)const
{
   return my->get_block_header_batch( block_nums );
}

map<uint64_t, optional<block_header>> database_api_impl::get_block_header_batch(const vector<uint64_t> block_nums) const
{
   map<uint64_t, optional<block_header>> results;
   for (const uint64_t block_num : block_nums)
   {
      results[block_num] = get_block_header(block_num);
   }
   return results;
}

optional<signed_block> database_api::get_block(uint64_t block_num)const
{
   return my->get_block( block_num );
}

optional<signed_block> database_api_impl::get_block(uint64_t block_num)const
{    
   try {
    if(block_num>_db.head_block_num())    
     return optional<signed_block>();
   return _db.fetch_block_by_number(block_num);
   } catch ( ... ) {
      return optional<signed_block>();
   }
}

optional<signed_block_info> database_api::get_block_ids(uint64_t block_num)const
{
   return my->get_block_ids( block_num );
}

struct memo_visitor
{
   typedef void result_type;
   template<typename W>
   void operator()( W& op )
   {       
      // if(op.which() ==  operation::tag<transfer_operation>::value) {
      //    // idump((op.from));
      //    // idump((op));
      // }
   } 
};
void decode_memo(transfer_operation &trans, application *pApp) {
  try {
   for(auto const &acc: pApp ->_memo_keys) {
      if(trans.to == acc.first) {          
         std::string msg = trans.memo->get_message(acc.second,trans.memo->from);
         trans.memo->set_message(msg);
         idump((msg)(*trans.memo));         
         break;         
      } else if(trans.from == acc.first) {          
         std::string msg = trans.memo->get_message(acc.second,trans.memo->to);
         trans.memo->set_message(msg);
         idump((msg)(*trans.memo));       
         break;         
      }
   }
   }catch(...) {      
     idump(("decode failed")(trans));
   }
}
optional<signed_block_info> database_api_impl::get_block_ids(uint64_t block_num)const
{
   try
   {
      if (block_num > _db.head_block_num())
         return optional<signed_block_info>();
      optional<signed_block> get = _db.fetch_block_by_number(block_num);
      if (get.valid())
      {
         signed_block_info getids;
         getids.version = get->version;
         getids.previous = get->previous;
         getids.timestamp = get->timestamp;
         getids.witness = get->witness;
         getids.transaction_merkle_root = get->transaction_merkle_root;
         getids.witness_signature = get->witness_signature;
         getids.height = block_num;
         getids.block_id = get->id();
         for (const auto &a : get->transactions)
         {
            getids.trxids.push_back(a.id());
            if(_app._memo_keys.size() >0) {
               processed_transaction add = a;
               for (auto &oper : add.operations) {                  
                  if(oper.which() ==  operation::tag<transfer_operation>::value) {
                     transfer_operation& op = oper.get<transfer_operation>();
                     if(op.memo.valid() && op.memo->nonce != unenc_nonce) {
                       decode_memo(op, &_app);                 
                     }
                  }
               }
               getids.transactions.push_back(add);
            } else {
              getids.transactions.push_back(a);
            }
         }
         return getids;
      } else
         return optional<signed_block_info>();
   }catch (...)
   {
      return optional<signed_block_info>();
   }
}


    


optional<signed_block_info> database_api::get_block_with_id(uint64_t block_num, transaction_id_type trx_id)const
{
   return my->get_block_with_id( block_num,trx_id );
}

optional<signed_block_info> database_api_impl::get_block_with_id(uint64_t block_num, transaction_id_type trx_id)const
{
   try
   {
      if (block_num <= 0)
      {
         const auto &idx = _db.get_index_type<operation_history_index>().indices().get<by_trxid>();
         auto itr = idx.lower_bound(trx_id);
         if (itr != idx.end())
            block_num = itr->block_num;
         else
            return optional<signed_block_info>();
      }

      if (block_num > _db.head_block_num())
         return optional<signed_block_info>();
      auto opt_block = _db.fetch_block_by_number(block_num);
      FC_ASSERT(opt_block);
      // signed_block_header reh = *opt_block;
      signed_block_info re;
      *(signed_block_header *)&re = *opt_block;
      re.block_id = opt_block->id();
      re.height = block_num;      
      for (const auto &a : opt_block->transactions)
      {
         if (a.id() == trx_id)
         {
            re.transactions.push_back(a);
            re.trxids.push_back(trx_id);
            break;
         }
      }
      return re;
   } catch ( ... ) {
      return optional<signed_block_info>();
   }
}

processed_transaction database_api::get_transaction( uint64_t block_num, uint32_t trx_in_block )const
{
   return my->get_transaction( block_num, trx_in_block );
}

optional<signed_transaction> database_api::get_recent_transaction_by_id( const transaction_id_type& id )const
{
   try {
      return my->_db.get_recent_transaction( id );
   } catch ( ... ) {
      return optional<signed_transaction>();
   }
}

processed_transaction database_api_impl::get_transaction(uint64_t block_num, uint32_t trx_num)const
{
    try
    {
   if(block_num>_db.head_block_num())    
     return {};
   auto opt_block = _db.fetch_block_by_number(block_num);
   FC_ASSERT( opt_block );
   FC_ASSERT( opt_block->transactions.size() > trx_num );
   return opt_block->transactions[trx_num];
    }
    catch(...)
    {
         return {};
    }
}

optional<processed_transaction>  database_api::get_transaction_by_id( uint64_t block_num, transaction_id_type trx_id)const
{
    return my->get_transaction_by_id( block_num, trx_id );
}

optional<processed_transaction>  database_api_impl::get_transaction_by_id( uint64_t block_num, transaction_id_type trx_id)const
{
   try
    {
       if(block_num <=0) {  
         const auto& idx = _db.get_index_type<operation_history_index>().indices().get<by_trxid>();
         auto itr = idx.lower_bound(trx_id);
         if(itr != idx.end()) block_num = itr->block_num;
         else return optional<processed_transaction>();
       }

      if(block_num>_db.head_block_num())    
         return optional<processed_transaction>();
      auto opt_block = _db.fetch_block_by_number(block_num);
      FC_ASSERT( opt_block );
      for(const auto &a : opt_block->transactions)
      {
         if(a.id() == trx_id)
            return a;
      }
      return optional<processed_transaction>();
    }
    catch(...)
    {
         return optional<processed_transaction>();
    }
}


  
//////////////////////////////////////////////////////////////////////
//                                                                  //
// Globals                                                          //
//                                                                  //
//////////////////////////////////////////////////////////////////////

chain_property_object database_api::get_chain_properties()const
{
   return my->get_chain_properties();
}

chain_property_object database_api_impl::get_chain_properties()const
{
   return _db.get(chain_property_id_type());
}

global_property_object database_api::get_global_properties()const
{
   return my->get_global_properties();
}
bitlender_paramers_object database_api::get_bitlender_paramers()const
{
   return my->get_bitlender_paramers(); 
}
bitlender_paramers_object_key database_api::get_bitlender_paramers_key(optional<bitlender_key> key)const
{
   return my->get_bitlender_paramers_key(key); 
}
finance_paramers_object database_api::get_finance_paramers()const
{
   return my->get_finance_paramers(); 
}


global_property_object database_api_impl::get_global_properties()const
{
   return _db.get(global_property_id_type());
}
bitlender_paramers_object database_api_impl::get_bitlender_paramers()const
{
    return _db.get(bitlender_paramers_id_type(0));
}

bitlender_paramers_object_key database_api_impl::get_bitlender_paramers_key(optional<bitlender_key> key)const
{ try{
    bitlender_paramers_object_key defaaa;
    const auto &obj = bitlender_paramers_id_type(0)(_db);
    defaaa.set_data(obj,key);
    return defaaa;
} FC_CAPTURE_AND_RETHROW( (key)) }

finance_paramers_object database_api_impl::get_finance_paramers()const
{
    return _db.get(finance_paramers_id_type(0));
}
fc::variant_object database_api::get_config()const
{
   return my->get_config();
}

fc::variant_object database_api_impl::get_config()const
{
   return graphene::chain::get_config();
}

chain_id_type database_api::get_chain_id()const
{
   return my->get_chain_id();
}

chain_id_type database_api_impl::get_chain_id()const
{
   return _db.get_chain_id();
}

dynamic_global_property_object database_api::get_dynamic_global_properties()const
{
   return my->get_dynamic_global_properties();
}

dynamic_global_property_object database_api_impl::get_dynamic_global_properties()const
{
   return _db.get(dynamic_global_property_id_type());
}

fc::string temp_to_string(double d)
{
    // +2 is required to ensure that the double is rounded correctly when read back in.  http://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html
    std::stringstream ss;
    ss << std::setprecision(2) << std::fixed << d;
    return ss.str();
}

variant  database_api::get_block_summary()
{
    uint64_t utrans_count = 0;
    uint64_t ublock_count = 0;
    uint64_t confirmed_time = 0;
    fc::time_point_sec time_start;
    fc::time_point_sec time_end;

    dynamic_global_property_object gb = my->get_dynamic_global_properties();
    time_end = block_summary_id_type(gb.head_block_number& 0xffff)(my->_db).block_time;
    for (int64_t i = gb.head_block_number; i > 0 && ublock_count < 30; i--)
    {
        try
        {
            const auto &tapos_block_summary = block_summary_id_type((i & 0xffff)) (my->_db);
            utrans_count += tapos_block_summary.trans_count;
            confirmed_time += tapos_block_summary.confrim_time;
            time_start = tapos_block_summary.block_time;
            ublock_count++;
        }
        catch (...)
        {
            continue;
        }
    }

    if(time_start == time_end)
        time_end = time_start + 1;

    fc::mutable_variant_object count;
    
    const auto &core_asset = GRAPHENE_CORE_ASSET(my->_db);
    const auto &core_dy    = core_asset.dynamic_data(my->_db);
    flat_set<asset_id_type> a;   
    vector<asset_summary> ass = get_account_balances(GRAPHENE_PLATFORM_ACCOUNT, a);
    share_type zos_all = 0;

    for(const auto &a : ass)
    {
        if(a.asset_id == GRAPHENE_CORE_ASSET)
           zos_all += a.amount;
        else
        {
            const auto &aget = a.asset_id(my->_db);
            asset aa;
            aa.asset_id = a.asset_id;
            aa.amount = a.amount;
            asset zosget = aget.to_core(my->_db, aa);
            zos_all += zosget.amount;
        }
    }

    count["current-block-num"]     = gb.head_block_number;
    count["current-block-time"]    = gb.time;
    count["recently_missed_count"] = gb.recently_missed_count;
    count["core-current-supply"]   = core_dy.current_supply.value;
    count["core-confidential-supply"] = core_dy.confidential_supply.value;
    count["core-fundraise-supply"]    = core_dy.fundraise_supply.value;
    count["core-accumulated-fees"]    = core_dy.accumulated_fees.value;
    count["miner-budget"]  = gb.miners_budget[miner_witness].value;
    count["miner-payed"]   = gb.miners_payed[miner_witness].value;
    count["worker-budget"] = gb.miners_budget[miner_worker].value;
    count["worker-payed"]  = gb.miners_payed[miner_worker].value;

    count["miner-supply"]     = gb.miner_supply.value;
    count["miner_fee_used"]   = gb.miner_fee_used.value;    
    count["budget-supply"]    = gb.budget_supply.value;
    count["budget_fees_used"] = gb.budget_fees_used.value;
    count["coupon-supply"]    = gb.coupon_supply.value;    
    count["platform-fee"]     = zos_all.value; 
    count["account-count"]    = my->get_object_count(account_id_type(0),false);

    count["trans-per-second"] = temp_to_string(utrans_count * 1.0 / (time_end - time_start).to_seconds());
    count["trans-per-block"]  = temp_to_string(utrans_count  *1.0/ ublock_count);    
    count["avg-confirm-tmie"] = temp_to_string(confirmed_time  *1.0/ (1000000 *ublock_count));    
    return count;
}
bool     database_api::is_plugin_enable(const std::string &plugin) const 
{
    try{
    std::shared_ptr<abstract_plugin> p = _app.get_plugin(plugin);
    return !p;
    }catch(...)
    {
      return false;
    }
}
variant  database_api::get_info() const 
{  
    try
    {
       fc::mutable_variant_object result ;//   
       _app.get_info(result);
       my->_db.getinfo(result);

       fc::mutable_variant_object count;
       count["account"]            = my->get_object_count(account_id_type(0),false);
       count["asset"]              = my->get_object_count(GRAPHENE_CORE_ASSET,false);
       count["committee_member"]   = my->get_object_count(committee_member_id_type(0),false);
       count["witness"]            = my->get_object_count(witness_id_type(0),false);
       count["worker"]             = my->get_object_count(worker_id_type(0),false);
       count["gateway"]            = my->get_object_count(gateway_id_type(0),false);
       count["carrier"]            = my->get_object_count(carrier_id_type(0),false);
       count["author"]            = my->get_object_count(author_id_type(0),false);
       count["budget_member"]      = my->get_object_count(budget_member_id_type(0),false);
       count["proposal"]           = my->get_object_count(proposal_id_type(0),false);

       result["object_count"] = count;     

       return result;
    }
    catch(...)
    {
        return {};
    }
}



//////////////////////////////////////////////////////////////////////
//                                                                  //
// Keys                                                             //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<vector<account_id_type>> database_api::get_key_references( vector<public_key_type> key )const
{
   return my->get_key_references( key );
}

/**
 *  @return all accounts that referr to the key or account id in their owner or active authorities.
 */
vector<vector<account_id_type>> database_api_impl::get_key_references( vector<public_key_type> keys )const
{
   vector< vector<account_id_type> > final_result;
   final_result.reserve(keys.size());

   for( auto& key : keys )
   {

      address a1( pts_address(key, false, 56) );
      address a2( pts_address(key, true, 56) );
      address a3( pts_address(key, false, 0)  );
      address a4( pts_address(key, true, 0)  );
      address a5( key );

      subscribe_to_item( key );
      subscribe_to_item( a1 );
      subscribe_to_item( a2 );
      subscribe_to_item( a3 );
      subscribe_to_item( a4 );
      subscribe_to_item( a5 );

      const auto& idx = _db.get_index_type<account_index>();
      const auto& aidx = dynamic_cast<const primary_index<account_index>&>(idx);
      const auto& refs = aidx.get_secondary_index<graphene::chain::account_member_index>();
      auto itr = refs.account_to_key_memberships.find(key);
      vector<account_id_type> result;

      for( auto& a : {a1,a2,a3,a4,a5} )
      {
          auto itr = refs.account_to_address_memberships.find(a);
          if( itr != refs.account_to_address_memberships.end() )
          {
             result.reserve( itr->second.size() );
             for( auto item : itr->second )
             {
                result.push_back(item);
             }
          }
      }

      if( itr != refs.account_to_key_memberships.end() )
      {
         result.reserve( itr->second.size() );
         for( auto item : itr->second ) result.push_back(item);
      }
      final_result.emplace_back( std::move(result) );
   }

   for( auto i : final_result )
      subscribe_to_item(i);

   return final_result;
}

bool database_api::is_public_key_registered(string public_key) const
{
    return my->is_public_key_registered(public_key);
}

bool database_api_impl::is_public_key_registered(string public_key) const
{
    // Short-circuit
    if (public_key.empty()) {
        return false;
    }

    // Search among all keys using an existing map of *current* account keys
    public_key_type key;
    try {
        key = public_key_type(public_key);
    } catch ( ... ) {
        // An invalid public key was detected
        return false;
    }
    const auto& idx = _db.get_index_type<account_index>();
    const auto& aidx = dynamic_cast<const primary_index<account_index>&>(idx);
    const auto& refs = aidx.get_secondary_index<graphene::chain::account_member_index>();
    auto itr = refs.account_to_key_memberships.find(key);
    bool is_known = itr != refs.account_to_key_memberships.end();

    return is_known;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Accounts                                                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<optional<account_object>> database_api::get_accounts(const vector<account_id_type>& account_ids)const
{
   return my->get_accounts( account_ids );
}

vector<optional<account_object>> database_api_impl::get_accounts(const vector<account_id_type>& account_ids)const
{    try{
   vector<optional<account_object>> result; result.reserve(account_ids.size());
   std::transform(account_ids.begin(), account_ids.end(), std::back_inserter(result),
                  [this](account_id_type id) -> optional<account_object> {
      if(auto o = _db.find(id))
      {
         subscribe_to_item( id );
         return *o;
      }
      return {};
   });
   return result;
} FC_CAPTURE_AND_RETHROW( (account_ids)) }

vector<optional<account_statistics_object>> database_api::get_accounts_statistics(const vector<account_id_type>& account_ids)const
{
   return my->get_accounts_statistics( account_ids );
}

vector<optional<account_statistics_object>> database_api_impl::get_accounts_statistics(const vector<account_id_type>& account_ids)const
{ try{
   vector<optional<account_statistics_object>> result;
   vector<optional<account_object>> acc = get_accounts(account_ids);
   for(const auto &a : acc)   
   {
      if(a.valid())
      {
         result.push_back(a->statistics(_db));
      }
      else
      {
         result.push_back(optional<account_statistics_object>());
      }
   }
   return result;
} FC_CAPTURE_AND_RETHROW( (account_ids)) }

std::map<string,full_account> database_api::get_full_accounts( const vector<string>& names_or_ids, bool subscribe )
{
   return my->get_full_accounts( names_or_ids, subscribe );
}

std::map<std::string, full_account> database_api_impl::get_full_accounts( const vector<std::string>& names_or_ids, bool subscribe)
{
   std::map<std::string, full_account> results;

   for (const std::string& account_name_or_id : names_or_ids)
   {
      const account_object* account = nullptr;
      if (std::isdigit(account_name_or_id[0]))
         account = _db.find(fc::variant(account_name_or_id, 1).as<account_id_type>(1));
      else
      {
         const auto& idx = _db.get_index_type<account_index>().indices().get<by_name>();
         auto itr = idx.find(account_name_or_id);
         if (itr != idx.end())
            account = &*itr;
      }
      if (account == nullptr)
         continue;

      if( subscribe )
      {
         if(_subscribed_accounts.size() < 100) {
            _subscribed_accounts.insert( account->get_id() );
            subscribe_to_item( account->id );
         }
      }

      full_account acnt;
      acnt.account = *account;
      acnt.statistics = account->statistics(_db);
      acnt.registrar_name = account->registrar(_db).name;
      acnt.referrer_name = account->referrer(_db).name;
      acnt.lifetime_referrer_name = account->lifetime_referrer(_db).name;
      acnt.votes = lookup_vote_ids( vector<vote_id_type>(account->options.votes.begin(),account->options.votes.end()) );

      if (account->cashback_vb)
      {
         acnt.cashback_balance = (*account->cashback_vb)(_db);
      }
      if (account->locktoken_vb)
      {
         acnt.locktoken_balance = (*account->locktoken_vb)(_db);
      }
      

      
      // Add the account's proposals
      vector<proposal_object> my_proposal = get_proposed_transactions(account->id);
      for( auto proposal : my_proposal )
        acnt.proposals.push_back(proposal);

   /*   const auto& proposal_idx = _db.get_index_type<proposal_index>();
      const auto& pidx = dynamic_cast<const primary_index<proposal_index>&>(proposal_idx);
      const auto& proposals_by_account = pidx.get_secondary_index<graphene::chain::required_approval_index>();
      auto  required_approvals_itr = proposals_by_account._account_to_proposals.find( account->id );
      if( required_approvals_itr != proposals_by_account._account_to_proposals.end() )
      {
         acnt.proposals.reserve( required_approvals_itr->second.size() );
         for( auto proposal_id : required_approvals_itr->second )
            acnt.proposals.push_back( proposal_id(_db) );
      }
    */
     
      // Add the account's balances
      auto balance_range = _db.get_index_type<account_balance_index>().indices().get<by_account_asset>().equal_range(boost::make_tuple(account->id));
      std::for_each(balance_range.first, balance_range.second,
                    [&acnt](const account_balance_object& balance) {
                       acnt.balances.emplace_back(balance);
                    });

      // Add the account's vesting balances
      auto vesting_range = _db.get_index_type<vesting_balance_index>().indices().get<by_account>().equal_range(account->id);
      std::for_each(vesting_range.first, vesting_range.second,
                    [&acnt](const vesting_balance_object& balance) {
                       acnt.vesting_balances.emplace_back(balance);
                    });

      // Add the account's orders
      auto order_range = _db.get_index_type<limit_order_index>().indices().get<by_account>().equal_range(account->id);
      std::for_each(order_range.first, order_range.second,
                    [&acnt] (const limit_order_object& order) {
                       acnt.limit_orders.emplace_back(order);
                    });
      auto call_range = _db.get_index_type<call_order_index>().indices().get<by_account>().equal_range(account->id);
      std::for_each(call_range.first, call_range.second,
                    [&acnt] (const call_order_object& call) {
                       acnt.call_orders.emplace_back(call);
                    });
      auto settle_range = _db.get_index_type<force_settlement_index>().indices().get<by_account>().equal_range(account->id);
      std::for_each(settle_range.first, settle_range.second,
                    [&acnt] (const force_settlement_object& settle) {
                       acnt.settle_orders.emplace_back(settle);
                    });

      // get assets issued by user
      auto asset_range = _db.get_index_type<asset_index>().indices().get<by_issuer>().equal_range(account->id);
      std::for_each(asset_range.first, asset_range.second,
                    [&acnt] (const asset_object& asset) {
                       acnt.assets.emplace_back(asset.id);
                    });

      // get withdraws permissions
      auto withdraw_range = _db.get_index_type<withdraw_permission_index>().indices().get<by_from>().equal_range(account->id);
      std::for_each(withdraw_range.first, withdraw_range.second,
                    [&acnt] (const withdraw_permission_object& withdraw) {
                       acnt.withdraws.emplace_back(withdraw);
                    });


      results[account_name_or_id] = acnt;
   }
   return results;
}

optional<account_object> database_api::get_account_by_name( string name )const
{
   return my->get_account_by_name( name );
}

optional<account_object> database_api_impl::get_account_by_name( string name )const
{
   const auto& idx = _db.get_index_type<account_index>().indices().get<by_name>();
   auto itr = idx.find(name);
   if (itr != idx.end())
      return *itr;
   return optional<account_object>();
}
vector<account_info> database_api::get_account_author( account_id_type account_id)const
{
   return my->get_account_author( account_id );
}

vector<account_info> database_api_impl::get_account_author( account_id_type account_id )const
{
   try{
      vector<account_info> info;
      const auto &acc = account_id(_db);
      if (!acc.auth_data.valid())
         return info;
      for(const auto &aid: acc.auth_data->auth_state)  
      {
         account_info add;
         const auto &a = aid.first(_db);
         a.get_info(add);
         if(a.uaccount_property & account_authenticator)
         {
            const auto &b = a.get_author_id(_db)(_db);
            add.info = b.url;
         }
         info.push_back(add);
      }
      return info;
   }
   FC_CAPTURE_AND_RETHROW((account_id))
}

vector<string> database_api::get_account_config( account_id_type account_id,vector<string> sindex )const
{
   return my->get_account_config( account_id,sindex );
}
vector<string>  database_api_impl::get_account_config( account_id_type account_id,vector<string> sindex )const
{ 
   try{
      vector<string>  re;
      const auto &idx = account_id(_db);
      if(sindex.size()<=0)
      {
         for (auto &a : idx.configs)
            re.push_back(a.first + ":" + a.second);
      }
      else
      {
         for (auto &a : sindex)
         {
            auto iter = idx.configs.find(a);
            if (iter != idx.configs.end())
               re.push_back(iter->second);
            else
               re.push_back("");
         }
      }
      return re;
} FC_CAPTURE_AND_RETHROW( (account_id)(sindex)) }
vector<operation_history_object>  database_api::get_account_history(account_id_type account,vector<int64_t> optypes, int64_t start, int64_t offset) const
{
    return my->get_account_history( account,optypes, start, offset);
}
vector<operation_history_object>  database_api_impl::get_account_history(account_id_type account,vector<int64_t> optypes, int64_t start, int64_t offset) const
{
   try{   
      vector<operation_history_object> result;
      const auto& hist_idx = _db.get_index_type<account_transaction_history_index>();
      const auto& idx = hist_idx.indices().get<by_seq>();
      const auto &statics = account(_db).statistics(_db);
      const account_transaction_history_object &node = statics.most_recent_op(_db);
      int64_t maxindex  = node.sequence;  
      if (start == -1)   start = maxindex;   
      else if (start <0) start = 0;
      if (start < statics.removed_ops) start = statics.removed_ops;
      if (offset >= 0)
      {
         auto start_itr = idx.lower_bound( boost::make_tuple( account, start) );
         auto end_itr = idx.end();
         while (start_itr != end_itr) {

            if(start_itr ->account != account || start_itr ->sequence < start) {
               start_itr++;
               continue;
            }
            if((int64_t)result.size() > offset ) {
               break;
            }
            if(optypes.size() >0 && std::find(optypes.begin(),optypes.end(),start_itr ->operation_type) == optypes.end()) {
               start_itr++;
               continue;
            }

            auto const &ops = start_itr->operation_id(_db);
            operation_history_object read = ops;
            read.op_in_history = start_itr->sequence;
            result.push_back(read);

            start_itr++;
         }
      }
      else {
         auto start_itr = idx.lower_bound(boost::make_tuple(account, start));
         auto end_itr = idx.begin();
         while (start_itr != end_itr)
         {
            if(start_itr ->account != account || start_itr ->sequence > start) {
               start_itr--;
               continue;
            }
            if((int64_t)result.size() > -offset ) {
               break;
            }            
           if(optypes.size() >0 && std::find(optypes.begin(),optypes.end(),start_itr ->operation_type) == optypes.end()) {
               start_itr--;
               continue;
            }
           
            auto const &ops = start_itr->operation_id(_db);
            operation_history_object read = ops;
            read.op_in_history = start_itr->sequence;
            result.push_back(read);

            start_itr--;
         }
        
      }
      return result;

} FC_CAPTURE_AND_RETHROW( (account)(optypes)(start)(offset)) }
vector<account_id_type> database_api::get_account_references( account_id_type account_id )const
{
   return my->get_account_references( account_id );
}

vector<account_id_type> database_api_impl::get_account_references( account_id_type account_id )const
{
   const auto& idx = _db.get_index_type<account_index>();
   const auto& aidx = dynamic_cast<const primary_index<account_index>&>(idx);
   const auto& refs = aidx.get_secondary_index<graphene::chain::account_member_index>();
   auto itr = refs.account_to_account_memberships.find(account_id);
   vector<account_id_type> result;

   if( itr != refs.account_to_account_memberships.end() )
   {
      result.reserve( itr->second.size() );
      for( auto item : itr->second ) result.push_back(item);
   }
   return result;
}

vector<optional<account_object>> database_api::lookup_account_names(const vector<string>& account_names)const
{
   return my->lookup_account_names( account_names );
}

vector<optional<account_object>> database_api_impl::lookup_account_names(const vector<string>& account_names)const
{
   const auto& accounts_by_name = _db.get_index_type<account_index>().indices().get<by_name>();
   vector<optional<account_object> > result;
   result.reserve(account_names.size());
   std::transform(account_names.begin(), account_names.end(), std::back_inserter(result),
                  [&accounts_by_name](const string& name) -> optional<account_object> {
      auto itr = accounts_by_name.find(name);
      return itr == accounts_by_name.end()? optional<account_object>() : *itr;
   });
   return result;
}

map<string,account_id_type> database_api::lookup_accounts(const string& lower_bound_name, uint32_t limit)const
{
   return my->lookup_accounts( lower_bound_name, limit );
}
map<string,account_id_type> database_api::lookup_accounts_by_type(const string& lower_bound_name, uint32_t utype, uint32_t limit)const
{
   return my->lookup_accounts_by_type( lower_bound_name,utype, limit );
}
map<string,account_id_type> database_api_impl::lookup_accounts_by_type(const string& lower_bound_name, uint32_t utype, uint32_t limit)const
{
    FC_ASSERT( limit <= 1000 );
   const auto& accounts_by_name = _db.get_index_type<account_index>().indices().get<by_name>();
   map<string,account_id_type> result;

   for( auto itr = accounts_by_name.lower_bound(lower_bound_name);limit-- && itr != accounts_by_name.end(); ++itr )
   {
      if((utype & account_type_lifemember) ==  account_type_lifemember && !itr ->is_lifetime_member())
          continue;
      if((utype & account_type_comparename) ==  account_type_comparename && itr ->name.find(lower_bound_name) == std::string::npos )
          continue;    
      result.insert(make_pair(itr->name, itr->get_id()));
      if( limit == 1 )
         subscribe_to_item( itr->get_id() );
   }
   return result;
}
vector<account_info>   database_api::lookup_accounts_by_property(uint32_t utype, uint32_t limit) const
{
     return my->lookup_accounts_by_property( utype, limit );
}
vector<account_info>   database_api_impl::lookup_accounts_by_property(uint32_t utype, uint32_t limit) const
{
   try
   {
      vector<account_info> re;
      FC_ASSERT(limit <= 1000);
      const auto &accounts_by_name = _db.get_index_type<account_index>().indices().get<by_id>();
      for (const auto &a : accounts_by_name)
      {
         if ((a.uaccount_property & utype) == utype)
         {
            account_info add;
            a.get_info(add);
            re.push_back(add);
            limit--;
         }
         if (limit <= 0)
            break;
      }
      return re;
   }
   FC_CAPTURE_AND_RETHROW((utype))
}
vector<carrier_id_type>  database_api::lookup_carrier(const string &url) const
{
    return my->lookup_carrier(url);
}

vector<carrier_id_type>    database_api_impl::lookup_carrier(const string& url)const
{    
  vector<carrier_id_type>  result;  
  const auto& accounts = _db.get_index_type<carrier_index>().indices().get<by_id>();
   for(const auto itr : accounts)
   {
       if(itr.url == url)
       {
         carrier_id_type add = itr.id;
         result.push_back(add);
       }
   }
   return result;
}

map<string,account_id_type> database_api_impl::lookup_accounts(const string& lower_bound_name, uint32_t limit)const
{
   FC_ASSERT( limit <= 1000 );
   const auto& accounts_by_name = _db.get_index_type<account_index>().indices().get<by_name>();
   map<string,account_id_type> result;

   for( auto itr = accounts_by_name.lower_bound(lower_bound_name);
        limit-- && itr != accounts_by_name.end();
        ++itr )
   {
      result.insert(make_pair(itr->name, itr->get_id()));
      if( limit == 1 )
         subscribe_to_item( itr->get_id() );
   }

   return result;
}

uint64_t database_api::get_account_count()const
{
   return my->get_account_count();
}

uint64_t database_api_impl::get_account_count()const
{
   return _db.get_index_type<account_index>().indices().size();
}

uint64_t  database_api::get_object_count(object_id_type id,bool bmaxid)const
{
   return    my->get_object_count(id,bmaxid);
}
uint64_t  database_api_impl::get_object_count(object_id_type id,bool bmaxid)const
{
  try {
    if (bmaxid) {
      return _db.get_index(id.space(),id.type()).get_next_id().instance();
    } else {
      if (id.space() == protocol_ids) {
        switch (id.type()) {
        case account_object_type:
          return _db.get_index_type<account_index>().indices().size();
        case asset_object_type:
          return _db.get_index_type<asset_index>().indices().size();
        case force_settlement_object_type:
          return _db.get_index_type<force_settlement_index>().indices().size();
        case committee_member_object_type:
          return _db.get_index_type<committee_member_index>().indices().size();
        case witness_object_type:
          return _db.get_index_type<witness_index>().indices().size();
        case limit_order_object_type:
          return _db.get_index_type<limit_order_index>().indices().size();
        case call_order_object_type:
          return _db.get_index_type<call_order_index>().indices().size();
        case proposal_object_type:
          return _db.get_index_type<proposal_index>().indices().size();
        case operation_history_object_type:
          return _db.get_index_type<operation_history_index>().indices().size();
        case withdraw_permission_object_type:
          return _db.get_index_type<withdraw_permission_index>().indices().size();
        case vesting_balance_object_type:
          return _db.get_index_type<vesting_balance_index>().indices().size();
        case worker_object_type:
          return _db.get_index_type<worker_index>().indices().size();
        case locktoken_object_type:
          return _db.get_index_type<locktoken_index>().indices().size();
        case locktoken_options_object_type:
          return _db.get_index_type<locktoken_option_index>().indices().size();
        case sell_exchange_object_type:
          return _db.get_index_type<sell_exchange_index>().indices().size();
        case buy_exchange_object_type:
          return _db.get_index_type<buy_exchange_index>().indices().size();
        case balance_object_type:
          return _db.get_index_type<balance_index>().indices().size();
        case bitlender_option_object_type:
          return _db.get_index_type<bitlender_option_index>().indices().size();        
        case bitlender_order_object_type:
          return _db.get_index_type<bitlender_order_index>().indices().size();
        case issue_fundraise_object_type:
          return _db.get_index_type<issue_fundraise_index>().indices().size();
          
        case buy_fundraise_object_type:
          return _db.get_index_type<buy_fundraise_index>().indices().size();
        case gateway_object_type:
          return _db.get_index_type<gateway_index>().indices().size();
        case carrier_object_type:
          return _db.get_index_type<carrier_index>().indices().size();
        case author_object_type:
          return _db.get_index_type<author_index>().indices().size();  
        case budget_member_object_type:
          return _db.get_index_type<budget_member_index>().indices().size();
        default:
          return 0;
        }
      } else if (id.space() == implementation_ids) {
        switch (id.type()) {
        case impl_global_property_object_type:
          return 0;
        case impl_dynamic_global_property_object_type:
          return 0;
        case impl_reserved0_object_type:
          return 0;
        case impl_asset_dynamic_data_type:
          return 0;
        case impl_asset_bitasset_data_type:
          return 0;
        case impl_account_balance_object_type:
          return 0;
        case impl_account_statistics_object_type:
          return 0;
        case impl_transaction_object_type:
          return 0;
        case impl_block_summary_object_type:
          return 0;
        case impl_account_transaction_history_object_type:
          return 0;
        case impl_account_balance_history_object_type:
           return _db.get_index_type<balance_history_index>().indices().size();
        case impl_blinded_balance_object_type:
          return 0;
        case impl_chain_property_object_type:
          return 0;
        case impl_witness_schedule_object_type:
          return 0;
        case impl_budget_record_object_type:
          return _db.get_index(id.space(),id.type()).get_next_id().instance();
        case impl_special_authority_object_type:
          return 0;
        case impl_buyback_object_type:
          return 0;
        case impl_fba_accumulator_object_type:
          return 0;
        case impl_collateral_bid_object_type:
          return 0;
        case impl_bitlender_invest_object_type:
          return _db.get_index_type<bitlender_invest_index>().indices().size();
        case impl_bitlender_paramers_object_type:
          return _db.get_index_type<bitlender_paramers_index>().indices().size();
        case impl_finance_paramers_object_type:
          return _db.get_index_type<finance_paramers_index>().indices().size();                               
        case impl_carrier_dynamic_object_type:
          return _db.get_index_type<carrier_dynamic_index>().indices().size();                               
        case impl_gateway_dynamic_object_type:
          return _db.get_index_type<gateway_dynamic_index>().indices().size();       
        case impl_locktoken_history_object_type:
          return _db.get_index_type<account_locktoken_his_index>().indices().size();                                         
        case  impl_asset_exchange_feed_type:
           return _db.get_index_type<asset_exchange_feed_index>().indices().size();
        default:
          return 0;
        }
      }
    }
  } catch (...) {
    return 0;
  }
  return 0;
}
//////////////////////////////////////////////////////////////////////
//                                                                  //
// Balances                                                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////
vector<account_balance_object_ex> database_api::get_sort_balances(asset_id_type ass_id, share_type amount,uint32_t limit )
{ try {
   vector<account_balance_object_ex> ret;
    uint32_t index = 0;  
    FC_ASSERT(limit <=1000);
   const auto& balance_i = my->_db.get_index_type<account_balance_index>().indices().get<by_balances>();   
   auto iter = balance_i.upper_bound(amount);
   iter--;
   while(iter != balance_i.begin() && index < limit)  {
      if(iter->asset_type == ass_id) {
         index++;        
         account_balance_object_ex objectex;
         const auto &ass = iter ->asset_type(my->_db);
         const auto &acc = iter ->owner(my->_db);
         objectex.name = acc.name;
         objectex.precision = ass.precision;
         objectex.symbol = ass.symbol;

         objectex.owner = iter ->owner;
         objectex.asset_type = iter ->asset_type;
         objectex.balance = iter ->balance;
         objectex.id = iter ->id;

         ret.push_back(objectex);
      }
      iter--;         
   } 
   return ret;
}FC_CAPTURE_AND_RETHROW((ass_id)(amount)(limit))}       

vector<asset_summary> database_api::get_account_balances(account_id_type id, const flat_set<asset_id_type>& assets)const
{
   return my->get_account_balances( id, assets );
}

vector<asset_summary> database_api_impl::get_account_balances(account_id_type acnt, const flat_set<asset_id_type>& assets)const
{   
   vector<asset> result;
   if (assets.empty())
   {
      // if the caller passes in an empty list of assets, return balances for all assets the account owns
      const account_balance_index& balance_index = _db.get_index_type<account_balance_index>();
      auto range = balance_index.indices().get<by_account_asset>().equal_range(boost::make_tuple(acnt));
      for (const account_balance_object& balance : boost::make_iterator_range(range.first, range.second))
         result.push_back(asset(balance.get_balance()));
   }
   else
   {
      result.reserve(assets.size());

      std::transform(assets.begin(), assets.end(), std::back_inserter(result),
                     [this, acnt](asset_id_type id) { return _db.get_balance(acnt, id); });
   }
   const auto &acc = acnt(_db).statistics(_db);

   vector<asset_summary> retu;
   for (const auto &a : result)
   {
      asset_summary add;
      auto _asset = a.asset_id(_db);
      add.precision = _asset.precision;
      add.symbol = _asset.symbol;
      add.real_symbol = _asset.real_asset;
      add.asset_id = a.asset_id;
      add.uasset_property = _asset.uasset_property;
      add.amount = a.amount;      
      add.identify_lock = 0;
      if(add.asset_id == GRAPHENE_CORE_ASSET)
         add.identify_lock = acc.lock_asset.amount;    
      retu.push_back(add);
   }
   return retu;
}

vector<asset_summary> database_api::get_named_account_balances(const std::string& name, const flat_set<asset_id_type>& assets)const
{
   return my->get_named_account_balances( name, assets );
}

vector<asset_summary> database_api_impl::get_named_account_balances(const std::string& name, const flat_set<asset_id_type>& assets) const
{
   const auto& accounts_by_name = _db.get_index_type<account_index>().indices().get<by_name>();
   auto itr = accounts_by_name.find(name);
   FC_ASSERT( itr != accounts_by_name.end() );
   return get_account_balances(itr->get_id(), assets);
}
vector<asset>  database_api::get_account_lock_balances(account_id_type id, uint32_t utype) const
{
   return my->get_account_lock_balances( id, utype );
}
    
 
vector<asset> database_api_impl::get_account_lock_balances(account_id_type id,uint32_t utype) const
{
   try
   {
      flat_map<asset_id_type,share_type> result;  
      //抵押
      if(utype & 0x00001)
      {
         const auto &loan_index = _db.get_index_type<bitlender_order_index>().indices().get<by_id>();
         auto itr_min = loan_index.begin();
         while (itr_min != loan_index.end())
         {
            if (itr_min->issuer == id)
            {
               if (!itr_min->is_principalfinish() && !itr_min->is_recyclefinish() && !itr_min->is_recyclefinishing())
               {
                  result[itr_min->lock_collateralize.asset_id] += itr_min->lock_collateralize.amount;
               }
            }
            itr_min++;
         }
      }
      //投资
      else if(utype & 0x00002)
      {
         const auto &invest_index = _db.get_index_type<bitlender_invest_index>().indices().get<by_id>();
         auto itr_min = invest_index.begin();
         while (itr_min != invest_index.end())
         {
            if (itr_min->issuer == id)
            {
               auto &order = itr_min->order_id(_db);
               if (!order.is_principalfinish() && !order.is_recyclefinish() && !order.is_recyclefinishing())
               {
                  result[itr_min->lock_invest.asset_id] += itr_min->lock_invest.amount;
               }
            }
            itr_min++;
         }
      }
      //定期
      else if(utype & 0x00004)
      {
         const auto &idx = _db.get_index_type<locktoken_index>().indices().get<by_id>();
         auto iter = idx.rbegin();
         while (iter != idx.rend())
         {
            if (iter->issuer == id && locktoken_fixed == iter->type)
            {
               result[iter->locked.asset_id] += iter->locked.amount;
            }              
            iter++;
         }
      }
      //活期
      else if(utype & 0x00008)
      {
         const auto &idx = _db.get_index_type<locktoken_index>().indices().get<by_id>();
         auto iter = idx.rbegin();
         while (iter != idx.rend())
         {
            if (iter->issuer == id && locktoken_dy == iter->type)
            {
               result[iter->locked.asset_id] += iter->locked.amount;
            }              
            iter++;
         }
      }
      //节点
      else if(utype & 0x00010)
      {
          const auto &idx = _db.get_index_type<locktoken_index>().indices().get<by_id>();
         auto iter = idx.rbegin();
         while (iter != idx.rend())
         {
            if (iter->issuer == id && locktoken_node == iter->type)
            {
               result[iter->locked.asset_id] += iter->locked.amount;
            }              
            iter++;
         }        
      }
      //冻结
      else if(utype & 0x00020)
      {
         const auto &invest_index = _db.get_index_type<vesting_balance_index>().indices().get<by_account>();
         auto iter = invest_index.begin();
         while (iter != invest_index.end())
         {
            if (iter->owner == id)
            {
               result[iter->balance.asset_id] += iter->balance.amount;               
            }
            iter++;
         }
      }
      vector<asset> retu;
      for (const auto &a : result)
      {
         asset add;         
         add.asset_id = a.first;
         add.amount   = a.second;
         retu.push_back(add);
      }
      return retu;
   }
   FC_CAPTURE_AND_RETHROW((id))
}

vector<asset_summary> database_api::get_account_balances_summary(account_id_type id,uint32_t utype)const
{  try
   { 
     return my->get_account_balances_summary( id ,utype);
   }
   FC_CAPTURE_AND_RETHROW( (id) )
}
vector<asset_summary> database_api_impl::get_account_balances_summary(account_id_type acnt,uint32_t utype)const
{     
   flat_set<asset_id_type> assets;   
   vector<asset_summary> aBalance = get_account_balances(acnt, assets);   
   //所有资产
   {
      bool zosFind = false;
      for (auto &a : aBalance)
      { 
         if(a.asset_id == GRAPHENE_CORE_ASSET) {
           zosFind = true;
           break;
         }
      }
      //如果没有缺省应该有ZOS
      if(!zosFind)
      {
        auto zos = GRAPHENE_CORE_ASSET(_db);
        asset_summary add;
        add.asset_id = zos.id;
        add.symbol = zos.symbol;
        add.amount = 0;
        add.precision = zos.precision;
        add.uasset_property = zos.uasset_property;
        aBalance.push_back(add);
      }
   }

   //抵押
   if(utype & 0x00001)
   {
      vector<asset> aLock = get_account_lock_balances(acnt,0x00001); 
      for (auto &a : aBalance)
      { 
         for (auto &aL : aLock)
         {
           if(a.asset_id == aL.asset_id)
           {
            a.lending_lock = aL.amount;
           }
         }
      }
   }
   //投资
   if(utype & 0x00002)
   {
      vector<asset> aLock = get_account_lock_balances(acnt,0x00002); 
      for (auto &a : aBalance)
      { 
         for (auto &aL : aLock)
         {
           if(a.asset_id == aL.asset_id)
           {
            a.invest_lock = aL.amount;
           }
         }
      }
   }
   //定期
   if(utype & 0x00004)
   {
      vector<asset> aLock = get_account_lock_balances(acnt,0x00004); 
      for (auto &a : aBalance)
      { 
         for (auto &aL : aLock)
         {
           if(a.asset_id == aL.asset_id)
           {
            a.fixed_lock = aL.amount;
           }
         }
      }
   }        

   //活期
   if(utype & 0x00008)
   {
      vector<asset> aLock = get_account_lock_balances(acnt,0x00008); 
      for (auto &a : aBalance)
      { 
         for (auto &aL : aLock)
         {
           if(a.asset_id == aL.asset_id)
           {
            a.dy_lock = aL.amount;
           }
         }
      }
   }
   //节点
   if(utype & 0x00010)
   {
      vector<asset> aLock = get_account_lock_balances(acnt,0x00010); 
      for (auto &a : aBalance)
      {    
         for (auto &aL : aLock)
         {
           if(a.asset_id == aL.asset_id)
           {
            a.node_lock = aL.amount;
           }
         }
      }
   }
   //冻结
   if(utype & 0x00020)
   {        
      vector<asset> aLock = get_account_lock_balances(acnt,0x00020); 
      for (auto &a : aBalance)
      { 
         for (auto &aL : aLock)
         {
           if(a.asset_id == aL.asset_id)
           {
            a.vesting_lock = aL.amount;
           }
         }
      }      
   }  
   
   if(utype & 0x00040)
   {
      auto zos = GRAPHENE_CORE_ASSET(_db);
      const auto &acc = acnt(_db).statistics(_db);
      for (auto &a : aBalance)
      {           
         if(a.asset_id == GRAPHENE_CORE_ASSET)
         {
           a.coupon = acc.amount_coupon;
           a.identify_lock= acc.lock_asset.amount;
           break;
         }
      }           
   }
   if(utype & 0x00080)
   {
      for (auto &a : aBalance)
      {  
        auto zos = a.asset_id(_db);
        a.asset_id = zos.id;
        a.symbol = zos.symbol;
        a.precision = zos.precision;
        a.uasset_property = zos.uasset_property;
        a.all_amount = a.amount + a.lending_lock + a.invest_lock + a.dy_lock + a.fixed_lock + a.node_lock + a.vesting_lock + a.identify_lock;
      }
   }
   return aBalance;
}
 

vector<balance_object> database_api::get_balance_objects( const vector<address>& addrs )const
{
   return my->get_balance_objects( addrs );
}
 
fc::optional<bitlender_option_object>  database_api::get_bitlender_option(asset_id_type asset_id)const
{
    return my->get_bitlender_option( asset_id );
}

fc::optional<bitlender_option_object> database_api_impl::get_bitlender_option(asset_id_type asset_id)const
{    
   try
   {    
      if (asset_id.get_instance() == 0)    
      {
         bitlender_option_object re;
         re.sproduct = lender_desc;
         return re;
      }
      else 
      {
         auto& bitlender_options = _db.get_index_type<bitlender_option_index>().indices().get<by_id>();       
         for( const bitlender_option_object& option : bitlender_options)
         {      
          if(option.asset_id == asset_id && option.sproduct == lender_desc)
              return option;
          }        
          return fc::optional<bitlender_option_object>(); 
      }
    }
   FC_CAPTURE_AND_RETHROW( (asset_id) )
}

 

fc::optional<bitlender_option_object_key>  database_api::get_bitlender_option_key(asset_id_type asset_id,optional<bitlender_key> key)const
{
    return my->get_bitlender_option_key( asset_id,key );
}
fc::optional<bitlender_option_object_key> database_api_impl::get_bitlender_option_key(asset_id_type asset_id,optional<bitlender_key> key) const
{    
   try
   {
      if (asset_id.get_instance() == 0)    
      {
         bitlender_option_object_key re;
         re.sproduct = lender_desc;
         return re;
      }
      else
      {
        fc::optional<bitlender_option_object> obj = get_bitlender_option(asset_id);
        if(obj.valid())
        {
          bitlender_option_object_key re;
          re.set_data(*obj,key);
          return re;
        }
        else 
          return fc::optional<bitlender_option_object_key>(); 
      }
   }
   FC_CAPTURE_AND_RETHROW( (asset_id)(key) )
}

std::string   database_api::dec_message(string prikey_wifi, public_key_type pubkey,uint64_t nonce,vector<char> message) {
   try {
      fc::optional<fc::ecc::private_key> optional_private_key = graphene::utilities::wif_to_key(prikey_wifi);
      if (!optional_private_key)
         FC_THROW("Invalid private key");     
      memo_data memo;
      memo. nonce = nonce;
      memo. message = message;
      memo. from = optional_private_key->get_public_key();
      memo.to = pubkey;
      idump((memo));
      if(memo.nonce == unenc_nonce) return memo.get_message();
      else return memo.get_message(*optional_private_key,pubkey);
   }
   FC_CAPTURE_AND_RETHROW( (prikey_wifi)(pubkey)(nonce)(message) )
}
memo_data   database_api::enc_message(string prikey_wifi, public_key_type pubkey,  uint64_t nonce, std::string message){
   try {
      fc::optional<fc::ecc::private_key> optional_private_key = graphene::utilities::wif_to_key(prikey_wifi);
      if (!optional_private_key)
         FC_THROW("Invalid private key");

      memo_data data;
      data.from = optional_private_key->get_public_key();
      data.to = pubkey;
      data.set_message(*optional_private_key, pubkey,message,nonce);
      return data;
   }
   FC_CAPTURE_AND_RETHROW( (prikey_wifi)(pubkey)(nonce)(message) )
}
fc::optional<asset_exchange_feed_object>  database_api::get_asset_exchange_feed(asset_id_type asset_base,asset_id_type asset_quote,uint32_t utype) const
{
   return my->get_asset_exchange_feed( asset_base, asset_quote,utype);
}
fc::optional<asset_exchange_feed_object> database_api_impl::get_asset_exchange_feed(asset_id_type asset_base,asset_id_type asset_quote,uint32_t utype)const
{
   try
   {
      if (utype == ASSET_FEED_ASSET)
      {
         auto &a_base = asset_base(_db);
         auto &a_quote = asset_quote(_db);
         price p_base, p_quote;
         if (a_base.is_bitasset_issued()) {
            auto &bitasset = a_base.bitasset_data(_db);
            p_base = bitasset.current_feed.settlement_price;
            if(p_base.is_null())  return fc::optional<asset_exchange_feed_object>();
         } else p_base = a_base.options.core_exchange_rate;

         if (a_quote.is_bitasset_issued()) {
            auto &bitasset = a_quote.bitasset_data(_db);
            p_quote = bitasset.current_feed.settlement_price;
            if(p_quote.is_null())  return fc::optional<asset_exchange_feed_object>();
         } else p_quote = a_quote.options.core_exchange_rate;
         if (p_quote.is_for(GRAPHENE_CORE_ASSET) && p_base.is_for(GRAPHENE_CORE_ASSET))
         {
            asset_exchange_feed_object re;
            re.current_feed.settlement_price = p_base * p_quote;
            return re;
         } else  return fc::optional<asset_exchange_feed_object>();

      }
      else if (utype == ASSET_FEED_LENDER)
      {
         auto &feed_options = _db.get_index_type<asset_exchange_feed_index>().indices().get<by_feeds>();
         const auto itr = feed_options.find(boost::make_tuple(asset_base, asset_quote, utype));
         if (itr != feed_options.end())
            return *itr;
         else
            return fc::optional<asset_exchange_feed_object>();
      }
      else
      {
         FC_ASSERT("error type");
         return fc::optional<asset_exchange_feed_object>();
      }
   }
   FC_CAPTURE_AND_RETHROW( (asset_base)(asset_quote)(utype) )
}

fc::optional<issue_fundraise_object>  database_api::get_issue_fundraise(asset_id_type asset_id,asset_id_type pay_id)const
{
    return my->get_issue_fundraise( asset_id ,pay_id);
}

fc::optional<issue_fundraise_object> database_api_impl::get_issue_fundraise(asset_id_type asset_id,asset_id_type pay_id)const
{    
   try
   {
      auto& bitlender_options = _db.get_index_type<issue_fundraise_index>().indices().get<by_id>();
       
      for( const issue_fundraise_object& option : bitlender_options)
      {      
          if(option.issue_id == asset_id && option.buy_id == pay_id)
              return option;
      }   
      return  {};      
    }
   FC_CAPTURE_AND_RETHROW( (asset_id) )
}
 


vector<balance_object> database_api_impl::get_balance_objects( const vector<address>& addrs )const
{
   try
   {
      const auto& bal_idx = _db.get_index_type<balance_index>();
      const auto& by_owner_idx = bal_idx.indices().get<by_owner>();

      vector<balance_object> result;

      for( const auto& owner : addrs )
      {
         subscribe_to_item( owner );
         auto itr = by_owner_idx.lower_bound( boost::make_tuple( owner, GRAPHENE_CORE_ASSET ) );
         while( itr != by_owner_idx.end() && itr->owner == owner )
         {
            result.push_back( *itr );
            ++itr;
         }
      }
      return result;
   }
   FC_CAPTURE_AND_RETHROW( (addrs) )
}

vector<asset> database_api::get_vested_balances( const vector<balance_id_type>& objs )const
{
   return my->get_vested_balances( objs );
}

vector<asset> database_api_impl::get_vested_balances( const vector<balance_id_type>& objs )const
{
   try
   {
      vector<asset> result;
      result.reserve( objs.size() );
      auto now = _db.head_block_time();
      for( auto obj : objs )
         result.push_back( obj(_db).available( now ) );
      return result;
   } FC_CAPTURE_AND_RETHROW( (objs) )
}

vector<vesting_balance_object> database_api::get_vesting_balances( account_id_type account_id )const
{
   return my->get_vesting_balances( account_id );
}

vector<vesting_balance_object> database_api_impl::get_vesting_balances( account_id_type account_id )const
{
   try
   {
      vector<vesting_balance_object> result;
      auto vesting_range = _db.get_index_type<vesting_balance_index>().indices().get<by_account>().equal_range(account_id);
      std::for_each(vesting_range.first, vesting_range.second,
                    [&result](const vesting_balance_object& balance) {
                       result.emplace_back(balance);
                    });
      return result;
   }
   FC_CAPTURE_AND_RETHROW( (account_id) );
}

vector<vesting_balance_data> database_api::get_vesting_balances_data( account_id_type account_id ) const
{
     return my->get_vesting_balances_data( account_id );
}
vector<vesting_balance_data> database_api_impl::get_vesting_balances_data( account_id_type account_id )const
{
   try
   {
      vector<vesting_balance_data> result;
      auto now = _db.head_block_time();
      const auto& vesting_range = _db.get_index_type<vesting_balance_index>().indices().get<by_account>();
      auto itr = vesting_range.begin();
      while(itr != vesting_range.end())
      {
         if(itr ->owner == account_id && itr->balance.amount >0)
         {
           vesting_balance_data add;
           itr->balance.asset_id(_db).get_info(add.assetinfo);
           add.balance = itr->balance;
           add.allow = itr->get_allowed_withdraw(now);
           add.id = itr->id;
           add.owner = itr->owner;
           result.push_back(add);
         }
         itr++;
      }
      return result;
   }
   FC_CAPTURE_AND_RETHROW( (account_id) );
}
//////////////////////////////////////////////////////////////////////
//                                                                  //
// Assets                                                           //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<optional<asset_object>> database_api::get_assets(const vector<asset_id_type>& asset_ids)const
{
   return my->get_assets( asset_ids );
}

vector<optional<asset_object>> database_api_impl::get_assets(const vector<asset_id_type>& asset_ids)const
{
   vector<optional<asset_object>> result; result.reserve(asset_ids.size());
   std::transform(asset_ids.begin(), asset_ids.end(), std::back_inserter(result),
                  [this](asset_id_type id) -> optional<asset_object> {
      if(auto o = _db.find(id))
      {
         subscribe_to_item( id );
         return *o;
      }
      return {};
   });
   return result;
}

vector<asset_object> database_api::list_assets(const string& lower_bound_symbol, uint32_t limit)const
{
   return my->list_assets( lower_bound_symbol, limit );
}

vector<asset_object> database_api_impl::list_assets(const string& lower_bound_symbol, uint32_t limit)const
{
   FC_ASSERT( limit <= 100 );
   const auto& assets_by_symbol = _db.get_index_type<asset_index>().indices().get<by_symbol>();
   vector<asset_object> result;
   result.reserve(limit);

   auto itr = assets_by_symbol.lower_bound(lower_bound_symbol);

   if( lower_bound_symbol == "" )
      itr = assets_by_symbol.begin();

   while(limit-- && itr != assets_by_symbol.end())
      result.emplace_back(*itr++);

   return result;
}

vector<optional<asset_object>> database_api::lookup_asset_symbols(const vector<string>& symbols_or_ids)const
{
   return my->lookup_asset_symbols( symbols_or_ids );
}

vector<optional<asset_object>> database_api_impl::lookup_asset_symbols(const vector<string>& symbols_or_ids)const
{
   const auto& assets_by_symbol = _db.get_index_type<asset_index>().indices().get<by_symbol>();
   vector<optional<asset_object> > result;
   result.reserve(symbols_or_ids.size());
   std::transform(symbols_or_ids.begin(), symbols_or_ids.end(), std::back_inserter(result),
                  [this, &assets_by_symbol](const string& symbol_or_id) -> optional<asset_object> {
      if( !symbol_or_id.empty() && std::isdigit(symbol_or_id[0]) )
      {
         auto ptr = _db.find(variant(symbol_or_id, 1).as<asset_id_type>(1));
         return ptr == nullptr? optional<asset_object>() : *ptr;
      }
      auto itr = assets_by_symbol.find(symbol_or_id);
      return itr == assets_by_symbol.end()? optional<asset_object>() : *itr;
   });
   return result;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Markets / feeds                                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<limit_order_object> database_api::get_limit_orders(asset_id_type a, asset_id_type b, uint32_t limit)const
{
   return my->get_limit_orders( a, b, limit );
}

/**
 *  @return the limit orders for both sides of the book for the two assets specified up to limit number on each side.
 */
vector<limit_order_object> database_api_impl::get_limit_orders(asset_id_type a, asset_id_type b, uint32_t limit)const
{
   const auto& limit_order_idx = _db.get_index_type<limit_order_index>();
   const auto& limit_price_idx = limit_order_idx.indices().get<by_price>();

   vector<limit_order_object> result;

   uint32_t count = 0;
   auto limit_itr = limit_price_idx.lower_bound(price::max(a,b));
   auto limit_end = limit_price_idx.upper_bound(price::min(a,b));
   while(limit_itr != limit_end && count < limit)
   {
      result.push_back(*limit_itr);
      ++limit_itr;
      ++count;
   }
   count = 0;
   limit_itr = limit_price_idx.lower_bound(price::max(b,a));
   limit_end = limit_price_idx.upper_bound(price::min(b,a));
   while(limit_itr != limit_end && count < limit)
   {
      result.push_back(*limit_itr);
      ++limit_itr;
      ++count;
   }

   return result;
}

vector<object_id_type> database_api::get_account_limit_orders(account_id_type acc, asset_id_type a, asset_id_type b)const
{
   return my->get_account_limit_orders(acc, a, b);
}

/**
 *  @return the limit orders for both sides of the book for the two assets specified up to limit number on each side.
 */
vector<object_id_type> database_api_impl::get_account_limit_orders(account_id_type acc, asset_id_type a, asset_id_type b)const
{
   vector<object_id_type> result;
   const auto &limit_order_idx = _db.get_index_type<limit_order_index>().indices().get<by_id>();;
   auto limit_itr = limit_order_idx.rbegin();   
   while(limit_itr != limit_order_idx.rend())
   {
      if (acc == limit_itr ->seller &&  ((limit_itr->sell_price.base.asset_id == a && limit_itr->sell_price.quote.asset_id == b)  ||  
         (limit_itr->sell_price.base.asset_id == b && limit_itr->sell_price.quote.asset_id == a)))
         result.push_back(limit_itr->id);
      ++limit_itr;      
   }
   return result;
}

vector<call_order_object> database_api::get_call_orders(asset_id_type a, uint32_t limit)const
{
   return my->get_call_orders( a, limit );
}

vector<call_order_object> database_api_impl::get_call_orders(asset_id_type a, uint32_t limit)const
{
   const auto& call_index = _db.get_index_type<call_order_index>().indices().get<by_price>();
   const asset_object& mia = _db.get(a);
   price index_price = price::min(mia.bitasset_data(_db).options.short_backing_asset, mia.get_id());
   
   vector< call_order_object> result;
   auto itr_min = call_index.lower_bound(index_price.min());
   auto itr_max = call_index.lower_bound(index_price.max());
   while( itr_min != itr_max && result.size() < limit ) 
   {
      result.emplace_back(*itr_min);
      ++itr_min;
   }
   return result;
}

vector<force_settlement_object> database_api::get_settle_orders(asset_id_type a, uint32_t limit)const
{
   return my->get_settle_orders( a, limit );
}

vector<force_settlement_object> database_api_impl::get_settle_orders(asset_id_type a, uint32_t limit)const
{
   const auto& settle_index = _db.get_index_type<force_settlement_index>().indices().get<by_expiration>();
   const asset_object& mia = _db.get(a);

   vector<force_settlement_object> result;
   auto itr_min = settle_index.lower_bound(mia.get_id());
   auto itr_max = settle_index.upper_bound(mia.get_id());
   while( itr_min != itr_max && result.size() < limit )
   {
      result.emplace_back(*itr_min);
      ++itr_min;
   }
   return result;
}
bool is_account(const vector<account_id_type> &account,account_id_type id)
{
    if(account.size()==0)
        return true;
    for (auto it = account.begin(); it != account.end(); it++)
    {
      if(*it == id)
        return true;
    }
    return false;
}
bool is_asset(const vector<asset_id_type> &account,asset_id_type id)
{
    if(account.size()==0)
        return true;
    for (auto it = account.begin(); it != account.end(); it++)
    {
      if(*it == id)
        return true;
    }
    return false;
}
bool is_status(const vector<uint32_t> &account,uint32_t id)
{
    if(account.size()==0)
        return true;
    for (auto it = account.begin(); it != account.end(); it++)
    {
      if(*it == id)
        return true;
    }
    return false;
}
vector<bitlender_order_info> database_api::get_loan_orders(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const vector<uint32_t> &status, uint32_t start,uint32_t limit) const 
{
  return my->get_loan_orders(account, ass,status, start, limit);
}
vector<bitlender_order_info> database_api_impl::get_loan_orders(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const  vector<uint32_t> &status,uint32_t start,uint32_t limit) const 
{    
        vector<bitlender_order_info> result;
        const auto &settle_index = _db.get_index_type<bitlender_order_index>().indices().get<by_time>();
        auto itr_min = settle_index.rbegin();
        uint32_t iIndex = 0;
        while (itr_min != settle_index.rend() && result.size() < limit)
        {
            if (!is_account(account, itr_min->issuer) )
            {
                ++itr_min;
                continue;
            }
            if (!is_asset(ass, itr_min->amount_to_loan.asset_id) )
            {
                ++itr_min;
                continue;
            }            
            if (!is_status(status, itr_min->order_state))
            {
                ++itr_min;
                continue;
            }             
            if (iIndex < start)
            {
                iIndex++;
                ++itr_min;
                continue;
            }
            bitlender_order_info info;
            itr_min->get_info(_db, info);
            result.push_back(info);
            ++itr_min;
        }
        return result;
    
}
vector<bitlender_order_info> database_api::get_account_notify_orders(const account_id_type& id,uint32_t status, uint32_t start, uint32_t limit) const
{
    return my->get_notify_orders(id,status, start, limit);
}
vector<bitlender_order_info> database_api::get_notify_orders(uint32_t status, uint32_t start,uint32_t limit) const 
{
  account_id_type id;
  return my->get_notify_orders(id,status, start, limit);
}
vector<bitlender_order_info> database_api_impl::get_notify_orders(const account_id_type& id,uint32_t status, uint32_t start,uint32_t limit) const 
{
    if (status ==  ORDER_NOTIFY_SQUEEZE)
    {
        vector<bitlender_order_info> result;
        uint32_t iIndex = 0;
        vector<bitlender_order_id_type> &_squeeze_bitlender = _db.get_squeeze_bitlender();
        for (auto &a : _squeeze_bitlender)
        {
            if (iIndex < start)
            {
               iIndex++;
               continue;
            }           
            auto &_object = a(_db);
            bitlender_order_info info;
            _object.get_info(_db, info);
            result.push_back(info);
            if(result.size() >= limit)
                break;
        }
        return result;
    }
    else if (status ==  ORDER_NOTIFY_ING)
    {
        vector<bitlender_order_info> result;
        const auto &settle_index = _db.get_index_type<bitlender_order_index>().indices().get<by_time>();
        auto itr_min = settle_index.rbegin();
        uint32_t iIndex = 0;
        
        while (itr_min != settle_index.rend() && result.size() < limit)
        {                       
            if (!itr_min->is_notify(_db))
            {
                ++itr_min;
                continue;
            }
            if (iIndex < start)
            {
                iIndex++;
                ++itr_min;
                continue;
            }
            bitlender_order_info info;
            itr_min->get_info(_db, info);
            result.push_back(info);
            ++itr_min;
        }
        return result;
    }

    else if (status ==  ORDER_NOTIFY_NEEDCOLLATE)
    {
        vector<bitlender_order_info> result;
        const auto &settle_index = _db.get_index_type<bitlender_order_index>().indices().get<by_time>();
        auto itr_min = settle_index.rbegin();
        uint32_t iIndex = 0;
        
        while (itr_min != settle_index.rend() && result.size() < limit)
        {                       
            if(id != account_id_type() && id != itr_min->issuer)
            {
               ++itr_min;
               continue;
            }
            if (!itr_min->is_needaddcoll(_db))
            {
                ++itr_min;
                continue;
            }
            if (iIndex < start)
            {
                iIndex++;
                ++itr_min;
                continue;
            }
            bitlender_order_info info;
            itr_min->get_info(_db, info);
            result.push_back(info);
            ++itr_min;
        }
        return result;
    }

    else
    {
       vector<bitlender_order_info> result;
        const auto &settle_index = _db.get_index_type<bitlender_order_index>().indices().get<by_time>();
        auto itr_min = settle_index.rbegin();
        uint32_t iIndex = 0;
        while (itr_min != settle_index.rend() && result.size() < limit)
        {               
            if (status != itr_min->order_state)
            {
                ++itr_min;
                continue;
            }             
            if (iIndex < start)
            {
                iIndex++;
                ++itr_min;
                continue;
            }           
             bitlender_order_info info;
             itr_min->get_info(_db, info);
             result.push_back(info);
             ++itr_min;
        }
        return result;
    }
}
uint32_t database_api::get_loan_counts(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const vector<uint32_t> &status)const
{
 return my->get_loan_counts(account, ass,status);
}
uint32_t  database_api_impl::get_loan_counts(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const vector<uint32_t> &status)const
{    
    const auto &settle_index = _db.get_index_type<bitlender_order_index>().indices().get<by_time>();
    auto itr_min = settle_index.begin();
    uint32_t iIndex = 0;
    while (itr_min != settle_index.end())
    {
        if(!is_account(account,itr_min->issuer))
        {
            ++itr_min;
            continue;
        }
        if (!is_asset(ass, itr_min->amount_to_loan.asset_id) )
        {
            ++itr_min;
            continue;
        }   
        if(!is_status(status,itr_min->order_state))
        {
            ++itr_min;
            continue;
        }
        iIndex++;
        ++itr_min;
    }
    return iIndex;
}
vector<bitlender_invest_info> database_api::get_invest_orders(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const  vector<uint32_t> &status, uint32_t start, uint32_t limit) const 
{
  return my->get_invest_orders(account,ass, status, start, limit);
}
vector<bitlender_invest_info> database_api_impl::get_invest_orders(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const  vector<uint32_t> &status, uint32_t start,uint32_t limit) const 
{
   vector<bitlender_invest_info> result;
   const auto &settle_index = _db.get_index_type<bitlender_invest_index>().indices().get<by_time>();
    auto itr_min = settle_index.rbegin();
    uint32_t iIndex = 0;
    while (itr_min != settle_index.rend() && result.size() < limit)
    {
        if(!is_account(account,itr_min->issuer))
        {
            ++itr_min;
            continue;
        }
        if(!is_asset(ass, itr_min->amount_to_loan.asset_id) )
        {
            ++itr_min;
            continue;
        }   
        auto &order = itr_min->order_id(_db);
        if (!is_status(status, order.order_state))
        {
            ++itr_min;
            continue;
        }
        if(iIndex<start)
        {
            iIndex++;
            ++itr_min;
            continue;
        }
        bitlender_invest_info info;
        itr_min->get_info(_db, info);
        result.push_back(info);
        ++itr_min;  
    }
    return result;
}

uint32_t database_api::get_invest_counts(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const vector<uint32_t> &status)const
{
    return my->get_invest_counts(account,ass, status);
}
uint32_t  database_api_impl::get_invest_counts(const vector<account_id_type> &account,const vector<asset_id_type> &ass,const vector<uint32_t> &status)const
{     
    const auto &settle_index = _db.get_index_type<bitlender_invest_index>().indices().get<by_time>();
    auto itr_min = settle_index.begin();
    uint32_t iIndex = 0;
    while (itr_min != settle_index.end())
    {
        if(!is_account(account,itr_min->issuer))
        {
            ++itr_min;
            continue;
        }
        if (!is_asset(ass, itr_min->amount_to_loan.asset_id) )
        {
            ++itr_min;
            continue;
        }   
        auto &order = itr_min->order_id(_db);
        if (!is_status(status, order.order_state))
        {
            ++itr_min;
            continue;
        }
        ++itr_min;
        iIndex++;
    }
    return iIndex;
}

vector<call_order_object> database_api::get_margin_positions(const account_id_type &id) const 
{
  return my->get_margin_positions(id);
}

vector<call_order_object> database_api_impl::get_margin_positions( const account_id_type& id )const
{
   try
   {
      const auto& idx = _db.get_index_type<call_order_index>();
      const auto& aidx = idx.indices().get<by_account>();
      auto start = aidx.lower_bound( boost::make_tuple( id, GRAPHENE_CORE_ASSET ) );
      auto end = aidx.lower_bound( boost::make_tuple( id+1, GRAPHENE_CORE_ASSET ) );
      vector<call_order_object> result;
      while( start != end )
      {
         result.push_back(*start);
         ++start;
      }
      return result;
   } FC_CAPTURE_AND_RETHROW( (id) )
}

vector<collateral_bid_object> database_api::get_collateral_bids(const asset_id_type asset, uint32_t limit, uint32_t start)const
{
   return my->get_collateral_bids( asset, limit, start );
}

vector<collateral_bid_object> database_api_impl::get_collateral_bids(const asset_id_type asset_id, uint32_t limit, uint32_t skip)const
{ try {
   FC_ASSERT( limit <= 100 );
   const asset_object& swan = asset_id(_db);
   FC_ASSERT( swan.is_market_issued() );
   const asset_bitasset_data_object& bad = swan.bitasset_data(_db);
   const asset_object& back = bad.options.short_backing_asset(_db);
   const auto& idx = _db.get_index_type<collateral_bid_index>();
   const auto& aidx = idx.indices().get<by_price>();
   auto start = aidx.lower_bound( boost::make_tuple( asset_id, price::max(back.id, asset_id), collateral_bid_id_type() ) );
   auto end = aidx.lower_bound( boost::make_tuple( asset_id, price::min(back.id, asset_id), collateral_bid_id_type(GRAPHENE_DB_MAX_INSTANCE_ID) ) );
   vector<collateral_bid_object> result;
   while( skip-- > 0 && start != end ) { ++start; }
   while( start != end && limit-- > 0)
   {
      result.push_back(*start);
      ++start;
   }
   return result;
} FC_CAPTURE_AND_RETHROW( (asset_id)(limit)(skip) ) }

void database_api::subscribe_to_market(std::function<void(const variant&)> callback, asset_id_type a, asset_id_type b)
{
   my->subscribe_to_market( callback, a, b );
}

void database_api_impl::subscribe_to_market(std::function<void(const variant&)> callback, asset_id_type a, asset_id_type b)
{
   if(a > b) std::swap(a,b);
   FC_ASSERT(a != b);
   _market_subscriptions[ std::make_pair(a,b) ] = callback;
}

void database_api::unsubscribe_from_market(asset_id_type a, asset_id_type b)
{
   my->unsubscribe_from_market( a, b );
}

void database_api_impl::unsubscribe_from_market(asset_id_type a, asset_id_type b)
{
   if(a > b) std::swap(a,b);
   FC_ASSERT(a != b);
   _market_subscriptions.erase(std::make_pair(a,b));
}

void database_api::subscribe_to_bitlender(std::function<void(const variant&)> callback, asset_id_type a, asset_id_type b)
{
   my->subscribe_to_bitlender( callback, a, b );
}

void database_api_impl::subscribe_to_bitlender(std::function<void(const variant&)> callback, asset_id_type a, asset_id_type b)
{
   if(a > b) std::swap(a,b);
   FC_ASSERT(a != b);
   _bitlender_subscriptions[ std::make_pair(a,b) ] = callback;
}

void database_api::unsubscribe_from_bitlender(asset_id_type a, asset_id_type b)
{
   my->unsubscribe_from_bitlender( a, b );
}

void database_api_impl::unsubscribe_from_bitlender(asset_id_type a, asset_id_type b)
{
   if(a > b) std::swap(a,b);
   FC_ASSERT(a != b);
   _bitlender_subscriptions.erase(std::make_pair(a,b));
}






string database_api_impl::price_to_string( const price& _price, const asset_object& _base, const asset_object& _quote )
{ try {
   if( _price.base.asset_id == _base.id && _price.quote.asset_id == _quote.id )
      return graphene::app::price_to_string( _price, _base.precision, _quote.precision );
   else if( _price.base.asset_id == _quote.id && _price.quote.asset_id == _base.id )
      return graphene::app::price_to_string( ~_price, _base.precision, _quote.precision );
   else
   {
      FC_ASSERT( !"bad parameters" );
      return "";
   }
} FC_CAPTURE_AND_RETHROW( (_price)(_base)(_quote) ) }

market_ticker database_api::get_ticker( const string& base, const string& quote )const
{
    return my->get_ticker( base, quote );
}
market_ticker database_api::get_finance_ticker( const string& base, const string& quote )const
{
    return my->get_finance_ticker( base, quote );
}
market_ticker database_api_impl::get_ticker( const string& base, const string& quote, bool skip_order_book )const
{
   market_ticker result;  
   // FC_ASSERT(_db._app_status.has_market_history_plugin, "Market history plugin is not enabled.");
   const auto assets = lookup_asset_symbols( {base, quote} );
   FC_ASSERT( assets[0], "Invalid base asset symbol: ${s}", ("s",base) );
   FC_ASSERT( assets[1], "Invalid quote asset symbol: ${s}", ("s",quote) );

   const fc::time_point_sec now = fc::time_point::now();

   result.time = now;
   result.base = base;
   result.quote = quote;
   result.latest = "0";
   result.lowest_ask = "0";
   result.highest_bid = "0";
   result.percent_change = "0";

   auto base_id = assets[0]->id;
   auto quote_id = assets[1]->id;
   if( base_id > quote_id ) std::swap( base_id, quote_id );

   if (_db._app_status.has_market_history_plugin)
   {
      fc::uint128 base_volume = 0;
      fc::uint128 quote_volume = 0;

      const auto &ticker_idx = _db.get_index_type<graphene::market_history::market_ticker_index>().indices().get<by_market>();
      auto itr = ticker_idx.find(std::make_tuple(base_id, quote_id));
      if (itr != ticker_idx.end())
      {
         price latest_price = asset(itr->latest_base, itr->base) / asset(itr->latest_quote, itr->quote);
         if (itr->base != assets[0]->id)
            latest_price = ~latest_price;
         result.latest = price_to_string(latest_price, *assets[0], *assets[1]);
         if (itr->last_day_base != 0 && itr->last_day_quote != 0                                      // has trade data before 24 hours
             && (itr->last_day_base != itr->latest_base || itr->last_day_quote != itr->latest_quote)) // price changed
         {
            price last_day_price = asset(itr->last_day_base, itr->base) / asset(itr->last_day_quote, itr->quote);
            if (itr->base != assets[0]->id)
               last_day_price = ~last_day_price;
            result.percent_change = price_diff_percent_string(last_day_price, latest_price);
         }
         if (assets[0]->id == itr->base)
         {
            base_volume = itr->base_volume;
            quote_volume = itr->quote_volume;
         }
         else
         {
            base_volume = itr->quote_volume;
            quote_volume = itr->base_volume;
         }
      }

      result.base_volume = uint128_amount_to_string(base_volume, assets[0]->precision);
      result.quote_volume = uint128_amount_to_string(quote_volume, assets[1]->precision);

      if (!skip_order_book)
      {
         const auto orders = get_order_book(base, quote, 1);
         if (!orders.asks.empty())
            result.lowest_ask = orders.asks[0].price;
         if (!orders.bids.empty())
            result.highest_bid = orders.bids[0].price;
      }
   }
   return result;
}

market_ticker database_api_impl::get_finance_ticker( const string& base, const string& quote, bool skip_order_book )const
{
   FC_ASSERT( _db._app_status.has_finance_history_plugin, "finance history plugin is not enabled." );

   const auto assets = lookup_asset_symbols( {base, quote} );
   FC_ASSERT( assets[0], "Invalid base asset symbol: ${s}", ("s",base) );
   FC_ASSERT( assets[1], "Invalid quote asset symbol: ${s}", ("s",quote) );

   const fc::time_point_sec now = fc::time_point::now();

   market_ticker result;
   result.time = now;
   result.base = base;
   result.quote = quote;
   result.latest = "0";
   result.lowest_ask = "0";
   result.highest_bid = "0";
   result.percent_change = "0";

   auto base_id = assets[0]->id;
   auto quote_id = assets[1]->id;
   if( base_id > quote_id ) std::swap( base_id, quote_id );

   fc::uint128 base_volume = 0;
   fc::uint128 quote_volume = 0;

   const auto& ticker_idx = _db.get_index_type<graphene::finance_history::finance_ticker_index>().indices().get<graphene::finance_history::by_finance>();
   auto itr = ticker_idx.find( std::make_tuple( base_id, quote_id ) );
   if( itr != ticker_idx.end() )
   {
      price latest_price = asset( itr->latest_base, itr->base ) / asset( itr->latest_quote, itr->quote );
      if( itr->base != assets[0]->id )
         latest_price = ~latest_price;
      result.latest = price_to_string( latest_price, *assets[0], *assets[1] );
      if( itr->last_day_base != 0 && itr->last_day_quote != 0 // has trade data before 24 hours
            && ( itr->last_day_base != itr->latest_base || itr->last_day_quote != itr->latest_quote ) ) // price changed
      {
         price last_day_price = asset( itr->last_day_base, itr->base ) / asset( itr->last_day_quote, itr->quote );
         if( itr->base != assets[0]->id )
            last_day_price = ~last_day_price;
         result.percent_change = price_diff_percent_string( last_day_price, latest_price );
      }
      if( assets[0]->id == itr->base )
      {
         base_volume = itr->base_volume;
         quote_volume = itr->quote_volume;
      }
      else
      {
         base_volume = itr->quote_volume;
         quote_volume = itr->base_volume;
      }
   }

   result.base_volume  = uint128_amount_to_string( base_volume, assets[0]->precision );
   result.quote_volume = uint128_amount_to_string( quote_volume, assets[1]->precision );

   if(0)// !skip_order_book )
   {
      const auto orders = get_order_book( base, quote, 1 );
      if( !orders.asks.empty() ) result.lowest_ask = orders.asks[0].price;
      if( !orders.bids.empty() ) result.highest_bid = orders.bids[0].price;
   }

   return result;
}
market_volume database_api::get_24_volume( const string& base, const string& quote )const
{
    return my->get_24_volume( base, quote );
}

market_volume database_api_impl::get_24_volume( const string& base, const string& quote )const
{
   const auto& ticker = get_ticker( base, quote, true );

   market_volume result;
   result.time = ticker.time;
   result.base = ticker.base;
   result.quote = ticker.quote;
   result.base_volume = ticker.base_volume;
   result.quote_volume = ticker.quote_volume;

   return result;
}

order_book database_api::get_order_book( const string& base, const string& quote, unsigned limit )const
{
   return my->get_order_book( base, quote, limit);
}

order_book database_api_impl::get_order_book( const string& base, const string& quote, unsigned limit )const
{
   using boost::multiprecision::uint128_t;
   FC_ASSERT( limit <= 50 );

   order_book result;
   result.base = base;
   result.quote = quote;

   auto assets = lookup_asset_symbols( {base, quote} );
   FC_ASSERT( assets[0], "Invalid base asset symbol: ${s}", ("s",base) );
   FC_ASSERT( assets[1], "Invalid quote asset symbol: ${s}", ("s",quote) );

   auto base_id = assets[0]->id;
   auto quote_id = assets[1]->id;
   auto orders = get_limit_orders( base_id, quote_id, limit );

   for( const auto& o : orders )
   {
      if(o.sell_price.base.amount <=0  || o.sell_price.quote.amount <=0) 
         continue;
      if( o.sell_price.base.asset_id == base_id )
      {
         order ord;
         ord.price = price_to_string( o.sell_price, *assets[0], *assets[1] );
         ord.quote = assets[1]->amount_to_string( share_type( ( uint128_t( o.for_sale.value ) * o.sell_price.quote.amount.value ) / o.sell_price.base.amount.value ) );
         ord.base = assets[0]->amount_to_string( o.for_sale );
         result.bids.push_back( ord );
      }
      else
      {
         order ord;
         ord.price = price_to_string( o.sell_price, *assets[0], *assets[1] );
         ord.quote = assets[1]->amount_to_string( o.for_sale );
         ord.base = assets[0]->amount_to_string( share_type( ( uint128_t( o.for_sale.value ) * o.sell_price.quote.amount.value ) / o.sell_price.base.amount.value ) );
         result.asks.push_back( ord );
      }
   }

   return result;
}

vector<market_volume> database_api::get_top_markets(uint32_t limit)const
{
   return my->get_top_markets(limit);
}

vector<market_volume> database_api_impl::get_top_markets(uint32_t limit)const
{
   FC_ASSERT(_db._app_status.has_market_history_plugin, "Market history plugin is not enabled." );

   FC_ASSERT( limit <= 100 );

   const auto& volume_idx = _db.get_index_type<graphene::market_history::market_ticker_index>().indices().get<by_volume>();
   auto itr = volume_idx.rbegin();
   vector<market_volume> result;
   result.reserve(limit);

   const fc::time_point_sec now = fc::time_point::now();

   while( itr != volume_idx.rend() && result.size() < limit)
   {
      market_volume mv;
      mv.time = now;
      const auto assets = get_assets( { itr->base, itr->quote } );
      mv.base = assets[0]->symbol;
      mv.quote = assets[1]->symbol;
      mv.base_volume = uint128_amount_to_string( itr->base_volume, assets[0]->precision );
      mv.quote_volume = uint128_amount_to_string( itr->quote_volume, assets[1]->precision );
      result.emplace_back( std::move(mv) );
      ++itr;
   }
   return result;
}

vector<market_trade> database_api::get_trade_history( const string& base,
                                                      const string& quote,
                                                      fc::time_point_sec start,
                                                      fc::time_point_sec stop,
                                                      unsigned limit )const
{
   return my->get_trade_history( base, quote, start, stop, limit );
}

vector<market_trade> database_api_impl::get_trade_history( const string& base,
                                                           const string& quote,
                                                           fc::time_point_sec start,
                                                           fc::time_point_sec stop,
                                                           unsigned limit )const
{
   FC_ASSERT( _db._app_status.has_market_history_plugin, "Market history plugin is not enabled." );

   FC_ASSERT( limit <= 100 );

   auto assets = lookup_asset_symbols( {base, quote} );
   FC_ASSERT( assets[0], "Invalid base asset symbol: ${s}", ("s",base) );
   FC_ASSERT( assets[1], "Invalid quote asset symbol: ${s}", ("s",quote) );

   auto base_id = assets[0]->id;
   auto quote_id = assets[1]->id;

   if( base_id > quote_id ) std::swap( base_id, quote_id );

   if ( start.sec_since_epoch() == 0 )
      start = fc::time_point_sec( fc::time_point::now() );

   uint32_t count = 0;
   const auto& history_idx = _db.get_index_type<graphene::market_history::history_index>().indices().get<by_market_time>();
   auto itr = history_idx.lower_bound( std::make_tuple( base_id, quote_id, start ) );
   vector<market_trade> result;

   while( itr != history_idx.end() && count < limit && !( itr->key.base != base_id || itr->key.quote != quote_id || itr->time < stop ) )
   {
      {
         market_trade trade;

         if( assets[0]->id == itr->op.receives.asset_id )
         {
            trade.amount = assets[1]->amount_to_string( itr->op.pays );
            trade.value = assets[0]->amount_to_string( itr->op.receives );
         }
         else
         {
            trade.amount = assets[1]->amount_to_string( itr->op.receives );
            trade.value = assets[0]->amount_to_string( itr->op.pays );
         }

         trade.date = itr->time;
         trade.price = price_to_string( itr->op.fill_price, *assets[0], *assets[1] );

         if( itr->op.is_maker )
         {
            trade.sequence = -itr->key.sequence;
            trade.side1_account_id = itr->op.account_id;
         }
         else
            trade.side2_account_id = itr->op.account_id;

         auto next_itr = std::next(itr);
         // Trades are usually tracked in each direction, exception: for global settlement only one side is recorded
         if( next_itr != history_idx.end() && next_itr->key.base == base_id && next_itr->key.quote == quote_id
             && next_itr->time == itr->time && next_itr->op.is_maker != itr->op.is_maker )
         {  // next_itr now could be the other direction // FIXME not 100% sure
            if( next_itr->op.is_maker )
            {
               trade.sequence = -next_itr->key.sequence;
               trade.side1_account_id = next_itr->op.account_id;
            }
            else
               trade.side2_account_id = next_itr->op.account_id;
            // skip the other direction
            itr = next_itr;
         }

         result.push_back( trade );
         ++count;
      }

      ++itr;
   }

   return result;
}

vector<market_trade> database_api::get_trade_history_by_sequence(
                                                      const string& base,
                                                      const string& quote,
                                                      int64_t start,
                                                      fc::time_point_sec stop,
                                                      unsigned limit )const
{
   return my->get_trade_history_by_sequence( base, quote, start, stop, limit );
}

vector<market_trade> database_api_impl::get_trade_history_by_sequence(
                                                           const string& base,
                                                           const string& quote,
                                                           int64_t start,
                                                           fc::time_point_sec stop,
                                                           unsigned limit )const
{
   FC_ASSERT( _db._app_status.has_market_history_plugin, "Market history plugin is not enabled." );

   FC_ASSERT( limit <= 100 );
   FC_ASSERT( start >= 0 );
   int64_t start_seq = -start;

   auto assets = lookup_asset_symbols( {base, quote} );
   FC_ASSERT( assets[0], "Invalid base asset symbol: ${s}", ("s",base) );
   FC_ASSERT( assets[1], "Invalid quote asset symbol: ${s}", ("s",quote) );

   auto base_id = assets[0]->id;
   auto quote_id = assets[1]->id;

   if( base_id > quote_id ) std::swap( base_id, quote_id );
   const auto& history_idx = _db.get_index_type<graphene::market_history::history_index>().indices().get<by_key>();
   history_key hkey;
   hkey.base = base_id;
   hkey.quote = quote_id;
   hkey.sequence = start_seq;

   uint32_t count = 0;
   auto itr = history_idx.lower_bound( hkey );
   vector<market_trade> result;

   while( itr != history_idx.end() && count < limit && !( itr->key.base != base_id || itr->key.quote != quote_id || itr->time < stop ) )
   {
      if( itr->key.sequence == start_seq ) // found the key, should skip this and the other direction if found
      {
         auto next_itr = std::next(itr);
         if( next_itr != history_idx.end() && next_itr->key.base == base_id && next_itr->key.quote == quote_id
             && next_itr->time == itr->time && next_itr->op.is_maker != itr->op.is_maker )
         {  // next_itr now could be the other direction // FIXME not 100% sure
            // skip the other direction
            itr = next_itr;
         }
      }
      else
      {
         market_trade trade;

         if( assets[0]->id == itr->op.receives.asset_id )
         {
            trade.amount = assets[1]->amount_to_string( itr->op.pays );
            trade.value = assets[0]->amount_to_string( itr->op.receives );
         }
         else
         {
            trade.amount = assets[1]->amount_to_string( itr->op.receives );
            trade.value = assets[0]->amount_to_string( itr->op.pays );
         }

         trade.date = itr->time;
         trade.price = price_to_string( itr->op.fill_price, *assets[0], *assets[1] );

         if( itr->op.is_maker )
         {
            trade.sequence = -itr->key.sequence;
            trade.side1_account_id = itr->op.account_id;
         }
         else
            trade.side2_account_id = itr->op.account_id;

         auto next_itr = std::next(itr);
         // Trades are usually tracked in each direction, exception: for global settlement only one side is recorded
         if( next_itr != history_idx.end() && next_itr->key.base == base_id && next_itr->key.quote == quote_id
             && next_itr->time == itr->time && next_itr->op.is_maker != itr->op.is_maker )
         {  // next_itr now could be the other direction // FIXME not 100% sure
            if( next_itr->op.is_maker )
            {
               trade.sequence = -next_itr->key.sequence;
               trade.side1_account_id = next_itr->op.account_id;
            }
            else
               trade.side2_account_id = next_itr->op.account_id;
            // skip the other direction
            itr = next_itr;
         }

         result.push_back( trade );
         ++count;
      }

      ++itr;
   }

   return result;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Witnesses                                                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<optional<witness_object>> database_api::get_witnesses(const vector<witness_id_type>& witness_ids)const
{
   return my->get_witnesses( witness_ids );
}

vector<optional<witness_object>> database_api_impl::get_witnesses(const vector<witness_id_type>& witness_ids)const
{
   vector<optional<witness_object>> result; result.reserve(witness_ids.size());
   std::transform(witness_ids.begin(), witness_ids.end(), std::back_inserter(result),
                  [this](witness_id_type id) -> optional<witness_object> {
      if(auto o = _db.find(id))
         return *o;
      return {};
   });
   return result;
}

fc::optional<witness_object> database_api::get_witness_by_account(account_id_type account)const
{
   return my->get_witness_by_account( account );
}

fc::optional<witness_object> database_api_impl::get_witness_by_account(account_id_type account) const
{
   const auto& idx = _db.get_index_type<witness_index>().indices().get<by_account>();
   auto itr = idx.find(account);
   if( itr != idx.end() )
      return *itr;
   return {};
}

map<string, witness_id_type> database_api::lookup_witness_accounts(const string& lower_bound_name, uint32_t limit)const
{
   return my->lookup_witness_accounts( lower_bound_name, limit );
}

map<string, witness_id_type> database_api_impl::lookup_witness_accounts(const string& lower_bound_name, uint32_t limit)const
{
   FC_ASSERT( limit <= 1000 );
   const auto& witnesses_by_id = _db.get_index_type<witness_index>().indices().get<by_id>();

   // we want to order witnesses by account name, but that name is in the account object
   // so the witness_index doesn't have a quick way to access it.
   // get all the names and look them all up, sort them, then figure out what
   // records to return.  This could be optimized, but we expect the
   // number of witnesses to be few and the frequency of calls to be rare
   std::map<std::string, witness_id_type> witnesses_by_account_name;
   for (const witness_object& witness : witnesses_by_id)
       if (auto account_iter = _db.find(witness.witness_account))
           if (account_iter->name >= lower_bound_name) // we can ignore anything below lower_bound_name
               witnesses_by_account_name.insert(std::make_pair(account_iter->name, witness.id));

   auto end_iter = witnesses_by_account_name.begin();
   while (end_iter != witnesses_by_account_name.end() && limit--)
       ++end_iter;
   witnesses_by_account_name.erase(end_iter, witnesses_by_account_name.end());
   return witnesses_by_account_name;
}

uint64_t database_api::get_witness_count()const
{
   return my->get_witness_count();
}

uint64_t database_api_impl::get_witness_count()const
{
   return _db.get_index_type<witness_index>().indices().size();
}




//////////////////////////////////////////////////////////////////////
//                                                                  //
// carrieres                                                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<optional<carrier_object>> database_api::get_carrieres(const vector<carrier_id_type>& carrier_ids)const
{
   return my->get_carrieres( carrier_ids );
}

vector<optional<carrier_object>> database_api_impl::get_carrieres(const vector<carrier_id_type>& carrier_ids)const
{
   vector<optional<carrier_object>> result; result.reserve(carrier_ids.size());
   std::transform(carrier_ids.begin(), carrier_ids.end(), std::back_inserter(result),
                  [this](carrier_id_type id) -> optional<carrier_object> {
      if(auto o = _db.find(id))
         return *o;
      return {};
   });
   return result;
}

fc::optional<carrier_object> database_api::get_carrier_by_account(account_id_type account)const
{
   return my->get_carrier_by_account( account );
}

fc::optional<carrier_object> database_api_impl::get_carrier_by_account(account_id_type account) const
{
   const auto& idx = _db.get_index_type<carrier_index>().indices().get<by_account>();
   auto itr = idx.find(account);
   if( itr != idx.end() )
      return *itr;
   return {};
}

map<string, carrier_id_type> database_api::lookup_carrier_accounts(const string& lower_bound_name, uint32_t limit)const
{
   return my->lookup_carrier_accounts( lower_bound_name, limit );
}

map<string, carrier_id_type> database_api_impl::lookup_carrier_accounts(const string& lower_bound_name, uint32_t limit)const
{
   FC_ASSERT( limit <= 1000 );
   const auto& carrieres_by_id = _db.get_index_type<carrier_index>().indices().get<by_id>();

   // we want to order carrieres by account name, but that name is in the account object
   // so the carrier_index doesn't have a quick way to access it.
   // get all the names and look them all up, sort them, then figure out what
   // records to return.  This could be optimized, but we expect the
   // number of carrieres to be few and the frequency of calls to be rare
   std::map<std::string, carrier_id_type> carrieres_by_account_name;
   for (const carrier_object& carrier : carrieres_by_id)
       if (auto account_iter = _db.find(carrier.carrier_account))
           if (account_iter->name >= lower_bound_name) // we can ignore anything below lower_bound_name
               carrieres_by_account_name.insert(std::make_pair(account_iter->name, carrier.id));

   auto end_iter = carrieres_by_account_name.begin();
   while (end_iter != carrieres_by_account_name.end() && limit--)
       ++end_iter;
   carrieres_by_account_name.erase(end_iter, carrieres_by_account_name.end());
   return carrieres_by_account_name;
}

uint64_t database_api::get_carrier_count()const
{
   return my->get_carrier_count();
}

uint64_t database_api_impl::get_carrier_count()const
{
   return _db.get_index_type<carrier_index>().indices().size();
}






//////////////////////////////////////////////////////////////////////
//                                                                  //
// authores                                                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<optional<author_object>> database_api::get_authors(const vector<author_id_type>& author_ids)const
{
   return my->get_authors( author_ids );
}

vector<optional<author_object>> database_api_impl::get_authors(const vector<author_id_type>& author_ids)const
{
   vector<optional<author_object>> result; result.reserve(author_ids.size());
   std::transform(author_ids.begin(), author_ids.end(), std::back_inserter(result),
                  [this](author_id_type id) -> optional<author_object> {
      if(auto o = _db.find(id))
         return *o;
      return {};
   });
   return result;
}

fc::optional<author_object> database_api::get_author_by_account(account_id_type account)const
{
   return my->get_author_by_account( account );
}

fc::optional<author_object> database_api_impl::get_author_by_account(account_id_type account) const
{
   const auto& idx = _db.get_index_type<author_index>().indices().get<by_account>();
   auto itr = idx.find(account);
   if( itr != idx.end() )
      return *itr;
   return {};
}

optional<carrier_cfg>  database_api::get_carrier_cfg(const carrier_id_type carrier_id, const asset_id_type asset_id) const 
{
   return my->get_carrier_cfg(carrier_id, asset_id);
}
optional<carrier_cfg>  database_api_impl::get_carrier_cfg(const carrier_id_type carrier_id, const asset_id_type asset_id) const 
{
    try {   
      auto &ass   = asset_id(_db);  
      auto &agg   = carrier_id(_db);
      auto &acc   = agg.carrier_account(_db);      

      carrier_cfg add;
      add.assetid           = asset_id;
      add.symbol            = ass.symbol;
      add.uasset_property   = ass.uasset_property;      
      add.lendCarrier       = agg.id;
      add.lendaccount       = acc.id;
      add.lendname          = acc.name;
      add.lendUrl           = agg.url;
      add.lendauthor        = agg.get_author(_db, asset_id);
      add.lendenable        = agg.is_enable();
      if (add.lendauthor.valid())
      {
        auto &author    = (*add.lendauthor)(_db);
        add.lendauthor_account = author.author_account;         
        add.lendauthor_url  = author.url;
        auto &account   = (*add.lendauthor_account)(_db);
        add.lendauthor_name = account.name;
      }      
      return add;
} FC_CAPTURE_AND_RETHROW((carrier_id)(asset_id)) }

fc::optional<author_object>  database_api::get_author_by_trust(object_id_type object_id,asset_id_type asset_id)const
{
   return my->get_author_by_trust( object_id ,asset_id);
}
fc::optional<author_object> database_api_impl::get_author_by_trust(object_id_type object_id,asset_id_type asset_id)const
{
   try
   {
    if(object_id.type() == carrier_object_type )
    {
       carrier_id_type id = object_id;
       optional<author_id_type> a = id(_db).get_author(_db,asset_id);
       if(a.valid())
          return (*a)(_db);
    }
    else if(object_id.type() == gateway_object_type )
    {
       gateway_id_type id = object_id;
       optional<author_id_type> a = id(_db).get_author(_db,asset_id);
       if(a.valid())
          return (*a)(_db);
    }
   return fc::optional<author_object>();
} FC_CAPTURE_AND_RETHROW( (object_id)(asset_id)) }

map<string, author_id_type> database_api::lookup_author_accounts(const string& lower_bound_name, uint32_t limit)const
{
   return my->lookup_author_accounts( lower_bound_name, limit );
}

map<string, author_id_type> database_api_impl::lookup_author_accounts(const string& lower_bound_name, uint32_t limit)const
{
   FC_ASSERT( limit <= 1000 );
   const auto& authores_by_id = _db.get_index_type<author_index>().indices().get<by_id>();

   // we want to order authores by account name, but that name is in the account object
   // so the author_index doesn't have a quick way to access it.
   // get all the names and look them all up, sort them, then figure out what
   // records to return.  This could be optimized, but we expect the
   // number of authores to be few and the frequency of calls to be rare
   std::map<std::string, author_id_type> authores_by_account_name;
   for (const author_object& author : authores_by_id)
       if (auto account_iter = _db.find(author.author_account))
           if (account_iter->name >= lower_bound_name) // we can ignore anything below lower_bound_name
               authores_by_account_name.insert(std::make_pair(account_iter->name, author.id));

   auto end_iter = authores_by_account_name.begin();
   while (end_iter != authores_by_account_name.end() && limit--)
       ++end_iter;
   authores_by_account_name.erase(end_iter, authores_by_account_name.end());
   return authores_by_account_name;
}

uint64_t database_api::get_author_count()const
{
   return my->get_author_count();
}

uint64_t database_api_impl::get_author_count()const
{
   return _db.get_index_type<author_index>().indices().size();
}


asset database_api::get_allowed_withdraw(const object_id_type id,const fc::time_point_sec &now )const
{
 return my->get_allowed_withdraw( id, now);
}
asset database_api_impl::get_allowed_withdraw(const object_id_type id,const fc::time_point_sec &now )const
{
    try{
        if(id.type() == balance_object_type )
        {
            balance_id_type a = id;
            const auto  &ao = a(_db);
            return ao.available(now);
        }
        else if(id.type() == vesting_balance_object_type )
        {
            vesting_balance_id_type a = id;
            const auto   &ao = a(_db);
            return ao.get_allowed_withdraw(now);
        }
        else
            return asset(0, asset_id_type(0));        

} FC_CAPTURE_AND_RETHROW( (id)(now)) }


//////////////////////////////////////////////////////////////////////
//                                                                  //
// gatewayes                                                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<optional<gateway_object>> database_api::get_gatewayes(const vector<gateway_id_type>& gateway_ids)const
{
   return my->get_gatewayes( gateway_ids );
}

vector<optional<gateway_object>> database_api_impl::get_gatewayes(const vector<gateway_id_type>& gateway_ids)const
{
   vector<optional<gateway_object>> result; result.reserve(gateway_ids.size());
   std::transform(gateway_ids.begin(), gateway_ids.end(), std::back_inserter(result),
                  [this](gateway_id_type id) -> optional<gateway_object> {
      if(auto o = _db.find(id))
         return *o;
      return {};
   });
   return result;
}
fc::variant_object   database_api::get_account_attachinfo(const account_id_type acc,uint32_t utype) const
{
   return my->get_account_attachinfo( acc ,utype);
}
fc::variant_object   database_api_impl::get_account_attachinfo(const account_id_type acc,uint32_t utype) const
{ try{
      fc::mutable_variant_object result_obj;
      fc::variant info_j,balance_j;
      if (utype == gateway_object_type)
      {
         fc::optional<gateway_object> pgate = get_gateway_by_account(acc);
         if (pgate.valid())
         {
            vector<asset_info> info;
            for (auto a : pgate->allowed_assets)
            {
               const auto &ass = a(_db);
               asset_info add;
               ass.get_info(add);
               info.push_back(add);
            }
            vector<asset_summary> balances = get_account_balances(acc, pgate->allowed_assets);
            result_obj.set("base", pgate->to_variant());
            fc::to_variant(info, info_j, GRAPHENE_NET_MAX_NESTED_OBJECTS);
            result_obj.set("asset_list",info_j);
            fc::to_variant(balances, balance_j, GRAPHENE_NET_MAX_NESTED_OBJECTS);
            result_obj.set("balance_list",balance_j);
            return result_obj;
         }
   }
   else if(utype == carrier_object_type){
      fc::optional<carrier_object> pgate = get_carrier_by_account(acc);
      if(pgate.valid())
      {
         result_obj.set("base", pgate->to_variant());
         return result_obj;         
      }
   }
   return result_obj;
} FC_CAPTURE_AND_RETHROW((acc)(utype)) }
optional<gateway_cfg>  database_api::get_gateway_cfg(const gateway_id_type gateway_id,const asset_id_type asset_id)const
{
   return my->get_gateway_cfg( gateway_id ,asset_id);
}

optional<gateway_cfg>  database_api_impl::get_gateway_cfg(const gateway_id_type gateway_id,const asset_id_type asset_id)const
{  try {   
      auto &ass   = asset_id(_db);  
      auto &agg   = gateway_id(_db);
      auto &acc   = agg.gateway_account(_db);        
      auto &aggd  = agg.dynamic_id(_db);
      auto iter1  = aggd.deposit_amount.find(asset_id);
      auto iter2  = aggd.withdraw_amount.find(asset_id);
      gateway_cfg add;
      add.asset_id          = asset_id;
      add.symbol            = ass.symbol;
      add.real_symbol       = ass.real_asset;
      add.real_asset_sub    = ass.real_asset_sub;
      add.uasset_property   = ass.uasset_property;
      add.gateway_id        = agg.id;
      add.account_id        = acc.id;      
      add.account_name      = acc.name;
      add.gateway_url       = agg.url;
      add.precision         = ass.precision;
      add.deposit_amount    = iter1 != aggd.deposit_amount.end()  ? iter1 ->second : 0;
      add.withdraw_amount   = iter2 != aggd.withdraw_amount.end() ? iter2 ->second : 0;
      add.precision         = ass.precision;
      add.author_id         = agg.get_author(_db, asset_id);
      add.enable            = agg.is_enable();
      if (add.author_id.valid())
      {
        add.author_account_id = (*add.author_id)(_db).author_account; 
        auto &author    = (*add.author_id)(_db);
        add.author_url  = author.url;
        auto &account   = (*add.author_account_id)(_db);
        add.author_name = account.name;
      }
      return add;
} FC_CAPTURE_AND_RETHROW((gateway_id)) }

fc::optional<gateway_object> database_api::get_gateway_by_account(account_id_type account)const
{
   return my->get_gateway_by_account( account );
}

fc::optional<gateway_object> database_api_impl::get_gateway_by_account(account_id_type account) const
{
   const auto& idx = _db.get_index_type<gateway_index>().indices().get<by_account>();
   auto itr = idx.find(account);
   if( itr != idx.end() )
      return *itr;
   return {};
}

map<string, gateway_id_type> database_api::lookup_gateway_accounts(const string& lower_bound_name, uint32_t limit)const
{
   return my->lookup_gateway_accounts( lower_bound_name, limit );
}

map<string, gateway_id_type> database_api_impl::lookup_gateway_accounts(const string& lower_bound_name, uint32_t limit)const
{
   FC_ASSERT( limit <= 1000 );
   const auto& gatewayes_by_id = _db.get_index_type<gateway_index>().indices().get<by_id>();

   // we want to order gatewayes by account name, but that name is in the account object
   // so the gateway_index doesn't have a quick way to access it.
   // get all the names and look them all up, sort them, then figure out what
   // records to return.  This could be optimized, but we expect the
   // number of gatewayes to be few and the frequency of calls to be rare
   std::map<std::string, gateway_id_type> gatewayes_by_account_name;
   for (const gateway_object& gateway : gatewayes_by_id)
       if (auto account_iter = _db.find(gateway.gateway_account))
           if (account_iter->name >= lower_bound_name) // we can ignore anything below lower_bound_name
               gatewayes_by_account_name.insert(std::make_pair(account_iter->name, gateway.id));

   auto end_iter = gatewayes_by_account_name.begin();
   while (end_iter != gatewayes_by_account_name.end() && limit--)
       ++end_iter;
   gatewayes_by_account_name.erase(end_iter, gatewayes_by_account_name.end());
   return gatewayes_by_account_name;
}

uint64_t database_api::get_gateway_count()const
{
   return my->get_gateway_count();
}

uint64_t database_api_impl::get_gateway_count()const
{
   return _db.get_index_type<gateway_index>().indices().size();
}











//////////////////////////////////////////////////////////////////////
//                                                                  //
// Committee members                                                //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<optional<committee_member_object>> database_api::get_committee_members(const vector<committee_member_id_type>& committee_member_ids)const
{
   return my->get_committee_members( committee_member_ids );
}

vector<optional<committee_member_object>> database_api_impl::get_committee_members(const vector<committee_member_id_type>& committee_member_ids)const
{
   vector<optional<committee_member_object>> result; result.reserve(committee_member_ids.size());
   std::transform(committee_member_ids.begin(), committee_member_ids.end(), std::back_inserter(result),
                  [this](committee_member_id_type id) -> optional<committee_member_object> {
      if(auto o = _db.find(id))
         return *o;
      return {};
   });
   return result;
}

fc::optional<committee_member_object> database_api::get_committee_member_by_account(account_id_type account)const
{
   return my->get_committee_member_by_account( account );
}

fc::optional<committee_member_object> database_api_impl::get_committee_member_by_account(account_id_type account) const
{
   const auto& idx = _db.get_index_type<committee_member_index>().indices().get<by_account>();
   auto itr = idx.find(account);
   if( itr != idx.end() )
      return *itr;
   return {};
}


map<string, committee_member_id_type> database_api::lookup_committee_member_accounts(const string& lower_bound_name, uint32_t limit)const
{
   return my->lookup_committee_member_accounts( lower_bound_name, limit );
}

map<string, committee_member_id_type> database_api_impl::lookup_committee_member_accounts(const string& lower_bound_name, uint32_t limit)const
{
   FC_ASSERT( limit <= 1000 );
   const auto& committee_members_by_id = _db.get_index_type<committee_member_index>().indices().get<by_id>();

   // we want to order committee_members by account name, but that name is in the account object
   // so the committee_member_index doesn't have a quick way to access it.
   // get all the names and look them all up, sort them, then figure out what
   // records to return.  This could be optimized, but we expect the
   // number of committee_members to be few and the frequency of calls to be rare
   std::map<std::string, committee_member_id_type> committee_members_by_account_name;
   for (const committee_member_object& committee_member : committee_members_by_id)
       if (auto account_iter = _db.find(committee_member.committee_member_account))
           if (account_iter->name >= lower_bound_name) // we can ignore anything below lower_bound_name
               committee_members_by_account_name.insert(std::make_pair(account_iter->name, committee_member.id));

   auto end_iter = committee_members_by_account_name.begin();
   while (end_iter != committee_members_by_account_name.end() && limit--)
       ++end_iter;
   committee_members_by_account_name.erase(end_iter, committee_members_by_account_name.end());
   return committee_members_by_account_name;
}





uint64_t database_api::get_committee_count()const
{
    return my->get_committee_count();
}

uint64_t database_api_impl::get_committee_count()const
{
    return _db.get_index_type<committee_member_index>().indices().size();
}






//////////////////////////////////////////////////////////////////////
//                                                                  //
// Budget members                                                //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<optional<budget_member_object>> database_api::get_budget_members(const vector<budget_member_id_type>& budget_member_ids)const
{
   return my->get_budget_members( budget_member_ids );
}

vector<optional<budget_member_object>> database_api_impl::get_budget_members(const vector<budget_member_id_type>& budget_member_ids)const
{
   vector<optional<budget_member_object>> result; result.reserve(budget_member_ids.size());
   std::transform(budget_member_ids.begin(), budget_member_ids.end(), std::back_inserter(result),
                  [this](budget_member_id_type id) -> optional<budget_member_object> {
      if(auto o = _db.find(id))
         return *o;
      return {};
   });
   return result;
}

fc::optional<budget_member_object> database_api::get_budget_member_by_account(account_id_type account)const
{
   return my->get_budget_member_by_account( account );
}

fc::optional<budget_member_object> database_api_impl::get_budget_member_by_account(account_id_type account) const
{
   const auto& idx = _db.get_index_type<budget_member_index>().indices().get<by_account>();
   auto itr = idx.find(account);
   if( itr != idx.end() )
      return *itr;
   return {};
}


map<string, budget_member_id_type> database_api::lookup_budget_member_accounts(const string& lower_bound_name, uint32_t limit)const
{
   return my->lookup_budget_member_accounts( lower_bound_name, limit );
}

map<string, budget_member_id_type> database_api_impl::lookup_budget_member_accounts(const string& lower_bound_name, uint32_t limit)const
{
   FC_ASSERT( limit <= 1000 );
   const auto& budget_members_by_id = _db.get_index_type<budget_member_index>().indices().get<by_id>();

   // we want to order budget_members by account name, but that name is in the account object
   // so the budget_member_index doesn't have a quick way to access it.
   // get all the names and look them all up, sort them, then figure out what
   // records to return.  This could be optimized, but we expect the
   // number of budget_members to be few and the frequency of calls to be rare
   std::map<std::string, budget_member_id_type> budget_members_by_account_name;
   for (const budget_member_object& budget_member : budget_members_by_id)
       if (auto account_iter = _db.find(budget_member.budget_member_account))
           if (account_iter->name >= lower_bound_name) // we can ignore anything below lower_bound_name
               budget_members_by_account_name.insert(std::make_pair(account_iter->name, budget_member.id));

   auto end_iter = budget_members_by_account_name.begin();
   while (end_iter != budget_members_by_account_name.end() && limit--)
       ++end_iter;
   budget_members_by_account_name.erase(end_iter, budget_members_by_account_name.end());
   return budget_members_by_account_name;
}





uint64_t database_api::get_budget_count()const
{
    return my->get_budget_count();
}

uint64_t database_api_impl::get_budget_count()const
{
    return _db.get_index_type<budget_member_index>().indices().size();
}

 


vector<locktoken_id_type> database_api::get_locktokens(account_id_type account,asset_id_type asset_id,uint32_t type) const
{
    return my->get_locktokens(account,asset_id,type);
}

vector<locktoken_id_type> database_api_impl::get_locktokens(account_id_type account,asset_id_type asset_id,uint32_t type) const
{
    vector<locktoken_id_type> result;
    const auto& idx = _db.get_index_type<locktoken_index>().indices().get<by_id>();
    auto iter = idx.rbegin();
    while(iter != idx.rend())
    {
       if(iter ->issuer == account || account.get_instance() == 0)
       {  
         if(type >= locktoken_typemax || iter ->type == type) {
            if(asset_id == GRAPHENE_NULL_ASSET || iter ->locked.asset_id == asset_id) {
              result.push_back(iter->id);
            }
          }
       }
       iter++;
    }
    return result;
}
fc::variant_object database_api::get_locktoken_sum(account_id_type account, asset_id_type asset_id, uint32_t utype) const
{
    return my->get_locktoken_sum(account,asset_id, utype);
}

fc::variant_object database_api_impl::get_locktoken_sum(account_id_type account, asset_id_type asset_id, uint32_t utype) const
{
   try {
   fc::mutable_variant_object result_obj;
   fc::variant info_j,info_h;

   vector<account_locktoken_item> locktoken;
   vector<account_locktoken_item> locktoken_his;
   uint32_t allLockCount = 0;
   uint32_t allLockCount_his = 0;

   share_type    nodeamount = 0;
   uint32_t      nodeautolock = 0; 

   const auto &ass = asset_id(_db);
   const auto &a = account(_db);
   const auto &ad = a.statistics(_db);

   account_locktoken_item add;
   add.total.asset_id = ass.id;
   add.total.precision = ass.precision;
   add.total.symbol = ass.symbol;
   for (int l = locktoken_dy; l < locktoken_typemax; l++)
   {
      add.type = l;
      locktoken.push_back(add);
   }
   for (int l = locktoken_dy; l < locktoken_typemax; l++)
   {
      add.type = l;
      locktoken_his.push_back(add);
   }

   if (utype & 0x08)
   {
      const auto &idx = _db.get_index_type<locktoken_index>().indices().get<by_id>();
      auto iter = idx.rbegin();
      while (iter != idx.rend())
      {
         if (iter->issuer == account && asset_id == iter->locked.asset_id)
         {
            locktoken[iter->type].total.amount += iter->locked.amount;
            locktoken[iter->type].add_interest(iter->interest);  
            if(iter->type == locktoken_dy)                
               locktoken[iter->type].add_interest(iter->dy_interest);  
            locktoken[iter->type].count++;
            allLockCount++;
            if(iter->type == locktoken_node)
            {
               nodeamount = iter->locked.amount;
               nodeautolock = iter->autolock;               
            }               
         }
         iter++;
      }
   }
    
   if(utype & 0x04) {
     if(_db._app_status.has_locktoken_history_plugin) {
       const auto &acount_idx = _db.get_index_type<account_locktoken_his_index>();
       const auto &by_key_idx = acount_idx.indices().get<by_locktoken_account>();
       auto bucket_itr = by_key_idx.find(account);
       if (bucket_itr != by_key_idx.end())
       {
         for (int l = locktoken_dy; l < locktoken_typemax; l++)
         {
            auto itr_issue = bucket_itr->locktoken[l].find(asset_id);
            if(itr_issue != bucket_itr->locktoken[l].end()) {
              locktoken_his[l].total.amount += itr_issue->second.total.amount;  
              for(auto inter : itr_issue->second.interest)
                 locktoken_his[l].add_interest(inter.second);
              locktoken_his[l].count += itr_issue->second.count;         
              allLockCount_his++;     
            }
         }          
       }    
     }
   }
   if(utype & 0x10) {
      for (int l = locktoken_dy; l < locktoken_typemax; l++){
         for(auto &inter : locktoken[l].interest) {
            if(inter.second.precision == 0) {
              const auto &as = inter.second.asset_id(_db);
              inter.second.precision = as.precision;
              inter.second.symbol = as.symbol;
            }
         }
      } 
      for (int l = locktoken_dy; l < locktoken_typemax; l++){
         for(auto &inter : locktoken_his[l].interest) {
            if(inter.second.precision == 0) {
              const auto &as = inter.second.asset_id(_db);
              inter.second.precision = as.precision;
              inter.second.symbol = as.symbol;
            }
         }
      } 
   }
   fc::to_variant(locktoken, info_j, GRAPHENE_NET_MAX_NESTED_OBJECTS);
   result_obj.set("locktoken",info_j);
   fc::to_variant(locktoken_his, info_h, GRAPHENE_NET_MAX_NESTED_OBJECTS);
   result_obj.set("locktoken_his",info_h);
   result_obj.set("allLockCount",allLockCount);   
   result_obj.set("allLockCount_his",allLockCount_his);   
   result_obj.set("symbol", ass.symbol);
   result_obj.set("precision", ass.precision);
   result_obj.set("asset_id", ass.id.to_string());   
   result_obj.set("name", a.name);    
   result_obj.set("lock_asset", ad.lock_asset.amount.value); 
   result_obj.set("amount_coupon", ad.amount_coupon.value);  
   result_obj.set("nodeamount", nodeamount.value); 
   result_obj.set("nodeautolock", nodeautolock);    

   return result_obj; 

   }FC_CAPTURE_AND_RETHROW( (account)(asset_id)(utype) )
}

 
vector<locktoken_option_object>  database_api::get_locktoken_options(uint32_t lock_type, uint32_t checkasset)const
{
    return my->get_locktoken_options( lock_type, checkasset );
}

vector<locktoken_option_object> database_api_impl::get_locktoken_options(uint32_t lock_type, uint32_t checkasset)const
{    
   try
   {
      vector<locktoken_option_object> result;
      auto &locktoken_options = _db.get_index_type<locktoken_option_index>().indices().get<by_id>();
      for (const locktoken_option_object &option : locktoken_options)
      {
         if (option.lock_mode == lock_type) {
            if(checkasset) {
               const auto &ass = option.asset_id(_db);
               const auto &ca = option.lockoptions.carrier(_db);  
               if(!option.payer_enable || (option.lockoptions.carrier != GRAPHENE_EMPTY_ACCOUNT && !ca.is_carrier_enable(_db))) {
               }  
               else if (option.lock_mode == locktoken_dy || option.lock_mode == locktoken_fixed)
               {
                  if((ass.uasset_property & ASSET_DYFIXED)  && option.payer_enable) {
                     result.push_back(option);
                  }
               } else if(option.lock_mode == locktoken_node) {
                  if((ass.uasset_property & ASSET_LOCKNODE) && option.payer_enable) {
                     result.push_back(option);
                  }                 
               }
            }
            else {
               result.push_back(option);
            }
         }
      }
      return result;
    }
   FC_CAPTURE_AND_RETHROW( (lock_type) )
}
fc::optional<locktoken_option_object>  database_api::get_locktoken_option(asset_id_type asset_id, uint32_t lock_type, uint32_t checkasset)const
{
    return my->get_locktoken_option( asset_id,lock_type , checkasset);
}

fc::optional<locktoken_option_object> database_api_impl::get_locktoken_option(asset_id_type asset_id, uint32_t lock_type, uint32_t checkasset)const
{    
   try
   {          
         auto& locktoken_options = _db.get_index_type<locktoken_option_index>().indices().get<by_id>();       
         for( const locktoken_option_object& option : locktoken_options)
         {      
            if((option.asset_id == asset_id && option.lock_mode == lock_type))
            {
              if(checkasset ) {                 
               const auto &ass = option.asset_id(_db);
               const auto &ca = option.lockoptions.carrier(_db);  
               if(!option.payer_enable ||  (option.lockoptions.carrier != GRAPHENE_EMPTY_ACCOUNT && !ca.is_carrier_enable(_db))) {
                  return fc::optional<locktoken_option_object>(); 
               } 
               else if (option.lock_mode == locktoken_dy || option.lock_mode == locktoken_fixed)
               {
                  if((ass.uasset_property & ASSET_DYFIXED) <= 0  || !option.payer_enable) {
                     return fc::optional<locktoken_option_object>();       
                  }
               } else if(option.lock_mode == locktoken_node) {
                  if((ass.uasset_property & ASSET_LOCKNODE) <=0 || !option.payer_enable) {
                     return fc::optional<locktoken_option_object>();       
                  }                 
               }
             }
             return option;            
            }
         }        
         return fc::optional<locktoken_option_object>(); 
      
    }
   FC_CAPTURE_AND_RETHROW( (asset_id)(lock_type) )
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Workers                                                          //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<worker_object> database_api::get_all_workers()const
{
    return my->get_all_workers();
}

vector<worker_object> database_api_impl::get_all_workers()const
{
    vector<worker_object> result;
    const auto& workers_idx = _db.get_index_type<worker_index>().indices().get<by_id>();
    for( const auto& w : workers_idx )
    {
       result.push_back( w );
    }
    return result;
}

vector<optional<worker_object>> database_api::get_workers_by_account(account_id_type account)const
{
    return my->get_workers_by_account( account );
}

vector<optional<worker_object>> database_api_impl::get_workers_by_account(account_id_type account)const
{
    vector<optional<worker_object>> result;
    const auto& workers_idx = _db.get_index_type<worker_index>().indices().get<by_account>();

    for( const auto& w : workers_idx )
    {
        if( w.worker_account == account )
            result.push_back( w );
    }
    return result;
}

vector<optional<worker_object>> database_api::get_workers_by_name(string name)const
{
    return my->get_workers_by_name( name );
}

vector<optional<worker_object>> database_api_impl::get_workers_by_name(string name)const
{
    vector<optional<worker_object>> result;
    const auto& workers_idx = _db.get_index_type<worker_index>().indices().get<by_account>();

    for( const auto& w : workers_idx )
    {
        if( w.name == name )
            result.push_back( w );
    }
    return result;
}

uint64_t database_api::get_worker_count()const
{
    return my->get_worker_count();
}

uint64_t database_api_impl::get_worker_count()const
{
    return _db.get_index_type<worker_index>().indices().size();
}



//////////////////////////////////////////////////////////////////////
//                                                                  //
// Votes                                                            //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<variant> database_api::lookup_vote_ids( const vector<vote_id_type>& votes )const
{
   return my->lookup_vote_ids( votes );
}

vector<variant> database_api_impl::lookup_vote_ids( const vector<vote_id_type>& votes )const
{
   FC_ASSERT( votes.size() < 1000, "Only 1000 votes can be queried at a time" );

   const auto& witness_idx = _db.get_index_type<witness_index>().indices().get<by_vote_id>();
   const auto& gateway_idx = _db.get_index_type<gateway_index>().indices().get<by_vote_id>();
   const auto& carrier_idx = _db.get_index_type<carrier_index>().indices().get<by_vote_id>();
   const auto& author_idx  = _db.get_index_type<author_index>().indices().get<by_vote_id>();
   const auto& committee_idx = _db.get_index_type<committee_member_index>().indices().get<by_vote_id>();   
   const auto& budget_idx    = _db.get_index_type<budget_member_index>().indices().get<by_vote_id>();   
   const auto& for_worker_idx = _db.get_index_type<worker_index>().indices().get<by_vote_for>();
   const auto& against_worker_idx = _db.get_index_type<worker_index>().indices().get<by_vote_against>();

   vector<variant> result;
   result.reserve( votes.size() );
   for( auto id : votes )
   {
      switch( id.type() )
      {
         case vote_id_type::committee:
         {
            auto itr = committee_idx.find( id );
            if( itr != committee_idx.end() )
               result.emplace_back( variant( *itr, 2 ) ); // Depth of committee_member_object is 1, add 1 here to be safe
            else
               result.emplace_back( variant() );
            break;
         }

         case vote_id_type::budget:
         {
            auto itr = budget_idx.find( id );
            if( itr != budget_idx.end() )
               result.emplace_back( variant( *itr, 2 ) ); // Depth of budget_member_object is 1, add 1 here to be safe
            else
               result.emplace_back( variant() );
            break;
         }   

      
         case vote_id_type::witness:
         {
            auto itr = witness_idx.find( id );
            if( itr != witness_idx.end() )
               result.emplace_back( variant( *itr, 2 ) ); // Depth of witness_object is 1, add 1 here to be safe
            else
               result.emplace_back( variant() );
            break;
         }
           case vote_id_type::gateway:
         {
            auto itr = gateway_idx.find( id );
            if( itr != gateway_idx.end() )
               result.emplace_back( variant( *itr, 2 ) ); // Depth of gateway_object is 1, add 1 here to be safe
            else
               result.emplace_back( variant() );
            break;
         }
           case vote_id_type::carrier:
         {
            auto itr = carrier_idx.find( id );
            if( itr != carrier_idx.end() )
               result.emplace_back( variant( *itr, 2 ) ); // Depth of carrier_object is 1, add 1 here to be safe
            else
               result.emplace_back( variant() );
            break;
         }
         case vote_id_type::author:
         {
            auto itr = author_idx.find( id );
            if( itr != author_idx.end() )
               result.emplace_back( variant( *itr, 2 ) ); // Depth of author_object is 1, add 1 here to be safe
            else
               result.emplace_back( variant() );
            break;
         }
         case vote_id_type::worker:
         {
            auto itr = for_worker_idx.find( id );
            if( itr != for_worker_idx.end() ) {
               result.emplace_back( variant( *itr, 4 ) ); // Depth of worker_object is 3, add 1 here to be safe.
                                                          // If we want to extract the balance object inside,
                                                          //   need to increase this value
            }
            else {
               auto itr = against_worker_idx.find( id );
               if( itr != against_worker_idx.end() ) {
                  result.emplace_back( variant( *itr, 4 ) ); // Depth of worker_object is 3, add 1 here to be safe.
                                                             // If we want to extract the balance object inside,
                                                             //   need to increase this value
               }
               else {
                  result.emplace_back( variant() );
               }
            }
            break;
         }
         case vote_id_type::VOTE_TYPE_COUNT: break; // supress unused enum value warnings
         default:
            FC_CAPTURE_AND_THROW( fc::out_of_range_exception, (id) );
      }
   }
   return result;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Authority / validation                                           //
//                                                                  //
//////////////////////////////////////////////////////////////////////

std::string database_api::get_transaction_hex(const signed_transaction& trx)const
{
   return my->get_transaction_hex( trx );
}

std::string database_api_impl::get_transaction_hex(const signed_transaction& trx)const
{
   return fc::to_hex(fc::raw::pack(trx));
}

set<public_key_type> database_api::get_required_signatures( const signed_transaction& trx, const flat_set<public_key_type>& available_keys )const
{
   return my->get_required_signatures( trx, available_keys );
}

set<public_key_type> database_api_impl::get_required_signatures( const signed_transaction& trx, const flat_set<public_key_type>& available_keys )const
{
   auto result = trx.get_required_signatures( _db.get_chain_id(),
                                       available_keys,
                                       [&]( account_id_type id ){ return &id(_db).active; },
                                       [&]( account_id_type id ){ return &id(_db).owner; },
                                       _db.get_global_properties().parameters.max_authority_depth );
   return result;
}

set<public_key_type> database_api::get_potential_signatures( const signed_transaction& trx )const
{
   return my->get_potential_signatures( trx );
}
set<address> database_api::get_potential_address_signatures( const signed_transaction& trx )const
{
   return my->get_potential_address_signatures( trx );
}

set<public_key_type> database_api_impl::get_potential_signatures( const signed_transaction& trx )const
{
   set<public_key_type> result;
   trx.get_required_signatures(
      _db.get_chain_id(),
      flat_set<public_key_type>(),
      [&]( account_id_type id )
      {
         const auto& auth = id(_db).active;
         for( const auto& k : auth.get_keys() )
            result.insert(k);
         return &auth;
      },
      [&]( account_id_type id )
      {
         const auto& auth = id(_db).owner;
         for( const auto& k : auth.get_keys() )
            result.insert(k);
         return &auth;
      },
      _db.get_global_properties().parameters.max_authority_depth
   );

   // Insert keys in required "other" authories
   flat_set<account_id_type> required_active;
   flat_set<account_id_type> required_owner;
   vector<authority> other;
   trx.get_required_authorities( required_active, required_owner, other );
   for( const auto& auth : other )
      for( const auto& key : auth.get_keys() )
         result.insert( key );

   //idump((result));

   return result;
}

set<address> database_api_impl::get_potential_address_signatures( const signed_transaction& trx )const
{
   set<address> result;
   trx.get_required_signatures(
      _db.get_chain_id(),
      flat_set<public_key_type>(),
      [&]( account_id_type id )
      {
         const auto& auth = id(_db).active;
         for( const auto& k : auth.get_addresses() )
            result.insert(k);
         return &auth;
      },
      [&]( account_id_type id )
      {
         const auto& auth = id(_db).owner;
         for( const auto& k : auth.get_addresses() )
            result.insert(k);
         return &auth;
      },
      _db.get_global_properties().parameters.max_authority_depth
   );
   return result;
}
bool database_api::is_cheap_name(const std::string &name) const
{
   return graphene::chain::is_cheap_name(name);
}
bool database_api::is_object(object_id_type id) const
{
   try
   {
   return my->_db.is_object_exist(id);
} FC_CAPTURE_AND_RETHROW( (id)) }

int32_t database_api::is_authenticator(const account_id_type account,const uint32_t type,const asset_id_type asset_id,const account_id_type author) const
{
    return my->is_authenticator( account, type,asset_id,author);
}
bool database_api::is_notify_info(const account_id_type account) const
{
    return my->is_notify_info( account);
}
vector<asset_id_type>     database_api::get_auth_asset(const account_id_type account,const uint32_t type) const
{
   return my->get_auth_asset( account, type);
}
vector<asset_id_type> database_api_impl::get_auth_asset(const account_id_type account,const uint32_t type) const
{
    try
   {
      const auto issuer = account(_db);    
      return issuer.get_auth_asset(_db);
} FC_CAPTURE_AND_RETHROW( (account)(type)) }

bool database_api_impl::is_notify_info(const account_id_type account_id) const
{ try
   {
      auto &account = account_id(_db);
      if(!account.user_info.valid())
         return false;
      vector<string> admin;
      admin.push_back("admin-notify");
      vector<optional<account_object>> admin_ = lookup_account_names(admin) ;
      FC_ASSERT(admin_.size() == 1, "no admin notify");
      FC_ASSERT(admin_[0].valid(), "no admin notify");
      if(account.options.auth_key != account.user_info->from)
         return false;
      if (admin_[0]->options.auth_key != account.user_info->to)
         return false;
      else
         return true;
} FC_CAPTURE_AND_RETHROW( (account_id)) }
int32_t database_api_impl::is_authenticator(const account_id_type account_id,const uint32_t type,const asset_id_type asset_id,const account_id_type author_id) const
{
   try
   {
      auto &account = account_id(_db);
      auto &author  = author_id(_db);
      auto &ass     = asset_id(_db);
      if(type == 0 || type == 1) //提 充 
      {
         gateway_id_type g_id = author.get_gateway_id(_db);
         auto gateway = g_id(_db);
         if(!gateway.allowed_assets.count(asset_id))
            return false;
         if (type == 0)
            return get_can_withdraw(_db,gateway,account,asset_id);
         else if(type == 1)
            return get_can_deposit(_db,gateway,account,asset_id);
      }
      else if(type == 2 || type == 3) //借 投
      {
         carrier_id_type g_id = author.get_carrier_id(_db);
         auto carrier = g_id(_db);
         if(type == 2)
         {
            if(!ass.is_loan_issued()) return false;
            return bitlender_calc::get_can_loan(_db, carrier, account, asset_id);
         }
         else if(type == 3)
         {
            if(!ass.is_loan_issued()) return false;
            return bitlender_calc::get_can_invest(_db, carrier, account, asset_id);
         }
      }
      else if(type == 4)
      {
         return author_id(_db).is_auth_asset(_db, asset_id);
      }
      else if(type == 5)
      {
         return 2000;
      }
      else if(type == 6 || type == 7) //锁仓，定期活期
      {
         carrier_id_type g_id = author.get_carrier_id(_db);
         auto carrier = g_id(_db);
         if(type == 6)
         {
            return bitlender_calc::get_can_node(_db, carrier, account, asset_id);
         }
         else if(type == 7)
         {          
            return bitlender_calc::get_can_dyfixed(_db, carrier, account, asset_id);
         }
      }
      FC_ASSERT(false, "error type");
      return 0;
} FC_CAPTURE_AND_RETHROW( (account_id)(type)(asset_id)(author_id)) }
bool database_api::verify_authority( const signed_transaction& trx )const
{
   return my->verify_authority( trx );
}

bool database_api_impl::verify_authority( const signed_transaction& trx )const
{
   trx.verify_authority( _db.get_chain_id(),
                         [this]( account_id_type id ){ return &id(_db).active; },
                         [this]( account_id_type id ){ return &id(_db).owner; },
                          _db.get_global_properties().parameters.max_authority_depth );
   return true;
}

bool database_api::verify_account_authority( const string& name_or_id, const flat_set<public_key_type>& signers )const
{
   return my->verify_account_authority( name_or_id, signers );
}

bool database_api_impl::verify_account_authority( const string& name_or_id, const flat_set<public_key_type>& keys )const
{
   FC_ASSERT( name_or_id.size() > 0);
   const account_object* account = nullptr;
   if (std::isdigit(name_or_id[0]))
      account = _db.find(fc::variant(name_or_id, 1).as<account_id_type>(1));
   else
   {
      const auto& idx = _db.get_index_type<account_index>().indices().get<by_name>();
      auto itr = idx.find(name_or_id);
      if (itr != idx.end())
         account = &*itr;
   }
   FC_ASSERT( account, "no such account" );


   /// reuse trx.verify_authority by creating a dummy transfer
   signed_transaction trx;
   transfer_operation op;
   op.from = account->id;
   trx.operations.emplace_back(op);

   return verify_authority( trx );
}

processed_transaction database_api::validate_transaction( const signed_transaction& trx )const
{
   return my->validate_transaction( trx );
}

vector<account_id_type> database_api::vertify_transaction( const signed_transaction& trx )
{
     trx.validate();
     return  my->_db.vertify_transaction(false,trx);
}
vector<account_id_type>   database_api::vertify_account_login(const signed_transaction &trx)
{
     trx.validate();
     return  my->_db.vertify_transaction(true,trx);
}
processed_transaction database_api_impl::validate_transaction( const signed_transaction& trx )const
{
   return _db.validate_transaction(trx);
}
void database_api::validate_proposal( const signed_transaction& trx )const
{
     return my->validate_proposal( trx);
}
void database_api::validate_opertation( const operation &op ,bool is_proposed_trx)const
{
   return my->validate_opertation( op ,is_proposed_trx);
}

void database_api_impl::validate_opertation( const operation &op ,bool is_proposed_trx)const
{
   return _db.validate_opertation(op,is_proposed_trx);
}

void database_api_impl::validate_proposal( const signed_transaction& trx )const
{
    bool _proposal_create = false;
    FC_ASSERT(trx.operations.size() > 0);
    for (auto &proposal : trx.operations) 
    {
       if(proposal.which() != operation::tag<proposal_create_operation>::value)
           continue;
       const auto& op = proposal.get<proposal_create_operation>();
       for (auto &p : op.proposed_ops)
       {
          _db.validate_opertation(p.op, true);
          _proposal_create = true;
       }
    }
    FC_ASSERT(_proposal_create);
}
vector< fc::variant > database_api::get_required_fees( const vector<operation>& ops, asset_id_type id )const
{
   return my->get_required_fees( ops, id );
}

/**
 * Container method for mutually recursive functions used to
 * implement get_required_fees() with potentially nested proposals.
 */
struct get_required_fees_helper
{
   get_required_fees_helper(
      const fee_schedule& _current_fee_schedule,
      const price& _core_exchange_rate,
      uint32_t _max_recursion
      )
      : current_fee_schedule(_current_fee_schedule),
        core_exchange_rate(_core_exchange_rate),
        max_recursion(_max_recursion)
   {}

   fc::variant set_op_fees( operation& op )
   {
      if( op.which() == operation::tag<proposal_create_operation>::value )
      {
         return set_proposal_create_op_fees( op );
      }
      else
      {
         asset fee = current_fee_schedule.set_fee( op, core_exchange_rate );
         fc::variant result;
         fc::to_variant( fee, result, GRAPHENE_NET_MAX_NESTED_OBJECTS );
         return result;
      }
   }

   fc::variant set_proposal_create_op_fees( operation& proposal_create_op )
   {
      proposal_create_operation& op = proposal_create_op.get<proposal_create_operation>();
      std::pair< asset, fc::variants > result;
      for( op_wrapper& prop_op : op.proposed_ops )
      {
         FC_ASSERT( current_recursion < max_recursion );
         ++current_recursion;
         result.second.push_back( set_op_fees( prop_op.op ) );
         --current_recursion;
      }
      // we need to do this on the boxed version, which is why we use
      // two mutually recursive functions instead of a visitor
      result.first = current_fee_schedule.set_fee( proposal_create_op, core_exchange_rate );
      fc::variant vresult;
      fc::to_variant( result, vresult, GRAPHENE_NET_MAX_NESTED_OBJECTS );
      return vresult;
   }

   const fee_schedule& current_fee_schedule;
   const price& core_exchange_rate;
   uint32_t max_recursion;
   uint32_t current_recursion = 0;
};

vector< fc::variant > database_api_impl::get_required_fees( const vector<operation>& ops, asset_id_type id )const
{
   vector< operation > _ops = ops;
   FC_ASSERT(id.is_null()); //eric
   //
   // we copy the ops because we need to mutate an operation to reliably
   // determine its fee, see #435
   //

   vector< fc::variant > result;
   result.reserve(ops.size());
   const asset_object& a = id(_db);
   get_required_fees_helper helper(
      _db.current_fee_schedule(),
      a.options.core_exchange_rate,
      GET_REQUIRED_FEES_MAX_RECURSION );
   for( operation& op : _ops )
   {
      result.push_back( helper.set_op_fees( op ) );
   }
   return result;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Proposed transactions                                            //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<proposal_object> database_api::get_proposed_transactions( account_id_type id )const
{
   return my->get_proposed_transactions( id );
}
flat_map<public_key_type,proposal_object>  database_api::get_proposed_key_transactions( account_id_type id )const
{
    return my->get_proposed_key_transactions( id );
}
/*
for(auto &key : account.active.key_auths)    
       {
           if(author.active.key_auths.find(key.first) != author.active.key_auths.end())
             return true;
            if(author.owner.key_auths.find(key.first) != author.owner.key_auths.end())
             return true;
       }
       for(auto &key : account.owner.key_auths)    
       {
           if(author.active.key_auths.find(key.first) != author.active.key_auths.end())
             return true;
            if(author.owner.key_auths.find(key.first) != author.owner.key_auths.end())
             return true;
       }     
*/
void database_api_impl::is_proposal_key(database &db,const vector<account_id_type> &account_id, account_id_type &author_id,const proposal_object &p,flat_map<public_key_type,proposal_object> &keys, uint32_t &dep) const
{
     dep --;
     if(dep<=0)
      return ;    
     const auto &author = author_id(db);       
     for(auto &a_id : account_id)
     {  
       const auto &account = a_id(db);  
       for(auto &key : author.active.key_auths)    
       {
           if(account.active.key_auths.find(key.first) != account.active.key_auths.end())
             keys[key.first] = p;
           if(account.owner.key_auths.find(key.first) != account.owner.key_auths.end())
             keys[key.first] = p;
       }
       for(auto &key : author.owner.key_auths)    
       {
           if(account.active.key_auths.find(key.first) != account.active.key_auths.end())
               keys[key.first] = p;
            if(account.owner.key_auths.find(key.first) != account.owner.key_auths.end())
               keys[key.first] = p;
       }     
       vector <account_id_type> approvals;
       for(auto &it :account.owner.account_auths) 
           approvals.push_back(it.first);
       for(auto &it :account.active.account_auths) 
           approvals.push_back(it.first);    
        is_proposal_key(db,approvals,author_id,p,keys,dep);
     }

}
bool database_api_impl::is_proposal_author(database &db,const vector<account_id_type> &account_id, account_id_type &author_id,uint32_t &dep) const
{
     dep --;
     if(dep<=0)
      return false;
     if (find(account_id.begin(),account_id.end(), author_id) != account_id.end())
       return true;
     //const auto &author = author_id(db);  
     for(auto &a_id : account_id)
     {  
       const auto &account = a_id(db);  
       if(account.owner.account_auths.find(author_id) != account.owner.account_auths.end())
         return true;
       else if(account.active.account_auths.find(author_id) != account.active.account_auths.end())
         return true;       
       else
       {
           vector <account_id_type> approvals;
           for(auto &it :account.owner.account_auths) 
              approvals.push_back(it.first);
           for(auto &it :account.active.account_auths) 
              approvals.push_back(it.first);    
            if(is_proposal_author(db,approvals,author_id,dep))  
            return true;           
       }  
     }
     return false;
}
/** TODO: add secondary index that will accelerate this process */
flat_map<public_key_type,proposal_object> database_api_impl::get_proposed_key_transactions( account_id_type id )const
{ try{
    flat_map<public_key_type,proposal_object> result;
    const auto& idx = _db.get_index_type<proposal_index>();
   uint32_t udep  = _db.get_global_properties().parameters.maximum_authority_membership;

   idx.inspect_all_objects([&](const object &obj) {
       const proposal_object &p = static_cast<const proposal_object &>(obj);           
           vector <account_id_type> approvals;
           for(auto &it  : p.required_active_approvals) 
              approvals.push_back(it);
            for(auto &it : p.required_owner_approvals) 
              approvals.push_back(it);
            for(auto &it : p.available_active_approvals) 
              approvals.push_back(it);
           is_proposal_key(_db,approvals,id,p,result,udep);                                   
      });
   return result;

    return result;
} FC_CAPTURE_AND_RETHROW( (id)) }
vector<proposal_object> database_api_impl::get_proposed_transactions( account_id_type id )const
{ try{
   const auto& idx = _db.get_index_type<proposal_index>();
   vector<proposal_object> result;

   uint32_t udep  = _db.get_global_properties().parameters.maximum_authority_membership;
  
   idx.inspect_all_objects([&](const object &obj) {
       const proposal_object &p = static_cast<const proposal_object &>(obj);
           if(id.is_null())
           {
              result.push_back(p);
              return;
           }
           vector <account_id_type> approvals;
           for(auto &it  : p.required_active_approvals) 
              approvals.push_back(it);
            for(auto &it : p.required_owner_approvals) 
              approvals.push_back(it);
            for(auto &it : p.available_active_approvals) 
              approvals.push_back(it);
           if (is_proposal_author(_db,approvals,id,udep))                    
               result.push_back(p);           
      });
   if(result.size()>0)   
   {
       std::sort(result.begin(), result.end(), [&](const proposal_object &a, const proposal_object &b) -> bool {
           return a.id > b.id;
       });
   } 
   return result;
} FC_CAPTURE_AND_RETHROW( (id)) }
//////////////////////////////////////////////////////////////////////
//                                                                  //
// Proposed transactions                                            //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<proposal_object> database_api::get_my_proposed_transactions( account_id_type id)const
{
   return my->get_my_proposed_transactions( id);
}

/** TODO: add secondary index that will accelerate this process */
vector<proposal_object> database_api_impl::get_my_proposed_transactions( account_id_type id)const
{
   const auto& idx = _db.get_index_type<proposal_index>();
   vector<proposal_object> result;

   idx.inspect_all_objects([&](const object &obj) {
       const proposal_object &p = static_cast<const proposal_object &>(obj);
       
           if (p.proposer == id || id .is_null())
               result.push_back(p);
      
   });
   return result;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Blinded balances                                                 //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<blinded_balance_object> database_api::get_blinded_balances( const flat_set<commitment_type>& commitments )const
{
   return my->get_blinded_balances( commitments );
}

vector<blinded_balance_object> database_api_impl::get_blinded_balances( const flat_set<commitment_type>& commitments )const
{
   vector<blinded_balance_object> result; result.reserve(commitments.size());
   const auto& bal_idx = _db.get_index_type<blinded_balance_index>();
   const auto& by_commitment_idx = bal_idx.indices().get<by_commitment>();
   for( const auto& c : commitments )
   {
      auto itr = by_commitment_idx.find( c );
      if( itr != by_commitment_idx.end() )
         result.push_back( *itr );
   }
   return result;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//  Withdrawals                                                     //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<withdraw_permission_object> database_api::get_withdraw_permissions_by_giver(account_id_type account, withdraw_permission_id_type start, uint32_t limit)const
{
   return my->get_withdraw_permissions_by_giver( account, start, limit );
}

vector<withdraw_permission_object> database_api_impl::get_withdraw_permissions_by_giver(account_id_type account, withdraw_permission_id_type start, uint32_t limit)const
{
   FC_ASSERT( limit <= 101 );
   vector<withdraw_permission_object> result;

   const auto& withdraw_idx = _db.get_index_type<withdraw_permission_index>().indices().get<by_from>();
   auto withdraw_index_end = withdraw_idx.end();
   auto withdraw_itr = withdraw_idx.lower_bound(boost::make_tuple(account, start));
   while(withdraw_itr != withdraw_index_end && withdraw_itr->withdraw_from_account == account && result.size() < limit)
   {
      result.push_back(*withdraw_itr);
      ++withdraw_itr;
   }
   return result;
}

vector<withdraw_permission_object> database_api::get_withdraw_permissions_by_recipient(account_id_type account, withdraw_permission_id_type start, uint32_t limit)const
{
   return my->get_withdraw_permissions_by_recipient( account, start, limit );
}

vector<withdraw_permission_object> database_api_impl::get_withdraw_permissions_by_recipient(account_id_type account, withdraw_permission_id_type start, uint32_t limit)const
{
   FC_ASSERT( limit <= 101 );
   vector<withdraw_permission_object> result;

   const auto& withdraw_idx = _db.get_index_type<withdraw_permission_index>().indices().get<by_authorized>();
   auto withdraw_index_end = withdraw_idx.end();
   auto withdraw_itr = withdraw_idx.lower_bound(boost::make_tuple(account, start));
   while(withdraw_itr != withdraw_index_end && withdraw_itr->authorized_account == account && result.size() < limit)
   {
      result.push_back(*withdraw_itr);
      ++withdraw_itr;
   }
   return result;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Private methods                                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

void database_api_impl::broadcast_updates( const vector<variant>& updates )
{
   if( updates.size() && _subscribe_callback ) {
      auto capture_this = shared_from_this();
      fc::async([capture_this,updates](){
          if(capture_this->_subscribe_callback)
            capture_this->_subscribe_callback( fc::variant(updates) );
      });
   }
}

void database_api_impl::broadcast_market_updates( const market_queue_type& queue)
{
   if( queue.size() )
   {
      auto capture_this = shared_from_this();
      fc::async([capture_this, this, queue](){
          for( const auto& item : queue )
          {
            auto sub = _market_subscriptions.find(item.first);
            if( sub != _market_subscriptions.end() )
                sub->second( fc::variant(item.second ) );
          }
      });
   }
}
void database_api_impl::broadcast_bitlender_updates( const bitlender_queue_type& queue)
{
   if( queue.size() )
   {
      auto capture_this = shared_from_this();
      fc::async([capture_this, this, queue](){
          for( const auto& item : queue )
          {
            auto sub = _bitlender_subscriptions.find(item.first);
            if( sub != _bitlender_subscriptions.end() )
                sub->second( fc::variant(item.second ) );
          }
      });
   }
}

void database_api_impl::on_objects_removed( const vector<object_id_type>& ids, const vector<const object*>& objs, const flat_set<account_id_type>& impacted_accounts)
{
   handle_object_changed(_notify_remove_create, false, ids, impacted_accounts,
      [objs](object_id_type id) -> const object* {
         auto it = std::find_if(
               objs.begin(), objs.end(),
               [id](const object* o) {return o != nullptr && o->id == id;});

         if (it != objs.end())
            return *it;

         return nullptr;
      }
   );
}

void database_api_impl::on_objects_new(const vector<object_id_type>& ids, const flat_set<account_id_type>& impacted_accounts)
{
   handle_object_changed(_notify_remove_create, true, ids, impacted_accounts,
      std::bind(&object_database::find_object, &_db, std::placeholders::_1)
   );
}

void database_api_impl::on_objects_changed(const vector<object_id_type>& ids, const flat_set<account_id_type>& impacted_accounts)
{
   handle_object_changed(false, true, ids, impacted_accounts,
      std::bind(&object_database::find_object, &_db, std::placeholders::_1)
   );
}

void database_api_impl::handle_object_changed(bool force_notify, bool full_object, const vector<object_id_type>& ids, const flat_set<account_id_type>& impacted_accounts, std::function<const object*(object_id_type id)> find_object)
{
   if( _subscribe_callback )
   {
      vector<variant> updates;

      for(auto id : ids)
      {
         if( force_notify || is_subscribed_to_item(id) || is_impacted_account(impacted_accounts) )
         {
            if( full_object )
            {
               auto obj = find_object(id);
               if( obj )
               {
                  updates.emplace_back( obj->to_variant() );
               }
            }
            else
            {
               updates.emplace_back( fc::variant( id, 1 ) );
            }
         }
      }

      if( updates.size() )
         broadcast_updates(updates);
   }
    //交易记录
   if( _market_subscriptions.size() )
   {
      market_queue_type broadcast_queue;

      for(auto id : ids)
      {
         if( id.is<call_order_object>() )
         {
            enqueue_if_subscribed_to_market<call_order_object>( find_object(id), broadcast_queue, full_object );
         }
         else if( id.is<limit_order_object>() )
         {
            enqueue_if_subscribed_to_market<limit_order_object>( find_object(id), broadcast_queue, full_object );
         }
         else if( id.is<force_settlement_object>() )
         {
            enqueue_if_subscribed_to_market<force_settlement_object>( find_object(id), broadcast_queue, full_object );
         }
          
      }
      if( broadcast_queue.size() )
         broadcast_market_updates(broadcast_queue);
   }
   if( _bitlender_subscriptions.size() )
   {
      bitlender_queue_type broadcast_queue;

      for(auto id : ids)
      {
         if( id.is<bitlender_order_object>() )
         {
            enqueue_if_subscribed_to_bitlender<bitlender_order_object>( find_object(id), broadcast_queue, full_object );
         }
         else if( id.is<bitlender_invest_object>() )
         {
            enqueue_if_subscribed_to_bitlender<bitlender_invest_object>( find_object(id), broadcast_queue, full_object );
         }
      }
      if( broadcast_queue.size() )
         broadcast_bitlender_updates(broadcast_queue);
   }
}

/** note: this method cannot yield because it is called in the middle of
 * apply a block.
 */
void database_api_impl::on_applied_block()
{
   if (_block_applied_callback)
   {
      auto capture_this = shared_from_this();
      block_id_type block_id = _db.head_block_id();
      fc::async([this,capture_this,block_id](){
         _block_applied_callback(fc::variant(block_id, 1));
      });
   }

   if(_market_subscriptions.size() == 0)
      return;

   const auto& ops = _db.get_applied_operations();
   map< std::pair<asset_id_type,asset_id_type>, vector<pair<operation, operation_result>> > subscribed_markets_ops;
   map< std::pair<asset_id_type,asset_id_type>, vector<pair<operation, operation_result>> > subscribed_bitlenders_ops;
   for(const optional< operation_history_object >& o_op : ops)
   {
      if( !o_op.valid() )
         continue;
      const operation_history_object& op = *o_op;

      optional< std::pair<asset_id_type,asset_id_type> > market;
      optional< std::pair<asset_id_type,asset_id_type> > bitlender;
      switch(op.op.which())
      {
         /*  This is sent via the object_changed callback
         case operation::tag<limit_order_create_operation>::value:
            market = op.op.get<limit_order_create_operation>().get_market();
            break;
         */
         case operation::tag<fill_order_operation>::value:
            market = op.op.get<fill_order_operation>().get_market();
            break;
            /*
         case operation::tag<limit_order_cancel_operation>::value:
         */
         default: break;
      }
      if( market.valid() && _market_subscriptions.count(*market) )
         // FIXME this may cause fill_order_operation be pushed before order creation
         subscribed_markets_ops[*market].emplace_back(std::make_pair( op.op, op.result )  );
      if( bitlender.valid() && _bitlender_subscriptions.count(*bitlender) )
         // FIXME this may cause fill_order_operation be pushed before order creation
         subscribed_bitlenders_ops[*bitlender].emplace_back( std::make_pair( op.op, op.result ) );
   }
   /// we need to ensure the database_api is not deleted for the life of the async operation
   auto capture_this = shared_from_this();
   fc::async([this,capture_this,subscribed_markets_ops](){
      for(auto item : subscribed_markets_ops)
      {
         auto itr = _market_subscriptions.find(item.first);
         if(itr != _market_subscriptions.end())
            itr->second(fc::variant(item.second, GRAPHENE_NET_MAX_NESTED_OBJECTS));
      }
   });

    fc::async([this,capture_this,subscribed_bitlenders_ops](){
      for(auto item : subscribed_bitlenders_ops)
      {
         auto itr = _bitlender_subscriptions.find(item.first);
         if(itr != _bitlender_subscriptions.end())
            itr->second(fc::variant(item.second, GRAPHENE_NET_MAX_NESTED_OBJECTS));
      }
   });


}

} } // graphene::app
