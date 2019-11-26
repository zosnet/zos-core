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

#pragma once
#include <graphene/chain/protocol/operations.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <graphene/db/generic_index.hpp>

/**
 * @defgroup prediction_market Prediction Market
 *
 * A prediction market is a specialized BitAsset such that total debt and total collateral are always equal amounts
 * (although bitlender_option IDs differ). No margin calls or force settlements may be performed on a prediction market bitlender_option. A
 * prediction market is globally settled by the issuer after the event being predicted resolves, thus a prediction
 * market must always have the @ref global_settle permission enabled. The maximum price for global settlement or short
 * sale of a prediction market bitlender_option is 1-to-1.
 */

namespace graphene { namespace chain {
   class account_object;
   class database;

   using namespace graphene::db;
   using std::string;
   using std::vector;

   
   /**
    *  @brief tracks the parameters of an bitlender_option
    *  @ingroup object
    *
    *  All bitlender_options have a globally unique symbol name that controls how they are traded and an issuer who
    *  has authority over the parameters of the bitlender_option.
    */



   class bitlender_paramers_object :  public abstract_object<bitlender_paramers_object>
   {
      public:
         static const uint8_t space_id = implementation_ids;
         static const uint8_t type_id  = impl_bitlender_paramers_object_type;      

         flat_map<bitlender_index,bitlender_paramers> options;//基础参数       
         flat_map<bitlender_index,bitlender_paramers> pending_options;         

         bool is_pending() const;
         void apply_pending();
         void set_type();
         const bitlender_paramers &get_option(const bitlender_key &type,bool bcheck = true) const;
         void  set_option(const bitlender_key &type, const bitlender_paramers &set);  

   };

   typedef multi_index_container<
      bitlender_paramers_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >       
      >
   > bitlender_paramers_object_multi_index_type;
   typedef generic_index<bitlender_paramers_object, bitlender_paramers_object_multi_index_type> bitlender_paramers_index;

   class bitlender_paramers_object_key :  public abstract_object<bitlender_paramers_object_key>
   {
      public:
         static const uint8_t space_id = implementation_ids;
         static const uint8_t type_id  = impl_bitlender_paramers_object_type;      

         bitlender_paramers options;//基础参数       
         optional<bitlender_paramers> pending_options;

         void set_data(const bitlender_paramers_object &obj,const optional<bitlender_key> &key);
      };



   struct bitlender_option_item
   {
      bitlender_option                   options;
      flat_set<account_id_type>          loan_carriers;
      flat_set<account_id_type>          invest_carriers;
      flat_map<uint32_t, bitlender_rate> interest_rate;
   };  
   class bitlender_option_object : public abstract_object<bitlender_option_object>
   {
      public:
         static const uint8_t space_id = protocol_ids;
         static const uint8_t type_id  = bitlender_option_object_type;

         asset_id_type                                 asset_id; //资产         
         string                                        sproduct;
         account_id_type                               issuer;  //创建人
         account_id_type                               author;  //小董事长

         flat_map<bitlender_index,bitlender_option_item> options; //基础参数         
         fee_mode_type                                   fee_mode;      //费用模式 
         bitlender_feed_option                           feed_option;   //喂价参数
         flat_set<account_id_type>                       gateways;      //网关            
          
         bool                                            stop = 0;   //是否停运 1 停运  0 营业
        
         void validate1(bitlender_paramers &param) const;
         bool is_stop() const { return stop  == 1; }

         const bitlender_option &get_option(const bitlender_key &key,bool bcheck = true) const ;
         const bitlender_option_item &get_optionItem(const bitlender_key &key,bool bcheck = true) const ;
         void  set_option(const bitlender_key &key,const bitlender_option & set);
         const flat_map<uint32_t,bitlender_rate>  &get_rate(const bitlender_key &key, const bool check = true) const;     
         void set_rate(const bitlender_key &key, const flat_map<uint32_t,bitlender_rate> &interest_rate_add,const vector<uint32_t>&interest_rate_remove);    
   };

   struct by_asset;
   struct by_produce;
   typedef multi_index_container<
      bitlender_option_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,        
         ordered_unique<
         tag<by_produce>,
         composite_key<
            bitlender_option_object,
            member<bitlender_option_object, asset_id_type, &bitlender_option_object::asset_id>,            
            member<bitlender_option_object, string, &bitlender_option_object::sproduct> > >,
         ordered_non_unique< tag<by_asset>, member<bitlender_option_object, asset_id_type, &bitlender_option_object::asset_id > >       
      >
   > bitlender_option_object_multi_index_type;
   typedef generic_index<bitlender_option_object, bitlender_option_object_multi_index_type> bitlender_option_index;

  
   class bitlender_option_object_key :  public abstract_object<bitlender_option_object_key>
   {
      public:
         static const uint8_t space_id = protocol_ids;
         static const uint8_t type_id  = bitlender_option_object_type;

         asset_id_type    asset_id; //资产         
         string           sproduct;
         account_id_type  issuer;  //创建人
         account_id_type  author;  //小董事长

         bitlender_option_1903             options;//基础参数
         fee_mode_type                     fee_mode;      //费用模式 
         bitlender_feed_option             feed_option;//喂价参数
         flat_set<account_id_type>         gateways;            //网关 
         flat_map<uint32_t,bitlender_rate> interest_rate;//利息表
         bool                              stop = 0;   //是否停运 1 停运  0 营业

         void set_data(const bitlender_option_object &obj,const optional<bitlender_key> &key);
      };


}   } // graphene::chain

FC_REFLECT( graphene::chain::bitlender_option_item,
            (options)
            (loan_carriers)
            (invest_carriers)
            (interest_rate)
           )  

FC_REFLECT_DERIVED( graphene::chain::bitlender_option_object_key, (graphene::db::object),
            (asset_id)          
            (sproduct)
            (issuer)
            (author)
            (options)       
            (fee_mode)
            (feed_option)
            (gateways) 
            (interest_rate)    
            (stop)
          )  

FC_REFLECT_DERIVED( graphene::chain::bitlender_option_object, (graphene::db::object),
            (asset_id)          
            (sproduct)
            (issuer)
            (author)
            (options)                   
            (fee_mode)
            (feed_option)
            (gateways)             
            (stop)            
          )  

FC_REFLECT_DERIVED( graphene::chain::bitlender_paramers_object_key, (graphene::db::object),
            (options)     
            (pending_options)
          )            
FC_REFLECT_DERIVED( graphene::chain::bitlender_paramers_object, (graphene::db::object),
            (options)     
            (pending_options)
          )  