cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE SANDBOX_SRCS "*.cpp")

find_package(foxjson REQUIRED)

add_executable(app ${SANDBOX_SRCS})
target_include_directories(app PRIVATE "include")
target_link_libraries(app PRIVATE fox_json reflect)