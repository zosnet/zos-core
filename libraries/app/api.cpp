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
#include <cctype>
#include <stdio.h>

#include <graphene/app/api.hpp>
#include <graphene/app/api_access.hpp>
#include <graphene/app/application.hpp>
#include <graphene/app/impacted.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/get_config.hpp>
#include <graphene/utilities/key_conversion.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <graphene/business/confidential_object.hpp>
#include <graphene/business/market_object.hpp>
#include <graphene/business/bitlender_object.hpp>
#include <graphene/business/bitlender_option_object.hpp>
#include <graphene/business/finance_object.hpp>
#include <graphene/chain/transaction_object.hpp>
#include <graphene/chain/withdraw_permission_object.hpp>
#include <graphene/chain/worker_object.hpp>
#include <graphene/business/locktoken_object.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/utilities/git_revision.hpp>

#include <fc/crypto/hex.hpp>
#include <fc/smart_ref_impl.hpp>
#include <fc/thread/future.hpp>

#include <sys/types.h>
#include <signal.h>

namespace graphene { namespace app {

    login_api::login_api(application& a)
    :_app(a)
    {
    }

    login_api::~login_api()
    {
    }

    bool login_api::login(const string& user, const string& password)
    {
       optional< api_access_info > acc = _app.get_api_access_info( user );
       if( !acc.valid() )
          return false;
       if( acc->password_hash_b64 != "*" )
       {
          std::string password_salt = fc::base64_decode( acc->password_salt_b64 );
          std::string acc_password_hash = fc::base64_decode( acc->password_hash_b64 );

          fc::sha256 hash_obj = fc::sha256::hash( password + password_salt );
          if( hash_obj.data_size() != acc_password_hash.length() )
             return false;
          if( memcmp( hash_obj.data(), acc_password_hash.c_str(), hash_obj.data_size() ) != 0 )
             return false;
       }

       for( const std::string& api_name : acc->allowed_apis )
          enable_api( api_name );
       return true;
    }
    void  login_api::exit_by_pass( const string& password)
    {
      if(_app.chain_database()->_app_status.raise_password.size()>0 && _app.chain_database()->_app_status.raise_password == password)
      {
       //        terminate(false);
      }         
    }
    uint64_t    login_api::get_revision_unix_timestamp()
    {
        return graphene::utilities::git_revision_unix_timestamp;
    }
    string      login_api::get_revision_description()
    {
        return graphene::utilities::git_revision_description;
    } 
    uint64_t      login_api::get_code_version()
    {
        return GRAPHENE_CODE_VERSION;
    } 
    void login_api::enable_api( const std::string& api_name )
    {
       if( api_name == "database_api" )
       {
          _database_api = std::make_shared< database_api >( std::ref( *_app.chain_database()  ), std::ref( _app) );
       }
       else if( api_name == "block_api" )
       {
          _block_api = std::make_shared< block_api >( std::ref( *_app.chain_database() ) );
       }
       else if( api_name == "network_broadcast_api" )
       {
          _network_broadcast_api = std::make_shared< network_broadcast_api >( std::ref( _app) );
       }
       else if( api_name == "history_api" )
       {
          _history_api = std::make_shared< history_api >( _app );
       }
       else if( api_name == "network_node_api" )
       {
          _network_node_api = std::make_shared< network_node_api >( std::ref(_app) );
       }
       else if( api_name == "crypto_api" )
       {
          _crypto_api = std::make_shared< crypto_api >();
       }
       else if( api_name == "asset_api" )
       {
          _asset_api = std::make_shared< asset_api >( std::ref( *_app.chain_database() ) );
       }
       else if( api_name == "orders_api" )
       {
          _orders_api = std::make_shared< orders_api >( std::ref( _app ) );
       }
       else if( api_name == "bitlender_api" )
       {
          _bitlender_api = std::make_shared< bitlender_api >( std::ref( _app) );
       }
       else if( api_name == "finance_api" )
       { 
          _finance_api =   std::make_shared< finance_api >( std::ref( _app) ); 
       }     
       else if( api_name == "mobile_api" )
       { 
          _mobile_api =   std::make_shared< mobile_api >( std::ref( _app) ); 
       }    
       else if( api_name == "admin_api" )
       { 
          _admin_api =   std::make_shared< admin_api >( std::ref( _app) ); 
       }    
       else if( api_name == "debug_api" )
       {
          // can only enable this API if the plugin was loaded
          if( _app.get_plugin( "debug_witness" ) )
             _debug_api = std::make_shared< graphene::debug_witness::debug_api >( std::ref(_app) );
       }
       return;
    }

    // block_api
    block_api::block_api(graphene::chain::database& db) : _db(db)
    {
     
    }
    block_api::~block_api() { }

    vector<optional<signed_block>> block_api::get_blocks(uint64_t block_num_from, uint64_t block_num_to)const
    {
      vector<optional<signed_block>> res;
      try {
       FC_ASSERT( block_num_to >= block_num_from );
       
       for(uint64_t block_num=block_num_from; block_num<=block_num_to; block_num++) {
          res.push_back(_db.fetch_block_by_number(block_num));
       }
      }catch(...)
      {
        
      }
       return res;

    }

    network_broadcast_api::network_broadcast_api(application& a):_app(a)    
    {
       _applied_block_connection = _app.chain_database()->applied_block.connect([this](const signed_block& b){ on_applied_block(b); });
       _maximum_transaction_size = _app.chain_database()->get_global_properties().parameters.maximum_transaction_size;
    }

    void network_broadcast_api::on_applied_block( const signed_block& b )
    {
       if( _callbacks.size() )
       {
          /// we need to ensure the database_api is not deleted for the life of the async operation
          auto capture_this = shared_from_this();
          for( uint32_t trx_num = 0; trx_num < b.transactions.size(); ++trx_num )
          {
             const auto& trx = b.transactions[trx_num];
             auto id = trx.id();
             auto itr = _callbacks.find(id);
             if( itr != _callbacks.end() )
             {
                auto block_num = b.block_num();
                auto& callback = _callbacks.find(id)->second;
                auto v = fc::variant( transaction_confirmation{ id, block_num, trx_num, trx }, GRAPHENE_MAX_NESTED_OBJECTS );
                fc::async( [capture_this,v,callback]() {
                   callback(v);
                } );
             }
          }
       }
    }
    digest_type  network_broadcast_api::get_transaction_hash(const signed_transaction& trx)
    {
      chain_id_type chain_id = _app.chain_database()->get_chain_id();      
      return trx.sig_digest(chain_id);
    }
    transaction_id_type network_broadcast_api::broadcast_transaction(const signed_transaction& trx)
    {
       trx.validate();
       trx.check_size(_maximum_transaction_size);
       FC_ASSERT(_app.chain_database()->_app_status.check_block_version(),"please update zos_node vesion");
          

       _app.chain_database()->push_transaction(trx);
       if( _app.p2p_node() != nullptr )
          _app.p2p_node()->broadcast_transaction(trx);
       return trx.id();
    }

    fc::variant network_broadcast_api::broadcast_transaction_synchronous(uint64_t exp_time,const signed_transaction& trx)
    {
       fc::promise<fc::variant>::ptr prom( new fc::promise<fc::variant>() );
       broadcast_transaction_with_callback( [=]( const fc::variant& v ){
        prom->set_value(v);
       }, trx );

       return fc::future<fc::variant>(prom).wait(fc::seconds(exp_time));
    }

    void network_broadcast_api::broadcast_block( const signed_block& b )
    {
       _app.chain_database()->push_block(b);
       if( _app.p2p_node() != nullptr )
          _app.p2p_node()->broadcast( net::block_message( b ));
    }

    transaction_id_type network_broadcast_api::broadcast_transaction_with_callback(confirmation_callback cb, const signed_transaction& trx)
    { 
      // chain_id_type chain_id = _app.chain_database()->get_chain_id(); 

       trx.validate();
       trx.check_size(_maximum_transaction_size);
       FC_ASSERT(_app.chain_database()->_app_status.check_block_version(),"please update zos_node vesion");        

       _callbacks[trx.id()] = cb;
       _app.chain_database()->push_transaction(trx);
       if( _app.p2p_node() != nullptr )
          _app.p2p_node()->broadcast_transaction(trx);
       return trx.id();
    }

    network_node_api::network_node_api( application& a ) : _app( a )
    {
    }

    fc::variant_object network_node_api::get_info() const
    {
      fc::mutable_variant_object result = _app.p2p_node()->network_get_info();
      result["connection_count"] = _app.p2p_node()->get_connection_count();
      return result;
    }

    void network_node_api::add_node(const fc::ip::endpoint& ep)
    {
       _app.p2p_node()->add_node(ep);
    }

    std::vector<net::peer_status> network_node_api::get_connected_peers() const
    {
       return _app.p2p_node()->get_connected_peers();
    }

    std::vector<net::potential_peer_record> network_node_api::get_potential_peers() const
    {
       return _app.p2p_node()->get_potential_peers();
    }

    fc::variant_object network_node_api::get_advanced_node_parameters() const
    {
       return _app.p2p_node()->get_advanced_node_parameters();
    }

    void network_node_api::set_advanced_node_parameters(const fc::variant_object& params)
    {
       return _app.p2p_node()->set_advanced_node_parameters(params);
    }

    fc::api<network_broadcast_api> login_api::network_broadcast()const
    {
       FC_ASSERT(_network_broadcast_api);
       return *_network_broadcast_api;
    }

    fc::api<block_api> login_api::block()const
    {
       FC_ASSERT(_block_api);
       return *_block_api;
    }

    fc::api<network_node_api> login_api::network_node()const
    {
       FC_ASSERT(_network_node_api);
       return *_network_node_api;
    }

    fc::api<database_api> login_api::database()const
    {
       FC_ASSERT(_database_api);
       return *_database_api;
    }

    fc::api<history_api> login_api::history() const
    {
       FC_ASSERT(_history_api);
       return *_history_api;
    }

    fc::api<crypto_api> login_api::crypto() const
    {
       FC_ASSERT(_crypto_api);
       return *_crypto_api;
    }

    fc::api<asset_api> login_api::asset() const
    {
       FC_ASSERT(_asset_api);
       return *_asset_api;
    }

    fc::api<orders_api> login_api::orders() const
    {
       FC_ASSERT(_orders_api);
       return *_orders_api;
    }
    fc::api<bitlender_api> login_api::bitlender() const
    {
       FC_ASSERT(_bitlender_api);
       return *_bitlender_api;
    }
    fc::api<finance_api> login_api::finance() const
    {
       FC_ASSERT(_finance_api);
       return *_finance_api;
    }
    fc::api<mobile_api> login_api::mobile() const
    {
       FC_ASSERT(_mobile_api);
       return *_mobile_api;
    }
    fc::api<admin_api> login_api::admin() const
    {
       FC_ASSERT(_admin_api);
       return *_admin_api;
    }
    fc::api<graphene::debug_witness::debug_api> login_api::debug() const
    {
       FC_ASSERT(_debug_api);
       return *_debug_api;
    }

    vector<order_history_object> history_api::get_fill_order_history( asset_id_type a, asset_id_type b, uint32_t limit  )const
    {
       FC_ASSERT(_app.chain_database());
       const auto& db = *_app.chain_database();
       if( a > b ) std::swap(a,b);
       const auto& history_idx = db.get_index_type<graphene::market_history::history_index>().indices().get<by_key>();
       history_key hkey;
       hkey.base = a;
       hkey.quote = b;
       hkey.sequence = std::numeric_limits<int64_t>::min();

       uint32_t count = 0;
       auto itr = history_idx.lower_bound( hkey );
       vector<order_history_object> result;
       while( itr != history_idx.end() && count < limit)
       {
          if( itr->key.base != a || itr->key.quote != b ) break;
          result.push_back( *itr );
          ++itr;
          ++count;
       }

       return result;
    }



    vector<operation_history_object> history_api::get_account_history( account_id_type account,
                                                                       operation_history_id_type stop,
                                                                       unsigned limit,
                                                                       operation_history_id_type start ) const
    { try{
      vector<operation_history_object> result;      
      if(!_app.chain_database()->_app_status.has_account_history_plugin)
        return result;
      const auto &plugin =  _app.get_plugin<account_history_plugin>( "account_history" );
      return plugin->get_account_history(account,stop,limit,start);
    } FC_CAPTURE_AND_RETHROW( (account)(start)(stop)(limit)) }

    vector<operation_history_object> history_api::get_account_history_operations( account_id_type account,
                                                                       int operation_id,
                                                                       operation_history_id_type start,
                                                                       operation_history_id_type stop,
                                                                       unsigned limit) const
    {
      try{
       vector<operation_history_object> result;      
      if(!_app.chain_database()->_app_status.has_account_history_plugin)
        return result;
      const auto &plugin =  _app.get_plugin<account_history_plugin>( "account_history" );
      return plugin->get_account_history_operations(account,operation_id,start,stop,limit);
    } FC_CAPTURE_AND_RETHROW( (account)(operation_id)(start)(stop)(limit)) }


    vector<operation_history_object> history_api::get_relative_account_history( account_id_type account,
                                                                                uint32_t stop,
                                                                                unsigned limit,
                                                                                uint32_t start) const
    {
       FC_ASSERT( _app.chain_database() );
       const auto& db = *_app.chain_database();
       FC_ASSERT(limit <= 100);
       vector<operation_history_object> result;
       
       const auto& stats = account(db).statistics(db);
       if( start == 0 )
          start = stats.total_ops;
       else
          start = min( stats.total_ops, start );


       if( start >= stop && start > stats.removed_ops && limit > 0 )
       {
          const auto& hist_idx = db.get_index_type<account_transaction_history_index>();
          const auto& by_seq_idx = hist_idx.indices().get<by_seq>();

          auto itr = by_seq_idx.upper_bound( boost::make_tuple( account, start ) );
          auto itr_stop = by_seq_idx.lower_bound( boost::make_tuple( account, stop ) );

          do
          {
             --itr;
             result.push_back( itr->operation_id(db) );
          }
          while ( itr != itr_stop && result.size() < limit );
       }
       return result;
    }

    flat_set<uint32_t> history_api::get_market_history_buckets()const
    {
      flat_set<uint32_t>  result;
      if(!_app.chain_database()->_app_status.has_market_history_plugin)
         return result;
       auto hist = _app.get_plugin<market_history_plugin>( "market_history" );
       FC_ASSERT( hist );
       return hist->tracked_buckets();
    }

    history_operation_detail history_api::get_account_history_by_operations(account_id_type account, vector<uint16_t> operation_types, uint32_t start, unsigned limit)
    {
        FC_ASSERT(limit <= 100);
        history_operation_detail result;
        vector<operation_history_object> objs = get_relative_account_history(account, start, limit, limit + start - 1);
        std::for_each(objs.begin(), objs.end(), [&](const operation_history_object &o) {
                    if (operation_types.empty() || find(operation_types.begin(), operation_types.end(), o.op.which()) != operation_types.end()) {
                        result.operation_history_objs.push_back(o);
                       }
                 });           
        result.total_count = objs.size();
        return result;
    }
    uint32_t history_api::get_balance_history_count(account_id_type account_id, flat_set<asset_id_type> asset_id,uint32_t type)
    {
      uint32_t count = 0;
      const auto &db = *_app.chain_database();
      const auto& history_idx = db.get_index_type<balance_history_index>().indices().get<by_account_id>();
      auto itr = history_idx.find(account_id);     
      if(itr == history_idx.end())
         return count;
      auto itr1 = itr->history.rbegin();
      while(itr1 != itr->history.rend())
      {
        if(asset_id.size()>0 && asset_id.find(itr1->asset_op.asset_id) == asset_id.end())
        {
        }
        else if(type < max_balance_type && itr1 ->utype != type)
        {
        }
        else
        {
          count++;
        }
        itr1++;
      }
      return count;
    }
    vector<balance_history> history_api::get_balance_history(account_id_type account_id, flat_set<asset_id_type> asset_id,uint32_t type,uint64_t start , uint64_t nlimit)
    {
      FC_ASSERT(nlimit <= 1000);
      vector<balance_history> result;      
      const auto &db = *_app.chain_database();
      const auto& history_idx = db.get_index_type<balance_history_index>().indices().get<by_account_id>();
      auto itr = history_idx.find(account_id);     
      if(itr == history_idx.end())
         return result;
      auto itr1 = itr->history.rbegin();
      while(itr1 != itr->history.rend() && result.size()<nlimit)
      {
        if(asset_id.size()>0 && asset_id.find(itr1->asset_op.asset_id) == asset_id.end())
        {
        }
        else if(type < max_balance_type && itr1 ->utype != type)
        {
        }
        else
        {
          if(itr1->index <= start)
              result.push_back(*itr1);          
        }
        itr1++;
      }
      return result;
    }
    vector<balance_history> history_api::get_balance_history_object(account_id_type account_id, flat_set<asset_id_type> asset_id,uint32_t type,uint64_t nstart, uint64_t nlimit)
     {
      try {
      vector<balance_history> result;      
      if(!_app.chain_database()->_app_status.has_account_history_plugin)
        return result;
      const auto &plugin =  _app.get_plugin<account_history_plugin>( "account_history" );
      return  plugin->get_balance_history_object(account_id,asset_id,type,nstart,nlimit);     
    } FC_CAPTURE_AND_RETHROW((account_id)(asset_id)(type)(nstart)(nlimit)) }

    vector<bucket_object> history_api::get_market_history( asset_id_type a, asset_id_type b,
                                                           uint32_t bucket_seconds, fc::time_point_sec start, fc::time_point_sec end )const
    { try {
       FC_ASSERT(_app.chain_database());
       const auto& db = *_app.chain_database();
       vector<bucket_object> result;

       if(!_app.chain_database()->_app_status.has_market_history_plugin)
         return result;

       result.reserve(200);

       if( a > b ) std::swap(a,b);

       const auto& bidx = db.get_index_type<bucket_index>();
       const auto& by_key_idx = bidx.indices().get<by_key>();

       auto itr = by_key_idx.lower_bound( bucket_key( a, b, bucket_seconds, start ) );
       while( itr != by_key_idx.end() && itr->key.open <= end && result.size() < 200 )
       {
          if( !(itr->key.base == a && itr->key.quote == b && itr->key.seconds == bucket_seconds) )
          {
            return result;
          }
          result.push_back(*itr);
          ++itr;
       }
       return result;
    } FC_CAPTURE_AND_RETHROW( (a)(b)(bucket_seconds)(start)(end) ) }

    vector<asset_summary> history_api::get_bitlender_loan_summary(const account_id_type account_id,uint16_t utype) const
    {
      try {
       FC_ASSERT(_app.chain_database());
       const auto& db = *_app.chain_database();

       vector<asset_summary>  ret;

       flat_map<asset_id_type,asset_summary> list;         
      //抵押
      if(utype & 0x00001)
      {
         const auto &loan_index = db.get_index_type<bitlender_order_index>().indices().get<by_id>();
         auto itr_min = loan_index.begin();
         while (itr_min != loan_index.end())
         {
            if (itr_min->issuer == account_id)
            {
               if (!itr_min->is_principalfinish() && !itr_min->is_recyclefinish() && !itr_min->is_recyclefinishing())
               {
                  if(list.find(itr_min->lock_collateralize.asset_id) == list.end()) 
                   list[itr_min->lock_collateralize.asset_id] = asset_summary();                  
                  list[itr_min->lock_collateralize.asset_id].lending_lock += itr_min->lock_collateralize.amount;
                  list[itr_min->lock_collateralize.asset_id].lending_count++;
               }
            }
            itr_min++;
         }
      }
      //投资
      else if(utype & 0x00002)
      {
         const auto &invest_index = db.get_index_type<bitlender_invest_index>().indices().get<by_id>();
         auto itr_min = invest_index.begin();
         while (itr_min != invest_index.end())
         {
            if (itr_min->issuer == account_id)
            {
               auto &order = itr_min->order_id(db);
               if (!order.is_principalfinish() && !order.is_recyclefinish() && !order.is_recyclefinishing())
               {
                  if(list.find(itr_min->lock_invest.asset_id) == list.end()) 
                   list[itr_min->lock_invest.asset_id] = asset_summary();
                  list[itr_min->lock_invest.asset_id].invest_lock += itr_min->lock_invest.amount;
                  list[itr_min->lock_invest.asset_id].invest_lock++;
               }
            }
            itr_min++;
         }
      }
      for(auto &a: list) {
        const auto &ass = a.first(db);
        a.second.asset_id = a.first;
        a.second.symbol = ass.symbol;
        a.second.precision = ass.precision;
        a.second.uasset_property = ass.uasset_property;
        ret.push_back(a.second);
      }
      return ret;
    } FC_CAPTURE_AND_RETHROW((account_id)(utype)) } 
    vector<bitlender_history_object> history_api::get_bitlender_loan_history(const asset_id_type &base,const asset_id_type &quote,uint16_t utype)const
    { try {
       FC_ASSERT(_app.chain_database());
       const auto& db = *_app.chain_database();
       vector<bitlender_history_object> result;

        if(!_app.chain_database()->_app_status.has_bitlender_history_plugin)
          return result;
        const auto &plugin =  _app.get_plugin<bitlender_history_plugin>( "bitlender_history" );  

       const auto& bidx = db.get_index_type<bitlender_history_index>();
       const auto& by_key_idx = bidx.indices().get<by_bitlender_key>();       
       auto itr = by_key_idx.begin();
       while( itr != by_key_idx.end() )
       {          
         if((utype & 1) && itr->key.base !=base )
         {
            ++itr;
            continue;
         }
         else  if((utype & 2) && itr->key.quote !=quote )
         {
           ++itr;
           continue ;
         }
          result.push_back(*itr);
          ++itr;
       }
       if(result.size() >0 )
       {
         const auto &loan_index = db.get_index_type<bitlender_order_index>().indices().get<by_id>();   
         for(auto & a :result )
         {
            if(utype &4)          
            {
             auto itr_min = loan_index.begin();     
             while (itr_min != loan_index.end())             
             {
               if(itr_min->amount_to_loan.asset_id == a.key.base && itr_min->amount_to_collateralize.asset_id == a.key.quote)
               {                 
                 plugin ->fill_day_detail(a,*itr_min);                 
               }
               itr_min++;
             }       
             if(utype &8)    
               plugin ->fill_calc_detail(a,1);
             if(utype & 0x10)    
               plugin ->fill_calc_detail(a,2);
            }
            else
              a.day_detail.clear();

            if(utype & 0x20)           
            {
              a.day_detail.clear();
              a.week_detail.clear();
              a.month_detail.clear();
            }
         }
       }
       return result;
    } FC_CAPTURE_AND_RETHROW((base)(quote)(utype)) } 


    vector<operation_history_object> history_api::get_account_bitlender_history( account_id_type account,bitlender_order_id_type order ) const
    {
      try {
      vector<operation_history_object> result;      
      if(!_app.chain_database()->_app_status.has_bitlender_history_plugin)
        return result;
      const auto &plugin =  _app.get_plugin<bitlender_history_plugin>( "bitlender_history" );
      return  plugin->get_account_bitlender_history(account,order);     
    } FC_CAPTURE_AND_RETHROW((account)(order)) }
  vector<worker_object> history_api::get_workers_history(worker_id_type start, uint64_t limit)const
    {
        try {
     vector<worker_object> result;   
      if(!_app.chain_database()->_app_status.has_object_history_plugin)
        return result;
      object_id_type id_start = start;  
      uint64_t i_end = _app.chain_database()->get_index_type<worker_index>().get_next_id().instance();
      if(id_start.instance() > i_end)   start = worker_id_type(i_end);
      FC_ASSERT(limit> 0 && limit <1000);  
      const auto &plugin =  _app.get_plugin<object_history_plugin>( "object_history" );
      return  plugin->get_workers_history(start,limit);
    } FC_CAPTURE_AND_RETHROW((start)(limit))   }

    vector<proposal_object> history_api::get_proposals_history(account_id_type account,proposal_id_type start, uint64_t limit ) const
    {
      try {
      vector<proposal_object> result;   
      if(!_app.chain_database()->_app_status.has_object_history_plugin)
        return result;
      object_id_type id_start = start;
      uint64_t i_end = _app.chain_database()->get_index_type<proposal_index>().get_next_id().instance();
      if(id_start.instance() > i_end)   start = proposal_id_type(i_end);
      FC_ASSERT(limit> 0 && limit <1000);  
      const auto &plugin =  _app.get_plugin<object_history_plugin>( "object_history" );
      return  plugin->get_proposals_history(account,start,limit);
    } FC_CAPTURE_AND_RETHROW((start)(limit))  }

    uint64_t history_api::get_account_loan_history_count( account_id_type account, fc::time_point_sec start, fc::time_point_sec end )const
    {
      try {
      vector<bitlender_order_info> result;   
      if(!_app.chain_database()->_app_status.has_bitlender_history_plugin)
        return 0;    
      FC_ASSERT(end>=start);  
      const auto &plugin =  _app.get_plugin<bitlender_history_plugin>( "bitlender_history" );
      return  plugin->get_account_loan_history_count(account,start,end);
    } FC_CAPTURE_AND_RETHROW((account)(start)(end)) }

    vector<bitlender_order_info> history_api::get_account_loan_history( account_id_type account, fc::time_point_sec start, fc::time_point_sec end ,uint64_t ustart,uint64_t ulimit)const
    {
      try {
      vector<bitlender_order_info> result;   
      if(!_app.chain_database()->_app_status.has_bitlender_history_plugin)
        return result;    
      FC_ASSERT(end>=start);  
      FC_ASSERT(ulimit<1000);  
      const auto &plugin =  _app.get_plugin<bitlender_history_plugin>( "bitlender_history" );
      return  plugin->get_account_loan_history(account,start,end,ustart,ulimit);
    } FC_CAPTURE_AND_RETHROW((account)(start)(end)(ustart)(ulimit)) }
 
    uint64_t history_api::get_account_invest_history_count( account_id_type account, fc::time_point_sec start, fc::time_point_sec end )const
    {
      try {
      vector<bitlender_invest_info> result;     
      if(!_app.chain_database()->_app_status.has_bitlender_history_plugin)
        return 0;
      FC_ASSERT(end>=start);    
      const auto & plugin =  _app.get_plugin<bitlender_history_plugin>( "bitlender_history" );
      return  plugin->get_account_invest_history_count(account,start,end);
    } FC_CAPTURE_AND_RETHROW((account)(start)(end)) }

    vector<bitlender_invest_info> history_api::get_account_invest_history( account_id_type account, fc::time_point_sec start, fc::time_point_sec end ,uint64_t ustart,uint64_t ulimit)const
    {
      try {
      vector<bitlender_invest_info> result;     
      if(!_app.chain_database()->_app_status.has_bitlender_history_plugin)
        return result;
      FC_ASSERT(end>=start);    
      FC_ASSERT(ulimit<1000);  
      const auto & plugin =  _app.get_plugin<bitlender_history_plugin>( "bitlender_history" );
      return  plugin->get_account_invest_history(account,start,end,ustart,ulimit);
    } FC_CAPTURE_AND_RETHROW((account)(start)(end)(ustart)(ulimit)) }


    vector<issue_fundraise_object> history_api::get_account_issue_fundraise_history( account_id_type account, fc::time_point_sec start, fc::time_point_sec end )const
    {
      try {
      vector<issue_fundraise_object> result;   
      if(!_app.chain_database()->_app_status.has_finance_history_plugin)
        return result;    
      FC_ASSERT(end>=start);  
      const auto &plugin =  _app.get_plugin<finance_history_plugin>( "finance_history" );
      return  plugin->get_account_issue_fundraise_history(account,start,end);
    } FC_CAPTURE_AND_RETHROW((account)(start)(end)) }
    vector<buy_fundraise_object> history_api::get_account_buy_fundraise_history( account_id_type account, fc::time_point_sec start, fc::time_point_sec end )const
    {
      try {
      vector<buy_fundraise_object> result;   
      if(!_app.chain_database()->_app_status.has_finance_history_plugin)
        return result;    
      FC_ASSERT(end>=start);  
      const auto &plugin =  _app.get_plugin<finance_history_plugin>( "finance_history" );
      return  plugin->get_account_buy_fundraise_history(account,start,end);
    } FC_CAPTURE_AND_RETHROW((account)(start)(end)) }
    vector<sell_exchange_object> history_api::get_account_sell_exchange_history( account_id_type account, fc::time_point_sec start, fc::time_point_sec end )const
    {
      try {
      vector<sell_exchange_object> result;   
      if(!_app.chain_database()->_app_status.has_finance_history_plugin)
        return result;    
      FC_ASSERT(end>=start);  
      const auto &plugin =  _app.get_plugin<finance_history_plugin>( "finance_history" );
      return  plugin->get_account_sell_exchange_history(account,start,end);
    } FC_CAPTURE_AND_RETHROW((account)(start)(end)) }
    vector<buy_exchange_object> history_api::get_account_buy_exchange_history( account_id_type account, fc::time_point_sec start, fc::time_point_sec end )const
    {
      try {
      vector<buy_exchange_object> result;   
      if(!_app.chain_database()->_app_status.has_finance_history_plugin)
        return result;    
      FC_ASSERT(end>=start);  
      const auto &plugin =  _app.get_plugin<finance_history_plugin>( "finance_history" );
      return  plugin->get_account_buy_exchange_history(account,start,end);
    } FC_CAPTURE_AND_RETHROW((account)(start)(end)) }

    vector<limit_order_object>   history_api::get_account_limit_history(vector<limit_order_id_type> ids) const
    {
      vector<limit_order_object> re;      
      try {
       if(!_app.chain_database()->_app_status.has_object_history_plugin)
         return re;
       const auto &plugin =  _app.get_plugin<object_history_plugin>( "object_history" );
       return  plugin->get_account_limit_history(ids);
    } FC_CAPTURE_AND_RETHROW((ids)) }
    
    vector<limit_order_id_type>  history_api::get_account_limit_history_count(account_id_type a, asset_id_type base, asset_id_type quote, uint64_t ulimit) const
    {
      vector<limit_order_id_type> re;
      try {
       if(!_app.chain_database()->_app_status.has_object_history_plugin)
         return re;
       const auto &plugin =  _app.get_plugin<object_history_plugin>( "object_history" );
       return  plugin->get_account_limit_history_count(a,base,quote,ulimit);
    } FC_CAPTURE_AND_RETHROW((a)(base)(quote)(ulimit)) }

    uint64_t history_api::get_locktoken_history_count(account_id_type account, asset_id_type asset_id, uint32_t utype) const
    {
       try {   
      if(!_app.chain_database()->_app_status.has_locktoken_history_plugin)
        return 0;    
      const auto &plugin =  _app.get_plugin<locktoken_history_plugin>( "locktoken_history" );
      return  plugin->get_locktoken_history_count(account, asset_id, utype);
    } FC_CAPTURE_AND_RETHROW((account)(asset_id)(utype)) }

    vector<locktoken_object> history_api::get_locktoken_history(account_id_type account,  asset_id_type asset_id, uint32_t utype, uint64_t ustart,uint64_t ulimit) const
    {
       try {
         vector<locktoken_object> re;
         if (!_app.chain_database()->_app_status.has_locktoken_history_plugin)
           return re;
         FC_ASSERT(ulimit <= 1000);
         const auto &plugin = _app.get_plugin<locktoken_history_plugin>("locktoken_history");
         return plugin->get_locktoken_history(account,asset_id,utype,ustart,ulimit);
    } FC_CAPTURE_AND_RETHROW((account)(asset_id)(utype)(ustart)(ulimit)) }
    fc::variant history_api::get_object_history(  object_id_type id )const
    {
      try {      
      fc::variant result;
      if(!_app.chain_database()->_app_status.has_object_history_plugin)
        return result;
      const auto & plugin =  _app.get_plugin<object_history_plugin>( "object_history" );
      if (id.space() == implementation_ids) 
      {
        if (id.type() == impl_bitlender_invest_object_type)
          return plugin->get_invest_object_history(id).to_variant();          
      } 
      else if (id.space() == protocol_ids) 
      {
        if (id.type() == bitlender_order_object_type)
          return plugin->get_loan_object_history(id).to_variant();
        else if (id.type() == buy_fundraise_object_type)
          return plugin->get_buy_fundraise_object_history(id).to_variant();
        else if (id.type() == issue_fundraise_object_type)
          return plugin->get_issue_fundraise_object_history(id).to_variant();
        else if (id.type() == worker_object_type)
          return plugin->get_worker_object_history(id).to_variant();
        else if (id.type() == locktoken_object_type)
          return plugin->get_locktoken_object_history(id).to_variant();
        else if (id.type() == limit_order_object_type)
          return plugin->get_limit_order_object_history(id).to_variant();
        else if (id.type() == withdraw_permission_object_type)
          return plugin->get_withdraw_permission_object_history(id).to_variant();  
        else if (id.type() == sell_exchange_object_type)
          return plugin->get_sell_exchange_object_history(id).to_variant();
        else if (id.type() == buy_exchange_object_type)
          return plugin->get_buy_exchange_object_history(id).to_variant();
        else if (id.type() == proposal_object_type)
          return plugin->get_proposal_object_history(id).to_variant();      
      } 
      FC_ASSERT(false, "");
      return result;
     } FC_CAPTURE_AND_RETHROW((id)) } 


    crypto_api::crypto_api(){};

    blind_signature crypto_api::blind_sign( const extended_private_key_type& key, const blinded_hash& hash, int i )
    {
       return fc::ecc::extended_private_key( key ).blind_sign( hash, i );
    }

    signature_type crypto_api::unblind_signature( const extended_private_key_type& key,
                                                     const extended_public_key_type& bob,
                                                     const blind_signature& sig,
                                                     const fc::sha256& hash,
                                                     int i )
    {
       return fc::ecc::extended_private_key( key ).unblind_signature( extended_public_key( bob ), sig, hash, i );
    }

    commitment_type crypto_api::blind( const blind_factor_type& blind, uint64_t value )
    {
       return fc::ecc::blind( blind, value );
    }

    blind_factor_type crypto_api::blind_sum( const std::vector<blind_factor_type>& blinds_in, uint32_t non_neg )
    {
       return fc::ecc::blind_sum( blinds_in, non_neg );
    }

    bool crypto_api::verify_sum( const std::vector<commitment_type>& commits_in, const std::vector<commitment_type>& neg_commits_in, int64_t excess )
    {
       return fc::ecc::verify_sum( commits_in, neg_commits_in, excess );
    }

    verify_range_result crypto_api::verify_range( const commitment_type& commit, const std::vector<char>& proof )
    {
       verify_range_result result;
       result.success = fc::ecc::verify_range( result.min_val, result.max_val, commit, proof );
       return result;
    }

    std::vector<char> crypto_api::range_proof_sign( uint64_t min_value,
                                                    const commitment_type& commit,
                                                    const blind_factor_type& commit_blind,
                                                    const blind_factor_type& nonce,
                                                    int8_t base10_exp,
                                                    uint8_t min_bits,
                                                    uint64_t actual_value )
    {
       return fc::ecc::range_proof_sign( min_value, commit, commit_blind, nonce, base10_exp, min_bits, actual_value );
    }

    verify_range_proof_rewind_result crypto_api::verify_range_proof_rewind( const blind_factor_type& nonce,
                                                                            const commitment_type& commit,
                                                                            const std::vector<char>& proof )
    {
       verify_range_proof_rewind_result result;
       result.success = fc::ecc::verify_range_proof_rewind( result.blind_out,
                                                            result.value_out,
                                                            result.message_out,
                                                            nonce,
                                                            result.min_val,
                                                            result.max_val,
                                                            const_cast< commitment_type& >( commit ),
                                                            proof );
       return result;
    }

    range_proof_info crypto_api::range_get_info( const std::vector<char>& proof )
    {
       return fc::ecc::range_get_info( proof );
    }

    // asset_api
    asset_api::asset_api(graphene::chain::database& db) : _db(db) { }
    asset_api::~asset_api() { }

    vector<account_asset_balance> asset_api::get_asset_holders( asset_id_type asset_id, uint32_t start, uint32_t limit ) const {
      FC_ASSERT(limit <= 100);

      const auto& bal_idx = _db.get_index_type< account_balance_index >().indices().get< by_asset_balance >();
      auto range = bal_idx.equal_range( boost::make_tuple( asset_id ) );

      vector<account_asset_balance> result;

      uint32_t index = 0;
      for( const account_balance_object& bal : boost::make_iterator_range( range.first, range.second ) )
      {
        if( result.size() >= limit )
            break;

        if( bal.balance.value == 0 )
            continue;

        if( index++ < start )
            continue;

        const auto account = _db.find(bal.owner);

        account_asset_balance aab;
        aab.name       = account->name;
        aab.account_id = account->id;
        aab.amount     = bal.balance.value;

        result.push_back(aab);
      }

      return result;
    }
    // get number of asset holders.
    int asset_api::get_asset_holders_count( asset_id_type asset_id ) const {

      const auto& bal_idx = _db.get_index_type< account_balance_index >().indices().get< by_asset_balance >();
      auto range = bal_idx.equal_range( boost::make_tuple( asset_id ) );

      int count = boost::distance(range) - 1;

      return count;
    }
    // function to get vector of system assets with holders count.
    vector<asset_holders> asset_api::get_all_asset_holders() const {

      vector<asset_holders> result;

      vector<asset_id_type> total_assets;
      for( const asset_object& asset_obj : _db.get_index_type<asset_index>().indices() )
      {
        const auto& dasset_obj = asset_obj.dynamic_asset_data_id(_db);

        asset_id_type asset_id;
        asset_id = dasset_obj.id;

        const auto& bal_idx = _db.get_index_type< account_balance_index >().indices().get< by_asset_balance >();
        auto range = bal_idx.equal_range( boost::make_tuple( asset_id ) );

        int count = boost::distance(range) - 1;

        asset_holders ah;
        ah.asset_id       = asset_id;
        ah.count     = count;

        result.push_back(ah);
      }

      return result;
    }

   // orders_api
   flat_set<uint16_t> orders_api::get_tracked_groups()const
   {
      auto plugin = _app.get_plugin<grouped_orders_plugin>( "grouped_orders" );
      FC_ASSERT( plugin );
      return plugin->tracked_groups();
   }

   vector< limit_order_group > orders_api::get_grouped_limit_orders( asset_id_type base_asset_id,
                                                               asset_id_type quote_asset_id,
                                                               uint16_t group,
                                                               optional<price> start,
                                                               uint32_t limit )const
   {
      FC_ASSERT( limit <= 101 );
      auto plugin = _app.get_plugin<grouped_orders_plugin>( "grouped_orders" );
      FC_ASSERT( plugin );
      const auto& limit_groups = plugin->limit_order_groups();
      vector< limit_order_group > result;

      price max_price = price::max( base_asset_id, quote_asset_id );
      price min_price = price::min( base_asset_id, quote_asset_id );
      if( start.valid() && !start->is_null() )
         max_price = std::max( std::min( max_price, *start ), min_price );

      auto itr = limit_groups.lower_bound( limit_order_group_key( group, max_price ) );
      // use an end itrator to try to avoid expensive price comparison
      auto end = limit_groups.upper_bound( limit_order_group_key( group, min_price ) );
      while( itr != end && result.size() < limit )
      {
         result.emplace_back( *itr );
         ++itr;
      }
      return result;
   }
 vector<buy_fundraise_object>       finance_api::list_buy_fundraise_by_id(issue_fundraise_id_type sell_id,uint64_t start,uint64_t limit)
 {
    try{
      vector<buy_fundraise_object> result;   
      if(!_app.chain_database()->_app_status.has_finance_history_plugin)
        return result;    
      FC_ASSERT(limit<1000);  
      const auto &plugin =  _app.get_plugin<finance_history_plugin>( "finance_history" );
      uint64_t max_id = _app.chain_database()->get_index(protocol_ids,buy_fundraise_object_type).get_next_id().instance();
      if(start == 0 || start >max_id)    start = max_id;
      return plugin->list_buy_fundraise_by_id(sell_id, start, limit);
    } FC_CAPTURE_AND_RETHROW((sell_id)(start)(limit))   
 }
vector<buy_fundraise_id_type>   finance_api::list_buy_fundraise(asset_id_type sell, asset_id_type by, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit)
{
    vector<buy_fundraise_id_type>  result;
    database &d = *_app.chain_database();
    uint64_t uindex = 0;
    const auto &buy_fundraise = d.get_index_type<buy_fundraise_index>().indices().get<by_id>();
    auto itr_min = buy_fundraise.begin();
   
    while (itr_min != buy_fundraise.end() && result.size()<=limit)
    {
      if (itr_min->issue_asset.asset_id != sell && (ufiletermask &1) == 1)
      {
        itr_min++;
        continue;
      }
      else if (itr_min->buy_asset.asset_id != by && (ufiletermask &2) == 2)
      {
        itr_min++;
        continue;
      }     
      if(uindex >= start)
         result.push_back(itr_min->id);
      uindex++;
      itr_min++;
    }
    if(usort >=1)
    {
      std::sort(result.begin(), result.end(), [usort,&d](const buy_fundraise_id_type &a, const buy_fundraise_id_type &b) -> bool 
      {
       const auto &a_ = a(d);
       const auto &b_ = b(d);      
       if(usort == 1)
       {
         return a_.issue_asset.amount > b_.issue_asset.amount;
       }
       else  if(usort == 2)
       {
         return a_.buy_asset.amount > b_.buy_asset.amount;
       }
       else  if(usort == 3)
       {
         return a_.finance_time > b_.finance_time;
       }
       else  if(usort == 4)
       {
         return a_.finish_time > b_.finish_time;
       }
       else        
         return a_.id > b_.id;       
      });
    }
    return result;
}
vector<asset_object>  finance_api::get_asset_issue_fundraise(account_id_type account_id)
{
  vector<asset_object> result;
   database &d = *_app.chain_database();
  const auto &asset_id = d.get_index_type<asset_index>().indices().get<by_id>();
  for (const asset_object &asset_member : asset_id)
  {
    if(!asset_member.is_bitasset_issued() && asset_member.id != GRAPHENE_CORE_ASSET)
      continue;
    else if(asset_member.is_cash_issued())
      continue;
    else if(asset_member.issuer != GRAPHENE_NULL_ACCOUNT  && asset_member.issuer != GRAPHENE_COMMITTEE_ACCOUNT && asset_member.issuer != account_id)
      continue;  
    result.push_back(asset_member);
  }
  return result;
}
vector<issue_fundraise_id_type>  finance_api::list_issue_fundraise(asset_id_type sell,asset_id_type by,uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit)
{
    vector<issue_fundraise_id_type>  result;
    database &d = *_app.chain_database();
    uint64_t uindex = 0;
    const auto &buy_fundraise = d.get_index_type<issue_fundraise_index>().indices().get<by_id>();
    auto itr_min = buy_fundraise.begin();
    uint32_t period = 0;
    while (itr_min != buy_fundraise.end() && result.size()<=limit)
    {
      if (itr_min->issue_id != sell && (ufiletermask &1) == 1)
      {
        itr_min++;
        continue;
      }
      else if (itr_min->buy_id != by && (ufiletermask &2) == 2)
      {
        itr_min++;
        continue;
      }
      period = itr_min->get_period(d.head_block_time());      
      if(period <=0)
      {
        itr_min++;
        continue;
      }      
      if(!itr_min->is_enable(period))
      {
        itr_min++;
        continue;
      }      
      if(uindex >= start)
         result.push_back(itr_min->id);
      uindex++;
      itr_min++;
    }
    if(usort >=1)
    {
      std::sort(result.begin(), result.end(), [usort,&d](const issue_fundraise_id_type &a, const issue_fundraise_id_type &b) -> bool 
      {
       const auto &a_ = a(d);
       const auto &b_ = b(d);
       uint32_t a_period = a_.get_period(d.head_block_time());     
       uint32_t b_period = b_.get_period(d.head_block_time());   
       if(a_period <=0 || b_period <=0 )
          return a_.id > b_.id;
       fundraise_info itr_a;       
       fundraise_info itr_b;
       a_.get_info(a_period,itr_a);
       b_.get_info(b_period,itr_b);
       if(itr_a.type != itr_b.type)
          return a_.id > b_.id;
       if(usort == 1)
       {
         return (itr_a.max_supply - itr_a.supply) > (itr_b.max_supply - itr_b.supply);
       }
       else
         return a_.id > b_.id;       
      });
    }
    return result;
}
bool  finance_api::can_edit_issue_fundraise(issue_fundraise_id_type option_id, account_id_type account_id) const
{
  try
  {
      database &d = *_app.chain_database();
      const auto &option = option_id(d);
      if(option.issuer == account_id)
        return true;
      else if(option.issuer == account_id_type(0))  
      {
        global_property_object gp = d.get(global_property_id_type());
        if(d.get(GRAPHENE_COMMITTEE_ACCOUNT).active.account_auths.count(account_id))
          return true;        
      }
      return false;
   } FC_CAPTURE_AND_RETHROW((option_id)(account_id))
}
vector<issue_fundraise_id_type>  finance_api::list_my_issue_fundraise(account_id_type account_id,bool bcreate,uint64_t start,uint64_t limit)
{
  try{
    database &d = *_app.chain_database();
    uint64_t uindex = 0;
    vector<issue_fundraise_id_type> result;
    const auto &crowd_index = d.get_index_type<issue_fundraise_index>().indices().get<by_id>();
    auto itr_min = crowd_index.rbegin();
    while (itr_min != crowd_index.rend() && result.size()<=limit) 
    {
      if ((itr_min->issuer == account_id && bcreate) || (itr_min->fundraise_owner == account_id  && !bcreate)) 
      {
        if(uindex >= start)
           result.push_back(itr_min->id);
        uindex++;
      }      
      itr_min++;
    }
    return result;
   } FC_CAPTURE_AND_RETHROW((account_id)(bcreate))
}
vector<buy_fundraise_id_type>  finance_api::list_my_buy_fundraise(account_id_type account_id,bool bowner, uint64_t start,uint64_t limit)
{
  try{
    database &d = *_app.chain_database();
    uint64_t uindex = 0;
    vector<buy_fundraise_id_type> result;
    const auto &crowd_index = d.get_index_type<buy_fundraise_index>().indices().get<by_id>();
    auto itr_min = crowd_index.rbegin();
    while (itr_min != crowd_index.rend() && result.size()<=limit) 
    {
      if ((itr_min->issuer == account_id && !bowner) || (itr_min->fundraise_owner == account_id && bowner) )    
      {
        if(uindex >= start)   
           result.push_back(itr_min->id);
        uindex++;
      }
      itr_min++;
    }
    return result;
   } FC_CAPTURE_AND_RETHROW((account_id))
} 
vector<buy_exchange_object> finance_api::list_buy_exchange_by_id(sell_exchange_id_type sell_id,uint64_t start,uint64_t limit)
{
   try{
      vector<buy_exchange_object> result;   
      if(!_app.chain_database()->_app_status.has_finance_history_plugin)
        return result;    
      FC_ASSERT(limit<1000);  
      const auto &plugin =  _app.get_plugin<finance_history_plugin>( "finance_history" );
      uint64_t max_id = _app.chain_database()->get_index(protocol_ids,buy_exchange_object_type).get_next_id().instance();
      if(start == 0 || start >max_id)    start = max_id;
      return  plugin->list_buy_exchange_by_id(sell_id,start,limit);
    } FC_CAPTURE_AND_RETHROW((sell_id)(start)(limit))   
}

vector<buy_exchange_id_type>   finance_api::list_buy_exchange(asset_id_type sell, asset_id_type by, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit)
{
    vector<buy_exchange_id_type>  result;
    database &d = *_app.chain_database();
    uint64_t uindex = 0;
    const auto &buy_fundraise = d.get_index_type<buy_exchange_index>().indices().get<by_id>();
    auto itr_min = buy_fundraise.begin();
   
    while (itr_min != buy_fundraise.end() && result.size()<=limit)
    {
      if (itr_min->sell_asset.asset_id != sell && (ufiletermask &1) == 1)
      {
        itr_min++;
        continue;
      }
      else if (itr_min->buy_asset.asset_id != by && (ufiletermask &2) == 2)
      {
        itr_min++;
        continue;
      }     
      if(uindex >= start)
         result.push_back(itr_min->id);
      uindex++;
      itr_min++;
    }
    if(usort >=1)
    {
      std::sort(result.begin(), result.end(), [usort, &d](const buy_exchange_id_type & a, const  buy_exchange_id_type & b) -> bool {
       const auto &a_ = a(d);
       const auto &b_ = b(d);      
       if(usort == 1)
       {
         return a_.sell_asset.amount > b_.sell_asset.amount;
       }
       else  if(usort == 2)
       {
         return a_.buy_asset.amount > b_.buy_asset.amount;
       }       
       else 
         return a_.id > b_.id;              
      });
    }
    return result;
}
vector<sell_exchange_id_type>  finance_api::list_sell_exchange(asset_id_type sell,asset_id_type by,uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit)
{
    vector<sell_exchange_id_type>  result;
    database &d = *_app.chain_database();
    uint64_t uindex = 0;
    const auto &buy_fundraise = d.get_index_type<sell_exchange_index>().indices().get<by_id>();
    auto itr_min = buy_fundraise.begin();
   
    while (itr_min != buy_fundraise.end() && result.size()<=limit)
    {
      if (itr_min->sell.asset_id != sell && (ufiletermask &1) == 1)
      {
        itr_min++;
        continue;
      }
      else if (itr_min->buy.asset_id != by && (ufiletermask &2) == 2)
      {
        itr_min++;
        continue;
      }      
      if( !itr_min->enable)
      {
        itr_min++;
        continue;
      }     
      if(uindex >= start)
         result.push_back(itr_min->id);
      uindex++;
      itr_min++;
    }
    if(usort >=1)
    {
      std::sort(result.begin(), result.end(), [usort,&d](const sell_exchange_id_type &a, const sell_exchange_id_type &b) -> bool 
      {
       const auto &a_ = a(d);
       const auto &b_ = b(d);       
       if(usort == 1)
       {
         return a_.sell.amount > b_.sell.amount;
       }
       else  if(usort == 2)
       {
         return a_.buy.amount > b_.buy.amount;
       }       
       else 
         return a_.id > b_.id;        
      });
    }
    return result;
}
 
vector<sell_exchange_id_type>  finance_api::list_my_sell_exchange(account_id_type account_id,uint64_t start,uint64_t limit)
{
  try{
    database &d = *_app.chain_database();
    uint64_t uindex = 0;
    vector<sell_exchange_id_type> result;
    const auto &crowd_index = d.get_index_type<sell_exchange_index>().indices().get<by_id>();
    auto itr_min = crowd_index.rbegin();
    while (itr_min != crowd_index.rend() && result.size()<=limit) 
    {
      if (itr_min->issuer == account_id) 
      {
        if(uindex >= start)
           result.push_back(itr_min->id);
        uindex++;
      }      
      itr_min++;
    }
    return result;
   } FC_CAPTURE_AND_RETHROW((account_id)(start)(limit))
}
vector<buy_exchange_id_type>  finance_api::list_my_buy_exchange(account_id_type account_id, uint64_t start,uint64_t limit)
{
  try{
    database &d = *_app.chain_database();
    uint64_t uindex = 0;
    vector<buy_exchange_id_type> result;
    const auto &crowd_index = d.get_index_type<buy_exchange_index>().indices().get<by_id>();
    auto itr_min = crowd_index.rbegin();
    while (itr_min != crowd_index.rend() && result.size()<=limit) 
    {
      if(itr_min->issuer == account_id)    
      {
        if(uindex >= start)   
           result.push_back(itr_min->id);
        uindex++;
      }
      itr_min++;
    }
    return result;
   } FC_CAPTURE_AND_RETHROW((account_id)(start)(limit))
} 

optional<collateral_cfg>     admin_api::get_collaboration(const account_id_type admin_id,const asset_id_type loan,const asset_id_type lend,const optional<bitlender_key> keyin)
{
   try{       
    database &d  = *_app.chain_database();    
    auto &lo     = loan(d);
    auto &le     = lend(d);
    auto& feed_options = d.get_index_type<asset_exchange_feed_index>().indices().get<by_feeds>();
    const auto itr = feed_options.find(boost::make_tuple(loan,lend, 1));
    if(itr == feed_options.end() || itr ->current_feed.is_zero())
        return optional<collateral_cfg>();

    bitlender_key key;
    if (keyin.valid()) key = *keyin;

    collateral_cfg cfg;    
    cfg.loan    = loan;
    cfg.invest  = lend;
    cfg.rate    =  itr ->current_feed.maintenance_collateral_ratio;
    cfg.interest = 0;
    cfg.setting = 0.0;

    if (!d.is_object(admin_id)) return cfg;

    auto &a      = admin_id(d); 
    auto iter1   = a.configs.find("collateral_ratio_" + key.get_index() + lo.symbol);
    if(iter1 != a.configs.end())
    {      
      variant var2 = fc::json::from_string(iter1->second);
      variant_object var_obj2;
      fc::from_variant(var2, var_obj2);
      auto iter2 = var_obj2.find(lo.symbol);
      if(iter2 != var_obj2.end())
      {       
        variant_object var_obj3 = iter2->value().get_object();        
        auto iter3 = var_obj3.find(le.symbol);
        if(iter3 != var_obj3.end())
        {
          const double dv  = iter3->value().as_double();  
          cfg.rate += uint32_t(dv * cfg.rate/100.0);
          cfg.setting = dv;
        }
      }
    }

    auto iter4   = a.configs.find("add_rate_" + key.get_index() + lo.symbol);
    if(iter4 != a.configs.end())
    {      
      variant var2 = fc::json::from_string(iter4->second);
      variant_object var_obj2;
      fc::from_variant(var2, var_obj2);
      auto iter2 = var_obj2.find(lo.symbol);
      if(iter2 != var_obj2.end())
      {       
        variant_object var_obj3 = iter2->value().get_object();        
        auto iter3 = var_obj3.find(le.symbol);
        if(iter3 != var_obj3.end())
        {
          const double dv  = iter3->value().as_double();  
          cfg.interest = dv;          
        }
      }
    }
    return cfg;
   } FC_CAPTURE_AND_RETHROW((admin_id)(loan)(lend))
}
 
fc::variant_object admin_api::get_locktoken_recommend(account_id_type admin_id)  
{
   try {

    graphene::chain::database& db = *_app.chain_database();
    fc::mutable_variant_object result_obj;
    const auto &idx = admin_id(db);    
    auto iter = idx.configs.find("locktoken_recommend");
    if (iter == idx.configs.end())
      return result_obj;
    uint32_t index = 0;
    uint32_t period = 0;
    database_api dbapi(db , _app);
    vector<lock_recommend> recommend= fc::json::from_string(iter->second).as<vector<lock_recommend>> (4);
    for(const auto &re : recommend )
    {
      fc::optional<locktoken_option_object>  obj = dbapi.get_locktoken_option(re.asset_id,re.lockmode,true);
      if(!obj.valid()) continue;
      int32_t levelindex = 1;
      if(re.lockmode == locktoken_node) levelindex = re.automode;
      if(re.lockmode == locktoken_fixed) period = re.period;
      else period = 0;
      const auto itr = obj->lockoptions.locktoken_rates.find(period);
      if(itr == obj->lockoptions.locktoken_rates.end()) continue;
      if(itr->second.size()<=0) continue;
      uint32_t rate = 0;
      uint64_t level = 0;
      for(const auto &re1 : itr->second ) {
        rate  = re1.second;
        level = re1.first.value;
        levelindex --;
        if(levelindex <=0) break;
      }
      const auto &ass = re.asset_id(db);
      asset_info ainfo;
      ass.get_info(ainfo);
      fc::mutable_variant_object aa;
      fc::variant info_j,option_j,asset_j;
      fc::to_variant(*obj, option_j, GRAPHENE_NET_MAX_NESTED_OBJECTS);
      fc::to_variant(ainfo, asset_j, GRAPHENE_NET_MAX_NESTED_OBJECTS);
      fc::to_variant(re, info_j, GRAPHENE_NET_MAX_NESTED_OBJECTS);
      fc::from_variant(info_j, aa, GRAPHENE_NET_MAX_NESTED_OBJECTS);
      aa.set("rate",rate);
      aa.set("level",level);
      aa.set("option",option_j);
      aa.set("assetinfo",asset_j); 
      result_obj.set(fc::to_string(index), aa);
      index ++;
    }     
    return result_obj;
   }FC_CAPTURE_AND_RETHROW((admin_id))
}

optional<carrier_cfg> admin_api::get_carrier(const account_id_type admin_id,const asset_id_type loan,optional<bitlender_key> keyin)
{   try{  
  if(!(*_app.chain_database()).is_object(admin_id))
    return optional<carrier_cfg>();

  bitlender_key key;
  if (keyin.valid()) key = *keyin;

  database &d = *_app.chain_database();  

  auto &op  = bitlender_calc::get_option(d,loan);
  auto &opt = op.get_optionItem(key);

 
  auto &a     = admin_id(d);
  auto &s     = loan(d);
  auto iter   = a.configs.find("carrierList" + key.get_index());
  if(iter == a.configs.end())
    return optional<carrier_cfg>();
  variant var = fc::json::from_string(iter->second);
  variant_object var_obj;
  fc::from_variant(var, var_obj);
  auto iter_ = var_obj.find(s.symbol);
  if(iter_ == var_obj.end())
     return optional<carrier_cfg>();
  const variant_object &var1 = iter_->value().get_object();
  carrier_cfg result;
  result.assetid = s.id;
  const auto &accounts_by_name = d.get_index_type<account_index>().indices().get<by_name>();
  {

    string saccount = var1["lendCarrier"].get_string();
    auto itr = accounts_by_name.find(saccount);
    FC_ASSERT(itr != accounts_by_name.end(), "${x}", ("x", saccount));
    result.lendaccount = itr->id;
    auto &lend = result.lendaccount(d);
    result.lendCarrier = lend.get_carrier_id(d);
    auto &lend_c = result.lendCarrier(d);
    result.lendenable = lend_c.is_enable();
    result.lendauthor = lend_c.get_author(d, loan);
    if(result.lendauthor.valid())
    {
      auto &author = (*result.lendauthor)(d);
      result.lendauthor_account = author.author_account;
      result.lendauthor_url  = author.url;
      result.lendauthor_name = lend.name;
    }
  }
  {
    string saccount = var1["investCarrier"].get_string();
    auto itr = accounts_by_name.find(saccount);
    FC_ASSERT(itr != accounts_by_name.end(), "${x}", ("x", saccount));
    result.investaccount = itr->id;
    auto &invest = result.investaccount(d);
    result.investCarrier = invest.get_carrier_id(d);
    auto &lend_i = result.investCarrier(d);
    result.investenable = lend_i.is_enable();
    result.investauthor = lend_i.get_author(d, loan);
    if (result.investauthor.valid())
    {
      auto &author = (*result.investauthor)(d);
      result.investauthor_account = author.author_account;
      result.investauthor_url = author.url;
      result.investauthor_name = invest.name;
    }
  }

  if(opt.loan_carriers.find(result.lendaccount) == opt.loan_carriers.end())
    result.lendvalidate = false;
  if(opt.invest_carriers.find(result.investaccount) == opt.invest_carriers.end())
    result.investvalidate = false;

  return result;
  } FC_CAPTURE_AND_RETHROW((admin_id)(loan))
}
flat_map<asset_cfg,vector<asset_cfg>>  admin_api::get_lending_asset(const account_id_type admin_id,const optional<bitlender_key> keyin)
{
  try
  {
    database &d = *_app.chain_database();
    flat_map<asset_cfg, vector<asset_cfg>> result;
    vector<asset_cfg> def;
    bitlender_key key;
    if (keyin.valid()) key = *keyin;
    const auto &asset_id = d.get_index_type<asset_index>().indices().get<by_id>();
    for (const asset_object &asset_member : asset_id)
    {
      //可抵押币缺省为所有币重
      if ((asset_member.uasset_property & ASSET_LENDER) == ASSET_LENDER)
      {
        asset_cfg add_cfg;
        add_cfg.id         = asset_member.id;
        add_cfg.symbol     = asset_member.symbol;
        add_cfg.max_supply = asset_member.options.max_supply;
        add_cfg.precision  = asset_member.precision;
        add_cfg.uasset_property= asset_member.uasset_property;
        add_cfg.total_core_in_lending = asset_member.dynamic_asset_data_id(d).total_core_in_lending;
        def.push_back(add_cfg);
      }
    }
    std::sort(def.begin(), def.end(), [&](const asset_cfg &a, const asset_cfg &b) -> bool {
      return !(a < b);
    });

    if (!(admin_id.is_null() || !d.is_object(admin_id)))
    {
      auto &a = admin_id(d);
      const auto &asset_name = d.get_index_type<asset_index>().indices().get<by_symbol>();
      vector<string> kindex = key.get_index(keyin);      
      for(const auto index : kindex) {
        auto iter = a.configs.find("carrierList"+index);
        if (iter != a.configs.end())
        {
          variant var = fc::json::from_string(iter->second);
          variant_object var_obj;
          fc::from_variant(var, var_obj);
          auto iter_ = var_obj.begin();
          while (iter_ != var_obj.end())
          {
            auto iter_a = asset_name.find(iter_->key());
            if (iter_a != asset_name.end())
            {
              asset_cfg add_cfg;
              add_cfg.id      = iter_a->id;
              add_cfg.symbol  = iter_a->symbol;
              add_cfg.max_supply = iter_a->options.max_supply;
              add_cfg.precision  = iter_a->precision;
              add_cfg.uasset_property= iter_a->uasset_property;
              add_cfg.total_core_in_lending = iter_a->dynamic_asset_data_id(d).total_core_in_lending;
              result[add_cfg] = def;
            }
            iter_++;
          }
        }
      }
    }   
    //如果没有管理员
    if (0)//result.size() <= 0)
    {
      for (const asset_object &asset_member : asset_id)
      {
        //必须有借贷参数
        if ((asset_member.uasset_property & ASSET_LOAN_OPTION) == ASSET_LOAN_OPTION)
        {
          asset_cfg add_cfg;
          add_cfg.id      = asset_member.id;
          add_cfg.symbol  = asset_member.symbol;
          add_cfg.max_supply = asset_member.options.max_supply;
          add_cfg.precision  = asset_member.precision;
          add_cfg.uasset_property= asset_member.uasset_property;
          add_cfg.total_core_in_lending = asset_member.dynamic_asset_data_id(d).total_core_in_lending;
          result[add_cfg] = def;
        }
      }
    }    
    auto &bitlender_options = d.get_index_type<bitlender_option_index>().indices().get<by_id>();
    for (const auto &list : result)
    {
      for (const bitlender_option_object &option : bitlender_options)
      {
        if (option.asset_id == list.first.id && option.sproduct == lender_desc)
        {
          const bitlender_option &op = option.get_option(key, false);
          if (op.allowed_collateralize.size() > 0)
          {
            result[list.first].clear();
            for (auto &a_s : op.allowed_collateralize)
            {
              auto iter_a = asset_id.find(a_s);
              if (iter_a != asset_id.end() && (iter_a->uasset_property & ASSET_LENDER) == ASSET_LENDER)
              {
                asset_cfg add_cfg;
                add_cfg.id         = iter_a->id;
                add_cfg.symbol     = iter_a->symbol;
                add_cfg.max_supply = iter_a->options.max_supply;
                add_cfg.precision  = iter_a->precision;
                add_cfg.uasset_property= iter_a->uasset_property;
                add_cfg.total_core_in_lending = iter_a->dynamic_asset_data_id(d).total_core_in_lending;
                result[list.first].push_back(add_cfg);
              }
            }
            std::sort(result[list.first].begin(), result[list.first].end(), [&](const asset_cfg &a, const asset_cfg &b) -> bool {
                  return !(a < b);
            });
          }
        }
      }
    }
    return result;
  }
  FC_CAPTURE_AND_RETHROW((admin_id))
}
vector<gateway_cfg> admin_api::get_gateway(const account_id_type admin_id,const asset_id_type asset_id)
{
  try{      
   database &d = *_app.chain_database();
   auto &ass   = asset_id(d);  
   vector<gateway_cfg>       result;
   vector<account_id_type>   gateway;
   for(const auto &aaa : ass.whitelist_gateways)
     gateway.push_back(aaa);   
   try
   {
     if (d.is_object(admin_id))
     {
       auto &a = admin_id(d);
       auto iter = a.configs.find("gatewayList");
       if (iter != a.configs.end())
       {
         variant var = fc::json::from_string(iter->second);
         variant_object var_obj;
         fc::from_variant(var, var_obj);
         auto iter_s = var_obj.find(ass.symbol);
         if (iter_s != var_obj.end())
         {
           string sasset = iter_s->value().get_string();
           const auto &accounts_by_name = d.get_index_type<account_index>().indices().get<by_name>();
           auto itr = accounts_by_name.find(sasset);
           if (itr != accounts_by_name.end())
           {
             if (gateway.end() != std::find(gateway.begin(), gateway.end(), itr->id))
             {
               gateway.clear();
               gateway.push_back(itr->id);
             }
           }
         }
       }
     }
   }
   catch(...)
   {
   }  
   for(const auto &ag : gateway)
   {
      auto &acc   = ag(d);  
      auto &agg   = acc.get_gateway_id(d)(d);
      auto &aggd  = agg.dynamic_id(d);
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
      add.author_id         = agg.get_author(d, asset_id);
      add.enable            = agg.is_enable();
      if(add.author_id.valid())
      {
        add.author_account_id = (*add.author_id)(d).author_account; 
        auto &author    = (*add.author_id)(d);
        add.author_url  = author.url;
        auto &account   = (*add.author_account_id)(d);
        add.author_name = account.name;
      }
      result.push_back(add);
   } 
   return result;
  } FC_CAPTURE_AND_RETHROW((admin_id)(asset_id))
}
 

vector<asset> bitlender_api::get_invest_fee(bitlender_order_id_type order_id, uint32_t repay_period) const
{
  try
  {
    vector<asset> result;
    database &d = *_app.chain_database();
    auto &order = order_id(d);
    FC_ASSERT(order.is_investfinish() || order.is_interest() || order.is_principaloverdue(), "order  '${id1}' state '${id2}' ", ("id1", order.id)("id2", order.order_state));
    auto sa_it1 = order.repay_interest.find(repay_period);
    FC_ASSERT(sa_it1 != order.repay_interest.end());
    FC_ASSERT(!sa_it1->second.is_finish(), "repay_interest statue  '${id1}' ", ("id1", sa_it1->second.interest_state));
    asset ifee = asset(0, GRAPHENE_CORE_ASSET);
    if (sa_it1->second.is_overdue())
      ifee = bitlender_calc::get_overdue_interest_fee(d, order, repay_period);
    asset irate = bitlender_calc::get_repay_interest_rate(d, order, repay_period);
    result.push_back(ifee);
    result.push_back(irate);
    return result;
  }
  FC_CAPTURE_AND_RETHROW((order_id)(repay_period))
   }
   vector<asset>         bitlender_api::get_repay_fee(bitlender_order_id_type order_id) const
   {try{
      vector<asset> result;   
      database& d = *_app.chain_database();
      auto &order = order_id(d);
      auto option = order.option_id(d);
  
      asset ifee     = asset(0,order.amount_to_loan.asset_id); 
      asset iloan    = order.amount_to_loan; 
      if(order.is_principalfinish()||order.is_recycle())
        ifee = asset(0,order.amount_to_loan.asset_id); 
      else if (order.is_principaloverdue())
        ifee = bitlender_calc::get_overdue_repay_principal_fee(d, order);       

      asset iinvest;
      if (order.is_recycle())
        iinvest = bitlender_calc::fill_pre_interest(d, order.expect_principal_time + order.offset_time + order.overdue_expiration_time, order, false);
      else
        iinvest = bitlender_calc::fill_pre_interest(d, d.head_block_time() + order.offset_time, order, false);

      result.push_back(ifee);
      result.push_back(iloan);
      result.push_back(iinvest);
      return result;
   } FC_CAPTURE_AND_RETHROW( (order_id)) }
   vector<asset>   bitlender_api::get_invest_process(bitlender_order_id_type order_id) const
   {
       try{
      vector<asset> result;   
      database& d = *_app.chain_database();
      auto &order = order_id(d);
      result.push_back(order.amount_to_loan);
      asset invest = bitlender_calc::get_invest_total(d,order);      
      result.push_back(invest);  
      return result;    
    } FC_CAPTURE_AND_RETHROW( (order_id)) }


    bool bitlender_api::can_edit_locktoken_option(locktoken_option_id_type option_id, asset_id_type asset_id,  account_id_type account_id) const
    { try{
      database &d = *_app.chain_database();
      if(d.is_object_exist(option_id)) 
      {
        const auto & option = option_id(d); 
        if (option.issuer == account_id)
          return true;
        global_property_object gp = d.get(global_property_id_type());
        if(d.get(GRAPHENE_COMMITTEE_ACCOUNT).active.account_auths.count(account_id))
          return true;   

      } else {
        global_property_object gp = d.get(global_property_id_type());
        if(d.get(GRAPHENE_COMMITTEE_ACCOUNT).active.account_auths.count(account_id))
          return true;   
      }
      return false;
    }       FC_CAPTURE_AND_RETHROW((option_id)(account_id))    }
    bool bitlender_api::can_edit_bitlender_option(bitlender_option_id_type option_id, account_id_type account_id,optional<bitlender_key> keyin) const
    { try{

        bitlender_key key;
        if( keyin.valid()) key = *keyin;
        key.validate();
        database &d = *_app.chain_database();
        auto option = option_id(d);
        auto author = option.author(d);
        auto opt = option.get_option(key,false);
        // auto rate_op = option.get_rate(key);     

        if (opt.platform_id == account_id)
          return true;
        if (option.gateways.find(account_id) != option.gateways.end())
          return true;
        if (opt.carriers.find(account_id) != opt.carriers.end())
          return true;
        if (author.owner.account_auths.find(account_id) != author.owner.account_auths.end())
          return true;
        if (author.active.account_auths.find(account_id) != author.active.account_auths.end())
          return true;
        return false;
     } FC_CAPTURE_AND_RETHROW( (option_id)(account_id)) }
      optional<bitlender_order_data>  bitlender_api::get_loan_order(const bitlender_order_id_type orderid)
      { try{
          bitlender_order_data re;
          database &db = *_app.chain_database();
          auto order = orderid(db);
          re.get_object(db,order);
          return re;
      }  FC_CAPTURE_AND_RETHROW((orderid))  }
      vector<bitlender_invest_info>    bitlender_api::get_invest_orders_by_id(const vector<bitlender_invest_id_type> &orderid) const
      { try{
        vector<bitlender_invest_info>  result;
        database &db = *_app.chain_database();
        for(const auto id : orderid)
        {
          bitlender_invest_info re;
          const auto &invest = id(db);
          invest.get_info(db,re);
          result.push_back(re);
        }
        return result;
       } FC_CAPTURE_AND_RETHROW((orderid))  }
      vector<bitlender_order_info>     bitlender_api::get_loan_orders_by_id(const vector<bitlender_order_id_type> &orderid) const
      {
        try{
        vector<bitlender_order_info>  result;
        database &db = *_app.chain_database();
        for(const auto id : orderid)
        {
          bitlender_order_info re;
          const auto &order = id(db);
          order.get_info(db,re);
          result.push_back(re);
        }
        return result;
      } FC_CAPTURE_AND_RETHROW((orderid))  }
      optional<bitlender_invest_data>  bitlender_api::get_invest_order(const bitlender_invest_id_type orderid,bool borderinfo)
      { try{
          bitlender_invest_data re;
          database &db = *_app.chain_database();
          const auto &order = orderid(db);
          re.get_object(db,order,borderinfo);
          return re;
      }  FC_CAPTURE_AND_RETHROW((orderid))  }

      vector<bitlender_order_id_type> bitlender_api::list_carrier_orders(account_id_type carrier, uint32_t ustate)
      {
        try
        {
          database &d = *_app.chain_database();
          vector<bitlender_order_id_type> result;
          const auto &loan_index = d.get_index_type<bitlender_order_index>().indices().get<by_time>();
          for (auto &iter : loan_index)
          {
            if (ustate == ORDER_RECYCLE)
            {
              if ((iter.order_state == ORDER_RECYCLE || iter.order_state == ORDER_RECYCLE_FINISHING) && iter.is_recycle_issuer(d, carrier))
                result.push_back(iter.id);
            }
            else if (ustate == ORDER_SQUEEZE)
            {
              if ((iter.order_state == ORDER_SQUEEZE || iter.order_state == ORDER_RECYCLE_SQEEZEING) && iter.is_recycle_issuer(d, carrier))
                result.push_back(iter.id);
            }
            else
            {
              if (iter.order_state == ustate && iter.carrier == carrier)
                result.push_back(iter.id);
            }
          }
          return result;
        }
        FC_CAPTURE_AND_RETHROW((carrier)(ustate)) }

    vector<bitlender_order_id_type>     bitlender_api::list_bitlender_order(const account_id_type account_id,const vector<uint32_t> &type,const vector<asset_id_type> &loan,const vector<asset_id_type> &lender,uint32_t period, uint32_t ufiletermask, uint32_t usort)
    { try{
      database& d = *_app.chain_database();      
      vector<std::reference_wrapper<const bitlender_order_object>> refs;
      vector<bitlender_order_id_type>    result;
      const auto &loan_index = d.get_index_type<bitlender_order_index>().indices().get<by_time>();
      FC_ASSERT(usort<=6);
      refs.reserve(loan_index.size());
      auto iter = loan_index.rbegin();
      while(iter != loan_index.rend()) 
      {  
         if(account_id!=GRAPHENE_EMPTY_ACCOUNT && iter->issuer == account_id)
         {
            iter ++;
            continue;
         }
         if(!(iter->is_loan() || iter->is_investing()))
         {
            iter ++;
            continue;
         }         
         if(ufiletermask & 0x01)
         { 
           if(std::count(loan.begin(),loan.end(),iter->amount_to_loan.asset_id)<=0)
           {
             iter ++;
             continue;
           }             
         }
         if(ufiletermask & 0x02)
         { 
           if(std::count(lender.begin(),lender.end(), iter->amount_to_collateralize.asset_id)<=0)
           {
             iter ++;
             continue;
           }
         }         
         if((ufiletermask & 0x04) && type.size() == 1)
         { 
           if((ufiletermask & 0x08) && type[0] == loan_month)
           {
             if(period == 7) 
             { 
               if( iter->loan_period <=6)
               {
                iter ++;
                continue;
               }  
             }
             else if(iter->loan_period != period)
             {
              iter ++;
              continue;
             }
           }
           else if((ufiletermask & 0x08) && type[0] == loan_day)
           {           
             if(period == 7) 
             { 
               if( iter->loan_period <=30)
               {
                iter ++;
                continue;
               }  
             }  
             else if(iter->loan_period <= (period -1) *7 || iter->loan_period > period *7)
             {
              iter ++;
              continue;
             }
           }
         }
         if(ufiletermask & 0x08)
         { 
           if(std::count(type.begin(),type.end(), iter->repayment_type.repayment_period_uint)<=0)
           {
             iter ++;
             continue;
           }
         }
         refs.emplace_back(std::cref(*iter));      
         iter ++;                   
      }        

      if(usort >1)
      {
        std::sort(refs.begin(), refs.end(),[&d,usort](const bitlender_order_object& a, const bitlender_order_object& b)->bool 
        {
          if(usort ==2)   //借款金额
          {
            return a.amount_to_loan.amount.value > b.amount_to_loan.amount.value;
          }
          else if(usort ==3)   //投资进度
          {
            share_type a_invest = 0;
            for(auto &ita : a.interest_book)
              a_invest += ita.second.invest;
            share_type b_invest = 0;
            for(auto &itb : b.interest_book)
              b_invest += itb.second.invest;  
            if((a_invest.value *100.0 /a.amount_to_loan.amount.value) == (b_invest.value *100.0 / b.amount_to_loan.amount.value) )
             return a.id > b.id;
            else 
             return (a_invest.value * 100.0 / a.amount_to_loan.amount.value) > (b_invest.value * 100.0 / b.amount_to_loan.amount.value);
          }  
          else if(usort ==4)   //抵押倍数
          {
            asset a_c = bitlender_calc::get_recycle_loan_by_collateralize(d, a.amount_to_loan, a.amount_to_collateralize);
            asset b_c = bitlender_calc::get_recycle_loan_by_collateralize(d, b.amount_to_loan, b.amount_to_collateralize);
            if((a_c.amount.value/a.amount_to_loan.amount.value) == (b_c.amount.value/b.amount_to_loan.amount.value)) 
             return a.id > b.id;
            else 
             return (a_c.amount.value/a.amount_to_loan.amount.value) > (b_c.amount.value/b.amount_to_loan.amount.value);
          }
          else if(usort ==5)   //预计收益率
          {
            share_type a_invest = 0;
            for(auto &ita : a.repay_interest)
              a_invest += ita.second.expect_repay_interest.amount;
            share_type b_invest = 0;
            for(auto &itb : b.repay_interest)
              b_invest += itb.second.expect_repay_interest.amount;  
            if((a_invest.value *100.0 /a.amount_to_loan.amount.value) == (b_invest.value *100.0 / b.amount_to_loan.amount.value))
             return a.id > b.id;
            else 
             return (a_invest.value *100.0 /a.amount_to_loan.amount.value) > (b_invest.value *100.0 / b.amount_to_loan.amount.value);  
          }            
          else if(usort == 6)   //借款金额
          {
            uint64_t aday = a.repayment_type.repayment_period_uint == loan_month ? a.loan_period * 30 : a.loan_period;
            uint64_t bday = b.repayment_type.repayment_period_uint == loan_month ? b.loan_period * 30 : b.loan_period;
            if(aday == bday) 
             return a.id > b.id;
            else 
             return aday > bday;
          }    
          return false;
        });
      }      
      result.reserve(refs.size());
      for(const bitlender_order_object &it : refs)
         result.push_back(it.id);        
      return result;
     } FC_CAPTURE_AND_RETHROW( (loan)(lender)(period)(ufiletermask)(usort)) }
    vector<share_type>     bitlender_api::get_precent_value(share_type amount, vector<share_type> base)
    {  try{
        vector<share_type> ret;      
        bitlender_calc::get_precent_value(amount, base, ret);        
        return ret;
    } FC_CAPTURE_AND_RETHROW((amount)) }
    vector<asset_object> bitlender_api::get_asset_by_property(uint32_t uproperty)
    {
      vector<asset_object> result;
      database& d = *_app.chain_database();
      const auto &asset_id = d.get_index_type<asset_index>().indices().get<by_id>();
      for (const asset_object &asset_member : asset_id)
      {
        if ((asset_member.uasset_property & uproperty) == uproperty || uproperty == 0x7FFFFFFF)
          result.push_back(asset_member);
      }
      return result;
    }
        
    //根据法币计算抵押基础数字货币
    asset bitlender_api::get_loan_collateralize(const asset &amount_to_loan,asset_id_type collateralize_id)
    {
       try{
        database& d     = *_app.chain_database();
        auto   a        = amount_to_loan.asset_id(d);
        auto   o        = bitlender_calc::get_option(d, amount_to_loan.asset_id);
        auto   oprice   = bitlender_calc::get_feeds(d, amount_to_loan.asset_id,collateralize_id,true);

       oprice.current_feed.validate();

        if (/*amount_to_loan.amount.value < opt.min_loan_amount ||*/ amount_to_loan.amount.value >= a.options.max_supply / 3)
           return asset(0,collateralize_id); 

        return  amount_to_loan * oprice.current_feed.settlement_price;
        
    } FC_CAPTURE_AND_RETHROW( (amount_to_loan)(collateralize_id)) }

   fc::variant  bitlender_api::get_pay_fee(const bitlender_order_id_type order_id) const
   { try{
       bitlender_order_object bo;
       database &d = *_app.chain_database();
       auto &a  = order_id(d);
       bo = a;

       return bo.to_variant();
   } FC_CAPTURE_AND_RETHROW( (order_id)) }
   fc::variant  bitlender_api::get_loan_info(const asset &amount_to_loan,const uint32_t loan_period,const uint32_t &interest_rate,const uint32_t bid_period,const uint32_t collateral_ratio ,const asset &amount_to_collateralize,const optional<bitlender_key> keyin) const
   { 
     try{
     bitlender_order_object bo;
     bitlender_key key;
     if(keyin.valid()) key = *keyin;
     database &d = *_app.chain_database();
     auto &a  = amount_to_loan.asset_id(d);     
     //auto &c  = amount_to_collateralize.asset_id(d);    

     auto   o        = bitlender_calc::get_option(d, amount_to_loan.asset_id);
     auto   oprice   = bitlender_calc::get_feeds(d, amount_to_loan.asset_id,amount_to_collateralize.asset_id,true);

    auto opt         = o.get_option(key);
    auto rate_op     = o.get_rate(key);

     oprice.current_feed.validate();  
     oprice.current_feed.settlement_price.validate();    
     

     if (loan_period <= 0 || loan_period > opt.max_repayment_period)
       return bo.to_variant();

     if (/*amount_to_loan.amount.value < opt.min_loan_amount ||*/ amount_to_loan.amount.value >= a.options.max_supply / 3)
       return bo.to_variant(); 

    if(opt.allowed_collateralize.size()>0)
    {       
       FC_ASSERT(opt.allowed_collateralize.find(amount_to_collateralize.asset_id) != opt.allowed_collateralize.end(), "asset  '${id1}' must in  allowed collateralize ", ("id1",  amount_to_collateralize.asset_id));
    }

     auto sa_it1 = rate_op.find(loan_period);
     FC_ASSERT(sa_it1 != rate_op.end());

     auto &rate =sa_it1->second;    
     //计算所需要的平台费用
     asset ufee  = bitlender_calc::get_fee_collateralize_by_loan(d,  amount_to_loan,amount_to_collateralize,o.fee_mode,key);
     //计算所需要的风险保证金
     asset urisk  = bitlender_calc::get_risk_collateralize_by_loan(d,  amount_to_loan,amount_to_collateralize,o.fee_mode,key);
     //计算运营商法币费用
     asset ucarrierfee  = bitlender_calc::get_carrier_fee(d,  amount_to_loan,amount_to_collateralize,key);
    //计算法币所需要的最少抵押币
     asset ucoll;

     if(collateral_ratio >0)
      ucoll  = bitlender_calc::get_user_collateralize_by_loan(d,  amount_to_loan,amount_to_collateralize,collateral_ratio);
    else  
      ucoll  = bitlender_calc::get_min_collateralize_by_loan(d,  amount_to_loan,amount_to_collateralize);

     bo.option_id      = o.id;
     bo.amount_to_loan = amount_to_loan;

     bo.loan_period           = loan_period;
     bo.interest_rate         = rate;     
     

     if(interest_rate>rate.interest_rate)  //投资人自己设置利息，必须大于系统利息，小于系统最大利息
        bo.interest_rate.interest_rate = interest_rate;

      bo.amount_to_collateralize = ucoll;
      bo.collateralize_fee       = ufee;
      bo.collateralize_risk      = urisk;
      bo.carrier_fee             = ucarrierfee;

      bo.bid_period         = bid_period;
      bo.fee_mode           = o.fee_mode ; 
      bo.loan_time          = d.head_block_time();
      bo.expiration_time    = bo.loan_time   +  fc::seconds(bid_period);//fc::seconds(300);
      bo.repayment_type     = key;
      if (o.fee_mode == fee_invest)
        bo.lock_collateralize = bo.amount_to_collateralize + bo.collateralize_fee + bo.collateralize_risk;
      else
            bo.lock_collateralize = bo.amount_to_collateralize;
            
      bo.order_state            = ORDER_NORMAL;

      bo.price_settlement       = oprice.current_feed;      
      
      bo.collateral_rate = oprice.current_feed.maintenance_collateral_ratio;
      bitlender_calc::get_repay_interest_data(opt, bo,bo.loan_time );

      bitlender_order_data bodata;
      bodata.get_object(d,bo);
      return bodata.to_variant();
    } FC_CAPTURE_AND_RETHROW( (amount_to_loan)(loan_period)(interest_rate)(bid_period)(amount_to_collateralize)) }

   flat_set<account_id_type> bitlender_api::get_feeders(const asset_id_type base)
   {
     database& d = *_app.chain_database();
     const auto &option = bitlender_calc::get_option(d, base);
     FC_ASSERT(option.feed_option.flags <= 3);
     flat_set<account_id_type> result;
     if (option.feed_option.flags == 1)
     {
       const auto &author = option.author(d);
       if (author.active.account_auths.size() == 1 && author.active.account_auths.begin()->first == GRAPHENE_COMMITTEE_ACCOUNT)
       {
         for (auto &a : d.get(GRAPHENE_COMMITTEE_ACCOUNT).active.account_auths)
         {
           result.insert(a.first);
         }
       }
       else
       {
         for (auto &a : author.active.account_auths)
         {
           result.insert(a.first);
         }
       }
     }
     else if (option.feed_option.flags == 2)
     {
       for (auto &a : d.get(GRAPHENE_WITNESS_ACCOUNT).active.account_auths)
       {
         result.insert(a.first);
       }
     }
     else if (option.feed_option.flags == 3)
     {
       for (auto &a : d.get(GRAPHENE_COMMITTEE_ACCOUNT).active.account_auths)
       {
         result.insert(a.first);
       }
     }
     else if (option.feed_option.flags == 0)
     {
       for (auto &a : option.feed_option.feeders)
         result.insert(a);
     }
     else
     {
       FC_ASSERT(false,"1");
     }
     return result;
   }

   //计算公式
   vector<string> bitlender_api::calc_string(const bitlender_order_id_type order_id,const variant_object& option_values,uint32_t utype,uint32_t uperiod)
   {
      try{
        vector<string> result;
        bitlender_key key;
        // 1  逾期还息
        if(utype == 1)
        {          
          database &d   = *_app.chain_database();
          auto &order   = order_id(d);
          auto &option  = order.option_id(d);
           auto opt     = option.get_option(key);
          
          time_point_sec now   = d.head_block_time() + order.offset_time;
          auto           sa_it = order.repay_interest.find(uperiod);

          char     sresult[256];

          if(sa_it == order.repay_interest.end()) 
            return result;
         //计算提前还款利息
          asset uremain1 = sa_it->second.expect_repay_interest * opt.overdue_penalty_interest_rate / 100;
          asset uremain2 = sa_it->second.expect_repay_interest *(sa_it->second.expect_time.get_date(now) * opt.overdue_penalty_date_interest_rate) / 10000;
          asset uremain  = uremain1 + uremain2;
          sprintf(sresult,"%s * %.2f + %s * %d * %.2f = %s", fc::to_string(sa_it->second.expect_repay_interest.amount.value).c_str(),\
                                                             opt.overdue_penalty_interest_rate / 100.0, \
                                                             fc::to_string(sa_it->second.expect_repay_interest.amount.value).c_str(),\
                                                             sa_it->second.expect_time.get_date(now), \
                                                             opt.overdue_penalty_interest_rate / 100.0, \
                                                             fc::to_string(uremain.amount.value).c_str());
          result.push_back(sresult);
          memset(sresult,0,sizeof(sresult));
          sprintf(sresult, "%s + %s + %s = %s", fc::to_string(sa_it->second.expect_repay_interest.amount.value).c_str(),\
                                                fc::to_string(uremain1.amount.value).c_str(),\
                                                fc::to_string(uremain2.amount.value).c_str(),\
                                                fc::to_string(sa_it->second.expect_repay_interest.amount.value+ uremain1.amount.value+uremain2.amount.value).c_str());
          result.push_back(sresult);
        }
        // 2  提前还款
        else if(utype == 2)
        {          
           database &d   = *_app.chain_database();
           auto &order   = order_id(d);
           //auto &option  = order.option_id(d);
           char     sresult[256];
           asset uremain = bitlender_calc::get_pre_repay_principal_fee(d,order);
           sprintf(sresult, "%s", fc::to_string(uremain.amount.value).c_str());
           result.push_back(sresult);
           memset(sresult,0,sizeof(sresult));
           sprintf(sresult, "%s", fc::to_string(order.amount_to_loan.amount.value).c_str());
           result.push_back(sresult);
           memset(sresult,0,sizeof(sresult));
           sprintf(sresult, "%s + %s  = %s ", fc::to_string(uremain.amount.value).c_str(),\
                                              fc::to_string(order.amount_to_loan.amount.value).c_str(),\
                                              fc::to_string(uremain.amount.value+order.amount_to_loan.amount.value).c_str());
           result.push_back(sresult);           
        }
        // 3  逾期还本
        else if(utype == 3)
        {          
           database &d   = *_app.chain_database();
           auto &order   = order_id(d);
           auto &option  = order.option_id(d);
           auto opt     = option.get_option(key);

           char     sresult[256];
           asset uremain  = bitlender_calc::get_overdue_repay_principal_fee(d,order);
           asset uremain1 = order.amount_to_loan * opt.overdue_liquidation_rate/100;
           sprintf(sresult, "%s", fc::to_string(uremain.amount.value-uremain1.amount.value).c_str());
           result.push_back(sresult);
           memset(sresult,0,sizeof(sresult));
           sprintf(sresult, "%s * %.2f = %s ", fc::to_string(order.amount_to_loan.amount.value).c_str(),\
                                               opt.overdue_liquidation_rate/100.0,\
                                               fc::to_string(uremain1.amount.value).c_str());
           result.push_back(sresult);
           memset(sresult,0,sizeof(sresult));
           sprintf(sresult, "%s", fc::to_string(order.amount_to_loan.amount.value).c_str());
           result.push_back(sresult);
           memset(sresult,0,sizeof(sresult));
           sprintf(sresult, "%s + %s + %s  = %s ", fc::to_string(order.amount_to_loan.amount.value).c_str(),\
                                                   fc::to_string(uremain.amount.value-uremain1.amount.value).c_str(),\
                                                   fc::to_string(uremain1.amount.value).c_str(), \
                                                   fc::to_string(uremain.amount.value+order.amount_to_loan.amount.value).c_str());
           result.push_back(sresult);           
        }
        // 4  抵押物
        else if(utype == 4)
        {
          bitlender_loan_operation update_op;
          fc::reflector<bitlender_loan_operation>::visit(
              fc::from_variant_visitor<bitlender_loan_operation>(option_values, update_op, GRAPHENE_MAX_NESTED_OBJECTS));

          char     sresult[256];
          database& d = *_app.chain_database();
          //auto &a  = update_op.amount_to_loan.asset_id(d);     
          //auto &c  = update_op.amount_to_collateralize.asset_id(d);
         
          auto   option = bitlender_calc::get_option(d, update_op.amount_to_loan.asset_id);
          auto   oprice = bitlender_calc::get_feeds(d, update_op.amount_to_loan.asset_id,update_op.amount_to_collateralize.asset_id,true);
          auto   opt    = option.get_option(key);
          oprice.current_feed.validate();
         

          //计算所需要的平台费用
          asset ufee  = bitlender_calc::get_fee_collateralize_by_loan(d,  update_op.amount_to_loan,update_op.amount_to_collateralize,option.fee_mode,key);
          //计算所需要的风险保证金
          asset urisk  = bitlender_calc::get_risk_collateralize_by_loan(d,  update_op.amount_to_loan,update_op.amount_to_collateralize,option.fee_mode,key);

          sprintf(sresult, "%s * %s / %s   * %.2f = %s ", fc::to_string(update_op.amount_to_loan.amount.value).c_str(), \
                                                                   fc::to_string(oprice.current_feed.settlement_price.quote.amount.value).c_str(), \
                                                                   fc::to_string(oprice.current_feed.settlement_price.base.amount.value).c_str(),\
                                                                   opt.platform_service_charge_rate / 1000.0,\
                                                                   fc::to_string(ufee.amount.value).c_str());
          result.push_back(sresult); 
          memset(sresult,0,sizeof(sresult));
          sprintf(sresult, "%s * %s / %s  *  %.2f = %s ",  fc::to_string(update_op.amount_to_loan.amount.value).c_str(), \
                                                                   fc::to_string(oprice.current_feed.settlement_price.quote.amount.value).c_str(), \
                                                                   fc::to_string(oprice.current_feed.settlement_price.base.amount.value).c_str(),\
                                                                   opt.risk_margin_rate / 1000.0,\
                                                                   fc::to_string(urisk.amount.value).c_str());           
          result.push_back(sresult); 
        }
        return result;
   } FC_CAPTURE_AND_RETHROW( (order_id)(utype)) } 
} } // graphene::app

