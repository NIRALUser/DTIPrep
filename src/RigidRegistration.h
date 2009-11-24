#pragma once

#include "itkImage.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkImageRegistrationMethod.h"
#include "itkMattesMutualInformationImageToImageMetric.h"

#include "itkLinearInterpolateImageFunction.h"
#include "itkVersorRigid3DTransform.h"
#include "itkCenteredTransformInitializer.h"
#include "itkVersorRigid3DTransformOptimizer.h"

typedef struct  InterlaceResults {
  bool bRegister;
  double AngleX;      // in degrees
  double AngleY;      // in degrees
  double AngleZ;      // in degrees
  double TranslationX;
  double TranslationY;
  double TranslationZ;
  double MutualInformation;      // -Metrix
  double Correlation;            // graylevel correlation
  } struInterlaceResults,  *pstruInterlaceResults;

#include "itkCommand.h"
class CommandIterationUpdate3DRigid : public itk::Command
  {
public:
  typedef  CommandIterationUpdate3DRigid Self;
  typedef  itk::Command                  Superclass;
  typedef itk::SmartPointer<Self>        Pointer;
  itkNewMacro( Self );
protected:
  CommandIterationUpdate3DRigid() {}
public:
  typedef itk::VersorRigid3DTransformOptimizer OptimizerType;
  typedef   const OptimizerType                *OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
    Execute( (const itk::Object *)caller, event );
  }

  void Execute(const itk::Object *object, const itk::EventObject & event)
  {
    OptimizerPointer optimizer
      = dynamic_cast<OptimizerPointer>( object );

    if ( !itk::IterationEvent().CheckEvent( &event ) )
      {
      return;
      }
    std::cout << optimizer->GetCurrentIteration() << "   ";
    std::cout << optimizer->GetValue() << "   ";
    std::cout << optimizer->GetCurrentPosition() << std::endl;
  }
  };

class CRigidRegistration
  {
public:
  typedef  unsigned short          PixelType;
  typedef itk::Image<PixelType, 3> ImageType;

  CRigidRegistration(ImageType::Pointer fixed,
    ImageType::Pointer moving,
    unsigned int BinsNumber,
    double SamplesPercent,
    bool ExplicitPDFDerivatives );
  CRigidRegistration(ImageType::Pointer fixed, ImageType::Pointer moving);
  ~CRigidRegistration(void);
public:

  typedef itk::VersorRigid3DTransform<double>
  TransformType;
  typedef itk::VersorRigid3DTransformOptimizer
  OptimizerType;
  typedef itk::MattesMutualInformationImageToImageMetric<ImageType,
    ImageType> MetricType;
  typedef itk::LinearInterpolateImageFunction<ImageType,
    double>    InterpolatorType;
  typedef itk::ImageRegistrationMethod<ImageType,
    ImageType> RegistrationType;

  typedef ImageType::SpacingType
  SpacingType;
  typedef ImageType::PointType
  OriginType;
  typedef ImageType::RegionType
  RegionType;
  typedef ImageType::SizeType
  SizeType;
private:
  MetricType::Pointer           metric;
  OptimizerType::Pointer        optimizer;
  InterpolatorType::Pointer     interpolator;
  RegistrationType::Pointer     registration;
  OptimizerType::ParametersType finalParameters;
  TransformType::Pointer        transform;

  ImageType::Pointer fixedImage;
  ImageType::Pointer movingImage;

  unsigned int numberOfBins;
  double       percentOfSamples; // 1% ~ 20%
  bool         useExplicitPDFDerivatives;
private:
  void SetupFramework();

  void SetupParameters();

public:
  struInterlaceResults Run(void);

  OptimizerType::ParametersType GetFinalParameters();
  };
