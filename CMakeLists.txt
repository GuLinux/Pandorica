cmake_minimum_required(VERSION 2.8)
project(videostreaming)
add_definitions(-std=gnu++11)

find_package(Wt REQUIRED)
include_directories(${Wt_INCLUDE_DIR} ${CMAKE_SOURCE_DIR} )


set(Wt_CONNECTOR_LIBRARY "${Wt_HTTP_LIBRARY}")
set(Wt_CONNECTOR_DEBUG_LIBRARY ${Wt_HTTP_DEBUG_LIBRARY})

execute_process(COMMAND hostname OUTPUT_VARIABLE HOSTNAME)
#execute_process(COMMAND echo Shogoki OUTPUT_VARIABLE HOSTNAME)

STRING(REGEX MATCH ".*Zerogoki.*" HOST_IS_ZEROGOKI "${HOSTNAME}")
# if(NOT ${HOST_IS_ZEROGOKI} STREQUAL "")
#   set(Wt_CONNECTOR_LIBRARY ${Wt_FCGI_LIBRARY})
#   set(Wt_CONNECTOR_DEBUG_LIBRARY ${Wt_FCGI_DEBUG_LIBRARY})
#   message("Zerogoki hostname: using FCGI Connector (${Wt_CONNECTOR_LIBRARY} - ${Wt_CONNECTOR_DEBUG_LIBRARY}")
# endif(NOT ${HOST_IS_ZEROGOKI} STREQUAL "")

SET(INSTALL_DIR "/home/www-data/data/videostreaming" CACHE STRING "Install directory for Wt Applications and files")

add_executable(videostreaming MediaScanner/savesubtitlestodatabase.cpp MediaScanner/createthumbnails.cpp MediaScanner/scanmediainfostep.cpp MediaScanner/mediascannerdialog.cpp ffmpegmedia.cpp latestcommentsdialog.cpp groupsdialog.cpp user.cpp group.cpp settingspage.cpp settings.cpp media.cpp mediacollectionbrowser.cpp mediacollection.cpp Wt-Commons/whtmltemplateslocalizedstrings.cpp player/html5player.cpp 
readbwstats.cpp commentscontainerwidget.cpp sessiondetailsdialog.cpp customitemdelegates.cpp loggedusersdialog.cpp playlist.cpp session.cpp 
player/videojs.cpp player/wmediaplayerwrapper.cpp streamingapp.cpp main.cpp)


string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
if(${build_type} STREQUAL "debug")
  message("Linking to WT Debug Libraries")
  set(WT_LINK_LIBS ${Wt_DEBUG_LIBRARY} ${Wt_DBO_DEBUG_LIBRARY} ${Wt_DBOPOSTGRES_DEBUG_LIBRARY} ${Wt_DBOSQLITE3_DEBUG_LIBRARY} ${Wt_CONNECTOR_DEBUG_LIBRARY} )
else(${build_type} STREQUAL "debug")
  message("Linking to WT Release Libraries")
  set(WT_LINK_LIBS ${Wt_LIBRARY} ${Wt_DBO_LIBRARY} ${Wt_DBOPOSTGRES_LIBRARY} ${Wt_DBOSQLITE3_LIBRARY} ${Wt_CONNECTOR_LIBRARY})
endif(${build_type} STREQUAL "debug")

SET(BOOST_LIBS 
-lboost_signals
-lboost_system
-lboost_filesystem
-lboost_thread
-lboost_regex
)

target_link_libraries(videostreaming -lavformat -lavcodec -lavutil -lffmpegthumbnailer ${BOOST_LIBS} ${WT_LINK_LIBS} )
install(TARGETS videostreaming RUNTIME DESTINATION ${INSTALL_DIR}/bin)
set(TRANSLATION_FILES strings.xml strings_it.xml )
INSTALL(FILES ${TRANSLATION_FILES} DESTINATION ${INSTALL_DIR} )
INSTALL(FILES title_from_filename_replacements.json DESTINATION ${INSTALL_DIR})
INSTALL(DIRECTORY html_templates DESTINATION ${INSTALL_DIR} FILES_MATCHING PATTERN "*.html" )
