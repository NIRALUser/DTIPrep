if( NOT EXTERNAL_SOURCE_DIRECTORY )
  set( EXTERNAL_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ExternalSources )
endif()
if( NOT EXTERNAL_BINARY_DIRECTORY )
  set( EXTERNAL_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
endif()

# Make sure this file is included only once by creating globally unique varibles
# based on the name of this included file.
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

## External_${extProjName}.cmake files can be recurisvely included,
## and cmake variables are global, so when including sub projects it
## is important make the extProjName and proj variables
## appear to stay constant in one of these files.
## Store global variables before overwriting (then restore at end of this file.)
ProjectDependancyPush(CACHED_extProjName ${extProjName})
ProjectDependancyPush(CACHED_proj ${proj})

# Make sure that the ExtProjName/IntProjName variables are unique globally
# even if other External_${ExtProjName}.cmake files are sourced by
# SlicerMacroCheckExternalProjectDependency
set(extProjName DCMTK) #The find_package known name
set(proj        DCMTK) #This local name
set(${extProjName}_REQUIRED_VERSION "")  #If a required version is necessary, then set this, else leave blank

#if(${USE_SYSTEM_${extProjName}})
#  unset(${extProjName}_DIR CACHE)
#endif()

# Sanity checks
if(DEFINED ${extProjName}_DIR AND NOT EXISTS ${${extProjName}_DIR})
  message(FATAL_ERROR "${extProjName}_DIR variable is defined but corresponds to non-existing directory (${${extProjName}_DIR})")
endif()

# Set dependency list
set(${proj}_DEPENDENCIES "" )
# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(${proj})

if(NOT ( DEFINED "USE_SYSTEM_${extProjName}" AND "${USE_SYSTEM_${extProjName}}" ) )
  #message(STATUS "${__indent}Adding project ${proj}")
  if( ${PROJECT_NAME}_BUILD_TIFF_SUPPORT )
    list(APPEND ${proj}_DEPENDENCIES TIFF)
  endif()
  if( ${PROJECT_NAME}_BUILD_JPEG_SUPPORT )
    list(APPEND ${proj}_DEPENDENCIES JPEG)
  endif()
  if( ${PROJECT_NAME}_BUILD_ZLIB_SUPPORT )
    list(APPEND ${proj}_DEPENDENCIES zlib)
  endif()
  SlicerMacroCheckExternalProjectDependency(${proj})

  if( ${PROJECT_NAME}_BUILD_TIFF_SUPPORT )
    set(${proj}_TIFF_ARGS
      -DDCMTK_WITH_TIFF:BOOL=ON  # see CTK github issue #25
      -DTIFF_DIR:PATH=${TIFF_DIR}
       )
  else()
    set(${proj}_TIFF_ARGS
      -DDCMTK_WITH_TIFF:BOOL=OFF
       )
  endif()
  if( ${PROJECT_NAME}_BUILD_JPEG_SUPPORT )
    set(${proj}_JPEG_ARGS
      -DDCMTK_WITH_JPEG:BOOL=ON  # see CTK github issue #25
      -DJPEG_DIR:FILEPATH=${JPEG_DIR}
      )
  endif()
  if( ${PROJECT_NAME}_BUILD_ZLIB_SUPPORT )
    set(${proj}_ZLIB_ARGS
      -DDCMTK_WITH_ZLIB:BOOL=ON
      -DZLIB_DIR:FILEPATH=${ZLIB_DIR}
      )
  endif()


  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  set(CMAKE_PROJECT_INCLUDE_EXTERNAL_PROJECT_ARG)
  if(CTEST_USE_LAUNCHERS)
    set(CMAKE_PROJECT_INCLUDE_EXTERNAL_PROJECT_ARG
      "-DCMAKE_PROJECT_DCMTK_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake")
  endif()

  ### --- Project specific additions here
  set(${proj}_CMAKE_OPTIONS
      -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIRECTORY}/${proj}-install
      #-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      ${CMAKE_PROJECT_INCLUDE_EXTERNAL_PROJECT_ARG}
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DDCMTK_WITH_DOXYGEN:BOOL=OFF
      -DDCMTK_WITH_OPENSSL:BOOL=OFF # see CTK github issue #25
      -DDCMTK_WITH_PNG:BOOL=OFF # see CTK github issue #25
      -DDCMTK_WITH_XML:BOOL=OFF  # see CTK github issue #25
      -DDCMTK_WITH_ICONV:BOOL=OFF  # see CTK github issue #178
      -DDCMTK_FORCE_FPIC_ON_UNIX:BOOL=ON
      -DDCMTK_OVERWRITE_WIN32_COMPILER_FLAGS:BOOL=OFF
      -DDCMTK_WITH_WRAP:BOOL=OFF   # CTK does not build on Mac with this option turned ON due to library dependencies missing
      ${${proj}_TIFF_ARGS}
      ${${proj}_JPEG_ARGS}
      ${${proj}_ZLIB_ARGS}
  )
  ### --- End Project specific additions
  set(${proj}_REPOSITORY ${git_protocol}://github.com/commontk/DCMTK.git)
  set(${proj}_GIT_TAG "f461865d1759854db56e4c840991c81c77e45bb9")
  ExternalProject_Add(${proj}
    GIT_REPOSITORY ${${proj}_REPOSITORY}
    GIT_TAG ${${proj}_GIT_TAG}
    ${git_config_arg}
    SOURCE_DIR ${EXTERNAL_SOURCE_DIRECTORY}/${proj}
    BINARY_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-build
    INSTALL_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install
    LOG_CONFIGURE 0  # Wrap configure in script to ignore log output from dashboards
    LOG_BUILD     0  # Wrap build in script to to ignore log output from dashboards
    LOG_TEST      0  # Wrap test in script to to ignore log output from dashboards
    LOG_INSTALL   0  # Wrap install in script to to ignore log output from dashboards
    ${cmakeversion_external_update} "${cmakeversion_external_update_value}"
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      ${COMMON_EXTERNAL_PROJECT_ARGS}
      ${${proj}_CMAKE_OPTIONS}
## We really do want to install in order to limit # of include paths INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
  )
  set(${extProjName}_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install/lib/cmake/dcmtk)
else()
  if(${USE_SYSTEM_${extProjName}})
    find_package(${extProjName} REQUIRED NO_MODULE)
    message("USING the system ${extProjName}, set ${extProjName}_DIR=${${extProjName}_DIR}")
  endif()
  if( NOT TARGET ${proj} )
    # The project is provided using ${extProjName}_DIR, nevertheless since other
    # project may depend on ${extProjName}, let's add an 'empty' one
    SlicerMacroEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
  endif()
endif()
set( ${PRIMARY_PROJECT_NAME}_BUILD_DICOM_SUPPORT ON )
list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS ${extProjName}_DIR:PATH)
list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS ${PRIMARY_PROJECT_NAME}_BUILD_DICOM_SUPPORT:BOOL)
_expand_external_project_vars()
set(COMMON_EXTERNAL_PROJECT_ARGS ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})

ProjectDependancyPop(CACHED_extProjName extProjName)
ProjectDependancyPop(CACHED_proj proj)
