#pragma once

#include <vector>
struct ImageInformationCheckResult {
  bool space;
  bool size;
  bool origin;
  bool spacing;
  bool spacedirection;
  };

struct DiffusionInformationCheckResult {
  bool b;
  bool gradient;
  bool measurementFrame;
  };

struct GradientIntensityMotionCheckResult {
  int processing;
  double OriginalDir[3];
  double ReplacedDir[3];
  double CorrectedDir[3];
  };


struct  InterlaceWiseCheckResult{
   
  double AngleX;      // in degrees
  double AngleY;      // in degrees
  double AngleZ;      // in degrees
  double TranslationX;
  double TranslationY;
  double TranslationZ;
  double Metric;                // MutualInformation;
  double Correlation;           // graylevel correlation
};

struct  GradientWiseCheckResult {
  double AngleX;      // in degrees
  double AngleY;      // in degrees
  double AngleZ;      // in degrees
  double TranslationX;
  double TranslationY;
  double TranslationZ;
  double MutualInformation;      // -Metrix
};

struct SliceWiseCheckResult{
  int GradientNum;
  int SliceNum;
  double Correlation;
};


class QCResult
  {
public:
  QCResult(void);
  ~QCResult(void);

  enum {
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

  struct DiffusionInformationCheckResult & GetDiffusionInformationCheckResult()
  {
    return diffusionInformationCheckResult;
  }

  std::vector<GradientIntensityMotionCheckResult>   &
  GetIntensityMotionCheckResult()
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

  void Clear()
  {
    intensityMotionCheckResult.clear();

    imageInformationCheckResult.origin = true;
    imageInformationCheckResult.size = true;
    imageInformationCheckResult.space = true;
    imageInformationCheckResult.spacedirection = true;
    imageInformationCheckResult.spacing = true;

    diffusionInformationCheckResult.b = true;
    diffusionInformationCheckResult.gradient = true;
    diffusionInformationCheckResult.measurementFrame = true;
  }

private:
  ImageInformationCheckResult imageInformationCheckResult;
  DiffusionInformationCheckResult
                                                  diffusionInformationCheckResult;
  std::vector<GradientIntensityMotionCheckResult> intensityMotionCheckResult;

  std::vector<InterlaceWiseCheckResult> interlaceWiseCheckResult;

  std::vector<GradientWiseCheckResult> gradientWiseCheckResult;
  
  std::vector<SliceWiseCheckResult> sliceWiseCheckResult;

};
  
