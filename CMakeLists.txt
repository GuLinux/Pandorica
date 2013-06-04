cmake_minimum_required(VERSION 2.8)
project(streamingapp)
add_definitions(-std=gnu++11)
find_package(Wt REQUIRED)
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_magick_cmake_conf ${CMAKE_CURRENT_BINARY_DIR}/imagemagick.cmake )
INCLUDE (CheckIncludeFiles)
include(${CMAKE_CURRENT_BINARY_DIR}/imagemagick.cmake)
include(FindPackageHandleStandardArgs)

include_directories(${Wt_INCLUDE_DIR} ${CMAKE_SOURCE_DIR} ${MAGICK_INCLUDES} )

find_path(HAVE_DBO_MYSQL Wt/Dbo/backend/MySQL)
find_path(HAVE_DBO_POSTGRES Wt/Dbo/backend/Postgres)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DBO_MYSQL DEFAULT_MSG HAVE_DBO_MYSQL)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DBO_POSTGRES DEFAULT_MSG HAVE_DBO_POSTGRES)

if(${DBO_MYSQL_FOUND})
  add_definitions(-DHAVE_MYSQL)
  set(Wt_DBOMYSQL_DEBUG_LIBRARY -lwtdbomysqld)
  set(Wt_DBOMYSQL_LIBRARY -lwtdbomysql)
endif(${DBO_MYSQL_FOUND})
if(${DBO_POSTGRES_FOUND})
  add_definitions(-DHAVE_POSTGRES)
endif(${DBO_POSTGRES_FOUND})



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

SET(INSTALL_DIR "/home/www-data/data/streamingapp" CACHE STRING "Install directory for Wt Applications and files")

SET( models_srcs Models/group.cpp Models/user.cpp Models/mediarating.cpp Models/mediaattachment.cpp )
SET( Wt_Commons_srcs Wt-Commons/whtmltemplateslocalizedstrings.cpp )
SET( mediascanner_srcs
    MediaScanner/savesubtitlestodatabase.cpp
    MediaScanner/createthumbnails.cpp
    MediaScanner/scanmediainfostep.cpp
    MediaScanner/mediascannerdialog.cpp
)
SET( player_srcs Models/setting.cpp selectdirectories.cpp Wt-Commons/compositeresource.cpp findorphansdialog.cpp
    player/videojs.cpp
    player/wmediaplayerwrapper.cpp
    player/html5player.cpp
)

add_executable(streamingapp ${Wt_Commons_srcs} ${models_srcs} ${mediascanner_srcs} ${player_srcs}
    authpage.cpp
    utils.cpp
    ffmpegmedia.cpp
    latestcommentsdialog.cpp
    groupsdialog.cpp
    settingspage.cpp
    settings.cpp
    media.cpp
    mediacollectionbrowser.cpp
    mediacollection.cpp 
    commentscontainerwidget.cpp
    sessiondetailsdialog.cpp
    customitemdelegates.cpp
    loggedusersdialog.cpp
    playlist.cpp
    session.cpp 
    streamingapp.cpp
    main.cpp
)


string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
if(${build_type} STREQUAL "debug")
  message("Linking to WT Debug Libraries")
  set(WT_LINK_LIBS ${Wt_DEBUG_LIBRARY} ${Wt_DBO_DEBUG_LIBRARY} ${Wt_DBOPOSTGRES_DEBUG_LIBRARY} ${Wt_DBOMYSQL_DEBUG_LIBRARY} ${Wt_DBOSQLITE3_DEBUG_LIBRARY} ${Wt_CONNECTOR_DEBUG_LIBRARY} )
else(${build_type} STREQUAL "debug")
  message("Linking to WT Release Libraries")
  set(WT_LINK_LIBS ${Wt_LIBRARY} ${Wt_DBO_LIBRARY} ${Wt_DBOPOSTGRES_LIBRARY} ${Wt_DBOMYSQL_LIBRARY} ${Wt_DBOSQLITE3_LIBRARY} ${Wt_CONNECTOR_LIBRARY})
endif(${build_type} STREQUAL "debug")

SET(BOOST_LIBS 
-lboost_signals
-lboost_system
-lboost_filesystem
-lboost_thread
-lboost_regex
)

target_link_libraries(streamingapp -lavformat -lavcodec -lavutil -lffmpegthumbnailer ${BOOST_LIBS} ${WT_LINK_LIBS} ${MAGICK_LIBS})
install(TARGETS streamingapp RUNTIME DESTINATION ${INSTALL_DIR}/bin)
set(TRANSLATION_FILES files/strings.xml files/strings_it.xml )
INSTALL(FILES ${TRANSLATION_FILES} DESTINATION ${INSTALL_DIR} )
INSTALL(FILES files/title_from_filename_replacements.json DESTINATION ${INSTALL_DIR})
INSTALL(DIRECTORY files/html_templates DESTINATION ${INSTALL_DIR} FILES_MATCHING PATTERN "*.html" )
INSTALL(DIRECTORY files/static DESTINATION ${INSTALL_DIR}/files)
