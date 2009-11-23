#ifndef _itk_DWIHeadMotionEddyCurrent_correction_txx
#define _itk_DWIHeadMotionEddyCurrent_correction_txx

#include "itkDWIHeadMotionEddyCurrentCorrection.h"

inline DWIHeadMotionEddyCurrentCorrection::DWIHeadMotionEddyCurrentCorrection(){
    translationScale = 1e-3;
    stepLength = 0.1; //default value of initial step size
    factor = 0.5;//default value of reduction ratio of step size
    numberOfBins = 24;//default value of histogram_size
    samples = 0; //default value of samples
    maxNumberOfIterations = 500;
}

inline VectorImage<float, 3>::Pointer  DWIHeadMotionEddyCurrentCorrection::Registration(){

    std::cout << "staring registration... " << std::endl;
    dwis = VectorImageType::New();
    dwis->CopyInformation(fixedImage);
    dwis->SetRegions(fixedImage->GetLargestPossibleRegion());
    dwis->SetVectorLength( movingImages.size() + 1 ) ; // grads+b0
    dwis->Allocate();

    dwiContainer.push_back( fixedImage );
    //generate target images
	for(unsigned int i=1; i<=movingImages.size(); i++){
		ImageType::Pointer dwi = ImageType::New();
		dwi->CopyInformation(fixedImage);
		dwi->SetRegions(fixedImage->GetLargestPossibleRegion());
		dwi->Allocate();
		dwi->FillBuffer( 0.0 );
		dwiContainer.push_back( dwi );
	}
	
    for(unsigned int i=0; i<movingImages.size(); i++){
	std::cout << "************************************************" << std::endl;
	std::cout << "registration DWI:" << i << std::endl;
         
    /*const bool flag = i*/
    RegistrationSingleDWI( fixedImage, movingImages.at(i), originDirs.at(i), i);
					
    std::cout<< "origin dir: " << originDirs.at(i) << std::endl;
    std::cout<< "update dir: " << updateDirs.at(i)<< std::endl;
    }

    //header.SetGradContainer( updateDirs );
    ImageRegionIterator< VectorImageType > out( dwis , dwis->GetLargestPossibleRegion() );
    
    typedef ImageRegionIterator< ImageType > IteratorImageType ;
    std::vector< IteratorImageType > in ;
	for(unsigned int i = 0 ; i < dwis->GetVectorLength() ; i++ ){
		IteratorImageType intemp( dwiContainer.at( i ) , dwiContainer.at( i )->GetLargestPossibleRegion() ) ;
		intemp.GoToBegin();
		in.push_back( intemp ) ;
	}

    VectorImageType::PixelType value ;
    value.SetSize( dwiContainer.size() ) ;
    for( out.GoToBegin() ; !out.IsAtEnd() ; ++out ){
	for( unsigned int i = 0 ; i < dwis->GetVectorLength() ; i++ ){
	      value.SetElement( i , in.at( i ).Get() ) ;
	      ++in[ i ] ;
	}
	      out.Set( value ) ;
    }
  
    return dwis;
}


inline bool DWIHeadMotionEddyCurrentCorrection::RegistrationSingleDWI(
    ImageType::Pointer fixedImageLocal, ImageType::Pointer movingImageLocal,
    const GradientType& gradDir, unsigned int no )
{


  MetricType::Pointer         metric        = MetricType::New();
  OptimizerType::Pointer      optimizer     = OptimizerType::New();
  LinearInterpolatorType::Pointer   linearInterpolator  = LinearInterpolatorType::New();
  RegistrationType::Pointer   registration  = RegistrationType::New();

  registration->SetMetric(        metric        );
  registration->SetOptimizer(     optimizer     );
  registration->SetInterpolator(  linearInterpolator  );

  TransformType::Pointer  transform = TransformType::New();
  registration->SetTransform( transform );

  if( samples > 0) {
	metric->SetNumberOfSpatialSamples( samples );
  	metric->SetUseAllPixels( false );
        //Ensure the random number generator has the same seed
	metric->ReinitializeSeed(0);
	std::cout << std::endl << "Samples Size: " << samples << std::endl;
  }
  else {
	metric->SetUseAllPixels( true );
  }

  RescaleFilterType::Pointer    fixedRescaleFilter   
                                                  = RescaleFilterType::New();
  fixedRescaleFilter->SetInput(  fixedImageLocal  );
  fixedRescaleFilter->SetOutputMinimum(  0 );
  fixedRescaleFilter->SetOutputMaximum( numberOfBins - 1 );  
  fixedRescaleFilter->Update();
 

  RescaleFilterType::Pointer    movingRescaleFilter  
                                                  = RescaleFilterType::New();
  movingRescaleFilter->SetInput(  movingImageLocal );
  movingRescaleFilter->SetOutputMinimum(  0 );
  movingRescaleFilter->SetOutputMaximum( numberOfBins - 1 );  
  movingRescaleFilter->Update();

  registration->SetFixedImage(    fixedRescaleFilter->GetOutput()    );
  registration->SetMovingImage(   movingRescaleFilter->GetOutput()   );

  registration->SetFixedImageRegion( 
  			fixedRescaleFilter->GetOutput()->GetBufferedRegion() );


  TransformInitializerType::Pointer initializer = TransformInitializerType::New();

  AffineTransformType::Pointer  affine_transform = AffineTransformType::New();

  initializer->SetTransform( affine_transform );
  initializer->SetFixedImage(  fixedRescaleFilter->GetOutput() );
  initializer->SetMovingImage( movingRescaleFilter->GetOutput() );
  initializer->MomentsOn();
  initializer->InitializeTransform();

  MetricType::TransformParametersType parameter = transform->GetParameters();
  //std::cout << "initial parameters: " << parameter << std::endl;
  
  parameter[0]=0.0; //versor Rx part
  parameter[1]=0.0; //versor Ry part
  parameter[2]=0.0; //versor Rz part
  parameter[3]=0.0; //versor x translation
  parameter[4]=0.0; //versor y translation
  parameter[5]=0.0; //versor z translation
  
  parameter[6]=0.0; //Eddy Shear x 
  parameter[7]=1.0; //Eddy Scale y
  parameter[8]=0.0; //Eddy Shear z 
  parameter[9]=0.0; //Eddy y global translation

  transform->SetParameters(parameter);
  transform->SetCenter( affine_transform->GetCenter() );
  //std::cout << "initial center:" << transform->GetCenter() << std::endl << std::endl;

  registration->SetInitialTransformParameters( parameter );

  OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );

  optimizerScales[0] =  1.0;
  optimizerScales[1] =  1.0;
  optimizerScales[2] =  1.0;
  optimizerScales[3] =  translationScale;
  optimizerScales[4] =  translationScale;
  optimizerScales[5] =  translationScale;
  //optimizerScales[5] =  2.5 / 2.0  * translationScale; //Z axis space a little larger
  optimizerScales[6] =  1.0;
  optimizerScales[7] =  1.0;
  optimizerScales[8] =  1.0;
  optimizerScales[9] =  translationScale;

  optimizer->SetScales( optimizerScales );
  optimizer->SetMaximumStepLength( stepLength ); 
  optimizer->SetMinimumStepLength( 0.001 );
  optimizer->SetNumberOfIterations( maxNumberOfIterations );
  optimizer->SetRelaxationFactor( factor );
  optimizer->MinimizeOn();

  // Create the Command observer and register it with the optimizer.
  //CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  //optimizer->AddObserver( itk::IterationEvent(), observer );

  try  { 
      registration->StartRegistration(); 
  } 
  catch( itk::ExceptionObject & err ) { 
      std::cerr << "ExceptionObject caught !" << std::endl; 
      std::cerr << err << std::endl; 
      return EXIT_FAILURE;
  } 
 
 
  OptimizerType::ParametersType finalParameters = 
                    optimizer->GetBestPosition();
  const double finalRotationCenterX = transform->GetCenter()[0];
  const double finalRotationCenterY = transform->GetCenter()[1];
  const double finalRotationCenterZ = transform->GetCenter()[2];
  const unsigned int numberOfIterations = optimizer->GetCurrentIteration();
  const double bestValue = optimizer->GetBestValue();

  // Print out results

  std::cout << std::endl << "Result = " << std::endl;
  std::cout << " Iterations    = " << numberOfIterations << std::endl;
  std::cout << " Stop Condition  = " << optimizer->GetStopCondition() << std::endl;
  std::cout << " Metric value  = " << bestValue          << std::endl;

  std::cout << " Center X      = " << finalRotationCenterX  << std::endl;
  std::cout << " Center Y      = " << finalRotationCenterY  << std::endl;
  std::cout << " Center Z      = " << finalRotationCenterZ  << std::endl;
  std::cout << " Best parameter = " << finalParameters  << std::endl;

  //std::cout << " Rotation Matrix = " << transform->GetHeadMotionRotationMatrix()  << std::endl;

  typedef itk::Matrix< PrecisionType, 3, 3 > MatrixType2D;
  MatrixType2D rotation_matrix = transform->GetHeadMotionRotationMatrix();

  //generate output 4*4 matrix
  //typedef itk::Matrix< double, 4, 4 > MatrixType3D;

  vnl_matrix_fixed<PrecisionType,4,4> head_motion_matrix;
  vnl_matrix_fixed<PrecisionType,4,4> eddy_matrix;
  vnl_matrix_fixed<PrecisionType,4,4> output_matrix;

  head_motion_matrix.set_identity();
  eddy_matrix.set_identity();
  output_matrix.set_identity();

  head_motion_matrix[0][0] = rotation_matrix[0][0];
  head_motion_matrix[0][1] = rotation_matrix[0][1];
  head_motion_matrix[0][2] = rotation_matrix[0][2];
  head_motion_matrix[1][0] = rotation_matrix[1][0];
  head_motion_matrix[1][1] = rotation_matrix[1][1];
  head_motion_matrix[1][2] = rotation_matrix[1][2];
  head_motion_matrix[2][0] = rotation_matrix[2][0];
  head_motion_matrix[2][1] = rotation_matrix[2][1];
  head_motion_matrix[2][2] = rotation_matrix[2][2];

  head_motion_matrix[0][3] = finalParameters[3];
  head_motion_matrix[1][3] = finalParameters[4];
  head_motion_matrix[2][3] = finalParameters[5];

  eddy_matrix[1][0] = finalParameters[6];
  eddy_matrix[1][1] = finalParameters[7];
  eddy_matrix[1][2] = finalParameters[8];
  eddy_matrix[1][3] = finalParameters[9];

  output_matrix = eddy_matrix * head_motion_matrix;
  //std::cout << "4 * 4 output matrix: " << output_matrix << std::endl;

  //The following code is used to rotate the gradient direction
  vnl_matrix_fixed<PrecisionType,4,4> grad_matrix;
  grad_matrix.set_identity();

  grad_matrix[0][0] = rotation_matrix[0][0];
  grad_matrix[0][1] = rotation_matrix[0][1];
  grad_matrix[0][2] = rotation_matrix[0][2];
  grad_matrix[1][0] = rotation_matrix[1][0];
  grad_matrix[1][1] = rotation_matrix[1][1];
  grad_matrix[1][2] = rotation_matrix[1][2];
  grad_matrix[2][0] = rotation_matrix[2][0];
  grad_matrix[2][1] = rotation_matrix[2][1];
  grad_matrix[2][2] = rotation_matrix[2][2];

  /**
  std::ofstream outfile;
  outfile.open("parameters.txt",std::ios_base::app);
  for(int m=0; m<10; m++)
  	outfile<< finalParameters[m] << " ";
  outfile << std::endl;
  outfile.close();


  outfile.open("centers.txt",std::ios_base::app);
  outfile<< finalRotationCenterX << " "
	<< finalRotationCenterY << " "
	<< finalRotationCenterZ;
  outfile << std::endl;
  outfile.close();
  **/

  vnl_vector_fixed<PrecisionType, 4> vec1, vec2;

  //std::cout << "grad: " << gradDir << std::endl;

  vec1[0] = gradDir[0]; 
  vec1[1] = gradDir[1];
  vec1[2] = gradDir[2];
  vec1[3] = 1.0;

  GradientType updateDir;

  vnl_matrix_fixed<PrecisionType,4,4> T = 
      grad_matrix.transpose();
  vec2 = T  * vec1;

  updateDir[0] = vec2[0];
  updateDir[1] = vec2[1];
  updateDir[2] = vec2[2];

  updateDirs.push_back( updateDir );
  //  The following code is used to dump output images to files.
  //  They illustrate the final results of the registration.
  //  We will resample the moving image

  typedef itk::CastImageFilter< ImageType,
                                ImageType >   CastFilterType;

  CastFilterType::Pointer caster = CastFilterType::New();
  caster->SetInput( movingImageLocal );

  typedef itk::ResampleImageFilter< 
                            ImageType, 
                            ImageType >    ResampleFilterType;

  MetricType::TransformParametersType parameter2 = transform->GetParameters();

  TransformType::Pointer finalTransform = TransformType::New();
  finalTransform->SetCenter( transform->GetCenter() );
  //std::cout << "final center:" << transform->GetCenter() << std::endl; 
  finalTransform->SetParameters( transform->GetParameters() );

  ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetTransform( finalTransform );
  resampler->SetInput( caster->GetOutput() );

  resampler->SetSize(    fixedImageLocal->GetLargestPossibleRegion().GetSize() );
  resampler->SetOutputOrigin(  fixedImageLocal->GetOrigin() );
  resampler->SetOutputSpacing( fixedImageLocal->GetSpacing() );
  resampler->SetDefaultPixelValue( 0 );
  resampler->SetInterpolator( linearInterpolator );

  //Brightness correction factor
  typedef itk::ShiftScaleImageFilter<
               ImageType, ImageType >  ShiftScaleFilterType;

  ShiftScaleFilterType::Pointer shiftFilter      = ShiftScaleFilterType::New();
  shiftFilter->SetInput( resampler->GetOutput() );
  shiftFilter->SetScale( 2.0 - parameter2[7] );
  shiftFilter->Update();

  ImageRegionIteratorWithIndex<ImageType> itIndex(shiftFilter->GetOutput(),
		 shiftFilter->GetOutput()->GetLargestPossibleRegion());
  
  ImageRegionIterator<ImageType> it(shiftFilter->GetOutput(),
		 shiftFilter->GetOutput()->GetLargestPossibleRegion());
 
  for (it.GoToBegin(), itIndex.GoToBegin(); !it.IsAtEnd(); ++it, ++itIndex){
		PrecisionType b = it.Get() ;
		dwiContainer.at(no+1)->SetPixel(itIndex.GetIndex(), b);
  }
 
  return true;
}

#endif
    
