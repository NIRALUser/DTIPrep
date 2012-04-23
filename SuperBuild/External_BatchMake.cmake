if(OPT_USE_SYSTEM_BatchMake)
  find_package(BatchMake REQUIRED)
  include(${BatchMake_USE_FILE})
  set(BatchMake_DEPEND "")
  return()
endif(OPT_USE_SYSTEM_BatchMake)

set(proj BatchMake)
ExternalProject_Add(${proj}
  GIT_REPOSITORY "git://batchmake.org/BatchMake.git"
  GIT_TAG "1f5bf4f92e8678c34dc6f7558be5e6613804d988"
  SOURCE_DIR BatchMake
  BINARY_DIR BatchMake-build
  CMAKE_GENERATOR ${gen}
  DEPENDS  ${ITK_EXTERNAL_NAME}
  CMAKE_ARGS
  --no-warn-unused-cli
  ${COMMON_EXTERNAL_PROJECT_ARGS}
  -DUSE_FLTK:BOOL=OFF
  -DGRID_SUPPORT:BOOL=ON
  -DUSE_SPLASHSCREEN:BOOL=OFF
  INSTALL_COMMAND ""
  )

set(BatchMake_DEPEND BatchMake )
set(BatchMake_DIR  ${CMAKE_CURRENT_BINARY_DIR}/BatchMake-build)
