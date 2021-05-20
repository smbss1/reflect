cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 14)

file(GLOB_RECURSE CORE_SRCS "*.cpp")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_library(reflect STATIC ${CORE_SRCS} )
target_include_directories(reflect PUBLIC "include")