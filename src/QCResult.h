#pragma once

#include <vector>
#include <QString>
struct ImageInformationCheckResult {
  QString info;
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
  int VisualChecking;
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
  int InterlaceWiseCheckProcessing;  // the result of the InterlaceWiseCheck processing
};

struct  GradientWiseCheckResult {
  int GradientWiseCheckProcessing; // the result of the GradientWiseCheck processing
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

struct OverallQCResult{
  bool SWCk; //SliceWiseCheck 
  bool IWCk; //InterlaceWiseCheck
  bool GWCk; //GradientWiseCheck

};

struct Original_ForcedConformance_Map	
  {
     std::vector<int> index_original;
     int index_ForcedConformance;
     
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

    imageInformationCheckResult.origin = true;
    imageInformationCheckResult.size = true;
    imageInformationCheckResult.space = true;
    imageInformationCheckResult.spacedirection = true;
    imageInformationCheckResult.spacing = true;

    diffusionInformationCheckResult.b = true;
    diffusionInformationCheckResult.gradient = true;
    diffusionInformationCheckResult.measurementFrame = true;

    result = 0;
  }

  void Set_result( unsigned char r )
  {
    result = r;
  }

  unsigned char & Get_result()
  {
    return result;
  }

private:
  ImageInformationCheckResult imageInformationCheckResult;

  DiffusionInformationCheckResult  diffusionInformationCheckResult;

  OverallQCResult  overallQCResult;

  std::vector<GradientIntensityMotionCheckResult> intensityMotionCheckResult;

  std::vector<InterlaceWiseCheckResult> interlaceWiseCheckResult;

  std::vector<GradientWiseCheckResult> gradientWiseCheckResult;
  
  std::vector<SliceWiseCheckResult> sliceWiseCheckResult;

  std::vector< int > sliceWiseCheckProcessing;  // the result of the SliceWiseCheck processing

  std::vector<Original_ForcedConformance_Map> m_Original_ForcedConformance_Map; // showing gradients indices included in the conformance image along their correspondings in the original image. 

  unsigned char result;

  

};
  
