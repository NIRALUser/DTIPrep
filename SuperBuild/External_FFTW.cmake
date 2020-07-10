if( NOT EXTERNAL_SOURCE_DIRECTORY )
  set( EXTERNAL_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ExternalSources )
endif()
if( NOT EXTERNAL_BINARY_DIRECTORY )
  set( EXTERNAL_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
endif()
# Make sure this file is included only once
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
# Include dependent projects if any
set(extProjName FFTW)         #The find_package known name
set(proj        ${extProjName} ) #This local name

#if(${USE_SYSTEM_${extProjName}})
#  unset(${extProjName}_DIR CACHE)
#endif()

# Sanity checks
#if(DEFINED ${extProjName}_DIR AND NOT EXISTS ${${extProjName}_DIR})
#  message(FATAL_ERROR "${extProjName}_DIR variable is defined but corresponds to non-existing directory")
#endif()


# Set dependency list
set(${proj}_DEPENDENCIES "")

SlicerMacroCheckExternalProjectDependency(${proj})
if(NOT ( DEFINED "USE_SYSTEM_${extProjName}" AND "${USE_SYSTEM_${extProjName}}" ) )
  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  ### --- Project specific additions here
  set(${proj}_CMAKE_OPTIONS
    )

  if(WIN32) # If windows, no recompilation so just download binary
    set(FFTW_DOWNLOAD_ARGS
      URL "ftp://ftp.fftw.org/pub/fftw/fftw-3.3.4-dll64.zip")
  else(WIN32) # Download source code and recompile
    set(FFTW_DOWNLOAD_ARGS
      URL "http://www.fftw.org/fftw-3.3.4.tar.gz")
      # URL_MD5 2edab8c06b24feeb3b82bbb3ebf3e7b3)
      #URL_MD5 d41d8cd98f00b204e9800998ecf8427e)
endif(WIN32)

  ### --- End Project specific additions
  ExternalProject_Add(${proj}
    ${FFTW_DOWNLOAD_ARGS}
    DOWNLOAD_DIR ${EXTERNAL_BINARY_DIRECTORY}
    SOURCE_DIR ${EXTERNAL_SOURCE_DIRECTORY}/${proj}
    BINARY_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-build
    CONFIGURE_COMMAND ""
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      ${COMMON_EXTERNAL_PROJECT_ARGS}
      ${${proj}_CMAKE_OPTIONS}
    INSTALL_COMMAND ""
    BUILD_COMMAND ${CMAKE_COMMAND} -DTOP_BINARY_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild/InstallFFTW.cmake # -DARGNAME:TYPE=VALUE -P <cmake file> = Process script mode
    )
  set( ${extProjName}_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install )
  set( ${extProjName}_INCLUDE_PATH ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install/include )
  set( ${extProjName}_INSTALL_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install)
  if(WIN32)
    set( ${extProjName}D_LIB ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install/lib/fftw3.lib )
    set( ${extProjName}F_LIB ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install/lib/fftw3f.lib )
  else()
    set( ${extProjName}D_LIB ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install/lib/libfftw3.a )
    set( ${extProjName}F_LIB ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install/lib/libfftw3f.a )
    set( ${extProjName}D_THREADS_LIB ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install/lib/libfftw3_threads.a )
    set( ${extProjName}F_THREADS_LIB ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install/lib/libfftw3f_threads.a )
  endif()
else()
  if(${USE_SYSTEM_${extProjName}})
    find_package(${extProjName} REQUIRED)
    if(NOT ${extProjName}_DIR)
      message(FATAL_ERROR "To use the system ${extProjName}, set ${extProjName}_DIR")
    endif()
  endif()
  # The project is provided using ${extProjName}_DIR, nevertheless since other
  # project may depend on ${extProjName}v4, let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
endif()


list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS ${extProjName}_DIR:PATH)
list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS ${extProjName}_INSTALL_DIR:PATH)
_expand_external_project_vars()
set(COMMON_EXTERNAL_PROJECT_ARGS ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})

ProjectDependancyPop(CACHED_extProjName extProjName)
ProjectDependancyPop(CACHED_proj proj)
