project(DTIPrepTools)
set(LOCAL_PROJECT_NAME DTIPrepTools)
cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

enable_testing()
include(Dart)
include(CPack)

if(NOT ITK_FOUND)
    find_package(ITK REQUIRED)
    include(${ITK_USE_FILE})
endif(NOT ITK_FOUND)

if(NOT VTK_FOUND)
    find_package(VTK REQUIRED)
    include(${VTK_USE_FILE})
endif(NOT VTK_FOUND)

find_package(BRAINSCommonLib NO_MODULE REQUIRED)
include(${BRAINSCommonLib_USE_FILE})

find_package(MultiImageRegistration NO_MODULE REQUIRED)
include(${MultiImageRegistration_USE_FILE})

# use what QVTK built with
#SET(QT_MOC_EXECUTABLE ${VTK_QT_MOC_EXECUTABLE} CACHE FILEPATH "")
#SET(QT_UIC_EXECUTABLE ${VTK_QT_UIC_EXECUTABLE} CACHE FILEPATH "")
#SET(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE} CACHE FILEPATH "")
#SET(DESIRED_QT_VERSION ${VTK_DESIRED_QT_VERSION} CACHE FILEPATH "")

if(NOT Qt_FOUND)
  find_package( Qt4 REQUIRED )
  if(QT_USE_FILE)
#    set(QT_USE_QTXML TRUE)
#    set(QT_USE_DBUS FALSE)
    include(${QT_USE_FILE})
  endif(QT_USE_FILE)
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
INCLUDE_DIRECTORIES(${CMAKE_INSTALL_PREFIX}/MultiImageRegistration/Source)
INCLUDE_DIRECTORIES(${CMAKE_INSTALL_PREFIX}/MultiImageRegistration/Source/Common)

#ADD_SUBDIRECTORY(src)
#ADD_SUBDIRECTORY(gui)

SET( DTIPrep_SRCS
  main.cxx
  GMainWindow.cxx
  Dicom2NrrdPanel.cxx
  IntensityMotionCheckPanel.cxx
  ImageView2DPanelWithControls.cxx
  IntensityMotionCheck.cxx
  IntraGradientRigidRegistration.cxx
  InterGradientRigidRegistration.cxx
  RigidRegistration.cxx
  ThreadIntensityMotionCheck.cxx
  ThreadEddyMotionCorrect.cxx
  ThreadDicomToNrrd.cxx
  ThreadDicomSeriesRead.cxx
  XmlStreamReader.cpp
  XmlStreamWriter.cpp
  Protocol.cpp
  QCResult.cpp
# new itk Classes
  itkDWICropper.cpp
  itkDWIBaselineAverager.cpp
  itkDWIQCSliceChecker.cpp
  itkDWIQCInterlaceChecker.cpp
  itkDWIQCGradientChecker.cpp
  itkDWIEddyCurrentHeadMotionCorrector.cpp
# Utah
#  itkDWIHeadMotionEddyCurrentCorrection.txx
  itkDWIHeadMotionEddyCurrentCorrection.cpp
  itkGradientSteepestDescentBaseOptimizer.cxx
  itkGradientSteepestDescentOptimizer.cxx
# Iowa
  itkVectorImageRegisterAffineFilter.h
  itkVectorImageRegisterAffineFilter.txx
)

SET( DTIPrep_HRDS
  GMainWindow.h
  Dicom2NrrdPanel.h
  IntensityMotionCheckPanel.h
  ImageView2DPanelWithControls.h
  ThreadIntensityMotionCheck.h
  ThreadEddyMotionCorrect.h
  ThreadDicomToNrrd.h
  ThreadDicomSeriesRead.h
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

#ADD_DEFINITIONS(-DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT)

SET_SOURCE_FILES_PROPERTIES(${DTIPrep_SRCS} PROPERTIES
  OBJECT_DEPENDS "${UIHeaders}")

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
  BRAINSCommonLib
  MultiImageRegistrationLib
)


INSTALL(TARGETS
  DTIPrep
                 BUNDLE DESTINATION  bin
                 RUNTIME DESTINATION bin
                 LIBRARY DESTINATION lib
                 ARCHIVE DESTINATION lib/static)

