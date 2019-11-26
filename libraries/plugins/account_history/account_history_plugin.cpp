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

#include <graphene/account_history/account_history_plugin.hpp>

#include <graphene/app/impacted.hpp>

#include <graphene/chain/account_evaluator.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/config.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/chain/transaction_evaluation_state.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/thread/thread.hpp>

namespace graphene { namespace account_history {

struct balance_index
{
    uint64_t index =0;
    uint64_t pos = 0;
    uint64_t size = 0;
};

namespace detail
{



class account_history_plugin_impl
{
   public:
      account_history_plugin_impl(account_history_plugin& _plugin)
         : _self( _plugin )
      { }
      virtual ~account_history_plugin_impl();


      /** this method is called as a callback after a block is applied
       * and will process/index all operations that were applied in the block.
       */
      void update_account_histories( const signed_block& b );
      void update_account_balances( const signed_block& b );

      graphene::chain::database& database()
      {
         return _self.database();
      }

      account_history_plugin& _self;
      flat_set<account_id_type> _tracked_accounts;
      flat_set<uint32_t> _tracked_operations;      
      bool _partial_operations = false;
      bool _save_balancess = false;
      primary_index< operation_history_index >* _oho_index;
      uint32_t _max_ops_per_account = 0;
      uint32_t _max_balance_per_account = 0;
      fc::path  _data_dir_balances;

      void init_data_dir();
    private:
      /** add one history record, then check and remove the earliest history record */
      void add_account_history( const account_id_type account_id, const operation_history_id_type op_id , int64_t op_type);
      void save_balance(balance_history_object &obj ,int ierase);
     
};

account_history_plugin_impl::~account_history_plugin_impl()
{
   return;
}

void account_history_plugin_impl::init_data_dir( )
{
  if(_data_dir_balances.string().length()<=0)
  {
    graphene::chain::database& db = database();  
    _data_dir_balances = db.get_data_dir()   / "balance_histroy";   
    if (!fc::exists(_data_dir_balances))
      fc::create_directories(_data_dir_balances);   
  }
} 
void  account_history_plugin_impl::save_balance(balance_history_object& obj, int ierase)
{
  init_data_dir();

  balance_index _index;
  std::fstream _blocks;
  std::fstream _indexs;
  fc::path file_block = _data_dir_balances / obj.issuer.to_string();
  fc::path file_index = _data_dir_balances / (obj.issuer.to_string() + ".index");
  
  bool bopen = false; 
  for (auto &a : obj.history)
  {
      if(a.index> obj.save_index)
      {
          if(!bopen)
          {
            _blocks.open(file_block.generic_string().c_str(), std::fstream::binary | std::fstream::out |   std::fstream::app);
            _indexs.open(file_index.generic_string().c_str(), std::fstream::binary | std::fstream::out|   std::fstream::app);
            bopen = true;
            _blocks.seekp(0,_blocks.end);
          }
          _index.index = a.index;
          _index.pos   = _blocks.tellp();
          auto vec = fc::raw::pack(a);
          _blocks.write( vec.data(), vec.size() );
          _index.size =  vec.size();
          _indexs.seekp(_index.index * sizeof(balance_index));
          _indexs.write((char *)&_index, sizeof(balance_index) );
          obj.save_index = a.index;
      }
  }
  if(bopen)
  {
    //_indexs.flush();
    //_blocks.flush(); 
    _indexs.close();
    _blocks.close(); 
  }
}

void  account_history_plugin_impl::update_account_balances( const signed_block& b )
{
    try{
    graphene::chain::database &db = database();
    if(_max_balance_per_account<=0 || !db._app_status._apply_block )
       return;   
    uint64_t num_block = db.head_block_num();
    fc::time_point_sec time_block = db.head_block_time();
    const vector<balance_history_> &hist = db.get_applied_balances();
    for( const auto&o_op : hist )
    {
        const auto& his_idx = db.get_index_type<balance_history_index>();
        const auto& by_seq_idx = his_idx.indices().get<by_account_id>();
        auto itr = by_seq_idx.find( o_op.issuer);
        if(itr == by_seq_idx.end())
        {
            db.create<balance_history_object>([&](balance_history_object &h) {
                h.issuer = o_op.issuer;
            });
            itr = by_seq_idx.find(o_op.issuer);
        }
        if(itr == by_seq_idx.end())
            continue;
       db.modify( *itr, [&]( balance_history_object& obj ){   
            balance_history add;
            add.index = ++obj.max_index;
            add.utype = o_op.utype;
            add.info  = o_op.info;
            add.asset_op = o_op.asset_op;  
            add.balance = o_op.balance;  
            add.block_time = time_block;
            add.block_num = num_block;
            obj.history.push_back(add);
            if(obj.history.size()>_max_balance_per_account *3/2)
            {
                int ierase = obj.history.size() - _max_balance_per_account;          
                if(_save_balancess)
                    save_balance(obj, ierase);
                obj.history.erase(obj.history.begin(), obj.history.begin() + ierase);
            }
       });    
    }
   }catch(const fc::exception& e) 
  {
    elog(e.to_string());
  } 
}
void account_history_plugin_impl::update_account_histories( const signed_block& b1 )
{
   graphene::chain::database& db = database();
   const vector<optional< operation_history_object > >& hist = db.get_applied_operations();
   for( const optional< operation_history_object >& o_op : hist )
   {
      if( !o_op.valid() )
         continue;

      optional<operation_history_object> oho;

      auto create_oho = [&]() {
         return optional<operation_history_object>( db.create<operation_history_object>( [&]( operation_history_object& h )
         {
            if( o_op.valid() )
            {
               h.op           = o_op->op;
               h.result       = o_op->result;
               h.block_num    = o_op->block_num;
               h.trx_in_block = o_op->trx_in_block;
               h.op_in_trx    = o_op->op_in_trx;
               h.trxid        = o_op->trxid;
               h.block_time   = b1.timestamp;
               h.virtual_op   = o_op->virtual_op;
            }
         } ) );
      };      
       
      int64_t value = o_op->op.which();
      if(_tracked_operations.size() > 0) {
         if( _tracked_operations.find( value ) == _tracked_operations.end())
            continue;
      }

      if (value == operation::tag<fill_object_history_operation>::value || value == operation::tag<bitlender_test_operation>::value ||
          value == operation::tag<bitlender_publish_feed_operation>::value || value == operation::tag<bitlender_publish_feed_operation>::value ||
          value == operation::tag<issue_fundraise_publish_feed_operation>::value)
         continue;

      if( !o_op.valid() || ( _max_ops_per_account == 0 && _partial_operations ) )
      {
         // Note: the 2nd and 3rd checks above are for better performance, when the db is not clean,
         //       they will break consistency of account_stats.total_ops and removed_ops and most_recent_op
         _oho_index->use_next_id();
         continue;
      }
      else if( !_partial_operations )
         // add to the operation history index
         oho = create_oho();

      const operation_history_object& op = *o_op;

      // get the set of accounts this operation applies to
      flat_set<account_id_type> impacted;
      vector<authority> other;
      operation_get_required_authorities( op.op, impacted, impacted, other ); // fee_payer is added here

      if( op.op.which() == operation::tag< account_create_operation >::value )
         impacted.insert( op.result.get<object_id_type>() );
      else
         graphene::app::operation_get_impacted_accounts( op.op, impacted );

      for( auto& a : other )
         for( auto& item : a.account_auths )
            impacted.insert( item.first );

      // be here, either _max_ops_per_account > 0, or _partial_operations == false, or both
      // if _partial_operations == false, oho should have been created above
      // so the only case should be checked here is:
      //    whether need to create oho if _max_ops_per_account > 0 and _partial_operations == true

      // for each operation this account applies to that is in the config link it into the history
      if( _tracked_accounts.size() == 0 ) // tracking all accounts
      {
         // if tracking all accounts, when impacted is not empty (although it will always be),
         //    still need to create oho if _max_ops_per_account > 0 and _partial_operations == true
         //    so always need to create oho if not done
         if (!impacted.empty() && !oho.valid()) { oho = create_oho(); }

         if( _max_ops_per_account > 0 )
         {
            // Note: the check above is for better performance, when the db is not clean,
            //       it breaks consistency of account_stats.total_ops and removed_ops and most_recent_op,
            //       but it ensures it's safe to remove old entries in add_account_history(...)
            for( auto& account_id : impacted )
            {
               // we don't do index_account_keys here anymore, because
               // that indexing now happens in observers' post_evaluate()

               // add history
               add_account_history( account_id, oho->id, value );
            }
         }
      }
      else // tracking a subset of accounts
      {
         // whether need to create oho if _max_ops_per_account > 0 and _partial_operations == true ?
         // the answer: only need to create oho if a tracked account is impacted and need to save history

         if( _max_ops_per_account > 0 )
         {
            // Note: the check above is for better performance, when the db is not clean,
            //       it breaks consistency of account_stats.total_ops and removed_ops and most_recent_op,
            //       but it ensures it's safe to remove old entries in add_account_history(...)
            for( auto account_id : _tracked_accounts )
            {
               if( impacted.find( account_id ) != impacted.end() )
               {
                  if (!oho.valid()) { oho = create_oho(); }
                  // add history
                  add_account_history( account_id, oho->id,value );
               }
            }
         }
      }
      if (_partial_operations && ! oho.valid())
         _oho_index->use_next_id();
   }
}

void account_history_plugin_impl::add_account_history( const account_id_type account_id, const operation_history_id_type op_id, int64_t op_type)
{
   graphene::chain::database& db = database();
   const auto& stats_obj = account_id(db).statistics(db);
   // add new entry
   const auto& ath = db.create<account_transaction_history_object>( [&]( account_transaction_history_object& obj ){
       obj.operation_id = op_id;
       obj.operation_type = op_type;
       obj.account = account_id;
       obj.sequence = stats_obj.total_ops + 1;
       obj.next = stats_obj.most_recent_op;
   });
   db.modify( stats_obj, [&]( account_statistics_object& obj ){
       obj.most_recent_op = ath.id;
       obj.total_ops = ath.sequence;
   });
   // remove the earliest account history entry if too many
   // _max_ops_per_account is guaranteed to be non-zero outside
   if( stats_obj.total_ops - stats_obj.removed_ops > _max_ops_per_account )
   {
      // look for the earliest entry
      const auto& his_idx = db.get_index_type<account_transaction_history_index>();
      const auto& by_seq_idx = his_idx.indices().get<by_seq>();
      auto itr = by_seq_idx.lower_bound( boost::make_tuple( account_id, 0 ) );
      // make sure don't remove the one just added
      if( itr != by_seq_idx.end() && itr->account == account_id && itr->id != ath.id )
      {
         // if found, remove the entry, and adjust account stats object
         const auto remove_op_id = itr->operation_id;
         const auto itr_remove = itr;
         ++itr;
         db.remove( *itr_remove );
         db.modify( stats_obj, [&]( account_statistics_object& obj ){
             obj.removed_ops = obj.removed_ops + 1;
         });
         // modify previous node's next pointer
         // this should be always true, but just have a check here
         if( itr != by_seq_idx.end() && itr->account == account_id )
         {
            db.modify( *itr, [&]( account_transaction_history_object& obj ){
               obj.next = account_transaction_history_id_type();
            });
         }
         // else need to modify the head pointer, but it shouldn't be true

         // remove the operation history entry (1.11.x) if configured and no reference left
         if( _partial_operations )
         {
            // check for references
            const auto& by_opid_idx = his_idx.indices().get<by_opid>();
            if( by_opid_idx.find( remove_op_id ) == by_opid_idx.end() )
            {
               // if no reference, remove
               db.remove( remove_op_id(db) );
            }
         }
      }
   }
}

} // end namespace detail






account_history_plugin::account_history_plugin() :
   my( new detail::account_history_plugin_impl(*this) )
{
}

account_history_plugin::~account_history_plugin()
{
}

std::string account_history_plugin::plugin_name()const
{
   return "account_history";
}

void account_history_plugin::plugin_set_program_options(
   boost::program_options::options_description& cli,
   boost::program_options::options_description& cfg
   )
{
   cli.add_options()
         ("track-account", boost::program_options::value<std::vector<std::string>>()->composing()->multitoken(), "Account ID to track history for (may specify multiple times)")
         ("track-operations",boost::program_options::value<string>()->default_value("[]"), "Operations to track history for (may specify multiple times)")
         ("partial-operations", boost::program_options::value<bool>(), "Keep only those operations in memory that are related to account history tracking")
         ("save-balances", boost::program_options::value<bool>(), "save balances operations")
         ("max-ops-per-account", boost::program_options::value<uint32_t>(), "Maximum number of operations per account will be kept in memory")
         ("max-balance-per-account", boost::program_options::value<uint32_t>(), "Maximum number of balances per account will be kept in memory")
         ;
   cfg.add(cli);
}
void account_history_plugin::get_info(fc::mutable_variant_object &result) 
{     
    result["partial-operations"] = my->_partial_operations;
    result["save-balances"] = my->_save_balancess;
    result["max-ops-per-account"] = my->_max_ops_per_account;
    result["max-balance-per-account"] = my->_max_balance_per_account;
}
void account_history_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{
   database().applied_block.connect( [&]( const signed_block& b){ my->update_account_histories(b); } );
   database().applied_block.connect( [&]( const signed_block& b){ my->update_account_balances(b); } );

   my->_oho_index = database().add_index< primary_index< operation_history_index > >();
   database().add_index< primary_index< account_transaction_history_index > >();
   database().add_index< primary_index< balance_history_index > >();

   LOAD_VALUE_SET(options, "track-account", my->_tracked_accounts, graphene::chain::account_id_type);
   if( options.count( "track-operations" ) )
   {
      const std::string& groups = options["track-operations"].as<string>();
      if(groups.size() >2) my->_tracked_operations = fc::json::from_string(groups).as<flat_set<uint32_t>>( 2 );                  
   }
   if (options.count("partial-operations")) {
       my->_partial_operations = options["partial-operations"].as<bool>();
   }
   if (options.count("save-balances")) {
       my->_save_balancess = options["save-balances"].as<bool>();
   }
   if (options.count("max-ops-per-account")) {
       my->_max_ops_per_account = options["max-ops-per-account"].as<uint32_t>();
   }
   if (options.count("max-balance-per-account")) {
       my->_max_balance_per_account = options["max-balance-per-account"].as<uint32_t>();
   }
   if(my->_max_balance_per_account > 1000)
        my->_max_balance_per_account = 1000;
//注意，必须放这个位置，否则 replay收不到数据
   database()._app_status.has_account_history_plugin = true;    
}

void account_history_plugin::plugin_startup()
{

}

flat_set<account_id_type> account_history_plugin::tracked_accounts() const
{
   return my->_tracked_accounts;
}
vector<balance_history> account_history_plugin::get_balance_history_object(account_id_type account_id, flat_set<asset_id_type> asset_id,uint32_t type,uint64_t ustart, uint64_t nlimit)
{
    my->init_data_dir();

    vector<balance_history> result;
    fc::path file_block = my->_data_dir_balances / account_id.to_string();
    fc::path file_index = my->_data_dir_balances / (account_id.to_string() + ".index");
    if (!fc::exists(file_block) || !fc::exists(file_index))
        return result;

    try
    {
        balance_index _index;
        std::fstream _blocks;
        std::fstream _indexs;        
        _blocks.open(file_block.generic_string().c_str(), std::fstream::binary | std::fstream::in);        
        _indexs.open(file_index.generic_string().c_str(), std::fstream::binary | std::fstream::in);   

        int64_t size = sizeof(balance_index),start = ustart;        

        _indexs.seekg(0,_indexs.end);
        int64_t pos = _indexs.tellg();
        if (pos >= size)
        {
            pos = pos / size * size-size;
            if(start * size < pos)
              pos = start * size;

            while (result.size() < nlimit && pos >= 0)
            {
                _index.size = 0;
                _indexs.seekg(pos);
                _indexs.read((char *)&_index, size);
                if(_index.size ==0)
                {
                   pos -= size;
                   continue;
                }
                _blocks.seekg(_index.pos);
                vector<char> data(_index.size);
                _blocks.read(data.data(), _index.size);
                auto vec = fc::raw::unpack<balance_history>(data);
                if (vec.index <= ustart)
                {
                    if (asset_id.size() <= 0 || asset_id.find(vec.asset_op.asset_id) != asset_id.end())
                    {
                        if(type >=max_balance_type || vec.utype == type)
                            result.push_back(vec);
                    }
                }
                pos -= size;
            }
        }
        _blocks.close();
        _indexs.close();
    }
    catch(...)
    {
    }
    
    return result;
} 

vector<operation_history_object> account_history_plugin::get_account_history( account_id_type account,
                                                                       operation_history_id_type stop,
                                                                       unsigned limit,
                                                                       operation_history_id_type start ) const
{
       const graphene::chain::database& db = *app().chain_database();
       FC_ASSERT( limit <= 100 );
       vector<operation_history_object> result;
       try {
          const account_transaction_history_object& node = account(db).statistics(db).most_recent_op(db);
          if(start == operation_history_id_type() || start.instance.value > node.operation_id.instance.value)
             start = node.operation_id;
       } catch(...) { return result; }

       const auto& hist_idx = db.get_index_type<account_transaction_history_index>();
       const auto& by_op_idx = hist_idx.indices().get<by_op>();
       auto index_start = by_op_idx.begin();
       auto itr = by_op_idx.lower_bound(boost::make_tuple(account, start));

       while(itr != index_start && itr->account == account && itr->operation_id.instance.value > stop.instance.value && result.size() < limit)
       {
          if(itr->operation_id.instance.value <= start.instance.value)
          {
             try {
             result.push_back(itr->operation_id(db));
             } catch(...) { }
          }
          --itr;
       }
       if(stop.instance.value == 0 && result.size() < limit && itr->account == account) {
         try {
         result.push_back(itr->operation_id(db));
          } catch(...) { }
       }

       return result;
}
vector<operation_history_object> account_history_plugin::get_account_history_operations( account_id_type account,
                                                                       int operation_id,
                                                                       operation_history_id_type start,
                                                                       operation_history_id_type stop,
                                                                       unsigned limit)  const
{       
       const  graphene::chain::database& db = *app().chain_database();
       FC_ASSERT( limit <= 100 );
       vector<operation_history_object> result;

       const auto& stats = account(db).statistics(db);
       if( stats.most_recent_op == account_transaction_history_id_type() ) return result;
       const account_transaction_history_object* node = &stats.most_recent_op(db);
       if( start == operation_history_id_type() )
          start = node->operation_id;

       while(node && node->operation_id.instance.value > stop.instance.value && result.size() < limit)
       {
          if( node->operation_id.instance.value <= start.instance.value ) {

             if(node->operation_id(db).op.which() == operation_id)
               result.push_back( node->operation_id(db) );
             }
          if( node->next == account_transaction_history_id_type() )
             node = nullptr;
          else node = &node->next(db);
       }
       if( stop.instance.value == 0 && result.size() < limit ) {
          const account_transaction_history_object head = account_transaction_history_id_type()(db);
          if( head.account == account && head.operation_id(db).op.which() == operation_id )
             result.push_back(head.operation_id(db));
       }
       return result;
}

} }
