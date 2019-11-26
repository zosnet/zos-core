file(GLOB HEADERS "include/graphene/locktoken_history/*.hpp")

add_library( graphene_locktoken_history 
             locktoken_history_plugin.cpp            
           )

target_link_libraries( graphene_locktoken_history graphene_chain   graphene_business graphene_app )
target_include_directories( graphene_locktoken_history
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties( locktoken_history_plugin.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   graphene_locktoken_history

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/graphene/locktoken_history" )

