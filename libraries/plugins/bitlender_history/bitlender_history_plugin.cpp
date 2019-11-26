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

#include <graphene/bitlender_history/bitlender_history_plugin.hpp>

#include <graphene/chain/account_evaluator.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/config.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/chain/transaction_evaluation_state.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/business/bitlender_object.hpp> 
#include <graphene/business/bitlender_evaluator.hpp>
#include <graphene/business/protocol/bitlender_ops.hpp>

#include <fc/thread/thread.hpp>
#include <fc/smart_ref_impl.hpp>
#include <boost/algorithm/string.hpp>

namespace graphene { namespace bitlender_history {
 
namespace detail
{
struct  operation_index
{
   string  account_id;
   string  object_id;
   operation_history_object h;
};
class bitlender_history_plugin_impl
{
   public:
      bitlender_history_plugin_impl(bitlender_history_plugin& _plugin)
      :_self( _plugin ) {}
      virtual ~bitlender_history_plugin_impl();

      /** this method is called as a callback after a block is applied
       * and will process/index all operations that were applied in the block.
       */
      void update_bitlender_histories( const signed_block& b );

      graphene::chain::database& database()
      {
         return _self.database();
      }

      void     init_data_dir( );
      void     check_today_flag(const fc::time_point_sec &now);

      string   bitlender_account_id(const account_id_type &id);
      string   bitlender_object_id(const object_id_type &id);
      string   bitlender_order_id(const bitlender_order_id_type &id);
      string   bitlender_operation_result(const operation_result &id);

      void     fill_loan_operation_finish(const fc::time_point_sec &now,const fill_object_history_operation &o);
      void     fill_loan_operation_remove(const fc::time_point_sec &now,const fill_object_history_operation &o);
      void     fill_day_detail(bitlender_history_object &a,const bitlender_order_object &b);
      void     fill_day_detail(bitlender_day_detail &a,const bitlender_order_object &b,uint16_t type = 0);
      void     fill_calc_detail(bitlender_history_object &a,uint16_t type);
      void     fill_day_detailex(bitlender_history_object &a, const bitlender_order_object &b);//test

      bitlender_history_plugin &_self;
      flat_set<uint32_t>            _tracked_buckets;
      uint32_t                      _maximum_history_per_bucket_size = 1000;
      uint32_t                      _maximum_detail_day = 186;
      fc::path                      _data_dir_loan;
      fc::path                      _data_dir_object;      
      uint32_t                      today_flag = 0; 
    
};
void bitlender_history_plugin_impl::init_data_dir( )
{
  if(_data_dir_loan.string().length()<=0)
  {
   graphene::chain::database& db = database();  
   _data_dir_loan = db.get_data_dir()   / "loan_database";   
   if (!fc::exists(_data_dir_loan))
     fc::create_directories(_data_dir_loan);
   _data_dir_object = db.get_data_dir()   / "object_histroy" / "order" ;        
  }
}
void bitlender_history_plugin_impl::fill_calc_detail(bitlender_history_object &a, uint16_t type)
{
  if (type == 1) //month
  {
    for (auto ad : a.day_detail)
    {
      uint32_t day_f = get_month_flag(ad.day_tm);
      bool     new_  = true;
      for (auto &iter : a.month_detail)
      {
        if (iter.day == day_f)
        {
          iter += ad;
          new_  = false;
          break;
        }
      }
      if(new_)
      {
        bitlender_day_detail ab;
        ab.day = day_f;
        ab.day_tm = ad.day_tm;
        ab += ad;
        a.month_detail.push_back(ab);
      }
    }
    std::sort(a.month_detail.begin(), a.month_detail.end(), [&](const bitlender_day_detail &a, const bitlender_day_detail &b) -> bool 
    {    
      return a.day_tm < b.day_tm;       
    });  
  }
  else if (type == 2) //week
  {
    for (auto ad : a.day_detail)
    {
      uint32_t day_f = get_week_flag(ad.day_tm);
      bool     new_  = true;
      for (auto &iter : a.week_detail)
      {
        if (iter.day == day_f)
        {
          iter += ad;
          new_  = false;
          break;
        }
      }
      if(new_)
      {
        bitlender_day_detail ab;
        ab.day = day_f;
        ab.day_tm = ad.day_tm;
        ab += ad;
        a.week_detail.push_back(ab);
      }
    }
    std::sort(a.week_detail.begin(), a.week_detail.end(), [&](const bitlender_day_detail &a, const bitlender_day_detail &b) -> bool 
    {    
      return a.day_tm < b.day_tm;       
    });  
  }
}
void  bitlender_history_plugin_impl::fill_day_detail(bitlender_day_detail &a,const bitlender_order_object &b,uint16_t type)
{
  a.total_loan += b.amount_to_loan.amount;
  a.total_collateralize += b.amount_to_collateralize.amount;
  a.total_count ++;

  if(type == 1 || bitlender_calc::is_short_squeeze( database(),b,true))
  {
    a.squeeze_loan += b.amount_to_loan.amount;
    a.squeeze_collateralize += b.amount_to_collateralize.amount;
    a.squeeze_count ++;  
  }  
  else if(type == 2 || bitlender_calc::is_min_collateralize( database(),b,true))
  {
    a.collateral_loan += b.amount_to_loan.amount;
    a.collateral_collateralize += b.amount_to_collateralize.amount;
    a.collateral_count ++;
  } 
}
void  bitlender_history_plugin_impl::fill_day_detailex(bitlender_history_object &a,const bitlender_order_object &b)  
{
   if(a.day_detail.size()<=0)
   {
     uint16_t tt = 0;
     fc::time_point now = database().head_block_time();
     for (int ll =_maximum_detail_day ; ll >=0 ;ll--)
     {
       fc::time_point _time = now - fc::seconds(ll * 3600 * 24);
       srand(tt + 1000);
       uint64_t _rand = rand();
       _rand = _rand % 3 + 3;
       bitlender_order_object bb;
       bb.amount_to_loan          = b.amount_to_loan;
       bb.amount_to_collateralize = b.amount_to_collateralize;
       bb.amount_to_collateralize.amount   = 1234 * _rand * 100000000;
       bb.amount_to_loan.amount            = 2234 * _rand * 100;
       bitlender_day_detail ab;
       ab.day    = get_day_flag(_time);;
       ab.day_tm = _time;
       fill_day_detail(ab, bb,tt %3);
       a.day_detail.push_back(ab);
       tt++;
     }
   }
}
void  bitlender_history_plugin_impl::fill_day_detail(bitlender_history_object &a,const bitlender_order_object &b)
{
  fc::time_point_sec  _time = b.invest_finish_time - b.offset_time;
  uint32_t day_f   = get_day_flag(_time);
  uint32_t day_now = get_day_flag(database().head_block_time());

  if(b.is_loan() || b.is_investing())
    return;
   if(b.is_recycleex() || b.is_squeeze()) 
      a.total_recycle_count ++;    
  if (day_now - day_f >= _maximum_detail_day)
    return;
  for (auto &iter : a.day_detail)
  {
    if(iter.day == day_f)
    {
      fill_day_detail(iter,b);
      return;
    }
  }  
  {
    bitlender_day_detail ab;
    ab.day    = day_f;
    ab.day_tm = _time;
    fill_day_detail(ab, b);
    a.day_detail.push_back(ab);
    if(a.day_detail.size()>_maximum_detail_day)
      a.day_detail.erase(a.day_detail.begin(),a.day_detail.begin() + (a.day_detail.size() -_maximum_detail_day));
     std::sort(a.day_detail.begin(), a.day_detail.end(), [&](const bitlender_day_detail &a, const bitlender_day_detail &b) -> bool 
     {    
       return a.day_tm < b.day_tm;       
     });  
  }
}
//投标销毁
void  bitlender_history_plugin_impl::fill_loan_operation_remove(const fc::time_point_sec &now,const fill_object_history_operation &o)
{
  try{
    if(o.itype != fill_object_order)
      return;
    auto order  = fc::raw::unpack<bitlender_order_object>(o.data);    
    if(!(order.is_principalfinish() || order.is_recyclefinish()))
      return;
    bitlender_bucket_key key;
    key.base  = order.amount_to_loan.asset_id;
    key.quote = order.amount_to_collateralize.asset_id;  
    const auto& bucket_idx =  database().get_index_type<bitlender_history_index>();
    const auto& by_key_idx = bucket_idx.indices().get<by_bitlender_key>();
    auto bucket_itr = by_key_idx.find( key );   
    if(bucket_itr == by_key_idx.end())
      return;
    share_type earnings_amount = 0;        
    for(auto &invest : order.repay_interest)
    {      
        earnings_amount += invest.second.amount_repay_interest.amount;
        earnings_amount += invest.second.fines_repay_interest.amount;      
    }
    earnings_amount += order.repay_principal_fee.amount; 
    

    database().modify(*bucket_itr, [&](bitlender_history_object &b) {
      b.today_earnings_amount += earnings_amount;
      b.total_earnings_amount += earnings_amount;      
      if(order.is_recyclefinish()) 
          b.total_recycle_count ++; 
      fill_day_detail(b,order);    
    });
  }
  catch(...)
  {
  }
} 
void bitlender_history_plugin_impl::check_today_flag(const fc::time_point_sec &now) 
{
  try{
    if(get_day_flag(now)!= today_flag)
    {
        today_flag = get_day_flag(now);
        const auto& bucket_idx =  database().get_index_type<bitlender_history_index>().indices().get<by_id>();       
        auto bucket_itr = bucket_idx.begin();
        while(bucket_itr != bucket_idx.end())
        {          
          database().modify(*bucket_itr, [&](bitlender_history_object &b)
          {
            b.reset_today_flag();
          });          
          bucket_itr++;
        }
    }
  }catch(const fc::exception& e) 
  {
    elog(e.to_string());
  }
}

//投标结束
 void bitlender_history_plugin_impl::fill_loan_operation_finish(const fc::time_point_sec &now,const fill_object_history_operation& o ) 
 {
   try{ 
   if(o.itype != fill_object_finish)
      return;
   auto &d = database();
   bitlender_order_id_type orderid = o.id;
   auto &order = orderid(database());
   if (!(order.is_investfinish() || order.is_interest() || order.is_interestfinish()))
     return;
   bitlender_bucket_key key;
   key.base = order.amount_to_loan.asset_id;
   key.quote = order.amount_to_collateralize.asset_id;
   const auto &bucket_idx = database().get_index_type<bitlender_history_index>();
   const auto &by_key_idx = bucket_idx.indices().get<by_bitlender_key>();
   auto bucket_itr = by_key_idx.find(key);
   if (bucket_itr == by_key_idx.end())
   {
     database().create<bitlender_history_object>([&](bitlender_history_object &b) {
       b.key = key;
       auto base = key.base(d);
       auto quote = key.quote(d);
       b.base_symbol     = base.symbol;
       b.base_precision  = base.precision;
       b.quote_symbol    = quote.symbol;
       b.quote_precision = quote.precision;        
     });
     bucket_itr = by_key_idx.find(key);   
    }
    if(bucket_itr == by_key_idx.end())
      return;
    database().modify(*bucket_itr, [&](bitlender_history_object &b)
    {      
      b.fee_mode = order.fee_mode;
      b.today_laon_amount += order.amount_to_loan.amount;        
      b.today_collateralize_amount += order.amount_to_collateralize.amount; 
      if(order.collateralize_fee.asset_id == order.amount_to_loan.asset_id)       
      {
         b.today_loan_collateralize_fee += order.collateralize_fee.amount;
         b.today_loan_collateralize_risk += order.collateralize_risk.amount;
      }
      else 
      {
        b.today_invest_collateralize_fee += order.collateralize_fee.amount;
        b.today_invest_collateralize_risk += order.collateralize_risk.amount;
      }
      b.today_loan_carrier_fee += order.carrier_fee.amount;
      if(b.today_max_ratio<order.interest_rate.interest_rate)        
        b.today_max_ratio = order.interest_rate.interest_rate;
      if(b.today_min_ratio>order.interest_rate.interest_rate)
        b.today_min_ratio = order.interest_rate.interest_rate;        
      b.today_laon_count +=1;
      b.today_invest_count += order.interest_book.size();


      b.total_laon_amount += order.amount_to_loan.amount;        
      b.total_collateralize_amount += order.amount_to_collateralize.amount;  
      if(order.collateralize_fee.asset_id == order.amount_to_loan.asset_id) 
      {      
        b.total_loan_collateralize_fee += order.collateralize_fee.amount;
        b.total_loan_collateralize_risk += order.collateralize_risk.amount;
      }
      else 
      {
        b.total_invest_collateralize_fee += order.collateralize_fee.amount;
        b.total_invest_collateralize_risk += order.collateralize_risk.amount;
      }
      b.total_loan_carrier_fee += order.carrier_fee.amount;
      if(b.total_max_ratio<order.interest_rate.interest_rate)        
        b.total_max_ratio = order.interest_rate.interest_rate;
      if(b.total_min_ratio>order.interest_rate.interest_rate)
        b.total_min_ratio = order.interest_rate.interest_rate;        
      b.total_laon_count +=1;
       b.total_invest_count += order.interest_book.size();

      // fill_day_detailex(b,order);  
    });
   }
    catch(...)
    {

    }
 }

 bitlender_history_plugin_impl::~bitlender_history_plugin_impl()
 {}

string   bitlender_history_plugin_impl::bitlender_account_id(const account_id_type &id)
{
    fc::variant result;
    fc::to_variant( id, result, GRAPHENE_NET_MAX_NESTED_OBJECTS );
    return result.as_string();
}
string   bitlender_history_plugin_impl::bitlender_object_id(const object_id_type &id)
{
    fc::variant result;
    fc::to_variant( id, result, GRAPHENE_NET_MAX_NESTED_OBJECTS );
    return result.as_string();
}
string   bitlender_history_plugin_impl::bitlender_order_id(const bitlender_order_id_type &id)
{
    fc::variant result;
    fc::to_variant( id, result, GRAPHENE_NET_MAX_NESTED_OBJECTS );
    return result.as_string();
}
string   bitlender_history_plugin_impl::bitlender_operation_result(const operation_result &id)
{
   // fc::variant result;
   // fc::to_variant( id, result, GRAPHENE_NET_MAX_NESTED_OBJECTS );
   // return result[1].as_string();    
    return bitlender_object_id(id.get<object_id_type>())  ;
} 

void bitlender_history_plugin_impl::update_bitlender_histories( const signed_block& b )
{
   vector<operation_index> *pindex = new vector<operation_index>;
   vector<fill_object_history_operation> *paloan = new vector<fill_object_history_operation>;

  try{

    init_data_dir();
    graphene::chain::database& db = database(); 

    const vector<optional<operation_history_object>> &hist = db.get_applied_operations();
    check_today_flag(b.timestamp);

    string stime = fc::to_string(b.timestamp.sec_since_epoch());
    for (const optional<operation_history_object> &o_op : hist)
    {
      operation_index index;
      switch (o_op->op.which())
      {
      case operation::tag<bitlender_loan_operation>::value:
      {
        index.account_id = bitlender_account_id(o_op->op.get<bitlender_loan_operation>().issuer);
        index.object_id = bitlender_operation_result(o_op->result);
        break;
      }
      case operation::tag<bitlender_invest_operation>::value:
      {
        auto &o = o_op->op.get<bitlender_invest_operation>();
        index.account_id = bitlender_account_id(o.issuer);
        index.object_id = bitlender_order_id(o.order_id);
        break;
      }
      case operation::tag<bitlender_repay_interest_operation>::value:
      {
        auto &o = o_op->op.get<bitlender_repay_interest_operation>();
        index.account_id = bitlender_account_id(o.issuer);
        index.object_id = bitlender_order_id(o.order_id);
        break;
      }
      case operation::tag<bitlender_overdue_interest_operation>::value:
      {
        auto &o = o_op->op.get<bitlender_overdue_interest_operation>();
        index.account_id = bitlender_account_id(o.issuer);
        index.object_id = bitlender_order_id(o.order_id);
        break;
      }
      case operation::tag<bitlender_recycle_interest_operation>::value:
      {
        auto &o = o_op->op.get<bitlender_recycle_interest_operation>();
        index.account_id = bitlender_account_id(o.issuer);
        index.object_id = bitlender_order_id(o.order_id);
        break;
      }      
      case operation::tag<bitlender_repay_principal_operation>::value:
      {
        auto &o = o_op->op.get<bitlender_repay_principal_operation>();
        index.account_id = bitlender_account_id(o.issuer);
        index.object_id = bitlender_order_id(o.order_id);
        break;
      }
      case operation::tag<bitlender_pre_repay_principal_operation>::value:
      {
        auto &o = o_op->op.get<bitlender_pre_repay_principal_operation>();
        index.account_id = bitlender_account_id(o.issuer);
        index.object_id = bitlender_order_id(o.order_id);
        break;
      }
      case operation::tag<bitlender_setautorepayer_operation>::value:
      {
        auto &o = o_op->op.get<bitlender_setautorepayer_operation>();
        index.account_id = bitlender_account_id(o.issuer);
        index.object_id = bitlender_order_id(o.order_id);
        break;
      }
      case operation::tag<bitlender_autorepayment_operation>::value:
      {
        auto &o = o_op->op.get<bitlender_autorepayment_operation>();
        index.account_id = bitlender_account_id(o.issuer);
        index.object_id = bitlender_order_id(o.order_id);
        break;
      }
      case operation::tag<bitlender_overdue_repay_principal_operation>::value:
      {
        auto &o = o_op->op.get<bitlender_overdue_repay_principal_operation>();
        index.account_id = bitlender_account_id(o.issuer);
        index.object_id = bitlender_order_id(o.order_id);
        break;
      }
      case operation::tag<bitlender_add_collateral_operation>::value:
      {
        auto &o = o_op->op.get<bitlender_add_collateral_operation>();
        index.account_id = bitlender_account_id(o.issuer);
        index.object_id = bitlender_order_id(o.order_id);
        break;
      }
      case operation::tag<bitlender_recycle_operation>::value:
      {
        auto &o = o_op->op.get<bitlender_recycle_operation>();
        index.account_id = bitlender_account_id(o.issuer);
        index.object_id = bitlender_order_id(o.order_id);
        break;
      }
      case operation::tag<bitlender_remove_operation>::value:
      {
        auto &o = o_op->op.get<bitlender_remove_operation>();
        index.account_id = bitlender_account_id(o.issuer);
        index.object_id = bitlender_order_id(o.order_id);
        break;
      }
      case operation::tag<bitlender_squeeze_operation>::value:
      {
        auto &o = o_op->op.get<bitlender_squeeze_operation>();
        index.account_id = bitlender_account_id(o.issuer);
        index.object_id = bitlender_order_id(o.order_id);
        break;
      }      
      case operation::tag<fill_object_history_operation>::value:
      {
        auto &o = o_op->op.get<fill_object_history_operation>();
        if (o.itype == fill_object_finish)
          fill_loan_operation_finish(b.timestamp, o);
        if (o.itype == fill_object_order)
          fill_loan_operation_remove(b.timestamp,o);
        if (o.itype == fill_object_order || o.itype == fill_object_invest)
          paloan->push_back(o);         
        continue;
      }
      default:
        continue;
      }
      index.h.id = o_op->id;
      index.h.op = o_op->op;
      index.h.result = o_op->result;
      index.h.block_num = o_op->block_num;
      index.h.trx_in_block = o_op->trx_in_block;
      index.h.op_in_trx = o_op->op_in_trx;
      index.h.virtual_op = o_op->virtual_op;
      pindex->push_back(index);      
   }            
   if(pindex ->size()>0)
   {      
      fc::async( [pindex,this,stime]() {         
        auto itr_order = pindex->begin();
        int  ifileindex = 1;
        while (itr_order !=  pindex->end())
        {
          operation_index &index = *itr_order;
          try{          
          fc::path  filew = this->_data_dir_loan / index.account_id;
          if (!fc::exists(filew))
             fc::create_directories(filew);
          filew /= index.object_id;
          if (!fc::exists(filew))
            fc::create_directories(filew);   
          fc::path sfile =  filew /  (stime + "-" +  fc::to_string(index.h.op.which()) + "-" + fc::to_string(ifileindex));        

          std::fstream _blocks;
          _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::out | std::fstream::trunc);
          auto vec = fc::raw::pack( index.h);
          _blocks.write( vec.data(), vec.size() );
          _blocks.close();

          ifileindex ++;                 
          itr_order++;
          }FC_CAPTURE_AND_RETHROW( (index.h)) 
        }
        delete pindex;
      } );
   }
   else 
     delete pindex;

   if(paloan->size()>0)
   {     
      fc::async( [paloan,this,stime]() {   
        int  ifileindex = 1;      
        auto itr_order = paloan->begin();        
        while (itr_order !=  paloan->end())
        {
          fill_object_history_operation &index = *itr_order;
          try{          
          string account_id = bitlender_account_id(index.issuer);  
          fc::path  filew = this->_data_dir_loan / account_id;
          if (!fc::exists(filew))
             fc::create_directories(filew);

          if(index.id.type()  == bitlender_order_object_type) 
             filew /= "order";
          else if(index.id.type()  == impl_bitlender_invest_object_type) 
             filew /= "invest";           
          else 
             filew /= fc::to_string(index.id.type());   

          if (!fc::exists(filew))
            fc::create_directories(filew);   

          fc::path sfile  =  filew /  (stime + "-" +  fc::to_string(ifileindex) + "-" + bitlender_object_id(index.id));                 

          std::fstream _blocks;
          _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::out | std::fstream::trunc);          
          _blocks.write( index.data.data(), index.data.size() );
          _blocks.close();

          
                   
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
     delete pindex;
     delete paloan;
   }      
}

} // end namespace detail






bitlender_history_plugin::bitlender_history_plugin() :
   my( new detail::bitlender_history_plugin_impl(*this) )
{
}

bitlender_history_plugin::~bitlender_history_plugin()
{
}

std::string bitlender_history_plugin::plugin_name()const
{
   return "bitlender_history";
}

void bitlender_history_plugin::plugin_set_program_options(
   boost::program_options::options_description& cli,
   boost::program_options::options_description& cfg
   )
{
   cli.add_options()
         ("bitlender_bucket-size", boost::program_options::value<string>()->default_value("[60,300,900,1800,3600,14400,86400]"),
           "Track market history by grouping orders into buckets of equal size measured in seconds specified as a JSON array of numbers")
         ("bitlender_history-max-day", boost::program_options::value<uint32_t>()->default_value(186),
           "How far max day (default: 186)")  
         ("bitlender_history-per-size", boost::program_options::value<uint32_t>()->default_value(1000),
           "How far back in time to track history for each bucket size, measured in the number of buckets (default: 1000)")         
         ;
   cfg.add(cli);
}
void bitlender_history_plugin::get_info(fc::mutable_variant_object &result) 
{
  result["day-flag"] = my->today_flag;  
  result["bitlender_history-per-size"] = my->_maximum_history_per_bucket_size;   
  result["bitlender_history-max-day"] = my->_maximum_detail_day;
}
void bitlender_history_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{ try {  
    
    database().applied_block.connect([this](const signed_block &b) { my->update_bitlender_histories(b); });
    database().add_index< primary_index< bitlender_history_index  > >();   
    my->today_flag = get_day_flag(fc::time_point::now());

     if( options.count( "bucket-size" ) )
   {
      const std::string& buckets = options["bucket-size"].as<string>();
      my->_tracked_buckets = fc::json::from_string(buckets).as<flat_set<uint32_t>>(2);
      my->_tracked_buckets.erase( 0 );
   }
   if( options.count( "history-per-size" ) )
      my->_maximum_history_per_bucket_size = options["history-per-size"].as<uint32_t>();
  if( options.count( "bitlender_history-max-day" ) )
      my->_maximum_detail_day = options["bitlender_history-max-day"].as<uint32_t>();

   database()._app_status.has_bitlender_history_plugin = true;    
  } 

  FC_CAPTURE_AND_RETHROW()
}

void bitlender_history_plugin::plugin_startup()
{

}
const flat_set<uint32_t>& bitlender_history_plugin::tracked_buckets() const
{
   return my->_tracked_buckets;
}

 bool compfile(const string &a, const string &b){
     return a > b;
 }

 void   bitlender_history_plugin::fill_day_detail(bitlender_history_object &a,const bitlender_order_object &b)
 {
   return my->fill_day_detail(a,b);
 }
 void   bitlender_history_plugin::fill_calc_detail(bitlender_history_object &a,uint16_t type)
 {
     return my->fill_calc_detail(a,type);
 }
vector<operation_history_object> bitlender_history_plugin::get_account_bitlender_history( account_id_type account,bitlender_order_id_type order ) const
{
  string  account_id  = my->bitlender_account_id(account);
  string  object_id   = my->bitlender_order_id(order);
  fc::path sfile = my->_data_dir_loan;
  sfile /= account_id;
  sfile /= object_id;
  vector<operation_history_object> result;
  if (!fc::exists(sfile))
    return result;
   
  fc::directory_iterator itr(sfile);
  vector<string> findfile;
  for( ; itr != fc::directory_iterator(); itr++ )
  {    
    string current_filename = itr->filename().string();    
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
           auto vec = fc::raw::unpack<operation_history_object>( data);         
           _blocks.close();
           result.push_back(vec);
         }
         itr_order ++;
     }
  }
  return result;
}
fc::time_point_sec get_string_time(string stext)
{
  vector< string >  SplitVec; 
  boost::split(SplitVec, stext, boost::is_any_of("-"), boost::token_compress_on );  
  if(SplitVec.size()<=0)
    return fc::time_point_sec::min();
  else
   return fc::time_point_sec(fc::to_int64(SplitVec[0]));  
}
uint64_t bitlender_history_plugin::get_account_loan_history_count( account_id_type account, fc::time_point_sec start, fc::time_point_sec end )const
{
   string  account_id  = my->bitlender_account_id(account);  
  fc::path sfile = my->_data_dir_loan;
  sfile /= account_id;
  sfile /= "order";
  
  if (!fc::exists(sfile))
    return 0;
 
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

  return findfile.size();
}
vector<bitlender_order_info> bitlender_history_plugin::get_account_loan_history( account_id_type account, fc::time_point_sec start, fc::time_point_sec end ,uint64_t ustart,uint64_t ulimit )const
{ 
  uint64_t index = 0;
  string  account_id  = my->bitlender_account_id(account);  
  fc::path sfile = my->_data_dir_loan;
  sfile /= account_id;
  sfile /= "order";
  vector<bitlender_order_info> result;
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
  const graphene::chain::database& d = my->database();  
  if(findfile.size()>0)
  {
    sort(findfile.begin(),findfile.end(),compfile);
    auto itr_order = findfile.begin();    
     while (itr_order !=  findfile.end() &&  index <ulimit + ustart)
     {
        index++;
       if(index<ustart+1)
       {
         itr_order ++;
         continue;
       }

         std::fstream _blocks;         
         _blocks.open( (sfile/(*itr_order)).generic_string().c_str(), std::fstream::binary | std::fstream::in );
         if(_blocks.is_open())
         {
           _blocks.seekg (0, std::ios::end); 
           long size = _blocks.tellg();  
           _blocks.seekg (0, std::ios::beg); 
           vector<char> data( size);
           _blocks.read( data.data(), size );
           auto vec = fc::raw::unpack<bitlender_order_object>( data);         
           _blocks.close();
           bitlender_order_info info;            
           vec.get_info(d,info);          
           result.push_back(info);            
         }
         itr_order ++;
     }
  }
  return result;

}
optional<bitlender_order_object> bitlender_history_plugin::get_loan_history( bitlender_order_id_type order)  const
{
  fc::path sfile = my->_data_dir_object;
  sfile /= order.to_string(); 
  if (!fc::exists(sfile))
    return optional<bitlender_order_object>();    
  
  std::fstream _blocks;         
  _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::in );
  if(_blocks.is_open())
  {
    _blocks.seekg (0, std::ios::end); 
    long size = _blocks.tellg();  
    _blocks.seekg (0, std::ios::beg); 
    vector<char> data( size);
    _blocks.read( data.data(), size );
    auto vec = fc::raw::unpack<bitlender_order_object>( data);         
    _blocks.close();  
    return vec;
  }
  return optional<bitlender_order_object>();
}
uint64_t bitlender_history_plugin::get_account_invest_history_count( account_id_type account, fc::time_point_sec start, fc::time_point_sec end )const
{
  string account_id = my->bitlender_account_id(account);
  fc::path sfile = my->_data_dir_loan;
  sfile /= account_id;
  sfile /= "invest";
  
  if (!fc::exists(sfile))
    return 0;
 
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

  return findfile.size();
} 
 
vector<bitlender_invest_info> bitlender_history_plugin::get_account_invest_history( account_id_type account, fc::time_point_sec start, fc::time_point_sec end ,uint64_t ustart,uint64_t ulimit)const
{
  uint64_t index = 0;
  string account_id = my->bitlender_account_id(account);
  fc::path sfile = my->_data_dir_loan;
  sfile /= account_id;
  sfile /= "invest";
  vector<bitlender_invest_info> result;
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
  
  const graphene::chain::database &d = my->database();
  if (findfile.size() > 0)
  {
    sort(findfile.begin(),findfile.end(),compfile);
    auto itr_order = findfile.begin();    
     while (itr_order !=  findfile.end() && index <ulimit + ustart)
     {
       index++;
       if(index<ustart+1)
       {
          itr_order ++;
         continue;
       }
       std::fstream _blocks;
       _blocks.open((sfile / (*itr_order)).generic_string().c_str(), std::fstream::binary | std::fstream::in);
       if (_blocks.is_open())
       {
         _blocks.seekg(0, std::ios::end);
         long size = _blocks.tellg();
         _blocks.seekg(0, std::ios::beg);
         vector<char> data(size);
         _blocks.read(data.data(), size);
         auto vec = fc::raw::unpack<bitlender_invest_object>(data);
         _blocks.close();
         bitlender_invest_info info;
         vec.get_info(d, info, false);
         optional<bitlender_order_object> porder = get_loan_history( vec.order_id);
         if(porder)
         {
           bitlender_order_info ginfo;
           porder->get_info(d, ginfo);
           info.order_info = ginfo;
           result.push_back(info);
         }         
         itr_order ++;
       }
     }
  }
  return result;
} 

} }
