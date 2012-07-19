ExternalProject_Add(DTIReg
  SVN_REPOSITORY https://www.nitrc.org/svn/dtireg/trunk/DTI-Reg
  SVN_REVISION -r "18"
  SOURCE_DIR DTIReg
  BINARY_DIR DTIReg-build  
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  INSTALL_COMMAND ""
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    --no-warn-unused-cli # HACK Only expected variables should be passed down.
    ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
    ${COMMON_EXTERNAL_PROJECT_ARGS}
    # -DOPT_USE_SYSTEM_ITK:BOOL=ON
    # -DOPT_USE_SYSTEM_SlicerExecutionModel:BOOL=ON
    -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
    # -DOPT_USE_SYSTEM_BatchMake:BOOL=ON
    -DBatchMake_DIR:PATH=${BatchMake_DIR}
    -DDTIReg_ADDITIONAL_LINK_DIRS:PATH=${CMAKE_CURRENT_BINARY_DIR}/lib
    DEPENDS ${ITK_EXTERNAL_NAME} BatchMake
  )

## Force rebuilding of the main subproject every time building from super structure
ExternalProject_Add_Step(DTIReg forcebuild
    COMMAND ${CMAKE_COMMAND} -E remove
    ${CMAKE_CURRENT_BUILD_DIR}/DTIReg-prefix/src/DTIReg-stamp/DTIReg-build
    DEPENDEES configure
    DEPENDERS build
    ALWAYS 1
  )
