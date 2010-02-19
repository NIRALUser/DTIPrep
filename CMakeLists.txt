PROJECT ( DTIPrep )
set(LOCAL_PROJECT_NAME DTIPrep)
cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6)

ENABLE_TESTING()
include(Dart)
#include(CPack)

## A simple macro to set variables ONLY if it has not been set
## This is needed when stand-alone packages are combined into
## a larger package, and the desired behavior is that all the
## binary results end up in the combined directory.
if(NOT SETIFEMPTY)
macro(SETIFEMPTY)
  set(KEY ${ARGV0})
  set(VALUE ${ARGV1})
  if(NOT ${KEY})
    set(${KEY} ${VALUE})
  endif(NOT ${KEY})
endmacro(SETIFEMPTY KEY VALUE)
endif(NOT SETIFEMPTY)
###
SETIFEMPTY(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
SETIFEMPTY(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
SETIFEMPTY(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
SETIFEMPTY(CMAKE_BUNDLE_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/bin)


if(NOT ITK_FOUND)
    find_package(ITK REQUIRED)
    include(${ITK_USE_FILE})
endif(NOT ITK_FOUND)

if(NOT VTK_FOUND)
    find_package(VTK REQUIRED)
    include(${VTK_USE_FILE})
endif(NOT VTK_FOUND)

##  In many cases stand-alone sub-projects include private versions
##  of DicomToNrrd
##  that can be built stand alone are combined in larger packages.
##  This logic will include DicomToNrrd only if it
##  has not already been included by a previous package.
if(NOT ADDFIRSTINSTANCE_DIRECTORY)
macro(ADDFIRSTINSTANCE_DIRECTORY PROJECT_NAMESPACE STANDALONENAME)
  if(BUILD${STANDALONENAME}FROM${PROJECT_NAMESPACE} OR NOT ${STANDALONENAME}_ALREADYINCLUDED)
    set(BUILD${STANDALONENAME}FROM${PROJECT_NAMESPACE} ON CACHE BOOL "FLAG FOR ${STANDALONENAME} building to prevent recursion.")
    set(${STANDALONENAME}_ALREADYINCLUDED ON CACHE BOOL "FLAG FOR ${STANDALONENAME} to indicate that it is already included.")
    add_subdirectory(${STANDALONENAME})
  endif( BUILD${STANDALONENAME}FROM${PROJECT_NAMESPACE} OR NOT ${STANDALONENAME}_ALREADYINCLUDED)
endmacro(ADDFIRSTINSTANCE_DIRECTORY)
endif(NOT ADDFIRSTINSTANCE_DIRECTORY)
###
ADDFIRSTINSTANCE_DIRECTORY(${LOCAL_PROJECT_NAME} BRAINSFit)

# use what QVTK built with
SET(QT_MOC_EXECUTABLE ${VTK_QT_MOC_EXECUTABLE} CACHE FILEPATH "")
SET(QT_UIC_EXECUTABLE ${VTK_QT_UIC_EXECUTABLE} CACHE FILEPATH "")
SET(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE} CACHE FILEPATH "")
SET(DESIRED_QT_VERSION ${VTK_DESIRED_QT_VERSION} CACHE FILEPATH "")

if(NOT Qt_FOUND)
FIND_PACKAGE( Qt REQUIRED )
IF(QT_USE_FILE)
  SET(QT_USE_QTXML TRUE)
  SET(QT_USE_DBUS FALSE)
  INCLUDE(${QT_USE_FILE})
ENDIF(QT_USE_FILE)
endif(NOT Qt_FOUND)

# Use the include path and library for Qt that is used by VTK.
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${VTK_INCLUDE_DIR}
  ${ITK_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)

# No need to add with nothing to compile. ADD_SUBDIRECTORY(BRAINSFit_Common)
##HACK:  
SET(BRAINSFit_SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/BRAINSFit)
INCLUDE_DIRECTORIES(${BRAINSFit_SOURCE_DIR})
INCLUDE_DIRECTORIES(${BRAINSFit_SOURCE_DIR}/BRAINSFit_Common)
INCLUDE_DIRECTORIES(${BRAINSFit_SOURCE_DIR}/LargestForegroundFilledMaskImageFilter)
INCLUDE_DIRECTORIES(${BRAINSFit_SOURCE_DIR}/FindCenterOfBrainFilter)


#ADD_SUBDIRECTORY(src)
#ADD_SUBDIRECTORY(gui)

SET( DTIPrep_SRCS
  src/main.cxx
  src/GMainWindow.cxx
  src/Dicom2NrrdPanel.cxx
  src/IntensityMotionCheckPanel.cxx
  src/ImageView2DPanelWithControls.cxx
  src/IntensityMotionCheck.cxx
  src/IntraGradientRigidRegistration.cxx
  src/InterGradientRigidRegistration.cxx
  src/RigidRegistration.cxx
  src/ThreadIntensityMotionCheck.cxx
  src/ThreadEddyMotionCorrect.cxx
  src/ThreadDicomToNrrd.cxx
  src/ThreadDicomSeriesRead.cxx
  src/XmlStreamReader.cpp
  src/XmlStreamWriter.cpp
  src/Protocol.cpp
  src/QCResult.cpp
# new itk Classes
  src/itkDWICropper.cpp
  src/itkDWIBaselineAverager.cpp
  src/itkDWIQCSliceChecker.cpp
  src/itkDWIQCInterlaceChecker.cpp
  src/itkDWIQCGradientChecker.cpp
  src/itkDWIEddyCurrentHeadMotionCorrector.cpp
# Utah
#  src/itkDWIHeadMotionEddyCurrentCorrection.txx
  src/itkDWIHeadMotionEddyCurrentCorrection.cpp
  src/itkGradientSteepestDescentBaseOptimizer.cxx
  src/itkGradientSteepestDescentOptimizer.cxx
# Iowa
  src/itkVectorImageRegisterAffineFilter.h
  src/itkVectorImageRegisterAffineFilter.txx
)

SET( DTIPrep_HRDS
  src/GMainWindow.h
  src/Dicom2NrrdPanel.h
  src/IntensityMotionCheckPanel.h
  src/ImageView2DPanelWithControls.h
  src/ThreadIntensityMotionCheck.h
  src/ThreadEddyMotionCorrect.h
  src/ThreadDicomToNrrd.h
  src/ThreadDicomSeriesRead.h
)

SET( DTIPrep_UIS
  gui/MainWindow.ui
  gui/Dicom2NrrdPanel.ui
  gui/IntensityMotionCheckPanel.ui
  gui/ImageView2DPanelWithControls.ui
)

SET(DTIPrep_RCCS gui/DTIPrep.qrc)

# generate rules for building source files from the resources
QT4_ADD_RESOURCES(RCC_SRCS ${DTIPrep_RCCS})

QT4_WRAP_UI( UIHeaders ${DTIPrep_UIS} )
QT4_WRAP_CPP( MOCSrcs ${DTIPrep_HRDS} )

ADD_DEFINITIONS(-DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT)

SET_SOURCE_FILES_PROPERTIES(${DTIPrep_SRCS} PROPERTIES
  OBJECT_DEPENDS "${UIHeaders}")

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

ADD_EXECUTABLE( DTIPrep
  MACOSX_BUNDLE
  ${DTIPrep_SRCS}
  ${UISrcs}
  ${MOCSrcs}
  ${RCC_SRCS}
)

TARGET_LINK_LIBRARIES( DTIPrep
  QVTK
  ${QT_LIBRARIES}
  ${QT_QTDBUS_LIBRARY}
  ${ITK_LIBRARIES}
  ${VTK_LIBRARIES}
  BRAINSFITCOMMONLIB
)


INSTALL(TARGETS
  DTIPrep
                 BUNDLE DESTINATION  bin
                 RUNTIME DESTINATION bin
                 LIBRARY DESTINATION lib
                 ARCHIVE DESTINATION lib/static)

