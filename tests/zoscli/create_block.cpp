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

 
   
void create_bitlender(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
{ 
 /*   uint64_t umember       = option.g_umember;   
    uint64_t ulender       = option.g_ulender;

    bitlender_key key;
    if (option.g_ulender > 0)
    {
    bitlender_option_object ocny = con.wallet_api_ptr->get_bitlender_option("CNY");
    object_id_type acny = *ocny.get_option(key).carriers.begin();
    string  cny_carrier = con.wallet_api_ptr->get_account(acny.to_string()).name;
    bitlender_option_object ousd = con.wallet_api_ptr->get_bitlender_option("USD");
    object_id_type ausd = *ousd.get_option(key).carriers.begin();
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

     try{
      con.wallet_api_ptr->gateway_deposit("gateway-btc", account_s, a_btc.amount_to_string(o_cny.lock_collateralize.amount.value+30), "BTC", true);
      con.wallet_api_ptr->gateway_deposit("gateway-eth", account_s, a_eth.amount_to_string(o_usd.lock_collateralize.amount.value+30), "ETH", true);
      if(con.wallet_api_ptr->get_account_balance(account_s,"ZOS").amount.value <2100000)
         con.wallet_api_ptr->transfer(option.nathan_account, account_s, "20", "ZOS", "", true);
      }catch(...)
      {
        break;
      }
      string pass_s = account_s + "_" + account_s;
      con.wallet_api_ptr->import_password(account_s, pass_s);
      int bfalse = 0;
      std::cerr << "create loan" << account_s << "\n";

      optional<variant_object> optional_def;

      //cny
      {
        variant type = fc::json::from_string("{\"order\":\"order00000\",\"amount_to_loan\":{\"amount\":100000,\"asset_id\":\"" + a_cny.id.to_string() + "\"},\"loan_period\":6,\"interest_rate\":120,\"repayment_type\":1,\"amount_to_collateralize\":{\"amount\":"+fc::to_string(o_cny.amount_to_collateralize.amount.value+2)+",\"asset_id\":\"" + a_btc.id.to_string() + "\"},\"collateral_rate\":"+fc::to_string(o_cny.collateral_rate)+",\"bid_period\":600000}");
        variant_object type_object;
        fc::from_variant(type, type_object);
        try{
        con.wallet_api_ptr->bitlender_lend_order(account_s, cny_carrier,optional_def, type_object, true);
        }catch(...)
        {
          bfalse++;
        }
      }
      //usd
      {
        variant type = fc::json::from_string("{\"order\":\"order00000\",\"amount_to_loan\":{\"amount\":100000,\"asset_id\":\"" + a_usd.id.to_string() + "\"},\"loan_period\":1,\"interest_rate\":120,\"repayment_type\":1,\"amount_to_collateralize\":{\"amount\":"+fc::to_string(o_usd.amount_to_collateralize.amount.value+2)+",\"asset_id\":\"" + a_eth.id.to_string() + "\"},\"collateral_rate\":"+fc::to_string(o_usd.collateral_rate)+",\"bid_period\":600000}");
        variant_object type_object;
        fc::from_variant(type, type_object);
        try{
        con.wallet_api_ptr->bitlender_lend_order(account_s, use_carrier,optional_def, type_object, true);
        }catch(...)
        {
          bfalse++;
        }
      }
      if(bfalse>=2)
        break;
      FC_SLEEP(option.uintdex);
     }
     uint64_t uloan = con.wallet_api_ptr->get_object_count(bitlender_order_id_type(0), true) - 1;

     string account_s = "test-" + fc::to_string(umember);
     string pass_s = account_s + "_" + account_s;  
     con.wallet_api_ptr->import_password(account_s, pass_s);

     for (uint64_t l = 1; l < uloan+100;l+=100)
     {
       FC_SLEEP(option.uintdex);
        vector<bitlender_order_info>  wo_s =  con.wallet_api_ptr-> get_loan_orders("all",0, l, 100);
        if(wo_s.size()<=0)
          continue;
        for(auto wo : wo_s)  
        {    
           if(wo.amount_to_loan.amount != 100000 || !(wo.is_loan() || wo.is_investing()) )
            continue;     

           string susd = "{\"order_id\":\"1.17."+ fc::to_string(wo.id.instance())+ "\",\"amount_to_invest\":{\"amount\":10000,\"asset_id\":\""+a_usd.id.to_string()+"\"},}";
           string scny = "{\"order_id\":\"1.17."+ fc::to_string(wo.id.instance())+ "\",\"amount_to_invest\":{\"amount\":10000,\"asset_id\":\""+a_cny.id.to_string()+"\"},}";          
          if(con.wallet_api_ptr->get_account_balance(account_s,"ZOS").amount.value <1100000)
              con.wallet_api_ptr->transfer(option.nathan_account, account_s, "10", "ZOS", "", true);
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
                 if(con.wallet_api_ptr->get_account_balance(account_s,"USD").amount.value <10200)
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
                 if(con.wallet_api_ptr->get_account_balance(account_s,"CNY").amount.value <10200)
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
    }*/
}

void create_worker(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
{
    //创建worker
    uint64_t uworker_base  = 100;
    int      maxvote       = 500;

    uint64_t umember       = option.g_umember;   
    uint64_t uworker       = option.g_uworker;

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
          con.wallet_api_ptr->transfer(option.nathan_account, account_s, "51000", "ZOS", "", true);
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
      if(con.wallet_api_ptr->get_account_balance(account_s,"ZOS").amount.value <4200000)
           con.wallet_api_ptr->transfer(option.nathan_account, account_s, "42", "ZOS", "", true);
      con.wallet_api_ptr->create_worker(account_s, cur_now, cur_now+ fc::seconds(24 * 3600 + 1), "0.00001", account_s + "exchange", "worker_url_" + account_s,"", type1, true);
      con.wallet_api_ptr->create_worker(account_s, cur_now, cur_now + fc::seconds(24 * 3600 + 1), "0.00001", account_s + "vesting", "worker_url_" + account_s,"", type2, true);
      FC_SLEEP(option.uintdex);
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
          con.wallet_api_ptr->transfer(option.nathan_account, account_s, "1", "ZOS", "", true);
          con.wallet_api_ptr->vote_for_worker(account_s, votedet, true);
        }
        catch (...)
        {
          continue;
        }
        FC_SLEEP(option.uintdex);
      }
    }
}

void create_member(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
{
 //创建管理用户
    uint64_t umember       = option.g_umember; 
    int      uvote         = 4;
    int      maxvote       = 500;
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
          con.wallet_api_ptr->transfer(option.nathan_account, account_s, "52000", "ZOS", "", true);
          con.wallet_api_ptr->upgrade_account(account_s, true);
        }
        if (!nathan_obj.is_carrier_member())
        {
          con.wallet_api_ptr->transfer(option.nathan_account, account_s, "1011", "ZOS", "", true);
          con.wallet_api_ptr->create_carrier(account_s, "committee_url_" + account_s,"", true);
          con.wallet_api_ptr->approve_proposal(option.nathan_account, "1.10.0", approval, true);
        }
        if (!nathan_obj.is_gateway_member())
        {
          con.wallet_api_ptr->transfer(option.nathan_account, account_s, "1011", "ZOS", "", true);
          con.wallet_api_ptr->create_gateway(account_s, "committee_url_" + account_s,"",{}, true);
          con.wallet_api_ptr->approve_proposal(option.nathan_account, "1.10.0", approval, true);
        }
        if (!nathan_obj.is_committee_member())
        {
          con.wallet_api_ptr->create_committee_member(account_s, "committee_url_" + account_s, "",true);
        }
        if (!nathan_obj.is_witness())
        {
          con.wallet_api_ptr->create_witness(account_s, "witness_url_" + account_s, "",true);
        }
        if (!nathan_obj.is_budget_member())
        {
          con.wallet_api_ptr->create_budget_member(account_s, "budget_url_" + account_s,"", true);
        }       
      }
      catch (...)
      {
        continue;
      }
      FC_SLEEP(option.uintdex);
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

      con.wallet_api_ptr->transfer(option.nathan_account, account_s, "1100", "ZOS", "", true);
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
        FC_SLEEP(option.uintdex);
      }
    }
}
void create_accounts(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
{
 //创建用户
    uint64_t uaccount_base = 100;
    uint64_t uaccount = option.g_uaccount;
    std::cerr << "create account" << uaccount << "\n";
    int64_t uindex = con.wallet_api_ptr->get_object_count(account_id_type(0), true) - uaccount_base;
    if (uindex < 1)
      uindex = 1;
    for (uint64_t l = uindex; l < uaccount; l++)
    {
      try
      {
        string account_s = "test-" + fc::to_string(l);
        string pass_s = account_s + "_" + account_s;
        std::cerr << "create account " << account_s << "\n";        
        signed_transaction create_acct_tx = con.wallet_api_ptr->create_account(account_s, pass_s, option.nathan_account, option.nathan_account, 20, true);
        con.wallet_api_ptr->import_password(account_s, pass_s);
      }
      catch (...)
      {
        
      }

      FC_SLEEP(option.uintdex);
     }
}
void create_base_account(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
{
//创建用户     
    uint64_t umember       = option.g_umember;
    int      uvote         = 4;
    std::cerr << "create base account"  << umember+uvote << "\n";   
    for (uint64_t l = 1; l < umember+uvote+1; l++)
    {
      string account_s = "test-" + fc::to_string(l);
      string pass_s = account_s + "_" + account_s;
      std::cerr << "create account " << account_s  << "\n";
      try
      {
        con.wallet_api_ptr->create_account(account_s, pass_s, option.nathan_account, option.nathan_account, 20, true);
      }
      catch(...)
      {
      }
      con.wallet_api_ptr->import_password(account_s, pass_s);     
    }
}
void create_asset(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
{
    //创建资产
    uint64_t uasset_base   = 100; 
    uint64_t uasset        = option.g_uasset;

    std::cerr << "create asset"  << uasset << "\n";
    asset_options common;
    bitasset_options bitasset_opts;

    auto btc_s = con.wallet_api_ptr->get_asset("ZOS");
    common.max_supply = GRAPHENE_MAX_SHARE_SUPPLY;
    common.market_fee_percent = 100;
    common.core_exchange_rate = price({asset(1, asset_id_type(10)), asset(1, GRAPHENE_CORE_ASSET)});
    bitasset_opts.short_backing_asset = btc_s.id;
   

    uint64_t uindex = con.wallet_api_ptr->get_object_count(GRAPHENE_CORE_ASSET, true) -uasset_base+1;
    if(uindex<1)
      uindex = 1;
    for (uint64_t l = uindex; l < uasset; l++)
    {
      string asset_s = "A" + fc::to_string(l) + "T";  
      try{
        auto a = con.wallet_api_ptr->get_asset(asset_s);
        continue;
      }
      catch(...)
      {
      } 
      std::cerr << "create asset " << asset_s << "\n";
      con.wallet_api_ptr->create_asset(option.nathan_account, "committee-account", asset_s, 8, common, bitasset_opts, true);    
      con.wallet_api_ptr->approve_proposal(option.nathan_account, "1.10.0", approval, true);       
    }    
} 
void create_asset_test(client_connection &con,options_data &option,graphene::wallet::approval_delta &approval)
{
   std::cerr << "create_asset_test" << "\n";
   vector<string> ass;
   vector<asset_object>    a =   con.wallet_api_ptr->list_assets("A",50);
   for(auto &as : a)
      ass.push_back(as.symbol);
   vector<asset_summary> asend = con.wallet_api_ptr->list_account_balances("gateway-test");
   if(asend.size()<=0 || asend[0].amount < 1100000)
      con.wallet_api_ptr->transfer(option.nathan_account,"gateway-test", "11","ZOS","",true);   
   con.wallet_api_ptr->update_gateway("gateway-test", "",optional<string>(),ass, true);    
   con.wallet_api_ptr->approve_proposal(option.nathan_account, "1.10.0", approval, true);
}
 

void create_block(options_data &option)
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
 
    
    create_base_account(con,option,approval);    
    create_asset(con,option,approval);    
    create_asset_test(con,option,approval);  
    create_member(con,option,approval);    
    create_worker(con,option,approval);
    create_bitlender(con,option,approval);
    create_accounts(con,option,approval);
   
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
 