SET( models_srcs Models/group.cpp Models/user.cpp Models/mediarating.cpp Models/mediaattachment.cpp Models/sessioninfo.cpp)
SET( Wt_Commons_srcs ../Wt-Commons/whtmltemplateslocalizedstrings.cpp ../Wt-Commons/compositeresource.cpp ../Wt-Commons/quitresource.cpp ../Wt-Commons/migratedbo.cpp )
SET( mediascanner_srcs
    MediaScanner/savesubtitlestodatabase.cpp
    MediaScanner/createthumbnails.cpp
    MediaScanner/scanmediainfostep.cpp
    MediaScanner/mediascannerdialog.cpp
)
SET( player_srcs serversettingspage.cpp Models/setting.cpp selectdirectories.cpp  findorphansdialog.cpp
    player/videojs.cpp
    player/wmediaplayerwrapper.cpp
    player/html5player.cpp
    player/mediaelementjs.cpp
    player/purehtml5js.cpp
)
include(FindPkgConfig)
find_package(Boost REQUIRED COMPONENTS signals system filesystem thread regex program_options )

message("PKGConfig Executable: ${PKG_CONFIG_EXECUTABLE}")
# TODO : difficult to find on windows 32. Remove,  or implement alternatives (standalone ffmpeg binary? )
pkg_check_modules(FFMPEG_THUMBNAILER libffmpegthumbnailer)
pkg_check_modules(AVUTIL libavutil)
pkg_check_modules(AVFORMAT libavformat)
pkg_check_modules(AVCODEC libavcodec)

if(FFMPEG_THUMBNAILER_FOUND)
  add_definitions(-DFFMPEG_THUMBNAILER_FOUND)
  include_directories(${FFMPEG_THUMBNAILER_INCLUDE_DIRS})
endif(FFMPEG_THUMBNAILER_FOUND)
if(AVUTIL_FOUND AND AVFORMAT_FOUND AND AVCODEC_FOUND)
  add_definitions(-DFFMPEG_FOUND)
  include_directories(${AVUTIL_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIRS} ${AVCODEC_INCLUDE_DIRS})
  set(ffmpeg_media_class ffmpegmedia.cpp)
endif(AVUTIL_FOUND AND AVFORMAT_FOUND AND AVCODEC_FOUND)

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
    ${ffmpeg_media_class}
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
    pandorica.cpp
    navigationbar.cpp 
    main.cpp
)


string(TOLOWER "x${CMAKE_BUILD_TYPE}" build_type)
if("${build_type}" STREQUAL "xdebug")
  message("Linking to WT Debug Libraries")
  set(WT_LINK_LIBS ${Wt_DEBUG_LIBRARY} ${Wt_DBO_DEBUG_LIBRARY} ${Wt_DBOPOSTGRES_DEBUG_LIBRARY} ${Wt_DBOMYSQL_DEBUG_LIBRARY} ${Wt_DBOSQLITE3_DEBUG_LIBRARY} ${Wt_CONNECTOR_DEBUG_LIBRARY} )
else("${build_type}" STREQUAL "xdebug")
  message("Linking to WT Release Libraries")
  set(WT_LINK_LIBS ${Wt_LIBRARY} ${Wt_DBO_LIBRARY} ${Wt_DBOPOSTGRES_LIBRARY} ${Wt_DBOMYSQL_LIBRARY} ${Wt_DBOSQLITE3_LIBRARY} ${Wt_CONNECTOR_LIBRARY})
endif("${build_type}" STREQUAL "xdebug")

target_link_libraries(${app_name} ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTGUI_LIBRARY} 
  ${AVFORMAT_LIBRARIES} ${AVCODEC_LIBRARIES} ${AVUTIL_LIBRARIES} ${FFMPEG_THUMBNAILER_LIBRARIES} ${Boost_LIBRARIES} ${WT_LINK_LIBS} ${MAGICK_LIBS})
install(TARGETS ${app_name} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)