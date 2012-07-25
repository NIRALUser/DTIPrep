

#include "itkMultiImageRegistrationFilter.h"
#include "itkMinimumMaximumImageCalculator.h"

namespace itk
{
MultiImageRegistrationFilter
::MultiImageRegistrationFilter()
{
  m_Optimizer = LineSearchOptimizerType::New();
  m_Registration = RegistrationType::New();
  m_Metric = EntropyMetricType::New();
  m_Output = ImageType::New();
  m_Command = CommandType::New();
  m_NumberOfImages = 0;

  m_AffineTransformArray.reserve(2);
  m_BsplineTransformArray.reserve(2);
  m_InterpolatorArray.reserve(2);
  m_BsplineInitializerArray.reserve(2);

  m_MultiLevelAffine = 3;
  m_MultiLevelBspline = 3;

  m_OptAffineLearningRate = 1e-4;
  m_OptBsplineLearningRate = 1e4;
  m_OptAffineNumberOfIterations = 50;
  m_OptBsplineNumberOfIterations = 75;
  m_NumberOfSpatialSamplesAffinePercentage = 0.0025;
  m_NumberOfSpatialSamplesBsplinePercentage = 0.02;
  m_NumberOfSpatialSamplesAffine = 0;
  m_NumberOfSpatialSamplesBspline = 0;
  m_AffineMultiScaleSamplePercentageIncrease = 4.0;
  m_BsplineMultiScaleSamplePercentageIncrease = 4.0;
  m_AffineMultiScaleMaximumIterationIncrease = 2.5;
  m_BsplineMultiScaleMaximumIterationIncrease = 3.0;
  m_AffineMultiScaleStepLengthIncrease = 4.0;
  m_BsplineMultiScaleStepLengthIncrease = 4.0;
  m_TranslationScaleCoeffs = 1e-4;
  m_MaximumLineIteration = 6;
  m_BsplineInitialGridSize = 28;
  m_NumberOfBsplineLevel = 2;
  m_BsplineRegularizationFactor = 1e-1;
  m_ParzenWindowStandardDeviation = 0.4;
  m_GaussianFilterKernelWidth = 2.0;
}

MultiImageRegistrationFilter
::~MultiImageRegistrationFilter()
{
}

void MultiImageRegistrationFilter
::Initialize()
{
  m_NumberOfImages = m_InputImageArray.size();
  if( m_NumberOfImages < 2 )
    {
    std::cout << "Not enough input images!" << std::endl;
    return;
    }
  m_Registration->SetNumberOfImages(m_NumberOfImages);
  m_Registration->SetNumberOfLevels( m_MultiLevelAffine );
  for( unsigned int i = 0; i < m_NumberOfImages; i++ )
    {
    // Set up the Image Mask
    LFFMaskFilterType::Pointer LFF = LFFMaskFilterType::New();
    LFF->SetInput(m_InputImageArray[i]);
    LFF->SetOtsuPercentileThreshold(0.01);
    LFF->SetClosingSize(7);
    LFF->Update();

    ImageMaskSpatialObjectType::Pointer maskImage = ImageMaskSpatialObjectType::New();
    maskImage->SetImage(LFF->GetOutput() );
    m_Registration->SetImageMaskArray(i, maskImage);
    // Set up the Image Pyramid
    ImagePyramidType::Pointer imagePyramid = ImagePyramidType::New();
    imagePyramid->SetNumberOfLevels( m_MultiLevelAffine );
    imagePyramid->SetInput(m_InputImageArray[i] );
    imagePyramid->Update();
    // Set the input into the registration method
    m_ImagePyramidArray.push_back(imagePyramid);
    m_Registration->SetImagePyramidArray(i, m_ImagePyramidArray[i] );

    AffineTransformType::Pointer    affineTransform = AffineTransformType::New();
    BSplineTransformType::Pointer   bsplineTransform = BSplineTransformType::New();
    BSplineInitializerType::Pointer bsplineInitializer = BSplineInitializerType::New();
    InterpolatorType::Pointer       interpolator = InterpolatorType::New();
    m_AffineTransformArray.push_back(affineTransform);
    m_BsplineTransformArray.push_back(bsplineTransform);
    m_InterpolatorArray.push_back(interpolator);
    m_BsplineInitializerArray.push_back(bsplineInitializer);

    m_Registration->SetTransformArray(i, m_AffineTransformArray[i]);
    m_Registration->SetInterpolatorArray(i, m_InterpolatorArray[i] );
    }

  m_Registration->SetOptimizer(m_Optimizer);

  typedef MinimumMaximumImageCalculator<ImageType> MinMaxFilterType;
  MinMaxFilterType::Pointer minmaxfilter = MinMaxFilterType::New();
  minmaxfilter->SetImage(m_InputImageArray[0]);
  minmaxfilter->ComputeMaximum();
  minmaxfilter->ComputeMinimum();
  m_ParzenWindowStandardDeviation = 0.05 * (minmaxfilter->GetMaximum() - minmaxfilter->GetMinimum() );

  m_Metric->SetImageStandardDeviation(m_ParzenWindowStandardDeviation);
  m_Metric->SetGaussianFilterKernelWidth(m_GaussianFilterKernelWidth);
  m_Metric->SetNumberOfImages(m_NumberOfImages);
  m_Registration->SetMetric( m_Metric  );

  m_Command->bsplineGridSize = m_BsplineInitialGridSize;

  m_Registration->AddObserver( itk::IterationEvent(), m_Command );
  std::cout << "Initialize..." << std::endl;
}

void
MultiImageRegistrationFilter
::AffineRegistration()
{
  // Set the scales of the optimizer
  // We set a large scale for the parameters corresponding to translation
  OptimizerScalesType optimizerAffineScales( m_AffineTransformArray[0]->GetNumberOfParameters() * m_NumberOfImages );

  optimizerAffineScales.Fill(1.0);
  int numberOfParameters = m_AffineTransformArray[0]->GetNumberOfParameters();
  for( unsigned int i = 0; i < m_NumberOfImages; i++ )
    {
    for( unsigned int j = 0; j < Dimension * Dimension; j++ )
      {
      optimizerAffineScales[i * numberOfParameters + j] = 1.0;  // scale for indices in 2x2 (3x3) Matrix
      }
    for( unsigned int j = Dimension * Dimension; j < Dimension + Dimension * Dimension; j++ )
      {
      optimizerAffineScales[i * numberOfParameters + j] = m_TranslationScaleCoeffs; // scale for translation on X,Y,Z
      }
    }

  m_Optimizer->SetStepLength(m_OptAffineLearningRate);
  m_Optimizer->SetMaximize(false);
  m_Optimizer->SetMaximumIteration( m_OptAffineNumberOfIterations );
  m_Optimizer->SetMaximumLineIteration( m_MaximumLineIteration );
  m_Optimizer->SetScales( optimizerAffineScales );

  // Set the number of spatial samples for the metric
  unsigned int numberOfSamples = m_NumberOfSpatialSamplesAffine;
  // Get the number of pixels (voxels) in the images

  ImageType::RegionType fixedImageRegion =
    m_ImagePyramidArray[0]->GetOutput(m_ImagePyramidArray[0]->GetNumberOfLevels() - 1)->GetBufferedRegion();
  m_Registration->SetFixedImageRegion( fixedImageRegion );

  const unsigned int numberOfPixels = fixedImageRegion.GetNumberOfPixels();

  if( m_NumberOfSpatialSamplesAffinePercentage > 0 )
    {
    numberOfSamples = static_cast<unsigned int>( numberOfPixels * m_NumberOfSpatialSamplesAffinePercentage );
    }
  m_Metric->SetNumberOfSpatialSamples( numberOfSamples );

  // Set the parameters of the command observer
  m_Command->SetMultiScaleSamplePercentageIncrease(m_AffineMultiScaleSamplePercentageIncrease);
  m_Command->SetMultiScaleMaximumIterationIncrease(m_AffineMultiScaleMaximumIterationIncrease);
  m_Command->SetMultiScaleStepLengthIncrease(m_AffineMultiScaleStepLengthIncrease);

  ParametersType initialAffineParameters( m_AffineTransformArray[0]->GetNumberOfParameters() * m_NumberOfImages );
  initialAffineParameters.Fill(0.0);
  m_Registration->SetInitialTransformParameters( initialAffineParameters );

  // int numberOfParameters = affineTransformArray[0]->GetNumberOfParameters();
  for( unsigned int i = 0; i < m_NumberOfImages; i++ )
    {
    m_AffineTransformArray[i]->SetIdentity();
    CenterPointType     center;
    ImageType::SizeType size =
      m_ImagePyramidArray[i]->GetOutput(m_ImagePyramidArray[0]->GetNumberOfLevels()
                                        - 1)->GetLargestPossibleRegion().GetSize();
    ImageType::IndexType startIndex =
      m_ImagePyramidArray[i]->GetOutput(m_ImagePyramidArray[0]->GetNumberOfLevels()
                                        - 1)->GetLargestPossibleRegion().GetIndex();

    ContinuousIndexType centerIndex;
    // Place the center of rotation to the center of the image
    for( unsigned int j = 0; j < Dimension; j++ )
      {
      centerIndex[j] = static_cast<ContinuousIndexValueType>(startIndex[j] )
        + static_cast<ContinuousIndexValueType>(size[j] - 1) / 2.0;
      }
    m_ImagePyramidArray[i]->GetOutput(m_ImagePyramidArray[0]->GetNumberOfLevels()
                                      - 1)->TransformContinuousIndexToPhysicalPoint(centerIndex, center);
    m_AffineTransformArray[i]->SetCenter(center);
    m_Registration->SetInitialTransformParameters( i, m_AffineTransformArray[i]->GetParameters() );
    }

  try
    {
    m_Registration->Update();
    }
  catch( ExceptionObject & err )
    {
    std::cout << "ExceptionObject caught !" << std::endl;
    std::cout << err << std::endl;
    return;
    }
}

void
MultiImageRegistrationFilter
::BsplineRegistraiton()
{
  ParametersType affineParameters = m_Registration->GetLastTransformParameters();
  ParametersType affineCurrentParameters(m_AffineTransformArray[0]->GetNumberOfParameters() );
  // Update the affine transform parameters
  for( unsigned int i = 0; i < m_NumberOfImages; i++ )
    {
    for( unsigned int j = 0; j < m_AffineTransformArray[0]->GetNumberOfParameters(); j++ )
      {
      affineCurrentParameters[j] = affineParameters[i * m_AffineTransformArray[0]->GetNumberOfParameters() + j];
      }
    m_AffineTransformArray[i]->SetParametersByValue(affineCurrentParameters);
    }

  // Initialize the size of the parameters array
  m_Registration->SetTransformParametersLength( static_cast<int>( pow( static_cast<double>(m_BsplineInitialGridSize
                                                                                           + SplineOrder),
                                                                       static_cast<int>(Dimension) ) * Dimension
                                                                  * m_NumberOfImages ) );

  // Set the initial Bspline parameters to zero
  typedef BSplineTransformType::ParametersType BSplineParametersType;
  BSplineParametersType bsplineParameters;
  for( unsigned int i = 0; i < m_NumberOfImages; i++ )
    {
    m_BsplineInitializerArray[i]->SetTransform(m_BsplineTransformArray[i]);
    m_BsplineInitializerArray[i]->SetImage(m_ImagePyramidArray[i]->GetOutput(m_ImagePyramidArray[i]->GetNumberOfLevels()
                                                                             - 1) );
    m_BsplineInitializerArray[i]->SetNumberOfGridNodesInsideTheImage(m_BsplineInitialGridSize);
    m_BsplineInitializerArray[i]->InitializeTransform();
    unsigned int numberOfParameters =
      m_BsplineTransformArray[i]->GetNumberOfParameters();

    // Set the initial Bspline parameters to zero
    bsplineParameters.SetSize( numberOfParameters );
    bsplineParameters.Fill( 0.0 );

    // Set the affine tranform and initial paramters of Bsplines
    m_BsplineTransformArray[i]->SetBulkTransform(m_AffineTransformArray[i]);
    m_BsplineTransformArray[i]->SetParameters( bsplineParameters);

    // register Bspline pointers with the registration method
    m_Registration->SetInitialTransformParameters( i, m_BsplineTransformArray[i]->GetParameters() );
    m_Registration->SetTransformArray(i, m_BsplineTransformArray[i] );
    m_Metric->SetBSplineTransformArray(i, m_BsplineTransformArray[i] );
    }

  OptimizerScalesType optimizerScales( m_BsplineTransformArray[0]->GetNumberOfParameters() * m_NumberOfImages );
  optimizerScales.Fill( 1.0 );
  m_Optimizer->SetScales( optimizerScales );
  m_Optimizer->SetStepLength(m_OptBsplineLearningRate);
  m_Optimizer->SetMaximumIteration( m_OptBsplineNumberOfIterations );

  ImageType::RegionType fixedImageRegion =
    m_ImagePyramidArray[0]->GetOutput(m_ImagePyramidArray[0]->GetNumberOfLevels() - 1)->GetBufferedRegion();

  const unsigned int numberOfPixels = fixedImageRegion.GetNumberOfPixels();

  int numberOfSamples = m_NumberOfSpatialSamplesBspline;
  if( m_NumberOfSpatialSamplesBsplinePercentage > 0 )
    {
    numberOfSamples = static_cast<unsigned int>( numberOfPixels * m_NumberOfSpatialSamplesBsplinePercentage );
    }
  m_Metric->SetNumberOfSpatialSamples( numberOfSamples );
  m_Registration->SetNumberOfLevels( m_MultiLevelBspline );

  // Set the parameters of the command observer
  m_Command->SetMultiScaleSamplePercentageIncrease(m_BsplineMultiScaleSamplePercentageIncrease);
  m_Command->SetMultiScaleMaximumIterationIncrease(m_BsplineMultiScaleMaximumIterationIncrease);
  m_Command->SetMultiScaleStepLengthIncrease(m_BsplineMultiScaleStepLengthIncrease);

  try
    {
    m_Registration->Update();
    }
  catch( ExceptionObject & err )
    {
    std::cout << "ExceptionObject caught !" << std::endl;
    std::cout << err << std::endl;
    return;
    }
}

void
MultiImageRegistrationFilter
::Update()
{
//   this->AllocateOutputs();
  this->Initialize();
  this->AffineRegistration();
  this->BsplineRegistraiton();

  NaryMEANImageFilter::Pointer naryMeanImageFilter = NaryMEANImageFilter::New();
  for( unsigned int i = 0; i < m_NumberOfImages; i++ )
    {
    ResampleFilterType::Pointer imageResample = ResampleFilterType::New();

    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    imageResample->SetInterpolator( interpolator );
    imageResample->SetTransform( m_BsplineTransformArray[i] );

    imageResample->SetInput( m_InputImageArray[i] );
    imageResample->SetSize(    m_InputImageArray[0]->GetLargestPossibleRegion().GetSize() );
    imageResample->SetOutputOrigin(  m_InputImageArray[0]->GetOrigin() );
    imageResample->SetOutputSpacing( m_InputImageArray[0]->GetSpacing() );
    imageResample->SetOutputDirection( m_InputImageArray[0]->GetDirection() );
    imageResample->SetDefaultPixelValue( 0 );
    imageResample->Update();
    naryMeanImageFilter->SetInput(i, imageResample->GetOutput() );
    }
  naryMeanImageFilter->Update();
  m_Output =  naryMeanImageFilter->GetOutput();
}

} // end namespace itk
