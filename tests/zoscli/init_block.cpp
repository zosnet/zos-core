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
 
#include "zoscli.hpp"
   
 
void create_block_asset_feed(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)

{
     fc::usleep(fc::seconds(option.g_isleep));

     FC_MESSAGE("set asset feeders");

    asset_object  a_cny  =  con.wallet_api_ptr->get_asset("CNY");
    asset_object  a_usd  =  con.wallet_api_ptr->get_asset("USD");
    asset_object  a_usdt =  con.wallet_api_ptr->get_asset("USDT");
    asset_object  a_php  =  con.wallet_api_ptr->get_asset("PHP");
    asset_object  a_cad  =  con.wallet_api_ptr->get_asset("CAD");
    asset_object  a_btc  =  con.wallet_api_ptr->get_asset("BTC");
    asset_object  a_eth  =  con.wallet_api_ptr->get_asset("ETH");    


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
      uint32_t flags = a_cad.options.flags;
      flags &= ~(witness_fed_asset | committee_fed_asset);
      variant type = fc::json::from_string("{\"flags\":"+fc::to_string(flags)+"}");
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_asset("nathan","CAD","",type_object,true);
      con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    }
    {
      uint32_t flags = a_php.options.flags;
      flags &= ~(witness_fed_asset | committee_fed_asset);
      variant type = fc::json::from_string("{\"flags\":"+fc::to_string(flags)+"}");
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_asset("nathan","PHP","",type_object,true);
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
    

   
    fc::usleep(fc::seconds(option.g_isleep*2));
    FC_MESSAGE("set feeder");

    con.wallet_api_ptr->update_asset_feed_producers("nathan", "CNY", {"feeder-0","feeder-1", "feeder-2", "feeder-3", "feeder-4", "feeder-5", "feeder-6", "feeder-7"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->update_asset_feed_producers("nathan", "USD", {"feeder-0","feeder-1", "feeder-2", "feeder-3", "feeder-4", "feeder-5", "feeder-6", "feeder-7"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->update_asset_feed_producers("nathan", "CAD", {"feeder-0","feeder-1", "feeder-2", "feeder-3", "feeder-4", "feeder-5", "feeder-6", "feeder-7"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->update_asset_feed_producers("nathan", "PHP", {"feeder-0","feeder-1", "feeder-2", "feeder-3", "feeder-4", "feeder-5", "feeder-6", "feeder-7"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->update_asset_feed_producers("nathan", "USDT", {"feeder-0","feeder-1", "feeder-2", "feeder-3", "feeder-4", "feeder-5", "feeder-6", "feeder-7"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->update_asset_feed_producers("nathan", "BTC", {"feeder-0","feeder-1", "feeder-2", "feeder-3", "feeder-4", "feeder-5", "feeder-6", "feeder-7"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->update_asset_feed_producers("nathan", "ETH", {"feeder-0","feeder-1", "feeder-2", "feeder-3", "feeder-4", "feeder-5", "feeder-6", "feeder-7"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);

    fc::usleep(fc::seconds(option.g_isleep*2));


    FC_MESSAGE("publish_asset_feed"); 
    {
       auto a_asset = con.wallet_api_ptr->get_asset("CNY");
       price_feed feed;
       feed.settlement_price = price(asset(87, a_asset.id), asset(28769, GRAPHENE_CORE_ASSET));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;   
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
       auto a_asset = con.wallet_api_ptr->get_asset("PHP");
       price_feed feed;
       feed.settlement_price = price(asset(25, a_asset.id), asset(63647, GRAPHENE_CORE_ASSET));
       feed.maintenance_collateral_ratio = 1751;       
       feed.maximum_short_squeeze_ratio = 1501;   
       feed.core_exchange_rate = price(asset(164, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-0" , "PHP", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-1", "PHP", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-2" , "PHP", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-3" , "PHP", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-4" , "PHP", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-5" , "PHP", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-6" , "PHP", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-7" , "PHP", feed, true);
       
     }

        {
       auto a_asset = con.wallet_api_ptr->get_asset("CAD");
       price_feed feed;
       feed.settlement_price = price(asset(25, a_asset.id), asset(63647, GRAPHENE_CORE_ASSET));
       feed.maintenance_collateral_ratio = 1751;       
       feed.maximum_short_squeeze_ratio = 1501;   
       feed.core_exchange_rate = price(asset(164, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-0" , "CAD", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-1", "CAD", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-2" , "CAD", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-3" , "CAD", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-4" , "CAD", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-5" , "CAD", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-6" , "CAD", feed, true);
       feed.settlement_price.base.amount++;
       con.wallet_api_ptr->publish_asset_feed("feeder-7" , "CAD", feed, true);
       
     }


      {
       auto a_asset = con.wallet_api_ptr->get_asset("USDT");
       price_feed feed;
       feed.settlement_price = price(asset(25, a_asset.id), asset(63647, GRAPHENE_CORE_ASSET));
       feed.maintenance_collateral_ratio = 1751;       
       feed.maximum_short_squeeze_ratio = 1501;       
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
void create_block_admin(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
{
   con.wallet_api_ptr->create_account("admin-lending", "zos2018-lending", "nathan", "nathan", 20, true);
   con.wallet_api_ptr->import_password("admin-lending","zos2018-lending");

   con.wallet_api_ptr->create_account("admin-notify", "zos2018-notify", "nathan", "nathan", 20, true);
   con.wallet_api_ptr->import_password("admin-notify","zos2018-notify");
}

void create_block_faucet(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
 {
      FC_MESSAGE("create faucet");
      con.wallet_api_ptr->create_account("onboarding.zos.foundation", "zos2018-faucet", "nathan", "nathan", 20, true);
      con.wallet_api_ptr->import_password("onboarding.zos.foundation", "zos2018-faucet");
      con.wallet_api_ptr->transfer("nathan", "onboarding.zos.foundation", "10000", "ZOS", "", true);
      con.wallet_api_ptr->upgrade_account("onboarding.zos.foundation", true);         
}
void create_block_feeders(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
{
   FC_MESSAGE("create feeders");

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
   
   /* con.wallet_api_ptr->transfer("nathan", "feeder-0", "200", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-1", "200", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-2", "200", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-3", "200", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-4", "200", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-5", "200", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-6", "200", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-7", "200", "ZOS", "", true);*/
}

void create_block_bitlender_feed(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
{
    fc::usleep(fc::seconds(option.g_isleep));

     FC_MESSAGE("set bitlender feeder");

    asset_object  a_cny  =  con.wallet_api_ptr->get_asset("CNY");
    asset_object  a_usd  =  con.wallet_api_ptr->get_asset("USD");
    asset_object  a_usdt =  con.wallet_api_ptr->get_asset("USDT");
    asset_object  a_btc  =  con.wallet_api_ptr->get_asset("BTC");
    asset_object  a_eth  =  con.wallet_api_ptr->get_asset("ETH");

 
    string sflag   = "{\"flags\":0,\"minimum_feeds\":7,\"feed_lifetime_sec\":2000000000}";
    string sfeeder = "{\"feeder-0\",\"feeder-1\", \"feeder-2\", \"feeder-3\", \"feeder-4\", \"feeder-5\", \"feeder-6\", \"feeder-7\"}";

     variant type = fc::json::from_string(sflag);
     variant_object type_object;
     fc::from_variant(type, type_object);

     flat_set<string> feeders;

     fc::usleep(fc::seconds(option.g_isleep * 2));
     FC_MESSAGE("set feeder");
     for (int l = 0;l<=7;l++)
     {
       string feeder = "feeder-" + fc::to_string(l);
       feeders.insert(feeder);
     }

    graphene::wallet::approval_delta approval_cny;

    approval_cny.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("gateway-cny").id.to_string());
    approval_cny.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-cny1").id.to_string());
    approval_cny.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-cny2").id.to_string());
    approval_cny.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-cny3").id.to_string());
    approval_cny.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-cny4").id.to_string());    
 
    graphene::wallet::approval_delta approval_usd;

    approval_usd.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("gateway-usd").id.to_string());
    approval_usd.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-usd1").id.to_string());
    approval_usd.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-usd2").id.to_string());
    approval_usd.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-usd3").id.to_string());        
    
 
    graphene::wallet::approval_delta approval_cad;

    approval_cad.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("gateway-cad").id.to_string());
    approval_cad.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-cad1").id.to_string());
    approval_cad.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-cad2").id.to_string());
    approval_cad.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-cad3").id.to_string());      

    graphene::wallet::approval_delta approval_php;

    approval_php.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("gateway-php").id.to_string());
    approval_php.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-php1").id.to_string());
    approval_php.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-php2").id.to_string());
    approval_php.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-php3").id.to_string());      


    graphene::wallet::approval_delta approval_usdt;

    approval_usdt.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("gateway-usdt").id.to_string());
    approval_usdt.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-usdt1").id.to_string());
    approval_usdt.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-usdt2").id.to_string());
    approval_usdt.active_approvals_to_add.push_back(con.wallet_api_ptr->get_account("carrier-usdt3").id.to_string());    
    



     con.wallet_api_ptr->propose_bitlender_feed_producers("nathan", "CNY", type_object, feeders, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval_cny, true);
     con.wallet_api_ptr->propose_bitlender_feed_producers("nathan", "USD", type_object, feeders, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval_usd, true);
     con.wallet_api_ptr->propose_bitlender_feed_producers("nathan", "CAD", type_object, feeders, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval_cad, true);
     con.wallet_api_ptr->propose_bitlender_feed_producers("nathan", "PHP", type_object, feeders, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval_php, true);
     con.wallet_api_ptr->propose_bitlender_feed_producers("nathan", "USDT", type_object, feeders, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval_usdt, true); 
 

     fc::usleep(fc::seconds(option.g_isleep * 2));

     FC_MESSAGE("publish_bitlender_feed");
     {
       auto a_asset = con.wallet_api_ptr->get_asset("CNY");
       auto a_btc   = con.wallet_api_ptr->get_asset("BTC");
       auto a_eth   = con.wallet_api_ptr->get_asset("ETH");
       auto a_zos   = con.wallet_api_ptr->get_asset("ZOS");
       price_feed feed;
       
       feed.settlement_price = price(asset(820, a_asset.id), asset(28769, a_btc.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(160, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder,"CNY", feed, true);
       }    


       feed.settlement_price = price(asset(830, a_asset.id), asset(28769, a_eth.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(150, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder, "CNY",feed, true);
       }    

       feed.settlement_price = price(asset(830, a_asset.id), asset(28769, a_zos.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(150, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder,"CNY", feed, true);
       }    
       
     }

      {
       auto a_asset = con.wallet_api_ptr->get_asset("USD");
       auto a_btc   = con.wallet_api_ptr->get_asset("BTC");
       auto a_eth   = con.wallet_api_ptr->get_asset("ETH");
       auto a_zos   = con.wallet_api_ptr->get_asset("ZOS");
       price_feed feed;
       
       feed.settlement_price = price(asset(840, a_asset.id), asset(28769, a_btc.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(140, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder, "USD",feed, true);
       }    


       feed.settlement_price = price(asset(850, a_asset.id), asset(28769, a_eth.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(130, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder,"USD", feed, true);
       }    

       feed.settlement_price = price(asset(850, a_asset.id), asset(28769, a_zos.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(130, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder, "USD",feed, true);
       }    
       
     }

      {
       auto a_asset = con.wallet_api_ptr->get_asset("CAD");
       auto a_btc   = con.wallet_api_ptr->get_asset("BTC");
       auto a_eth   = con.wallet_api_ptr->get_asset("ETH");
       auto a_zos   = con.wallet_api_ptr->get_asset("ZOS");
       price_feed feed;
       
       feed.settlement_price = price(asset(840, a_asset.id), asset(28769, a_btc.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(140, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder, "CAD",feed, true);
       }    


       feed.settlement_price = price(asset(850, a_asset.id), asset(28769, a_eth.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(130, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder,"CAD", feed, true);
       }    

       feed.settlement_price = price(asset(850, a_asset.id), asset(28769, a_zos.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(130, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder, "CAD",feed, true);
       }    
       
     }

      {
       auto a_asset = con.wallet_api_ptr->get_asset("PHP");
       auto a_btc   = con.wallet_api_ptr->get_asset("BTC");
       auto a_eth   = con.wallet_api_ptr->get_asset("ETH");
       auto a_zos   = con.wallet_api_ptr->get_asset("ZOS");
       price_feed feed;
       
       feed.settlement_price = price(asset(840, a_asset.id), asset(28769, a_btc.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(140, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder, "PHP",feed, true);
       }    


       feed.settlement_price = price(asset(850, a_asset.id), asset(28769, a_eth.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(130, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder,"PHP", feed, true);
       }    

       feed.settlement_price = price(asset(850, a_asset.id), asset(28769, a_zos.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(130, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder, "PHP",feed, true);
       }    
       
     }


      {
       auto a_asset = con.wallet_api_ptr->get_asset("USDT");
       auto a_btc   = con.wallet_api_ptr->get_asset("BTC");
       auto a_eth   = con.wallet_api_ptr->get_asset("ETH");
       auto a_zos   = con.wallet_api_ptr->get_asset("ZOS");
       price_feed feed;
        
       feed.settlement_price = price(asset(86000000, a_asset.id), asset(28769, a_btc.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(120, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder, "USDT",feed, true);
       }    


       feed.settlement_price = price(asset(87000000, a_asset.id), asset(28769, a_eth.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(110, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder,"USDT", feed, true);
       }           

       feed.settlement_price = price(asset(87000000, a_asset.id), asset(28769, a_zos.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(110, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder,"USDT", feed, true);
       }           
     } 
}
 
void create_block_bitlender_option(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
 {
      FC_MESSAGE("create_bitlender_option");

      optional<variant_object> optional_def;

      con.wallet_api_ptr->create_account("cny-rate-feeder", "rate-feeder", "nathan", "nathan", 20, true);
      con.wallet_api_ptr->create_account("usd-rate-feeder", "rate-feeder", "nathan", "nathan", 20, true);
      con.wallet_api_ptr->create_account("cad-rate-feeder", "rate-feeder", "nathan", "nathan", 20, true);
      con.wallet_api_ptr->create_account("php-rate-feeder", "rate-feeder", "nathan", "nathan", 20, true);
      con.wallet_api_ptr->create_account("usdt-rate-feeder", "rate-feeder", "nathan", "nathan", 20, true);
      con.wallet_api_ptr->import_password("cny-rate-feeder", "rate-feeder");
      con.wallet_api_ptr->import_password("usd-rate-feeder", "rate-feeder");
      con.wallet_api_ptr->import_password("cad-rate-feeder", "rate-feeder");
      con.wallet_api_ptr->import_password("php-rate-feeder", "rate-feeder");
      con.wallet_api_ptr->import_password("usdt-rate-feeder", "rate-feeder");

    
      {
        auto a_f = con.wallet_api_ptr->get_account("cny-rate-feeder");
        auto a_1 = con.wallet_api_ptr->get_account("carrier-cny1");
        auto a_2 = con.wallet_api_ptr->get_account("carrier-cny2");
        auto a_3 = con.wallet_api_ptr->get_account("carrier-cny3");
        auto a_4 = con.wallet_api_ptr->get_account("carrier-cny4");

        string option = "";
        string s_cny = "{\"issuer_rate\":\"" + a_f.id.to_string() + "\",\"carriers\":[\"" + a_1.id.to_string() + "\",\"" + a_2.id.to_string() + "\",\"" + a_3.id.to_string() + "\",\"" + a_4.id.to_string() + "\"]," + option + "}";
        variant type = fc::json::from_string(s_cny);
        variant_object type_object;
        fc::from_variant(type, type_object);
        con.wallet_api_ptr->create_bitlender_option("nathan", "CNY",optional_def, type_object, true);
        con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
      }
      {
        auto a_f = con.wallet_api_ptr->get_account("usd-rate-feeder");
        auto a_1 = con.wallet_api_ptr->get_account("carrier-usd1");
        auto a_2 = con.wallet_api_ptr->get_account("carrier-usd2");
        auto a_3 = con.wallet_api_ptr->get_account("carrier-usd3");
        string option = "";
        string s_cny = "{\"issuer_rate\":\"" + a_f.id.to_string() + "\",\"carriers\":[\"" + a_1.id.to_string() + "\",\"" + a_2.id.to_string() + "\",\"" + a_3.id.to_string() + "\"]," + option + "}";
        variant type = fc::json::from_string(s_cny);
        variant_object type_object;
        fc::from_variant(type, type_object);
        con.wallet_api_ptr->create_bitlender_option("nathan", "USD", optional_def,type_object, true);
        con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true); 
      } 
      {
        auto a_f = con.wallet_api_ptr->get_account("cad-rate-feeder");
        auto a_1 = con.wallet_api_ptr->get_account("carrier-cad1");
        auto a_2 = con.wallet_api_ptr->get_account("carrier-cad2");
        auto a_3 = con.wallet_api_ptr->get_account("carrier-cad3");
        string option = "";
        string s_cny = "{\"issuer_rate\":\"" + a_f.id.to_string() + "\",\"carriers\":[\"" + a_1.id.to_string() + "\",\"" + a_2.id.to_string() + "\",\"" + a_3.id.to_string() + "\"]," + option + "}";
        variant type = fc::json::from_string(s_cny);
        variant_object type_object;
        fc::from_variant(type, type_object);
        con.wallet_api_ptr->create_bitlender_option("nathan", "CAD", optional_def,type_object, true);
        con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);        
      }             
      {
        auto a_f = con.wallet_api_ptr->get_account("php-rate-feeder");
        auto a_1 = con.wallet_api_ptr->get_account("carrier-php1");
        auto a_2 = con.wallet_api_ptr->get_account("carrier-php2");
        auto a_3 = con.wallet_api_ptr->get_account("carrier-php3");
        string option = "";
        string s_cny = "{\"issuer_rate\":\"" + a_f.id.to_string() + "\",\"carriers\":[\"" + a_1.id.to_string() + "\",\"" + a_2.id.to_string() + "\",\"" + a_3.id.to_string() + "\"]," + option + "}";
        variant type = fc::json::from_string(s_cny);
        variant_object type_object;
        fc::from_variant(type, type_object);
        con.wallet_api_ptr->create_bitlender_option("nathan", "PHP", optional_def,type_object, true);
        con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);        
      } 
      {
       auto a_f = con.wallet_api_ptr->get_account("usdt-rate-feeder");
       auto a_1 = con.wallet_api_ptr->get_account("carrier-usdt1");
       auto a_2 = con.wallet_api_ptr->get_account("carrier-usdt2");
       auto a_3 = con.wallet_api_ptr->get_account("carrier-usdt3");
       string option = "\"min_invest_amount\":\"100000000000\",\"min_loan_amount\":\"100000000000\",\"min_invest_increase_range\":\"10000000000\",\"min_loan_increase_range\":\"10000000000\",\"max_interest_rate\":\"500000000000\",\"max_risk_margin\":\"500000000000\",\"max_carrier_service_charge\":\"500000000000\"";  
       string s_cny = "{\"issuer_rate\":\""+a_f.id.to_string()+"\","+option+",\"carriers\":[\""+a_1.id.to_string()+"\",\""+a_2.id.to_string()+"\",\""+a_3.id.to_string()+"\"]}";
       variant type = fc::json::from_string(s_cny);
       variant_object type_object;
       fc::from_variant(type, type_object);
       con.wallet_api_ptr->create_bitlender_option("nathan", "USDT", optional_def,type_object, true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }

     fc::usleep(fc::seconds(option.g_isleep * 3));

     FC_MESSAGE("set_rate_option");
     {
      string s_cny = "{\"1\":{\"interest_rate\":70},\"2\":{\"interest_rate\":75},\"3\":{\"interest_rate\":80},\"4\":{\"interest_rate\":85},\"5\":{\"interest_rate\":90},\"6\":{\"interest_rate\":100},\"7\":{\"interest_rate\":105}}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_bitlender_rate("cny-rate-feeder", "CNY",optional_def,type_object, {}, true);
    }
    {
      string s_cny = "{\"1\":{\"interest_rate\":74},\"2\":{\"interest_rate\":76},\"3\":{\"interest_rate\":81},\"4\":{\"interest_rate\":82},\"5\":{\"interest_rate\":91},\"6\":{\"interest_rate\":101},\"7\":{\"interest_rate\":115}}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_bitlender_rate("usd-rate-feeder", "USD", optional_def,type_object,{}, true);
    }
    {
      string s_cny = "{\"1\":{\"interest_rate\":74},\"2\":{\"interest_rate\":76},\"3\":{\"interest_rate\":81},\"4\":{\"interest_rate\":82},\"5\":{\"interest_rate\":91},\"6\":{\"interest_rate\":101},\"7\":{\"interest_rate\":115}}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_bitlender_rate("cad-rate-feeder", "CAD", optional_def,type_object,{}, true);
    }
    {
      string s_cny = "{\"1\":{\"interest_rate\":74},\"2\":{\"interest_rate\":76},\"3\":{\"interest_rate\":81},\"4\":{\"interest_rate\":82},\"5\":{\"interest_rate\":91},\"6\":{\"interest_rate\":101},\"7\":{\"interest_rate\":115}}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_bitlender_rate("php-rate-feeder", "PHP", optional_def,type_object,{}, true);
    }
    {
      string s_cny = "{\"1\":{\"interest_rate\":74},\"2\":{\"interest_rate\":76},\"3\":{\"interest_rate\":81},\"4\":{\"interest_rate\":82},\"5\":{\"interest_rate\":91},\"6\":{\"interest_rate\":101},\"7\":{\"interest_rate\":115}}";
      variant type = fc::json::from_string(s_cny);
      variant_object type_object;
      fc::from_variant(type, type_object);
      con.wallet_api_ptr->update_bitlender_rate("usdt-rate-feeder", "USDT",optional_def, type_object,{}, true);
    }

     fc::usleep(fc::seconds(option.g_isleep));

     
     

    con.wallet_api_ptr->update_bitlender_option_author("nathan", "CNY", {"gateway-cny", "carrier-cny1", "carrier-cny2", "carrier-cny3", "carrier-cny4"}, 4, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);

    con.wallet_api_ptr->update_bitlender_option_author("nathan", "USD", {"gateway-usd", "carrier-usd1", "carrier-usd2", "carrier-usd3"}, 3, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);

    con.wallet_api_ptr->update_bitlender_option_author("nathan", "CAD", {"gateway-cad", "carrier-cad1", "carrier-cad2", "carrier-cad3"}, 3, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);

    con.wallet_api_ptr->update_bitlender_option_author("nathan", "PHP", {"gateway-php", "carrier-php1", "carrier-php2", "carrier-php3"}, 3, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);

    con.wallet_api_ptr->update_bitlender_option_author("nathan", "USDT", {"gateway-usdt", "carrier-usdt1", "carrier-usdt2", "carrier-usdt3"}, 3, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
}

void create_block_change_password(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
 {
      FC_MESSAGE("change_password");
      fc::usleep(fc::seconds(option.g_isleep));

      //理事会，见证人，预算委员会
     con.wallet_api_ptr->change_password("nathan", "zos2018-nathan", true);
      con.wallet_api_ptr->change_password("messager", "zos2018-messager", true);
      con.wallet_api_ptr->change_password("init0", "zos2018-init0", true);
      con.wallet_api_ptr->change_password("init1", "zos2018-init1", true);
      con.wallet_api_ptr->change_password("init2", "zos2018-init2", true);
      con.wallet_api_ptr->change_password("init3", "zos2018-init3", true);
      con.wallet_api_ptr->change_password("init4", "zos2018-init4", true);
      con.wallet_api_ptr->change_password("init5", "zos2018-init5", true);
      con.wallet_api_ptr->change_password("init6", "zos2018-init6", true);
      con.wallet_api_ptr->change_password("init7", "zos2018-init7", true);
      con.wallet_api_ptr->change_password("init8", "zos2018-init8", true);
      con.wallet_api_ptr->change_password("init9", "zos2018-init9", true);
      con.wallet_api_ptr->change_password("init10", "zos2018-init10", true);
      con.wallet_api_ptr->change_password("platform-account", "zos2018-platform", true);

      fc::usleep(fc::seconds(option.g_isleep));

      //喂价人
      con.wallet_api_ptr->change_password("feeder-0", "zos2018-feed", true);
      con.wallet_api_ptr->change_password("feeder-1", "zos2018-feed", true);
      con.wallet_api_ptr->change_password("feeder-2", "zos2018-feed", true);
      con.wallet_api_ptr->change_password("feeder-3", "zos2018-feed", true);
      con.wallet_api_ptr->change_password("feeder-4", "zos2018-feed", true);
      con.wallet_api_ptr->change_password("feeder-5", "zos2018-feed", true);
      con.wallet_api_ptr->change_password("feeder-6", "zos2018-feed", true);
      con.wallet_api_ptr->change_password("feeder-7", "zos2018-feed", true);

      //水龙头
      con.wallet_api_ptr->change_password("onboarding.zos.foundation", "zos2018-faucet", true);
      
      con.wallet_api_ptr->change_password("admin-lending","zos2018-lending",true);   
      con.wallet_api_ptr->change_password("admin-notify","zos2018-notify",true);

      //网关
      con.wallet_api_ptr->change_password("gateway-cny", "gateway-pwd-cny", true);
      con.wallet_api_ptr->change_password("gateway-usd", "gateway-pwd-usd", true);
      con.wallet_api_ptr->change_password("gateway-cad", "gateway-pwd-cad", true);
      con.wallet_api_ptr->change_password("gateway-php", "gateway-pwd-php", true);
      con.wallet_api_ptr->change_password("gateway-usdt", "gateway-pwd-usdt", true);
      con.wallet_api_ptr->change_password("gateway-eth", "gateway-pwd-eth", true);
      con.wallet_api_ptr->change_password("gateway-btc", "gateway-pwd-btc", true);
      con.wallet_api_ptr->change_password("gateway-test", "gateway-pwd-test", true);

      con.wallet_api_ptr->change_password("author-cny", "author-pwd-cny", true);
      con.wallet_api_ptr->change_password("author-usd", "author-pwd-usd", true);
      con.wallet_api_ptr->change_password("author-cad", "author-pwd-cad", true);
      con.wallet_api_ptr->change_password("author-php", "author-pwd-php", true);
      con.wallet_api_ptr->change_password("author-usdt", "author-pwd-usdt", true);
      con.wallet_api_ptr->change_password("author-eth", "author-pwd-eth", true);
      con.wallet_api_ptr->change_password("author-btc", "author-pwd-btc", true);

      // con.wallet_api_ptr->set_auth_key("author-cny", "author-pwd-cny", true);
      // con.wallet_api_ptr->set_auth_key("author-usd", "author-pwd-usd", true);
      // con.wallet_api_ptr->set_auth_key("author-cad", "author-pwd-cad", true);
      // con.wallet_api_ptr->set_auth_key("author-php", "author-pwd-php", true);
      // con.wallet_api_ptr->set_auth_key("author-usdt", "author-pwd-usdt", true);
      // con.wallet_api_ptr->set_auth_key("author-eth", "author-pwd-eth", true);
      // con.wallet_api_ptr->set_auth_key("author-btc", "author-pwd-btc", true);   


      //网关提案用户
      con.wallet_api_ptr->change_password("gateway-cny-proposal", "gateway-cny-proposal", true);
      con.wallet_api_ptr->change_password("gateway-usd-proposal", "gateway-usd-proposal", true);      
      con.wallet_api_ptr->change_password("gateway-cad-proposal", "gateway-cad-proposal", true);
      con.wallet_api_ptr->change_password("gateway-php-proposal", "gateway-php-proposal", true);
      con.wallet_api_ptr->change_password("gateway-usdt-proposal", "gateway-usdt-proposal", true);
      con.wallet_api_ptr->change_password("gateway-eth-proposal", "gateway-eth-proposal", true);
      con.wallet_api_ptr->change_password("gateway-btc-proposal", "gateway-btc-proposal", true);
      //运营商
      con.wallet_api_ptr->change_password("carrier-cny1", "carrier-pwd-cny1", true);
      con.wallet_api_ptr->change_password("carrier-cny2", "carrier-pwd-cny2", true);
      con.wallet_api_ptr->change_password("carrier-cny3", "carrier-pwd-cny3", true);
      con.wallet_api_ptr->change_password("carrier-cny4", "carrier-pwd-cny4", true);
      con.wallet_api_ptr->change_password("carrier-usd1", "carrier-pwd-usd1", true);
      con.wallet_api_ptr->change_password("carrier-usd2", "carrier-pwd-usd2", true);
      con.wallet_api_ptr->change_password("carrier-usd3", "carrier-pwd-usd3", true);
      con.wallet_api_ptr->change_password("carrier-cad1", "carrier-pwd-cad1", true);
      con.wallet_api_ptr->change_password("carrier-cad2", "carrier-pwd-cad2", true);
      con.wallet_api_ptr->change_password("carrier-cad3", "carrier-pwd-cad3", true);
      con.wallet_api_ptr->change_password("carrier-php1", "carrier-pwd-php1", true);
      con.wallet_api_ptr->change_password("carrier-php2", "carrier-pwd-php2", true);
      con.wallet_api_ptr->change_password("carrier-php3", "carrier-pwd-php3", true);
      con.wallet_api_ptr->change_password("carrier-usdt1", "carrier-pwd-usdt1", true);
      con.wallet_api_ptr->change_password("carrier-usdt2", "carrier-pwd-usdt2", true);
      con.wallet_api_ptr->change_password("carrier-usdt3", "carrier-pwd-usdt3", true);

      //借贷利率修改人
      con.wallet_api_ptr->change_password("cny-rate-feeder", "cny-rate-feeder", true);
      con.wallet_api_ptr->change_password("usd-rate-feeder", "usd-rate-feeder", true);
      con.wallet_api_ptr->change_password("cad-rate-feeder", "cad-rate-feeder", true);
      con.wallet_api_ptr->change_password("php-rate-feeder", "php-rate-feeder", true);
      con.wallet_api_ptr->change_password("usdt-rate-feeder", "usdt-rate-feeder", true);

    
      //基金会组长
      con.wallet_api_ptr->change_password("committee-group", "committee-group", true);
      //运营团队组长
      con.wallet_api_ptr->change_password("business-group", "business-group", true);           
}    
void create_block_group(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)  
 {
      FC_MESSAGE("trans balances");
      con.wallet_api_ptr->create_account("committee-group", "group-feed", "nathan", "nathan", 20, true);
      con.wallet_api_ptr->create_account("business-group", "group-feed", "nathan", "nathan", 20, true);
      con.wallet_api_ptr->import_password("committee-group", "group-feed");
      con.wallet_api_ptr->import_password("business-group", "group-feed");
      //con.wallet_api_ptr->transfer("nathan", "committee-group", "500000", "ZOS", "", true);
      con.wallet_api_ptr->upgrade_account("committee-group", true);
      //con.wallet_api_ptr->transfer("nathan", "business-group", "500000", "ZOS", "", true);
      con.wallet_api_ptr->upgrade_account("business-group", true);

      vector<asset_summary> asend = con.wallet_api_ptr->list_account_balances("nathan");
      share_type remain = asend[0].amount / 100000;
      con.wallet_api_ptr->transfer("nathan", "business-group", fc::to_string(remain.value / 2), "ZOS", "", true);
      con.wallet_api_ptr->transfer("nathan", "committee-group", fc::to_string(remain.value / 2), "ZOS", "", true);
 }      
void create_block_vote(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval,string snathan) 
{
    FC_MESSAGE("vote");

     con.wallet_api_ptr->vote_for_witness(snathan, "init0", true, true);
     con.wallet_api_ptr->vote_for_witness(snathan, "init1", true, true);
     con.wallet_api_ptr->vote_for_witness(snathan, "init2", true, true);
     con.wallet_api_ptr->vote_for_witness(snathan, "init3", true, true);
     con.wallet_api_ptr->vote_for_witness(snathan, "init4", true, true);
     con.wallet_api_ptr->vote_for_witness(snathan, "init5", true, true);
     con.wallet_api_ptr->vote_for_witness(snathan, "init6", true, true);
     con.wallet_api_ptr->vote_for_witness(snathan, "init7", true, true);
     con.wallet_api_ptr->vote_for_witness(snathan, "init8", true, true);
     con.wallet_api_ptr->vote_for_witness(snathan, "init9", true, true);
     con.wallet_api_ptr->vote_for_witness(snathan, "init10", true, true);    

     con.wallet_api_ptr->vote_for_committee_member(snathan, "init0", true, true);
     con.wallet_api_ptr->vote_for_committee_member(snathan, "init1", true, true);
     con.wallet_api_ptr->vote_for_committee_member(snathan, "init2", true, true);
     con.wallet_api_ptr->vote_for_committee_member(snathan, "init3", true, true);
     con.wallet_api_ptr->vote_for_committee_member(snathan, "init4", true, true);
     con.wallet_api_ptr->vote_for_committee_member(snathan, "init5", true, true);
     con.wallet_api_ptr->vote_for_committee_member(snathan, "init6", true, true);
     con.wallet_api_ptr->vote_for_committee_member(snathan, "init7", true, true);
     con.wallet_api_ptr->vote_for_committee_member(snathan, "init8", true, true);
     con.wallet_api_ptr->vote_for_committee_member(snathan, "init9", true, true);
     con.wallet_api_ptr->vote_for_committee_member(snathan, "init10", true, true);

     con.wallet_api_ptr->vote_for_budget_member(snathan, "init0", true, true);
     con.wallet_api_ptr->vote_for_budget_member(snathan, "init1", true, true);
     con.wallet_api_ptr->vote_for_budget_member(snathan, "init2", true, true);
     con.wallet_api_ptr->vote_for_budget_member(snathan, "init3", true, true);
     con.wallet_api_ptr->vote_for_budget_member(snathan, "init4", true, true);
     con.wallet_api_ptr->vote_for_budget_member(snathan, "init5", true, true);
     con.wallet_api_ptr->vote_for_budget_member(snathan, "init6", true, true);
     con.wallet_api_ptr->vote_for_budget_member(snathan, "init7", true, true);
     con.wallet_api_ptr->vote_for_budget_member(snathan, "init8", true, true);
     con.wallet_api_ptr->vote_for_budget_member(snathan, "init9", true, true);
     con.wallet_api_ptr->vote_for_budget_member(snathan, "init10", true, true);
}
void create_block_register_gateway(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval) 
{
       fc::usleep(fc::seconds(option.g_isleep));  
     FC_MESSAGE("register gateway");

    con.wallet_api_ptr->create_account("gateway-cny","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("gateway-usd","gateway-pwd","nathan","nathan",20,true);    
    con.wallet_api_ptr->create_account("gateway-cad","gateway-pwd","nathan","nathan",20,true);    
    con.wallet_api_ptr->create_account("gateway-php","gateway-pwd","nathan","nathan",20,true);    
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
    con.wallet_api_ptr->create_account("carrier-cad1","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-cad2","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-cad3","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-php1","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-php2","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-php3","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-usdt1","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-usdt2","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("carrier-usdt3","gateway-pwd","nathan","nathan",20,true);  
    
    con.wallet_api_ptr->create_account("author-cny","author-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("author-usd","author-pwd","nathan","nathan",20,true);    
    con.wallet_api_ptr->create_account("author-cad","author-pwd","nathan","nathan",20,true);    
    con.wallet_api_ptr->create_account("author-php","author-pwd","nathan","nathan",20,true);    
    con.wallet_api_ptr->create_account("author-usdt","author-pwd","nathan","nathan",20,true);  
    con.wallet_api_ptr->create_account("author-eth","author-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("author-btc","author-pwd","nathan","nathan",20,true);



    con.wallet_api_ptr->create_account("gateway-test","gateway-pwd","nathan","nathan",20,true);

   

    con.wallet_api_ptr->import_password("gateway-cny","gateway-pwd");
    con.wallet_api_ptr->import_password("gateway-usd","gateway-pwd");    
    con.wallet_api_ptr->import_password("gateway-cad","gateway-pwd");
    con.wallet_api_ptr->import_password("gateway-php","gateway-pwd");
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
    con.wallet_api_ptr->import_password("carrier-cad1","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-cad2","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-cad3","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-php1","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-php2","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-php3","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-usdt1","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-usdt2","gateway-pwd");
    con.wallet_api_ptr->import_password("carrier-usdt3","gateway-pwd");
    con.wallet_api_ptr->import_password("gateway-test","gateway-pwd");

    con.wallet_api_ptr->import_password("author-cny","author-pwd");
    con.wallet_api_ptr->import_password("author-usd","author-pwd");    
    con.wallet_api_ptr->import_password("author-cad","author-pwd");
    con.wallet_api_ptr->import_password("author-php","author-pwd");
    con.wallet_api_ptr->import_password("author-usdt","author-pwd");   
    con.wallet_api_ptr->import_password("author-eth","author-pwd");
    con.wallet_api_ptr->import_password("author-btc","author-pwd");

    
     /*con.wallet_api_ptr->transfer("nathan", "gateway-cny",  "51100", "ZOS", "", true);
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
     con.wallet_api_ptr->transfer("nathan", "carrier-usdt3", "51100", "ZOS", "", true);     */

     con.wallet_api_ptr->upgrade_account("gateway-cny", true);
     con.wallet_api_ptr->upgrade_account("gateway-usd", true);     
     con.wallet_api_ptr->upgrade_account("gateway-cad", true);     
     con.wallet_api_ptr->upgrade_account("gateway-php", true);     
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
     con.wallet_api_ptr->upgrade_account("carrier-cad1", true);
     con.wallet_api_ptr->upgrade_account("carrier-cad2", true);      
     con.wallet_api_ptr->upgrade_account("carrier-cad3", true); 
     con.wallet_api_ptr->upgrade_account("carrier-php1", true);
     con.wallet_api_ptr->upgrade_account("carrier-php2", true);      
     con.wallet_api_ptr->upgrade_account("carrier-php3", true);   
     con.wallet_api_ptr->upgrade_account("carrier-usdt1", true);
     con.wallet_api_ptr->upgrade_account("carrier-usdt2", true);      
     con.wallet_api_ptr->upgrade_account("carrier-usdt3", true);     
     con.wallet_api_ptr->upgrade_account("gateway-test", true);

     con.wallet_api_ptr->upgrade_account("author-cny", true);
     con.wallet_api_ptr->upgrade_account("author-usd", true);     
     con.wallet_api_ptr->upgrade_account("author-cad", true);     
     con.wallet_api_ptr->upgrade_account("author-php", true);     
     con.wallet_api_ptr->upgrade_account("author-usdt", true);     
     con.wallet_api_ptr->upgrade_account("author-btc", true);
     con.wallet_api_ptr->upgrade_account("author-eth", true);


    FC_MESSAGE("create gateway proposal");

    con.wallet_api_ptr->create_account("gateway-cny-proposal","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("gateway-usd-proposal","gateway-pwd","nathan","nathan",20,true);    
    con.wallet_api_ptr->create_account("gateway-cad-proposal","gateway-pwd","nathan","nathan",20,true);    
    con.wallet_api_ptr->create_account("gateway-php-proposal","gateway-pwd","nathan","nathan",20,true);    
    con.wallet_api_ptr->create_account("gateway-usdt-proposal","gateway-pwd","nathan","nathan",20,true);  
    con.wallet_api_ptr->create_account("gateway-eth-proposal","gateway-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("gateway-btc-proposal","gateway-pwd","nathan","nathan",20,true);

    con.wallet_api_ptr->import_password("gateway-cny-proposal","gateway-pwd");
    con.wallet_api_ptr->import_password("gateway-usd-proposal","gateway-pwd");    
    con.wallet_api_ptr->import_password("gateway-cad-proposal","gateway-pwd"); 
    con.wallet_api_ptr->import_password("gateway-php-proposal","gateway-pwd"); 
    con.wallet_api_ptr->import_password("gateway-usdt-proposal","gateway-pwd");   
    con.wallet_api_ptr->import_password("gateway-eth-proposal","gateway-pwd");
    con.wallet_api_ptr->import_password("gateway-btc-proposal","gateway-pwd");
    
   /* con.wallet_api_ptr->transfer("nathan", "gateway-cny-proposal",  "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "gateway-usd-proposal",  "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "gateway-usdt-proposal", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "gateway-btc-proposal",  "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "gateway-eth-proposal",  "100", "ZOS", "", true);*/

     fc::usleep(fc::seconds(option.g_isleep));
  
     FC_MESSAGE("create member");
   
     con.wallet_api_ptr->create_gateway("gateway-cny", option.g_gateway, "", {"CNY"}, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_gateway("gateway-usd", option.g_gateway, "", {"USD"},true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_gateway("gateway-cad", option.g_gateway, "", {"CAD"},true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_gateway("gateway-php", option.g_gateway, "", {"PHP"},true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_gateway("gateway-usdt", option.g_gateway, "", {"USDT"},true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_gateway("gateway-btc", option.g_gateway, "", {"BTC"},true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_gateway("gateway-eth", option.g_gateway, "", {"ETH"},true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_gateway("gateway-test", option.g_gateway, "", {"CNY"}, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);

     con.wallet_api_ptr->create_carrier("carrier-cny1", option.g_gateway, "", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-cny2", option.g_gateway, "", true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-cny3", option.g_gateway,  "",true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-cny4", option.g_gateway,  "",true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-usd1",option.g_gateway,  "",true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-usd2", option.g_gateway,  "",true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true); 
     con.wallet_api_ptr->create_carrier("carrier-usd3", option.g_gateway,  "",true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-cad1",option.g_gateway,  "",true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-cad2", option.g_gateway,  "",true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true); 
     con.wallet_api_ptr->create_carrier("carrier-cad3", option.g_gateway,  "",true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-php1",option.g_gateway,  "",true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-php2", option.g_gateway,  "",true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true); 
     con.wallet_api_ptr->create_carrier("carrier-php3", option.g_gateway,  "",true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true); 
     con.wallet_api_ptr->create_carrier("carrier-usdt1", option.g_gateway,  "",true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_carrier("carrier-usdt2", option.g_gateway,  "",true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true); 
     con.wallet_api_ptr->create_carrier("carrier-usdt3", option.g_gateway, "",true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true); 

     con.wallet_api_ptr->create_author("author-cny", option.g_gateway, "", {"CNY"}, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_author("author-usd", option.g_gateway, "", {"USD"},true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_author("author-cad", option.g_gateway, "", {"CAD"},true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_author("author-php", option.g_gateway, "", {"PHP"},true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_author("author-usdt", option.g_gateway, "", {"USDT"},true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_author("author-btc", option.g_gateway, "", {"BTC"},true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->create_author("author-eth", option.g_gateway, "", {"ETH"},true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
      

     con.wallet_api_ptr->set_carrier_need_auth("carrier-cny1",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-cny",true);
     con.wallet_api_ptr->set_carrier_need_auth("carrier-cny2",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-cny",true);
     con.wallet_api_ptr->set_carrier_need_auth("carrier-cny3",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-cny",true);
     con.wallet_api_ptr->set_carrier_need_auth("carrier-cny4",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-cny",true);

     con.wallet_api_ptr->set_carrier_need_auth("carrier-usd1",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-usd",true);
     con.wallet_api_ptr->set_carrier_need_auth("carrier-usd2",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-usd",true);
     con.wallet_api_ptr->set_carrier_need_auth("carrier-usd3",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-usd",true);
    

     con.wallet_api_ptr->set_carrier_need_auth("carrier-cad1",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-cad",true);
     con.wallet_api_ptr->set_carrier_need_auth("carrier-cad2",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-cad",true);
     con.wallet_api_ptr->set_carrier_need_auth("carrier-cad3",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-cad",true);
     

     con.wallet_api_ptr->set_carrier_need_auth("carrier-php1",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-php",true);
     con.wallet_api_ptr->set_carrier_need_auth("carrier-php2",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-php",true);
     con.wallet_api_ptr->set_carrier_need_auth("carrier-php3",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-php",true);
     

     con.wallet_api_ptr->set_carrier_need_auth("carrier-usdt1",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-usdt",true);
     con.wallet_api_ptr->set_carrier_need_auth("carrier-usdt2",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-usdt",true);
     con.wallet_api_ptr->set_carrier_need_auth("carrier-usdt3",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-usdt",true);

     con.wallet_api_ptr->set_gateway_need_auth("gateway-cny",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-cny",true);
     con.wallet_api_ptr->set_gateway_need_auth("gateway-usd",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-usd",true);
     con.wallet_api_ptr->set_gateway_need_auth("gateway-usdt",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-usdt",true);
     con.wallet_api_ptr->set_gateway_need_auth("gateway-cad",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-cad",true);
     con.wallet_api_ptr->set_gateway_need_auth("gateway-php",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-php",true);
     con.wallet_api_ptr->set_gateway_need_auth("gateway-eth",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-eth",true);
     con.wallet_api_ptr->set_gateway_need_auth("gateway-btc",optional<uint32_t>(),optional<vector<vector<string>>>(),"author-btc",true);
 
    
    
     
     fc::usleep(fc::seconds(option.g_isleep));

     FC_MESSAGE("gateway_issue_currency");   

}
void create_block_transfer_balance(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval) 
{
     FC_MESSAGE("transfer balance");

   /*  con.wallet_api_ptr->transfer("nathan", "committee-account", "1000", "ZOS", "", true);

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

     con.wallet_api_ptr->transfer("nathan", "platform-account", "100", "ZOS", "", true);  */
}  
void create_block_change_scale(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval) 
{
   FC_MESSAGE("change price");
  {
    //string s_cny = "{\"committee_proposal_review_period\":3600,\"maximum_proposal_lifetime\":10800,\"maintenance_interval\":300}";
    string s_cny = "{\"scale\":10000}";
    variant type = fc::json::from_string(s_cny);
    variant_object type_object;
    fc::from_variant(type, type_object);

    dynamic_global_property_object dy = con.wallet_api_ptr->get_dynamic_global_properties();
    global_property_object gl = con.wallet_api_ptr->get_global_properties();
    con.wallet_api_ptr->propose_fee_change("nathan", dy.time + gl.parameters.maximum_proposal_lifetime, type_object, "change fee", true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
  }      
}
void create_block_change_param(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval) 
{  

  fc::usleep(fc::seconds(option.g_isleep));
  FC_MESSAGE("change param");
  {
    //string s_cny = "{\"committee_proposal_review_period\":3600,\"maximum_proposal_lifetime\":10800,\"maintenance_interval\":300}";
    string s_cny = "{\"maintenance_interval\":300}";
    variant type = fc::json::from_string(s_cny);
    variant_object type_object;
    fc::from_variant(type, type_object);

    dynamic_global_property_object dy = con.wallet_api_ptr->get_dynamic_global_properties();
    global_property_object gl = con.wallet_api_ptr->get_global_properties();
    con.wallet_api_ptr->propose_parameter_change("nathan", dy.time + gl.parameters.maximum_proposal_lifetime, type_object, "change param", true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
  }  
 /* FC_MESSAGE("change price");
  {
    //string s_cny = "{\"committee_proposal_review_period\":3600,\"maximum_proposal_lifetime\":10800,\"maintenance_interval\":300}";
    string s_cny = "{\"scale\":0}";
    variant type = fc::json::from_string(s_cny);
    variant_object type_object;
    fc::from_variant(type, type_object);

    dynamic_global_property_object dy = con.wallet_api_ptr->get_dynamic_global_properties();
    global_property_object gl = con.wallet_api_ptr->get_global_properties();
    con.wallet_api_ptr->propose_fee_change("nathan", dy.time + gl.parameters.maximum_proposal_lifetime, type_object, "change fee", true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
  }*/      
}
     

void create_ext_asset(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
{
    //创建资产 

  
    asset_options common;
    bitasset_options bitasset_opts;

    auto btc_s = con.wallet_api_ptr->get_asset("ZOS");
    common.max_supply = GRAPHENE_MAX_SHARE_SUPPLY;
    common.market_fee_percent = 100;
    common.core_exchange_rate = price({asset(1, asset_id_type(10)), asset(1, GRAPHENE_CORE_ASSET)});
    bitasset_opts.short_backing_asset = btc_s.id;
    

    string asset_s = "CAD";
    std::cerr << "create asset " << asset_s << "\n";
    con.wallet_api_ptr->create_asset("nathan", "committee-account", asset_s, 2, common, bitasset_opts, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->propose_asset_bitlender_property("nathan", asset_s, ASSET_CASH | ASSET_LOAN, ASSET_CASH | ASSET_LOAN | ASSET_BIT, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);

 
    asset_s = "PHP";
    std::cerr << "create asset " << asset_s << "\n";
    con.wallet_api_ptr->create_asset("nathan", "committee-account", asset_s, 2, common, bitasset_opts, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);       
    con.wallet_api_ptr->propose_asset_bitlender_property("nathan", asset_s, ASSET_CASH | ASSET_LOAN, ASSET_CASH | ASSET_LOAN | ASSET_BIT, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
}   
/*
void create_block_author(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
{
    con.wallet_api_ptr->create_account("author-cny","author-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("author-usd","author-pwd","nathan","nathan",20,true);    
    con.wallet_api_ptr->create_account("author-cad","author-pwd","nathan","nathan",20,true);    
    con.wallet_api_ptr->create_account("author-php","author-pwd","nathan","nathan",20,true);    
    con.wallet_api_ptr->create_account("author-usdt","author-pwd","nathan","nathan",20,true);  
    con.wallet_api_ptr->create_account("author-eth","author-pwd","nathan","nathan",20,true);
    con.wallet_api_ptr->create_account("author-btc","author-pwd","nathan","nathan",20,true);

    con.wallet_api_ptr->import_password("author-cny","author-pwd");
    con.wallet_api_ptr->import_password("author-usd","author-pwd");    
    con.wallet_api_ptr->import_password("author-cad","author-pwd");
    con.wallet_api_ptr->import_password("author-php","author-pwd");
    con.wallet_api_ptr->import_password("author-usdt","author-pwd");   
    con.wallet_api_ptr->import_password("author-eth","author-pwd");
    con.wallet_api_ptr->import_password("author-btc","author-pwd");

   con.wallet_api_ptr->transfer("nathan", "author-cny", "6000", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "author-usd", "6000", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "author-cad", "6000", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "author-php", "6000", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "author-usdt", "6000", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "author-btc", "6000", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "author-eth", "6000", "ZOS", "", true);


    con.wallet_api_ptr->upgrade_account("author-cny", true);
    con.wallet_api_ptr->upgrade_account("author-usd", true);
    con.wallet_api_ptr->upgrade_account("author-cad", true);
    con.wallet_api_ptr->upgrade_account("author-php", true);
    con.wallet_api_ptr->upgrade_account("author-usdt", true);
    con.wallet_api_ptr->upgrade_account("author-btc", true);
    con.wallet_api_ptr->upgrade_account("author-eth", true);

   con.wallet_api_ptr->create_author("author-cny", option.g_gateway, "", {"CNY"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->create_author("author-usd", option.g_gateway, "", {"USD"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->create_author("author-cad", option.g_gateway, "", {"CAD"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->create_author("author-php", option.g_gateway, "", {"PHP"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->create_author("author-usdt", option.g_gateway, "", {"USDT"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->create_author("author-btc", option.g_gateway, "", {"BTC"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    con.wallet_api_ptr->create_author("author-eth", option.g_gateway, "", {"ETH"}, true);
    con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
    

    con.wallet_api_ptr->set_carrier_need_auth("carrier-cny1", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-cny", true);
    con.wallet_api_ptr->set_carrier_need_auth("carrier-cny2", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-cny", true);
    con.wallet_api_ptr->set_carrier_need_auth("carrier-cny3", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-cny", true);
    con.wallet_api_ptr->set_carrier_need_auth("carrier-cny4", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-cny", true);

    con.wallet_api_ptr->set_carrier_need_auth("carrier-usd1", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-usd", true);
    con.wallet_api_ptr->set_carrier_need_auth("carrier-usd2", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-usd", true);
    con.wallet_api_ptr->set_carrier_need_auth("carrier-usd3", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-usd", true);

    con.wallet_api_ptr->set_carrier_need_auth("carrier-cad1", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-cad", true);
    con.wallet_api_ptr->set_carrier_need_auth("carrier-cad2", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-cad", true);
    con.wallet_api_ptr->set_carrier_need_auth("carrier-cad3", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-cad", true);

    con.wallet_api_ptr->set_carrier_need_auth("carrier-php1", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-php", true);
    con.wallet_api_ptr->set_carrier_need_auth("carrier-php2", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-php", true);
    con.wallet_api_ptr->set_carrier_need_auth("carrier-php3", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-php", true);

    con.wallet_api_ptr->set_carrier_need_auth("carrier-usdt1", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-usdt", true);
    con.wallet_api_ptr->set_carrier_need_auth("carrier-usdt2", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-usdt", true);
    con.wallet_api_ptr->set_carrier_need_auth("carrier-usdt3", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-usdt", true);

    con.wallet_api_ptr->set_gateway_need_auth("gateway-cny", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-cny", true);
    con.wallet_api_ptr->set_gateway_need_auth("gateway-usd", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-usd", true);
    con.wallet_api_ptr->set_gateway_need_auth("gateway-usdt", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-usdt", true);
    con.wallet_api_ptr->set_gateway_need_auth("gateway-cad", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-cad", true);
    con.wallet_api_ptr->set_gateway_need_auth("gateway-php", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-php", true);
    con.wallet_api_ptr->set_gateway_need_auth("gateway-eth", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-eth", true);
    con.wallet_api_ptr->set_gateway_need_auth("gateway-btc", optional<uint32_t>(), optional<vector<vector<string>>>(), "author-btc", true);

    con.wallet_api_ptr->change_password("author-cny", "author-pwd-cny", true);
    con.wallet_api_ptr->change_password("author-usd", "author-pwd-usd", true);
    con.wallet_api_ptr->change_password("author-cad", "author-pwd-cad", true);
    con.wallet_api_ptr->change_password("author-php", "author-pwd-php", true);
    con.wallet_api_ptr->change_password("author-usdt", "author-pwd-usdt", true);
    con.wallet_api_ptr->change_password("author-eth", "author-pwd-eth", true);
    con.wallet_api_ptr->change_password("author-btc", "author-pwd-btc", true);
}
*/


void init_block(options_data &option)
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

    auto nathan_key = fc::ecc::private_key::regenerate(fc::sha256::hash(string("nathan")));
    auto public_key = nathan_key.get_public_key();
    string wif_key  = graphene::utilities::key_to_wif(nathan_key); 
     
    {
    FC_MESSAGE("import- key");

    FC_ASSERT(con.wallet_api_ptr->import_key("nathan",wif_key));
    FC_ASSERT(con.wallet_api_ptr->import_key("messager",wif_key));
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
    FC_ASSERT(con.wallet_api_ptr->import_key("platform-account",wif_key));
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
   // if (!nathan_acct_before_upgrade.is_lifetime_member())
    {
    //  FC_MESSAGE("Importing nathan's balance");
      std::vector<signed_transaction> import_txs = con.wallet_api_ptr->import_balance("nathan", {wif_key}, true);
      // upgrade nathan
    //  FC_MESSAGE("Upgrading Nathan to LTM");
   //   signed_transaction upgrade_tx = con.wallet_api_ptr->upgrade_account("nathan", true);
      account_object nathan_acct_after_upgrade = con.wallet_api_ptr->get_account("nathan");     
    }
     con.wallet_api_ptr->set_propose_register("nathan", true, true);
   }
  
   create_block_transfer_balance(con, option, approval);
   create_ext_asset(con, option, approval);
   //create_block_change_param(con, option, approval);  
   create_block_register_gateway(con, option, approval);
   create_block_feeders(con, option, approval);
   create_block_asset_feed(con, option, approval);
   create_block_admin(con, option, approval);   
   create_block_faucet(con, option, approval);   
   create_block_bitlender_option(con, option, approval);
   create_block_bitlender_feed(con, option, approval);
   create_block_group(con, option, approval);
   create_block_vote(con, option, approval,"committee-group");
   create_block_vote(con, option, approval,"business-group");
   create_block_change_scale(con, option, approval);  
 
   create_block_change_password(con, option, approval);

   con.close();
   fc::usleep(fc::seconds(1));
  }
  catch (fc::exception &e)
  {
    edump((e.to_detail_string()));
    throw;
  }  
}
 