#ifndef INTENSITYMOTIONCHECKPANEL_H
#define INTENSITYMOTIONCHECKPANEL_H

#include <QtGui/QDockWidget>
#include <QtGui/QMainWindow>
#include "ui_IntensityMotionCheckPanel.h"

#include "itkImage.h"
#include "itkVectorImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiffusionTensor3DReconstructionImageFilter.h"

#include "Protocol.h"
#include "QCResult.h"

class CThreadIntensityMotionCheck;

class IntensityMotionCheckPanel : public QDockWidget,
  private Ui_IntensityMotionCheckPanel
  {
  Q_OBJECT
public:
  IntensityMotionCheckPanel(QMainWindow *parent = 0);
  ~IntensityMotionCheckPanel(void);

  void SetFileName(QString nrrd );

signals:
  void status(const QString &);

  // void loadProtocol();
  void ProtocolChanged();

  void currentGradient(int winID, int gradient );

  void UpdateOutputDWIDiffusionVectorActors();

private slots:
  // void on_comboBox_Protocol_currentIndexChanged(QString protocolName);
  void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

  void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);

  void on_treeWidget_currentItemChanged( QTreeWidgetItem *current,
    QTreeWidgetItem *previous);

  void on_treeWidget_DiffusionInformation_itemClicked( QTreeWidgetItem *item,
    int column);

  void on_treeWidget_DiffusionInformation_currentItemChanged(
    QTreeWidgetItem *current,
    QTreeWidgetItem *previous);

  void on_treeWidget_Results_itemDoubleClicked(QTreeWidgetItem *item,
    int column);

  void on_treeWidget_Results_currentItemChanged( QTreeWidgetItem *current,
    QTreeWidgetItem *previous);

  void on_treeWidget_Results_itemChanged(QTreeWidgetItem *item, int column);

  void on_treeWidget_Results_itemClicked( QTreeWidgetItem *item,  int column);

  void on_pushButton_Save_clicked( );

  void on_pushButton_DefaultProtocol_clicked( );

  void on_pushButton_SaveProtocolAs_clicked( );

  void on_toolButton_ProtocolFileOpen_clicked( );

  void on_pushButton_RunPipeline_clicked( );

  void on_pushButton_SaveDWIAs_clicked( );

  void on_pushButton_DefaultQCResult_clicked( );

  void ResultUpdate();

  void UpdateProgressBar(const int pos);

private:
  CThreadIntensityMotionCheck *ThreadIntensityMotionCheck;
public:

  bool GetSliceProtocolParameters(
    double beginSkip,
    double endSkip,
    double & baselineCorrelationThreshold,
    double & gradientCorrelationThreshold,
    double & baselineCorrelationDeviationThreshold,
    double & gradientCorrelationDeviationThreshold
    );

  bool GetInterlaceProtocolParameters(
    double & correlationThresholdBaseline,
    double & correlationThresholdGradient,
    double & correlationBaselineDevTimes,
    double & correlationGradientDevTimes
    );

  CThreadIntensityMotionCheck  * GetThreadIntensityMotionCheck()
  {
    return ThreadIntensityMotionCheck;
  }

  typedef unsigned short
  DwiPixelType;
  typedef itk::Image<DwiPixelType,
    2>                    SliceImageType;
  typedef itk::Image<DwiPixelType,
    3>                    GradientImageType;
  typedef itk::VectorImage<DwiPixelType,
    3>                    DwiImageType;
  typedef itk::ImageFileReader<DwiImageType>
  DwiReaderType;
  typedef itk::ImageFileWriter<DwiImageType>
  DwiWriterType;

  typedef itk::DiffusionTensor3DReconstructionImageFilter<DwiPixelType,
    DwiPixelType, double> TensorReconstructionImageFilterType;
  typedef  TensorReconstructionImageFilterType::GradientDirectionContainerType
  GradientDirectionContainerType;

  // void SetFileName(std::string filename) {DwiFileName = filename; };
  bool GetGridentDirections( bool bDisplay);

  bool GetGridentDirections();

  bool LoadDwiImage();

  QCResult & GetQCResult()
  {
    return qcResult;
  }

  Protocol & GetProtocol()
  {
    return protocol;
  }

  void UpdatePanelDWI( );

  void UpdateProtocolToTreeWidget( );

  void SetDWIImage(DwiImageType::Pointer DWIImage)
  {
    DwiImage = DWIImage; bDwiLoaded = true;
  }

  void GenerateCheckOutputImage( std::string filename);

  void DefaultProcess( );

  void OpenQCReport( );

  void OpenXML();

  void DefaultProtocol();

  void SetProtocolTreeEditable( bool editable )
  {
    bProtocolTreeEditable = editable;
  }

private:

  bool     bProtocol;
  Protocol protocol;
  QCResult qcResult;

  std::string            DwiFileName;
  DwiReaderType::Pointer DwiReader;
  DwiImageType::Pointer  DwiImage;

  bool                                    bDwiLoaded;
  bool                                    bGetGridentDirections;
  bool                                    readb0;
  double                                  b0;
  GradientDirectionContainerType::Pointer GradientDirectionContainer;

  bool bResultTreeEditable;
  bool bProtocolTreeEditable;
  };

#endif // INTENSITYMOTIONCHECKPANEL_H
