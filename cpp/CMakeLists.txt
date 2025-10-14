find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(_ascend
  ascend.cpp
  bindings.cpp
)
target_compile_features(_ascend PRIVATE cxx_std_17)
target_include_directories(_ascend PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(_ascend PRIVATE stdc++)

install(TARGETS _ascend DESTINATION ${PROJECT_NAME})
