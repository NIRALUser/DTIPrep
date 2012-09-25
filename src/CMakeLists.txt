#ADD_SUBDIRECTORY(src)
#ADD_SUBDIRECTORY(gui)

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

include_directories(${CMAKE_BINARY_DIR}/src)
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

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
  FurtherQCThread.cxx
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
# MIT
 itkMultiImageRegistrationFilter.cxx
 GradientDescentLineSearchOptimizer.cxx
 DominantDirectional/DominantDirectional_Detector.cxx
 DominantDirectional/Counter.cc
 DominantDirectional/Point.cc
 DominantDirectional/Sphere.cc
 DominantDirectional/Triangle.cc
 DominantDirectional/Vector.cc
 DominantDirectional/Geometry_Common.cc
)

generateclp(DTIPrep_SRCS DTIPrep.xml )

SET( DTIPrep_MOC_HDRS
  GMainWindow.h
  Dicom2NrrdPanel.h
  IntensityMotionCheckPanel.h
  ImageView2DPanelWithControls.h
  ThreadIntensityMotionCheck.h
  ThreadEddyMotionCorrect.h
  ThreadDicomToNrrd.h
  ThreadDicomSeriesRead.h
  FurtherQCThread.h
  #Combo_Box.h
)

SET( DTIPrep_HDRS
  DominantDirectional/DominantDirectional_Detector.h
  DominantDirectional/Counter.h
  DominantDirectional/SphereIkosahedron.h
  DominantDirectional/Point.h
  DominantDirectional/Sphere.h
  DominantDirectional/Triangle.h
  DominantDirectional/Vector.h
  DominantDirectional/Geometry_Common.h
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
QT4_WRAP_CPP( MOCSrcs ${DTIPrep_MOC_HDRS} )

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
  ModuleDescriptionParser
  QVTK
  ${QT_LIBRARIES}
  ${QT_QTDBUS_LIBRARY}
  ${ITK_LIBRARIES}
  ${VTK_LIBRARIES}
  BRAINSCommonLib
)


INSTALL(TARGETS
  DTIPrep
                 BUNDLE DESTINATION  bin
                 RUNTIME DESTINATION bin
                 LIBRARY DESTINATION lib
                 ARCHIVE DESTINATION lib/static)

SET(plugin_dest_dir bin)
SET(qtconf_dest_dir bin)
SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/DTIPrep")
IF(APPLE)
  SET(plugin_dest_dir DTIPrep.app/Contents/MacOS)
  SET(qtconf_dest_dir DTIPrep.app/Contents/Resources)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/DTIPrep.app")
ENDIF(APPLE)
IF(WIN32)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/DTIPrep.exe")
ENDIF(WIN32)

#--------------------------------------------------------------------------------
# Install needed Qt plugins by copying directories from the qt installation
# One can cull what gets copied by using 'REGEX "..." EXCLUDE'
#INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime)

if(APPLE)
#--------------------------------------------------------------------------------
# install a qt.conf file
# this inserts some cmake code into the install script to write the file
INSTALL(CODE "
    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")
    " COMPONENT Runtime)
INSTALL(CODE "
    execute_process(COMMAND cp -r \"${QT_LIBRARY_DIR}/QtGui.framework/Resources/qt_menu.nib\" \".\" WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir})")
endif(APPLE)
#--------------------------------------------------------------------------------
# Use BundleUtilities to get all other dependencies for the application to work.
# It takes a bundle or executable along with possible plugins and inspects it
# for dependencies.  If they are not system dependencies, they are copied.

SET(DIRS
${QT_LIBRARY_DIRS}
${ITK_DIR}
${VTK_DIR}
${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib
)

# Now the work of copying dependencies into the bundle/package
# The quotes are escaped and variables to use at install time have their $ escaped
# An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
# Note that the image plugins depend on QtSvg and QtXml, and it got those copied
# over.
INSTALL(CODE "
   include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${DIRS}\")
    " COMPONENT Runtime)


# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# where CPackConfig.cmake is created by including CPack
# And then there's ways to customize this as well
set(CPACK_BINARY_DRAGNDROP ON)
include(CPack)

