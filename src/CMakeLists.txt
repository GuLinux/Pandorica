SET( models_srcs Models/group.cpp Models/user.cpp Models/mediarating.cpp Models/mediaattachment.cpp Models/sessioninfo.cpp)
SET( Wt_Commons_srcs ../Wt-Commons/whtmltemplateslocalizedstrings.cpp ../Wt-Commons/compositeresource.cpp ../Wt-Commons/quitresource.cpp )
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

SET(BOOST_LIBS 
-lboost_signals
-lboost_system
-lboost_filesystem
-lboost_thread
-lboost_regex
-lboost_program_options
)

target_link_libraries(${app_name} ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTGUI_LIBRARY} 
  -lavformat -lavcodec -lavutil -lffmpegthumbnailer ${BOOST_LIBS} ${WT_LINK_LIBS} ${MAGICK_LIBS})
install(TARGETS ${app_name} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)