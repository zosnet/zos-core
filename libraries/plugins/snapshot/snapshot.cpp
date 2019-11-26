/*
 * Copyright (c) 2017 Peter Conrad, and contributors.
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
#include <graphene/snapshot/snapshot.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/protocol/block.hpp>
#include <graphene/chain/protocol/transaction.hpp>

#include <fc/io/fstream.hpp>

using namespace graphene::snapshot_plugin;
using namespace graphene::chain;
using std::string;
using std::vector;

namespace bpo = boost::program_options;

static const char* OPT_BLOCK_START  = "snapshot-at-block";
static const char* OPT_BLOCK_PER    = "snapshot-per-block";
static const char* OPT_DEST         = "snapshot-to";

void snapshot_plugin::plugin_set_program_options(
   boost::program_options::options_description& command_line_options,
   boost::program_options::options_description& config_file_options)
{
   command_line_options.add_options()
         (OPT_BLOCK_START, bpo::value<uint64_t>(), "Block number after which to do a snapshot")
         (OPT_BLOCK_PER,   bpo::value<uint64_t>(), "Block per which to do a snapshot")
         (OPT_DEST, bpo::value<string>(), "Pathname of JSON file where to store the snapshot")
         ;
   config_file_options.add(command_line_options);
}

std::string snapshot_plugin::plugin_name()const
{
   return "snapshot";
}

std::string snapshot_plugin::plugin_description()const
{
   return "Create snapshots at a specified time or block number.";
}
void snapshot_plugin::get_info(fc::mutable_variant_object &result) 
{
    result[OPT_BLOCK_START] = snapshot_block;
    result[OPT_BLOCK_PER]   = snapshot_per;
    result[OPT_DEST]        = dest.generic_string().c_str();
}
void snapshot_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{ try {
   ilog("snapshot plugin: plugin_initialize() begin");

   if( options.count(OPT_BLOCK_START) || options.count(OPT_BLOCK_PER) )
   {
      FC_ASSERT( options.count(OPT_DEST), "Must specify snapshot-to in addition to snapshot-at-block or snapshot-at-time!" );
      dest = options[OPT_DEST].as<std::string>();
   
      if (options.count(OPT_BLOCK_START))
          snapshot_block = options[OPT_BLOCK_START].as<uint64_t>();
      if( options.count(OPT_BLOCK_PER) )
         snapshot_per   =  options[OPT_BLOCK_PER].as<uint64_t>();
      FC_ASSERT(snapshot_per >= 3, "Block per must big than  ${a}", ("a", snapshot_per));
      database().applied_block.connect( [&]( const graphene::chain::signed_block& b ) {
         check_snapshot( b );
      });
   }
   else
      FC_ASSERT( !options.count("snapshot-to"), "Must specify snapshot-at-block or snapshot-at-time in addition to snapshot-to!" );
   ilog("snapshot plugin: plugin_initialize() end");
} FC_LOG_AND_RETHROW() }

void snapshot_plugin::plugin_startup() {}

void snapshot_plugin::plugin_shutdown() {}

string   object_id_to_string(const graphene::chain::object_id_type &id)
{
    fc::variant result;
    fc::to_variant( id, result, GRAPHENE_NET_MAX_NESTED_OBJECTS );
    return result.as_string();
} 

static void create_snapshot(const uint64_t current_block, const graphene::chain::database& db, const fc::path& dest )
{  
   fc::path destpath = dest / ("snap-" +  fc::to_string(current_block));
   try
   {
     if (!fc::exists(destpath))
        fc::create_directories(destpath);
   }
   catch ( fc::exception e )
   {
      wlog( "Failed to open snapshot destination: ${ex}", ("ex",e) );
      return;
   }
   for( uint32_t space_id = 0; space_id < 256; space_id++ )
      for( uint32_t type_id = 0; type_id < 256; type_id++ )
      {
         if(type_id == graphene::chain::impl_block_summary_object_type)
             continue;
         try
         {
            db.get_index( (uint8_t)space_id, (uint8_t)type_id );       
         }
         catch(...)
         {
             continue;
         }
         try{
         auto &index = db.get_index((uint8_t)space_id, (uint8_t)type_id);
         index.inspect_all_objects([&destpath](const graphene::db::object &o) {
             fc::ofstream out;
             fc::path destfile = destpath / (object_id_to_string(o.id) + ".json");
             out.open(destfile);
             out << fc::json::to_string(o.to_variant()) << '\n';
             out.close();
         });
         }
         catch(fc::exception e)
         {
            wlog( "Failed to write snapshot destination: ${ex}", ("ex",e) );
            continue;
         }
      }
   
   ilog("snapshot plugin: created snapshot");
}
 
void snapshot_plugin::check_snapshot( const graphene::chain::signed_block& b )
{ try {
    uint64_t current_block = b.block_num();
    if(current_block >= snapshot_block && (current_block % snapshot_per == 0))           
       create_snapshot( current_block,database(), dest );

   
} FC_LOG_AND_RETHROW() }
