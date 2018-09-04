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

include(${CMAKE_CURRENT_LIST_DIR}/EP_Autoconf_Utils.cmake)

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
set(extProjName TIFF) #The find_package known name
set(proj        TIFF) #This local name

#if(${USE_SYSTEM_${extProjName}})
#  unset(${extProjName}_DIR CACHE)
#endif()

# Sanity checks
if(DEFINED ${extProjName}_DIR AND NOT EXISTS ${${extProjName}_DIR})
  message(FATAL_ERROR "${extProjName}_DIR variable is defined but corresponds to non-existing directory (${${extProjName}_DIR})")
endif()

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(${proj})

if(NOT ( DEFINED "USE_SYSTEM_${extProjName}" AND "${USE_SYSTEM_${extProjName}}" ) )
  #message(STATUS "${__indent}Adding project ${proj}")
  if( ${PROJECT_NAME}_BUILD_ZLIB_SUPPORT )
    list(APPEND ${proj}_DEPENDENCIES zlib)
  endif()
  if( ${PROJECT_NAME}_BUILD_JPEG_SUPPORT )
    list(APPEND ${proj}_DEPENDENCIES JPEG)
  endif()
  # Include dependent projects if any
  SlicerMacroCheckExternalProjectDependency(${proj})

  if( ${PROJECT_NAME}_BUILD_ZLIB_SUPPORT )
    set(${proj}_ZLIB_ARGS 
        --with-zlib-include-dir=${ZLIB_INCLUDE_DIR}
        --with-zlib-lib=${ZLIB_LIBRARY}
      )
  endif()
  if( ${PROJECT_NAME}_BUILD_JPEG_SUPPORT )
    set(${proj}_JPEG_ARGS
        --with-jpeg-lib-dir=${JPEG_LIB_DIR}
        --with-jpeg-include-dir=${JPEG_INCLUDE_DIR}
      )
  endif()
  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set(APPLE_CFLAGS " -DHAVE_APPLE_OPENGL_FRAMEWORK")
  endif()

  ### --- Project specific additions here

  AutoConf_FLAGS(${proj}_CFLAGS C "${APPLE_CFLAGS}")
  AutoConf_FLAGS(${proj}_CXXFLAGS CXX "${APPLE_CFLAGS}")

  ### --- End Project specific additions
  set(${proj}_URL "http://download.osgeo.org/libtiff/tiff-4.0.9.tar.gz")
  set(${proj}_MD5 "54bad211279cc93eb4fca31ba9bfdc79")

  ExternalProject_Add(${proj}
    URL ${${proj}_URL}
    ${URL_HASH_CLAUSE}
    URL_MD5 ${${proj}_MD5}
    SOURCE_DIR ${EXTERNAL_SOURCE_DIRECTORY}/${proj}
    BINARY_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-build
    INSTALL_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install
    LOG_CONFIGURE 0  # Wrap configure in script to ignore log output from dashboards
    LOG_BUILD     0  # Wrap build in script to to ignore log output from dashboards
    LOG_TEST      0  # Wrap test in script to to ignore log output from dashboards
    LOG_INSTALL   0  # Wrap install in script to to ignore log output from dashboards
    ${cmakeversion_external_update} "${cmakeversion_external_update_value}"
    CMAKE_GENERATOR ${gen}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
    --enable-shared=No
    --enable-static=Yes
    --disable-lzma
    ${${proj}_ZLIB_ARGS}
    ${${proj}_JPEG_ARGS}
    CC=${CMAKE_C_COMPILER}
    CXX=${CMAKE_CXX_COMPILER}
    "CFLAGS=${${proj}_CFLAGS}"
    "CXXFLAGS=${${proj}_CXXFLAGS}"
    DEPENDS
      ${${proj}_DEPENDENCIES}
  )
  set(${extProjName}_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install)
  set(${extProjName}_INCLUDE_DIR
    ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install/include)
  set(${extProjName}_LIBRARY
    ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install/lib/libtiff.a)
else()
  if(${USE_SYSTEM_${extProjName}})
    find_package(${extProjName} REQUIRED)
    message("USING the system ${extProjName}, set ${extProjName}_DIR=${${extProjName}_DIR}")
  endif()
  if( NOT TARGET ${proj} )
    # The project is provided using ${extProjName}_DIR, nevertheless since other
    # project may depend on ${extProjName}, let's add an 'empty' one
    SlicerMacroEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
  endif()
endif()

list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS ${extProjName}_DIR:PATH)

ProjectDependancyPop(CACHED_extProjName extProjName)
ProjectDependancyPop(CACHED_proj proj)
