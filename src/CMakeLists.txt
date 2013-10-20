SET( models_srcs Models/group.cpp Models/user.cpp Models/mediarating.cpp Models/mediaattachment.cpp Models/sessioninfo.cpp)
SET( Wt_Commons_srcs ../Wt-Commons/whtmltemplateslocalizedstrings.cpp ../Wt-Commons/compositeresource.cpp ../Wt-Commons/quitresource.cpp ../Wt-Commons/migratedbo.cpp )
SET( mediascanner_srcs
    MediaScanner/savesubtitlestodatabase.cpp
    MediaScanner/createthumbnails.cpp
    libffmpegthumbnailer/videothumbnailer.cpp
    libffmpegthumbnailer/filmstripfilter.cpp
    libffmpegthumbnailer/pngwriter.cpp
    libffmpegthumbnailer/moviedecoder.cpp
    MediaScanner/scanmediainfostep.cpp
    MediaScanner/mediascannerdialog.cpp
    MediaScanner/mediascannerstep.cpp
)
SET( player_srcs serversettingspage.cpp Models/setting.cpp selectdirectories.cpp  findorphansdialog.cpp
    player/videojs.cpp
    player/wmediaplayerwrapper.cpp
    player/html5player.cpp
    player/mediaelementjs.cpp
    player/purehtml5js.cpp
)
include(FindPkgConfig)
message("PKGConfig Executable: ${PKG_CONFIG_EXECUTABLE}")
# TODO : difficult to find on windows 32. Remove,  or implement alternatives (standalone ffmpeg binary? )
pkg_check_modules(AVUTIL libavutil REQUIRED)
pkg_check_modules(AVFORMAT libavformat REQUIRED)
pkg_check_modules(AVCODEC libavcodec REQUIRED)
pkg_check_modules(SWSCALE libswscale REQUIRED)
pkg_check_modules(LIBPNG libpng REQUIRED)

include_directories(libffmpegthumbnailer)

include_directories(${AVUTIL_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIRS} ${AVCODEC_INCLUDE_DIRS} ${SWSCALE_INCLUDE_DIRS} ${LIBPNG_INCLUDE_DIRS})

if(QT4_FOUND)
  INCLUDE(   ${QT_USE_FILE}   )
  include_directories(${QT_INCLUDE_DIR}/QtNetwork) # Hack to skip include <QtModule/QtClass> include style
  message("Qt4 headers found: compiling with system tray icon support")
  add_definitions(-DHAVE_QT)
  if(WIN32)
    QT4_ADD_RESOURCES(Pandorica_RCC_SRCS ${CMAKE_SOURCE_DIR}/files/icons.qrc)
  endif(WIN32)
  QT4_WRAP_CPP(Pandorica_HEADERS_MOC qttrayicon.h)
  set(qt_only_classes qttrayicon.cpp ${Pandorica_HEADERS_MOC})
endif(QT4_FOUND)
if(Qt5Widgets_FOUND)
#INCLUDE(   ${QT_USE_FILE}   )
  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  message("Qt5 headers found: compiling with system tray icon support")
  add_definitions(-DHAVE_QT -DHAVE_QT5 -fPIC)
  if(WIN32)
    qt5_add_resources(Pandorica_RCC_SRCS ${CMAKE_SOURCE_DIR}/files/icons.qrc)
  endif(WIN32)
  qt5_wrap_cpp(Pandorica_HEADERS_MOC qttrayicon.h)
  set(qt_only_classes qttrayicon.cpp ${Pandorica_HEADERS_MOC})
endif(Qt5Widgets_FOUND)

set(pandorica_SOURCES ${Wt_Commons_srcs} ${models_srcs} ${mediascanner_srcs} ${player_srcs} 
  ${qt_only_classes}
    authpage.cpp
    ffmpegmedia.cpp
    latestcommentsdialog.cpp
    groupsdialog.cpp
    settingspage.cpp
    settings.cpp
    mediacollectionbrowser.cpp
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

add_subdirectory(media)
add_subdirectory(utils)

if(WIN32)
  if( (QT4_FOUND OR Qt5Widgets_FOUND) AND NOT DISABLE_WIN32)
    add_executable(${app_name} WIN32 ${pandorica_SOURCES}  pandorica.rc ${Pandorica_RCC_SRCS} )
  else((QT4_FOUND OR Qt5Widgets_FOUND) AND NOT DISABLE_WIN32)
    add_executable(${app_name} ${pandorica_SOURCES}  pandorica.rc ${Pandorica_RCC_SRCS} )
  endif((QT4_FOUND OR Qt5Widgets_FOUND) AND NOT DISABLE_WIN32)
else(WIN32)
  add_executable(${app_name} ${pandorica_SOURCES})
endif(WIN32)

if(Qt5Widgets_FOUND)
  qt5_use_modules(Pandorica Network)
endif(Qt5Widgets_FOUND)
  

string(TOLOWER "x${CMAKE_BUILD_TYPE}" build_type)
if("${build_type}" STREQUAL "xdebug")
  message("Linking to WT Debug Libraries")
  set(WT_LINK_LIBS ${Wt_DEBUG_LIBRARY} ${Wt_DBO_DEBUG_LIBRARY} ${Wt_DBOPOSTGRES_DEBUG_LIBRARY} ${Wt_DBOMYSQL_DEBUG_LIBRARY} ${Wt_DBOSQLITE3_DEBUG_LIBRARY} ${Wt_CONNECTOR_DEBUG_LIBRARY} )
else("${build_type}" STREQUAL "xdebug")
  message("Linking to WT Release Libraries")
  set(WT_LINK_LIBS ${Wt_LIBRARY} ${Wt_DBO_LIBRARY} ${Wt_DBOPOSTGRES_LIBRARY} ${Wt_DBOMYSQL_LIBRARY} ${Wt_DBOSQLITE3_LIBRARY} ${Wt_CONNECTOR_LIBRARY})
endif("${build_type}" STREQUAL "xdebug")
if(WIN32)
  # TODO: there has to be a better way to do all this, even on this infernal operating system...
  pkg_check_modules(SQLITE3 sqlite3)
  pkg_check_modules(PNG libpng)
  pkg_check_modules(TIFF libtiff-4)
  pkg_check_modules(ZLIB zlib)
  pkg_check_modules(XML libxml-2.0)
  pkg_check_modules(QTGUI QtGui)
  message("QtGui libs: ${QTGUI_LIBRARIES}, LDFLAGS: ${QTDGUI_LDFLAGS_OTHER}")
  set(WIN32_LIBS -lwsock32 -lws2_32 -lssp -lwinpthread -lgomp -lfreetype ${ZLIB_LIBRARIES} ${XML_LIBRARIES} -ljpeg -llcms2 ${SQLITE3_LIBRARIES} ${PNG_LIBRARIES} ${TIFF_LIBRARIES})
  if(${DBO_POSTGRES_FOUND} AND NOT (${DISABLE_POSTGRESQL}) )
    set(WIN32_LIBS ${WIN32_LIBS} -lpq)
  endif(${DBO_POSTGRES_FOUND} AND NOT (${DISABLE_POSTGRESQL}) ) 

endif(WIN32)

target_link_libraries(${app_name} ${Qt5Widgets_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTGUI_LIBRARY} 
  ${AVFORMAT_LDFLAGS} ${AVCODEC_LDFLAGS} ${AVUTIL_LDFLAGS} ${SWSCALE_LDFLAGS} ${LIBPNG_LIBRARIES}
  ${WT_LINK_LIBS} ${Boost_LIBRARIES} ${GRAPHICSMAGICK_LIBRARIES} ${WIN32_LIBS} )
install(TARGETS ${app_name} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

