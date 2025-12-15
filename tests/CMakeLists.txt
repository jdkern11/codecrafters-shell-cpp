cmake_minimum_required(VERSION 3.13)

project(shell-tests)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

set(SOURCE_FILES
  ../src/utils.cpp
)

find_package(Catch2 2 REQUIRED)
add_executable(tests ./test_utils.cpp ${SOURCE_FILES})
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_include_directories(tests PRIVATE ../src)
