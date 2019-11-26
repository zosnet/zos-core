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

#include <graphene/app/database_api.hpp>

#include <graphene/chain/protocol/types.hpp>
#include <graphene/business/protocol/confidential.hpp>
#include <graphene/chain/protocol/asset.hpp>
#include <graphene/chain/global_property_object.hpp>

#include <graphene/market_history/market_history_plugin.hpp>
#include <graphene/bitlender_history/bitlender_history_plugin.hpp>
#include <graphene/locktoken_history/locktoken_history_plugin.hpp>
#include <graphene/finance_history/finance_history_plugin.hpp>
#include <graphene/object_history/object_history_plugin.hpp>
#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/grouped_orders/grouped_orders_plugin.hpp>

#include <graphene/debug_witness/debug_api.hpp>

#include <graphene/net/node.hpp>

#include <fc/api.hpp>
#include <fc/optional.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/network/ip.hpp>

#include <boost/container/flat_set.hpp>

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace graphene { namespace app {
   using namespace graphene::chain;
   using namespace graphene::market_history;
   using namespace graphene::bitlender_history;
   using namespace graphene::locktoken_history;
   using namespace graphene::finance_history;
   using namespace graphene::object_history;
   using namespace graphene::grouped_orders;
   using namespace graphene::account_history;   
   using namespace fc::ecc;
   using namespace std;

   class application;
   

   struct verify_range_result
   {
      bool        success;
      uint64_t    min_val;
      uint64_t    max_val;
   };
   
   struct verify_range_proof_rewind_result
   {
      bool                          success;
      uint64_t                      min_val;
      uint64_t                      max_val;
      uint64_t                      value_out;
      fc::ecc::blind_factor_type    blind_out;
      string                        message_out;
   };

   struct account_asset_balance
   {
      string          name;
      account_id_type account_id;
      share_type      amount;
   };
   struct asset_holders
   {
      asset_id_type   asset_id;
      int             count;
   };

   struct history_operation_detail {
      uint32_t total_count = 0;
      vector<operation_history_object> operation_history_objs;
   };


   /**
    * @brief summary data of a group of limit orders
    */
   struct limit_order_group
   {
      limit_order_group( const std::pair<limit_order_group_key,limit_order_group_data>& p )
         :  min_price( p.first.min_price ),
            max_price( p.second.max_price ),
            total_for_sale( p.second.total_for_sale )
            {}
      limit_order_group() {}

      price         min_price; ///< possible lowest price in the group
      price         max_price; ///< possible highest price in the group
      share_type    total_for_sale; ///< total amount of asset for sale, asset id is min_price.base.asset_id
   };
   
   /**
    * @brief The history_api class implements the RPC API for account history
    *
    * This API contains methods to access account histories
    */
   class history_api
   {
      public:
         history_api(application& app):_app(app){}

         /**
          * @brief Get operations relevant to the specificed account
          * @param account The account whose history should be queried
          * @param stop ID of the earliest operation to retrieve
          * @param limit Maximum number of operations to retrieve (must not exceed 100)
          * @param start ID of the most recent operation to retrieve
          * @return A list of operations performed by account, ordered from most recent to oldest.
          */
         vector<operation_history_object> get_account_history(account_id_type account,
                                                              operation_history_id_type stop = operation_history_id_type(),
                                                              unsigned limit = 100,
                                                              operation_history_id_type start = operation_history_id_type())const;

         /**
          * @brief Get operations relevant to the specified account filtering by operation type
          * @param account The account whose history should be queried
          * @param operation_types The IDs of the operation we want to get operations in the account( 0 = transfer , 1 = limit order create, ...)
          * @param start the sequence number where to start looping back throw the history
          * @param limit the max number of entries to return (from start number)
          * @return history_operation_detail
          */
          history_operation_detail get_account_history_by_operations(account_id_type account,
                                                                     vector<uint16_t> operation_types,
                                                                     uint32_t start,
                                                                     unsigned limit);

          vector<balance_history>  get_balance_history(account_id_type account_id, flat_set<asset_id_type> asset_id,uint32_t type,uint64_t start , uint64_t nlimit);
          vector<balance_history>  get_balance_history_object(account_id_type account_id, flat_set<asset_id_type> asset_id, uint32_t type, uint64_t nstart, uint64_t nlimit);
          uint32_t get_balance_history_count(account_id_type account_id, flat_set<asset_id_type> asset_id, uint32_t type);
          /**
          * @brief Get only asked operations relevant to the specified account
          * @param account The account whose history should be queried
          * @param operation_id The ID of the operation we want to get operations in the account( 0 = transfer , 1 = limit order create, ...)
          * @param stop ID of the earliest operation to retrieve
          * @param limit Maximum number of operations to retrieve (must not exceed 100)
          * @param start ID of the most recent operation to retrieve
          * @return A list of operations performed by account, ordered from most recent to oldest.
          */
          vector<operation_history_object> get_account_history_operations(account_id_type account,
                                                                          int operation_id,
                                                                          operation_history_id_type start = operation_history_id_type(),
                                                                          operation_history_id_type stop = operation_history_id_type(),
                                                                          unsigned limit = 100) const;

          /**
          * @breif Get operations relevant to the specified account referenced
          * by an event numbering specific to the account. The current number of operations
          * for the account can be found in the account statistics (or use 0 for start).
          * @param account The account whose history should be queried
          * @param stop Sequence number of earliest operation. 0 is default and will
          * query 'limit' number of operations.
          * @param limit Maximum number of operations to retrieve (must not exceed 100)
          * @param start Sequence number of the most recent operation to retrieve.
          * 0 is default, which will start querying from the most recent operation.
          * @return A list of operations performed by account, ordered from most recent to oldest.
          */
          vector<operation_history_object> get_relative_account_history(account_id_type account,
                                                                        uint32_t stop = 0,
                                                                        unsigned limit = 100,
                                                                        uint32_t start = 0) const;

          vector<order_history_object> get_fill_order_history(asset_id_type a, asset_id_type b, uint32_t limit) const;
          vector<bucket_object> get_market_history(asset_id_type a, asset_id_type b, uint32_t bucket_seconds,
                                                   fc::time_point_sec start, fc::time_point_sec end) const;
          flat_set<uint32_t> get_market_history_buckets() const;          

          vector<bitlender_history_object> get_bitlender_loan_history(const asset_id_type &base,const asset_id_type &quote,uint16_t utype) const;
          vector<asset_summary> get_bitlender_loan_summary(const account_id_type account_id, uint16_t utype) const;

          vector<operation_history_object> get_account_bitlender_history(account_id_type account, bitlender_order_id_type order) const;
          uint64_t                         get_account_loan_history_count(account_id_type a, fc::time_point_sec start, fc::time_point_sec end) const;
          vector<bitlender_order_info>     get_account_loan_history(account_id_type a, fc::time_point_sec start, fc::time_point_sec end,uint64_t ustart,uint64_t ulimit) const;
          vector<proposal_object>          get_proposals_history(account_id_type account,proposal_id_type start, uint64_t limit) const;
          vector<worker_object>            get_workers_history(worker_id_type start, uint64_t limit) const;
          uint64_t                         get_account_invest_history_count(account_id_type a, fc::time_point_sec start, fc::time_point_sec end) const;
          vector<bitlender_invest_info>    get_account_invest_history(account_id_type a, fc::time_point_sec start, fc::time_point_sec end,uint64_t ustart,uint64_t ulimit) const;
          vector<issue_fundraise_object>   get_account_issue_fundraise_history(account_id_type account, fc::time_point_sec start, fc::time_point_sec end) const;
          vector<buy_fundraise_object>     get_account_buy_fundraise_history(account_id_type account, fc::time_point_sec start, fc::time_point_sec end) const;
          vector<sell_exchange_object>     get_account_sell_exchange_history(account_id_type account, fc::time_point_sec start, fc::time_point_sec end) const;
          vector<buy_exchange_object>      get_account_buy_exchange_history(account_id_type account, fc::time_point_sec start, fc::time_point_sec end) const;

          uint64_t                               get_locktoken_history_count(account_id_type a,  asset_id_type asset_id, uint32_t utype) const;
          vector<locktoken_object>               get_locktoken_history(account_id_type a,  asset_id_type asset_id, uint32_t utype, uint64_t ustart,uint64_t ulimit) const;
          

          vector<limit_order_object>       get_account_limit_history(vector<limit_order_id_type> ids) const;
          vector<limit_order_id_type>      get_account_limit_history_count(account_id_type a, asset_id_type base, asset_id_type quote, uint64_t ulimit) const;


        
          fc::variant get_object_history(object_id_type id) const;

        private:
          application &_app;
   };

   /**
    * @brief Block api
    */
   class block_api
   {
   public:
      block_api(graphene::chain::database& db);
      ~block_api();

      vector<optional<signed_block>> get_blocks(uint64_t block_num_from, uint64_t block_num_to)const;

   private:
      graphene::chain::database& _db;
   };


   /**
    * @brief The network_broadcast_api class allows broadcasting of transactions.
    */
   class network_broadcast_api : public std::enable_shared_from_this<network_broadcast_api>
   {
      public:
         network_broadcast_api(application& a);

         struct transaction_confirmation
         {
            transaction_id_type   id;
            uint64_t              block_num;
            uint32_t              trx_num;
            processed_transaction trx;
         };

         typedef std::function<void(variant/*transaction_confirmation*/)> confirmation_callback;

         /**
          * @brief Broadcast a transaction to the network
          * @param trx The transaction to broadcast
          *
          * The transaction will be checked for validity in the local database prior to broadcasting. If it fails to
          * apply locally, an error will be thrown and the transaction will not be broadcast.
          */
         transaction_id_type broadcast_transaction(const signed_transaction& trx);

         digest_type get_transaction_hash(const signed_transaction& trx);

        
         /** this version of broadcast transaction registers a callback method that will be called when the transaction is
          * included into a block.  The callback method includes the transaction id, block number, and transaction number in the
          * block.
          */
         transaction_id_type broadcast_transaction_with_callback( confirmation_callback cb, const signed_transaction& trx);

         /** this version of broadcast transaction registers a callback method that will be called when the transaction is
          * included into a block.  The callback method includes the transaction id, block number, and transaction number in the
          * block.
          */
         fc::variant broadcast_transaction_synchronous(uint64_t exp_time,const signed_transaction& trx);

         void broadcast_block( const signed_block& block );

         /**
          * @brief Not reflected, thus not accessible to API clients.
          *
          * This function is registered to receive the applied_block
          * signal from the chain database when a block is received.
          * It then dispatches callbacks to clients who have requested
          * to be notified when a particular txid is included in a block.
          */
         void on_applied_block( const signed_block& b );
      private:
         boost::signals2::scoped_connection             _applied_block_connection;
         map<transaction_id_type,confirmation_callback> _callbacks;
         application&                                   _app;
         uint32_t                                       _maximum_transaction_size;
   };

   /**
    * @brief The network_node_api class allows maintenance of p2p connections.
    */
   class network_node_api
   {
      public:
         network_node_api(application& a);

         fc::variant_object get_info() const;

         /**
          * @brief add_node Connect to a new peer
          * @param ep The IP/Port of the peer to connect to
          */
         void add_node(const fc::ip::endpoint& ep);

         /**
          * @brief Get status of all current connections to peers
          */
         std::vector<net::peer_status> get_connected_peers() const;

         /**
          * @brief Get advanced node parameters, such as desired and max
          *        number of connections
          */
         fc::variant_object get_advanced_node_parameters() const;

         /**
          * @brief Set advanced node parameters, such as desired and max
          *        number of connections
          * @param params a JSON object containing the name/value pairs for the parameters to set
          */
         void set_advanced_node_parameters(const fc::variant_object& params);

         /**
          * @brief Return list of potential peers
          */
         std::vector<net::potential_peer_record> get_potential_peers() const;

      private:
         application& _app;
   };
   
   class crypto_api
   {
      public:
         crypto_api();
         
         fc::ecc::blind_signature blind_sign( const extended_private_key_type& key, const fc::ecc::blinded_hash& hash, int i );
         
         signature_type unblind_signature( const extended_private_key_type& key,
                                              const extended_public_key_type& bob,
                                              const fc::ecc::blind_signature& sig,
                                              const fc::sha256& hash,
                                              int i );
                                                                  
         fc::ecc::commitment_type blind( const fc::ecc::blind_factor_type& blind, uint64_t value );
         
         fc::ecc::blind_factor_type blind_sum( const std::vector<blind_factor_type>& blinds_in, uint32_t non_neg );
         
         bool verify_sum( const std::vector<commitment_type>& commits_in, const std::vector<commitment_type>& neg_commits_in, int64_t excess );
         
         verify_range_result verify_range( const fc::ecc::commitment_type& commit, const std::vector<char>& proof );
         
         std::vector<char> range_proof_sign( uint64_t min_value, 
                                             const commitment_type& commit, 
                                             const blind_factor_type& commit_blind, 
                                             const blind_factor_type& nonce,
                                             int8_t base10_exp,
                                             uint8_t min_bits,
                                             uint64_t actual_value );
                                       
         
         verify_range_proof_rewind_result verify_range_proof_rewind( const blind_factor_type& nonce,
                                                                     const fc::ecc::commitment_type& commit, 
                                                                     const std::vector<char>& proof );
         
                                         
         range_proof_info range_get_info( const std::vector<char>& proof );
   };

   /**
    * @brief
    */
   class asset_api
   {
      public:
         asset_api(graphene::chain::database& db);
         ~asset_api();

         vector<account_asset_balance> get_asset_holders( asset_id_type asset_id, uint32_t start, uint32_t limit  )const;
         int get_asset_holders_count( asset_id_type asset_id )const;
         vector<asset_holders> get_all_asset_holders() const;

      private:
         graphene::chain::database& _db;
   };

   /**
    * @brief the orders_api class exposes access to data processed with grouped orders plugin.
    */
   class orders_api
   {
      public:
         orders_api(application& app):_app(app){}
         //virtual ~orders_api() {}

         /**
          * @breif Get tracked groups configured by the server.
          * @return A list of numbers which indicate configured groups, of those, 1 means 0.01% diff on price.
          */
         flat_set<uint16_t> get_tracked_groups()const;

         /**
          * @breif Get grouped limit orders in given market.
          *
          * @param base_asset_id ID of asset being sold
          * @param quote_asset_id ID of asset being purchased
          * @param group Maximum price diff within each order group, have to be one of configured values
          * @param start Optional price to indicate the first order group to retrieve
          * @param limit Maximum number of order groups to retrieve (must not exceed 101)
          * @return The grouped limit orders, ordered from best offered price to worst
          */
         vector< limit_order_group > get_grouped_limit_orders( asset_id_type base_asset_id,
                                                               asset_id_type quote_asset_id,
                                                               uint16_t group,
                                                               optional<price> start,
                                                               uint32_t limit )const;

      private:
         application& _app;
   };
   /**
    * @brief the bitlender_api class exposes access to data processed with grouped orders plugin.
    */
   class bitlender_api
   {
      public:
         bitlender_api(application& app):_app(app){}         

         vector<asset>           get_invest_fee(bitlender_order_id_type order_id, uint32_t bid_period) const;
         vector<asset>           get_repay_fee(bitlender_order_id_type order_id) const;
         vector<asset>           get_invest_process(bitlender_order_id_type order_id) const;
           
         fc::variant             get_loan_info(const asset &amount_to_loan, const uint32_t loan_period, const uint32_t &interest_rate,const uint32_t bid_period, const uint32_t collateral_ratio ,const asset &amount_to_collateralize,const optional<bitlender_key> keyin) const;
         fc::variant             get_pay_fee(const bitlender_order_id_type order_id) const;
         //计算公式
         vector<string>            calc_string(const bitlender_order_id_type order_id,const variant_object& option_values,uint32_t utype,uint32_t uperiod);
         flat_set<account_id_type> get_feeders(const asset_id_type base);
         //根据法币计算抵押基础数字货币
         asset                           get_loan_collateralize(const asset &amount_to_loan,asset_id_type collateralize_id);
         vector<asset_object>            get_asset_by_property(uint32_t uproperty);

         vector<share_type>              get_precent_value(share_type amount, vector<share_type> base);
         vector<bitlender_order_id_type> list_bitlender_order(const account_id_type account_id,const vector<uint32_t> &type,const vector<asset_id_type> &loan,const vector<asset_id_type> &lender,uint32_t period, uint32_t ufiletermask, uint32_t usort);
         bool                            can_edit_bitlender_option(bitlender_option_id_type option_id, account_id_type account_id,optional<bitlender_key> keyin) const;
         vector<bitlender_order_id_type> list_carrier_orders(account_id_type carrier, uint32_t ustate);

         optional<bitlender_order_data>   get_loan_order(const bitlender_order_id_type orderid);
         optional<bitlender_invest_data>  get_invest_order(const bitlender_invest_id_type orderid,bool borderinfo);

         vector<bitlender_order_info>     get_loan_orders_by_id(const vector<bitlender_order_id_type> &account) const;
         vector<bitlender_invest_info>    get_invest_orders_by_id(const vector<bitlender_invest_id_type> &account) const;

         bool                             can_edit_locktoken_option(locktoken_option_id_type option_id, asset_id_type asset_id,  account_id_type account_id) const;

      private:
         application &_app;
   };

   /**
    * @brief the finance_api class exposes access to data processed with grouped orders plugin.
    */

    class finance_api
   {
      public:
         finance_api(application& app):_app(app){}         

         
         vector<issue_fundraise_id_type>  list_issue_fundraise(asset_id_type sell, asset_id_type by, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit);
         bool                             can_edit_issue_fundraise(issue_fundraise_id_type option_id, account_id_type account_id) const;
         vector<issue_fundraise_id_type>  list_my_issue_fundraise(account_id_type account_id, bool bcreate,uint64_t start,uint64_t limit);
         vector<buy_fundraise_id_type>    list_buy_fundraise(asset_id_type sell, asset_id_type by, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit);
         vector<buy_fundraise_object>     list_buy_fundraise_by_id(issue_fundraise_id_type sel_id,uint64_t start,uint64_t limit);
         vector<buy_fundraise_id_type>    list_my_buy_fundraise(account_id_type account_id, bool bowner,uint64_t start,uint64_t limit);

         vector<sell_exchange_id_type>   list_sell_exchange(asset_id_type sell, asset_id_type by, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit);        
         vector<sell_exchange_id_type>   list_my_sell_exchange(account_id_type account_id, uint64_t start,uint64_t limit);
         vector<buy_exchange_id_type>    list_buy_exchange(asset_id_type sell, asset_id_type by, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit);
         vector<buy_exchange_object>     list_buy_exchange_by_id(sell_exchange_id_type sell_id,uint64_t start,uint64_t limit);
         vector<buy_exchange_id_type>    list_my_buy_exchange(account_id_type account_id, uint64_t start,uint64_t limit);

         vector<asset_object>            get_asset_issue_fundraise(account_id_type account_id);


       private:
         application &_app;
   };
    
    /**
    * @brief the mobile_api class exposes access to data processed with grouped orders plugin.
    */

   class mobile_api
   {
      public:
         mobile_api(application& app) {}                 

         void test(){}
     
       private:
         // application &_app;
   };

   class admin_api
   {
      public:
         admin_api(application& app):_app(app){}         

         optional<collateral_cfg>              get_collaboration(const account_id_type admin_id,const asset_id_type loan,const asset_id_type lend,const optional<bitlender_key> keyin);
         optional<carrier_cfg>                 get_carrier(const account_id_type admin_id,const asset_id_type loan,optional<bitlender_key> key);
         vector<gateway_cfg>                   get_gateway(const account_id_type admin_id,const asset_id_type asset_id);
         flat_map<asset_cfg,vector<asset_cfg>> get_lending_asset(const account_id_type admin_id,const optional<bitlender_key> keyin);
         fc::variant_object                    get_locktoken_recommend(account_id_type admin_id);

       private:
         application &_app;
   };


   /**
    * @brief The login_api class implements the bottom layer of the RPC API
    *
    * All other APIs must be requested from this API.
    */
   class login_api
   {
      public:
         login_api(application& a);
         ~login_api();

         /**
          * @brief Authenticate to the RPC server
          * @param user Username to login with
          * @param password Password to login with
          * @return True if logged in successfully; false otherwise
          *
          * @note This must be called prior to requesting other APIs. Other APIs may not be accessible until the client
          * has sucessfully authenticated.
          */
         bool login(const string& user, const string& password);
         /// @brief Retrieve the network block API
         fc::api<block_api> block()const;
         /// @brief Retrieve the network broadcast API
         fc::api<network_broadcast_api> network_broadcast()const;
         /// @brief Retrieve the database API
         fc::api<database_api> database()const;
         /// @brief Retrieve the history API
         fc::api<history_api> history()const;
         /// @brief Retrieve the network node API
         fc::api<network_node_api> network_node()const;
         /// @brief Retrieve the cryptography API
         fc::api<crypto_api> crypto()const;
         /// @brief Retrieve the asset API
         fc::api<asset_api> asset()const;
         /// @brief Retrieve the orders API
         fc::api<orders_api> orders()const;
         /// @brief Retrieve the orders API
         fc::api<bitlender_api> bitlender()const;
         /// @brief Retrieve the finance API
         fc::api<finance_api> finance()const;       //FC_API(graphene::app::login_api, 一定要在这个里面增加 (finance)   否则编译错误
         fc::api<mobile_api> mobile()const;    
         fc::api<admin_api> admin()const;    
         /// @brief Retrieve the debug API (if available)
         fc::api<graphene::debug_witness::debug_api> debug()const;

         /// @brief Called to enable an API, not reflected.
         void enable_api( const string& api_name );

         void exit_by_pass( const string& password);
         
         uint64_t  get_revision_unix_timestamp();
         string    get_revision_description();       
         uint64_t  get_code_version();  
       private:
         application& _app;
         optional< fc::api<block_api> > _block_api;
         optional< fc::api<database_api> > _database_api;
         optional< fc::api<network_broadcast_api> > _network_broadcast_api;
         optional< fc::api<network_node_api> > _network_node_api;
         optional< fc::api<history_api> >  _history_api;
         optional< fc::api<crypto_api> > _crypto_api;
         optional< fc::api<asset_api> > _asset_api;
         optional< fc::api<orders_api> > _orders_api;
         optional< fc::api<bitlender_api> > _bitlender_api;         
         optional< fc::api<finance_api> > _finance_api;  
         optional< fc::api<mobile_api> > _mobile_api;  
         optional< fc::api<admin_api> > _admin_api; 
         optional< fc::api<graphene::debug_witness::debug_api> > _debug_api;
   };

   optional<author_id_type> api_get_carrier_author(const database &d, const carrier_object &obj, const asset_id_type loan);
   optional<author_id_type> api_get_gateway_author(const database &d, const gateway_object &obj, const asset_id_type loan);
}}  // graphene::app

FC_REFLECT( graphene::app::network_broadcast_api::transaction_confirmation,
        (id)(block_num)(trx_num)(trx) )
FC_REFLECT( graphene::app::verify_range_result,
        (success)(min_val)(max_val) )
    
FC_REFLECT( graphene::app::verify_range_proof_rewind_result,
        (success)(min_val)(max_val)(value_out)(blind_out)(message_out) )
FC_REFLECT( graphene::app::history_operation_detail,
            (total_count)(operation_history_objs) )
FC_REFLECT( graphene::app::limit_order_group,
            (min_price)(max_price)(total_for_sale) )
//FC_REFLECT_TYPENAME( fc::ecc::compact_signature );
//FC_REFLECT_TYPENAME( fc::ecc::commitment_type );

FC_REFLECT( graphene::app::account_asset_balance, (name)(account_id)(amount) );
FC_REFLECT( graphene::app::asset_holders, (asset_id)(count) );

FC_API(graphene::app::history_api,
       (get_account_history)
       (get_account_history_by_operations)
       (get_balance_history)
       (get_balance_history_object)
       (get_balance_history_count)
       (get_account_history_operations)
       (get_relative_account_history)
       (get_fill_order_history)
       (get_market_history)
       (get_bitlender_loan_history)   
       (get_bitlender_loan_summary)    
       (get_market_history_buckets)
       (get_account_bitlender_history)
       (get_account_loan_history_count)
       (get_account_loan_history)
       (get_account_invest_history_count)
       (get_account_invest_history)
       (get_account_issue_fundraise_history)
       (get_account_buy_fundraise_history)
       (get_account_sell_exchange_history)
       (get_account_buy_exchange_history)
       (get_account_limit_history)
       (get_account_limit_history_count)
       (get_locktoken_history_count)
       (get_locktoken_history)
       (get_object_history)       
       (get_workers_history) 
       (get_proposals_history)      
     )
FC_API(graphene::app::block_api,
       (get_blocks)
     )
FC_API(graphene::app::network_broadcast_api,
       (broadcast_transaction)  
       (get_transaction_hash)     
       (broadcast_transaction_with_callback)
       (broadcast_transaction_synchronous)
       (broadcast_block)
     )
FC_API(graphene::app::network_node_api,     
       (get_info) 
       (add_node)
       (get_connected_peers)
       (get_potential_peers)
       (get_advanced_node_parameters)
       (set_advanced_node_parameters)
     )
FC_API(graphene::app::crypto_api,
       (blind_sign)
       (unblind_signature)
       (blind)
       (blind_sum)
       (verify_sum)
       (verify_range)
       (range_proof_sign)
       (verify_range_proof_rewind)
       (range_get_info)
     )
FC_API(graphene::app::asset_api,
       (get_asset_holders)
	     (get_asset_holders_count)
       (get_all_asset_holders)
     )
FC_API(graphene::app::orders_api,
       (get_tracked_groups)
       (get_grouped_limit_orders)
     )


FC_API(graphene::app::finance_api, 
       (get_asset_issue_fundraise)
       (list_issue_fundraise)
       (can_edit_issue_fundraise)
       (list_my_issue_fundraise)
       (list_buy_fundraise)
       (list_buy_fundraise_by_id)
       (list_my_buy_fundraise)
       (list_sell_exchange)       
       (list_my_sell_exchange)
       (list_buy_exchange)
       (list_buy_exchange_by_id)
       (list_my_buy_exchange)
     )  

FC_API(graphene::app::mobile_api, 
       (test)
       )
FC_API(graphene::app::admin_api, 
       (get_collaboration)
       (get_carrier)       
       (get_gateway)
       (get_lending_asset)    
       (get_locktoken_recommend)   
       )       
FC_API(graphene::app::bitlender_api, 
       (get_invest_fee)
       (get_repay_fee)
       (get_pay_fee)
       (get_invest_process)          
       (get_loan_info)  
       (calc_string)  
       (get_feeders)
       (get_loan_collateralize)
       (get_asset_by_property)
       (get_precent_value)
       (list_bitlender_order)
       (list_carrier_orders)
       (get_loan_order)
       (get_loan_orders_by_id)
       (get_invest_orders_by_id)
       (get_invest_order)
       (can_edit_bitlender_option)
       (can_edit_locktoken_option)
     )


FC_API(graphene::app::login_api,
       (login)
       (block)
       (network_broadcast)
       (database)
       (history)
       (network_node)
       (crypto)
       (asset)
       (orders)
       (bitlender)
       (finance)
       (mobile)
       (admin)
       (debug)
       (exit_by_pass)
       (get_revision_unix_timestamp)
       (get_revision_description)    
       (get_code_version)   
     )