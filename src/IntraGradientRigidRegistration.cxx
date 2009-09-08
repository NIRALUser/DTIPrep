#include "IntraGradientRigidRegistration.h"

CIntraGradientRigidRegistration::CIntraGradientRigidRegistration(ImageType::Pointer fixed, ImageType::Pointer moving)
{
	fixedImage = fixed;
    movingImage = moving;

}

CIntraGradientRigidRegistration::~CIntraGradientRigidRegistration(void)
{
}

void CIntraGradientRigidRegistration::SetupFramework()
{
	metric        = MetricType::New();
	optimizer     = OptimizerType::New();
	interpolator  = InterpolatorType::New();
	registration  = RegistrationType::New();
	transform	  = TransformType::New();

	registration->SetMetric(        metric        );
	registration->SetOptimizer(     optimizer     );
	registration->SetInterpolator(  interpolator  );
	registration->SetTransform(     transform	  );

	registration->SetFixedImage(    fixedImage);
	registration->SetMovingImage(   movingImage  );

	CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
	optimizer->AddObserver( itk::IterationEvent(), observer );
}

void CIntraGradientRigidRegistration::SetupParameters()
{
	registration->SetFixedImageRegion( fixedImage->GetBufferedRegion() );

	const SpacingType fixedSpacing = fixedImage->GetSpacing();
	const OriginType  fixedOrigin  = fixedImage->GetOrigin();
	const RegionType  fixedRegion  = fixedImage->GetLargestPossibleRegion(); 
	const SizeType    fixedSize    = fixedRegion.GetSize();

	TransformType::InputPointType centerFixed;

	centerFixed[0] = fixedOrigin[0] + fixedSpacing[0] * fixedSize[0] / 2.0;
	centerFixed[1] = fixedOrigin[1] + fixedSpacing[1] * fixedSize[1] / 2.0;

	const SpacingType movingSpacing = movingImage->GetSpacing();
	const OriginType  movingOrigin  = movingImage->GetOrigin();
	const RegionType  movingRegion  = movingImage->GetLargestPossibleRegion();
	const SizeType    movingSize    = movingRegion.GetSize();

	TransformType::InputPointType centerMoving;

	centerMoving[0] = movingOrigin[0] + movingSpacing[0] * movingSize[0] / 2.0;
	centerMoving[1] = movingOrigin[1] + movingSpacing[1] * movingSize[1] / 2.0;

	transform->SetCenter( centerFixed );
	transform->SetTranslation( centerMoving - centerFixed );
	transform->SetAngle( 0.0 );

	registration->SetInitialTransformParameters( transform->GetParameters() );

	typedef OptimizerType::ScalesType       OptimizerScalesType;
	OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );
	
	const double translationScale = 1.0 / 1000.0;

	optimizerScales[0] = 1.0;
	optimizerScales[1] = translationScale;
	optimizerScales[2] = translationScale;
	optimizerScales[3] = translationScale;
	optimizerScales[4] = translationScale;

	optimizer->SetScales( optimizerScales );
	optimizer->SetRelaxationFactor( 0.6 );
	optimizer->SetMaximumStepLength( 0.2 ); 
	optimizer->SetMinimumStepLength( 0.0001 );
	optimizer->SetNumberOfIterations( 500 );
}

struIntra2DResults CIntraGradientRigidRegistration::Run( bool bRegister )
{
	struIntra2DResults results;
	results.Angle = 0.0;
	results.TranslationX=0.0;
	results.TranslationY=0.0;
	results.Correlation=0.0;

	if(bRegister)
	{
		results.bRegister=true;
		SetupFramework();
		SetupParameters();

		try 
		{ 
			registration->StartRegistration(); 
		} 
		catch( itk::ExceptionObject & err ) 
		{ 
			std::cerr << "ExceptionObject caught !" << std::endl; 
			std::cerr << err << std::endl; 
			return results;
		} 

		OptimizerType::ParametersType finalParameters =registration->GetLastTransformParameters();

		const double finalAngle           = finalParameters[0];
		const double finalRotationCenterX = finalParameters[1];
		const double finalRotationCenterY = finalParameters[2];
		const double finalTranslationX    = finalParameters[3];
		const double finalTranslationY    = finalParameters[4];

		const unsigned int numberOfIterations = optimizer->GetCurrentIteration();
		const double bestValue = optimizer->GetValue();

		// Print out results
		const double finalAngleInDegrees = finalAngle * 45.0 / atan(1.0);

		std::cout << "Result = " << std::endl;
		std::cout << " Angle (radians)   = " << finalAngle  << std::endl;
		std::cout << " Angle (degrees)   = " << finalAngleInDegrees  << std::endl;
		std::cout << " Center X      = " << finalRotationCenterX  << std::endl;
		std::cout << " Center Y      = " << finalRotationCenterY  << std::endl;
		std::cout << " Translation X = " << finalTranslationX  << std::endl;
		std::cout << " Translation Y = " << finalTranslationY  << std::endl;
		std::cout << " Iterations    = " << numberOfIterations << std::endl;
		std::cout << " Metric value  = " << bestValue          << std::endl;

		results.Angle = finalAngleInDegrees;
		results.TranslationX=finalTranslationX;
		results.TranslationY=finalTranslationY;
		//results.Correlation=-bestValue;

		typedef itk::ImageRegionConstIterator<ImageType>  citType;
		citType cit1(fixedImage, fixedImage ->GetBufferedRegion() );
		citType cit2(movingImage,movingImage->GetBufferedRegion() );

		cit1.GoToBegin();
		cit2.GoToBegin();

		double sAB=0.0,sA2=0.0,sB2=0.0;
		while (!cit1.IsAtEnd())
		{
			sAB+=cit1.Get()*cit2.Get();
			sA2+=cit1.Get()*cit1.Get();
			sB2+=cit2.Get()*cit2.Get();
			++cit1;
			++cit2;
		}

		if( sA2*sB2 == 0 )
		{
			results.Correlation = 1;
		}
		else
			results.Correlation=sAB/sqrt(sA2*sB2);
		std::cout << " Metric value  = " << results.Correlation << std::endl;
	}
	else
	{
		results.bRegister=false;

		typedef itk::ImageRegionConstIterator<ImageType>  citType;
		citType cit1(fixedImage, fixedImage ->GetBufferedRegion() );
		citType cit2(movingImage,movingImage->GetBufferedRegion() );

		cit1.GoToBegin();
		cit2.GoToBegin();

		double sAB=0.0,sA2=0.0,sB2=0.0;
		while (!cit1.IsAtEnd())
		{
			sAB+=cit1.Get()*cit2.Get();
			sA2+=cit1.Get()*cit1.Get();
			sB2+=cit2.Get()*cit2.Get();
			++cit1;
			++cit2;
		}

		if( sA2*sB2 == 0 )
		{
			//if(sA2==sB2)
			//	results.Correlation = 1;
			//else
			//	results.Correlation = 0;
			results.Correlation = 1;
		}
		else
			results.Correlation=sAB/sqrt(sA2*sB2);

// 		std::cout << " Metric value  = " << results.Correlation << std::endl;
		//if(1)
		//{
		//	std::cout << " sAB  = " << sAB << std::endl;
		//	std::cout << " sA2  = " << sA2 << std::endl;
		//	std::cout << " sB2  = " << sB2 << std::endl;
		//	std::cout << " sqrt(sA2*sB2)  = " << sqrt(sA2*sB2) << std::endl;
		//	std::cout << " sAB/sqrt(sA2*sB2)  = " << sAB/sqrt(sA2*sB2) << std::endl;
		//}
	}
	return results;

}
