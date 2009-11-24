#include "RigidRegistration.h"

CRigidRegistration::CRigidRegistration(ImageType::Pointer fixed,
  ImageType::Pointer moving,
  unsigned int BinsNumber,
  double SamplesPercent,
  bool ExplicitPDFDerivatives )
  {
  fixedImage = fixed;
  movingImage = moving;

  numberOfBins = BinsNumber;
  percentOfSamples = SamplesPercent; // 1% ~ 20%
  useExplicitPDFDerivatives = ExplicitPDFDerivatives;
  }

CRigidRegistration::CRigidRegistration(ImageType::Pointer fixed,
  ImageType::Pointer moving)
  {
  fixedImage = fixed;
  movingImage = moving;

  numberOfBins = 25;
  percentOfSamples = 0.1; // 1% ~ 20%
  useExplicitPDFDerivatives = 1;
  }

CRigidRegistration::~CRigidRegistration(void)
     {}

void CRigidRegistration::SetupFramework()
{
  metric        = MetricType::New();
  optimizer     = OptimizerType::New();
  interpolator  = InterpolatorType::New();
  registration  = RegistrationType::New();
  transform    = TransformType::New();

  // typedef itk::MeanSquaresImageToImageMetric< ImageType,ImageType >
  //    MetricType;
  // MetricType::Pointer  metric  = MetricType::New();

  registration->SetMetric(        metric        );
  registration->SetOptimizer(     optimizer     );
  registration->SetInterpolator(  interpolator  );
  registration->SetTransform(     transform    );

  registration->SetFixedImage(   fixedImage );
  registration->SetMovingImage(  movingImage);

  CommandIterationUpdate3DRigid::Pointer observer
    = CommandIterationUpdate3DRigid::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );
}

void CRigidRegistration::SetupParameters()
{
  registration->SetFixedImageRegion( fixedImage->GetBufferedRegion() );

  typedef itk::CenteredTransformInitializer<TransformType, ImageType,
    ImageType> TransformInitializerType;
  TransformInitializerType::Pointer initializer = TransformInitializerType::New();

  initializer->SetTransform(   transform );
  initializer->SetFixedImage(  fixedImage );
  initializer->SetMovingImage( movingImage);
  initializer->MomentsOn();
  // initializer->GeometryOn();
  initializer->InitializeTransform();

  typedef TransformType::VersorType VersorType;
  typedef VersorType::VectorType    VectorType;

  VersorType rotation;
  VectorType axis;

  axis[0] = 0.0;
  axis[1] = 0.0;
  axis[2] = 1.0;

  const double angle = 0;
  rotation.Set(  axis, angle  );
  transform->SetRotation( rotation );

  registration->SetInitialTransformParameters( transform->GetParameters() );

  typedef OptimizerType::ScalesType OptimizerScalesType;
  OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );
  const double        translationScale = 1.0 / 1000.0;

  optimizerScales[0] = 1.0;
  optimizerScales[1] = 1.0;
  optimizerScales[2] = 1.0;
  optimizerScales[3] = translationScale;
  optimizerScales[4] = translationScale;
  optimizerScales[5] = translationScale;

  optimizer->SetScales( optimizerScales );
  optimizer->SetMaximumStepLength( 0.2000  );
  optimizer->SetMinimumStepLength( 0.0001 );

  optimizer->SetNumberOfIterations( 1000 );

  // numberOfBins = 25;
  // percentOfSamples = 0.10; // 1% ~ 20%

  metric->SetNumberOfHistogramBins( numberOfBins );

  int SampleSize
    = (int)(fixedImage->GetPixelContainer()->Size() * percentOfSamples);
  if ( SampleSize > 100000 )
    {
    metric->SetNumberOfSpatialSamples( SampleSize );
    }
  else
    {
    metric->UseAllPixelsOn();
    }
  // metric->SetUseExplicitPDFDerivatives( useExplicitPDFDerivatives ); //true
  // for small # of parameters; false for big # of transform paramrters
}

struInterlaceResults CRigidRegistration::Run(void)
{
  struInterlaceResults results;

  results.bRegister = true;
  SetupFramework();
  SetupParameters();

  try
    {
    registration->StartRegistration();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    return results;
    }

  OptimizerType::ParametersType finalParameters
    = registration->GetLastTransformParameters();

  const double finalAngleX           = finalParameters[0];
  const double finalAngleY           = finalParameters[1];
  const double finalAngleZ           = finalParameters[2];
  const double finalTranslationX     = finalParameters[3];
  const double finalTranslationY     = finalParameters[4];
  const double finalTranslationZ     = finalParameters[5];

  const unsigned int numberOfIterations = optimizer->GetCurrentIteration();
  const double       bestValue = optimizer->GetValue();

  // Print out results
  const double finalAngleInDegreesX = finalAngleX * 45.0 / atan(1.0);
  const double finalAngleInDegreesY = finalAngleY * 45.0 / atan(1.0);
  const double finalAngleInDegreesZ = finalAngleZ * 45.0 / atan(1.0);

  std::cout << "Result = " << std::endl;
  std::cout << " AngleX (radians)   = " << finalAngleX  << std::endl;
  std::cout << " AngleX (degrees)   = " << finalAngleInDegreesX  << std::endl;
  std::cout << " AngleY (radians)   = " << finalAngleY  << std::endl;
  std::cout << " AngleY (degrees)   = " << finalAngleInDegreesY  << std::endl;
  std::cout << " AngleZ (radians)   = " << finalAngleZ  << std::endl;
  std::cout << " AngleZ (degrees)   = " << finalAngleInDegreesZ  << std::endl;
  std::cout << " Translation X = " << finalTranslationX  << std::endl;
  std::cout << " Translation Y = " << finalTranslationY  << std::endl;
  std::cout << " Translation Z = " << finalTranslationZ  << std::endl;
  std::cout << " Iterations    = " << numberOfIterations << std::endl;
  std::cout << " Metric value  = " << bestValue          << std::endl;

  results.AngleX = finalAngleInDegreesX;
  results.AngleY = finalAngleInDegreesY;
  results.AngleZ = finalAngleInDegreesZ;
  results.TranslationX = finalTranslationX;
  results.TranslationY = finalTranslationY;
  results.TranslationZ = finalTranslationZ;
  results.MutualInformation = -bestValue;

  return results;
}
