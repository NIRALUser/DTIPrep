# This project is designed to be built outside the Insight source tree.
PROJECT(Eddy_RESEARCH)

cmake_minimum_required(VERSION 2.4)

SET(CMAKE_CXX_FLAGS "-g")

# Find ITK.
FIND_PACKAGE(ITK)
IF(ITK_FOUND)
  INCLUDE(${ITK_USE_FILE})
ELSE(ITK_FOUND)
  MESSAGE(FATAL_ERROR "Cannot build without ITK.  Please set ITK_DIR.")
ENDIF(ITK_FOUND)


#Eddy Current Part

ADD_EXECUTABLE(demo demo.cxx
itkGradientSteepestDescentBaseOptimizer.cxx itkGradientSteepestDescentOptimizer.cxx)
TARGET_LINK_LIBRARIES(demo ITKCommon ITKIO ITKNumerics ITKStatistics)

