message("Configuring FFMPEG Libraries in ${CMAKE_CURRENT_BINARY_DIR}/build")
execute_process(COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/build)
set(configure_log_file ${CMAKE_CURRENT_BINARY_DIR}/pandorica_ffmpeg_configure.log)
set(make_log_file ${CMAKE_CURRENT_BINARY_DIR}/pandorica_ffmpeg_make.log)

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/build/config.mak)
  message("Configuring FFMPEG Library (logs in ${configure_log_file} and ${make_log_file}")
  execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/configure
    --disable-programs
    --disable-doc
    --disable-avdevice
    --disable-postproc
    --disable-avfilter
    --prefix="${CMAKE_CURRENT_BINARY_DIR}/ffmpeg"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build"
    OUTPUT_FILE ${configure_log_file}
    ERROR_FILE ${configure_log_file}
    RESULT_VARIABLE FFMPEG_CONFIGURE_RESULT
  )
  if(NOT FFMPEG_CONFIGURE_RESULT EQUAL 0)
    message("Error configuring ffmpeg!")
    execute_process(COMMAND cat ${configure_log_file})
    message(FATAL_ERROR "Aborting Pandorica Configuration")
  endif(NOT FFMPEG_CONFIGURE_RESULT EQUAL 0)
endif(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/build/config.mak)


if(NOT DEFINED PROCESSOR_COUNT)
  # Unknown:
  set(PROCESSOR_COUNT 0)

  # Linux:
  set(cpuinfo_file "/proc/cpuinfo")
  if(EXISTS "${cpuinfo_file}")
    file(STRINGS "${cpuinfo_file}" procs REGEX "^processor.: [0-9]+$")
    list(LENGTH procs PROCESSOR_COUNT)
  endif()

  # Mac:
  if(APPLE)
    find_program(cmd_sys_pro "system_profiler")
    if(cmd_sys_pro)
      execute_process(COMMAND ${cmd_sys_pro} OUTPUT_VARIABLE info)
      string(REGEX REPLACE "^.*Total Number Of Cores: ([0-9]+).*$" "\\1"
        PROCESSOR_COUNT "${info}")
    endif()
  endif()

  # Windows:
  if(WIN32)
    set(PROCESSOR_COUNT "$ENV{NUMBER_OF_PROCESSORS}")
  endif()
endif()
set(MAKE_JOB_OPTIONS ${PROCESSOR_COUNT})
#math(EXPR MAKE_JOB_OPTIONS ${PROCESSOR_COUNT}+1)
#message("FFMPEG Build with -j${MAKE_JOB_OPTIONS}")

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

set(ffmpeg_build_command test -r ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/lib/libavcodec.a || make -j${MAKE_JOB_OPTIONS} all install > ${make_log_file} 2>&1)
add_custom_target(ffmpeg_libraries ${ffmpeg_build_command} WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build" )
set(FFMPEG_STATIC_ROOT  ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_static PARENT_SCOPE)
