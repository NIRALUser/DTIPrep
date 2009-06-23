#ifndef GMAINWINDOW_H
#define GMAINWINDOW_H

#include "qmainwindow.h"
#include "ui_MainWindow.h"

// Forward class declarations
class vtkCylinderSource;
class vtkPolyDataMapper;
class vtkExodusReader;
class vtkDataSetMapper;
class vtkActor;
class vtkRenderer;

class Dicom2NrrdPanel;
class DiffusionCheckPanel;
class DiffusionEditPanel;
class DTIEstimatePanel;
class EddyMotionCorrectPanel;
class ImageView2DPanel;
class IntensityMotionCheckPanel;
class ImageView2DPanelWithControls;

class QProgressBar;
class QActionGroup;
class QAction;
class QActionGroup;
class QLabel;
class QMenu;

#include <itkImage.h>
#include <itkImageFileReader.h>


#include "itkVTKImageExport.h"
#include "vtkImageImport.h"
#include "vtkImageData.h"
#include "itkImageToVTKImageFilter.h"
#include "vtkImagePlaneWidget.h"
#include "vtkCommand.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

#include "vtkSphereSource.h"
#include "vtkLineSource.h"
#include "vtkPropAssembly.h"
#include "vtkCylinderSource.h"
#include "vtkTubeFilter.h"
#include "vtkLight.h"



class QStyleFactory;

class vtkEventQtSlotConnect;
class vtkObject;
class vtkCommand;


class GMainWindow : public QMainWindow, private Ui_MainWindow
{
    Q_OBJECT

public:
    GMainWindow();
	~GMainWindow();
	QProgressBar* GatProgressWidget(){return progressWidget;};

	void ChangeStyleTo(QString style);

private slots:
	void save();
	void print();
	void about();


	// open menu
	void on_actionOpenDWINrrd_triggered();
	void on_actionOpen_XML_triggered();
	void on_actionOpen_QC_Report_triggered();

	// styles menu
	void on_actionWindows_triggered();
	void on_actionWindowsXP_triggered();
	void on_actionMotif_triggered();
	void on_actionCDE_triggered();
	void on_actionPlastique_triggered();
	void on_actionCleanlooks_triggered();

	// vector view
	void on_actionFrom_Protocol_toggled( bool);
	void on_actionFrom_DWI_toggled( bool);
	void on_actionIncluded_toggled( bool);
	void on_actionExcluded_toggled( bool);
	void on_actionSphere_toggled( bool);

	void on_actionExit_triggered();
	void UpdateProgressbar(int pos);

	// 2D sliderbar
	void ImageIndexChanged(int winID, int index);	
	void OrientationChanged(int winID, int newOrient);
	void OpacityChanged(int WinID,  double opacity);
	void VisibleChanged(int WinID, bool visible);
	void GradientChanged(int WinID, int index);
	void InterpolationChanged(int WinID, int index);
	void ContentsChanged(int WinID, int index);

	// syn for 3-imagePlanes
	void WindowLevelSyn(bool syn);
	void ContentSyn(bool syn);
	void InterpolationSyn(bool synx);
	void OrientationSyn(bool syn);



	// Qt-vtk connections
	void BackGroundColor(QAction*);
	void popup(vtkObject * obj, unsigned long, void * client_data, void *,vtkCommand * command);

	void WindowLevelChanged(vtkObject * obj, unsigned long, void * client_data, void *,vtkCommand * command);

//
	void SetAllWindowLevel(double window, double level);

	

private:
	bool bDwiLoaded;//=false;

	bool bWindowLevelSyn; 	//=true;
	bool bContentSyn;	//=true;
	bool bInterpolationSyn;	//=true;
	bool bOrientationSyn;	//=false;

// 3D window
	vtkActor *actorSphere;
	vtkPropAssembly *actorDirProtocol;
	vtkPropAssembly *actorDirFile;
	vtkPropAssembly *actorDirInclude;
	vtkPropAssembly *actorDirExclude;


// docking panels
	Dicom2NrrdPanel		    *dicom2NrrdPanel;
	DiffusionCheckPanel		*diffusionCheckPanel;
	DiffusionEditPanel		*diffusionEditPanel;
	DTIEstimatePanel		*dTIEstimatePanel;
	EddyMotionCorrectPanel	*eddyMotionCorrectPanel;
	IntensityMotionCheckPanel	*DTIPrepPanel;

//docking 2D image planes
//	ImageView2DPanel		*imageView2DPanel1;
//	ImageView2DPanel		*imageView2DPanel2;
//	ImageView2DPanel		*imageView2DPanel3;

	ImageView2DPanelWithControls *imageView2DPanelWithControls1;
	ImageView2DPanelWithControls *imageView2DPanelWithControls2;
	ImageView2DPanelWithControls *imageView2DPanelWithControls3;

    void createActions();
    void createStatusBar();
    void createDockPanels();
  
    QToolBar *panelToolBar;
    QToolBar *cameraToolBar;
    QToolBar *View3DToolBar;

    QAction *saveAct;
    QAction *printAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *quitAct;

    QActionGroup *styleGroup;

    vtkCylinderSource* source;
    vtkPolyDataMapper* mapper;
    vtkActor* actor;
    vtkRenderer* ren;
	//vtkRenderer* ren1;

	std::vector<vtkRenderer *> renders;

	QProgressBar* progressWidget;

public:
//////////////////// nrrd DWI reader /////////////////////////////////////////////////////
	typedef unsigned short DwiPixelType;
	typedef itk::Image<DwiPixelType, 2>			SliceImageType;
	typedef itk::Image<DwiPixelType, 3>			GradientImageType;
	typedef itk::VectorImage<DwiPixelType, 3>	DwiImageType;    
	typedef itk::ImageFileReader<DwiImageType>	DwiReaderType;

	//itk::NrrdImageIO::Pointer  NrrdImageIO 

	DwiImageType::Pointer DWIImage;
	DwiReaderType::Pointer DwiReader;

	typedef itk::ImageToVTKImageFilter<	GradientImageType>	ItkVtkImageFilterTypeUShort;
	ItkVtkImageFilterTypeUShort::Pointer gradientConnecter;

	typedef itk::VectorIndexSelectionCastImageFilter<DwiImageType, GradientImageType > FilterType;
	FilterType::Pointer componentExtractor;


	FilterType::Pointer componentExtractor1;
	FilterType::Pointer componentExtractor2;
	FilterType::Pointer componentExtractor3;

	ItkVtkImageFilterTypeUShort::Pointer gradientConnecter1;
	ItkVtkImageFilterTypeUShort::Pointer gradientConnecter2;
	ItkVtkImageFilterTypeUShort::Pointer gradientConnecter3;

	// for 3D Image
	vtkRenderer		*pvtkRenderer;
	vtkImagePlaneWidget	*planeWidgetX;
	vtkImagePlaneWidget	*planeWidgetY;
	vtkImagePlaneWidget	*planeWidgetZ;

	// for 3D view
	vtkRenderer		*pvtkRenderer_3DView;


public:
	bool CreateImagePlaneWidgets( vtkImageData *GradientImage );
	bool CreateImagePlaneWidgets( vtkImageData *GradientImage1, vtkImageData *GradientImage2, vtkImageData *GradientImage3);

	void UpdateImagePlaneWidgets(int gradient);
	void UpdateImagePlaneWidgets(int gradient1, int gradient2, int gradient3);

	void UpdateImageView2DWindows(int gradient, int numbGradients);
	void UpdateImageView2DWindows(int gradient1, int gradient2, int gradient3, int numbGradients);


	vtkRenderer*	      GetRenderer(){return pvtkRenderer;};
	vtkImagePlaneWidget * GetplaneWidgetX(){ return planeWidgetX;};
	vtkImagePlaneWidget * GetplaneWidgetY(){ return planeWidgetY;};
	vtkImagePlaneWidget * GetplaneWidgetZ(){ return planeWidgetZ;};

private:
	vtkImageData* image1;
	vtkImageData* image2;
	vtkImageData* image3;

	// vtkQtConnection
	vtkEventQtSlotConnect* vtkQtConnections;

	//	vtkIPWCallbackUpdateImage2D* IPWCallbackUpdateImage2D;
	int whichWindow[3];

	void ProbeWithSplineWidget();

};

#endif // GMAINWINDOW_H


