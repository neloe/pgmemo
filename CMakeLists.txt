cmake_minimum_required (VERSION 2.8)
project(pgmemo)

#enable c++ 11
SET(CMAKE_CXX_FLAGS "-std=c++11 -g ${CMAKE_CXX_FLAGS}")
add_subdirectory(bson)
#Protocol buffers
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/bson)

set ( pgmemo_src
      pgmemo.cpp
      pgmemo_util.cpp
      pgmemo.pb.cc
)

add_executable(pgmemo ${pgmemo_src})

target_link_libraries(pgmemo fastbson ${PROTOBUF_LIBRARY} pqxx hiredis)

add_custom_command( OUTPUT pgmemo.pb.cc
                    COMMAND protoc pgmemo.proto --cpp_out=${CMAKE_BINARY_DIR}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})