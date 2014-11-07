cmake_minimum_required (VERSION 2.8)
project(idificator_unit_tests)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

#enable c++ 11
SET(CMAKE_CXX_FLAGS "-std=c++11 -g ${CMAKE_CXX_FLAGS}")

#add gtest
if (NOT TARGET gtest)
add_subdirectory(gtest-1.7.0)
endif()
include_directories(${gtest_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR})
#and protobuf
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

#the actual source directory
include_directories(..)

#add files with new test here
set (pb_src
pgmemo.pb.cc
)
set (test_sources
testmemo.cpp
)

add_executable(memotests ${test_sources} ${pb_src})
target_link_libraries(memotests gtest_main zmqcpp ${PROTOBUF_LIBRARY})

#add_custom_command(TARGET tests 
#                   POST_BUILD
#                   COMMAND ../server & tests/tests; killall server
#                   )
                   
foreach (pb ${pb_src})
add_custom_command(OUTPUT ${pb}
                   COMMAND protoc ${CMAKE_SOURCE_DIR}/`basename ${pb} .pb.cc`.proto --cpp_out=${CMAKE_CURRENT_BINARY_DIR} --proto_path=${CMAKE_SOURCE_DIR}
                   WORKING_DIRECTORY )
endforeach(pb)