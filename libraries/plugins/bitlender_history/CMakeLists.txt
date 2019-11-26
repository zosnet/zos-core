file(GLOB HEADERS "include/graphene/bitlender_history/*.hpp")

add_library( graphene_bitlender_history 
             bitlender_history_plugin.cpp            
           )

target_link_libraries( graphene_bitlender_history graphene_chain   graphene_business graphene_app )
target_include_directories( graphene_bitlender_history
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties( bitlender_history_plugin.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   graphene_bitlender_history

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/graphene/bitlender_history" )

