ExternalProject_Add(FVLight
  SVN_REPOSITORY https://www.nitrc.org/svn/fvlight/trunk
  SOURCE_DIR FVLight
  BINARY_DIR FVLight-build
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  ${cmakeversion_external_update} "${cmakeversion_external_update_value}"
  INSTALL_COMMAND ""
  PATCH_COMMAND ${CMAKE_COMMAND}
    -Dfixfile=${CMAKE_CURRENT_BINARY_DIR}/FVLight/CMakeLists.txt
    -P ${CMAKE_CURRENT_LIST_DIR}/FVLightFix.cmake
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    --no-warn-unused-cli # HACK Only expected variables should be passed down.
    ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
    ${COMMON_EXTERNAL_PROJECT_ARGS}
    -DUSE_SYSTEM_SlicerExecutionModel:BOOL=ON
    -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
    -DQWT_LIBRARY:PATH=${QWT_LIBRARY}
    -DQWT_INCLUDE_DIR:PATH=${QWT_INCLUDE_DIR}
    DEPENDS ${ITK_EXTERNAL_NAME} VTK QWT
  )

## Force rebuilding of the main subproject every time building from super structure
ExternalProject_Add_Step(FVLight forcebuild
    COMMAND ${CMAKE_COMMAND} -E remove
    ${CMAKE_CURRENT_BUILD_DIR}/FVLight-prefix/src/FVLight-stamp/FVLight-build
    DEPENDEES configure
    DEPENDERS build
    ALWAYS 1
  )
