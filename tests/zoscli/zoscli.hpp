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
 
 
namespace bpo = boost::program_options;


#define FC_MESSAGE(x)  std::cerr << x << "\n";
#define FC_SLEEP(x)  if((x++)%10 ==0) fc::usleep(fc::seconds(6));

struct options_data
{
   uint64_t g_uaccount      = 1101;
   uint64_t g_uworker       = 10000;
   uint64_t g_uasset        = 3000;
   uint64_t g_umember       = 1001;
   uint64_t g_ulender       = 10000;
   
   string   g_server        = "ws://127.0.0.1:8290";
   string   g_gateway       = "";
   int      g_isleep        = 3;

   string   nathan_account  = "business-group";
   uint64_t uintdex         = 0;
};



class client_connection
{
public:
  /////////
  // constructor
  /////////
  client_connection(  const fc::temp_directory &data_dir,string _server)
  {
    //wallet_data.chain_id = app->chain_database()->get_chain_id();
    wallet_data.ws_server = _server;   //"ws://47.75.107.157:8290";
    //wallet_data.ws_server = "ws://127.0.0.1:8290";
    wallet_data.ws_user = "";
    wallet_data.ws_password = "";
    websocket_connection = websocket_client.connect(wallet_data.ws_server);

    api_connection = std::make_shared<fc::rpc::websocket_api_connection>(*websocket_connection, GRAPHENE_MAX_NESTED_OBJECTS);

    remote_login_api = api_connection->get_remote_api<graphene::app::login_api>(1);
    database_api = api_connection->get_remote_api<graphene::app::database_api>(0);
    wallet_data.chain_id = database_api->get_chain_id();
    FC_ASSERT(remote_login_api->login(wallet_data.ws_user, wallet_data.ws_password));

    wallet_api_ptr = std::make_shared<graphene::wallet::wallet_api>(wallet_data, remote_login_api);
    wallet_filename = data_dir.path().generic_string() + "/wallet.json";
    wallet_api_ptr->set_wallet_filename(wallet_filename, false);

    wallet_api = fc::api<graphene::wallet::wallet_api>(wallet_api_ptr);

    wallet_cli = std::make_shared<fc::rpc::cli>(GRAPHENE_MAX_NESTED_OBJECTS);
    for (auto &name_formatter : wallet_api_ptr->get_result_formatters())
      wallet_cli->format_result(name_formatter.first, name_formatter.second);

    boost::signals2::scoped_connection closed_connection(websocket_connection->closed.connect([=] {
      cerr << "Server has disconnected us.\n";
      wallet_cli->stop();
    }));
    (void)(closed_connection);
  }
  void close()
  {
    wallet_cli->stop();
  }

public:
  fc::http::websocket_client websocket_client;
  graphene::wallet::wallet_data wallet_data;
  fc::http::websocket_connection_ptr websocket_connection;
  std::shared_ptr<fc::rpc::websocket_api_connection> api_connection;
  fc::api<login_api> remote_login_api;
  fc::api<graphene::app::database_api> database_api;
  std::shared_ptr<graphene::wallet::wallet_api> wallet_api_ptr;
  fc::api<graphene::wallet::wallet_api> wallet_api;
  std::shared_ptr<fc::rpc::cli> wallet_cli;
  std::string wallet_filename;
};



inline void import_password(client_connection &con,graphene::wallet::approval_delta &approval)
{
    con.wallet_api_ptr->import_password("nathan", "zos2018-nathan");
    con.wallet_api_ptr->import_password("messager", "zos2018-messager");
    con.wallet_api_ptr->import_password("init0", "zos2018-init0");
    con.wallet_api_ptr->import_password("init1", "zos2018-init1");
    con.wallet_api_ptr->import_password("init2", "zos2018-init2");
    con.wallet_api_ptr->import_password("init3", "zos2018-init3");
    con.wallet_api_ptr->import_password("init4", "zos2018-init4");
    con.wallet_api_ptr->import_password("init5", "zos2018-init5");
    con.wallet_api_ptr->import_password("init6", "zos2018-init6");
    con.wallet_api_ptr->import_password("init7", "zos2018-init7");
    con.wallet_api_ptr->import_password("init8", "zos2018-init8");
    con.wallet_api_ptr->import_password("init9", "zos2018-init9");
    con.wallet_api_ptr->import_password("init10", "zos2018-init10");
    con.wallet_api_ptr->import_password("platform-account", "zos2018-platform");


    //网关
    con.wallet_api_ptr->import_password("gateway-cny","gateway-pwd-cny");
    con.wallet_api_ptr->import_password("gateway-usd","gateway-pwd-usd");    
    con.wallet_api_ptr->import_password("gateway-usdt","gateway-pwd-usdt");    
    con.wallet_api_ptr->import_password("gateway-eth","gateway-pwd-eth");
    con.wallet_api_ptr->import_password("gateway-btc","gateway-pwd-btc");
    con.wallet_api_ptr->import_password("gateway-cad","gateway-pwd-cad");
    con.wallet_api_ptr->import_password("gateway-php","gateway-pwd-php");
    con.wallet_api_ptr->import_password("gateway-test","gateway-pwd-test");
      //基金会组长
    con.wallet_api_ptr->import_password("committee-group", "committee-group");
  //运营团队组长  
    con.wallet_api_ptr->import_password("business-group",  "business-group");

      //运营商  
    con.wallet_api_ptr->import_password("carrier-cny1","carrier-pwd-cny1");
    con.wallet_api_ptr->import_password("carrier-cny2","carrier-pwd-cny2");
    con.wallet_api_ptr->import_password("carrier-cny3","carrier-pwd-cny3");
    con.wallet_api_ptr->import_password("carrier-cny4","carrier-pwd-cny4");
    con.wallet_api_ptr->import_password("carrier-usd1","carrier-pwd-usd1");
    con.wallet_api_ptr->import_password("carrier-usd2","carrier-pwd-usd2");
    con.wallet_api_ptr->import_password("carrier-usd3","carrier-pwd-usd3");
    con.wallet_api_ptr->import_password("carrier-usdt1","carrier-pwd-usdt1");
    con.wallet_api_ptr->import_password("carrier-usdt2","carrier-pwd-usdt2");
    con.wallet_api_ptr->import_password("carrier-usdt3","carrier-pwd-usdt3");
    con.wallet_api_ptr->import_password("carrier-cad1","carrier-pwd-cad1");
    con.wallet_api_ptr->import_password("carrier-cad2","carrier-pwd-cad2");
    con.wallet_api_ptr->import_password("carrier-cad3","carrier-pwd-cad3");    
    con.wallet_api_ptr->import_password("carrier-php1","carrier-pwd-php1");
    con.wallet_api_ptr->import_password("carrier-php2","carrier-pwd-php2");
    con.wallet_api_ptr->import_password("carrier-php3","carrier-pwd-php3"); 

    con.wallet_api_ptr->import_password("author-cny", "author-pwd-cny");
    con.wallet_api_ptr->import_password("author-usd", "author-pwd-usd");
    con.wallet_api_ptr->import_password("author-cad", "author-pwd-cad");
    con.wallet_api_ptr->import_password("author-php", "author-pwd-php");
    con.wallet_api_ptr->import_password("author-usdt", "author-pwd-usdt");
    con.wallet_api_ptr->import_password("author-eth", "author-pwd-eth");
    con.wallet_api_ptr->import_password("author-btc", "author-pwd-btc");


    approval.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("nathan").id.to_string());
    approval.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("init0").id.to_string());
    approval.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("init1").id.to_string());
    approval.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("init2").id.to_string());
    approval.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("init3").id.to_string());
    approval.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("init4").id.to_string());
    approval.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("init5").id.to_string());
    approval.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("init6").id.to_string());
    approval.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("init7").id.to_string());
    approval.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("init8").id.to_string());
    approval.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("init9").id.to_string());
    approval.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("init10").id.to_string());



    //con.wallet_api_ptr->save_wallet_file();


      
  

} 


 
