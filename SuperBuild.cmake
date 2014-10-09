
#-----------------------------------------------------------------------------
set(verbose FALSE)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
enable_language(C)
enable_language(CXX)

#-----------------------------------------------------------------------------
enable_testing()
include(CTest)

#-----------------------------------------------------------------------------
include(${CMAKE_CURRENT_SOURCE_DIR}/Common.cmake)
#-----------------------------------------------------------------------------
#If it is build as an extension
#-----------------------------------------------------------------------------
if( DTIPrep_BUILD_SLICER_EXTENSION )
  set( EXTERNAL_SOURCE_IN_BINARY_DIR ON)
  set( USE_SYSTEM_VTK ON CACHE BOOL "Use system VTK" FORCE )
  #VTK_VERSION_MAJOR is define but not a CACHE variable
  set( VTK_VERSION_MAJOR ${VTK_VERSION_MAJOR} CACHE STRING "Choose the expected VTK major version to build Slicer (5 or 6).")
  set( USE_SYSTEM_DCMTK ON CACHE BOOL "Use system DCMTK" FORCE )
  set( USE_SYSTEM_Teem ON CACHE BOOL "Use system Teem" FORCE )
  set( BUILD_SHARED_LIBS OFF CACHE BOOL "Use shared libraries" FORCE)
  unsetForSlicer(NAMES CMAKE_MODULE_PATH CMAKE_C_COMPILER CMAKE_CXX_COMPILER DCMTK_DIR ITK_DIR SlicerExecutionModel_DIR VTK_DIR QT_QMAKE_EXECUTABLE ITK_VERSION_MAJOR CMAKE_CXX_FLAGS CMAKE_C_FLAGS Teem_DIR)
  find_package(Slicer REQUIRED)
  unsetAllForSlicerBut( NAMES VTK_DIR QT_QMAKE_EXECUTABLE DCMTK_DIR Teem_DIR )
  resetForSlicer(NAMES CMAKE_MODULE_PATH CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_CXX_FLAGS CMAKE_C_FLAGS ITK_DIR SlicerExecutionModel_DIR  ITK_VERSION_MAJOR )
  if( APPLE )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,@loader_path/../../../../../")
  endif()
else()
  set( USE_ITK_Module_MGHIO ON )
  set( SUPERBUILD_NOT_EXTENSION TRUE )
  option(USE_DTIProcess "Build DTIProcess" ON)
endif()
option(USE_NIRALUtilities "Build NIRALUtilities" ON)


if( EXTERNAL_SOURCE_IN_BINARY_DIR )
  set( EXTERNAL_SOURCE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
endif()

#-----------------------------------------------------------------------------
# Git protocole option
#-----------------------------------------------------------------------------
option(${CMAKE_PROJECT_NAME}_USE_GIT_PROTOCOL "If behind a firewall turn this off to use http instead." ON)
set(git_protocol "git")
if(NOT ${CMAKE_PROJECT_NAME}_USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()

find_package(Git REQUIRED)

# I don't know who removed the Find_Package for QT, but it needs to be here
# in order to build VTK if ${LOCAL_PROJECT_NAME}_USE_QT is set.
if(${LOCAL_PROJECT_NAME}_USE_QT AND NOT DTIPrep_BUILD_SLICER_EXTENSION)
    find_package(Qt4 REQUIRED)
endif()

#-----------------------------------------------------------------------------
# Enable and setup External project global properties
#-----------------------------------------------------------------------------
include(ExternalProject)
include(SlicerMacroEmptyExternalProject)
include(SlicerMacroCheckExternalProjectDependency)

# Compute -G arg for configuring external projects with the same CMake generator:
if(CMAKE_EXTRA_GENERATOR)
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()


# With CMake 2.8.9 or later, the UPDATE_COMMAND is required for updates to occur.
# For earlier versions, we nullify the update state to prevent updates and
# undesirable rebuild.
option(FORCE_EXTERNAL_BUILDS "Force rebuilding of external project (if they are updated)" OFF)
if(CMAKE_VERSION VERSION_LESS 2.8.9 OR NOT FORCE_EXTERNAL_BUILDS)
  set(cmakeversion_external_update UPDATE_COMMAND)
  set(cmakeversion_external_update_value "" )
else()
  set(cmakeversion_external_update LOG_UPDATE )
  set(cmakeversion_external_update_value 1)
endif()

#-----------------------------------------------------------------------------
# Platform check
#-----------------------------------------------------------------------------

set(PLATFORM_CHECK true)

if(PLATFORM_CHECK)
  # See CMake/Modules/Platform/Darwin.cmake)
  #   6.x == Mac OSX 10.2 (Jaguar)
  #   7.x == Mac OSX 10.3 (Panther)
  #   8.x == Mac OSX 10.4 (Tiger)
  #   9.x == Mac OSX 10.5 (Leopard)
  #  10.x == Mac OSX 10.6 (Snow Leopard)
  if (DARWIN_MAJOR_VERSION LESS "9")
    message(FATAL_ERROR "Only Mac OSX >= 10.5 are supported !")
  endif()
endif()

#-----------------------------------------------------------------------------
# Superbuild option(s)
#-----------------------------------------------------------------------------
option(BUILD_STYLE_UTILS "Build uncrustify, cppcheck, & KWStyle" OFF)
CMAKE_DEPENDENT_OPTION(
  USE_SYSTEM_Uncrustify "Use system Uncrustify program" OFF
  "BUILD_STYLE_UTILS" OFF
  )
CMAKE_DEPENDENT_OPTION(
  USE_SYSTEM_KWStyle "Use system KWStyle program" OFF
  "BUILD_STYLE_UTILS" OFF
  )
CMAKE_DEPENDENT_OPTION(
  USE_SYSTEM_Cppcheck "Use system Cppcheck program" OFF
  "BUILD_STYLE_UTILS" OFF
  )

set(EXTERNAL_PROJECT_BUILD_TYPE "Release" CACHE STRING "Default build type for support libraries")

option(USE_SYSTEM_ITK "Build using an externally defined version of ITK" OFF)
option(USE_SYSTEM_SlicerExecutionModel "Build using an externally defined version of SlicerExecutionModel"  OFF)
option(USE_SYSTEM_VTK "Build using an externally defined version of VTK" OFF)
option(USE_SYSTEM_DCMTK "Build using an externally defined version of DCMTK" OFF)
option(USE_SYSTEM_Teem "Build using external Teem" OFF)
option(USE_SYSTEM_zlib "Build using external zlib" OFF)
option(USE_ANTs "Build BRAINSTools with ANTs" OFF)
option(${PROJECT_NAME}_BUILD_FFTW_SUPPORT "Build external FFTW" OFF)
#option(${PROJECT_NAME}_BUILD_DICOM_SUPPORT "Build Dicom Support" ON)

#------------------------------------------------------------------------------
# ${LOCAL_PROJECT_NAME} dependency list
#------------------------------------------------------------------------------
set( ${LOCAL_PROJECT_NAME}_DEPENDENCIES DCMTK ITKv4 SlicerExecutionModel VTK BRAINSTools )
set( ${PROJECT_NAME}_BUILD_DICOM_SUPPORT ON )
set( ${PROJECT_NAME}_BUILD_ZLIB_SUPPORT ON )
if( UNIX )
  set( ${PROJECT_NAME}_BUILD_TIFF_SUPPORT ON )
  set( ${PROJECT_NAME}_BUILD_JPEG_SUPPORT ON )
endif()

if(BUILD_STYLE_UTILS)
  list(APPEND ${LOCAL_PROJECT_NAME}_DEPENDENCIES Cppcheck KWStyle Uncrustify)
endif()


#-----------------------------------------------------------------------------
# Define Superbuild global variables
#-----------------------------------------------------------------------------

# This variable will contain the list of CMake variable specific to each external project
# that should passed to ${CMAKE_PROJECT_NAME}.
# The item of this list should have the following form: <EP_VAR>:<TYPE>
# where '<EP_VAR>' is an external project variable and TYPE is either BOOL, STRING, PATH or FILEPATH.
# TODO Variable appended to this list will be automatically exported in ${LOCAL_PROJECT_NAME}Config.cmake,
# prefix '${LOCAL_PROJECT_NAME}_' will be prepended if it applies.
set(${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS)

# The macro '_expand_external_project_vars' can be used to expand the list of <EP_VAR>.
set(${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS) # List of CMake args to configure BRAINS
set(${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES) # List of CMake variable names

# Convenient macro allowing to expand the list of EP_VAR listed in ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS
# The expanded arguments will be appended to the list ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS
# Similarly the name of the EP_VARs will be appended to the list ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES.
macro(_expand_external_project_vars)
  set(${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS "")
  set(${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES "")
  foreach(arg ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS})
    string(REPLACE ":" ";" varname_and_vartype ${arg})
    set(target_info_list ${target_info_list})
    list(GET varname_and_vartype 0 _varname)
    list(GET varname_and_vartype 1 _vartype)
    list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS -D${_varname}:${_vartype}=${${_varname}})
    list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES ${_varname})
  endforeach()
endmacro()

#-----------------------------------------------------------------------------
# Common external projects CMake variables
#-----------------------------------------------------------------------------
list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS
  MAKECOMMAND:STRING
  CMAKE_SKIP_RPATH:BOOL
  CMAKE_BUILD_TYPE:STRING
  CMAKE_MODULE_PATH:PATH
  BUILD_SHARED_LIBS:BOOL
  CMAKE_CXX_COMPILER:PATH
  CMAKE_CXX_FLAGS_RELEASE:STRING
  CMAKE_CXX_FLAGS_DEBUG:STRING
  CMAKE_CXX_FLAGS:STRING
  CMAKE_C_COMPILER:PATH
  CMAKE_C_FLAGS_RELEASE:STRING
  CMAKE_C_FLAGS_DEBUG:STRING
  CMAKE_C_FLAGS:STRING
  CMAKE_SHARED_LINKER_FLAGS:STRING
  CMAKE_EXE_LINKER_FLAGS:STRING
  CMAKE_MODULE_LINKER_FLAGS:STRING
  CMAKE_GENERATOR:STRING
  CMAKE_EXTRA_GENERATOR:STRING
  CMAKE_INSTALL_PREFIX:PATH
  CMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH
  CMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH
  CMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH
  CMAKE_BUNDLE_OUTPUT_DIRECTORY:PATH
  CTEST_NEW_FORMAT:BOOL
  MEMORYCHECK_COMMAND_OPTIONS:STRING
  MEMORYCHECK_COMMAND:PATH
  SITE:STRING
  BUILDNAME:STRING
  ${PROJECT_NAME}_BUILD_DICOM_SUPPORT:BOOL
  USE_ANTS:BOOL
  ANTs_SOURCE_DIR:PATH
  ANTs_LIBRARY_DIR:PATH
  BOOST_INCLUDE_DIR:PATH
  BOOST_ROOT:PATH
  )

if(${LOCAL_PROJECT_NAME}_USE_QT)
  list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS
    ${LOCAL_PROJECT_NAME}_USE_QT:BOOL
    QT_QMAKE_EXECUTABLE:PATH
    QT_MOC_EXECUTABLE:PATH
    QT_UIC_EXECUTABLE:PATH
    )
endif()

_expand_external_project_vars()
set(COMMON_EXTERNAL_PROJECT_ARGS ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})
set(extProjName ${LOCAL_PROJECT_NAME})
set(proj        ${LOCAL_PROJECT_NAME})
SlicerMacroCheckExternalProjectDependency(${proj})

#-----------------------------------------------------------------------------
# Set CMake OSX variable to pass down the external project
#-----------------------------------------------------------------------------
set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
if(APPLE)
  list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

set(${LOCAL_PROJECT_NAME}_CLI_RUNTIME_DESTINATION  bin)
set(${LOCAL_PROJECT_NAME}_CLI_LIBRARY_DESTINATION  lib)
set(${LOCAL_PROJECT_NAME}_CLI_ARCHIVE_DESTINATION  lib)

#-----------------------------------------------------------------------------
# Add external project CMake args
#-----------------------------------------------------------------------------
list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS
  BUILD_EXAMPLES:BOOL
  ITK_VERSION_MAJOR:STRING
  ITK_DIR:PATH

  ${LOCAL_PROJECT_NAME}_CLI_LIBRARY_DESTINATION:PATH
  ${LOCAL_PROJECT_NAME}_CLI_ARCHIVE_DESTINATION:PATH
  ${LOCAL_PROJECT_NAME}_CLI_RUNTIME_DESTINATION:PATH

  VTK_DIR:PATH
  GenerateCLP_DIR:PATH
  SlicerExecutionModel_DIR:PATH
  BRAINSCommonLib_DIR:PATH
  Teem_DIR:PATH
  INSTALL_RUNTIME_DESTINATION:STRING
  INSTALL_LIBRARY_DESTINATION:STRING
  INSTALL_ARCHIVE_DESTINATION:STRING
  SUPERBUILD_BINARY_DIR:PATH
  )

set( SUPERBUILD_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR} )
if( DTIPrep_BUILD_SLICER_EXTENSION )
  list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS
    MIDAS_PACKAGE_API_KEY:STRING
    MIDAS_PACKAGE_EMAIL:STRING
    MIDAS_PACKAGE_URL:STRING
    Slicer_DIR:PATH
    DTIPrep_BUILD_SLICER_EXTENSION:BOOL
    )
else()
  list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS
    SUPERBUILD_NOT_EXTENSION:BOOL
    )
endif()
_expand_external_project_vars()
set(COMMON_EXTERNAL_PROJECT_ARGS ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})

if(verbose)
  message("Inner external project args:")
  foreach(arg ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})
    message("  ${arg}")
  endforeach()
endif()

string(REPLACE ";" "^" ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES "${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES}")

if(verbose)
  message("Inner external project argnames:")
  foreach(argname ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES})
    message("  ${argname}")
  endforeach()
endif()


if(USE_DTIProcess)
  include(${CMAKE_CURRENT_LIST_DIR}/SuperBuild/External_DTIProcess.cmake)
  list( APPEND ${LOCAL_PROJECT_NAME}_DEPENDENCIES DTIProcess )
endif()

if(USE_NIRALUtilities)
  include(${CMAKE_CURRENT_LIST_DIR}/SuperBuild/External_niral_utilities.cmake)
  list( APPEND ${LOCAL_PROJECT_NAME}_DEPENDENCIES niral_utilities )
endif()

#------------------------------------------------------------------------------
# Configure and build
#------------------------------------------------------------------------------
set(proj ${LOCAL_PROJECT_NAME})
ExternalProject_Add(${proj}
  DEPENDS ${${LOCAL_PROJECT_NAME}_DEPENDENCIES}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${LOCAL_PROJECT_NAME}-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    --no-warn-unused-cli # HACK Only expected variables should be passed down.
    ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
    ${COMMON_EXTERNAL_PROJECT_ARGS}
    -D${LOCAL_PROJECT_NAME}_SUPERBUILD:BOOL=OFF
    -DBUILD_TESTING:BOOL=${BUILD_TESTING}
    -DUSE_NIRALUtilities:BOOL=${USE_NIRALUtilities}
    -DUSE_DTIProcess:BOOL=${USE_DTIProcess}
  INSTALL_COMMAND ""
  )

## Force rebuilding of the main subproject every time building from super structure
ExternalProject_Add_Step(${proj} forcebuild
    COMMAND ${CMAKE_COMMAND} -E remove
    ${CMAKE_CURRENT_BUILD_DIR}/${proj}-prefix/src/${proj}-stamp/${proj}-build
    DEPENDEES configure
    DEPENDERS build
    ALWAYS 1
  )


