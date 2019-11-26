file(GLOB HEADERS "include/graphene/store_db/*.hpp")

add_library( graphene_store_db
        store_operation.cpp 
        store_object.cpp
        store_db.cpp
           )

target_link_libraries( graphene_store_db graphene_chain   graphene_business graphene_app  )
target_include_directories( graphene_store_db
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties(store.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   graphene_store_db

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/graphene/store_db_plugin" )

