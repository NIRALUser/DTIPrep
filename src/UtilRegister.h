#ifndef __UTILREGISTER_H__
#define __UTILREGISTER_H__

namespace itk
{
class struRigidRegResult
{
public:
  typedef struRigidRegResult Self;

  struRigidRegResult()
  {
    m_MutualInformation = 0;
    m_InternalTransform = itk::VersorRigid3DTransform<double>::New();
    m_InternalTransform->SetIdentity();
  }

  const Self & operator=(const Self & setNew)
  {
    this->m_MutualInformation = setNew.GetMutualInformation();
    this->m_InternalTransform = setNew.GetTransform();
    return *this;
  }

  struRigidRegResult(const Self & newSelf)
  {
    this->operator=(newSelf);

  }

  void Print() const
  {
    // Print out results
    std::cout << "Result = " << std::endl;
    std::cout << " AngleX (radians) = " << this->GetFinalAngleInRadiansX() << " (degrees) = "
              << this->GetFinalAngleInDegreesX() << std::endl;
    std::cout << " AngleY (radians) = " << this->GetFinalAngleInRadiansY() << " (degrees) = "
              << this->GetFinalAngleInDegreesY()  << std::endl;
    std::cout << " AngleZ (radians) = " << this->GetFinalAngleInRadiansZ() << " (degrees) = "
              << this->GetFinalAngleInDegreesZ()  << std::endl;
    std::cout << " Translation X = " << this->GetTranslationX() << std::endl;
    std::cout << " Translation Y = " << this->GetTranslationY() << std::endl;
    std::cout << " Translation Z = " << this->GetTranslationZ() << std::endl;
    std::cout << " Metric value  = " << this->GetMutualInformation() << std::endl;
  }

  double GetFinalAngleInRadiansX() const
  {
    return this->m_InternalTransform->GetParameters()[0];
  }

  double GetFinalAngleInRadiansY() const
  {
    return this->m_InternalTransform->GetParameters()[1];
  }

  double GetFinalAngleInRadiansZ() const
  {
    return this->m_InternalTransform->GetParameters()[2];
  }

  double GetTranslationX() const
  {
    return this->m_InternalTransform->GetParameters()[3];
  }

  double GetTranslationY() const
  {
    return this->m_InternalTransform->GetParameters()[4];
  }

  double GetTranslationZ() const
  {
    return this->m_InternalTransform->GetParameters()[5];
  }

  double GetFinalAngleInDegreesX() const
  {
    return this->m_InternalTransform->GetParameters()[0] * 45.0 / vcl_atan(1.0);
  }

  double GetFinalAngleInDegreesY() const
  {
    return this->m_InternalTransform->GetParameters()[1] * 45.0 / vcl_atan(1.0);
  }

  double GetFinalAngleInDegreesZ() const
  {
    return this->m_InternalTransform->GetParameters()[2] * 45.0 / vcl_atan(1.0);
  }

  double GetMutualInformation() const
  {
    return this->m_MutualInformation;
  }

  void SetMutualInformation(const double newVal)
  {
    this->m_MutualInformation = newVal;
  }

  itk::VersorRigid3DTransform<double>::Pointer GetTransform() const
  {
    return this->m_InternalTransform;
  }

  void SetTransform(itk::VersorRigid3DTransform<double>::Pointer newTransform)
  {
    this->m_InternalTransform = newTransform;
  }

private:
  double                                       m_MutualInformation; // The metric value for these parameters.
  itk::VersorRigid3DTransform<double>::Pointer m_InternalTransform;
};

} // end namespace itk

#include "BRAINSFitHelper.h"
namespace itk
{
/**
 *
 */
template <class TFixedImageType, class TMovingImageType, class TOutputImageType>

struRigidRegResult rigidRegistration(
  typename TFixedImageType::Pointer fixedImage,
  typename TMovingImageType::Pointer movingImage,
  unsigned int /* BinsNumber */,
  double /* SamplesPercent */,
  bool /* ExplicitPDFDerivatives */,
  const struRigidRegResult &  regResult,
  int /* baselineORidwi */,       // 0: baseline; otherwise: idwi
  typename TOutputImageType::Pointer & OutputBaselineImage,
  typename TOutputImageType::Pointer     /* outputIDWIImage */,
  int interpolationMethod
  )
{
  typedef itk::BRAINSFitHelper HelperType;
  HelperType::Pointer intraSubjectRegistrationHelper = HelperType::New();
  intraSubjectRegistrationHelper->SetObserveIterations(false);
  intraSubjectRegistrationHelper->SetNumberOfSamples(500000);
  intraSubjectRegistrationHelper->SetNumberOfHistogramBins(50);
    {
    std::vector<int> numberOfIterations(1);
    numberOfIterations[0] = 1500;
    intraSubjectRegistrationHelper->SetNumberOfIterations(numberOfIterations);
    }
  //  intraSubjectRegistrationHelper->SetMaximumStepLength(maximumStepSize);
  intraSubjectRegistrationHelper->SetTranslationScale(1000);
  intraSubjectRegistrationHelper->SetReproportionScale(1.0);
  intraSubjectRegistrationHelper->SetSkewScale(1.0);
  intraSubjectRegistrationHelper->SetBackgroundFillValue(0);
    {
    typedef typename itk::CastImageFilter<TFixedImageType, itk::Image<float, 3> > CasterType;
    typename CasterType::Pointer myCaster = CasterType::New();
    myCaster->SetInput(fixedImage);
    myCaster->Update();
    intraSubjectRegistrationHelper->SetFixedVolume(myCaster->GetOutput() );
    }
    {
    typedef typename itk::CastImageFilter<TMovingImageType, itk::Image<float, 3> > CasterType;
    typename CasterType::Pointer myCaster = CasterType::New();
    myCaster->SetInput(movingImage);
    myCaster->Update();
    intraSubjectRegistrationHelper->SetMovingVolume(myCaster->GetOutput() );
    }
    {
    std::vector<double> minimumStepSize(1);
    minimumStepSize[0] = 0.00005;
    intraSubjectRegistrationHelper->SetMinimumStepLength(minimumStepSize);
    std::vector<std::string> transformType(1);
    transformType[0] = "Rigid";
    intraSubjectRegistrationHelper->SetTransformType(transformType);
    }
#if 1
  intraSubjectRegistrationHelper->SetCurrentGenericTransform(regResult.GetTransform().GetPointer() );
#else
    {
    const std::string initializeTransformMode("MomentsOn");
    intraSubjectRegistrationHelper->SetInitializeTransformMode(initializeTransformMode);
    }
#endif
  //  intraSubjectRegistrationHelper->SetUseWindowedSinc(useWindowedSinc);

  intraSubjectRegistrationHelper->SetCurrentGenericTransform( regResult.GetTransform().GetPointer() );
  // if( this->m_DebugLevel > 7 )
  //  {
  // intraSubjectRegistrationHelper->PrintCommandLine(true);
  //  }
  intraSubjectRegistrationHelper->Update();
  typename itk::VersorRigid3DTransform<double>::Pointer tempInitializerITKTransform
    = dynamic_cast<itk::VersorRigid3DTransform<double> *>(
        intraSubjectRegistrationHelper->GetCurrentGenericTransform().GetPointer() );

  struRigidRegResult outputTransformResult;
  outputTransformResult.SetTransform(tempInitializerITKTransform);
  outputTransformResult.SetMutualInformation(intraSubjectRegistrationHelper->GetFinalMetricValue() );
  // HACK: NEED TO GET LAST METRIC VALUE HERE

  // resample the input baselines
  typedef itk::ResampleImageFilter<TMovingImageType, TOutputImageType> ResampleFilterType;
  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput( movingImage );
  resampler->SetTransform( outputTransformResult.GetTransform() );
  // resampler->SetOutputParametersFromImage( fixedImage );
  resampler->SetOutputOrigin( fixedImage->GetOrigin() );
  resampler->SetOutputSpacing( fixedImage->GetSpacing() );
  resampler->SetOutputDirection( fixedImage->GetDirection() );
  resampler->SetOutputStartIndex( fixedImage->GetLargestPossibleRegion().GetIndex() );
  resampler->SetSize( fixedImage->GetLargestPossibleRegion().GetSize() );
  typedef InterpolateImageFunction< TMovingImageType , double > InterpolatorType ;
  typedef LinearInterpolateImageFunction< TMovingImageType , double > LinearInterpolatorType ;
  typedef BSplineInterpolateImageFunction< TMovingImageType , double > BSplineInterpolatorType ;//we will use order 3
  typedef WindowedSincInterpolateImageFunction< TMovingImageType , 4 > WindowedSincInterpolatorType ;//default is Hamming
  typename InterpolatorType::Pointer interpolator ;
  if( interpolationMethod == 1 )
  {
      typename BSplineInterpolatorType::Pointer bsinterpolator = BSplineInterpolatorType::New() ;
      bsinterpolator->SetSplineOrder( 3 ) ;
      interpolator = bsinterpolator ;
  }
  else if( interpolationMethod == 2 )
  {
      interpolator = WindowedSincInterpolatorType::New() ;
  }
  else
  {
      interpolator = LinearInterpolatorType::New() ;
  }
  resampler->SetInterpolator( interpolator );
  resampler->SetDefaultPixelValue( 0 );
  resampler->Update();

  typename TOutputImageType::Pointer transformedImage = resampler->GetOutput();
  OutputBaselineImage = transformedImage;
  return outputTransformResult;
}

} // end namespace itk

#endif // __UTILREGISTER_H__
