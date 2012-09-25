#ifndef _itk_DWIHeadMotionEddyCurrent_correction_h
#define _itk_DWIHeadMotionEddyCurrent_correction_h

#include "itkImageRegistrationMethod.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkGradientSteepestDescentOptimizer.h"
#include "itkGradientSteepestDescentBaseOptimizer.h"

#include "itkWindowedSincInterpolateImageFunction.h"
#include "itkCenteredTransformInitializer.h"
#include "itkAffineTransform.h"
#include "itkLinearHeadEddy3DCorrection.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkShiftScaleImageFilter.h"

#include "itkMatrix.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"
#include "vnl/algo/vnl_svd.h"
#include "vnl/vnl_vector.h"

#include "itkCommand.h"
#include "itkVectorImage.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionIteratorWithIndex.h"

#if (ITK_VERSION_MAJOR < 4)
#include "itkOrientedImage.h"
#endif
namespace itk
{
class DWIHeadMotionEddyCurrentCorrection
{
public:
  const static unsigned int Dimension = 3;

  typedef double      PrecisionType;
  typedef std::string TextType;
  typedef float       PixelType;

  typedef Image<PixelType, Dimension> ImageType;
  typedef ImageType::IndexType        IndexType;
  // typedef NrrdHeader::VectorType GradientType;
  // typedef NrrdHeader::VectorContainerType GradientContainerType;

  typedef vnl_vector_fixed<PrecisionType, Dimension> GradientType;
  typedef std::vector<GradientType>                  GradientContainerType;

  typedef std::vector<ImageType::Pointer> ImageContainerType;

  typedef itk::VectorImage<float, Dimension> VectorImageType;
#if (ITK_VERSION_MAJOR < 4)
  typedef OrientedImage<PixelType, Dimension> OrientImageType;
#else
  typedef itk::Image<PixelType, Dimension> OrientImageType;
#endif
  typedef std::vector<OrientImageType::Pointer> OrientImageContainerType;

  DWIHeadMotionEddyCurrentCorrection();

  VectorImageType::Pointer  Registration();

  /*
      inline NrrdHeader GetNrrdHeader( ){
    return this->header;
      }

      inline void SetNrrdHeader( NrrdHeader header ){
    this->header = header;
      }
  */

  inline GradientContainerType GetGradients()
  {
    return this->updateDirs;
  }

  inline void SetGradients( GradientContainerType grads )
  {
    this->originDirs = grads;
  }

  inline void SetBaseLines( ImageType::Pointer baseline )
  {
    this->baseLines.push_back( baseline );
  }

  inline void SetFixedImage( ImageType::Pointer fixedImageNew )
  {
    this->fixedImage = fixedImageNew;
  }

  inline void SetMovingImage( ImageType::Pointer movingImageNew )
  {
    this->movingImages.push_back( movingImageNew );
  }

  inline void SetTranslationScale( PrecisionType translationScaleNew )
  {
    this->translationScale = translationScaleNew;
  }

  inline void SetStepLength( PrecisionType stepLengthNew )
  {
    this->stepLength = stepLengthNew;
  }

  inline void SetFactor( PrecisionType factorNew )
  {
    this->factor = factorNew;
  }

  inline void SetNumberOfBins( const unsigned int numberOfBinsNew )
  {
    this->numberOfBins = numberOfBinsNew;
  }

  inline void SetSamples( const unsigned int samplesNew )
  {
    this->samples = samplesNew;
  }

  inline void SetMaxNumberOfIterations(
    const unsigned int maxNumberOfIterationsNew )
  {
    this->maxNumberOfIterations = maxNumberOfIterationsNew;
  }

  inline void SetPrefix( const TextType & outputImageFileNameNew  )
  {
    this->outputImageFileName = outputImageFileNameNew;
  }

private:
  // HACK:  THese all need to have m_ for their file names.
  PrecisionType translationScale;
  PrecisionType stepLength;
  PrecisionType factor;

  unsigned int numberOfBins;
  unsigned int samples;
  unsigned int maxNumberOfIterations;

  TextType fixedImageFileName;
  TextType movingImageFileName;
  TextType outputImageFileName;
  TextType outputImagePrefix;

  ImageType::Pointer fixedImage;
  ImageContainerType movingImages;
  ImageContainerType baseLines;

  ImageContainerType m_dwiRegisteredImagesContainer;
  // OrientImageContainerType dwiRegisteredImagesContainer;

  // NrrdHeader header;
  GradientContainerType originDirs;
  GradientContainerType updateDirs;

  VectorImageType::Pointer dwis;

  // TODO:  Need to provide a description of what this funciton does.
  //       What type of registration is this?
  bool  RegistrationSingleDWI( ImageType::Pointer fixedImage, ImageType::Pointer movingImage,
                               const GradientType & gradDir,
                               unsigned int no);

  typedef itk::AffineTransform<
    PrecisionType,
    Dimension>     AffineTransformType;

  typedef itk::LinearHeadEddy3DCorrection<
    PrecisionType,
    Dimension>     TransformType;

  typedef itk::GradientSteepestDescentOptimizer OptimizerType;

  typedef itk::MattesMutualInformationImageToImageMetric<
    ImageType,
    ImageType>    MetricType;

  typedef itk::LinearInterpolateImageFunction<
    ImageType,
    PrecisionType>    LinearInterpolatorType;

  typedef itk::ImageRegistrationMethod<
    ImageType,
    ImageType>    RegistrationType;

  typedef itk::RescaleIntensityImageFilter<
    ImageType, ImageType>  RescaleFilterType;

  typedef itk::CenteredTransformInitializer<
    AffineTransformType,
    ImageType,
    ImageType>  TransformInitializerType;

  typedef OptimizerType::ScalesType OptimizerScalesType;

  class CommandIterationUpdate : public itk::Command
  {
public:
    typedef  CommandIterationUpdate Self;
    typedef  itk::Command           Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    itkNewMacro( Self );
protected:
    CommandIterationUpdate()
    {
    }

public:
    typedef itk::GradientSteepestDescentOptimizer OptimizerType;
    typedef   const OptimizerType *               OptimizerPointer;

    void Execute(itk::Object *caller, const itk::EventObject & event)
    {
      Execute( (const itk::Object *)caller, event );
    }

    void Execute(const itk::Object *object, const itk::EventObject & event)
    {
      OptimizerPointer optimizer
        = dynamic_cast<OptimizerPointer>( object );

      if( !itk::IterationEvent().CheckEvent( &event ) )
        {
        return;
        }
      std::cout << std::endl << "No: " << optimizer->GetCurrentIteration()
                << "   ";
      std::cout << "Best Value: " << optimizer->GetBestValue() << std::endl;
      std::cout << "Best Para:" << optimizer->GetBestPosition() << std::endl;

      std::cout << "current value: " << optimizer->GetValue() << "  ";
      std::cout << "Step = " << optimizer->GetCurrentStepLength() << std::endl;
      std::cout << "next para: " << optimizer->GetCurrentPosition()
                << std::endl;
    }

  };
};

} // end of namespace

// Changed to a .cpp file #include "itkDWIHeadMotionEddyCurrentCorrection.hxx"

#endif /*__itk_DWIHeadMotionEddyCurrent_correction_h */
