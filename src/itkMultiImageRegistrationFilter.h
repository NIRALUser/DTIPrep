#ifndef __itkMultiImageRegistrationFilter_h
#define __itkMultiImageRegistrationFilter_h

#include <limits>
#include <itkImage.h>
#include "MultiResolutionMultiImageRegistrationMethod.h"
#include "UnivariateEntropyMultiImageMetric.h"

#include "itkAffineTransform.h"
#include "itkBSplineDeformableTransform.h"
#include "itkBSplineDeformableTransformInitializer.h"

#include "itkGradientDescentOptimizer.h"
#include "GradientDescentLineSearchOptimizer.h"

// Interpolator headers
#include "itkLinearInterpolateImageFunction.h"
#include <itkBSplineInterpolateImageFunction.h>
#include <itkWindowedSincInterpolateImageFunction.h>
#include "itkRecursiveMultiResolutionPyramidImageFilter.h"
#include "GradientDescentLineSearchOptimizer.h"
#include <itkClampImageFilter.h>//We clamp values under 0 that could appear after bspline of windowedsinc interpolation

// header for creating mask
#include "itkImageMaskSpatialObject.h"
#include "itkLargestForegroundFilledMaskImageFilter.h"

#include "itkNaryFunctorImageFilter.h"

namespace itk
{

/**
 * \author Yongqiang Zhao
 * This filter implements entropy-based groupwise B-Spline registration method
 * to construct the synthesized average-shape image.
 *
 */

class MEAN
{
public:
  MEAN()
  {
  };
  ~MEAN()
  {
  };
  typedef float PixelType;
  bool operator!=( const MEAN & ) const
  {
    return false;
  }

  bool operator==( const MEAN & other ) const
  {
    return !(*this != other);
  }

  inline PixelType operator()( const std::vector<PixelType> pixelStack)
  {
    double sum = 0.0;

    for( unsigned int i = 0; i < pixelStack.size(); i++ )
      {
      sum += pixelStack[i];
      }
    return (PixelType)(sum / (double)pixelStack.size() );
  }

};

class RegistrationInterfaceCommand : public Command
{
  static const unsigned int Dimension = 3;
public:
  typedef  RegistrationInterfaceCommand Self;
  typedef  Command                      Superclass;
  typedef  SmartPointer<Self>           Pointer;
  itkNewMacro( Self );

  // Set-Get methods to change between the scales
  itkSetMacro(MultiScaleSamplePercentageIncrease, double);
  itkSetMacro(MultiScaleMaximumIterationIncrease, double);
  itkSetMacro(MultiScaleStepLengthIncrease, double);
public:
  typedef   Image<float, Dimension>                              ImageType;
  typedef MultiResolutionMultiImageRegistrationMethod<ImageType> RegistrationType;
  typedef   RegistrationType *                                   RegistrationPointer;

  typedef   SingleValuedNonLinearOptimizer     OptimizerType;
  typedef   OptimizerType *                    OptimizerPointer;
  typedef   GradientDescentOptimizer           GradientOptimizerType;
  typedef   GradientOptimizerType *            GradientOptimizerPointer;
  typedef   GradientDescentLineSearchOptimizer LineSearchOptimizerType;
  typedef   LineSearchOptimizerType *          LineSearchOptimizerPointer;

  typedef   MultiImageMetric<ImageType> MetricType;
  typedef   MetricType *                MetricPointer;

  void Execute(itk::Object * object, const itk::EventObject & event)
  {
    if( !(itk::IterationEvent().CheckEvent( &event ) ) )
      {
      return;
      }
    RegistrationPointer registration =  dynamic_cast<RegistrationPointer>( object );
    OptimizerPointer    optimizer = dynamic_cast<OptimizerPointer>(
        registration->GetOptimizer() );
    MetricPointer metric = dynamic_cast<MetricPointer>
      (registration->GetMetric() );

    // Output message about registration
    std::cout << "message: metric type: " << metric->GetNameOfClass() << std::endl;
    std::cout << "message: Transform Type: " << registration->GetTransformArray(0)->GetNameOfClass();
    if( !strcmp(registration->GetTransformArray(0)->GetNameOfClass(), "BSplineDeformableTransform") )
      {
      std::cout << " (" << bsplineGridSize << "x" << bsplineGridSize << "x" << bsplineGridSize << ")";
      }
    std::cout << std::endl;
    std::cout << "message: Image Resolution: "
              << registration->GetImagePyramidArray(0)->
    GetOutput(registration->GetCurrentLevel() )->GetLargestPossibleRegion().GetSize() << std::endl;
    // registration->GetImagePyramidArray(0)->GetOutput(registration->GetCurrentLevel())->Print(std::cout,6);
    std::cout << "message: Number of Images: " << metric->GetNumberOfImages() << std::endl;
    std::cout << "message: Number of total parameters: " << registration->GetTransformParametersLength() << std::endl;
    std::cout << "message: Optimizertype: " << optimizer->GetNameOfClass() << std::endl;

    if( registration->GetCurrentLevel() == 0 )
      {
      // Set the number of spatial samples according to the current level
      metric->SetNumberOfSpatialSamples(
        (unsigned int) (metric->GetNumberOfSpatialSamples()
                        / vcl_pow( vcl_pow(2.0F, (float)Dimension ) / m_MultiScaleSamplePercentageIncrease,
                                   (double) (registration->GetNumberOfLevels() - 1.0) ) ) );

      if( !strcmp(optimizer->GetNameOfClass(), "GradientDescentLineSearchOptimizer") )
        {
        LineSearchOptimizerPointer lineSearchOptimizerPointer =
          dynamic_cast<LineSearchOptimizerPointer>(
            registration->GetOptimizer() );
        lineSearchOptimizerPointer->SetMaximumIteration(
          (int)(lineSearchOptimizerPointer->GetMaximumIteration()
                * vcl_pow(m_MultiScaleMaximumIterationIncrease, (double) (registration->GetNumberOfLevels() - 1.0) ) ) );
        lineSearchOptimizerPointer->SetStepLength(lineSearchOptimizerPointer->GetStepLength()
                                                  * vcl_pow(m_MultiScaleStepLengthIncrease,
                                                            (double) (registration->GetNumberOfLevels() - 1.0) ) );
        // print messages
        std::cout << "message: Optimizer #of Iter. to go : "
                  << lineSearchOptimizerPointer->GetMaximumIteration() << std::endl;
        std::cout << "message: Optimizer learning rate : "
                  << lineSearchOptimizerPointer->GetStepLength() << std::endl;
        }
      }
    else
      {
      // Set the number of spatial samples according to the current level
      metric->SetNumberOfSpatialSamples(
        (unsigned int) (metric->GetNumberOfSpatialSamples()
                        * vcl_pow(2.0F, (float)Dimension ) / m_MultiScaleSamplePercentageIncrease ) );

      // Decrease the learning rate at each increasing multiresolution level
      // Increase the number of steps
      if( !strcmp(optimizer->GetNameOfClass(), "GradientDescentLineSearchOptimizer") )
        {
        LineSearchOptimizerPointer lineSearchOptimizerPointer =
          dynamic_cast<LineSearchOptimizerPointer>(registration->GetOptimizer() );
        lineSearchOptimizerPointer->SetMaximumIteration(
          (int)(lineSearchOptimizerPointer->GetMaximumIteration() / m_MultiScaleMaximumIterationIncrease ) );
        lineSearchOptimizerPointer->SetStepLength(
          lineSearchOptimizerPointer->GetStepLength() / m_MultiScaleStepLengthIncrease );

        std::cout << "message: Optimizer #of Iter. to go : "
                  << lineSearchOptimizerPointer->GetMaximumIteration() << std::endl;
        std::cout << "message: Optimizer learning rate : "
                  << lineSearchOptimizerPointer->GetStepLength() << std::endl;
        }
      }
  }

  void Execute(const itk::Object *, const itk::EventObject & )
  {
    return;
  }

  int bsplineGridSize;

  // Constructor initialize the variables
  RegistrationInterfaceCommand()
  {
    m_MultiScaleSamplePercentageIncrease = 1;
    m_MultiScaleMaximumIterationIncrease = 1;
    m_MultiScaleStepLengthIncrease = 1;
  };
private:
  double m_MultiScaleSamplePercentageIncrease;
  double m_MultiScaleMaximumIterationIncrease;
  double m_MultiScaleStepLengthIncrease;

  std::vector<itk::Transform<double, Dimension, Dimension> *> transformArray;
};

class MultiImageRegistrationFilter :
  public Object
{
private:
  static const unsigned int Dimension = 3;
  static const unsigned int SplineOrder = 3;
public:
  typedef float                               InternalPixelType;
  typedef double                              ScalarType;
  typedef Image<InternalPixelType, Dimension> ImageType;
  typedef ImageType::Pointer                  ImagePointer;

  typedef AffineTransform<ScalarType, Dimension> AffineTransformType;

  // Optimizer types
  typedef GradientDescentOptimizer           OptimizerType;
  typedef GradientDescentLineSearchOptimizer LineSearchOptimizerType;

  // Interpolator typedef
  typedef InterpolateImageFunction<ImageType,ScalarType        >           InterpolatorType;
  typedef LinearInterpolateImageFunction<ImageType, ScalarType>            LinearInterpolatorType;
  typedef BSplineInterpolateImageFunction<ImageType, ScalarType>           BSplineInterpolatorType;//we will use order 3
  typedef WindowedSincInterpolateImageFunction< ImageType , 4 >            WindowedSincInterpolatorType ;//default is Hamming
  typedef UnivariateEntropyMultiImageMetric<ImageType>                     EntropyMetricType;
  typedef OptimizerType::ScalesType                                        OptimizerScalesType;
  typedef MultiResolutionMultiImageRegistrationMethod<ImageType>           RegistrationType;
  typedef RecursiveMultiResolutionPyramidImageFilter<ImageType, ImageType> ImagePyramidType;

  // Mask related typedefs
  typedef Image<unsigned char, Dimension>                                  ImageMaskType;
  typedef ImageMaskSpatialObject<Dimension>                                ImageMaskSpatialObjectType;
  typedef LargestForegroundFilledMaskImageFilter<ImageType, ImageMaskType> LFFMaskFilterType;

  typedef AffineTransformType::InputPointType      CenterPointType;
  typedef CenterPointType::ValueType               CoordRepType;
  typedef ContinuousIndex<CoordRepType, Dimension> ContinuousIndexType;
  typedef ContinuousIndexType::ValueType           ContinuousIndexValueType;
  typedef BSplineDeformableTransform<ScalarType, Dimension, SplineOrder>
  BSplineTransformType;
  typedef BSplineDeformableTransformInitializer<BSplineTransformType, ImageType> BSplineInitializerType;

  typedef RegistrationType::ParametersType                     ParametersType;
  typedef ResampleImageFilter<ImageType, ImageType>            ResampleFilterType;
  typedef NaryFunctorImageFilter<ImageType,  ImageType,  MEAN> NaryMEANImageFilter;
  typedef RegistrationInterfaceCommand                         CommandType;

  typedef MultiImageRegistrationFilter Self;
  typedef Object                       Superclass;
  typedef SmartPointer<Self>           Pointer;
  typedef SmartPointer<const Self>     ConstPointer;

  itkNewMacro(Self);

  itkTypeMacro(MultiImageRegistrationFilter, Object);

  itkGetObjectMacro( Output, ImageType );
  itkGetMacro(MultiLevelAffine, int);
  itkSetMacro(MultiLevelAffine, int);

  itkGetMacro(InterpolationMethod, int);
  itkSetMacro(InterpolationMethod, int);

  itkGetMacro(MultiLevelBspline, int);
  itkSetMacro(MultiLevelBspline, int);

  itkGetMacro(OptAffineLearningRate, double);
  itkSetMacro(OptAffineLearningRate, double);

  itkGetMacro(OptBsplineLearningRate, double);
  itkSetMacro(OptBsplineLearningRate, double);

  itkGetMacro(OptAffineNumberOfIterations, int);
  itkSetMacro(OptAffineNumberOfIterations, int);

  itkGetMacro(OptBsplineNumberOfIterations, int);
  itkSetMacro(OptBsplineNumberOfIterations, int);

  itkGetMacro(NumberOfSpatialSamplesAffinePercentage, double);
  itkSetMacro(NumberOfSpatialSamplesAffinePercentage, double);

  itkGetMacro(NumberOfSpatialSamplesBsplinePercentage, double);
  itkSetMacro(NumberOfSpatialSamplesBsplinePercentage, double);

  itkSetMacro(BsplineInitialGridSize, int);
  itkGetMacro(BsplineInitialGridSize, int);
  itkSetMacro(NumberOfBsplineLevel, int);
  itkGetMacro(NumberOfBsplineLevel, int);

  itkGetMacro(ParzenWindowStandardDeviation, double);
  itkGetMacro(GaussianFilterKernelWidth, double);
  itkSetMacro(GaussianFilterKernelWidth, double);

  void SetImages(std::vector<ImagePointer>& imageArray)
  {
    if( imageArray.size() < 2 )
      {
      std::cout << "Not enough input images!" << std::endl;
      return;
      }
    m_InputImageArray = imageArray;
  }

  void Update();

protected:
  MultiImageRegistrationFilter();
  virtual  ~MultiImageRegistrationFilter();
  void Initialize();

  void AffineRegistration();

  void BsplineRegistraiton();

private:
  MultiImageRegistrationFilter(const Self &);
  void operator=(const Self &);

  std::vector<ImagePointer>                    m_InputImageArray;
  LineSearchOptimizerType::Pointer             m_Optimizer;
  RegistrationType::Pointer                    m_Registration;
  EntropyMetricType::Pointer                   m_Metric;
  unsigned int                                 m_NumberOfImages;
  ImagePointer                                 m_Output;
  std::vector<ImagePyramidType::Pointer>       m_ImagePyramidArray;
  std::vector<AffineTransformType::Pointer>    m_AffineTransformArray;
  std::vector<BSplineTransformType::Pointer>   m_BsplineTransformArray;
  std::vector<LinearInterpolatorType::Pointer>       m_InterpolatorArray;
  std::vector<BSplineInitializerType::Pointer> m_BsplineInitializerArray;
  int                                          m_MultiLevelAffine;
  int                                          m_MultiLevelBspline;
  int                                          m_InterpolationMethod;
  double m_OptAffineLearningRate;
  double m_OptBsplineLearningRate;

  int m_OptAffineNumberOfIterations;
  int m_OptBsplineNumberOfIterations;

  double m_NumberOfSpatialSamplesAffinePercentage;
  double m_NumberOfSpatialSamplesBsplinePercentage;

  unsigned int m_NumberOfSpatialSamplesAffine;
  unsigned int m_NumberOfSpatialSamplesBspline;

  double m_AffineMultiScaleSamplePercentageIncrease;
  double m_BsplineMultiScaleSamplePercentageIncrease;

  double m_AffineMultiScaleMaximumIterationIncrease;
  double m_BsplineMultiScaleMaximumIterationIncrease;

  double m_AffineMultiScaleStepLengthIncrease;
  double m_BsplineMultiScaleStepLengthIncrease;

  double m_TranslationScaleCoeffs;
  int    m_MaximumLineIteration;

  int m_BsplineInitialGridSize;
  int m_NumberOfBsplineLevel;

  double m_BsplineRegularizationFactor;

  double               m_ParzenWindowStandardDeviation;
  double               m_GaussianFilterKernelWidth;
  CommandType::Pointer m_Command;
};

} // end namespace itk

#endif
