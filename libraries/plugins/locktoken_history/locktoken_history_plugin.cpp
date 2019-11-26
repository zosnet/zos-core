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

#include <graphene/locktoken_history/locktoken_history_plugin.hpp> 
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/chain/config.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/evaluator.hpp> 
#include <graphene/business/protocol/bitlender_ops.hpp>
#include <fc/thread/thread.hpp>
#include <fc/smart_ref_impl.hpp>
#include <boost/algorithm/string.hpp>

namespace graphene { namespace locktoken_history {
 
namespace detail
{ 
class locktoken_history_plugin_impl
{
   public:
      locktoken_history_plugin_impl(locktoken_history_plugin& _plugin)
      :_self( _plugin ) {}
      virtual ~locktoken_history_plugin_impl();

      void update_locktoken_histories( const signed_block& b );
      void fill_locktoken_remove(const fc::time_point_sec &now, const fill_object_history_operation &o);
      graphene::chain::database &database()
      {
         return _self.database();
      }
      void     init_data_dir( );    
      locktoken_history_plugin &_self;
      fc::path _data_dir_loan;
};
void locktoken_history_plugin_impl::init_data_dir( )
{
  if(_data_dir_loan.string().length()<=0)
  {
   graphene::chain::database& db = database();  
   _data_dir_loan = db.get_data_dir()   / "locktoken_database";   
   if (!fc::exists(_data_dir_loan))
     fc::create_directories(_data_dir_loan);   
  }
}
locktoken_history_plugin_impl::~locktoken_history_plugin_impl()
{} 
void  locktoken_history_plugin_impl::fill_locktoken_remove(const fc::time_point_sec &now,const fill_object_history_operation &o)
{
  try{
    if(o.itype != fill_object_locktoken)
      return;
    auto order  = fc::raw::unpack<locktoken_object>(o.data);   
    const auto &acount_idx = database().get_index_type<account_locktoken_his_index>();
    const auto &by_key_idx = acount_idx.indices().get<by_locktoken_account>();
    auto bucket_itr = by_key_idx.find(order.issuer);
    if (bucket_itr == by_key_idx.end())
    {
      flat_map<asset_id_type, account_locktoken_item> add;
      database().create<account_locktoken_his_object>([&](account_locktoken_his_object &b) {
        b.issuer = order.issuer;        
        for (int l = 0; l < 8;l++) {          
          b.locktoken.push_back(add);
        }
      });
      bucket_itr = by_key_idx.find(order.issuer);   
    }
    if(bucket_itr == by_key_idx.end())
      return;
    const auto &ass = order.locked.asset_id(database());
    const auto &ass_i = order.locked.asset_id(database());
    asset_precision interest = order.interest;
    asset_precision locked = order.locked;
    locked.precision = ass.precision;
    locked.symbol = ass.symbol;
    interest.precision = ass_i.precision;
    interest.symbol = ass_i.symbol;

    database().modify(*bucket_itr, [&](account_locktoken_his_object &b) {  
      auto itr_issue = b.locktoken[order.type].find(order.locked.asset_id);
      if(itr_issue == b.locktoken[order.type].end())
      {
        account_locktoken_item add;
        add.count = 1;
        add.total = locked;
        add.type = order.type;
        add.interest[order.interest.asset_id] = interest;
        b.locktoken[order.type][order.locked.asset_id] = add;
      }
      else
      {
        itr_issue->second.count++;
        itr_issue->second.total.amount += locked.amount;
        itr_issue->second.add_interest(interest);
      }
    });
  }
  catch(...)
  {
  }
 }
void locktoken_history_plugin_impl::update_locktoken_histories( const signed_block& b )
{
  vector<fill_object_history_operation> *paloan = new vector<fill_object_history_operation>;
  try
  {
    init_data_dir();
    graphene::chain::database &db = database();
    const vector<optional<operation_history_object>> &hist = db.get_applied_operations();
    string stime = fc::to_string(b.timestamp.sec_since_epoch());
    for (const optional<operation_history_object> &o_op : hist)
    {
      if (o_op->op.which() == operation::tag<fill_object_history_operation>::value)
      {
        auto &o = o_op->op.get<fill_object_history_operation>();
        if (o.itype == fill_object_locktoken) {
          fill_locktoken_remove(b.timestamp, o);
          paloan->push_back(o);
        }
        continue;
      }
    }
    if (paloan->size() > 0)
    {
      fc::async([paloan, this, stime]() {
        int ifileindex = 1;
        auto itr_order = paloan->begin();
        while (itr_order != paloan->end())
        {
          fill_object_history_operation &index = *itr_order;
          try
          {
            string account_id = index.issuer.to_string();
            fc::path filew = this->_data_dir_loan / account_id;
            if (!fc::exists(filew))
              fc::create_directories(filew);

            if (index.id.type() == locktoken_object_type)
              filew /= "locktoken";
            else
              filew /= fc::to_string(index.id.type());

            if (!fc::exists(filew))
              fc::create_directories(filew);

            filew /= fc::to_string(index.idatatype);  

            if (!fc::exists(filew))
              fc::create_directories(filew);

            fc::path sfile = filew / (stime + "-" + index.src_id.to_string() + "-" + index.id.to_string());

            std::fstream _blocks;
            _blocks.open(sfile.generic_string().c_str(), std::fstream::binary | std::fstream::out | std::fstream::trunc);
            _blocks.write(index.data.data(), index.data.size());
            _blocks.close();

            itr_order++;
            ifileindex++;
          }
          FC_CAPTURE_AND_RETHROW((index.id)(index.issuer))
        }
        delete paloan;
      });
    }
    else
      delete paloan;
  }
  catch (...)
  {
    delete paloan;
  }
}

} // end namespace detail






locktoken_history_plugin::locktoken_history_plugin() :
   my( new detail::locktoken_history_plugin_impl(*this) )
{
}

locktoken_history_plugin::~locktoken_history_plugin()
{
}

std::string locktoken_history_plugin::plugin_name()const
{
   return "locktoken_history";
}

void locktoken_history_plugin::plugin_set_program_options(
   boost::program_options::options_description& cli,
   boost::program_options::options_description& cfg
   )
{
  
}
void locktoken_history_plugin::get_info(fc::mutable_variant_object &result) 
{
  
}
void locktoken_history_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{ try {      
    database().applied_block.connect([this](const signed_block &b) { my->update_locktoken_histories(b); });
    database()._app_status.has_locktoken_history_plugin = true;    
    database().add_index< primary_index< account_locktoken_his_index  > >();   
  } 
  FC_CAPTURE_AND_RETHROW()
}

void locktoken_history_plugin::plugin_startup()
{
}

bool compfile(const string &a, const string &b)
{
  return a > b;
}
uint64_t locktoken_history_plugin::get_locktoken_history_count(account_id_type account, asset_id_type asset_id, uint32_t utype) const
{
  string account_id = account.to_string();
  string asset_str = asset_id.to_string();
  fc::path sfile = my->_data_dir_loan;
  sfile /= account_id;
  sfile /= "locktoken";
  sfile /= fc::to_string(utype);  
  if (!fc::exists(sfile))
    return 0;

  fc::directory_iterator itr(sfile);
  vector<string> findfile;
  for (; itr != fc::directory_iterator(); itr++)
  {
    string current_filename = itr->filename().string();
    if(asset_id == GRAPHENE_NULL_ASSET) findfile.push_back(current_filename);    
    else if(current_filename.find(asset_str)  != std::string::npos)  findfile.push_back(current_filename);    
  }
  return findfile.size();
}
vector<locktoken_object> locktoken_history_plugin::get_locktoken_history(account_id_type account,  asset_id_type asset_id, uint32_t utype, uint64_t ustart, uint64_t ulimit) const
{
  uint64_t index = 0;
  string account_id = account.to_string();
  string asset_str = asset_id.to_string();
  fc::path sfile = my->_data_dir_loan;
  sfile /= account_id;
  sfile /= "locktoken";
  sfile /= fc::to_string(utype);  
  
  vector<locktoken_object> result;
  if (!fc::exists(sfile))
    return result;
  fc::directory_iterator itr(sfile);
  vector<string> findfile;
  for (; itr != fc::directory_iterator(); itr++)
  {  
    string current_filename = itr->filename().string();
    if(asset_id == GRAPHENE_NULL_ASSET) findfile.push_back(current_filename);    
    else if(current_filename.find(asset_str) != std::string::npos)  findfile.push_back(current_filename);    
  }
  if (findfile.size() > 0)
  {
    sort(findfile.begin(), findfile.end(), compfile);
    auto itr_order = findfile.begin();
    while (itr_order != findfile.end() && index < ulimit + ustart)
    {
      index++;
      if (index < ustart + 1)
      {
        itr_order++;
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
        auto vec = fc::raw::unpack<locktoken_object>(data);
        _blocks.close();        
        result.push_back(vec);
      }
      itr_order++;
    }
  }
  return result;
}
} }
