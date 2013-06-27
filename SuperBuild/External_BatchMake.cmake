if(OPT_USE_SYSTEM_BatchMake)
  find_package(BatchMake REQUIRED)
  include(${BatchMake_USE_FILE})
  set(BatchMake_DEPEND "")
  return()
endif()

set(proj BatchMake)
if(NOT DEFINED git_protocol)
  set(git_protocol "git")
endif()

#message(STATUS "${__indent}Adding project ${proj}")
ExternalProject_Add(${proj}
  GIT_REPOSITORY "${git_protocol}://batchmake.org/BatchMake.git"
  GIT_TAG "1f5bf4f92e8678c34dc6f7558be5e6613804d988"
  SOURCE_DIR BatchMake
  BINARY_DIR BatchMake-build
  ${cmakeversion_external_update} "${cmakeversion_external_update_value}"
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
  --no-warn-unused-cli
  ${COMMON_EXTERNAL_PROJECT_ARGS}
  -DUSE_FLTK:BOOL=OFF
  -DGRID_SUPPORT:BOOL=ON
  -DUSE_SPLASHSCREEN:BOOL=OFF
  INSTALL_COMMAND ""
  DEPENDS
    ITKv4
  )

set(BatchMake_DEPEND BatchMake )
set(BatchMake_DIR  ${CMAKE_CURRENT_BINARY_DIR}/BatchMake-build)
