project(DTIPrep)
set(LOCAL_PROJECT_NAME DTIPrep)
cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

enable_testing()
include(Dart)
include(CPack)
include(ExternalProject)
set(BRAINS_CMAKE_HELPER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/BuildScripts)
include(${BRAINS_CMAKE_HELPER_DIR}/CMakeBuildMacros.cmake)

###
SETIFEMPTY(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
SETIFEMPTY(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
SETIFEMPTY(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
SETIFEMPTY(CMAKE_BUNDLE_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/bin)
#if(CMAKE_INSTALL_PREFIX EQUAL "/usr/local")
  set(CMAKE_INSTALL_PREFIX "/opt/NAMIC/DTIPrep" CACHE PATH "The place to install everything" FORCE)
#endif()



set(LOCAL_CMAKE_BUILD_OPTIONS
  -DMAKECOMMAND:STRING=${MAKECOMMAND}
  -DCMAKE_SKIP_RPATH:BOOL=ON
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  -DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE}
  -DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}
  -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
  -DCMAKE_C_FLAGS_RELEASE:STRING=${CMAKE_C_FLAGS_RELEASE}
  -DCMAKE_C_FLAGS_DEBUG:STRING=${CMAKE_C_FLAGS_DEBUG}
  -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
  -DBUILD_EXAMPLES:BOOL=OFF
  -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
  -DBUILD_TESTING:BOOL=${BUILD_TESTING}
  -DCMAKE_GENERATOR:STRING=${CMAKE_GENERATOR}
  -DCMAKE_EXTRA_GENERATOR:STRING=${CMAKE_EXTRA_GENERATOR}
  -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  -DCMAKE_BUNDLE_OUTPUT_DIRECTORY:PATH=${CMAKE_BUNDLE_OUTPUT_DIRECTORY}
  -DLIBRARY_OUTPUT_PATH:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  -DEXECUTABLE_OUTPUT_PATH:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  -DDART_TESTING_TIMEOUT:STRING=${DART_TESTING_TIMEOUT}
  -DMEMORYCHECK_COMMAND_OPTIONS:STRING=${MEMORYCHECK_COMMAND_OPTIONS}
  -DMEMORYCHECK_COMMAND:PATH=${MEMORYCHECK_COMMAND}
  -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
  -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
  -DCMAKE_MODULE_LINKER_FLAGS:STRING=${CMAKE_MODULE_LINKER_FLAGS}
  -DSITE:STRING=${SITE}
  -DBUILDNAME:STRING=${BUILDNAME}
  -DBRAINS_CMAKE_HELPER_DIR:PATH=${BRAINS_CMAKE_HELPER_DIR}
)

# Compute -G arg for configuring external projects with the same CMake generator:
#
if(CMAKE_EXTRA_GENERATOR)
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

include(${BRAINS_CMAKE_HELPER_DIR}/CMakeCommonExternalDefinitions.cmake)
PACKAGE_NEEDS_ITK                 ("${LOCAL_CMAKE_BUILD_OPTIONS}" "${gen}")
PACKAGE_NEEDS_VTKWITHQT           ("${LOCAL_CMAKE_BUILD_OPTIONS}" "${gen}")
PACKAGE_NEEDS_SlicerExecutionModel("${LOCAL_CMAKE_BUILD_OPTIONS}" "${gen}")
PACKAGE_NEEDS_BRAINSCommonLib     ("${LOCAL_CMAKE_BUILD_OPTIONS}" "${gen}")

### Build DicomToNrrdConverter   http://svn.slicer.org/Slicer3/trunk/Applications/CLI/DicomToNrrdConverter
set(EXTERNAL_PACKAGES DicomToNrrdConverter)
foreach(proj ${EXTERNAL_PACKAGES})
  ExternalProject_Add(${proj}
    SVN_REPOSITORY "http://svn.slicer.org/Slicer3/trunk/Applications/CLI/DicomToNrrdConverter"
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    DEPENDS  ${ITK_DEPEND} SlicerExecutionModel
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${LOCAL_CMAKE_BUILD_OPTIONS}
      ${QT_ARGS}
      -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
      -DITK_DIR:PATH=${ITK_DIR}
  INSTALL_COMMAND ""
  #  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
  )
endforeach()

### Build DTIProcess             https://www.nitrc.org/svn/dtiprocess/trunk
set(EXTERNAL_PACKAGES DTIProcess)
foreach(proj ${EXTERNAL_PACKAGES})
  ExternalProject_Add(${proj}
    SVN_REPOSITORY "https://www.nitrc.org/svn/dtiprocess/trunk"
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    DEPENDS  ${VTK_DEPENDANCY} ${ITK_DEPEND} SlicerExecutionModel
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${LOCAL_CMAKE_BUILD_OPTIONS}
      ${QT_ARGS}
      -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
      -DITK_DIR:PATH=${ITK_DIR}
      -DVTK_DIR:PATH=${VTK_DIR}
  INSTALL_COMMAND ""
  #  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
  )
endforeach()

### Build GTRACT                 https://www.nitrc.org/svn/vmagnotta/GTRACT
set(EXTERNAL_PACKAGES GTRACTprog)
foreach(proj ${EXTERNAL_PACKAGES})
  ExternalProject_Add(${proj}
    SVN_REPOSITORY "https://www.nitrc.org/svn/vmagnotta/GTRACT"
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    DEPENDS  BRAINSCommonLib ${VTK_DEPENDANCY} ${ITK_DEPEND} SlicerExecutionModel
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${LOCAL_CMAKE_BUILD_OPTIONS}
      ${QT_ARGS}
      -DBRAINSCommonLib_DIR:PATH=${BRAINSCommonLib_DIR}
      -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
      -DITK_DIR:PATH=${ITK_DIR}
      -DVTK_DIR:PATH=${VTK_DIR}
  INSTALL_COMMAND ""
  #  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
  )
endforeach()

### Build MultiImageRegistration http://svn.na-mic.org/NAMICSandBox/trunk/MultiImageRegistration
set(EXTERNAL_PACKAGES MultiImageRegistration)
foreach(proj ${EXTERNAL_PACKAGES})
  ExternalProject_Add(${proj}
    SVN_REPOSITORY "http://svn.na-mic.org/NAMICSandBox/trunk/MultiImageRegistration"
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    DEPENDS  BRAINSCommonLib ${VTK_DEPENDANCY} ${ITK_DEPEND} SlicerExecutionModel
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${LOCAL_CMAKE_BUILD_OPTIONS}
      ${QT_ARGS}
      -DBRAINSCommonLib_DIR:PATH=${BRAINSCommonLib_DIR}
      -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
      -DITK_DIR:PATH=${ITK_DIR}
      -DVTK_DIR:PATH=${VTK_DIR}
  INSTALL_COMMAND ""
  #  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
  )
endforeach()
set(MultiImageRegistration_DIR ${CMAKE_CURRENT_BINARY_DIR}/MultiImageRegistration-build)

### Build DTIPrep
set(EXTERNAL_PACKAGES src)
foreach(proj ${EXTERNAL_PACKAGES})
  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${proj}
    DOWNLOAD_COMMAND ""
    BINARY_DIR ${proj}-build
    DEPENDS  MultiImageRegistration BRAINSCommonLib ${VTK_DEPENDANCY} ${ITK_DEPEND} SlicerExecutionModel
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${LOCAL_CMAKE_BUILD_OPTIONS}
      ${QT_ARGS}
      -DBRAINSCommonLib_DIR:PATH=${BRAINSCommonLib_DIR}
      -DMultiImageRegistration_DIR:PATH=${MultiImageRegistration_DIR}
      -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
      -DITK_DIR:PATH=${ITK_DIR}
      -DVTK_DIR:PATH=${VTK_DIR}
  INSTALL_COMMAND ""
  #  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
  )
endforeach()

#QuantitativeFiberClustering     https://www.nitrc.org/svn/quantitativedti/QuantitativeDiffusionTools/trunk
#QuantitativeFiberClusteringDocs https://www.nitrc.org/svn/quantitativedti/QuantitativeDiffusionDocs/trunk
