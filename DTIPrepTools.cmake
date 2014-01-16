
macro(INSTALL_EXECUTABLE)
  set(options )
  set( oneValueArgs OUTPUT_DIR )
  set(multiValueArgs LIST_EXEC )
  CMAKE_PARSE_ARGUMENTS(LOCAL
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )
  foreach( tool ${LOCAL_LIST_EXEC})
    install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${tool} DESTINATION ${LOCAL_OUTPUT_DIR} )
  endforeach()
endmacro()


include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)

set(MODULE_NAME ${EXTENSION_NAME}) # Do not use 'project()'
set(MODULE_TITLE ${MODULE_NAME})

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

#-----------------------------------------------------------------------------
# Update CMake module path
#------------------------------------------------------------------------------
set(CMAKE_MODULE_PATH
  ${${PROJECT_NAME}_SOURCE_DIR}/CMake
  ${${PROJECT_NAME}_BINARY_DIR}/CMake
  ${CMAKE_MODULE_PATH}
  )

#-----------------------------------------------------------------------------
set(expected_ITK_VERSION_MAJOR ${ITK_VERSION_MAJOR})
find_package(ITK NO_MODULE REQUIRED)
if(${ITK_VERSION_MAJOR} VERSION_LESS ${expected_ITK_VERSION_MAJOR})
  # Note: Since ITKv3 doesn't include a ITKConfigVersion.cmake file, let's check the version
  #       explicitly instead of passing the version as an argument to find_package() command.
  message(FATAL_ERROR "Could not find a configuration file for package \"ITK\" that is compatible "
                      "with requested version \"${expected_ITK_VERSION_MAJOR}\".\n"
                      "The following configuration files were considered but not accepted:\n"
                      "  ${ITK_CONFIG}, version: ${ITK_VERSION_MAJOR}.${ITK_VERSION_MINOR}.${ITK_VERSION_PATCH}\n")
endif()

include(${ITK_USE_FILE})

#-----------------------------------------------------------------------------
find_package(VTK NO_MODULE REQUIRED)
include(${VTK_USE_FILE})

#-----------------------------------------------------------------------------
find_package(SlicerExecutionModel NO_MODULE REQUIRED GenerateCLP)
include(${GenerateCLP_USE_FILE})
include(${SlicerExecutionModel_USE_FILE})
include(${SlicerExecutionModel_CMAKE_DIR}/SEMMacroBuildCLI.cmake)

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

if(USE_ANTS)
  # find ANTS includes
  # HACK:
  #set( BOOST_INCLUDE_DIR  /Shared/sinapse/sharedopt/20130601/RHEL6/DTIPrep/BRAINSTools-build/Boost)
  #set( ANTS_SOURCE_DIR    /Shared/sinapse/sharedopt/20130601/RHEL6/DTIPrep/BRAINSTools-build/ANTS )
  #message( STATUS "XXXXXXXXXXXXX ${BOOST_INCLUDE_DIR} XXXXXXX")
  include_directories(${BOOST_INCLUDE_DIR})
  include_directories(${ANTs_SOURCE_DIR}/Temporary)
  include_directories(${ANTs_SOURCE_DIR}/Tensor)
  include_directories(${ANTs_SOURCE_DIR}/Utilities)
  include_directories(${ANTs_SOURCE_DIR}/Examples)
  include_directories(${ANTs_SOURCE_DIR}/ImageRegistration)
#  link_directories(${BRAINSTools_LIBRARY_PATH} ${BRAINSTools_CLI_ARCHIVE_OUTPUT_DIRECTORY})
  link_directories(${ANTs_LIBRARY_DIR} ${BRAINSTools_LIBRARY_PATH})
  set(ANTs_LIBS antsUtilities)
endif()


#-----------------------------------------------------------------------------
# Add module sub-directory if USE_<MODULENAME> is both defined and true
#-----------------------------------------------------------------------------
add_subdirectory(src)

if( EXTENSION_SUPERBUILD_BINARY_DIR )
  unsetForSlicer( NAMES SlicerExecutionModel_DIR ITK_DIR VTK_DIR CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_CXX_FLAGS CMAKE_C_FLAGS ITK_LIBRARIES )
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
  resetForSlicer( NAMES ITK_DIR SlicerExecutionModel_DIR CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_CXX_FLAGS CMAKE_C_FLAGS ITK_LIBRARIES )
endif()

IF(BUILD_TESTING)
  include(CTest)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)

if(USE_DTIProcess)
  set( NotCLIToolsList
    dtiestim
    dtiprocess
  )
endif()
if(USE_NIRALUtilities)
  list(APPEND NotCLIToolsList
    ImageMath
    convertITKformats
  )
endif()

if( EXTENSION_SUPERBUILD_BINARY_DIR )
  set(HIDDEN_CLI_INSTALL_DIR ${SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION}/../hidden-cli-modules )
  if(APPLE) # On mac, Ext/cli_modules/DTIAtlasBuilder so Ext/ExternalBin is ../ExternalBin
    set(NOCLI_INSTALL_DIR ${SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION}/../ExternalBin)
  else() # On Windows : idem Linux : Ext/lib/Slicer4.2/cli_modules/DTIAtlasBuilder so Ext/ExternalBin is ../../../ExternalBin
    set(NOCLI_INSTALL_DIR ${SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION}/../../../ExternalBin)
  endif()
  set( CLIToolsList
    DTIPrepLauncher
     )
  INSTALL_EXECUTABLE( OUTPUT_DIR ${SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION} LIST_EXEC ${CLIToolsList} )
  set( hiddenCLIToolsList
    DTIPrep
     )
  INSTALL_EXECUTABLE( OUTPUT_DIR ${HIDDEN_CLI_INSTALL_DIR} LIST_EXEC ${hiddenCLIToolsList} )
  INSTALL_EXECUTABLE( OUTPUT_DIR ${NOCLI_INSTALL_DIR} LIST_EXEC ${NotCLIToolsList} )
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
  include(${Slicer_EXTENSION_CPACK})
endif()
if( SUPERBUILD_NOT_EXTENSION )
  INSTALL_EXECUTABLE( OUTPUT_DIR . LIST_EXEC DTIPrep )
  INSTALL_EXECUTABLE( OUTPUT_DIR . LIST_EXEC ${NotCLIToolsList} )
endif()
