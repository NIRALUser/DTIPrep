
include(${CMAKE_CURRENT_SOURCE_DIR}/Common.cmake)

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

#-----------------------------------------------------------------------------
enable_testing()
include(CTest)

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


#-----------------------------------------------------------------------------
# Add module sub-directory if USE_<MODULENAME> is both defined and true
#-----------------------------------------------------------------------------
add_subdirectory(src)

