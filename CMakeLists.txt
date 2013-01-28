cmake_minimum_required(VERSION 2.8)
project(videostreaming)
add_definitions(-std=gnu++11)

find_package(Wt REQUIRED)
include_directories(${Wt_INCLUDE_DIR}  )


set(Wt_CONNECTOR_LIBRARY "${Wt_HTTP_LIBRARY}")
set(Wt_CONNECTOR_DEBUG_LIBRARY ${Wt_HTTP_DEBUG_LIBRARY})
execute_process(COMMAND hostname OUTPUT_VARIABLE HOSTNAME)
if(${HOSTNAME} STREQUAL "Zerogoki")
  set(Wt_CONNECTOR_LIBRARY ${Wt_FCGI_LIBRARY})
  set(Wt_CONNECTOR_DEBUG_LIBRARY ${Wt_FCGI_DEBUG_LIBRARY})
  message("Zerogoki hostname: using FCGI Connector (${Wt_CONNECTOR_LIBRARY} - ${Wt_CONNECTOR_DEBUG_LIBRARY}")
endif(${HOSTNAME} STREQUAL "Zerogoki")

SET(INSTALL_DIR "/var/www/videostreaming" CACHE STRING "Install directory for Wt Applications and files")


add_executable(videostreaming streamingapp.cpp main.cpp)


string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
if(${build_type} STREQUAL "debug")
  message("Linking to WT Debug Libraries")
  set(WT_LINK_LIBS ${Wt_DEBUG_LIBRARY} ${Wt_DBO_DEBUG_LIBRARY} ${Wt_CONNECTOR_DEBUG_LIBRARY} )
else(${build_type} STREQUAL "debug")
  message("Linking to WT Release Libraries")
  set(WT_LINK_LIBS ${Wt_LIBRARY} ${Wt_DBO_LIBRARY} ${Wt_CONNECTOR_LIBRARY} )
endif(${build_type} STREQUAL "debug")

target_link_libraries(videostreaming ${WT_LINK_LIBS} -lboost_signals -lboost_system -lboost_filesystem )
install(TARGETS videostreaming RUNTIME DESTINATION ${INSTALL_DIR}/bin)

