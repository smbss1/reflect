cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 14)

project(Reflection VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
# set(REFLECT_EXAMPLE 1)
# set(REFL_JSON 1)

add_subdirectory(refl)

if (REFLECT_EXAMPLE)
    add_subdirectory(sandbox)
endif()