#ifndef IMAGEVIEW2DPANELWITHCONTROLS_H
#define IMAGEVIEW2DPANELWITHCONTROLS_H

#include <QtGui/QDockWidget>
#include <QtGui/QMainWindow>
#include "ui_ImageView2DPanelWithControls.h"
#include <vtkImageViewer2.h>
#include <vtkImageActor.h>

#include <itkImage.h>
#include <itkImageFileReader.h>

#include "itkVTKImageExport.h"
#include "vtkImageImport.h"
#include "vtkImageData.h"
#include "itkImageToVTKImageFilter.h"
#include "vtkImagePlaneWidget.h"
#include "vtkCommand.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkImageFileReader.h"

class vtkCylinderSource;
class vtkPolyDataMapper;
class vtkExodusReader;
class vtkDataSetMapper;
class vtkActor;
class vtkRenderer;
class vtkImageViewer2;
class vtkImageData;

class vtkEventQtSlotConnect;

class ImageView2DPanelWithControls : public QDockWidget,
  private Ui_ImageView2DPanelWithControls
  {
  Q_OBJECT
public:
  ImageView2DPanelWithControls(QString title, QMainWindow *parent = 0);
  ~ImageView2DPanelWithControls(void);

  void Setup( vtkImageData *image,
    double ww,
    double wl,
    int SliceIndex,
    int orient,
    int numGradients,
    int id);

  void Update();

  inline vtkImageViewer2 * GetImageViewer2()
  {
    return ImageViewer2;
  }

  void toggle_toolButton_windowlevel_Syn(bool toggle)
  {
    toolButton_windowlevel_Syn->setChecked(toggle);
  }

  void toggle_toolButton_orientation_Syn(bool toggle)
  {
    toolButton_orientation_Syn->setChecked(toggle);
  }

  void toggle_toolButton_interpolation_Syn(bool toggle)
  {
    toolButton_interpolation_Syn->setChecked(toggle);
  }

  void toggle_toolButton_content_Syn(bool toggle)
  {
    toolButton_content_Syn->setChecked(toggle);
  }

public:
  enum {
    ORIENTATION_AXIAL    = 0,
    ORIENTATION_SAGITTAL,
    ORIENTATION_CORONAL,
    };

  typedef unsigned short                    DwiPixelType;
  typedef itk::Image<DwiPixelType, 2>       SliceImageType;
  typedef itk::Image<DwiPixelType, 3>       GradientImageType;
  typedef itk::VectorImage<DwiPixelType, 3> DWIImageType;
public:
  QComboBox * GetComboBox_Interpolation(void)
  {
    return comboBox_Interpolation;
  }

  QComboBox * GetComboBox_Orientation(void)
  {
    return comboBox_Orientation;
  }

  QComboBox * GetComboBox_Contents(void)
  {
    return comboBox_Contents;
  }

  QSlider * GetHorizontalSlider_Gradient()
  {
    return horizontalSlider_Gradient;
  }

  QLineEdit * GetLineEdit_Gradient()
  {  
    return lineEdit_Gradient;
  }

signals:
  void visible(int which, bool vis);

  void Opacity(int whick, double opacity);

  void indexchanged( int, int);

  void orientationchanged(int, int);

  void gradientchanged(int, int);

  void interpolation(int, int);

  void WindowLevel( double window, double level);

  void contentschanged(int window, int index);

  void SynWindowLevel(bool syn);

  void SynContent(bool syn);

  void SynOrientation(bool syn);

  void SynInterpolation(bool syn);

private slots:
  void on_horizontalSlider_SliceIndex_valueChanged(int index);

  void on_horizontalSlider_Gradient_valueChanged(int index);

  void on_toolButton_Play_clicked();

  void on_toolButton_GradientPlay_clicked();

  void on_comboBox_Orientation_currentIndexChanged(int indexOrient);

  void on_comboBox_Contents_currentIndexChanged(int index);

  void on_comboBox_Interpolation_currentIndexChanged(int index);

  void on_toolButton_Visible_toggled(bool vis);

  // void on_toolButton_Syncronize_toggled(bool syn);
  void on_doubleSpinBox_valueChanged(double value);

  // connect with ImagePlaneWidgets in MainWindow
  void SliceIndexChanged(vtkObject *obj,
    unsigned long,
    void *client_data,
    void *,
    vtkCommand *command);

  void WindowLevelChanged(vtkObject *obj,
    unsigned long,
    void *client_data,
    void *,
    vtkCommand *command);

  // syn
  void on_toolButton_windowlevel_Syn_toggled(bool syn);

  void on_toolButton_interpolation_Syn_toggled(bool syn);

  void on_toolButton_orientation_Syn_toggled(bool syn);

  void on_toolButton_content_Syn_toggled(bool syn);

private:
  vtkCylinderSource *source;
  vtkPolyDataMapper *mapper;
  vtkActor          *actor;
  vtkRenderer       *ren;

  vtkImageViewer2 *ImageViewer2;

  int Orientation;
  int WindowID;

  vtkImageData *vtkImage;
  //TODO:  m_ImageDimension and m_ImageSpacing should be itk::ImageBase::ImageSpacing types for spacing and dimension
  int          m_ImageDimension[3];
  double       m_ImageSpacing[3];
  int          NumberOfGradients;

  // vtkQtConnection
  vtkEventQtSlotConnect *vtkQtConnections;

  bool bSetup;
  };

#endif // IMAGEVIEW2DPANELWITHCONTROLS_H
