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

#include <graphene/chain/database.hpp>

#include <graphene/chain/operation_history_object.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>

#include <fc/io/fstream.hpp>

#include <fstream>
#include <functional>
#include <iostream>

namespace graphene { namespace chain {

void app_status::set_block_version(uint32_t version)
{
  if(version > GRAPHENE_BLOCK_VERSION)
    version_high++;
  else
    version_low++;
}         
bool app_status::check_block_version()
{
  if(version_high + version_low <1000)
    return true;
  else if(version_high > version_low)
    return false;
  else
    return true;
}
void app_status::add_exchange_check(const asset_id_type &a, const asset_id_type &b)
{
   if(_apply_block)
   {
     uint64_t ass = a.get_instance() * 0xffffffff + b.get_instance();
     asset_key add;
     add.base = a;
     add.quote = b;
     exchange_check[ass] = add;
   }
}
database::database()
{
   _business_db =   std::make_shared< business_db >(this); 

   initialize_indexes();
   initialize_evaluators();   
}

database::~database()
{
   clear_pending();

   _business_db = nullptr;
}
void database::resign( fc::path data_src_dir,fc::path data_dest_dir,flat_map<witness_id_type,fc::ecc::private_key> &key)
{
    block_database   src_block;
    src_block.open(data_src_dir / "database" / "block_num_to_block");


    block_database   dest_block;
    dest_block.open(data_dest_dir / "database" / "block_num_to_block");

    auto last_block = src_block.last();
    if(!last_block ) 
    {
      elog( "!no last block" );
      edump((last_block));
      return;
    }
    const auto last_block_num  =   last_block->block_num();
    uint64_t   start_block_num = 1;
    if (last_block_num <= 1)
      return;

    block_id_type  previous;
    for( uint64_t i = start_block_num; i <= last_block_num; ++i )
    {       
      fc::optional< signed_block > block = src_block.fetch_by_number(i);
      if( !block.valid() )
      {
         wlog( "Reindexing terminated due to gap:  Block ${i} does not exist!", ("i", i) );         
         continue;
      }
      if(i == start_block_num )
      {
        previous = block->id();
        dest_block.store(block->id(), *block);
        continue;
      }
      else
      {
        const auto key_ptr = key.find(block->witness);
        if(key_ptr == key.end())
        {
            wlog( "key  Block ${i} does not exist!", ("i", i) );
            return;
        }         
        block->previous = previous;
        block->sign(key_ptr->second);
        dest_block.store(block->id(),*block);
        previous = block->id();
      }
    }

    src_block.close();
    dest_block.flush();
    dest_block.close();
}
void database::reindex( fc::path data_dir,uint64_t _index )
{ try {
   auto last_block = _block_id_to_block.last();
   if( !last_block ) {
      elog( "!no last block" );
      edump((last_block));
      return;
   }
   if( last_block->block_num() <= head_block_num()) return;
   if( _index > last_block->block_num())   return;

   ilog( "reindexing blockchain" );
   auto start = fc::time_point::now();
   const auto last_block_num = _index >10 ? _index : last_block->block_num();
   uint32_t flush_point = last_block_num < 10000 ? 0 : last_block_num - 10000;
   uint32_t undo_point = last_block_num < 50 ? 0 : last_block_num - 50;

   ilog( "Replaying blocks, starting at ${next} end ${end}  ", ("next",head_block_num() + 1)("end",last_block_num) );
   if( head_block_num() >= undo_point )
   {
      if( head_block_num() > 0 )
         _fork_db.start_block( *fetch_block_by_number( head_block_num() ) );
   }
   else
      _undo_db.disable();
   for( uint32_t i = head_block_num() + 1; i <= last_block_num; ++i )
   {
      if( i % 10000 == 0 ) std::cerr << "   " << double(i*100)/last_block_num << "%   "<<i << " of " <<last_block_num<<"   \n";
      if( i == flush_point )
      {
         ilog( "Writing database to disk at block ${i}", ("i",i) );
         flush();
         ilog( "Done" );
      }
      fc::optional< signed_block > block = _block_id_to_block.fetch_by_number(i);
      if( !block.valid() )
      {
         wlog( "Reindexing terminated due to gap:  Block ${i} does not exist!", ("i", i) );
         uint32_t dropped_count = 0;
         while( true )
         {
            fc::optional< block_id_type > last_id = _block_id_to_block.last_id();
            // this can trigger if we attempt to e.g. read a file that has block #2 but no block #1
            if( !last_id.valid() )
               break;
            // we've caught up to the gap
            if( block_header::num_from_id( *last_id ) <= i )
               break;
            _block_id_to_block.remove( *last_id );
            dropped_count++;
         }
         wlog( "Dropped ${n} blocks from after the gap", ("n", dropped_count) );
         break;
      }
      if( i < undo_point )
         apply_block(*block, skip_witness_signature |
                             skip_transaction_signatures |
                             skip_transaction_dupe_check |
                             skip_tapos_check |
                             skip_witness_schedule_check |
                             skip_authority_check);
      else
      {
         _undo_db.enable();
         push_block(*block, skip_witness_signature |
                            skip_transaction_signatures |
                            skip_transaction_dupe_check |
                            skip_tapos_check |
                            skip_witness_schedule_check |
                            skip_authority_check);
      }
   }
   _undo_db.enable();
   auto end = fc::time_point::now();
   ilog( "Done reindexing, elapsed time: ${t} sec", ("t",double((end-start).count())/1000000.0 ) );
} FC_CAPTURE_AND_RETHROW( (data_dir) ) }

void database::wipe(const fc::path& data_dir, bool include_blocks)
{
   ilog("Wiping database", ("include_blocks", include_blocks));
   if (_opened) {
     close();
   }
   object_database::wipe(data_dir);
   if( include_blocks )
      fc::remove_all( data_dir / "database" );
}

void database::open(
   const fc::path& data_dir,
   std::function<genesis_state_type()> genesis_loader,
   const std::string& db_version,
   uint64_t re_index)
{
   try
   {
      bool wipe_object_db = false;
      if( !fc::exists( data_dir / "db_version" ) )
         wipe_object_db = true;
      else
      {
         std::string version_string;
         fc::read_file_contents( data_dir / "db_version", version_string );
         wipe_object_db = ( version_string != db_version );
         if( wipe_object_db ) {
           ilog("object_database version ${x} ${y}",("x",version_string)("y",db_version));
         }
      }
      if( wipe_object_db ) {
          ilog("Wiping object_database due to missing or wrong version");
          object_database::wipe( data_dir );
          std::ofstream version_file( (data_dir / "db_version").generic_string().c_str(),
                                      std::ios::out | std::ios::binary | std::ios::trunc );
          version_file.write( db_version.c_str(), db_version.size() );
          version_file.close();
      }

      object_database::open(data_dir);


      _block_id_to_block.open(data_dir / "database" / "block_num_to_block");

      if( !find(global_property_id_type()) )
         init_genesis(genesis_loader());

      fc::optional<block_id_type> last_block = _block_id_to_block.last_id();
      if( last_block.valid() )
      {
         FC_ASSERT( *last_block >= head_block_id(),
                    "last block ID does not match current chain state",
                    ("last_block->id", last_block)("head_block_id",head_block_num()) );
         reindex( data_dir,re_index );
      }
      _opened = true;
   }
   FC_CAPTURE_LOG_AND_RETHROW( (data_dir) )
}
void  database::getinfo(fc::mutable_variant_object &result)
{  
}
void database::close(bool rewind)
{
   // TODO:  Save pending tx's on close()
   clear_pending();

   // pop all of the blocks that we can given our undo history, this should
   // throw when there is no more undo history to pop
   if( rewind )
   {
      try
      {
         uint32_t cutoff = get_dynamic_global_properties().last_irreversible_block_num;

         ilog( "Rewinding from ${head} to ${cutoff}", ("head",head_block_num())("cutoff",cutoff) );
         while( head_block_num() > cutoff )
         {
            block_id_type popped_block_id = head_block_id();
            pop_block();
            _fork_db.remove(popped_block_id); // doesn't throw on missing
         }
      }
      catch ( const fc::exception& e )
      {
         wlog( "Database close unexpected exception: ${e}", ("e", e) );
      }
   }

   // Since pop_block() will move tx's in the popped blocks into pending,
   // we have to clear_pending() after we're done popping to get a clean
   // DB state (issue #336).
   clear_pending();

   if(object_database::is_open()) { 
     object_database::flush();
     object_database::close();
   }

   if( _block_id_to_block.is_open() )
      _block_id_to_block.close();

   _fork_db.reset();

   _opened = false;
}

} }
