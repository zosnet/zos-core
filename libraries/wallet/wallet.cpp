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
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <list>
#include <time.h>

#include <boost/version.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/range/algorithm/unique.hpp>
#include <boost/range/algorithm/sort.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/algorithm/string.hpp>

#include <fc/git_revision.hpp>
#include <fc/io/fstream.hpp>
#include <fc/io/json.hpp>
#include <fc/io/stdio.hpp>
#include <fc/network/http/websocket.hpp>
#include <fc/rpc/cli.hpp>
#include <fc/rpc/websocket_api.hpp>
#include <fc/crypto/aes.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/thread/mutex.hpp>
#include <fc/thread/scoped_lock.hpp>

#include <graphene/app/api.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <graphene/utilities/git_revision.hpp>
#include <graphene/utilities/key_conversion.hpp>
#include <graphene/utilities/words.hpp>
#include <graphene/wallet/wallet.hpp>
#include <graphene/wallet/api_documentation.hpp>
#include <graphene/wallet/reflect_util.hpp>
#include <graphene/debug_witness/debug_api.hpp>
#include <graphene/chain/protocol/carrier_ops.hpp>

#include <fc/smart_ref_impl.hpp>
#include <boost/thread/thread.hpp>

#ifndef WIN32
# include <sys/types.h>
# include <sys/stat.h>
#endif

#define BRAIN_KEY_WORD_COUNT 16
#define PRO_DELAY            0

enum K_INDEX{
   K_ACT,
   K_OWNER,
   K_MEMO,
   K_AUTHOR
};

namespace graphene { namespace wallet {

namespace detail {


const private_key_type fixed_private_key = fc::ecc::private_key::generate_from_seed(fc::sha256::hash("ZOS chain 2018"));
const public_key_type  fixed_public_key = fixed_private_key.get_public_key();


struct operation_result_printer
{
public:
   explicit operation_result_printer( const wallet_api_impl& w )
      : _wallet(w) {}
   const wallet_api_impl& _wallet;
   typedef std::string result_type;

   std::string operator()(const void_result& x) const;
   std::string operator()(const object_id_type& oid);
   std::string operator()(const asset& a);
};

// BLOCK  TRX  OP  VOP
struct operation_printer
{
private:
   ostream& out;
   const wallet_api_impl& wallet;
   operation_result result;

   std::string fee(const asset& a) const;

public:
   operation_printer( ostream& out, const wallet_api_impl& wallet, const operation_result& r = operation_result() )
      : out(out),
        wallet(wallet),
        result(r)
   {}
   typedef std::string result_type;

   template<typename T>
   std::string operator()(const T& op)const;

   std::string operator()(const transfer_operation& op)const;
   std::string operator()(const transfer_from_blind_operation& op)const;
   std::string operator()(const transfer_to_blind_operation& op)const;
   std::string operator()(const account_create_operation& op)const;
   std::string operator()(const account_update_operation& op)const;
   std::string operator()(const asset_create_operation& op)const;
   
   
};

template<class T>
optional<T> maybe_id( const string& name_or_id )
{
   if( std::isdigit( name_or_id.front() ) )
   {
      try
      {
         return fc::variant(name_or_id, 1).as<T>(1);
      }
      catch (const fc::exception&)
      { // not an ID
      }
   }
   return optional<T>();
}

string address_to_shorthash( const address& addr )
{
   uint32_t x = addr.addr._hash[0];
   static const char hd[] = "0123456789abcdef";
   string result;

   result += hd[(x >> 0x1c) & 0x0f];
   result += hd[(x >> 0x18) & 0x0f];
   result += hd[(x >> 0x14) & 0x0f];
   result += hd[(x >> 0x10) & 0x0f];
   result += hd[(x >> 0x0c) & 0x0f];
   result += hd[(x >> 0x08) & 0x0f];
   result += hd[(x >> 0x04) & 0x0f];
   result += hd[(x        ) & 0x0f];

   return result;
}

fc::ecc::private_key derive_private_key( const std::string& prefix_string,
                                         int sequence_number )
{
   std::string sequence_string = std::to_string(sequence_number);
   fc::sha512 h = fc::sha512::hash(prefix_string + " " + sequence_string);
   fc::ecc::private_key derived_key = fc::ecc::private_key::regenerate(fc::sha256::hash(h));
   return derived_key;
}

string normalize_brain_key( string s )
{
   size_t i = 0, n = s.length();
   std::string result;
   char c;
   result.reserve( n );

   bool preceded_by_whitespace = false;
   bool non_empty = false;
   while( i < n )
   {
      c = s[i++];
      switch( c )
      {
      case ' ':  case '\t': case '\r': case '\n': case '\v': case '\f':
         preceded_by_whitespace = true;
         continue;

      case 'a': c = 'A'; break;
      case 'b': c = 'B'; break;
      case 'c': c = 'C'; break;
      case 'd': c = 'D'; break;
      case 'e': c = 'E'; break;
      case 'f': c = 'F'; break;
      case 'g': c = 'G'; break;
      case 'h': c = 'H'; break;
      case 'i': c = 'I'; break;
      case 'j': c = 'J'; break;
      case 'k': c = 'K'; break;
      case 'l': c = 'L'; break;
      case 'm': c = 'M'; break;
      case 'n': c = 'N'; break;
      case 'o': c = 'O'; break;
      case 'p': c = 'P'; break;
      case 'q': c = 'Q'; break;
      case 'r': c = 'R'; break;
      case 's': c = 'S'; break;
      case 't': c = 'T'; break;
      case 'u': c = 'U'; break;
      case 'v': c = 'V'; break;
      case 'w': c = 'W'; break;
      case 'x': c = 'X'; break;
      case 'y': c = 'Y'; break;
      case 'z': c = 'Z'; break;

      default:
         break;
      }
      if( preceded_by_whitespace && non_empty )
         result.push_back(' ');
      result.push_back(c);
      preceded_by_whitespace = false;
      non_empty = true;
   }
   return result;
}

struct op_prototype_visitor
{
   typedef void result_type;

   int t = 0;
   flat_map< std::string, operation >& name2op;

   op_prototype_visitor(
      int _t,
      flat_map< std::string, operation >& _prototype_ops
      ):t(_t), name2op(_prototype_ops) {}

   template<typename Type>
   result_type operator()( const Type& op )const
   {
      string name = fc::get_typename<Type>::name();
      size_t p = name.rfind(':');
      if( p != string::npos )
         name = name.substr( p+1 );
      name2op[ name ] = Type();
   }
};


class wallet_api_impl
{
public:
   api_documentation method_documentation;
private:
   void claim_registered_account(const account_object& account)
   {
      auto it = _wallet.pending_account_registrations.find( account.name );
      FC_ASSERT( it != _wallet.pending_account_registrations.end() );
      for (const std::string& wif_key : it->second)
         if( !import_key( account.name, wif_key ) )
         {
            // somebody else beat our pending registration, there is
            //    nothing we can do except log it and move on
            elog( "account ${name} registered by someone else first!",
                  ("name", account.name) );
            // might as well remove it from pending regs,
            //    because there is now no way this registration
            //    can become valid (even in the extremely rare
            //    possibility of migrating to a fork where the
            //    name is available, the user can always
            //    manually re-register)
         }
      _wallet.pending_account_registrations.erase( it );
   }

   // after a witness registration succeeds, this saves the private key in the wallet permanently
   //
   void claim_registered_witness(const std::string& witness_name)
   {
      auto iter = _wallet.pending_witness_registrations.find(witness_name);
      FC_ASSERT(iter != _wallet.pending_witness_registrations.end());
      std::string wif_key = iter->second;

      // get the list key id this key is registered with in the chain
      fc::optional<fc::ecc::private_key> witness_private_key = wif_to_key(wif_key);
      FC_ASSERT(witness_private_key);

      auto pub_key = witness_private_key->get_public_key();
      _keys[pub_key] = wif_key;
      _wallet.pending_witness_registrations.erase(iter);
   }

   fc::mutex _resync_mutex;
   void resync()
   {
      fc::scoped_lock<fc::mutex> lock(_resync_mutex);
      // this method is used to update wallet_data annotations
      //   e.g. wallet has been restarted and was not notified
      //   of events while it was down
      //
      // everything that is done "incremental style" when a push
      //   notification is received, should also be done here
      //   "batch style" by querying the blockchain

      if( !_wallet.pending_account_registrations.empty() )
      {
         // make a vector of the account names pending registration
         std::vector<string> pending_account_names = boost::copy_range<std::vector<string> >(boost::adaptors::keys(_wallet.pending_account_registrations));

         // look those up on the blockchain
         std::vector<fc::optional<graphene::chain::account_object >>
               pending_account_objects = _remote_db->lookup_account_names( pending_account_names );

         // if any of them exist, claim them
         for( const fc::optional<graphene::chain::account_object>& optional_account : pending_account_objects )
            if( optional_account )
               claim_registered_account(*optional_account);
      }

      if (!_wallet.pending_witness_registrations.empty())
      {
         // make a vector of the owner accounts for witnesses pending registration
         std::vector<string> pending_witness_names = boost::copy_range<std::vector<string> >(boost::adaptors::keys(_wallet.pending_witness_registrations));

         // look up the owners on the blockchain
         std::vector<fc::optional<graphene::chain::account_object>> owner_account_objects = _remote_db->lookup_account_names(pending_witness_names);

         // if any of them have registered witnesses, claim them
         for( const fc::optional<graphene::chain::account_object>& optional_account : owner_account_objects )
            if (optional_account)
            {
               fc::optional<witness_object> witness_obj = _remote_db->get_witness_by_account(optional_account->id);
               if (witness_obj)
                  claim_registered_witness(optional_account->name);
            }
      }
   }

   void enable_umask_protection()
   {
#ifdef __unix__
      _old_umask = umask( S_IRWXG | S_IRWXO );
#endif
   }

   void disable_umask_protection()
   {
#ifdef __unix__
      umask( _old_umask );
#endif
   }

   void init_prototype_ops()
   {
      operation op;
      for( int t=0; t<op.count(); t++ )
      {
         op.set_which( t );
         op.visit( op_prototype_visitor(t, _prototype_ops) );
      }
      return;
   }

   map<transaction_handle_type, signed_transaction> _builder_transactions;

   // if the user executes the same command twice in quick succession,
   // we might generate the same transaction id, and cause the second
   // transaction to be rejected.  This can be avoided by altering the
   // second transaction slightly (bumping up the expiration time by
   // a second).  Keep track of recent transaction ids we've generated
   // so we can know if we need to do this
   struct recently_generated_transaction_record
   {
      fc::time_point_sec generation_time;
      graphene::chain::transaction_id_type transaction_id;
   };
   struct timestamp_index{};
   typedef boost::multi_index_container<recently_generated_transaction_record,
                                        boost::multi_index::indexed_by<boost::multi_index::hashed_unique<boost::multi_index::member<recently_generated_transaction_record,
                                        graphene::chain::transaction_id_type,
                                        &recently_generated_transaction_record::transaction_id>,
                                        std::hash<graphene::chain::transaction_id_type> >,
                                        boost::multi_index::ordered_non_unique<boost::multi_index::tag<timestamp_index>,
                                        boost::multi_index::member<recently_generated_transaction_record, fc::time_point_sec, &recently_generated_transaction_record::generation_time> > > > recently_generated_transaction_set_type;
   recently_generated_transaction_set_type _recently_generated_transactions;

public:
   wallet_api& self;
   wallet_api_impl( wallet_api& s, const wallet_data& initial_data, fc::api<login_api> rapi )
      : self(s),
        _chain_id(initial_data.chain_id),
        _remote_api(rapi),
        _remote_db(rapi->database()),
        _remote_bitlender(rapi->bitlender()),
        _remote_finance(rapi->finance()),
        _remote_mobile(rapi->mobile()),
        _remote_admin(rapi->admin()),
        _remote_net_broadcast(rapi->network_broadcast()),
        _remote_hist(rapi->history())        
   {
      chain_id_type remote_chain_id = _remote_db->get_chain_id();
      if( remote_chain_id != _chain_id )
      {
         FC_THROW( "Remote server gave us an unexpected chain_id",
            ("remote_chain_id", remote_chain_id)
            ("chain_id", _chain_id) );
      }
      init_prototype_ops();

      _remote_db->set_block_applied_callback( [this](const variant& block_id )
      {
         on_block_applied( block_id );
      } );

      _bautosave = true;      

      _wallet.chain_id = _chain_id;
      _wallet.ws_server = initial_data.ws_server;
      _wallet.ws_user = initial_data.ws_user;
      _wallet.ws_password = initial_data.ws_password;
   }
   virtual ~wallet_api_impl()
   {
      try
      {
         _remote_db->cancel_all_subscriptions();
      }
      catch (const fc::exception& e)
      {
         // Right now the wallet_api has no way of knowing if the connection to the
         // witness has already disconnected (via the witness node exiting first).
         // If it has exited, cancel_all_subscriptsions() will throw and there's
         // nothing we can do about it.
         // dlog("Caught exception ${e} while canceling database subscriptions", ("e", e));
      }
   }

   void encrypt_keys()
   {
      if( !is_locked() )
      {
         plain_keys data;
         data.keys = _keys;
         data.checksum = _checksum;
         auto plain_txt = fc::raw::pack(data);
         _wallet.cipher_keys = fc::aes_encrypt( data.checksum, plain_txt );
      }
   }

   void on_block_applied( const variant& block_id )
   {
      fc::async([this]{resync();}, "Resync after block");
   }

   bool copy_wallet_file( string destination_filename )
   {
      fc::path src_path = get_wallet_filename();
      if( !fc::exists( src_path ) )
         return false;
      fc::path dest_path = destination_filename + _wallet_filename_extension;
      
      fc::path src_parent  = fc::absolute(src_path).parent_path();
      fc::path dest_parent = fc::absolute(dest_path).parent_path() / "wallet";

      int suffix = 0;
      while( fc::exists(dest_parent/dest_path) )
      {
         ++suffix;
         dest_path =destination_filename + "-" + to_string( suffix ) + _wallet_filename_extension;
      }
      wlog( "backing up wallet ${src} to ${dest}",
            ("src", src_path)
            ("dest", dest_path) );

     
      try
      {         
         enable_umask_protection();
         if( !fc::exists( dest_parent ) )
            fc::create_directories( dest_parent );       
         fc::copy( src_parent/src_path, dest_parent/dest_path);
         disable_umask_protection();
      }
      catch(...)
      {
         disable_umask_protection();
         throw;
      }
      return true;
   }

   bool is_locked()const
   {
      return _checksum == fc::sha512();
   }

   template<typename T>
   T get_object(object_id<T::space_id, T::type_id, T> id)const
   {
      auto ob = _remote_db->get_objects({id}).front();
      return ob.template as<T>( GRAPHENE_MAX_NESTED_OBJECTS );
   }
   template<typename T>
   bool   is_same(const T &s1,const T &s2)
   {
    fc::variant result1,result2;
    fc::to_variant( s1, result1, GRAPHENE_NET_MAX_NESTED_OBJECTS );
    fc::to_variant( s2, result2, GRAPHENE_NET_MAX_NESTED_OBJECTS );
    return (fc::json::to_string(result1) == fc::json::to_string(result2));
    
   }

   void set_proposa_fees( proposal_create_operation& prop_op, const fee_schedule& s  )
   {
      for( auto& op : prop_op.proposed_ops )      
         s.set_fee(op.op);      
      
   }


   void set_operation_fees( signed_transaction& tx, const fee_schedule& s  )
   {
      for( auto& op : tx.operations )      
         s.set_fee(op);      
      
   }
 
   

   variant info() const
   {
      auto chain_props   = get_chain_properties();
      auto global_props  = get_global_properties();
      auto dynamic_props = get_dynamic_global_properties();
      variant block_info = _remote_db->get_block_summary();

      fc::mutable_variant_object result;
      result["genesis_time"] = dynamic_props.initial_time.to_iso_string();
      result["head_block_num"] = dynamic_props.head_block_number;
      result["head_block_id"] = fc::variant(dynamic_props.head_block_id, 1);
      result["head_block_time"] = dynamic_props.time.to_iso_string();
      result["head_block_age"] = fc::get_approximate_relative_time_string(dynamic_props.time,
                                                                          time_point_sec(time_point::now()),
                                                                          " old");
      result["next_maintenance_time"] = fc::get_approximate_relative_time_string(dynamic_props.next_maintenance_time);      
      result["chain_id"]      = chain_props.chain_id;
      result["version"]       = graphene::utilities::git_revision_sha;      
      result["participation"] = (100 * dynamic_props.recent_slots_filled.popcount()) / 128.0;
      result["active_witnesses"] = fc::variant(global_props.active_witnesses, GRAPHENE_MAX_NESTED_OBJECTS);
      result["active_committee_members"] = fc::variant(global_props.active_committee_members, GRAPHENE_MAX_NESTED_OBJECTS);
      result["active_budget_members"]    = fc::variant(global_props.active_budget_members, GRAPHENE_MAX_NESTED_OBJECTS);
      result["block_summary"]            = block_info;
      

      return result;
   }

   variant_object about() const
   {
      string client_version( graphene::utilities::git_revision_description );
      size_t pos = client_version.find( '/' );
      if( pos != string::npos && client_version.size() > pos )
         client_version = client_version.substr( pos + 1 );

      string node_version = _remote_api->get_revision_description();
      pos = node_version.find( '/' );
      if( pos != string::npos && node_version.size() > pos )
         node_version = node_version.substr( pos + 1 );     

      fc::mutable_variant_object result;
    
      result["client_version"]           = client_version;      
      result["node_version"]             = node_version;
      result["code_version"]             = _remote_api->get_code_version();
      //result["graphene_revision"]        = graphene::utilities::git_revision_sha;
      //result["graphene_revision_age"]    = fc::get_approximate_relative_time_string( fc::time_point_sec( graphene::utilities::git_revision_unix_timestamp ) );
      //result["fc_revision"]              = fc::git_revision_sha;
      //result["fc_revision_age"]          = fc::get_approximate_relative_time_string( fc::time_point_sec( fc::git_revision_unix_timestamp ) );
      result["compile_date"]             = "compiled on " __DATE__ " at " __TIME__;
      result["boost_version"]            = boost::replace_all_copy(std::string(BOOST_LIB_VERSION), "_", ".");
      result["openssl_version"]          = OPENSSL_VERSION_TEXT;
            

      std::string bitness = boost::lexical_cast<std::string>(8 * sizeof(int*)) + "-bit";
#if defined(__APPLE__)
      std::string os = "osx";
#elif defined(__linux__)
      std::string os = "linux";
#elif defined(_MSC_VER)
      std::string os = "win32";
#else
      std::string os = "other";
#endif
      result["build"] = os + " " + bitness;

      return result;
   }

   chain_property_object get_chain_properties() const
   {
      return _remote_db->get_chain_properties();
   }
   global_property_object get_global_properties() const
   {
      return _remote_db->get_global_properties();
   }    
   dynamic_global_property_object get_dynamic_global_properties() const
   {
      return _remote_db->get_dynamic_global_properties();
   }
   account_object get_account(account_id_type id) const
   {
      auto rec = _remote_db->get_accounts({id}).front();
      FC_ASSERT(rec);
      return *rec;
   }
   void get_bitlender_key(bitlender_key &def_key,const optional<variant_object> key)
   {
       if(key.valid()) 
       {
         fc::reflector<bitlender_key>::visit(
            fc::from_variant_visitor<bitlender_key>(  *key, def_key,GRAPHENE_MAX_NESTED_OBJECTS )
        );  
       }
       def_key.validate();
   }

   account_object get_account(string account_name_or_id) const
   {
      FC_ASSERT( account_name_or_id.size() > 0 );

      if( auto id = maybe_id<account_id_type>(account_name_or_id) )
      {
         // It's an ID
         return get_account(*id);
      } else {
         auto rec = _remote_db->lookup_account_names({account_name_or_id}).front();
         FC_ASSERT( rec && rec->name == account_name_or_id, "name not exist ${x}",("x",account_name_or_id));
         return *rec;
      }
   }
   account_id_type get_account_id(string account_name_or_id) const
   {
      return get_account(account_name_or_id).get_id();
   }
   optional<asset_object> find_asset(asset_id_type id)const
   {
      auto rec = _remote_db->get_assets({id}).front();
      if( rec )
         _asset_cache[id] = *rec;
      return rec;
   }
   optional<asset_object> find_asset(string asset_symbol_or_id)const
   {
      FC_ASSERT( asset_symbol_or_id.size() > 0 );

      if( auto id = maybe_id<asset_id_type>(asset_symbol_or_id) )
      {
         // It's an ID
         return find_asset(*id);
      } else {
         // It's a symbol
         auto rec = _remote_db->lookup_asset_symbols({asset_symbol_or_id}).front();
         if( rec )
         {
            if( rec->symbol != asset_symbol_or_id )
               return optional<asset_object>();

            _asset_cache[rec->get_id()] = *rec;
         }
         return rec;
      }
   }
   asset_object get_asset(asset_id_type id)const
   {
      auto opt = find_asset(id);
      FC_ASSERT(opt,"${x}",("x",id));
      return *opt;
   }
   asset_object get_asset(string asset_symbol_or_id)const
   {
      auto opt = find_asset(asset_symbol_or_id);
      FC_ASSERT(opt,"${x}",("x",asset_symbol_or_id));
      return *opt;
   }

   asset_id_type get_asset_id(string asset_symbol_or_id) const
   {
      FC_ASSERT( asset_symbol_or_id.size() > 0 );
      vector<optional<asset_object>> opt_asset;
      if( std::isdigit( asset_symbol_or_id.front() ) )
         return fc::variant(asset_symbol_or_id, 1).as<asset_id_type>( 1 );
      opt_asset = _remote_db->lookup_asset_symbols( {asset_symbol_or_id} );
      FC_ASSERT( (opt_asset.size() > 0) && (opt_asset[0].valid()) );
      return opt_asset[0]->id;
   }

   string                            get_wallet_filename() const
   {
      return _wallet_filename;
   }
   
   fc::ecc::private_key              get_private_key(const public_key_type& id)const
   {
      if(id == fixed_public_key) return fixed_private_key;
      auto it = _keys.find(id);
      FC_ASSERT( it != _keys.end() );

      fc::optional< fc::ecc::private_key > privkey = wif_to_key( it->second );
      FC_ASSERT( privkey );
      return *privkey;
   }

   fc::ecc::private_key get_private_key_for_account(const account_object& account)const
   {
      vector<public_key_type> active_keys = account.active.get_keys();
      if (active_keys.size() != 1)
         FC_THROW("Expecting a simple authority with one active key");
      return get_private_key(active_keys.front());
   }
   
   vector<account_key>   suggest_account_key(string name, string password) const
   {
     try{
         vector<account_key> info;
         FC_ASSERT(password.size() >= 8);
         string a_pass = name + "active" + password;
         string o_pass = name + "owner" + password;
         string m_pass = name + "memo" + password;
         string p_pass = name + "author" + password;         

         fc::ecc::private_key a_privatekey = fc::ecc::private_key::generate_from_seed(fc::sha256::hash(a_pass));
         fc::ecc::private_key o_privatekey = fc::ecc::private_key::generate_from_seed(fc::sha256::hash(o_pass));
         fc::ecc::private_key m_privatekey = fc::ecc::private_key::generate_from_seed(fc::sha256::hash(m_pass));
         fc::ecc::private_key p_privatekey = fc::ecc::private_key::generate_from_seed(fc::sha256::hash(p_pass));

         string a_wif_key = key_to_wif(a_privatekey);
         string o_wif_key = key_to_wif(o_privatekey);
         string m_wif_key = key_to_wif(m_privatekey);
         string p_wif_key = key_to_wif(p_privatekey);

         graphene::chain::public_key_type a_wif_pub_key = a_privatekey.get_public_key();
         graphene::chain::public_key_type o_wif_pub_key = o_privatekey.get_public_key();
         graphene::chain::public_key_type m_wif_pub_key = m_privatekey.get_public_key();
         graphene::chain::public_key_type p_wif_pub_key = p_privatekey.get_public_key();

         //资产密钥
         account_key a;
         a.wif_key = a_wif_key;
         a.pub_key = a_wif_pub_key;
         a.add_ress = address(a_wif_pub_key);
         info.push_back(a);
         //账户密钥
         a.wif_key = o_wif_key;
         a.pub_key = o_wif_pub_key;
         a.add_ress = address(o_wif_pub_key);
         info.push_back(a);
         //memo 密钥
         if(true) {
           a.wif_key = m_wif_key;
           a.pub_key = m_wif_pub_key;
           a.add_ress = address(m_wif_pub_key);
           info.push_back(a);
         } else {
           a.wif_key = a_wif_key;
           a.pub_key = a_wif_pub_key;
           a.add_ress = address(a_wif_pub_key);
           info.push_back(a);
         }
         //author 密码 
         if(true) {
           a.wif_key = p_wif_key;
           a.pub_key = p_wif_pub_key;
           a.add_ress = address(p_wif_pub_key);
           info.push_back(a);
         } else {
           a.wif_key = m_wif_key;
           a.pub_key = m_wif_pub_key;
           a.add_ress = address(m_wif_pub_key);
           info.push_back(a);
         }

         return info;

    } FC_CAPTURE_AND_RETHROW((name)(password)) }

    signed_transaction        set_account_authkey(string issuer,public_key_type authorkey,bool broadcast)
    { try
        {
      FC_ASSERT( !self.is_locked() );
     
      account_object from_account = get_account(issuer); 

      account_authenticate_operation account_update_op;
      account_update_op.issuer    = from_account.id;
      account_update_op.op_type   = 4;
      account_update_op.auth_key = authorkey;

      signed_transaction tx;
      tx.operations.push_back( account_update_op );
      set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast);

  } FC_CAPTURE_AND_RETHROW((issuer)(authorkey)) }
    signed_transaction set_account_pubkey(string issuer,public_key_type ownerkey,public_key_type actkey,public_key_type memokey,public_key_type authkey,bool broadcast)
    {
        try
        {
            FC_ASSERT(!is_locked());
            account_object account = get_account(issuer);
            FC_ASSERT(account.owner.is_simple());
            FC_ASSERT(account.active.is_simple());                        
         
            account_update_operation account_update_op;
            account_update_op.account = account.id;
            account_update_op.new_options = account.options;
            account_update_op.owner  = authority(1, ownerkey, 1);
            account_update_op.active = authority(1, actkey, 1);
            account_update_op.new_options->memo_key = memokey;
            account_update_op.new_options->auth_key = authkey;

            //fc::optional<fc::ecc::private_key> optional_private_key = wif_to_key(wif_key);

            signed_transaction tx;
            tx.operations.push_back(account_update_op);
            
            if(account.auth_data.valid()) {
               string info ="",keyinfo = "";
               if(account.auth_data->info.valid() && account.auth_data->info->from == account.options.auth_key) {
                  try {
                    info = read_memo(*(account.auth_data->info));
                  } catch(...) {}
               }
               if(account.auth_data->key.from == account.options.auth_key) {
                  try {
                    keyinfo = read_memo(account.auth_data->key);
                  } catch(...) {}
               }
               if(info.size() >0 || keyinfo.size() >0) {
                  account_auth  studata;
                  if(keyinfo.size()>0)
                  {
                     memo_data key_memo = account.auth_data->key;
                     studata.key = key_memo;
                     studata.key->from = authkey;         
                     studata.key->set_message(get_private_key(studata.key->from),studata.key->to , keyinfo);
                     FC_ASSERT(studata.key->nonce != unenc_nonce);                         
                  }
                  if(info.size()>0)
                  {
                     memo_data info_memo = *(account.auth_data->info);
                     studata.info = info_memo;         
                     studata.info->from = authkey;
                     studata.info->to = authkey;
                     studata.info->set_message(get_private_key(studata.info->from),studata.info->to , info);
                     FC_ASSERT(studata.info->nonce != unenc_nonce);
                  }
                  account_authenticate_operation account_auth_op;
                  account_auth_op.issuer    = account.id;
                  account_auth_op.op_type   = 5;
                  account_auth_op.auth_data = studata;
                  tx.operations.push_back(account_auth_op);
               }
            }            
            if(account.user_info.valid())     
            {
               string info ="";
               if(account.user_info->from == account.options.auth_key) {
                  try {
                    info = read_memo(*(account.user_info));
                  } catch(...) {}
               }
               if(info.size() >0) {
                  account_auth  studata;

                  memo_data info_memo = *(account.user_info);
                  studata.info = info_memo;         
                  studata.info->from = authkey;
                  studata.info->set_message(get_private_key(studata.info->from),studata.info->to , info);
                  FC_ASSERT(studata.info->nonce != unenc_nonce);                       
                   
                  account_authenticate_operation account_auth_op;
                  account_auth_op.issuer    = account.id;
                  account_auth_op.op_type   = 9;
                  account_auth_op.auth_data = studata;
                  tx.operations.push_back(account_auth_op);
               }
            }  

            
            set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
            tx.validate();

            return sign_transaction(tx, broadcast,true);
       
    } FC_CAPTURE_AND_RETHROW((issuer)(ownerkey)(actkey)(memokey)(authkey)) }

    signed_transaction change_password(string account_name_or_id,string password, bool broadcast)
    {
        try
        {
            FC_ASSERT(!is_locked());
            FC_ASSERT(password.size()>40,"password size must >=40");
            account_object account = get_account(account_name_or_id);
            FC_ASSERT(account.owner.is_simple());
            FC_ASSERT(account.active.is_simple());               
         
            vector<account_key>  a_key= suggest_account_key(account_name_or_id,password);

            _keys[a_key[K_AUTHOR].pub_key] = a_key[K_AUTHOR].wif_key;             

            // account_update_operation account_update_op;
            // account_update_op.account = account.id;
            // account_update_op.new_options = account.options;
            // account_update_op.owner  = authority(1, a_key[K_OWNER].pub_key, 1);
            // account_update_op.active = authority(1, a_key[K_ACT].pub_key, 1);
            // account_update_op.new_options->memo_key = a_key[K_MEMO].pub_key;
            // account_update_op.new_options->auth_key = a_key[K_AUTHOR].pub_key;

            // signed_transaction tx;
            // tx.operations.push_back(account_update_op);
            // set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
            // tx.validate();

            // return sign_transaction(tx, broadcast,true);
            signed_transaction ret = set_account_pubkey(account.name,a_key[K_OWNER].pub_key,a_key[K_ACT].pub_key,a_key[K_MEMO].pub_key,a_key[K_AUTHOR].pub_key,broadcast);
            _keys.erase(a_key[K_AUTHOR].pub_key);
            return ret;

    } FC_CAPTURE_AND_RETHROW((account_name_or_id)(password)) }
    signed_transaction set_account_config(string account_name_or_id,string index,string config, bool broadcast)
   {
        try
        {
            FC_ASSERT(!is_locked());     
            FC_ASSERT(index.size()>0);       
            account_object account = get_account(account_name_or_id);              
          
            account_config_operation account_update_op;            
            account_update_op.issuer         = account.id;
            account_update_op.op_type        = 0;            
            account_update_op.config[index]  = config;   
            if(config.size()<=0)        
              account_update_op.op_type = 1;

            signed_transaction tx;
            tx.operations.push_back(account_update_op);
            set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
            tx.validate();       
            

            return sign_transaction(tx, broadcast);
    } FC_CAPTURE_AND_RETHROW((account_name_or_id)(index)(config)) }
    signed_transaction update_account_flags(string account_name_or_id,uint32_t uproperty,uint32_t mask, bool broadcast)
    {
        try
        {
            FC_ASSERT(!is_locked());            
            account_object account = get_account(account_name_or_id);  
            signed_transaction tx;
            bool callback = false;

            if( mask & account_flag_mask)
            {
              account_authenticate_operation account_update_op;            
              account_update_op.issuer = account.id;
              account_update_op.op_type = 1;
              account_update_op.flags = uproperty;           
              tx.operations.push_back(account_update_op);
            }
            else  if( mask & account_authenticator)
            {
              FC_ASSERT(false);

             /* account_authenticate_operation account_update_op;            
              account_update_op.issuer = account.id;
              account_update_op.op_type = 2;
              account_update_op.auth_flag = (uproperty & account_authenticator) ? true:false; 

              auto now =  _remote_db->get_dynamic_global_properties().time;
              proposal_create_operation prop_op;
              prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
              prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
              prop_op.fee_paying_account = account.id;  
              prop_op.proposed_ops.emplace_back( account_update_op );
              set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
              _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);
          
              tx.operations.push_back(prop_op);*/

              callback = true;
            }
            else  if( mask & account_lock_balance)
            {
              account_authenticate_operation account_update_op;            
              account_update_op.issuer = account.id;
              account_update_op.op_type = 3;
              account_update_op.lock_balance = (uproperty & account_lock_balance) ? true:false;           
              tx.operations.push_back(account_update_op);
            }          

            set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
            tx.validate();

            return sign_transaction(tx, broadcast,callback);
    } FC_CAPTURE_AND_RETHROW((account_name_or_id)(uproperty)(mask)) }
   // imports the private key into the wallet, and associate it in some way (?) with the
   // given account name.
   // @returns true if thatches a current active/owner/memo ke y for the  named
   //          account, false otherwise (but it is stored either way)
   
   bool import_key(string account_name_or_id, string wif_key)
   {
       try{
      fc::optional<fc::ecc::private_key> optional_private_key = wif_to_key(wif_key);
      if (!optional_private_key)
         FC_THROW("Invalid private key");
      graphene::chain::public_key_type wif_pub_key = optional_private_key->get_public_key();

      account_object account = get_account( account_name_or_id );

      // make a list of all current public keys for the named account
      flat_set<public_key_type> all_keys_for_account;
      std::vector<public_key_type> active_keys = account.active.get_keys();
      std::vector<public_key_type> owner_keys = account.owner.get_keys();
      std::copy(active_keys.begin(), active_keys.end(), std::inserter(all_keys_for_account, all_keys_for_account.end()));
      std::copy(owner_keys.begin(), owner_keys.end(), std::inserter(all_keys_for_account, all_keys_for_account.end()));
      all_keys_for_account.insert(account.options.memo_key);
      all_keys_for_account.insert(account.options.auth_key);

      _keys[wif_pub_key] = wif_key;

      _wallet.update_account(account);

      _wallet.extra_keys[account.id].insert(wif_pub_key);

      return all_keys_for_account.find(wif_pub_key) != all_keys_for_account.end();
    } FC_CAPTURE_AND_RETHROW( (account_name_or_id)(wif_key)) }

    vector< signed_transaction > import_balance( string name_or_id, const vector<string>& wif_keys, bool broadcast );
 
   bool load_wallet_file(string wallet_filename = "")
   {
      // TODO:  Merge imported wallet with existing wallet,
      //        instead of replacing it
      if( wallet_filename == "" )
         wallet_filename = _wallet_filename;

      if( ! fc::exists( wallet_filename ) )
         return false;

      _wallet = fc::json::from_file( wallet_filename ).as< wallet_data >( 2 * GRAPHENE_MAX_NESTED_OBJECTS );
      if( _wallet.chain_id != _chain_id )
         FC_THROW( "Wallet chain ID does not match",
            ("wallet.chain_id", _wallet.chain_id)
            ("chain_id", _chain_id) );

      size_t account_pagination = 100;
      vector< account_id_type > account_ids_to_send;
      size_t n = _wallet.my_accounts.size();
      account_ids_to_send.reserve( std::min( account_pagination, n ) );
      auto it = _wallet.my_accounts.begin();

      for( size_t start=0; start<n; start+=account_pagination )
      {
         size_t end = std::min( start+account_pagination, n );
         assert( end > start );
         account_ids_to_send.clear();
         std::vector< account_object > old_accounts;
         for( size_t i=start; i<end; i++ )
         {
            assert( it != _wallet.my_accounts.end() );
            old_accounts.push_back( *it );
            account_ids_to_send.push_back( old_accounts.back().id );
            ++it;
         }
         std::vector< optional< account_object > > accounts = _remote_db->get_accounts(account_ids_to_send);
         // server response should be same length as request
         FC_ASSERT( accounts.size() == account_ids_to_send.size() );
         size_t i = 0;
         for( const optional< account_object >& acct : accounts )
         {
            account_object& old_acct = old_accounts[i];
            if( !acct.valid() )
            {
               elog( "Could not find account ${id} : \"${name}\" does not exist on the chain!", ("id", old_acct.id)("name", old_acct.name) );
               i++;
               continue;
            }
            // this check makes sure the server didn't send results
            // in a different order, or accounts we didn't request
            FC_ASSERT( acct->id == old_acct.id );
            if( fc::json::to_string(*acct) != fc::json::to_string(old_acct) )
            {
               wlog( "Account ${id} : \"${name}\" updated on chain", ("id", acct->id)("name", acct->name) );
            }
            _wallet.update_account( *acct );
            i++;
         }
      }

      return true;
   }
   void save_wallet_file(string wallet_filename = "")
   {
      //
      // Serialize in memory, then save to disk
      //
      // This approach lessens the risk of a partially written wallet
      // if exceptions are thrown in serialization
      //

      encrypt_keys();

      if( wallet_filename == "" )
         wallet_filename = _wallet_filename;

      wlog( "saving wallet to file ${fn}", ("fn", wallet_filename) );

      string data = fc::json::to_pretty_string( _wallet );
      try
      {
         enable_umask_protection();
         //
         // Parentheses on the following declaration fails to compile,
         // due to the Most Vexing Parse.  Thanks, C++
         //
         // http://en.wikipedia.org/wiki/Most_vexing_parse
         //
         fc::ofstream outfile{ fc::path( wallet_filename ) };
         outfile.write( data.c_str(), data.length() );
         outfile.flush();
         outfile.close();
         disable_umask_protection();
      }
      catch(...)
      {
         disable_umask_protection();
         throw;
      }
   }

   transaction_handle_type begin_builder_transaction()
   {
      int trx_handle = _builder_transactions.empty()? 0
                                                    : (--_builder_transactions.end())->first + 1;
      _builder_transactions[trx_handle];
      return trx_handle;
   }
   void add_operation_to_builder_transaction(transaction_handle_type transaction_handle, const operation& op)
   {
      FC_ASSERT(_builder_transactions.count(transaction_handle));
      _builder_transactions[transaction_handle].operations.emplace_back(op);
   }
   void replace_operation_in_builder_transaction(transaction_handle_type handle,
                                                 uint32_t operation_index,
                                                 const operation& new_op)
   {
      FC_ASSERT(_builder_transactions.count(handle));
      signed_transaction& trx = _builder_transactions[handle];
      FC_ASSERT( operation_index < trx.operations.size());
      trx.operations[operation_index] = new_op;
   }
   asset set_fees_on_builder_transaction(transaction_handle_type handle, string fee_asset = GRAPHENE_SYMBOL)
   {
      FC_ASSERT(_builder_transactions.count(handle));

      auto fee_asset_obj = get_asset(fee_asset);
      asset total_fee = fee_asset_obj.amount(0);

      auto gprops = _remote_db->get_global_properties().parameters;
      if( fee_asset_obj.get_id() != GRAPHENE_CORE_ASSET )
      {
         for( auto& op : _builder_transactions[handle].operations )
            total_fee += gprops.current_fees->set_fee( op, fee_asset_obj.options.core_exchange_rate );

         FC_ASSERT((total_fee * fee_asset_obj.options.core_exchange_rate).amount <=
                   get_object<asset_dynamic_data_object>(fee_asset_obj.dynamic_asset_data_id).fee_pool,
                   "Cannot pay fees in ${asset}, as this asset's fee pool is insufficiently funded.",
                   ("asset", fee_asset_obj.symbol));
      } else {
         for( auto& op : _builder_transactions[handle].operations )
            total_fee += gprops.current_fees->set_fee( op );
      }

      return total_fee;
   }
   transaction preview_builder_transaction(transaction_handle_type handle)
   {
      FC_ASSERT(_builder_transactions.count(handle));
      return _builder_transactions[handle];
   }
   signed_transaction sign_builder_transaction(transaction_handle_type transaction_handle, bool broadcast = true)
   {
      FC_ASSERT(_builder_transactions.count(transaction_handle));

      return _builder_transactions[transaction_handle] = sign_transaction(_builder_transactions[transaction_handle], broadcast);
   }
   pair<transaction_id_type,signed_transaction> broadcast_build_transaction(transaction_handle_type transaction_handle)
   {
       try {
           FC_ASSERT(_builder_transactions.count(transaction_handle));
           _remote_net_broadcast->broadcast_transaction(_builder_transactions[transaction_handle]);
       }
       catch (const fc::exception& e) {
           elog("Caught exception while broadcasting tx ${id}:  ${e}", ("id", _builder_transactions[transaction_handle].id().str())("e", e.to_detail_string()));
           throw;
       }
       return std::make_pair(_builder_transactions[transaction_handle].id(),_builder_transactions[transaction_handle]);
   } 
   pair<transaction_id_type,signed_transaction> broadcast_transaction(signed_transaction tx)
   {
       try {
           _remote_net_broadcast->broadcast_transaction(tx);
       }
       catch (const fc::exception& e) {
           elog("Caught exception while broadcasting tx ${id}:  ${e}", ("id", tx.id().str())("e", e.to_detail_string()));
           throw;
       }
       return std::make_pair(tx.id(),tx);
   }

 
   signed_transaction propose_builder_transaction(
      transaction_handle_type handle,
      string account_name_or_id,
      time_point_sec expiration = time_point::now() + fc::minutes(1),
      uint32_t review_period_seconds = 0, bool broadcast = true)
   {
       FC_ASSERT( !self.is_locked() );
      FC_ASSERT(_builder_transactions.count(handle));
      proposal_create_operation op;
      op.fee_paying_account = get_account(account_name_or_id).get_id();
      op.expiration_time = expiration;
      signed_transaction& trx = _builder_transactions[handle];
      std::transform(trx.operations.begin(), trx.operations.end(), std::back_inserter(op.proposed_ops),
                     [](const operation& op) -> op_wrapper { return op; });
      if( review_period_seconds )
         op.review_period_seconds = review_period_seconds;
      trx.operations = {op};
      _remote_db->get_global_properties().parameters.current_fees->set_fee( trx.operations.front() );

      return trx = sign_transaction(trx, broadcast,true);
   }

   void remove_builder_transaction(transaction_handle_type handle)
   {
      _builder_transactions.erase(handle);
   }
   vector<string>  get_account_auth(string account) 
   {   
       try{
          bool binfo = true;
          vector<string> re;
          account_object from_account = get_account(account);
          if (!from_account.auth_data.valid())
             return re;
          if(binfo && from_account.auth_data->info.valid())
             re.push_back(read_memo(*(from_account.auth_data->info)));
         re.push_back(read_memo(from_account.auth_data->key));
         return re;

   } FC_CAPTURE_AND_RETHROW( (account)) }
   
   signed_transaction  set_carrier_can_invest(string issuer,bool bcan,bool broadcast)
   {
      try{
         FC_ASSERT(!is_locked());            
         carrier_object account = get_carrier(issuer);
         account_authenticate_operation account_update_op;
         account_update_op.issuer     = account.carrier_account;
         account_update_op.op_type    = 10;

         uint32_t flags               = account.op_type;

         if(bcan)
            flags |= carrier_op_invest;          
         else  
            flags &= ~carrier_op_invest;          

         account_update_op.flags      = flags;

         signed_transaction tx;
         tx.operations.push_back( account_update_op );
         set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
         tx.validate();
            
         return sign_transaction(tx, broadcast);

        } FC_CAPTURE_AND_RETHROW( (issuer)(bcan)(broadcast) )
   }
   signed_transaction         set_account_need_auth(string issuer,optional<uint32_t> need_auth,optional<vector<vector<string>>> trust_auth,optional<string> def_author, uint32_t type,bool broadcast)
   { try{
         FC_ASSERT(!is_locked());            
         account_object account = get_account(issuer);
         FC_ASSERT(need_auth.valid() || trust_auth.valid() ||def_author.valid());
         FC_ASSERT(type == 7  || type == 8);                   

         account_authenticate_operation account_update_op;
         account_update_op.issuer = account.id;
         account_update_op.op_type = type;
         if(need_auth.valid())
         {
            account_update_op.need_auth  = need_auth;
         }
         if(trust_auth.valid())
         {
            flat_map<asset_id_type,account_id_type> add;
            for(auto const a : *trust_auth)
            {               
               add[get_asset(a[0]).id] = get_account(a[1]).id;         
            }
            account_update_op.trust_auth = add;
         }
         if(def_author.valid())
         {
            account_auth auth_account;
            auth_account.authenticator = get_account(*def_author).id;
            account_update_op.auth_data = auth_account;
         }

         signed_transaction tx;
         tx.operations.push_back( account_update_op );
         set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
         tx.validate();
            
         return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(need_auth)(trust_auth)(broadcast) ) }
    int32_t  is_authenticator(const string account,const uint32_t type,const string symbol,const string author) const
    { try{

         account_object account_obj = get_account(account);
         account_object author_obj  = get_account(author);
         asset_object   symbol_obj  = get_asset(symbol);
          return _remote_db->is_authenticator(account_obj.id,type,symbol_obj.id,author_obj.id);

    } FC_CAPTURE_AND_RETHROW((account)(type)(symbol)(author))  }
    

    signed_transaction        set_auth_payfor(string issuer,string foracc,string forrefer,bool broadcast)
    {
       try{

           FC_ASSERT(!is_locked());           
           fc::optional<asset_object> asset_obj = get_asset("ZOS");
           FC_ASSERT(asset_obj, "Could not find asset matching ZOS");

           account_object account = get_account(issuer);      
            account_authenticate_operation account_update_op;
            account_update_op.issuer = account.id;
            account_update_op.op_type = 11;
            account_update_op.flags = asset_obj->amount_from_string(foracc).amount.value;
            account_update_op.need_auth = asset_obj->amount_from_string(forrefer).amount.value;

            signed_transaction tx;
            tx.operations.push_back( account_update_op );
            set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
            tx.validate();
            
            return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(foracc)(forrefer)(broadcast) ) }
   signed_transaction        set_auth_state(string issuer,string account,uint32_t state,bool broadcast)
   {
      try {
      
      FC_ASSERT( !self.is_locked() );
      
      account_object from_account  = get_account(account);
      account_object issuer_account = get_account(issuer);

      FC_ASSERT((issuer_account.uaccount_property & can_auth_mask) >0);   
      FC_ASSERT(issuer_account.id != from_account.id);

      account_author data;

      data.state   = state & (~author_mask);  
      data.account = from_account.id;      
      data.hash64  = 0;

      FC_ASSERT(data.state < auth_nosetting);

      account_authenticate_operation account_update_op;
      account_update_op.issuer    = issuer_account.id;
      account_update_op.op_type   = 6;
      account_update_op.auth_account = data;

      signed_transaction tx;
      tx.operations.push_back( account_update_op );
      set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast);

   } FC_CAPTURE_AND_RETHROW( (issuer)(account)(state)(broadcast) ) }
   signed_transaction  set_account_info(string account,variant_object info_set,bool broadcast) 
   {
       try {
          account_info info;
          account_info info_org;
          try{
             optional<account_info> info_get = get_account_info(account);
             if(info_get.valid())
             {
                info = *info_get;
                info_org = *info_get;
             }
          }catch (...) {}
 
          fc::reflector<account_info>::visit(
            fc::from_variant_visitor<account_info>(  info_set, info,GRAPHENE_MAX_NESTED_OBJECTS )
           );    

        FC_ASSERT(!is_same(info, info_org), "no info changed");

        string memo = fc::json::to_string(info);
        account_auth a_data;
        a_data.info = sign_memo_auth(account, "admin-notify", memo);
        account_authenticate_operation account_update_op;
        account_update_op.issuer    = get_account(account).id;
        account_update_op.op_type   = 9;
        account_update_op.auth_data = a_data;

        signed_transaction tx;
        tx.operations.push_back( account_update_op );
        set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
        tx.validate();

        return sign_transaction(tx, broadcast);

   } FC_CAPTURE_AND_RETHROW( (account)(info_set)(broadcast) ) }
   void   get_admin_config(string admin_name,string loan_symbol,string invest_symbol)
   {
      try {
      const auto &admin  = get_account(admin_name);
      const auto &loan   = get_asset(loan_symbol);
      optional<carrier_cfg>    carrier_list = _remote_admin->get_carrier(admin.id,loan.id,optional<bitlender_key>());
      if(carrier_list.valid()) {idump((*carrier_list));}      
      if(invest_symbol.size()>0 && carrier_list.valid())
      {
         const auto &invest = get_asset(invest_symbol);
         optional<collateral_cfg> min_coll = _remote_admin->get_collaboration(carrier_list->investaccount,loan.id,invest.id, optional<bitlender_key>());
         if(min_coll.valid()) { idump((*min_coll));}
      }
      vector<gateway_cfg> def_gateway   = _remote_admin->get_gateway(admin.id,loan.id); 
      idump((def_gateway));

      flat_map<asset_cfg,vector<asset_cfg>>  asset_list = _remote_admin->get_lending_asset(admin.id, optional<bitlender_key>());
      idump((asset_list));
      
   } FC_CAPTURE_AND_RETHROW( (admin_name)(loan_symbol)(invest_symbol)) }   
   vector<string>   get_account_config(string account)
   {
      try {
         account_info info;
         vector<string> index;
         const auto &acc = get_account(account);          
         return _remote_db->get_account_config(acc.id, index);
   } FC_CAPTURE_AND_RETHROW( (account)) }
   optional<account_info>    get_account_info(string account) 
   {
      try {
         account_info info;
         const auto &acc = get_account(account);
         if(!acc.user_info.valid())
            return  optional<account_info>();
         string s_info = read_memo(*acc.user_info);
         if(s_info.size()<5)
            return  optional<account_info>();
         info = fc::json::from_string(s_info).as<account_info>(10);         
         return info;
   } FC_CAPTURE_AND_RETHROW( (account)) }
   signed_transaction  set_account_auth(string account,string to,string info,string key,bool broadcast) 
   {
       try {
      
      FC_ASSERT( !self.is_locked() );
      if(to.length()<=0) to = account;
      account_object from_account = get_account(account);
      account_object to_account = get_account(to);
      
      FC_ASSERT(key.size() > 0 || info.size() > 0);

      account_auth  studata;
      if(key.size()>0)
      {
         memo_data key_memo;
         studata.key = key_memo;
         studata.key->from = from_account.options.auth_key;
         studata.key->to = to_account.options.auth_key;
         if (account != to)
            studata.authenticator = to_account.id;
         studata.key->set_message(get_private_key(studata.key->from),
                                  studata.key->to , key);
         FC_ASSERT(studata.key->nonce != unenc_nonce);                         
      }
      if(info.size()>0)
      {
         memo_data key_memo;
         studata.info = key_memo;
         studata.info->from = from_account.options.auth_key;
         studata.info->to = from_account.options.auth_key; //发送给自己
         if (account != to)
            studata.authenticator = to_account.id;
         studata.info->set_message(get_private_key(studata.info->from),
                                  studata.info->to , info);
         FC_ASSERT(studata.info->nonce != unenc_nonce);
      }

      account_authenticate_operation account_update_op;
      account_update_op.issuer    = from_account.id;
      account_update_op.op_type   = 5;
      account_update_op.auth_data = studata;

      signed_transaction tx;
      tx.operations.push_back( account_update_op );
      set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast);

   } FC_CAPTURE_AND_RETHROW( (account)(to)(info)(key)(broadcast) ) }
    

   signed_transaction propose_register_account(string issuer, string name,
                                                public_key_type owner,
                                                public_key_type active,
                                                public_key_type memo,
                                                public_key_type author,
                                                string  registrar_account,
                                                string  referrer_account,
                                                uint32_t referrer_percent,
                                                bool broadcast)
     { try {
      FC_ASSERT( !self.is_locked());
      FC_ASSERT( is_valid_name(name));
      FC_ASSERT( !is_cheap_name(name));   
      FC_ASSERT(name.size()>=3) ;      

      account_create_operation account_create_op;
              
      FC_ASSERT( referrer_percent <= 100 );      

      account_object registrar_account_object = this->get_account( registrar_account );
      FC_ASSERT( registrar_account_object.is_lifetime_member() || !is_cheap_name(name));

      FC_ASSERT((registrar_account_object.uaccount_property & account_propose_register) == account_propose_register);

      account_id_type registrar_account_id = registrar_account_object.id;

      account_object referrer_account_object =  this->get_account( referrer_account );
      account_create_op.referrer = referrer_account_object.id;
      account_create_op.referrer_percent = uint16_t( referrer_percent * GRAPHENE_1_PERCENT );
      account_create_op.registrar = registrar_account_id;
      account_create_op.name = name;
      account_create_op.owner = authority(1, owner, 1);
      account_create_op.active = authority(1, active, 1);
      account_create_op.options.memo_key = memo;
      account_create_op.options.auth_key = author ;

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      //prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = get_account(issuer).id;
      prop_op.proposed_ops.emplace_back( account_create_op);
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);
    
      signed_transaction tx;
      tx.operations.push_back(prop_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();

      return sign_transaction(tx, broadcast,true); 

   } FC_CAPTURE_AND_RETHROW((issuer)(name)(owner)(active)(registrar_account)(referrer_account)(referrer_percent)(broadcast) ) }

  
 
   fc::variant broadcast_transaction_synchronous(const signed_transaction& trx)
   {    
      return _remote_net_broadcast->broadcast_transaction_synchronous(30, trx );
   }



   signed_transaction register_account(string name,
                                       public_key_type owner,
                                       public_key_type active,
                                       public_key_type memo,
                                       public_key_type author,
                                       string  registrar_account,
                                       string  referrer_account,
                                       uint32_t referrer_percent,
                                       bool broadcast = false)
   { try {
      FC_ASSERT( !self.is_locked() );
      FC_ASSERT( is_valid_name(name) );
      FC_ASSERT(name.size()>=3) ;    


      account_create_operation account_create_op;

      // #449 referrer_percent is on 0-100 scale, if user has larger
      // number it means their script is using GRAPHENE_100_PERCENT scale
      // instead of 0-100 scale.          
      FC_ASSERT( referrer_percent <= 100 );
      // TODO:  process when pay_from_account is ID

      account_object registrar_account_object =
            this->get_account( registrar_account );
      FC_ASSERT( registrar_account_object.is_lifetime_member() || !is_cheap_name(name));

      account_id_type registrar_account_id = registrar_account_object.id;

      account_object referrer_account_object =  this->get_account( referrer_account );
      account_create_op.referrer = referrer_account_object.id;
      account_create_op.referrer_percent = uint16_t( referrer_percent * GRAPHENE_1_PERCENT );

      account_create_op.registrar = registrar_account_id;
      account_create_op.name = name;
      account_create_op.owner = authority(1, owner, 1);
      account_create_op.active = authority(1, active, 1);
      account_create_op.options.memo_key = memo;
      account_create_op.options.auth_key =  author;

      signed_transaction tx;

      tx.operations.push_back( account_create_op );

      auto current_fees = _remote_db->get_global_properties().parameters.current_fees;
      set_operation_fees( tx, current_fees );

      vector<public_key_type> paying_keys = registrar_account_object.active.get_keys();

      auto dyn_props = get_dynamic_global_properties();
      tx.set_reference_block( dyn_props.head_block_id );
      tx.set_expiration( dyn_props.time + fc::seconds(30) );
      tx.validate();

      for( public_key_type& key : paying_keys )
      {
         auto it = _keys.find(key);
         if( it != _keys.end() )
         {
            fc::optional< fc::ecc::private_key > privkey = wif_to_key( it->second );
            if( !privkey.valid() )
            {
               FC_ASSERT( false, "Malformed private key in _keys" );
            }
            tx.sign( *privkey, _chain_id );
         }
      }

      if( broadcast )
          broadcast_transaction_synchronous(tx );
      return tx;
   } FC_CAPTURE_AND_RETHROW( (name)(owner)(active)(registrar_account)(referrer_account)(referrer_percent)(broadcast) ) }


   signed_transaction upgrade_account(string name, bool broadcast)
   { try {
      FC_ASSERT( !self.is_locked() );
      account_object account_obj = get_account(name);
      FC_ASSERT( !account_obj.is_lifetime_member() );

      signed_transaction tx;
      account_upgrade_operation op;
      op.account_to_upgrade = account_obj.get_id();
      op.upgrade_to_lifetime_member = true;
      tx.operations = {op};
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (name) ) }

  
   // This function generates derived keys starting with index 0 and keeps incrementing
   // the index until it finds a key that isn't registered in the block chain.  To be
   // safer, it continues checking for a few more keys to make sure there wasn't a short gap
   // caused by a failed registration or the like.
   int find_first_unused_derived_key_index(const fc::ecc::private_key& parent_key)
   {
      int first_unused_index = 0;
      int number_of_consecutive_unused_keys = 0;
      for (int key_index = 0; ; ++key_index)
      {
         fc::ecc::private_key derived_private_key = derive_private_key(key_to_wif(parent_key), key_index);
         graphene::chain::public_key_type derived_public_key = derived_private_key.get_public_key();
         if( _keys.find(derived_public_key) == _keys.end() )
         {
            if (number_of_consecutive_unused_keys)
            {
               ++number_of_consecutive_unused_keys;
               if (number_of_consecutive_unused_keys > 5)
                  return first_unused_index;
            }
            else
            {
               first_unused_index = key_index;
               number_of_consecutive_unused_keys = 1;
            }
         }
         else
         {
            // key_index is used
            first_unused_index = 0;
            number_of_consecutive_unused_keys = 0;
         }
      }
   }

   signed_transaction create_account_with_private_key(fc::ecc::private_key owner_privkey,
                                                      string account_name,
                                                      string registrar_account,
                                                      string referrer_account,
                                                      bool broadcast = false,
                                                      bool save_wallet = true)
   { try {

         FC_ASSERT(account_name.size()>=3) ; 
         FC_ASSERT( is_valid_name(account_name) );   
         int active_key_index = find_first_unused_derived_key_index(owner_privkey);
         fc::ecc::private_key active_privkey = derive_private_key( key_to_wif(owner_privkey), active_key_index);

         int memo_key_index = find_first_unused_derived_key_index(active_privkey);
         fc::ecc::private_key memo_privkey = derive_private_key( key_to_wif(active_privkey), memo_key_index);

         int author_key_index = find_first_unused_derived_key_index(memo_privkey);
         fc::ecc::private_key author_privkey = derive_private_key( key_to_wif(memo_privkey), author_key_index);

         graphene::chain::public_key_type owner_pubkey = owner_privkey.get_public_key();
         graphene::chain::public_key_type active_pubkey = active_privkey.get_public_key();
         graphene::chain::public_key_type memo_pubkey = memo_privkey.get_public_key();
         graphene::chain::public_key_type author_pubkey = author_privkey.get_public_key();

         account_create_operation account_create_op;

         // TODO:  process when pay_from_account is ID

         account_object registrar_account_object = get_account( registrar_account );

         account_id_type registrar_account_id = registrar_account_object.id;

          FC_ASSERT( registrar_account_object.is_lifetime_member() || !is_cheap_name(account_name));


         account_object referrer_account_object = get_account( referrer_account );
         account_create_op.referrer = referrer_account_object.id;
         account_create_op.referrer_percent = referrer_account_object.referrer_rewards_percentage;

         account_create_op.registrar = registrar_account_id;
         account_create_op.name = account_name;
         account_create_op.owner = authority(1, owner_pubkey, 1);
         account_create_op.active = authority(1, active_pubkey, 1);
         account_create_op.options.memo_key = memo_pubkey;
         account_create_op.options.auth_key = author_pubkey;

         // current_fee_schedule()
         // find_account(pay_from_account)

         // account_create_op.fee = account_create_op.calculate_fee(db.current_fee_schedule());

         signed_transaction tx;

         tx.operations.push_back( account_create_op );

         set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);

         vector<public_key_type> paying_keys = registrar_account_object.active.get_keys();

         auto dyn_props = get_dynamic_global_properties();
         tx.set_reference_block( dyn_props.head_block_id );
         tx.set_expiration( dyn_props.time + fc::seconds(30) );
         tx.validate();

         for( public_key_type& key : paying_keys )
         {
            auto it = _keys.find(key);
            if( it != _keys.end() )
            {
               fc::optional< fc::ecc::private_key > privkey = wif_to_key( it->second );
               FC_ASSERT( privkey.valid(), "Malformed private key in _keys" );
               tx.sign( *privkey, _chain_id );
            }
         }

         // we do not insert owner_privkey here because
         //    it is intended to only be used for key recovery
         _wallet.pending_account_registrations[account_name].push_back(key_to_wif( owner_privkey ));
         _wallet.pending_account_registrations[account_name].push_back(key_to_wif( active_privkey ));
         _wallet.pending_account_registrations[account_name].push_back(key_to_wif( memo_privkey ));
         if( save_wallet )
            save_wallet_file();
         if( broadcast )
             broadcast_transaction_synchronous(tx );
         return tx;
   } FC_CAPTURE_AND_RETHROW( (account_name)(registrar_account)(referrer_account)(broadcast) ) }

   signed_transaction account_coupon(string account_name, bool broadcast)
   {
      try {
           FC_ASSERT( !self.is_locked() );
      account_object issuer_account = get_account( account_name );
      account_coupon_operation create_op;
      create_op.issuer = issuer_account.id;
      
      signed_transaction tx;
      tx.operations.push_back( create_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (account_name)(broadcast)) }


   signed_transaction withdraw_exchange_fee(string account_name,string ufee,string symbol, bool broadcast)
   {
      try {
          
      FC_ASSERT( !self.is_locked() );
      asset_object asset_obj = get_asset(symbol);
      asset_object core_obj = get_asset(GRAPHENE_CORE_ASSET);
      account_object issuer_account = get_account( account_name );
      withdraw_exchange_fee_operation create_op;
      create_op.issuer = issuer_account.id;
      create_op.core_amount = core_obj.amount_from_string(ufee).amount;
      create_op.withdraw_asset_id = asset_obj.id;

      signed_transaction tx;
      tx.operations.push_back( create_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (account_name)(ufee)(symbol)(broadcast)) }


   signed_transaction create_account_with_brain_key(string brain_key,
                                                    string account_name,
                                                    string registrar_account,
                                                    string referrer_account,
                                                    bool broadcast = false,
                                                    bool save_wallet = true)
   { try {
      FC_ASSERT( !self.is_locked() );
      FC_ASSERT(brain_key.size() >=40);
      string normalized_brain_key = normalize_brain_key( brain_key );
      // TODO:  scan blockchain for accounts that exist with same brain key
      fc::ecc::private_key owner_privkey = derive_private_key( normalized_brain_key, 0 );
      return create_account_with_private_key(owner_privkey, account_name, registrar_account, referrer_account, broadcast, save_wallet);
   } FC_CAPTURE_AND_RETHROW( (account_name)(registrar_account)(referrer_account) ) }
 
  

    signed_transaction asset_claim_fees(string issuer,
                                   string amount,
                                   string symbol,
                                   bool broadcast)
     { 
       try {
      FC_ASSERT( !self.is_locked() );
      account_object issuer_account = get_account( issuer );
      fc::optional<asset_object> asset_obj = get_asset(symbol);
      FC_ASSERT(asset_obj, "Could not find asset matching ${asset}", ("asset", symbol));    
      

      asset_claim_fees_operation create_op;
      create_op.issuer = issuer_account.id;
      create_op.amount_to_claim = asset_obj->amount_from_string(amount);

      signed_transaction tx;
      tx.operations.push_back( create_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (issuer)(symbol)(amount)(broadcast) ) }                               

   signed_transaction asset_reserve_fees(string issuer,
                                   string amount,
                                   string symbol,
                                   bool broadcast)
     { 
       try {
      FC_ASSERT( !self.is_locked() );
      account_object issuer_account = get_account( issuer );
      fc::optional<asset_object> asset_obj = get_asset(symbol);
      FC_ASSERT(asset_obj, "Could not find asset matching ${asset}", ("asset", symbol));    
      

      asset_reserve_fees_operation create_op;
      create_op.issuer = issuer_account.id;
      create_op.amount_to_reserve = asset_obj->amount_from_string(amount);

      signed_transaction tx;
      tx.operations.push_back( create_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (issuer)(symbol)(amount)(broadcast) ) }     

   signed_transaction proposal_create_asset(string issuer,
                                      string asset_issuer,
                                      string symbol,
                                      uint8_t precision,
                                      asset_options common,
                                      fc::optional<bitasset_options> bitasset_opts,                                                                        
                                      bool broadcast = false)
    { try {
        FC_ASSERT( !self.is_locked() );
      account_object issuer_account = get_account( issuer );
      account_object asset_account = get_account( asset_issuer );
      FC_ASSERT(!find_asset(symbol).valid(), "Asset with that symbol already exists!");

      asset_create_operation create_op;
      create_op.issuer = asset_account.id;
      create_op.symbol = symbol;
      create_op.precision       = precision;
      create_op.common_options  = common;
      create_op.bitasset_opts   = bitasset_opts;     


      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = issuer_account.id;  
      prop_op.proposed_ops.emplace_back( create_op );
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);


      signed_transaction tx;
      tx.operations.push_back( prop_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast ,true);
   } FC_CAPTURE_AND_RETHROW( (issuer)(asset_issuer)(symbol)(precision)(common)(bitasset_opts)(broadcast) ) }                                   

   signed_transaction create_asset(string issuer,
                                   string symbol,
                                   uint8_t precision,
                                   asset_options common,
                                   fc::optional<bitasset_options> bitasset_opts,                                                                                          
                                   bool broadcast = false)
   { try {
       FC_ASSERT( !self.is_locked() );
      account_object issuer_account = get_account( issuer );
      FC_ASSERT(!find_asset(symbol).valid(), "Asset with that symbol already exists!");

      asset_create_operation create_op;
      create_op.issuer = issuer_account.id;
      create_op.symbol = symbol;
      create_op.precision       = precision;
      create_op.common_options  = common;
      create_op.bitasset_opts   = bitasset_opts;     

      signed_transaction tx;
      tx.operations.push_back( create_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (issuer)(symbol)(precision)(common)(bitasset_opts)(broadcast) ) }

   signed_transaction update_asset(string symbol,
                                   optional<string> new_issuer,
                                   const  variant_object& option_values,                                     
                                   bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      optional<asset_object> asset_to_update = find_asset(symbol);
      if (!asset_to_update)
        FC_THROW("No asset with that symbol exists!");    

       asset_options new_options = asset_to_update->options;
      fc::reflector<asset_options>::visit(
         fc::from_variant_visitor<asset_options>(  option_values, new_options,GRAPHENE_MAX_NESTED_OBJECTS )
      );         
      
      asset_update_operation update_op;
      update_op.issuer = asset_to_update->issuer;
      update_op.asset_to_update = asset_to_update->id;
      if (new_issuer.valid()&&new_issuer->size()>0)      
        update_op.new_issuer = get_account(*new_issuer).id;        
      update_op.new_options = new_options;      

      signed_transaction tx;
      tx.operations.push_back( update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (symbol)(new_issuer)(option_values)(broadcast) ) }


   signed_transaction propose_update_asset(string    issuer,string symbol,
                                   optional<string> new_issuer,
                                   const  variant_object& option_values,                                 
                                   bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      optional<asset_object> asset_to_update = find_asset(symbol);
      if (!asset_to_update)
        FC_THROW("No asset with that symbol exists!");    
     

      asset_options new_options = asset_to_update->options;
      fc::reflector<asset_options>::visit(
         fc::from_variant_visitor<asset_options>(  option_values, new_options,GRAPHENE_MAX_NESTED_OBJECTS )
      );      

      if(new_options.core_exchange_rate.is_null())
      {
        new_options.core_exchange_rate.quote.amount = 1;
        new_options.core_exchange_rate.quote.asset_id = asset_to_update->id;
        new_options.core_exchange_rate.base.amount = 1;
        new_options.core_exchange_rate.base.asset_id = asset_id_type(0);
      }

      asset_update_operation update_op;
      update_op.issuer = asset_to_update->issuer;
      update_op.asset_to_update = asset_to_update->id;
      if (new_issuer.valid() && new_issuer->size()>0 )      
        update_op.new_issuer = get_account(*new_issuer).id;      
      update_op.new_options = new_options;      
    

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = get_account(issuer).id;  
      prop_op.proposed_ops.emplace_back( update_op );
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);

      signed_transaction tx;
      tx.operations.push_back( prop_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast ,true);
   } FC_CAPTURE_AND_RETHROW( (issuer)(symbol)(new_issuer)(option_values)(broadcast) ) }


   signed_transaction create_bitlender_option(string   issuer,
                                               string  symbol,              
                                               const   optional<variant_object> key,                               
                                               const   variant_object& option_values,                                                 
                                               bool    broadcast)
  {try {
   FC_ASSERT( !self.is_locked() );
    const auto asset_id = get_asset_id(symbol);  
      fc::optional<bitlender_option_object> option_object = _remote_db->get_bitlender_option(asset_id);
      FC_ASSERT(!option_object.valid(), "exist");
      bitlender_paramers_object paramer = _remote_db->get_bitlender_paramers();

      bitlender_option options;
      fc::reflector<bitlender_option>::visit(
         fc::from_variant_visitor<bitlender_option>(  option_values, options,GRAPHENE_MAX_NESTED_OBJECTS )
        );  

      bitlender_key def_key;
      get_bitlender_key(def_key,key);

      options.platform_id     = GRAPHENE_PLATFORM_ACCOUNT;      
      options.repayment_type  = def_key.get_type();

      bitlender_option_create_operation update_op;
      update_op.issuer   = get_account(issuer).id;     
      update_op.asset_id = get_asset_id(symbol);           
      update_op.sproduct = lender_desc;
      options.validate(paramer.get_option(def_key));      

      update_op.options = options;

     

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = update_op.issuer;
      prop_op.proposed_ops.emplace_back( update_op );
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);
    
      signed_transaction tx;
      tx.operations.push_back(prop_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();

      return sign_transaction(tx, broadcast,true);
  
   } FC_CAPTURE_AND_RETHROW( (issuer)(symbol)(key)(option_values)(broadcast)) }

   signed_transaction update_bitlender_option_author(string    issuer, 
                                              string    symbol,                                               
                                              const     vector<string> authors,
                                              const     uint32_t     weight_threshold,                                                                                                                                                         
                                              bool      broadcast)
    {try {     
      FC_ASSERT( !self.is_locked() );
      const auto asset_id = get_asset_id(symbol);  
      fc::optional<bitlender_option_object> option_object = _remote_db->get_bitlender_option(asset_id);
       FC_ASSERT(option_object.valid(), "exist");

      bitlender_option_author_operation update_op;   
      for( const std::string& name : authors )
        update_op.authors.insert( get_account( name ).id );
     
      update_op.issuer   = get_account(issuer).id;                  
      update_op.option_id = option_object->id;
      update_op.weight_threshold = weight_threshold;     
      FC_ASSERT(update_op.authors.size()>=weight_threshold);

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = update_op.issuer;
      prop_op.proposed_ops.emplace_back( update_op );
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);   
    
      signed_transaction tx;
      tx.operations.push_back(prop_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();

      return sign_transaction(tx, broadcast,true);
  
    } FC_CAPTURE_AND_RETHROW( (issuer)(symbol)(authors)(weight_threshold)(broadcast)) }


     

    signed_transaction bitlender_lend_order(string issuer,    string carrier,  
                                      const optional<variant_object> key_in,                                               
                                      const variant_object& option_values,                                                                                              
                                      bool  broadcast)
    {try {
       FC_ASSERT( !self.is_locked() );
      bitlender_loan_operation update_op;
      bitlender_key  key;
      get_bitlender_key(key, key_in);
      
      fc::reflector<bitlender_loan_operation>::visit(
          fc::from_variant_visitor<bitlender_loan_operation>(option_values, update_op, GRAPHENE_MAX_NESTED_OBJECTS));
      update_op.fee.amount = 1; //防止校验不通过
      update_op.validate();

      update_op.issuer   = get_account(issuer).id; 
      update_op.carrier  = get_account(carrier).id;
      update_op.repayment_type = key.get_type();

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();      
       
      return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(carrier)(key_in)(option_values)(broadcast)) }

    signed_transaction bitlender_invest_order(string issuer,  string carrier,                                            
                                      const variant_object& option_values,                                                                                              
                                      bool  broadcast)
    {try {
       FC_ASSERT( !self.is_locked() );
      bitlender_invest_operation update_op;
      fc::reflector<bitlender_invest_operation>::visit(
         fc::from_variant_visitor<bitlender_invest_operation>(  option_values, update_op,GRAPHENE_MAX_NESTED_OBJECTS )
        );
                
      update_op.loan_id    = get_object(update_op.order_id).issuer;  
      update_op.fee.amount = 1; //防止校验不通过
      update_op.validate();

      update_op.issuer   = get_account(issuer).id; 
      update_op.carrier  = get_account(carrier).id; 
      
      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();      
       
      return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(carrier)(option_values)(broadcast)) }
   
    signed_transaction bitlender_repay_interest(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                              
                                      const   uint32_t uperiod,
                                      bool    broadcast)                                            
    {try {
       FC_ASSERT( !self.is_locked() );
         bitlender_repay_interest_operation update_op;
      
         update_op.fee.amount = 1; //防止校验不通过
         update_op.order_id = id;
         update_op.repay_period = uperiod;
         update_op.issuer = get_account(issuer).id;
         update_op.validate();     

         signed_transaction tx;
         tx.operations.push_back(update_op);
         set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
         tx.validate();      
       
         return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(id)(uperiod)(broadcast)) }   

    signed_transaction bitlender_overdue_interest(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                              
                                      const   uint32_t uperiod,
                                      bool    broadcast)
    {try {
       FC_ASSERT( !self.is_locked() );
      bitlender_overdue_interest_operation update_op;
      
      update_op.fee.amount = 1; //防止校验不通过
      update_op.order_id = id;
      update_op.repay_period = uperiod;
      update_op.issuer = get_account(issuer).id;
      update_op.validate();     

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();      
       
      return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(id)(uperiod)(broadcast)) }                                        
    
   signed_transaction bitlender_recycle_interest(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                              
                                      const   uint32_t uperiod,
                                      bool    broadcast)
    {try {
       FC_ASSERT( !self.is_locked() );
      bitlender_recycle_interest_operation update_op;
      
      update_op.fee.amount = 1; //防止校验不通过
      update_op.order_id = id;
      update_op.repay_period = uperiod;
      update_op.issuer = get_account(issuer).id;
      update_op.validate();     

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();      
       
      return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(id)(uperiod)(broadcast)) }   

    signed_transaction bitlender_repay_principal(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                                                                    
                                      bool    broadcast)
    {try {
       FC_ASSERT( !self.is_locked() );
      bitlender_repay_principal_operation update_op;
      
      update_op.fee.amount = 1; //防止校验不通过
      update_op.order_id = id;
      update_op.issuer   = get_account(issuer).id; 
      update_op.validate();     

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();      
       
      return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(id)(broadcast)) }                                       
    
   signed_transaction bitlender_test_principal(string issuer,                                           
                                      const   object_id_type id,    
                                      uint32_t  utype,
                                      uint32_t  uvalue,                                                                                                                                
                                      bool    broadcast)
    {try {
       FC_ASSERT( !self.is_locked() );
      if(utype == 10000)
      {
          boost::this_thread::sleep(boost::posix_time::milliseconds(uvalue *1000));          
          signed_transaction tx;
          return tx;          
      }
      bitlender_test_operation update_op;
      
      update_op.fee.amount = 1; //防止校验不通过
      update_op.order_id = id;
      update_op.issuer   = get_account(issuer).id;
      update_op.ntype = utype;
      update_op.nvalue = uvalue;
      update_op.validate();

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();      
       
      return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(id)(utype)(uvalue)(broadcast)) }       

    signed_transaction bitlender_pre_repay_principal(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                                                                    
                                      bool    broadcast)
    {try {
       FC_ASSERT( !self.is_locked() );
      bitlender_pre_repay_principal_operation update_op;
      
      update_op.fee.amount = 1; //防止校验不通过
      update_op.order_id = id;
      update_op.issuer   = get_account(issuer).id; 
      update_op.validate();     

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();      
       
      return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(id)(broadcast)) }                                    
      
                                                                                  
    signed_transaction bitlender_overdue_repay_principal(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                                                                    
                                      bool    broadcast)
    {try {
       FC_ASSERT( !self.is_locked() );
      bitlender_overdue_repay_principal_operation update_op;
      
      update_op.fee.amount = 1; //防止校验不通过
      update_op.order_id = id;
      update_op.issuer   = get_account(issuer).id; 
      update_op.validate();     

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();      
       
      return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(id)(broadcast)) }


    signed_transaction bitlender_recycle_principal(string issuer,                                           
                                      const   bitlender_order_id_type id,      
                                      const   string                  pay,                                                                                                                              
                                      bool    broadcast)
    {try {
       FC_ASSERT( !self.is_locked() );
      bitlender_recycle_operation update_op;
      bitlender_order_object loan = get_object(id);
      asset_object asset_object = get_asset(loan.amount_to_loan.asset_id);

      update_op.fee.amount = 1; //防止校验不通过
      update_op.order_id = id;
      update_op.issuer   = get_account(issuer).id;
      update_op.asset_pay = asset_object.amount_from_string(pay).amount;
;
      update_op.validate();

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();      
       
      return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(id)(broadcast)) }

     signed_transaction bitlender_remove_order(string issuer,                                           
                                      const   bitlender_order_id_type id,                                            
                                      bool    broadcast)
    {try {
       FC_ASSERT( !self.is_locked() );
      bitlender_remove_operation update_op;            

      update_op.fee.amount = 1; //防止校验不通过
      update_op.order_id = id;
      update_op.issuer   = get_account(issuer).id;      
;
      update_op.validate();

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();      
       
      return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(id)(broadcast)) }


    signed_transaction bitlender_set_auto_repayment(string issuer,                                           
                                      const   bitlender_order_id_type id,   
                                      const   bool    bset  ,                                                                                                                               
                                      bool    broadcast)
    {try {
       FC_ASSERT( !self.is_locked() );
      bitlender_setautorepayer_operation update_op;
      
      update_op.fee.amount = 1; //防止校验不通过
      update_op.order_id = id;
      update_op.issuer   = get_account(issuer).id; 
      update_op.bset   = bset;
      update_op.validate();     

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();      
       
      return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(id)(broadcast)) }


    signed_transaction bitlender_add_collateral(string issuer,                                              
                                      const   variant_object& option_values,                                                                                              
                                      bool    broadcast)
    {try {
       FC_ASSERT( !self.is_locked() );
      bitlender_add_collateral_operation update_op;
      fc::reflector<bitlender_add_collateral_operation>::visit(
         fc::from_variant_visitor<bitlender_add_collateral_operation>(  option_values, update_op,GRAPHENE_MAX_NESTED_OBJECTS )
        );
      update_op.fee.amount = 1; //防止校验不通过
      update_op.validate();

      update_op.issuer   = get_account(issuer).id; 

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();      
       
      return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(option_values)(broadcast)) }
 
  bitlender_option_object_key get_bitlender_option_key(string symbol,optional<bitlender_key> key)
   { try{
      const auto asset_id = get_asset_id(symbol);  
      fc::optional<bitlender_option_object_key> option_object = _remote_db->get_bitlender_option_key(asset_id,key);
      if(option_object.valid())
           return * option_object;
      FC_THROW("No bitlender_option is registered for id ${id}", ("id", asset_id));
   } FC_CAPTURE_AND_RETHROW( (symbol))}

   bitlender_option_object get_bitlender_option(string symbol)
   { try{
      const auto asset_id = get_asset_id(symbol);  
      fc::optional<bitlender_option_object> option_object = _remote_db->get_bitlender_option(asset_id);
      if(option_object.valid())
           return * option_object;
      FC_THROW("No bitlender_option is registered for id ${id}", ("id", asset_id));
   } FC_CAPTURE_AND_RETHROW( (symbol))}
   issue_fundraise_object    get_issue_fundraise(string issue,string buy)
   { try{
      const auto issue_id = get_asset_id(issue);  
      const auto buy_id = get_asset_id(buy);  
      fc::optional<issue_fundraise_object> option_object = _remote_db->get_issue_fundraise(issue_id,buy_id);
      if(option_object.valid())
           return * option_object;
      FC_THROW("No finance_option is registered for id ${id}", ("id", issue));
   } FC_CAPTURE_AND_RETHROW( (issue)(buy))}


    vector<issue_fundraise_id_type>  list_issue_fundraise(string issue, string buy, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit)
    { try{
        asset_id_type issue_id = asset_id_type(0);
        asset_id_type buy_id = asset_id_type(0);  
        if(ufiletermask &1) issue_id =get_asset(issue).id; 
        if(ufiletermask &2) buy_id   =get_asset(issue).id;
        return _remote_finance->list_issue_fundraise(issue_id,buy_id,ufiletermask,usort,start,limit);

    } FC_CAPTURE_AND_RETHROW( (issue)(buy)(ufiletermask)(usort))}

    vector<issue_fundraise_id_type>  list_my_issue_fundraise(string account, bool bcreate,uint64_t start,uint64_t limit)
    { try{
         return _remote_finance->list_my_issue_fundraise(get_account(account).id,bcreate,start,limit);

     } FC_CAPTURE_AND_RETHROW( (account)(bcreate))}

    vector<buy_fundraise_id_type>  list_buy_fundraise(string issue, string buy, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit)
    { try{
        asset_id_type issue_id = asset_id_type(0);
        asset_id_type buy_id = asset_id_type(0);
        if(ufiletermask &1) issue_id =get_asset(issue).id;
        if(ufiletermask &2) buy_id   =get_asset(issue).id;
        return _remote_finance->list_buy_fundraise(issue_id,buy_id,ufiletermask,usort,start,limit);

    } FC_CAPTURE_AND_RETHROW( (issue)(buy)(ufiletermask)(usort))}

    vector<buy_fundraise_id_type>  list_my_buy_fundraise(string account, bool bowner,uint64_t start,uint64_t limit)
    { try{
         return _remote_finance->list_my_buy_fundraise(get_account(account).id,bowner,start,limit);

     } FC_CAPTURE_AND_RETHROW( (account)(bowner))}

    
     vector<sell_exchange_id_type>  list_sell_exchange(string issue, string buy, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit)
    { try{
        asset_id_type issue_id = asset_id_type(0);
        asset_id_type buy_id = asset_id_type(0);
        if(ufiletermask &1) issue_id =get_asset(issue).id;
        if(ufiletermask &2) buy_id   =get_asset(issue).id;
        return _remote_finance->list_sell_exchange(issue_id,buy_id,ufiletermask,usort,start,limit);

    } FC_CAPTURE_AND_RETHROW( (issue)(buy)(ufiletermask)(usort))}

    vector<sell_exchange_id_type>  list_my_sell_exchange(string account, uint64_t start,uint64_t limit)
    { try{
         return _remote_finance->list_my_sell_exchange(get_account(account).id,start,limit);

     } FC_CAPTURE_AND_RETHROW( (account)(start)(limit))}

    vector<buy_exchange_id_type>  list_buy_exchange(string issue, string buy, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit)
    { try{
        asset_id_type issue_id = asset_id_type(0);
        asset_id_type buy_id = asset_id_type(0);
        if(ufiletermask &1) issue_id =get_asset(issue).id;
        if(ufiletermask &2) buy_id   =get_asset(issue).id;
        return _remote_finance->list_buy_exchange(issue_id,buy_id,ufiletermask,usort,start,limit);

    } FC_CAPTURE_AND_RETHROW( (issue)(buy)(ufiletermask)(usort))}

    vector<buy_exchange_id_type>  list_my_buy_exchange(string account,uint64_t start,uint64_t limit)
    { try{
         return _remote_finance->list_my_buy_exchange(get_account(account).id,start,limit);

     } FC_CAPTURE_AND_RETHROW( (account)(start)(limit))}

   signed_transaction update_bitlender_rate(string    issuer,
                                            string    symbol,
                                            const     optional<variant_object> key_in,   
                                            const     variant_object& changed_rate_add,
                                            const     vector<uint32_t>  changed_rate_remove,
                                            bool      broadcast)
  { try {
       FC_ASSERT( !self.is_locked() );
      const auto asset_id = get_asset_id(symbol);  
      fc::optional<bitlender_option_object> option_object = _remote_db->get_bitlender_option(asset_id);
      FC_ASSERT(option_object.valid());
        bitlender_paramers_object paramer = _remote_db->get_bitlender_paramers();

        bitlender_key key;
        get_bitlender_key(key,key_in);

        auto opt = paramer.get_option(key);
        bitlender_rate_update_operation update_op;

        update_op.issuer = get_account(issuer).id;
        update_op.option_id = option_object->id;
        //update_op.repayment_type = key;

        flat_map<int, fee_parameters> fee_map;
        update_op.interest_rate_add.reserve(opt.max_rate_month);
        for (const auto &item : changed_rate_add)
        {
           const string &key = item.key();
           auto is_numeric = [&key]() -> bool {
              size_t n = key.size();
              for (size_t i = 0; i < n; i++)
              {
                 if (!isdigit(key[i]))
                    return false;
              }
              return true;
           };

           uint32_t which = 0;
           if (is_numeric())
           {
              which = std::stoi(key);
              FC_ASSERT(which > 0 && which <= opt.max_rate_month, " number must in 1 ~ ${id} .", ("id", opt.max_rate_month));
           }
           else
           {
              FC_ASSERT(false, "must be number ${id} .", ("id", key));
           }

           variant_object vo;
           fc::from_variant(item.value(), vo, 1);
           bitlender_rate fp;
           fc::reflector<bitlender_rate>::visit(
               fc::from_variant_visitor<bitlender_rate>(vo, fp, GRAPHENE_MAX_NESTED_OBJECTS));
           update_op.interest_rate_add[which] = fp;  
      }      
      for( const auto& item_remove : changed_rate_remove )
      {
          update_op.interest_rate_remove.push_back(item_remove);
      }

      FC_ASSERT(update_op.interest_rate_add.size() + update_op.interest_rate_remove.size()>0,"no rate setting");       

      bitlender_rate add;
      add.interest_rate = key.get_type();
      update_op.interest_rate_add[btlender_rate_type] = add;  

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();

      return sign_transaction(tx, broadcast);

  } FC_CAPTURE_AND_RETHROW( (issuer)(symbol)(changed_rate_add)(changed_rate_remove)(broadcast) )}

   signed_transaction stop_bitlender_option(string     issuer,
                                              string   symbol,
                                              const    bool stop,
                                              bool     broadcast)
    {try {
         FC_ASSERT( !self.is_locked() );
       bitlender_option options;
       const auto asset_id = get_asset_id(symbol);  
        fc::optional<bitlender_option_object> option_object = _remote_db->get_bitlender_option(asset_id);
        FC_ASSERT(option_object.valid());
        FC_ASSERT(option_object->stop != stop);
        

      bitlender_option_stop_operation update_op;
      update_op.issuer    = get_account(issuer).id;     
      update_op.option_id = option_object->id; 
      update_op.author    = option_object->author;
      update_op.stop      = stop;     
      

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = update_op.issuer;
      prop_op.proposed_ops.emplace_back( update_op );
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);    
    
      signed_transaction tx;
      tx.operations.push_back(prop_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();

      return sign_transaction(tx, broadcast,true);
    } FC_CAPTURE_AND_RETHROW( (issuer)(symbol)(stop)(broadcast)) }

    signed_transaction fee_mode_bitlender_option(string     issuer,
                                              string   symbol,
                                              const    uint32_t mode,
                                              bool     broadcast)
    {try {
         FC_ASSERT( !self.is_locked() );
       bitlender_option options;
       const auto asset_id = get_asset_id(symbol);  
        fc::optional<bitlender_option_object> option_object = _remote_db->get_bitlender_option(asset_id);
        FC_ASSERT(option_object.valid());
        FC_ASSERT(option_object->fee_mode != mode);
        
        FC_ASSERT(mode == fee_invest || mode == fee_loan);

      bitlender_option_fee_mode_operation update_op;
      update_op.issuer    = get_account(issuer).id;     
      update_op.option_id = option_object->id;        
      update_op.fee_mode   = mode;     
      

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = update_op.issuer;
      prop_op.proposed_ops.emplace_back( update_op );
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);    
    
      signed_transaction tx;
      tx.operations.push_back(prop_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();

      return sign_transaction(tx, broadcast,true);
    } FC_CAPTURE_AND_RETHROW( (issuer)(symbol)(mode)(broadcast)) }


   signed_transaction update_bitlender_option(string   issuer,
                                              string   symbol,                                                    
                                              const    optional<variant_object> key_in,                                          
                                              const    variant_object& option_values,
                                              bool     broadcast)
    {try {
         FC_ASSERT( !self.is_locked() );
       bitlender_option options;
       bitlender_key key;
       get_bitlender_key(key,key_in);

       const auto asset_id = get_asset_id(symbol);
       fc::optional<bitlender_option_object> option_object = _remote_db->get_bitlender_option(asset_id);
       if (option_object.valid())
          options = option_object->get_option(key,false);
       else
          FC_THROW("No bitlender_option is registered for id ${id}", ("id", asset_id));

       fc::reflector<bitlender_option>::visit(
         fc::from_variant_visitor<bitlender_option>(  option_values, options,GRAPHENE_MAX_NESTED_OBJECTS )
        );         

      bitlender_paramers_object paramer = _remote_db->get_bitlender_paramers();  
      options.platform_id = GRAPHENE_PLATFORM_ACCOUNT;
      
      options.repayment_type        = key.get_type();

      bitlender_option_update_operation update_op;
      update_op.issuer    = get_account(issuer).id;     
      update_op.option_id = option_object->id; 
      update_op.author    = option_object->author;

      options.validate(paramer.get_option(key));
      update_op.options = options;
      

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = update_op.issuer;
      prop_op.proposed_ops.emplace_back( update_op );
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);    
    
      signed_transaction tx;
      tx.operations.push_back(prop_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();

      return sign_transaction(tx, broadcast,true);

    } FC_CAPTURE_AND_RETHROW( (issuer)(symbol)(key_in)(option_values)(broadcast)) }

    signed_transaction update_bitlender_paramers(string issuer,
                                                 const optional<variant_object> key_in,
                                                 const variant_object &option_values,                                                 
                                                 bool broadcast)
    {try {
         FC_ASSERT( !self.is_locked() );
         bitlender_key key;
         get_bitlender_key(key,key_in);

         bitlender_paramers_object paramer = _remote_db->get_bitlender_paramers();
         bitlender_paramers options = paramer.get_option(key,false);
         fc::reflector<bitlender_paramers>::visit(
             fc::from_variant_visitor<bitlender_paramers>(option_values, options, GRAPHENE_MAX_NESTED_OBJECTS));

         options.validate(key);

         FC_ASSERT(!is_same(options, paramer.get_option(key,false)), "no paramer changed");

         bitlender_paramers_update_operation update_op;
         update_op.issuer = get_account(issuer).id;
         update_op.options = options;
         update_op.repayment_type = key.get_type();

         auto now = _remote_db->get_dynamic_global_properties().time;
         proposal_create_operation prop_op;
         prop_op.expiration_time = fc::time_point_sec(now) - PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
         prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
         prop_op.fee_paying_account = update_op.issuer;
         prop_op.proposed_ops.emplace_back(update_op);
         set_proposa_fees(prop_op, _remote_db->get_global_properties().parameters.current_fees);
         _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op, true);

         signed_transaction tx;
         tx.operations.push_back(prop_op);
         set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
         tx.validate();

         return sign_transaction(tx, broadcast,true);

    } FC_CAPTURE_AND_RETHROW( (issuer)(key_in)(option_values)(broadcast)) }

    signed_transaction update_finance_paramers(string   issuer,
                                              const     variant_object& option_values,
                                              bool     broadcast)
    {try {
         FC_ASSERT( !self.is_locked() );
       finance_paramers_object paramer = _remote_db->get_finance_paramers();    
       finance_paramers options = paramer.options;
       fc::reflector<finance_paramers>::visit(
           fc::from_variant_visitor<finance_paramers>(option_values, options, GRAPHENE_MAX_NESTED_OBJECTS));

       options.validate();

       FC_ASSERT(!is_same(options,paramer.options),"no paramer changed");
       //FC_ASSERT(options.to_variant() != paramer.options.to_variant(),"no paramer changed");

       finance_paramers_update_operation update_op;
       update_op.issuer = get_account(issuer).id;  
       update_op.options = options;

       auto now = _remote_db->get_dynamic_global_properties().time;
       proposal_create_operation prop_op;
       prop_op.expiration_time = fc::time_point_sec(now) - PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
       prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
       prop_op.fee_paying_account = update_op.issuer;
       prop_op.proposed_ops.emplace_back(update_op);
       set_proposa_fees(prop_op, _remote_db->get_global_properties().parameters.current_fees);
       _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op, true);

       signed_transaction tx;
       tx.operations.push_back(prop_op);
       set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
       tx.validate();

       return sign_transaction(tx, broadcast,true);

    } FC_CAPTURE_AND_RETHROW( (issuer)(option_values)(broadcast)) }

   template<typename ExchangeInit>
   static ExchangeInit _create_fundraise_initializer( const variant& fundraise_settings )
   {
      ExchangeInit result;
      from_variant( fundraise_settings, result, GRAPHENE_MAX_NESTED_OBJECTS );
      return result;
   }

   signed_transaction create_issue_fundraise(string    issuer,                                                                          
                                          string    issue_asset,  
                                          string    buy_asset, 
                                          string    pay_to, 
                                          string    type,
                                          const     variant_object& fundraise_settings,                                                                                                                                                                                                       
                                          bool      broadcast)
    {try {
      FC_ASSERT( !self.is_locked() );
      const auto a_base_asset  = get_asset(issue_asset);  
      const auto a_pay_asset   = get_asset(buy_asset); 
      const auto a_pay_to      = get_account(pay_to);  
      const auto a_issuer      = get_account(issuer);

      bool proposal = a_base_asset.issuer != a_issuer.id;

      fc::optional<issue_fundraise_object> option_obj = _remote_db->get_issue_fundraise(a_base_asset.id,a_pay_asset.id);
      FC_ASSERT(!option_obj.valid(),"option exists");
      
      fundraise_type init;       
      if( type == "feed" ||   type.size()<=0)      
         init = _create_fundraise_initializer< fundraise_feed_type >( fundraise_settings );      
      else if( type == "bancor" )
         init = _create_fundraise_initializer< fundraise_bancor_type >( fundraise_settings );
      else if( type == "market" )
         init = _create_fundraise_initializer< fundraise_market_type >( fundraise_settings );
      else  
          FC_ASSERT(false,"type not support ${x}",("x",type));
  
      issue_fundraise_create_operation update_op;
      update_op.issuer                     = a_issuer.id;     
      update_op.issue_asset_id             = a_base_asset.id;                 
      update_op.issue_asset_owner          = a_base_asset.issuer;
      update_op.buy_asset_id               = a_pay_asset.id;
      update_op.fundraise_owner            = a_pay_to.id;         
      update_op.initializer                = init;   

      
      proposal_create_operation prop_op;
      if(proposal)
      {
        auto now =  _remote_db->get_dynamic_global_properties().time;
        prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
        prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
        prop_op.fee_paying_account = a_issuer.id;
        prop_op.proposed_ops.emplace_back( update_op );
        set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
        _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);
      }
     
      signed_transaction tx;
      if(proposal)
        tx.operations.push_back(prop_op);
      else 
        tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();

      return sign_transaction(tx, broadcast,proposal);
  
   } FC_CAPTURE_AND_RETHROW( (issuer)(issue_asset)(buy_asset)(pay_to)(type)(fundraise_settings)(broadcast)) }

   signed_transaction  issue_fundraise_publish_feed(string issuer,
                                         string   issue,
                                         string   buy,  
                                         uint32_t period,
                                         price    feed,
                                         bool broadcast)
   {
       try{
       FC_ASSERT( !self.is_locked() );
       
       const auto a_base_asset  = get_asset(issue);  
       const auto a_pay_asset   = get_asset(buy); 
       const auto a_issuer      = get_account(issuer);          

        fc::optional<issue_fundraise_object>   option_o = _remote_db->get_issue_fundraise(a_base_asset.id,a_pay_asset.id);
        FC_ASSERT(option_o.valid(),"object not exit");         

        issue_fundraise_publish_feed_operation update_op;
        update_op.issuer          = a_issuer.id;
        update_op.fundraise_id    = option_o->id;
        update_op.period          = period;        
        update_op.fundraise_price = feed;        

        signed_transaction tx;
        tx.operations.push_back(update_op);
        set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
        tx.validate();

        return sign_transaction(tx, broadcast);
   } FC_CAPTURE_AND_RETHROW((issuer)(issue)(buy)(feed)(broadcast)) }

   template<typename ExchangeInit>
   static ExchangeInit _update_fundraise_initializer( const variant_object& fundraise_settings ,const ExchangeInit &obj)
   {
      ExchangeInit result = obj;
      fc::reflector<ExchangeInit>::visit(
         fc::from_variant_visitor<ExchangeInit>(  fundraise_settings, result,GRAPHENE_MAX_NESTED_OBJECTS )
        );
      //from_variant( fundraise_settings, result, GRAPHENE_MAX_NESTED_OBJECTS );
      return result;
   }

   signed_transaction update_issue_fundraise(string issuer,                                             
                                               string issue_asset,
                                               string buy_asset,
                                               uint32_t period,
                                               const optional<variant_object> fundraise_settings,
                                               uint32_t op_type,
                                               bool broadcast)
   {
       try
       {
           FC_ASSERT(!self.is_locked());           
           const auto a_base_asset = get_asset(issue_asset);
           const auto a_pay_asset = get_asset(buy_asset);
           const auto a_issuer = get_account(issuer);

           bool proposal = a_base_asset.issuer != a_issuer.id;
           fc::optional<issue_fundraise_object> option_obj = _remote_db->get_issue_fundraise(a_base_asset.id, a_pay_asset.id);

           if (!option_obj.valid())
           {         
               FC_THROW("No finance_option is registered for   ${id1}   ${id2} ", ("id1", a_base_asset.symbol)("id2", a_pay_asset.symbol));
           }
           
          issue_fundraise_update_operation update_op;

        if(fundraise_settings.valid()) 
         {
         switch(option_obj->fundraise_op.which())
          {
              case fundraise_data::tag<fundraise_feed_data>::value:
               update_op.update_op =  _update_fundraise_initializer< fundraise_feed_type >( *fundraise_settings,option_obj->fundraise_op.get<fundraise_feed_data>().get_fundraise_type(period));
               break;
              case fundraise_data::tag<fundraise_bancor_data>::value:
              update_op.update_op =  _update_fundraise_initializer< fundraise_bancor_type >( *fundraise_settings,option_obj->fundraise_op.get<fundraise_bancor_data>().get_fundraise_type(period));
               break;
              case fundraise_data::tag<fundraise_market_data>::value:
              update_op.update_op =  _update_fundraise_initializer< fundraise_market_type >( *fundraise_settings,option_obj->fundraise_op.get<fundraise_market_data>().get_fundraise_type(period));
               break;
              default:
               FC_ASSERT(false);
               break;
          }       
       
        }

            
          
           update_op.issuer            = a_issuer.id;
           update_op.fundraise_id      = option_obj->id;
           update_op.issue_asset_owner = a_base_asset.issuer;
           update_op.issue_asset_id    = option_obj->issue_id;
           update_op.buy_asset_id      = option_obj->buy_id;
           update_op.period            = period;
           update_op.op_type           = op_type;          
            

           proposal_create_operation prop_op;
           if (proposal)
           {
               auto now = _remote_db->get_dynamic_global_properties().time;
               prop_op.expiration_time = fc::time_point_sec(now) - PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
               prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
               prop_op.fee_paying_account = a_issuer.id;
               prop_op.proposed_ops.emplace_back(update_op);
               set_proposa_fees(prop_op, _remote_db->get_global_properties().parameters.current_fees);
               _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op, true);
           }

           signed_transaction tx;
           if (proposal)
               tx.operations.push_back(prop_op);
           else
               tx.operations.push_back(update_op);

           set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
           tx.validate();

           return sign_transaction(tx, broadcast,proposal);
       }
       FC_CAPTURE_AND_RETHROW((issuer)(issue_asset)(buy_asset)(period)(fundraise_settings)(op_type)(broadcast)) }

      signed_transaction remove_issue_fundraise(string issuer,                                             
                                               string issue_asset,
                                               string buy_asset,
                                               bool broadcast)
   {
       try
       {
           FC_ASSERT(!self.is_locked());
           fundraise_option options;
           const auto a_base_asset = get_asset(issue_asset);
           const auto a_pay_asset = get_asset(buy_asset);
           const auto a_issuer = get_account(issuer);

           bool proposal = a_base_asset.issuer != a_issuer.id;
           fc::optional<issue_fundraise_object> option_obj = _remote_db->get_issue_fundraise(a_base_asset.id, a_pay_asset.id);

           FC_ASSERT(option_obj.valid(), "No finance_option is registered for   ${id1}   ${id2} ", ("id1", a_base_asset.symbol)("id2", a_pay_asset.symbol));

           issue_fundraise_remove_operation update_op;
           update_op.issuer = a_issuer.id;
           update_op.fundraise_id = option_obj->id;
           update_op.issue_asset_owner = a_base_asset.issuer;           

           proposal_create_operation prop_op;
           if (proposal)
           {
               auto now = _remote_db->get_dynamic_global_properties().time;
               prop_op.expiration_time = fc::time_point_sec(now) - PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
               prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
               prop_op.fee_paying_account = a_issuer.id;
               prop_op.proposed_ops.emplace_back(update_op);
               set_proposa_fees(prop_op, _remote_db->get_global_properties().parameters.current_fees);
               _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op, true);
           }

           signed_transaction tx;
           if (proposal)
               tx.operations.push_back(prop_op);
           else
               tx.operations.push_back(update_op);

           set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
           tx.validate();

           return sign_transaction(tx, broadcast,proposal);
       }
       FC_CAPTURE_AND_RETHROW((issuer)(issue_asset)(buy_asset)(broadcast)) }



    signed_transaction create_buy_fundraise(string  issuer,
                                        string   issue,
                                        string   buy,  
                                        string   amount,                                              
                                        const   uint32_t bimmediately,                                                                                              
                                        bool    broadcast)
    {try {
        
          FC_ASSERT( !self.is_locked() );
      const auto a_base_asset  = get_asset(issue);  
      const auto a_pay_asset   = get_asset(buy);   
      const auto a_issuer      = get_account(issuer);  
      const auto dynamic_props = get_dynamic_global_properties();

      

      fc::optional<issue_fundraise_object> option_obj = _remote_db->get_issue_fundraise(a_base_asset.id,a_pay_asset.id);
      FC_ASSERT(option_obj.valid(),"option no exists");
 

        buy_fundraise_create_operation update_op;
        update_op.issuer         = a_issuer.id; 
        update_op.fundraise_id   =  option_obj->id;
        update_op.amount         = a_pay_asset.amount_from_string(amount);                           
        update_op.bimmediately   = bimmediately;
        update_op.period         = option_obj->get_period( dynamic_props.time);

        signed_transaction tx;
        tx.operations.push_back(update_op);
        set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
        tx.validate();

        return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(issue)(buy)(amount)(bimmediately)(broadcast)) }  

    signed_transaction enable_buy_fundraise(string  issuer,
                                      buy_fundraise_id_type finance_id,                                        
                                      const   bool benable,                                                                                              
                                      bool    broadcast)
     {try {       
     FC_ASSERT( !self.is_locked() );
      buy_fundraise_enable_operation update_op;
      update_op.issuer         = get_account(issuer).id;     
      update_op.fundraise_id = finance_id;                 
      update_op.benable        = benable;      

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);      
      tx.validate();

      return sign_transaction(tx, broadcast);

    } FC_CAPTURE_AND_RETHROW( (issuer)(finance_id)(benable)(broadcast)) }     

   template<typename ExchangeInit>
   static ExchangeInit _create_exchange_initializer( const variant& exchange_settings )
   {
      ExchangeInit result;
      from_variant( exchange_settings, result, GRAPHENE_MAX_NESTED_OBJECTS );
      return result;
   }

   signed_transaction create_sell_exchange(
                                  string      issuer,        
                                  string      sell_asset,
                                  string      sell_amount,                                  
                                  string      buy_asset,   
                                  string      buy_amount, 
                                  string      type,
                                  variant     exchange_settings,     
                                  bool        broadcast)
   {   
      try{
 FC_ASSERT( !self.is_locked() );
      const auto a_sell_asset  = get_asset(sell_asset);  
      const auto a_buy_asset   = get_asset(buy_asset);     


      exchange_type init;      
 
      if( type == "feed"  ||   type.size()<=0)
         init = _create_exchange_initializer< exchange_feed_type >( exchange_settings );
      else if( type == "bancor" )
         init = _create_exchange_initializer< exchange_bancor_type >( exchange_settings );
      else if( type == "market" )
         init = _create_exchange_initializer< exchange_market_type >( exchange_settings );
      else  
          FC_ASSERT(false,"type not support ${x}",("x",type));

     // FC_ASSERT(url.size() < GRAPHENE_MAX_URL_LENGTH );

      sell_exchange_create_operation op;
      op.issuer      = get_account( issuer ).id;  
      op.sell        = a_sell_asset.amount_from_string(sell_amount);
      op.buy         = a_buy_asset.amount_from_string(buy_amount);
      op.initializer = init;
     

      signed_transaction tx;
      tx.operations.push_back( op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW((issuer)(sell_asset)(sell_amount)(buy_asset)(buy_amount)(type)(exchange_settings)(broadcast)) }

   template<typename ExchangeInit>
   static ExchangeInit _update_exchange_initializer( const variant_object& exchange_settings ,const ExchangeInit &obj)
   {
      ExchangeInit result = obj;
      fc::reflector<ExchangeInit>::visit(
         fc::from_variant_visitor<ExchangeInit>(  exchange_settings, result,GRAPHENE_MAX_NESTED_OBJECTS )
        );     
      //from_variant( exchange_settings, result, GRAPHENE_MAX_NESTED_OBJECTS );
      return result;
   }
   signed_transaction update_sell_exchange(
                                  string                issuer,   
                                  sell_exchange_id_type sell_id,                                           
                                  string                sell_amount,                                                                    
                                  string                buy_amount, 
                                  uint32_t              op_type,
                                  optional<variant_object> exchange_settings,     
                                  bool                  broadcast)
   {   
      try{
        FC_ASSERT( !self.is_locked() );
        const auto &wo = get_object( sell_id);      
      
     // FC_ASSERT(url.size() < GRAPHENE_MAX_URL_LENGTH );

        sell_exchange_update_operation op;
        op.issuer      = get_account( issuer ).id;  
        if(sell_amount.size()>0)
          op.sell      = get_asset(wo.sell.asset_id).amount_from_string(sell_amount);
        if(buy_amount.size()>0)   
          op.buy       = get_asset(wo.buy.asset_id).amount_from_string(buy_amount);
        op.op_type     = op_type;
        op.sell_id     = sell_id;

        if(exchange_settings.valid()) 
        {
          switch(wo.exchange_op.which())
          {
              case exchange_data::tag<exchange_feed_data>::value:
               op.update_op =  _update_exchange_initializer< exchange_feed_type >( *exchange_settings,wo.exchange_op.get<exchange_feed_data>().get_exchange_type());
               break;
              case exchange_data::tag<exchange_bancor_data>::value:
              op.update_op =  _update_exchange_initializer< exchange_bancor_type >( *exchange_settings,wo.exchange_op.get<exchange_bancor_data>().get_exchange_type());
               break;
              case exchange_data::tag<exchange_market_data>::value:
              op.update_op =  _update_exchange_initializer< exchange_market_type >( *exchange_settings,wo.exchange_op.get<exchange_market_data>().get_exchange_type());
               break;
              default:
               FC_ASSERT(false);
               break;
          }
        }

      signed_transaction tx;
      tx.operations.push_back( op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW((issuer)(sell_id)(sell_amount)(buy_amount)(op_type)(exchange_settings)(broadcast)) }

   signed_transaction remove_sell_exchange(
                                  string                issuer,
                                  sell_exchange_id_type sell_id,
                                  bool                  broadcast)
     {   
      try{
          FC_ASSERT( !self.is_locked() );
      const auto a_sell  = get_object(sell_id);      

      sell_exchange_remove_operation op;
      op.issuer      = get_account( issuer ).id;  
      op.sell_id     = sell_id;
      
      signed_transaction tx;
      tx.operations.push_back( op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW((issuer)(sell_id)(broadcast)) }   
  

   signed_transaction create_buy_exchange(
                                  string      issuer,        
                                  string      amount,   
                                  string      asset,                                                                
                                  optional<sell_exchange_id_type> sell_id,   
                                  optional<price>                 price, 
                                  bool        broadcast)
   {   
      try{      
          FC_ASSERT( !self.is_locked() );
      const auto a_buy_asset   = get_asset(asset);

      FC_ASSERT(!(sell_id.valid() && price.valid()));

      buy_exchange_create_operation op;
      op.issuer      = get_account( issuer ).id;       
      op.buy         = a_buy_asset.amount_from_string(amount);
      if(sell_id.valid())
          op.sell_id = *sell_id;
      if(price.valid())
      {
         price->validate();
         op.min_price = *price;
      }

      signed_transaction tx;
      tx.operations.push_back( op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW((issuer)(issuer)(amount)(asset)(sell_id)(price)(broadcast)) }

   signed_transaction update_buy_exchange(
                                  string      issuer,        
                                  string      amount,   
                                  string      asset,                                                                
                                  buy_exchange_id_type buy_id,   
                                  optional<price>      price, 
                                  bool        broadcast)
   {   
      try{      
          FC_ASSERT( !self.is_locked() );
      const auto a_buy_asset   = get_asset(asset);      

      buy_exchange_update_operation op;
      op.issuer      = get_account( issuer ).id;       
      op.buy         = a_buy_asset.amount_from_string(amount);      
      op.buy_id      = buy_id;
      if(price.valid())
          op.min_price = *price;    

      signed_transaction tx;
      tx.operations.push_back( op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW((issuer)(issuer)(amount)(asset)(buy_id)(price)(broadcast)) }



   signed_transaction remove_buy_exchange(
                                  string                issuer,
                                  buy_exchange_id_type  buy_id,
                                  bool                  broadcast)
     {   
      try{
          FC_ASSERT( !self.is_locked() );
      const auto a_sell  = get_object(buy_id);      

      buy_exchange_remove_operation op;
      op.issuer      = get_account( issuer ).id;  
      op.buy_id      = buy_id;
      
      signed_transaction tx;
      tx.operations.push_back( op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW((issuer)(buy_id)(broadcast)) }


   signed_transaction update_bitasset(string symbol,
                                      const  variant_object& option_values,                                       
                                      bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      optional<asset_object> asset_to_update = find_asset(symbol);
      if (!asset_to_update)
        FC_THROW("No asset with that symbol exists!");

      bitasset_options new_options = get_object(*asset_to_update->bitasset_data_id).options;   
      fc::reflector<bitasset_options>::visit(
         fc::from_variant_visitor<bitasset_options>(  option_values, new_options,GRAPHENE_MAX_NESTED_OBJECTS )
      );

      asset_update_bitasset_operation update_op;
      update_op.issuer = asset_to_update->issuer;
      update_op.asset_to_update = asset_to_update->id;
      update_op.new_options = new_options;

      signed_transaction tx;
      tx.operations.push_back( update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (symbol)(option_values)(broadcast) ) }




   signed_transaction propose_update_bitasset(string    issuer,string symbol,
                                      const  variant_object& option_values,                                       
                                      bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      optional<asset_object> asset_to_update = find_asset(symbol);
      if (!asset_to_update)
        FC_THROW("No asset with that symbol exists!");
      FC_ASSERT(asset_to_update->bitasset_data_id);
      bitasset_options new_options = get_object(*asset_to_update->bitasset_data_id).options;
      fc::reflector<bitasset_options>::visit(
         fc::from_variant_visitor<bitasset_options>(  option_values, new_options,GRAPHENE_MAX_NESTED_OBJECTS )
      );

      asset_update_bitasset_operation update_op;
      update_op.issuer = asset_to_update->issuer;
      update_op.asset_to_update = asset_to_update->id;
      update_op.new_options = new_options;

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = get_account(issuer).id;  
      prop_op.proposed_ops.emplace_back( update_op );
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);

      signed_transaction tx;
      tx.operations.push_back( prop_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast ,true);
   } FC_CAPTURE_AND_RETHROW( (issuer)(symbol)(option_values)(broadcast) ) }


   signed_transaction update_asset_gateway(string symbol,
                                      const  vector<string> & gateways,                                       
                                      bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      optional<asset_object> asset_to_update = find_asset(symbol);
      if (!asset_to_update)
        FC_THROW("No asset with that symbol exists!");


      asset_update_gateway_operation update_op;
      update_op.issuer = asset_to_update->issuer;
      update_op.asset_to_update = asset_to_update->id;
      
      for(auto &a : gateways)
      {
         auto acc = get_account(a);
         update_op.whitelist_gateways.push_back(acc.id);
         FC_ASSERT(_remote_db->get_gateway_by_account(acc.id).valid());
      }

      signed_transaction tx;
      tx.operations.push_back( update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (symbol)(gateways)(broadcast) ) }




   signed_transaction propose_update_asset_gateway(string    issuer,string symbol,
                                       const  vector<string> & gateways,                                            
                                      bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      optional<asset_object> asset_to_update = find_asset(symbol);
      if (!asset_to_update)
        FC_THROW("No asset with that symbol exists!");           

      asset_update_gateway_operation update_op;
      update_op.issuer = asset_to_update->issuer;
      update_op.asset_to_update = asset_to_update->id;

      if(update_op.issuer == GRAPHENE_NULL_ACCOUNT)
          update_op.issuer = GRAPHENE_COMMITTEE_ACCOUNT;

      for(auto &a : gateways)
      {
         auto acc = get_account(a);
         update_op.whitelist_gateways.push_back(acc.id);
         FC_ASSERT(_remote_db->get_gateway_by_account(acc.id).valid());
      }

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = get_account(issuer).id;  
      prop_op.proposed_ops.emplace_back( update_op );
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);

      signed_transaction tx;
      tx.operations.push_back( prop_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast ,true);
   } FC_CAPTURE_AND_RETHROW( (issuer)(symbol)(gateways)(broadcast) ) }

   signed_transaction update_asset_feed_producers(string symbol,
                                                  flat_set<string> new_feed_producers,
                                                  bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      optional<asset_object> asset_to_update = find_asset(symbol);
      if (!asset_to_update)
        FC_THROW("No asset with that symbol exists!");

      asset_update_feed_producers_operation update_op;
      update_op.issuer = asset_to_update->issuer;
      update_op.asset_to_update = asset_to_update->id;
      update_op.new_feed_producers.reserve(new_feed_producers.size());
      std::transform(new_feed_producers.begin(), new_feed_producers.end(),
                     std::inserter(update_op.new_feed_producers, update_op.new_feed_producers.end()),
                     [this](const std::string& account_name_or_id){ return get_account_id(account_name_or_id); });

      signed_transaction tx;
      tx.operations.push_back( update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (symbol)(new_feed_producers)(broadcast) ) }

   signed_transaction propose_asset_feed_producers(string saccount,string symbol,
                                                  flat_set<string> new_feed_producers,
                                                  bool broadcast /* = false */)
   { try {
 FC_ASSERT( !self.is_locked() );
      account_object from_account = get_account(saccount);
      optional<asset_object> asset_to_update = find_asset(symbol);
      if (!asset_to_update)
        FC_THROW("No asset with that symbol exists!");

      asset_update_feed_producers_operation update_op;
      update_op.issuer = asset_to_update->issuer;
      update_op.asset_to_update = asset_to_update->id;
      update_op.new_feed_producers.reserve(new_feed_producers.size());
      std::transform(new_feed_producers.begin(), new_feed_producers.end(),
                     std::inserter(update_op.new_feed_producers, update_op.new_feed_producers.end()),
                     [this](const std::string& account_name_or_id){ return get_account_id(account_name_or_id); });



      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = from_account.id;
      prop_op.proposed_ops.emplace_back(update_op);
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);


      signed_transaction tx;
      tx.operations.push_back( prop_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast ,true);
   } FC_CAPTURE_AND_RETHROW(  (saccount)(symbol)(new_feed_producers)(broadcast) ) }


   signed_transaction publish_asset_feed(string publishing_account,
                                         string symbol,
                                         price_feed feed,
                                         bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      optional<asset_object> asset_to_update = find_asset(symbol);
      if (!asset_to_update)
        FC_THROW("No asset with that symbol exists!");

      asset_publish_feed_operation publish_op;
      publish_op.publisher = get_account_id(publishing_account);
      publish_op.asset_id = asset_to_update->id;
      publish_op.feed = feed;
      publish_op.feed.settlement_price.base.asset_id = publish_op.asset_id;
      publish_op.feed.core_exchange_rate.base.asset_id = publish_op.asset_id;

      signed_transaction tx;
      tx.operations.push_back( publish_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (publishing_account)(symbol)(feed)(broadcast) ) }

   signed_transaction propose_bitlender_feed_producers(string saccount, string symbol,
                                                       const  variant_object& option,
                                                       flat_set<string> new_feed_producers,
                                                       bool broadcast /* = false */)
   { try {
 FC_ASSERT( !self.is_locked() );
      account_object from_account = get_account(saccount);
      optional<asset_object> asset_to_update = find_asset(symbol);
      if (!asset_to_update)
        FC_THROW("No asset with that symbol exists!");

     fc::optional<bitlender_option_object> option_object = _remote_db->get_bitlender_option(asset_to_update->id);
       if (!option_object.valid())
        FC_THROW("No asset option with that symbol exists!");   
      
      bitlender_feed_option  new_params = option_object->feed_option;
      fc::reflector<bitlender_feed_option>::visit(
         fc::from_variant_visitor<bitlender_feed_option>( option, new_params, GRAPHENE_MAX_NESTED_OBJECTS )
         );

      bitlender_update_feed_producers_operation update_op;
      update_op.issuer = asset_to_update->issuer;
      update_op.option_id = option_object->id;
      update_op.author = option_object->author;
      update_op.new_feed_option = new_params;
      update_op.new_feed_option.feeders.clear();
      update_op.new_feed_option.feeders.reserve(new_feed_producers.size());
      std::transform(new_feed_producers.begin(), new_feed_producers.end(),
                     std::inserter(update_op.new_feed_option.feeders, update_op.new_feed_option.feeders.end()),
                     [this](const std::string& account_name_or_id){ return get_account_id(account_name_or_id); });


      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = from_account.id;
      prop_op.proposed_ops.emplace_back(update_op);
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);

      signed_transaction tx;
      tx.operations.push_back( prop_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast ,true);
   } FC_CAPTURE_AND_RETHROW(  (saccount)(symbol)(option)(new_feed_producers)(broadcast) ) }


   signed_transaction publish_bitlender_feed(string publishing_account,    
                                         string symbol,                                                                    
                                         price_feed feed,
                                         bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
       fc::optional<bitlender_option_object> option_object = _remote_db->get_bitlender_option(feed.settlement_price.base.asset_id);
       auto const &a = get_asset(symbol);
       if (!option_object.valid())
           FC_THROW("No asset option with that symbol exists!");

       bitlender_publish_feed_operation publish_op;
       publish_op.publisher = get_account_id(publishing_account);
       publish_op.feed = feed;
       publish_op.asset_id = a.id;
       signed_transaction tx;
       tx.operations.push_back(publish_op);
       set_operation_fees(tx, _remote_db->get_global_properties().parameters.current_fees);
       tx.validate();

       return sign_transaction(tx, broadcast);
   } FC_CAPTURE_AND_RETHROW( (publishing_account)(symbol)(feed)(broadcast) ) }

   signed_transaction fund_asset_fee_pool(string from,
                                          string symbol,
                                          string amount,
                                          bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      account_object from_account = get_account(from);
      optional<asset_object> asset_to_fund = find_asset(symbol);
      if (!asset_to_fund)
        FC_THROW("No asset with that symbol exists!");
      asset_object core_asset = get_asset(GRAPHENE_CORE_ASSET);

      asset_fund_fee_pool_operation fund_op;
      fund_op.from_account = from_account.id;
      fund_op.asset_id = asset_to_fund->id;
      fund_op.amount = core_asset.amount_from_string(amount).amount;

      signed_transaction tx;
      tx.operations.push_back( fund_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (from)(symbol)(amount)(broadcast) ) }

   signed_transaction reserve_asset(string from,
                                 string amount,
                                 string symbol,
                                 bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      account_object from_account = get_account(from);
      optional<asset_object> asset_to_reserve = find_asset(symbol);
      if (!asset_to_reserve)
        FC_THROW("No asset with that symbol exists!");

      asset_reserve_operation reserve_op;
      reserve_op.payer = from_account.id;
      reserve_op.amount_to_reserve = asset_to_reserve->amount_from_string(amount);

      signed_transaction tx;
      tx.operations.push_back( reserve_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (from)(amount)(symbol)(broadcast) ) }

   signed_transaction global_settle_asset(string symbol,
                                          price settle_price,
                                          bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      optional<asset_object> asset_to_settle = find_asset(symbol);
      if (!asset_to_settle)
        FC_THROW("No asset with that symbol exists!");

      asset_global_settle_operation settle_op;
      settle_op.issuer = asset_to_settle->issuer;
      settle_op.asset_to_settle = asset_to_settle->id;
      settle_op.settle_price = settle_price;

      signed_transaction tx;
      tx.operations.push_back( settle_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (symbol)(settle_price)(broadcast) ) }

   signed_transaction settle_asset(string account_to_settle,
                                   string amount_to_settle,
                                   string symbol,
                                   bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      optional<asset_object> asset_to_settle = find_asset(symbol);
      if (!asset_to_settle)
        FC_THROW("No asset with that symbol exists!");

      asset_settle_operation settle_op;
      settle_op.account = get_account_id(account_to_settle);
      settle_op.amount = asset_to_settle->amount_from_string(amount_to_settle);

      signed_transaction tx;
      tx.operations.push_back( settle_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (account_to_settle)(amount_to_settle)(symbol)(broadcast) ) }

   signed_transaction bid_collateral(string bidder_name,
                                     string debt_amount, string debt_symbol,
                                     string additional_collateral,
                                     bool broadcast )
   { try {
       FC_ASSERT( !self.is_locked() );
      optional<asset_object> debt_asset = find_asset(debt_symbol);
      if (!debt_asset)
        FC_THROW("No asset with that symbol exists!");
      FC_ASSERT(debt_asset->bitasset_data_id);
      const asset_object& collateral = get_asset(get_object(*debt_asset->bitasset_data_id).options.short_backing_asset);

      bid_collateral_operation op;
      op.bidder = get_account_id(bidder_name);
      op.debt_covered = debt_asset->amount_from_string(debt_amount);
      op.additional_collateral = collateral.amount_from_string(additional_collateral);

      signed_transaction tx;
      tx.operations.push_back( op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (bidder_name)(debt_amount)(debt_symbol)(additional_collateral)(broadcast) ) }

   signed_transaction whitelist_account(string authorizing_account,
                                        string account_to_list,
                                        account_whitelist_operation::account_listing new_listing_status,
                                        bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      account_whitelist_operation whitelist_op;
      whitelist_op.authorizing_account = get_account_id(authorizing_account);
      whitelist_op.account_to_list = get_account_id(account_to_list);
      whitelist_op.new_listing = new_listing_status;

      signed_transaction tx;
      tx.operations.push_back( whitelist_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (authorizing_account)(account_to_list)(new_listing_status)(broadcast) ) }

   signed_transaction create_committee_member(string owner_account, string url,string memo,
                                      bool broadcast /* = false */)
   { try {
  FC_ASSERT( !self.is_locked() );
      committee_member_create_operation committee_member_create_op;
      committee_member_create_op.committee_member_account = get_account_id(owner_account);
      committee_member_create_op.url = url;
      committee_member_create_op.memo = memo;
      if (_remote_db->get_committee_member_by_account(committee_member_create_op.committee_member_account))
         FC_THROW("Account ${owner_account} is already a committee_member", ("owner_account", owner_account));

      signed_transaction tx;
      tx.operations.push_back( committee_member_create_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (owner_account)(url)(memo)(broadcast) ) }

   signed_transaction update_committee_member(string committee_member_name,
                                     string url,optional<string> memo,                                     
                                     bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      committee_member_object committee_member = get_committee_member(committee_member_name);
      account_object committee_account = get_account( committee_member.committee_member_account );

      committee_member_update_operation committee_member_update_op;
      committee_member_update_op.committee_member = committee_member.id;
      committee_member_update_op.committee_member_account = committee_account.id;
      if( url != "" )
         committee_member_update_op.new_url = url;      
      if( memo.valid())
         committee_member_update_op.new_memo = *memo;      

      signed_transaction tx;
      tx.operations.push_back( committee_member_update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (committee_member_name)(url)(memo)(broadcast) ) }


    signed_transaction create_budget_member(string owner_account, string url,string memo,
                                      bool broadcast /* = false */)
   { try {
  FC_ASSERT( !self.is_locked() );
      budget_member_create_operation budget_member_create_op;
      budget_member_create_op.budget_member_account = get_account_id(owner_account);
      budget_member_create_op.url = url;
      budget_member_create_op.memo = memo;
      if (_remote_db->get_budget_member_by_account(budget_member_create_op.budget_member_account))
         FC_THROW("Account ${owner_account} is already a budget_member", ("owner_account", owner_account));

      signed_transaction tx;
      tx.operations.push_back( budget_member_create_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (owner_account)(url)(memo)(broadcast) ) }

   signed_transaction update_budget_member(string budget_member_name,
                                     string url,        
                                     optional<string> memo,                             
                                     bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      budget_member_object budget_member = get_budget_member(budget_member_name);
      account_object budget_account = get_account( budget_member.budget_member_account );

      budget_member_update_operation budget_member_update_op;
      budget_member_update_op.budget_member = budget_member.id;
      budget_member_update_op.budget_member_account = budget_account.id;
      if( url != "" )
         budget_member_update_op.new_url = url;      
      if(memo.valid())   
         budget_member_update_op.new_memo = memo;      

      signed_transaction tx;
      tx.operations.push_back( budget_member_update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (budget_member_name)(url)(memo)(broadcast) ) }


   signed_transaction create_gateway(string owner_account,
                                     string url,string memo,
                                     vector<string> a_asset,
                                     bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      account_object gateway_account = get_account(owner_account);     
      gateway_create_operation gateway_create_op;
      gateway_create_op.gateway_account = gateway_account.id;      
      gateway_create_op.url = url;
      gateway_create_op.memo = memo;
      for(auto &a:a_asset)
        gateway_create_op.allowed_asset.insert(get_asset(a).id);       

      if (_remote_db->get_gateway_by_account(gateway_create_op.gateway_account))
         FC_THROW("Account ${owner_account} is already a gateway", ("owner_account", owner_account));

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = gateway_create_op.gateway_account;
      prop_op.proposed_ops.emplace_back(gateway_create_op);
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);


      signed_transaction tx;
      tx.operations.push_back( prop_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();       

      return sign_transaction( tx, broadcast,true );
   } FC_CAPTURE_AND_RETHROW( (owner_account)(url)(memo)(broadcast) ) }

   signed_transaction update_gateway(string gateway_name,
                                     string url,optional<string> memo,        
                                     optional<vector<string>> a_asset,                             
                                     bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      gateway_object gateway = get_gateway(gateway_name);
      account_object gateway_account = get_account( gateway.gateway_account );

      gateway_update_operation gateway_update_op;
      gateway_update_op.gateway = gateway.id;
      gateway_update_op.gateway_account = gateway_account.id;
      if( url != "" )      
         gateway_update_op.new_url = url; 
      if( memo.valid() && memo->size()>0)
         gateway_update_op.new_memo = memo;    

      flat_set<asset_id_type> allowed_asset;        
      if(a_asset.valid())
      {
        for(auto &a:*a_asset)
           allowed_asset.insert(get_asset(a).id); 
        gateway_update_op.allowed_asset = allowed_asset;   
      }   

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = gateway_account.id;
      prop_op.proposed_ops.emplace_back(gateway_update_op);
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);


      signed_transaction tx;
      tx.operations.push_back( prop_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast ,true);
   } FC_CAPTURE_AND_RETHROW( (gateway_name)(url)(memo)(broadcast) ) }

   signed_transaction create_carrier(string owner_account,
                                     string url,string memo,
                                     bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      account_object carrier_account = get_account(owner_account);       

      carrier_create_operation carrier_create_op;
      carrier_create_op.carrier_account = carrier_account.id;      
      carrier_create_op.url       = url;
      carrier_create_op.memo      = memo;
      

      if (_remote_db->get_carrier_by_account(carrier_create_op.carrier_account))
         FC_THROW("Account ${owner_account} is already a carrier", ("owner_account", owner_account));

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = carrier_account.id;
      prop_op.proposed_ops.emplace_back(carrier_create_op);
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);

      signed_transaction tx;
      tx.operations.push_back( prop_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();       

      return sign_transaction( tx, broadcast,true );
   } FC_CAPTURE_AND_RETHROW( (owner_account)(url)(memo)(broadcast) ) }

   signed_transaction update_carrier(string carrier_name,
                                     string url,    optional<string> memo,                                  
                                     bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      carrier_object carrier = get_carrier(carrier_name);
      account_object carrier_account = get_account( carrier.carrier_account );

      carrier_update_operation carrier_update_op;
      carrier_update_op.carrier = carrier.id;
      carrier_update_op.carrier_account = carrier_account.id;      
      if( url != ""  && url != carrier.url)
         carrier_update_op.new_url = url;       
      if(memo.valid())
         carrier_update_op.new_memo = *memo;         

     proposal_create_operation prop_op; 
      bool proposal = carrier_update_op.new_url.valid();
      if(proposal)    
      {
          auto now =  _remote_db->get_dynamic_global_properties().time;      
          prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
          prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
          prop_op.fee_paying_account = carrier_account.id;
          prop_op.proposed_ops.emplace_back( carrier_update_op );
          set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
          _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);
      }
      signed_transaction tx;
      if(proposal)         
            tx.operations.push_back( prop_op );
      else       
            tx.operations.push_back( carrier_update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast ,proposal);
   } FC_CAPTURE_AND_RETHROW( (carrier_name)(url)(broadcast) ) }

    
     signed_transaction create_author(string owner_account,
                                     string url,string memo,
                                     vector<string> asset_symbol,
                                     bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      account_object author_account = get_account(owner_account);

      vector<asset_id_type> add_asset;
      for (const auto &aa : asset_symbol)
         add_asset.push_back(get_asset(aa).id);

      author_create_operation author_create_op;
      author_create_op.author_account = author_account.id;      
      author_create_op.url = url;
      author_create_op.memo = memo;
      author_create_op.auth_type = author_mask;
      author_create_op.allow_asset = add_asset;

      if (_remote_db->get_author_by_account(author_create_op.author_account))
         FC_THROW("Account ${owner_account} is already a author", ("owner_account", owner_account));

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = author_account.id;
      prop_op.proposed_ops.emplace_back(author_create_op);
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);

      signed_transaction tx;
      tx.operations.push_back( prop_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();       

      return sign_transaction( tx, broadcast ,true);
   } FC_CAPTURE_AND_RETHROW( (owner_account)(url)(memo)(asset_symbol)(broadcast) ) }

   signed_transaction update_author(string author_name,
                                     string url,    optional<string> memo,  optional<vector<string>> asset_symbol,                                  
                                     bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      author_object author = get_author(author_name);
      account_object author_account = get_account( author.author_account );

      author_update_operation author_update_op;
      author_update_op.author = author.id;
      author_update_op.author_account = author_account.id;
      if( url != ""  && url != author.url)
         author_update_op.new_url = url;       
      if(memo.valid())
         author_update_op.new_memo = *memo;     
      if(asset_symbol.valid())  
      {
         vector<asset_id_type> add;
         for (const auto &aa : (*asset_symbol))
            add.push_back(get_asset(aa).id);
         author_update_op.new_allow_asset = add;
      }  

     proposal_create_operation prop_op; 
      bool proposal = author_update_op.new_url.valid();
      if(proposal)    
      {
          auto now =  _remote_db->get_dynamic_global_properties().time;      
          prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
          prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
          prop_op.fee_paying_account = author_account.id;
          prop_op.proposed_ops.emplace_back( author_update_op );
          set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
          _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);
      }
      signed_transaction tx;
      if(proposal)         
            tx.operations.push_back( prop_op );
      else       
            tx.operations.push_back( author_update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast,proposal );
   } FC_CAPTURE_AND_RETHROW( (author_name)(url)(asset_symbol)(broadcast) ) }


   signed_transaction gateway_issue_currency(string owner_account,string gateway_account, string amount,string symbol, bool send ,bool broadcast)
   { try {
    FC_ASSERT(!is_locked());
      asset_object asset_obj = get_asset(symbol);
      gateway_issue_currency_operation gateway_issue_currency_op;
      gateway_issue_currency_op.issuer     = get_account_id(owner_account);
      gateway_issue_currency_op.account_to = get_account_id(gateway_account);      
      gateway_issue_currency_op.issue_currency   = asset_obj.amount_from_string(amount);;
      gateway_issue_currency_op.revoke = !send;

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = gateway_issue_currency_op.issuer;
      prop_op.proposed_ops.emplace_back( gateway_issue_currency_op );
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);

      signed_transaction tx;
      tx.operations.push_back( prop_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      

      return sign_transaction( tx, broadcast ,true);
   } FC_CAPTURE_AND_RETHROW( (owner_account)(gateway_account)(symbol)(amount)(send)(broadcast) ) }


   signed_transaction gateway_deposit(string from, string to, string deposit, string symbol,  bool broadcast )
   { try {
    FC_ASSERT(!is_locked());
      asset_object asset_obj = get_asset(symbol);
      gateway_deposit_operation gateway_deposit_op;
      gateway_deposit_op.from = get_account_id(from);
      gateway_deposit_op.to= get_account_id(to);      
      gateway_deposit_op.deposit   =  asset_obj.amount_from_string(deposit);       

      signed_transaction tx;
      tx.operations.push_back( gateway_deposit_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();   

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (from)(to)(symbol)(deposit)(broadcast) ) }

    signed_transaction gateway_propose_deposit(string account, string from, string to, string deposit, string symbol, string memo, bool broadcast )
   { try {
    FC_ASSERT(!is_locked());
     asset_object asset_obj = get_asset(symbol);
      gateway_deposit_operation gateway_deposit_op;
      gateway_deposit_op.from = get_account_id(from);
      gateway_deposit_op.to= get_account_id(to);     
      gateway_deposit_op.deposit   = asset_obj.amount_from_string(deposit);;   

      proposal_create_operation prop_op;
      auto now =  _remote_db->get_dynamic_global_properties().time;       
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      //prop_op.review_period_seconds = current_params.committee_proposal_review_period;
      prop_op.fee_paying_account =   get_account_id(account);;
      prop_op.memo = memo;

      prop_op.proposed_ops.emplace_back( gateway_deposit_op );
       _remote_db->get_global_properties().parameters.current_fees->set_fee( prop_op.proposed_ops.back().op );
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);


      signed_transaction tx;
      tx.operations.push_back( prop_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();   

      return sign_transaction( tx, broadcast,true );
   } FC_CAPTURE_AND_RETHROW( (account)(from)(to)(symbol)(deposit)(broadcast) ) }

   signed_transaction gateway_withdraw(string from, string to, string amount, string symbol,  bool broadcast)
   { try {
      FC_ASSERT(!is_locked());
      asset_object asset_obj = get_asset(symbol);
      gateway_withdraw_operation gateway_withdraw_op;
      gateway_withdraw_op.from = get_account_id(from);
      gateway_withdraw_op.to= get_account_id(to);   
      gateway_withdraw_op.withdraw   = asset_obj.amount_from_string(amount);   

      signed_transaction tx;
      tx.operations.push_back( gateway_withdraw_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();      

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (from)(to)(symbol)(amount)(broadcast) ) }

  signed_transaction gateway_create_withdraw(string from, string to, string amount, string symbol,uint32_t withdrawal_period_sec,uint32_t periods_until_expiration ,uint32_t identify, bool broadcast)
  { try {
      FC_ASSERT(!is_locked());
      asset_object asset_obj = get_asset(symbol);
      auto dyn_props = get_dynamic_global_properties();
      withdraw_permission_create_operation gateway_withdraw_op;
      gateway_withdraw_op.withdraw_from_account = get_account_id(from);
      gateway_withdraw_op.authorized_account= get_account_id(to);   
      gateway_withdraw_op.withdrawal_limit   = asset_obj.amount_from_string(amount);
      gateway_withdraw_op.period_start_time = dyn_props.time + fc::seconds(10);
      gateway_withdraw_op.periods_until_expiration = periods_until_expiration;
      gateway_withdraw_op.withdrawal_period_sec  = withdrawal_period_sec;
      gateway_withdraw_op.authorized_identify = identify;

      signed_transaction tx;
      tx.operations.push_back( gateway_withdraw_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();      

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (from)(to)(symbol)(amount)(withdrawal_period_sec)(periods_until_expiration)(broadcast) ) }

  signed_transaction gateway_update_withdraw(withdraw_permission_id_type id, string amount, string symbol,uint32_t withdrawal_period_sec,uint32_t periods_until_expiration , bool broadcast)
  { try {
      FC_ASSERT(!is_locked());
      asset_object asset_obj = get_asset(symbol);      
      withdraw_permission_object data = get_object<withdraw_permission_object>(id);
      FC_ASSERT(asset_obj.id == data.withdrawal_limit.asset_id);

      auto dyn_props = get_dynamic_global_properties();
      withdraw_permission_update_operation gateway_withdraw_op;
      gateway_withdraw_op.withdraw_from_account = data.withdraw_from_account;
      gateway_withdraw_op.authorized_account= data.authorized_account;   
      gateway_withdraw_op.withdrawal_limit   = asset_obj.amount_from_string(amount);
      gateway_withdraw_op.period_start_time = dyn_props.time + 1000;
      gateway_withdraw_op.periods_until_expiration = periods_until_expiration;
      gateway_withdraw_op.withdrawal_period_sec  = withdrawal_period_sec;
      gateway_withdraw_op.permission_to_update = id;

      signed_transaction tx;
      tx.operations.push_back( gateway_withdraw_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();      

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (id)(symbol)(amount)(withdrawal_period_sec)(periods_until_expiration)(broadcast) ) }

      signed_transaction gateway_remove_withdraw(withdraw_permission_id_type id,bool broadcast)
  { try {
      FC_ASSERT(!is_locked());         
      withdraw_permission_object data = get_object<withdraw_permission_object>(id);
      auto dyn_props = get_dynamic_global_properties();
      withdraw_permission_delete_operation gateway_withdraw_op;
      gateway_withdraw_op.withdraw_from_account = data.withdraw_from_account;
      gateway_withdraw_op.authorized_account    = data.authorized_account;   
      gateway_withdraw_op.withdraw_permission   = id;
      
      signed_transaction tx;
      tx.operations.push_back( gateway_withdraw_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();      

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (id)(broadcast) ) }

   signed_transaction gateway_claim_withdraw(withdraw_permission_id_type id, string amount, string symbol,  bool broadcast)
  { try {
      FC_ASSERT(!is_locked());
      asset_object asset_obj = get_asset(symbol);      
      withdraw_permission_object data = get_object<withdraw_permission_object>(id);
      FC_ASSERT(asset_obj.id == data.withdrawal_limit.asset_id);

      auto dyn_props = get_dynamic_global_properties();
      withdraw_permission_claim_operation gateway_withdraw_op;
      gateway_withdraw_op.withdraw_from_account = data.withdraw_from_account;
      gateway_withdraw_op.withdraw_to_account   = data.authorized_account;   
      gateway_withdraw_op.amount_to_withdraw    = asset_obj.amount_from_string(amount);      
      gateway_withdraw_op.withdraw_permission   = id;
      
      signed_transaction tx;
      tx.operations.push_back( gateway_withdraw_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();      

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (id)(symbol)(amount)(broadcast) ) }


   witness_object get_witness(string owner_account)
   {
      try
      {
         fc::optional<witness_id_type> witness_id = maybe_id<witness_id_type>(owner_account);
         if (witness_id)
         {
            std::vector<witness_id_type> ids_to_get;
            ids_to_get.push_back(*witness_id);
            std::vector<fc::optional<witness_object>> witness_objects = _remote_db->get_witnesses(ids_to_get);
            if (witness_objects.front())
               return *witness_objects.front();
            FC_THROW("No witness is registered for id ${id}", ("id", owner_account));
         }
         else
         {
            // then maybe it's the owner account
            try
            {
               account_id_type owner_account_id = get_account_id(owner_account);
               fc::optional<witness_object> witness = _remote_db->get_witness_by_account(owner_account_id);
               if (witness)
                  return *witness;
               else
                  FC_THROW("No witness is registered for account ${account}", ("account", owner_account));
            }
            catch (const fc::exception&)
            {
               FC_THROW("No account or witness named ${account}", ("account", owner_account));
            }
         }
      }
      FC_CAPTURE_AND_RETHROW( (owner_account) )
   }

    carrier_object get_carrier(string owner_account)
   {
      try
      {
         fc::optional<carrier_id_type> carrier_id = maybe_id<carrier_id_type>(owner_account);
         if (carrier_id)
         {
            std::vector<carrier_id_type> ids_to_get;
            ids_to_get.push_back(*carrier_id);
            std::vector<fc::optional<carrier_object>> carrier_objects = _remote_db->get_carrieres(ids_to_get);
            if (carrier_objects.front())
               return *carrier_objects.front();
            FC_THROW("No carrier is registered for id ${id}", ("id", owner_account));
         }
         else
         {
            // then maybe it's the owner account
            try
            {
               account_id_type owner_account_id = get_account_id(owner_account);
               fc::optional<carrier_object> carrier = _remote_db->get_carrier_by_account(owner_account_id);
               if (carrier)
                  return *carrier;
               else
                  FC_THROW("No carrier is registered for account ${account}", ("account", owner_account));
            }
            catch (const fc::exception&)
            {
               FC_THROW("No account or carrier named ${account}", ("account", owner_account));
            }
         }
      }
      FC_CAPTURE_AND_RETHROW( (owner_account) )
   }

    author_object get_author(string owner_account)
   {
      try
      {
         fc::optional<author_id_type> author_id = maybe_id<author_id_type>(owner_account);
         if (author_id)
         {
            std::vector<author_id_type> ids_to_get;
            ids_to_get.push_back(*author_id);
            std::vector<fc::optional<author_object>> author_objects = _remote_db->get_authors(ids_to_get);
            if (author_objects.front())
               return *author_objects.front();
            FC_THROW("No author is registered for id ${id}", ("id", owner_account));
         }
         else
         {
            // then maybe it's the owner account
            try
            {
               account_id_type owner_account_id = get_account_id(owner_account);
               fc::optional<author_object> author = _remote_db->get_author_by_account(owner_account_id);
               if (author)
                  return *author;
               else
                  FC_THROW("No author is registered for account ${account}", ("account", owner_account));
            }
            catch (const fc::exception&)
            {
               FC_THROW("No account or author named ${account}", ("account", owner_account));
            }
         }
      }
      FC_CAPTURE_AND_RETHROW( (owner_account) )
   }

   gateway_object get_gateway(string owner_account)
   {
      try
      {
         fc::optional<gateway_id_type> gateway_id = maybe_id<gateway_id_type>(owner_account);
         if (gateway_id)
         {
            std::vector<gateway_id_type> ids_to_get;
            ids_to_get.push_back(*gateway_id);
            std::vector<fc::optional<gateway_object>> gateway_objects = _remote_db->get_gatewayes(ids_to_get);
            if (gateway_objects.front())
               return *gateway_objects.front();
            FC_THROW("No gateway is registered for id ${id}", ("id", owner_account));
         }
         else
         {
            // then maybe it's the owner account
            try
            {
               account_id_type owner_account_id = get_account_id(owner_account);
               fc::optional<gateway_object> gateway = _remote_db->get_gateway_by_account(owner_account_id);
               if (gateway)
                  return *gateway;
               else
                  FC_THROW("No gateway is registered for account ${account}", ("account", owner_account));
            }
            catch (const fc::exception&)
            {
               FC_THROW("No account or gateway named ${account}", ("account", owner_account));
            }
         }
      }
      FC_CAPTURE_AND_RETHROW( (owner_account) )
   }
   committee_member_object get_committee_member(string owner_account)
   {
      try
      {
         fc::optional<committee_member_id_type> committee_member_id = maybe_id<committee_member_id_type>(owner_account);
         if (committee_member_id)
         {
            std::vector<committee_member_id_type> ids_to_get;
            ids_to_get.push_back(*committee_member_id);
            std::vector<fc::optional<committee_member_object>> committee_member_objects = _remote_db->get_committee_members(ids_to_get);
            if (committee_member_objects.front())
               return *committee_member_objects.front();
            FC_THROW("No committee_member is registered for id ${id}", ("id", owner_account));
         }
         else
         {
            // then maybe it's the owner account
            try
            {
               account_id_type owner_account_id = get_account_id(owner_account);
               fc::optional<committee_member_object> committee_member = _remote_db->get_committee_member_by_account(owner_account_id);
               if (committee_member)
                  return *committee_member;
               else
                  FC_THROW("No committee_member is registered for account ${account}", ("account", owner_account));
            }
            catch (const fc::exception&)
            {
               FC_THROW("No account or committee_member named ${account}", ("account", owner_account));
            }
         }
      }
      FC_CAPTURE_AND_RETHROW( (owner_account) )
   }



   budget_member_object get_budget_member(string owner_account)
   {
      try
      {
         fc::optional<budget_member_id_type> budget_member_id = maybe_id<budget_member_id_type>(owner_account);
         if (budget_member_id)
         {
            std::vector<budget_member_id_type> ids_to_get;
            ids_to_get.push_back(*budget_member_id);
            std::vector<fc::optional<budget_member_object>> budget_member_objects = _remote_db->get_budget_members(ids_to_get);
            if (budget_member_objects.front())
               return *budget_member_objects.front();
            FC_THROW("No budget_member is registered for id ${id}", ("id", owner_account));
         }
         else
         {
            // then maybe it's the owner account
            try
            {
               account_id_type owner_account_id = get_account_id(owner_account);
               fc::optional<budget_member_object> budget_member = _remote_db->get_budget_member_by_account(owner_account_id);
               if (budget_member)
                  return *budget_member;
               else
                  FC_THROW("No budget_member is registered for account ${account}", ("account", owner_account));
            }
            catch (const fc::exception&)
            {
               FC_THROW("No account or budget_member named ${account}", ("account", owner_account));
            }
         }
      }
      FC_CAPTURE_AND_RETHROW( (owner_account) )
   }

   void get_gas_info( )
   {
      /* time_point_sec tsrc = fc::time_point::now();
       time_t t = tsrc.sec_since_epoch();
        tm tmthis = *gmtime(&t);    
       tmthis.tm_mon = 0;
       tmthis.tm_mday = 31;

       tsrc = fc::time_point_sec(fc::make_utc_time(&tmthis));
       std::cerr << "    " << tsrc.to_iso_string() << "\n";
       for(int l=1;l<=24;l++)
       {
           time_point_sec tsrc1 = fc :: add_month(tsrc,l);
           std::cerr << "    " << tsrc1.to_iso_string() << "\n";
       }*/               

           global_property_object gp = get_global_properties();
           dynamic_global_property_object dgp = get_dynamic_global_properties();
           asset_object asset_obj = get_object(GRAPHENE_CORE_ASSET);
           asset_dynamic_data_object dadata = get_object<asset_dynamic_data_object>(asset_obj.dynamic_asset_data_id);

           // propose_parameter_change

           uint32_t budget_per_block = GRAPHENE_BLOCKCHAIN_BUDGET * gp.parameters.block_interval / 60;

           std::cerr << "block_interval                  " << fc::to_string(gp.parameters.block_interval) << "\n";

           std::cerr << "maintenance_interval            " << fc::to_string(gp.parameters.maintenance_interval) << "\n";
           std::cerr << "budget per block                " << fc::to_string(budget_per_block) << "\n";
           std::cerr << "genines time                    " << dgp.initial_time.to_iso_string() << "\n";

           std::cerr << "max_supply                      " << fc::to_string(asset_obj.options.max_supply.value / GRAPHENE_BLOCKCHAIN_PRECISION) << "\n";
           std::cerr << "current_supply                  " << fc::to_string(dadata.current_supply.value / GRAPHENE_BLOCKCHAIN_PRECISION) << "\n";
           std::cerr << "remain_supply                   " << fc::to_string((asset_obj.options.max_supply.value - dadata.current_supply.value) / GRAPHENE_BLOCKCHAIN_PRECISION) << "\n";
           std::cerr << "confidential_supply             " << fc::to_string(dadata.confidential_supply.value / GRAPHENE_BLOCKCHAIN_PRECISION) << "\n";
           std::cerr << "accumulated_fees                " << fc::to_string(dadata.accumulated_fees.value / GRAPHENE_BLOCKCHAIN_PRECISION) << "\n";
           std::cerr << "fee_pool                        " << fc::to_string(dadata.fee_pool.value) << "\n";

           time_t genis_t = dgp.initial_time.sec_since_epoch();
           tm genistm = *gmtime(&genis_t);

           tm nowtm;
           nowtm = genistm;
           nowtm.tm_year = genistm.tm_year;
           nowtm.tm_mon = 11;
           nowtm.tm_mday = 31;
           nowtm.tm_hour = 23;
           nowtm.tm_min = 59;
           nowtm.tm_sec = 59;

           int64_t gast = 0, gasye = 0;
           fc::time_point_sec tpre = dgp.initial_time;
           while (true)
           {
               time_t t1 = mktime(&nowtm);
               fc::time_point_sec t2 = fc::time_point_sec(t1);
               uint32_t lb = (t2.sec_since_epoch() - tpre.sec_since_epoch()) / gp.parameters.block_interval;
               uint32_t lpre = _remote_db->get_budget_pre_block(t2);
               gast = lb * lpre;
               gasye += gast;

               std::cerr << t2.to_iso_string() << "     ";
               std::cerr << "block num  pre year " << fc::to_string(lb) << "   ";
               std::cerr << "gas pre block " << fc::to_string(lpre) << "   ";
               std::cerr << "gas pre year " << fc::to_string(gast) << "   ";
               std::cerr << "gas total  " << fc::to_string(gasye) << "\n";

               if (lpre <= 0)
                   break;
               nowtm.tm_year++;
               tpre = t2;
           }

           std::cerr << "gas total     " << fc::to_string(gasye) << "\n";
           std::cerr << "remain_supply " << fc::to_string((asset_obj.options.max_supply.value - dadata.current_supply.value) / GRAPHENE_BLOCKCHAIN_PRECISION - gasye) << "\n"; //  ;
           
}

   signed_transaction create_witness(string owner_account,
                                     string url,string memo,
                                     bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      account_object witness_account = get_account(owner_account);
      fc::ecc::private_key active_private_key = get_private_key_for_account(witness_account);
      int witness_key_index = find_first_unused_derived_key_index(active_private_key);
      fc::ecc::private_key witness_private_key = derive_private_key(key_to_wif(active_private_key), witness_key_index);
      graphene::chain::public_key_type witness_public_key = witness_private_key.get_public_key();

      witness_create_operation witness_create_op;
      witness_create_op.witness_account = witness_account.id;
      witness_create_op.block_signing_key = witness_public_key;
      witness_create_op.url = url;
      witness_create_op.memo = memo;

      if (_remote_db->get_witness_by_account(witness_create_op.witness_account))
         FC_THROW("Account ${owner_account} is already a witness", ("owner_account", owner_account));

      signed_transaction tx;
      tx.operations.push_back( witness_create_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      _wallet.pending_witness_registrations[owner_account] = key_to_wif(witness_private_key);

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (owner_account)(url)(memo)(broadcast) ) }

   signed_transaction update_witness(string witness_name,
                                     string url, optional<string> memo,
                                     optional<string> block_signing_key,
                                     bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      witness_object witness = get_witness(witness_name);
      account_object witness_account = get_account( witness.witness_account );

      witness_update_operation witness_update_op;
      witness_update_op.witness = witness.id;
      witness_update_op.witness_account = witness_account.id;
      if( url != "" )
         witness_update_op.new_url = url;
      if( memo.valid())
         witness_update_op.new_memo = memo;   
      if( block_signing_key.valid() )
         witness_update_op.new_signing_key = public_key_type(*block_signing_key );

      signed_transaction tx;
      tx.operations.push_back( witness_update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (witness_name)(url)(memo)(block_signing_key)(broadcast) ) }

   signed_transaction change_identity( string issuer_name,
                                       object_id_type object_id,       
                                       bool benable,
                                       bool broadcast = false)
   { try {
       FC_ASSERT( !self.is_locked() );
      account_object account = get_account(issuer_name);      

      change_identity_operation witness_update_op;
      witness_update_op.issuer    = account.id;       
      witness_update_op.object_id = object_id;
      witness_update_op.enable    = benable;     

      signed_transaction tx;
      tx.operations.push_back( witness_update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (issuer_name)(object_id)(benable)(broadcast) ) }

   signed_transaction enable_witness( string witness_name, bool broadcast = false)
   { try {
       FC_ASSERT( !self.is_locked() );
      witness_object witness = get_witness(witness_name);
      account_object witness_account = get_account( witness.witness_account );
      FC_ASSERT(witness.enable == identity_enable_lost);
      change_identity_operation witness_update_op;
      witness_update_op.issuer    = witness_account.id;       
      witness_update_op.object_id = witness.id;
      witness_update_op.enable    = true;     

      signed_transaction tx;
      tx.operations.push_back( witness_update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (witness_name)(broadcast) ) }


   template<typename WorkerInit>
   static WorkerInit _create_worker_initializer( const variant& worker_settings )
   {
      WorkerInit result;
      from_variant( worker_settings, result, GRAPHENE_MAX_NESTED_OBJECTS );
      return result;
   }

   signed_transaction create_worker(
      string owner_account,
      time_point_sec work_begin_date,
      time_point_sec work_end_date,
      string daily_pay,
      string name,
      string url,
      string memo,
      variant worker_settings,
      bool broadcast
      )
   {    try{
       FC_ASSERT( !self.is_locked() );
      worker_initializer init;
      std::string wtype = worker_settings["type"].get_string();
      const auto &zos_core = get_asset("ZOS");

      // TODO:  Use introspection to do this dispatch
      if( wtype == "burn" )
         init = _create_worker_initializer< burn_worker_initializer >( worker_settings );
      else if( wtype == "refund" )
         init = _create_worker_initializer< refund_worker_initializer >( worker_settings );
      else if( wtype == "vesting" )
         init = _create_worker_initializer< vesting_balance_worker_initializer >( worker_settings );
      else if( wtype == "exchange" )
         init = _create_worker_initializer< exchange_worker_initializer >( worker_settings );      

      else
      {
         FC_ASSERT( false, "unknown worker[\"type\"] value" );
      }
      FC_ASSERT(name.size() < GRAPHENE_MAX_WORKER_NAME_LENGTH );
      FC_ASSERT(url.size() < GRAPHENE_MAX_URL_LENGTH );

      worker_create_operation op;
      op.owner = get_account( owner_account ).id;
      op.work_begin_date = work_begin_date;
      op.work_end_date = work_end_date;
      op.daily_pay = zos_core.amount_from_string(daily_pay).amount;
      op.name = name;
      op.url = url;
      op.memo = memo;
      op.initializer = init;

      signed_transaction tx;
      tx.operations.push_back( op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW((owner_account)(work_begin_date)(work_end_date)(daily_pay)(name)(url)(memo)(worker_settings)) }


   signed_transaction vote_for_worker(
      string budget_member_name,
      worker_vote_delta delta,
      bool broadcast
      )
   { try{       
      budget_member_object budget_member = get_budget_member(budget_member_name);
      account_object budget_account = get_account( budget_member.budget_member_account );
      FC_ASSERT( !self.is_locked() );
      // you could probably use a faster algorithm for this, but flat_set is fast enough :)
      flat_set< worker_id_type > merged;
      merged.reserve( delta.vote_for.size() + delta.vote_against.size() + delta.vote_abstain.size() );
      for( const worker_id_type& wid : delta.vote_for )
      {
         bool inserted = merged.insert( wid ).second;
         FC_ASSERT( inserted, "worker ${wid} specified multiple times", ("wid", wid) );
      }
      for( const worker_id_type& wid : delta.vote_against )
      {
         bool inserted = merged.insert( wid ).second;
         FC_ASSERT( inserted, "worker ${wid} specified multiple times", ("wid", wid) );
      }
      for( const worker_id_type& wid : delta.vote_abstain )
      {
         bool inserted = merged.insert( wid ).second;
         FC_ASSERT( inserted, "worker ${wid} specified multiple times", ("wid", wid) );
      }

      // should be enforced by FC_ASSERT's above
      assert( merged.size() == delta.vote_for.size() + delta.vote_against.size() + delta.vote_abstain.size() );

      vector< object_id_type > query_ids;
      for( const worker_id_type& wid : merged )
         query_ids.push_back( wid );

      flat_set<vote_id_type> new_votes( budget_member.votes );

      fc::variants objects = _remote_db->get_objects( query_ids );
      for( const variant& obj : objects )
      {
         worker_object wo;         
         from_variant( obj, wo, GRAPHENE_MAX_NESTED_OBJECTS );
         
         FC_ASSERT(wo.work_end_date >=_remote_db->get_dynamic_global_properties().time);

         new_votes.erase(wo.vote_for);
         new_votes.erase( wo.vote_against );
         if( delta.vote_for.find( wo.id ) != delta.vote_for.end() )
            new_votes.insert( wo.vote_for );
         else if( delta.vote_against.find( wo.id ) != delta.vote_against.end() )
            new_votes.insert( wo.vote_against );
         else
            assert( delta.vote_abstain.find( wo.id ) != delta.vote_abstain.end() );
      }
      FC_ASSERT(!budget_member.is_samevotes(new_votes));

      budget_member_update_operation update_op;
      update_op.budget_member         = budget_member.id;
      update_op.budget_member_account = budget_account.id;    
      update_op.votes                 = new_votes;

      signed_transaction tx;
      tx.operations.push_back( update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
    } FC_CAPTURE_AND_RETHROW((budget_member_name)(delta)) }

   vector< vesting_balance_object_with_info > get_vesting_balances( string account_name )
   { try {
      fc::optional<vesting_balance_id_type> vbid = maybe_id<vesting_balance_id_type>( account_name );
      std::vector<vesting_balance_object_with_info> result;
      fc::time_point_sec now = _remote_db->get_dynamic_global_properties().time;

      if( vbid )
      {
         result.emplace_back( get_object<vesting_balance_object>(*vbid), now );
         return result;
      }

      // try casting to avoid a round-trip if we were given an account ID
      fc::optional<account_id_type> acct_id = maybe_id<account_id_type>( account_name );
      if( !acct_id )
         acct_id = get_account( account_name ).id;

      vector< vesting_balance_object > vbos = _remote_db->get_vesting_balances( *acct_id );
      if( vbos.size() == 0 )
         return result;

      for( const vesting_balance_object& vbo : vbos )
         result.emplace_back( vbo, now );

      return result;
   } FC_CAPTURE_AND_RETHROW( (account_name) )  }
   
   signed_transaction revoke_vesting(
         string account_name,
         vesting_balance_id_type object_id,         
         bool broadcast)
    { try {  
        FC_ASSERT( !self.is_locked() );

      const vesting_balance_object  &vest = get_object(object_id); 
      const account_object  &acc = get_account(account_name);

      signed_transaction tx;

      revoke_vesting_operation x_op;
      x_op.issuer = vest.from;
      x_op.vesting_id = object_id;     

      if(vest.from !=  acc.id) {
         proposal_create_operation prop_op;
         auto now =  _remote_db->get_dynamic_global_properties().time;       
         prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;       
     // prop_op.review_period_seconds = current_params.committee_proposal_review_period;
         prop_op.fee_paying_account =  acc.id ;

         prop_op.proposed_ops.emplace_back( x_op );
         _remote_db->get_global_properties().parameters.current_fees->set_fee( prop_op.proposed_ops.back().op );
         _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);
      
         tx.operations.push_back(prop_op);
         set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      } else {      
         tx.operations.push_back( x_op );
         set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      }
      tx.validate();
      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (account_name)(object_id)(broadcast) )}

   signed_transaction withdraw_vesting(
      string witness_name,
      string amount,
      string asset_symbol,
      bool broadcast = false )
   { try {
       FC_ASSERT( !self.is_locked() );
      asset_object asset_obj = get_asset( asset_symbol );
      fc::optional<vesting_balance_id_type> vbid = maybe_id<vesting_balance_id_type>(witness_name);
      if( !vbid )
      {
         witness_object wit = get_witness( witness_name );
         FC_ASSERT( wit.pay_vb );
         vbid = wit.pay_vb;
      }

      vesting_balance_object vbo = get_object< vesting_balance_object >( *vbid );
      vesting_balance_withdraw_operation vesting_balance_withdraw_op;

      vesting_balance_withdraw_op.vesting_balance = *vbid;
      vesting_balance_withdraw_op.owner = vbo.owner;
      vesting_balance_withdraw_op.amount = asset_obj.amount_from_string(amount);

      signed_transaction tx;
      tx.operations.push_back( vesting_balance_withdraw_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (witness_name)(amount) )  }

   signed_transaction vote_for_committee_member(string voting_account,
                                        string committee_member,
                                        bool approve,
                                        bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      account_object voting_account_object = get_account(voting_account);
      account_id_type committee_member_owner_account_id = get_account_id(committee_member);
      fc::optional<committee_member_object> committee_member_obj = _remote_db->get_committee_member_by_account(committee_member_owner_account_id);
      if (!committee_member_obj)
         FC_THROW("Account ${committee_member} is not registered as a committee_member", ("committee_member", committee_member));
      if (approve)
      {
         auto insert_result = voting_account_object.options.votes.insert(committee_member_obj->vote_id);
         if (!insert_result.second)
            FC_THROW("Account ${account} was already voting for committee_member ${committee_member}", ("account", voting_account)("committee_member", committee_member));
      }
      else
      {
         unsigned votes_removed = voting_account_object.options.votes.erase(committee_member_obj->vote_id);
         if (!votes_removed)
            FC_THROW("Account ${account} is already not voting for committee_member ${committee_member}", ("account", voting_account)("committee_member", committee_member));
      }
      account_update_operation account_update_op;
      account_update_op.account = voting_account_object.id;
      account_update_op.new_options = voting_account_object.options;

      signed_transaction tx;
      tx.operations.push_back( account_update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (voting_account)(committee_member)(approve)(broadcast) ) }

   signed_transaction vote_for_budget_member(string voting_account,
                                        string budget_member,
                                        bool approve,
                                        bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      account_object voting_account_object = get_account(voting_account);
      account_id_type budget_member_owner_account_id = get_account_id(budget_member);
      fc::optional<budget_member_object> budget_member_obj = _remote_db->get_budget_member_by_account(budget_member_owner_account_id);
      if (!budget_member_obj)
         FC_THROW("Account ${budget_member} is not registered as a budget_member", ("budget_member", budget_member));
      if (approve)
      {
         auto insert_result = voting_account_object.options.votes.insert(budget_member_obj->vote_id);
         if (!insert_result.second)
            FC_THROW("Account ${account} was already voting for budget_member ${budget_member}", ("account", voting_account)("budget_member", budget_member));
      }
      else
      {
         unsigned votes_removed = voting_account_object.options.votes.erase(budget_member_obj->vote_id);
         if (!votes_removed)
            FC_THROW("Account ${account} is already not voting for budget_member ${budget_member}", ("account", voting_account)("budget_member", budget_member));
      }
      account_update_operation account_update_op;
      account_update_op.account = voting_account_object.id;
      account_update_op.new_options = voting_account_object.options;

      signed_transaction tx;
      tx.operations.push_back( account_update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (voting_account)(budget_member)(approve)(broadcast) ) }


   signed_transaction vote_for_witness(string voting_account,
                                        string witness,
                                        bool approve,
                                        bool broadcast /* = false */)
   { try {
       FC_ASSERT( !self.is_locked() );
      account_object voting_account_object = get_account(voting_account);
      account_id_type witness_owner_account_id = get_account_id(witness);
      fc::optional<witness_object> witness_obj = _remote_db->get_witness_by_account(witness_owner_account_id);
      if (!witness_obj)
         FC_THROW("Account ${witness} is not registered as a witness", ("witness", witness));
      if (approve)
      {
         auto insert_result = voting_account_object.options.votes.insert(witness_obj->vote_id);
         if (!insert_result.second)
            FC_THROW("Account ${account} was already voting for witness ${witness}", ("account", voting_account)("witness", witness));
      }
      else
      {
         unsigned votes_removed = voting_account_object.options.votes.erase(witness_obj->vote_id);
         if (!votes_removed)
            FC_THROW("Account ${account} is already not voting for witness ${witness}", ("account", voting_account)("witness", witness));
      }
      account_update_operation account_update_op;
      account_update_op.account = voting_account_object.id;
      account_update_op.new_options = voting_account_object.options;

      signed_transaction tx;
      tx.operations.push_back( account_update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (voting_account)(witness)(approve)(broadcast) ) }

   signed_transaction set_voting_proxy(string account_to_modify,
                                       optional<string> voting_account,
                                       bool broadcast /* = false */)
   { try {
      account_object account_object_to_modify = get_account(account_to_modify);
       FC_ASSERT( !self.is_locked() );
      if (voting_account)
      {
         account_id_type new_voting_account_id = get_account_id(*voting_account);
         if (account_object_to_modify.options.voting_account == new_voting_account_id)
            FC_THROW("Voting proxy for ${account} is already set to ${voter}", ("account", account_to_modify)("voter", *voting_account));
         account_object_to_modify.options.voting_account = new_voting_account_id;
      }
      else
      {
         if (account_object_to_modify.options.voting_account == GRAPHENE_PROXY_TO_SELF_ACCOUNT)
            FC_THROW("Account ${account} is already voting for itself", ("account", account_to_modify));
         account_object_to_modify.options.voting_account = GRAPHENE_PROXY_TO_SELF_ACCOUNT;
      }

      account_update_operation account_update_op;
      account_update_op.account = account_object_to_modify.id;
      account_update_op.new_options = account_object_to_modify.options;

      signed_transaction tx;
      tx.operations.push_back( account_update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (account_to_modify)(voting_account)(broadcast) ) }

   signed_transaction set_desired_member_count(string account_to_modify,
                                                             uint16_t desired_number_of_witnesses,
                                                             uint16_t desired_number_of_committee_members,
                                                             uint16_t desired_number_of_budget_members,
                                                             bool broadcast /* = false */)
   { try {
      account_object account_object_to_modify = get_account(account_to_modify);

      if (account_object_to_modify.options.num_witness == desired_number_of_witnesses &&
          account_object_to_modify.options.num_committee == desired_number_of_committee_members &&
          account_object_to_modify.options.num_budget == desired_number_of_budget_members)

         FC_THROW("Account ${account} is already voting for ${witnesses} witnesses and ${committee_members} committee_members and ${budget_members} budget_members",
                  ("account", account_to_modify)("witnesses", desired_number_of_witnesses)("committee_members",desired_number_of_committee_members)("budget_members",desired_number_of_budget_members));
      account_object_to_modify.options.num_witness = desired_number_of_witnesses;
      account_object_to_modify.options.num_committee = desired_number_of_committee_members;
      account_object_to_modify.options.num_budget = desired_number_of_budget_members;

      account_update_operation account_update_op;
      account_update_op.account = account_object_to_modify.id;
      account_update_op.new_options = account_object_to_modify.options;

      signed_transaction tx;
      tx.operations.push_back( account_update_op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW( (account_to_modify)(desired_number_of_witnesses)(desired_number_of_committee_members)(desired_number_of_budget_members)(broadcast) ) }

   signed_transaction sign_transaction(signed_transaction tx, bool broadcast = false, bool bcallback = false)
   {
      FC_ASSERT(tx.operations.size() > 0);
      set<public_key_type> pks = _remote_db->get_potential_signatures(tx);
      flat_set<public_key_type> owned_keys;
      owned_keys.reserve(pks.size());
      std::copy_if(pks.begin(), pks.end(), std::inserter(owned_keys, owned_keys.end()),
                   [this](const public_key_type &pk) { return _keys.find(pk) != _keys.end(); });
      set<public_key_type> approving_key_set = _remote_db->get_required_signatures(tx, owned_keys);

      auto dyn_props = get_dynamic_global_properties();
      tx.set_reference_block(dyn_props.head_block_id);

      // first, some bookkeeping, expire old items from _recently_generated_transactions
      // since transactions include the head block id, we just need the index for keeping transactions unique
      // when there are multiple transactions in the same block.  choose a time period that should be at
      // least one block long, even in the worst case.  2 minutes ought to be plenty.
      fc::time_point_sec oldest_transaction_ids_to_track(dyn_props.time - fc::minutes(2));
      auto oldest_transaction_record_iter = _recently_generated_transactions.get<timestamp_index>().lower_bound(oldest_transaction_ids_to_track);
      auto begin_iter = _recently_generated_transactions.get<timestamp_index>().begin();
      _recently_generated_transactions.get<timestamp_index>().erase(begin_iter, oldest_transaction_record_iter);

      uint32_t expiration_time_offset = 0;
      for (;;)
      {
         tx.set_expiration(dyn_props.time + fc::seconds(30 + expiration_time_offset));
         tx.signatures.clear();

         for (const public_key_type &key : approving_key_set)
            tx.sign(get_private_key(key), _chain_id);

         graphene::chain::transaction_id_type this_transaction_id = tx.id();
         auto iter = _recently_generated_transactions.find(this_transaction_id);
         if (iter == _recently_generated_transactions.end())
         {
            // we haven't generated this transaction before, the usual case
            recently_generated_transaction_record this_transaction_record;
            this_transaction_record.generation_time = dyn_props.time;
            this_transaction_record.transaction_id = this_transaction_id;
            _recently_generated_transactions.insert(this_transaction_record);
            break;
         }

         // else we've generated a dupe, increment expiration time and re-sign it
         ++expiration_time_offset;
      }

      if( broadcast )
      {   
         try
         {         
            if (bcallback)
                broadcast_transaction_synchronous(tx);          
            else 
                _remote_net_broadcast->broadcast_transaction( tx );          
         }
         catch (const fc::exception& e)
         {
            elog("Caught exception while broadcasting tx ${id}:  ${e}", ("id", tx.id().str())("e", e.to_detail_string()) );
            throw;
         }
      }
      return tx;
   }

   memo_data sign_memo(string from, string to, string memo)
   {
      FC_ASSERT( !self.is_locked() );

      memo_data md = memo_data();


      // get account memo key, if that fails, try a pubkey
      try {
         account_object from_account = get_account(from);
         md.from = from_account.options.memo_key;
      } catch (const fc::exception& e) {
         md.from =  self.get_public_key( from );
      }
      // same as above, for destination key
      try {
         account_object to_account = get_account(to);
         md.to = to_account.options.memo_key;
      } catch (const fc::exception& e) {
         md.to = self.get_public_key( to );
      }
      md.set_message(get_private_key(md.from), md.to, memo);
      return md;
   }
   memo_data sign_memo_auth(string from, string to, string memo)
   {
      FC_ASSERT( !self.is_locked() );

      memo_data md = memo_data();


      // get account memo key, if that fails, try a pubkey
      try {
         account_object from_account = get_account(from);
         md.from = from_account.options.auth_key;
      } catch (const fc::exception& e) {
         md.from =  self.get_public_key( from );
      }
      // same as above, for destination key
      try {
         account_object to_account = get_account(to);
         md.to = to_account.options.auth_key;
      } catch (const fc::exception& e) {
         md.to = self.get_public_key( to );
      }
      md.set_message(get_private_key(md.from), md.to, memo);
      return md;
   }

   string read_memo(const memo_data& md)
   {
      FC_ASSERT(!is_locked());
      std::string clear_text;

      const memo_data *memo = &md;

      try {
         FC_ASSERT(_keys.count(memo->to) || _keys.count(memo->from), "Memo is encrypted to a key ${to} or ${from} not in this wallet.", ("to", memo->to)("from",memo->from));
         if( _keys.count(memo->to) ) {
            auto my_key = wif_to_key(_keys.at(memo->to));
            FC_ASSERT(my_key, "Unable to recover private key to decrypt memo. Wallet may be corrupted.");
            clear_text = memo->get_message(*my_key, memo->from);
         } else {
            auto my_key = wif_to_key(_keys.at(memo->from));
            FC_ASSERT(my_key, "Unable to recover private key to decrypt memo. Wallet may be corrupted.");
            clear_text = memo->get_message(*my_key, memo->to);
         }
      } catch (const fc::exception& e) {
         elog("Error when decrypting memo: ${e}", ("e", e.to_detail_string()));
      }

      return clear_text;
   }

   signed_transaction sell_asset(string seller_account,
                                 string amount_to_sell,
                                 string symbol_to_sell,
                                 string min_to_receive,
                                 string symbol_to_receive,
                                 uint32_t timeout_sec = 0,
                                 uint16_t mode = 0,
                                 bool   broadcast = false)
   {
      try {
      account_object seller   = get_account( seller_account );
      FC_ASSERT( !self.is_locked() );
      FC_ASSERT(timeout_sec > 2);
      limit_order_create_operation op;
      op.seller = seller.id;
      op.amount_to_sell = get_asset(symbol_to_sell).amount_from_string(amount_to_sell);
      op.min_to_receive = get_asset(symbol_to_receive).amount_from_string(min_to_receive);
      if( timeout_sec )
         op.expiration = _remote_db->get_dynamic_global_properties().time + fc::seconds(timeout_sec);
      op.fill_or_kill = true;
      op.fill_mode = mode;

      signed_transaction tx;
      tx.operations.push_back(op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction( tx, broadcast );
   } FC_CAPTURE_AND_RETHROW((seller_account)(amount_to_sell)(symbol_to_sell)(min_to_receive)(symbol_to_sell)(timeout_sec)(mode)) }

   signed_transaction borrow_asset(string seller_name, string amount_to_borrow, string asset_symbol,
                                       string amount_of_collateral, bool broadcast = false)
   {
       FC_ASSERT( !self.is_locked() );
      account_object seller = get_account(seller_name);
      asset_object mia = get_asset(asset_symbol);
      FC_ASSERT(mia.is_market_issued());
      FC_ASSERT(mia.bitasset_data_id);
      asset_object collateral = get_asset(get_object(*mia.bitasset_data_id).options.short_backing_asset);

      call_order_update_operation op;
      op.funding_account = seller.id;
      op.delta_debt   = mia.amount_from_string(amount_to_borrow);
      op.delta_collateral = collateral.amount_from_string(amount_of_collateral);

      signed_transaction trx;
      trx.operations = {op};
      set_operation_fees( trx, _remote_db->get_global_properties().parameters.current_fees);
      trx.validate();
      idump((broadcast));

      return sign_transaction(trx, broadcast);
   }

   signed_transaction cancel_order(object_id_type order_id, bool broadcast = false)
   { try {
         FC_ASSERT(!is_locked());
         FC_ASSERT(order_id.space() == protocol_ids, "Invalid order ID ${id}", ("id", order_id));
         signed_transaction trx;

         limit_order_cancel_operation op;
         op.fee_paying_account = get_object<limit_order_object>(order_id).seller;
         op.order = order_id;
         trx.operations = {op};
         set_operation_fees( trx, _remote_db->get_global_properties().parameters.current_fees);

         trx.validate();
         return sign_transaction(trx, broadcast);
   } FC_CAPTURE_AND_RETHROW((order_id)) }

   signed_transaction transfer(string from, string to, string amount,
                               string asset_symbol, string memo, bool broadcast = false)
   { try {
      FC_ASSERT( !self.is_locked() );
      fc::optional<asset_object> asset_obj = get_asset(asset_symbol);
      FC_ASSERT(asset_obj, "Could not find asset matching ${asset}", ("asset", asset_symbol));

      account_object from_account = get_account(from);
      account_object to_account = get_account(to);
      account_id_type from_id = from_account.id;
      account_id_type to_id = get_account_id(to);

      transfer_operation xfer_op;

      xfer_op.from = from_id;
      xfer_op.to = to_id;
      xfer_op.amount = asset_obj->amount_from_string(amount);

      if( memo.size() )
      {
         xfer_op.memo = memo_data();
         xfer_op.memo->from = from_account.options.memo_key;
         xfer_op.memo->to = to_account.options.memo_key;

         xfer_op.memo->set_message(get_private_key(from_account.options.memo_key),
                                  to_account.options.memo_key, memo);
      }

      signed_transaction tx;
      tx.operations.push_back(xfer_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast);
   } FC_CAPTURE_AND_RETHROW( (from)(to)(amount)(asset_symbol)(memo)(broadcast) ) }

   signed_transaction transfer_noencmsg(string from, string to, string amount,
                               string asset_symbol, string memo, bool broadcast = false)
   { try {
      FC_ASSERT( !self.is_locked() );
      fc::optional<asset_object> asset_obj = get_asset(asset_symbol);
      FC_ASSERT(asset_obj, "Could not find asset matching ${asset}", ("asset", asset_symbol));

      account_object from_account = get_account(from);
      account_object to_account = get_account(to);
      account_id_type from_id = from_account.id;
      account_id_type to_id = get_account_id(to);

      transfer_operation xfer_op;

      xfer_op.from = from_id;
      xfer_op.to = to_id;
      xfer_op.amount = asset_obj->amount_from_string(amount);

      if( memo.size() )
         {
            xfer_op.memo = memo_data();
            xfer_op.memo->from = from_account.options.memo_key;
            xfer_op.memo->to = to_account.options.memo_key;
            xfer_op.memo->set_message(memo);
         }

      signed_transaction tx;
      tx.operations.push_back(xfer_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast);
   } FC_CAPTURE_AND_RETHROW( (from)(to)(amount)(asset_symbol)(memo)(broadcast) ) }

    signed_transaction propose_transfer(string from,
                                  string to,
                                  string amount,
                                  string asset_symbol,
                                  string memo,
                                  bool broadcast)
     { try {
      FC_ASSERT( !self.is_locked() );
      fc::optional<asset_object> asset_obj = get_asset(asset_symbol);
      FC_ASSERT(asset_obj, "Could not find asset matching ${asset}", ("asset", asset_symbol));

      account_object from_account = get_account(from);
      account_object to_account = get_account(to);
      account_id_type from_id = from_account.id;
      account_id_type to_id = get_account_id(to);

      transfer_operation xfer_op;

      xfer_op.from = from_id;
      xfer_op.to = to_id;
      xfer_op.amount = asset_obj->amount_from_string(amount);

      if( memo.size() )
         {
            xfer_op.memo = memo_data();
            xfer_op.memo->from = from_account.options.memo_key;
            xfer_op.memo->to = to_account.options.memo_key;
            xfer_op.memo->set_message(get_private_key(from_account.options.memo_key),
                                      to_account.options.memo_key, memo);
         }

      proposal_create_operation prop_op;
      auto now =  _remote_db->get_dynamic_global_properties().time;       
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;       
     // prop_op.review_period_seconds = current_params.committee_proposal_review_period;
      prop_op.fee_paying_account =  xfer_op.from ;

      prop_op.proposed_ops.emplace_back( xfer_op );
       _remote_db->get_global_properties().parameters.current_fees->set_fee( prop_op.proposed_ops.back().op );
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);


      signed_transaction tx;
      tx.operations.push_back(prop_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast,true);
   } FC_CAPTURE_AND_RETHROW( (from)(to)(amount)(asset_symbol)(memo)(broadcast) ) }

    signed_transaction transfer_vesting(string from, string to, string amount,
                               string asset_symbol, string memo, 
                               uint32_t vesting_cliff_seconds,uint32_t vesting_duration_seconds,
                               bool broadcast = false)
   { try {
      FC_ASSERT( !self.is_locked() );
      fc::optional<asset_object> asset_obj = get_asset(asset_symbol);
      FC_ASSERT(asset_obj, "Could not find asset matching ${asset}", ("asset", asset_symbol));

      account_object from_account = get_account(from);
      account_object to_account = get_account(to);
      account_id_type from_id = from_account.id;
      account_id_type to_id = get_account_id(to);

      transfer_vesting_operation xfer_op;

      xfer_op.from = from_id;
      xfer_op.to = to_id;
      xfer_op.amount = asset_obj->amount_from_string(amount);
      xfer_op.vesting_cliff_seconds = vesting_cliff_seconds;
      xfer_op.vesting_duration_seconds = vesting_duration_seconds;
  
      if( memo.size() )
      {
         xfer_op.memo = memo_data();
         xfer_op.memo->from = from_account.options.memo_key;
         xfer_op.memo->to = to_account.options.memo_key;
         xfer_op.memo->set_message(get_private_key(from_account.options.memo_key),
         to_account.options.memo_key, memo);
      }

      signed_transaction tx;
      tx.operations.push_back(xfer_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast);
   } FC_CAPTURE_AND_RETHROW( (from)(to)(amount)(asset_symbol)(memo)(broadcast) ) }

   signed_transaction issue_asset(string to_account, string amount, string symbol,
                                  string memo, bool broadcast = false)
   {
      auto asset_obj = get_asset(symbol);
      FC_ASSERT( !self.is_locked() );
      account_object to = get_account(to_account);
      account_object issuer = get_account(asset_obj.issuer);

      asset_issue_operation issue_op;
      issue_op.issuer           = asset_obj.issuer;
      issue_op.asset_to_issue   = asset_obj.amount_from_string(amount);
      issue_op.issue_to_account = to.id;

      if( memo.size() )
      {
         issue_op.memo = memo_data();
         issue_op.memo->from = issuer.options.memo_key;
         issue_op.memo->to = to.options.memo_key;
         issue_op.memo->set_message(get_private_key(issuer.options.memo_key),
                                    to.options.memo_key, memo);
         FC_ASSERT(issue_op.memo->nonce != unenc_nonce);                           
      }

      signed_transaction tx;
      tx.operations.push_back(issue_op);
      set_operation_fees(tx,_remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast);
   }

   std::map<string,std::function<string(fc::variant,const fc::variants&)>> get_result_formatters() const
   {
      std::map<string,std::function<string(fc::variant,const fc::variants&)> > m;
      m["help"] = [](variant result, const fc::variants& a)
      {
         return result.get_string();
      };

      m["gethelp"] = [](variant result, const fc::variants& a)
      {
         return result.get_string();
      };

      m["get_account_history"] = [this](variant result, const fc::variants& a)
      {
         auto r = result.as<vector<operation_detail>>( GRAPHENE_MAX_NESTED_OBJECTS );
         std::stringstream ss;

         for( operation_detail& d : r )
         {
            operation_history_object& i = d.op;
            auto b = _remote_db->get_block_header(i.block_num);
            FC_ASSERT(b);
            ss << b->timestamp.to_iso_string() << " ";
            i.op.visit(operation_printer(ss, *this, i.result));
            ss << " \n";
         }

         return ss.str();
      };
      m["get_relative_account_history"] = [this](variant result, const fc::variants& a)
      {
         auto r = result.as<vector<operation_detail>>( GRAPHENE_MAX_NESTED_OBJECTS );
         std::stringstream ss;

         for( operation_detail& d : r )
         {
            operation_history_object& i = d.op;
            auto b = _remote_db->get_block_header(i.block_num);
            FC_ASSERT(b);
            ss << b->timestamp.to_iso_string() << " ";
            i.op.visit(operation_printer(ss, *this, i.result));
            ss << " \n";
         }

         return ss.str();
      };
    

      m["get_account_history_by_operations"] = [this](variant result, const fc::variants& a) {
          auto r = result.as<account_history_operation_detail>( GRAPHENE_MAX_NESTED_OBJECTS );
          std::stringstream ss;
          ss << "total_count : ";
          ss << r.total_count;
          ss << " \n";
          ss << "result_count : ";
          ss << r.result_count;
          ss << " \n";
          for (operation_detail_ex& d : r.details) {
              operation_history_object& i = d.op;
              auto b = _remote_db->get_block_header(i.block_num);
              FC_ASSERT(b);
              ss << b->timestamp.to_iso_string() << " ";
              i.op.visit(operation_printer(ss, *this, i.result));
              ss << " transaction_id : ";
              ss << d.transaction_id.str();
              ss << " block_num:" <<i.block_num;
              ss << " trx_index:" <<i.trx_in_block;
              ss << " \n";
          }

          return ss.str();
      };

      m["list_account_balances"] = [this](variant result, const fc::variants& a)
      {
         auto r = result.as<vector<asset>>( GRAPHENE_MAX_NESTED_OBJECTS );
         vector<asset_object> asset_recs;
         std::transform(r.begin(), r.end(), std::back_inserter(asset_recs), [this](const asset& a) {
            return get_asset(a.asset_id);
         });

         std::stringstream ss;
         for( unsigned i = 0; i < asset_recs.size(); ++i )
            ss << asset_recs[i].amount_to_pretty_string(r[i]) << "\n";

         return ss.str();
      };

      m["get_blind_balances"] = [this](variant result, const fc::variants& a)
      {
         auto r = result.as<vector<asset>>( GRAPHENE_MAX_NESTED_OBJECTS );
         vector<asset_object> asset_recs;
         std::transform(r.begin(), r.end(), std::back_inserter(asset_recs), [this](const asset& a) {
            return get_asset(a.asset_id);
         });

         std::stringstream ss;
         for( unsigned i = 0; i < asset_recs.size(); ++i )
            ss << asset_recs[i].amount_to_pretty_string(r[i]) << "\n";

         return ss.str();
      };
      m["transfer_to_blind"] = [this](variant result, const fc::variants& a)
      {
         auto r = result.as<blind_confirmation>( GRAPHENE_MAX_NESTED_OBJECTS );
         std::stringstream ss;
         r.trx.operations[0].visit( operation_printer( ss, *this, operation_result() ) );
         ss << "\n";
         for( const auto& out : r.outputs )
         {
            asset_object a = get_asset( out.decrypted_memo.amount.asset_id );
            ss << a.amount_to_pretty_string( out.decrypted_memo.amount ) << " to  " << out.label << "\n\t  receipt: " << out.confirmation_receipt <<"\n\n";
         }
         return ss.str();
      };
      m["blind_transfer"] = [this](variant result, const fc::variants& a)
      {
         auto r = result.as<blind_confirmation>( GRAPHENE_MAX_NESTED_OBJECTS );
         std::stringstream ss;
         r.trx.operations[0].visit( operation_printer( ss, *this, operation_result() ) );
         ss << "\n";
         for( const auto& out : r.outputs )
         {
            asset_object a = get_asset( out.decrypted_memo.amount.asset_id );
            ss << a.amount_to_pretty_string( out.decrypted_memo.amount ) << " to  " << out.label << "\n\t  receipt: " << out.confirmation_receipt <<"\n\n";
         }
         return ss.str();
      };
      m["receive_blind_transfer"] = [this](variant result, const fc::variants& a)
      {
         auto r = result.as<blind_receipt>( GRAPHENE_MAX_NESTED_OBJECTS );
         std::stringstream ss;
         asset_object as = get_asset( r.amount.asset_id );
         ss << as.amount_to_pretty_string( r.amount ) << "  " << r.from_label << "  =>  " << r.to_label  << "  " << r.memo <<"\n";
         return ss.str();
      };
      m["blind_history"] = [this](variant result, const fc::variants& a)
      {
         auto records = result.as<vector<blind_receipt>>( GRAPHENE_MAX_NESTED_OBJECTS );
         std::stringstream ss;
         ss << "WHEN         "
            << "  " << "AMOUNT"  << "  " << "FROM" << "  =>  " << "TO" << "  " << "MEMO" <<"\n";
         ss << "====================================================================================\n";
         for( auto& r : records )
         {
            asset_object as = get_asset( r.amount.asset_id );
            ss << fc::get_approximate_relative_time_string( r.date )
               << "  " << as.amount_to_pretty_string( r.amount ) << "  " << r.from_label << "  =>  " << r.to_label  << "  " << r.memo <<"\n";
            ss << (string)r.conf<< "\n";
         }
         return ss.str();
      };
      m["get_order_book"] = [&](variant result, const fc::variants& a)
      {
         auto orders = result.as<order_book>( GRAPHENE_MAX_NESTED_OBJECTS );
         auto bids = orders.bids;
         auto asks = orders.asks;
         std::stringstream ss;
         std::stringstream sum_stream;
         sum_stream << "Sum(" << orders.base << ')';
         double bid_sum = 0;
         double ask_sum = 0;
         const int spacing = 20;

         auto prettify_num = [&ss]( double n )
         {
            if (abs( round( n ) - n ) < 0.00000000001 )
            {
               ss << (int) n;
            }
            else if (n - floor(n) < 0.000001)
            {
               ss << setiosflags( ios::fixed ) << setprecision(10) << n;
            }
            else
            {
               ss << setiosflags( ios::fixed ) << setprecision(6) << n;
            }
         };
         auto prettify_num_string = [&]( string& num_string )
         {
            double n = fc::to_double( num_string );
            prettify_num( n );
         };

         ss << setprecision( 8 ) << setiosflags( ios::fixed ) << setiosflags( ios::left );

         ss << ' ' << setw( (spacing * 4) + 6 ) << "BUY ORDERS" << "SELL ORDERS\n"
            << ' ' << setw( spacing + 1 ) << "Price" << setw( spacing ) << orders.quote << ' ' << setw( spacing )
            << orders.base << ' ' << setw( spacing ) << sum_stream.str()
            << "   " << setw( spacing + 1 ) << "Price" << setw( spacing ) << orders.quote << ' ' << setw( spacing )
            << orders.base << ' ' << setw( spacing ) << sum_stream.str()
            << "\n====================================================================================="
            << "|=====================================================================================\n";

         for (unsigned int i = 0; i < bids.size() || i < asks.size() ; i++)
         {
            if ( i < bids.size() )
            {
                bid_sum += fc::to_double( bids[i].base );
                ss << ' ' << setw( spacing );
                prettify_num_string( bids[i].price );
                ss << ' ' << setw( spacing );
                prettify_num_string( bids[i].quote );
                ss << ' ' << setw( spacing );
                prettify_num_string( bids[i].base );
                ss << ' ' << setw( spacing );
                prettify_num( bid_sum );
                ss << ' ';
            }
            else
            {
                ss << setw( (spacing * 4) + 5 ) << ' ';
            }

            ss << '|';

            if ( i < asks.size() )
            {
               ask_sum += fc::to_double( asks[i].base );
               ss << ' ' << setw( spacing );
               prettify_num_string( asks[i].price );
               ss << ' ' << setw( spacing );
               prettify_num_string( asks[i].quote );
               ss << ' ' << setw( spacing );
               prettify_num_string( asks[i].base );
               ss << ' ' << setw( spacing );
               prettify_num( ask_sum );
            }

            ss << '\n';
         }

         ss << endl
            << "Buy Total:  " << bid_sum << ' ' << orders.base << endl
            << "Sell Total: " << ask_sum << ' ' << orders.base << endl;

         return ss.str();
      };

      return m;
   }

   signed_transaction propose_zosparameter_change(
      const string& proposing_account,
      fc::time_point_sec expiration_time,
      const variant_object& changed_values,
      optional<string> memo,
      bool broadcast = false)
   {      
      FC_ASSERT( !self.is_locked() );
      const zos_parameters& current_params = get_global_properties().zosparameters;
      const chain_parameters& params = get_global_properties().parameters;
      zos_parameters new_params = current_params;
      fc::reflector<zos_parameters>::visit(
         fc::from_variant_visitor<zos_parameters>( changed_values, new_params, GRAPHENE_MAX_NESTED_OBJECTS )
         );

      fc::variant result;
      fc::to_variant( new_params, result, GRAPHENE_NET_MAX_NESTED_OBJECTS );
      committee_member_update_zos_parameters_operation update_op;
      update_op.new_parameters = fc::json::to_string(result);

      proposal_create_operation prop_op;

      prop_op.expiration_time = expiration_time;
      prop_op.review_period_seconds = params.committee_proposal_review_period;
      prop_op.fee_paying_account = get_account(proposing_account).id;
      prop_op.memo = memo;

      prop_op.proposed_ops.emplace_back( update_op );
      set_proposa_fees( prop_op, params.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);

      signed_transaction tx;
      tx.operations.push_back(prop_op);
      set_operation_fees(tx, params.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast,true);
   }
   signed_transaction   locktoken_remove(
        const string   issuer,      
        const locktoken_id_type  id,              
        bool broadcast) 
   { try {
      FC_ASSERT( !self.is_locked() );
      account_object from_account = get_account(issuer);

      locktoken_remove_operation xfer_op;

      xfer_op.issuer = from_account.id; 
      xfer_op.locktoken_id = id; 
     
      signed_transaction tx;
      tx.operations.push_back(xfer_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast);
   } FC_CAPTURE_AND_RETHROW( (issuer)(id)) } 

   signed_transaction locktoken_update(
        const string   issuer,      
        const locktoken_id_type locktoken_id,      
        const uint32_t   op_type,      
        const string   amount,
        const string   asset_symbol,        
        const uint32_t type,
        const uint32_t period,        
        const uint32_t autolock, 
        bool broadcast)
    { try {
      FC_ASSERT( !self.is_locked() );

      fc::optional<asset_object> asset_obj = get_asset(asset_symbol);
      FC_ASSERT(asset_obj, "Could not find asset matching ${asset}", ("asset", asset_symbol));

      account_object from_account = get_account(issuer);
      account_id_type from_id = from_account.id;;

      locktoken_update_operation xfer_op;

      xfer_op.issuer = from_id; 
      xfer_op.locked = asset_obj->amount_from_string(amount); 
      xfer_op.locktoken_id = locktoken_id;     
      xfer_op.op_type = op_type;     
      xfer_op.period = period; 
      xfer_op.type = type;
      xfer_op.autolock = autolock;       
     
      signed_transaction tx;
      tx.operations.push_back(xfer_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast);
   } FC_CAPTURE_AND_RETHROW( (issuer)(locktoken_id)(op_type)(amount)(asset_symbol)(type)(period)(autolock)) }     

   signed_transaction locktoken_create(
        const string   issuer,      
        const string   to,      
        const string   amount,
        const string   asset_symbol,        
        const uint32_t type,
        const uint32_t period,        
        const uint32_t autolock, 
        bool broadcast)
   { try {
      FC_ASSERT( !self.is_locked() );

      fc::optional<asset_object> asset_obj = get_asset(asset_symbol);
      FC_ASSERT(asset_obj, "Could not find asset matching ${asset}", ("asset", asset_symbol));

      account_object from_account = get_account(issuer);
      account_object to_account = get_account(to);
      account_id_type from_id = from_account.id;
      account_id_type to_id = get_account_id(to);

      locktoken_create_operation xfer_op;

      xfer_op.issuer = from_id; 
      xfer_op.locked = asset_obj->amount_from_string(amount);; 
      xfer_op.to = to_id;     
      xfer_op.period = period; 
      xfer_op.type = type;
      xfer_op.autolock = autolock;       
     
      signed_transaction tx;
      tx.operations.push_back(xfer_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast);
   } FC_CAPTURE_AND_RETHROW( (issuer)(to)(amount)(asset_symbol)(type)(period)(autolock)) }
   
   signed_transaction locktoken_options_param(
      const string& issuer,            
      const string& asset_lock,    
      const uint32_t& lock_mode, 
      const variant_object& changed_values,
      bool broadcast = false)
   {      
      FC_ASSERT( !self.is_locked() );
      FC_ASSERT(lock_mode == locktoken_dy || lock_mode == locktoken_fixed || lock_mode == locktoken_node);
      account_object account = get_account(issuer);
      asset_object   asset   = get_asset(asset_lock);
      const chain_parameters& params = get_global_properties().parameters;
      FC_ASSERT(asset.uasset_property & (ASSET_DYFIXED | ASSET_LOCKNODE));
      if(account.id != asset.issuer) {
         FC_ASSERT(account.is_committee_member());
      }
      uint32_t op_type = 0;
      locktoken_option_id_type op_id;
      locktoken_options new_params;
      {
         fc::optional<locktoken_option_object> iter = _remote_db->get_locktoken_option(asset.id,lock_mode,false);
         if(iter.valid()) {
            new_params = iter->lockoptions;
            op_id = iter->id;
            op_type = 0;
         } else {
            iter = _remote_db->get_locktoken_option(GRAPHENE_CORE_ASSET,lock_mode,false);
            if(iter.valid()) {
              new_params = iter->lockoptions;
              new_params.payer = GRAPHENE_EMPTY_ACCOUNT;
              new_params.carrier = GRAPHENE_EMPTY_ACCOUNT;
            } 
            op_type = 1;
         }        
      }

      fc::reflector<locktoken_options>::visit(
          fc::from_variant_visitor<locktoken_options>(changed_values, new_params, GRAPHENE_MAX_NESTED_OBJECTS));      
          
      new_params.version = locktoken_options_version;
      new_params.lock_mode = lock_mode;
      new_params.op_id = op_id;

      new_params.validate();

      signed_transaction tx;
  
      fc::variant result;
      fc::to_variant( new_params, result, GRAPHENE_NET_MAX_NESTED_OBJECTS );
      locktoken_option_operation update_op;
      update_op.sParam = fc::json::to_string(result);
      update_op.issuer = account.id;
      update_op.asset_lock = asset.id;
      update_op.op_type = op_type;

      if(asset.issuer !=  account.id) {
        proposal_create_operation prop_op; 
        auto now =  _remote_db->get_dynamic_global_properties().time;       
        prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;;
        prop_op.review_period_seconds = params.committee_proposal_review_period;
        prop_op.fee_paying_account = account.id;       
        update_op.issuer = asset.issuer;
        if(update_op.issuer == GRAPHENE_NULL_ACCOUNT)
           update_op.issuer = GRAPHENE_COMMITTEE_ACCOUNT;

        prop_op.proposed_ops.emplace_back( update_op );
        set_proposa_fees( prop_op, params.current_fees);
        _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);

        tx.operations.push_back(prop_op);
      } else {
        tx.operations.push_back(update_op);
      } 

      set_operation_fees(tx, params.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast,true);
   }

   signed_transaction locktoken_options_enablepayer(
      const string& issuer,            
      const string& asset_lock,    
      const uint32_t& lock_mode,
      bool broadcast )
   {      
      FC_ASSERT( !self.is_locked() );
      FC_ASSERT(lock_mode == locktoken_dy || lock_mode == locktoken_fixed || lock_mode == locktoken_node);
      account_object account = get_account(issuer);
      asset_object   asset   = get_asset(asset_lock);
      const chain_parameters& params = get_global_properties().parameters;
      FC_ASSERT(asset.uasset_property & (ASSET_DYFIXED | ASSET_LOCKNODE));
      
      
      locktoken_options new_params;
      
      fc::optional<locktoken_option_object> iter = _remote_db->get_locktoken_option(asset.id,lock_mode,false);
      FC_ASSERT(iter.valid());
      new_params = iter->lockoptions; 

      new_params.version = locktoken_options_version;
      new_params.lock_mode = lock_mode;
      new_params.op_id = iter->id;
      new_params.payer = account.id;

      new_params.validate();

      signed_transaction tx;
  
      fc::variant result;
      fc::to_variant( new_params, result, GRAPHENE_NET_MAX_NESTED_OBJECTS );
      locktoken_option_operation update_op;
      update_op.sParam = fc::json::to_string(result);
      update_op.issuer = account.id;
      update_op.asset_lock = asset.id;
      update_op.op_type = 2;

      tx.operations.push_back(update_op);      

      set_operation_fees(tx, params.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast,true);
   }


   signed_transaction propose_parameter_change(
      const string& proposing_account,
      fc::time_point_sec expiration_time,
      const variant_object& changed_values,
      optional<string> memo,
      bool broadcast = false)
   {
      FC_ASSERT( !changed_values.contains("current_fees") );
      FC_ASSERT( !self.is_locked() );
      const chain_parameters& current_params = get_global_properties().parameters;
      chain_parameters new_params = current_params;
      fc::reflector<chain_parameters>::visit(
         fc::from_variant_visitor<chain_parameters>( changed_values, new_params, GRAPHENE_MAX_NESTED_OBJECTS )
         );

      committee_member_update_global_parameters_operation update_op;
      update_op.new_parameters = new_params;

      proposal_create_operation prop_op;

      prop_op.expiration_time = expiration_time;
      prop_op.review_period_seconds = current_params.committee_proposal_review_period;
      prop_op.fee_paying_account = get_account(proposing_account).id;
      prop_op.memo = memo;

      prop_op.proposed_ops.emplace_back( update_op );
      current_params.current_fees->set_fee( prop_op.proposed_ops.back().op );
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);

      signed_transaction tx;
      tx.operations.push_back(prop_op);
      set_operation_fees(tx, current_params.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast,true);
   }
  
   signed_transaction propose_fee_change(
      const string& proposing_account,
      fc::time_point_sec expiration_time,
      const variant_object& changed_fees,
      optional<string> memo,
      bool broadcast = false)
   {

 FC_ASSERT( !self.is_locked() );
      const chain_parameters& current_params = get_global_properties().parameters;
      const fee_schedule_type& current_fees = *(current_params.current_fees);

      flat_map< int, fee_parameters > fee_map;
      fee_map.reserve( current_fees.parameters.size() );
      for( const fee_parameters& op_fee : current_fees.parameters )
         fee_map[ op_fee.which() ] = op_fee;
      uint32_t scale = current_fees.scale;

      for( const auto& item : changed_fees )
      {
         const string& key = item.key();
         if( key == "scale" )
         {
            int64_t _scale = item.value().as_int64();
            FC_ASSERT( _scale >= 0 );
            FC_ASSERT( _scale <= std::numeric_limits<uint32_t>::max() );
            scale = uint32_t( _scale );
            continue;
         }
         // is key a number?
         auto is_numeric = [&key]() -> bool
         {
            size_t n = key.size();
            for( size_t i=0; i<n; i++ )
            {
               if( !isdigit( key[i] ) )
                  return false;
            }
            return true;
         };

         int which;
         if( is_numeric() )
            which = std::stoi( key );
         else
         {
            const auto& n2w = _operation_which_map.name_to_which;
            auto it = n2w.find( key );
            FC_ASSERT( it != n2w.end(), "unknown operation" );
            which = it->second;
         }

         fee_parameters fp = from_which_variant< fee_parameters >( which, item.value(), GRAPHENE_MAX_NESTED_OBJECTS );
         fee_map[ which ] = fp;
      }

      fee_schedule_type new_fees;

      for( const std::pair< int, fee_parameters >& item : fee_map )
         new_fees.parameters.insert( item.second );
      new_fees.scale = scale;

      chain_parameters new_params = current_params;
      new_params.current_fees = new_fees;

      committee_member_update_global_parameters_operation update_op;
      update_op.new_parameters = new_params;

      proposal_create_operation prop_op;

      prop_op.expiration_time = expiration_time;
      prop_op.review_period_seconds = current_params.committee_proposal_review_period;
      prop_op.fee_paying_account = get_account(proposing_account).id;
      prop_op.memo = memo;

      prop_op.proposed_ops.emplace_back( update_op );
      current_params.current_fees->set_fee( prop_op.proposed_ops.back().op );
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);

      signed_transaction tx;
      tx.operations.push_back(prop_op);
      set_operation_fees(tx, current_params.current_fees);
      tx.validate();

      return sign_transaction(tx, broadcast,true);
   }
  
   signed_transaction propose_asset_bitlender_property(
         const string& proposing_account,
         const string& symbol,
         uint32_t      ubitproperty,  
         uint32_t      umask,         
         bool          broadcast)
    { try
     {      
         FC_ASSERT( !self.is_locked() );
      uint32_t umaskp = ~umask;
      FC_ASSERT((ubitproperty & umaskp) == 0);
      asset_property_operation update_op;
      update_op.issuer   = get_account(proposing_account).id;
      update_op.uasset_property = ubitproperty;
      update_op.uasset_mask     = umask;
      update_op.asset_id = get_asset_id(symbol);

      update_op.validate();

      auto now =  _remote_db->get_dynamic_global_properties().time;
      proposal_create_operation prop_op;
      prop_op.expiration_time = fc::time_point_sec(now)-PRO_DELAY + _remote_db->get_global_properties().parameters.maximum_proposal_lifetime;
      prop_op.review_period_seconds = _remote_db->get_global_properties().parameters.committee_proposal_review_period;
      prop_op.fee_paying_account = update_op.issuer;
      prop_op.proposed_ops.emplace_back( update_op );
      set_proposa_fees( prop_op, _remote_db->get_global_properties().parameters.current_fees);
      _remote_db->validate_opertation(prop_op.proposed_ops.begin()->op,true);
    
      signed_transaction tx;
      tx.operations.push_back(prop_op);
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees);
      
      tx.validate();
      return sign_transaction(tx, broadcast,true);
      
     }FC_CAPTURE_AND_RETHROW( (proposing_account)(symbol)(ubitproperty)(umask)(broadcast))
   }
   signed_transaction remove_proposal(
      const string& fee_paying_account,
      const string& proposal_id,   
      bool  using_owner_authority ,  
      bool broadcast = false)
   {
       try
     { 
         FC_ASSERT( !self.is_locked() );
      proposal_delete_operation update_op;

      update_op.fee_paying_account = get_account(fee_paying_account).id;      
      update_op.using_owner_authority = false;       
      update_op.proposal =  fc::variant(proposal_id, 1).as<proposal_id_type>( 1 );

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees(tx, get_global_properties().parameters.current_fees);
      tx.validate();
      return sign_transaction(tx, broadcast);
     }FC_CAPTURE_AND_RETHROW( (fee_paying_account)(proposal_id)(using_owner_authority)(broadcast))
   }


   signed_transaction approve_proposal(
      const string& fee_paying_account,
      const string& proposal_id,
      const approval_delta& delta,
      bool broadcast = false)
   {
       try
     { 
         FC_ASSERT( !self.is_locked() );
      proposal_update_operation update_op;

      update_op.fee_paying_account = get_account(fee_paying_account).id;      
      update_op.proposal = fc::variant(proposal_id, 1).as<proposal_id_type>( 1 );
      if(update_op.proposal == proposal_id_type(0))
      {
         uint64_t num = _remote_db->get_object_count(proposal_id_type(0),true);
         FC_ASSERT(num> 1);
         update_op.proposal = proposal_id_type(num-1);          
      }
      // make sure the proposal exists
      get_object( update_op.proposal );
      std::cerr << "approve_proposal:  " << update_op.proposal.to_string() << "\n";

      for( const std::string& name : delta.active_approvals_to_add )
         update_op.active_approvals_to_add.insert( get_account( name ).id );
      for( const std::string& name : delta.active_approvals_to_remove )
         update_op.active_approvals_to_remove.insert( get_account( name ).id );
      for( const std::string& name : delta.owner_approvals_to_add )
         update_op.owner_approvals_to_add.insert( get_account( name ).id );
      for( const std::string& name : delta.owner_approvals_to_remove )
         update_op.owner_approvals_to_remove.insert( get_account( name ).id );
      for( const std::string& k : delta.key_approvals_to_add )
         update_op.key_approvals_to_add.insert( public_key_type( k ) );
      for( const std::string& k : delta.key_approvals_to_remove )
         update_op.key_approvals_to_remove.insert( public_key_type( k ) );

      signed_transaction tx;
      tx.operations.push_back(update_op);
      set_operation_fees(tx, get_global_properties().parameters.current_fees);
      tx.validate();
      return sign_transaction(tx, broadcast,true);
     }FC_CAPTURE_AND_RETHROW( (fee_paying_account)(proposal_id)(delta)(broadcast))
   }
 

   void dbg_make_uia(string creator, string symbol)
   {
      asset_options opts;
      opts.flags &= ~(white_list | disable_force_settle | global_settle);
      opts.issuer_permissions = opts.flags;
      opts.core_exchange_rate = price(asset(1,GRAPHENE_CORE_ASSET), asset(1,asset_id_type(1)));
      create_asset(get_account(creator).name, symbol, 2, opts, {}, true);
   }

   void dbg_make_mia(string creator, string symbol)
   {
      asset_options opts;
      opts.flags &= ~white_list;
      opts.issuer_permissions = opts.flags;
      opts.core_exchange_rate = price(asset(1,GRAPHENE_CORE_ASSET), asset(1,asset_id_type(1)));
      bitasset_options bopts;
      create_asset(get_account(creator).name, symbol, 2, opts, bopts,true);
   }

   void dbg_push_blocks( const std::string& src_filename, uint32_t count )
   {
      use_debug_api();
      (*_remote_debug)->debug_push_blocks( src_filename, count );
      (*_remote_debug)->debug_stream_json_objects_flush();
   }

   void dbg_generate_blocks( const std::string& debug_wif_key, uint32_t count )
   {
      use_debug_api();
      (*_remote_debug)->debug_generate_blocks( debug_wif_key, count );
      (*_remote_debug)->debug_stream_json_objects_flush();
   }

   void dbg_stream_json_objects( const std::string& filename )
   {
      use_debug_api();
      (*_remote_debug)->debug_stream_json_objects( filename );
      (*_remote_debug)->debug_stream_json_objects_flush();
   }

   void dbg_update_object( const fc::variant_object& update )
   {
      use_debug_api();
      (*_remote_debug)->debug_update_object( update );
      (*_remote_debug)->debug_stream_json_objects_flush();
   }

   void use_network_node_api()
   {
      if( _remote_net_node )
         return;
      try
      {
         _remote_net_node = _remote_api->network_node();
      }
      catch( const fc::exception& e )
      {
         std::cerr << "\nCouldn't get network node API.  You probably are not configured\n"
         "to access the network API on the witness_node you are\n"
         "connecting to.  Please follow the instructions in README.md to set up an apiaccess file.\n"
         "\n";
         throw;
      }
   }

   void use_debug_api()
   {
      if( _remote_debug )
         return;
      try
      {
        _remote_debug = _remote_api->debug();
      }
      catch( const fc::exception& e )
      {
         std::cerr << "\nCouldn't get debug node API.  You probably are not configured\n"
         "to access the debug API on the node you are connecting to.\n"
         "\n"
         "To fix this problem:\n"
         "- Please ensure you are running debug_node, not witness_node.\n"
         "- Please follow the instructions in README.md to set up an apiaccess file.\n"
         "\n";
      }
   }

   void network_add_nodes( const vector<string>& nodes )
   {
      use_network_node_api();
      for( const string& node_address : nodes )
      {
         (*_remote_net_node)->add_node( fc::ip::endpoint::from_string( node_address ) );
      }
   }
   vector< variant > network_get_potential_peers()
   {
      use_network_node_api();
      const auto peers = (*_remote_net_node)->get_potential_peers();
      vector< variant > result;
      result.reserve( peers.size() );
      for( const auto& peer : peers )
      {
         variant v;
         fc::to_variant( peer, v, GRAPHENE_MAX_NESTED_OBJECTS );
         result.push_back( v );
      }
      return result;
   }
   vector< variant > network_get_connected_peers()
   {
      use_network_node_api();
      const auto peers = (*_remote_net_node)->get_connected_peers();
      vector< variant > result;
      result.reserve( peers.size() );
      for( const auto& peer : peers )
      {
         variant v;
         fc::to_variant( peer, v, GRAPHENE_MAX_NESTED_OBJECTS );
         result.push_back( v );
      }
      return result;
   }
   
   fc::variant_object  network_get_advanced_node_parameters()  
   { try{
     use_network_node_api();
    return (*_remote_net_node)->get_advanced_node_parameters();
   } FC_CAPTURE_AND_RETHROW() }

   void  network_set_advanced_node_parameters(const fc::variant_object& params)
   { try{
    use_network_node_api();
    (*_remote_net_node)->set_advanced_node_parameters(params);
   }  FC_CAPTURE_AND_RETHROW((params)) }

   void flood_network(string prefix, uint32_t number_of_transactions)
   {
      try
      {
         const account_object& master = *_wallet.my_accounts.get<by_name>().lower_bound("import");
         int number_of_accounts = number_of_transactions / 3;
         number_of_transactions -= number_of_accounts;
         try {
            dbg_make_uia(master.name, "SHILL");
         } catch(...) {/* Ignore; the asset probably already exists.*/}

         fc::time_point start = fc::time_point::now();
         for( int i = 0; i < number_of_accounts; ++i )
         {
            std::ostringstream brain_key;
            brain_key << "brain key for account " << prefix << i;
            signed_transaction trx = create_account_with_brain_key(brain_key.str(), prefix + fc::to_string(i), master.name, master.name, /* broadcast = */ true, /* save wallet = */ false);
         }
         fc::time_point end = fc::time_point::now();
         ilog("Created ${n} accounts in ${time} milliseconds",
              ("n", number_of_accounts)("time", (end - start).count() / 1000));

         start = fc::time_point::now();
         for( int i = 0; i < number_of_accounts; ++i )
         {
            signed_transaction trx = transfer(master.name, prefix + fc::to_string(i), "10", "CORE", "", true);
            trx = transfer(master.name, prefix + fc::to_string(i), "1", "CORE", "", true);
         }
         end = fc::time_point::now();
         ilog("Transferred to ${n} accounts in ${time} milliseconds",
              ("n", number_of_accounts*2)("time", (end - start).count() / 1000));

         start = fc::time_point::now();
         for( int i = 0; i < number_of_accounts; ++i )
         {
            signed_transaction trx = issue_asset(prefix + fc::to_string(i), "1000", "SHILL", "", true);
         }
         end = fc::time_point::now();
         ilog("Issued to ${n} accounts in ${time} milliseconds",
              ("n", number_of_accounts)("time", (end - start).count() / 1000));
      }
      catch (...)
      {
         throw;
      }

   }

   operation get_prototype_operation( string operation_name )
   {
      auto it = _prototype_ops.find( operation_name );
      if( it == _prototype_ops.end() )
         FC_THROW("Unsupported operation: \"${operation_name}\"", ("operation_name", operation_name));
      return it->second;
   }

 
   flat_map<string, operation> get_prototype_operations()
   {
     return _prototype_ops;
   }

   string                  _wallet_filename;
   wallet_data             _wallet;
   bool                    _bautosave;   
   variant                 _trx_return;

   map<public_key_type,string> _keys;
   fc::sha512                  _checksum;

   chain_id_type            _chain_id;
   fc::api<login_api>       _remote_api;
   fc::api<database_api>    _remote_db;
   fc::api<bitlender_api>   _remote_bitlender;
   fc::api<finance_api>     _remote_finance;
   fc::api<mobile_api>      _remote_mobile;
   fc::api<admin_api>       _remote_admin;
   fc::api<network_broadcast_api>   _remote_net_broadcast;
   fc::api<history_api>             _remote_hist;
   optional< fc::api<network_node_api> > _remote_net_node;
   optional< fc::api<graphene::debug_witness::debug_api> > _remote_debug;

   flat_map<string, operation> _prototype_ops;

   static_variant_map _operation_which_map = create_static_variant_map< operation >();

#ifdef __unix__
   mode_t                  _old_umask;
#endif
   const string _wallet_filename_extension = ".wallet";

   mutable map<asset_id_type, asset_object> _asset_cache;
};

std::string operation_printer::fee(const asset& a)const {
   out << "   (Fee: " << wallet.get_asset(a.asset_id).amount_to_pretty_string(a) << ")";
   return "";
}

template<typename T>
std::string operation_printer::operator()(const T& op)const
{
   //balance_accumulator acc;
   //op.get_balance_delta( acc, result );
   auto a = wallet.get_asset( op.fee.asset_id );
   auto payer = wallet.get_account( op.fee_payer() );

   string op_name = fc::get_typename<T>::name();
   if( op_name.find_last_of(':') != string::npos )
      op_name.erase(0, op_name.find_last_of(':')+1);
   out << op_name <<" ";
  // out << "balance delta: " << fc::json::to_string(acc.balance) <<"   ";
   out << payer.name << " fee: " << a.amount_to_pretty_string( op.fee );
   operation_result_printer rprinter(wallet);
   std::string str_result = result.visit(rprinter);
   if( str_result != "" )
   {
      out << "   result: " << str_result;
   }
   return "";
}
std::string operation_printer::operator()(const transfer_from_blind_operation& op)const
{
   auto a = wallet.get_asset( op.fee.asset_id );
   auto receiver = wallet.get_account( op.to );

   out <<  receiver.name
   << " received " << a.amount_to_pretty_string( op.amount ) << " from blinded balance";
   return "";
}
std::string operation_printer::operator()(const transfer_to_blind_operation& op)const
{
   auto fa = wallet.get_asset( op.fee.asset_id );
   auto a = wallet.get_asset( op.amount.asset_id );
   auto sender = wallet.get_account( op.from );

   out <<  sender.name
   << " sent " << a.amount_to_pretty_string( op.amount ) << " to " << op.outputs.size() << " blinded balance" << (op.outputs.size()>1?"s":"")
   << " fee: " << fa.amount_to_pretty_string( op.fee );
   return "";
}
string operation_printer::operator()(const transfer_operation& op) const
{
   out << "Transfer " << wallet.get_asset(op.amount.asset_id).amount_to_pretty_string(op.amount)
       << " from " << wallet.get_account(op.from).name << " to " << wallet.get_account(op.to).name;
   std::string memo;
   if( op.memo )
   {
      if( wallet.is_locked() )
      {
         out << " -- Unlock wallet to see memo.";
      } else {
         try {
            FC_ASSERT(wallet._keys.count(op.memo->to) || wallet._keys.count(op.memo->from), "Memo is encrypted to a key ${to} or ${from} not in this wallet.", ("to", op.memo->to)("from",op.memo->from));
            if( wallet._keys.count(op.memo->to) ) {
               auto my_key = wif_to_key(wallet._keys.at(op.memo->to));
               FC_ASSERT(my_key, "Unable to recover private key to decrypt memo. Wallet may be corrupted.");
               memo = op.memo->get_message(*my_key, op.memo->from);
               out << " -- Memo: " << memo;
            } else {
               auto my_key = wif_to_key(wallet._keys.at(op.memo->from));
               FC_ASSERT(my_key, "Unable to recover private key to decrypt memo. Wallet may be corrupted.");
               memo = op.memo->get_message(*my_key, op.memo->to);
               out << " -- Memo: " << memo;
            }
         } catch (const fc::exception& e) {
            out << " -- could not decrypt memo";
         }
      }
   }
   fee(op.fee);
   return memo;
}

std::string operation_printer::operator()(const account_create_operation& op) const
{
   out << "Create Account '" << op.name << "'";
   return fee(op.fee);
}

std::string operation_printer::operator()(const account_update_operation& op) const
{
   out << "Update Account '" << wallet.get_account(op.account).name << "'";
   return fee(op.fee);
}

std::string operation_printer::operator()(const asset_create_operation& op) const
{
   out << "Create ";
   if( op.bitasset_opts.valid() )
      out << "BitAsset ";
   else
      out << "User-Issue Asset ";
   out << "'" << op.symbol << "' with issuer " << wallet.get_account(op.issuer).name;
   return fee(op.fee);
}

std::string operation_result_printer::operator()(const void_result& x) const
{
   return "";
}

std::string operation_result_printer::operator()(const object_id_type& oid)
{
   return std::string(oid);
}

std::string operation_result_printer::operator()(const asset& a)
{
   return _wallet.get_asset(a.asset_id).amount_to_pretty_string(a);
}

}}}

namespace graphene { namespace wallet {
   vector<brain_key_info> utility::derive_owner_keys_from_brain_key(string brain_key, int number_of_desired_keys)
   {
      // Safety-check
      FC_ASSERT( number_of_desired_keys >= 1 );

      // Create as many derived owner keys as requested
      vector<brain_key_info> results;
      brain_key = graphene::wallet::detail::normalize_brain_key(brain_key);
      for (int i = 0; i < number_of_desired_keys; ++i) {
        fc::ecc::private_key priv_key = graphene::wallet::detail::derive_private_key( brain_key, i );

        brain_key_info result;
        result.brain_priv_key = brain_key;
        result.wif_priv_key = key_to_wif( priv_key );
        result.pub_key = priv_key.get_public_key();

        results.push_back(result);
      }

      return results;
   }
}}

namespace graphene { namespace wallet {

 


wallet_api::wallet_api(const wallet_data& initial_data, fc::api<login_api> rapi)
   : my(new detail::wallet_api_impl(*this, initial_data, rapi))
{
}
wallet_api::wallet_api(const wallet_data& initial_data)   
{
}


wallet_api::~wallet_api()
{
}

bool wallet_api::copy_wallet_file(string destination_filename)
{
   return my->copy_wallet_file(destination_filename);
}

optional<signed_block_with_info> wallet_api::get_block(uint64_t num)
{
   return my->_remote_db->get_block(num);
}
optional<signed_block_with_info> wallet_api::get_block_ids(uint64_t num)
{
   return my->_remote_db->get_block_ids(num);
}
processed_transaction wallet_api::get_transaction( uint64_t block_num, uint32_t trx_in_block )const
{
   return my->_remote_db->get_transaction(block_num,trx_in_block); 
}
uint64_t wallet_api::get_account_count() const
{
   return my->_remote_db->get_account_count();
}
 uint64_t wallet_api::get_object_count(object_id_type id, bool bmaxid) const
 {
     return my->_remote_db->get_object_count(id,bmaxid);
 }




vector<account_object> wallet_api::list_my_accounts()
{
   return vector<account_object>(my->_wallet.my_accounts.begin(), my->_wallet.my_accounts.end());
}

map<string,account_id_type> wallet_api::list_accounts(const string& lowerbound, uint32_t limit)
{
   return my->_remote_db->lookup_accounts(lowerbound, limit);
}

vector<asset_summary> wallet_api::list_account_balances(const string& id)
{
   if( auto real_id = detail::maybe_id<account_id_type>(id) )
      return my->_remote_db->get_account_balances(*real_id, flat_set<asset_id_type>());
   return my->_remote_db->get_account_balances(get_account(id).id, flat_set<asset_id_type>());
}

vector<asset_summary> wallet_api::get_account_balances_summary(const string& id)
{
   if( auto real_id = detail::maybe_id<account_id_type>(id) )
      return my->_remote_db->get_account_balances_summary(*real_id, 0xFFFF);
   return my->_remote_db->get_account_balances_summary(get_account(id).id, 0xFFFF);
}
 
 
asset_summary wallet_api::get_account_balance(const string& id,const string &asset_id)
{
   flat_set<asset_id_type> a;
   a.insert(get_asset(asset_id).id);
   return my->_remote_db->get_account_balances(get_account(id).id, a).front();
}
void  wallet_api::sort_balances(const string& symbol, uint32_t limit) {
   try{
      auto ass = get_asset(symbol);  
      // private_key_type pri = fc::ecc::private_key::generate_from_seed(fc::sha256::hash("ZOS chain 2018"));      
      // public_key_type pub =  pri.get_public_key();
      // string pstr = key_to_wif(pri);
      // idump((pstr)(pub));
      // public_key_type pub1 =  public_key_type("ZOS6FUsQ2hYRj1JSvabewWfUWTXyoDq6btmfLFjmXwby5GJgzEvT5");
      // FC_ASSERT(pub1 == pub);

      vector<account_balance_object_ex>  balance_index = my->_remote_db->get_sort_balances(ass.id, GRAPHENE_MAX_SHARE_SUPPLY, limit);      
      for(const auto &iter : balance_index)
      {        
         auto acc = my->get_account(iter.owner);           
         std::cerr << "    " << acc.name << "   " << acc.id.to_string();
         std::cerr << "    " << ass.amount_to_pretty_string(iter.balance) << "\n";         
      }  
} FC_CAPTURE_AND_RETHROW((symbol)(limit))} 
void  wallet_api::list_all_balances(const string& symbol)
{ try{
    flat_set<asset_id_type> find;
    if(symbol.size()>1)
    {
        auto a = get_asset(symbol);
        find.insert(a.id);
    }
    map<string, account_id_type> accounts = my->_remote_db->lookup_accounts("", 1000);
    auto ptr = accounts.begin();
    while (ptr != accounts.end())
    {
        std::cerr << ptr->first << "\n";
        vector<asset_summary> assets =  my->_remote_db->get_account_balances(ptr->second, find);
        for(auto &a : assets)
        {             
             auto aa = my->_remote_db->get_assets({a.asset_id}).front();
            if(aa && a.amount >0)
               std::cerr << "    " << aa->amount_to_pretty_string(a.amount) << "\n";
        }
        ptr++;
    }
 } FC_CAPTURE_AND_RETHROW((symbol))} 




vector<asset_object> wallet_api::list_assets(const string& lowerbound, uint32_t limit)const
{
   return my->_remote_db->list_assets( lowerbound, limit );
}

vector<operation_detail> wallet_api::get_account_history(string name, int limit)const
{
   vector<operation_detail> result;
   auto account_id = get_account(name).get_id();

   while( limit > 0 )
   {
      operation_history_id_type start;
      if( result.size() )
      {
         start = result.back().op.id;
         start = start + 1;
      }


      vector<operation_history_object> current = my->_remote_hist->get_account_history(account_id, operation_history_id_type(), std::min(100,limit), start);
      for( auto& o : current ) {
         std::stringstream ss;
         auto memo = o.op.visit(detail::operation_printer(ss, *my, o.result));
         result.push_back( operation_detail{ memo, ss.str(), o } );
      }
      if( int(current.size()) < std::min(100,limit) )
         break;
      limit -= current.size();
   }

   return result;
}
vector<balance_history>    wallet_api::get_balance_history_object(string account, vector<string> asset_name,uint32_t type,uint64_t ustart, uint64_t nlimit)
{ try{    
    flat_set<asset_id_type> a_asset;
    for (const auto &fee_asset : asset_name)
    {
        auto fee_asset_obj = get_asset(fee_asset);
        a_asset.insert(fee_asset_obj.id);
    }
    return my->_remote_hist->get_balance_history_object(get_account_id(account), a_asset,type,ustart, nlimit);
 } FC_CAPTURE_AND_RETHROW((account)(asset_name)(type)(ustart)(nlimit)) }


vector<balance_history> wallet_api::get_balance_history(string account, vector<string> asset_name,uint32_t type,uint64_t start , uint64_t nlimit)
{ try{    
    flat_set<asset_id_type> a_asset;
    for (const auto &fee_asset : asset_name)
    {
        auto fee_asset_obj = get_asset(fee_asset);
        a_asset.insert(fee_asset_obj.id);
    }
    return my->_remote_hist->get_balance_history(get_account_id(account), a_asset,type,start, nlimit);
 } FC_CAPTURE_AND_RETHROW((account)(asset_name)(type)(start)(nlimit)) }

vector<operation_detail> wallet_api::get_relative_account_history(string name, uint32_t stop, int limit, uint32_t start)const
{
   vector<operation_detail> result;
   auto account_id = get_account(name).get_id();

   const account_object& account = my->get_account(account_id);
   const account_statistics_object& stats = my->get_object(account.statistics);

   if(start == 0)
       start = stats.total_ops;
   else
      start = std::min<uint32_t>(start, stats.total_ops);

   while( limit > 0 )
   {
      vector <operation_history_object> current = my->_remote_hist->get_relative_account_history(account_id, stop, std::min<uint32_t>(100, limit), start);
      for (auto &o : current) {
         std::stringstream ss;
         auto memo = o.op.visit(detail::operation_printer(ss, *my, o.result));
         result.push_back(operation_detail{memo, ss.str(), o});
      }
      if (current.size() < std::min<uint32_t>(100, limit))
         break;
      limit -= current.size();
      start -= 100;
      if( start == 0 ) break;
   }
   return result;
}

account_history_operation_detail wallet_api::get_account_history_by_operations(string name, vector<uint16_t> operation_types, uint32_t start, int limit)
{
    account_history_operation_detail result;
    auto account_id = get_account(name).get_id();

    const auto& account = my->get_account(account_id);
    const auto& stats = my->get_object(account.statistics);

    // sequence of account_transaction_history_object start with 1
    start = start == 0 ? 1 : start;

    if (start <= stats.removed_ops) {
        start = stats.removed_ops;
        result.total_count =stats.removed_ops;
    }

    while (limit > 0 && start <= stats.total_ops) {
        uint32_t min_limit = std::min<uint32_t> (100, limit);
        auto current = my->_remote_hist->get_account_history_by_operations(account_id, operation_types, start, min_limit);
        for (auto& obj : current.operation_history_objs) {
            std::stringstream ss;
            auto memo = obj.op.visit(detail::operation_printer(ss, *my, obj.result));

            transaction_id_type transaction_id;
            auto block = get_block(obj.block_num);
            if (block.valid() && obj.trx_in_block < block->transaction_ids.size()) {
                transaction_id = block->transaction_ids[obj.trx_in_block];
            }
            result.details.push_back(operation_detail_ex{memo, ss.str(), obj, transaction_id});
        }
        result.result_count += current.operation_history_objs.size();
        result.total_count += current.total_count;

        start += current.total_count > 0 ? current.total_count : min_limit;
        limit -= current.operation_history_objs.size();
    }

    return result;
}

vector<operation_history_object> wallet_api::get_account_bitlender_history( string account,bitlender_order_id_type order ) const
{
 return my->_remote_hist->get_account_bitlender_history( get_account_id(account),order);
}
vector<bitlender_order_info>   wallet_api::get_account_loan_history( string account, fc::time_point_sec start, fc::time_point_sec end ) const
{
 return my->_remote_hist->get_account_loan_history( get_account_id(account), start, end ,0,1000);
}
vector<bitlender_invest_info>  wallet_api::get_account_invest_history( string account, fc::time_point_sec start, fc::time_point_sec end ) const
{
 return my->_remote_hist->get_account_invest_history( get_account_id(account), start, end ,0,1000);
}

vector<issue_fundraise_object>   wallet_api::get_account_issue_fundraise_history( string account, fc::time_point_sec start, fc::time_point_sec end ) const
{
 return my->_remote_hist->get_account_issue_fundraise_history( get_account_id(account), start, end );
}
vector<buy_fundraise_object>   wallet_api::get_account_buy_fundraise_history( string account, fc::time_point_sec start, fc::time_point_sec end ) const
{
 return my->_remote_hist->get_account_buy_fundraise_history( get_account_id(account), start, end );
}
vector<sell_exchange_object>   wallet_api::get_account_sell_exchange_history( string account, fc::time_point_sec start, fc::time_point_sec end ) const
{
 return my->_remote_hist->get_account_sell_exchange_history( get_account_id(account), start, end );
}
vector<buy_exchange_object>   wallet_api::get_account_buy_exchange_history( string account, fc::time_point_sec start, fc::time_point_sec end ) const
{
 return my->_remote_hist->get_account_buy_exchange_history( get_account_id(account), start, end );
}
fc::variant  wallet_api::get_object_history(  object_id_type id )const
{
 return my->_remote_hist->get_object_history( id);
}
 

vector<bucket_object> wallet_api::get_market_history( string symbol1, string symbol2, uint32_t bucket , fc::time_point_sec start, fc::time_point_sec end )const
{
   return my->_remote_hist->get_market_history( get_asset_id(symbol1), get_asset_id(symbol2), bucket, start, end );
}

vector<bitlender_history_object> wallet_api::get_bitlender_loan_history( string base, string quote, uint32_t type )const
{
   try
   {
      asset_id_type a_base  = asset_id_type(0);
      asset_id_type a_quote = asset_id_type(0);
      if(base.size() >0)
      {
         a_base = get_asset(base).id;
         type |=1;
      }
      else
        type &= ~1;
      if(quote.size() >0)
      {
         a_quote = get_asset(quote).id;
         type |=2;
      }
      else
        type &= ~2;  
     return my->_remote_hist->get_bitlender_loan_history(a_base,a_quote,type);
} FC_CAPTURE_AND_RETHROW((base)(quote)(type)) }
vector<limit_order_object> wallet_api::get_limit_orders(string a, string b, uint32_t limit)const
{
   return my->_remote_db->get_limit_orders(get_asset(a).id, get_asset(b).id, limit);
}

vector<call_order_object> wallet_api::get_call_orders(string a, uint32_t limit)const
{
   return my->_remote_db->get_call_orders(get_asset(a).id, limit);
}

vector<force_settlement_object> wallet_api::get_settle_orders(string a, uint32_t limit)const
{
   return my->_remote_db->get_settle_orders(get_asset(a).id, limit);
}

vector<collateral_bid_object> wallet_api::get_collateral_bids(string asset, uint32_t limit, uint32_t start)const
{
   return my->_remote_db->get_collateral_bids(get_asset(asset).id, limit, start);
}
vector<bitlender_order_info> wallet_api::get_notify_orders(uint32_t status,uint32_t start, uint32_t limit)const
{  try {     
    return my->_remote_db->get_notify_orders(status, start, limit);
} FC_CAPTURE_AND_RETHROW((start)(limit)) }

vector<bitlender_order_info> wallet_api::get_loan_orders(string a,uint32_t statue, uint32_t start, uint32_t limit)const
{  try {
    vector<account_id_type> a1;
    vector<uint32_t>        b1;
    if(a!="all")
      a1.push_back(get_account_id(a));
    if(statue!=0)  
      b1.push_back(statue);
    return my->_remote_db->get_loan_orders(a1,vector<asset_id_type>(),  b1, start, limit);
} FC_CAPTURE_AND_RETHROW((a)(statue)(start)(limit)) }

vector<bitlender_invest_info> wallet_api::get_invest_orders(string a,uint32_t statue, uint32_t start, uint32_t limit)const
{  try {
    vector<account_id_type> a1;
    vector<uint32_t>        b1;
   if(a!="all")
      a1.push_back(get_account_id(a));
    if(statue!=0)  
      b1.push_back(statue);      
   return my->_remote_db->get_invest_orders(a1,vector<asset_id_type>(), b1,start, limit );
} FC_CAPTURE_AND_RETHROW((a)(statue)(start)(limit)) }
market_ticker wallet_api::get_ticker( const string& base, const string& quote, bool ismarket)const
{ try {
    if(ismarket)
      return my->_remote_db->get_ticker(base,quote);
    else   
      return my->_remote_db->get_finance_ticker(base,quote);

} FC_CAPTURE_AND_RETHROW((base)(quote)(ismarket)) }

 vector<account_key>  wallet_api::suggest_account_key(string name, string password) const
 {
     try{  
        FC_ASSERT(password.size()>=40,"password size must >=40");
        return my->suggest_account_key( name,  password);         
 } FC_CAPTURE_AND_RETHROW((name)(password))}

 brain_key_info wallet_api::suggest_brain_key() const
 {
     brain_key_info result;
     // create a private key for secure entropy
     fc::sha256 sha_entropy1 = fc::ecc::private_key::generate().get_secret();
     fc::sha256 sha_entropy2 = fc::ecc::private_key::generate().get_secret();
     fc::bigint entropy1(sha_entropy1.data(), sha_entropy1.data_size());
     fc::bigint entropy2(sha_entropy2.data(), sha_entropy2.data_size());
     fc::bigint entropy(entropy1);
     entropy <<= 8 * sha_entropy1.data_size();
     entropy += entropy2;
     string brain_key = "";

     for (int i = 0; i < BRAIN_KEY_WORD_COUNT; i++)
     {
         fc::bigint choice = entropy % graphene::words::word_list_size;
         entropy /= graphene::words::word_list_size;
         if (i > 0)
             brain_key += " ";
         brain_key += graphene::words::word_list[choice.to_int64()];
     }

     brain_key = normalize_brain_key(brain_key);
     fc::ecc::private_key priv_key = derive_private_key(brain_key, 0);
     result.brain_priv_key = brain_key;
     result.wif_priv_key = key_to_wif(priv_key);
     result.pub_key = priv_key.get_public_key();
     result.add_ress = address(result.pub_key);
     return result;
}

vector<brain_key_info> wallet_api::derive_owner_keys_from_brain_key(string brain_key, int number_of_desired_keys) const
{
   return graphene::wallet::utility::derive_owner_keys_from_brain_key(brain_key, number_of_desired_keys);
}

bool wallet_api::is_public_key_registered(string public_key) const
{
   bool is_known = my->_remote_db->is_public_key_registered(public_key);
   return is_known;
}


string wallet_api::serialize_transaction( signed_transaction tx )const
{
   return fc::to_hex(fc::raw::pack(tx));
}

variant wallet_api::get_object( object_id_type id ) const
{
   return my->_remote_db->get_objects({id});
}
vector<proposal_object> wallet_api::get_proposed_transaction( account_id_type id ) const
{
   return my->_remote_db->get_proposed_transactions(id);
}   
vector<proposal_object> wallet_api::get_my_proposed_transaction( account_id_type id) const
{
   return my->_remote_db->get_my_proposed_transactions(id);
}   

string wallet_api::get_wallet_filename() const
{
   return my->get_wallet_filename();
}

transaction_handle_type wallet_api::begin_builder_transaction()
{
   return my->begin_builder_transaction();
}

void wallet_api::add_operation_to_builder_transaction(transaction_handle_type transaction_handle, const operation& op)
{
   my->add_operation_to_builder_transaction(transaction_handle, op);
}

void wallet_api::replace_operation_in_builder_transaction(transaction_handle_type handle, unsigned operation_index, const operation& new_op)
{
   my->replace_operation_in_builder_transaction(handle, operation_index, new_op);
}

asset wallet_api::set_fees_on_builder_transaction(transaction_handle_type handle, string fee_asset)
{
   return my->set_fees_on_builder_transaction(handle, fee_asset);
}

transaction wallet_api::preview_builder_transaction(transaction_handle_type handle)
{
   return my->preview_builder_transaction(handle);
}

signed_transaction wallet_api::sign_builder_transaction(transaction_handle_type transaction_handle, bool broadcast)
{
   return my->sign_builder_transaction(transaction_handle, broadcast);
}

pair<transaction_id_type,signed_transaction> wallet_api::broadcast_transaction(signed_transaction tx)
{
    return my->broadcast_transaction(tx);
}
pair<transaction_id_type,signed_transaction> wallet_api::broadcast_build_transaction(transaction_handle_type transaction_handle)
{
    return my->broadcast_build_transaction(transaction_handle);
}
 
signed_transaction wallet_api::propose_builder_transaction(
   transaction_handle_type handle,
   string account_name_or_id,
   time_point_sec expiration,
   uint32_t review_period_seconds,
   bool broadcast)
{
   return my->propose_builder_transaction(handle, account_name_or_id, expiration, review_period_seconds, broadcast);
}

void wallet_api::remove_builder_transaction(transaction_handle_type handle)
{
   return my->remove_builder_transaction(handle);
}

account_object wallet_api::get_account(string account_name_or_id) const
{
   return my->get_account(account_name_or_id);
}

asset_object wallet_api::get_asset(string asset_name_or_id) const
{
   auto a = my->find_asset(asset_name_or_id);
   FC_ASSERT(a);
   return *a;
}

asset_dynamic_data_object wallet_api::get_dynamic_data_object(string asset_name_or_id) const
{
   auto asset = get_asset(asset_name_or_id);   
   return my->get_object<asset_dynamic_data_object>(asset.dynamic_asset_data_id);
}
asset_bitasset_data_object wallet_api::get_bitasset_data(string asset_name_or_id) const
{
   auto asset = get_asset(asset_name_or_id);
   FC_ASSERT(asset.is_market_issued() && asset.bitasset_data_id);
   return my->get_object<asset_bitasset_data_object>(*asset.bitasset_data_id);
}
fc::optional<asset_exchange_feed_object> wallet_api::get_asset_exchange_feed(string asset_base,string asset_quote,uint32_t utype)const
{ 
    try{
     auto asset1 = get_asset(asset_base);
     auto asset2 = get_asset(asset_quote);   
     return  my->_remote_db->get_asset_exchange_feed(asset1.id,asset2.id,utype);     
} FC_CAPTURE_AND_RETHROW( (asset_base)(asset_quote)(utype)) }

account_id_type wallet_api::get_account_id(string account_name_or_id) const
{
   return my->get_account_id(account_name_or_id);
}

asset_id_type wallet_api::get_asset_id(string asset_symbol_or_id) const
{
   return my->get_asset_id(asset_symbol_or_id);
} 
signed_transaction wallet_api::set_propose_register(string account_name_or_id,bool bset, bool broadcast)
{
    return my->update_account_flags(account_name_or_id,bset ? account_propose_register : 0,account_propose_register,broadcast);
}
signed_transaction  wallet_api::set_account_config(string account_name_or_id, string index, string config, bool broadcast)
{
   return my->set_account_config(account_name_or_id, index ,config,broadcast);
}
signed_transaction  wallet_api::change_password(string account_name_or_id, string password,bool broadcast )
{
    return my->change_password(account_name_or_id,password,broadcast);
}
void  wallet_api::remove_key(public_key_type pub_key)
{
    FC_ASSERT(!is_locked());
    string shorthash = detail::address_to_shorthash(pub_key);
    copy_wallet_file( "before-remove-key-" + shorthash );
    my->_keys.erase(pub_key);
    save_wallet_file();
    copy_wallet_file( "after-remove-key-" + shorthash );
}
bool  wallet_api::import_password(string account_name_or_id, string password)
{
    try{
        FC_ASSERT(password.size() >= 8);
        bool bsavefile = my->_bautosave;
        vector<account_key>  a_key= my->suggest_account_key(account_name_or_id,password);


        string shorthash = detail::address_to_shorthash( a_key[K_ACT].pub_key);
        if(bsavefile)
           copy_wallet_file( "before-import-key-" + shorthash );


        my->import_key(account_name_or_id, a_key[K_MEMO].wif_key);
        my->import_key(account_name_or_id, a_key[K_AUTHOR].wif_key);
        
        if( my->import_key(account_name_or_id, a_key[K_ACT].wif_key) &&  my->import_key(account_name_or_id, a_key[K_OWNER].wif_key))
        {
          if(bsavefile)
          {
            save_wallet_file();
            copy_wallet_file( "after-import-key-" + shorthash );
          }
          return true;
        }
       return false;
} FC_CAPTURE_AND_RETHROW( (account_name_or_id)(password)) }

bool wallet_api::import_key(string account_name_or_id, string wif_key)
{
    try{
   FC_ASSERT(!is_locked());
   // backup wallet,
   bool bsavefile = my->_bautosave;

   fc::optional<fc::ecc::private_key> optional_private_key = wif_to_key(wif_key);
   if (!optional_private_key)
      FC_THROW("Invalid private key");
   string shorthash = detail::address_to_shorthash(optional_private_key->get_public_key());
   if(bsavefile)
     copy_wallet_file( "before-import-key-" + shorthash );

   if( my->import_key(account_name_or_id, wif_key) )
   {
      if(bsavefile)
      {
        save_wallet_file();
        copy_wallet_file( "after-import-key-" + shorthash );
      }
      return true;
   }
   return false;
} FC_CAPTURE_AND_RETHROW( (account_name_or_id)(wif_key)) }

map<string, bool> wallet_api::import_accounts( string filename, string password )
{
   FC_ASSERT( !is_locked() );
   FC_ASSERT( fc::exists( filename ) );

   const auto imported_keys = fc::json::from_file<exported_keys>( filename );

   const auto password_hash = fc::sha512::hash( password );
   FC_ASSERT( fc::sha512::hash( password_hash ) == imported_keys.password_checksum );

   map<string, bool> result;
   for( const auto& item : imported_keys.account_keys )
   {
       const auto import_this_account = [ & ]() -> bool
       {
           try
           {
               const account_object account = get_account( item.account_name );
               const auto& owner_keys = account.owner.get_keys();
               const auto& active_keys = account.active.get_keys();

               for( const auto& public_key : item.public_keys )
               {
                   if( std::find( owner_keys.begin(), owner_keys.end(), public_key ) != owner_keys.end() )
                       return true;

                   if( std::find( active_keys.begin(), active_keys.end(), public_key ) != active_keys.end() )
                       return true;
               }
           }
           catch( ... )
           {
           }

           return false;
       };

       const auto should_proceed = import_this_account();
       result[ item.account_name ] = should_proceed;

       if( should_proceed )
       {
           uint32_t import_successes = 0;
           uint32_t import_failures = 0;
           // TODO: First check that all private keys match public keys
           for( const auto& encrypted_key : item.encrypted_private_keys )
           {
               try
               {
                  const auto plain_text = fc::aes_decrypt( password_hash, encrypted_key );
                  const auto private_key = fc::raw::unpack<private_key_type>( plain_text );

                  import_key( item.account_name, string( graphene::utilities::key_to_wif( private_key ) ) );
                  ++import_successes;
               }
               catch( const fc::exception& e )
               {
                  elog( "Couldn't import key due to exception ${e}", ("e", e.to_detail_string()) );
                  ++import_failures;
               }
           }
           ilog( "successfully imported ${n} keys for account ${name}", ("n", import_successes)("name", item.account_name) );
           if( import_failures > 0 )
              elog( "failed to import ${n} keys for account ${name}", ("n", import_failures)("name", item.account_name) );
       }
   }

   return result;
}

bool wallet_api::import_account_keys( string filename, string password, string src_account_name, string dest_account_name )
{
   FC_ASSERT( !is_locked() );
   FC_ASSERT( fc::exists( filename ) );

   bool is_my_account = false;
   const auto accounts = list_my_accounts();
   for( const auto& account : accounts )
   {
       if( account.name == dest_account_name )
       {
           is_my_account = true;
           break;
       }
   }
   FC_ASSERT( is_my_account );

   const auto imported_keys = fc::json::from_file<exported_keys>( filename );

   const auto password_hash = fc::sha512::hash( password );
   FC_ASSERT( fc::sha512::hash( password_hash ) == imported_keys.password_checksum );

   bool found_account = false;
   for( const auto& item : imported_keys.account_keys )
   {
       if( item.account_name != src_account_name )
           continue;

       found_account = true;

       for( const auto& encrypted_key : item.encrypted_private_keys )
       {
           const auto plain_text = fc::aes_decrypt( password_hash, encrypted_key );
           const auto private_key = fc::raw::unpack<private_key_type>( plain_text );

           my->import_key( dest_account_name, string( graphene::utilities::key_to_wif( private_key ) ) );
       }

       return true;
   }
   save_wallet_file();

   FC_ASSERT( found_account );

   return false;
}

string wallet_api::normalize_brain_key(string s) const
{
   return detail::normalize_brain_key( s );
}

variant wallet_api::info()
{
   return my->info();
}
variant wallet_api::node_info()
{
   return my->_remote_db->get_info();
}

variant_object wallet_api::about() const
{
    return my->about();
}

fc::ecc::private_key wallet_api::derive_private_key(const std::string& prefix_string, int sequence_number) const
{
   return detail::derive_private_key( prefix_string, sequence_number );
}
vector<string>   wallet_api::get_account_auth(string account) const
{
   return my->get_account_auth(account);
} 
signed_transaction    wallet_api::set_account_pubkey(string account,public_key_type ownerkey,public_key_type actkey,public_key_type memokey,public_key_type authkey,bool broadcast)
{
   return my->set_account_pubkey(account,ownerkey,actkey,memokey,authkey,broadcast);   
}
 
signed_transaction wallet_api::set_auth_payfor(string issuer,string foracc,string forrefer,bool broadcast)
{
   return my->set_auth_payfor(issuer,foracc,forrefer,broadcast);   
}
int32_t  wallet_api::is_authenticator(const string account,const uint32_t type,const string symbol,const string author) const
{
   return my->is_authenticator(account,type,symbol,author);  
}
signed_transaction  wallet_api::set_gateway_need_auth(string issuer,optional<uint32_t> need_auth,optional<vector<vector<string>>> trust_auth,optional<string> def_author,bool broadcast)
{
     return my->set_account_need_auth(issuer,need_auth,trust_auth,def_author,7,broadcast);    
}
signed_transaction  wallet_api::set_carrier_need_auth(string issuer,optional<uint32_t> need_auth,optional<vector<vector<string>>> trust_auth,optional<string> def_author,bool broadcast)
{
   return my->set_account_need_auth(issuer,need_auth,trust_auth,def_author,8,broadcast);    
}
signed_transaction  wallet_api::set_carrier_can_invest(string issuer,bool bcan,bool broadcast)
{
   return my->set_carrier_can_invest(issuer,bcan,broadcast); 
}
signed_transaction  wallet_api::set_auth_state(string issuer,string account,uint32_t state,bool broadcast)
{
   return my->set_auth_state(issuer,account,state,broadcast);    
}
signed_transaction  wallet_api::set_account_auth(string account,string to,string info,string key,bool broadcast) 
{
   return my->set_account_auth(account,to,info,key,broadcast);    
}
 signed_transaction    wallet_api::set_account_info(string account,variant_object info,bool broadcast) 
 {
    return my->set_account_info(account,info,broadcast);    
 }
 optional<account_info>    wallet_api::get_account_info(string account) 
 {
    return my->get_account_info(account);    
 }
 vector<string>   wallet_api::get_account_config(string account)
 {
    return my->get_account_config(account);    
 } 
 void  wallet_api::get_admin_config(string admin_name, string loan_symbol, string invest_symbol)
 {
    return my->get_admin_config(admin_name,loan_symbol,invest_symbol);    
 }
 
signed_transaction wallet_api::register_account(string name,
                                                public_key_type owner_pubkey,
                                                public_key_type active_pubkey,
                                                public_key_type memo_pubkey,
                                                public_key_type author_pubkey,
                                                string  registrar_account,
                                                string  referrer_account,
                                                uint32_t referrer_percent,
                                                bool broadcast)
{
   return my->register_account( name, owner_pubkey, active_pubkey,memo_pubkey,author_pubkey, registrar_account, referrer_account, referrer_percent, broadcast );
}

signed_transaction  wallet_api::create_account(string name, string password,                                         
                                        string  registrar_account,
                                        string  referrer_account,
                                        uint32_t referrer_percent,
                                        bool broadcast)
{
    try{ 
      FC_ASSERT(password.size()>=40,"password size must >=40");
      vector<account_key>  a_key= my->suggest_account_key(name,password); 
   
      return my->register_account( name, a_key[K_OWNER].pub_key, a_key[K_ACT].pub_key, a_key[K_MEMO].pub_key,a_key[K_AUTHOR].pub_key,registrar_account, referrer_account, referrer_percent, broadcast );
} FC_CAPTURE_AND_RETHROW((name)(password)(registrar_account)(referrer_account)(referrer_percent)(broadcast)) }


signed_transaction wallet_api::propose_register_account(string issuer, string name,
                                                public_key_type owner,
                                                public_key_type active,
                                                public_key_type memo,
                                                public_key_type author,
                                                string  registrar_account,
                                                string  referrer_account,
                                                uint32_t referrer_percent,
                                                bool broadcast)
{
    return my->propose_register_account(issuer, name, owner, active,memo,author, registrar_account, referrer_account, referrer_percent, broadcast );
}

signed_transaction wallet_api::account_coupon(string account_name, bool broadcast)
{
    return my->account_coupon(account_name, broadcast);
}
signed_transaction wallet_api::withdraw_exchange_fee(string account_name,string ufee,string symbol, bool broadcast)
{
   return my->withdraw_exchange_fee(account_name,ufee, symbol,broadcast);
}

signed_transaction wallet_api::create_account_with_brain_key(string brain_key, string account_name,
                                                             string registrar_account, string referrer_account,
                                                             bool broadcast /* = false */)
{
   return my->create_account_with_brain_key(
            brain_key, account_name, registrar_account,
            referrer_account, broadcast
            );
}
signed_transaction wallet_api::issue_asset(string to_account, string amount, string symbol,
                                           string memo, bool broadcast)
{
   return my->issue_asset(to_account, amount, symbol, memo, broadcast);
}

signed_transaction wallet_api::transfer(string from, string to, string amount,
                                        string asset_symbol, string memo, bool broadcast /* = false */)
{
   return my->transfer(from, to, amount, asset_symbol, memo, broadcast);
}
signed_transaction wallet_api::transfer_noencmsg(string from, string to, string amount,
                                        string asset_symbol, string memo, bool broadcast /* = false */)
{
   return my->transfer_noencmsg(from, to, amount, asset_symbol, memo, broadcast);
}
signed_transaction  wallet_api::propose_transfer(string from,
                                  string to,
                                  string amount,
                                  string asset_symbol,
                                  string memo,
                                  bool broadcast)
 {
   return my->propose_transfer(from, to, amount, asset_symbol, memo, broadcast);
}                                 
signed_transaction wallet_api::transfer_vesting(string from, string to, string amount,
                                        string asset_symbol, string memo, 
                                        uint32_t vesting_cliff_seconds,uint32_t vesting_duration_seconds,
                                        bool broadcast /* = false */)
{
   return my->transfer_vesting(from, to, amount, asset_symbol, memo, vesting_cliff_seconds,vesting_duration_seconds,broadcast);
}

 signed_transaction  wallet_api::asset_reserve_fees(string issuer,
                                   string amount,
                                   string symbol,
                                   bool broadcast)
 {
    return my->asset_reserve_fees(issuer,amount, symbol,broadcast);
 }
                                   
 signed_transaction  wallet_api::asset_claim_fees(string issuer,
                                   string amount,
                                   string symbol,
                                   bool broadcast)
{
     return my->asset_claim_fees(issuer,amount, symbol,broadcast);
}


signed_transaction wallet_api::create_asset(string issuer,
                                            string asset_issuer,
                                            string symbol,
                                            uint8_t precision,
                                            asset_options common,
                                            fc::optional<bitasset_options> bitasset_opts,                                                                              
                                            bool broadcast)

{
   if(issuer == asset_issuer || asset_issuer.size() <=0)
     return my->create_asset(issuer, symbol, precision, common, bitasset_opts, broadcast);
   else   
     return my->proposal_create_asset(issuer,asset_issuer, symbol, precision, common, bitasset_opts, broadcast);                                        
}
 


signed_transaction wallet_api::update_asset(string issuer,string symbol,        
                                            optional<string> new_issuer,
                                            const  variant_object& option_values,                                      
                                            bool broadcast /* = false */)
{
   try{
    optional<asset_object> asset_to_update = my->find_asset(symbol);
    if (!asset_to_update)
        FC_THROW("No asset with that symbol exists!");  
   if(issuer.size()<=0 || asset_to_update->issuer == get_account(issuer).id)
      return my->update_asset(symbol, new_issuer, option_values, broadcast);
   else    
      return my->propose_update_asset(issuer,symbol, new_issuer, option_values, broadcast);

} FC_CAPTURE_AND_RETHROW((issuer)(symbol)) }   

 
signed_transaction wallet_api::create_bitlender_option(string issuer,
                                      string   symbol,         
                                      const    optional<variant_object> key,                                                                      
                                      const    variant_object& option_values,                                                                                                                                                              
                                      bool     broadcast)
{
    return my->create_bitlender_option(issuer, symbol,key,option_values, broadcast);
}

signed_transaction wallet_api::update_bitlender_option_author(string    issuer, 
                                              string    symbol,                                               
                                               const vector<string> authors,
                                              const     uint32_t     weight_threshold,                                                                                                                                                         
                                              bool      broadcast)
{
    return my->update_bitlender_option_author(issuer, symbol,authors,weight_threshold, broadcast);
}

fc::variant wallet_api::bitlender_loan_info(const variant_object& option_values)
{try {
      
      bitlender_loan_operation update_op;
      fc::reflector<bitlender_loan_operation>::visit(
         fc::from_variant_visitor<bitlender_loan_operation>(  option_values, update_op,GRAPHENE_MAX_NESTED_OBJECTS )
        );      
      return my->_remote_bitlender->get_loan_info(update_op.amount_to_loan,update_op.loan_period,update_op.interest_rate,update_op.bid_period,0,update_op.amount_to_collateralize,optional<bitlender_key>());

} FC_CAPTURE_AND_RETHROW((option_values)) }                                       

signed_transaction wallet_api::bitlender_lend_order(string issuer,     string carrier, 
                                      const   optional<variant_object> key,                                               
                                      const   variant_object& option_values,                                                                                              
                                      bool    broadcast)
{
    return my->bitlender_lend_order(issuer,carrier,key, option_values, broadcast);
}     
signed_transaction wallet_api::bitlender_invest_order(string issuer,     string carrier,                                             
                                      const   variant_object& option_values,                                                                                              
                                      bool    broadcast)
{
    return my->bitlender_invest_order(issuer,carrier, option_values, broadcast);
}     
 
signed_transaction  wallet_api::bitlender_repay_interest(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                              
                                      const   uint32_t uperiod,
                                      bool    broadcast)
{
    return my->bitlender_repay_interest(issuer,id, uperiod, broadcast);
}     

signed_transaction  wallet_api::bitlender_overdue_interest(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                              
                                      const   uint32_t uperiod,
                                      bool    broadcast)
{
    return my->bitlender_overdue_interest(issuer,id, uperiod, broadcast);
}     
signed_transaction  wallet_api::bitlender_recycle_interest(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                              
                                      const   uint32_t uperiod,
                                      bool    broadcast)
{
    return my->bitlender_recycle_interest(issuer,id, uperiod, broadcast);
}     
     
signed_transaction  wallet_api::bitlender_repay_principal(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                                                                    
                                      bool    broadcast)
{
    return my->bitlender_repay_principal(issuer,id,  broadcast);
}          

signed_transaction  wallet_api::bitlender_pre_repay_principal(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                                                                    
                                      bool    broadcast)
{
    return my->bitlender_pre_repay_principal(issuer,id,  broadcast);
}             
                                                                                  
signed_transaction  wallet_api::bitlender_overdue_repay_principal(string issuer,                                           
                                      const   bitlender_order_id_type id,                                                                                                                                    
                                      bool    broadcast)
{
    return my->bitlender_overdue_repay_principal(issuer,id,  broadcast);
}    
vector<bitlender_order_id_type>   wallet_api::list_bitlender_order(string loan, string lender, uint32_t period, uint32_t ufiletermask, uint32_t usort)
{    try{
    fc::optional<asset_object> loan_obj = get_asset(loan);
    fc::optional<asset_object> lender_obj = get_asset(lender);
    FC_ASSERT(loan_obj.valid());
    FC_ASSERT(lender_obj.valid());
    vector<uint32_t> type;
    vector<asset_id_type> loan;  
    vector<asset_id_type> lender;
    loan.push_back(loan_obj->id);
    lender.push_back(lender_obj->id);
    return my->_remote_bitlender->list_bitlender_order(GRAPHENE_EMPTY_ACCOUNT,type,loan,lender,period,ufiletermask,usort);
}FC_CAPTURE_AND_RETHROW((loan)(lender)(period)(ufiletermask)(usort))}

signed_transaction  wallet_api::bitlender_recycle_principal(string issuer,                                           
                                      const   bitlender_order_id_type id,    
                                      const   string                pay,                                                                                                                                     
                                      bool    broadcast)
{
    return my->bitlender_recycle_principal(issuer,id,pay,  broadcast);
}    

signed_transaction  wallet_api::bitlender_remove_order(string issuer,                                           
                                      const   bitlender_order_id_type id,                                            
                                      bool    broadcast)  
{
   return my->bitlender_remove_order(issuer,id,  broadcast);
}

signed_transaction  wallet_api::bitlender_set_auto_repayment(string issuer,                                           
                                      const   bitlender_order_id_type id,  
                                       const   bool    bset  ,                                                                                                                                   
                                      bool    broadcast)
{
    return my->bitlender_set_auto_repayment(issuer,id, bset, broadcast);
} 
      
                   
signed_transaction wallet_api::bitlender_add_collateral(string issuer,                                              
                                      const   variant_object& option_values,                                                                                              
                                      bool    broadcast)
{
    return my->bitlender_add_collateral(issuer, option_values, broadcast);
}    
 


signed_transaction  wallet_api::bitlender_test_principal(string issuer,                                           
                                      const   object_id_type order_id,    
                                      uint32_t  utype,
                                      uint32_t  uvalue,                                                                                                                                
                                      bool    broadcast) 
{
    return my->bitlender_test_principal(issuer, order_id, utype,uvalue,broadcast);
}     
signed_transaction wallet_api::update_bitlender_option(string issuer,
                                      string   symbol,      
                                      const     optional<variant_object> key,                                     
                                       const     variant_object& option_values,                                   
                                      bool broadcast)
 {
     return my->update_bitlender_option(issuer, symbol, key,option_values,broadcast);
 }
 signed_transaction wallet_api::stop_bitlender_option(string issuer,
                                      string   symbol,
                                      const     bool stop,                                   
                                      bool broadcast)
 {
     return my->stop_bitlender_option(issuer, symbol, stop,broadcast);
 }
  signed_transaction wallet_api::fee_mode_bitlender_option(string issuer,
                                      string   symbol,
                                      const     uint32_t mode,                                   
                                      bool broadcast)
 {
     return my->fee_mode_bitlender_option(issuer, symbol, mode,broadcast);
 }
      

signed_transaction wallet_api::update_bitlender_paramers(string issuer, 
                                      const optional<variant_object> key,                                   
                                      const variant_object& option_values,                                                                        
                                      bool  broadcast)
 {
     return my->update_bitlender_paramers(issuer, key, option_values,broadcast);
 }
signed_transaction wallet_api::update_finance_paramers(string issuer,                                    
                                       const     variant_object& option_values,                                   
                                      bool broadcast)
 {
     return my->update_finance_paramers(issuer,  option_values,broadcast);
 }

 

 signed_transaction wallet_api::create_issue_fundraise(string issuer,                                     
                                      string    base_asset,  
                                      string    pay_asset, 
                                      string    pay_to,   
                                      string   type,         
                                      const     variant_object& exchange_settings,                                                                                                                                                                                          
                                      bool      broadcast)
{
    return my->create_issue_fundraise(issuer, base_asset,pay_asset,pay_to,type,exchange_settings, broadcast);
} 
signed_transaction wallet_api::update_issue_fundraise(string    issuer,           
                                                           string    base_asset,  
                                                           string    pay_asset, 
                                                           uint32_t  period,
                                                           const     optional<variant_object> exchange_settings,
                                                           uint32_t  op_type,                                                                                          
                                                           bool      broadcast)
 {
     return my->update_issue_fundraise(issuer, base_asset,pay_asset, period,exchange_settings,op_type,broadcast);
 }
 signed_transaction wallet_api::remove_issue_fundraise(string    issuer,           
                                                           string    base_asset,  
                                                           string    pay_asset,                                       
                                                           bool      broadcast)
 {
     return my->remove_issue_fundraise(issuer, base_asset,pay_asset, broadcast);
 }


 signed_transaction  wallet_api::issue_fundraise_publish_feed(string issuer,
                                         string   issue,
                                         string   buy,  
                                         uint32_t period,
                                         price    feed,
                                         bool broadcast)
{
 return my->issue_fundraise_publish_feed(issuer, issue,buy,period, feed,broadcast);
}
 
 signed_transaction wallet_api::create_buy_fundraise(string  issuer,
                                   string   issue,
                                   string   buy,  
                                   string   amount,  
                                   const   uint32_t bimmediately,                                                                                              
                                   bool    broadcast)
  {
    return my->create_buy_fundraise(issuer,issue,buy, amount,bimmediately, broadcast);
  }
  signed_transaction wallet_api::enable_buy_fundraise(string  issuer,
                                    buy_fundraise_id_type finance_id,                                        
                                    const   bool benable,                                                                                              
                                    bool    broadcast  )                                  

 {
    return my->enable_buy_fundraise(issuer, finance_id,benable, broadcast);
 } 
 signed_transaction  wallet_api::create_sell_exchange(
                                  string      issuer,        
                                  string      sell_asset,
                                  string      sell_amount,                                  
                                  string      buy_asset,   
                                  string      buy_amount, 
                                  string      type,
                                  variant     exchange_settings,     
                                  bool        broadcast)
 {
    return my->create_sell_exchange(issuer,sell_asset,sell_amount,buy_asset,buy_amount, type,exchange_settings, broadcast);
 }
signed_transaction   wallet_api::update_sell_exchange(
                                  string      issuer,     
                                  sell_exchange_id_type sell_id,                                         
                                  string      sell_amount,                                                                    
                                  string      buy_amount, 
                                  uint32_t    op_type,
                                  optional<variant_object> exchange_settings,     
                                  bool        broadcast)
{
    return my->update_sell_exchange(issuer,sell_id,sell_amount,buy_amount, op_type,exchange_settings, broadcast);
 }

signed_transaction   wallet_api::remove_sell_exchange(
                                  string      issuer,
                                  sell_exchange_id_type sell_id,
                                  bool        broadcast)
{
    return my->remove_sell_exchange(issuer,sell_id,broadcast);
}

signed_transaction    wallet_api::create_buy_exchange(
                                  string      issuer,        
                                  string      amount,   
                                  string      asset,                                                                
                                  optional<sell_exchange_id_type> sell_id,   
                                  optional<price>                 price, 
                                  bool        broadcast)
{
   return my->create_buy_exchange(issuer,amount,asset,sell_id,price,broadcast);
}
signed_transaction    wallet_api::update_buy_exchange(
                                  string      issuer,        
                                  string      amount,   
                                  string      asset,                                                                
                                  buy_exchange_id_type buy_id,   
                                  optional<price>      price, 
                                  bool        broadcast)
{
    return my->update_buy_exchange(issuer,amount,asset,buy_id,price,broadcast);
}



signed_transaction    wallet_api::remove_buy_exchange(
                                  string                issuer,
                                  buy_exchange_id_type  buy_id,
                                  bool                  broadcast)
{
   return my->remove_buy_exchange(issuer,buy_id,broadcast);
}

signed_transaction wallet_api::update_bitlender_rate(string issuer,
                                                     string symbol,
                                                     const  optional<variant_object> key,   
                                                     const variant_object &changed_rate_add,
                                                     const vector<uint32_t> changed_rate_remove,
                                                     bool broadcast)
{
     return my->update_bitlender_rate(issuer, symbol, key,changed_rate_add,changed_rate_remove,broadcast);
}

 bitlender_option_object wallet_api::get_bitlender_option(string symbol)
 {
     return  my->get_bitlender_option(symbol);
 }
 bitlender_option_object_key wallet_api::get_bitlender_option_key(string symbol,optional<bitlender_key> key)
 {
     return  my->get_bitlender_option_key(symbol,key);
 }
 issue_fundraise_object wallet_api::get_issue_fundraise(string symbol,string pay)
 {
     return  my->get_issue_fundraise(symbol,pay);
 }
vector<issue_fundraise_id_type>   wallet_api::list_issue_fundraise(string  issue, string buy, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit)
{
    return  my->list_issue_fundraise(issue,buy,ufiletermask,usort,start,limit);
}
vector<issue_fundraise_id_type>   wallet_api::list_my_issue_fundraise(string account, bool bcreate,uint64_t start,uint64_t limit)
{
     return  my->list_my_issue_fundraise(account,bcreate,start,limit);
}
vector<buy_fundraise_id_type>   wallet_api::list_buy_fundraise(string  issue, string buy, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit)
{
    return  my->list_buy_fundraise(issue,buy,ufiletermask,usort,start,limit);
}
vector<buy_fundraise_id_type>   wallet_api::list_my_buy_fundraise(string account, bool bowner,uint64_t start,uint64_t limit)
{
     return  my->list_my_buy_fundraise(account,bowner,start,limit);
}
vector<buy_fundraise_object>   wallet_api::list_buy_fundraise_by_id(issue_fundraise_id_type id, uint64_t start,uint64_t limit)
{
   return my->_remote_finance->list_buy_fundraise_by_id(id,start,limit);
}
vector<sell_exchange_id_type>     wallet_api::list_sell_exchange(string  issue, string buy, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit)
{
    return  my->list_sell_exchange( issue, buy,  ufiletermask,  usort, start, limit);
}
vector<sell_exchange_id_type>     wallet_api::list_my_sell_exchange(string account, uint64_t start,uint64_t limit)     
{
    return  my->list_my_sell_exchange( account,  start, limit);
}
vector<buy_exchange_id_type>       wallet_api::list_buy_exchange(string  issue, string buy, uint32_t ufiletermask, uint32_t usort,uint64_t start,uint64_t limit)          
{
    return  my->list_buy_exchange(  issue,  buy,  ufiletermask,  usort, start, limit);
}
vector<buy_exchange_object>      wallet_api::list_buy_exchange_by_id(sell_exchange_id_type id, uint64_t start,uint64_t limit)
{
   return my->_remote_finance->list_buy_exchange_by_id(id, start,limit);
}
vector<buy_exchange_id_type>       wallet_api::list_my_buy_exchange(string account, uint64_t start,uint64_t limit)     
{
    return  my->list_my_buy_exchange( account,  start, limit);
}
signed_transaction wallet_api::update_bitasset(string issuer,string symbol,
                                               const  variant_object& option_values,                                                
                                               bool   broadcast)
{   
   try{
    optional<asset_object> asset_to_update = my->find_asset(symbol);
    if (!asset_to_update)
        FC_THROW("No asset with that symbol exists!");  
   if(issuer.size()<=0 || asset_to_update->issuer == get_account(issuer).id)
      return my->update_bitasset(symbol, option_values, broadcast);
   else    
      return my->propose_update_bitasset(issuer,symbol, option_values, broadcast);

} FC_CAPTURE_AND_RETHROW((issuer)(symbol)) }   

signed_transaction wallet_api::update_asset_gateway(string issuer,string symbol,
                                               const  vector<string> & gateways,                                                 
                                               bool   broadcast)
{   
   try{
    optional<asset_object> asset_to_update = my->find_asset(symbol);
    if (!asset_to_update)
        FC_THROW("No asset with that symbol exists!");  
   if(issuer.size()<=0 || asset_to_update->issuer == get_account(issuer).id)
      return my->update_asset_gateway(symbol, gateways, broadcast);
   else    
      return my->propose_update_asset_gateway(issuer,symbol, gateways, broadcast);

} FC_CAPTURE_AND_RETHROW((issuer)(symbol)(gateways)) }   

 
signed_transaction wallet_api::update_asset_feed_producers(string    issuer,string symbol,
                                                           flat_set<string> new_feed_producers,
                                                           bool broadcast /* = false */)

{   
   try{
    optional<asset_object> asset_to_update = my->find_asset(symbol);
    if (!asset_to_update)
        FC_THROW("No asset with that symbol exists!");  
   if(issuer.size()<=0 || asset_to_update->issuer == get_account(issuer).id)
      return my->update_asset_feed_producers(symbol, new_feed_producers, broadcast);
   else    
       return my->propose_asset_feed_producers(issuer,symbol, new_feed_producers, broadcast);

} FC_CAPTURE_AND_RETHROW((issuer)(symbol)) }   
 
 signed_transaction wallet_api::propose_bitlender_feed_producers(string saccount, string symbol,
                                                       const  variant_object& option,
                                                       flat_set<string> new_feed_producers,
                                                       bool broadcast /* = false */)
{
     return my->propose_bitlender_feed_producers(saccount,symbol, option,new_feed_producers, broadcast);                                                      
}

signed_transaction wallet_api::publish_asset_feed(string publishing_account,
                                                  string symbol,
                                                  price_feed feed,
                                                  bool broadcast /* = false */)
{
   return my->publish_asset_feed(publishing_account, symbol, feed, broadcast);
}

 signed_transaction  wallet_api::publish_bitlender_feed(string publishing_account,  
                                         string symbol,                                                                         
                                         price_feed feed,
                                         bool broadcast )
{
   return my->publish_bitlender_feed(publishing_account,  symbol,feed, broadcast);
}


signed_transaction wallet_api::fund_asset_fee_pool(string from,
                                                   string symbol,
                                                   string amount,
                                                   bool broadcast /* = false */)
{
   return my->fund_asset_fee_pool(from, symbol, amount, broadcast);
}

signed_transaction wallet_api::reserve_asset(string from,
                                          string amount,
                                          string symbol,
                                          bool broadcast /* = false */)
{
   return my->reserve_asset(from, amount, symbol, broadcast);
}

signed_transaction wallet_api::global_settle_asset(string symbol,
                                                   price settle_price,
                                                   bool broadcast /* = false */)
{
   return my->global_settle_asset(symbol, settle_price, broadcast);
}

signed_transaction wallet_api::settle_asset(string account_to_settle,
                                            string amount_to_settle,
                                            string symbol,
                                            bool broadcast /* = false */)
{
   return my->settle_asset(account_to_settle, amount_to_settle, symbol, broadcast);
}

signed_transaction wallet_api::bid_collateral(string bidder_name,
                                              string debt_amount, string debt_symbol,
                                              string additional_collateral,
                                              bool broadcast )
{
   return my->bid_collateral(bidder_name, debt_amount, debt_symbol, additional_collateral, broadcast);
}

signed_transaction wallet_api::whitelist_account(string authorizing_account,
                                                 string account_to_list,
                                                 account_whitelist_operation::account_listing new_listing_status,
                                                 bool broadcast /* = false */)
{
   return my->whitelist_account(authorizing_account, account_to_list, new_listing_status, broadcast);
}

signed_transaction wallet_api::create_committee_member(string owner_account, string url,string memo,
                                               bool broadcast /* = false */)
{
   return my->create_committee_member(owner_account, url,memo, broadcast);
}
 signed_transaction  wallet_api::update_committee_member(string carrier_name,
                                       string url, optional<string> memo,                                    
                                       bool broadcast)
{
     return my->update_committee_member(carrier_name, url, memo,broadcast);
}


signed_transaction wallet_api::create_budget_member(string owner_account, string url,string memo,
                                               bool broadcast /* = false */)
{
   return my->create_budget_member(owner_account, url, memo,broadcast);
}
 signed_transaction  wallet_api::update_budget_member(string carrier_name,
                                       string url,    optional<string> memo,                                         
                                       bool broadcast)
{
     return my->update_budget_member(carrier_name, url,memo, broadcast);
}


signed_transaction wallet_api::create_gateway(string owner_account, string url,string memo,
                                              vector<string> a_asset,
                                               bool broadcast /* = false */)
{
   return my->create_gateway(owner_account, url,memo,a_asset, broadcast);
}
gateway_object wallet_api::get_gateway(string owner_account)
{
    return my->get_gateway(owner_account);
}
signed_transaction wallet_api::update_gateway(string owner_account, string url,optional<string> memo,
                                               optional<vector<string>> a_asset,
                                               bool broadcast /* = false */)
{
   return my->update_gateway(owner_account, url,memo,a_asset, broadcast);
}
signed_transaction wallet_api::create_carrier(string owner_account, string url,string memo,
                                               bool broadcast /* = false */)
{
   return my->create_carrier(owner_account, url, memo,broadcast);
}
carrier_object  wallet_api::get_carrier(string owner_account)
{
    return my->get_carrier(owner_account);
}
signed_transaction wallet_api::update_carrier(string carrier_name,
                                     string url,    optional<string> memo,                                  
                                     bool broadcast /* = false */)
{
   return my->update_carrier(carrier_name,url,memo,broadcast);
}


signed_transaction wallet_api::update_author(string owner_account, string url,optional<string> memo,optional<vector<string>> asset_symbol,  
                                               bool broadcast /* = false */)
{
   return my->update_author(owner_account, url, memo,asset_symbol,broadcast);
}
signed_transaction wallet_api::create_author(string owner_account, string url,string memo, vector<string> asset_symbol,
                                               bool broadcast /* = false */)
{
   return my->create_author(owner_account, url, memo,asset_symbol,broadcast);
}
author_object  wallet_api::get_author(string owner_account)
{
    return my->get_author(owner_account);
}
 
signed_transaction wallet_api::gateway_issue_currency(string owner_account, string gateway_account, string usupply,string symbol,  bool send ,
                                               bool broadcast /* = false */)
{
   return my->gateway_issue_currency(owner_account, gateway_account, usupply,symbol,send,broadcast);
}  
signed_transaction  wallet_api::gateway_deposit(string from, string to, string deposit, string symbol,  bool broadcast )
{
   return my->gateway_deposit(from, to, deposit,symbol,broadcast);
}  
signed_transaction  wallet_api::gateway_propose_deposit(string account,string from, string to, string deposit, string symbol,string memo,  bool broadcast )
{
   return my->gateway_propose_deposit(account,from, to, deposit,symbol,memo,broadcast);
}  
signed_transaction  wallet_api::gateway_withdraw(string from, string to, string uwithdraw, string symbol,  bool broadcast)
{
   return my->gateway_withdraw(from, to, uwithdraw,symbol,broadcast);
}  
signed_transaction   wallet_api::gateway_create_withdraw(string from, string to, string amount, string symbol,uint32_t withdrawal_period_sec,uint32_t periods_until_expiration , uint32_t identify,bool broadcast)
{
 return my->gateway_create_withdraw(from, to,  amount, symbol,withdrawal_period_sec,periods_until_expiration,identify,broadcast);
}
signed_transaction   wallet_api::gateway_update_withdraw(withdraw_permission_id_type id, string amount, string symbol,uint32_t withdrawal_period_sec,uint32_t periods_until_expiration , bool broadcast)
{
 return my->gateway_update_withdraw(id, amount, symbol,withdrawal_period_sec,periods_until_expiration,broadcast);
}
signed_transaction   wallet_api::gateway_remove_withdraw(withdraw_permission_id_type id,bool broadcast)
{
 return my->gateway_remove_withdraw(id,broadcast);
}
signed_transaction   wallet_api::gateway_claim_withdraw(withdraw_permission_id_type id, string amount, string symbol,  bool broadcast)
{
 return my->gateway_claim_withdraw(id, amount,symbol,broadcast);
}


map<string,witness_id_type> wallet_api::list_witnesses(const string& lowerbound, uint32_t limit)
{
   return my->_remote_db->lookup_witness_accounts(lowerbound, limit);
}

map<string,committee_member_id_type> wallet_api::list_committee_members(const string& lowerbound, uint32_t limit)
{
   return my->_remote_db->lookup_committee_member_accounts(lowerbound, limit);
}
map<string,budget_member_id_type> wallet_api::list_budget_members(const string& lowerbound, uint32_t limit)
{
   return my->_remote_db->lookup_budget_member_accounts(lowerbound, limit);
}
map<string,gateway_id_type> wallet_api::list_gateways(const string& lowerbound, uint32_t limit)
{
   return my->_remote_db->lookup_gateway_accounts(lowerbound, limit);
}
map<string,carrier_id_type> wallet_api::list_carriers(const string& lowerbound, uint32_t limit)
{
   return my->_remote_db->lookup_carrier_accounts(lowerbound, limit);
}
map<string,author_id_type> wallet_api::list_authors(const string& lowerbound, uint32_t limit)
{
   return my->_remote_db->lookup_author_accounts(lowerbound, limit);
}
vector<worker_object> wallet_api::list_workers( )
{
     return my->_remote_db->get_all_workers();
}
vector<worker_object> wallet_api::list_workers_history(worker_id_type start, uint64_t limit )
{
   return my->_remote_hist->get_workers_history(start,limit); 
}
vector<proposal_object> wallet_api::list_proposals_history(string owner_account,proposal_id_type start, uint64_t limit )
{
    try{
   account_object worker = get_account( owner_account );
   return my->_remote_hist->get_proposals_history(worker.id,start,limit); 
} FC_CAPTURE_AND_RETHROW((owner_account)(start)(limit)) }

witness_object wallet_api::get_witness(string owner_account)
{
   return my->get_witness(owner_account);
}

committee_member_object wallet_api::get_committee_member(string owner_account)
{
   return my->get_committee_member(owner_account);
}
 budget_member_object wallet_api::get_budget_member(string owner_account)
{
   return my->get_budget_member(owner_account);
}
 

signed_transaction wallet_api::create_witness(string owner_account,
                                              string url,string memo,
                                              bool broadcast /* = false */)
{
   return my->create_witness(owner_account, url, memo,broadcast);
}
signed_transaction  wallet_api::change_identity(string issuer_name,
                                       object_id_type object_id,      
                                       bool benable,
                                       bool broadcast)
{
    return my->change_identity(issuer_name,object_id, benable, broadcast);
}

signed_transaction   wallet_api::enable_witness( string witness_name, bool broadcast)
{
    return my->enable_witness(witness_name, broadcast);
}

signed_transaction wallet_api::create_worker(
   string owner_account,
   time_point_sec work_begin_date,
   time_point_sec work_end_date,
   string daily_pay,
   string name,
   string url,
   string memo,
   variant worker_settings,
   bool broadcast /* = false */)
{
   return my->create_worker( owner_account, work_begin_date, work_end_date,
      daily_pay, name, url, memo,worker_settings, broadcast );
}

vector<optional<worker_object>>  wallet_api::get_workers(string account)
{ try{
   account_object worker = get_account( account );
   return my->_remote_db->get_workers_by_account(worker.id);
} FC_CAPTURE_AND_RETHROW((account)) }

vector<optional<worker_object>>  wallet_api::get_workers_by_name(string worker)
{
    return my->_remote_db->get_workers_by_name(worker);
}

signed_transaction wallet_api::vote_for_worker(
   string owner_account,
   worker_vote_delta delta,
   bool broadcast /* = false */)
{
   return my->vote_for_worker( owner_account, delta, broadcast );
}

signed_transaction wallet_api::update_witness(
   string witness_name,
   string url,optional<string> memo,
   optional<string> block_signing_key,
   bool broadcast /* = false */)
{
   return my->update_witness(witness_name, url, memo,block_signing_key, broadcast);
}

vector< vesting_balance_object_with_info > wallet_api::get_vesting_balances( string account_name )
{
   return my->get_vesting_balances( account_name );
}

signed_transaction wallet_api::withdraw_vesting(
   string witness_name,
   string amount,
   string asset_symbol,
   bool broadcast /* = false */)
{
   return my->withdraw_vesting( witness_name, amount, asset_symbol, broadcast );
}
signed_transaction  wallet_api::revoke_vesting(
         string account_name,
         vesting_balance_id_type object_id,         
         bool broadcast)
{
   return my->revoke_vesting( account_name, object_id,  broadcast );
}
signed_transaction wallet_api::vote_for_committee_member(string voting_account,
                                                 string committee,
                                                 bool approve,
                                                 bool broadcast /* = false */)
{
   return my->vote_for_committee_member(voting_account, committee, approve, broadcast);
}
signed_transaction wallet_api::vote_for_budget_member(string voting_account,
                                                 string budget,
                                                 bool approve,
                                                 bool broadcast /* = false */)
{
   return my->vote_for_budget_member(voting_account, budget, approve, broadcast);
}

signed_transaction wallet_api::vote_for_witness(string voting_account,
                                                string witness,
                                                bool approve,
                                                bool broadcast /* = false */)
{
   return my->vote_for_witness(voting_account, witness, approve, broadcast);
}

signed_transaction wallet_api::set_voting_proxy(string account_to_modify,
                                                optional<string> voting_account,
                                                bool broadcast /* = false */)
{
   return my->set_voting_proxy(account_to_modify, voting_account, broadcast);
}

signed_transaction wallet_api::set_desired_member_count(string account_to_modify,
                                                                      uint16_t desired_number_of_witnesses,
                                                                      uint16_t desired_number_of_committee_members,
                                                                       uint16_t desired_number_of_budget_members,
                                                                      bool broadcast /* = false */)
{
   return my->set_desired_member_count(account_to_modify, desired_number_of_witnesses,
                                                     desired_number_of_committee_members, desired_number_of_budget_members,broadcast);
}

void wallet_api::set_wallet_filename(string wallet_filename,bool bautosave)
{
   my->_wallet_filename = wallet_filename;
   my->_bautosave = bautosave;  
}
 

signed_transaction wallet_api::sign_transaction(signed_transaction tx, bool broadcast /* = false */)
{ try {
   return my->sign_transaction( tx, broadcast);
} FC_CAPTURE_AND_RETHROW( (tx) ) }

operation wallet_api::get_prototype_operation(string operation_name)
{
   return my->get_prototype_operation( operation_name );
}
flat_map<string, operation> wallet_api::get_prototype_operations()
{
    return my->get_prototype_operations(  );
}
void wallet_api::dbg_make_uia(string creator, string symbol)
{
   FC_ASSERT(!is_locked());
   my->dbg_make_uia(creator, symbol);
}

void wallet_api::dbg_make_mia(string creator, string symbol)
{
   FC_ASSERT(!is_locked());
   my->dbg_make_mia(creator, symbol);
}

void wallet_api::dbg_push_blocks( std::string src_filename, uint32_t count )
{
   my->dbg_push_blocks( src_filename, count );
}

void wallet_api::dbg_generate_blocks( std::string debug_wif_key, uint32_t count )
{
   my->dbg_generate_blocks( debug_wif_key, count );
}

void wallet_api::dbg_stream_json_objects( const std::string& filename )
{
   my->dbg_stream_json_objects( filename );
}

void wallet_api::dbg_update_object( fc::variant_object update )
{
   my->dbg_update_object( update );
}

void wallet_api::network_add_nodes( const vector<string>& nodes )
{
   my->network_add_nodes( nodes );
}
 vector< variant >  wallet_api::network_get_potential_peers()
 {
   return my->network_get_potential_peers();  
 }
vector< variant > wallet_api::network_get_connected_peers()
{
   return my->network_get_connected_peers();
}
fc::variant_object  wallet_api::network_get_advanced_node_parameters() const
{ 
   return my->network_get_advanced_node_parameters();
}
 void  wallet_api::network_set_advanced_node_parameters(const fc::variant_object& params)
 {
   return my->network_set_advanced_node_parameters(params);
 }
 void wallet_api::flood_network(string prefix, uint32_t number_of_transactions)
 {
     FC_ASSERT(!is_locked());
     my->flood_network(prefix, number_of_transactions);
}
signed_transaction wallet_api::propose_zosparameter_change(
   const string& proposing_account,
   fc::time_point_sec expiration_time,
   const variant_object& changed_values,
   optional<string> memo,
   bool broadcast /* = false */
   )
{
   return my->propose_zosparameter_change( proposing_account, expiration_time, changed_values, memo,broadcast );
}
signed_transaction  wallet_api::locktoken_options_param(
      const string& issuer,    
      const string& asset_lock,     
      const uint32_t& lock_mode,    
      const variant_object& changed_values,
      bool broadcast) {
         return my->locktoken_options_param( issuer, asset_lock, lock_mode, changed_values,broadcast );
}
signed_transaction   wallet_api::locktoken_options_enablepayer(
      const string& issuer,            
      const string& asset_lock,    
      const uint32_t& lock_mode,
      bool broadcast) {
   return my->locktoken_options_enablepayer(issuer, asset_lock, lock_mode,broadcast);
}

signed_transaction   wallet_api::locktoken_create(
        const string   issuer,      
        const string   to,      
        const string   amount,
        const string   asset_symbol,        
        const uint32_t type,
        const uint32_t period,        
        const uint32_t autolock, 
        bool broadcast) {
      return my->locktoken_create(issuer, to, amount, asset_symbol,type,period,autolock,broadcast);
}

signed_transaction   wallet_api::locktoken_update(
        const string   issuer,      
        const locktoken_id_type locktoken_id,      
        const uint32_t   op_type,      
        const string   amount,
        const string   asset_symbol,        
        const uint32_t type,
        const uint32_t period,        
        const uint32_t autolock, 
        bool broadcast)
{
      return my->locktoken_update(issuer, locktoken_id,op_type, amount, asset_symbol,type,period,autolock,broadcast);
}        

signed_transaction   wallet_api::locktoken_remove(
        const string   issuer,      
        const locktoken_id_type  id,              
        bool broadcast) {
      return my->locktoken_remove(issuer, id,broadcast);
}
fc::variant_object    wallet_api::get_locktoken_sum(string account, string symbol ,uint32_t utype)
{
   account_object author_obj  = get_account(account);
   asset_object   symbol_obj  = get_asset(symbol);
   return my->_remote_db->get_locktoken_sum(author_obj.id, symbol_obj.id, utype);
}
fc::optional<locktoken_option_object>    wallet_api::get_locktoken_option(string symbol, uint32_t lock_type , bool checkasset)const
{    
   asset_object   symbol_obj  = get_asset(symbol);
   return my->_remote_db->get_locktoken_option(symbol_obj.id, lock_type,checkasset);
}
vector<locktoken_id_type>     wallet_api::get_locktokens(string account, string symbol, uint32_t utype) const 
{
   account_object author_obj  = get_account(account);
   asset_id_type asset_id = GRAPHENE_NULL_ASSET;
   if(symbol != "")  asset_id = get_asset(symbol).id;
   return my->_remote_db->get_locktokens(author_obj.id, asset_id, utype);
}
vector<locktoken_object>     wallet_api::get_locktoken_history(string account, string symbol, uint32_t utype, uint64_t ustart, uint64_t ulimit) const
{
   account_object author_obj  = get_account(account);   
   asset_id_type asset_id = GRAPHENE_NULL_ASSET;
   if(symbol != "")  asset_id = get_asset(symbol).id;
   return my->_remote_hist->get_locktoken_history(author_obj.id, asset_id, utype,ustart,ulimit);
}

signed_transaction wallet_api::propose_parameter_change(
   const string& proposing_account,
   fc::time_point_sec expiration_time,
   const variant_object& changed_values,
   optional<string> memo,
   bool broadcast /* = false */
   )
{
   return my->propose_parameter_change( proposing_account, expiration_time, changed_values, memo,broadcast );
}

 
signed_transaction wallet_api::propose_fee_change(
   const string& proposing_account,
   fc::time_point_sec expiration_time,
   const variant_object& changed_fees,
   optional<string> memo,
   bool broadcast /* = false */
   )
{
   return my->propose_fee_change( proposing_account, expiration_time, changed_fees,memo, broadcast );
}

 
signed_transaction   wallet_api::propose_asset_bitlender_property(
         const string& proposing_account,
         const string& symbol,
         uint32_t ubitproperty,  
         uint32_t umask,                  
         bool  broadcast)
 { 
     return my->propose_asset_bitlender_property( proposing_account, symbol, ubitproperty,umask,broadcast);
 }

signed_transaction wallet_api::approve_proposal(
   const string& fee_paying_account,
   const string& proposal_id,
   const approval_delta& delta,
   bool broadcast /* = false */
   )
{
   return my->approve_proposal( fee_paying_account, proposal_id, delta, broadcast );
}
 
signed_transaction  wallet_api::remove_proposal(
      const string& fee_paying_account,
      const string& proposal_id,
      bool  using_owner_authority , 
      bool broadcast )
{
   return my->remove_proposal( fee_paying_account, proposal_id, using_owner_authority, broadcast );
}



global_property_object wallet_api::get_global_properties() const
{
   return my->get_global_properties();
}
 bitlender_paramers_object wallet_api::get_bitlender_paramers() const
 {
      return my->_remote_db->get_bitlender_paramers();
 }
 bitlender_paramers_object_key wallet_api::get_bitlender_paramers_key(optional<bitlender_key> key) const
 {
      return my->_remote_db->get_bitlender_paramers_key(key);
 }
finance_paramers_object wallet_api::get_finance_paramers() const
 {
      return my->_remote_db->get_finance_paramers();
 }
dynamic_global_property_object wallet_api::get_dynamic_global_properties() const
{
   return my->get_dynamic_global_properties();
}

string wallet_api::help()const
{
   std::vector<std::string> method_names = my->method_documentation.get_method_names();
   std::stringstream ss;
   for (const std::string method_name : method_names)
   {
      try
      {
         ss << my->method_documentation.get_brief_description(method_name);
      }
      catch (const fc::key_not_found_exception&)
      {
         ss << method_name << " (no help available)\n";
      }
   }
   return ss.str();
}

string wallet_api::gethelp(const string& method)const
{
   fc::api<wallet_api> tmp;
   std::stringstream ss;
   ss << "\n";

   if( method == "import_key" )
   {
      ss << "usage: import_key ACCOUNT_NAME_OR_ID  WIF_PRIVATE_KEY\n\n";
      ss << "example: import_key \"1.3.11\" 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3\n";
      ss << "example: import_key \"usera\" 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3\n";
   }
   else if( method == "transfer" )
   {
      ss << "usage: transfer FROM TO AMOUNT SYMBOL \"memo\" BROADCAST\n\n";
      ss << "example: transfer \"1.3.11\" \"1.3.4\" 1000.03 CORE \"memo\" true\n";
      ss << "example: transfer \"usera\" \"userb\" 1000.123 CORE \"memo\" true\n";
   }
   else if( method == "create_account_with_brain_key" )
   {
      ss << "usage: create_account_with_brain_key BRAIN_KEY ACCOUNT_NAME REGISTRAR REFERRER BROADCAST\n\n";
      ss << "example: create_account_with_brain_key \"my really long brain key\" \"newaccount\" \"1.3.11\" \"1.3.11\" true\n";
      ss << "example: create_account_with_brain_key \"my really long brain key\" \"newaccount\" \"someaccount\" \"otheraccount\" true\n";
      ss << "\n";
      ss << "This method should be used if you would like the wallet to generate new keys derived from the brain key.\n";
      ss << "The BRAIN_KEY will be used as the owner key, and the active key will be derived from the BRAIN_KEY.  Use\n";
      ss << "register_account if you already know the keys you know the public keys that you would like to register.\n";

   }
   else if( method == "register_account" )
   {
      ss << "usage: register_account ACCOUNT_NAME OWNER_PUBLIC_KEY ACTIVE_PUBLIC_KEY REGISTRAR REFERRER REFERRER_PERCENT BROADCAST\n\n";
      ss << "example: register_account \"newaccount\" \"CORE6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV\" \"CORE6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV\" \"1.3.11\" \"1.3.11\" 50 true\n";
      ss << "\n";
      ss << "Use this method to register an account for which you do not know the private keys.";
   }
   else if( method == "create_asset" )
   {
      ss << "usage: ISSUER SYMBOL PRECISION_DIGITS OPTIONS BITASSET_OPTIONS BROADCAST\n\n";
      ss << "PRECISION_DIGITS: the number of digits after the decimal point\n\n";
      ss << "Example value of OPTIONS: \n";
      ss << fc::json::to_pretty_string( graphene::chain::asset_options() );
      ss << "\nExample value of BITASSET_OPTIONS: \n";
      ss << fc::json::to_pretty_string( graphene::chain::bitasset_options() );
      ss << "\nBITASSET_OPTIONS may be null\n";
   }
   else
   {
      std::string doxygenHelpString = my->method_documentation.get_detailed_description(method);
      if (!doxygenHelpString.empty())
         ss << doxygenHelpString;
      else
         ss << "No help defined for method " << method << "\n";
   }

   return ss.str();
}

bool wallet_api::load_wallet_file( string wallet_filename )
{
   return my->load_wallet_file( wallet_filename );
}

void wallet_api::save_wallet_file( string wallet_filename )
{
   my->save_wallet_file( wallet_filename );
}

std::map<string,std::function<string(fc::variant,const fc::variants&)> >
wallet_api::get_result_formatters() const
{
   return my->get_result_formatters();
}

bool wallet_api::is_locked()const
{
   return my->is_locked();
}
bool wallet_api::is_new()const
{
   return my->_wallet.cipher_keys.size() == 0;
}

void wallet_api::encrypt_keys()
{
   my->encrypt_keys();
}

void wallet_api::lock()
{ try {
   FC_ASSERT( !is_locked() );
   encrypt_keys();
   for( auto key : my->_keys )
      key.second = key_to_wif(fc::ecc::private_key());
   my->_keys.clear();
   my->_checksum = fc::sha512();
   my->self.lock_changed(true);
} FC_CAPTURE_AND_RETHROW() }

void wallet_api::unlock(string password)
{ try {
   FC_ASSERT(password.size() > 0);
   auto pw = fc::sha512::hash(password.c_str(), password.size());
   vector<char> decrypted = fc::aes_decrypt(pw, my->_wallet.cipher_keys);
   auto pk = fc::raw::unpack<plain_keys>(decrypted);
   FC_ASSERT(pk.checksum == pw);
   my->_keys = std::move(pk.keys);
   my->_checksum = pk.checksum;
   my->self.lock_changed(false);
} FC_CAPTURE_AND_RETHROW() }

void wallet_api::set_password( string password )
{
   if( !is_new() )
      FC_ASSERT( !is_locked(), "The wallet must be unlocked before the password can be set" );
   my->_checksum = fc::sha512::hash( password.c_str(), password.size() );
   lock();
}

vector< signed_transaction > wallet_api::import_balance( string name_or_id, const vector<string>& wif_keys, bool broadcast )
{
   return my->import_balance( name_or_id, wif_keys, broadcast );
}



namespace detail {

vector< signed_transaction > wallet_api_impl::import_balance( string name_or_id, const vector<string>& wif_keys, bool broadcast )
{ try {
   FC_ASSERT(!is_locked());
   const dynamic_global_property_object& dpo = _remote_db->get_dynamic_global_properties();
   account_object claimer = get_account( name_or_id );
   uint32_t max_ops_per_tx = 30;

   map< address, private_key_type > keys;  // local index of address -> private key
   vector< address > addrs;
   bool has_wildcard = false;
   addrs.reserve( wif_keys.size() );
   for( const string& wif_key : wif_keys )
   {
      if( wif_key == "*" )
      {
         if( has_wildcard )
            continue;
         for( const public_key_type& pub : _wallet.extra_keys[ claimer.id ] )
         {
            addrs.push_back( pub );
            auto it = _keys.find( pub );
            if( it != _keys.end() )
            {
               fc::optional< fc::ecc::private_key > privkey = wif_to_key( it->second );
               FC_ASSERT( privkey );
               keys[ addrs.back() ] = *privkey;
            }
            else
            {
               wlog( "Somehow _keys has no private key for extra_keys public key ${k}", ("k", pub) );
            }
         }
         has_wildcard = true;
      }
      else
      {
         optional< private_key_type > key = wif_to_key( wif_key );
         FC_ASSERT( key.valid(), "Invalid private key" );
         fc::ecc::public_key pk = key->get_public_key();
         addrs.push_back( pk );
         keys[addrs.back()] = *key;
         // see chain/balance_evaluator.cpp
         addrs.push_back( pts_address( pk, false, 56 ) );
         keys[addrs.back()] = *key;
         addrs.push_back( pts_address( pk, true, 56 ) );
         keys[addrs.back()] = *key;
         addrs.push_back( pts_address( pk, false, 0 ) );
         keys[addrs.back()] = *key;
         addrs.push_back( pts_address( pk, true, 0 ) );
         keys[addrs.back()] = *key;
      }
   }

   vector< balance_object > balances = _remote_db->get_balance_objects( addrs );
   addrs.clear();

   set<asset_id_type> bal_types;
   for( auto b : balances ) bal_types.insert( b.balance.asset_id );

   struct claim_tx
   {
      vector< balance_claim_operation > ops;
      set< address > addrs;
   };
   vector< claim_tx > claim_txs;

   for( const asset_id_type& a : bal_types )
   {
      balance_claim_operation op;
      op.deposit_to_account = claimer.id;
      for( const balance_object& b : balances )
      {
         if( b.balance.asset_id == a )
         {
            op.total_claimed = b.available( dpo.time );
            if( op.total_claimed.amount == 0 )
               continue;
            op.balance_to_claim = b.id;
            op.balance_owner_key = keys[b.owner].get_public_key();
            if( (claim_txs.empty()) || (claim_txs.back().ops.size() >= max_ops_per_tx) )
               claim_txs.emplace_back();
            claim_txs.back().ops.push_back(op);
            claim_txs.back().addrs.insert(b.owner);
         }
      }
   }

   vector< signed_transaction > result;

   for( const claim_tx& ctx : claim_txs )
   {
      signed_transaction tx;
      tx.operations.reserve( ctx.ops.size() );
      for( const balance_claim_operation& op : ctx.ops )
         tx.operations.emplace_back( op );
      set_operation_fees( tx, _remote_db->get_global_properties().parameters.current_fees );
      tx.validate();
      signed_transaction signed_tx = sign_transaction( tx, false );
      for( const address& addr : ctx.addrs )
         signed_tx.sign( keys[addr], _chain_id );
      // if the key for a balance object was the same as a key for the account we're importing it into,
      // we may end up with duplicate signatures, so remove those
      boost::erase(signed_tx.signatures, boost::unique<boost::return_found_end>(boost::sort(signed_tx.signatures)));
      result.push_back( signed_tx );
      if( broadcast )
         _remote_net_broadcast->broadcast_transaction(signed_tx);
   }

   return result;
} FC_CAPTURE_AND_RETHROW( (name_or_id) ) }
 
}
account_key_info wallet_api::dump_account_keys(string name)
{ try{
    FC_ASSERT(!is_locked());

    account_key_info info;
    fc::ecc::private_key  privatekey;
    account_object account = get_account(name);
    info.name  = name;
    info.id  = account.id;
   
     std::vector<public_key_type> active_keys = account.active.get_keys();
     std::vector<public_key_type> owner_keys = account.owner.get_keys();
     
     for( const auto& item : active_keys )
     { 
         account_key su;
         su.pub_key = item;
         su.add_ress = address(su.pub_key);
         try
         {
             privatekey = my->get_private_key(item);
             su.wif_key = key_to_wif(privatekey);
         } catch(...) {}
         
         info.active.push_back(su);        
     }    
     for( const auto& item : owner_keys )
     {
         account_key su;
         su.pub_key = item;
         su.add_ress = address(su.pub_key);
          try{
         privatekey= my->get_private_key(item);
         su.wif_key = key_to_wif(privatekey);
         } catch(...) {}
         info.owner.push_back(su);
     } 
     info.memo.pub_key = account.options.memo_key;
     info.memo.add_ress = address(info.memo.pub_key);
      try{
     privatekey= my->get_private_key(info.memo.pub_key);
     info.memo.wif_key = key_to_wif(privatekey);
     } catch(...) {}
     info.auth.pub_key = account.options.auth_key;
     info.auth.add_ress = address(info.auth.pub_key);
      try{
     privatekey= my->get_private_key(info.auth.pub_key);
     info.auth.wif_key = key_to_wif(privatekey);
     } catch(...) {}
     try{
         witness_object witness = get_witness(name);
         info.witness.pub_key = witness.signing_key;
         info.witness.add_ress = address(info.witness.pub_key);
         privatekey= my->get_private_key(info.witness.pub_key);
         info.witness.wif_key = key_to_wif(privatekey);
     } catch(...) {}    
    
    return info;    
} FC_CAPTURE_AND_RETHROW( (name)  ) }
map<public_key_type, string> wallet_api::dump_private_keys()
{
   FC_ASSERT(!is_locked());
   return my->_keys;
}

signed_transaction wallet_api::upgrade_account( string name, bool broadcast )
{
   return my->upgrade_account(name,broadcast);
}
signed_transaction wallet_api::set_account_lock_balance( string name,bool lock, bool broadcast )
{
   return my->update_account_flags(name,lock ? account_lock_balance : 0,account_lock_balance,broadcast);
} 

signed_transaction wallet_api::sell_asset(string seller_account,
                                          string amount_to_sell,
                                          string symbol_to_sell,
                                          string min_to_receive,
                                          string symbol_to_receive,
                                          uint32_t expiration,
                                          uint16_t mode,
                                          bool   broadcast)
{
   uint16_t fill_mode = (mode == 0) ? limit_order_price : limit_order_full;   
   return my->sell_asset(seller_account, amount_to_sell, symbol_to_sell, min_to_receive,
                         symbol_to_receive, expiration, fill_mode, broadcast);
}
signed_transaction  wallet_api::buy_asset(string seller_account,
                                    string amount_to_sell,
                                    string   symbol_to_sell,
                                    string min_to_receive,
                                    string   symbol_to_receive,
                                    uint32_t expiration,
                                    uint16_t  mode,
                                    bool     broadcast)
{
   uint16_t fill_mode = limit_order_buytype | ((mode == 0) ? limit_order_price : limit_order_full);   
   return my->sell_asset(seller_account, amount_to_sell, symbol_to_sell, min_to_receive,
                         symbol_to_receive, expiration, fill_mode, broadcast);
}
signed_transaction wallet_api::borrow_asset(string seller_name, string amount_to_sell,
                                                string asset_symbol, string amount_of_collateral, bool broadcast)
{
   FC_ASSERT(!is_locked());
   return my->borrow_asset(seller_name, amount_to_sell, asset_symbol, amount_of_collateral, broadcast);
}

signed_transaction wallet_api::cancel_order(object_id_type order_id, bool broadcast)
{
   FC_ASSERT(!is_locked());
   return my->cancel_order(order_id, broadcast);
}

memo_data wallet_api::sign_memo(string from, string to, string memo)
{
   FC_ASSERT(!is_locked());
   return my->sign_memo(from, to, memo);
}

string wallet_api::read_memo(const memo_data& memo)
{
   FC_ASSERT(!is_locked());
   return my->read_memo(memo);
}

string wallet_api::get_key_label( public_key_type key )const
{
   auto key_itr   = my->_wallet.labeled_keys.get<by_key>().find(key);
   if( key_itr != my->_wallet.labeled_keys.get<by_key>().end() )
      return key_itr->label;
   return string();
}

string wallet_api::get_private_key( public_key_type pubkey )const
{
   return key_to_wif( my->get_private_key( pubkey ) );
}

public_key_type  wallet_api::get_public_key( string label )const
{
   try { return fc::variant(label, 1).as<public_key_type>( 1 ); } catch ( ... ){}

   auto key_itr   = my->_wallet.labeled_keys.get<by_label>().find(label);
   if( key_itr != my->_wallet.labeled_keys.get<by_label>().end() )
      return key_itr->key;
   return public_key_type();
}

bool               wallet_api::set_key_label( public_key_type key, string label )
{
   auto result = my->_wallet.labeled_keys.insert( key_label{label,key} );
   if( result.second  ) return true;

   auto key_itr   = my->_wallet.labeled_keys.get<by_key>().find(key);
   auto label_itr = my->_wallet.labeled_keys.get<by_label>().find(label);
   if( label_itr == my->_wallet.labeled_keys.get<by_label>().end() )
   {
      if( key_itr != my->_wallet.labeled_keys.get<by_key>().end() )
         return my->_wallet.labeled_keys.get<by_key>().modify( key_itr, [&]( key_label& obj ){ obj.label = label; } );
   }
   return false;
}
 
map<string,public_key_type> wallet_api::get_blind_accounts()const
{
   FC_ASSERT( !is_locked() );
   map<string,public_key_type> result;
   for( const auto& item : my->_wallet.labeled_keys )
   {
      if( my->_keys.find(item.key) != my->_keys.end() )
         result[item.label] = item.key;
   }
   return result;
}

public_key_type    wallet_api::create_blind_account( string label, string brain_key  )
{
   FC_ASSERT( !is_locked() );

   auto label_itr = my->_wallet.labeled_keys.get<by_label>().find(label);
   if( label_itr !=  my->_wallet.labeled_keys.get<by_label>().end() )
      FC_ASSERT( !"Key with label already exists" );
   brain_key = fc::trim_and_normalize_spaces( brain_key );
   auto secret = fc::sha256::hash( brain_key.c_str(), brain_key.size() );
   auto priv_key = fc::ecc::private_key::regenerate( secret );
   public_key_type pub_key  = priv_key.get_public_key();

   FC_ASSERT( set_key_label( pub_key, label ) );

   my->_keys[pub_key] = graphene::utilities::key_to_wif( priv_key );

   save_wallet_file();
   return pub_key;
}

vector<asset>   wallet_api::get_blind_balances( string key_or_label )
{
   vector<asset> result;
   map<asset_id_type, share_type> balances;

   vector<commitment_type> used;

   auto pub_key = get_public_key( key_or_label );
   auto& to_asset_used_idx = my->_wallet.blind_receipts.get<by_to_asset_used>();
   auto start = to_asset_used_idx.lower_bound( std::make_tuple(pub_key,GRAPHENE_CORE_ASSET,false)  );
   auto end = to_asset_used_idx.lower_bound( std::make_tuple(pub_key,asset_id_type(uint32_t(0xffffffff)),true)  );
   while( start != end )
   {
      if( !start->used  )
      {
         auto answer = my->_remote_db->get_blinded_balances( {start->commitment()} );
         if( answer.size() )
            balances[start->amount.asset_id] += start->amount.amount;
         else
            used.push_back( start->commitment() );
      }
      ++start;
   }
   for( const auto& u : used )
   {
      auto itr = my->_wallet.blind_receipts.get<by_commitment>().find( u );
      my->_wallet.blind_receipts.modify( itr, []( blind_receipt& r ){ r.used = true; } );
   }
   for( auto item : balances )
      result.push_back( asset( item.second, item.first ) );
   return result;
}


blind_confirmation wallet_api::transfer_from_blind( string from_blind_account_key_or_label,
                                                    string to_account_id_or_name,
                                                    string amount_in,
                                                    string symbol,
                                                    bool broadcast )
{ try {
   transfer_from_blind_operation from_blind;

   FC_ASSERT( !is_locked() );

   auto fees  = my->_remote_db->get_global_properties().parameters.current_fees;
   fc::optional<asset_object> asset_obj = get_asset(symbol);
   FC_ASSERT(asset_obj.valid(), "Could not find asset matching ${asset}", ("asset", symbol));
   auto amount = asset_obj->amount_from_string(amount_in);

   from_blind.fee  = fees->calculate_fee( from_blind, asset_obj->options.core_exchange_rate );

   auto blind_in = asset_obj->amount_to_string( from_blind.fee + amount );


   auto conf = blind_transfer_help( from_blind_account_key_or_label,
                               from_blind_account_key_or_label,
                               blind_in, symbol, false, true/*to_temp*/ );
   FC_ASSERT( conf.outputs.size() > 0 );

   auto to_account = my->get_account( to_account_id_or_name );
   from_blind.to = to_account.id;
   from_blind.amount = amount;
   from_blind.blinding_factor = conf.outputs.back().decrypted_memo.blinding_factor;
   from_blind.inputs.push_back( {conf.outputs.back().decrypted_memo.commitment, authority() } );
   from_blind.fee  = fees->calculate_fee( from_blind, asset_obj->options.core_exchange_rate );

   idump( (from_blind) );
   conf.trx.operations.push_back(from_blind);
   ilog( "about to validate" );
   conf.trx.validate();

   ilog( "about to broadcast" );
   conf.trx = sign_transaction( conf.trx, broadcast );

   if( broadcast && conf.outputs.size() == 2 ) {

       // Save the change
       blind_confirmation::output conf_output;
       blind_confirmation::output change_output = conf.outputs[0];

       // The wallet must have a private key for confirmation.to, this is used to decrypt the memo
       public_key_type from_key = get_public_key(from_blind_account_key_or_label);
       conf_output.confirmation.to = from_key;
       conf_output.confirmation.one_time_key = change_output.confirmation.one_time_key;
       conf_output.confirmation.encrypted_memo = change_output.confirmation.encrypted_memo;
       conf_output.confirmation_receipt = conf_output.confirmation;
       //try {
       receive_blind_transfer( conf_output.confirmation_receipt, from_blind_account_key_or_label, "@"+to_account.name );
       //} catch ( ... ){}
   }

   return conf;
} FC_CAPTURE_AND_RETHROW( (from_blind_account_key_or_label)(to_account_id_or_name)(amount_in)(symbol) ) }

blind_confirmation wallet_api::blind_transfer( string from_key_or_label,
                                               string to_key_or_label,
                                               string amount_in,
                                               string symbol,
                                               bool broadcast )
{
   return blind_transfer_help( from_key_or_label, to_key_or_label, amount_in, symbol, broadcast, false );
}
blind_confirmation wallet_api::blind_transfer_help( string from_key_or_label,
                                               string to_key_or_label,
                                               string amount_in,
                                               string symbol,
                                               bool broadcast,
                                               bool to_temp )
{
   blind_confirmation confirm;
   try {

   FC_ASSERT( !is_locked() );
   public_key_type from_key = get_public_key(from_key_or_label);
   public_key_type to_key   = get_public_key(to_key_or_label);

   fc::optional<asset_object> asset_obj = get_asset(symbol);
   FC_ASSERT(asset_obj.valid(), "Could not find asset matching ${asset}", ("asset", symbol));

   blind_transfer_operation blind_tr;
   blind_tr.outputs.resize(2);

   auto fees  = my->_remote_db->get_global_properties().parameters.current_fees;

   auto amount = asset_obj->amount_from_string(amount_in);

   asset total_amount = asset_obj->amount(0);

   vector<fc::sha256> blinding_factors;

   //auto from_priv_key = my->get_private_key( from_key );

   blind_tr.fee  = fees->calculate_fee( blind_tr, asset_obj->options.core_exchange_rate );

   vector<commitment_type> used;

   auto& to_asset_used_idx = my->_wallet.blind_receipts.get<by_to_asset_used>();
   auto start = to_asset_used_idx.lower_bound( std::make_tuple(from_key,amount.asset_id,false)  );
   auto end = to_asset_used_idx.lower_bound( std::make_tuple(from_key,amount.asset_id,true)  );
   while( start != end )
   {
      auto result = my->_remote_db->get_blinded_balances( {start->commitment() } );
      if( result.size() == 0 )
      {
         used.push_back( start->commitment() );
      }
      else
      {
         blind_tr.inputs.push_back({start->commitment(), start->control_authority});
         blinding_factors.push_back( start->data.blinding_factor );
         total_amount += start->amount;

         if( total_amount >= amount + blind_tr.fee )
            break;
      }
      ++start;
   }
   for( const auto& u : used )
   {
      auto itr = my->_wallet.blind_receipts.get<by_commitment>().find( u );
      my->_wallet.blind_receipts.modify( itr, []( blind_receipt& r ){ r.used = true; } );
   }

   FC_ASSERT( total_amount >= amount+blind_tr.fee, "Insufficient Balance", ("available",total_amount)("amount",amount)("fee",blind_tr.fee) );

   auto one_time_key = fc::ecc::private_key::generate();
   auto secret       = one_time_key.get_shared_secret( to_key );
   auto child        = fc::sha256::hash( secret );
   auto nonce        = fc::sha256::hash( one_time_key.get_secret() );
   auto blind_factor = fc::sha256::hash( child );

   auto from_secret  = one_time_key.get_shared_secret( from_key );
   auto from_child   = fc::sha256::hash( from_secret );
   auto from_nonce   = fc::sha256::hash( nonce );

   auto change = total_amount - amount - blind_tr.fee;
   fc::sha256 change_blind_factor;
   fc::sha256 to_blind_factor;
   if( change.amount > 0 )
   {
      idump(("to_blind_factor")(blind_factor) );
      blinding_factors.push_back( blind_factor );
      change_blind_factor = fc::ecc::blind_sum( blinding_factors, blinding_factors.size() - 1 );
      wdump(("change_blind_factor")(change_blind_factor) );
   }
   else // change == 0
   {
      blind_tr.outputs.resize(1);
      blind_factor = fc::ecc::blind_sum( blinding_factors, blinding_factors.size() );
      idump(("to_sum_blind_factor")(blind_factor) );
      blinding_factors.push_back( blind_factor );
      idump(("nochange to_blind_factor")(blind_factor) );
   }
   fc::ecc::public_key from_pub_key = from_key;
   fc::ecc::public_key to_pub_key = to_key;

   blind_output to_out;
   to_out.owner       = to_temp ? authority() : authority( 1, public_key_type( to_pub_key.child( child ) ), 1 );
   to_out.commitment  = fc::ecc::blind( blind_factor, amount.amount.value );
   idump(("to_out.blind")(blind_factor)(to_out.commitment) );


   if( blind_tr.outputs.size() > 1 )
   {
      to_out.range_proof = fc::ecc::range_proof_sign( 0, to_out.commitment, blind_factor, nonce,  0, 0, amount.amount.value );

      blind_output change_out;
      change_out.owner       = authority( 1, public_key_type( from_pub_key.child( from_child ) ), 1 );
      change_out.commitment  = fc::ecc::blind( change_blind_factor, change.amount.value );
      change_out.range_proof = fc::ecc::range_proof_sign( 0, change_out.commitment, change_blind_factor, from_nonce,  0, 0, change.amount.value );
      blind_tr.outputs[1] = change_out;


      blind_confirmation::output conf_output;
      conf_output.label = from_key_or_label;
      conf_output.pub_key = from_key;
      conf_output.decrypted_memo.from = from_key;
      conf_output.decrypted_memo.amount = change;
      conf_output.decrypted_memo.blinding_factor = change_blind_factor;
      conf_output.decrypted_memo.commitment = change_out.commitment;
      conf_output.decrypted_memo.check   = from_secret._hash[0];
      conf_output.confirmation.one_time_key = one_time_key.get_public_key();
      conf_output.confirmation.to = from_key;
      conf_output.confirmation.encrypted_memo = fc::aes_encrypt( from_secret, fc::raw::pack( conf_output.decrypted_memo ) );
      conf_output.auth = change_out.owner;
      conf_output.confirmation_receipt = conf_output.confirmation;

      confirm.outputs.push_back( conf_output );
   }
   blind_tr.outputs[0] = to_out;

   blind_confirmation::output conf_output;
   conf_output.label = to_key_or_label;
   conf_output.pub_key = to_key;
   conf_output.decrypted_memo.from = from_key;
   conf_output.decrypted_memo.amount = amount;
   conf_output.decrypted_memo.blinding_factor = blind_factor;
   conf_output.decrypted_memo.commitment = to_out.commitment;
   conf_output.decrypted_memo.check   = secret._hash[0];
   conf_output.confirmation.one_time_key = one_time_key.get_public_key();
   conf_output.confirmation.to = to_key;
   conf_output.confirmation.encrypted_memo = fc::aes_encrypt( secret, fc::raw::pack( conf_output.decrypted_memo ) );
   conf_output.auth = to_out.owner;
   conf_output.confirmation_receipt = conf_output.confirmation;

   confirm.outputs.push_back( conf_output );

   /** commitments must be in sorted order */
   std::sort( blind_tr.outputs.begin(), blind_tr.outputs.end(),
              [&]( const blind_output& a, const blind_output& b ){ return a.commitment < b.commitment; } );
   std::sort( blind_tr.inputs.begin(), blind_tr.inputs.end(),
              [&]( const blind_input& a, const blind_input& b ){ return a.commitment < b.commitment; } );

   confirm.trx.operations.emplace_back( std::move(blind_tr) );
   ilog( "validate before" );
   confirm.trx.validate();
   confirm.trx = sign_transaction(confirm.trx, broadcast);

   if( broadcast )
   {
      for( const auto& out : confirm.outputs )
      {
         try { receive_blind_transfer( out.confirmation_receipt, from_key_or_label, "" ); } catch ( ... ){}
      }
   }

   return confirm;
} FC_CAPTURE_AND_RETHROW( (from_key_or_label)(to_key_or_label)(amount_in)(symbol)(broadcast)(confirm) ) }



/**
 *  Transfers a public balance from @from to one or more blinded balances using a
 *  stealth transfer.
 */
blind_confirmation wallet_api::transfer_to_blind( string from_account_id_or_name,
                                                  string asset_symbol,
                                                  /** map from key or label to amount */
                                                  vector<pair<string, string>> to_amounts,
                                                  bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );
   idump((to_amounts));

   blind_confirmation confirm;
   account_object from_account = my->get_account(from_account_id_or_name);

   fc::optional<asset_object> asset_obj = get_asset(asset_symbol);
   FC_ASSERT(asset_obj, "Could not find asset matching ${asset}", ("asset", asset_symbol));

   transfer_to_blind_operation bop;
   bop.from   = from_account.id;

   vector<fc::sha256> blinding_factors;

   asset total_amount = asset_obj->amount(0);

   for( auto item : to_amounts )
   {
      auto one_time_key = fc::ecc::private_key::generate();
      auto to_key       = get_public_key( item.first );
      auto secret       = one_time_key.get_shared_secret( to_key );
      auto child        = fc::sha256::hash( secret );
      auto nonce        = fc::sha256::hash( one_time_key.get_secret() );
      auto blind_factor = fc::sha256::hash( child );

      blinding_factors.push_back( blind_factor );

      auto amount = asset_obj->amount_from_string(item.second);
      total_amount += amount;


      fc::ecc::public_key to_pub_key = to_key;
      blind_output out;
      out.owner       = authority( 1, public_key_type( to_pub_key.child( child ) ), 1 );
      out.commitment  = fc::ecc::blind( blind_factor, amount.amount.value );
      if( to_amounts.size() > 1 )
         out.range_proof = fc::ecc::range_proof_sign( 0, out.commitment, blind_factor, nonce,  0, 0, amount.amount.value );


      blind_confirmation::output conf_output;
      conf_output.label = item.first;
      conf_output.pub_key = to_key;
      conf_output.decrypted_memo.amount = amount;
      conf_output.decrypted_memo.blinding_factor = blind_factor;
      conf_output.decrypted_memo.commitment = out.commitment;
      conf_output.decrypted_memo.check   = secret._hash[0];
      conf_output.confirmation.one_time_key = one_time_key.get_public_key();
      conf_output.confirmation.to = to_key;
      conf_output.confirmation.encrypted_memo = fc::aes_encrypt( secret, fc::raw::pack( conf_output.decrypted_memo ) );
      conf_output.confirmation_receipt = conf_output.confirmation;

      confirm.outputs.push_back( conf_output );

      bop.outputs.push_back(out);
   }
   bop.amount          = total_amount;
   bop.blinding_factor = fc::ecc::blind_sum( blinding_factors, blinding_factors.size() );

   /** commitments must be in sorted order */
   std::sort( bop.outputs.begin(), bop.outputs.end(),
              [&]( const blind_output& a, const blind_output& b ){ return a.commitment < b.commitment; } );

   confirm.trx.operations.push_back( bop );
   my->set_operation_fees( confirm.trx, my->_remote_db->get_global_properties().parameters.current_fees);
   confirm.trx.validate();
   confirm.trx = sign_transaction(confirm.trx, broadcast);

   if( broadcast )
   {
      for( const auto& out : confirm.outputs )
      {
         try { receive_blind_transfer( out.confirmation_receipt, "@"+from_account.name, "from @"+from_account.name ); } catch ( ... ){}
      }
   }

   return confirm;
} FC_CAPTURE_AND_RETHROW( (from_account_id_or_name)(asset_symbol)(to_amounts) ) }

blind_receipt wallet_api::receive_blind_transfer( string confirmation_receipt, string opt_from, string opt_memo )
{
   FC_ASSERT( !is_locked() );
   stealth_confirmation conf(confirmation_receipt);
   FC_ASSERT( conf.to );

   blind_receipt result;
   result.conf = conf;

   auto to_priv_key_itr = my->_keys.find( *conf.to );
   FC_ASSERT( to_priv_key_itr != my->_keys.end(), "No private key for receiver", ("conf",conf) );


   auto to_priv_key = wif_to_key( to_priv_key_itr->second );
   FC_ASSERT( to_priv_key );

   auto secret       = to_priv_key->get_shared_secret( conf.one_time_key );
   auto child        = fc::sha256::hash( secret );

   auto child_priv_key = to_priv_key->child( child );
   //auto blind_factor = fc::sha256::hash( child );

   auto plain_memo = fc::aes_decrypt( secret, conf.encrypted_memo );
   auto memo = fc::raw::unpack<stealth_confirmation::memo_data>( plain_memo );

   result.to_key   = *conf.to;
   result.to_label = get_key_label( result.to_key );
   if( memo.from )
   {
      result.from_key = *memo.from;
      result.from_label = get_key_label( result.from_key );
      if( result.from_label == string() )
      {
         result.from_label = opt_from;
         set_key_label( result.from_key, result.from_label );
      }
   }
   else
   {
      result.from_label = opt_from;
   }
   result.amount = memo.amount;
   result.memo = opt_memo;

   // confirm the amount matches the commitment (verify the blinding factor)
   auto commtiment_test = fc::ecc::blind( memo.blinding_factor, memo.amount.amount.value );
   FC_ASSERT( fc::ecc::verify_sum( {commtiment_test}, {memo.commitment}, 0 ) );

   blind_balance bal;
   bal.amount = memo.amount;
   bal.to     = *conf.to;
   if( memo.from ) bal.from   = *memo.from;
   bal.one_time_key = conf.one_time_key;
   bal.blinding_factor = memo.blinding_factor;
   bal.commitment = memo.commitment;
   bal.used = false;

   auto child_pubkey = child_priv_key.get_public_key();
   auto owner = authority(1, public_key_type(child_pubkey), 1);
   result.control_authority = owner;
   result.data = memo;

   auto child_key_itr = owner.key_auths.find( child_pubkey );
   if( child_key_itr != owner.key_auths.end() )
      my->_keys[child_key_itr->first] = key_to_wif( child_priv_key );

   // my->_wallet.blinded_balances[memo.amount.asset_id][bal.to].push_back( bal );

   result.date = fc::time_point::now();
   my->_wallet.blind_receipts.insert( result );
   my->_keys[child_pubkey] = key_to_wif( child_priv_key );

   save_wallet_file();

   return result;
}

vector<blind_receipt> wallet_api::blind_history( string key_or_account )
{
   vector<blind_receipt> result;
   auto pub_key = get_public_key( key_or_account );

   if( pub_key == public_key_type() )
      return vector<blind_receipt>();

   for( auto& r : my->_wallet.blind_receipts )
   {
      if( r.from_key == pub_key || r.to_key == pub_key )
         result.push_back( r );
   }
   std::sort( result.begin(), result.end(), [&]( const blind_receipt& a, const blind_receipt& b ){ return a.date > b.date; } );
   return result;
}

order_book wallet_api::get_order_book( const string& base, const string& quote, unsigned limit )
{
   return( my->_remote_db->get_order_book( base, quote, limit ) );
}
void  wallet_api::get_gas_info()
{
    return my ->get_gas_info( );
}
void  wallet_api::test(uint64_t start)
{
   auto dynamic_props = get_dynamic_global_properties();     
   for(uint64_t l = start;l<dynamic_props.head_block_number-1;l++){
     my ->_remote_db->get_block_ids(l);
   }
}

signed_block_with_info::signed_block_with_info( const signed_block& block )
   : signed_block( block )
{
   block_id = id();
   signing_key = signee();
   transaction_ids.reserve( transactions.size() );
   for( const processed_transaction& tx : transactions )
      transaction_ids.push_back( tx.id() );
}

vesting_balance_object_with_info::vesting_balance_object_with_info( const vesting_balance_object& vbo, fc::time_point_sec now )
   : vesting_balance_object( vbo )
{
   allowed_withdraw = get_allowed_withdraw( now );
   allowed_withdraw_time = now;
}

} } // graphene::wallet

namespace fc {
   void to_variant( const account_multi_index_type& accts, variant& vo, uint32_t max_depth )
   {
      to_variant( std::vector<account_object>(accts.begin(), accts.end()), vo, max_depth );
   }

   void from_variant( const variant& var, account_multi_index_type& vo, uint32_t max_depth )
   {
      const std::vector<account_object>& v = var.as<std::vector<account_object>>( max_depth );
      vo = account_multi_index_type(v.begin(), v.end());
   }
}



