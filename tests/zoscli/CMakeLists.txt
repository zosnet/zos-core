add_executable( zoscli_test main.cpp  init_node.cpp init_block.cpp create_block.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()



find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling zoscli_test with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif() 

if(MSVC)
  set_source_files_properties( main.cpp init_node.cpp init_block.cpp create_block.cpp  PROPERTIES COMPILE_FLAGS "/bigobj" ) 
endif(MSVC)  

target_link_libraries( zoscli_test graphene_app graphene_wallet graphene_witness graphene_account_history graphene_net graphene_chain  graphene_business  graphene_egenesis_none fc ${PLATFORM_SPECIFIC_LIBS})


install( TARGETS
   zoscli_test

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
