cmake_minimum_required(VERSION 3.13)

project(shell-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

add_executable(shell ${SOURCE_FILES})

target_link_libraries(shell PRIVATE readline)

# Fast version
# list(APPEND CMAKE_PREFIX_PATH spdlog/include)
# find_package(spdlog REQUIRED)
#  target_link_libraries(shell PRIVATE spdlog::spdlog)

# Slow version
add_subdirectory(spdlog)
target_link_libraries(shell PRIVATE spdlog::spdlog_header_only)

