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

  register_unit_test(mediadirectory mediadirectorytest.cpp ${CMAKE_SOURCE_DIR}/src/media/mediadirectory.cpp ${CMAKE_SOURCE_DIR}/src/media/media.cpp )
  link_unit_test(mediadirectory ${Wt_DBO_LIBRARY} ${Wt_LIBRARIES} ${WIN32_LIBS})
  
  add_definitions(-DTHREADPOOL_TEST=1)
  register_unit_test(threadpool threadpool_test.cpp ${CMAKE_SOURCE_DIR}/src/threadpool.cpp)
  link_unit_test(threadpool)
#   
#   pkg_check_modules(AVUTIL libavutil REQUIRED)
#   pkg_check_modules(AVFORMAT libavformat REQUIRED)
#   pkg_check_modules(AVCODEC libavcodec REQUIRED)
#   
#   add_definitions(-DFFMPEG_MEDIA_INTEGRATION_TESTS_SAMPLES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data\" -DLOGGER_TESTING)
#   register_integration_test(ffmpegmedia 
#     ffmpegmedia_integration_test.cpp
#     ${CMAKE_SOURCE_DIR}/src/media/media.cpp
#     ${CMAKE_SOURCE_DIR}/src/ffmpegmedia.cpp
#     ${CMAKE_SOURCE_DIR}/src/utils/concatstrings.cpp
#   )
#   link_integration_test(ffmpegmedia ${Wt_DBO_LIBRARY} ${Wt_LIBRARIES} ${WIN32_LIBS} ${AVUTIL_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVCODEC_LIBRARIES})
#   
endif(Boost_UNIT_TEST_FRAMEWORK_LIBRARY)
  
