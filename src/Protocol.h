#pragma once

#include <string>
#include <vector>
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"

struct ImageProtocol
  {
  bool bCheck;
  int type;
  int space;
  int dimension;
  vnl_vector_fixed<unsigned int, 4> size;
  vnl_vector_fixed<double, 3> origin;
  vnl_vector_fixed<double, 3> spacing;
  vnl_matrix_fixed<double, 3, 3> spacedirection;

  bool bCrop;
  std::string croppedDWIFileNameSuffix;

  std::string reportFileNameSuffix;
  int reportFileMode; // 0: new   1: append

  bool bQuitOnCheckSpacingFailure;
  bool bQuitOnCheckSizeFailure;
  };

struct DiffusionProtocol
  {
  bool bCheck;
  double bValue;
  // std::vector< std::vector<double> > gradients;
  std::vector<vnl_vector_fixed<double, 3> > gradients;
  vnl_matrix_fixed<double, 3, 3> measurementFrame;

  bool bUseDiffusionProtocol;

  std::string diffusionReplacedDWIFileNameSuffix;
  std::string reportFileNameSuffix;
  int reportFileMode; // 0: new   1: append

  bool bQuitOnCheckFailure;

  double bValueAcceptablePercentageTolerance_;
  float gradientToleranceForSameness_degree; //allow degree

  };

struct DenoisingLMMSE
  {

  bool bCheck;
  std::string LMMSECommand;
  std::string ParameterSet;
  int NumIter;
  vnl_vector_fixed<int, 3> Est_Radius;
  vnl_vector_fixed<int, 3> Filter_Radius;
  int Min_VoxelNum_Filter;
  int Min_VoxelNum_Est;
  int MinNoiseSTD;
  int MaxNoiseSTD;
  double HistogramResolution;
  bool AbsoluteValue;

  };

struct DenoisingJointLMMSE
  {

  bool bCheck;
  std::string JointLMMSECommand;
  std::string ParameterSet;
  int NumNeighborGradients;
  vnl_vector_fixed<int, 3> Est_Radius;
  vnl_vector_fixed<int, 3> Filter_Radius;
  };


struct SliceCheckProtocol
  {
  bool bCheck;

  int checkTimes;
  double headSkipSlicePercentage;
  double tailSkipSlicePercentage;
  double correlationDeviationThresholdbaseline;
  double correlationDeviationThresholdgradient;

  bool bSubregionalCheck;
  double subregionalCheckRelaxationFactor;

  std::string outputDWIFileNameSuffix;
  std::string reportFileNameSuffix;
  int reportFileMode; // 0: new   1: append

  std::string excludedDWINrrdFileNameSuffix;
  bool bQuitOnCheckFailure;
  };

struct InterlaceCheckProtocol
  {
  bool bCheck;

  double correlationThresholdBaseline;
  double correlationThresholdGradient;
  double correlationDeviationBaseline;
  double correlationDeviationGradient;
  double translationThreshold;
  double rotationThreshold;

  std::string outputDWIFileNameSuffix;
  std::string reportFileNameSuffix;
  int reportFileMode; // 0: new   1: append

  std::string excludedDWINrrdFileNameSuffix;
  bool bQuitOnCheckFailure;
  };

struct GradientCheckProtocol
  {
  bool bCheck;

  double translationThreshold;
  double rotationThreshold;

  std::string outputDWIFileNameSuffix;
  std::string reportFileNameSuffix;
  int reportFileMode; // 0: new   1: append

  std::string excludedDWINrrdFileNameSuffix;
  bool bQuitOnCheckFailure;
  };

struct BaselineAverageProtocol
  {
  bool bAverage;

  int averageMethod;
  double stopThreshold;
  double b0Threshold; // default is 1e-7 which is defined in itkBaselineAverager.h (change the value using DWIBaselineAverager::setB0Threshold(float))

  std::string outputDWIFileNameSuffix;
  std::string reportFileNameSuffix;
  int reportFileMode; // 0: new   1: append
  int interpolation; //0: linear , 1:bspline of order 3, 2: WindowedSinc (hamming)
  };

struct EddyMotionCorrectionProtocol
  {
  bool bCorrect;
  // int toolSource; // 0:Utah; 1:IOWA
  int numberOfIterations; // number of iterations
  int numberOfSamples;
  float translationScale;
  float maxStepLength;
  float minStepLength;
  float relaxFactor;

  std::string outputDWIFileNameSuffix;
  std::string finalTransformFileSuffix;
  std::string reportFileNameSuffix;
  int reportFileMode; // 0: new   1: append
  int interpolation; //0: linear , 1:bspline of order 3, 2: WindowedSinc (hamming)
  };

struct BrainMaskProtocol
  {
  bool bMask;
  int BrainMask_Method; // 0 : FSL_bet_using_B0 1:Slicer DiffusionWeightedVolumeMasking 2:user 3:FSL_bet_using_IDWI
  std::string BrainMask_SystemPath_FSL;
  std::string BrainMask_SystemPath_Slicer;
  std::string BrainMask_SystemPath_convertITK;
  std::string BrainMask_SystemPath_imagemath;
  std::string BrainMask_Image;
  std::string reportFileNameSuffix;
  int reportFileMode;
  bool bQuitOnCheckFailure;
  };

struct DominantDirectional_Detector
  {
  bool bCheck;
  double Mean;
  double Deviation;
  double Threshold_Acceptance;
  double Threshold_Suspicion_Unacceptance;
  std::string reportFileNameSuffix;
  int reportFileMode;
  bool bQuitOnCheckFailure;
  };

struct DTIProtocol
  {
  bool bCompute;
  std::string dtiestimCommand;
  std::string dtiprocessCommand;

  int method;
  int baselineThreshold;
  // std::string mask;
  std::string tensorSuffix;

  std::string idwiSuffix;
  std::string baselineSuffix;
  std::string faSuffix;
  std::string mdSuffix;
  std::string coloredfaSuffix;
  std::string frobeniusnormSuffix;

  bool bidwi;
  bool bbaseline;
  bool bfa;
  bool bmd;
  bool bcoloredfa;
  bool bfrobeniusnorm;

  std::string reportFileNameSuffix;
  int reportFileMode; // 0: new   1: append
  };

class Protocol
{
public:
  Protocol(void);
  ~Protocol(void);

  enum
    {
    REPORT_TYPE_SIMPLE = 0,
    REPORT_TYPE_VERBOSE,
    REPORT_TYPE_EASY_PARSE,
    };

  enum
    {
    TYPE_SHORT = 0,
    TYPE_USHORT,
    TYPE_UNKNOWN,
    };

  enum
    {
    SPACE_LAI = 0,
    SPACE_LAS,
    SPACE_LPI,
    SPACE_LPS,
    SPACE_RAI,
    SPACE_RAS,
    SPACE_RPI,
    SPACE_RPS,
    SPACE_UNKNOWN,
    };

  enum {  METHOD_WLS = 0,
          METHOD_LLS,
          METHOD_ML,
          METHOD_NLS,
          METHOD_UNKNOWN, };

  enum
    {
    BRAINMASK_METHOD_FSL = 0,
    BRAINMASK_METHOD_SLICER,
    BRAINMASK_METHOD_OPTION,
    BRAINMASK_METHOD_FSL_IDWI,
    };

  enum
  {
      LINEAR_INTERPOLATION = 0,
      BSPLINE_INTERPOLATION,
      WINDOWEDSINC_INTERPOLATION
  };

  struct DiffusionDir
    {
    std::vector<double> gradientDir;
    int repetitionNumber;
    };

  void initProtocols();

  void initImageProtocol();

  void initDiffusionProtocol();

  void initDenoisingLMMSE();

  void initSliceCheckProtocol();

  void initInterlaceCheckProtocol();

  void initGradientCheckProtocol();

  void initBaselineAverageProtocol();

  void initEddyMotionCorrectionProtocol();

  void initDenoisingJointLMMSE();

  void initDominantDirectional_Detector();

  void initDTIProtocol();

  void initBrainMaskProtocol();

  // HACK: print functions should be const
  void printProtocols();

  void printImageProtocol();

  void printDiffusionProtocol();

  void printSliceCheckProtocol();

  void printInterlaceCheckProtocol();

  void printGradientCheckProtocol();

  void printBaselineAverageProtocol();

  void printEddyMotionCorrectionProtocol();

  void printDTIProtocol();

  void clear();

  void collectDiffusionStatistics();

  // HACK:  get functions should be const, and should start with capital G
  int getBaselineNumber() const
  {
    return baselineNumber;
  }

  int getBValueNumber() const
  {
    return bValueNumber;
  }

  int getgradientDirNumber() const
  {
    return gradientDirNumber;
  }

  int getRepetitionNumber() const
  {
    return repetitionNumber;
  }

  // Get functions should be const, returning a reference to the internal
  // class variable breaks encapsulation.
  // PREFER:
  // "const struct ImageProtocol & GetImageProtocol() const"
  // OR
  // "struct ImageProtocol GetImageProtocol() const"
  struct ImageProtocol & GetImageProtocol()
  {
    return imageProtocol;
  }

  struct DenoisingLMMSE & GetDenoisingLMMSEProtocol()
  {
    return denoisingLMMSE;
  }

  struct DenoisingJointLMMSE & GetDenoisingJointLMMSE()
  {
    return denoisingJointLMMSE;
  }

  struct DominantDirectional_Detector & GetDominantDirectional_Detector()
  {
    return dominantDirectional_Detector;
  }

  struct DiffusionProtocol & GetDiffusionProtocol()
  {
    return diffusionProtocol;
  }

  struct SliceCheckProtocol & GetSliceCheckProtocol()
  {
    return sliceCheckProtocol;
  }

  struct InterlaceCheckProtocol & GetInterlaceCheckProtocol()
  {
    return interlaceCheckProtocol;
  }

  struct GradientCheckProtocol & GetGradientCheckProtocol()
  {
    return gradientCheckProtocol;
  }

  struct BaselineAverageProtocol & GetBaselineAverageProtocol()
  {
    return baselineAverageProtocol;
  }

  struct EddyMotionCorrectionProtocol & GetEddyMotionCorrectionProtocol()
  {
    return eddyMotionCorrectionProtocol;
  }

  struct BrainMaskProtocol & GetBrainMaskProtocol()
  {
    return brainMaskProtocol;
  }

  struct DTIProtocol & GetDTIProtocol()
  {
    return dTIProtocol;
  }

  std::string & GetQCOutputDirectory()
  {
    return QCOutputDirectory;
  }

  void SetQCOutputDirectory( std::string QCDirectory )
  {
    QCOutputDirectory = QCDirectory;
  }

  std::string GetQCedDWIFileNameSuffix() const
  {
    return QCedDWIFileNameSuffix;
  }

  std::string GetReportFileNameSuffix() const
  {
    return reportFileNameSuffix;
  }

  double GetBadGradientPercentageTolerance() const
  {
    return m_BadGradientPercentageTolerance;
  }

  void SetBadGradientPercentageTolerance(const double tor)
  {
    m_BadGradientPercentageTolerance = tor;
  }

  int GetReportType() const
  {
    return m_ReportType;
  }

  void SetReportType( const int type)
  {
    m_ReportType = type;
  }

  void Save( std::string xml);

  // TODO:  A family of these GetXXXXXReportFileName() functions needs to be written
  // HACK:  These functions should all be const, which means that the functions they depend on
  // also need to be const
  // HACK:  Body of these functions should be moved to the .cpp file
  // HACK:  Common code accross all functions should be refactored into private function
  // to remove as much duplicate code as possible.
  // std::string GetDiffusionProtocolReportFileName(const std::string referenceDwiFileName) const
  std::string GetDiffusionProtocolReportFileName(const std::string referenceDwiFileName)
  {
    std::string ReportFileName;

    if( this->GetImageProtocol().reportFileNameSuffix.length() > 0 )
      {
      if( this->GetQCOutputDirectory().length() > 0 )
        {
        if( this->GetQCOutputDirectory().at( this->GetQCOutputDirectory()
                                             .length() - 1 ) == '\\'
            || this->GetQCOutputDirectory().at( this->
                                                GetQCOutputDirectory().length() - 1 ) == '/'     )
          {
          ReportFileName = this->GetQCOutputDirectory().substr(
              0, this->GetQCOutputDirectory().find_last_of("/\\") );
          }
        else
          {
          ReportFileName = this->GetQCOutputDirectory();
          }

        ReportFileName.append( "/" );

        std::string str = referenceDwiFileName.substr( 0, referenceDwiFileName.find_last_of('.') );
        str = str.substr( str.find_last_of("/\\") + 1);

        ReportFileName.append( str );
        ReportFileName.append(
          this->GetDiffusionProtocol().reportFileNameSuffix );
        }
      else
        {
        ReportFileName = referenceDwiFileName.substr( 0, referenceDwiFileName.find_last_of('.') );
        ReportFileName.append(
          this->GetDiffusionProtocol().reportFileNameSuffix );
        }
      }
    return ReportFileName;
  }

private:

  // HACK:  All private member variables should start with "m_" to indicate
  //       that they are private, and to avoid namespace polution/collision with the class itself
  std::string QCOutputDirectory;
  std::string QCedDWIFileNameSuffix;
  std::string reportFileNameSuffix;
  double      m_BadGradientPercentageTolerance;
  int         m_ReportType; // -1:no; 0:simple, 1:verbose, 2: easy parse

  int baselineNumber;
  int bValueNumber;
  int gradientDirNumber;
  int repetitionNumber;

  ImageProtocol                imageProtocol;
  DiffusionProtocol            diffusionProtocol;
  DenoisingLMMSE               denoisingLMMSE;
  SliceCheckProtocol           sliceCheckProtocol;
  InterlaceCheckProtocol       interlaceCheckProtocol;
  BaselineAverageProtocol      baselineAverageProtocol;
  EddyMotionCorrectionProtocol eddyMotionCorrectionProtocol;
  GradientCheckProtocol        gradientCheckProtocol;
  DenoisingJointLMMSE          denoisingJointLMMSE;
  DominantDirectional_Detector dominantDirectional_Detector;
  DTIProtocol                  dTIProtocol;
  BrainMaskProtocol            brainMaskProtocol;

};
