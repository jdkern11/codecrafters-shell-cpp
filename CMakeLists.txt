cmake_minimum_required(VERSION 3.13)

project(shell-starter-cpp)

list(APPEND CMAKE_PREFIX_PATH spdlog/build)
find_package(spdlog REQUIRED)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

add_executable(shell ${SOURCE_FILES})

target_link_libraries(shell PRIVATE spdlog::spdlog)
target_link_libraries(shell PRIVATE readline)
