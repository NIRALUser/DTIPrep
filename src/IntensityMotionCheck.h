#include "itkImage.h"
#include "itkVectorImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiffusionTensor3DReconstructionImageFilter.h"

#include "Protocol.h"
#include "QCResult.h"

#include <iostream>
#include <string>

#include "itkNrrdImageIO.h"

#include "itkDWICropper.h"
#include "itkDWIQCSliceChecker.h"
#include "itkDWIQCInterlaceChecker.h"
#include "itkDWIBaselineAverager.h"
#include "itkDWIQCGradientChecker.h"

#include "itkDWIEddyCurrentHeadMotionCorrector.h" // eddy-motion Utah
#include "itkVectorImageRegisterAffineFilter.h"   // eddy-motion IOWA

class CIntensityMotionCheck // : public QObject
  {
  // Q_OBJECT
public:
  CIntensityMotionCheck(void);
  ~CIntensityMotionCheck(void);

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

  //eddy-motion Utah
  typedef itk::DWIEddyCurrentHeadMotionCorrector<DwiImageType> EddyMotionCorrectorType;
  //eddy-motion Iowa
  typedef itk::VectorImageRegisterAffineFilter<DwiImageType, DwiImageType>
    EddyMotionCorrectorTypeIowa;

  void GetImagesInformation();

  unsigned int GetGradientsNumber()
  {
    return numGradients;
  }

  bool GetGridentDirections();

  bool GetGridentDirections( DwiImageType::Pointer dwi,
    double & bValue,
    GradientDirectionContainerType::Pointer GradDireContainer);

  bool ImageCheck( DwiImageType::Pointer dwi );
  bool DiffusionCheck( DwiImageType::Pointer dwi );
  bool SliceWiseCheck( DwiImageType::Pointer dwi );
  bool InterlaceWiseCheck( DwiImageType::Pointer dwi );
  bool BaselineAverage( DwiImageType::Pointer dwi );
  bool EddyMotionCorrect( DwiImageType::Pointer dwi );
  bool EddyMotionCorrectIowa( DwiImageType::Pointer dwi );
  bool GradientWiseCheck( DwiImageType::Pointer dwi );
  bool SaveQCedDWI(DwiImageType::Pointer dwi);



  bool DTIComputing();

  bool dtiprocess();
  bool dtiestim();

  bool validateDiffusionStatistics();
  unsigned char  validateLeftDiffusionStatistics();  // 00000CBA:


  void GenerateCheckOutputImage( std::string filename);

  void SetProtocol(Protocol *p)
  {
    this->protocol = p;
  }

  void SetQCResult(QCResult *r)
  {
    qcResult = r;
  }

  QCResult * GetQCResult()
  {
    return qcResult;
  }


  int getBaselineNumber()
  {
    return baselineNumber;
  }

  int getBValueNumber()
  {
    return bValueNumber;
  }

  int getGradientDirNumber()
  {
    return gradientDirNumber;
  }

  int getRepetitionNumber()
  {
    return repetitionNumber;
  }

  int getGradientNumber()
  {
    return gradientNumber;
  }

  int getBaselineLeftNumber() const
  {
    return baselineLeftNumber;
  }

  int getBValueLeftNumber() const
  {
    return bValueLeftNumber;
  }

  int getGradientDirLeftNumber() const
  {
    return gradientDirLeftNumber;
  }

  int getGradientLeftNumber() const
  {
    return gradientLeftNumber;
  }

  std::vector<int> getRepetitionLeftNumber() const
  {
    return repetitionLeftNumber;
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

  inline DwiImageType::Pointer GetDwiImage() const
  {
    return m_DwiOriginalImage;
  }

  inline DwiImageType::Pointer Getm_DwiForcedConformanceImage() const
  {
    return m_DwiForcedConformanceImage;
  }

  inline GradientDirectionContainerType::Pointer GetGradientDirectionContainer() const
  {
    return GradientDirectionContainer;
  }

  inline bool GetDwiLoadStatus() const
  {
    return bDwiLoaded;
  }

  inline std::string GetDwiFileName() const
  {
    return m_DwiFileName;
  }
  inline void SetDwiFileName(const std::string NewDwiFileName)
  {
    this->m_DwiFileName=NewDwiFileName;
  }

  bool LoadDwiImage();

private:
  void collectDiffusionStatistics();
  void collectLeftDiffusionStatistics( DwiImageType::Pointer dwi, std::string reportfilename );
  vnl_matrix_fixed<double, 3, 3> GetMeasurementFrame(
    DwiImageType::Pointer DwiImageExtractMF);

  //Code that crops the dwi images
  void ForceCroppingOfImage(const bool bReport, const std::string ImageCheckReportFileName);
  //All these variables need to have m_ in front of them.
  bool bDwiLoaded;

  int baselineNumber;
  int bValueNumber;
  int gradientDirNumber;
  int repetitionNumber;
  int gradientNumber;

  int              baselineLeftNumber;
  int              bValueLeftNumber;
  int              gradientDirLeftNumber;
  int              gradientLeftNumber;
  std::vector<int> repetitionLeftNumber;

  bool bGetGridentDirections;

  std::string m_DwiFileName;
  std::string GlobalReportFileName;

  DwiImageType::Pointer m_DwiForcedConformanceImage;
  DwiImageType::Pointer  m_DwiOriginalImage;
  DwiReaderType::Pointer DwiReader;

  unsigned int numGradients;

  GradientDirectionContainerType::Pointer GradientDirectionContainer;

  // for all gradients  slice wise correlation
  std::vector<double> means;
  std::vector<double> deviations;

  // for all baseline slice wise correlation
  std::vector<double> baselineMeans;
  std::vector<double> baselineDeviations;

  // for interlace baseline correlation
  double interlaceBaselineMeans;
  double interlaceBaselineDeviations;

  // for interlace gradient correlation
  double interlaceGradientMeans;
  double interlaceGradientDeviations;

  Protocol *protocol;
  QCResult *qcResult;

  bool   readb0;
  double b0;
  };
