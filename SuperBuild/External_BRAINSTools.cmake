if( NOT EXTERNAL_SOURCE_DIRECTORY )
  set( EXTERNAL_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ExternalSources )
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
set(extProjName BRAINSTools) #The find_package known name
set(proj        BRAINSTools) #This local name
set(${extProjName}_REQUIRED_VERSION "")  #If a required version is necessary, then set this, else leave blank

#if(${USE_SYSTEM_${extProjName}})
#  unset(${extProjName}_DIR CACHE)
#endif()

# Sanity checks
if(DEFINED ${extProjName}_DIR AND NOT EXISTS ${${extProjName}_DIR})
  message(FATAL_ERROR "${extProjName}_DIR variable is defined but corresponds to non-existing directory (${${extProjName}_DIR})")
endif()

# Set dependency list
set(${extProjName}_DEPENDENCIES ITKv4 SlicerExecutionModel VTK DCMTK teem OpenCV zlib )
#if(${PROJECT_NAME}_BUILD_DICOM_SUPPORT)
#  list(APPEND ${proj}_DEPENDENCIES DCMTK)
#endif()

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(${proj})

if(NOT ( DEFINED "${extProjName}_SOURCE_DIR" OR ( DEFINED "USE_SYSTEM_${extProjName}" AND "${USE_SYSTEM_${extProjName}}" ) ) )
  #message(STATUS "${__indent}Adding project ${proj}")
  if(USE_ANTs)
    list(APPEND ${extProjName}_DEPENDENCIES ANTs Boost)
  endif()

  if( ${PROJECT_NAME}_BUILD_TIFF_SUPPORT )
    list(APPEND ${proj}_DEPENDENCIES TIFF)
  endif()
  if( ${PROJECT_NAME}_BUILD_JPEG_SUPPORT )
    list(APPEND ${proj}_DEPENDENCIES JPEG)
  endif()
  # Include dependent projects if any
  SlicerMacroCheckExternalProjectDependency(${proj})

  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
                      #This code was taken from BRAINSTools, for some reason is not working when doing the superbuild but if done here it works
                      # Waiting universal binaries are supported and tested, complain if
                      # multiple architectures are specified.
                      if(NOT "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
                        list(LENGTH CMAKE_OSX_ARCHITECTURES arch_count)
                        if(arch_count GREATER 1)
                          message(FATAL_ERROR "error: Only one value (i386 or x86_64) should be associated with CMAKE_OSX_ARCHITECTURES.")
                        endif()
                      endif()

                      # See CMake/Modules/Platform/Darwin.cmake)
                      #   8.x == Mac OSX 10.4 (Tiger)
                      #   9.x == Mac OSX 10.5 (Leopard)
                      #  10.x == Mac OSX 10.6 (Snow Leopard)
                      #  11.x == Mac OSX 10.7 (Lion)
                      #  12.x == Mac OSX 10.8 (Mountain Lion)
                      #  13.x == Mac OSX 10.9 (Mavericks)
                      #  14.x == Mac OSX 10.10 (Yosemite)
                      set(OSX_SDK_104_NAME "Tiger")
                      set(OSX_SDK_105_NAME "Leopard")
                      set(OSX_SDK_106_NAME "Snow Leopard")
                      set(OSX_SDK_107_NAME "Lion")
                      set(OSX_SDK_108_NAME "Mountain Lion")
                      set(OSX_SDK_109_NAME "Mavericks")
                      set(OSX_SDK_1010_NAME "Yosemite")
                      set(OSX_SDK_1011_NAME "El Capitan")

                      set(OSX_SDK_ROOTS
                        /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs
                        /Developer/SDKs
                        )

                      # Explicitly set the OSX_SYSROOT to the latest one, as its required
                      #       when the SX_DEPLOYMENT_TARGET is explicitly set
                      foreach(SDK_ROOT ${OSX_SDK_ROOTS})
                        if( "x${CMAKE_OSX_SYSROOT}x" STREQUAL "xx")
                          file(GLOB SDK_SYSROOTS "${SDK_ROOT}/MacOSX*.sdk")

                          if(NOT "x${SDK_SYSROOTS}x" STREQUAL "xx")
                            set(SDK_SYSROOT_NEWEST "")
                            set(CMAKE_OSX_DEPLOYMENT_TARGET "0")
                            # find the latest SDK
                            foreach(SDK_ROOT_I ${SDK_SYSROOTS})
                              # extract version from SDK
                              string(REGEX MATCH "MacOSX([0-9]+\\.[0-9]+)\\.sdk" _match "${SDK_ROOT_I}")
                              if("${CMAKE_MATCH_1}" VERSION_GREATER "${CMAKE_OSX_DEPLOYMENT_TARGET}")
                                set(SDK_SYSROOT_NEWEST "${SDK_ROOT_I}")
                                set(CMAKE_OSX_DEPLOYMENT_TARGET "${CMAKE_MATCH_1}")
                              endif()
                            endforeach()

                            if(NOT "x${SDK_SYSROOT_NEWEST}x" STREQUAL "xx")
                              string(REPLACE "." "" sdk_version_no_dot ${CMAKE_OSX_DEPLOYMENT_TARGET})
                              set(OSX_NAME ${OSX_SDK_${sdk_version_no_dot}_NAME})
                              set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Force build for 64-bit ${OSX_NAME}." FORCE)
                              set(CMAKE_OSX_SYSROOT "${SDK_SYSROOT_NEWEST}" CACHE PATH "Force build for 64-bit ${OSX_NAME}." FORCE)
                              set(CMAKE_OSX_DEPLOYMENT_TARGET "${CMAKE_OSX_DEPLOYMENT_TARGET}" CACHE PATH "Force Deployment Target ${OSX_NAME}." FORCE)
                              message(STATUS "Setting OSX_ARCHITECTURES to '${CMAKE_OSX_ARCHITECTURES}' as none was specified.")
                              message(STATUS "Setting OSX_SYSROOT to latest '${CMAKE_OSX_SYSROOT}' as none was specified.")
                              message(STATUS "Setting OSX_DEPLOYMENT_TARGET to latest '${CMAKE_OSX_DEPLOYMENT_TARGET}' as none was specified.")
                            endif()
                          endif()
                        endif()
                      endforeach()

                      if(NOT "${CMAKE_OSX_SYSROOT}" STREQUAL "")
                        if(NOT EXISTS "${CMAKE_OSX_SYSROOT}")
                          message(FATAL_ERROR "error: CMAKE_OSX_SYSROOT='${CMAKE_OSX_SYSROOT}' does not exist")
                        endif()
                      endif()

    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  set(BRAINS_ANTS_PARAMS
    -DUSE_ANTS:BOOL=${USE_ANTs}
    )
  if(USE_ANTs)
    list(APPEND BRAINS_ANTS_PARAMS
      -DUSE_SYSTEM_ANTS:BOOL=ON
      -DANTs_SOURCE_DIR:PATH=${ANTs_SOURCE_DIR}
      -DANTs_LIBRARY_DIR:PATH=${ANTs_LIBRARY_DIR}
      -DUSE_SYSTEM_Boost:BOOL=ON
      -DBoost_NO_BOOST_CMAKE:BOOL=ON #Set Boost_NO_BOOST_CMAKE to ON to disable the search for boost-cmake
      -DBoost_DIR:PATH=${BOOST_ROOT}
      -DBOOST_DIR:PATH=${BOOST_ROOT}
      -DBOOST_ROOT:PATH=${BOOST_ROOT}
      -DBOOST_INCLUDE_DIR:PATH=${BOOST_INCLUDE_DIR}
      )
  endif()
  ### --- Project specific additions here
  # message("VTK_DIR: ${VTK_DIR}")
  # message("ITK_DIR: ${ITK_DIR}")
  # message("SlicerExecutionModel_DIR: ${SlicerExecutionModel_DIR}")
  # message("BOOST_INCLUDE_DIR:PATH=${BOOST_INCLUDE_DIR}")

  if( ${PROJECT_NAME}_BUILD_TIFF_SUPPORT )
    set(${proj}_TIFF_ARGS
      -DUSE_SYSTEM_TIFF:BOOL=ON
      -DTIFF_DIR:PATH=${TIFF_DIR}
       )
  endif()
  if( ${PROJECT_NAME}_BUILD_JPEG_SUPPORT )
    set(${proj}_JPEG_ARGS
      -DUSE_SYSTEM_JPEG:BOOL=ON
      -DJPEG_DIR:PATH=${JPEG_DIR}
      )
  endif()

  set(${proj}_CMAKE_OPTIONS
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/${proj}-install
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DUSE_SYSTEM_ITK:BOOL=ON
      -DUSE_SYSTEM_VTK:BOOL=ON
      -DUSE_SYSTEM_DCMTK:BOOL=ON
      -DUSE_SYSTEM_Teem:BOOL=ON
      -DUSE_SYSTEM_OpenCV:BOOL=ON
      -DOpenCV_DIR:PATH=${OpenCV_DIR}
      -DUSE_SYSTEM_ReferenceAtlas:BOOL=ON
      -DReferenceAtlas_DIR:STRING=${ReferenceAtlas_DIR}
      -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
      -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
      -DUSE_SYSTEM_SlicerExecutionModel:BOOL=ON
      -DDCMTK_DIR:PATH=${DCMTK_DIR}
      -DDCMTK_config_INCLUDE_DIR:PATH=${DCMTK_DIR}/include
      -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
      -DSuperBuild_BRAINSTools_USE_GIT_PROTOCOL=${${CMAKE_PROJECT_NAME}_USE_GIT_PROTOCOL}
      -DBRAINSTools_SUPERBUILD:BOOL=OFF
      -DITK_DIR:PATH=${ITK_DIR}
      -DVTK_DIR:PATH=${VTK_DIR}
      -DTeem_DIR:PATH=${Teem_DIR}
      -D${proj}_USE_QT:BOOL=${LOCAL_PROJECT_NAME}_USE_QT
      -DUSE_SYSTEM_ZLIB:BOOL=ON
      -Dzlib_DIR:PATH=${zlib_DIR}
      -DZLIB_ROOT:PATH=${zlib_DIR}
      -DZLIB_INCLUDE_DIR:PATH=${zlib_DIR}/include
      -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARY}
      -DUSE_BRAINSABC:BOOL=OFF
      -DUSE_BRAINSConstellationDetector:BOOL=OFF
      -DUSE_BRAINSContinuousClass:BOOL=OFF
      -DUSE_BRAINSCut:BOOL=OFF
      -DUSE_BRAINSDemonWarp:BOOL=OFF
      -DUSE_BRAINSFit:BOOL=OFF
      -DUSE_BRAINSFitEZ:BOOL=OFF
      -DUSE_BRAINSTalairach:BOOL=OFF
      -DUSE_BRAINSImageConvert:BOOL=OFF
      -DUSE_BRAINSInitializedControlPoints:BOOL=OFF
      -DUSE_BRAINSLandmarkInitializer:BOOL=OFF
      -DUSE_BRAINSMultiModeSegment:BOOL=OFF
      -DUSE_BRAINSMush:BOOL=OFF
      -DUSE_BRAINSImageConvert:BOOL=OFF
      -DUSE_BRAINSInitializedControlPoints:BOOL=OFF
      -DUSE_BRAINSLandmarkInitializer:BOOL=OFF
      -DUSE_BRAINSMultiModeSegment:BOOL=OFF
      -DUSE_BRAINSMush:BOOL=OFF
      -DUSE_BRAINSROIAuto:BOOL=OFF
      -DUSE_BRAINSResample:BOOL=OFF
      -DUSE_BRAINSSnapShotWriter:BOOL=OFF
      -DUSE_BRAINSSurfaceTools:BOOL=OFF
      -DUSE_BRAINSTransformConvert:BOOL=OFF
      -DUSE_BRAINSPosteriorToContinuousClass:BOOL=OFF
      -DUSE_BRAINSCreateLabelMapFromProbabilityMaps:BOOL=OFF
      -DUSE_DebugImageViewer:BOOL=OFF
      -DUSE_GTRACT:BOOL=OFF
      -DUSE_ICCDEF:BOOL=OFF
      -DUSE_ConvertBetweenFileFormats:BOOL=OFF
      -DUSE_ImageCalculator:BOOL=OFF
      -DUSE_AutoWorkup:BOOL=OFF
      ${BRAINS_ANTS_PARAMS}
    )

  ### --- End Project specific additions
  set(${proj}_REPOSITORY "${git_protocol}://github.com/BRAINSia/BRAINSTools.git")
  set(${proj}_GIT_TAG fe408f49fd757691190a0282ab71a14d4bafa83a )
  ExternalProject_Add(${proj}
    GIT_REPOSITORY ${${proj}_REPOSITORY}
    GIT_TAG ${${proj}_GIT_TAG}
    ${git_config_arg}
    SOURCE_DIR ${EXTERNAL_SOURCE_DIRECTORY}/${proj}
    BINARY_DIR ${proj}-build
    LOG_CONFIGURE 0  # Wrap configure in script to ignore log output from dashboards
    LOG_BUILD     0  # Wrap build in script to to ignore log output from dashboards
    LOG_TEST      0  # Wrap test in script to to ignore log output from dashboards
    LOG_INSTALL   0  # Wrap install in script to to ignore log output from dashboards
    ${cmakeversion_external_update} "${cmakeversion_external_update_value}"
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -Wno-dev
      --no-warn-unused-cli
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      ${COMMON_EXTERNAL_PROJECT_ARGS}
      ${${proj}_CMAKE_OPTIONS}
    INSTALL_COMMAND ""
    DEPENDS
      ${${extProjName}_DEPENDENCIES}
    )
  set(${extProjName}_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(${extProjName}_SOURCE_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj})
  set(${extProjName}_INSTALL_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-build)
  set(BRAINSCommonLib_DIR    ${CMAKE_BINARY_DIR}/${proj}-build/BRAINSCommonLib)
else()
  if(${USE_SYSTEM_${extProjName}})
    find_package(${extProjName} ${${extProjName}_REQUIRED_VERSION} REQUIRED)
    message("USING the system ${extProjName}, set ${extProjName}_DIR=${${extProjName}_DIR}")
  endif()
  if( NOT TARGET ${proj} )
    # The project is provided using ${extProjName}_DIR, nevertheless since other
    # project may depend on ${extProjName}, let's add an 'empty' one
    SlicerMacroEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
  endif()
endif()

list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS ${extProjName}_DIR:PATH)
list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS ${extProjName}_INSTALL_DIR:PATH)
_expand_external_project_vars()
ProjectDependancyPop(CACHED_extProjName extProjName)
ProjectDependancyPop(CACHED_proj proj)
