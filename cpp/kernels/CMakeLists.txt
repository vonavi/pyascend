add_library(aiv_obj OBJECT add_custom.cpp)
target_compile_options(aiv_obj PRIVATE
  -x cce --cce-aicore-only --cce-aicore-arch=dav-c220-vec
)
target_include_directories(aiv_obj PRIVATE
  "${ASCENDC_DEVKIT_PATH}/ascendc/include/basic_api"
  "${ASCENDC_DEVKIT_PATH}/ascendc/include/basic_api/impl"
  "${ASCENDC_DEVKIT_PATH}/ascendc/include/basic_api/interface"
  "${ASCENDC_DEVKIT_PATH}/ascendc/include/highlevel_api"
)

install(FILES
  $<TARGET_OBJECTS:aiv_obj>
  DESTINATION ${PROJECT_NAME}/tests
  RENAME add_custom.o
)
