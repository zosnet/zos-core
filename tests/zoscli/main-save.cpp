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

/*
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
#include <sys/socket.h>
#include <netinet/ip.h>
#include <sys/types.h>

#include <boost/filesystem/path.hpp>
 
 
namespace bpo = boost::program_options;


#define FC_MESSAGE(x)  std::cerr << x << "\n";
#define FC_SLEEP(x)  if((x++)%10 ==0) fc::usleep(fc::seconds(6));

uint64_t g_uaccount      = 1101;
uint64_t g_uworker       = 10000;
uint64_t g_uasset        = 3000;
uint64_t g_umember       = 1001;
uint64_t g_ulender       = 10000;
string   g_server        = "ws://127.0.0.1:8290";

class client_connection
{
public:
  /////////
  // constructor
  /////////
  client_connection(  const fc::temp_directory &data_dir)
  {
    //wallet_data.chain_id = app->chain_database()->get_chain_id();
    wallet_data.ws_server = g_server;   //"ws://47.75.107.157:8290";
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

void init_block()
{
  using namespace graphene::chain;
  using namespace graphene::app;
  try
  {
    fc::temp_directory app_dir(graphene::utilities::temp_directory_path());
   
    // connect to the server
    client_connection con( app_dir);

    int isleep = 6;

    FC_MESSAGE("Setting wallet password");
    con.wallet_api_ptr->set_password("supersecret");
    con.wallet_api_ptr->unlock("supersecret");

    auto nathan_key = fc::ecc::private_key::regenerate(fc::sha256::hash(string("nathan")));
    auto public_key = nathan_key.get_public_key();
    string wif_key  = graphene::utilities::key_to_wif(nathan_key); 
     
    FC_MESSAGE("import- key");

    FC_ASSERT(con.wallet_api_ptr->import_key("nathan",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init0",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init1",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init2",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init3",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init4",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init5",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init6",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init7",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init8",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init9",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init10",wif_key));

    graphene::wallet::approval_delta approval;
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

    con.wallet_api_ptr->save_wallet_file();

    account_object nathan_acct_before_upgrade = con.wallet_api_ptr->get_account("nathan");
    if (!nathan_acct_before_upgrade.is_lifetime_member())
    {
      FC_MESSAGE("Importing nathan's balance");
      std::vector<signed_transaction> import_txs = con.wallet_api_ptr->import_balance("nathan", {wif_key}, true);
      // upgrade nathan
      FC_MESSAGE("Upgrading Nathan to LTM");
      signed_transaction upgrade_tx = con.wallet_api_ptr->upgrade_account("nathan", true);
      account_object nathan_acct_after_upgrade = con.wallet_api_ptr->get_account("nathan");
    }
    
    FC_MESSAGE("register_account");

    con.wallet_api_ptr->register_account("gateway-cny",public_key,public_key,"nathan","nathan",20,true);
    con.wallet_api_ptr->register_account("gateway-usd",public_key,public_key,"nathan","nathan",20,true);    
    con.wallet_api_ptr->register_account("gateway-eth",public_key,public_key,"nathan","nathan",20,true);
    con.wallet_api_ptr->register_account("gateway-btc",public_key,public_key,"nathan","nathan",20,true);
    con.wallet_api_ptr->register_account("carrier-cny1",public_key,public_key,"nathan","nathan",20,true);
    con.wallet_api_ptr->register_account("carrier-cny2",public_key,public_key,"nathan","nathan",20,true);
    con.wallet_api_ptr->register_account("carrier-usd1",public_key,public_key,"nathan","nathan",20,true);
    con.wallet_api_ptr->register_account("carrier-usd2",public_key,public_key,"nathan","nathan",20,true);
 
    con.wallet_api_ptr->import_key("gateway-cny",wif_key);
    con.wallet_api_ptr->import_key("gateway-usd",wif_key);    
    con.wallet_api_ptr->import_key("gateway-eth",wif_key);
    con.wallet_api_ptr->import_key("gateway-btc",wif_key);
    con.wallet_api_ptr->import_key("carrier-cny1",wif_key);
    con.wallet_api_ptr->import_key("carrier-cny2",wif_key);
    con.wallet_api_ptr->import_key("carrier-usd1",wif_key);
    con.wallet_api_ptr->import_key("carrier-usd2",wif_key);

    fc::usleep(fc::seconds(isleep));

  

    FC_MESSAGE("vote");

     con.wallet_api_ptr->vote_for_witness("nathan", "init0", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init1", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init2", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init3", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init4", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init5", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init6", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init7", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init8", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init9", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init10", true, true);
    

     con.wallet_api_ptr->vote_for_committee_member("nathan", "init1", true, true);
     con.wallet_api_ptr->vote_for_committee_member("nathan", "init3", true, true);
     con.wallet_api_ptr->vote_for_committee_member("nathan", "init5", true, true);
     con.wallet_api_ptr->vote_for_committee_member("nathan", "init7", true, true);
     con.wallet_api_ptr->vote_for_committee_member("nathan", "init9", true, true);

     con.wallet_api_ptr->vote_for_budget_member("nathan", "init1", true, true);
     con.wallet_api_ptr->vote_for_budget_member("nathan", "init3", true, true);
     con.wallet_api_ptr->vote_for_budget_member("nathan", "init5", true, true);
     con.wallet_api_ptr->vote_for_budget_member("nathan", "init7", true, true);
     con.wallet_api_ptr->vote_for_budget_member("nathan", "init9", true, true);

     FC_MESSAGE("transfer");

     con.wallet_api_ptr->transfer("nathan", "init0", "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init1", "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init2", "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init3", "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init4", "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init5", "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init6", "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init7", "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init8", "100000000", "ZOS", "", true);

     con.wallet_api_ptr->transfer("nathan", "gateway-cny",  "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "gateway-usd",  "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "gateway-btc",  "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "gateway-eth",  "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "carrier-cny1", "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "carrier-cny2", "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "carrier-usd1", "100000000", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "carrier-usd2", "100000000", "ZOS", "", true);

     con.wallet_api_ptr->transfer("nathan", "committee-account", "1000", "ZOS", "", true);

     fc::usleep(fc::seconds(isleep));

    FC_MESSAGE("change param");

    {
      string s_cny = "{\"committee_proposal_review_period\":3600,\"maximum_proposal_lifetime\":10800}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);

      dynamic_global_property_object dy = con.wallet_api_ptr->get_dynamic_global_properties();
      global_property_object         gl = con.wallet_api_ptr->get_global_properties();
      con.wallet_api_ptr->propose_parameter_change("nathan", dy.time + gl.parameters.maximum_proposal_lifetime, type_object ,"aaaaaaa", true);
      con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    }    
    

     con.wallet_api_ptr->upgrade_account("gateway-cny", true);
     con.wallet_api_ptr->upgrade_account("gateway-usd", true);     
     con.wallet_api_ptr->upgrade_account("gateway-btc", true);
     con.wallet_api_ptr->upgrade_account("gateway-eth", true);
     con.wallet_api_ptr->upgrade_account("carrier-cny1", true);
     con.wallet_api_ptr->upgrade_account("carrier-cny2", true);
     con.wallet_api_ptr->upgrade_account("carrier-usd1", true);
     con.wallet_api_ptr->upgrade_account("carrier-usd2", true);        

     fc::usleep(fc::seconds(isleep));
  
     FC_MESSAGE("create member");
   
     con.wallet_api_ptr->create_gateway("gateway-cny", "http://47.75.107.157/zos-gateway/v1", {"CNY"}, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_gateway("gateway-usd", "http://47.75.107.157/zos-gateway/v1", {"USD"}, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_gateway("gateway-btc", "http://47.75.107.157/zos-gateway/v1", {"BTC"}, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_gateway("gateway-eth", "http://47.75.107.157/zos-gateway/v1", {"ETH"}, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);

     con.wallet_api_ptr->create_carrier("carrier-cny1", "", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-cny2", "", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-usd1", "", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-usd2", "", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);

     fc::usleep(fc::seconds(isleep));

     FC_MESSAGE("gateway_issue_currency");

    asset_object  a_cny =  con.wallet_api_ptr->get_asset("CNY");
    asset_object  a_usd =  con.wallet_api_ptr->get_asset("USD");
    asset_object  a_btc =  con.wallet_api_ptr->get_asset("BTC");
    asset_object  a_eth =  con.wallet_api_ptr->get_asset("ETH");


     {
       auto a_cny = con.wallet_api_ptr->get_asset("CNY");
       con.wallet_api_ptr->gateway_issue_currency("nathan", "gateway-cny", a_cny.amount_to_string(a_cny.options.max_supply.value - 1), "CNY", true,true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }
     {
       auto a_usd = con.wallet_api_ptr->get_asset("USD");
       con.wallet_api_ptr->gateway_issue_currency("nathan", "gateway-usd", a_usd.amount_to_string(a_usd.options.max_supply.value - 1), "USD",true, true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }
     {
       auto a_btc = con.wallet_api_ptr->get_asset("BTC");
       con.wallet_api_ptr->gateway_issue_currency("nathan", "gateway-btc", a_btc.amount_to_string(a_btc.options.max_supply.value - 1), "BTC",true, true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }
     {
       auto a_cny = con.wallet_api_ptr->get_asset("ETH");
       con.wallet_api_ptr->gateway_issue_currency("nathan", "gateway-eth", a_eth.amount_to_string(a_cny.options.max_supply.value - 1), "ETH", true,true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }

     con.wallet_api_ptr->propose_asset_feed_producers("nathan", "USD", {"nathan", "init0", "init1", "init2", "init3", "init4", "init5", "init6", "init7", "init8"}, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->propose_asset_feed_producers("nathan", "CNY", {"nathan", "init0", "init1", "init2", "init3", "init4", "init5", "init6", "init7", "init8"}, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
   
      FC_MESSAGE("publish_asset_feed");
 
     {
       auto a_asset = con.wallet_api_ptr->get_asset("CNY");
       price_feed feed;
       feed.settlement_price = price(asset(87, a_asset.id), asset(28769, GRAPHENE_CORE_ASSET));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.maintenance_collateral_cash_ratio = 1450;
       feed.maximum_short_squeeze_cash_ratio = 1100;
       feed.core_exchange_rate = price(asset(164, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));
       for (int l = 0; l <= 8;l++)
       {
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_asset_feed("init" + fc::to_string(l), "CNY", feed, true);
       }
     }

     {
       auto a_asset = con.wallet_api_ptr->get_asset("USD");
       price_feed feed;
       feed.settlement_price = price(asset(25, a_asset.id), asset(63647, GRAPHENE_CORE_ASSET));
       feed.maintenance_collateral_ratio = 1751;       
       feed.maximum_short_squeeze_ratio = 1501;
       feed.maintenance_collateral_cash_ratio = 1451;
       feed.maximum_short_squeeze_cash_ratio = 1101;
       feed.core_exchange_rate = price(asset(164, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));
       for (int l = 0; l <= 8;l++)
       {
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_asset_feed("init" + fc::to_string(l), "USD", feed, true);
       }
     }

      {
       auto a_asset = con.wallet_api_ptr->get_asset("BTC");
       price_feed feed;
       feed.settlement_price = price(asset(559, a_asset.id), asset(43613, GRAPHENE_CORE_ASSET));
       feed.maintenance_collateral_ratio = 1751;       
       feed.maximum_short_squeeze_ratio = 1501;
       feed.maintenance_collateral_cash_ratio = 1451;
       feed.maximum_short_squeeze_cash_ratio = 1101;
       feed.core_exchange_rate = price(asset(123, a_asset.id), asset(9360, GRAPHENE_CORE_ASSET));
       for (int l = 0; l <= 8;l++)
       {
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_asset_feed("init" + fc::to_string(l), "BTC", feed, true);
       }
     }

      {
       auto a_asset = con.wallet_api_ptr->get_asset("ETH");
       price_feed feed;
       feed.settlement_price = price(asset(600, a_asset.id), asset(43613, GRAPHENE_CORE_ASSET));
       feed.maintenance_collateral_ratio = 1751;       
       feed.maximum_short_squeeze_ratio = 1501;
       feed.maintenance_collateral_cash_ratio = 1451;
       feed.maximum_short_squeeze_cash_ratio = 1101;
       feed.core_exchange_rate = price(asset(123, a_asset.id), asset(9360, GRAPHENE_CORE_ASSET));
       for (int l = 0; l <= 8;l++)
       {
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_asset_feed("init" + fc::to_string(l), "ETH", feed, true);
       }
     }

    fc::usleep(fc::seconds(isleep));

     FC_MESSAGE("create_bitlender_option");

     auto a_nathan = con.wallet_api_ptr->get_account("nathan");
     {
       auto a_1 = con.wallet_api_ptr->get_account("carrier-cny1");
       auto a_2 = con.wallet_api_ptr->get_account("carrier-cny2");
       string s_cny = "{\"issuer_rate\":\""+a_nathan.id.to_string()+"\",\"carriers\":[\""+a_1.id.to_string()+"\",\""+a_2.id.to_string()+"\"]}";
       variant type = fc::json::from_string(s_cny);
       variant_object type_object;
       fc::from_variant(type, type_object);
       con.wallet_api_ptr->create_bitlender_option("nathan", "CNY", 0,type_object, true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }
     {
       auto a_1 = con.wallet_api_ptr->get_account("carrier-usd1");
       auto a_2 = con.wallet_api_ptr->get_account("carrier-usd2");
       string s_cny = "{\"issuer_rate\":\""+a_nathan.id.to_string()+"\",\"carriers\":[\""+a_1.id.to_string()+"\",\""+a_2.id.to_string()+"\"]}";
       variant type = fc::json::from_string(s_cny);
       variant_object type_object;
       fc::from_variant(type, type_object);
       con.wallet_api_ptr->create_bitlender_option("nathan", "USD", 0,type_object, true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }

     fc::usleep(fc::seconds(isleep));

     con.wallet_api_ptr->update_bitlender_option_author("nathan", "CNY", {"gateway-cny", "carrier-cny1", "carrier-cny2"}, 2, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);

    {
      string s_cny = "{\"risk_margin_rate\":2,\"platform_service_charge_rate\":20}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);

      con.wallet_api_ptr->update_bitlender_option("nathan", "CNY", type_object, true);
      graphene::wallet::approval_delta approval1;
      approval1.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("gateway-cny").id.to_string());
      approval1.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-cny1").id.to_string());
      approval1.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-cny2").id.to_string());
      con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval1, true);
    }
    {
      string s_cny = "{\"risk_margin_rate\":21,\"platform_service_charge_rate\":20}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_bitlender_option("nathan", "USD", type_object, true);
      con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    }
    {
      string s_cny = "{\"1\":{\"interest_rate\":70},\"2\":{\"interest_rate\":75},\"3\":{\"interest_rate\":80},\"4\":{\"interest_rate\":85},\"5\":{\"interest_rate\":90},\"6\":{\"interest_rate\":100},\"7\":{\"interest_rate\":105}}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_bitlender_rate("nathan", "CNY",0,type_object, {}, true);
    }
    {
      string s_cny = "{\"1\":{\"interest_rate\":74},\"2\":{\"interest_rate\":76},\"3\":{\"interest_rate\":81},\"4\":{\"interest_rate\":82},\"5\":{\"interest_rate\":91},\"6\":{\"interest_rate\":101},\"7\":{\"interest_rate\":115}}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_bitlender_rate("nathan", "USD",0, type_object,{}, true);
    }
    {
      string s_cny = "{\"feed_lifetime_sec\":2000000000}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->propose_update_bitasset("init0", "CNY", type_object,true);
      con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    }
    {
      string s_cny = "{\"feed_lifetime_sec\":2000000000}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->propose_update_bitasset("init0", "USD", type_object,true);
      con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    }
    {
      string s_cny = "{\"feed_lifetime_sec\":2000000000}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->propose_update_bitasset("init0", "BTC", type_object,true);
      con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    }
    {
      string s_cny = "{\"feed_lifetime_sec\":2000000000}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->propose_update_bitasset("init0", "ETH", type_object,true);
      con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    }

    FC_MESSAGE("gateway_deposit");
    fc::usleep(fc::seconds(isleep));

    con.wallet_api_ptr->gateway_deposit("gateway-cny", "init0", "100000", "CNY", true);
    con.wallet_api_ptr->gateway_deposit("gateway-cny", "init1", "100000", "CNY", true);
    con.wallet_api_ptr->gateway_deposit("gateway-cny", "init2", "100000", "CNY", true);
    con.wallet_api_ptr->gateway_deposit("gateway-cny", "init3", "100000", "CNY", true);
    con.wallet_api_ptr->gateway_deposit("gateway-cny", "init4", "100000", "CNY", true);
    con.wallet_api_ptr->gateway_deposit("gateway-cny", "init5", "100000", "CNY", true);
    con.wallet_api_ptr->gateway_deposit("gateway-cny", "init6", "100000", "CNY", true);

    con.wallet_api_ptr->gateway_deposit("gateway-cny", "nathan", "100000", "CNY", true);
    con.wallet_api_ptr->gateway_deposit("gateway-usd", "nathan", "100000", "USD", true);
    con.wallet_api_ptr->gateway_deposit("gateway-btc", "nathan", "100000", "BTC", true);
    con.wallet_api_ptr->gateway_deposit("gateway-eth", "nathan", "100000", "ETH", true);

    FC_MESSAGE("witness-a");

    con.wallet_api_ptr->create_account_with_brain_key("as", "witness-a", "nathan", "nathan", true);
    fc::usleep(fc::seconds(isleep));
    con.wallet_api_ptr->transfer("nathan", "witness-a", "20000000", "ZOS", "", true);
    con.wallet_api_ptr->upgrade_account("witness-a", true);
    con.wallet_api_ptr->create_witness("witness-a", "", true);
    con.wallet_api_ptr->vote_for_witness("nathan", "witness-a", true, true);

    fc::usleep(fc::seconds(isleep));
    FC_MESSAGE("bitlender_lend_order");

   {
     auto a_cny = con.wallet_api_ptr->get_asset("CNY");     
     string s_cny = "{\"order\":\"order00000\",\"amount_to_loan\":{\"amount\":100000,\"asset_id\":\""+a_cny.id.to_string()+"\"},\"loan_period\":1,\"interest_rate\":110,\"repayment_type\":1,\"amount_to_collateralize\":{\"amount\":33067816,\"asset_id\":\"1.3.0\"},\"collateral_rate\":1800,\"bid_period\":86400}";
     variant type = fc::json::from_string(s_cny);
     variant_object type_object;
     fc::from_variant(type, type_object);
     con.wallet_api_ptr->bitlender_lend_order("nathan", "carrier-cny1", 0,type_object,true);
   }
   {
     auto a_cny = con.wallet_api_ptr->get_asset("CNY");     
     string s_cny = "{\"order_id\":\"1.17.0\",\"amount_to_invest\":{\"amount\":30000,\"asset_id\":\"" + a_cny.id.to_string() + "\"},}";  
     variant type = fc::json::from_string(s_cny);
     variant_object type_object;
     fc::from_variant(type, type_object);
     con.wallet_api_ptr->bitlender_invest_order("init2", "carrier-cny2",type_object, true);
     con.wallet_api_ptr->bitlender_invest_order("init3", "carrier-cny2",type_object, true);
     con.wallet_api_ptr->bitlender_invest_order("init4", "carrier-cny2",type_object, true);
     con.wallet_api_ptr->bitlender_invest_order("init5", "carrier-cny2",type_object, true);
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
void create_account()
{
  using namespace graphene::chain;
  using namespace graphene::app;
  try
  {
    fc::temp_directory app_dir(graphene::utilities::temp_directory_path());
   
    // connect to the server
    client_connection con( app_dir);

    FC_MESSAGE("Setting wallet password");
    con.wallet_api_ptr->set_password("supersecret");
    con.wallet_api_ptr->unlock("supersecret");

    auto nathan_key = fc::ecc::private_key::regenerate(fc::sha256::hash(string("nathan")));
    auto public_key = nathan_key.get_public_key();
    string wif_key  = graphene::utilities::key_to_wif(nathan_key);

    
    FC_ASSERT(con.wallet_api_ptr->import_key("nathan",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init0",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init1",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init2",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init3",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init4",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init5",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init6",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init7",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init8",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init9",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init10",wif_key));


    con.wallet_api_ptr->import_key("gateway-cny",wif_key);
    con.wallet_api_ptr->import_key("gateway-usd",wif_key);    
    con.wallet_api_ptr->import_key("gateway-eth",wif_key);
    con.wallet_api_ptr->import_key("gateway-btc",wif_key);
    con.wallet_api_ptr->import_key("carrier-cny1",wif_key);
    con.wallet_api_ptr->import_key("carrier-cny2",wif_key);
    con.wallet_api_ptr->import_key("carrier-usd1",wif_key);
    con.wallet_api_ptr->import_key("carrier-usd2",wif_key);

    graphene::wallet::approval_delta approval;
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

    con.wallet_api_ptr->save_wallet_file();

    account_object nathan_acct_before_upgrade = con.wallet_api_ptr->get_account("nathan");
    if (!nathan_acct_before_upgrade.is_lifetime_member())
    {
      FC_MESSAGE("Importing nathan's balance");
      std::vector<signed_transaction> import_txs = con.wallet_api_ptr->import_balance("nathan", {wif_key}, true);
      // upgrade nathan
      FC_MESSAGE("Upgrading Nathan to LTM");
      signed_transaction upgrade_tx = con.wallet_api_ptr->upgrade_account("nathan", true);
      account_object nathan_acct_after_upgrade = con.wallet_api_ptr->get_account("nathan");
    }
    int      maxvote       = 500;
    int      uvote         = 4;
    uint64_t uaccount      = 1111;
    uint64_t uaccount_base = 100;
    uint64_t umember       = g_umember;
    uint64_t uworker       = g_uworker;
    uint64_t uworker_base  = 100;
    uint64_t uasset        = g_uasset;
    uint64_t uasset_base   = 100;
    uint64_t ulender       = g_ulender; 
    uint64_t uintdex       = 0;
    //创建用户

    std::cerr << "create account"  << uaccount << "\n";   
    for (uint64_t l = 1; l < umember+uvote+1; l++)
    {
      string account_s = "test-" + fc::to_string(l);
      string pass_s = account_s + "_" + account_s;
      std::cerr << "create account " << account_s  << "\n";
      try
      {
        con.wallet_api_ptr->create_account(account_s, pass_s, "nathan", "nathan", 20, true);
      }
      catch(...)
      {

      }
      con.wallet_api_ptr->import_password(account_s, pass_s);     
    }
    //创建管理用户
    std::cerr << "create member"  << umember << "\n";   
    for (uint64_t l = 1; l < umember; l++)
    {
      string account_s = "test-" + fc::to_string(l);      
      try
      {
        account_object nathan_obj = con.wallet_api_ptr->get_account(account_s);      
        std::cerr << "create member " << account_s  << "\n";
        if (!nathan_obj.is_lifetime_member())
        {
          con.wallet_api_ptr->transfer("nathan", account_s, "51000", "ZOS", "", true);
          con.wallet_api_ptr->upgrade_account(account_s, true);
        }
        if (!nathan_obj.is_carrier_member())
        {
          con.wallet_api_ptr->transfer("nathan", account_s, "1001", "ZOS", "", true);
          con.wallet_api_ptr->create_carrier(account_s, "committee_url_" + account_s, true);
          con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
        }
        if (!nathan_obj.is_gateway_member())
        {
          con.wallet_api_ptr->transfer("nathan", account_s, "1001", "ZOS", "", true);
          con.wallet_api_ptr->create_gateway(account_s, "committee_url_" + account_s,{}, true);
          con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
        }
        if (!nathan_obj.is_committee_member())
        {
          con.wallet_api_ptr->create_committee_member(account_s, "committee_url_" + account_s, true);
        }
        if (!nathan_obj.is_witness())
        {
          con.wallet_api_ptr->create_witness(account_s, "witness_url_" + account_s, true);
        }
        if (!nathan_obj.is_budget_member())
        {
          con.wallet_api_ptr->create_budget_member(account_s, "budget_url_" + account_s, true);
        }       
      }
      catch (...)
      {
        continue;
      }
    }
    //给管理账户投票
    std::cerr << "votes " << "\n";
    for (uint64_t l = umember + 1; l < umember + uvote; l++)
    {
      string account_s = "test-" + fc::to_string(l);
      try
      {
        account_object nathan_s = con.wallet_api_ptr->get_account(account_s);
        if (nathan_s.options.votes.size() >= 10)
          continue;        
      }
      catch (...)
      {
        continue;
      }

      con.wallet_api_ptr->transfer("nathan", account_s, "1100", "ZOS", "", true);
      for (int32_t k = 1; k < std::min(maxvote,int(umember)); k++)
      {
        string a_s = "test-" + fc::to_string(k);
        account_object nathan_a_s;
        try
        {
          nathan_a_s = con.wallet_api_ptr->get_account(a_s);
        }
        catch (...)
        {
          continue;
        }
        std::cerr << "votes " << account_s << "   " << a_s  << "\n";
        try
        {
          //if (nathan_a_s.is_witness() && k<11)
            //con.wallet_api_ptr->vote_for_witness(account_s, a_s, true, true);
        }
        catch (...)
        {
        }
        try
        {
          if (nathan_a_s.is_committee_member())
            con.wallet_api_ptr->vote_for_committee_member(account_s, a_s, true, true);
        }
        catch (...)
        {
        }
        try
        {
          if (nathan_a_s.is_budget_member())
            con.wallet_api_ptr->vote_for_budget_member(account_s, a_s, true, true);
        }
        catch (...)
        {
        }
        FC_SLEEP(uintdex);
      }
    }

    //创建资产
    std::cerr << "create asset"  << uasset << "\n";
    asset_options common;
    bitasset_options bitasset_opts;

    auto btc_s = con.wallet_api_ptr->get_asset("BTC");
    common.max_supply = GRAPHENE_MAX_SHARE_SUPPLY;
    common.market_fee_percent = 100;
    common.core_exchange_rate = price({asset(1, btc_s.id), asset(1, GRAPHENE_CORE_ASSET)});
    bitasset_opts.short_backing_asset = btc_s.id;
    uint64_t uindex = con.wallet_api_ptr->get_object_count(GRAPHENE_CORE_ASSET, true) -uasset_base+1;
    if(uindex<1)
      uindex = 1;
    for (uint64_t l = uindex; l < uasset; l++)
    {
      string asset_s = "A" + fc::to_string(l) + "T";
      std::cerr << "create asset " << asset_s << "\n";
      con.wallet_api_ptr->create_asset("nathan", asset_s, 8, common, bitasset_opts, true);
      FC_SLEEP(uintdex);
    }

    //创建worker
    std::cerr << "create worker" << uworker << "\n";
    variant type1 = fc::json::from_string("{\"type\" : \"exchange\" }");
    variant type2 = fc::json::from_string("{\"type\" : \"vesting\" }");
    uint64_t uworker_member = 0;
    
    while(con.wallet_api_ptr->get_object_count(worker_id_type(0), false)<uworker)
    {      
      uworker_member = uworker_member % (umember -1) + 1;
      string account_s = "test-" + fc::to_string(uworker_member);
      try
      {
        account_object nathan_obj = con.wallet_api_ptr->get_account(account_s);
        if (!nathan_obj.is_lifetime_member())
        {
          con.wallet_api_ptr->transfer("nathan", account_s, "51000", "ZOS", "", true);
          con.wallet_api_ptr->upgrade_account(account_s, true);
        }
      }
      catch (...)
      {
        continue;
      }
      std::cerr << "create worker"  << account_s << "\n";
      string pass_s = account_s + "_" + account_s;  
      con.wallet_api_ptr->import_password(account_s, pass_s);
      const fc::time_point_sec cur_now = fc::time_point::now() + fc::seconds(6000);
      con.wallet_api_ptr->transfer("nathan", account_s, "20", "ZOS", "", true);
      con.wallet_api_ptr->create_worker(account_s, cur_now, cur_now+ fc::seconds(24 * 3600 + 1), 1, account_s + "exchange", "worker_url_" + account_s, type1, true);
      con.wallet_api_ptr->create_worker(account_s, cur_now, cur_now + fc::seconds(24 * 3600 + 1), 1, account_s + "vesting", "worker_url_" + account_s, type2, true);
      FC_SLEEP(uintdex);
    }
    //给worker投票
    std::cerr << "votes for worker"  << "\n";
    for (uint64_t l = 1; l < 20; l++)
    {
      string account_s = "test-" + fc::to_string(l);
      try
      {
        budget_member_object nathan_s = con.wallet_api_ptr->get_budget_member(account_s);
        if (nathan_s.votes.size() >= 10)
          continue;
      }
      catch (...)
      {
        continue;
      }
      uint64_t uworker_num = con.wallet_api_ptr->get_object_count(worker_id_type(0), false);
      for(int32_t l = 1; l < std::min(maxvote,int(uworker_num)); l++)
      {
        try
        {
          std::cerr << "votes for worker" <<account_s <<" " << l << "\n";
          graphene::wallet::worker_vote_delta votedet;
          votedet.vote_for.insert(worker_id_type(l+uworker_base));
          con.wallet_api_ptr->transfer("nathan", account_s, "1", "ZOS", "", true);
          con.wallet_api_ptr->vote_for_worker(account_s, votedet, true);
        }
        catch (...)
        {
          continue;
        }
        FC_SLEEP(uintdex);
      }
    }
    if(g_ulender >0)
    {
    bitlender_option_object ocny = con.wallet_api_ptr->get_bitlender_option("CNY");
    object_id_type acny = *ocny.options.carriers.begin();
    string  cny_carrier = con.wallet_api_ptr->get_account(acny.to_string()).name;
    bitlender_option_object ousd = con.wallet_api_ptr->get_bitlender_option("USD");
    object_id_type ausd = *ousd.options.carriers.begin();
    string  use_carrier = con.wallet_api_ptr->get_account(ausd.to_string()).name;
    asset_object  a_cny =  con.wallet_api_ptr->get_asset("CNY");
    asset_object  a_usd =  con.wallet_api_ptr->get_asset("USD");
    asset_object  a_btc =  con.wallet_api_ptr->get_asset("BTC");
    asset_object  a_eth =  con.wallet_api_ptr->get_asset("ETH");

    bitlender_order_object o_cny;
    bitlender_order_object o_usd;
  
 
    std::cerr << "create loan" << ulender << cny_carrier << use_carrier << "\n";
    uint64_t ulend_member = 0;
    while (con.wallet_api_ptr->get_object_count(bitlender_order_id_type(0), false) < ulender)
    {
      ulend_member = ulend_member % (umember - 1) + 1;
      string account_s = "test-" + fc::to_string(ulend_member);
      try
      {
        con.wallet_api_ptr->get_account(account_s);
      }
      catch (...)
      {
        continue;
      }

      {
        variant type = fc::json::from_string("{\"order\":\"order00000\",\"amount_to_loan\":{\"amount\":100000,\"asset_id\":\"" + a_cny.id.to_string() + "\"},\"loan_period\":6,\"interest_rate\":1000,\"repayment_type\":1,\"amount_to_collateralize\":{\"amount\":4001338,\"asset_id\":\"" + a_btc.id.to_string() + "\"},\"collateral_rate\":1800,\"bid_period\":86400}");
        variant_object type_object;
        fc::from_variant(type, type_object);
        variant bo = con.wallet_api_ptr->bitlender_loan_info(type_object);
        from_variant( bo, o_cny, GRAPHENE_MAX_NESTED_OBJECTS );
      }
      {
        variant type = fc::json::from_string("{\"order\":\"order00000\",\"amount_to_loan\":{\"amount\":100000,\"asset_id\":\"" + a_usd.id.to_string() + "\"},\"loan_period\":1,\"interest_rate\":1000,\"repayment_type\":1,\"amount_to_collateralize\":{\"amount\":500540859,\"asset_id\":\"" + a_eth.id.to_string() + "\"},\"collateral_rate\":1800,\"bid_period\":86400}");
        variant_object type_object;
        fc::from_variant(type, type_object);
        variant bo = con.wallet_api_ptr->bitlender_loan_info(type_object);
        from_variant( bo, o_usd, GRAPHENE_MAX_NESTED_OBJECTS );
      }

      con.wallet_api_ptr->gateway_deposit("gateway-btc", account_s, a_btc.amount_to_string(o_cny.lock_collateralize.amount.value+30), "BTC", true);
      con.wallet_api_ptr->gateway_deposit("gateway-eth", account_s, a_eth.amount_to_string(o_usd.lock_collateralize.amount.value+30), "ETH", true);
      con.wallet_api_ptr->transfer("nathan", account_s, "20", "ZOS", "", true);
      string pass_s = account_s + "_" + account_s;
      con.wallet_api_ptr->import_password(account_s, pass_s);

      std::cerr << "create loan" << account_s << "\n";
      //cny
      {
        variant type = fc::json::from_string("{\"order\":\"order00000\",\"amount_to_loan\":{\"amount\":100000,\"asset_id\":\"" + a_cny.id.to_string() + "\"},\"loan_period\":6,\"interest_rate\":120,\"repayment_type\":1,\"amount_to_collateralize\":{\"amount\":"+fc::to_string(o_cny.amount_to_collateralize.amount.value+2)+",\"asset_id\":\"" + a_btc.id.to_string() + "\"},\"collateral_rate\":"+fc::to_string(o_cny.collateral_rate)+",\"bid_period\":600000}");
        variant_object type_object;
        fc::from_variant(type, type_object);
        con.wallet_api_ptr->bitlender_lend_order(account_s, cny_carrier, 0,type_object, true);
      }
      //usd
      {
        variant type = fc::json::from_string("{\"order\":\"order00000\",\"amount_to_loan\":{\"amount\":100000,\"asset_id\":\"" + a_usd.id.to_string() + "\"},\"loan_period\":1,\"interest_rate\":120,\"repayment_type\":1,\"amount_to_collateralize\":{\"amount\":"+fc::to_string(o_usd.amount_to_collateralize.amount.value+2)+",\"asset_id\":\"" + a_eth.id.to_string() + "\"},\"collateral_rate\":"+fc::to_string(o_usd.collateral_rate)+",\"bid_period\":600000}");
        variant_object type_object;
        fc::from_variant(type, type_object);
        con.wallet_api_ptr->bitlender_lend_order(account_s, use_carrier, 0,type_object, true);
      }
      FC_SLEEP(uintdex);
     }
     uint64_t uloan = con.wallet_api_ptr->get_object_count(bitlender_order_id_type(0), true) - 1;

     string account_s = "test-" + fc::to_string(umember);
     string pass_s = account_s + "_" + account_s;  
     con.wallet_api_ptr->import_password(account_s, pass_s);

     for (uint64_t l = 1; l < uloan+100;l+=100)
     {
       FC_SLEEP(uintdex);
        vector<bitlender_order_info>  wo_s =  con.wallet_api_ptr-> get_loan_orders("all",0, l, 100);
        if(wo_s.size()<=0)
          continue;
        for(auto wo : wo_s)  
        {    
           if(wo.amount_to_loan.amount != 100000 || !(wo.is_loan() || wo.is_investing()) )
            continue;     

           string susd = "{\"order_id\":\"1.17."+ fc::to_string(wo.id.instance())+ "\",\"amount_to_invest\":{\"amount\":10000,\"asset_id\":\""+a_usd.id.to_string()+"\"},}";
           string scny = "{\"order_id\":\"1.17."+ fc::to_string(wo.id.instance())+ "\",\"amount_to_invest\":{\"amount\":10000,\"asset_id\":\""+a_cny.id.to_string()+"\"},}";          

           con.wallet_api_ptr->transfer("nathan", account_s, "10", "ZOS", "", true);
           if (wo.amount_to_loan.asset_id == a_usd.id) // usd
           {
             std::cerr << susd << "\n";
             variant type = fc::json::from_string(susd);
             variant_object type_object;
             fc::from_variant(type, type_object);
             try
             {
               for (int k = wo.interest_book.size(); k < 10; k++)
               {
                 con.wallet_api_ptr->gateway_deposit("gateway-usd", account_s, "101", "USD", true);
                 con.wallet_api_ptr->bitlender_invest_order(account_s,use_carrier, type_object, true);
               }
             }
             catch (...)
             {
             }
           }
           else if (wo.amount_to_loan.asset_id == a_cny.id) // cny
           {
             std::cerr <<  scny << "\n";
             variant type = fc::json::from_string(scny);
             variant_object type_object;
             fc::from_variant(type, type_object);
             try
             {
               for (int k = wo.interest_book.size(); k < 10; k++)
               {
                 con.wallet_api_ptr->gateway_deposit("gateway-cny", account_s, "101", "CNY", true);
                 con.wallet_api_ptr->bitlender_invest_order(account_s,cny_carrier, type_object, true);
               }
             }
             catch (...)
             {
             }
           }
        }
     }
    }

       //创建用户
     uaccount = g_uaccount;
     std::cerr << "create account" << uaccount << "\n";
     uindex = con.wallet_api_ptr->get_object_count(account_id_type(0), true) - uaccount_base;
     if (uindex < 1)
       uindex = 1;
     for (uint64_t l = uindex; l < uaccount; l++)
     {
       try
       {
         string account_s = "test-" + fc::to_string(l);
         string pass_s = account_s + "_" + account_s;
         std::cerr << "create account " << account_s << "\n";
         signed_transaction create_acct_tx = con.wallet_api_ptr->create_account(account_s, pass_s, "nathan", "nathan", 20, true);
         con.wallet_api_ptr->import_password(account_s, pass_s);
       }
       catch (...)
       {
       }
     }

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

  
void trans()
{
  using namespace graphene::chain;
  using namespace graphene::app;
  try
  {
    fc::temp_directory app_dir(graphene::utilities::temp_directory_path());
   
    // connect to the server
    client_connection con( app_dir);
  

    FC_MESSAGE("Setting wallet password");
    con.wallet_api_ptr->set_password("supersecret");
    con.wallet_api_ptr->unlock("supersecret");

    auto nathan_key = fc::ecc::private_key::regenerate(fc::sha256::hash(string("nathan")));
    auto public_key = nathan_key.get_public_key();
    string wif_key  = graphene::utilities::key_to_wif(nathan_key); 
     
    FC_MESSAGE("import- key");

    FC_ASSERT(con.wallet_api_ptr->import_key("nathan",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init0",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init1",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init2",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init3",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init4",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init5",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init6",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init7",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init8",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init9",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init10",wif_key));

    graphene::wallet::approval_delta approval;
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

    con.wallet_api_ptr->save_wallet_file();

    account_object nathan_acct_before_upgrade = con.wallet_api_ptr->get_account("nathan");
    if (!nathan_acct_before_upgrade.is_lifetime_member())
    {
      FC_MESSAGE("Importing nathan's balance");
      std::vector<signed_transaction> import_txs = con.wallet_api_ptr->import_balance("nathan", {wif_key}, true);
      // upgrade nathan
      FC_MESSAGE("Upgrading Nathan to LTM");
      signed_transaction upgrade_tx = con.wallet_api_ptr->upgrade_account("nathan", true);
      account_object nathan_acct_after_upgrade = con.wallet_api_ptr->get_account("nathan");
    }

    uint64_t utrans = 0;
    while(utrans ++ < 0xFFFFFFFFFFFFF)
    {
      std::cerr <<   utrans << "\n";
      for (int l = 0; l < 100;l++)  
        con.wallet_api_ptr->transfer("nathan", "init0", "100", "ZOS", "", true);
      for (int l = 0; l < 100;l++)  
        con.wallet_api_ptr->transfer("init0",  "nathan",  "100", "ZOS", "", true);
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
void init_node()
{
  using namespace graphene::chain;
  using namespace graphene::app;
  try
  {
    fc::temp_directory app_dir(graphene::utilities::temp_directory_path());
   
    // connect to the server
    client_connection con( app_dir);

    int isleep = 3;

    FC_MESSAGE("Setting wallet password");
    con.wallet_api_ptr->set_password("supersecret");
    con.wallet_api_ptr->unlock("supersecret");

    auto nathan_key = fc::ecc::private_key::regenerate(fc::sha256::hash(string("nathan")));
    auto public_key = nathan_key.get_public_key();
    string wif_key  = graphene::utilities::key_to_wif(nathan_key); 
     
    {
    FC_MESSAGE("import- key");

    FC_ASSERT(con.wallet_api_ptr->import_key("nathan",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init0",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init1",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init2",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init3",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init4",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init5",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init6",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init7",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init8",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init9",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("init10",wif_key));
    }
    graphene::wallet::approval_delta approval;
    {

    FC_MESSAGE("add- approve");

    
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
    }

    
   { 
    account_object nathan_acct_before_upgrade = con.wallet_api_ptr->get_account("nathan");
    if (!nathan_acct_before_upgrade.is_lifetime_member())
    {
      FC_MESSAGE("Importing nathan's balance");
      std::vector<signed_transaction> import_txs = con.wallet_api_ptr->import_balance("nathan", {wif_key}, true);
      // upgrade nathan
      FC_MESSAGE("Upgrading Nathan to LTM");
      signed_transaction upgrade_tx = con.wallet_api_ptr->upgrade_account("nathan", true);
      account_object nathan_acct_after_upgrade = con.wallet_api_ptr->get_account("nathan");
    }       
   }
   {
    FC_MESSAGE("vote");

     con.wallet_api_ptr->vote_for_witness("nathan", "init0", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init1", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init2", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init3", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init4", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init5", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init6", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init7", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init8", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init9", true, true);
     con.wallet_api_ptr->vote_for_witness("nathan", "init10", true, true);    

     con.wallet_api_ptr->vote_for_committee_member("nathan", "init0", true, true);
     con.wallet_api_ptr->vote_for_committee_member("nathan", "init1", true, true);
     con.wallet_api_ptr->vote_for_committee_member("nathan", "init2", true, true);
     con.wallet_api_ptr->vote_for_committee_member("nathan", "init3", true, true);
     con.wallet_api_ptr->vote_for_committee_member("nathan", "init4", true, true);
     con.wallet_api_ptr->vote_for_committee_member("nathan", "init5", true, true);
     con.wallet_api_ptr->vote_for_committee_member("nathan", "init6", true, true);
     con.wallet_api_ptr->vote_for_committee_member("nathan", "init7", true, true);
     con.wallet_api_ptr->vote_for_committee_member("nathan", "init8", true, true);
     con.wallet_api_ptr->vote_for_committee_member("nathan", "init9", true, true);
     con.wallet_api_ptr->vote_for_committee_member("nathan", "init10", true, true);

     con.wallet_api_ptr->vote_for_budget_member("nathan", "init0", true, true);
     con.wallet_api_ptr->vote_for_budget_member("nathan", "init1", true, true);
     con.wallet_api_ptr->vote_for_budget_member("nathan", "init2", true, true);
     con.wallet_api_ptr->vote_for_budget_member("nathan", "init3", true, true);
     con.wallet_api_ptr->vote_for_budget_member("nathan", "init4", true, true);
     con.wallet_api_ptr->vote_for_budget_member("nathan", "init5", true, true);
     con.wallet_api_ptr->vote_for_budget_member("nathan", "init6", true, true);
     con.wallet_api_ptr->vote_for_budget_member("nathan", "init7", true, true);
     con.wallet_api_ptr->vote_for_budget_member("nathan", "init8", true, true);
     con.wallet_api_ptr->vote_for_budget_member("nathan", "init9", true, true);
     con.wallet_api_ptr->vote_for_budget_member("nathan", "init10", true, true);
   }
   {
     FC_MESSAGE("transfer balance");

     con.wallet_api_ptr->transfer("nathan", "committee-account", "1000", "ZOS", "", true);

     con.wallet_api_ptr->transfer("nathan", "init0",  "100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init1",  "100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init2",  "100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init3",  "100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init4",  "100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init5",  "100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init6",  "100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init7",  "100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init8",  "100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init9",  "100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "init10", "100", "ZOS", "", true);   
   }  

     fc::usleep(fc::seconds(isleep));
     {
     FC_MESSAGE("change param");
     {
      string s_cny = "{\"committee_proposal_review_period\":3600,\"maximum_proposal_lifetime\":10800,\"maintenance_interval\":300}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);

      dynamic_global_property_object dy = con.wallet_api_ptr->get_dynamic_global_properties();
      global_property_object         gl = con.wallet_api_ptr->get_global_properties();
      con.wallet_api_ptr->propose_parameter_change("nathan", dy.time + gl.parameters.maximum_proposal_lifetime, type_object ,"change param", true);
      con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }    
     }

     fc::usleep(fc::seconds(isleep));  
    {
     FC_MESSAGE("register gateway");

    con.wallet_api_ptr->create_account("gateway-cny","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("gateway-usd","gateway-pwd","nathan","nathan",20,true);    
    con.wallet_api_ptr->create_account("gateway-usdt","gateway-pwd","nathan","nathan",20,true);  
    con.wallet_api_ptr->create_account("gateway-eth","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("gateway-btc","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-cny1","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-cny2","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-cny3","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-cny4","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-usd1","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-usd2","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-usd3","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-usdt1","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-usdt2","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-usdt3","gateway-pwd","nathan","nathan",20,true);

   

    con.wallet_api_ptr->import_password("gateway-cny","gateway-pwd");
    con.wallet_api_ptr->import_password("gateway-usd","gateway-pwd");    
    con.wallet_api_ptr->import_password("gateway-usdt","gateway-pwd");   
    con.wallet_api_ptr->import_password("gateway-eth","gateway-pwd");
    con.wallet_api_ptr->import_password("gateway-btc","gateway-pwd");

    con.wallet_api_ptr->import_password("carrier-cny1","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-cny2","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-cny3","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-cny4","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-usd1","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-usd2","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-usd3","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-usdt1","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-usdt2","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-usdt3","gateway-pwd");
    
     con.wallet_api_ptr->transfer("nathan", "gateway-cny",  "51100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "gateway-usd",  "51100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "gateway-usdt",  "51100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "gateway-btc",  "51100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "gateway-eth",  "51100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "carrier-cny1", "51100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "carrier-cny2", "51100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "carrier-cny3", "51100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "carrier-cny4", "51100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "carrier-usd1", "51100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "carrier-usd2", "51100", "ZOS", "", true);     
     con.wallet_api_ptr->transfer("nathan", "carrier-usd3", "51100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "carrier-usdt1", "51100", "ZOS", "", true);
     con.wallet_api_ptr->transfer("nathan", "carrier-usdt2", "51100", "ZOS", "", true);     
     con.wallet_api_ptr->transfer("nathan", "carrier-usdt3", "51100", "ZOS", "", true);     

     con.wallet_api_ptr->upgrade_account("gateway-cny", true);
     con.wallet_api_ptr->upgrade_account("gateway-usd", true);     
     con.wallet_api_ptr->upgrade_account("gateway-usdt", true);     
     con.wallet_api_ptr->upgrade_account("gateway-btc", true);
     con.wallet_api_ptr->upgrade_account("gateway-eth", true);
     con.wallet_api_ptr->upgrade_account("carrier-cny1", true);
     con.wallet_api_ptr->upgrade_account("carrier-cny2", true);
     con.wallet_api_ptr->upgrade_account("carrier-cny3", true);
     con.wallet_api_ptr->upgrade_account("carrier-cny4", true);
     con.wallet_api_ptr->upgrade_account("carrier-usd1", true);
     con.wallet_api_ptr->upgrade_account("carrier-usd2", true);      
     con.wallet_api_ptr->upgrade_account("carrier-usd3", true);     
     con.wallet_api_ptr->upgrade_account("carrier-usdt1", true);
     con.wallet_api_ptr->upgrade_account("carrier-usdt2", true);      
     con.wallet_api_ptr->upgrade_account("carrier-usdt3", true);     


    FC_MESSAGE("create gateway proposal");

    con.wallet_api_ptr->create_account("gateway-cny-proposal","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("gateway-usd-proposal","gateway-pwd","nathan","nathan",20,true);    
    con.wallet_api_ptr->create_account("gateway-usdt-proposal","gateway-pwd","nathan","nathan",20,true);  
    con.wallet_api_ptr->create_account("gateway-eth-proposal","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("gateway-btc-proposal","gateway-pwd","nathan","nathan",20,true);

    con.wallet_api_ptr->import_password("gateway-cny-proposal","gateway-pwd");
    con.wallet_api_ptr->import_password("gateway-usd-proposal","gateway-pwd");    
    con.wallet_api_ptr->import_password("gateway-usdt-proposal","gateway-pwd");   
    con.wallet_api_ptr->import_password("gateway-eth-proposal","gateway-pwd");
    con.wallet_api_ptr->import_password("gateway-btc-proposal","gateway-pwd");
    
    con.wallet_api_ptr->transfer("nathan", "gateway-cny-proposal",  "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "gateway-usd-proposal",  "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "gateway-usdt-proposal", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "gateway-btc-proposal",  "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "gateway-eth-proposal",  "100", "ZOS", "", true);

     fc::usleep(fc::seconds(isleep));
  
     FC_MESSAGE("create member");
   
     con.wallet_api_ptr->create_gateway("gateway-cny", "http://gateway.zos.io", {"CNY"}, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_gateway("gateway-usd", "http://gateway.zos.io", {"USD"}, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_gateway("gateway-usdt", "http://gateway.zos.io", {"USDT"}, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_gateway("gateway-btc", "http://gateway.zos.io", {"BTC"}, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_gateway("gateway-eth", "http://gateway.zos.io", {"ETH"}, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);

     con.wallet_api_ptr->create_carrier("carrier-cny1", "carrier-cny1 url", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-cny2", "carrier-cny2 url", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-cny3", "carrier-cny3 url", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-cny4", "carrier-cny4 url", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-usd1", "carrier-usd1 url", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-usd2", "carrier-usd2 url", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true); 
     con.wallet_api_ptr->create_carrier("carrier-usd3", "carrier-usd3 url", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true); 
     con.wallet_api_ptr->create_carrier("carrier-usdt1", "carrier-usdt1 url", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-usdt2", "carrier-usdt2 url", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true); 
     con.wallet_api_ptr->create_carrier("carrier-usdt3", "carrier-usdt3 url", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true); 


    
     FC_MESSAGE("gateway_issue_currency");
     fc::usleep(fc::seconds(isleep));
     {
       auto a_cny = con.wallet_api_ptr->get_asset("CNY");
       con.wallet_api_ptr->gateway_issue_currency("nathan", "gateway-cny", "100000", "CNY", true,true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }
     {
       auto a_usd = con.wallet_api_ptr->get_asset("USD");
       con.wallet_api_ptr->gateway_issue_currency("nathan", "gateway-usd", "100000", "USD",true, true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }
     {
       auto a_usd = con.wallet_api_ptr->get_asset("USDT");
       con.wallet_api_ptr->gateway_issue_currency("nathan", "gateway-usdt", "100000", "USDT",true, true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }
     {
       auto a_btc = con.wallet_api_ptr->get_asset("BTC");
       con.wallet_api_ptr->gateway_issue_currency("nathan", "gateway-btc", "100000", "BTC",true, true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }
     {
       auto a_cny = con.wallet_api_ptr->get_asset("ETH");
       con.wallet_api_ptr->gateway_issue_currency("nathan", "gateway-eth", "100000", "ETH", true,true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }         

    }

    {
     fc::usleep(fc::seconds(isleep));

     FC_MESSAGE("create feeder");

    asset_object  a_cny  =  con.wallet_api_ptr->get_asset("CNY");
    asset_object  a_usd  =  con.wallet_api_ptr->get_asset("USD");
    asset_object  a_usdt =  con.wallet_api_ptr->get_asset("USDT");
    asset_object  a_btc  =  con.wallet_api_ptr->get_asset("BTC");
    asset_object  a_eth  =  con.wallet_api_ptr->get_asset("ETH");

    con.wallet_api_ptr->create_account("feeder-0", "zos2018feed0", "nathan", "nathan" ,20, true);
    con.wallet_api_ptr->create_account("feeder-1", "zos2018feed1", "nathan", "nathan" ,20, true);
    con.wallet_api_ptr->create_account("feeder-2", "zos2018feed2", "nathan", "nathan" ,20, true);
    con.wallet_api_ptr->create_account("feeder-3", "zos2018feed3", "nathan", "nathan" ,20, true);
    con.wallet_api_ptr->create_account("feeder-4", "zos2018feed4", "nathan", "nathan" ,20, true);
    con.wallet_api_ptr->create_account("feeder-5", "zos2018feed5", "nathan", "nathan" ,20, true);
    con.wallet_api_ptr->create_account("feeder-6", "zos2018feed6", "nathan", "nathan" ,20, true);
    con.wallet_api_ptr->create_account("feeder-7", "zos2018feed7", "nathan", "nathan" ,20, true);
    

    con.wallet_api_ptr->import_password("feeder-0", "zos2018feed0");
    con.wallet_api_ptr->import_password("feeder-1", "zos2018feed1");
    con.wallet_api_ptr->import_password("feeder-2", "zos2018feed2");
    con.wallet_api_ptr->import_password("feeder-3", "zos2018feed3");
    con.wallet_api_ptr->import_password("feeder-4", "zos2018feed4");
    con.wallet_api_ptr->import_password("feeder-5", "zos2018feed5");
    con.wallet_api_ptr->import_password("feeder-6", "zos2018feed6");
    con.wallet_api_ptr->import_password("feeder-7", "zos2018feed7");
   
    con.wallet_api_ptr->transfer("nathan", "feeder-0", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-1", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-2", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-3", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-4", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-5", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-6", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-7", "100", "ZOS", "", true);  


    {
      uint32_t flags = a_cny.options.flags;
      flags &= ~(witness_fed_asset | committee_fed_asset);
      variant type = fc::json::from_string("{\"flags\":"+fc::to_string(flags)+"}");
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_asset("nathan","CNY","",type_object,true);
      con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    }
    {
      uint32_t flags = a_usd.options.flags;
      flags &= ~(witness_fed_asset | committee_fed_asset);
      variant type = fc::json::from_string("{\"flags\":"+fc::to_string(flags)+"}");
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_asset("nathan","USD","",type_object,true);
      con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    }
    {
      uint32_t flags = a_usdt.options.flags;
      flags &= ~(witness_fed_asset | committee_fed_asset);
      variant type = fc::json::from_string("{\"flags\":"+fc::to_string(flags)+"}");
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_asset("nathan","USDT","",type_object,true);
      con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    }
    {
      uint32_t flags = a_btc.options.flags;
      flags &= ~(witness_fed_asset | committee_fed_asset);
      variant type = fc::json::from_string("{\"flags\":"+fc::to_string(flags)+"}");
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_asset("nathan","BTC","",type_object,true);
      con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    }
    {
      uint32_t flags = a_eth.options.flags;
      flags &= ~(witness_fed_asset | committee_fed_asset);
      variant type = fc::json::from_string("{\"flags\":"+fc::to_string(flags)+"}");
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_asset("nathan","ETH","",type_object,true);
      con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    }
    }

    {
    fc::usleep(fc::seconds(isleep*2));
    FC_MESSAGE("set feeder");

    con.wallet_api_ptr->propose_asset_feed_producers("nathan", "CNY", {"feeder-0","feeder-1", "feeder-2", "feeder-3", "feeder-4", "feeder-5", "feeder-6", "feeder-7"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->propose_asset_feed_producers("nathan", "USD", {"feeder-0","feeder-1", "feeder-2", "feeder-3", "feeder-4", "feeder-5", "feeder-6", "feeder-7"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->propose_asset_feed_producers("nathan", "USDT", {"feeder-0","feeder-1", "feeder-2", "feeder-3", "feeder-4", "feeder-5", "feeder-6", "feeder-7"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->propose_asset_feed_producers("nathan", "BTC", {"feeder-0","feeder-1", "feeder-2", "feeder-3", "feeder-4", "feeder-5", "feeder-6", "feeder-7"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->propose_asset_feed_producers("nathan", "ETH", {"feeder-0","feeder-1", "feeder-2", "feeder-3", "feeder-4", "feeder-5", "feeder-6", "feeder-7"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);

    fc::usleep(fc::seconds(isleep*2));


    FC_MESSAGE("publish_asset_feed"); 
    {
       auto a_asset = con.wallet_api_ptr->get_asset("CNY");
       price_feed feed;
       feed.settlement_price = price(asset(87, a_asset.id), asset(28769, GRAPHENE_CORE_ASSET));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.maintenance_collateral_cash_ratio = 1450;
       feed.maximum_short_squeeze_cash_ratio = 1100;
       feed.core_exchange_rate = price(asset(164, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));
       
        feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-0"  , "CNY", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-1"  , "CNY", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-2"  , "CNY", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-3"  , "CNY", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-4"  , "CNY", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-5"  , "CNY", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-6"  , "CNY", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-7"  , "CNY", feed, true);    
       
     }

     {
       auto a_asset = con.wallet_api_ptr->get_asset("USD");
       price_feed feed;
       feed.settlement_price = price(asset(25, a_asset.id), asset(63647, GRAPHENE_CORE_ASSET));
       feed.maintenance_collateral_ratio = 1751;       
       feed.maximum_short_squeeze_ratio = 1501;
       feed.maintenance_collateral_cash_ratio = 1451;
       feed.maximum_short_squeeze_cash_ratio = 1101;
       feed.core_exchange_rate = price(asset(164, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-0" , "USD", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-1", "USD", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-2" , "USD", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-3" , "USD", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-4" , "USD", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-5" , "USD", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-6" , "USD", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-7" , "USD", feed, true);
       
     }
      {
       auto a_asset = con.wallet_api_ptr->get_asset("USDT");
       price_feed feed;
       feed.settlement_price = price(asset(25, a_asset.id), asset(63647, GRAPHENE_CORE_ASSET));
       feed.maintenance_collateral_ratio = 1751;       
       feed.maximum_short_squeeze_ratio = 1501;
       feed.maintenance_collateral_cash_ratio = 1451;
       feed.maximum_short_squeeze_cash_ratio = 1101;
       feed.core_exchange_rate = price(asset(164, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-0" , "USDT", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-1", "USDT", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-2" , "USDT", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-3" , "USDT", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-4" , "USDT", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-5" , "USDT", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-6" , "USDT", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-7" , "USDT", feed, true);
       
     }

      {
       auto a_asset = con.wallet_api_ptr->get_asset("BTC");
       price_feed feed;
       feed.settlement_price = price(asset(559, a_asset.id), asset(43613, GRAPHENE_CORE_ASSET));
       feed.maintenance_collateral_ratio = 1751;       
       feed.maximum_short_squeeze_ratio = 1501;
       feed.maintenance_collateral_cash_ratio = 1451;
       feed.maximum_short_squeeze_cash_ratio = 1101;
       feed.core_exchange_rate = price(asset(123, a_asset.id), asset(9360, GRAPHENE_CORE_ASSET));

       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-0" , "BTC", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-1" , "BTC", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-2" , "BTC", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-3" , "BTC", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-4" , "BTC", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-5" , "BTC", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-6" , "BTC", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-7" , "BTC", feed, true);
     
     }

      {
       auto a_asset = con.wallet_api_ptr->get_asset("ETH");
       price_feed feed;
       feed.settlement_price = price(asset(600, a_asset.id), asset(43613, GRAPHENE_CORE_ASSET));
       feed.maintenance_collateral_ratio = 1751;       
       feed.maximum_short_squeeze_ratio = 1501;
       feed.maintenance_collateral_cash_ratio = 1451;
       feed.maximum_short_squeeze_cash_ratio = 1101;
       feed.core_exchange_rate = price(asset(123, a_asset.id), asset(9360, GRAPHENE_CORE_ASSET));
        
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-0" , "ETH", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-1" , "ETH", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-2" , "ETH", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-3" , "ETH", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-4" , "ETH", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-5" , "ETH", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-6" , "ETH", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-7" , "ETH", feed, true);
       
     }
    }
  
   {
     FC_MESSAGE("create faucet");
     con.wallet_api_ptr->create_account("onboarding.zos.foundation", "zos2018-faucet", "nathan", "nathan" ,20, true);    
     con.wallet_api_ptr->import_password("onboarding.zos.foundation", "zos2018-faucet");    
     con.wallet_api_ptr->transfer("nathan", "onboarding.zos.foundation", "60000", "ZOS", "", true);
     con.wallet_api_ptr->upgrade_account("onboarding.zos.foundation", true);         
   }

   {
    FC_MESSAGE("create_bitlender_option");

    con.wallet_api_ptr->create_account("cny-rate-feeder", "rate-feeder", "nathan", "nathan" ,20, true);
    con.wallet_api_ptr->create_account("usd-rate-feeder", "rate-feeder", "nathan", "nathan" ,20, true);
    con.wallet_api_ptr->create_account("usdt-rate-feeder", "rate-feeder", "nathan", "nathan" ,20, true);
    con.wallet_api_ptr->import_password("cny-rate-feeder", "rate-feeder");
    con.wallet_api_ptr->import_password("usd-rate-feeder", "rate-feeder");    
    con.wallet_api_ptr->import_password("usdt-rate-feeder", "rate-feeder");    

    con.wallet_api_ptr->transfer("nathan", "cny-rate-feeder",  "10", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "usd-rate-feeder",  "10", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "usdt-rate-feeder", "10", "ZOS", "", true);
    {       
       auto a_f = con.wallet_api_ptr->get_account("cny-rate-feeder");
       auto a_1 = con.wallet_api_ptr->get_account("carrier-cny1");
       auto a_2 = con.wallet_api_ptr->get_account("carrier-cny2");
       auto a_3 = con.wallet_api_ptr->get_account("carrier-cny3");
       auto a_4 = con.wallet_api_ptr->get_account("carrier-cny4");

       string option = "";  
       string s_cny = "{\"issuer_rate\":\""+a_f.id.to_string()+"\",\"carriers\":[\""+a_1.id.to_string()+"\",\""+a_2.id.to_string()+"\",\""+a_3.id.to_string()+"\",\""+a_4.id.to_string()+"\"],"+option+"}";
       variant type = fc::json::from_string(s_cny);
       variant_object type_object;
       fc::from_variant(type, type_object);
       con.wallet_api_ptr->create_bitlender_option("nathan", "CNY", 0,type_object, true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }
     {
       auto a_f = con.wallet_api_ptr->get_account("usd-rate-feeder");
       auto a_1 = con.wallet_api_ptr->get_account("carrier-usd1");
       auto a_2 = con.wallet_api_ptr->get_account("carrier-usd2");
       auto a_3 = con.wallet_api_ptr->get_account("carrier-usd3");
       string option = "";  
       string s_cny = "{\"issuer_rate\":\""+a_f.id.to_string()+"\",\"carriers\":[\""+a_1.id.to_string()+"\",\""+a_2.id.to_string()+"\",\""+a_3.id.to_string()+"\"],"+option+"}";
       variant type = fc::json::from_string(s_cny);
       variant_object type_object;
       fc::from_variant(type, type_object);
       con.wallet_api_ptr->create_bitlender_option("nathan", "USD", 0,type_object, true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }

      {
       auto a_f = con.wallet_api_ptr->get_account("usdt-rate-feeder");
       auto a_1 = con.wallet_api_ptr->get_account("carrier-usdt1");
       auto a_2 = con.wallet_api_ptr->get_account("carrier-usdt2");
       auto a_3 = con.wallet_api_ptr->get_account("carrier-usdt3");
       string option = "\"min_invest_amount\":\"100000000000\",\"min_loan_amount\":\"100000000000\",\"min_invest_increase_range\":\"10000000000\",\"min_loan_increase_range\":\"10000000000\"";  
       string s_cny = "{\"issuer_rate\":\""+a_f.id.to_string()+"\","+option+",\"carriers\":[\""+a_1.id.to_string()+"\",\""+a_2.id.to_string()+"\",\""+a_3.id.to_string()+"\"]}";
       variant type = fc::json::from_string(s_cny);
       variant_object type_object;
       fc::from_variant(type, type_object);
       con.wallet_api_ptr->create_bitlender_option("nathan", "USDT", 0,type_object, true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }


     FC_MESSAGE("set_rate_option");
     {
      string s_cny = "{\"1\":{\"interest_rate\":70},\"2\":{\"interest_rate\":75},\"3\":{\"interest_rate\":80},\"4\":{\"interest_rate\":85},\"5\":{\"interest_rate\":90},\"6\":{\"interest_rate\":100},\"7\":{\"interest_rate\":105}}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_bitlender_rate("cny-rate-feeder", "CNY",type_object, {}, true);
    }
    {
      string s_cny = "{\"1\":{\"interest_rate\":74},\"2\":{\"interest_rate\":76},\"3\":{\"interest_rate\":81},\"4\":{\"interest_rate\":82},\"5\":{\"interest_rate\":91},\"6\":{\"interest_rate\":101},\"7\":{\"interest_rate\":115}}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_bitlender_rate("usd-rate-feeder", "USD", 0,type_object,{}, true);
    }
    {
      string s_cny = "{\"1\":{\"interest_rate\":74},\"2\":{\"interest_rate\":76},\"3\":{\"interest_rate\":81},\"4\":{\"interest_rate\":82},\"5\":{\"interest_rate\":91},\"6\":{\"interest_rate\":101},\"7\":{\"interest_rate\":115}}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_bitlender_rate("usdt-rate-feeder", "USDT", 0,type_object,{}, true);
    }

     fc::usleep(fc::seconds(isleep));

     con.wallet_api_ptr->create_account("council-usd1", "zos-council", "nathan", "nathan" ,20, true);
     con.wallet_api_ptr->create_account("council-usd2", "zos-council", "nathan", "nathan" ,20, true);
     
     con.wallet_api_ptr->create_account("council-usdt1", "zos-council", "nathan", "nathan" ,20, true);
     con.wallet_api_ptr->create_account("council-usdt2", "zos-council", "nathan", "nathan" ,20, true);
  

     con.wallet_api_ptr->create_account("council-cny1", "zos-council", "nathan", "nathan" ,20, true);
     con.wallet_api_ptr->create_account("council-cny2", "zos-council", "nathan", "nathan" ,20, true);
     

    con.wallet_api_ptr->import_password("council-usd1", "zos-council");
    con.wallet_api_ptr->import_password("council-usd2", "zos-council");

    con.wallet_api_ptr->import_password("council-usdt1", "zos-council");
    con.wallet_api_ptr->import_password("council-usdt2", "zos-council");
    
    con.wallet_api_ptr->import_password("council-cny1", "zos-council");
    con.wallet_api_ptr->import_password("council-cny2", "zos-council");
    

    con.wallet_api_ptr->transfer("nathan", "council-usd1", "10", "ZOS", "", true);  
    con.wallet_api_ptr->transfer("nathan", "council-usd2", "10", "ZOS", "", true);  

    con.wallet_api_ptr->transfer("nathan", "council-usdt1", "10", "ZOS", "", true);  
    con.wallet_api_ptr->transfer("nathan", "council-usdt2", "10", "ZOS", "", true);  
    
    con.wallet_api_ptr->transfer("nathan", "council-cny1", "10", "ZOS", "", true);  
    con.wallet_api_ptr->transfer("nathan", "council-cny2", "10", "ZOS", "", true);  
    
    
    con.wallet_api_ptr->transfer("nathan", "council-usd1", "50000", "ZOS", "", true);     
    con.wallet_api_ptr->upgrade_account("council-usd1", true);

    con.wallet_api_ptr->transfer("nathan", "council-usd2", "50000", "ZOS", "", true);     
    con.wallet_api_ptr->upgrade_account("council-usd2", true);

    con.wallet_api_ptr->transfer("nathan", "council-usdt1", "50000", "ZOS", "", true);     
    con.wallet_api_ptr->upgrade_account("council-usdt1", true);

    con.wallet_api_ptr->transfer("nathan", "council-usdt2", "50000", "ZOS", "", true);     
    con.wallet_api_ptr->upgrade_account("council-usdt2", true);

    con.wallet_api_ptr->transfer("nathan", "council-cny1", "50000", "ZOS", "", true);     
    con.wallet_api_ptr->upgrade_account("council-cny1", true);

    con.wallet_api_ptr->transfer("nathan", "council-cny2", "50000", "ZOS", "", true);     
    con.wallet_api_ptr->upgrade_account("council-cny2", true);


    con.wallet_api_ptr->update_bitlender_option_author("nathan", "CNY", {"gateway-cny", "carrier-cny1", "carrier-cny2", "carrier-cny3", "carrier-cny4","council-cny1","council-cny2"}, 5, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);

    con.wallet_api_ptr->update_bitlender_option_author("nathan", "USD", {"gateway-usd", "carrier-usd1", "carrier-usd2", "carrier-usd3","council-usd1","council-usd2"}, 4, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);

    con.wallet_api_ptr->update_bitlender_option_author("nathan", "USDT", {"gateway-usdt", "carrier-usdt1", "carrier-usdt2", "carrier-usdt3","council-usdt1","council-usdt2"}, 4, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
   }

   { 
    FC_MESSAGE("trans balances");
    con.wallet_api_ptr->create_account("committee-group", "group-feed", "nathan", "nathan" ,20, true);
    con.wallet_api_ptr->create_account("business-group",  "group-feed", "nathan", "nathan" ,20, true);
    con.wallet_api_ptr->import_password("committee-group", "group-feed");
    con.wallet_api_ptr->import_password("business-group",  "group-feed");    
    con.wallet_api_ptr->transfer("nathan", "committee-group", "500000", "ZOS", "", true);     
    con.wallet_api_ptr->upgrade_account("committee-group", true);
    con.wallet_api_ptr->transfer("nathan", "business-group", "500000", "ZOS", "", true);     
    con.wallet_api_ptr->upgrade_account("business-group", true);

    vector<asset_summary> asend = con.wallet_api_ptr->list_account_balances("nathan");
    share_type remain = asend[0].amount/100000;
    con.wallet_api_ptr->transfer("nathan", "business-group",  fc::to_string(remain.value/2), "ZOS", "", true); 
    con.wallet_api_ptr->transfer("nathan", "committee-group", fc::to_string(remain.value/2), "ZOS", "", true); 


    FC_MESSAGE("change_password");
    fc::usleep(fc::seconds(isleep));

    //理事会，见证人，预算委员会
    con.wallet_api_ptr->change_password("nathan", "zos2018-nathan", true);
    con.wallet_api_ptr->change_password("init0", "zos2018-init0", true);
    con.wallet_api_ptr->change_password("init1", "zos2018-init1", true);
    con.wallet_api_ptr->change_password("init2", "zos2018-init2", true);
    con.wallet_api_ptr->change_password("init3", "zos2018-init3", true);
    con.wallet_api_ptr->change_password("init4", "zos2018-init4", true);
    con.wallet_api_ptr->change_password("init5", "zos2018-init5", true);
    con.wallet_api_ptr->change_password("init6", "zos2018-init6", true);
    con.wallet_api_ptr->change_password("init7", "zos2018-init7", true);
    con.wallet_api_ptr->change_password("init8", "zos2018-init8", true);
    con.wallet_api_ptr->change_password("init9", "zos2018-init8", true);
    con.wallet_api_ptr->change_password("init10", "zos2018-init10", true);
    fc::usleep(fc::seconds(isleep));   

    //喂价人
    con.wallet_api_ptr->change_password("feeder-0", "zos2018-feed",true); 
    con.wallet_api_ptr->change_password("feeder-1", "zos2018-feed",true);
    con.wallet_api_ptr->change_password("feeder-2", "zos2018-feed",true);
    con.wallet_api_ptr->change_password("feeder-3", "zos2018-feed",true);
    con.wallet_api_ptr->change_password("feeder-4", "zos2018-feed",true);
    con.wallet_api_ptr->change_password("feeder-5", "zos2018-feed",true);
    con.wallet_api_ptr->change_password("feeder-6", "zos2018-feed",true);
    con.wallet_api_ptr->change_password("feeder-7", "zos2018-feed",true);
        
   //水龙头
    con.wallet_api_ptr->change_password("onboarding.zos.foundation", "zos2018-faucet",true);   

  //网关
    con.wallet_api_ptr->change_password("gateway-cny","gateway-pwd-cny",true);
    con.wallet_api_ptr->change_password("gateway-usd","gateway-pwd-usd",true);    
    con.wallet_api_ptr->change_password("gateway-usdt","gateway-pwd-usdt",true);    
    con.wallet_api_ptr->change_password("gateway-eth","gateway-pwd-eth",true);
    con.wallet_api_ptr->change_password("gateway-btc","gateway-pwd-btc",true);
  //网关提案用户
    con.wallet_api_ptr->change_password("gateway-cny-proposal","gateway-cny-proposal",true);
    con.wallet_api_ptr->change_password("gateway-usd-proposal","gateway-usd-proposal",true);    
    con.wallet_api_ptr->change_password("gateway-usdt-proposal","gateway-usdt-proposal",true);   
    con.wallet_api_ptr->change_password("gateway-eth-proposal","gateway-eth-proposal",true);
    con.wallet_api_ptr->change_password("gateway-btc-proposal","gateway-btc-proposal",true);
  //运营商  
    con.wallet_api_ptr->change_password("carrier-cny1","carrier-pwd-cny1",true);
    con.wallet_api_ptr->change_password("carrier-cny2","carrier-pwd-cny2",true);
    con.wallet_api_ptr->change_password("carrier-cny3","carrier-pwd-cny3",true);
    con.wallet_api_ptr->change_password("carrier-cny4","carrier-pwd-cny4",true);
    con.wallet_api_ptr->change_password("carrier-usd1","carrier-pwd-usd1",true);
    con.wallet_api_ptr->change_password("carrier-usd2","carrier-pwd-usd2",true);
    con.wallet_api_ptr->change_password("carrier-usd3","carrier-pwd-usd3",true);
    con.wallet_api_ptr->change_password("carrier-usdt1","carrier-pwd-usdt1",true);
    con.wallet_api_ptr->change_password("carrier-usdt2","carrier-pwd-usdt2",true);
    con.wallet_api_ptr->change_password("carrier-usdt3","carrier-pwd-usdt3",true);

  //借贷利率修改人
    con.wallet_api_ptr->change_password("cny-rate-feeder",  "cny-rate-feeder",true);
    con.wallet_api_ptr->change_password("usd-rate-feeder",  "usd-rate-feeder",true);    
    con.wallet_api_ptr->change_password("usdt-rate-feeder", "usdt-rate-feeder",true);    

  //借贷小理事会成员
    con.wallet_api_ptr->change_password("council-usd1", "council-usd1",true);
    con.wallet_api_ptr->change_password("council-usd2", "council-usd2",true);

    con.wallet_api_ptr->change_password("council-usdt1", "council-usdt1",true);
    con.wallet_api_ptr->change_password("council-usdt2", "council-usdt2",true);
    
    con.wallet_api_ptr->change_password("council-cny1", "council-cny1",true);
    con.wallet_api_ptr->change_password("council-cny2", "council-cny2",true);
  //基金会组长
    con.wallet_api_ptr->change_password("committee-group", "committee-group",true);
  //运营团队组长  
    con.wallet_api_ptr->change_password("business-group",  "business-group",true);

   } 



   con.close();
   fc::usleep(fc::seconds(1));
  }
  catch (fc::exception &e)
  {
    edump((e.to_detail_string()));
    throw;
  }  
}

int main( int argc, char** argv )
 {
   try
   {
      bpo::options_description cli_options("Graphene empty blocks");
      cli_options.add_options()         
            ("ip,i", bpo::value<string>()->default_value("ws://127.0.0.1:8290"), "connect ip")            
            ("account,a", bpo::value<uint64_t>()->default_value(1112), "account count") 
            ("asset,s", bpo::value<uint64_t>()->default_value(300), "asset count") 
            ("worker,w", bpo::value<uint64_t>()->default_value(100), "worker count") 
            ("loan,w", bpo::value<uint64_t>()->default_value(100), "loan count")
            ("member,m", bpo::value<uint64_t>()->default_value(1001), "member count") 
            ("init,i", bpo::value<bool>()->default_value(0), "init block") 
            ("node,n", bpo::value<bool>()->default_value(0), "init node")             
            ("add,d", bpo::value<bool>()->default_value(0), "add data") 
            ("trans,t", bpo::value<bool>()->default_value(0), "trans data") 
            ;
      bpo::variables_map options;
      try
      {
         boost::program_options::store( boost::program_options::parse_command_line(argc, argv, cli_options), options );
      }
      catch (const boost::program_options::error& e)
      {
         std::cerr << "empty_blocks:  error parsing command line: " << e.what() << "\n";
         return 0;
      }
      if (options.count("ip"))
        g_server = options.at("ip").as<string>();     
      if (options.count("account"))
        g_uaccount = options.at("account").as<uint64_t>();
      if (options.count("asset"))
        g_uasset = options.at("asset").as<uint64_t>();
      if (options.count("worker"))
        g_uworker = options.at("worker").as<uint64_t>();
      if (options.count("loan"))
        g_ulender = options.at("loan").as<uint64_t>();
      if (options.count("member"))
        g_umember = options.at("member").as<uint64_t>();

      std::cerr << "ip " << g_server << "\n";
      std::cerr << "account " << g_uaccount << "\n";
      std::cerr << "asset " << g_uasset << "\n";
      std::cerr << "worker " << g_uworker << "\n";
      std::cerr << "loan " << g_ulender << "\n";
      std::cerr << "member " << g_umember << "\n";

      if (options.count("init"))
      {
        bool binit = options.at("init").as<bool>();
        if (binit)
          init_block();
      }
      if (options.count("node"))
      {
        bool binit = options.at("node").as<bool>();
        if (binit)
          init_node();
      }
      if (options.count("add"))
      {
        bool binit = options.at("add").as<bool>();
        if (binit)
          create_account();
      }
      if (options.count("trans"))
      {
        bool binit = options.at("trans").as<bool>();
        if (binit)
          trans();
      }
   }
   catch(...)
   {
   }      
   return 0;
}
*/