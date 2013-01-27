cmake_minimum_required(VERSION 2.8)
project(videostreaming)
add_definitions(-std=gnu++11)

find_package(Wt REQUIRED)
include_directories(${Wt_INCLUDE_DIR}  )

add_executable(videostreaming streamingapp.cpp main.cpp)

target_link_libraries(videostreaming ${Wt_DEBUG_LIBRARIES} -lboost_signals -lboost_system -lboost_filesystem )
install(TARGETS videostreaming RUNTIME DESTINATION bin)
