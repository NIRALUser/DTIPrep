cmake_minimum_required( VERSION 2.4 )

PROJECT ( DTIPrep )

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(gui)

FIND_PACKAGE( VTK REQUIRED )
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE( ITK REQUIRED )
INCLUDE(${ITK_USE_FILE})


# use what QVTK built with
SET(QT_MOC_EXECUTABLE ${VTK_QT_MOC_EXECUTABLE} CACHE FILEPATH "")
SET(QT_UIC_EXECUTABLE ${VTK_QT_UIC_EXECUTABLE} CACHE FILEPATH "")
SET(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE} CACHE FILEPATH "")
SET(DESIRED_QT_VERSION ${VTK_DESIRED_QT_VERSION} CACHE FILEPATH "")

FIND_PACKAGE( Qt REQUIRED )
IF(QT_USE_FILE)
  INCLUDE(${QT_USE_FILE})
  SET(QT_USE_QTXML 1)
ELSE(QT_USE_FILE)
  SET(QT_LIBRARIES   ${QT_QT_LIBRARY})
ENDIF(QT_USE_FILE)


# Use the include path and library for Qt that is used by VTK.
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${VTK_INCLUDE_DIR}
  ${ITK_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


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
  src/Protocal.cpp
  src/QCResult.cpp
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
#	VTKWIDGETS
	${QT_LIBRARIES} 
#	${QT_QTNETWORK_LIBRARIES}
	${ITK_LIBRARIES}
	${VTK_LIBRARIES}
)


