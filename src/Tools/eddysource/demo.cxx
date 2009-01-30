/*=========================================================================

 This is a demo for how to register diffusion weighted images by using
 the correction program, which correct the distortion caused by
 head motion and eddy current.

=========================================================================*/

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "itkDWIHeadMotionEddyCurrentCorrection.h"
#include "itkVectorImage.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"

#include <iomanip>

#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"
#include "vnl/algo/vnl_svd.h"

#include "metaCommand.h"

int main( int argc, char *argv[] )
{
  const unsigned int dim = 3;
  
  if( argc < 3 )
    {
    std::cerr << "Missing Parameters " << std::endl;
    std::cerr << "Usage: " << argv[0];
    std::cerr << "   -i inputImageFile   -o outputImageFile " << std::endl;
    std::cerr << "   [-h histogram_size] [-s samples] [-r translation_scale]  "
	      << std::endl;
    std::cerr << "   [-l initial_stepLength] [-e factor] [-n maxNumberOfIterations] "
	      << std::endl;
    return EXIT_FAILURE;
    }

   /* Part I: Parsing parameters and checking */
  MetaCommand command;
  command.SetOption("InputImage","i", false,"input image");
  command.AddOptionField("InputImage","inputimagefilename",MetaCommand::STRING,true);

  command.SetOption("OutputImage","o", false,"input of output image");
  command.AddOptionField("OutputImage","outputimagefilename",MetaCommand::STRING,true);

  command.SetOption("HistogramSize","h", false,"histogram size for registration");
  command.AddOptionField("HistogramSize","histogramsize",MetaCommand::INT, false, "24");

  command.SetOption("Samples","s", false,"samples for registration");
  command.AddOptionField("Samples","samples",MetaCommand::INT, false, "100000");

  command.SetOption("TranslationScale","r", false,"translation_scale for registration");
  command.AddOptionField("TranslationScale","translationscale",MetaCommand::FLOAT, false, "0.001");

  command.SetOption("InitialStepLength","l", false,"initial step length for registration");
  command.AddOptionField("InitialStepLength","length",MetaCommand::FLOAT, false, "0.1");

  command.SetOption("Factor","e", false,"default value of reduction ratio of step size");
  command.AddOptionField("Factor","factor",MetaCommand::FLOAT, false, "0.5");

  command.SetOption("MaximumIterations","n", false,"default value of maxNumberOfIterations");
  command.AddOptionField("MaximumIterations","iterations",MetaCommand::INT, false, "500");

  if( !command.Parse(argc,argv) )
		return EXIT_FAILURE;

  std::string inputImageFile = command.GetValueAsString("InputImage","inputimagefilename");
  std::string outputImageFile = command.GetValueAsString("OutputImage","outputimagefilename");
  int numberOfBins = command.GetValueAsInt("HistogramSize","histogramsize");
  int samples = command.GetValueAsInt("Samples","samples");
  float translationScale = command.GetValueAsFloat("TranslationScale","translationscale");
  float stepLength = command.GetValueAsFloat("InitialStepLength","length");
  float factor = command.GetValueAsFloat("Factor","factor");
  int  maxNumberOfIterations  = command.GetValueAsInt("MaximumIterations","iterations");

  std::cout << std::endl << "Input information : " << std::endl;
  std::cout << "input image: " << "  " << inputImageFile << std::endl;
  std::cout << "output image: " << "  " << outputImageFile << std::endl;
  std::cout << "Histogram Size: " << numberOfBins << std::endl;
  std::cout << "Samples: " << samples << std::endl;
  std::cout << "Translation scale:" << translationScale << std::endl;
  std::cout << "Initial step size:"<< stepLength << std::endl;
  std::cout << "Step size reduce factor:" << factor << std::endl;
  std::cout << "Maxinum Number Of Iterations:"<< 
      maxNumberOfIterations << std::endl;
  std::cout << "check arguments OK!" << std::endl << std::endl;
 
  /* initilization */
  //1. read DWIs volume
  typedef itk::Image< float, dim >  ScalarImageType;
  typedef itk::VectorImage<float, dim> VectorImageType;
  typedef itk::ImageFileReader< VectorImageType  > VectorImageReaderType;
  VectorImageReaderType::Pointer  imageReader  
      = VectorImageReaderType::New();
  VectorImageType::Pointer img = VectorImageType::New();

  imageReader->SetFileName(  inputImageFile );
  try{
		imageReader->Update();
		img = imageReader->GetOutput();
  }
  catch (itk::ExceptionObject &ex){
    std::cout << ex << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "read DWIs OK." << std::endl << std::endl;

  //   parse diffusion vectors
  /**/
  itk::MetaDataDictionary imgMetaDictionary = img->GetMetaDataDictionary();
  std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
  std::vector<std::string>::iterator itKey = imgMetaKeys.begin();
  std::string metaString;

  int k = 0;
  vnl_vector_fixed<double, 3> vect3d;
  std::vector< vnl_vector_fixed<double, 3> > DiffusionVectors;

  bool Baseline = true;
  int nBaseline = 0;

  std::vector<unsigned int> baselineIndicator( 0 );
  std::vector<unsigned int> diffusionIndicator( 0 );

  for ( ; itKey != imgMetaKeys.end(); itKey ++){
		int pos;
		double x,y,z;

		ExposeMetaData(imgMetaDictionary, *itKey, metaString);
		pos = itKey->find("DWMRI_gradient");
		if (pos == -1){
		    continue;
		}

		std::cout  << metaString << std::endl;     

		std::sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
		vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
		if (x == 0 && y == 0 && z == 0)  // indicating baseline image
		{
			nBaseline ++;
			baselineIndicator.push_back( k );
			k++;
			continue;
		}

		DiffusionVectors.push_back(vect3d);
		diffusionIndicator.push_back(k);
		k++;
	}
  unsigned int nMeasurement = DiffusionVectors.size();
  std::cout << "gradient size:" << nMeasurement 
	  << "  baseline size: " << nBaseline << std::endl;

  //averaging baseline images
  VectorImageType::IndexType nrrdIdx;

  ScalarImageType::Pointer b0 = ScalarImageType::New();
  b0->CopyInformation(img);
  b0->SetRegions(b0->GetLargestPossibleRegion());
  b0->Allocate();
  itk::ImageRegionIteratorWithIndex<ScalarImageType> itB0(b0, b0->GetLargestPossibleRegion());

  for (itB0.GoToBegin(); !itB0.IsAtEnd(); ++itB0)
  {
    for (int k = 0; k < dim; k++)
      nrrdIdx[k] = itB0.GetIndex()[k];

    double b = 0;
    for (int k = 0; k < nBaseline; k++)
      b += static_cast<double> (img->GetPixel(nrrdIdx)[baselineIndicator[k]]);

    b /= static_cast<double>(nBaseline);
    itB0.Set( static_cast<float> (b) );
   }

  itk::DWIHeadMotionEddyCurrentCorrection re; 
  re.SetGradients( DiffusionVectors );
  re.SetFixedImage( b0 );

  //set baseline images
  for (int k = 0; k < nBaseline; k++){
      ScalarImageType::Pointer baseline = ScalarImageType::New();
      baseline->CopyInformation(img);
      baseline->SetRegions(baseline->GetLargestPossibleRegion());
      baseline->Allocate();
      
      itk::ImageRegionIteratorWithIndex<ScalarImageType> itBaseLine(baseline,
			baseline->GetLargestPossibleRegion());

       for (itBaseLine.GoToBegin(); !itBaseLine.IsAtEnd(); ++itBaseLine) {
	    for (int j = 0; j < dim; j++)
			nrrdIdx[j] = itBaseLine.GetIndex()[j];

	    double b 
		      = static_cast<double> (img->GetPixel(nrrdIdx)[baselineIndicator[k]]);
	    itBaseLine.Set( static_cast<float> (b) );
    }

	re.SetBaseLines( baseline );
  }

  //separate moving images
  for (int k = 0; k < nMeasurement; k++){
      ScalarImageType::Pointer dwi = ScalarImageType::New();
      dwi->CopyInformation(img);
      dwi->SetRegions(dwi->GetLargestPossibleRegion());
      dwi->Allocate();
      itk::ImageRegionIteratorWithIndex<ScalarImageType> itDWI(dwi, 
					      dwi->GetLargestPossibleRegion());

      for (itDWI.GoToBegin(); !itDWI.IsAtEnd(); ++itDWI) {
		for (int j = 0; j < dim; j++)
			nrrdIdx[j] = itDWI.GetIndex()[j];

		double b 
			= static_cast<double> (img->GetPixel(nrrdIdx)[diffusionIndicator[k]]);
		itDWI.Set( static_cast<float> (b) );
    }

	re.SetMovingImage( dwi );
  }
  
  re.SetNumberOfBins( numberOfBins );
  re.SetSamples( samples );
  re.SetTranslationScale( translationScale );
  re.SetStepLength( stepLength );
  re.SetFactor( factor );
  re.SetPrefix( "DWI_" );

 
  VectorImageType::Pointer corr = re.Registration( );
  
  itk::ImageFileWriter<VectorImageType>::Pointer VolWriter 
		= itk::ImageFileWriter<VectorImageType>::New();
  VolWriter->SetFileName( outputImageFile );
  VolWriter->SetInput(corr);
  VolWriter->Update();


  char *aryOut = new char[outputImageFile.length()+1];
  strcpy ( aryOut, outputImageFile.c_str() );

  std::ofstream header;
  header.open(aryOut, std::ios_base::app);
  
  for ( itKey=imgMetaKeys.begin(); itKey != imgMetaKeys.end(); itKey ++){
		int pos;

		ExposeMetaData(imgMetaDictionary, *itKey, metaString);
		pos = itKey->find("modality");
		if (pos == -1){
		    continue;
		}

		std::cout  << metaString << std::endl;    
		header << "modality:=" << metaString << std::endl;
  }


   
  for ( itKey=imgMetaKeys.begin(); itKey != imgMetaKeys.end(); itKey ++){
		int pos;

		ExposeMetaData(imgMetaDictionary, *itKey, metaString);
		pos = itKey->find("DWMRI_b-value");
		if (pos == -1){
		    continue;
		}

		std::cout  << metaString << std::endl;    
		header << "DWMRI_b-value:=" << metaString << std::endl;
  }


  //output baseline dir
  header << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << 0 << ":="          << 0.0 << "   " 
             << 0.0 << "   " 
             << 0.0 << std::endl;



  for (int k = 0; k < re.GetGradients().size(); k++){		
           header << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << k+1 << ":=" 
             << re.GetGradients().at(k)[0] << "   " 
             << re.GetGradients().at(k)[1] << "   " 
             << re.GetGradients().at(k)[2] << std::endl;
  }

  header.close();

  return EXIT_SUCCESS;
}

