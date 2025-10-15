find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(_ascend
  ascend.cpp
  bindings.cpp
)
target_include_directories(_ascend PRIVATE
  "${ASCENDC_DEVKIT_PATH}/include"
  "${ASCENDC_DEVKIT_PATH}/include/experiment/msprof"
  "${ASCENDC_DEVKIT_PATH}/include/experiment/runtime"
  "${CMAKE_SOURCE_DIR}/include"
)
target_link_directories(_ascend PRIVATE
  "${ASCENDC_DEVKIT_PATH}/lib64"
)
target_link_libraries(_ascend PRIVATE
  stdc++ ascendcl runtime
)

install(TARGETS _ascend DESTINATION ${PROJECT_NAME})
