add_executable( zos_node main.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling zos_node with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()
IF( WIN32 )
  list( APPEND PLATFORM_SPECIFIC_LIBS wldap32.lib Ws2_32.lib )
ENDIF(WIN32)

# We have to link against graphene_debug_witness because deficiency in our API infrastructure doesn't allow plugins to be fully abstracted #246
target_link_libraries( zos_node
   PRIVATE graphene_app graphene_account_history graphene_market_history graphene_bitlender_history graphene_finance_history graphene_object_history  graphene_grouped_orders graphene_witness graphene_chain graphene_business graphene_debug_witness graphene_egenesis_full graphene_snapshot fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )

install( TARGETS
   zos_node
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
