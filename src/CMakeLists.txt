SET( models_srcs 
  Models/group.cpp
  Models/user.cpp
  Models/mediarating.cpp
  Models/mediaattachment.cpp
)
SET( player_srcs serversettingspage.cpp Models/setting.cpp selectdirectories.cpp  findorphansdialog.cpp
    player/videojs.cpp
    player/wmediaplayerwrapper.cpp
    player/html5player.cpp
    player/mediaelementjs.cpp
    player/purehtml5js.cpp
)

include_directories(${FFMPEG_STATIC_INCLUDES})

if(QT4_FOUND)
  INCLUDE(   ${QT_USE_FILE}   )
  include_directories(${QT_INCLUDE_DIR}/QtNetwork) # Hack to skip include <QtModule/QtClass> include style
  message("Qt4 headers found: compiling with system tray icon support")
  add_definitions(-DHAVE_QT)
  QT4_ADD_RESOURCES(Pandorica_RCC_SRCS ${CMAKE_SOURCE_DIR}/files/icons.qrc)
  QT4_WRAP_CPP(Pandorica_HEADERS_MOC qttrayicon.h)
  set(qt_only_classes qttrayicon.cpp ${Pandorica_HEADERS_MOC})
endif(QT4_FOUND)
if(Qt5Widgets_FOUND)
#INCLUDE(   ${QT_USE_FILE}   )
  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  message("Qt5 headers found: compiling with system tray icon support")
  add_definitions(-DHAVE_QT -DHAVE_QT5 )
  qt5_add_resources(Pandorica_RCC_SRCS ${CMAKE_SOURCE_DIR}/files/icons.qrc)
  qt5_wrap_cpp(Pandorica_HEADERS_MOC qttrayicon.h)
  set(qt_only_classes qttrayicon.cpp ${Pandorica_HEADERS_MOC})
endif(Qt5Widgets_FOUND)

set(pandorica_SOURCES ${models_srcs} ${player_srcs} 
  ${qt_only_classes}
    authpage.cpp
    ffmpegmedia.cpp
    latestcommentsdialog.cpp
    groupsdialog.cpp
    settingspage.cpp
    settings.cpp
    mediacollectionbrowser.cpp
    mediainfopanel.cpp
    commentscontainerwidget.cpp
    customitemdelegates.cpp
    playlist.cpp
    session.cpp 
    pandorica.cpp
    navigationbar.cpp 
    usersmanagementpage.cpp
    main.cpp
    utils/image.cpp
    fileresource.cpp
    savemediainformation.cpp
    savemediathumbnail.cpp
    mediathumbnailgenerator.cpp
    threadpool.cpp
    googlepicker.cpp
    mediapreviewwidget.cpp
    pandoricawizard.cpp
    ${Pandorica_RCC_SRCS}
)

add_subdirectory(media)
add_subdirectory(utils)

add_executable(${app_name} ${pandorica_SOURCES})

if(Qt5Widgets_FOUND)
  qt5_use_modules(Pandorica Network Widgets)
  set(Pandorica_Qt_Libraries Qt5::Widgets Qt5::Network)
endif(Qt5Widgets_FOUND)

add_dependencies(${app_name} ffmpeg_libraries)
message("FFMPEG Include Directory: ${AVFORMAT_INCLUDE_DIRS}, library directory: ${AVFORMAT_LIBRARY_DIRS})")
target_link_libraries(${app_name}
  -rdynamic
  Wt_Commons_Library
  pandorica_libffmpegthumbnailer
  ${Wt_LIBRARIES}
  ${Pandorica_Qt_Libraries}
  ${Wt_STATIC_DEPENDENCIES}
  ${Boost_LIBRARIES} 
  ${GraphicsMagick_LDFLAGS} 
  ${AVFORMAT_LDFLAGS} 
  ${AVCODEC_LDFLAGS} 
  ${AVUTIL_LDFLAGS} 
  ${SWSCALE_LDFLAGS} 
  ${PNG_LIBRARIES} 
  ${JPEG_LIBRARIES}
)
install(TARGETS ${app_name} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

