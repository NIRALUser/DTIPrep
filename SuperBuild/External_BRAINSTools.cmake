
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED BRAINSTools_SOURCE_DIR AND NOT EXISTS ${BRAINSTools_SOURCE_DIR})
  message(FATAL_ERROR "BRAINSTools_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(BRAINSTools_DEPENDENCIES ${ITK_EXTERNAL_NAME} SlicerExecutionModel VTK DCMTK)

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(BRAINSTools)
set(proj BRAINSTools)

# Set CMake OSX variable to pass down the external project
set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
if(APPLE)
  list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

if(NOT DEFINED BRAINSTools_SOURCE_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")
  set(GIT_TAG "485b20f1cf5eb1af890c14c15af9b0aa092c04e8")  ## Update for Hitachi DWIConvert testing.

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  #message("VTK_DIR: ${VTK_DIR}")
  #message("ITK_DIR: ${ITK_DIR}")
  #message("SlicerExecutionModel_DIR: ${SlicerExecutionModel_DIR}")
  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/BRAINSia/BRAINSTools.git"
    GIT_TAG "${GIT_TAG}"
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    ${cmakeversion_external_update} "${cmakeversion_external_update_value}"
    CMAKE_ARGS
      -Wno-dev
      --no-warn-unused-cli
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      ${COMMON_EXTERNAL_PROJECT_ARGS}
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DUSE_SYSTEM_ITK:BOOL=ON
      -DUSE_SYSTEM_VTK:BOOL=ON
      -DUSE_SYSTEM_DCMTK:BOOL=ON
      -DDCMTK_ofstd_INCLUDE_DIR:PATH=${DCMTK_ofstd_INCLUDE_DIR}
      -DDCMTK_dcmdata_INCLUDE_DIR:PATH=${DCMTK_dcmdata_INCLUDE_DIR}
      -DDCMTK_dcmimgle_INCLUDE_DIR:PATH=${DCMTK_dcmimgle_INCLUDE_DIR}
      -DUSE_SYSTEM_SlicerExecutionModel:BOOL=ON
      -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
      -DSuperBuild_BRAINSTools_USE_GIT_PROTOCOL=${${CMAKE_PROJECT_NAME}_USE_GIT_PROTOCOL}
      -DITK_DIR:PATH=${ITK_DIR}
      -DVTK_DIR:PATH=${VTK_DIR}
      -DDCMTK_DIR:PATH=${DCMTK_DIR}
      -DUSE_ANTS:BOOL=OFF
      -DUSE_AutoWorkup:BOOL=OFF
      -DUSE_BRAINSABC:BOOL=OFF
      -DUSE_BRAINSConstellationDetector:BOOL=OFF
      -DUSE_BRAINSContinuousClass:BOOL=OFF
      -DUSE_BRAINSCut:BOOL=OFF
      -DUSE_BRAINSDemonWarp:BOOL=OFF
      -DUSE_BRAINSFit:BOOL=ON
      -DUSE_BRAINSFitEZ:BOOL=OFF
      -DUSE_BRAINSImageConvert:BOOL=OFF
      -DUSE_BRAINSInitializedControlPoints:BOOL=OFF
      -DUSE_BRAINSLandmarkInitializer:BOOL=OFF
      -DUSE_BRAINSMultiModeSegment:BOOL=OFF
      -DUSE_BRAINSMush:BOOL=OFF
      -DUSE_BRAINSROIAuto:BOOL=ON
      -DUSE_BRAINSResample:BOOL=ON
      -DUSE_BRAINSSnapShotWriter:BOOL=OFF
      -DUSE_BRAINSSurfaceTools:BOOL=OFF
      -DUSE_BRAINSTransformConvert:BOOL=OFF
      -DUSE_DebugImageViewer:BOOL=OFF
      -DUSE_GTRACT:BOOL=ON
      -DUSE_ICCDEF:BOOL=OFF
      -DUSE_ImageCalculator:BOOL=ON
      ${${proj}_CMAKE_OPTIONS}
    INSTALL_COMMAND ""
    DEPENDS
      ${BRAINSTools_DEPENDENCIES}
    )
  set(BRAINSTools_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(BRAINSCommonLib_DIR    ${CMAKE_BINARY_DIR}/${proj}-build/BRAINSTools-build/BRAINSCommonLib)
else()
  # The project is provided using BRAINSTools_DIR, nevertheless since other project may depend on BRAINSTools,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${BRAINSTools_DEPENDENCIES}")
endif()
