set(MODULE_NAME ${EXTENSION_NAME}) # Do not use 'project()'
set(MODULE_TITLE ${MODULE_NAME})

macro(INSTALL_EXECUTABLE)
  set(options )
  set( oneValueArgs OUTPUT_DIR )
  set(multiValueArgs LIST_EXEC PATHS)
  CMAKE_PARSE_ARGUMENTS(LOCAL
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )
  foreach( tool ${LOCAL_LIST_EXEC})
     find_program( programPATH NAMES ${tool} PATHS ${LOCAL_PATHS} NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_PATH NO_CMAKE_SYSTEM_PATH )
     install(PROGRAMS ${programPATH} DESTINATION ${LOCAL_OUTPUT_DIR} )
     unset( programPATH CACHE )
  endforeach()
endmacro()

#-----------------------------------------------------------------------------
# Update CMake module path
#------------------------------------------------------------------------------
set(CMAKE_MODULE_PATH
  ${${PROJECT_NAME}_SOURCE_DIR}/CMake
  ${${PROJECT_NAME}_BINARY_DIR}/CMake
  ${CMAKE_MODULE_PATH}
  )

#-----------------------------------------------------------------------------
find_package(ITK 4 NO_MODULE REQUIRED)

include(${ITK_USE_FILE})

#-----------------------------------------------------------------------------
set(VTK_FOUND OFF)
find_package(VTK COMPONENTS
      vtkCommonSystem
      vtkCommonCore
      vtkCommonSystem
      vtkCommonMath
      vtkCommonMisc
      vtkCommonTransforms
      vtkIOLegacy
      vtkIOXML
      vtkGUISupportQt
      vtkInteractionImage
      REQUIRED)
if(VTK_USE_FILE)
  include(${VTK_USE_FILE})
endif()


#-----------------------------------------------------------------------------
find_package(SlicerExecutionModel NO_MODULE REQUIRED GenerateCLP)
include(${GenerateCLP_USE_FILE})
include(${SlicerExecutionModel_USE_FILE})
include(${SlicerExecutionModel_CMAKE_DIR}/SEMMacroBuildCLI.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)

if(USE_ANTS)
  # find ANTS includes
  message("ANTs_SOURCE_DIR=${ANTs_SOURCE_DIR}")
  include_directories(${BOOST_INCLUDE_DIR})
  include_directories(${ANTs_SOURCE_DIR}/Temporary)
  include_directories(${ANTs_SOURCE_DIR}/Tensor)
  include_directories(${ANTs_SOURCE_DIR}/Utilities)
  include_directories(${ANTs_SOURCE_DIR}/Examples)
  include_directories(${ANTs_SOURCE_DIR}/ImageRegistration)
  link_directories(${BRAINSTools_LIBRARY_PATH} ${BRAINSTools_CLI_ARCHIVE_OUTPUT_DIRECTORY} ${ANTs_LIBRARY_DIR})
  set(ANTS_LIBS antsUtilities)
endif()

message("BRAINSCommonLib_DIR=${BRAINSCommonLib_DIR}")

find_package(BRAINSCommonLib NO_MODULE REQUIRED)
include(${BRAINSCommonLib_USE_FILE})
include_directories(${BRAINSCommonLib_INCLUDE_DIRS})
link_directories(${BRAINSCommonLib_LIBRARY_DIRS})


#find_package(MultiImageRegistration NO_MODULE REQUIRED)
#include(${MultiImageRegistration_USE_FILE})

# Use the include path and library for Qt that is used by VTK.
#include_directories(
#  ${CMAKE_CURRENT_BINARY_DIR}
#  ${CMAKE_CURRENT_SOURCE_DIR}
#  ${VTK_INCLUDE_DIR}
#  ${ITK_INCLUDE_DIR}
#  ${QT_INCLUDE_DIR}
#)

# No need to add with nothing to compile. add_subdirectory(BRAINSFit_Common)
##HACK:
#set(BRAINSFit_SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/BRAINSFit)
#include_directories(${BRAINSFit_SOURCE_DIR})
#include_directories(${BRAINSFit_SOURCE_DIR}/BRAINSFit_Common)
#include_directories(${BRAINSFit_SOURCE_DIR}/LargestForegroundFilledMaskImageFilter)
#include_directories(${BRAINSFit_SOURCE_DIR}/FindCenterOfBrainFilter)
#include_directories(${CMAKE_INSTALL_PREFIX}/MultiImageRegistration/Source)
#include_directories(${CMAKE_INSTALL_PREFIX}/MultiImageRegistration/Source/Common)

#-----------------------------------------------------------------------
# Setup locations to find externally maintained test data.
#-----------------------------------------------------------------------
list(APPEND ExternalData_URL_TEMPLATES
  # Local data store populated by the ITK pre-commit hook
  "file:///${${PROJECT_NAME}_SOURCE_DIR}/.ExternalData/%(algo)/%(hash)"
  # Data published by Iowa Psychiatry web interface
  ## This server is now obsolete "http://www.psychiatry.uiowa.edu/users/brainstestdata/ctestdata/%(algo)/%(hash)"
  ## The primary new home for data
  "http://slicer.kitware.com/midas3/api/rest?method=midas.bitstream.download&checksum=%(hash)"
  # Data published by MIDAS
  "http://midas.kitware.com/api/rest/midas.bitstream.by.hash?hash=%(hash)&algorithm=%(algo)"
  # Data published by developers using git-gerrit-push.
  "http://www.itk.org/files/ExternalData/%(algo)/%(hash)"
)

# Tell ExternalData commands to transform raw files to content links.
# TODO: Condition this feature on presence of our pre-commit hook.
#set(ExternalData_LINK_CONTENT MD5)
#set(ExternalData_SOURCE_ROOT ${${PROJECT_NAME}_SOURCE_DIR})
#include(ExternalData)

set(TestData_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TestData)

if( DTIPrep_BUILD_SLICER_EXTENSION )
  unsetForSlicer( NAMES SlicerExecutionModel_DIR ITK_DIR VTK_DIR CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_CXX_FLAGS CMAKE_C_FLAGS ITK_LIBRARIES )
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
  resetForSlicer( NAMES ITK_DIR SlicerExecutionModel_DIR CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_CXX_FLAGS CMAKE_C_FLAGS ITK_LIBRARIES )
endif()

#-----------------------------------------------------------------------------
# Add module sub-directory if USE_<MODULENAME> is both defined and true
#-----------------------------------------------------------------------------
add_subdirectory(src)

IF(BUILD_TESTING)
  include(CTest)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)

if(USE_DTIProcess)
  set( NotCLIToolsList
    dtiestim
    dtiprocess
  )
  list( APPEND ToolsPaths ${SUPERBUILD_BINARY_DIR}/DTIProcess-install/bin/ )
endif()
if(USE_NIRALUtilities)
  list(APPEND NotCLIToolsList
    ImageMath
    convertITKformats
  )
  list( APPEND ToolsPaths ${SUPERBUILD_BINARY_DIR}/niral_utilities-install/bin/ )
endif()

if( DTIPrep_BUILD_SLICER_EXTENSION )
  set(NOCLI_INSTALL_DIR ${Slicer_INSTALL_CLIMODULES_BIN_DIR}/../ExternalBin)
  INSTALL_EXECUTABLE( OUTPUT_DIR ${NOCLI_INSTALL_DIR} LIST_EXEC ${NotCLIToolsList} PATHS ${ToolsPaths} )
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
  include(${Slicer_EXTENSION_CPACK})
endif()
if( SUPERBUILD_NOT_EXTENSION )
  if( NOT APPLE )
    INSTALL_EXECUTABLE( OUTPUT_DIR bin LIST_EXEC ${NotCLIToolsList} PATHS ${ToolsPaths} )
  else()
    INSTALL_EXECUTABLE( OUTPUT_DIR ${CMAKE_INSTALL_PREFIX}/bin/DTIPrep.app/Contents/ExternalBin LIST_EXEC ${NotCLIToolsList} PATHS ${ToolsPaths} )
  endif()
endif()
