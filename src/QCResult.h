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
  };
