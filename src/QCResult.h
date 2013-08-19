#pragma once

#include <vector>
#include <QString>
struct ImageInformationCheckResult
  {
  QString info;
  bool space;
  bool size;
  bool origin;
  bool spacing;
  bool spacedirection;
  };

struct DiffusionInformationCheckResult
  {
  bool b;
  bool gradient;
  bool measurementFrame;
  };

struct GradientIntensityMotionCheckResult
  {
  int processing;
  double OriginalDir[3];
  double ReplacedDir[3];
  double CorrectedDir[3];
  int VisualChecking;
  int QCIndex;    // mapped to the index of QCed gradeint
  };

struct  InterlaceWiseCheckResult
  {
  double AngleX;      // in degrees
  double AngleY;      // in degrees
  double AngleZ;      // in degrees
  double TranslationX;
  double TranslationY;
  double TranslationZ;
  double Metric;                // MutualInformation;
  double Correlation;           // graylevel correlation
  // int InterlaceWiseCheckProcessing;  // the result of the InterlaceWiseCheck processing
  };

struct  GradientWiseCheckResult
  {
  // int GradientWiseCheckProcessing; // the result of the GradientWiseCheck processing
  double AngleX;      // in degrees
  double AngleY;      // in degrees
  double AngleZ;      // in degrees
  double TranslationX;
  double TranslationY;
  double TranslationZ;
  double MutualInformation;      // -Metrix
  };

struct SliceWiseCheckResult
  {
  int GradientNum;
  int SliceNum;
  double Correlation;
  };

struct OverallQCResult
  {
  bool SWCk;  // SliceWiseCheck
  bool IWCk;  // InterlaceWiseCheck
  bool GWCk;  // GradientWiseCheck
  bool BMCK;  // BrainMaskCheck
  bool DDDCK; // DominantDirectionalDetectionCheck

  };

struct Original_ForcedConformance_Map
  {
  std::vector<int> index_original;
  int index_ForcedConformance;

  };

struct DominantDirection_Detector
  {
  double z_score;
  double entropy_value;
  int detection_result;   // 0:acceptance 1: Unacceptance 2: Suspicious
  };

class QCResult
{
public:
  QCResult(void);
  ~QCResult(void);

  enum
    {
    GRADIENT_INCLUDE = 0,
    GRADIENT_BASELINE_AVERAGED,
    GRADIENT_EDDY_MOTION_CORRECTED,
    GRADIENT_EXCLUDE_SLICECHECK,
    GRADIENT_EXCLUDE_INTERLACECHECK,
    GRADIENT_EXCLUDE_GRADIENTCHECK,
    GRADIENT_EXCLUDE_MANUALLY,
    };

  struct ImageInformationCheckResult & GetImageInformationCheckResult()
  {
    return imageInformationCheckResult;
  }

  struct DominantDirection_Detector & GetDominantDirection_Detector()
  {
    return dominantDirection_Detector;
  }

  struct OverallQCResult & GetOverallQCResult()
  {
    return overallQCResult;
  }

  int & getProcessing(int index)
  {
    return GetIntensityMotionCheckResult()[index].processing;
  }

  struct DiffusionInformationCheckResult & GetDiffusionInformationCheckResult()
  {
    return diffusionInformationCheckResult;
  }

  std::vector<GradientIntensityMotionCheckResult>   & GetIntensityMotionCheckResult()
  {
    return intensityMotionCheckResult;
  }

  std::vector<InterlaceWiseCheckResult> & GetInterlaceWiseCheckResult()
  {
    return interlaceWiseCheckResult;
  }

  std::vector<GradientWiseCheckResult> & GetGradientWiseCheckResult()
  {
    return gradientWiseCheckResult;
  }

  std::vector<SliceWiseCheckResult> & GetSliceWiseCheckResult()
  {
    return sliceWiseCheckResult;
  }

  std::vector<int> & GetSliceWiseCheckProcessing()
  {
    return sliceWiseCheckProcessing;
  }

  std::vector<Original_ForcedConformance_Map> & GetOriginal_ForcedConformance_Map()
  {
    return m_Original_ForcedConformance_Map;
  }

  void Clear()
  {
    intensityMotionCheckResult.clear();
    interlaceWiseCheckResult.clear();
    gradientWiseCheckResult.clear();
    sliceWiseCheckResult.clear();
    sliceWiseCheckProcessing.clear();
    m_Original_ForcedConformance_Map.clear();

    imageInformationCheckResult.origin = true;
    imageInformationCheckResult.size = true;
    imageInformationCheckResult.space = true;
    imageInformationCheckResult.spacedirection = true;
    imageInformationCheckResult.spacing = true;

    dominantDirection_Detector.z_score = 0;
    dominantDirection_Detector.entropy_value = 0;

    diffusionInformationCheckResult.b = true;
    diffusionInformationCheckResult.gradient = true;
    diffusionInformationCheckResult.measurementFrame = true;

    result = 0;

  }

  void Set_result( unsigned r )
  {
    result = r;
  }

  unsigned & Get_result()
  {
    return result;
  }

  void ClearResult()
    {
      this->result = 0;
    }
  void SetImageCheckError()
    {
      this->result |= 1;
    }
  bool GetImageCheckError()
    {
      return (this->result & 1) != 0;
    }
  void SetDiffusionCheckError()
    {
      this->result |= 2;
    }
  bool GetDiffusionCheckError()
    {
      return (this->result & 2) != 0;
    }
  void SetSliceWiseCheckError()
    {
      this->result |= 4;
    }
  bool GetSliceWiseCheckError()
    {
      return (this->result & 4) != 0;
    }
  void SetInterlaceWiseCheckError()
    {
      this->result |= 8;
    }
  bool GetInterlaceWiseCheckError()
    {
      return (this->result & 8) != 0;
    }
  void SetGradientWiseCheckError()
    {
      this->result |= 16;
    }
  bool GetGradientWiseCheckError()
    {
      return (this->result & 16) != 0;
    }
  void SetBrainMaskCheckError()
    {
      this->result |= 32;
    }
  bool GetBrainMaskCheckError()
    {
      return (this->result & 32) != 0;
    }
  void SetDominantDirectionalCheckError()
    {
      this->result |= 64;
    }
  bool GetDominantDirectionalCheckError()
    {
      return (this->result & 64) != 0;
    }
  void SetGradientLeftCheckError()
    {
      this->result |= 128;
    }
  bool GetGradientLeftCheckError()
    {
      return (this->result & 128) != 0;
    }
  void SetBaselineLeftCheckError()
    {
      this->result |= 256;
    }
  bool GetBaselineLeftCheckError()
    {
      return (this->result & 256) != 0;
    }

  void SetBadGradientCheckError()
    {
      this->result |= 512;
    }
  bool GetBadGradientCheckError()
    {
      return (this->result & 512) != 0;
    }

private:
  ImageInformationCheckResult imageInformationCheckResult;

  DominantDirection_Detector dominantDirection_Detector;

  DiffusionInformationCheckResult diffusionInformationCheckResult;

  OverallQCResult overallQCResult;

  std::vector<GradientIntensityMotionCheckResult> intensityMotionCheckResult;

  std::vector<InterlaceWiseCheckResult> interlaceWiseCheckResult;

  std::vector<GradientWiseCheckResult> gradientWiseCheckResult;

  std::vector<SliceWiseCheckResult> sliceWiseCheckResult;

  std::vector<int> sliceWiseCheckProcessing;    // the result of the SliceWiseCheck processing

  std::vector<Original_ForcedConformance_Map> m_Original_ForcedConformance_Map; // showing gradients indices included in
                                                                                // the conformance image along their
                                                                                // correspondings in the original image.

  unsigned result;

};
