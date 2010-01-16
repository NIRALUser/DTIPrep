PROJECT ( DTIPrepWorld )
cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6)
#INCLUDE(CPack)

ENABLE_TESTING()
INCLUDE(Dart)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DTIPrep_BINARY_DIR}/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${DTIPrep_BINARY_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DTIPrep_BINARY_DIR}/bin)

FIND_PACKAGE( VTK REQUIRED )
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE( ITK REQUIRED )
INCLUDE(${ITK_USE_FILE})

##  In many cases sub-projects depending on SlicerExectuion Model
##  that can be built stand alone are combined in larger packages
##  This logic will include SlicerExectionModel only if it
##  has not already been built.
FIND_PACKAGE(GenerateCLP NO_MODULE QUIET)
IF( NOT GenerateCLP_DIR )
      ADD_SUBDIRECTORY(SlicerExecutionModel)
ENDIF( NOT GenerateCLP_DIR )
FIND_PACKAGE(GenerateCLP NO_MODULE REQUIRED)
IF(GenerateCLP_DIR)
  INCLUDE(${GenerateCLP_USE_FILE})
ELSE(GenerateCLP_DIR)
  MESSAGE(FATAL_ERROR "Can't build without GenerateCLP. Please set GenerateCLP_DIR")
ENDIF(GenerateCLP_DIR)

if( NOT DicomToNrrdConverter_Included )
  ADD_SUBDIRECTORY(DicomToNrrdConverter)
  SET(DicomToNrrdConverter_Included TRUE)
endif( NOT DicomToNrrdConverter_Included )

ADD_SUBDIRECTORY(GTRACT)
ADD_SUBDIRECTORY(DTIProcess)
ADD_SUBDIRECTORY(DTIPrep)
ADD_SUBDIRECTORY(MultiImageRegistration)

