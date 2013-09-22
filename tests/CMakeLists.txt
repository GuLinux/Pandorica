if(Boost_UNIT_TEST_FRAMEWORK_LIBRARY)
  add_definitions(-DBOOST_TEST_DYN_LINK)
  add_custom_target(compile_unit_tests)
  add_custom_target(compile_integration_tests)
  add_custom_target(unit_tests COMMAND ctest --output-on-failure -R "unit.*" DEPENDS compile_unit_tests)
  add_custom_target(integration_tests COMMAND ctest --output-on-failure -R "integration.*" DEPENDS compile_integration_tests)
  
  function(register_test name type)
    set(test_name ${type}_test_${name})
    add_executable(${test_name} EXCLUDE_FROM_ALL ${ARGN} )
    add_test(${type}_${name} ${test_name})
    add_dependencies(compile_${type}_tests ${test_name})
  endfunction()
  
  function(link_unit_test name)
    target_link_libraries(unit_test_${name}  ${ARGN} ${Boost_LIBRARIES})
  endfunction()
  
  function(link_integration_test name)
    target_link_libraries(integration_test_${name}  ${ARGN} ${Boost_LIBRARIES})
  endfunction()
  
  function(register_unit_test name)
    register_test(${name} unit  ${ARGN})
  endfunction()
  
  function(register_integration_test name)
    register_test(${name} integration  ${ARGN})
  endfunction()
  
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
  
  register_unit_test(mediadirectory mediadirectorytest.cpp ${CMAKE_SOURCE_DIR}/src/media/mediadirectory.cpp ${CMAKE_SOURCE_DIR}/src/media/media.cpp )
  link_unit_test(mediadirectory ${Wt_DBO_LIBRARY} ${Wt_LIBRARIES} ${WIN32_LIBS})
  
  
  pkg_check_modules(AVUTIL libavutil REQUIRED)
  pkg_check_modules(AVFORMAT libavformat REQUIRED)
  pkg_check_modules(AVCODEC libavcodec REQUIRED)
  
  add_definitions(-DFFMPEG_MEDIA_INTEGRATION_TESTS_SAMPLES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data\" -DLOGGER_TESTING)
  register_integration_test(ffmpegmedia ffmpegmedia_integration_test.cpp  ${CMAKE_SOURCE_DIR}/src/media/media.cpp  ${CMAKE_SOURCE_DIR}/src/ffmpegmedia.cpp)
  link_integration_test(ffmpegmedia ${Wt_DBO_LIBRARY} ${Wt_LIBRARIES} ${WIN32_LIBS} ${AVUTIL_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVCODEC_LIBRARIES})
  
endif(Boost_UNIT_TEST_FRAMEWORK_LIBRARY)
  