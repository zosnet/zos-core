
 

file(GLOB HEADERS "include/graphene/business/*.hpp")
file(GLOB PROTOCOL_HEADERS "include/graphene/business/protocol/*.hpp")
   
if (GRAPHENE_DISABLE_BUSINESS_BUILD  )  

message( STATUS "Graphene business unity imported" )

add_library(graphene_business STATIC IMPORTED GLOBAL)
if( APPLE )
  set_property(TARGET graphene_business PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libgraphene_business_ios.a)
else( APPLE )
  set_property(TARGET graphene_business PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libgraphene_business_linux.a)
endif( APPLE )
set_property(TARGET graphene_business PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include)


else (GRAPHENE_DISABLE_BUSINESS_BUILD  ) 

message( STATUS "Graphene business unity build" )

add_library( graphene_business             
             protocol/market.cpp                      
             protocol/finance_ops.cpp
             protocol/bitlender_option_ops.cpp
             protocol/bitlender_ops.cpp              
             protocol/locktoken.cpp   
             protocol/transfer_vesting.cpp 
             protocol/confidential.cpp
             db_market.cpp
             db_business.cpp
             bitlender_option_evaluator.cpp
             bitlender_evaluator.cpp
             bitlender_object.cpp                            
             finance_evaluator.cpp 
             market_evaluator.cpp
             market_object.cpp 
             locktoken_evaluator.cpp           
             locktoken_object.cpp
             bitlender_option_object.cpp
             finance_object.cpp  
             transfer_vesting_evaluator.cpp
             confidential_evaluator.cpp
             
             ${HEADERS}
             ${PROTOCOL_HEADERS}           
           )

 
target_link_libraries( graphene_business fc graphene_db graphene_chain)
target_include_directories( graphene_business 
                            PUBLIC "${CMAKE_SOURCE_DIR}/libraries/chain/include"
                            PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include" )

if(MSVC)
  set_source_files_properties( db_market.cpp db_business.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

INSTALL( TARGETS
   graphene_business

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

endif( GRAPHENE_DISABLE_BUSINESS_BUILD )


INSTALL( FILES ${HEADERS} DESTINATION "include/graphene/business" )
INSTALL( FILES ${PROTOCOL_HEADERS} DESTINATION "include/graphene/business/protocol" )

 