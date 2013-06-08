cmake_minimum_required(VERSION 2.8)
set(app_name Pandorica)
project(${app_name})
add_definitions(-std=gnu++11 -DWT_NO_SLOT_MACROS)
find_package(Wt REQUIRED)
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_magick_cmake_conf ${CMAKE_CURRENT_BINARY_DIR}/imagemagick.cmake )
INCLUDE (CheckIncludeFiles)
include(${CMAKE_CURRENT_BINARY_DIR}/imagemagick.cmake)
include(FindPackageHandleStandardArgs)
FIND_PACKAGE(Qt4 )

include_directories(${Wt_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_SOURCE_DIR} ${MAGICK_INCLUDES} )

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

SET(SHARED_FILES_DIR ${CMAKE_INSTALL_PREFIX}/share/${app_name})
add_definitions(-DSHARED_FILES_DIR="${SHARED_FILES_DIR}" )

SET( models_srcs Models/group.cpp Models/user.cpp Models/mediarating.cpp Models/mediaattachment.cpp )
SET( Wt_Commons_srcs Wt-Commons/whtmltemplateslocalizedstrings.cpp )
SET( mediascanner_srcs
    MediaScanner/savesubtitlestodatabase.cpp
    MediaScanner/createthumbnails.cpp
    MediaScanner/scanmediainfostep.cpp
    MediaScanner/mediascannerdialog.cpp
)
SET( player_srcs serversettingsdialog.cpp Models/setting.cpp selectdirectories.cpp Wt-Commons/compositeresource.cpp findorphansdialog.cpp
    player/videojs.cpp
    player/wmediaplayerwrapper.cpp
    player/html5player.cpp
)


if(${QT_FOUND})
  message("Qt4 headers found: compiling with system tray icon support")
  add_definitions(-DHAVE_QT)
  QT4_WRAP_CPP(Pandorica_HEADERS_MOC qttrayicon.h)
  set(qt_only_classes qttrayicon.cpp ${Pandorica_HEADERS_MOC})
endif(${QT_FOUND})

add_executable(${app_name} ${Wt_Commons_srcs} ${models_srcs} ${mediascanner_srcs} ${player_srcs} 
  ${qt_only_classes}
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
if("${build_type}" STREQUAL "debug")
  message("Linking to WT Debug Libraries")
  set(WT_LINK_LIBS ${Wt_DEBUG_LIBRARY} ${Wt_DBO_DEBUG_LIBRARY} ${Wt_DBOPOSTGRES_DEBUG_LIBRARY} ${Wt_DBOMYSQL_DEBUG_LIBRARY} ${Wt_DBOSQLITE3_DEBUG_LIBRARY} ${Wt_CONNECTOR_DEBUG_LIBRARY} )
else("${build_type}" STREQUAL "debug")
  message("Linking to WT Release Libraries")
  set(WT_LINK_LIBS ${Wt_LIBRARY} ${Wt_DBO_LIBRARY} ${Wt_DBOPOSTGRES_LIBRARY} ${Wt_DBOMYSQL_LIBRARY} ${Wt_DBOSQLITE3_LIBRARY} ${Wt_CONNECTOR_LIBRARY})
endif("${build_type}" STREQUAL "debug")

SET(BOOST_LIBS 
-lboost_signals
-lboost_system
-lboost_filesystem
-lboost_thread
-lboost_regex
-lboost_program_options
)

target_link_libraries(${app_name} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}  -lavformat -lavcodec -lavutil -lffmpegthumbnailer ${BOOST_LIBS} ${WT_LINK_LIBS} ${MAGICK_LIBS})
install(TARGETS ${app_name} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
set(TRANSLATION_FILES files/strings.xml files/strings_it.xml )
INSTALL(FILES ${TRANSLATION_FILES} DESTINATION ${SHARED_FILES_DIR} )
INSTALL(FILES files/title_from_filename_replacements.json DESTINATION ${SHARED_FILES_DIR})
INSTALL(DIRECTORY files/html_templates DESTINATION ${SHARED_FILES_DIR} FILES_MATCHING PATTERN "*.html" )
INSTALL(DIRECTORY files/static DESTINATION ${SHARED_FILES_DIR} )
if(${QT_FOUND})
  INSTALL(FILES files/desktop-files/pandorica.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
  INSTALL(DIRECTORY files/desktop-files/hicolor DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons )
endif(${QT_FOUND})
