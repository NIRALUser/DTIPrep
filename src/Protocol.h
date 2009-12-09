#pragma once

#include <string>
#include <vector>
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"

struct ImageProtocol {
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

  bool bQuitOnCheckFailure;
  };

struct DiffusionProtocol {
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
  };

struct SliceCheckProtocol {
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

  bool bQuitOnCheckFailure;
  };

struct InterlaceCheckProtocol {
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

  bool bQuitOnCheckFailure;
  };

struct GradientCheckProtocol {
  bool bCheck;

  double translationThreshold;
  double rotationThreshold;

  std::string outputDWIFileNameSuffix;
  std::string reportFileNameSuffix;
  int reportFileMode; // 0: new   1: append

  bool bQuitOnCheckFailure;
  };

struct BaselineAverageProtocol {
  bool bAverage;

  int averageMethod;
  double stopThreshold;

  std::string outputDWIFileNameSuffix;
  std::string reportFileNameSuffix;
  int reportFileMode; // 0: new   1: append
  };

struct EddyMotionCorrectionProtocol {
  bool bCorrect;

  int numberOfBins;
  int numberOfSamples;
  double translationScale;
  double stepLength;
  double relaxFactor;
  int maxNumberOfIterations;

  std::string outputDWIFileNameSuffix;
  std::string reportFileNameSuffix;
  int reportFileMode; // 0: new   1: append
  };

struct DTIProtocol {
  bool bCompute;
  std::string dtiestimCommand;
  std::string dtiprocessCommand;

  int method;
  int baselineThreshold;
  std::string mask;
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

  enum {
    REPORT_TYPE_SIMPLE = 0,
    REPORT_TYPE_VERBOSE,
    REPORT_TYPE_EASY_PARSE,
    };

  enum {
    TYPE_SHORT = 0,
    TYPE_USHORT,
    TYPE_UNKNOWN,
    };

  enum {
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

  struct DiffusionDir {
    std::vector<double> gradientDir;
    int repetitionNumber;
    };

  void initProtocols();

  void initImageProtocol();

  void initDiffusionProtocol();

  void initSliceCheckProtocol();

  void initInterlaceCheckProtocol();

  void initGradientCheckProtocol();

  void initBaselineAverageProtocol();

  void initEddyMotionCorrectionProtocol();

  void initDTIProtocol();

  //HACK: print functions should be const
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

  //HACK:  get functions should be const, and should start with capital G
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

  //Get functions should be const, returning a reference to the internal
  // class variable breaks encapsulation.
  // PREFER:
  // "const struct ImageProtocol & GetImageProtocol() const"
  // OR
  // "struct ImageProtocol GetImageProtocol() const"
  struct ImageProtocol & GetImageProtocol()
  {
    return imageProtocol;
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

  struct DTIProtocol & GetDTIProtocol()
  {
    return dTIProtocol;
  }

  std::string & GetQCOutputDirectory()
  {
    return QCOutputDirectory;
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

  //TODO:  A family of these GetXXXXXReportFileName() functions needs to be written
  // HACK:  These functions should all be const, which means that the functions they depend on
  // also need to be const
  // HACK:  Body of these functions should be moved to the .cpp file
  // HACK:  Common code accross all functions should be refactored into private function
  // to remove as much duplicate code as possible.
  //std::string GetDiffusionProtocolReportFileName(const std::string referenceDwiFileName) const
  std::string GetDiffusionProtocolReportFileName(const std::string referenceDwiFileName)
    {
    std::string ReportFileName;
    if ( this->GetImageProtocol().reportFileNameSuffix.length() > 0 )
      {
      if ( this->GetQCOutputDirectory().length() > 0 )
        {
        if ( this->GetQCOutputDirectory().at( this->GetQCOutputDirectory()
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

  //HACK:  All private member variables should start with "m_" to indicate
  //       that they are private, and to avoid namespace polution/collision with the class itself
  std::string QCOutputDirectory;
  std::string QCedDWIFileNameSuffix;
  std::string reportFileNameSuffix;
  double m_BadGradientPercentageTolerance;
  int m_ReportType; // 0:simple, 1:verbose, 2: iowa


  int baselineNumber;
  int bValueNumber;
  int gradientDirNumber;
  int repetitionNumber;

  ImageProtocol                imageProtocol;
  DiffusionProtocol            diffusionProtocol;
  SliceCheckProtocol           sliceCheckProtocol;
  InterlaceCheckProtocol       interlaceCheckProtocol;
  BaselineAverageProtocol      baselineAverageProtocol;
  EddyMotionCorrectionProtocol eddyMotionCorrectionProtocol;
  GradientCheckProtocol        gradientCheckProtocol;
  DTIProtocol                  dTIProtocol;
  };
