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

#include "ThreadIntensityMotionCheck.h"
#include "FurtherQCThread.h"

class IntensityMotionCheckPanel : public QDockWidget,
  private Ui_IntensityMotionCheckPanel
{
  Q_OBJECT
public:
  IntensityMotionCheckPanel(QMainWindow *parent = 0);
  ~IntensityMotionCheckPanel(void);

  CThreadIntensityMotionCheck myIntensityThread; // Object of ThreadIntensityMotionCheck class
  CFurtherQCThread            myFurtherQCThread; // object of FurtherQCThread class

  void SetFileName(QString nrrd );

  void SetName(QString nrrd_path );

signals:
  void status(const QString &);

  // void loadProtocol();
  void ProtocolChanged();

  void currentGradient(int winID, int gradient );

  void currentGradientChanged_VC( int gradient );

  void UpdateOutputDWIDiffusionVectorActors();

  void LoadQCResult(bool);

  void SignalLoadDwiFile();

  void SignalActivateSphere();

  void SignalLoadQCedDWI( QString qcdwiname);

  void Set_init_Path_Signal();

private slots:
  // void on_comboBox_Protocol_currentIndexChanged(QString protocolName);
  void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

  void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);

  void on_treeWidget_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous);

  void on_treeWidget_DiffusionInformation_itemClicked( QTreeWidgetItem *item, int column);

  void on_treeWidget_DiffusionInformation_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

  void on_treeWidget_Results_itemDoubleClicked(QTreeWidgetItem *item, int column);

  void on_treeWidget_Results_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous);

  void on_treeWidget_Results_itemChanged(QTreeWidgetItem *item, int column);

  // void on_treeWidget_Results_itemClicked( QTreeWidgetItem *item,  int column);

  void on_pushButton_Save_clicked();

  void on_pushButton_DefaultProtocol_clicked();

  void on_pushButton_SaveProtocolAs_clicked();

  void on_toolButton_ProtocolFileOpen_clicked();

  void on_toolButton_ResultFileOpen_clicked();

  void on_pushButton_RunPipeline_clicked();

  void on_pushButton_Pathdefault_clicked();

  void protocolLoaded_SetPath();

  // void on_pushButton_SaveDWIAs_clicked( );

  //void on_pushButton_DefaultQCResult_clicked();

  void on_pushButton_FSL_clicked();

  void on_pushButton_Slicer_clicked();

  void on_pushButton_dtiestim_clicked();

  void on_pushButton_dtiprocess_clicked();

  void on_pushButton_convertitk_clicked();

  void on_pushButton_imagemath_clicked();

  void ResultUpdate();

  void QCedResultUpdate();

  void SavingTreeWidgetResult_XmlFile();

  void SavingTreeWidgetResult_XmlFile_Default();

  void on_pushButton_SaveVisualChecking_clicked();

public slots:

  void StartProgressSlot();

  void StopProgressSlot();

  void f_StartProgressSlot();

  void f_StopProgressSlot();

  void SetVisualCheckingStatus( int index, int status);

  bool OpenMappingXML();

  // bool OpenQCedDWI();
  void Building_Mapping_XML();

  void LoadQCedDWI( QString qcdwiname);

  void Set_VCStatus();

  void Set_Original_ForcedConformance_Mapping();

  void Set_QCedDWI();

  void Set_init_Path();

public:

  bool GetSliceProtocolParameters(double beginSkip, double endSkip, double & baselineCorrelationThreshold,
                                  double & gradientCorrelationThreshold, double & baselineCorrelationDeviationThreshold,
                                  double & gradientCorrelationDeviationThreshold);

  bool GetInterlaceProtocolParameters(double & correlationThresholdBaseline, double & correlationThresholdGradient,
                                      double & correlationBaselineDevTimes,
                                      double & correlationGradientDevTimes);

  typedef unsigned short
  DwiPixelType;
  typedef itk::Image<DwiPixelType, 2>
  SliceImageType;
  typedef itk::Image<DwiPixelType, 3>
  GradientImageType;
  typedef itk::VectorImage<DwiPixelType, 3>
  DwiImageType;
  typedef itk::ImageFileReader<DwiImageType>
  DwiReaderType;
  typedef itk::ImageFileWriter<DwiImageType>
  DwiWriterType;

  typedef itk::DiffusionTensor3DReconstructionImageFilter<DwiPixelType,
                                                          DwiPixelType, double> TensorReconstructionImageFilterType;
  typedef  TensorReconstructionImageFilterType::GradientDirectionContainerType
  GradientDirectionContainerType;

  // void SetFileName(std::string filename) {DwiFileName = filename; };
  bool GetGradientDirections( bool bDisplay);

  bool GetGradientDirections();

  bool LoadDwiImage();

  QCResult & GetQCResult()
  {
    return qcResult;
  }

  Protocol & GetProtocol()
  {
    return protocol;
  }

  void SetProcessingQCResult(int & pro, int _status)
  {
    pro = _status;
  }

  QTreeWidget * & GetQTreeWidgetResult()
  {
    return treeWidget_Results;
  }

  QTreeWidget * & GetTreeWidgetProtocol()
  {
    return treeWidget;
  }

  void SaveVisualCheckingResult();

  void UpdatePanelDWI();

  void UpdateProtocolToTreeWidget();

  void SetDWIImage(DwiImageType::Pointer DWIImage)
  {
    m_DwiOriginalImage = DWIImage; bDwiLoaded = true;
  }

  void SetDwiOutputImage(DwiImageType::Pointer DWIImage)
  {
    m_DwiOutputImage = DWIImage;
  }

  DwiImageType::Pointer GetDwiOutputImage()
  {
    return m_DwiOutputImage;
  }

  void GenerateCheckOutputImage( const std::string filename);

  void GenerateCheckOutputImage( DwiImageType::Pointer dwi, const std::string filename);

  void GenerateOutput_VisualCheckingResult( std::string filename );

  void GenerateOutput_VisualCheckingResult2( std::string filename );

  void GenerateOutput_VisualCheckingResult();

  bool Search_index( int index, std::vector<int> list_index );

  void DefaultProcess();

  void OpenQCReport();

  void OpenXML();

  void OpenXML_ResultFile();

  void DefaultProtocol();

  void SetProtocolTreeEditable( bool editable )
  {
    bProtocolTreeEditable = editable;
  }

  void f_overallSliceWiseCheck();

  void f_overallInterlaceWiseCheck();

  void f_overallGradientWiseCheck();

  void Match_NameDwiQC();

  void Match_DwiQC();

  void Clear_VC_Status();

  struct VC_STATUS
    {
    int index;
    int VC_status;

    };
  std::vector<VC_STATUS> VC_Status;

  struct m_Original_ForcedConformance_Mapping
    {
    std::vector<int> index_original;
    int index_ForcedConformance;

    };

  std::vector<m_Original_ForcedConformance_Mapping> t_Original_ForcedConformance_Mapping;
  void set_Original_ForcedConformance_Mapping( std::vector<m_Original_ForcedConformance_Mapping> m_t )
  {
    t_Original_ForcedConformance_Mapping = m_t;
  }

private:
  void SetBrainMaskProtocol() ;
  void SetBaselineAverageMethod() ;
  void SetBaselineAverageInterpolationMethod() ;


  bool     bProtocol;
  Protocol protocol;
  QCResult qcResult;

  std::string           DwiFileName;
  QString               DwiName;     // Dwi file name only with no path string
  QString               DwiFilePath; // Dwi file name with full path
  DwiImageType::Pointer m_DwiOriginalImage;
  DwiImageType::Pointer m_DwiOutputImage; // QCed Dwi image
  std::string           QCedDwiFileName;  // ????

  bool bDwiLoaded;
  bool bDwi_VisualCheckLoad;                                    // set true when the new updated dwi after visual
                                                                // checking is created sucessfully
  bool                                    bGetGradientDirections;
  bool                                    readb0;
  double                                  b0;
  GradientDirectionContainerType::Pointer GradientDirectionContainer;
  GradientDirectionContainerType::Pointer GradientDirectionContainer_ConformanceImg;

  bool bResultTreeEditable;
  bool bProtocolTreeEditable;

  bool bMatchNameQCResult_DwiFile; // This variable is checked when the name of Dwi file and QCResult informatiom are
                                   // the same

  bool bLoadDefaultQC;  // Set by default false ans set true when the "Default Result" pushed
  bool bCancel_QC;
  bool bMatch_DwiQC;

  int r_SliceWiseCkeck;     // the overall SliceWiseChecking result
  int r_InterlaceWiseCheck; // the overall InterlaceWiseChecking result
  int r_GradWiseCheck;      // the overall GradientWiseChecking result

  unsigned char result; // the result of RunPipleline

  std::vector<int> index_listVCExcluded;      // contains the VC-excluded gradients id
  std::vector<int> index_listVCIncluded;      // contains the VC-included gradients id

  DwiReaderType::Pointer QCedDwiReader;

};

#endif // INTENSITYMOTIONCHECKPANEL_H
