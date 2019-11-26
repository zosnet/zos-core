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
 
#include <graphene/app/api.hpp>
#include <graphene/app/api_access.hpp>
#include <graphene/app/application.hpp>
 
#include <graphene/chain/protocol/types.hpp>

#include <graphene/egenesis/egenesis.hpp>

 
#include <graphene/net/exceptions.hpp>
 

 

#include <fc/io/fstream.hpp> 
#include <fc/crypto/base64.hpp>

 

#include <iostream>

 

namespace graphene { namespace app {
 
 
 #define GRAPHENE_MAX_CASH_SUPPLY        int64_t(1000000000000000ll)
                                      

namespace detail {

   void  create_genesis_cash(genesis_state_type  &initial_state) 
   {
         genesis_state_type::initial_asset_type asset_init;

         asset_init.symbol           = "CNY";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos CNY";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

       
         asset_init.symbol           = "USD";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos USD";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "PHP";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos PHP";
         asset_init.precision        = 2;
         asset_init.max_supply       = 1000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "CAD";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos CAD";
         asset_init.precision        = 2;
         asset_init.max_supply       = 100000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

       /*  asset_init.symbol           = "EUR";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos EUR";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "KRW";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos KRW";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "JPY";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos JPY";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "RUB";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos RUB";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "SGD";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos SGD";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;
         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         

         asset_init.symbol           = "VND";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos VND";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "IDR";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos IDR";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "INR";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos INR";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "PKR";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos PKR";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "TRY";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos TRY";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "AUD";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos AUD";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );        

         asset_init.symbol           = "GBP";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos GBP";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "NZD";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos NZD";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;
         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "MXN";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos MXN";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "THB";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos THB";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

           asset_init.symbol           = "HKD";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos HKD";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "MOP";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos MOP";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "TWD";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos TWD";
         asset_init.precision        = 2;
         asset_init.max_supply       = GRAPHENE_MAX_CASH_SUPPLY;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property = ASSET_CASH | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );*/


        
   }
  void  create_genesis_bits(genesis_state_type  &initial_state) 
   {
         genesis_state_type::initial_asset_type asset_init;

         asset_init.symbol           = "BTC";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos BTC";
         asset_init.precision        = 8;
         asset_init.max_supply       = 2000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0x80;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "ETH";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos ETH";
         asset_init.precision        = 8;
         asset_init.max_supply       = 10000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0x80;   
         initial_state.initial_assets.push_back( asset_init );


         asset_init.symbol           = "USDT";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos USDT";
         asset_init.precision        = 8;
         asset_init.max_supply       = 300000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LOAN;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );


       /*  asset_init.symbol           = "XRP";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos XRP";
         asset_init.precision        = 8;
         asset_init.max_supply       = 1000000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "BCH";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos BCH";
         asset_init.precision        = 8;
         asset_init.max_supply       = 2000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "EOS";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos EOS";
         asset_init.precision        = 8;
         asset_init.max_supply       = 100000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );


         asset_init.symbol           = "LTC";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos LTC";
         asset_init.precision        = 8;
         asset_init.max_supply       = 8400000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "XLM";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos XLM";
         asset_init.precision        = 8;
         asset_init.max_supply       = 1000000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "ADA";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos ADA";
         asset_init.precision        = 8;
         asset_init.max_supply       = 4500000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;
         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "MIOTA";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos MIOTA";
         asset_init.precision        = 8;
         asset_init.max_supply       = 300000000000000000;                                       
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "TRX";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos TRX";
         asset_init.precision        = 8;
         asset_init.max_supply       = 1000000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "NEO";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos NEO";
         asset_init.precision        = 8;
         asset_init.max_supply       = 10000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "DASH";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos DASH";
         asset_init.precision        = 8;
         asset_init.max_supply       = 2000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "XMR";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos XMR";
         asset_init.precision        = 8;
         asset_init.max_supply       = 2000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "BNB";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos BNB";
         asset_init.precision        = 8;
         asset_init.max_supply       = 20000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;
         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "XEM";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos XEM";
         asset_init.precision        = 8;
         asset_init.max_supply       = 1000000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "VEN";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos VEN";
         asset_init.precision        = 8;
         asset_init.max_supply       = 100000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "ETC";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos ETC";
         asset_init.precision        = 8;
         asset_init.max_supply       = 10000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;
         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "OKB";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos OKB";
         asset_init.precision        = 8;
         asset_init.max_supply       = 100000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );

         asset_init.symbol           = "HT";
         asset_init.issuer_name      = "committee-account";
         asset_init.description      = "zos HT";
         asset_init.precision        = 8;
         asset_init.max_supply       = 50000000000000000;
         asset_init.accumulated_fees = 0;
         asset_init.is_bitasset      = true;         
         asset_init.uasset_property  = ASSET_BIT | ASSET_LENDER;
         asset_init.flags            = 0;   
         initial_state.initial_assets.push_back( asset_init );*/
              
   }
    void  create_genesis_assets(genesis_state_type  &initial_state) 
   {
         create_genesis_cash(initial_state);
         create_genesis_bits(initial_state);
   }

   // namespace detail
} } }
