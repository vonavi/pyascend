cmake_minimum_required(VERSION 3.15)
project(pyascend LANGUAGES CXX)

find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(_ascend
  cpp/ascend.cpp
  cpp/bindings.cpp
)
target_compile_features(_ascend PRIVATE cxx_std_17)
target_include_directories(_ascend PRIVATE "${CMAKE_SOURCE_DIR}/include")

install(TARGETS _ascend DESTINATION ${PROJECT_NAME})
