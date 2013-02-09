
set(DTIProcess_DEPENDS ${ITK_EXTERNAL_NAME} VTK SlicerExecutionModel Boost)

ExternalProject_Add(DTIProcess
  SVN_REPOSITORY "https://www.nitrc.org/svn/dtiprocess/trunk"
  SVN_REVISION -r "151"
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  SOURCE_DIR DTIProcess
  BINARY_DIR DTIProcess-build
  DEPENDS ${DTIProcess_DEPENDS}
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
  ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
  ${COMMON_EXTERNAL_PROJECT_ARGS}
  -DBOOST_ROOT:PATH=${BOOST_ROOT}
  -DBOOST_INCLUDE_DIR:PATH=${BOOST_INCLUDE_DIR}
  -DBOOST_INCLUDEDIR:PATH=${BOOST_INCLUDE_DIR}
  -DBUILD_dwiAtlas:BOOL=ON
  ${cmakeversion_external_update} "${cmakeversion_external_update_value}"
  INSTALL_COMMAND ""
  )

