add_executable( store_node my_sql.cpp main.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling store_node with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

IF(NOT "$ENV{MYSQL_ROOT_DIR}" STREQUAL "")
  set(MYSQL_ROOT_DIR $ENV{MYSQL_ROOT_DIR} )
  set(MYSQL_INCLUDE_DIR $ENV{MYSQL_INCLUDE_DIR})
  message(STATUS "Setting up mysql root and include vars to ${MYSQL_ROOT_DIR}, ${MYSQL_INCLUDE_DIR}")
ENDIF()

message (STATUS "MYSQL_ROOT_DIR    = ${MYSQL_ROOT_DIR}")
message (STATUS "MYSQL_INCLUDE_DIR = ${MYSQL_INCLUDE_DIR}")

#find_package(MySQL REQUIRED)

target_include_directories(fc
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}   
    ${MYSQL_INCLUDE_DIR}
  )

target_link_libraries( store_node
                       PRIVATE graphene_app graphene_account_history graphene_market_history graphene_bitlender_history  graphene_finance_history graphene_store_db  graphene_delayed_node graphene_object_history  graphene_chain   graphene_business graphene_egenesis_full mysqlclient  fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )

install( TARGETS
   store_node

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
