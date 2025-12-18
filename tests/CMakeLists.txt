cmake_minimum_required(VERSION 3.13)

project(shell-tests)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
list(APPEND CMAKE_PREFIX_PATH ../spdlog/build)
find_package(spdlog CONFIG REQUIRED)

set(SOURCE_FILES
  ../src/utils.cpp
  ../src/trie.cpp
)

find_package(Catch2 2 REQUIRED)
file(GLOB TEST_FILES "*.cpp")
add_executable(tests ${TEST_FILES} ${SOURCE_FILES})
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_include_directories(tests PRIVATE ../src)
target_link_libraries(tests PRIVATE spdlog::spdlog)
