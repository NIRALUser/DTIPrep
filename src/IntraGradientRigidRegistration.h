
#include "itkImage.h"

#include "itkCenteredRigid2DTransform.h"
#include "itkImageRegistrationMethod.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkRegularStepGradientDescentOptimizer.h"

#include "itkResampleImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

typedef struct  Results
	{ 
		bool		bRegister;
		double		Angle; // in degrees
		double		TranslationX;
		double		TranslationY;
		double		Correlation; //-Metrix
	} struIntra2DResults,  *pstruIntra2DResults;

class CIntraGradientRigidRegistration
{
public:  
  static const    unsigned int    Dimension = 2;
  typedef  unsigned short   PixelType;
  typedef itk::Image< PixelType, Dimension >  ImageType;

public:
	//CIntraGradientRigidRegistration( );
	CIntraGradientRigidRegistration(ImageType::Pointer fixed, ImageType::Pointer moving);
	~CIntraGradientRigidRegistration(void);

public:
  
  typedef itk::CenteredRigid2DTransform< double > TransformType;
  
  typedef itk::RegularStepGradientDescentOptimizer  OptimizerType;
  typedef itk::NormalizedCorrelationImageToImageMetric<  ImageType, ImageType >    MetricType;
  typedef itk::LinearInterpolateImageFunction< ImageType,  double   >    InterpolatorType;
  typedef itk::ImageRegistrationMethod<  ImageType,  ImageType >    RegistrationType;

  typedef ImageType::SpacingType    SpacingType;
  typedef ImageType::PointType      OriginType;
  typedef ImageType::RegionType     RegionType;
  typedef ImageType::SizeType       SizeType;


private:

  MetricType::Pointer         metric ;
  OptimizerType::Pointer      optimizer;
  InterpolatorType::Pointer   interpolator;
  RegistrationType::Pointer   registration;
  TransformType::Pointer      transform;

  ImageType::Pointer fixedImage;
  ImageType::Pointer movingImage;
 
public:
	//void SetFixedImage(  ImageType::Pointer fixed)  { fixedImage=fixed;};
	//void SetMovingImage( ImageType::Pointer moving) { movingImage=moving;};

private:
	void SetupFramework();
	void SetupParameters();
public:
	//double Run();
	struIntra2DResults Run( bool bRegister );

};

#include "itkCommand.h"
class CommandIterationUpdate : public itk::Command 
{
public:
  typedef  CommandIterationUpdate   Self;
  typedef  itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  itkNewMacro( Self );
protected:
  CommandIterationUpdate() {};
public:
  typedef itk::RegularStepGradientDescentOptimizer     OptimizerType;
  typedef   const OptimizerType   *    OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event)
    {
      Execute( (const itk::Object *)caller, event);
    }

  void Execute(const itk::Object * object, const itk::EventObject & event)
    {
      OptimizerPointer optimizer = 
        dynamic_cast< OptimizerPointer >( object );
      if( ! itk::IterationEvent().CheckEvent( &event ) )
        {
        return;
        }
      std::cout << optimizer->GetCurrentIteration() << "   ";
      std::cout << optimizer->GetValue() << "   ";
      std::cout << optimizer->GetCurrentPosition() << std::endl;
    }
};

