#include "ImageView2DPanel.h"

#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include "vtkCylinderSource.h"
#include <vtkPolyDataMapper.h>
#include <vtkImageViewer2.h>
#include <vtkCamera.h>

#include <itkImage.h>
#include <itkImageFileReader.h>


#include "itkVTKImageExport.h"
#include "vtkImageImport.h"
#include "vtkImageData.h"
#include "itkImageToVTKImageFilter.h"


ImageView2DPanel::ImageView2DPanel(QString title, QMainWindow *parent):QDockWidget(title, parent)
{
  setupUi(this);
  this->verticalLayout->setContentsMargins(0,1,0,0);
  this->verticalLayout->setSpacing(1);
  this->setWindowTitle(title);
 
	  // QT/VTK interact
  ren = vtkRenderer::New();
  qvtkWidget->GetRenderWindow()->AddRenderer(ren);

  // Geometry
  source = vtkCylinderSource::New();

  // Mapper
  mapper = vtkPolyDataMapper::New();
  mapper->ImmediateModeRenderingOn();
  mapper->SetInputConnection(source->GetOutputPort());

  // Actor in scene
  actor = vtkActor::New();
  actor->SetMapper(mapper);

  // Add Actor to renderer
  ren->AddActor(actor);

  // Reset camera
  ren->ResetCamera();

  //ren->GetRenderWindow()->Render();

  /*
  ////////////////////////////////////////
  	vtkRenderWindowInteractor* renderWindowInteractorT =vtkRenderWindowInteractor::New();
	this->ImageViewer2=vtkImageViewer2::New();
	this->ImageViewer2->SetRenderWindow(qvtkWidget->GetRenderWindow());
	this->ImageViewer2->SetupInteractor(renderWindowInteractorT );
	//this->ImageViewer2->SetInput( this->Gradient);

	typedef short PixelType;
	typedef itk::Image<PixelType, 3> ImageType;
	typedef itk::ImageFileReader<ImageType> ReaderType;
	ReaderType::Pointer reader = ReaderType::New();

	//reader->SetFileName("d:\\DTI3_256x256x159_0.mha");
	//reader->Update();

	typedef itk::ImageToVTKImageFilter<	ImageType>	ItkVtkImageFilterTypeShort;
	ItkVtkImageFilterTypeShort::Pointer connecter = ItkVtkImageFilterTypeShort::New();
	connecter->SetInput( reader->GetOutput());
	//this->ImageViewer2->SetInput( connecter->GetOutput());
	this->ImageViewer2->SetSliceOrientationToXY();

	renderWindowInteractorT->Initialize();

	this->ImageViewer2->SetSlice(20);
	//if(ww!=0 && wl!=0)
	//{
		//this->ImageViewer2->SetColorWindow( 2000 );
		//this->ImageViewer2->SetColorLevel( 1000 );
	//}

	//CRect rect;
		
	this->ImageViewer2->SetSize(this->qvtkWidget->GetRenderWindow()->GetSize()[0],this->qvtkWidget->GetRenderWindow()->GetSize()[1]);
	this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetFocalPoint(0,0,0);
	this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetPosition(0,0,-1); // -1 if medical ?
	this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetViewUp(0,-1,0);
	this->ImageViewer2->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();


	this->ImageViewer2->GetRenderer()->ResetCamera();
	this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetParallelScale(-2 );
	this->ImageViewer2->Render();
*/

}

ImageView2DPanel::~ImageView2DPanel(void)
{
}
