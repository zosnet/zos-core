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
   
 
void create_node_asset_feed(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)

{
     fc::usleep(fc::seconds(option.g_isleep));

     FC_MESSAGE("set asset feeders");

    asset_object  a_cny  =  con.wallet_api_ptr->get_asset("CNY");
    asset_object  a_usd  =  con.wallet_api_ptr->get_asset("USD");
    asset_object  a_usdt =  con.wallet_api_ptr->get_asset("USDT");
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
void create_node_admin(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
{
   con.wallet_api_ptr->create_account("admin-lending", "zos2018-lending", "nathan", "nathan", 20, true);
   con.wallet_api_ptr->import_password("admin-lending","zos2018-lending");

   con.wallet_api_ptr->create_account("admin-notify", "zos2018-notify", "nathan", "nathan", 20, true);
   con.wallet_api_ptr->import_password("admin-notify","zos2018-notify");
}
void create_node_faucet(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
 {
      FC_MESSAGE("create faucet");
      con.wallet_api_ptr->create_account("onboarding.zos.foundation", "zos2018-faucet", "nathan", "nathan", 20, true);
      con.wallet_api_ptr->import_password("onboarding.zos.foundation", "zos2018-faucet");
      con.wallet_api_ptr->transfer("nathan", "onboarding.zos.foundation", "1000", "ZOS", "", true);
      con.wallet_api_ptr->upgrade_account("onboarding.zos.foundation", true);         
}
void create_node_feeders(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
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
   
   /* con.wallet_api_ptr->transfer("nathan", "feeder-0", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-1", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-2", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-3", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-4", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-5", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-6", "100", "ZOS", "", true);
    con.wallet_api_ptr->transfer("nathan", "feeder-7", "100", "ZOS", "", true);*/
}

void create_node_bitlender_feed(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
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

  


     con.wallet_api_ptr->propose_bitlender_feed_producers("nathan", "CNY", type_object, feeders, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->propose_bitlender_feed_producers("nathan", "USD", type_object, feeders, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     con.wallet_api_ptr->propose_bitlender_feed_producers("nathan", "USDT", type_object, feeders, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true); 
 

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
         con.wallet_api_ptr->publish_bitlender_feed(feeder, "CNY",feed, true);
       }    


       feed.settlement_price = price(asset(830, a_asset.id), asset(28769, a_eth.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(150, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder,"CNY", feed, true);
       }    

       feed.settlement_price = price(asset(830, a_asset.id), asset(28769, a_zos.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(150, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder, "CNY",feed, true);
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
         con.wallet_api_ptr->publish_bitlender_feed(feeder,"USD", feed, true);
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
         con.wallet_api_ptr->publish_bitlender_feed(feeder,"USD", feed, true);
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
         con.wallet_api_ptr->publish_bitlender_feed(feeder,"USDT", feed, true);
       }    


       feed.settlement_price = price(asset(87000000, a_asset.id), asset(28769, a_eth.id));
       feed.maintenance_collateral_ratio = 1750;       
       feed.maximum_short_squeeze_ratio = 1500;
       feed.core_exchange_rate = price(asset(110, a_asset.id), asset(56942, GRAPHENE_CORE_ASSET));

       for (int l = 0;l<=7;l++)
       {
         string feeder = "feeder-" + fc::to_string(l);
         feed.settlement_price.base.amount++;
         con.wallet_api_ptr->publish_bitlender_feed(feeder, "USDT",feed, true);
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
 
void create_node_bitlender_option(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
 {
   
   FC_MESSAGE("create_bitlender_option");

   optional<variant_object> optional_def;

   {

     string option = "";
     string s_cny = "{" + option + "}";
     variant type = fc::json::from_string(s_cny);
     variant_object type_object;
     fc::from_variant(type, type_object);
     con.wallet_api_ptr->create_bitlender_option("nathan", "CNY",optional_def, type_object, true);
     con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
      }
      {       
        string option = "";
        string s_cny = "{" + option + "}";
        variant type = fc::json::from_string(s_cny);
        variant_object type_object;
        fc::from_variant(type, type_object);
        con.wallet_api_ptr->create_bitlender_option("nathan", "USD", optional_def,type_object, true);
        con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);         
        con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);


      } 
      {
       string option = "\"min_invest_amount\":\"100000000000\",\"min_loan_amount\":\"100000000000\",\"min_invest_increase_range\":\"10000000000\",\"min_loan_increase_range\":\"10000000000\",\"max_interest_rate\":\"500000000000\",\"max_risk_margin\":\"500000000000\",\"max_carrier_service_charge\":\"500000000000\"";  
       string s_cny = "{"+option+"}";
       variant type = fc::json::from_string(s_cny);
       variant_object type_object;
       fc::from_variant(type, type_object);
       con.wallet_api_ptr->create_bitlender_option("nathan", "USDT",optional_def, type_object, true);
       con.wallet_api_ptr->approve_proposal("nathan", "1.10.0", approval, true);
     }

   

   
}

void create_node_change_password(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
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

     
      //基金会组长
     // con.wallet_api_ptr->change_password("committee-group", "committee-group", true);
      //运营团队组长
     // con.wallet_api_ptr->change_password("business-group", "business-group", true);           
}    
void create_node_group(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)  
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
void create_node_vote(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval,string snathan) 
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
void create_node_change_scale(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval) 
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
void create_node_transfer_balance(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval) 

{
     FC_MESSAGE("transfer balance");

     //con.wallet_api_ptr->transfer("nathan", "committee-account", "500", "ZOS", "", true);

     /*con.wallet_api_ptr->transfer("nathan", "init0",  "100", "ZOS", "", true);
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

     con.wallet_api_ptr->transfer("nathan", "platform-account", "100", "ZOS", "", true); 
     */
}   
     
void create_node(options_data &option)
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
    //if (!nathan_acct_before_upgrade.is_lifetime_member())
    {
      FC_MESSAGE("Importing nathan's balance");
      std::vector<signed_transaction> import_txs = con.wallet_api_ptr->import_balance("nathan", {wif_key}, true);
 
   //   signed_transaction upgrade_tx = con.wallet_api_ptr->upgrade_account("nathan", true);
    //  account_object nathan_acct_after_upgrade = con.wallet_api_ptr->get_account("nathan");     
    }
     con.wallet_api_ptr->set_propose_register("nathan", true, true);
   }

   create_node_transfer_balance(con, option, approval);  
   create_node_feeders(con, option, approval);
   create_node_asset_feed(con, option, approval);
   create_node_admin(con, option, approval);      
   create_node_faucet(con, option, approval);      
   //create_node_group(con, option, approval);
   create_node_vote(con, option, approval,"nathan");
   //create_node_vote(con, option, approval,"business-group");
   create_node_change_scale(con, option,approval);
   create_node_change_password(con, option, approval);

   con.close();
   fc::usleep(fc::seconds(1));
  }
  catch (fc::exception &e)
  {
    edump((e.to_detail_string()));
    throw;
  }  
}
 