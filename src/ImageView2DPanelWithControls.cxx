#include "ImageView2DPanelWithControls.h"

#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include "vtkCylinderSource.h"
#include <vtkPolyDataMapper.h>
#include <vtkImagePlaneWidget.h>
// #include <vtkImageViewer2.h>
#include <vtkCamera.h>

#include <itkImage.h>
#include <itkImageFileReader.h>

#include "itkVTKImageExport.h"
#include "vtkImageImport.h"
#include "vtkImageData.h"
#include "itkImageToVTKImageFilter.h"

#include <time.h>

#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>
#include <QtGui/QToolButton>

ImageView2DPanelWithControls::ImageView2DPanelWithControls(QString title, QMainWindow *parentLocal) : QDockWidget(title,
                                                                                                                  parentLocal)
{
  setupUi(this);
  this->verticalLayout->setContentsMargins(0, 1, 0, 0);
  this->verticalLayout->setSpacing(1);
  this->setWindowTitle(title);

  bSetup = false;

  Orientation = 0;

  this->m_ImageDimension[0] = 0;
  this->m_ImageDimension[1] = 0;
  this->m_ImageDimension[2] = 0;

  this->m_ImageSpacing[0] = 0.0;
  this->m_ImageSpacing[1] = 0.0;
  this->m_ImageSpacing[2] = 0.0;

  NumberOfGradients = 0;

  // QProgressBar* progressWidget = new QProgressBar(this);
  // progressWidget->setRange(0,100);
  // progressWidget->setValue(0);
  // progressWidget->setAlignment(Qt::AlignCenter);
  // progressWidget->setMaximumWidth(200);//
  // QComboBox* combo = new QComboBox(0);
  // combo->setMaximumSize(QSize(16777215, 15));
  // combo->setStyleSheet("color: blue; background-color: yellow");
  // QLineEdit* LineEdit = new QLineEdit(this);
  // this->setTitleBarWidget (combo );
  // this->setStyleSheet("color: blue; background-color: yellow");

  /*
  void QDockWidget::setTitleBarWidget ( QWidget * widget )
  Sets an arbitrary widget as the dock widget's title bar. If widget is 0, the title bar widget is removed, but not deleted.
  If a title bar widget is set, QDockWidget will not use native window decorations when it is floated.
  Here are some tips for implementing custom title bars:
  Mouse events that are not explicitly handled by the title bar widget must be ignored by calling QMouseEvent::ignore(). These events then propagate to the QDockWidget parentLocal, which handles them in the usual manner, moving when the title bar is dragged, docking and undocking when it is double-clicked, etc.
  When DockWidgetVerticalTitleBar is set on QDockWidget, the title bar widget is repositioned accordingly. In resizeEvent(), the title bar should check what orientation it should assume:
          QDockWidget *dockWidget = qobject_cast<QDockWidget*>(parentWidget());
          if (dockWidget->features() & QDockWidget::DockWidgetVerticalTitleBar) {
              // I need to be vertical
          } else {
              // I need to be horizontal
          }
  The title bar widget must have a valid QWidget::sizeHint() and QWidget::minimumSizeHint(). These functions should take into account the current orientation of the title bar.
  Using qobject_cast as shown above, the title bar widget has full access to its parentLocal QDockWidget. Hence it can perform such operations as docking and hiding in response to user actions.
  This function was introduced in Qt 4.3.
  See also titleBarWidget() and DockWidgetVerticalTitleBar.


  */

  // QT/VTK interact
  ren = vtkRenderer::New();
  qvtkWidget->GetRenderWindow()->AddRenderer(ren);

  vtkImage = vtkImageData::New();
  this->ImageViewer2 = vtkImageViewer2::New();

  /*
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
  */
  // Reset camera
  ren->ResetCamera();

  // ren->GetRenderWindow()->Render();

  /*
    typedef short PixelType;
    typedef itk::Image<PixelType, 3> ImageType;
    typedef itk::ImageFileReader<ImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();

    reader->SetFileName("d:\\DTI3_256x256x159_0.mha");
    reader->Update();

    typedef itk::ImageToVTKImageFilter<  ImageType>  ItkVtkImageFilterTypeShort;
    ItkVtkImageFilterTypeShort::Pointer connecter = ItkVtkImageFilterTypeShort::New();
    connecter->SetInput( reader->GetOutput());

      */
}

ImageView2DPanelWithControls::~ImageView2DPanelWithControls(void)
{
}

void ImageView2DPanelWithControls::Setup( vtkImageData *image,
                                          double ww,
                                          double wl,
                                          int SliceIndex,
                                          int orient,
                                          int numGradients,
                                          int id)
{
  if( id >= 0 && id <= 2 )
    {
    WindowID = id;
    }
  else
    {
    WindowID = 0;
    }

  vtkImage = image;
  Orientation = orient;
  NumberOfGradients = numGradients;

  // int this->m_ImageDimension[3];  // I believe that the setup and update functions should modify the class member
  // variable not a private version.
  // double this->m_ImageSpacing[3];
  vtkImage->GetDimensions(this->m_ImageDimension);
  vtkImage->GetSpacing(this->m_ImageSpacing);

  // std::cout<<this->m_ImageDimension[0]<<"    "<< this->m_ImageDimension[1]<<"
  //     "<<this->m_ImageDimension[2]<<std::endl;
  // std::cout<<this->m_ImageSpacing[0]<<"    "<< this->m_ImageSpacing[1]<<"
  //     "<<this->m_ImageSpacing[2]<<std::endl;
  this->ImageViewer2->SetRenderWindow( qvtkWidget->GetRenderWindow() );
  this->ImageViewer2->SetupInteractor(
    qvtkWidget->GetRenderWindow()->GetInteractor() );

  this->ImageViewer2->SetInput( vtkImage );
  // this->ImageViewer2->GetImageActor()->SetOpacity(0.2);

  int index;
  index = SliceIndex;

  this->comboBox_Contents->setCurrentIndex(0);

  this->horizontalSlider_Gradient->setRange(0, NumberOfGradients - 1);
  QString temp;
  this->lineEdit_Gradient->setText( temp.sprintf("%d", 0) );
  this->horizontalSlider_Gradient->setValue(0);

  switch( Orientation )
    {
    case ORIENTATION_AXIAL:
      if( index < 0 )
        {
        index = this->m_ImageDimension[2] / 2;
        }
      this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetPosition(0,
                                                                        0,
                                                                        -1);     //
                                                                                 //
                                                                                 // -1
                                                                                 //
                                                                                 // if
                                                                                 //
                                                                                 // medical
                                                                                 //
                                                                                 // ?
      this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetViewUp(0, -1, 0);
      this->horizontalSlider_SliceIndex->setRange(0, this->m_ImageDimension[2] - 1);
      this->ImageViewer2->SetSliceOrientationToXY();

      break;
    case ORIENTATION_SAGITTAL:
      this->ImageViewer2->SetSliceOrientationToYZ();
      if( index < 0 )
        {
        index = this->m_ImageDimension[0] / 2;
        }
      this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetPosition(-1,
                                                                        0,
                                                                        0);      //
                                                                                 //
                                                                                 // -1
                                                                                 //
                                                                                 // if
                                                                                 //
                                                                                 // medical
                                                                                 //
                                                                                 // ?
      this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetViewUp(0, 0, 1);
      this->horizontalSlider_SliceIndex->setRange(0, this->m_ImageDimension[0] - 1);

      break;
    case ORIENTATION_CORONAL:
      this->ImageViewer2->SetSliceOrientationToXZ();
      if( index < 0 )
        {
        index = this->m_ImageDimension[1] / 2;
        }
      this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetPosition(0,
                                                                        -1,
                                                                        0);      //
                                                                                 //
                                                                                 // 1
                                                                                 //
                                                                                 // if
                                                                                 //
                                                                                 // medical
                                                                                 //
                                                                                 // ?
      this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetViewUp(0, 0, 1);
      this->horizontalSlider_SliceIndex->setRange(0, this->m_ImageDimension[1] - 1);

      break;
    default:
      break;
    }
  this->ImageViewer2->SetSlice(index);

  QString str;
  this->lineEdit_SliceIndex->setText( str.sprintf("%d", index) );
  this->horizontalSlider_SliceIndex->setValue(index);

  if( ww != 0 && wl != 0 )
    {
    this->ImageViewer2->SetColorWindow( ww );
    this->ImageViewer2->SetColorLevel( wl );
    }
  else
    {
    double *range = this->ImageViewer2->GetInput()->GetScalarRange();
    this->ImageViewer2->SetColorWindow(range[1] - range[0]);
    this->ImageViewer2->SetColorLevel( 0.5 * ( range[1] + range[0] ) );
    }
  float scale;
  if( this->m_ImageDimension[0] != 0 && this->m_ImageDimension[1] != 0 && this->m_ImageDimension[2] !=
      0 )
    {
    float scaleTemp[3];
    if( this->m_ImageDimension[0] * this->m_ImageSpacing[0] >= this->m_ImageDimension[1]
        * this->m_ImageSpacing[1] )
      {
      scaleTemp[0] = this->m_ImageDimension[0] * this->m_ImageSpacing[0] / 2.0;
      }
    else
      {
      scaleTemp[0] = this->m_ImageDimension[1] * this->m_ImageSpacing[1] / 2.0;
      }

    if( this->m_ImageDimension[0] * this->m_ImageSpacing[0] >= this->m_ImageDimension[2]
        * this->m_ImageSpacing[2] )
      {
      scaleTemp[1] = this->m_ImageDimension[0] * this->m_ImageSpacing[0] / 2.0;
      }
    else
      {
      scaleTemp[1] = this->m_ImageDimension[2] * this->m_ImageSpacing[2] / 2.0;
      }

    if( this->m_ImageDimension[1] * this->m_ImageSpacing[1] >= this->m_ImageDimension[2]
        * this->m_ImageSpacing[2] )
      {
      scaleTemp[2] = this->m_ImageDimension[1] * this->m_ImageSpacing[1] / 2.0;
      }
    else
      {
      scaleTemp[2] = this->m_ImageDimension[2] * this->m_ImageSpacing[2] / 2.0;
      }

    if( scaleTemp[0] >= scaleTemp[1] && scaleTemp[0] >= scaleTemp[2] )
      {
      scale = scaleTemp[0]; // ((float)(this->m_ImageDimension[0]-1))/2.0;
      }
    else if( scaleTemp[1] >= scaleTemp[0] && scaleTemp[1] >= scaleTemp[2] )
      {
      scale = scaleTemp[1]; // ((float)(this->m_ImageDimension[1]-1))/2.0;
      }
    else // (dims[2]>=dims[0] && dims[2]>=dims[1])
      {
      scale = scaleTemp[2]; // ((float)(this->m_ImageDimension[2]-1))/2.0;
      }
    }
  else
    {
    // Setting scale to a default value if not previously set.  This may not be
    // the desired behavior, but scale must be set
    // in order to use it below.
    scale = 0;
    }

  this->ImageViewer2->SetSize(
    this->qvtkWidget->GetRenderWindow()->GetSize()[0],
    this->qvtkWidget->GetRenderWindow()->GetSize()[1]);
  this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetFocalPoint(0, 0, 0);
  this->ImageViewer2->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();
  this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetParallelScale(scale );

  this->ImageViewer2->GetImageActor()->InterpolateOff();

  this->ImageViewer2->GetRenderer()->ResetCamera();
  this->ImageViewer2->Render();
  qvtkWidget->GetRenderWindow()->GetInteractor()->Initialize();

  this->comboBox_Orientation->setCurrentIndex(Orientation);

  bSetup = true;
}

void ImageView2DPanelWithControls::Update()
{
  // int this->m_ImageDimension[3];  // I believe that the setup and update functions should modify the class member
  // variable not a private version.
  // double this->m_ImageSpacing[3];

  vtkImage->GetDimensions(this->m_ImageDimension);
  vtkImage->GetSpacing(this->m_ImageSpacing);

  std::cout << this->m_ImageDimension[0] << " ddd   " << this->m_ImageDimension[1]
            << "  dd   " << this->m_ImageDimension[2] << std::endl;
  std::cout << this->m_ImageSpacing[0] << "   dd " << this->m_ImageSpacing[1] << "  dd   "
            << this->m_ImageSpacing[2] << std::endl;
  int index = 0;

  switch( Orientation )
    {
    case ORIENTATION_AXIAL:
      if( index < 1 )
        {
        index = this->m_ImageDimension[2] / 2;
        }
      this->horizontalSlider_SliceIndex->setRange(0, this->m_ImageDimension[2] - 1);
      break;
    case ORIENTATION_SAGITTAL:
      this->horizontalSlider_SliceIndex->setRange(0, this->m_ImageDimension[0] - 1);
      if( index < 1 )
        {
        index = this->m_ImageDimension[0] / 2;
        }
      break;
    case ORIENTATION_CORONAL:
      if( index < 1 )
        {
        index = this->m_ImageDimension[1] / 2;
        }
      this->horizontalSlider_SliceIndex->setRange(0, this->m_ImageDimension[1] - 1);
      break;
    default:
      index = 0;
      break;
    }

  this->ImageViewer2->SetSlice(index);

  QString str;
  this->lineEdit_SliceIndex->setText( str.sprintf("%d", index) );
  this->horizontalSlider_SliceIndex->setValue(index);

  // this->ImageViewer2->Render();
}

void ImageView2DPanelWithControls::on_horizontalSlider_SliceIndex_valueChanged(
  int index)
{
  QString str;

  // this->lineEdit_SliceIndex->setText(str.sprintf("%d/%d", index,
  // horizontalSlider_SliceIndex->maximum()));
  if( this->m_ImageDimension[2] == 0 )
    {
    return;
    }

  this->lineEdit_SliceIndex->setText( str.sprintf("%d", index) );
  this->ImageViewer2->SetSlice(index);
  emit indexchanged(WindowID, index);
}

void ImageView2DPanelWithControls::on_toolButton_Play_clicked()
{
  if( this->m_ImageDimension[2] == 0 )
    {
    return;
    }

  int originalSlice;
  originalSlice = this->horizontalSlider_SliceIndex->value();

  switch( Orientation )
    {
    case ORIENTATION_AXIAL:
      for( int i = 0; i < this->m_ImageDimension[2]; i++ )
        {
        clock_t start;
        start = clock();
        while( true )
          {
          if( ( (float)(clock() - start) ) / ( (float)CLOCKS_PER_SEC ) > 0.01 )
            {
            this->horizontalSlider_SliceIndex->setValue(i);
            break;
            }
          }

        // for(int j=1;j<1000000;j++){;}
        // this->horizontalSlider_SliceIndex->setValue(i);
        }
      break;
    case ORIENTATION_SAGITTAL:
      for( int i = 0; i < this->m_ImageDimension[0]; i++ )
        {
        clock_t start;
        start = clock();
        while( true )
          {
          if( ( (float)(clock() - start) ) / ( (float)CLOCKS_PER_SEC ) > 0.01 )
            {
            this->horizontalSlider_SliceIndex->setValue(i);
            break;
            }
          }

        // for(int j=1;j<1000000;j++){;}
        // this->horizontalSlider_SliceIndex->setValue(i);
        }
      break;
    case ORIENTATION_CORONAL:
      for( int i = 0; i < this->m_ImageDimension[1]; i++ )
        {
        clock_t start;
        start = clock();
        while( true )
          {
          if( ( (float)(clock() - start) ) / ( (float)CLOCKS_PER_SEC ) > 0.01 )
            {
            this->horizontalSlider_SliceIndex->setValue(i);
            break;
            }
          }

        // for(int j=1;j<1000000;j++){;}
        // this->horizontalSlider_SliceIndex->setValue(i);
        }
      break;
    default:
      break;
    }

  this->horizontalSlider_SliceIndex->setValue(originalSlice);
}

void ImageView2DPanelWithControls::on_toolButton_GradientPlay_clicked()
{
  if( NumberOfGradients == 0 )
    {
    return;
    }

  int originalGradient;
  originalGradient = this->horizontalSlider_Gradient->value();
  for( int i = 0; i < NumberOfGradients; i++ )
    {
    clock_t start;
    start = clock();
    while( true )
      {
      if( ( (float)(clock() - start) ) / ( (float)CLOCKS_PER_SEC ) > 0.05 )
        {
        this->horizontalSlider_Gradient->setValue(i);
        break;
        }
      // for(int j=1;j<10000000;j++){;}
      // this->horizontalSlider_Gradient->setValue(i);
      }
    }
  this->horizontalSlider_Gradient->setValue(originalGradient);
}

void ImageView2DPanelWithControls::on_comboBox_Orientation_currentIndexChanged(
  int indexOrient)
{
  if( Orientation == indexOrient )
    {
    return;
    }
  if( !bSetup )
    {
    return;
    }

  Orientation = indexOrient;

  emit orientationchanged(WindowID, Orientation);

  int index = 0;

  switch( Orientation )
    {
    case ORIENTATION_AXIAL:
      this->ImageViewer2->SetSliceOrientationToXY();
      this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetPosition(0,
                                                                        0,
                                                                        -1);     //
                                                                                 //
                                                                                 // -1
                                                                                 //
                                                                                 // if
                                                                                 //
                                                                                 // medical
                                                                                 //
                                                                                 // ?
      this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetViewUp(0, -1, 0);
      this->horizontalSlider_SliceIndex->setRange(0, this->m_ImageDimension[2] - 1);
      index = this->m_ImageDimension[2] / 2;
      break;
    case ORIENTATION_SAGITTAL:
      this->ImageViewer2->SetSliceOrientationToYZ();
      this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetPosition(-1,
                                                                        0,
                                                                        0);      //
                                                                                 //
                                                                                 // -1
                                                                                 //
                                                                                 // if
                                                                                 //
                                                                                 // medical
                                                                                 //
                                                                                 // ?
      this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetViewUp(0, 0, 1);
      horizontalSlider_SliceIndex->setRange(0, this->m_ImageDimension[0] - 1);
      index = this->m_ImageDimension[0] / 2;
      break;
    case ORIENTATION_CORONAL:
      this->ImageViewer2->SetSliceOrientationToXZ();
      this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetPosition(0,
                                                                        -1,
                                                                        0);      //
                                                                                 //
                                                                                 // 1
                                                                                 //
                                                                                 // if
                                                                                 //
                                                                                 // medical
                                                                                 //
                                                                                 // ?
      this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetViewUp(0, 0, 1);
      horizontalSlider_SliceIndex->setRange(0, this->m_ImageDimension[1] - 1);
      index = this->m_ImageDimension[1] / 2;
      break;
    default:
      index = 0;
      break;
    }

  float scale;
  if( this->m_ImageDimension[0] != 0 && this->m_ImageDimension[1] != 0 && this->m_ImageDimension[2] !=
      0 )
    {
    float scaleTemp[3];
    if( this->m_ImageDimension[0] * this->m_ImageSpacing[0] >= this->m_ImageDimension[1]
        * this->m_ImageSpacing[1] )
      {
      scaleTemp[0] = this->m_ImageDimension[0] * this->m_ImageSpacing[0] / 2.0;
      }
    else
      {
      scaleTemp[0] = this->m_ImageDimension[1] * this->m_ImageSpacing[1] / 2.0;
      }

    if( this->m_ImageDimension[0] * this->m_ImageSpacing[0] >= this->m_ImageDimension[2]
        * this->m_ImageSpacing[2] )
      {
      scaleTemp[1] = this->m_ImageDimension[0] * this->m_ImageSpacing[0] / 2.0;
      }
    else
      {
      scaleTemp[1] = this->m_ImageDimension[2] * this->m_ImageSpacing[2] / 2.0;
      }

    if( this->m_ImageDimension[1] * this->m_ImageSpacing[1] >= this->m_ImageDimension[2]
        * this->m_ImageSpacing[2] )
      {
      scaleTemp[2] = this->m_ImageDimension[1] * this->m_ImageSpacing[1] / 2.0;
      }
    else
      {
      scaleTemp[2] = this->m_ImageDimension[2] * this->m_ImageSpacing[2] / 2.0;
      }

    if( scaleTemp[0] >= scaleTemp[1] && scaleTemp[0] >= scaleTemp[2] )
      {
      scale = scaleTemp[0]; // ((float)(this->m_ImageDimension[0]-1))/2.0;
      }
    else if( scaleTemp[1] >= scaleTemp[0] && scaleTemp[1] >= scaleTemp[2] )
      {
      scale = scaleTemp[1]; // ((float)(this->m_ImageDimension[1]-1))/2.0;
      }
    else // (dims[2]>=dims[0] && dims[2]>=dims[1])
      {
      scale = scaleTemp[2]; // ((float)(this->m_ImageDimension[2]-1))/2.0;
      }
    }
  else
    {
    // This case was not previously handled.  It seems like scale should always
    // be handled.
    scale = 0;
    }

  QString str;
  this->lineEdit_SliceIndex->setText( str.sprintf("%d", index) );
  this->horizontalSlider_SliceIndex->setValue(index);
  this->ImageViewer2->SetSlice(index);

  this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetFocalPoint(0, 0, 0);
  this->ImageViewer2->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();
  this->ImageViewer2->GetRenderer()->ResetCamera();
  this->ImageViewer2->GetRenderer()->GetActiveCamera()->SetParallelScale(scale );
  this->ImageViewer2->Render();
}

void ImageView2DPanelWithControls::on_comboBox_Contents_currentIndexChanged(
  int index)
{
  emit contentschanged(WindowID, index);
}

void ImageView2DPanelWithControls::on_comboBox_Interpolation_currentIndexChanged(int index)
{
  if( index > 0 )
    {
    ImageViewer2->GetImageActor()->InterpolateOn();
    }
  else
    {
    ImageViewer2->GetImageActor()->InterpolateOff();
    }

  emit interpolation(WindowID, index);
}

void ImageView2DPanelWithControls::on_toolButton_Visible_toggled(bool vis)
{
  emit visible(WindowID, vis);
}

void ImageView2DPanelWithControls::on_doubleSpinBox_valueChanged(double value)
{
  emit Opacity(WindowID, value);
}

void ImageView2DPanelWithControls::on_horizontalSlider_Gradient_valueChanged(
  int index)
{
  if( NumberOfGradients == 0 )
    {
    return;
    }
  QString str;
  // this->lineEdit_SliceIndex->setText(str.sprintf("%d/%d", index,
  // horizontalSlider_SliceIndex->maximum()));
  this->lineEdit_Gradient->setText( str.sprintf("%d", index) );

  emit gradientchanged(WindowID, index);
}

void ImageView2DPanelWithControls::SliceIndexChanged(vtkObject *obj,
                                                     unsigned long,
                                                     void *client_data,
                                                     void *,
                                                     vtkCommand * /* command */)
{
  vtkImagePlaneWidget *planeWidget
    = reinterpret_cast<vtkImagePlaneWidget *>( obj );
  int *whichwindow;

  whichwindow = (int *)client_data;

  short index = planeWidget->GetSliceIndex();
  // std::cout<<"orient"<<*whichwindow<<"  "<<*whichwindow<<"
  //  "<<*whichwindow<<std::endl;
  // std::cout<<"SliceIndex"<<index<<std::endl;
  QString str;
  this->lineEdit_SliceIndex->setText( str.sprintf("%d", index) );
  this->horizontalSlider_SliceIndex->setValue(index);

  this->ImageViewer2->SetSlice(index);
  this->ImageViewer2->Render();
  // emit SliceIndex(*whichwindow,Index);
}

void ImageView2DPanelWithControls::WindowLevelChanged(vtkObject *obj,
                                                      unsigned long,
                                                      void *client_data,
                                                      void *,
                                                      vtkCommand * /* command */)
{
  vtkImagePlaneWidget *planeWidget
    = reinterpret_cast<vtkImagePlaneWidget *>( obj );
  int *whichwindow;

  whichwindow = (int *)client_data;

  double wl[2];
  planeWidget->GetWindowLevel(wl);

  int windowLocal = (int)wl[0];
  int level = (int)wl[1];

  // std::cout<<"orient"<<*whichwindow<<"  "<<*whichwindow<<"
  //  "<<*whichwindow<<std::endl;
  // std::cout<<"windowLocal"<<windowLocal<<"Level: "<<level<<std::endl;

  this->ImageViewer2->SetColorWindow(windowLocal);
  this->ImageViewer2->SetColorLevel(level);
  this->ImageViewer2->Render();

  emit WindowLevel(windowLocal, level);
}

// syn

void ImageView2DPanelWithControls::on_toolButton_windowlevel_Syn_toggled(
  bool syn)
{
  emit SynWindowLevel(syn);
}

void ImageView2DPanelWithControls::on_toolButton_interpolation_Syn_toggled(
  bool syn)
{
  emit SynInterpolation(syn);
}

void ImageView2DPanelWithControls::on_toolButton_orientation_Syn_toggled(
  bool syn)
{
  emit SynOrientation(syn);
}

void ImageView2DPanelWithControls::on_toolButton_content_Syn_toggled(bool syn)
{
  emit SynContent(syn);
}
