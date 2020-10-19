
#-----------------------------------------------------------------------------
set(verbose FALSE)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
enable_language(C)
enable_language(CXX)

#-----------------------------------------------------------------------------
if(WIN32)
  set(fileextension .exe)
endif()

set( COMPILE_EXTERNAL_ITKTransformTools ON CACHE BOOL "Compile External ITKTransformTools" FORCE )
if( ${LOCAL_PROJECT_NAME}_BUILD_SLICER_EXTENSION )
  # Slicer
  find_package(Slicer REQUIRED)
  # Therefore we recompile all the libraries even though Slicer has already built the libraries we need.
  set( COMPILE_EXTERNAL_DTIProcess OFF CACHE BOOL "Compile External DTIProcess" FORCE )
  set( COMPILE_EXTERNAL_BRAINSTools OFF CACHE BOOL "Compile External BRAINSTools" FORCE )
  set( COMPILE_EXTERNAL_ResampleDTIlogEuclidean OFF CACHE BOOL "Compile External ResampleDTIlogEuclidean" FORCE )
  set( COMPILE_EXTERNAL_ANTs ON CACHE BOOL "Compile External ANTs" FORCE )
  set( EXTENSION_NO_CLI ITKTransformTools ANTS )
  set( CONFIGURE_TOOLS_PATHS OFF CACHE BOOL "Use CMake to find where the tools are and hard-code their path in the executable" FORCE )  

  set( USE_SYSTEM_ITK ON CACHE BOOL "Build using an externally defined version of ITK" FORCE )
  set( USE_SYSTEM_VTK ON CACHE BOOL "Build using an externally defined version of VTK" FORCE )

endif()


macro(COMPILE_EXTERNAL_TOOLS)
  set(options "")
  set(oneValueArgs
    TOOL_PROJECT_NAME
    )
  set(multiValueArgs
    TOOL_NAMES
    )
  CMAKE_PARSE_ARGUMENTS(LOCAL
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )
  option(COMPILE_EXTERNAL_${LOCAL_TOOL_PROJECT_NAME} "Compile External ${LOCAL_TOOL_PROJECT_NAME}" OFF )
  if( COMPILE_EXTERNAL_${LOCAL_TOOL_PROJECT_NAME} )
    list( APPEND ${LOCAL_PROJECT_NAME}_DEPENDENCIES ${LOCAL_TOOL_PROJECT_NAME} )
    list( APPEND LIST_TOOLS ${LOCAL_TOOL_NAMES} )
    foreach( var ${LOCAL_TOOL_NAMES} )
      set( ${var}_INSTALL_DIRECTORY ${EXTERNAL_BINARY_DIRECTORY}/${LOCAL_TOOL_PROJECT_NAME}-install )
      set( ${var}TOOL ${${var}_INSTALL_DIRECTORY}/${INSTALL_RUNTIME_DESTINATION}/${var}${fileextension} CACHE PATH "Path to a program." FORCE )
    endforeach()
  else()
    list( FIND LIST_TOOLS ${LOCAL_TOOL_PROJECT_NAME} pos )
    if( "${pos}" GREATER "-1" )
      list( REMOVE_ITEM ${LOCAL_PROJECT_NAME}_DEPENDENCIES ${LOCAL_TOOL_PROJECT_NAME} )
    endif()
    foreach( var ${LOCAL_TOOL_NAMES} )
      list( FIND LIST_TOOLS ${var} pos )
      if( "${pos}" GREATER "-1" )
        list( REMOVE_ITEM LIST_TOOLS ${var} )
      endif()
      unset( ${var}TOOL CACHE )
    endforeach()
  endif()
endmacro()
#-----------------------------------------------------------------------------
# Git protocole option
#-----------------------------------------------------------------------------
option(USE_GIT_PROTOCOL_${CMAKE_PROJECT_NAME} "If behind a firewall turn this off to use http instead." ON)
set(git_protocol "git")
if(NOT USE_GIT_PROTOCOL_${CMAKE_PROJECT_NAME})
  set(git_protocol "http")
endif()

find_package(Git REQUIRED)
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



SETIFEMPTY( EXTERNAL_SOURCE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
SETIFEMPTY( EXTERNAL_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
SETIFEMPTY( INSTALL_RUNTIME_DESTINATION bin )
SETIFEMPTY( INSTALL_LIBRARY_DESTINATION lib )
SETIFEMPTY( INSTALL_ARCHIVE_DESTINATION lib )

set(${LOCAL_PROJECT_NAME}_DEPENDENCIES "")
set(LIST_TOOLS "")

#------------------------------------------------------------------------------
# Configure tools paths
#------------------------------------------------------------------------------
option(CONFIGURE_TOOLS_PATHS "Use CMake to find where the tools are and hard-code their path in the executable" ON)
if( NOT CONFIGURE_TOOLS_PATHS )
  foreach( var ${LIST_TOOLS})
    mark_as_advanced( FORCE ${var}TOOL)
  endforeach()
else()
  foreach( var ${LIST_TOOLS})
    mark_as_advanced(CLEAR ${var}TOOL)
  endforeach()
endif()

# COMPILE_EXTERNAL_TOOLS( TOOL_NAMES dtiprocess TOOL_PROJECT_NAME DTIProcess)
# COMPILE_EXTERNAL_TOOLS( TOOL_NAMES ITKTransformTools TOOL_PROJECT_NAME ITKTransformTools)
# COMPILE_EXTERNAL_TOOLS( TOOL_NAMES ResampleDTIlogEuclidean TOOL_PROJECT_NAME ResampleDTIlogEuclidean)
# COMPILE_EXTERNAL_TOOLS( TOOL_NAMES BRAINSFit BRAINSDemonWarp TOOL_PROJECT_NAME BRAINSTools)
# COMPILE_EXTERNAL_TOOLS( TOOL_NAMES ANTS TOOL_PROJECT_NAME ANTs)

if( NOT ${LOCAL_PROJECT_NAME}_BUILD_SLICER_EXTENSION )
  # Do not configure external tools paths: extension will be run on a different computer,
  # we don't need to find the tools on the computer on which the extension is built
  include(FindExternalTools)
endif()

option(USE_SYSTEM_ITK "Build using an externally defined version of ITK" OFF)
option(USE_SYSTEM_SlicerExecutionModel "Build using an externally defined version of SlicerExecutionModel"  OFF)
option(USE_SYSTEM_BatchMake "Build using an externally defined version of BatchMake" OFF)
option(USE_SYSTEM_ANTs "Build using an externally defined version of ANTs" OFF)
option(${PROJECT_NAME}_BUILD_FFTW_SUPPORT "Build external FFTW" ON)
option(USE_SYSTEM_DTIProcess "Build using external DTIProcess" OFF)
option(USE_SYSTEM_niral_utilities "Build using external niral_utilities" OFF)
option(${PROJECT_NAME}_USE_QT "Use Qt" ON)

#------------------------------------------------------------------------------
# ${LOCAL_PROJECT_NAME} dependency list
#------------------------------------------------------------------------------

list( APPEND ${LOCAL_PROJECT_NAME}_DEPENDENCIES VTK DCMTK FFTW ITKv4 SlicerExecutionModel DTIProcess niral_utilities)
if( NOT DTIPrep_BUILD_SLICER_EXTENSION )
  list(APPEND ${LOCAL_PROJECT_NAME}_DEPENDENCIES
    BRAINSTools
    )
endif()

set(USE_ITK_Module_MGHIO TRUE)
set(${PROJECT_NAME}_BUILD_DICOM_SUPPORT TRUE )
set(${PROJECT_NAME}_BUILD_ZLIB_SUPPORT TRUE )

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
  CMAKE_BUNDLE_OUTPUT_DIRECTORY:PATH
  CTEST_NEW_FORMAT:BOOL
  MEMORYCHECK_COMMAND_OPTIONS:STRING
  MEMORYCHECK_COMMAND:PATH
  SITE:STRING
  BUILDNAME:STRING
  ${PROJECT_NAME}_BUILD_DICOM_SUPPORT:BOOL
  CMAKE_MODULE_PATH:PATH
  INSTALL_RUNTIME_DESTINATION:PATH
  INSTALL_LIBRARY_DESTINATION:PATH
  INSTALL_ARCHIVE_DESTINATION:PATH
  )

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

#-----------------------------------------------------------------------------
# Add external project CMake args
#-----------------------------------------------------------------------------
list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS
  BUILD_EXAMPLES:BOOL
  BUILD_TESTING:BOOL
  ITK_VERSION_MAJOR:STRING
  ITK_DIR:PATH
  VTK_DIR:PATH
  Slicer_DIR:PATH
  BatchMake_DIR:PATH
  GenerateCLP_DIR:PATH
  SlicerExecutionModel_DIR:PATH
  ${LOCAL_PROJECT_NAME}_BUILD_SLICER_EXTENSION:BOOL
  STATIC_${LOCAL_PROJECT_NAME}:BOOL
  ${LOCAL_PROJECT_NAME}_USE_QT:BOOL
  ANTSTOOL:PATH
  BRAINSCommonLib_DIR:PATH
  BRAINSCommonLib_HINTS_DIR:PATH
  ResampleDTIlogEuclideanTOOL:PATH
  DTIProcess_DIR:PATH
  niral_utilities_DIR:PATH 
  DCMTK_DIR:PATH 
  ITKTransformTools_DIR:PATH
  ANTs_DIR:PATH
  GLUT_DIR:PATH
  CONFIGURE_TOOLS_PATHS:BOOL
  JSON_DIR:PATH
  )

foreach( VAR ${LIST_TOOLS} )
  set( ${VAR}_INSTALL_DIRECTORY ${${VAR}_INSTALL_DIRECTORY}/${INSTALL_RUNTIME_DESTINATION}/${VAR}${fileextension} )
  list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS
    ${VAR}_INSTALL_DIRECTORY:PATH
    )
endforeach()

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

#------------------------------------------------------------------------------
# Configure and build
#------------------------------------------------------------------------------
option(CONFIGURE_TOOLS_PATHS "Use CMake to find where the tools are and hard-code their path in the executable" ON)
if( NOT CONFIGURE_TOOLS_PATHS )
  foreach( var ${LIST_TOOLS})
    mark_as_advanced( FORCE ${var}TOOL)
  endforeach()
else()
  foreach( var ${LIST_TOOLS})
    mark_as_advanced(CLEAR ${var}TOOL)
  endforeach()
endif()

set(proj ${LOCAL_PROJECT_NAME})
list(APPEND LIST_TOOLS ${LOCAL_PROJECT_NAME} )
set( ${LOCAL_PROJECT_NAME}TOOL ${LOCAL_PROJECT_NAME} )

if(NOT ${LOCAL_PROJECT_NAME}_INSTALL_DIRECTORY)
  set( ${LOCAL_PROJECT_NAME}_INSTALL_DIRECTORY ${EXTERNAL_BINARY_DIRECTORY}/${LOCAL_PROJECT_NAME}-install )
endif()

set(proj_build ${proj}-build)

ExternalProject_Add(${proj}-inner
  DEPENDS ${${LOCAL_PROJECT_NAME}_DEPENDENCIES}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${proj}-inner-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
    ${COMMON_EXTERNAL_PROJECT_ARGS}
    -D${LOCAL_PROJECT_NAME}_SUPERBUILD:BOOL=OFF
    -DCMAKE_INSTALL_PREFIX:PATH=${${LOCAL_PROJECT_NAME}_INSTALL_DIRECTORY}
    -DBatchMake_SOURCE_DIR:PATH=${EXTERNAL_SOURCE_DIRECTORY}/BatchMake

    -DInnerBuildCMakeLists:BOOL=ON
    -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
    ${COMMON_BUILD_OPTIONS_FOR_EXTERNALPACKAGES}
    -DUSE_GIT_PROTOCOL:BOOL=${USE_GIT_PROTOCOL}
    -DITK_DIR:PATH=${ITK_DIR}
    -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
    -DQT_QMAKE_EXECUTABLE:PATH=${QT_QMAKE_EXECUTABLE}
    -DBUILD_TESTING:BOOL=ON
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/${proj}-install
    # Installation step
    # Slicer extension
    -DDTIAtlasBuilder_BUILD_SLICER_EXTENSION:BOOL=${DTIAtlasBuilder_BUILD_SLICER_EXTENSION}
    -DSlicer_DIR:PATH=${Slicer_DIR}
    -DEXTENSION_NAME:STRING=${EXTENSION_NAME}
  )

if( ${LOCAL_PROJECT_NAME}_BUILD_SLICER_EXTENSION )
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CMAKE_BINARY_DIR}/${LOCAL_PROJECT_NAME}-inner-build;${EXTENSION_NAME};ALL;/")
  include(${Slicer_EXTENSION_CPACK})
endif()
