/*
 * Copyright (c) 2018 John Jones, and contributors.
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
#include <graphene/app/application.hpp>
#include <graphene/app/plugin.hpp>

#include <graphene/utilities/tempdir.hpp>

#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/witness/witness.hpp>
#include <graphene/market_history/market_history_plugin.hpp>
#include <graphene/bitlender_history/bitlender_history_plugin.hpp>
#include <graphene/egenesis/egenesis.hpp>
#include <graphene/wallet/wallet.hpp>
#include <graphene/app/database_api.hpp>
#include <graphene/chain/global_property_object.hpp>
#include <graphene/utilities/key_conversion.hpp>
#include <fc/thread/thread.hpp>
#include <fc/smart_ref_impl.hpp>
#include <fc/network/http/websocket.hpp>
#include <fc/rpc/websocket_api.hpp>
#include <fc/rpc/cli.hpp>
#include <boost/program_options.hpp>
//#include <sys/socket.h>
//#include <netinet/ip.h>
//#include <sys/types.h>

#include <boost/filesystem/path.hpp>
 
#include "zoscli.hpp"
  
extern void init_block(options_data &option);
extern void create_block(options_data &option);
extern void create_block_asset_feed(client_connection &con, options_data &option, graphene::wallet::approval_delta &approval);
extern void create_base_account(client_connection &con, options_data &option, graphene::wallet::approval_delta &approval);
extern void create_bitlender(client_connection &con, options_data &option, graphene::wallet::approval_delta &approval);
extern void issue_curreny(client_connection &con, options_data &option);
extern void create_node(options_data &option);
//extern void create_block_author(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval);

void issue_curreny(options_data &option)
{
  using namespace graphene::chain;
  using namespace graphene::app;
  try
  {
    fc::temp_directory app_dir(graphene::utilities::temp_directory_path());
    
    client_connection con( app_dir,option.g_server);

    FC_MESSAGE("Setting wallet password");
    con.wallet_api_ptr->set_password("supersecret");
    con.wallet_api_ptr->unlock("supersecret");

    graphene::wallet::approval_delta approval;    
    import_password(con,approval);    
    issue_curreny(con,option);   
   
     // wait for everything to finish up
     con.close();
     fc::usleep(fc::seconds(1));
  }
  catch (fc::exception &e)
  {
    edump((e.to_detail_string()));
    throw;
  }
}    
 

void issue_curreny(client_connection &con,options_data &option)
{
    graphene::wallet::approval_delta approval;
    import_password(con,approval);
    
    fc::usleep(fc::seconds(option.g_isleep));
  
    FC_MESSAGE("gateway_issue_currency");

    string nathan_account = "business-group"; 
  

     {
       auto a_cny = con.wallet_api_ptr->get_asset("CNY");
       asset_dynamic_data_object dy_obect = con.wallet_api_ptr->get_dynamic_data_object("CNY");            
       con.wallet_api_ptr->gateway_issue_currency(option.nathan_account, "gateway-cny", a_cny.amount_to_string(a_cny.options.max_supply.value - dy_obect.current_supply-1), "CNY", true,true);
       con.wallet_api_ptr->approve_proposal(option.nathan_account, "1.10.0", approval, true);
     }
     {
       auto a_usd = con.wallet_api_ptr->get_asset("USD");
       asset_dynamic_data_object dy_obect  = con.wallet_api_ptr->get_dynamic_data_object("USD");        
       con.wallet_api_ptr->gateway_issue_currency(option.nathan_account, "gateway-usd", a_usd.amount_to_string(a_usd.options.max_supply.value/2 - dy_obect.current_supply - 1), "USD",true, true);
       con.wallet_api_ptr->approve_proposal(option.nathan_account, "1.10.0", approval, true);
     }
     {
       auto a_btc = con.wallet_api_ptr->get_asset("BTC");
       asset_dynamic_data_object dy_obect = con.wallet_api_ptr->get_dynamic_data_object("BTC");
       con.wallet_api_ptr->gateway_issue_currency(option.nathan_account, "gateway-btc", a_btc.amount_to_string(a_btc.options.max_supply.value/2 - dy_obect.current_supply - 1), "BTC",true, true);
       con.wallet_api_ptr->approve_proposal(option.nathan_account, "1.10.0", approval, true);
     }
     {
       auto a_eth = con.wallet_api_ptr->get_asset("ETH");
       asset_dynamic_data_object dy_obect  = con.wallet_api_ptr->get_dynamic_data_object("ETH");
       con.wallet_api_ptr->gateway_issue_currency(option.nathan_account, "gateway-eth", a_eth.amount_to_string(a_eth.options.max_supply.value/2 - dy_obect.current_supply - 1), "ETH", true,true);
       con.wallet_api_ptr->approve_proposal(option.nathan_account, "1.10.0", approval, true);
     }
     {
       auto a_usdt = con.wallet_api_ptr->get_asset("USDT");
       asset_dynamic_data_object dy_obect = con.wallet_api_ptr->get_dynamic_data_object("USDT");
       con.wallet_api_ptr->gateway_issue_currency(option.nathan_account, "gateway-usdt", a_usdt.amount_to_string(a_usdt.options.max_supply.value/2 - dy_obect.current_supply - 1), "USDT", true,true);
       con.wallet_api_ptr->approve_proposal(option.nathan_account, "1.10.0", approval, true);
     }
}


void transfer_balances(const options_data &option)
{
  using namespace graphene::chain;
  using namespace graphene::app;
  try
  {
    fc::temp_directory app_dir(graphene::utilities::temp_directory_path());
   
    // connect to the server
    client_connection con( app_dir,option.g_server);
   

    FC_MESSAGE("Setting wallet password");
    con.wallet_api_ptr->set_password("supersecret");
    con.wallet_api_ptr->unlock("supersecret");
  //  con.wallet_api_ptr->set_status(true);

    graphene::wallet::approval_delta approval;
    import_password(con,approval);

    //std::cerr <<" transfer_balances "  << "\n";
    int64_t  g_utranzos      = 1;
    g_utranzos = (fc::time_point::now().sec_since_epoch() + rand()) % 1000 * 100;   
    share_type amount_ = 0;
    fc::time_point_sec start = fc::time_point::now();
    //uint64_t time_count = 0;
    uint64_t trx_count = 0;
    auto a_zos = con.wallet_api_ptr->get_asset("ZOS");
    
    while (1)    
    {     
      try
       {
        
          if(g_utranzos >10000000000)
            g_utranzos =  fc::time_point::now().sec_since_epoch() % 1000 * 100;  
          amount_ = con.wallet_api_ptr->get_account_balance("business-group", "ZOS").amount;
          while(amount_.value >25000000 * g_utranzos)          
          {              
             //std::cerr << fc::time_point_sec(fc::time_point::now()).to_iso_string()  << "  "<< g_utranzos << "\n";
             for (int l = 0; l < 100; l++)
             {
               std::cerr << ".";
                con.wallet_api_ptr->transfer("business-group", "committee-group", a_zos.amount_to_string(g_utranzos), "ZOS", "", true);
                trx_count++;
                g_utranzos++;
             }
             amount_ = con.wallet_api_ptr->get_account_balance("business-group", "ZOS").amount;
            // time_count = (fc::time_point_sec(fc::time_point::now()) - start).count() / 1000;
             //std::cerr << "\n 100 trxs time " << time_count << "   total trxs  " << trx_count << "\n";
             start = fc::time_point::now();
          }    
              
              amount_ = con.wallet_api_ptr->get_account_balance("committee-group","ZOS").amount;
              while(amount_.value >25000000 * g_utranzos)
              {
               //std::cerr << fc::time_point_sec(fc::time_point::now()).to_iso_string()  << "  "<< g_utranzos << "\n";
                for (int l = 0; l < 100; l++)
                {
                  std::cerr << ".";
                  con.wallet_api_ptr->transfer("committee-group", "business-group",  a_zos.amount_to_string(g_utranzos), "ZOS","", true);
                  trx_count ++ ;
                  g_utranzos++;
                }
                amount_ = con.wallet_api_ptr->get_account_balance("committee-group","ZOS").amount;
               // time_count = (fc::time_point_sec(fc::time_point::now()) - start).count() / 1000;
                //std::cerr << "\n 100 trxs time " << time_count << "  total trxs  " << trx_count << "\n";
                start = fc::time_point::now();
              }              
           
            
        } catch (...)
        {
            g_utranzos = (fc::time_point::now().sec_since_epoch() + rand()) % 1000 * 100;   
        }
    }
    //bitlender_order_object

    // wait for everything to finish up
      con.close();
      fc::usleep(fc::seconds(1));
  }
  catch (fc::exception &e)
  {
    edump((e.to_detail_string()));
    throw;
  }  
} 
void transfer_balances_main(options_data &option)
{
  for (int l = 0; l < 10;l++)
  {
    fc::async([option]() {
      transfer_balances(option);
    });
  }
  transfer_balances(option);
}

void transfer_asset_feed(options_data &option)
{
  using namespace graphene::chain;
  using namespace graphene::app;
  try
  {
    fc::temp_directory app_dir(graphene::utilities::temp_directory_path());
   
    // connect to the server
    client_connection con( app_dir,option.g_server);
   

    FC_MESSAGE("Setting wallet password");
    con.wallet_api_ptr->set_password("supersecret");
    con.wallet_api_ptr->unlock("supersecret");

    graphene::wallet::approval_delta approval;
    import_password(con,approval);

    con.wallet_api_ptr->import_password("feeder-0", "zos2018-feed");
    con.wallet_api_ptr->import_password("feeder-1", "zos2018-feed");
    con.wallet_api_ptr->import_password("feeder-2", "zos2018-feed");
    con.wallet_api_ptr->import_password("feeder-3", "zos2018-feed");
    con.wallet_api_ptr->import_password("feeder-4", "zos2018-feed");
    con.wallet_api_ptr->import_password("feeder-5", "zos2018-feed");
    con.wallet_api_ptr->import_password("feeder-6", "zos2018-feed");
    con.wallet_api_ptr->import_password("feeder-7", "zos2018-feed");

    create_block_asset_feed(con, option, approval);  

    // wait for everything to finish up
      con.close();
      fc::usleep(fc::seconds(1));
  }
  catch (fc::exception &e)
  {
    edump((e.to_detail_string()));
    throw;
  }  
}


void test_bitlender(options_data &option)
{
  using namespace graphene::chain;
  using namespace graphene::app;
  try
  {
    fc::temp_directory app_dir(graphene::utilities::temp_directory_path());
   
    // connect to the server
    client_connection con( app_dir,option.g_server);

    FC_MESSAGE("Setting wallet password");
    con.wallet_api_ptr->set_password("supersecret");
    con.wallet_api_ptr->unlock("supersecret");

    graphene::wallet::approval_delta approval;    
    import_password(con,approval);

    option.g_ulender = 5;
    option.g_umember = 20;
    create_base_account(con, option, approval);
    create_bitlender(con, option, approval);

    // wait for everything to finish up
      con.close();
      fc::usleep(fc::seconds(1));
  }
  catch (fc::exception &e)
  {
    edump((e.to_detail_string()));
    throw;
  }  
}  


void create_author(options_data &option)
{
  using namespace graphene::chain;
  using namespace graphene::app;
  try
  {
    fc::temp_directory app_dir(graphene::utilities::temp_directory_path());
   
    // connect to the server
    client_connection con( app_dir,option.g_server);

    FC_MESSAGE("Setting wallet password");
    con.wallet_api_ptr->set_password("supersecret");
    con.wallet_api_ptr->unlock("supersecret");

    graphene::wallet::approval_delta approval;    
    import_password(con,approval);

   
    //create_block_author(con, option, approval);
 

    // wait for everything to finish up
      con.close();
      fc::usleep(fc::seconds(1));
  }
  catch (fc::exception &e)
  {
    edump((e.to_detail_string()));
    throw;
  }  
}   


void get_account(options_data &option)
{
  using namespace graphene::chain;
  using namespace graphene::app;
  try
  {
    fc::temp_directory app_dir(graphene::utilities::temp_directory_path());
   
    // connect to the server
    client_connection con( app_dir,option.g_server);

    FC_MESSAGE("Setting wallet password");
    con.wallet_api_ptr->set_password("supersecret");
    con.wallet_api_ptr->unlock("supersecret");

    graphene::wallet::approval_delta approval;    
    import_password(con,approval);

    std::fstream out;
    out.open( "account.info",   std::fstream::out | std::fstream::trunc); 
     
    std::cerr << "get_account \n";
   
    account_object ab = con.wallet_api_ptr->get_account("business-group");
    uint64_t count = con.wallet_api_ptr->get_object_count(account_id_type(0), true);
    for (uint64_t l = ab.id.instance()+1; l <= count;l++)
    {
      try{
        account_object a = con.wallet_api_ptr->get_account(account_id_type(l).to_string());
        if( a.owner.key_auths.size()==0 )   continue;
        if( a.active.key_auths.size()==0 )  continue;       
        std::cerr << a.name  << "\n";
        /* 
        out <<"{\n";
        out <<"   \"name\": \""                << a.name  << "\"\n";
        out <<"   \"owner_key\": \""           << std::string(a.owner.key_auths.begin()->first)  << "\"\n";
        out <<"   \"active_key\": \""          << std::string(a.active.key_auths.begin()->first) << "\"\n";
        out <<"   \"is_lifetime_member\": \""  << (a.is_lifetime_member() ? "true" : "false" )  << "\"\n";       
        out << + "},";*/
        out << "register_account  ";
        out << a.name  ;
        out <<"    \"" << std::string(a.owner.key_auths.begin()->first)  << "\" ";
        out <<"\""     << std::string(a.active.key_auths.begin()->first) << "\"  nathan  nathan 20 true \n";
      }
      catch(...)
      {
        continue;
      }
    }
    out.close();
    // wait for everything to finish up
    con.close();
    fc::usleep(fc::seconds(1));
  }
  catch (fc::exception &e)
  {
    edump((e.to_detail_string()));
    throw;
  }  
}
void db_resign()
{
    fc::path data_src_dir  = "/Users/adminuser/zosrun/src/blockchain";
    fc::path data_dest_dir = "/Users/adminuser/zosrun/dest/blockchain";

    flat_map<witness_id_type, fc::ecc::private_key> key;

    fc::optional<fc::ecc::private_key> private_key = graphene::utilities::wif_to_key("5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3");
    key[witness_id_type(1)]  = *private_key;
    key[witness_id_type(2)]  = *private_key;
    key[witness_id_type(3)]  = *private_key;
    key[witness_id_type(4)]  = *private_key;
    key[witness_id_type(5)]  = *private_key;
    key[witness_id_type(6)]  = *private_key;
    key[witness_id_type(7)]  = *private_key;
    key[witness_id_type(8)]  = *private_key;
    key[witness_id_type(9)]  = *private_key;
    key[witness_id_type(10)] = *private_key;
    key[witness_id_type(11)] = *private_key;

    database::resign(data_src_dir, data_dest_dir, key);
}

int main( int argc, char** argv )
 {
   try
   {
     options_data option;
     bpo::options_description cli_options("Graphene empty blocks");
     cli_options.add_options()
     ("ip", bpo::value<string>()->default_value("ws://127.0.0.1:8290"), "connect ip")
     ("account", bpo::value<uint64_t>()->default_value(1112), "account count")
     ("asset", bpo::value<uint64_t>()->default_value(300), "asset count")     
     ("loan", bpo::value<uint64_t>()->default_value(100), "loan count")
     ("member", bpo::value<uint64_t>()->default_value(1001), "member count")
     ("init", bpo::value<bool>()->default_value(0), "init block")
     ("node", bpo::value<bool>()->default_value(0), "init node")
     ("release", bpo::value<bool>()->default_value(0), "init release")
     ("add", bpo::value<bool>()->default_value(0), "add data")
     ("trans", bpo::value<bool>()->default_value(0), "trans data")     
     ("issue", bpo::value<bool>()->default_value(0), "issue_curreny")        
     ("bitlender", bpo::value<bool>()->default_value(0), "bitlender")     
     ("getaccount", bpo::value<bool>()->default_value(0), "getaccount")           
 
     ("worker,w", bpo::value<uint64_t>()->default_value(100), "worker count");

     bpo::variables_map options;
     try
     {
       boost::program_options::store(boost::program_options::parse_command_line(argc, argv, cli_options), options);
      }
      catch (const boost::program_options::error& e)
      {
         std::cerr << "empty_blocks:  error parsing command line: " << e.what() << "\n";
         return 0;
      }
      if (options.count("ip"))
        option.g_server = options.at("ip").as<string>();     
      if (options.count("account"))
        option.g_uaccount = options.at("account").as<uint64_t>();
      if (options.count("asset"))
        option.g_uasset = options.at("asset").as<uint64_t>();
      if (options.count("worker"))
        option.g_uworker = options.at("worker").as<uint64_t>();
      if (options.count("loan"))
        option.g_ulender = options.at("loan").as<uint64_t>();
      if (options.count("member"))      
        option.g_umember = options.at("member").as<uint64_t>();
        

      std::cerr << "ip " << option.g_server << "\n";
      std::cerr << "account " << option.g_uaccount << "\n";
      std::cerr << "asset " << option.g_uasset << "\n";
      std::cerr << "worker " << option.g_uworker << "\n";
      std::cerr << "loan " << option.g_ulender << "\n";
      std::cerr << "member " << option.g_umember << "\n";      
      

      if (options.count("init"))
      {
        option.g_gateway = "http://47.75.107.157";
        bool binit = options.at("init").as<bool>();
        if (binit)
          init_block(option);
        // create_author(option);
      }
      if (options.count("node"))
      {
        option.g_gateway = "https://gateway.zos.io";
        bool binit = options.at("node").as<bool>();
        if (binit)
          init_block(option);
      }
      if (options.count("release"))
      {
        option.g_gateway = "https://gateway.zos.io";
        bool binit = options.at("release").as<bool>();
        if (binit)
          create_node(option);
      }     

      if (options.count("issue"))
      {
        bool bissue= options.at("issue").as<bool>();
        if (bissue)
          issue_curreny(option);
      }
      if (options.count("add"))
      {
        bool binit = options.at("add").as<bool>();
        if (binit)
          create_block(option);
      }
      if (options.count("trans"))
      {
        bool binit = options.at("trans").as<bool>();
        if (binit)
          //transfer_asset_feed(option);
          transfer_balances_main(option);
      }
       if (options.count("bitlender"))
      {
        bool binit = options.at("bitlender").as<bool>();
        if (binit)
          test_bitlender(option);
      }
      if (options.count("getaccount"))
      {
        bool binit = options.at("getaccount").as<bool>();
        if (binit)
          get_account(option);
      }

      //db_resign();


   }
   catch(...)
   {
   }      
   return 0;
}
