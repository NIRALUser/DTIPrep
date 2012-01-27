#ifndef IntensityMotionCheck_H
#define IntensityMotionCheck_H


#include "itkImage.h"
#include "itkVectorImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiffusionTensor3DReconstructionImageFilter.h"

#include "Protocol.h"
#include "QCResult.h"

//#include "SignalClass.h"

#include <iostream>
#include <string>

#include "itkNrrdImageIO.h"

#include "itkDWICropper.h"
#include "itkDWIQCSliceChecker.h"
#include "itkDWIQCInterlaceChecker.h"
#include "itkDWIBaselineAverager.h"
#include "itkDWIQCGradientChecker.h"
#include <itksys/Process.h>
#include <itksys/Glob.hxx>


#include "itkDWIEddyCurrentHeadMotionCorrector.h" // eddy-motion Utah
#include "itkVectorImageRegisterAffineFilter.h"   // eddy-motion IOWA

#include <QObject>
#include <QProcess>



class CIntensityMotionCheck //: public QObject
{
 
//Q_OBJECT
  	
public:
  CIntensityMotionCheck();

  ~CIntensityMotionCheck();

  struct DiffusionDir {
    std::vector<double> gradientDir;
    int repetitionNumber;
  };

  typedef unsigned short                     DwiPixelType;
  typedef itk::Image<DwiPixelType, 2>        SliceImageType;
  typedef itk::Image<DwiPixelType, 3>        GradientImageType;
  typedef itk::VectorImage<DwiPixelType, 3>  DwiImageType;
  typedef itk::ImageFileReader<DwiImageType> DwiReaderType;
  typedef itk::ImageFileWriter<DwiImageType> DwiWriterType;

  typedef itk::DiffusionTensor3DReconstructionImageFilter<DwiPixelType,
    DwiPixelType, double> TensorReconstructionImageFilterType;
  typedef  TensorReconstructionImageFilterType::GradientDirectionContainerType
    GradientDirectionContainerType;

  typedef itk::DWICropper<DwiImageType>            CropperType;
  typedef itk::DWIQCSliceChecker<DwiImageType>     SliceCheckerType;
  typedef itk::DWIQCInterlaceChecker<DwiImageType> InterlaceCheckerType;
  typedef itk::DWIBaselineAverager<DwiImageType>   BaselineAveragerType;
  typedef itk::DWIQCGradientChecker<DwiImageType>  GradientCheckerType;
  
  itksysProcess* m_Process;

  //eddy-motion Utah
  typedef itk::DWIEddyCurrentHeadMotionCorrector<DwiImageType> EddyMotionCorrectorType;
  //eddy-motion Iowa
  typedef itk::VectorImageRegisterAffineFilter<DwiImageType, DwiImageType>
    EddyMotionCorrectorTypeIowa;

  void GetImagesInformation();

  inline unsigned int GetGradientsNumber()
  {
    return m_numGradients;
  }

  bool GetGradientDirections();
 
  bool GetGradientDirections_FurtherQC();

  bool GetGradientDirections( 
    DwiImageType::Pointer dwi,
    double & bValue,
    GradientDirectionContainerType::Pointer GradDireContainer);

  unsigned char ImageCheck( DwiImageType::Pointer dwi ); 
  // 0000 0000: ok; 
  // 0000 0001: size mismatch; 
  // 0000 0010: spacing mismatch; 
  // 0000 0100: origins mismatch
  // 0000 0100: Space directions mismatch
  // 0000 0100: Space directions mismatch

  bool DiffusionCheck( DwiImageType::Pointer dwi );
  bool SliceWiseCheck( DwiImageType::Pointer dwi );
  int Denoising( DwiImageType::Pointer dwi );
  int JointDenoising( DwiImageType::Pointer dwi );
  bool InterlaceWiseCheck( DwiImageType::Pointer dwi );
  bool BaselineAverage( DwiImageType::Pointer dwi );
  bool EddyMotionCorrect( DwiImageType::Pointer dwi );
  bool EddyMotionCorrectIowa( DwiImageType::Pointer dwi );
  bool GradientWiseCheck( DwiImageType::Pointer dwi );
  bool SaveDwiForcedConformanceImage(void);
  bool SaveDwiForcedConformanceImage_FurtherQC(void) const;

  bool DTIComputing();

  bool dtiprocess();
  bool dtiestim();

  bool validateDiffusionStatistics();
  unsigned char  validateLeftDiffusionStatistics();  // 00000CBA:

  inline void SetProtocol(Protocol *p)
  {
    this->protocol = p;
  }

  inline void SetProtocol_FurtherQC(Protocol *p)
  {
    this->protocol = p;
    protocol_load = true;
  }
  
  inline void SetQCResult(QCResult *r)
  {
    qcResult = r;
  }

  
  inline QCResult * GetQCResult()
  {
    return qcResult;
  }

  inline int getBaselineNumber()
  {
    return m_baselineNumber;
  }

  inline int getBValueNumber()
  {
    return m_bValueNumber;
  }

  inline int getGradientDirNumber()
  {
    return m_gradientDirNumber;
  }

  inline int getRepetitionNumber()
  {
    return m_repetitionNumber;
  }

  inline int getGradientNumber()
  {
    return m_gradientNumber;
  }

  inline int getBaselineLeftNumber() const
  {
    return m_baselineLeftNumber;
  }

  inline int getBValueLeftNumber() const
  {
    return m_bValueLeftNumber;
  }

  inline int getGradientDirLeftNumber() const
  {
    return m_gradientDirLeftNumber;
  }

  inline int getGradientLeftNumber() const
  {
    return m_gradientLeftNumber;
  }

  inline std::vector<int> getRepetitionLeftNumber() const
  {
    return m_repetitionLeftNumber;
  }

  
  // A: Gradient direction # is less than 6!
  // B: Single b-value DWI without a b0/baseline!
  // C: Too many bad gradient directions found!
  // 0: valid
  // ZYXEDCBA:
  // X QC; Too many bad gradient directions found!
  // Y QC; Single b-value DWI without a b0/baseline!
  // Z QC: Gradient direction # is less than 6!
  // A:ImageCheck()
  // B:DiffusionCheckInternalDwiImage()
  // C: IntraCheck()
  // D:InterlaceCheck()
  // E: InterCheck()
  unsigned char  RunPipelineByProtocol();

  unsigned char  RunPipelineByProtocol_FurtherQC();	// In Further QC step: Runing Baseline Avg, Eddy motion correction, Gradient-wise checking, DWI saving


  inline DwiImageType::Pointer GetDwiImage() const
  {
    return m_DwiOriginalImage;
  }

  inline DwiImageType::Pointer Getm_DwiForcedConformanceImage() const
  {
    return m_DwiForcedConformanceImage;
  }

  inline void Setm_DwiForcedConformanceImage( const DwiImageType::Pointer dwi)
  {
    this->m_DwiForcedConformanceImage = dwi;
  } 

  inline GradientDirectionContainerType::Pointer GetGradientDirectionContainer() const
  {
    return m_GradientDirectionContainer;
  }

  inline bool GetDwiLoadStatus() const
  {
    return m_bDwiLoaded;
  }

  inline std::string GetDwiFileName() const
  {
    return m_DwiFileName;
  }

  inline std::string GetXmlFileName() const
  {
    return m_XmlFileName;
  }

  inline void SetDwiFileName(const std::string NewDwiFileName)
  {
    this->m_DwiFileName=NewDwiFileName;
  }

  inline void SetXmlFileName(const std::string NewXmlFileName)
  {
    this->m_XmlFileName=NewXmlFileName;
  }

  bool LoadDwiImage();
 
  bool LoadDwiImage_FurtherQC( DwiImageType::Pointer  new_dwi );

  bool MakeDefaultProtocol( Protocol *protocol);

  //CSignalClass * Signal;
//signals:
  //void kkk( int );


  struct Original_ForcedConformance_Mapping
  {
     std::vector<int> index_original;
     int index_ForcedConformance;
     
  };
  
  std::vector<Original_ForcedConformance_Mapping> m_Original_ForcedConformance_Mapping;

  inline std::vector<Original_ForcedConformance_Mapping> get_Original_ForcedConformance_Mapping()
  {
    return m_Original_ForcedConformance_Mapping;
  }

  std::string & GetOutputDWIFileName()
  {
    return m_outputDWIFileName;
  }

  void SetOutputDWIFileName( std::string outputDWIFileName )
  {
    m_outputDWIFileName = outputDWIFileName;
  }

  std::string m_outputDWIFileName;	// the name of QCed DWI file
  
private:
  void collectDiffusionStatistics();
  void collectLeftDiffusionStatistics( DwiImageType::Pointer dwi, std::string reportfilename );
  vnl_matrix_fixed<double, 3, 3> GetMeasurementFrame(
    DwiImageType::Pointer DwiImageExtractMF);

  //Code that crops the dwi images
  void ForceCroppingOfImage(const bool bReport, const std::string ImageCheckReportFileName);

  //HACK:  TODO:  Zhexing  private member variables should start with m_ so that it is easy to
  //distinguish them from local variables in the member functions.
  //All these variables need to have m_ in front of them.
  bool m_bDwiLoaded;
  bool m_bDwiLoaded_FurtherQC;

  int m_baselineNumber;
  int m_bValueNumber;
  int m_gradientDirNumber;
  int m_repetitionNumber;
  int m_gradientNumber;

  int m_baselineLeftNumber;
  int m_bValueLeftNumber;
  int m_gradientDirLeftNumber;
  int m_gradientLeftNumber;
  std::vector<int> m_repetitionLeftNumber;

  bool m_bGetGradientDirections;
  bool m_bGetGradientDirections_FurtherQC;

  std::string m_DwiFileName;
  std::string m_XmlFileName;
  //std::string m_outputDWIFileName;	// the name of QCed DWI file
  std::string m_GlobalReportFileName;

  DwiImageType::Pointer m_DwiForcedConformanceImage;
  DwiImageType::Pointer  m_DwiOriginalImage;

  unsigned int m_numGradients;

  GradientDirectionContainerType::Pointer m_GradientDirectionContainer;

  // for all gradients  slice wise correlation
  std::vector<double> m_means;
  std::vector<double> m_deviations;

  // for all baseline slice wise correlation
  std::vector<double> m_baselineMeans;
  std::vector<double> m_baselineDeviations;

  // for interlace baseline correlation
  double m_interlaceBaselineMeans;
  double m_interlaceBaselineDeviations;

  // for interlace gradient correlation
  double m_interlaceGradientMeans;
  double m_interlaceGradientDeviations;

  Protocol *protocol;
  QCResult *qcResult;
  

  bool   m_readb0;
  double m_b0;

  bool   m_readb0_FurtherQC;
  double m_b0_FurtherQC;

  bool protocol_load;

  // Log File
  std::string m_output;
  char m_LogFile[512];

};

#endif

