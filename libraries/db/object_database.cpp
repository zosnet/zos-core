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
#include <graphene/db/object_database.hpp>

#include <fc/io/raw.hpp>
#include <fc/container/flat.hpp>
#include <fc/uint128.hpp>

namespace graphene { namespace db {

object_database::object_database()
:_undo_db(*this)
{
   _index.resize(255);
   _undo_db.enable();
   _is_open = false;
}

object_database::~object_database(){}

void object_database::close()
{
   _is_open = false;
}
const bool  object_database::is_open() const
{
   return _is_open;
}
const object* object_database::find_object( object_id_type id )const
{
   return get_index(id.space(),id.type()).find( id );
}
const bool object_database::is_object( object_id_type id )const
{
   uint8_t space_id = id.space();
   uint8_t type_id = id.type();   
   if(_index.size() <= space_id)
      return false;
   if(_index[space_id].size() <= type_id)
      return false;
   return _index[space_id][type_id] != nullptr;
}
const bool object_database::is_object_exist( object_id_type id ) const
{
   uint8_t space_id = id.space();
   uint8_t type_id = id.type();
   if(_index.size() <= space_id)
      return false;
   if(_index[space_id].size() <= type_id)
      return false;
   if( _index[space_id][type_id] == nullptr)
      return false;
   return _index[space_id][type_id]->find(id) != nullptr;
}
const object& object_database::get_object( object_id_type id )const
{
   return get_index(id.space(),id.type()).get( id );
}

const index& object_database::get_index(uint8_t space_id, uint8_t type_id)const
{
   FC_ASSERT( _index.size() > space_id, "${space_id} ${type_id} ${index_size}", ("space_id",space_id)("type_id",type_id)("index_size",_index.size()) );
   FC_ASSERT( _index[space_id].size() > type_id, "${space_id} ${type_id} ${index_size}", ("space_id",space_id)("type_id",type_id)("index_size",_index[space_id].size()) );
   const auto& tmp = _index[space_id][type_id];
   FC_ASSERT( tmp );
   return *tmp;
}
index& object_database::get_mutable_index(uint8_t space_id, uint8_t type_id)
{
   FC_ASSERT( _index.size() > space_id, "${space_id} ${type_id} ${index_size}", ("space_id",space_id)("type_id",type_id)("index_size",_index.size()) );
   FC_ASSERT( _index[space_id].size() > type_id , "${space_id} ${type_id} ${index_size}", ("space_id",space_id)("type_id",type_id)("index_size",_index[space_id].size()) );
   const auto& idx = _index[space_id][type_id];
   FC_ASSERT( idx, "${space} ${type} ", ("space",space_id)("type",type_id) );
   return *idx;
}

void object_database::flush()
{
   ilog("Save object_database in ${d}", ("d", _data_dir));
   fc::create_directories( _data_dir / "object_database.tmp" / "lock" );
   for( uint32_t space = 0; space < _index.size(); ++space )
   {
      fc::create_directories( _data_dir / "object_database.tmp" / fc::to_string(space) );
      const auto types = _index[space].size();
      for( uint32_t type = 0; type  <  types; ++type )
         if( _index[space][type] )
            _index[space][type]->save( _data_dir / "object_database.tmp" / fc::to_string(space)/fc::to_string(type) );
   }
   fc::remove_all( _data_dir / "object_database.tmp" / "lock" );
   if( fc::exists( _data_dir / "object_database" ) )
      fc::rename( _data_dir / "object_database", _data_dir / "object_database.old" );
   fc::rename( _data_dir / "object_database.tmp", _data_dir / "object_database" );
   fc::remove_all( _data_dir / "object_database.old" );
}

void object_database::wipe(const fc::path& data_dir)
{
   close();  
   ilog("Wiping object database...");
   fc::remove_all(data_dir / "object_database");
   fc::remove_all(data_dir / "loan_database");
   fc::remove_all(data_dir / "locktoken_database");
   fc::remove_all(data_dir / "finance_database");
   fc::remove_all(data_dir / "object_histroy");   
   fc::remove_all(data_dir / "limit_histroy");   
   fc::remove_all(data_dir / "balance_histroy");      
   ilog("Done wiping object databse.");
}

void object_database::open(const fc::path& data_dir)
{ try {
   _data_dir = data_dir;
   if( fc::exists( _data_dir / "object_database" / "lock" ) )
   {
       wlog("Ignoring locked object_database");
       return;
   }
   if(!fc::exists( _data_dir / "loan_database" ) )   
      fc::create_directories( _data_dir / "loan_database");
   if(!fc::exists( _data_dir / "locktoken_database" ) )
      fc::create_directories( _data_dir / "locktoken_database");
   if(!fc::exists( _data_dir / "finance_database" ) )
      fc::create_directories( _data_dir / "finance_database");
   if(!fc::exists( _data_dir / "object_histroy" ) )
      fc::create_directories( _data_dir / "object_histroy"); 
   if(!fc::exists( _data_dir / "limit_histroy" ) )
      fc::create_directories( _data_dir / "limit_histroy");       
   if(!fc::exists( _data_dir / "balance_histroy" ) )
      fc::create_directories( _data_dir / "balance_histroy"); 

   if( fc::exists( _data_dir / "object_database" ) )
   {
    ilog("Opening object database from ${d} ...", ("d", data_dir));
    for( uint32_t space = 0; space < _index.size(); ++space )
      for( uint32_t type = 0; type  < _index[space].size(); ++type )
         if( _index[space][type] )
            _index[space][type]->open( _data_dir / "object_database" / fc::to_string(space)/fc::to_string(type) );
    ilog( "Done opening object database." );
  }
  else
    ilog( "No object database." );

    _is_open = true;


} FC_CAPTURE_AND_RETHROW( (data_dir) ) }


void object_database::pop_undo()
{ try {
   _undo_db.pop_commit();
} FC_CAPTURE_AND_RETHROW() }

void object_database::save_undo( const object& obj )
{
   _undo_db.on_modify( obj );
}

void object_database::save_undo_add( const object& obj )
{
   _undo_db.on_create( obj );
}

void object_database::save_undo_remove(const object& obj)
{
   _undo_db.on_remove( obj );
}

} } // namespace graphene::db
