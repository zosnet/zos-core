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

#include <graphene/object_history/object_history_plugin.hpp>

#include <graphene/chain/account_evaluator.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/config.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/chain/transaction_evaluation_state.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/business/protocol/bitlender_ops.hpp>

#include <fc/thread/thread.hpp>
#include <fc/smart_ref_impl.hpp>
#include <boost/algorithm/string.hpp>

namespace graphene { namespace object_history  {
 
namespace detail
{ 
class object_history_plugin_impl
{
   public:
      object_history_plugin_impl(object_history_plugin& _plugin)
      :_self( _plugin ) {}
      virtual ~object_history_plugin_impl();

      /** this method is called as a callback after a block is applied
       * and will process/index all operations that were applied in the block.
       */
      void update_object_histories( const signed_block& b );

      graphene::chain::database& database()
      {
         return _self.database();
      }
      void      init_data_dir( );    
      string    object_id_to_string(const object_id_type &id);          
      fc::path  get_account_object_path(   object_id_type id ,bool bcheck = false);
      fc::path  get_account_limit_path(object_id_type accid,  object_id_type src_id,object_id_type dest_id, bool bcheck = false);

      object_history_plugin  &_self;
      fc::path               _data_dir_object;
      fc::path               _limit_dir_object;
};
void object_history_plugin_impl::init_data_dir( )
{
  if(_data_dir_object.string().length()<=0)
  {
    graphene::chain::database& db = database(); 
    _limit_dir_object = db.get_data_dir() / "limit_histroy";   
    if (!fc::exists(_limit_dir_object))
      fc::create_directories(_limit_dir_object);  
    _data_dir_object = db.get_data_dir() / "object_histroy";   
    if (!fc::exists(_data_dir_object))
      fc::create_directories(_data_dir_object);  
    if (!fc::exists(_data_dir_object/"order"))
      fc::create_directories(_data_dir_object/"order");  
    if (!fc::exists(_data_dir_object/"locktoken"))
      fc::create_directories(_data_dir_object/"locktoken");  
    if (!fc::exists(_data_dir_object/"invest"))
      fc::create_directories(_data_dir_object/"invest");  
    if (!fc::exists(_data_dir_object/ "issuefundraise"))
      fc::create_directories(_data_dir_object/ "issuefundraise");  
    if (!fc::exists(_data_dir_object/ "buyfundraise"))
      fc::create_directories(_data_dir_object/ "buyfundraise");  
    if (!fc::exists(_data_dir_object/ "sellexchange"))
      fc::create_directories(_data_dir_object/ "sellexchange");  
    if (!fc::exists(_data_dir_object/ "buyxchange"))
      fc::create_directories(_data_dir_object/ "buyxchange");  
    if (!fc::exists(_data_dir_object/"worker"))
      fc::create_directories(_data_dir_object/"worker");  
    if (!fc::exists(_data_dir_object/"proposal"))
      fc::create_directories(_data_dir_object/"proposal");        
  }
} 

object_history_plugin_impl::~object_history_plugin_impl()
{}
 
string   object_history_plugin_impl::object_id_to_string(const object_id_type &id)
{
    fc::variant result;
    fc::to_variant( id, result, GRAPHENE_NET_MAX_NESTED_OBJECTS );
    return result.as_string();
} 
fc::path  object_history_plugin_impl::get_account_limit_path(  object_id_type accid,  object_id_type src_id,object_id_type dest_id, bool bcheck   )
{
   fc::path filew = this->_limit_dir_object;
   filew /= "limitorder";
   if (bcheck && !fc::exists(filew))
    fc::create_directories(filew);     
   if (src_id  > dest_id)
     filew /= accid.to_string() + src_id.to_string() + dest_id.to_string();
   else   
    filew /= accid.to_string() + dest_id.to_string() + src_id.to_string();
   if (bcheck && !fc::exists(filew))
     fc::create_directories(filew);  
   return filew;  
}
fc::path  object_history_plugin_impl::get_account_object_path(  object_id_type id ,bool bcheck   )
{
  fc::path filew = this->_data_dir_object;
  if (id.space() == implementation_ids)
  {
    if (id.type() == impl_bitlender_invest_object_type)
      filew /= "invest";
  }
  else
  {
    if (id.type() == bitlender_order_object_type)
      filew /= "order";
    else if (id.type() == buy_fundraise_object_type)
      filew /= "buyfundraise";
    else if (id.type() == issue_fundraise_object_type)
      filew /= "issuefundraise";
    else if (id.type() == worker_object_type)
      filew /= "worker";
    else if (id.type() == locktoken_object_type)
      filew /= "locktoken";  
    else if (id.type() == limit_order_object_type)
      filew /= "limitorder";    
    else if (id.type() == withdraw_permission_object_type)
      filew /= "withdrawpermission";  
    else if (id.type() == sell_exchange_object_type)
      filew /= "sellexchange";
    else if (id.type() == buy_exchange_object_type)
      filew /= "buyexchange";
    else if (id.type() == proposal_object_type)
      filew /= "proposal";
    else
      filew /= fc::to_string(id.type());
  }

  if (bcheck && !fc::exists(filew))
     fc::create_directories(filew);        
  
  string object_id = object_id_to_string(id);     
  return filew/object_id;  
}

void object_history_plugin_impl::update_object_histories( const signed_block& b )
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
        if (o.itype == fill_object_order || o.itype == fill_object_invest || o.itype == fill_object_issue_fundraise  || 
            o.itype == fill_object_buy_fundraise  ||  o.itype == fill_object_sell_exchange ||  o.itype == fill_object_buy_exchange || 
            o.itype == fill_object_worker ||  o.itype == fill_object_withdraw_permission || o.itype == fill_object_proposal || 
            o.itype == fill_object_limitorder || o.itype == fill_object_locktoken)
          paloan->push_back(std::move(o));     
        continue;
      }
      default:
        continue;
      }      
   }   
   if(paloan->size()>0)
   {     
      fc::async([paloan,this,stime]() 
      {           
        auto itr_order = paloan->begin();        
        while (itr_order !=  paloan->end())
        {
          fill_object_history_operation &index = *itr_order;
          try{                        
            {
              fc::path sfile =  get_account_object_path(index.id,true);        
              std::fstream _blocks;
              _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::out | std::fstream::trunc);          
              _blocks.write( index.data.data(), index.data.size() );
              _blocks.close();                   
            }
            if (index.itype == fill_object_limitorder) {
              fc::path sfile = get_account_limit_path(index.issuer, index.src_id, index.dest_id, true);
              sfile = sfile / index.id.to_string();  
              std::fstream _blocks;
              _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::out | std::fstream::trunc);          
              _blocks.write( index.data.data(), index.data.size() );
              _blocks.close();    
            }
            itr_order++;                    
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

object_history_plugin::object_history_plugin() :
   my( new detail::object_history_plugin_impl(*this) )
{
}

object_history_plugin::~object_history_plugin()
{
}

std::string object_history_plugin::plugin_name()const
{
   return "object_history";
}

void object_history_plugin::plugin_set_program_options(
   boost::program_options::options_description& cli,
   boost::program_options::options_description& cfg
   )
{
  
}
void object_history_plugin::get_info(fc::mutable_variant_object &result) 
{
  
}
void object_history_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{ try {    
    database().applied_block.connect( [this]( const signed_block& b){ my->update_object_histories(b); } );  
    database()._app_status.has_object_history_plugin = true;
    
    } FC_CAPTURE_AND_RETHROW()
}

void object_history_plugin::plugin_startup()
{
 
}  
 
 vector<worker_object> object_history_plugin::get_workers_history(worker_id_type start, uint64_t limit )const
 {
   vector<worker_object> re;
   object_id_type id_start = start;
   for (int64_t l = id_start.instance(); l >= 0; l--)
   {
     worker_id_type a = worker_id_type(l);
     fc::path sfile = my->get_account_object_path(a);
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
       worker_object result = fc::raw::unpack<worker_object>(data);
       re.push_back(result);
       _blocks.close();
     }
     limit--;
     if(limit<=0)
       break;
    }
   return re;
 }

 vector<proposal_object> object_history_plugin::get_proposals_history(account_id_type account,proposal_id_type start, uint64_t limit )const
 {
   vector<proposal_object> re;
   object_id_type id_start = start;
   for (int64_t l = id_start.instance(); l >= 0; l--)
   {
     proposal_id_type a = proposal_id_type(l);
     fc::path sfile = my->get_account_object_path(a);
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
       proposal_object result = fc::raw::unpack<proposal_object>(data);
       if(account == account_id_type(0) || result.proposer == account)
          re.push_back(result);
       _blocks.close();
     }
     if(re.size() >= limit)
       break;
   }
   return re;
 }
 bitlender_order_data object_history_plugin::get_loan_object_history(object_id_type id) const
 {
   bitlender_order_data result;
   fc::path sfile = my->get_account_object_path(id);

   if (!fc::exists(sfile))
   {
     FC_ASSERT(false, "file ${a} ", ("a", sfile.generic_string().c_str()));
     return result;
   }
   const graphene::chain::database& db = my->database();  
   std::fstream _blocks;
   _blocks.open(sfile.generic_string().c_str(), std::fstream::binary | std::fstream::in);
   if (_blocks.is_open())
   {
     _blocks.seekg(0, std::ios::end);
     long size = _blocks.tellg();
     _blocks.seekg(0, std::ios::beg);
     vector<char> data(size);
     _blocks.read(data.data(), size);
     bitlender_order_object vec = fc::raw::unpack<bitlender_order_object>(data);
     result.get_object(db, vec);
     _blocks.close();
   }
   return result; 
}

bitlender_invest_data  object_history_plugin::get_invest_object_history(   object_id_type id  )const
{  
  fc::path  sfile = my->get_account_object_path(id);
  bitlender_invest_data result;
  if (!fc::exists(sfile))
  {
    FC_ASSERT(false,"file ${a} ",("a", sfile.generic_string().c_str()));
    return result;
  }
  const graphene::chain::database& db = my->database();  

  std::fstream _blocks;         
  _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::in );
  if(_blocks.is_open())
  {
    _blocks.seekg (0, std::ios::end); 
    long size = _blocks.tellg();  
    _blocks.seekg (0, std::ios::beg); 
    vector<char> data( size);
    _blocks.read( data.data(), size );
    bitlender_invest_object vec = fc::raw::unpack<bitlender_invest_object>( data);   
    result.get_object(db,vec,false);
    bitlender_order_data   obj_data = get_loan_object_history(vec.order_id);
    result.order_info = obj_data;
    _blocks.close();
  }
  return result;
}
limit_order_object  object_history_plugin::get_limit_order_object_history(   object_id_type id  )const
{
   fc::path  sfile = my->get_account_object_path(id);
  limit_order_object result;
  if (!fc::exists(sfile))
  {
    FC_ASSERT(false,"file ${a} ",("a",sfile.to_native_ansi_path()));
    return result;
  }

  std::fstream _blocks;         
  _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::in );
  if(_blocks.is_open())
  {
    _blocks.seekg (0, std::ios::end); 
    long size = _blocks.tellg();  
    _blocks.seekg (0, std::ios::beg); 
    vector<char> data( size);
    _blocks.read( data.data(), size );
    result = fc::raw::unpack<limit_order_object>( data);           
    _blocks.close();        
  }
  return result;
}
buy_fundraise_object  object_history_plugin::get_buy_fundraise_object_history(   object_id_type id  )const
{  
  fc::path  sfile = my->get_account_object_path(id);
  buy_fundraise_object result;
  if (!fc::exists(sfile))
  {
    FC_ASSERT(false,"file ${a} ",("a",sfile.to_native_ansi_path()));
    return result;
  }

  std::fstream _blocks;         
  _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::in );
  if(_blocks.is_open())
  {
    _blocks.seekg (0, std::ios::end); 
    long size = _blocks.tellg();  
    _blocks.seekg (0, std::ios::beg); 
    vector<char> data( size);
    _blocks.read( data.data(), size );
    result = fc::raw::unpack<buy_fundraise_object>( data);   
        
    _blocks.close();        
  }
  return result;
}


issue_fundraise_object  object_history_plugin::get_issue_fundraise_object_history(   object_id_type id  )const
{  
  fc::path  sfile = my->get_account_object_path(id);
  issue_fundraise_object result;
  if (!fc::exists(sfile))
  {
    FC_ASSERT(false,"file ${a} ",("a",sfile.to_native_ansi_path()));
    return result;
  }

  std::fstream _blocks;         
  _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::in );
  if(_blocks.is_open())
  {
    _blocks.seekg (0, std::ios::end); 
    long size = _blocks.tellg();  
    _blocks.seekg (0, std::ios::beg); 
    vector<char> data( size);
    _blocks.read( data.data(), size );
    result = fc::raw::unpack<issue_fundraise_object>( data);   
        
    _blocks.close();        
  }
  return result;
}

withdraw_permission_object  object_history_plugin::get_withdraw_permission_object_history(   object_id_type id  )const
{  
  fc::path  sfile = my->get_account_object_path(id);
  withdraw_permission_object result;
  if (!fc::exists(sfile))
  {
    FC_ASSERT(false,"file ${a} ",("a",sfile.to_native_ansi_path()));
    return result;
  }

  std::fstream _blocks;         
  _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::in );
  if(_blocks.is_open())
  {
    _blocks.seekg (0, std::ios::end); 
    long size = _blocks.tellg();  
    _blocks.seekg (0, std::ios::beg); 
    vector<char> data( size);
    _blocks.read( data.data(), size );
    result = fc::raw::unpack<withdraw_permission_object>( data);   
        
    _blocks.close();        
  }
  return result;
}
locktoken_object  object_history_plugin::get_locktoken_object_history(   object_id_type id  )const
{   
  fc::path  sfile = my->get_account_object_path(id);
  locktoken_object result;
  if (!fc::exists(sfile))
  {
    FC_ASSERT(false,"file ${a} ",("a",sfile.to_native_ansi_path()));
    return result;
  }
  std::fstream _blocks;         
  _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::in );
  if(_blocks.is_open())
  {
    _blocks.seekg (0, std::ios::end); 
    long size = _blocks.tellg();  
    _blocks.seekg (0, std::ios::beg); 
    vector<char> data( size);
    _blocks.read( data.data(), size );
    result = fc::raw::unpack<locktoken_object>( data);           
    _blocks.close();        
  }
  return result;

}
worker_object  object_history_plugin::get_worker_object_history(   object_id_type id  )const
{  
  fc::path  sfile = my->get_account_object_path(id);
  worker_object result;
  if (!fc::exists(sfile))
  {
    FC_ASSERT(false,"file ${a} ",("a",sfile.to_native_ansi_path()));
    return result;
  }

  std::fstream _blocks;         
  _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::in );
  if(_blocks.is_open())
  {
    _blocks.seekg (0, std::ios::end); 
    long size = _blocks.tellg();  
    _blocks.seekg (0, std::ios::beg); 
    vector<char> data( size);
    _blocks.read( data.data(), size );
    result = fc::raw::unpack<worker_object>( data);   
        
    _blocks.close();        
  }
  return result;
}
proposal_object  object_history_plugin::get_proposal_object_history(   object_id_type id  )const
{  
  fc::path  sfile = my->get_account_object_path(id);
  proposal_object result;
  if (!fc::exists(sfile))
  {
    FC_ASSERT(false,"file ${a} ",("a",sfile.to_native_ansi_path()));
    return result;
  }

  std::fstream _blocks;         
  _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::in );
  if(_blocks.is_open())
  {
    _blocks.seekg (0, std::ios::end); 
    long size = _blocks.tellg();  
    _blocks.seekg (0, std::ios::beg); 
    vector<char> data( size);
    _blocks.read( data.data(), size );
    result = fc::raw::unpack<proposal_object>( data);   
        
    _blocks.close();        
  }
  return result;
}

sell_exchange_object  object_history_plugin::get_sell_exchange_object_history(   object_id_type id  )const
{  
  fc::path  sfile = my->get_account_object_path(id);
  sell_exchange_object result;
  if (!fc::exists(sfile))
  {
    FC_ASSERT(false,"file ${a} ",("a",sfile.to_native_ansi_path()));
    return result;
  }

  std::fstream _blocks;         
  _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::in );
  if(_blocks.is_open())
  {
    _blocks.seekg (0, std::ios::end); 
    long size = _blocks.tellg();  
    _blocks.seekg (0, std::ios::beg); 
    vector<char> data( size);
    _blocks.read( data.data(), size );
    result = fc::raw::unpack<sell_exchange_object>( data);   
        
    _blocks.close();        
  }
  return result;
}
 bool compfile(const string &a, const string &b){
     return a > b;
 }
buy_exchange_object  object_history_plugin::get_buy_exchange_object_history(   object_id_type id  )const
{  
  fc::path  sfile = my->get_account_object_path(id);
  buy_exchange_object result;
  if (!fc::exists(sfile))
  {
    FC_ASSERT(false,"file ${a} ",("a",sfile.to_native_ansi_path()));
    return result;
  }

  std::fstream _blocks;         
  _blocks.open( sfile.generic_string().c_str(), std::fstream::binary | std::fstream::in );
  if(_blocks.is_open())
  {
    _blocks.seekg (0, std::ios::end); 
    long size = _blocks.tellg();  
    _blocks.seekg (0, std::ios::beg); 
    vector<char> data( size);
    _blocks.read( data.data(), size );
    result = fc::raw::unpack<buy_exchange_object>( data);   
        
    _blocks.close();        
  }
  return result;
}

vector<limit_order_object>   object_history_plugin::get_account_limit_history(vector<limit_order_id_type> ids) const
{
  vector<limit_order_object> result;
  for(auto id : ids)
  {
    limit_order_object get = get_limit_order_object_history(id);
    result.push_back(get);
  }
  return result;
}
vector<limit_order_id_type>  object_history_plugin::get_account_limit_history_count(account_id_type account, asset_id_type base, asset_id_type quote, uint64_t ulimit) const
{
  vector<limit_order_id_type> result;
  fc::path sfile = my->get_account_limit_path(account, base, quote, false);
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
    for(auto s: findfile) {
      limit_order_id_type get;
      fc::from_string(s, get);
      result.push_back(get);
      if (result.size() > ulimit)
        break;
    }
  }
  return result;
}


} }
