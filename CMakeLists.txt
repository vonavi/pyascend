cmake_minimum_required(VERSION 3.15)
project(pyascend LANGUAGES CXX)

set(CCEC_PATH          "${ASCENDC_DEVKIT_PATH}/ccec_compiler/bin")
set(CMAKE_C_COMPILER   "${CCEC_PATH}/bisheng")
set(CMAKE_CXX_COMPILER "${CCEC_PATH}/bisheng")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(cpp)

option(ENABLE_TESTS "Build tests" ON)
if(ENABLE_TESTS)
  add_subdirectory(tests)
endif()
