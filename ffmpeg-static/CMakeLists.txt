message("Configuring FFMPEG Libraries in ${CMAKE_CURRENT_BINARY_DIR}/build")
execute_process(COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/build)
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/configure
  --disable-programs
  --disable-doc
  --disable-avdevice
  --disable-postproc
  --disable-avfilter
  --prefix="${CMAKE_CURRENT_BINARY_DIR}/ffmpeg"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build"
)
set(PKG_CONFIG_PATH ${PKG_CONFIG_PATH}:${CMAKE_CURRENT_BINARY_DIR}/build/libavcodec)
set(PKG_CONFIG_PATH ${PKG_CONFIG_PATH}:${CMAKE_CURRENT_BINARY_DIR}/build/libavformat)
set(PKG_CONFIG_PATH ${PKG_CONFIG_PATH}:${CMAKE_CURRENT_BINARY_DIR}/build/libswscale)
set(PKG_CONFIG_PATH ${PKG_CONFIG_PATH}:${CMAKE_CURRENT_BINARY_DIR}/build/libswresample)
set(PKG_CONFIG_PATH ${PKG_CONFIG_PATH}:${CMAKE_CURRENT_BINARY_DIR}/build/libavutil)
set(ENV{PKG_CONFIG_PATH} ${PKG_CONFIG_PATH})
include(FindPkgConfig)
pkg_check_modules(AVUTIL libavutil REQUIRED)
pkg_check_modules(AVFORMAT libavformat REQUIRED)
pkg_check_modules(AVCODEC libavcodec REQUIRED)
pkg_check_modules(SWSCALE libswscale REQUIRED)
pkg_check_modules(LIBPNG libpng REQUIRED)

add_custom_target(ffmpeg_libraries make ${MAKEOPTS} all install
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build"
)
set(FFMPEG_STATIC_ROOT  ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_static PARENT_SCOPE)
