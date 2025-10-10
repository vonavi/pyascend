set(CCEC_PATH          "${ASCENDC_DEVKIT_PATH}/ccec_compiler/bin")
set(CMAKE_C_COMPILER   "${CCEC_PATH}/bisheng")
set(CMAKE_CXX_COMPILER "${CCEC_PATH}/bisheng")

add_library(aiv_obj OBJECT add_custom.cpp)
target_compile_options(aiv_obj PRIVATE
  -x cce -std=c++17 --cce-aicore-only --cce-aicore-arch=dav-c220-vec
)
target_include_directories(aiv_obj PRIVATE
  "${ASCENDC_DEVKIT_PATH}/ascendc/include/basic_api"
  "${ASCENDC_DEVKIT_PATH}/ascendc/include/basic_api/impl"
  "${ASCENDC_DEVKIT_PATH}/ascendc/include/basic_api/interface"
  "${ASCENDC_DEVKIT_PATH}/ascendc/include/highlevel_api"
)
