cmake_minimum_required(VERSION 2.8.9)
enable_testing()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/" ${CMAKE_SOURCE_DIR}/Wt-Commons/cmake/)
set(app_name Pandorica)
project(${app_name})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_definitions(-DWT_NO_SLOT_MACROS)
find_package(Wt 3.3.1 REQUIRED)

set(HAVE_DBO_BACKEND FALSE)
if(Wt_DBOPOSTGRES_LIBRARY)
  add_definitions(-DHAVE_POSTGRES)
  message("Wt Postgres DBO Library found: ${Wt_DBOPOSTGRES_LIBRARY}")
  set(HAVE_DBO_BACKEND TRUE)
endif(Wt_DBOPOSTGRES_LIBRARY)

if(Wt_DBOSQLITE3_LIBRARY)
  add_definitions(-DHAVE_SQLITE3)
  message("Wt Sqlite3 DBO Library found: ${Wt_DBOSQLITE3_LIBRARY}")
  set(HAVE_DBO_BACKEND TRUE)
endif(Wt_DBOSQLITE3_LIBRARY)

if(NOT HAVE_DBO_BACKEND)
  message(FATAL_ERROR "You must install at least one database backend library (sqlite3, postgres)")
endif(NOT HAVE_DBO_BACKEND)

find_package(PNG REQUIRED)
find_package(JPEG)
if(JPEG_FOUND)
  add_definitions(-DHAVE_JPEG)
endif(JPEG_FOUND)
if(PNG_FOUND)
  add_definitions(-DHAVE_PNG)
endif(PNG_FOUND)

include_directories(${PNG_INCLUDE_DIRS} ${JPEG_INCLUDE_DIR})

INCLUDE (CheckIncludeFiles)
include(FindPackageHandleStandardArgs)
find_package(Boost REQUIRED COMPONENTS signals system filesystem chrono thread regex program_options random date_time serialization OPTIONAL_COMPONENTS unit_test_framework )
set(Disable_Qt5 False CACHE bool "Disable Qt5 detection")

set(Boost_Optional_Linker_Fix False CACHE bool "Enable fix to boost optional ostream linker error (default: false)")
if(Boost_Optional_Linker_Fix)
  add_definitions(-DBOOST_OSTREAM_OPERATOR_OPTIONAL_FIX)
endif(Boost_Optional_Linker_Fix)

if( NOT Disable_Qt5 )
  FIND_PACKAGE(Qt5Widgets)
  if(Qt5Widgets_FOUND)
    add_definitions(-DHAVE_QT5)
  endif(Qt5Widgets_FOUND)
endif( NOT Disable_Qt5 )

add_definitions(-DBOOST_SIGNALS_NO_DEPRECATION_WARNING)

#add_definitions(-fno-stack-protector )
if(NOT Qt5Widgets_FOUND)
  find_package(GraphicsMagick REQUIRED)
  if(GraphicsMagick_FOUND)
    add_definitions(-DIMAGE_USE_GRAPHICSMAGICK)
  endif(GraphicsMagick_FOUND)
endif(NOT Qt5Widgets_FOUND)

include_directories(${Wt_INCLUDE_DIR} ${CMAKE_SOURCE_DIR} ${GRAPHICSMAGICK_INCLUDE_DIRS} src )


SET(SHARED_FILES_DIR ${CMAKE_INSTALL_PREFIX}/share/${app_name} CACHE string "Shared files installation directory")
add_definitions(-DSHARED_FILES_DIR="${SHARED_FILES_DIR}" )
add_definitions(-DWT_SHARED_FILES_DIR="${Wt_SHARED_DIR}" )


add_subdirectory(files)
add_subdirectory(ffmpeg-static)
add_subdirectory(Wt-Commons)
add_subdirectory(libffmpegthumbnailer)
add_subdirectory(src)

set(ENABLE_TESTS Off CACHE BOOL "Enable tests execution")
if(ENABLE_TESTS)
  message("Enabling tests")
  add_subdirectory(tests)
else(ENABLE_TESTS)
  message("Tests disabled")
endif(ENABLE_TESTS)

