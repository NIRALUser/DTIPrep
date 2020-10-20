#include <QtGui>

#include "GMainWindow.h"
#include <vtkVersion.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkCellPicker.h>

#include "Dicom2NrrdPanel.h"
#include "ImageView2DPanelWithControls.h"
#include "IntensityMotionCheckPanel.h"
// #include "VisualCheckingStatus.h"

#include "vtkXYPlotWidget.h"

#include "ThreadIntensityMotionCheck.h"

#include "itkNrrdImageIO.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"

#include "vtkEventQtSlotConnect.h"

#include <vtkInteractorStyleImage.h>

#include "vtkOutlineFilter.h"
#include "vtkTextProperty.h"

#include "vtkVolume16Reader.h"
#include "vtkAnnotatedCubeActor.h"
#include "vtkPointWidget.h"
#include "vtkImagePlaneWidget.h"
#include "vtkSplineWidget.h"
#include "vtkProbeFilter.h"
#include "vtkXYPlotActor.h"
#include "vtkAxesActor.h"
#include "vtkPropAssembly.h"
#include "vtkCaptionActor2D.h"
#include "vtkOrientationMarkerWidget.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCommand.h"
#include "vtkImagePlaneWidget.h"

#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkStructuredGrid.h"

// Constructor
GMainWindow::GMainWindow()
{
  setupUi(this);

  bDwiLoaded = false;

  bWindowLevelSyn  = true;
  bContentSyn  = true;
  bInterpolationSyn = true;
  bOrientationSyn  = false;

  bWindowLevelSyn = false;

  DTIPrepPanel = NULL;

  vtkQtConnections = vtkEventQtSlotConnect::New();

  planeWidgetX = vtkImagePlaneWidget::New();
  planeWidgetY = vtkImagePlaneWidget::New();
  planeWidgetZ = vtkImagePlaneWidget::New();

  DWIImage = NULL;

  verticalLayout->setContentsMargins(0, 0, 0, 0);
  verticalLayout_2->setContentsMargins(0, 0, 0, 0);
  // verticalLayout_3->setContentsMargins(0, 0, 0, 0);
  //  verticalLayout_3->setSpacing(2);
  tabWidget->setCurrentIndex(0);

  setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
  setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );

  // QT/VTK interact
  qvtkWidget->GetRenderWindow()->SetStereoTypeToRedBlue();


  vtkRenderWindow *renderWindow = vtkRenderWindow::New(); 
  qvtkWidget_3DView->SetRenderWindow(renderWindow);
  qvtkWidget_3DView->GetRenderWindow()->SetStereoTypeToRedBlue();

  pvtkRenderer = vtkRenderer::New();
  //pvtkRenderer->SetBackground(0.35, 0.35, 0.35);

  pvtkRenderer_3DView = vtkRenderer::New();
  pvtkRenderer_3DView->SetBackground(0.35, 0.35, 0.35);
  pvtkRenderer_3DView->SetViewport(0.0,0.0,1.0,1.0);



  qvtkWidget->GetRenderWindow()->AddRenderer(pvtkRenderer);
  qvtkWidget_3DView->GetRenderWindow()->AddRenderer(pvtkRenderer_3DView);

  // 3D direction vectors
  actorDirProtocol = vtkPropAssembly::New();
  actorDirFile   = vtkPropAssembly::New();
  actorDirInclude  = vtkPropAssembly::New();

  // vtkSphereSource *SphereSource  =  vtkSphereSource::New();
  SphereSource  =  vtkSphereSource::New();
  SphereSource->SetRadius(1.0);
  SphereSource->SetThetaResolution(50);
  SphereSource->SetPhiResolution(50);
  SphereSource->SetCenter(0.0, 0.0, 0.0);
  min_length_vec = 1;

  this->doubleSpinBox_SphereRadius->setMinimum(0.0);
  this->doubleSpinBox_SphereRadius->setMaximum(1.0);
  this->doubleSpinBox_SphereRadius->setSingleStep(0.1);
  this->doubleSpinBox_SphereRadius->setValue(1.0);

  this->doubleSpinBox_SphereOpacity->setMinimum(0.0);
  this->doubleSpinBox_SphereOpacity->setMaximum(1);
  this->doubleSpinBox_SphereOpacity->setSingleStep(0.1);
  this->doubleSpinBox_SphereOpacity->setValue(1.0);

  vtkPolyDataMapper *mapperSphere = vtkPolyDataMapper::New();
#if (VTK_MAJOR_VERSION < 6)
  mapperSphere->SetInput( SphereSource->GetOutput() );
#else
  //mapperSphere->SetInputData( SphereSource->GetOutput() );
  mapperSphere->SetInputConnection( SphereSource->GetOutputPort() );
#endif

  actorSphere  = vtkActor::New();
  actorSphere->SetMapper(mapperSphere);
  actorSphere->GetProperty()->SetColor(0.5, 0.3, 0.3);
  //actorSphere->GetProperty()->SetColor(1., 1., 1.);
  actorSphere->GetProperty()->SetOpacity(1.0);
  actorSphere->SetVisibility(2);

  // this->horizontalSlider_SphereRadius->setRange(0.01, 2);

  pvtkRenderer_3DView->AddActor(actorSphere);
  pvtkRenderer_3DView->AddViewProp(actorSphere);
  pvtkRenderer_3DView->ResetCamera();

  // qvtkWidget->GetRenderWindow()->StereoCapableWindowOn();
  // this->pvtkRenderer->GetActiveCamera()->ParallelProjectionOn();
  // qvtkWidget->GetRenderWindow()->SetStereoTypeToCrystalEyes();
  // vtkInteractorStyleFlight* flightstyle=vtkInteractorStyleFlight::New();
  //
  // qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(flightstyle);

  QMenu *popupLocal = new QMenu(qvtkWidget);
  popupLocal->addAction("Background White");
  popupLocal->addAction("Background Black");
  popupLocal->addAction("Stereo Rendering");
  connect( popupLocal, SIGNAL( triggered(QAction *) ), this,
           SLOT( BackGroundColor(QAction *) ) );

  // createActions();
  createStatusBar();
  createDockPanels();

  actionFrom_DWI->setDisabled(1);
  actionFrom_Protocol->setDisabled(1);
  actionIncluded->setDisabled(1);
  actionExcluded->setDisabled(1);

  actionFrom_DWI->setCheckable(0);
  actionFrom_Protocol->setCheckable(0);
  actionIncluded->setCheckable(0);
  actionExcluded->setCheckable(0);

  // ProbeWithSplineWidget();
  //  connect( this->DTIPrepPanel->GetThreadIntensityMotionCheck(),
  //    SIGNAL(ResultUpdate()),
  //    this,
  //    SLOT(ResultUpdate()) );

  //  connect( this->DTIPrepPanel,
  //    SIGNAL(loadProtocol()),
  //    this,
  //    SLOT(on_actionOpen_XML_triggered()));

  // connect( this->dicom2NrrdPanel,SIGNAL(Dicom2NrrdEnded()),this,

  connect( this->DTIPrepPanel,
           SIGNAL( ProtocolChanged() ),
           this,
           SLOT( UpdateProtocolDiffusionVectorActors() ) );

  connect( &this->DTIPrepPanel->myIntensityThread,
           SIGNAL( allDone(const QString &) ),
           statusBar(),
           SLOT( showMessage(const QString &) ) );

  connect( this->DTIPrepPanel,
           SIGNAL( status(const QString &) ),
           statusBar(),
           SLOT( showMessage(const QString &) ) );

  connect( &( this->dicom2NrrdPanel->ThreadDicomToNrrd ),
           SIGNAL( allDone(const QString &) ),
           statusBar(),
           SLOT( showMessage(const QString &) ) );

  // for DTIPrep panel
  connect( this->DTIPrepPanel, SIGNAL( currentGradient(int, int ) ),
           this, SLOT( GradientChanged( int, int ) ) );

  connect( this, SIGNAL( currentGradient_VC_Include(int, int ) ),
           this, SLOT( GradientChanged_VC_Include( int, int ) ) );

  connect( this, SIGNAL( currentGradient_VC_Exclude(int, int ) ),
           this, SLOT( GradientChanged_VC_Exclude( int, int ) ) );

  connect( this->DTIPrepPanel, SIGNAL( currentGradientChanged_VC(int) ),
           this, SLOT( GradientUpdate( int ) ) );

  connect( this, SIGNAL( VisualCheckingStatus(int, int ) ),
           this->DTIPrepPanel, SLOT( SetVisualCheckingStatus( int, int ) ) );

  connect( this->DTIPrepPanel, SIGNAL( UpdateOutputDWIDiffusionVectorActors() ),
           this, SLOT( UpdateOutputDWIDiffusionVectorActors() ) );

  connect( this, SIGNAL( OpenMappingXML() ),
           this->DTIPrepPanel, SLOT( OpenMappingXML() ) );

  // connect( this, SIGNAL( OpenQCedDWI() ),
  // this->DTIPrepPanel, SLOT( OpenQCedDWI()) );

  connect( this->DTIPrepPanel, SIGNAL( SignalLoadQCedDWI( QString ) ),
           this, SLOT( LoadQCedDWI( QString ) ) );
  //

  connect( this->DTIPrepPanel, SIGNAL( LoadQCResult(bool) ), this, SLOT( LoadQCResult(bool) ) );

  connect( this->DTIPrepPanel, SIGNAL(SignalLoadDwiFile() ), this, SLOT( on_actionOpenDWINrrd_triggered() ) );

  //

  connect( this->imageView2DPanelWithControls1, SIGNAL( indexchanged(int, int ) ),
           this, SLOT( ImageIndexChanged( int, int ) ) );

  connect( this->imageView2DPanelWithControls2, SIGNAL( indexchanged(int, int ) ),
           this, SLOT( ImageIndexChanged( int, int ) ) );

  connect( this->imageView2DPanelWithControls3, SIGNAL( indexchanged(int, int ) ),
           this, SLOT( ImageIndexChanged( int, int ) ) );

  connect( this->imageView2DPanelWithControls1,
           SIGNAL( orientationchanged(int, int ) ),
           this, SLOT( OrientationChanged( int, int ) ) );

  connect( this->imageView2DPanelWithControls2,
           SIGNAL( orientationchanged(int, int ) ),
           this, SLOT( OrientationChanged( int, int ) ) );

  connect( this->imageView2DPanelWithControls3,
           SIGNAL( orientationchanged(int, int ) ),
           this, SLOT( OrientationChanged( int, int ) ) );

  connect( this->imageView2DPanelWithControls1, SIGNAL( visible(int, bool ) ),
           this, SLOT( VisibleChanged( int, bool ) ) );

  connect( this->imageView2DPanelWithControls2, SIGNAL( visible(int, bool ) ),
           this, SLOT( VisibleChanged( int, bool ) ) );

  connect( this->imageView2DPanelWithControls3, SIGNAL( visible(int, bool ) ),
           this, SLOT( VisibleChanged( int, bool ) ) );

  connect( this->imageView2DPanelWithControls1, SIGNAL( Opacity(int, double ) ),
           this, SLOT( OpacityChanged( int, double ) ) );

  connect( this->imageView2DPanelWithControls2, SIGNAL( Opacity(int, double ) ),
           this, SLOT( OpacityChanged( int, double ) ) );

  connect( this->imageView2DPanelWithControls3, SIGNAL( Opacity(int, double ) ),
           this, SLOT( OpacityChanged( int, double ) ) );

  connect( this->imageView2DPanelWithControls1,
           SIGNAL( gradientchanged(int, int ) ),
           this, SLOT( GradientChanged( int, int ) ) );

  connect( this->imageView2DPanelWithControls2,
           SIGNAL( gradientchanged(int, int ) ),
           this, SLOT( GradientChanged( int, int ) ) );

  connect( this->imageView2DPanelWithControls3,
           SIGNAL( gradientchanged(int, int ) ),
           this, SLOT( GradientChanged( int, int ) ) );

  connect( this->imageView2DPanelWithControls1, SIGNAL( interpolation(int, int ) ),
           this, SLOT( InterpolationChanged( int, int ) ) );

  connect( this->imageView2DPanelWithControls2, SIGNAL( interpolation(int, int ) ),
           this, SLOT( InterpolationChanged( int, int ) ) );

  connect( this->imageView2DPanelWithControls3, SIGNAL( interpolation(int, int ) ),
           this, SLOT( InterpolationChanged( int, int ) ) );

  connect( this->imageView2DPanelWithControls1,
           SIGNAL( contentschanged(int, int ) ),
           this, SLOT( ContentsChanged( int, int ) ) );

  connect( this->imageView2DPanelWithControls2,
           SIGNAL( contentschanged(int, int ) ),
           this, SLOT( ContentsChanged( int, int ) ) );

  connect( this->imageView2DPanelWithControls3,
           SIGNAL( contentschanged(int, int ) ),
           this, SLOT( ContentsChanged( int, int ) ) );

  // syn for 3-imagePlanes

  connect( this->imageView2DPanelWithControls1,
           SIGNAL( SynWindowLevel(bool) ),
           this,
           SLOT( WindowLevelSyn(bool ) ) );

  connect( this->imageView2DPanelWithControls1,
           SIGNAL( SynContent(bool) ),
           this, SLOT( ContentSyn(bool ) ) );

  connect( this->imageView2DPanelWithControls1,
           SIGNAL( SynInterpolation(bool) ),
           this, SLOT( InterpolationSyn(bool ) ) );

  connect( this->imageView2DPanelWithControls1,
           SIGNAL( SynOrientation(bool) ),
           this, SLOT( OrientationSyn(bool ) ) );

  connect( this->imageView2DPanelWithControls2,
           SIGNAL( SynWindowLevel(bool) ),
           this,
           SLOT( WindowLevelSyn(bool ) ) );

  connect( this->imageView2DPanelWithControls2,
           SIGNAL( SynContent(bool) ),
           this, SLOT( ContentSyn(bool ) ) );

  connect( this->imageView2DPanelWithControls2,
           SIGNAL( SynInterpolation(bool) ),
           this, SLOT( InterpolationSyn(bool ) ) );

  connect( this->imageView2DPanelWithControls2,
           SIGNAL( SynOrientation(bool) ),
           this, SLOT( OrientationSyn(bool ) ) );

  connect( this->imageView2DPanelWithControls3,
           SIGNAL( SynWindowLevel(bool) ),
           this,
           SLOT( WindowLevelSyn(bool ) ) );

  connect( this->imageView2DPanelWithControls3,
           SIGNAL( SynContent(bool) ),
           this, SLOT( ContentSyn(bool ) ) );

  connect( this->imageView2DPanelWithControls3,
           SIGNAL( SynInterpolation(bool) ),
           this, SLOT( InterpolationSyn(bool ) ) );

  connect( this->imageView2DPanelWithControls3,
           SIGNAL( SynOrientation(bool) ),
           this, SLOT( OrientationSyn(bool ) ) );

  // for imageplane window/leveling 2D received then emit
  // WindowLevel(window,level);
  connect( this->imageView2DPanelWithControls1,
           SIGNAL( WindowLevel(double, double) ),
           this, SLOT( SetAllWindowLevel(double, double ) ) );

  connect( this->imageView2DPanelWithControls2,
           SIGNAL( WindowLevel(double, double) ),
           this, SLOT( SetAllWindowLevel(double, double ) ) );

  connect( this->imageView2DPanelWithControls3,
           SIGNAL( WindowLevel(double, double) ),
           this, SLOT( SetAllWindowLevel(double, double ) ) );

  // setWindowTitle(tr("Grace DTI(Qt4)"));

  connect( this->DTIPrepPanel,
           SIGNAL( SignalActivateSphere() ),
           this, SLOT( SetactionIncluded() ) );

  connect( this,
           SIGNAL( Signal_actionOpenDWINrrd_triggered() ),
           this, SLOT( on_actionOpenDWINrrd_triggered() ) );

  connect( this->doubleSpinBox_SphereRadius, SIGNAL(valueChanged( double ) ),
           this, SLOT( doubleSpinBox_SphereRadiusValueChanged( double ) ) );

  connect( this->doubleSpinBox_SphereOpacity, SIGNAL(valueChanged( double ) ),
           this, SLOT( doubleSpinBox_SphereOpacityValueChanged( double ) ) );

}

void GMainWindow::LoadQCResult(bool b)
{
  bQCResultLoad = b;
  std::cout << "TestLoadingQCXml" << b << std::endl;
}

void GMainWindow::ProbeWithSplineWidget()
{
  vtkFloatArray *FloatArray = vtkFloatArray::New();

  FloatArray->SetNumberOfComponents(1);
  FloatArray->SetNumberOfTuples(15); //

  vtkStructuredGrid *myPointSet = vtkStructuredGrid::New();

  vtkPoints *Points = vtkPoints::New();
  for( int i = 0; i < 15; i++ )
    {
    FloatArray->SetTuple1( i, log(i + 3.0) * 1.0);
    Points->InsertPoint(i, i, 0, 0);
    }
  myPointSet->SetPoints(Points);
  Points->Delete();

  myPointSet->GetPointData()->SetScalars(FloatArray); // >SetTensors(tensors);

  // The plot of the profile data.
  vtkXYPlotActor *profile = vtkXYPlotActor::New();
#if (VTK_MAJOR_VERSION < 6)
  profile->AddInput( myPointSet);
#else
  profile->AddDataSetInput( myPointSet);
#endif
  profile->GetPositionCoordinate()->SetValue(0.05, 0.05, 0);
  profile->GetPosition2Coordinate()->SetValue(0.95, 0.95, 0);
  profile->SetXValuesToNormalizedArcLength();
  profile->SetNumberOfXLabels( 15);
  profile->SetTitle( "Profile Data ");
  profile->SetXTitle( "s");
  profile->SetYTitle( "I(s)");
  profile->SetXRange( 0, 1);
  profile->SetPlotColor(0, .6, .2, .4);

  // double range[3];
  // profile-> SetYRange(range[0], range[1]);
  profile->SetYRange(0, 10);
  // profile->GetProperty()->SetColor(0, 0, 0);
  // profile->GetProperty()->SetLineWidth( 2);
  profile->SetLabelFormat( "%g");
  profile->GetTitleTextProperty()->SetColor(0.02, 0.06, 0.62);
  profile->GetTitleTextProperty()->SetFontFamilyToArial();
  profile->SetAxisTitleTextProperty( profile->GetTitleTextProperty() );
  profile->SetAxisLabelTextProperty( profile->GetTitleTextProperty() );
  profile->SetTitleTextProperty( profile->GetTitleTextProperty() );

  vtkRenderer *ren2 = vtkRenderer::New();

  ren2->SetBackground( 0.5, 0.4, 0.4);

  ren2->SetViewport( 0.5, 0, 1, 1);

  ren2->AddActor2D( profile);

  qvtkWidget->GetRenderWindow()->AddRenderer(ren2);
}

// syn for 3-imagePlanes
void GMainWindow::WindowLevelSyn(bool syn)
{
  bWindowLevelSyn  = syn;
  this->imageView2DPanelWithControls1->toggle_toolButton_windowlevel_Syn(
    bWindowLevelSyn);
  this->imageView2DPanelWithControls2->toggle_toolButton_windowlevel_Syn(
    bWindowLevelSyn);
  this->imageView2DPanelWithControls3->toggle_toolButton_windowlevel_Syn(
    bWindowLevelSyn);
}

void GMainWindow::ContentSyn(bool syn)
{
  bContentSyn  = syn;
  this->imageView2DPanelWithControls1->toggle_toolButton_content_Syn(
    bContentSyn);
  this->imageView2DPanelWithControls2->toggle_toolButton_content_Syn(
    bContentSyn);
  this->imageView2DPanelWithControls3->toggle_toolButton_content_Syn(
    bContentSyn);
}

void GMainWindow::InterpolationSyn(bool syn)
{
  bInterpolationSyn = syn;
  this->imageView2DPanelWithControls1->toggle_toolButton_interpolation_Syn(
    bInterpolationSyn);
  this->imageView2DPanelWithControls2->toggle_toolButton_interpolation_Syn(
    bInterpolationSyn);
  this->imageView2DPanelWithControls3->toggle_toolButton_interpolation_Syn(
    bInterpolationSyn);
}

void GMainWindow::OrientationSyn(bool syn)
{
  bOrientationSyn  = syn;
  this->imageView2DPanelWithControls1->toggle_toolButton_orientation_Syn(
    bOrientationSyn);
  this->imageView2DPanelWithControls2->toggle_toolButton_orientation_Syn(
    bOrientationSyn);
  this->imageView2DPanelWithControls3->toggle_toolButton_orientation_Syn(
    bOrientationSyn);
}

void GMainWindow::ImageIndexChanged(int winID, int index)
{
  switch( winID )
    {
    case 0:
      planeWidgetZ->SetSliceIndex(index);
      break;
    case 1:
      planeWidgetX->SetSliceIndex(index);
      break;
    case 2:
      planeWidgetY->SetSliceIndex(index);
      break;
    default:
      break;
    }
  qvtkWidget->GetRenderWindow()->GetInteractor()->Render();
}

void GMainWindow::ContentsChanged(int /* WinID */, int index)
{
  if( bContentSyn )
    {
    this->imageView2DPanelWithControls1->GetComboBox_Contents()->
    setCurrentIndex(index);
    this->imageView2DPanelWithControls2->GetComboBox_Contents()->
    setCurrentIndex(index);
    this->imageView2DPanelWithControls3->GetComboBox_Contents()->
    setCurrentIndex(index);
    }
  else
    {
    }
  qvtkWidget->GetRenderWindow()->Render();
}

void GMainWindow::OrientationChanged(int winID, int newOrient)
{
  if( bOrientationSyn )
    {
    if( newOrient == 0 )
      {
      planeWidgetX->SetPlaneOrientationToZAxes();
      planeWidgetY->SetPlaneOrientationToZAxes();
      planeWidgetZ->SetPlaneOrientationToZAxes();
      }
    if( newOrient == 1 )
      {
      planeWidgetX->SetPlaneOrientationToXAxes();
      planeWidgetY->SetPlaneOrientationToXAxes();
      planeWidgetZ->SetPlaneOrientationToXAxes();
      }
    if( newOrient == 2 )
      {
      planeWidgetX->SetPlaneOrientationToYAxes();
      planeWidgetY->SetPlaneOrientationToYAxes();
      planeWidgetZ->SetPlaneOrientationToYAxes();
      }
    this->imageView2DPanelWithControls1->GetComboBox_Orientation()->
    setCurrentIndex(newOrient);
    this->imageView2DPanelWithControls2->GetComboBox_Orientation()->
    setCurrentIndex(newOrient);
    this->imageView2DPanelWithControls3->GetComboBox_Orientation()->
    setCurrentIndex(newOrient);
    }
  else
    {
    switch( winID )
      {
      case 0:
        if( newOrient == 0 )
          {
          planeWidgetZ->SetPlaneOrientationToZAxes();
          }
        if( newOrient == 1 )
          {
          planeWidgetZ->SetPlaneOrientationToXAxes();
          }
        if( newOrient == 2 )
          {
          planeWidgetZ->SetPlaneOrientationToYAxes();
          }
        break;
      case 1:
        if( newOrient == 0 )
          {
          planeWidgetX->SetPlaneOrientationToZAxes();
          }
        if( newOrient == 1 )
          {
          planeWidgetX->SetPlaneOrientationToXAxes();
          }
        if( newOrient == 2 )
          {
          planeWidgetX->SetPlaneOrientationToYAxes();
          }
        break;
      case 2:
        if( newOrient == 0 )
          {
          planeWidgetY->SetPlaneOrientationToZAxes();
          }
        if( newOrient == 1 )
          {
          planeWidgetY->SetPlaneOrientationToXAxes();
          }
        if( newOrient == 2 )
          {
          planeWidgetY->SetPlaneOrientationToYAxes();
          }
        break;
      default:
        break;
      }
    }
  qvtkWidget->GetRenderWindow()->Render();
}

GMainWindow::~GMainWindow()
{
  if( this->planeWidgetX )
    {
    this->planeWidgetX->Delete();
    }
  if( this->planeWidgetY )
    {
    this->planeWidgetY->Delete();
    }
  if( this->planeWidgetZ )
    {
    this->planeWidgetZ->Delete();
    }
}

void GMainWindow::ChangeStyleTo(QString styleLocal)
{
  qApp->setStyle( QStyleFactory::create(styleLocal) );
}

void GMainWindow::on_actionWindows_triggered()
{
  ChangeStyleTo( tr("Windows") );
}

void GMainWindow::on_actionWindowsXP_triggered()
{
  ChangeStyleTo( tr("WindowsXP") );
}

void GMainWindow::on_actionMotif_triggered()
{
  ChangeStyleTo( tr("Motif") );
}

void GMainWindow::on_actionCDE_triggered()
{
  ChangeStyleTo( tr("CDE") );
}

void GMainWindow::on_actionPlastique_triggered()
{
  ChangeStyleTo( tr("Plastique") );
}

void GMainWindow::on_actionCleanlooks_triggered()
{
  ChangeStyleTo( tr("Cleanlooks") );
}

// void GMainWindow::UpdateProgressbar(int posLocal)
// {
// if ( !progressWidget->isVisible() )
// {
// progressWidget->show();
// }
// GatProgressWidget()->setValue(posLocal);
// if ( posLocal >= 100 )
// {
// progressWidget->hide();
// }
// }

void GMainWindow::print()
{
  // #ifndef QT_NO_PRINTER
  //     QTextDocument *document ;
  //     QPrinter printer;
  //
  //     QPrintDialog *dlg = new QPrintDialog(&printer, this);
  //     if (dlg->exec() != QDialog::Accepted)
  //         return;
  //
  //     document->print(&printer);
  //
  //     statusBar()->showMessage(tr("Ready"), 2000);
  // #endif
}

void GMainWindow::save()
{
  /*
      QString fileName = QFileDialog::getSaveFileName(this,
                          tr("Choose a file name"), ".",
                          tr("HTML (*.html *.htm)"));
      if (fileName.isEmpty())
          return;
      QFile file(fileName);
      if (!file.open(QFile::WriteOnly | QFile::Text)) {
          QMessageBox::warning(this, tr("Dock Widgets"),
                               tr("Cannot write file %1:\n%2.")
                               .arg(fileName)
                               .arg(file.errorString()));
          return;
      }

      QTextStream out(&file);
      QApplication::setOverrideCursor(Qt::WaitCursor);
      out <<
      QApplication::restoreOverrideCursor();

      statusBar()->showMessage(tr("Saved '%1'").arg(fileName), 2000);*/
  statusBar()->showMessage(tr("Saved "), 2000);
}

void GMainWindow::about()
{
  QString text = "Version: " ;
  text += DTIPREP_VERSION ;
  text += "\nA Tool to do QC/Prep work for DWI/DTI. " ;
  text += "\nCopyright(c) UNC. All rights reserved. " ;
  text += "\nSee http://www.nitrc.org/projects/dtiprep for details." ;
  QMessageBox::about( this, tr("About DTIPrep"), tr( text.toStdString().c_str() ) );
}

void GMainWindow::help()
{
  QUrl url("http://www.nitrc.org/docman/index.php?group_id=283&selected_doc_group_id=2743&language_id=1#folder");
  url.setScheme("http");
  QDesktopServices::openUrl(url);
}

void GMainWindow::on_actionAbout_triggered()
{
  about();
}

void GMainWindow::on_action_Help_triggered()
{
  help();
}

void GMainWindow::createActions()
{
  saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save..."), this);
  saveAct->setShortcut( tr("Ctrl+S") );
  saveAct->setStatusTip( tr("Save the current form letter") );
  connect( saveAct, SIGNAL( triggered() ), this, SLOT( save() ) );

  printAct = new QAction(QIcon(":/images/print.png"), tr("&Print..."), this);
  printAct->setShortcut( tr("Ctrl+P") );
  printAct->setStatusTip( tr("Print the current form letter") );
  connect( printAct, SIGNAL( triggered() ), this, SLOT( print() ) );

  quitAct = new QAction(tr("&Quit"), this);
  quitAct->setShortcut( tr("Ctrl+Q") );
  quitAct->setStatusTip( tr("Quit the application") );
  connect( quitAct, SIGNAL( triggered() ), this, SLOT( close() ) );

  aboutAct = new QAction(tr("&About"), this);
  aboutAct->setStatusTip( tr("Show the application's About box") );
  connect( aboutAct, SIGNAL( triggered() ), this, SLOT( about() ) );

  aboutQtAct = new QAction(tr("About &Qt"), this);
  aboutQtAct->setStatusTip( tr("Show the Qt library's About box") );
  connect( aboutQtAct, SIGNAL( triggered() ), qApp, SLOT( aboutQt() ) );

  styleGroup = new QActionGroup(this);
  styleGroup->addAction(actionWindows);
  styleGroup->addAction(actionCDE);
  styleGroup->addAction(actionMotif);
  styleGroup->addAction(actionPlastique);
  styleGroup->addAction(actionCleanlooks);

  // leftAlignAct->setChecked(true);
}

void GMainWindow::createStatusBar()
{
  statusBar()->showMessage( tr("Ready") );

  // progressWidget = new QProgressBar( statusBar() );
  // progressWidget->setRange(0, 100);
  // progressWidget->setValue(0);
  // progressWidget->setAlignment(Qt::AlignCenter);
  // progressWidget->setMaximumWidth(200); //
  // statusBar()->addPermanentWidget(progressWidget, 0);

  // progressWidget->hide();
}

void GMainWindow::createDockPanels_IntensityMotionCheckPanel()
{
  DTIPrepPanel = new IntensityMotionCheckPanel(this);
  DTIPrepPanel->setAllowedAreas(
    Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea
    |  Qt::BottomDockWidgetArea  );
  addDockWidget(Qt::LeftDockWidgetArea, DTIPrepPanel);

}

void GMainWindow::createDockPanels_Dicom2NrrdPanel()
{
  dicom2NrrdPanel = new Dicom2NrrdPanel(this);
  dicom2NrrdPanel->setAllowedAreas(
    Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea    );
  addDockWidget(Qt::LeftDockWidgetArea, dicom2NrrdPanel);

}

void GMainWindow::createDockPanels_imageView2DPanel1()
{
  imageView2DPanelWithControls1
    = new ImageView2DPanelWithControls(tr("Image2DView 1"), this);
  // imageView2DPanelWithControls1->Setup(connecter->GetOutput(), 0,0, -1,
  // ImageView2DPanelWithControls::ORIENTATION_AXIAL, 0);
  imageView2DPanelWithControls1->setAllowedAreas(
    Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea
    | Qt::BottomDockWidgetArea    );
  imageView2DPanelWithControls1->setStyleSheet("color: blue;");
  addDockWidget(Qt::RightDockWidgetArea, imageView2DPanelWithControls1);

}

void GMainWindow::createDockPanels_imageView2DPanel2()
{
  imageView2DPanelWithControls2
    = new ImageView2DPanelWithControls(tr("Image2DView 2"), this);
  // imageView2DPanelWithControls2->Setup(connecter->GetOutput(), 0,0,
  // -1,ImageView2DPanelWithControls::ORIENTATION_SAGITTAL, 1);
  imageView2DPanelWithControls2->setAllowedAreas(
    Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea
    | Qt::BottomDockWidgetArea    );
  imageView2DPanelWithControls2->setStyleSheet("color: cyan;");
  addDockWidget(Qt::RightDockWidgetArea, imageView2DPanelWithControls2);
}

void GMainWindow::createDockPanels_imageView2DPanel3()
{
  imageView2DPanelWithControls3
    = new ImageView2DPanelWithControls(tr("Image2DView 3"), this);
  // imageView2DPanelWithControls3->Setup(connecter->GetOutput(), 0,0,
  // -1,ImageView2DPanelWithControls::ORIENTATION_CORONAL, 2);
  imageView2DPanelWithControls3->setAllowedAreas(
    Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea
    | Qt::BottomDockWidgetArea    );
  imageView2DPanelWithControls3->setStyleSheet("color: balck;");
  addDockWidget(Qt::RightDockWidgetArea, imageView2DPanelWithControls3);
}

void GMainWindow::createDockPanels()
{
  DTIPrepPanel = new IntensityMotionCheckPanel(this);
  DTIPrepPanel->setAllowedAreas(
    Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea
    |  Qt::BottomDockWidgetArea  );
  addDockWidget(Qt::LeftDockWidgetArea, DTIPrepPanel);

  dicom2NrrdPanel = new Dicom2NrrdPanel(this);
  dicom2NrrdPanel->setAllowedAreas(
    Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea    );
  addDockWidget(Qt::LeftDockWidgetArea, dicom2NrrdPanel);

  //    tabifyDockWidget ( DTIPrepPanel,dicom2NrrdPanel );
  tabifyDockWidget( dicom2NrrdPanel, DTIPrepPanel );

  // DTIPrepPanel->hide();
  // dicom2NrrdPanel ->hide();

  /*
      imageView2DPanel1 = new ImageView2DPanel(tr("Image2DView 1: Axial"),this);
      imageView2DPanel1->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea    );
      addDockWidget(Qt::RightDockWidgetArea, imageView2DPanel1);

      imageView2DPanel2 = new ImageView2DPanel(tr("Image2DView 2: Sagittal"),this);
      imageView2DPanel2->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea    );
      addDockWidget(Qt::RightDockWidgetArea, imageView2DPanel2);

      imageView2DPanel3 = new ImageView2DPanel(tr("Image2DView 3: Coronal"),this);
      imageView2DPanel3->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea    );
      addDockWidget(Qt::RightDockWidgetArea, imageView2DPanel3);

  */

  imageView2DPanelWithControls1
    = new ImageView2DPanelWithControls(tr("Image2DView 1"), this);
  // imageView2DPanelWithControls1->Setup(connecter->GetOutput(), 0,0, -1,
  // ImageView2DPanelWithControls::ORIENTATION_AXIAL, 0);
  imageView2DPanelWithControls1->setAllowedAreas(
    Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea
    | Qt::BottomDockWidgetArea    );
  imageView2DPanelWithControls1->setStyleSheet("color: blue;");
  addDockWidget(Qt::RightDockWidgetArea, imageView2DPanelWithControls1);

  imageView2DPanelWithControls2
    = new ImageView2DPanelWithControls(tr("Image2DView 2"), this);
  // imageView2DPanelWithControls2->Setup(connecter->GetOutput(), 0,0,
  // -1,ImageView2DPanelWithControls::ORIENTATION_SAGITTAL, 1);
  imageView2DPanelWithControls2->setAllowedAreas(
    Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea
    | Qt::BottomDockWidgetArea    );
  imageView2DPanelWithControls2->setStyleSheet("color: cyan;");
  addDockWidget(Qt::RightDockWidgetArea, imageView2DPanelWithControls2);

  imageView2DPanelWithControls3
    = new ImageView2DPanelWithControls(tr("Image2DView 3"), this);
  // imageView2DPanelWithControls3->Setup(connecter->GetOutput(), 0,0,
  // -1,ImageView2DPanelWithControls::ORIENTATION_CORONAL, 2);
  imageView2DPanelWithControls3->setAllowedAreas(
    Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea
    | Qt::BottomDockWidgetArea    );
  imageView2DPanelWithControls3->setStyleSheet("color: black;");
  addDockWidget(Qt::RightDockWidgetArea, imageView2DPanelWithControls3);
}

void GMainWindow::on_actionExit_triggered()
{
  qApp->quit();
}

void GMainWindow::doubleSpinBox_SphereOpacityValueChanged( double opacity)
{

  this->actorSphere->GetProperty()->SetOpacity(opacity);
  qvtkWidget_3DView->GetRenderWindow()->Render();

}

void GMainWindow::doubleSpinBox_SphereRadiusValueChanged( double radius )
{

  this->SphereSource->SetRadius(radius);
  qvtkWidget_3DView->GetRenderWindow()->Render();
}

void GMainWindow::ReloadQCedDWI( QString Qqcdwiname )
{
  // Load QCed DWI newly in DTIPrep which can be used for new running or visual checking
  if( Qqcdwiname.length() > 0 )
    {
    itk::NrrdImageIO::Pointer NrrdImageIO = itk::NrrdImageIO::New();
    QCedDwiReader = DwiReaderType::New();
    QCedDwiReader->SetImageIO(NrrdImageIO);

    try
      {
      QCedDwiReader->SetFileName( Qqcdwiname.toStdString() );
      std::cout << "Loading automatically QCed DWI in GMainWindow: " << Qqcdwiname.toStdString() << " ... ";
      QCedDwiReader->Update();
      }
    catch( itk::ExceptionObject & e )
      {
      std::cout << e.GetDescription() << std::endl;
      bDwiLoaded = false;
      return;
      }
    }
  else
    {
    std::cerr << "QCed DWI name is not set." << std::endl;
    }
  std::cout << "done " << std::endl;

  bDwiLoaded = true;
  QCedDWIImage = QCedDwiReader->GetOutput();

  setWindowTitle(tr("DTIPrep Tools(Qt4) - ") + Qqcdwiname);

  // update DTIPrepPanel
  DTIPrepPanel->SetFileName(Qqcdwiname);
  DTIPrepPanel->SetName(Qqcdwiname);
  DTIPrepPanel->SetDwiOutputImage(QCedDWIImage);
  DTIPrepPanel->SetDWIImage(QCedDWIImage);
  DTIPrepPanel->UpdatePanelDWI();

  // update 2D/3D Image display
  componentExtractor1 = FilterType::New();
  componentExtractor2 = FilterType::New();
  componentExtractor3 = FilterType::New();

  gradientConnecter1 = ItkVtkImageFilterTypeUShort::New();
  gradientConnecter2 = ItkVtkImageFilterTypeUShort::New();
  gradientConnecter3 = ItkVtkImageFilterTypeUShort::New();

  //  componentExtractor1->SetInput(DwiReader->GetOutput());
  //  componentExtractor2->SetInput(DwiReader->GetOutput());
  //  componentExtractor3->SetInput(DwiReader->GetOutput());

  componentExtractor1->SetInput( QCedDWIImage );
  componentExtractor2->SetInput( QCedDWIImage );
  componentExtractor3->SetInput( QCedDWIImage );

  componentExtractor1->SetIndex( 0 );
  componentExtractor2->SetIndex( 0 );
  componentExtractor3->SetIndex( 0 );

  gradientConnecter1->SetInput( componentExtractor1->GetOutput() );
  gradientConnecter2->SetInput( componentExtractor2->GetOutput() );
  gradientConnecter3->SetInput( componentExtractor3->GetOutput() );

  gradientConnecter1->Update();
  gradientConnecter2->Update();
  gradientConnecter3->Update();

  UpdateImagePlaneWidgets(0, 0, 0);
  UpdateImageView2DWindows( 0, 0, 0, QCedDwiReader->GetOutput()->GetVectorLength() );

  whichWindow[0] = 0;
  whichWindow[1] = 1;
  whichWindow[2] = 2;

  vtkQtConnections->Connect(
    imageView2DPanelWithControls2->GetImageViewer2()->GetRenderWindow()->
    GetInteractor()->GetInteractorStyle(),
    vtkCommand::WindowLevelEvent,
    this,
    SLOT( WindowLevelChanged( vtkObject *, unsigned long, void *, void *,
                              vtkCommand *) ),
    &whichWindow[1], 1.0);

  vtkQtConnections->Connect(
    imageView2DPanelWithControls3->GetImageViewer2()->GetRenderWindow()->
    GetInteractor()->GetInteractorStyle(),
    vtkCommand::WindowLevelEvent,
    this,
    SLOT( WindowLevelChanged( vtkObject *, unsigned long, void *, void *,
                              vtkCommand *) ),
    &whichWindow[2], 1.0);

  vtkQtConnections->Connect(
    imageView2DPanelWithControls1->GetImageViewer2()->GetRenderWindow()->
    GetInteractor()->GetInteractorStyle(),
    vtkCommand::WindowLevelEvent,
    this,
    SLOT( WindowLevelChanged( vtkObject *, unsigned long, void *, void *,
                              vtkCommand *) ),
    &whichWindow[0], 1.0);

  UpdateDWIDiffusionVectorActors( QCedDWIImage );

  bQCResultLoad = false;
  DTIPrepPanel->Clear_VC_Status();
  DTIPrepPanel->GetQCResult().Clear();

}

void GMainWindow::LoadQCedDWI( QString Qqcdwiname )
{
  // Load QCed DWI file automatically after running

  if( Qqcdwiname.length() > 0 )
    {
    itk::NrrdImageIO::Pointer NrrdImageIO = itk::NrrdImageIO::New();
    QCedDwiReader = DwiReaderType::New();
    QCedDwiReader->SetImageIO(NrrdImageIO);

    try
      {
      QCedDwiReader->SetFileName( Qqcdwiname.toStdString() );
      std::cout << "Loading automatically QCed DWI in GMainWindow: " << Qqcdwiname.toStdString() << " ... ";
      QCedDwiReader->Update();
      }
    catch( itk::ExceptionObject & e )
      {
      std::cout << e.GetDescription() << std::endl;
      bDwiLoaded = false;
      return;
      }
    }
  else
    {
    std::cerr << "QCed DWI name is not set." << std::endl;
    }
  std::cout << "done " << std::endl;

  bDwiLoaded = true;
  QCedDWIImage = QCedDwiReader->GetOutput();

  setWindowTitle(tr("DTIPrep Tools(Qt4) - ") + Qqcdwiname);

  // update DTIPrepPanel
  // DTIPrepPanel->SetFileName(Qqcdwiname);
  // DTIPrepPanel->SetName(Qqcdwiname);
  // DTIPrepPanel->SetDWIImage(QCedDWIImage);
  // DTIPrepPanel->UpdatePanelDWI();

  // update 2D/3D Image display
  componentExtractor1 = FilterType::New();
  componentExtractor2 = FilterType::New();
  componentExtractor3 = FilterType::New();

  gradientConnecter1 = ItkVtkImageFilterTypeUShort::New();
  gradientConnecter2 = ItkVtkImageFilterTypeUShort::New();
  gradientConnecter3 = ItkVtkImageFilterTypeUShort::New();

  //  componentExtractor1->SetInput(DwiReader->GetOutput());
  //  componentExtractor2->SetInput(DwiReader->GetOutput());
  //  componentExtractor3->SetInput(DwiReader->GetOutput());

  componentExtractor1->SetInput( QCedDWIImage );
  componentExtractor2->SetInput( QCedDWIImage );
  componentExtractor3->SetInput( QCedDWIImage );

  componentExtractor1->SetIndex( 0 );
  componentExtractor2->SetIndex( 0 );
  componentExtractor3->SetIndex( 0 );

  gradientConnecter1->SetInput( componentExtractor1->GetOutput() );
  gradientConnecter2->SetInput( componentExtractor2->GetOutput() );
  gradientConnecter3->SetInput( componentExtractor3->GetOutput() );

  gradientConnecter1->Update();
  gradientConnecter2->Update();
  gradientConnecter3->Update();

  UpdateImagePlaneWidgets(0, 0, 0);
  UpdateImageView2DWindows( 0, 0, 0, QCedDwiReader->GetOutput()->GetVectorLength() );

  whichWindow[0] = 0;
  whichWindow[1] = 1;
  whichWindow[2] = 2;

  vtkQtConnections->Connect(
    imageView2DPanelWithControls2->GetImageViewer2()->GetRenderWindow()->
    GetInteractor()->GetInteractorStyle(),
    vtkCommand::WindowLevelEvent,
    this,
    SLOT( WindowLevelChanged( vtkObject *, unsigned long, void *, void *,
                              vtkCommand *) ),
    &whichWindow[1], 1.0);

  vtkQtConnections->Connect(
    imageView2DPanelWithControls3->GetImageViewer2()->GetRenderWindow()->
    GetInteractor()->GetInteractorStyle(),
    vtkCommand::WindowLevelEvent,
    this,
    SLOT( WindowLevelChanged( vtkObject *, unsigned long, void *, void *,
                              vtkCommand *) ),
    &whichWindow[2], 1.0);

  vtkQtConnections->Connect(
    imageView2DPanelWithControls1->GetImageViewer2()->GetRenderWindow()->
    GetInteractor()->GetInteractorStyle(),
    vtkCommand::WindowLevelEvent,
    this,
    SLOT( WindowLevelChanged( vtkObject *, unsigned long, void *, void *,
                              vtkCommand *) ),
    &whichWindow[0], 1.0);

  // UpdateDWIDiffusionVectorActors();

  // bQCResultLoad = false;
  DTIPrepPanel->Clear_VC_Status();
  // DTIPrepPanel->GetQCResult().Clear();

}

void GMainWindow::on_actionOpenDWINrrd_triggered()
{
  QString DWINrrdFile = QFileDialog::getOpenFileName( this, tr(
                                                        "Open nrrd DWI"), QDir::currentPath(),
                                                      tr("Nrrd Files (*.nhdr *.nrrd)") );

  if( DWINrrdFile.contains("_QCed.nrrd", Qt::CaseSensitive) )
    {
    // User loads QCed file

    QString Messg = QString(
        " You are loading QCed file! Two options for loaded QCed file: 1) Visual checking 2) Running again. Do you want to continue ?" );
    QMessageBox msgBox;
    msgBox.setText( Messg );
    QPushButton * Yes = msgBox.addButton( tr("Yes"), QMessageBox::ActionRole);
    QPushButton * No = msgBox.addButton( tr("No"), QMessageBox::ActionRole);
    msgBox.exec();

    if( msgBox.clickedButton() == Yes )
      {
      ReloadQCedDWI( DWINrrdFile );
      return;

      }

    if( msgBox.clickedButton() == No )
      {
      emit Signal_actionOpenDWINrrd_triggered();
      return;
      }

    }
  else
    {

    if( DWINrrdFile.length() > 0 )
      {
      itk::NrrdImageIO::Pointer NrrdImageIO = itk::NrrdImageIO::New();
      DwiReader = DwiReaderType::New();
      DwiReader->SetImageIO(NrrdImageIO);

      try
        {
        DwiReader->SetFileName( DWINrrdFile.toStdString() );

        QString str;
        str.append( tr("Loading ") );
        str.append(DWINrrdFile);
        str.append(" ...");
        statusBar()->showMessage(str, 2000);
        std::cout << "Loading in GMainWindow: " << DWINrrdFile.toStdString()
                  << " ... ";
        DwiReader->Update();
        }
      catch( itk::ExceptionObject & e )
        {
        std::cout << e.GetDescription() << std::endl;
        bDwiLoaded = false;
        return;
        }
      }
    else
      {
      std::cout << "Dwi file name not set" << std::endl;
      bDwiLoaded = false;
      return;
      }
    statusBar()->showMessage(tr(" done"), 2000);
    std::cout << "done " << std::endl;
    // std::cout<<"Image size"<<
    // DwiReader->GetOutput()->GetLargestPossibleRegion().GetSize().GetSizeDimension()<<": ";
    // std::cout<<DwiReader->GetOutput()->GetLargestPossibleRegion().GetSize()[0]<<" ";
    // std::cout<<DwiReader->GetOutput()->GetLargestPossibleRegion().GetSize()[1]<<" ";
    // std::cout<<DwiReader->GetOutput()->GetLargestPossibleRegion().GetSize()[2]<<std::endl;
    // std::cout<<"Pixel Vector Length: "<<DwiReader->GetOutput()->GetVectorLength()<<std::endl;

    bDwiLoaded = true;
    DWIImage = DwiReader->GetOutput();

    setWindowTitle(tr("DTIPrep Tools(Qt4) - ") + DWINrrdFile);

    // update DTIPrepPanel
    DTIPrepPanel->SetFileName(DWINrrdFile);
    DTIPrepPanel->SetName(DWINrrdFile);
    DTIPrepPanel->SetDWIImage(DWIImage);
    DTIPrepPanel->UpdatePanelDWI();

    // update 2D/3D Image display
    componentExtractor1 = FilterType::New();
    componentExtractor2 = FilterType::New();
    componentExtractor3 = FilterType::New();

    gradientConnecter1 = ItkVtkImageFilterTypeUShort::New();
    gradientConnecter2 = ItkVtkImageFilterTypeUShort::New();
    gradientConnecter3 = ItkVtkImageFilterTypeUShort::New();

    //  componentExtractor1->SetInput(DwiReader->GetOutput());
    //  componentExtractor2->SetInput(DwiReader->GetOutput());
    //  componentExtractor3->SetInput(DwiReader->GetOutput());

    componentExtractor1->SetInput( DWIImage );
    componentExtractor2->SetInput( DWIImage );
    componentExtractor3->SetInput( DWIImage );

    componentExtractor1->SetIndex( 0 );
    componentExtractor2->SetIndex( 0 );
    componentExtractor3->SetIndex( 0 );

    gradientConnecter1->SetInput( componentExtractor1->GetOutput() );
    gradientConnecter2->SetInput( componentExtractor2->GetOutput() );
    gradientConnecter3->SetInput( componentExtractor3->GetOutput() );

    gradientConnecter1->Update();
    gradientConnecter2->Update();
    gradientConnecter3->Update();

    UpdateImagePlaneWidgets(0, 0, 0);
    UpdateImageView2DWindows( 0, 0, 0, DwiReader->GetOutput()->GetVectorLength() );

    whichWindow[0] = 0;
    whichWindow[1] = 1;
    whichWindow[2] = 2;

    vtkQtConnections->Connect(
      imageView2DPanelWithControls2->GetImageViewer2()->GetRenderWindow()->
      GetInteractor()->GetInteractorStyle(),
      vtkCommand::WindowLevelEvent,
      this,
      SLOT( WindowLevelChanged( vtkObject *, unsigned long, void *, void *,
                                vtkCommand *) ),
      &whichWindow[1], 1.0);

    vtkQtConnections->Connect(
      imageView2DPanelWithControls3->GetImageViewer2()->GetRenderWindow()->
      GetInteractor()->GetInteractorStyle(),
      vtkCommand::WindowLevelEvent,
      this,
      SLOT( WindowLevelChanged( vtkObject *, unsigned long, void *, void *,
                                vtkCommand *) ),
      &whichWindow[2], 1.0);

    vtkQtConnections->Connect(
      imageView2DPanelWithControls1->GetImageViewer2()->GetRenderWindow()->
      GetInteractor()->GetInteractorStyle(),
      vtkCommand::WindowLevelEvent,
      this,
      SLOT( WindowLevelChanged( vtkObject *, unsigned long, void *, void *,
                                vtkCommand *) ),
      &whichWindow[0], 1.0);

    UpdateDWIDiffusionVectorActors( DWIImage );

    bQCResultLoad = false;
    DTIPrepPanel->Clear_VC_Status();
    DTIPrepPanel->GetQCResult().Clear();
    }
}

void GMainWindow::UpdateDWIDiffusionVectorActors( DwiImageType::Pointer _DWIImage)
{
  itk::MetaDataDictionary imgMetaDictionary = _DWIImage->GetMetaDataDictionary();    //

  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string                              metaString;

  pvtkRenderer_3DView->RemoveActor(actorDirFile);

  actorDirFile->GetParts()->RemoveAllItems();
  float  vect3d[3];
  double length = 0;
  min_length_vec = 1.0;
  for( ; itKey != imgMetaKeys.end(); itKey++ )
    {
    // double x,y,z;
    itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
    if( itKey->find("DWMRI_gradient") != std::string::npos )
      {
      std::istringstream iss(metaString);
      iss >> vect3d[0] >> vect3d[1] >> vect3d[2];

      if( vect3d[0] == 0.0 &&  vect3d[1] == 0.0 &&  vect3d[2] == 0.0 )
        {
        continue;
        }

      vtkLineSource *LineSource    =  vtkLineSource::New();
      // LineSource->SetPoint1(0.0, 0.0, 0.0);

      LineSource->SetPoint1(-vect3d[0], -vect3d[1], -vect3d[2]);
      LineSource->SetPoint2(vect3d[0], vect3d[1], vect3d[2]);

      length = double (sqrt(vect3d[0] * vect3d[0] + vect3d[1] * vect3d[1] + vect3d[2] * vect3d[2]) );
      // std::cout << "length vect " << length << std::endl;
      if( length <= min_length_vec )
        {
        min_length_vec = length;
        }

      vtkTubeFilter *TubeFilter = vtkTubeFilter::New();
#if (VTK_MAJOR_VERSION < 6)
      TubeFilter->SetInput( LineSource->GetOutput() );
#else
      //TubeFilter->SetInputData( LineSource->GetOutput() );
      TubeFilter->SetInputConnection( LineSource->GetOutputPort() );
#endif
      TubeFilter->SetRadius(0.01);
      TubeFilter->SetNumberOfSides(10);
      TubeFilter->SetVaryRadiusToVaryRadiusOff();
      TubeFilter->SetCapping(1);
      TubeFilter->Update(); // /

      vtkPolyDataMapper *mapperLocal = vtkPolyDataMapper::New();
#if (VTK_MAJOR_VERSION < 6)
      mapperLocal->SetInput( TubeFilter->GetOutput() );
#else
      //mapperLocal->SetInputData( TubeFilter->GetOutput() );
      mapperLocal->SetInputConnection( TubeFilter->GetOutputPort() );
#endif

      vtkActor *actorLocal = vtkActor::New();
      actorLocal->SetMapper(mapperLocal);
      actorLocal->GetProperty()->SetColor(0.0, 0.0, 1.0);
      //       actorLocal->GetProperty()->SetOpacity(0.5);
      actorDirFile->AddPart(actorLocal);
      }
    }
  pvtkRenderer_3DView->AddActor(actorDirFile);
  //pvtkRenderer_3DView->AddViewProp(actorDirFile);
  // std::cout << "min_length_vec " << min_length_vec << std::endl;

  this->doubleSpinBox_SphereRadius->setMinimum(min_length_vec);

  //SphereSource->SetRadius(min_length_vec);
  actorDirFile->SetVisibility(1);
  pvtkRenderer_3DView->ResetCamera();
  actorDirFile->SetVisibility( actionFrom_DWI->isChecked() );
  qvtkWidget_3DView->GetRenderWindow()->Render();

  actionFrom_DWI->setEnabled(1);
  actionFrom_DWI->setCheckable(1);
}

void GMainWindow::on_actionFrom_Protocol_toggled( bool check)
{
  if( actorDirProtocol )
    {
    actorDirProtocol->SetVisibility(check);
    pvtkRenderer_3DView->ResetCamera();
    qvtkWidget_3DView->GetRenderWindow()->Render();
    }
}

void GMainWindow::on_actionFrom_DWI_toggled( bool check)
{
  if( actorDirFile )
    {
    actorDirFile->SetVisibility(check);
    pvtkRenderer_3DView->ResetCamera();
    qvtkWidget_3DView->GetRenderWindow()->Render();
    }
}

void GMainWindow::on_actionIncluded_toggled( bool check)
{
  if( actorDirInclude )
    {
    actorDirInclude->SetVisibility(check);
    pvtkRenderer_3DView->ResetCamera();
    qvtkWidget_3DView->GetRenderWindow()->Render();
    }
}

void GMainWindow::SetactionIncluded()
{
  actionIncluded->setDisabled(0);
}

void GMainWindow::on_actionSphere_toggled( bool check)
{
  if( actorSphere )
    {
    actorSphere->SetVisibility(check);
    pvtkRenderer_3DView->ResetCamera();
    qvtkWidget_3DView->GetRenderWindow()->Render();
    }
}

void GMainWindow::UpdateOutputDWIDiffusionVectorActors()
{
  pvtkRenderer_3DView->RemoveActor(actorDirInclude);
  actorDirInclude->GetParts()->RemoveAllItems();
  for( unsigned int i = 0;
       i < DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult().size();
       i++ )
    {

    if( (DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].VisualChecking == -1 &&
         DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
         processing == QCResult::GRADIENT_INCLUDE)
        || (DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].VisualChecking == -1 &&
            DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
            processing == QCResult::GRADIENT_EDDY_MOTION_CORRECTED) ||
        (DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
         VisualChecking == QCResult::GRADIENT_INCLUDE) )
      {

      if( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
          CorrectedDir[0] == 0.0
          && DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
          CorrectedDir[1] == 0.0
          && DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
          CorrectedDir[2] == 0.0    )
        {
        continue;
        }

      // std::cout<<"IncludeDirTest"<<std::endl;
      vtkLineSource *LineSource    =  vtkLineSource::New();

      LineSource->SetPoint1(
        DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
        CorrectedDir[0],
        DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
        CorrectedDir[1],
        DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
        CorrectedDir[2]  );

      LineSource->SetPoint2(
        -DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
        CorrectedDir[0],
        -DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
        CorrectedDir[1],
        -DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
        CorrectedDir[2] );

      vtkTubeFilter *TubeFilter = vtkTubeFilter::New();
#if (VTK_MAJOR_VERSION < 6)
      TubeFilter->SetInput( LineSource->GetOutput() );
#else
      TubeFilter->SetInputData( LineSource->GetOutput() );
#endif
      TubeFilter->SetRadius(0.01);
      TubeFilter->SetNumberOfSides(10);
      TubeFilter->SetVaryRadiusToVaryRadiusOff();
      TubeFilter->SetCapping(1);
      TubeFilter->Update();

      vtkPolyDataMapper *mapperLocal = vtkPolyDataMapper::New();
#if (VTK_MAJOR_VERSION < 6)
      mapperLocal->SetInput( TubeFilter->GetOutput() );
#else
      mapperLocal->SetInputData( TubeFilter->GetOutput() );
#endif

      vtkActor *actorLocal = vtkActor::New();
      actorLocal->SetMapper(mapperLocal);

      actorLocal->GetProperty()->SetColor(0.30, 0.90, 0.30);
      actorDirInclude->AddPart(actorLocal);
      }
    }
  pvtkRenderer_3DView->AddActor( actorDirInclude );
  actorDirInclude->SetVisibility( actionIncluded->isChecked() );
  // pvtkRenderer_3DView->ResetCamera();
  qvtkWidget_3DView->GetRenderWindow()->Render();

  actionIncluded->setEnabled(1);
  actionIncluded->setCheckable(1);
}

void GMainWindow::UpdateOutputDWIDiffusionVectorActors_VC()
{
  pvtkRenderer_3DView->RemoveActor(actorDirInclude);
  actorDirInclude->GetParts()->RemoveAllItems();
  for( unsigned int i = 0;
       i < DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult().size();
       i++ )
    {
    if( (DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].VisualChecking == -1 &&
         DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
         processing == QCResult::GRADIENT_INCLUDE)
        || (DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].VisualChecking == -1 &&
            DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
            processing == QCResult::GRADIENT_EDDY_MOTION_CORRECTED) ||
        (DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
         VisualChecking == QCResult::GRADIENT_INCLUDE) )
      {
      if( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
          CorrectedDir[0] == 0.0
          && DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
          CorrectedDir[1] == 0.0
          && DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
          CorrectedDir[2] == 0.0    )
        {
        continue;
        }

      vtkLineSource *LineSource    =  vtkLineSource::New();

      LineSource->SetPoint1(
        DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
        CorrectedDir[0],
        DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
        CorrectedDir[1],
        DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
        CorrectedDir[2]  );

      LineSource->SetPoint2(
        -DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
        CorrectedDir[0],
        -DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
        CorrectedDir[1],
        -DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[i].
        CorrectedDir[2] );

      vtkTubeFilter *TubeFilter = vtkTubeFilter::New();
#if (VTK_MAJOR_VERSION < 6)
      TubeFilter->SetInput( LineSource->GetOutput() );
#else
      TubeFilter->SetInputData( LineSource->GetOutput() );
#endif
      TubeFilter->SetRadius(0.01);
      TubeFilter->SetNumberOfSides(10);
      TubeFilter->SetVaryRadiusToVaryRadiusOff();
      TubeFilter->SetCapping(1);
      TubeFilter->Update();

      vtkPolyDataMapper *mapperLocal = vtkPolyDataMapper::New();
#if (VTK_MAJOR_VERSION < 6)
      mapperLocal->SetInput( TubeFilter->GetOutput() );
#else
      mapperLocal->SetInputData( TubeFilter->GetOutput() );
#endif

      vtkActor *actorLocal = vtkActor::New();
      actorLocal->SetMapper(mapperLocal);

      actorLocal->GetProperty()->SetColor(0.30, 0.90, 0.30);
      actorDirInclude->AddPart(actorLocal);
      }
    }
  pvtkRenderer_3DView->AddActor( actorDirInclude );
  actorDirInclude->SetVisibility( actionIncluded->isChecked() );
  // pvtkRenderer_3DView->ResetCamera();
  qvtkWidget_3DView->GetRenderWindow()->Render();

  actionIncluded->setEnabled(1);
  actionIncluded->setCheckable(1);
}

void GMainWindow::UpdateProtocolDiffusionVectorActors()
{
  pvtkRenderer_3DView->RemoveActor(actorDirProtocol);
  actorDirProtocol->GetParts()->RemoveAllItems();
  for( unsigned int i = 0;
       i < DTIPrepPanel->GetProtocol().GetDiffusionProtocol().gradients.size();
       i++ )
    {
    if( DTIPrepPanel->GetProtocol().GetDiffusionProtocol().gradients[i][0] ==
        0.0
        && DTIPrepPanel->GetProtocol().GetDiffusionProtocol().gradients[i][1]
        == 0.0
        && DTIPrepPanel->GetProtocol().GetDiffusionProtocol().gradients[i][2]
        == 0.0    )
      {
      continue;
      }

    vtkLineSource *LineSource    =  vtkLineSource::New();
    // LineSource->SetPoint1(0.0, 0.0, 0.0);

    LineSource->SetPoint1(
      DTIPrepPanel->GetProtocol().GetDiffusionProtocol().gradients[i][0],
      DTIPrepPanel->GetProtocol().GetDiffusionProtocol().gradients[i][1],
      DTIPrepPanel->GetProtocol().GetDiffusionProtocol().gradients[i][2]
      );

    LineSource->SetPoint2(
      -DTIPrepPanel->GetProtocol().GetDiffusionProtocol().gradients[i][0],
      -DTIPrepPanel->GetProtocol().GetDiffusionProtocol().gradients[i][1],
      -DTIPrepPanel->GetProtocol().GetDiffusionProtocol().gradients[i][2]
      );

    vtkTubeFilter *TubeFilter = vtkTubeFilter::New();
#if (VTK_MAJOR_VERSION < 6)
    TubeFilter->SetInput( LineSource->GetOutput() );
#else
    TubeFilter->SetInputData( LineSource->GetOutput() );
#endif
    TubeFilter->SetRadius(0.01);
    TubeFilter->SetNumberOfSides(10);
    TubeFilter->SetVaryRadiusToVaryRadiusOff();
    TubeFilter->SetCapping(1);
    TubeFilter->Update(); // /

    vtkPolyDataMapper *mapperLocal = vtkPolyDataMapper::New();
#if (VTK_MAJOR_VERSION < 6)
    mapperLocal->SetInput( TubeFilter->GetOutput() );
#else
    mapperLocal->SetInputData( TubeFilter->GetOutput() );
#endif

    vtkActor *actorLocal = vtkActor::New();
    actorLocal->SetMapper(mapperLocal);

    actorLocal->GetProperty()->SetColor(0.70, 0.88, 0.93);
    actorDirProtocol->AddPart(actorLocal);
    }
  pvtkRenderer_3DView->AddActor(actorDirProtocol);
  actorDirProtocol->SetVisibility( actionFrom_Protocol->isChecked() );
  // pvtkRenderer_3DView->ResetCamera();
  qvtkWidget_3DView->GetRenderWindow()->Render();

  actionFrom_Protocol->setEnabled(1);
  actionFrom_Protocol->setCheckable(1);
}

void GMainWindow::on_actionOpen_XML_triggered()
{
  DTIPrepPanel->OpenXML();
  DTIPrepPanel->SetProtocolTreeEditable( true );
  UpdateProtocolDiffusionVectorActors();
}

void GMainWindow::on_actionOpenMappingXML_triggered()
{
  OpenMappingXML();
}

// void  GMainWindow::on_actionOpen_QCed_DWI_triggered()
// {
//  OpenQCedDWI();
// }

void GMainWindow::on_actionDicom2NrrdPanel_triggered()
{
  createDockPanels_Dicom2NrrdPanel();
}

void GMainWindow::on_actionIntensityMotionCheckPanel_triggered()
{
  createDockPanels_IntensityMotionCheckPanel();
}

void GMainWindow::on_actionImageView1_triggered()
{
  createDockPanels_imageView2DPanel1();
}

void GMainWindow::on_actionImageView2_triggered()
{
  createDockPanels_imageView2DPanel2();
}

void GMainWindow::on_actionImageView3_triggered()
{
  createDockPanels_imageView2DPanel3();
}

void GMainWindow::on_actionQCResult_triggered()
{
  DTIPrepPanel->OpenXML_ResultFile();
}

bool GMainWindow::CreateImagePlaneWidgets(vtkImageData *GradientImage)
{
  int dims[3] = { 100, 100, 100};

  GradientImage->GetDimensions(dims);

  vtkCellPicker *picker = vtkCellPicker::New();
  picker->SetTolerance(0.005);

  vtkProperty *ipwProp = vtkProperty::New();
  // PrepareScalarImageData(this->GetImagePlaneSourceX(),IMAGE_PLANE_X); //

  // if(!planeWidgetX) planeWidgetX= vtkImagePlaneWidget::New();
  planeWidgetX->KeyPressActivationOn();
  planeWidgetX->SetKeyPressActivationValue('x');
  planeWidgetX->SetInteractor( qvtkWidget->GetRenderWindow()->GetInteractor() );
  // planeWidgetX->SetPicker(picker);
  planeWidgetX->RestrictPlaneToVolumeOn();
  planeWidgetX->GetPlaneProperty()->SetColor(1, 0, 0);
  planeWidgetX->SetTexturePlaneProperty(ipwProp);
  planeWidgetX->TextureInterpolateOff();
  // planeWidgetX->SetResliceInterpolateToNearestNeighbour();
#if (VTK_MAJOR_VERSION < 6)
  planeWidgetX->SetInput(GradientImage);
#else
  planeWidgetX->SetInputData(GradientImage);
#endif
  planeWidgetX->SetPlaneOrientationToXAxes();
  planeWidgetX->SetSliceIndex(dims[0] / 2);
  planeWidgetX->DisplayTextOn();
  planeWidgetX->RestrictPlaneToVolumeOn();
  planeWidgetX->SetMarginSizeX(0);
  planeWidgetX->SetMarginSizeY(0);
  planeWidgetX->GetMarginProperty()->SetColor(1.0, 1.0, 0.0);
  planeWidgetX->GetTexturePlaneProperty()->SetOpacity(1.0);
  planeWidgetX->GetSelectedPlaneProperty()->SetColor(1.0, 1.0, 0.0);
  planeWidgetX->On();

  // if(!planeWidgetY) planeWidgetY = vtkImagePlaneWidget::New();
  planeWidgetY->KeyPressActivationOn();
  planeWidgetY->SetKeyPressActivationValue('y');
  planeWidgetY->SetKeyPressActivation(1);
  planeWidgetY->SetInteractor( qvtkWidget->GetRenderWindow()->GetInteractor() );
  // planeWidgetY->SetPicker(picker);
  planeWidgetY->RestrictPlaneToVolumeOn();
  planeWidgetY->GetPlaneProperty()->SetColor(1, 1, 0);
  planeWidgetY->SetTexturePlaneProperty(ipwProp);
  planeWidgetY->TextureInterpolateOff();
  // planeWidgetY->SetResliceInterpolateToNearestNeighbour();
#if (VTK_MAJOR_VERSION < 6)
  planeWidgetY->SetInput(GradientImage);
#else
  planeWidgetY->SetInputData(GradientImage);
#endif
  planeWidgetY->SetPlaneOrientationToYAxes();
  planeWidgetY->SetSliceIndex(dims[1] / 2);
  planeWidgetY->SetLookupTable( planeWidgetX->GetLookupTable() );
  planeWidgetY->DisplayTextOn();
  planeWidgetY->UpdatePlacement();
  planeWidgetY->RestrictPlaneToVolumeOn();
  planeWidgetY->SetMarginSizeX(0);
  planeWidgetY->SetMarginSizeY(0);
  planeWidgetY->GetTexturePlaneProperty()->SetOpacity(1.0);
  planeWidgetY->GetPlaneProperty()->SetColor(0.0, 1.0, 0.0);
  planeWidgetY->GetMarginProperty()->SetColor(1.0, 1.0, 0.0);
  planeWidgetY->GetSelectedPlaneProperty()->SetColor(1.0, 1.0, 0.0);
  planeWidgetY->On();

  // if(!planeWidgetZ) planeWidgetZ = vtkImagePlaneWidget::New();
  planeWidgetZ->KeyPressActivationOn();
  planeWidgetZ->SetKeyPressActivationValue('z');
  planeWidgetZ->SetInteractor( qvtkWidget->GetRenderWindow()->GetInteractor() );
  // planeWidgetZ->SetPicker(picker);
  planeWidgetZ->RestrictPlaneToVolumeOn();
  planeWidgetZ->GetPlaneProperty()->SetColor(0, 0, 1);
  planeWidgetZ->SetTexturePlaneProperty(ipwProp);
  planeWidgetZ->TextureInterpolateOff();
  // planeWidgetZ->SetResliceInterpolateToNearestNeighbour();
#if (VTK_MAJOR_VERSION < 6)
  planeWidgetZ->SetInput(GradientImage);
#else
  planeWidgetZ->SetInputData(GradientImage);
#endif
  planeWidgetZ->SetPlaneOrientationToZAxes();
  planeWidgetZ->SetSliceIndex(dims[2] / 2);
  planeWidgetZ->SetLookupTable( planeWidgetX->GetLookupTable() );
  planeWidgetZ->DisplayTextOn();
  planeWidgetZ->RestrictPlaneToVolumeOn();
  planeWidgetZ->SetMarginSizeX(0);
  planeWidgetZ->SetMarginSizeY(0);
  planeWidgetZ->GetMarginProperty()->SetColor(1.0, 1.0, 0.0);
  planeWidgetZ->GetTexturePlaneProperty()->SetOpacity(1.0);
  planeWidgetZ->GetSelectedPlaneProperty()->SetColor(1.0, 1.0, 0.0);
  planeWidgetZ->On();

  GetRenderer()->ResetCamera();
  qvtkWidget->GetRenderWindow()->Render();

  return TRUE;
}

bool GMainWindow::CreateImagePlaneWidgets( vtkImageData *GradientImage1,
                                           vtkImageData *GradientImage2,
                                           vtkImageData *GradientImage3 )
{
  int dims1[3] = { 100, 100, 100};

  GradientImage1->GetDimensions(dims1);

  vtkCellPicker *picker = vtkCellPicker::New();
  picker->SetTolerance(0.005);

  vtkProperty *ipwProp = vtkProperty::New();
  // PrepareScalarImageData(this->GetImagePlaneSourceX(),IMAGE_PLANE_X); //

  // if(!planeWidgetX) planeWidgetX= vtkImagePlaneWidget::New();
  planeWidgetX->KeyPressActivationOn();
  planeWidgetX->SetKeyPressActivationValue('x');
  planeWidgetX->SetInteractor( qvtkWidget->GetRenderWindow()->GetInteractor() );
  // planeWidgetX->SetPicker(picker);
  planeWidgetX->RestrictPlaneToVolumeOn();
  planeWidgetX->GetPlaneProperty()->SetColor(1, 0, 0);
  planeWidgetX->SetTexturePlaneProperty(ipwProp);
  planeWidgetX->TextureInterpolateOff();
  // planeWidgetX->SetResliceInterpolateToNearestNeighbour();
#if (VTK_MAJOR_VERSION < 6)
  planeWidgetX->SetInput(GradientImage1);
#else
  planeWidgetX->SetInputData(GradientImage1);
#endif
  planeWidgetX->SetPlaneOrientationToXAxes();
  planeWidgetX->SetSliceIndex(dims1[0] / 2);
  planeWidgetX->DisplayTextOn();
  planeWidgetX->RestrictPlaneToVolumeOn();
  planeWidgetX->SetMarginSizeX(0);
  planeWidgetX->SetMarginSizeY(0);
  planeWidgetX->GetMarginProperty()->SetColor(1.0, 1.0, 0.0);
  planeWidgetX->GetTexturePlaneProperty()->SetOpacity(1.0);
  planeWidgetX->GetSelectedPlaneProperty()->SetColor(1.0, 1.0, 0.0);
  planeWidgetX->On();

  int dims2[3] = { 100, 100, 100};
  GradientImage2->GetDimensions(dims2);

  // if(!planeWidgetY) planeWidgetY = vtkImagePlaneWidget::New();
  planeWidgetY->KeyPressActivationOn();
  planeWidgetY->SetKeyPressActivationValue('y');
  planeWidgetY->SetKeyPressActivation(1);
  planeWidgetY->SetInteractor( qvtkWidget->GetRenderWindow()->GetInteractor() );
  // planeWidgetY->SetPicker(picker);
  planeWidgetY->RestrictPlaneToVolumeOn();
  planeWidgetY->GetPlaneProperty()->SetColor(1, 1, 0);
  planeWidgetY->SetTexturePlaneProperty(ipwProp);
  planeWidgetY->TextureInterpolateOff();
  // planeWidgetY->SetResliceInterpolateToNearestNeighbour();
#if (VTK_MAJOR_VERSION < 6)
  planeWidgetY->SetInput(GradientImage2);
#else
  planeWidgetY->SetInputData(GradientImage2);
#endif
  planeWidgetY->SetPlaneOrientationToYAxes();
  planeWidgetY->SetSliceIndex(dims2[1] / 2);
  // planeWidgetY->SetLookupTable( planeWidgetX->GetLookupTable());
  planeWidgetY->DisplayTextOn();
  planeWidgetY->UpdatePlacement();
  planeWidgetY->RestrictPlaneToVolumeOn();
  planeWidgetY->SetMarginSizeX(0);
  planeWidgetY->SetMarginSizeY(0);
  planeWidgetY->GetTexturePlaneProperty()->SetOpacity(1.0);
  planeWidgetY->GetPlaneProperty()->SetColor(0.0, 1.0, 0.0);
  planeWidgetY->GetMarginProperty()->SetColor(1.0, 1.0, 0.0);
  planeWidgetY->GetSelectedPlaneProperty()->SetColor(1.0, 1.0, 0.0);
  planeWidgetY->On();

  int dims3[3] = { 100, 100, 100};
  GradientImage3->GetDimensions(dims3);

  // if(!planeWidgetZ) planeWidgetZ = vtkImagePlaneWidget::New();
  planeWidgetZ->KeyPressActivationOn();
  planeWidgetZ->SetKeyPressActivationValue('z');
  planeWidgetZ->SetInteractor( qvtkWidget->GetRenderWindow()->GetInteractor() );
  // planeWidgetZ->SetPicker(picker);
  planeWidgetZ->RestrictPlaneToVolumeOn();
  planeWidgetZ->GetPlaneProperty()->SetColor(0, 0, 1);
  planeWidgetZ->SetTexturePlaneProperty(ipwProp);
  planeWidgetZ->TextureInterpolateOff();
  // planeWidgetZ->SetResliceInterpolateToNearestNeighbour();
#if (VTK_MAJOR_VERSION < 6)
  planeWidgetZ->SetInput(GradientImage3);
#else
  planeWidgetZ->SetInputData(GradientImage3);
#endif
  planeWidgetZ->SetPlaneOrientationToZAxes();
  planeWidgetZ->SetSliceIndex(dims3[2] / 2);
  // planeWidgetZ->SetLookupTable( planeWidgetX->GetLookupTable());
  planeWidgetZ->DisplayTextOn();
  planeWidgetZ->RestrictPlaneToVolumeOn();
  planeWidgetZ->SetMarginSizeX(0);
  planeWidgetZ->SetMarginSizeY(0);
  planeWidgetZ->GetMarginProperty()->SetColor(1.0, 1.0, 0.0);
  planeWidgetZ->GetTexturePlaneProperty()->SetOpacity(1.0);
  planeWidgetZ->GetSelectedPlaneProperty()->SetColor(1.0, 1.0, 0.0);
  planeWidgetZ->On();

  // enum   { VTK_CURSOR_ACTION = 0, VTK_SLICE_MOTION_ACTION = 1,
  // VTK_WINDOW_LEVEL_ACTION = 2 }
  planeWidgetX->SetLeftButtonAction(
    vtkImagePlaneWidget::VTK_WINDOW_LEVEL_ACTION);
  planeWidgetX->SetRightButtonAction(vtkImagePlaneWidget::VTK_CURSOR_ACTION);

  planeWidgetY->SetLeftButtonAction(
    vtkImagePlaneWidget::VTK_WINDOW_LEVEL_ACTION);
  planeWidgetY->SetRightButtonAction(vtkImagePlaneWidget::VTK_CURSOR_ACTION);

  planeWidgetZ->SetLeftButtonAction(
    vtkImagePlaneWidget::VTK_WINDOW_LEVEL_ACTION);
  planeWidgetZ->SetRightButtonAction(vtkImagePlaneWidget::VTK_CURSOR_ACTION);

  // IPWCallbackUpdateImage2D = vtkIPWCallbackUpdateImage2D::New();
  // IPWCallbackUpdateImage2D->orientation      =0; //0: Axial, 1:Sagittal; 2:
  // Coronal
  // IPWCallbackUpdateImage2D->whichMessage2DView  = 1;
  // IPWCallbackUpdateImage2D->whichMessageMPR    = MYWM_UPDATE_MPR_AXIAL;
  // planeWidgetZ-> AddObserver(
  // vtkCommand::InteractionEvent,IPWCallbackUpdateImage2D);
  // planeWidgetZ-> AddObserver(
  // vtkCommand::WindowLevelEvent,IPWCallbackUpdateImage2D);

  whichWindow[0] = 0;
  whichWindow[1] = 1;
  whichWindow[2] = 2;

  vtkQtConnections->Connect(planeWidgetX,
                            vtkCommand::InteractionEvent,
                            this->imageView2DPanelWithControls2,
                            SLOT( SliceIndexChanged( vtkObject *, unsigned long, void *, void *,
                                                     vtkCommand *) ),
                            &whichWindow[1], 1.0);

  vtkQtConnections->Connect(planeWidgetX,
                            vtkCommand::WindowLevelEvent,
                            this->imageView2DPanelWithControls2,
                            SLOT( WindowLevelChanged( vtkObject *, unsigned long, void *, void *,
                                                      vtkCommand *) ),
                            &whichWindow[1], 1.0);

  vtkQtConnections->Connect(planeWidgetY,
                            vtkCommand::InteractionEvent,
                            this->imageView2DPanelWithControls3,
                            SLOT( SliceIndexChanged( vtkObject *, unsigned long, void *, void *,
                                                     vtkCommand *) ),
                            &whichWindow[2], 1.0);

  vtkQtConnections->Connect(planeWidgetY,
                            vtkCommand::WindowLevelEvent,
                            this->imageView2DPanelWithControls3,
                            SLOT( WindowLevelChanged( vtkObject *, unsigned long, void *, void *,
                                                      vtkCommand *) ),
                            &whichWindow[2], 1.0);

  vtkQtConnections->Connect(planeWidgetZ,
                            vtkCommand::InteractionEvent,
                            this->imageView2DPanelWithControls1,
                            SLOT( SliceIndexChanged( vtkObject *, unsigned long, void *, void *,
                                                     vtkCommand *) ),
                            &whichWindow[0], 1.0);

  vtkQtConnections->Connect(planeWidgetZ,
                            vtkCommand::WindowLevelEvent,
                            this->imageView2DPanelWithControls1,
                            SLOT( WindowLevelChanged( vtkObject *, unsigned long, void *, void *,
                                                      vtkCommand *) ),
                            &whichWindow[0], 1.0);

  GetRenderer()->ResetCamera();
  qvtkWidget->GetRenderWindow()->Render();

  return TRUE;
}

void GMainWindow::UpdateImagePlaneWidgets(int /* gradient */)
{
  // componentExtractor = FilterType::New();
  // gradientConnecter = ItkVtkImageFilterTypeUShort::New();

  // componentExtractor->SetInput(DwiReader->GetOutput());
  // componentExtractor->SetIndex( gradient );

  // gradientConnecter->SetInput(componentExtractor->GetOutput());
  // gradientConnecter->Update();

  CreateImagePlaneWidgets( gradientConnecter->GetOutput() );
}

void GMainWindow::UpdateImagePlaneWidgets(int /* gradient1 */,
                                          int /* gradient2 */,
                                          int /* gradient3 */)
{
  // componentExtractor = FilterType::New();
  // gradientConnecter = ItkVtkImageFilterTypeUShort::New();

  // componentExtractor->SetInput(DwiReader->GetOutput());
  // componentExtractor->SetIndex( gradient );

  // gradientConnecter->SetInput(componentExtractor->GetOutput());
  // gradientConnecter->Update();

  // CreateImagePlaneWidgets(gradientConnecter->GetOutput());
  CreateImagePlaneWidgets( gradientConnecter1->GetOutput(),
                           gradientConnecter2->GetOutput(),
                           gradientConnecter3->GetOutput() );
}

void GMainWindow::UpdateImageView2DWindows(int /* gradient */,
                                           int numbGradients)
{
  // componentExtractor = FilterType::New();
  // gradientConnecter = ItkVtkImageFilterTypeUShort::New();

  // componentExtractor->SetInput(DwiReader->GetOutput());
  // componentExtractor->SetIndex( gradient );

  // gradientConnecter->SetInput(componentExtractor->GetOutput());
  // gradientConnecter->Update();

  imageView2DPanelWithControls1->Setup(
    gradientConnecter->GetOutput(), 0, 0, -1,
    ImageView2DPanelWithControls::ORIENTATION_AXIAL,  numbGradients,  0);
  imageView2DPanelWithControls2->Setup(
    gradientConnecter->GetOutput(), 0, 0, -1,
    ImageView2DPanelWithControls::ORIENTATION_SAGITTAL,  numbGradients, 1);
  imageView2DPanelWithControls3->Setup(
    gradientConnecter->GetOutput(), 0, 0, -1,
    ImageView2DPanelWithControls::ORIENTATION_CORONAL,  numbGradients,  2);
}

void GMainWindow::UpdateImageView2DWindows(int /* gradient1 */,
                                           int /* gradient2 */,
                                           int /* gradient3 */,
                                           int numbGradients)
{
  // componentExtractor = FilterType::New();
  // gradientConnecter = ItkVtkImageFilterTypeUShort::New();

  // componentExtractor->SetInput(DwiReader->GetOutput());
  // componentExtractor->SetIndex( gradient );

  // gradientConnecter->SetInput(componentExtractor->GetOutput());
  // gradientConnecter->Update();

  imageView2DPanelWithControls1->Setup(
    gradientConnecter3->GetOutput(), 0, 0, -1,
    ImageView2DPanelWithControls::ORIENTATION_AXIAL, numbGradients, 0);
  imageView2DPanelWithControls2->Setup(
    gradientConnecter1->GetOutput(), 0, 0, -1,
    ImageView2DPanelWithControls::ORIENTATION_SAGITTAL, numbGradients, 1);
  imageView2DPanelWithControls3->Setup(
    gradientConnecter2->GetOutput(), 0, 0, -1,
    ImageView2DPanelWithControls::ORIENTATION_CORONAL, numbGradients, 2);
}

void GMainWindow::OpacityChanged(int WinID,  double opacity)
{
  switch( WinID )
    {
    case 0:
      planeWidgetZ->GetTexturePlaneProperty()->SetOpacity(opacity);
      break;
    case 1:
      planeWidgetX->GetTexturePlaneProperty()->SetOpacity(opacity);
      break;
    case 2:
      planeWidgetY->GetTexturePlaneProperty()->SetOpacity(opacity);
      break;
    default:
      break;
    }

  qvtkWidget->GetRenderWindow()->Render();
}

void GMainWindow::VisibleChanged(int WinID, bool visible)
{
  switch( WinID )
    {
    case 0:
      if( visible )
        {
        planeWidgetZ->On();
        }
      else
        {
        planeWidgetZ->Off();
        }
      break;
    case 1:
      if( visible )
        {
        planeWidgetX->On();
        }
      else
        {
        planeWidgetX->Off();
        }
      break;
    case 2:
      if( visible )
        {
        planeWidgetY->On();
        }
      else
        {
        planeWidgetY->Off();
        }
      break;
    default:
      break;
    }
  qvtkWidget->GetRenderWindow()->Render();
}

void GMainWindow::GradientChanged(int WinID, int index)
{
  if( bDwiLoaded == false )
    {
    std::cout << " No DWINrrd image file is open " << std::endl;
    return;
    }

  if( bContentSyn )
    {
    componentExtractor1->SetIndex( index );
    componentExtractor2->SetIndex( index );
    componentExtractor3->SetIndex( index );

    gradientConnecter1->Update();
    gradientConnecter2->Update();
    gradientConnecter3->Update();

    imageView2DPanelWithControls1->GetHorizontalSlider_Gradient()->
    setSliderPosition(index);
    imageView2DPanelWithControls2->GetHorizontalSlider_Gradient()->
    setSliderPosition(index);
    imageView2DPanelWithControls3->GetHorizontalSlider_Gradient()->
    setSliderPosition(index);

    // std::cout<<"bQCResultLoad"<<bQCResultLoad<<std::endl;
    if( bQCResultLoad == true )
      {

      if( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == 0 ||
          ( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == -1 &&
            DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].processing <= 2) )
        {
        // std::cout << "WinID " << WinID << "indexID " << index <<" " <<  "Processing" <<
        // DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[ index ].processing << std::endl;
        imageView2DPanelWithControls1->GetLineEdit_Gradient()->setStyleSheet( "background-color: Green" );
        imageView2DPanelWithControls2->GetLineEdit_Gradient()->setStyleSheet( "background-color: Green" );
        imageView2DPanelWithControls3->GetLineEdit_Gradient()->setStyleSheet( "background-color: Green" );
        }

      else if( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == 6 ||
               ( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == -1 &&
                 DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].processing > 2) )
        {
        imageView2DPanelWithControls1->GetLineEdit_Gradient()->setStyleSheet( "background-color: red" );
        imageView2DPanelWithControls2->GetLineEdit_Gradient()->setStyleSheet( "background-color: red" );
        imageView2DPanelWithControls3->GetLineEdit_Gradient()->setStyleSheet( "background-color: red" );
        }
      }
    if( bQCResultLoad == false )
      {
      imageView2DPanelWithControls1->GetLineEdit_Gradient()->setStyleSheet( "background-color: white" );
      imageView2DPanelWithControls2->GetLineEdit_Gradient()->setStyleSheet( "background-color: white" );
      imageView2DPanelWithControls3->GetLineEdit_Gradient()->setStyleSheet( "background-color: white" );
      }

    imageView2DPanelWithControls1->GetImageViewer2()->Render();
    imageView2DPanelWithControls2->GetImageViewer2()->Render();
    imageView2DPanelWithControls3->GetImageViewer2()->Render();
    }
  else
    {

    switch( WinID )
      {
      case 0:
        componentExtractor1->SetIndex( index );
        // componentExtractor3->Update();
        gradientConnecter1->Update();
        imageView2DPanelWithControls1->GetHorizontalSlider_Gradient()->setSliderPosition(index);

        if( bQCResultLoad == true )
          {

          if( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == QCResult::GRADIENT_INCLUDE ||
              ( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == -1 &&
                DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].processing <= 2) )
            {
            imageView2DPanelWithControls1->GetLineEdit_Gradient()->setStyleSheet( "background-color: Green" );
            }

          else if( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == QCResult::GRADIENT_EXCLUDE_MANUALLY ||
                   ( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == -1 &&
                     DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].processing > 2) )
            {
            imageView2DPanelWithControls1->GetLineEdit_Gradient()->setStyleSheet( "background-color: red" );
            }
          }
        if( bQCResultLoad == false )
          {
          imageView2DPanelWithControls1->GetLineEdit_Gradient()->setStyleSheet( "background-color: white" );
          }

        imageView2DPanelWithControls1->GetImageViewer2()->Render();
        break;

      case 1:
        componentExtractor2->SetIndex( index);
        // componentExtractor1->Update();
        gradientConnecter2->Update();
        imageView2DPanelWithControls2->GetHorizontalSlider_Gradient()->setSliderPosition(index);
        if( bQCResultLoad == true )
          {
          if( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == QCResult::GRADIENT_INCLUDE ||
              ( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == -1 &&
                DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].processing <= 2) )
            {
            imageView2DPanelWithControls2->GetLineEdit_Gradient()->setStyleSheet( "background-color: Green" );
            }

          else if( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == QCResult::GRADIENT_EXCLUDE_MANUALLY ||
                   ( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == -1 &&
                     DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].processing > 2) )
            {
            imageView2DPanelWithControls2->GetLineEdit_Gradient()->setStyleSheet( "background-color: red" );
            }
          }
        if( bQCResultLoad == false )
          {
          imageView2DPanelWithControls2->GetLineEdit_Gradient()->setStyleSheet( "background-color: white" );
          }
        imageView2DPanelWithControls2->GetImageViewer2()->Render();
        break;

      case 2:
        componentExtractor3->SetIndex( index );
        // componentExtractor2->Update();
        gradientConnecter3->Update();
        imageView2DPanelWithControls3->GetHorizontalSlider_Gradient()->setSliderPosition(index);
        if( bQCResultLoad == true )
          {
          if( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == QCResult::GRADIENT_INCLUDE ||
              ( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == -1 &&
                DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].processing <= 2) )
            {
            imageView2DPanelWithControls3->GetLineEdit_Gradient()->setStyleSheet( "background-color: Green" );
            }

          else if( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == QCResult::GRADIENT_EXCLUDE_MANUALLY ||
                   ( DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].VisualChecking == -1 &&
                     DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].processing > 2) )
            {
            imageView2DPanelWithControls3->GetLineEdit_Gradient()->setStyleSheet( "background-color: red" );
            }
          }
        if( bQCResultLoad == false )
          {
          imageView2DPanelWithControls3->GetLineEdit_Gradient()->setStyleSheet( "background-color: white" );
          }
        imageView2DPanelWithControls3->GetImageViewer2()->Render();
        break;
      default:
        break;
      }
    }

  qvtkWidget->GetRenderWindow()->Render();
}

void GMainWindow::GradientUpdate( int index )
{

  QString Grad1 = QString( "Saving Gradient Change" );
  QString Grad2 = QString( "Status of gradient %1 based on Visual Checking" ).arg( index);

  QMessageBox msgBox;

  msgBox.setText( Grad2 );
  QPushButton * Include = msgBox.addButton( tr("Include"), QMessageBox::ActionRole);
  QPushButton * Exclude = msgBox.addButton( tr("Exclude"), QMessageBox::ActionRole);
  QPushButton * Nochange = msgBox.addButton( tr("NoChange"), QMessageBox::ActionRole);
  msgBox.exec();

  if( msgBox.clickedButton() == Include )
    {
    emit currentGradient_VC_Include( 0, index);
    emit currentGradient_VC_Include( 1, index);
    emit currentGradient_VC_Include( 2, index);

    std::cout << DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].processing << "processingTest"
              << std::endl;
    emit VisualCheckingStatus( index,  QCResult::GRADIENT_INCLUDE );
    }
  if( msgBox.clickedButton() == Exclude )
    {

    emit currentGradient_VC_Exclude( 0, index);
    emit currentGradient_VC_Exclude( 1, index);
    emit currentGradient_VC_Exclude( 2, index);

    emit VisualCheckingStatus( index,  QCResult::GRADIENT_EXCLUDE_MANUALLY);
    }
  if( msgBox.clickedButton() == Nochange )
    {

    int pro = DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[index].processing;
    if( pro <= 2 )
      {
      emit currentGradient_VC_Include( 0, index);
      emit currentGradient_VC_Include( 1, index);
      emit currentGradient_VC_Include( 2, index);
      }
    if( pro > 2 )
      {
      emit currentGradient_VC_Exclude( 0, index);
      emit currentGradient_VC_Exclude( 1, index);
      emit currentGradient_VC_Exclude( 2, index);
      }
    emit VisualCheckingStatus( index, -1);
    }
  // std::cout<< DTIPrepPanel->GetQCResult().GetIntensityMotionCheckResult()[ index
  // ].processing<<"processing_N"<<std::endl;

  UpdateOutputDWIDiffusionVectorActors();
  // UpdateOutputDWIDiffusionVectorActors_VC();
}

void GMainWindow::GradientChanged_VC_Include(int WinID, int index)
{
  // std::cout<<"VC2"<<std::endl;

  if( bDwiLoaded == false )
    {
    std::cout << " No DWINrrd image file is open " << std::endl;
    return;
    }
  if( bContentSyn )
    {
    componentExtractor1->SetIndex( index );
    componentExtractor2->SetIndex( index );
    componentExtractor3->SetIndex( index );

    gradientConnecter1->Update();
    gradientConnecter2->Update();
    gradientConnecter3->Update();

    imageView2DPanelWithControls1->GetHorizontalSlider_Gradient()->
    setSliderPosition(index);
    imageView2DPanelWithControls2->GetHorizontalSlider_Gradient()->
    setSliderPosition(index);
    imageView2DPanelWithControls3->GetHorizontalSlider_Gradient()->
    setSliderPosition(index);

    imageView2DPanelWithControls1->GetLineEdit_Gradient()->setStyleSheet( "background-color: Green" );
    imageView2DPanelWithControls2->GetLineEdit_Gradient()->setStyleSheet( "background-color: Green" );
    imageView2DPanelWithControls3->GetLineEdit_Gradient()->setStyleSheet( "background-color: Green" );

    imageView2DPanelWithControls1->GetImageViewer2()->Render();
    imageView2DPanelWithControls2->GetImageViewer2()->Render();
    imageView2DPanelWithControls3->GetImageViewer2()->Render();

    }
  else
    {

    switch( WinID )
      {
      case 0:
        componentExtractor3->SetIndex( index );
        // componentExtractor3->Update();
        gradientConnecter3->Update();
        imageView2DPanelWithControls1->GetImageViewer2()->Render();
        break;
      case 1:
        componentExtractor1->SetIndex( index);
        // componentExtractor1->Update();
        gradientConnecter1->Update();
        imageView2DPanelWithControls2->GetImageViewer2()->Render();
        break;
      case 2:
        componentExtractor2->SetIndex( index );
        // componentExtractor2->Update();
        gradientConnecter2->Update();
        imageView2DPanelWithControls3->GetImageViewer2()->Render();
        break;
      default:
        break;
      }
    }

  qvtkWidget->GetRenderWindow()->Render();
}

void GMainWindow::GradientChanged_VC_Exclude(int WinID, int index)
{
  if( bDwiLoaded == false )
    {
    std::cout << " No DWINrrd image file is open " << std::endl;
    return;
    }
  if( bContentSyn )
    {
    componentExtractor1->SetIndex( index );
    componentExtractor2->SetIndex( index );
    componentExtractor3->SetIndex( index );

    gradientConnecter1->Update();
    gradientConnecter2->Update();
    gradientConnecter3->Update();

    imageView2DPanelWithControls1->GetHorizontalSlider_Gradient()->
    setSliderPosition(index);
    imageView2DPanelWithControls2->GetHorizontalSlider_Gradient()->
    setSliderPosition(index);
    imageView2DPanelWithControls3->GetHorizontalSlider_Gradient()->
    setSliderPosition(index);

    imageView2DPanelWithControls1->GetLineEdit_Gradient()->setStyleSheet( "background-color: red" );
    imageView2DPanelWithControls2->GetLineEdit_Gradient()->setStyleSheet( "background-color: red" );
    imageView2DPanelWithControls3->GetLineEdit_Gradient()->setStyleSheet( "background-color: red" );

    imageView2DPanelWithControls1->GetImageViewer2()->Render();
    imageView2DPanelWithControls2->GetImageViewer2()->Render();
    imageView2DPanelWithControls3->GetImageViewer2()->Render();
    }
  else
    {

    switch( WinID )
      {
      case 0:
        componentExtractor3->SetIndex( index );
        // componentExtractor3->Update();
        gradientConnecter3->Update();
        imageView2DPanelWithControls1->GetImageViewer2()->Render();
        break;
      case 1:
        componentExtractor1->SetIndex( index);
        // componentExtractor1->Update();
        gradientConnecter1->Update();
        imageView2DPanelWithControls2->GetImageViewer2()->Render();
        break;
      case 2:
        componentExtractor2->SetIndex( index );
        // componentExtractor2->Update();
        gradientConnecter2->Update();
        imageView2DPanelWithControls3->GetImageViewer2()->Render();
        break;
      default:
        break;
      }
    }

  qvtkWidget->GetRenderWindow()->Render();
}

void GMainWindow::InterpolationChanged(int WinID, int index)
{
  if( bInterpolationSyn )
    {
    if( index == 1 )
      {
      planeWidgetX->TextureInterpolateOn();
      planeWidgetX->SetResliceInterpolateToNearestNeighbour();
      planeWidgetY->TextureInterpolateOn();
      planeWidgetY->SetResliceInterpolateToNearestNeighbour();
      planeWidgetZ->TextureInterpolateOn();
      planeWidgetZ->SetResliceInterpolateToNearestNeighbour();
      imageView2DPanelWithControls1->GetImageViewer2()->GetImageActor()->
      InterpolateOn();
      imageView2DPanelWithControls2->GetImageViewer2()->GetImageActor()->
      InterpolateOn();
      imageView2DPanelWithControls3->GetImageViewer2()->GetImageActor()->
      InterpolateOn();
      }
    else if( index == 2 )
      {
      planeWidgetX->TextureInterpolateOn();
      planeWidgetX->SetResliceInterpolateToLinear();
      planeWidgetY->TextureInterpolateOn();
      planeWidgetY->SetResliceInterpolateToLinear();
      planeWidgetZ->TextureInterpolateOn();
      planeWidgetZ->SetResliceInterpolateToLinear();
      imageView2DPanelWithControls1->GetImageViewer2()->GetImageActor()->
      InterpolateOn();
      imageView2DPanelWithControls2->GetImageViewer2()->GetImageActor()->
      InterpolateOn();
      imageView2DPanelWithControls3->GetImageViewer2()->GetImageActor()->
      InterpolateOn();
      }
    else if( index == 3 )
      {
      planeWidgetX->TextureInterpolateOn();
      planeWidgetX->SetResliceInterpolateToCubic();
      planeWidgetY->TextureInterpolateOn();
      planeWidgetY->SetResliceInterpolateToCubic();
      planeWidgetZ->TextureInterpolateOn();
      planeWidgetZ->SetResliceInterpolateToCubic();
      imageView2DPanelWithControls1->GetImageViewer2()->GetImageActor()->
      InterpolateOn();
      imageView2DPanelWithControls2->GetImageViewer2()->GetImageActor()->
      InterpolateOn();
      imageView2DPanelWithControls3->GetImageViewer2()->GetImageActor()->
      InterpolateOn();
      }
    else
      {
      planeWidgetX->TextureInterpolateOff();
      planeWidgetX->SetResliceInterpolate(5);
      planeWidgetY->TextureInterpolateOff();
      planeWidgetY->SetResliceInterpolate(5);
      planeWidgetZ->TextureInterpolateOff();
      planeWidgetZ->SetResliceInterpolate(5);
      imageView2DPanelWithControls1->GetImageViewer2()->GetImageActor()->
      InterpolateOff();                                                                   //
                                                                                          //
                                                                                          // this
                                                                                          //
                                                                                          // is
                                                                                          //
                                                                                          // a
                                                                                          //
                                                                                          // trick!
      imageView2DPanelWithControls2->GetImageViewer2()->GetImageActor()->
      InterpolateOff();                                                                   //
                                                                                          //
                                                                                          // this
                                                                                          //
                                                                                          // is
                                                                                          //
                                                                                          // a
                                                                                          //
                                                                                          // trick!
      imageView2DPanelWithControls3->GetImageViewer2()->GetImageActor()->
      InterpolateOff();                                                                   //
                                                                                          //
                                                                                          // this
                                                                                          //
                                                                                          // is
                                                                                          //
                                                                                          // a
                                                                                          //
                                                                                          // trick!
      }
    imageView2DPanelWithControls1->GetImageViewer2()->Render();
    imageView2DPanelWithControls2->GetImageViewer2()->Render();
    imageView2DPanelWithControls3->GetImageViewer2()->Render();

    imageView2DPanelWithControls1->GetComboBox_Interpolation()->setCurrentIndex(
      index);
    imageView2DPanelWithControls2->GetComboBox_Interpolation()->setCurrentIndex(
      index);
    imageView2DPanelWithControls3->GetComboBox_Interpolation()->setCurrentIndex(
      index);
    }
  else
    {
    switch( WinID )
      {
      case 0:
        if( index == 1 )
          {
          planeWidgetZ->TextureInterpolateOn();
          planeWidgetZ->SetResliceInterpolateToNearestNeighbour();
          }
        else if( index == 2 )
          {
          planeWidgetZ->TextureInterpolateOn();
          planeWidgetZ->SetResliceInterpolateToLinear();
          }
        else if( index == 3 )
          {
          planeWidgetZ->TextureInterpolateOn();
          planeWidgetZ->SetResliceInterpolateToCubic();
          }
        else
          {
          planeWidgetZ->TextureInterpolateOff();
          planeWidgetZ->SetResliceInterpolate(5);
          }                                                                                         //
                                                                                                    //
                                                                                                    // this
                                                                                                    //
                                                                                                    // is
                                                                                                    //
                                                                                                    // a
                                                                                                    //
                                                                                                    // trick!
        break;
      case 1:
        if( index == 1 )
          {
          planeWidgetX->TextureInterpolateOn();
          planeWidgetX->SetResliceInterpolateToNearestNeighbour();
          }
        else if( index == 2 )
          {
          planeWidgetX->TextureInterpolateOn();
          planeWidgetX->SetResliceInterpolateToLinear();
          }
        else if( index == 3 )
          {
          planeWidgetX->TextureInterpolateOn();
          planeWidgetX->SetResliceInterpolateToCubic();
          }
        else
          {
          planeWidgetX->TextureInterpolateOff();
          planeWidgetX->SetResliceInterpolate(5);
          }                                                                                         //
                                                                                                    //
                                                                                                    // this
                                                                                                    //
                                                                                                    // is
                                                                                                    //
                                                                                                    // a
                                                                                                    //
                                                                                                    // trick!
        break;
      case 2:
        if( index == 1 )
          {
          planeWidgetY->TextureInterpolateOn();
          planeWidgetY->SetResliceInterpolateToNearestNeighbour();
          }
        else if( index == 2 )
          {
          planeWidgetY->TextureInterpolateOn();
          planeWidgetY->SetResliceInterpolateToLinear();
          }
        else if( index == 3 )
          {
          planeWidgetY->TextureInterpolateOn();
          planeWidgetY->SetResliceInterpolateToCubic();
          }
        else
          {
          planeWidgetY->TextureInterpolateOff();
          planeWidgetY->SetResliceInterpolate(5);
          }                                                                                         //
                                                                                                    //
                                                                                                    // this
                                                                                                    //
                                                                                                    // is
                                                                                                    //
                                                                                                    // a
                                                                                                    //
                                                                                                    // trick!
        break;
      default:
        break;
      }
    }
  qvtkWidget->GetRenderWindow()->Render();
  statusBar()->showMessage(tr("Interpolation Changed"), 2000);
}

void GMainWindow::BackGroundColor(QAction *color)
{
  if( color->text() == "Background White" )
    {
    this->pvtkRenderer->SetBackground(1, 1, 1);
    }
  else if( color->text() == "Background Black" )
    {
    this->pvtkRenderer->SetBackground(0, 0, 0);
    }
  else if( color->text() == "Stereo Rendering" )
    {
    this->pvtkRenderer->GetRenderWindow()->SetStereoRender(
      !this->pvtkRenderer->GetRenderWindow()->GetStereoRender() );
    }
  this->pvtkRenderer->GetRenderWindow()->Render();
}

void GMainWindow::popup(vtkObject *obj,
                        unsigned long,
                        void * /* client_data */,
                        void *,
                        vtkCommand * /* command */)
{
  // A note about context menus in Qt and the QVTKWidget
  // You may find it easy to just do context menus on right button up,
  // due to the event proxy mechanism in place.

  // That usually works, except in some cases.
  // One case is where you capture context menu events that
  // child windows don't process.  You could end up with a second
  // context menu after the first one.

  // See QVTKWidget::ContextMenuEvent enum which was added after the
  // writing of this example.
  std::cout << "popup(vtkObject * obj" << std::endl;

  vtkImagePlaneWidget *planeWidget
    = reinterpret_cast<vtkImagePlaneWidget *>( obj );

  short Index = planeWidget->GetSliceIndex();
  std::cout << "SliceIndex" << Index << std::endl;
  // emit SliceIndex(Index);

  double wl[2];
  planeWidget->GetWindowLevel(wl);
  const int windowLocal = (int)wl[0];
  const int level = (int)wl[1];
  std::cout << "window" << windowLocal << "Level: " << level << std::endl;
  // emit WindowLevel(window,level);

  /*
    // get interactor
    vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(obj);
    // consume event so the interactor style doesn't get it
    //command->AbortFlagOn();
    // get popup menu
    QMenu* popupMenu = static_cast<QMenu*>(client_data);
    // get event location
    int* sz = iren->GetSize();
    int* position = iren->GetEventPosition();
    // remember to flip y
    QPoint pt = QPoint(position[0], sz[1]-position[1]);
    // map to global
    QPoint global_pt = popupMenu->parentWidget()->mapToGlobal(pt);
    // show popup menu at global point
    popupMenu->popup(global_pt);*/
}

void GMainWindow::SetAllWindowLevel(double windowLocal, double level)
{
  if( bWindowLevelSyn )
    {
    planeWidgetZ->SetWindowLevel(  windowLocal,  level );
    planeWidgetX->SetWindowLevel(  windowLocal,  level );
    planeWidgetY->SetWindowLevel(  windowLocal,  level );

    imageView2DPanelWithControls1->GetImageViewer2()->SetColorWindow(windowLocal);
    imageView2DPanelWithControls1->GetImageViewer2()->SetColorLevel(level);

    imageView2DPanelWithControls2->GetImageViewer2()->SetColorWindow(windowLocal);
    imageView2DPanelWithControls2->GetImageViewer2()->SetColorLevel(level);

    imageView2DPanelWithControls3->GetImageViewer2()->SetColorWindow(windowLocal);
    imageView2DPanelWithControls3->GetImageViewer2()->SetColorLevel(level);

    imageView2DPanelWithControls1->GetImageViewer2()->Render();
    imageView2DPanelWithControls2->GetImageViewer2()->Render();
    imageView2DPanelWithControls3->GetImageViewer2()->Render();
    }
}

void GMainWindow::WindowLevelChanged(vtkObject * /* obj */,
                                     unsigned long,
                                     void *client_data,
                                     void *,
                                     vtkCommand * /* command */)
{
  int *whichwindow;

  whichwindow = (int *)client_data;

  // std::cout<<"whichwindow"<<*whichwindow<<std::endl;
  switch( *whichwindow )
    {
    case 0:
      if( bWindowLevelSyn )
        {
        SetAllWindowLevel(
          imageView2DPanelWithControls1->GetImageViewer2()->GetColorWindow(),
          imageView2DPanelWithControls1->GetImageViewer2()->GetColorLevel() );
        }
      else
        {
        planeWidgetZ->SetWindowLevel(
          imageView2DPanelWithControls1->GetImageViewer2()->GetColorWindow(),
          imageView2DPanelWithControls1->GetImageViewer2()->GetColorLevel() );
        }
      break;
    case 1:
      if( bWindowLevelSyn )
        {
        SetAllWindowLevel(
          imageView2DPanelWithControls2->GetImageViewer2()->GetColorWindow(),
          imageView2DPanelWithControls2->GetImageViewer2()->GetColorLevel() );
        }
      else
        {
        planeWidgetX->SetWindowLevel(
          imageView2DPanelWithControls2->GetImageViewer2()->GetColorWindow(),
          imageView2DPanelWithControls2->GetImageViewer2()->GetColorLevel() );
        }
      break;
    case 2:
      if( bWindowLevelSyn )
        {
        SetAllWindowLevel(
          imageView2DPanelWithControls3->GetImageViewer2()->GetColorWindow(),
          imageView2DPanelWithControls3->GetImageViewer2()->GetColorLevel() );
        }
      else
        {
        planeWidgetY->SetWindowLevel(
          imageView2DPanelWithControls3->GetImageViewer2()->GetColorWindow(),
          imageView2DPanelWithControls3->GetImageViewer2()->GetColorLevel() );
        }
      break;
    default:
      break;
    }

  // std::cout<<"orient"<<std::endl;
  // std::cout<<"orient"<<*whichwindow<<"  "<<*whichwindow<<"
  //  "<<*whichwindow<<std::endl;
  // std::cout<<"window"<<ImageViewer2->GetColorWindow()<<"Level:
  // "<<ImageViewer2->GetColorLevel()<<std::endl;
}
