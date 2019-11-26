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

#include <graphene/finance_history/finance_history_plugin.hpp>


#include <graphene/chain/config.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/business/finance_object.hpp> 


#include <fc/thread/thread.hpp>
#include <fc/smart_ref_impl.hpp>
#include <boost/algorithm/string.hpp>

namespace graphene { namespace finance_history {
 
fc::time_point_sec get_string_time(string stext)
{
  vector< string >  SplitVec; 
  boost::split(SplitVec, stext, boost::is_any_of("-"), boost::token_compress_on );  
  if(SplitVec.size()<=0)
    return fc::time_point_sec::min();
  else
   return fc::time_point_sec(fc::to_int64(SplitVec[0]));  
}
bool compfile(const string &a, const string &b){
     return a > b;
}

namespace detail
{
class finance_history_plugin_impl
{
   public:
      finance_history_plugin_impl(finance_history_plugin& _plugin)
      :_self( _plugin ) {}
      virtual ~finance_history_plugin_impl();

      /** this method is called as a callback after a block is applied
       * and will process/index all operations that were applied in the block.
       */
      void update_finance_histories( const signed_block& b );
      void update_exchange_histories(const signed_block &b);

      graphene::chain::database &database()
      {
         return _self.database();
      }

      void     init_data_dir( );
     
 
      finance_history_plugin&       _self;  
      fc::path                      _data_dir_loan;
      flat_set<uint32_t>            _tracked_buckets;
      uint32_t                      _maximum_history_per_bucket_size = 1000;
      uint32_t                      _max_order_his_records_per_market = 1000;
      uint32_t                      _max_order_his_seconds_per_market = 259200;
    
    
};
void finance_history_plugin_impl::init_data_dir( )
{
  if(_data_dir_loan.string().length()<=0)
  {
   graphene::chain::database& db = database();  
   _data_dir_loan = db.get_data_dir()   / "finance_database";   
   if (!fc::exists(_data_dir_loan))
     fc::create_directories(_data_dir_loan);
   
  }
} 
  
 
struct operation_process_fill_order
{
   finance_history_plugin&            _plugin;
   fc::time_point_sec                _now;
   const finance_ticker_meta_object*& _meta;

   operation_process_fill_order( finance_history_plugin& mhp, fc::time_point_sec n, const finance_ticker_meta_object*& meta )
   :_plugin(mhp),_now(n),_meta(meta) {}

   typedef void result_type;

   /** do nothing for other operation types */
   template<typename T>
   void operator()( const T& )const{}

   void operator()( const fill_order_operation& o )const 
   {
      //ilog( "processing ${o}", ("o",o) );
      if(o.op_type != 2)
        return ;

      auto &db = _plugin.database();
      const auto& order_his_idx = db.get_index_type<finance_history_index>().indices();
      const auto& history_idx = order_his_idx.get<by_finance_key>();
      const auto& his_time_idx = order_his_idx.get<by_finance_time>();

      // To save new filled order data
      finance_history_key hkey;
      hkey.base = o.pays.asset_id;
      hkey.quote = o.receives.asset_id;
      if( hkey.base > hkey.quote ) 
         std::swap( hkey.base, hkey.quote );
      hkey.sequence = std::numeric_limits<int64_t>::min();

      auto itr = history_idx.lower_bound( hkey );

      if( itr != history_idx.end() && itr->key.base == hkey.base && itr->key.quote == hkey.quote )
         hkey.sequence = itr->key.sequence - 1;
      else
         hkey.sequence = 0;

      const auto& new_order_his_obj = db.create<finance_order_history_object>( [&]( finance_order_history_object& ho ) {
         ho.key = hkey;
         ho.time = _now;
         ho.op = o;
      });

      // save a reference to market ticker meta object
      if( _meta == nullptr )
      {
         const auto& meta_idx = db.get_index_type<simple_index<finance_ticker_meta_object>>();
         if( meta_idx.size() == 0 )
            _meta = &db.create<finance_ticker_meta_object>( [&]( finance_ticker_meta_object& mtm ) {
               mtm.rolling_min_order_his_id = new_order_his_obj.id;
               mtm.skip_min_order_his_id = false;
            });
         else
            _meta = &( *meta_idx.begin() );
      }

      // To remove old filled order data
      const auto max_records = _plugin.max_order_his_records_per_market();
      hkey.sequence += max_records;
      itr = history_idx.lower_bound( hkey );
      if( itr != history_idx.end() && itr->key.base == hkey.base && itr->key.quote == hkey.quote )
      {
         const auto max_seconds = _plugin.max_order_his_seconds_per_market();
         fc::time_point_sec min_time;
         if( min_time + max_seconds < _now )
            min_time = _now - max_seconds;
         auto time_itr = his_time_idx.lower_bound( std::make_tuple( hkey.base, hkey.quote, min_time ) );
         if( time_itr != his_time_idx.end() && time_itr->key.base == hkey.base && time_itr->key.quote == hkey.quote )
         {
            if( itr->key.sequence >= time_itr->key.sequence ) //根据索引删除
            {
               while( itr != history_idx.end() && itr->key.base == hkey.base && itr->key.quote == hkey.quote )
               {
                  auto old_itr = itr;
                  ++itr;
                  db.remove( *old_itr );
               }
            }
            else //根据时间删除
            {
               while( time_itr != his_time_idx.end() && time_itr->key.base == hkey.base && time_itr->key.quote == hkey.quote )
               {
                  auto old_itr = time_itr;
                  ++time_itr;
                  db.remove( *old_itr );
               }
            }
         }
      }

      // To update ticker data and buckets data, only update for maker orders
      if( !o.is_maker )  //如果不是买卖行为，不计入下面的流程
         return;

      finance_bucket_key key;
      key.base    = o.pays.asset_id;
      key.quote   = o.receives.asset_id;

      price trade_price = o.pays / o.receives;

      if( key.base > key.quote )
      {
         std::swap( key.base, key.quote );
         trade_price = ~trade_price;
      }

      price fill_price = o.fill_price;
      if( fill_price.base.asset_id > fill_price.quote.asset_id )
         fill_price = ~fill_price;

      // To update ticker data
      const auto& ticker_idx = db.get_index_type<finance_ticker_index>().indices().get<by_finance>();
      auto ticker_itr = ticker_idx.find( std::make_tuple( key.base, key.quote ) );
      if( ticker_itr == ticker_idx.end() )
      {
         db.create<finance_ticker_object>( [&]( finance_ticker_object& mt ) {
            mt.base           = key.base;
            mt.quote          = key.quote;
            mt.last_day_base  = 0;
            mt.last_day_quote = 0;
            mt.latest_base    = fill_price.base.amount; //交易挂单价格
            mt.latest_quote   = fill_price.quote.amount;
            mt.base_volume    = trade_price.base.amount.value; //实际数量
            mt.quote_volume   = trade_price.quote.amount.value;
         });
      }
      else
      {
         db.modify( *ticker_itr, [&]( finance_ticker_object& mt ) {
            mt.latest_base    = fill_price.base.amount; //交易挂单价格
            mt.latest_quote   = fill_price.quote.amount;
            mt.base_volume    += trade_price.base.amount.value;  // ignore overflow
            mt.quote_volume   += trade_price.quote.amount.value; // ignore overflow
         });
      }

      // To update buckets data
      const auto max_history = _plugin.max_history();
      if( max_history == 0 ) return;

      const auto& buckets = _plugin.tracked_buckets();
      if( buckets.size() == 0 ) return;

      const auto& bucket_idx = db.get_index_type<finance_bucket_index>();
      for( auto bucket : buckets )
      {
          auto bucket_num = _now.sec_since_epoch() / bucket;
          fc::time_point_sec cutoff;
          if( bucket_num > max_history )
             cutoff = cutoff + ( bucket * ( bucket_num - max_history ) );

          key.seconds = bucket;
          key.open    = fc::time_point_sec() + ( bucket_num * bucket );

          const auto& by_key_idx = bucket_idx.indices().get<by_finance_key>();
          auto bucket_itr = by_key_idx.find( key );
          if( bucket_itr == by_key_idx.end() )
          { // create new bucket
            /* const auto& obj = */
            db.create<finance_bucket_object>( [&]( finance_bucket_object& b ){
                 b.key = key;
                 b.base_volume = trade_price.base.amount;
                 b.quote_volume = trade_price.quote.amount;
                 b.open_base = fill_price.base.amount;
                 b.open_quote = fill_price.quote.amount;
                 b.close_base = fill_price.base.amount;
                 b.close_quote = fill_price.quote.amount;
                 b.high_base = b.close_base;
                 b.high_quote = b.close_quote;
                 b.low_base = b.close_base;
                 b.low_quote = b.close_quote;
            });
            //wlog( "    creating bucket ${b}", ("b",obj) );
          }
          else
          { // update existing bucket
             //wlog( "    before updating bucket ${b}", ("b",*bucket_itr) );
             db.modify( *bucket_itr, [&]( finance_bucket_object& b ){
                  try {
                     b.base_volume += trade_price.base.amount;
                  } catch( fc::overflow_exception ) {
                     b.base_volume = std::numeric_limits<int64_t>::max();
                  }
                  try {
                     b.quote_volume += trade_price.quote.amount;
                  } catch( fc::overflow_exception ) {
                     b.quote_volume = std::numeric_limits<int64_t>::max();
                  }
                  b.close_base = fill_price.base.amount;
                  b.close_quote = fill_price.quote.amount;
                  if( b.high() < fill_price )
                  {
                      b.high_base = b.close_base;
                      b.high_quote = b.close_quote;
                  }
                  if( b.low() > fill_price )
                  {
                      b.low_base = b.close_base;
                      b.low_quote = b.close_quote;
                  }
             });
             //wlog( "    after bucket bucket ${b}", ("b",*bucket_itr) );
          }

          {
             key.open = fc::time_point_sec();
             bucket_itr = by_key_idx.lower_bound( key );

             while( bucket_itr != by_key_idx.end() &&
                    bucket_itr->key.base == key.base &&
                    bucket_itr->key.quote == key.quote &&
                    bucket_itr->key.seconds == bucket &&
                    bucket_itr->key.open < cutoff )
             {
              //  elog( "    removing old bucket ${b}", ("b", *bucket_itr) );
                auto old_bucket_itr = bucket_itr;
                ++bucket_itr;
                db.remove( *old_bucket_itr );
             }
          }
      }
   }
};


 finance_history_plugin_impl::~finance_history_plugin_impl()
 {}

void finance_history_plugin_impl::update_exchange_histories( const signed_block& b )
{
   graphene::chain::database& db = database();
   const finance_ticker_meta_object* _meta = nullptr;
   const auto& meta_idx = db.get_index_type<simple_index<finance_ticker_meta_object>>();
   if( meta_idx.size() > 0 )
      _meta = &( *meta_idx.begin() );
   const vector<optional< operation_history_object > >& hist = db.get_applied_operations();
   for( const optional< operation_history_object >& o_op : hist )
   {
      if( o_op.valid() )
      {
         try
         {
            o_op->op.visit( operation_process_fill_order( _self, b.timestamp, _meta ) );
         } FC_CAPTURE_AND_LOG( (o_op) )
      }
   }
   // roll out expired data from ticker
   if( _meta != nullptr )
   {
      time_point_sec last_day = b.timestamp - 86400;
      object_id_type last_min_his_id = _meta->rolling_min_order_his_id;
      bool skip = _meta->skip_min_order_his_id;

      const auto& ticker_idx = db.get_index_type<finance_ticker_index>().indices().get<by_finance>();
      const auto& history_idx = db.get_index_type<finance_history_index>().indices().get<by_id>();
      auto history_itr = history_idx.lower_bound( _meta->rolling_min_order_his_id );
      while( history_itr != history_idx.end() && history_itr->time < last_day )
      {
         const fill_order_operation& o = history_itr->op;
         if( skip && history_itr->id == _meta->rolling_min_order_his_id )
            skip = false;
         else if( o.is_maker )
         {
            finance_bucket_key key;
            key.base    = o.pays.asset_id;
            key.quote   = o.receives.asset_id;

            price trade_price = o.pays / o.receives;

            if( key.base > key.quote )
            {
               std::swap( key.base, key.quote );
               trade_price = ~trade_price;
            }

            price fill_price = o.fill_price;
            if( fill_price.base.asset_id > fill_price.quote.asset_id )
               fill_price = ~fill_price;

            auto ticker_itr = ticker_idx.find( std::make_tuple( key.base, key.quote ) );
            if( ticker_itr != ticker_idx.end() ) // should always be true
            {
               db.modify( *ticker_itr, [&]( finance_ticker_object& mt ) {
                  mt.last_day_base  = fill_price.base.amount;
                  mt.last_day_quote = fill_price.quote.amount;
                  mt.base_volume    -= trade_price.base.amount.value;  // ignore underflow
                  mt.quote_volume   -= trade_price.quote.amount.value; // ignore underflow
               });
            }
         }
         last_min_his_id = history_itr->id;
         ++history_itr;
      }
      // update meta
      if( history_itr != history_idx.end() ) // if still has some data rolling
      {
         if( history_itr->id != _meta->rolling_min_order_his_id ) // if rolled out some
         {
            db.modify( *_meta, [&]( finance_ticker_meta_object& mtm ) {
               mtm.rolling_min_order_his_id = history_itr->id;
               mtm.skip_min_order_his_id = false;
            });
         }
      }
      else // if all data are rolled out
      {
         if( last_min_his_id != _meta->rolling_min_order_his_id ) // if rolled out some
         {
            db.modify( *_meta, [&]( finance_ticker_meta_object& mtm ) {
               mtm.rolling_min_order_his_id = last_min_his_id;
               mtm.skip_min_order_his_id = true;
            });
         }
      }
   }
}
void finance_history_plugin_impl::update_finance_histories( const signed_block& b )
{  
   vector<fill_object_history_operation> *paloan = new vector<fill_object_history_operation>;
   try{
    init_data_dir();
    graphene::chain::database& db = database(); 
    const vector<optional<operation_history_object>> &hist = db.get_applied_operations(); 
    string stime = fc::to_string(b.timestamp.sec_since_epoch());
    for (const optional<operation_history_object> &o_op : hist)
    {      
      switch (o_op->op.which())
      {      
      case operation::tag<fill_object_history_operation>::value:
      {
        auto &o = o_op->op.get<fill_object_history_operation>();        
        if (o.itype == fill_object_issue_fundraise || o.itype == fill_object_buy_fundraise || o.itype == fill_object_sell_exchange ||  o.itype == fill_object_buy_exchange)
          paloan->push_back(o);         
        continue;
      }
      default:
        continue;
      }        
   }   
   if(paloan->size()>0)
   {     
      fc::async( [paloan,this,stime]() {   
        int  ifileindex = 1;      
        auto itr_order = paloan->begin();        
        while (itr_order !=  paloan->end())
        {
          fill_object_history_operation &index = *itr_order;
          try{          
          string account_id = index.issuer.to_string();  
          fc::path  filew = this->_data_dir_loan / account_id;
          if (!fc::exists(filew))
             fc::create_directories(filew);

          if(index.id.type()  == buy_fundraise_object_type) 
             filew /= "buyfundraise";
          else if(index.id.type()  == issue_fundraise_object_type) 
             filew /= "issuefundraise";           
          else if(index.id.type()  == sell_exchange_object_type) 
             filew /= "sellexchange";
          else if(index.id.type()  == buy_exchange_object_type) 
             filew /= "buyexchange";       
          else 
             filew /= fc::to_string(index.id.type());   

          if (!fc::exists(filew))
            fc::create_directories(filew);   

          fc::path sfile  =  filew /  (stime + "-" +  fc::to_string(ifileindex) + "-" + index.id.to_string() );                 

          std::fstream _blocks;
          _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::out | std::fstream::trunc);          
          _blocks.write( index.data.data(), index.data.size() );
          _blocks.close();          
                   
          if (index.itype == fill_object_buy_fundraise || index.itype == fill_object_buy_exchange)
          {
             string account_id = index.src_id.to_string();  
             fc::path  filew = this->_data_dir_loan / account_id;
             if (!fc::exists(filew))
                  fc::create_directories(filew);

             fc::path sfile  =  filew /  index.id.to_string();                 

             std::fstream _blocks;
             _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::out | std::fstream::trunc);          
             _blocks.write( index.data.data(), index.data.size() );
             _blocks.close();   
          }

          itr_order++;
          ifileindex++;
          } FC_CAPTURE_AND_RETHROW( (index.id)(index.issuer)) 
        }     
        delete paloan;   
      } );
    }
   else 
     delete paloan;
   } catch ( ... )         
   {    
     delete paloan;
   }      
}


} // end namespace detail






finance_history_plugin::finance_history_plugin() :
   my( new detail::finance_history_plugin_impl(*this) )
{
}

finance_history_plugin::~finance_history_plugin()
{
}

std::string finance_history_plugin::plugin_name()const
{
   return "finance_history";
}

void finance_history_plugin::plugin_set_program_options(
   boost::program_options::options_description& cli,
   boost::program_options::options_description& cfg
   )
{
 cli.add_options()
         ("bucket-size", boost::program_options::value<string>()->default_value("[60,300,900,1800,3600,14400,86400]"),
           "Track market history by grouping orders into buckets of equal size measured in seconds specified as a JSON array of numbers")
         ("history-per-size", boost::program_options::value<uint32_t>()->default_value(1000),
           "How far back in time to track history for each bucket size, measured in the number of buckets (default: 1000)")
         ("max-order-his-records-per-market", boost::program_options::value<uint32_t>()->default_value(1000),
           "Will only store this amount of matched orders for each market in order history for querying, or those meet the other option, which has more data (default: 1000)")
         ("max-order-his-seconds-per-market", boost::program_options::value<uint32_t>()->default_value(259200),
           "Will only store matched orders in last X seconds for each market in order history for querying, or those meet the other option, which has more data (default: 259200 (3 days))")
         ;
   cfg.add(cli);
}
void finance_history_plugin::get_info(fc::mutable_variant_object &result) 
{
  result["bucket-size"] = my->_tracked_buckets.size();  
  result["history-per-size"] = my->_maximum_history_per_bucket_size;   
  result["max-order-his-records-per-market"] = my->_max_order_his_records_per_market;  
  result["max-order-his-records-per-market"] = my->_max_order_his_seconds_per_market;   
}
void finance_history_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{ try {  
    
    database().applied_block.connect([this](const signed_block &b) { my->update_finance_histories(b); });
    database().applied_block.connect([this](const signed_block &b) { my->update_exchange_histories(b); });
    
    database().add_index< primary_index< finance_bucket_index  > >();
    database().add_index< primary_index< finance_history_index  > >();
    database().add_index< primary_index< finance_ticker_index  > >();
    database().add_index< primary_index< simple_index< finance_ticker_meta_object > > >();

    if( options.count( "bucket-size" ) )
    {
      const std::string& buckets = options["bucket-size"].as<string>();
      my->_tracked_buckets = fc::json::from_string(buckets).as<flat_set<uint32_t>>(2);
      my->_tracked_buckets.erase( 0 );
    }
    if( options.count( "history-per-size" ) )
      my->_maximum_history_per_bucket_size = options["history-per-size"].as<uint32_t>();
    if( options.count( "max-order-his-records-per-market" ) )
      my->_max_order_his_records_per_market = options["max-order-his-records-per-market"].as<uint32_t>();
    if( options.count( "max-order-his-seconds-per-market" ) )
      my->_max_order_his_seconds_per_market = options["max-order-his-seconds-per-market"].as<uint32_t>();

   database()._app_status.has_finance_history_plugin = true;    
  } 

  FC_CAPTURE_AND_RETHROW()
}
void finance_history_plugin::plugin_startup()
{
} 
const flat_set<uint32_t>& finance_history_plugin::tracked_buckets() const
{
   return my->_tracked_buckets;
}

uint32_t finance_history_plugin::max_history()const
{
   return my->_maximum_history_per_bucket_size;
}

uint32_t finance_history_plugin::max_order_his_records_per_market()const
{
   return my->_max_order_his_records_per_market;
}

uint32_t finance_history_plugin::max_order_his_seconds_per_market()const
{
   return my->_max_order_his_seconds_per_market;
}

vector < issue_fundraise_object >    finance_history_plugin::get_account_issue_fundraise_history(account_id_type account, fc::time_point_sec start, fc::time_point_sec end)const
{
  string  account_id  = account.to_string();  
  fc::path sfile = my->_data_dir_loan;
  sfile /= account_id;
  sfile /= "issuefundraise";
  vector<issue_fundraise_object> result;
  if (!fc::exists(sfile))
    return result;
 
  fc::directory_iterator itr(sfile);
  vector<string> findfile;
  for( ; itr != fc::directory_iterator(); itr++ )
  {    
    string current_filename = itr->filename().string();   
    fc::time_point_sec timefile =  get_string_time(current_filename);
    if(timefile<start || timefile>end)
       continue;
    findfile.push_back(current_filename);                   
  } 
  if(findfile.size()>0)
  {
    sort(findfile.begin(),findfile.end(),compfile);
    auto itr_order = findfile.begin();    
     while (itr_order !=  findfile.end())
     {
         std::fstream _blocks;         
         _blocks.open( (sfile/(*itr_order)).generic_string().c_str(), std::fstream::binary | std::fstream::in );
         if(_blocks.is_open())
         {
           _blocks.seekg (0, std::ios::end); 
           long size = _blocks.tellg();  
           _blocks.seekg (0, std::ios::beg); 
           vector<char> data( size);
           _blocks.read( data.data(), size );
           auto vec = fc::raw::unpack<issue_fundraise_object>( data);         
           _blocks.close();
           result.push_back(vec);
         }
         itr_order ++;
     }
  }
  return result;
}
vector < buy_fundraise_object >      finance_history_plugin::get_account_buy_fundraise_history(account_id_type account, fc::time_point_sec start, fc::time_point_sec end)const
{
 string  account_id  = account.to_string();  
  fc::path sfile = my->_data_dir_loan;
  sfile /= account_id;
  sfile /= "buyfundraise";
  vector<buy_fundraise_object> result;
  if (!fc::exists(sfile))
    return result;
 
  fc::directory_iterator itr(sfile);
  vector<string> findfile;
  for( ; itr != fc::directory_iterator(); itr++ )
  {    
    string current_filename = itr->filename().string();   
    fc::time_point_sec timefile =  get_string_time(current_filename);
    if(timefile<start || timefile>end)
       continue;
    findfile.push_back(current_filename);                   
  } 
  if(findfile.size()>0)
  {
    sort(findfile.begin(),findfile.end(),compfile);
    auto itr_order = findfile.begin();    
     while (itr_order !=  findfile.end())
     {
         std::fstream _blocks;         
         _blocks.open( (sfile/(*itr_order)).generic_string().c_str(), std::fstream::binary | std::fstream::in );
         if(_blocks.is_open())
         {
           _blocks.seekg (0, std::ios::end); 
           long size = _blocks.tellg();  
           _blocks.seekg (0, std::ios::beg); 
           vector<char> data( size);
           _blocks.read( data.data(), size );
           auto vec = fc::raw::unpack<buy_fundraise_object>( data);         
           _blocks.close();
           result.push_back(vec);
         }
         itr_order ++;
     }
  }
  return result;
} 
vector < sell_exchange_object >      finance_history_plugin::get_account_sell_exchange_history(account_id_type account, fc::time_point_sec start, fc::time_point_sec end)const
{
  string  account_id  = account.to_string();  
  fc::path sfile = my->_data_dir_loan;
  sfile /= account_id;
  sfile /= "sellexchange";
  vector<sell_exchange_object> result;
  if (!fc::exists(sfile))
    return result;
 
  fc::directory_iterator itr(sfile);
  vector<string> findfile;
  for( ; itr != fc::directory_iterator(); itr++ )
  {    
    string current_filename = itr->filename().string();   
    fc::time_point_sec timefile =  get_string_time(current_filename);
    if(timefile<start || timefile>end)
       continue;
    findfile.push_back(current_filename);                   
  } 
  if(findfile.size()>0)
  {
    sort(findfile.begin(),findfile.end(),compfile);
    auto itr_order = findfile.begin();    
     while (itr_order !=  findfile.end())
     {
         std::fstream _blocks;         
         _blocks.open( (sfile/(*itr_order)).generic_string().c_str(), std::fstream::binary | std::fstream::in );
         if(_blocks.is_open())
         {
           _blocks.seekg (0, std::ios::end); 
           long size = _blocks.tellg();  
           _blocks.seekg (0, std::ios::beg); 
           vector<char> data( size);
           _blocks.read( data.data(), size );
           auto vec = fc::raw::unpack<sell_exchange_object>( data);         
           _blocks.close();
           result.push_back(vec);
         }
         itr_order ++;
     }
  }
  return result;
} 
vector < buy_exchange_object >       finance_history_plugin::get_account_buy_exchange_history(account_id_type account, fc::time_point_sec start, fc::time_point_sec end)const
{
   string  account_id  = account.to_string();  
  fc::path sfile = my->_data_dir_loan;
  sfile /= account_id;
  sfile /= "buyexchange";
  vector<buy_exchange_object> result;
  if (!fc::exists(sfile))
    return result;
 
  fc::directory_iterator itr(sfile);
  vector<string> findfile;
  for( ; itr != fc::directory_iterator(); itr++ )
  {    
    string current_filename = itr->filename().string();   
    fc::time_point_sec timefile =  get_string_time(current_filename);
    if(timefile<start || timefile>end)
       continue;
    findfile.push_back(current_filename);                   
  } 
  if(findfile.size()>0)
  {
    sort(findfile.begin(),findfile.end(),compfile);
    auto itr_order = findfile.begin();    
     while (itr_order !=  findfile.end())
     {
         std::fstream _blocks;         
         _blocks.open( (sfile/(*itr_order)).generic_string().c_str(), std::fstream::binary | std::fstream::in );
         if(_blocks.is_open())
         {
           _blocks.seekg (0, std::ios::end); 
           long size = _blocks.tellg();  
           _blocks.seekg (0, std::ios::beg); 
           vector<char> data( size);
           _blocks.read( data.data(), size );
           auto vec = fc::raw::unpack<buy_exchange_object>( data);         
           _blocks.close();
           result.push_back(vec);
         }
         itr_order ++;
     }
  }
  return result;
} 
vector<buy_fundraise_object> finance_history_plugin::list_buy_fundraise_by_id(issue_fundraise_id_type sel_id,uint64_t start,uint64_t limit)  const
{
   vector<buy_fundraise_object> re; 
   string s_id = sel_id.to_string();  
   fc::path  filew = my->_data_dir_loan / s_id;
   if (!fc::exists(filew))
         return   re;

   for (int64_t l = start; l >= 0; l--)
   {
     buy_fundraise_id_type a = buy_fundraise_id_type(l);
     fc::path sfile = filew/a.to_string();
     if (!fc::exists(sfile))
       continue;
     
     std::fstream _blocks;
     _blocks.open(sfile.generic_string().c_str(), std::fstream::binary | std::fstream::in);
     if (_blocks.is_open())
     {
       _blocks.seekg(0, std::ios::end);
       long size = _blocks.tellg();
       _blocks.seekg(0, std::ios::beg);
       vector<char> data(size);
       _blocks.read(data.data(), size);
       buy_fundraise_object result = fc::raw::unpack<buy_fundraise_object>(data);
       re.push_back(result);
       _blocks.close();
     }
     limit--;
     if(limit<=0)
       break;
    }
   return re;
}
vector<buy_exchange_object> finance_history_plugin::list_buy_exchange_by_id(sell_exchange_id_type id, uint64_t start,uint64_t limit)  const
{
   vector<buy_exchange_object> re; 
   string s_id = id.to_string();  
   fc::path  filew = my->_data_dir_loan / s_id;
   if (!fc::exists(filew))
         return   re;

   for (int64_t l = start; l >= 0; l--)
   {
     buy_exchange_id_type a = buy_exchange_id_type(l);
     fc::path sfile = filew/a.to_string();
     if (!fc::exists(sfile))
       continue;
     
     std::fstream _blocks;
     _blocks.open(sfile.generic_string().c_str(), std::fstream::binary | std::fstream::in);
     if (_blocks.is_open())
     {
       _blocks.seekg(0, std::ios::end);
       long size = _blocks.tellg();
       _blocks.seekg(0, std::ios::beg);
       vector<char> data(size);
       _blocks.read(data.data(), size);
       buy_exchange_object result = fc::raw::unpack<buy_exchange_object>(data);
       re.push_back(result);
       _blocks.close();
     }
     limit--;
     if(limit<=0)
       break;
    }
   return re;
}

}}