cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 14)

file(GLOB_RECURSE CORE_SRCS "*.cpp")

add_library(refl STATIC ${CORE_SRCS} )
target_include_directories(refl PUBLIC "include")

if (REFL_JSON)
    find_package(foxjson REQUIRED)
    target_include_directories(refl PUBLIC "optionnal")
    target_link_libraries(refl fox_json)
endif()