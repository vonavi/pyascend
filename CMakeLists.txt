cmake_minimum_required(VERSION 3.15)
project(pyascend LANGUAGES CXX)

add_subdirectory(cpp)

option(ENABLE_TESTS "Build tests" ON)
if(ENABLE_TESTS)
  add_subdirectory(tests)
endif()
