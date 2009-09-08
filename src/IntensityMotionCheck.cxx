#include "IntensityMotionCheck.h"

#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

#include "IntraGradientRigidRegistration.h"
#include "itkExtractImageFilter.h"
#include "RigidRegistration.h"
#include "itkImageRegionIterator.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

CIntensityMotionCheck::CIntensityMotionCheck(std::string filename,std::string reportFile)
{
	DwiFileName = filename;

	if(reportFile.length()==0)
	{
		ReportFileName.clear();
	}
	else
		ReportFileName = reportFile;

	baselineNumber		= 0;
	bValueNumber		= 1;
	repetitionNumber	= 1;
	gradientDirNumber	= 0;

	DwiImage=NULL;
	bDwiLoaded=false;
	bGetGridentDirections=false;
	//bGetGridentImages=false;
}


CIntensityMotionCheck::CIntensityMotionCheck(void)
{	
	baselineNumber		= 0;
	bValueNumber		= 1;
	repetitionNumber	= 1;
	gradientDirNumber	= 0;

	DwiImage=NULL;
	bDwiLoaded=false;
	bGetGridentDirections=false;
	//bGetGridentImages=false;
}

CIntensityMotionCheck::~CIntensityMotionCheck(void)
{

}

bool CIntensityMotionCheck::LoadDwiImage()
{
	// use with windows
	//std::string str;

	//str=DwiFileName.substr(0,DwiFileName.find_last_of('\\')+1);
	//std::cout<< str<<std::endl;
	//::SetCurrentDirectory(str.c_str());
	
	itk::NrrdImageIO::Pointer  NrrdImageIO = itk::NrrdImageIO::New();

	if(DwiFileName.length()!=0)
	{
		try
		{
			DwiReader = DwiReaderType::New();
			DwiReader->SetImageIO(NrrdImageIO);
			DwiReader->SetFileName(DwiFileName);
			std::cout<< "Loading in CIntensityMotionCheck: "<<DwiFileName<<" ... ";
			DwiReader->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;
			DwiImage=NULL;
			bDwiLoaded=false;
			return false;
		}
	}
	else
	{
		std::cout<< "Dwi file name not set"<<std::endl;
		DwiImage=NULL;
		bDwiLoaded=false;
		return false;
	}


	std::cout<< "Done "<<std::endl;

	DwiImage = DwiReader->GetOutput();
	DwiImageTemp = DwiImage;

	bDwiLoaded=true;

	GetGridentDirections();

	if( bGetGridentDirections )
		collectDiffusionStatistics();
// 	else
// 	{
// 		std::cout<< "Diffusion information read error"<<std::endl;
// 		return false;
// 	}
// 	std::cout<<"Image size"<< DwiImage->GetLargestPossibleRegion().GetSize().GetSizeDimension()<<": ";
// 	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[0]<<" ";
// 	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[1]<<" ";
// 	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[2]<<std::endl;

	this->numGradients = DwiImage->GetVectorLength();
// 	std::cout<<"Pixel Vector Length: "<<DwiImage->GetVectorLength()<<std::endl;

	// Create result
	GradientIntensityMotionCheckResult result;
	result.processing = QCResult::GRADIENT_INCLUDE;

	qcResult->Clear();
	for( unsigned int j = 0; j<DwiImage->GetVectorLength(); j++ )
	{
		result.OriginalDir[0] = this->GradientDirectionContainer->ElementAt(j)[0];
		result.OriginalDir[1] = this->GradientDirectionContainer->ElementAt(j)[1];
		result.OriginalDir[2] = this->GradientDirectionContainer->ElementAt(j)[2];

		result.ReplacedDir[0] = this->GradientDirectionContainer->ElementAt(j)[0];
		result.ReplacedDir[1] = this->GradientDirectionContainer->ElementAt(j)[1];
		result.ReplacedDir[2] = this->GradientDirectionContainer->ElementAt(j)[2];

		result.CorrectedDir[0] = this->GradientDirectionContainer->ElementAt(j)[0];
		result.CorrectedDir[1] = this->GradientDirectionContainer->ElementAt(j)[1];
		result.CorrectedDir[2] = this->GradientDirectionContainer->ElementAt(j)[2];

		qcResult->GetIntensityMotionCheckResult().push_back(result);
	}
	//std::cout<<"initilize the result.OriginalDir[0] and result.CorrectedDir[0] "<<std::endl;

	return true;
}

bool CIntensityMotionCheck::GetGridentDirections()
{
	if(!bDwiLoaded ) LoadDwiImage();
	if(!bDwiLoaded )
	{
		std::cout<<"DWI load error, no Gradient Direction Loaded"<<std::endl;
		bGetGridentDirections=false;
		return false;
	}		

	itk::MetaDataDictionary imgMetaDictionary = DwiImage->GetMetaDataDictionary();    //
	std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
	std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
	std::string metaString;

	//int numberOfImages=0;
	TensorReconstructionImageFilterType::GradientDirectionType vect3d;
	
	GradientDirectionContainer = GradientDirectionContainerType::New();
	GradientDirectionContainer->clear();
	
	for ( ; itKey != imgMetaKeys.end(); itKey ++)
	{
		//double x,y,z;
		itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
		if (itKey->find("DWMRI_gradient") != std::string::npos)
		{ 
			std::istringstream iss(metaString);
			iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
			//sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
			//vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
			GradientDirectionContainer->push_back(vect3d);
		}
		else if (itKey->find("DWMRI_b-value") != std::string::npos)
		{
			readb0 = true;
			b0 = atof(metaString.c_str());
			//std::cout<<"b Value: "<<b0<<std::endl;
		}
	}

	if(!readb0)
	{
		std::cout<<"BValue not specified in header file" <<std::endl;
		return false;
	}
	if(GradientDirectionContainer->size()<=6) 
	{
		std::cout<<"Gradient Images Less than 7" <<std::endl;
		//bGetGridentDirections=false;
		return false;
	}

	bGetGridentDirections=true;
	return true;
}

bool CIntensityMotionCheck::GetGridentDirections( DwiImageType::Pointer dwi , double &bValue, GradientDirectionContainerType::Pointer	GradDireContainer )
{
	if( !dwi )
	{
		std::cout<<"DWI error, no Gradient Direction Loaded"<<std::endl;
		return false;;
	}		

	itk::MetaDataDictionary imgMetaDictionary = dwi->GetMetaDataDictionary();    //
	std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
	std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
	std::string metaString;

	//int numberOfImages=0;
	TensorReconstructionImageFilterType::GradientDirectionType vect3d;
	
	for ( ; itKey != imgMetaKeys.end(); itKey ++)
	{
		//double x,y,z;
		itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
		if (itKey->find("DWMRI_gradient") != std::string::npos)
		{ 
			std::istringstream iss(metaString);
			iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
			GradDireContainer->push_back(vect3d);
		}
		else if (itKey->find("DWMRI_b-value") != std::string::npos)
		{
			bValue = atof(metaString.c_str());			
		}
	}

	if( bValue <0 )
	{
		std::cout<<"BValue not specified in header file" <<std::endl;
		return false;
	}
	if( GradDireContainer->size()<=6) 
	{
		std::cout<<"Gradient Images Less than 7" <<std::endl;
		return false;
	}

	return true;
}

void CIntensityMotionCheck::GetImagesInformation()
{
	if(!bDwiLoaded ) LoadDwiImage();
	if(!bDwiLoaded )
	{
		std::cout<<"DWI load error, no Gradient Direction Loaded"<<std::endl;
		bGetGridentDirections=false;
		return ;
	}		


	itk::MetaDataDictionary imgMetaDictionary = DwiImage->GetMetaDataDictionary();    //
	std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
	std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
	std::string metaString;

	TensorReconstructionImageFilterType::GradientDirectionType vect3d;
	
	GradientDirectionContainerType::Pointer GradientContainer = GradientDirectionContainerType::New();
	GradientContainer->clear();

	DwiImageType::SpacingType	spacing =  DwiImage->GetSpacing();
	DwiImageType::PointType		origin	=  DwiImage->GetOrigin();
	DwiImageType::DirectionType direction = DwiImage->GetDirection();

	int space;
	
	for ( ; itKey != imgMetaKeys.end(); itKey ++)
	{
		//double x,y,z;
		itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);

		if (itKey->find("DWMRI_gradient") != std::string::npos)
		{ 
			std::istringstream iss(metaString);
			iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
			GradientContainer->push_back(vect3d);
		}
		else if (itKey->find("DWMRI_b-value") != std::string::npos)
		{
			readb0 = true;
			b0 = atof(metaString.c_str());
		}
		else if (itKey->find("space") != std::string::npos)
		{
			if( metaString.compare("right-anterior-superior") ) space = Protocal::SPACE_RAS;
			else if( metaString.compare("left-posterior-inferior") ) space = Protocal::SPACE_LPI;
			else space = Protocal::SPACE_UNKNOWN;
		}
		else if (itKey->find("modality") != std::string::npos)
		{
			if( metaString!="DWMRI")
			{
				std::cout<<"Not a DWMRI modality!"<<std::endl;
				return ;
			}
		} 
		else
		{
			;
		}
	}
}

void CIntensityMotionCheck::GenerateCheckOutputImage( std::string filename)
{

		if(!bDwiLoaded  ) LoadDwiImage();
		if(!bDwiLoaded  )
		{
			std::cout<<"DWI load error, no Gradient Direction Loaded"<<std::endl;
			bGetGridentDirections=false;
			return ;
		}

		unsigned int gradientLeft=0;
		for(unsigned int i=0;i< qcResult->GetIntensityMotionCheckResult().size();i++)
		{
			if( qcResult->GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_INCLUDE )
				gradientLeft++;
		}

		std::cout << "gradientLeft: " << gradientLeft<<std::endl;

		if( gradientLeft == 0)
		{
			//outfile<<"No gradient data left."<<std::endl;
			std::cout<<"No gradient data left."<<std::endl;
			return;
		}

		if( gradientLeft == qcResult->GetIntensityMotionCheckResult().size())
		{
			itk::NrrdImageIO::Pointer  NrrdImageIO = itk::NrrdImageIO::New();
			try
			{
				DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
				DwiWriter->SetImageIO(NrrdImageIO);
				DwiWriter->SetFileName( filename );
				DwiWriter->SetInput(DwiReader->GetOutput());
				DwiWriter->UseCompressionOn();
				DwiWriter->Update();
			}
			catch(itk::ExceptionObject & e)
			{
				std::cout<< e.GetDescription()<<std::endl;
				return ;
			}
			return;
		}

		DwiImageType::Pointer newDwiImage = DwiImageType::New(); 
		newDwiImage->CopyInformation(DwiImage);
		newDwiImage->SetRegions(DwiImage->GetLargestPossibleRegion());
		newDwiImage->SetVectorLength( gradientLeft) ; 
		//	newDwiImage->SetMetaDataDictionary(imgMetaDictionary);
		newDwiImage->Allocate();

		typedef itk::ImageRegionConstIteratorWithIndex< DwiImageType > ConstIteratorType;
		ConstIteratorType oit( DwiImage, DwiImage->GetLargestPossibleRegion() );
		typedef itk::ImageRegionIteratorWithIndex< DwiImageType > IteratorType;
		IteratorType nit( newDwiImage, newDwiImage->GetLargestPossibleRegion() );

		oit.GoToBegin();
		nit.GoToBegin();

		DwiImageType::PixelType value ;
		value.SetSize( gradientLeft ) ;

		while (!oit.IsAtEnd())
		{
			unsigned int element = 0;
			for( unsigned int i = 0 ; i < qcResult->GetIntensityMotionCheckResult().size(); i++ )
			{
				if(qcResult->GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_INCLUDE)
				{
					value.SetElement( element , oit.Get()[i] ) ;
					element++;
				}
			}
			nit.Set(value);	
			++oit;
			++nit;
		}

		itk::NrrdImageIO::Pointer  NrrdImageIO = itk::NrrdImageIO::New();
		try
		{
			DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
			DwiWriter->SetImageIO(NrrdImageIO);
			DwiWriter->SetFileName( filename );
			DwiWriter->SetInput(newDwiImage);
			DwiWriter->UseCompressionOn();
			DwiWriter->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;
			return ;
		}

		//newDwiImage->Delete();

		itk::MetaDataDictionary imgMetaDictionary = DwiImage->GetMetaDataDictionary();    //
		std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
		std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
		std::string metaString;

		char *aryOut = new char[filename.length()+1];
		strcpy ( aryOut, filename.c_str() );

		std::ofstream header;
		header.open(aryOut, std::ios_base::app);

		//  measurement frame 
		if(imgMetaDictionary.HasKey("NRRD_measurement frame"))
		{
			// measurement frame
			vnl_matrix<double> mf(3,3);
			// imaging frame
			vnl_matrix<double> imgf(3,3);
			std::vector<std::vector<double> > nrrdmf;
			itk::ExposeMetaData<std::vector<std::vector<double> > >(imgMetaDictionary,"NRRD_measurement frame",nrrdmf);

			imgf = DwiImage->GetDirection().GetVnlMatrix();

			//Image frame
			//std::cout << "Image frame: " << std::endl;
			//std::cout << imgf << std::endl;

			for(unsigned int i = 0; i < 3; ++i)
			{
				for(unsigned int j = 0; j < 3; ++j)
				{
					mf(i,j) = nrrdmf[j][i];
					nrrdmf[j][i] = imgf(i,j);
				}
			}

			// Meausurement frame
			header	<< "measurement frame: (" 
				<< mf(0,0) << "," 
				<< mf(1,0) << "," 
				<< mf(2,0) << ") ("
				<< mf(0,1) << "," 
				<< mf(1,1) << "," 
				<< mf(2,1) << ") ("
				<< mf(0,2) << "," 
				<< mf(1,2) << "," 
				<< mf(2,2) << ")"
				<< std::endl;
		}

		for ( itKey=imgMetaKeys.begin(); itKey != imgMetaKeys.end(); itKey ++){
			int pos;

			itk::ExposeMetaData(imgMetaDictionary, *itKey, metaString);
			pos = itKey->find("modality");
			if (pos == -1){
				continue;
			}

			std::cout  << metaString << std::endl;    
			header << "modality:=" << metaString << std::endl;
		}

		if(!bGetGridentDirections)
			GetGridentDirections();

		for ( itKey=imgMetaKeys.begin(); itKey != imgMetaKeys.end(); itKey ++){
			int pos;

			itk::ExposeMetaData(imgMetaDictionary, *itKey, metaString);
			pos = itKey->find("DWMRI_b-value");
			if (pos == -1){
				continue;
			}

			std::cout  << metaString << std::endl;    
			header << "DWMRI_b-value:=" << metaString << std::endl;
		}


		unsigned int temp=0;
		for(unsigned int i=0;i< GradientDirectionContainer->size();i++ )
		{
			if(qcResult->GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_INCLUDE)
			{
				header	<< "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << temp << ":=" 
					<< GradientDirectionContainer->ElementAt(i)[0] << "   " 
					<< GradientDirectionContainer->ElementAt(i)[1] << "   " 
					<< GradientDirectionContainer->ElementAt(i)[2] << std::endl;
				++temp;
			}
		}

		header.flush();
		header.close();

		std::cout<<" QC Image saved "<<std::endl;

}


bool CIntensityMotionCheck::ImageCheck( DwiImageType::Pointer dwi )
{
	//First check
	bool returnValue = true;
	bool bReport = false;
	std::string ReportFileName;

	if( protocal->GetImageProtocal().reportFileNameSuffix.length()>0 )
	{
		if( protocal->GetQCOutputDirectory().length()>0 )
		{

			if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
				protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
				ReportFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
			else
				ReportFileName = protocal->GetQCOutputDirectory();

			ReportFileName.append( "/" );

			std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
			str = str.substr( str.find_last_of("/\\")+1);

			ReportFileName.append( str );
			ReportFileName.append( protocal->GetImageProtocal().reportFileNameSuffix );	
		}
		else
		{
			ReportFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			ReportFileName.append( protocal->GetImageProtocal().reportFileNameSuffix );			
		}
	}

// 	std::cout << "DwiFileName: " << DwiFileName<<std::endl;
// 	std::cout << "ReportFileName: " << ReportFileName<<std::endl;

	std::ofstream outfile;

	if( protocal->GetImageProtocal().reportFileMode == 1 )
		outfile.open( ReportFileName.c_str(), std::ios_base::app);
	else
		outfile.open( ReportFileName.c_str());

	if( outfile ) 
		bReport = true;

	if( bReport )
	{
		outfile <<std::endl;
		outfile <<"================================"<<std::endl;
		outfile <<"  Image Information checking    "<<std::endl;
		outfile <<"================================"<<std::endl;
	}
	else
	{
		std::cout<< "Image information check report file open failed." << std::endl;
	}	

	if( !protocal->GetImageProtocal().bCheck)
	{
		std::cout<<"Image information check NOT set."<<std::endl;
		if(bReport)
			outfile<<"Image information check NOT set."<<std::endl;

		return true;
	}
	else
	{
		if( !dwi  )
		{
			std::cout<<"DWI image error."<<std::endl;
			bGetGridentDirections=false;
			return false;
		}
		//size
		if( protocal->GetImageProtocal().size[0] ==	dwi->GetLargestPossibleRegion().GetSize()[0] && 
			protocal->GetImageProtocal().size[1] ==	dwi->GetLargestPossibleRegion().GetSize()[1] && 
			protocal->GetImageProtocal().size[2] ==	dwi->GetLargestPossibleRegion().GetSize()[2] )
		{
			qcResult->GetImageInformationCheckResult().size = true;
			if(bReport)
				outfile<<"Image size Check: " << "\t\tOK"<<std::endl;
			std::cout<<"Image size Check: " << "\t\tOK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().size = false;
			if(bReport)
				outfile<<"Image size Check: " << "\t\tFAILED"<<std::endl;
			std::cout<<"Image size Check: " << "\t\tFAILED"<<std::endl;
			returnValue = false;
		}
		//origion
		if( protocal->GetImageProtocal().origin[0] ==	dwi->GetOrigin()[0]	&& 
			protocal->GetImageProtocal().origin[1] ==	dwi->GetOrigin()[1]	&& 
			protocal->GetImageProtocal().origin[2] ==	dwi->GetOrigin()[2]	 	)
		{
			qcResult->GetImageInformationCheckResult().origin = true;
			if(bReport)
				outfile<<"Image Origin Check: " << "\t\tOK"<<std::endl;
			std::cout<<"Image Origin Check: " << "\t\tOK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().origin = false;
			if(bReport)
				outfile<<"Image Origin Check: " << "\t\tFAILED"<<std::endl;
			std::cout<<"Image Origin Check: " << "\t\tFAILED"<<std::endl;
			returnValue = false;
		}
		//spacing
		//std::cout<<"spacing: "<< protocal->GetImageProtocal().spacing[0]<<" "<<protocal->GetImageProtocal().spacing[1]<<" "<<protocal->GetImageProtocal().spacing[2]<<std::endl;
		//std::cout<<"spacing: "<< DwiImage->GetSpacing()[0]<<" "<<DwiImage->GetSpacing()[1]<<" "<<DwiImage->GetSpacing()[2]<<std::endl;
		if( protocal->GetImageProtocal().spacing[0] ==	dwi->GetSpacing()[0]	&& 
			protocal->GetImageProtocal().spacing[1] ==	dwi->GetSpacing()[1]	&& 
			protocal->GetImageProtocal().spacing[2] ==	dwi->GetSpacing()[2]	 	)
		{
			qcResult->GetImageInformationCheckResult().spacing = true;
			if(bReport)
				outfile<<"Image Spacing Check: " << "\t\tOK"<<std::endl;
			std::cout<<"Image Spacing Check: " << "\t\tOK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().spacing = false;
			if(bReport)
				outfile<<"Image Spacing Check: " << "\t\tFAILED"<<std::endl;
			std::cout<<"Image Spacing Check: " << "\t\tFAILED"<<std::endl;
			returnValue = false;
		}
		// space direction
		vnl_matrix<double> imgf(3,3);
		imgf = dwi->GetDirection().GetVnlMatrix();

		if( protocal->GetImageProtocal().spacedirection[0][0] ==	imgf(0,0)	&& 
			protocal->GetImageProtocal().spacedirection[0][1] ==	imgf(0,1)	&&
			protocal->GetImageProtocal().spacedirection[0][2] ==	imgf(0,2)	&&
			protocal->GetImageProtocal().spacedirection[1][0] ==	imgf(1,0)	&&
			protocal->GetImageProtocal().spacedirection[1][1] ==	imgf(1,1)	&&
			protocal->GetImageProtocal().spacedirection[1][2] ==	imgf(1,2)	&&
			protocal->GetImageProtocal().spacedirection[2][0] ==	imgf(2,0)	&&
			protocal->GetImageProtocal().spacedirection[2][1] ==	imgf(2,1)	&&
			protocal->GetImageProtocal().spacedirection[2][2] ==	imgf(2,2)	    )		
		{
			qcResult->GetImageInformationCheckResult().spacedirection = true;
			if(bReport)
				outfile<<"Image spacedirection Check: " << "\tOK"<<std::endl;
			std::cout<<"Image spacedirection Check: " << "\tOK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().spacedirection = false;
			if(bReport)
				outfile<<"Image spacedirection Check: " << "\tFAILED"<<std::endl;
			std::cout<<"Image spacedirection Check: " << "\tFAILED"<<std::endl;
			returnValue = false;
		}

		// space
		itk::MetaDataDictionary imgMetaDictionary = dwi->GetMetaDataDictionary(); 
		std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
		std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
		std::string metaString;

		itk::ExposeMetaData<std::string> (imgMetaDictionary, "NRRD_space", metaString);

		int space;
		if(		 metaString.compare( "left-posterior-superior") ==0 )	space = Protocal::SPACE_LPS;
		else if( metaString.compare( "left-posterior-inferior") ==0 )	space = Protocal::SPACE_LPI;
		else if( metaString.compare( "left-anterior-superior" )==0 )	space = Protocal::SPACE_LAS;
		else if( metaString.compare( "left-anterior-inferior" )==0 )	space = Protocal::SPACE_LAI;
		else if( metaString.compare( "right-posterior-superior")==0 )	space = Protocal::SPACE_RPS;
		else if( metaString.compare( "right-posterior-inferior")==0 )	space = Protocal::SPACE_RPI;
		else if( metaString.compare( "right-anterior-superior" )==0 )	space = Protocal::SPACE_RAS;
		else if( metaString.compare( "right-anterior-inferior" )==0 )	space = Protocal::SPACE_RAI;
		else space = Protocal::SPACE_UNKNOWN;

		if( protocal->GetImageProtocal().space == space)
		{
			qcResult->GetImageInformationCheckResult().space = true;
			if(bReport)
				outfile<<"Image space Check: " << "\t\tOK"<<std::endl;
			std::cout<<"Image space Check: " << "\t\tOK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().space = false;
			if(bReport)
				outfile<<"Image space Check: " << "\t\tFAILED"<<std::endl;
			std::cout<<"Image space Check: " << "\t\tFAILED"<<std::endl;
			returnValue = false;
		}
	}

	if(bReport)
		outfile.close();	

	// then crop/pad
	
	if( protocal->GetImageProtocal().bCrop && (!qcResult->GetImageInformationCheckResult().size) )
	{
		int *sizePara = NULL;
		sizePara = new int[3];
		sizePara[0]	= protocal->GetImageProtocal().size[0];
		sizePara[1]	= protocal->GetImageProtocal().size[1];
		sizePara[2]	= protocal->GetImageProtocal().size[2];

		Cropper	= CropperType::New();
		Cropper->SetInput( DwiImageTemp ); 
		Cropper->SetSize(sizePara);

		if(bReport)
		{
			Cropper->SetReportFileName( ReportFileName ); //protocal->GetImageProtocal().reportFileNameSuffix);
			Cropper->SetReportFileMode( protocal->GetImageProtocal().reportFileMode);
		}
		Cropper->Update();

		DwiImageTemp = Cropper->GetOutput();

		if( protocal->GetImageProtocal().croppedDWIFileNameSuffix.length()>0 )
		{
			try
			{
				std::string CroppedFileName;
// 				CroppedFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 				CroppedFileName.append( protocal->GetImageProtocal().croppedDWIFileNameSuffix );
				if( protocal->GetImageProtocal().reportFileNameSuffix.length()>0 )
				{
					if( protocal->GetQCOutputDirectory().length()>0 )
					{

						if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
							protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
							CroppedFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
						else
							CroppedFileName = protocal->GetQCOutputDirectory();

						CroppedFileName.append( "/" );

						std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
						str = str.substr( str.find_last_of("/\\")+1);

						CroppedFileName.append( str );
						CroppedFileName.append( protocal->GetImageProtocal().croppedDWIFileNameSuffix );	
					}
					else
					{
						CroppedFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
						CroppedFileName.append( protocal->GetImageProtocal().croppedDWIFileNameSuffix );			
					}
				}

				std::cout<< "Saving cropped DWI: "<< CroppedFileName <<" ... ";

				DwiWriter->SetFileName( CroppedFileName );//protocal->GetImageProtocal().croppedDWIFileNameSuffix );
				DwiWriter->SetInput( DwiImageTemp );
				DwiWriter->UseCompressionOn();
				DwiWriter->Update();
			}
			catch(itk::ExceptionObject & e)
			{
				std::cout<< e.GetDescription()<<std::endl;
				//return -1;
			}
			std::cout<< "DONE."<< std::endl;
		}
	}
	else
	{
		DwiImageTemp = dwi;
	}

	return returnValue;
}

bool CIntensityMotionCheck::DiffusionCheck( DwiImageType::Pointer dwi)
{
	bool bReport = false;

	std::string ReportFileName;

	if( protocal->GetImageProtocal().reportFileNameSuffix.length()>0 )
	{
		if( protocal->GetQCOutputDirectory().length()>0 )
		{

			if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
				protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
				ReportFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
			else
				ReportFileName = protocal->GetQCOutputDirectory();

			ReportFileName.append( "/" );

			std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
			str = str.substr( str.find_last_of("/\\")+1);

			ReportFileName.append( str );
			ReportFileName.append( protocal->GetDiffusionProtocal().reportFileNameSuffix );	
		}
		else
		{
			ReportFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			ReportFileName.append( protocal->GetDiffusionProtocal().reportFileNameSuffix );			
		}
	}

// 	if( protocal->GetDiffusionProtocal().reportFileNameSuffix.length()>0 )
// 	{
// 		ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 		ReportFileName.append( protocal->GetDiffusionProtocal().reportFileNameSuffix );			
// 	}

	std::ofstream outfile;
	if( protocal->GetImageProtocal().reportFileMode == 1 )
		outfile.open( ReportFileName.c_str(), std::ios_base::app);
	else
		outfile.open( ReportFileName.c_str());
	if(outfile) 
		bReport = true;

	if(bReport) 
	{
		outfile <<std::endl;
		outfile <<"================================"<<std::endl;
		outfile <<" Diffusion Information checking "<<std::endl;
		outfile <<"================================"<<std::endl;
	}

	bool returnValte = true;

	if( !protocal->GetDiffusionProtocal().bCheck)
	{
		if(bReport)
			outfile<<"Diffusion information check NOT set."<<std::endl;
		std::cout<<"Diffusion information check NOT set."<<std::endl;
		return true;
	}
	else
	{
		if( !dwi )
		{
			std::cout<<"DWI error."<<std::endl;
			bGetGridentDirections=false;
			return false;
		}

		GradientDirectionContainerType::Pointer	GradContainer = GradientDirectionContainerType::New();
		double bValue;
		this->GetGridentDirections( dwi,bValue, GradContainer);

		if( fabs(protocal->GetDiffusionProtocal().bValue - bValue) < 0.0000001 )
		{
			qcResult->GetDiffusionInformationCheckResult().b = true;
			if(bReport)
				outfile<<"DWMRI_bValue Check: " << "\t\t\tOK"<<std::endl;
			std::cout<<"DWMRI_bValue check: " << "\t\t\tOK"<<std::endl;
		}
		else
		{
			qcResult->GetDiffusionInformationCheckResult().b = false;
			if(bReport)
			{
				outfile<<"Diffusion b-value Check:\t\tFAILED" <<std::endl;
				outfile<<"DWMRI_bValue\t\tmismatch with DWI = " << this->b0 
					<< "\t\t\t\tprotocol = " << protocal->GetDiffusionProtocal().bValue <<std::endl;
			}
			std::cout<<"Diffusion b-value Check:\t\tFAILED" <<std::endl;
			// 			std::cout <<"DWMRI_bValue\t\tmismatch with DWI = " << this->b0 
			// 				<< "\t\t\t\tprotocol = " << protocal->GetDiffusionProtocal().bValue <<std::endl;

			if( protocal->GetDiffusionProtocal().bUseDiffusionProtocal )
			{
				std::ostringstream ossMetaString;
				ossMetaString << protocal->GetDiffusionProtocal().bValue;
				itk::EncapsulateMetaData<std::string>( dwi->GetMetaDataDictionary(), "DWMRI_b-value", ossMetaString.str());
			}
			returnValte = false;
		}

		bool result = true;
		if( GradContainer->size() != protocal->GetDiffusionProtocal().gradients.size())
		{
			qcResult->GetDiffusionInformationCheckResult().gradient = false;
			if(bReport)
				outfile <<"Diffusion vector # mismatch with protocol = "
				<< protocal->GetDiffusionProtocal().gradients.size()
				<< " image = " << GradContainer->size() <<std::endl;

			std::cout	<<"Diffusion vector # mismatch with protocol = "
				<< protocal->GetDiffusionProtocal().gradients.size()
				<< " image = " << GradContainer->size() <<std::endl;

			result = false;
		}
		else // Diffusion vector # matched
		{
			qcResult->GetDiffusionInformationCheckResult().gradient = true;
			for(unsigned int i=0; i< GradContainer->size();i++)
			{
				if( fabs(protocal->GetDiffusionProtocal().gradients[i][0] - GradContainer->ElementAt(i)[0]) < 0.00001 &&
					fabs(protocal->GetDiffusionProtocal().gradients[i][1] - GradContainer->ElementAt(i)[1]) < 0.00001 &&
					fabs(protocal->GetDiffusionProtocal().gradients[i][2] - GradContainer->ElementAt(i)[2]) < 0.00001 )			
				{
					result = result && true;
				}
				else
				{
					if(bReport)
					{
						outfile	<<"DWMRI_gradient_" << std::setw(4) << std::setfill('0') << i<< "\tmismatch with DWI = [ " 
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)<< GradContainer->ElementAt(i)[0]<< " "
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)<< GradContainer->ElementAt(i)[1]<< " "
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)<< GradContainer->ElementAt(i)[2]<< " ] \tprotocol = [ "
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)<< protocal->GetDiffusionProtocal().gradients[i][0]<< " "
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)<< protocal->GetDiffusionProtocal().gradients[i][1]<< " "
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)<< protocal->GetDiffusionProtocal().gradients[i][2]<< " ]" << std::endl;
					}

					if( protocal->GetDiffusionProtocal().bUseDiffusionProtocal )
					{
						std::ostringstream ossMetaString, ossMetaKey;
						ossMetaKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << i;
						ossMetaString << std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
							<< protocal->GetDiffusionProtocal().gradients[i][0] << "    " 
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
							<< protocal->GetDiffusionProtocal().gradients[i][1] << "    " 
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
							<< protocal->GetDiffusionProtocal().gradients[i][2] ;

						itk::EncapsulateMetaData<std::string>( dwi->GetMetaDataDictionary(), ossMetaKey.str(),  ossMetaString.str());


						qcResult->GetIntensityMotionCheckResult()[i].ReplacedDir[0] =  protocal->GetDiffusionProtocal().gradients[i][0];
						qcResult->GetIntensityMotionCheckResult()[i].ReplacedDir[1] =  protocal->GetDiffusionProtocal().gradients[i][1];
						qcResult->GetIntensityMotionCheckResult()[i].ReplacedDir[2] =  protocal->GetDiffusionProtocal().gradients[i][2];

						qcResult->GetIntensityMotionCheckResult()[i].CorrectedDir[0] = protocal->GetDiffusionProtocal().gradients[i][0];
						qcResult->GetIntensityMotionCheckResult()[i].CorrectedDir[1] = protocal->GetDiffusionProtocal().gradients[i][1];
						qcResult->GetIntensityMotionCheckResult()[i].CorrectedDir[2] = protocal->GetDiffusionProtocal().gradients[i][2];
					}

					qcResult->GetDiffusionInformationCheckResult().gradient = false;
					result = false;				
				}
			}
		}

		if( result )
		{
			qcResult->GetDiffusionInformationCheckResult().gradient = true;
			if(bReport)
				outfile<<"Diffusion gradient Check: \t\tOK"<<std::endl;
			std::cout<<"Diffusion gradient Check: \t\tOK"<<std::endl;
		}
		else
		{
			qcResult->GetDiffusionInformationCheckResult().gradient = false;
			if(bReport)
				outfile<<"Diffusion gradient Check: \t\tFAILED"<<std::endl;
			std::cout<<"Diffusion gradient Check: \t\tFAILED"<<std::endl;	
		}

		returnValte = returnValte && result;

		//  measurement frame 
		if( DwiImageTemp->GetMetaDataDictionary().HasKey("NRRD_measurement frame"))
		{
			// measurement frame
			vnl_matrix<double> mf(3,3);
			// imaging frame
			vnl_matrix<double> imgf(3,3);
			std::vector<std::vector<double> > nrrdmf;
			itk::ExposeMetaData<std::vector<std::vector<double> > >( dwi->GetMetaDataDictionary(),"NRRD_measurement frame",nrrdmf);

			imgf = DwiImageTemp->GetDirection().GetVnlMatrix();

			//Image frame
			for(unsigned int i = 0; i < 3; ++i)
			{
				for(unsigned int j = 0; j < 3; ++j)
				{
					mf(i,j) = nrrdmf[j][i];
					nrrdmf[j][i] = imgf(i,j);
				}
			}

			// Meausurement frame
			if( 
				protocal->GetDiffusionProtocal().measurementFrame[0][0] == mf(0,0) &&
				protocal->GetDiffusionProtocal().measurementFrame[0][1] == mf(0,1) &&
				protocal->GetDiffusionProtocal().measurementFrame[0][2] == mf(0,2) &&
				protocal->GetDiffusionProtocal().measurementFrame[1][0] == mf(1,0) &&
				protocal->GetDiffusionProtocal().measurementFrame[1][1] == mf(1,1) &&
				protocal->GetDiffusionProtocal().measurementFrame[1][2] == mf(1,2) &&
				protocal->GetDiffusionProtocal().measurementFrame[2][0] == mf(2,0) &&
				protocal->GetDiffusionProtocal().measurementFrame[2][1] == mf(2,1) &&
				protocal->GetDiffusionProtocal().measurementFrame[2][2] == mf(2,2)		)
			{
				qcResult->GetDiffusionInformationCheckResult().measurementFrame = true;
				if(bReport)
					outfile<<"Diffusion measurementFrame Check: \tOK"<<std::endl;
				std::cout<<"Diffusion measurementFrame Check: \tOK"<<std::endl;
			}
			else
			{
				returnValte = false;
				qcResult->GetDiffusionInformationCheckResult().measurementFrame = false;
				if(bReport)
					outfile<<"Diffusion measurementFrame Check: \tFAILED"<<std::endl;
				std::cout<<"Diffusion measurementFrame Check: \tFAILED"<<std::endl;

				if( protocal->GetDiffusionProtocal().bUseDiffusionProtocal )
				{
					nrrdmf[0][0] = protocal->GetDiffusionProtocal().measurementFrame[0][0];
					nrrdmf[0][1] = protocal->GetDiffusionProtocal().measurementFrame[0][1];
					nrrdmf[0][2] = protocal->GetDiffusionProtocal().measurementFrame[0][2] ;
					nrrdmf[1][0] = protocal->GetDiffusionProtocal().measurementFrame[1][0] ;
					nrrdmf[1][1] = protocal->GetDiffusionProtocal().measurementFrame[1][1] ;
					nrrdmf[1][2] = protocal->GetDiffusionProtocal().measurementFrame[1][2] ;
					nrrdmf[2][0] = protocal->GetDiffusionProtocal().measurementFrame[2][0] ;
					nrrdmf[2][1] = protocal->GetDiffusionProtocal().measurementFrame[2][1] ;
					nrrdmf[2][2] = protocal->GetDiffusionProtocal().measurementFrame[2][2] ;	
					itk::EncapsulateMetaData<std::vector<std::vector<double> > >( dwi->GetMetaDataDictionary(),"NRRD_measurement frame",nrrdmf);
				}
			}
		}

		if( !returnValte )
		{
			std::cout << "Mismatched informations was replaced with that from protocol." << std::endl;
			if(bReport)
				outfile << "Mismatched informations was replaced with that from protocol." << std::endl;
		}
	}

	if( bReport )
		outfile.close();

	// then save the updated DWI
	if( protocal->GetDiffusionProtocal().diffusionReplacedDWIFileNameSuffix.length()>0 && 
		( !qcResult->GetDiffusionInformationCheckResult().b || !qcResult->GetDiffusionInformationCheckResult().gradient || !qcResult->GetDiffusionInformationCheckResult().measurementFrame) )
	{
		std::string DWIFileName;
// 		DWIFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 		DWIFileName.append( protocal->GetDiffusionProtocal().diffusionReplacedDWIFileNameSuffix );		

		if( protocal->GetQCOutputDirectory().length()>0 )
		{

			if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
				protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
				DWIFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
			else
				DWIFileName = protocal->GetQCOutputDirectory();

			DWIFileName.append( "/" );

			std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
			str = str.substr( str.find_last_of("/\\")+1);

			DWIFileName.append( str );
			DWIFileName.append( protocal->GetDiffusionProtocal().diffusionReplacedDWIFileNameSuffix );	
		}
		else
		{
			DWIFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			DWIFileName.append( protocal->GetDiffusionProtocal().diffusionReplacedDWIFileNameSuffix );			
		}

		try
		{
			std::cout<< "Saving diffusion information updated DWI: "<< DWIFileName << "...";
			DwiWriter->SetFileName( DWIFileName );
			DwiWriter->SetInput( DwiImageTemp);			
			DwiWriter->UseCompressionOn();
			DwiWriter->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;
			//return false;
		}
		std::cout<< "DONE"<<std::endl;
	}

	return returnValte;
}



bool CIntensityMotionCheck::SliceWiseCheck( DwiImageType::Pointer dwi )
{
	if( protocal->GetSliceCheckProtocal().bCheck )
	{
		std::string ReportFileName;
		if( protocal->GetSliceCheckProtocal().reportFileNameSuffix.length()>0 )
		{
			if( protocal->GetQCOutputDirectory().length()>0 )
			{

				if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
					protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
					ReportFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
				else
					ReportFileName = protocal->GetQCOutputDirectory();

				ReportFileName.append( "/" );

				std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
				str = str.substr( str.find_last_of("/\\")+1);

				ReportFileName.append( str );
				ReportFileName.append( protocal->GetSliceCheckProtocal().reportFileNameSuffix );	
			}
			else
			{
				ReportFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
				ReportFileName.append( protocal->GetSliceCheckProtocal().reportFileNameSuffix );			
			}

// 			ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 			ReportFileName.append( protocal->GetSliceCheckProtocal().reportFileNameSuffix );			
		}

		SliceChecker = SliceCheckerType::New();
		SliceChecker->SetInput( dwi );
		SliceChecker->SetCheckTimes( protocal->GetSliceCheckProtocal().checkTimes );
		SliceChecker->SetHeadSkipRatio( protocal->GetSliceCheckProtocal().headSkipSlicePercentage );
		SliceChecker->SetTailSkipRatio( protocal->GetSliceCheckProtocal().tailSkipSlicePercentage );
		SliceChecker->SetBaselineStdevTimes( protocal->GetSliceCheckProtocal().correlationDeviationThresholdbaseline );
		SliceChecker->SetGradientStdevTimes( protocal->GetSliceCheckProtocal().correlationDeviationThresholdgradient );
		SliceChecker->SetReportFileName( ReportFileName );
		SliceChecker->SetReportFileMode( protocal->GetSliceCheckProtocal().reportFileMode );

		try
		{
			SliceChecker->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;			
		}

		DwiImageTemp = SliceChecker->GetOutput();

		for( unsigned int i=0; i< SliceChecker->GetGradientDirectionContainer()->size(); i++)
		{
			for( unsigned int j=0; j< this->qcResult->GetIntensityMotionCheckResult().size(); j++)
			{
				if( SliceChecker->GetGradientDirectionContainer()->at(i)[0] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0] &&
					SliceChecker->GetGradientDirectionContainer()->at(i)[1] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1] &&
					SliceChecker->GetGradientDirectionContainer()->at(i)[2] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
				{
					if( this->qcResult->GetIntensityMotionCheckResult()[j].processing != QCResult::GRADIENT_INCLUDE)
					{
						std::cout<< "gradient " << i << "has been excluded!" <<std::endl;

					}
					else
					{
						if(	!SliceChecker->getQCResults()[i])
							this->qcResult->GetIntensityMotionCheckResult()[i].processing = QCResult::GRADIENT_EXCLUDE_SLICECHECK;
					}
				}
			}
		}


		if( protocal->GetSliceCheckProtocal().outputDWIFileNameSuffix.length()>0 )
		{
			std::string SliceWiseOutput;
// 			SliceWiseOutput=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 			SliceWiseOutput.append( protocal->GetSliceCheckProtocal().outputDWIFileNameSuffix );		
			if( protocal->GetQCOutputDirectory().length()>0 )
			{

				if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
					protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
					SliceWiseOutput = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
				else
					SliceWiseOutput = protocal->GetQCOutputDirectory();

				SliceWiseOutput.append( "/" );

				std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
				str = str.substr( str.find_last_of("/\\")+1);

				SliceWiseOutput.append( str );
				SliceWiseOutput.append( protocal->GetSliceCheckProtocal().outputDWIFileNameSuffix );	
			}
			else
			{
				SliceWiseOutput = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
				SliceWiseOutput.append( protocal->GetSliceCheckProtocal().outputDWIFileNameSuffix );			
			}

			try
			{
				std::cout<< "Saving output of slice check: "<< SliceWiseOutput <<" ... ";
				DwiWriter->SetImageIO(NrrdImageIO);
				DwiWriter->SetFileName( SliceWiseOutput );
				DwiWriter->SetInput( DwiImageTemp );
				DwiWriter->UseCompressionOn();
				DwiWriter->Update();
			}
			catch(itk::ExceptionObject & e)
			{
				std::cout<< e.GetDescription()<<std::endl;
			}
			std::cout<< "DONE"<<std::endl;
		}
	}
	else
	{
		std::cout<<"Slice-wise check NOT set."<<std::endl;
	}

	return true;
}

bool CIntensityMotionCheck::InterlaceWiseCheck( DwiImageType::Pointer dwi )
{
	if( protocal->GetInterlaceCheckProtocal().bCheck )
	{
		std::string ReportFileName;
		if( protocal->GetInterlaceCheckProtocal().reportFileNameSuffix.length()>0 )
		{
			if( protocal->GetQCOutputDirectory().length()>0 )
			{

				if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
					protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
					ReportFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
				else
					ReportFileName = protocal->GetQCOutputDirectory();

				ReportFileName.append( "/" );

				std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
				str = str.substr( str.find_last_of("/\\")+1);

				ReportFileName.append( str );
				ReportFileName.append( protocal->GetInterlaceCheckProtocal().reportFileNameSuffix );	
			}
			else
			{
				ReportFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
				ReportFileName.append( protocal->GetInterlaceCheckProtocal().reportFileNameSuffix );			
			}

// 			ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 			ReportFileName.append( protocal->GetInterlaceCheckProtocal().reportFileNameSuffix );			
		}

		InterlaceChecker = InterlaceCheckerType::New();
		InterlaceChecker->SetInput( dwi );
		InterlaceChecker->SetCorrelationThresholdBaseline( protocal->GetInterlaceCheckProtocal().correlationThresholdBaseline );
		InterlaceChecker->SetCorrelationThresholdGradient( protocal->GetInterlaceCheckProtocal().correlationThresholdGradient );
		InterlaceChecker->SetCorrelationStedvTimesBaseline( protocal->GetInterlaceCheckProtocal().correlationDeviationBaseline );
		InterlaceChecker->SetCorrelationStdevTimesGradient( protocal->GetInterlaceCheckProtocal().correlationDeviationGradient );
		InterlaceChecker->SetTranslationThreshold( protocal->GetInterlaceCheckProtocal().translationThreshold );
		InterlaceChecker->SetRotationThreshold( protocal->GetInterlaceCheckProtocal().rotationThreshold );
		InterlaceChecker->SetReportFileName( ReportFileName );
		InterlaceChecker->SetReportFileMode( protocal->GetInterlaceCheckProtocal().reportFileMode );

		try
		{
			InterlaceChecker->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;
			return -1;
		}

		DwiImageTemp = InterlaceChecker->GetOutput();

		for( unsigned int i=0; i< InterlaceChecker->GetGradientDirectionContainer()->size(); i++)
		{
			for( unsigned int j=0; j< this->qcResult->GetIntensityMotionCheckResult().size(); j++)
			{
				if( InterlaceChecker->GetGradientDirectionContainer()->at(i)[0] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0] &&
					InterlaceChecker->GetGradientDirectionContainer()->at(i)[1] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1] &&
					InterlaceChecker->GetGradientDirectionContainer()->at(i)[2] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
				{
					if( this->qcResult->GetIntensityMotionCheckResult()[j].processing != QCResult::GRADIENT_INCLUDE)
					{
						std::cout<< "gradient " << i << "has been excluded!" <<std::endl;

					}
					else
					{
						if(	!InterlaceChecker->getQCResults()[i])
							this->qcResult->GetIntensityMotionCheckResult()[i].processing = QCResult::GRADIENT_EXCLUDE_INTERLACECHECK;
					}
				}
			}
		}

// 		for( unsigned int i=0; i<this->qcResult->GetIntensityMotionCheckResult().size(); i++)
// 		{
// 			if(	!InterlaceChecker->getQCResults()[i])
// 				this->qcResult->GetIntensityMotionCheckResult()[i].processing = QCResult::GRADIENT_EXCLUDE_INTERLACECHECK;
// 		}


		if( protocal->GetInterlaceCheckProtocal().outputDWIFileNameSuffix.length()>0 )
		{
			std::string outputDWIFileName;
// 			outputDWIFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 			outputDWIFileName.append( protocal->GetInterlaceCheckProtocal().outputDWIFileNameSuffix );		
			if( protocal->GetQCOutputDirectory().length()>0 )
			{

				if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
					protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
					outputDWIFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
				else
					outputDWIFileName = protocal->GetQCOutputDirectory();

				outputDWIFileName.append( "/" );

				std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
				str = str.substr( str.find_last_of("/\\")+1);

				outputDWIFileName.append( str );
				outputDWIFileName.append( protocal->GetInterlaceCheckProtocal().outputDWIFileNameSuffix );	
			}
			else
			{
				outputDWIFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
				outputDWIFileName.append( protocal->GetInterlaceCheckProtocal().outputDWIFileNameSuffix );			
			}
			try
			{
				std::cout<< "Saving output of interlace check: "<< outputDWIFileName <<" ... ";
				DwiWriter->SetImageIO(NrrdImageIO);
				DwiWriter->SetFileName( outputDWIFileName );
				DwiWriter->SetInput( DwiImageTemp );
				DwiWriter->UseCompressionOn();
				DwiWriter->Update();
			}
			catch(itk::ExceptionObject & e)
			{
				std::cout<< e.GetDescription()<<std::endl;
				return -1;
			}
			std::cout<< "DONE"<<std::endl;
		}
	}
	else
	{
		std::cout<<"Interlace-wise check NOT set."<<std::endl;
	}

	return true;
}

bool CIntensityMotionCheck::BaselineAverage( DwiImageType::Pointer dwi )
{
	if( protocal->GetBaselineAverageProtocal().bAverage )
	{
		std::string ReportFileName;
		if( protocal->GetBaselineAverageProtocal().reportFileNameSuffix.length()>0 )
		{
// 			ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 			ReportFileName.append( protocal->GetBaselineAverageProtocal().reportFileNameSuffix );		
			if( protocal->GetQCOutputDirectory().length()>0 )
			{

				if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
					protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
					ReportFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
				else
					ReportFileName = protocal->GetQCOutputDirectory();

				ReportFileName.append( "/" );

				std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
				str = str.substr( str.find_last_of("/\\")+1);

				ReportFileName.append( str );
				ReportFileName.append( protocal->GetBaselineAverageProtocal().reportFileNameSuffix );	
			}
			else
			{
				ReportFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
				ReportFileName.append( protocal->GetBaselineAverageProtocal().reportFileNameSuffix );			
			}
		}

		BaselineAverager	= BaselineAveragerType::New();

		BaselineAverager->SetInput( dwi );
		BaselineAverager->SetReportFileName( ReportFileName );
		BaselineAverager->SetReportFileMode(protocal->GetBaselineAverageProtocal().reportFileMode);
		BaselineAverager->SetAverageMethod( protocal->GetBaselineAverageProtocal().averageMethod );
		BaselineAverager->SetStopThreshold( protocal->GetBaselineAverageProtocal().stopThreshold );
		BaselineAverager->SetMaxIteration( 500 );

		try
		{
			BaselineAverager->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;
			return -1;
		}

		DwiImageTemp = BaselineAverager->GetOutput();

		for( unsigned int j=0; j< this->qcResult->GetIntensityMotionCheckResult().size(); j++)
		{
			if( 0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0] &&
				0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1] &&
				0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
			{
				if( this->qcResult->GetIntensityMotionCheckResult()[j].processing == QCResult::GRADIENT_INCLUDE)
				{
					this->qcResult->GetIntensityMotionCheckResult()[j].processing = QCResult::GRADIENT_BASELINE_AVERAGED;
				}
			}
		}

		if( protocal->GetBaselineAverageProtocal().outputDWIFileNameSuffix.length()>0 )
		{
			std::string outputDWIFileName;
// 			outputDWIFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 			outputDWIFileName.append( protocal->GetBaselineAverageProtocal().outputDWIFileNameSuffix );			
			if( protocal->GetQCOutputDirectory().length()>0 )
			{

				if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
					protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
					outputDWIFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
				else
					outputDWIFileName = protocal->GetQCOutputDirectory();

				outputDWIFileName.append( "/" );

				std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
				str = str.substr( str.find_last_of("/\\")+1);

				outputDWIFileName.append( str );
				outputDWIFileName.append( protocal->GetBaselineAverageProtocal().outputDWIFileNameSuffix );	
			}
			else
			{
				outputDWIFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
				outputDWIFileName.append( protocal->GetBaselineAverageProtocal().outputDWIFileNameSuffix );			
			}

			try
			{
				std::cout<< "Saving output of baseline average: "<< outputDWIFileName <<" ... ";
				DwiWriter->SetImageIO(NrrdImageIO);
				DwiWriter->SetFileName( outputDWIFileName );
				DwiWriter->SetInput( DwiImageTemp );
				DwiWriter->UseCompressionOn();
				DwiWriter->Update();
			}
			catch(itk::ExceptionObject & e)
			{
				std::cout<< e.GetDescription()<<std::endl;
				return -1;
			}
			std::cout<< "DONE"<<std::endl;
		}
	}
	else
	{
		std::cout<<"Baseline average NOT set."<<std::endl;
	}

	return true;
}

bool CIntensityMotionCheck::EddyMotionCorrectIowa( DwiImageType::Pointer dwi )
{
	if( protocal->GetEddyMotionCorrectionProtocal().bCorrect )
	{
		std::cout<<"Eddy-current and head motion correction using IOWA tool."<<std::endl;

		std::string ReportFileName;
		if( protocal->GetEddyMotionCorrectionProtocal().reportFileNameSuffix.length()>0 )
		{
// 			ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 			ReportFileName.append( protocal->GetEddyMotionCorrectionProtocal().reportFileNameSuffix );	
			if( protocal->GetQCOutputDirectory().length()>0 )
			{
				if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
					protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
					ReportFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
				else
					ReportFileName = protocal->GetQCOutputDirectory();

				ReportFileName.append( "/" );

				std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
				str = str.substr( str.find_last_of("/\\")+1);

				ReportFileName.append( str );
				ReportFileName.append( protocal->GetEddyMotionCorrectionProtocal().reportFileNameSuffix );	
			}
			else
			{
				ReportFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
				ReportFileName.append( protocal->GetEddyMotionCorrectionProtocal().reportFileNameSuffix );			
			}
		}

//		std::cout<<"ReportFileName: "<< ReportFileName <<std::endl;


		// get the inputgradDir 
		itk::MetaDataDictionary imgMetaDictionary = dwi->GetMetaDataDictionary();    //
		std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
		std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
		std::string metaString;
		TensorReconstructionImageFilterType::GradientDirectionType vect3d;

		GradientDirectionContainerType::Pointer inputGradDirContainer = GradientDirectionContainerType::New();
		inputGradDirContainer->clear();

		int baselineNumber = 0;

		for ( ; itKey != imgMetaKeys.end(); itKey ++)
		{
			itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
			if (itKey->find("DWMRI_gradient") != std::string::npos)
			{ 
				std::istringstream iss(metaString);
				iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
				inputGradDirContainer->push_back(vect3d);
				if( vect3d[0]==0.0 && vect3d[1]==0.0 && vect3d[2]==0.0 )
					baselineNumber ++;
			}
		}

//		std::cout<<"baselineNumber: "<< baselineNumber <<std::endl;

		// filtering for fixed image
		typedef itk::Image<signed short,3>  GradientImageType;

		EddyMotionCorrectorIowa = EddyMotionCorrectorTypeIowa::New();
		EddyMotionCorrectorIowa->SetInput( dwi );

		// find the gradient with the smallest b-value
		int smallestGradient = -1;
		if( baselineNumber == 0 )
		{
			double smallestBValue = 9999999999999999.0;
			for( int i = 0; i< inputGradDirContainer->size(); i++ )
			{
				double temp;
				temp =  inputGradDirContainer->ElementAt(i)[0]*inputGradDirContainer->ElementAt(i)[0] +
						inputGradDirContainer->ElementAt(i)[1]*inputGradDirContainer->ElementAt(i)[1] +
						inputGradDirContainer->ElementAt(i)[2]*inputGradDirContainer->ElementAt(i)[2]  ;
				if( temp < smallestBValue )
				{
					smallestBValue = temp;
					smallestGradient = i;
				}
			}

//			std::cout<<"smallestGradient: "<< smallestGradient <<std::endl;

			typedef itk::VectorIndexSelectionCastImageFilter< DwiImageType, GradientImageType > FilterType;
			FilterType::Pointer componentExtractor = FilterType::New();

			componentExtractor->SetInput( dwi);
			componentExtractor->SetIndex( smallestGradient );
			componentExtractor->Update();

			EddyMotionCorrectorIowa->SetFixedImage(componentExtractor->GetOutput());
		}
		else
		{
			GradientImageType::Pointer fixed = GradientImageType::New();

			fixed->CopyInformation( dwi );
			fixed->SetRegions( dwi->GetLargestPossibleRegion());
			fixed->Allocate();
			fixed->FillBuffer(0);

			typedef itk::ImageRegionIteratorWithIndex< GradientImageType > averagedBaselineIterator;
			averagedBaselineIterator aIt(fixed, fixed->GetLargestPossibleRegion());
			aIt.GoToBegin();

			typedef DwiImageType::ConstPointer  dwiImageConstPointer;
			dwiImageConstPointer  inputPtr = static_cast<dwiImageConstPointer> (dwi);
			GradientImageType::IndexType pixelIndex;
			double pixelValue ;

			while ( !aIt.IsAtEnd() ) 
			{
				// determine the index of the output pixel
				pixelIndex = aIt.GetIndex();
				pixelValue = 0.0;
				for( int i = 0 ; i < inputPtr->GetVectorLength(); i++ )
				{
					if( inputGradDirContainer->ElementAt(i)[0] == 0.0 &&
						inputGradDirContainer->ElementAt(i)[1] == 0.0 && 
						inputGradDirContainer->ElementAt(i)[2] == 0.0     )
					{
						pixelValue += inputPtr->GetPixel(pixelIndex)[i]/(static_cast<double>(baselineNumber));					
					}
				}
				aIt.Set( static_cast<unsigned short>(pixelValue) );
				++aIt;
			}		
			EddyMotionCorrectorIowa->SetFixedImage(fixed);
		}

// 		EddyMotionCorrectorIowa->SetNumberOfBins(protocal->GetEddyMotionCorrectionProtocal().numberOfBins );
// 		EddyMotionCorrectorIowa->SetSamples( protocal->GetEddyMotionCorrectionProtocal().numberOfSamples );
// 		EddyMotionCorrectorIowa->SetTranslationScale( protocal->GetEddyMotionCorrectionProtocal().translationScale );
// 		EddyMotionCorrectorIowa->SetStepLength(protocal->GetEddyMotionCorrectionProtocal().stepLength );
// 		EddyMotionCorrectorIowa->SetFactor( protocal->GetEddyMotionCorrectionProtocal().relaxFactor );
// 		EddyMotionCorrectorIowa->SetMaxNumberOfIterations( protocal->GetEddyMotionCorrectionProtocal().maxNumberOfIterations );
 
// 		float m_TranslationScale;
// 		float m_MaximumStepLength;
// 		float m_MinimumStepLength;
// 		float m_RelaxationFactor;
// 		int   m_NumberOfSpatialSamples;
// 		int   m_NumberOfIterations;
// 		std::string m_OutputParameterFile;

		try
		{
			EddyMotionCorrectorIowa->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;			
		}

		this->DwiImageTemp = EddyMotionCorrectorIowa->GetOutput();

// read the meta info from output and fill in qcResult
		imgMetaDictionary = EddyMotionCorrectorIowa->GetOutput()->GetMetaDataDictionary();

		GradientDirectionContainerType::Pointer GradDirContainer = GradientDirectionContainerType::New();
		GradientDirectionContainer->clear();

		itKey = imgMetaKeys.begin();
		for ( ; itKey != imgMetaKeys.end(); itKey ++)
		{
			itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
			if (itKey->find("DWMRI_gradient") != std::string::npos)
			{ 
				std::istringstream iss(metaString);
				iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
				GradDirContainer->push_back(vect3d);
			}
		}

		for( unsigned int i=0; i< GradDirContainer->size(); i++)
		{
			for( unsigned int j=0; j< this->qcResult->GetIntensityMotionCheckResult().size(); j++)
			{
				if( 0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0] &&
					0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1] &&
					0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
				{
					this->qcResult->GetIntensityMotionCheckResult()[j].processing = QCResult::GRADIENT_BASELINE_AVERAGED;
					continue;
				}

				if( GradDirContainer->at(i)[0] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0] &&
					GradDirContainer->at(i)[1] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1] &&
					GradDirContainer->at(i)[2] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
				{
					if( this->qcResult->GetIntensityMotionCheckResult()[j].processing > QCResult::GRADIENT_EDDY_MOTION_CORRECTED) //GRADIENT_EXCLUDE_SLICECHECK,
																														//GRADIENT_EXCLUDE_INTERLACECHECK,
																														//GRADIENT_EXCLUDE_GRADIENTCHECK,
																														//GRADIENT_EXCLUDE_MANUALLY,
					{
						std::cout<< "gradient " << i << "has been excluded!" <<std::endl;
					}
					else
					{
						this->qcResult->GetIntensityMotionCheckResult()[j].processing = QCResult::GRADIENT_EDDY_MOTION_CORRECTED;
						this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[0] = GradDirContainer->at(i)[0];
						this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[1] = GradDirContainer->at(i)[1];
						this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[2] = GradDirContainer->at(i)[2];
					}
				}
			}
		}

		if( protocal->GetGradientCheckProtocal().outputDWIFileNameSuffix.length()>0 )
		{
			std::string outputDWIFileName;
// 			outputDWIFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 			outputDWIFileName.append( protocal->GetEddyMotionCorrectionProtocal().outputDWIFileNameSuffix );	
			if( protocal->GetQCOutputDirectory().length()>0 )
			{

				if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
					protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
					outputDWIFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
				else
					outputDWIFileName = protocal->GetQCOutputDirectory();

				outputDWIFileName.append( "/" );

				std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
				str = str.substr( str.find_last_of("/\\")+1);

				outputDWIFileName.append( str );
				outputDWIFileName.append( protocal->GetEddyMotionCorrectionProtocal().outputDWIFileNameSuffix );	
			}
			else
			{
				outputDWIFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
				outputDWIFileName.append( protocal->GetEddyMotionCorrectionProtocal().outputDWIFileNameSuffix );			
			}

			try
			{
				std::cout<< "Saving output of eddy current motion correction: "<< outputDWIFileName <<" ... ";
				DwiWriter->SetImageIO(NrrdImageIO);
				DwiWriter->SetFileName( outputDWIFileName );
				DwiWriter->SetInput( DwiImageTemp);
				DwiWriter->UseCompressionOn();
				DwiWriter->Update();
			}
			catch(itk::ExceptionObject & e)
			{
				std::cout<< e.GetDescription()<<std::endl;
			}
			std::cout<< "DONE"<<std::endl;
		}
	}
	else
	{
		std::cout<<"Eddy-current and head motion correction NOT set."<<std::endl;
	}
	return true;
}



bool CIntensityMotionCheck::EddyMotionCorrect( DwiImageType::Pointer dwi )
{
	if( protocal->GetEddyMotionCorrectionProtocal().bCorrect )
	{
		std::string ReportFileName;
		if( protocal->GetEddyMotionCorrectionProtocal().reportFileNameSuffix.length()>0 )
		{
// 			ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 			ReportFileName.append( protocal->GetEddyMotionCorrectionProtocal().reportFileNameSuffix );	
			if( protocal->GetQCOutputDirectory().length()>0 )
			{

				if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
					protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
					ReportFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
				else
					ReportFileName = protocal->GetQCOutputDirectory();

				ReportFileName.append( "/" );

				std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
				str = str.substr( str.find_last_of("/\\")+1);

				ReportFileName.append( str );
				ReportFileName.append( protocal->GetEddyMotionCorrectionProtocal().reportFileNameSuffix );	
			}
			else
			{
				ReportFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
				ReportFileName.append( protocal->GetEddyMotionCorrectionProtocal().reportFileNameSuffix );			
			}
		}

		EddyMotionCorrector = EddyMotionCorrectorType::New();
		EddyMotionCorrector->SetInput( dwi);
		EddyMotionCorrector->SetReportFileName( ReportFileName );
		EddyMotionCorrector->SetReportFileMode( protocal->GetEddyMotionCorrectionProtocal().reportFileMode );

		EddyMotionCorrector->SetNumberOfBins(protocal->GetEddyMotionCorrectionProtocal().numberOfBins );
		EddyMotionCorrector->SetSamples( protocal->GetEddyMotionCorrectionProtocal().numberOfSamples );
		EddyMotionCorrector->SetTranslationScale( protocal->GetEddyMotionCorrectionProtocal().translationScale );
		EddyMotionCorrector->SetStepLength(protocal->GetEddyMotionCorrectionProtocal().stepLength );
		EddyMotionCorrector->SetFactor( protocal->GetEddyMotionCorrectionProtocal().relaxFactor );
		EddyMotionCorrector->SetMaxNumberOfIterations( protocal->GetEddyMotionCorrectionProtocal().maxNumberOfIterations );

		try
		{
			EddyMotionCorrector->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;			
		}

		this->DwiImageTemp = EddyMotionCorrector->GetOutput();

		for( unsigned int i=0; i< EddyMotionCorrector->GetOutputGradientDirectionContainer()->size(); i++)
		{
			for( unsigned int j=0; j< this->qcResult->GetIntensityMotionCheckResult().size(); j++)
			{
				if( 0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0] &&
					0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1] &&
					0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
				{
					this->qcResult->GetIntensityMotionCheckResult()[j].processing = QCResult::GRADIENT_BASELINE_AVERAGED;
					continue;
				}

				if( EddyMotionCorrector->GetFeedinGradientDirectionContainer()->at(i)[0] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0] &&
					EddyMotionCorrector->GetFeedinGradientDirectionContainer()->at(i)[1] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1] &&
					EddyMotionCorrector->GetFeedinGradientDirectionContainer()->at(i)[2] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
				{
					if( this->qcResult->GetIntensityMotionCheckResult()[j].processing > QCResult::GRADIENT_EDDY_MOTION_CORRECTED) //GRADIENT_EXCLUDE_SLICECHECK,
																														//GRADIENT_EXCLUDE_INTERLACECHECK,
																														//GRADIENT_EXCLUDE_GRADIENTCHECK,
																														//GRADIENT_EXCLUDE_MANUALLY,
					{
						std::cout<< "gradient " << i << "has been excluded!" <<std::endl;
					}
					else
					{
						this->qcResult->GetIntensityMotionCheckResult()[j].processing = QCResult::GRADIENT_EDDY_MOTION_CORRECTED;
						this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[0] = EddyMotionCorrector->GetOutputGradientDirectionContainer()->at(i)[0];
						this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[1] = EddyMotionCorrector->GetOutputGradientDirectionContainer()->at(i)[1];
						this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[2] = EddyMotionCorrector->GetOutputGradientDirectionContainer()->at(i)[2];
					}
				}
			}
		}

		if( protocal->GetGradientCheckProtocal().outputDWIFileNameSuffix.length()>0 )
		{
			std::string outputDWIFileName;
// 			outputDWIFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 			outputDWIFileName.append( protocal->GetEddyMotionCorrectionProtocal().outputDWIFileNameSuffix );	
			if( protocal->GetQCOutputDirectory().length()>0 )
			{

				if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
					protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
					outputDWIFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
				else
					outputDWIFileName = protocal->GetQCOutputDirectory();

				outputDWIFileName.append( "/" );

				std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
				str = str.substr( str.find_last_of("/\\")+1);

				outputDWIFileName.append( str );
				outputDWIFileName.append( protocal->GetEddyMotionCorrectionProtocal().outputDWIFileNameSuffix );	
			}
			else
			{
				outputDWIFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
				outputDWIFileName.append( protocal->GetEddyMotionCorrectionProtocal().outputDWIFileNameSuffix );			
			}


			try
			{
				std::cout<< "Saving output of eddy current motion correction: "<< outputDWIFileName <<" ... ";
				DwiWriter->SetImageIO(NrrdImageIO);
				DwiWriter->SetFileName( outputDWIFileName );
				DwiWriter->SetInput( DwiImageTemp);
				DwiWriter->UseCompressionOn();
				DwiWriter->Update();
			}
			catch(itk::ExceptionObject & e)
			{
				std::cout<< e.GetDescription()<<std::endl;
			}
			std::cout<< "DONE"<<std::endl;
		}
	}
	else
	{
		std::cout<<"Eddy-current and head motion correction NOT set."<<std::endl;
	}
	return true;
}



bool CIntensityMotionCheck::GradientWiseCheck( DwiImageType::Pointer dwi )
{
	if( protocal->GetGradientCheckProtocal().bCheck )
	{
		std::string ReportFileName;
		if( protocal->GetGradientCheckProtocal().reportFileNameSuffix.length()>0 )
		{
// 			ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 			ReportFileName.append( protocal->GetGradientCheckProtocal().reportFileNameSuffix );		
			if( protocal->GetQCOutputDirectory().length()>0 )
			{

				if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
					protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
					ReportFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
				else
					ReportFileName = protocal->GetQCOutputDirectory();

				ReportFileName.append( "/" );

				std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
				str = str.substr( str.find_last_of("/\\")+1);

				ReportFileName.append( str );
				ReportFileName.append( protocal->GetGradientCheckProtocal().reportFileNameSuffix );	
			}
			else
			{
				ReportFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
				ReportFileName.append( protocal->GetGradientCheckProtocal().reportFileNameSuffix );			
			}
		}

		GradientChecker = GradientCheckerType::New();
		GradientChecker->SetInput( dwi);
		GradientChecker->SetTranslationThreshold( protocal->GetGradientCheckProtocal().translationThreshold );
		GradientChecker->SetRotationThreshold( protocal->GetGradientCheckProtocal().rotationThreshold );
		GradientChecker->SetReportFileName( ReportFileName );
		GradientChecker->SetReportFileMode( protocal->GetGradientCheckProtocal().reportFileMode );

		try
		{
			GradientChecker->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;
			return -1;
		}

		DwiImageTemp = GradientChecker->GetOutput();

		for( unsigned int i=0; i< GradientChecker->GetGradientDirectionContainer()->size(); i++)
		{
			for( unsigned int j=0; j< this->qcResult->GetIntensityMotionCheckResult().size(); j++)
			{
				if( fabs(GradientChecker->GetGradientDirectionContainer()->at(i)[0] - this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[0]) < 0.000001 &&
					fabs(GradientChecker->GetGradientDirectionContainer()->at(i)[1] - this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[1]) < 0.000001 &&
					fabs(GradientChecker->GetGradientDirectionContainer()->at(i)[2] - this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[2]) < 0.000001    )
				{
					if( this->qcResult->GetIntensityMotionCheckResult()[j].processing > QCResult::GRADIENT_EDDY_MOTION_CORRECTED )
					{
						std::cout<< "gradient " << j << "has been excluded!" <<std::endl;

					}
					else
					{
						if(	!GradientChecker->getQCResults()[i])
						{
							//std::cout<< "gradient " << j << " GRADIENT_EXCLUDE_GRADIENTCHECK!" <<std::endl;
							this->qcResult->GetIntensityMotionCheckResult()[j].processing = QCResult::GRADIENT_EXCLUDE_GRADIENTCHECK;
						}
					}
				}
			}
		}

		if( protocal->GetGradientCheckProtocal().outputDWIFileNameSuffix.length()>0 )
		{
			std::string outputDWIFileName;
// 			outputDWIFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 			outputDWIFileName.append( protocal->GetGradientCheckProtocal().outputDWIFileNameSuffix );	
			if( protocal->GetQCOutputDirectory().length()>0 )
			{

				if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
					protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
					outputDWIFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
				else
					outputDWIFileName = protocal->GetQCOutputDirectory();

				outputDWIFileName.append( "/" );

				std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
				str = str.substr( str.find_last_of("/\\")+1);

				outputDWIFileName.append( str );
				outputDWIFileName.append( protocal->GetGradientCheckProtocal().outputDWIFileNameSuffix );	
			}
			else
			{
				outputDWIFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
				outputDWIFileName.append( protocal->GetGradientCheckProtocal().outputDWIFileNameSuffix );			
			}


			try
			{
				std::cout<< "Saving output of gradient check: "<< outputDWIFileName <<" ... ";
				DwiWriter->SetImageIO(NrrdImageIO);
				DwiWriter->SetFileName( outputDWIFileName );
				DwiWriter->SetInput( DwiImageTemp);
				DwiWriter->UseCompressionOn();
				DwiWriter->Update();
			}
			catch(itk::ExceptionObject & e)
			{
				std::cout<< e.GetDescription()<<std::endl;
				return -1;
			}
			std::cout<< "DONE"<<std::endl;
		}
	}
	else
	{
		std::cout<<"Gradient-wise check NOT set."<<std::endl;
	}
	return true;
}


bool CIntensityMotionCheck::SaveQCedDWI( DwiImageType::Pointer dwi )
{
 	if( protocal->GetQCedDWIFileNameSuffix().length()>0 )
 	{
 		std::string outputDWIFileName;
//  		outputDWIFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
//  		outputDWIFileName.append( protocal->GetQCedDWIFileNameSuffix());	
		if( protocal->GetQCOutputDirectory().length()>0 )
		{

			if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
				protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
				outputDWIFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
			else
				outputDWIFileName = protocal->GetQCOutputDirectory();

			outputDWIFileName.append( "/" );

			std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
			str = str.substr( str.find_last_of("/\\")+1);

			outputDWIFileName.append( str );
			outputDWIFileName.append( protocal->GetQCedDWIFileNameSuffix() );	
		}
		else
		{
			outputDWIFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			outputDWIFileName.append( protocal->GetQCedDWIFileNameSuffix() );			
		}

 		try
 		{
 			DwiWriter->SetImageIO(NrrdImageIO);
 			DwiWriter->SetFileName( outputDWIFileName );
 			DwiWriter->SetInput( dwi );
 			DwiWriter->UseCompressionOn();
 			DwiWriter->Update();
 		}
 		catch(itk::ExceptionObject & e)
 		{
 			std::cout<< e.GetDescription()<<std::endl;
 			return false;
 		}
		return true;
 	}
	return true;
}

unsigned char  CIntensityMotionCheck::RunPipelineByProtocal()
{
	if( !protocal )
	{
		std::cout<<"Protocal NOT set."<<std::endl;
		return -1;
	}

	if( DwiImage->GetVectorLength() != GradientDirectionContainer->size() )
	{
		std::cout<< "Bad DWI: mismatch between gradient image # and gradient vector #" << std::endl;
		return -1;
	}

	if(!bDwiLoaded  ) LoadDwiImage();

	bool bReport = false;
	std::ofstream outfile;

	std::string ReportFileName;
// 	ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
// 
// 	if( protocal->GetReportFileNameSuffix().length() > 0)
// 		ReportFileName.append( protocal->GetReportFileNameSuffix() );	
// 	else
// 		ReportFileName.append( "_QC_CheckReports.txt");
	if( protocal->GetQCOutputDirectory().length()>0 )
	{

		if( protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '\\' || 
			protocal->GetQCOutputDirectory().at( protocal->GetQCOutputDirectory().length()-1 ) == '/' 		)
			ReportFileName = protocal->GetQCOutputDirectory().substr( 0,protocal->GetQCOutputDirectory().find_last_of("/\\") );
		else
			ReportFileName = protocal->GetQCOutputDirectory();

		ReportFileName.append( "/" );

		std::string str = DwiFileName.substr( 0,DwiFileName.find_last_of('.') );
		str = str.substr( str.find_last_of("/\\")+1);

		ReportFileName.append( str );
		if( protocal->GetReportFileNameSuffix().length() > 0)
			ReportFileName.append( protocal->GetReportFileNameSuffix() );	
		else
			ReportFileName.append( "_QC_CheckReports.txt");
	}
	else
	{
		ReportFileName = DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		if( protocal->GetReportFileNameSuffix().length() > 0)
			ReportFileName.append( protocal->GetReportFileNameSuffix() );	
		else
			ReportFileName.append( "_QC_CheckReports.txt");
	}

	outfile.open( ReportFileName.c_str() );
	
	if(outfile)
		bReport = true;

	if(bReport)
	{
		outfile<<"================================= "<< std::endl;
		outfile<<"* DWI QC Report ( DTIPrep 1.0 ) * "<< std::endl;
		outfile<<"================================= "<< std::endl;
		outfile<<"DWI File: "<< DwiFileName << std::endl;
		time_t rawtime; time( &rawtime );	
		outfile<<"Check Time: "<< ctime(&rawtime) << std::endl;

		outfile.close();
	}	

	unsigned char result=0;   
	// ZYXEDCBA: 					
	// X  QC; Too many bad gradient directions found!
	// Y  QC; Single b-value DWI without a b0/baseline!
	// Z  QC: Gradient direction # is less than 6!
	// A: ImageCheck() 
	// B: DiffusionCheck() 
	// C: SliceCheck() 
	// D: InterlaceCheck() 
	// E:GradientCheck()

	//protocal->printProtocals();

	DwiWriter	= DwiWriterType::New();
	NrrdImageIO	= itk::NrrdImageIO::New();
	DwiWriter->SetImageIO(NrrdImageIO);

	DwiImageTemp = DwiImage;

	// image information check
	std::cout<<"====================="<<std::endl;
	std::cout<<"ImageCheck ... "<<std::endl;	
	if(!ImageCheck( DwiImageTemp ))
		result = result | 1;
	std::cout<<"ImageCheck DONE "<<std::endl;

	// diffusion information check
	std::cout<<"====================="<<std::endl;
	std::cout<<"DiffusionCheck ... "<<std::endl;
	if(!DiffusionCheck( DwiImageTemp ))
		result = result | 2;	   
 	std::cout<<"DiffusionCheck DONE "<<std::endl;

	// SliceChecker
	std::cout<<"====================="<<std::endl;
	std::cout<<"SliceWiseCheck ... "<<std::endl;
	if(!SliceWiseCheck( DwiImageTemp ))
		result = result | 4;
	std::cout<<"SliceWiseCheck DONE "<<std::endl;

	// InterlaceChecker
	std::cout<<"====================="<<std::endl;
	std::cout<<"InterlaceWiseCheck ... "<<std::endl;
	if(!InterlaceWiseCheck( DwiImageTemp ))
		result = result | 8;
	std::cout<<"InterlaceWiseCheck DONE "<<std::endl;

	// baseline average
	std::cout<<"====================="<<std::endl;
	std::cout<<"BaselineAverage ... "<<std::endl;
	BaselineAverage( DwiImageTemp );
	std::cout<<"BaselineAverage DONE "<<std::endl;

	// EddyMotionCorrect
	std::cout<<"====================="<<std::endl;
	std::cout<<"EddyCurrentHeadMotionCorrect ... "<<std::endl;
	//EddyMotionCorrect( DwiImageTemp );
	EddyMotionCorrectIowa(DwiImageTemp);
	std::cout<<"EddyCurrentHeadMotionCorrect DONE "<<std::endl;

	// GradientChecker
	std::cout<<"====================="<<std::endl;
	std::cout<<"GradientCheck ... "<<std::endl;
	if(!GradientWiseCheck( DwiImageTemp ))
		result = result | 16;
	std::cout<<"GradientCheck DONE "<<std::endl;

	// Save QC'ed DWI
 	std::cout<<"====================="<<std::endl;
 	std::cout<<"Save QC'ed DWI ... ";
	SaveQCedDWI(DwiImageTemp);
	/*
 	if( protocal->GetQCedDWIFileNameSuffix().length()>0 )
 	{
 		std::string outputDWIFileName;
 		outputDWIFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
 		outputDWIFileName.append( protocal->GetQCedDWIFileNameSuffix());	
 		try
 		{
 			DwiWriter->SetImageIO(NrrdImageIO);
 			DwiWriter->SetFileName( outputDWIFileName );
 			DwiWriter->SetInput( DwiImageTemp );
 			DwiWriter->UseCompressionOn();
 			DwiWriter->Update();
 		}
 		catch(itk::ExceptionObject & e)
 		{
 			std::cout<< e.GetDescription()<<std::endl;
 			return -1;
 		}
 		std::cout<< "DONE"<<std::endl;
 	}
	*/
	std::cout<< "DONE"<<std::endl;

	// DTIComputing
	std::cout<<"====================="<<std::endl;
	std::cout<<"DTIComputing ... "<<std::endl;
	DTIComputing();
	std::cout<<"DTIComputing DONE"<<std::endl;

	// 00000CBA:  
	// A: Gradient direction # is less than 6! 
	// B: Single b-value DWI without a b0/baseline!
	// C: Too many bad gradient directions found!
	// 0: valid

	unsigned char ValidateResult;
	collectLeftDiffusionStatistics( DwiImageTemp, ReportFileName );
	ValidateResult = validateLeftDiffusionStatistics();

	result = ( ValidateResult << 5 ) + result;

	return result;
}

unsigned char CIntensityMotionCheck::CheckByProtocal()
{
// result:
	// ZYXEDCBA: 
	// X QC; Too many bad gradient directions found!
	// Y QC; Single b-value DWI without a b0/baseline!
	// Z QC: Gradient direction # is less than 6!
	// A:ImageCheck() 
	// B:DiffusionCheck() 
	// C: IntraCheck()  
	// D:InterlaceCheck()  
	// E: InterCheck()
	return RunPipelineByProtocal();
}

bool CIntensityMotionCheck::validateDiffusionStatistics()
{
	return true;
}

unsigned char CIntensityMotionCheck::validateLeftDiffusionStatistics() 

// 00000CBA:  
// A: Gradient direction # is less than 6! 
// B: Single b-value DWI without a b0/baseline!
// C: Too many bad gradient directions found!
// 0: valid

{

	bool bReport = false;
	std::string ReportFileName;

	if( protocal->GetImageProtocal().reportFileNameSuffix.length()>0 )
	{
		ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		ReportFileName.append( protocal->GetImageProtocal().reportFileNameSuffix );			
	}

	std::ofstream outfile;
	outfile.open( ReportFileName.c_str(), std::ios_base::app);

	if( outfile ) 
		bReport = true;

	if(bReport)
	{
		outfile<<"================================"<<std::endl;
		outfile<<"  QC result summary:"<<std::endl;
		outfile<<"================================"<<std::endl;
	}

	std::cout<<"================================"<<std::endl;
	std::cout<<"  QC result summary:"<<std::endl;
	std::cout<<"================================"<<std::endl;

	unsigned char ret = 0;

	if(this->gradientDirLeftNumber<6)
	{
		std::cout<<"\tGradient direction # is less than 6!"<<std::endl;
		if(bReport)
			outfile<<"\tGradient direction # is less than 6!"<<std::endl;
		ret = ret| 1;
	}

	if(this->baselineLeftNumber==0 && this->bValueLeftNumber==1)
	{
		std::cout<<"\tSingle b-value DWI without a b0/baseline!"<<std::endl;
		if(bReport)
			outfile<<"\tSingle b-value DWI without a b0/baseline!"<<std::endl;
		ret = ret| 2;
	}

	if( ((this->gradientDirNumber)-(this->gradientDirLeftNumber)) > protocal->GetBadGradientPercentageTolerance()* (this->gradientDirNumber))
	{
		std::cout<<"\tToo many bad gradient directions found! "<<std::endl;
		if(bReport)
			outfile  <<"\tToo many bad gradient directions found! "<<std::endl;
		ret = ret| 4;
	}

	//std::cout<<"validateDiffusionStatistics(): ret "<<ret<<std::endl;
	outfile.close();

	return ret;
}

void CIntensityMotionCheck::collectLeftDiffusionStatistics( DwiImageType::Pointer dwi, std::string reportfilename )
{
	GradientDirectionContainerType::Pointer	GradContainer = GradientDirectionContainerType::New();
	double bValue;
	this->GetGridentDirections( dwi,bValue, GradContainer);

	/////////
	std::vector<DiffusionDir> DiffusionDirections;
	DiffusionDirections.clear();

	for( unsigned int i=0; i< GradContainer->size();i++) 
	{
		if(DiffusionDirections.size()>0)
		{
			bool newDir = true;
			for(unsigned int j=0;j<DiffusionDirections.size();j++)
			{
				if( GradContainer->ElementAt(i)[0] == DiffusionDirections[j].gradientDir[0] && 
					GradContainer->ElementAt(i)[1] == DiffusionDirections[j].gradientDir[1] && 
					GradContainer->ElementAt(i)[2] == DiffusionDirections[j].gradientDir[2] )
				{
					DiffusionDirections[j].repetitionNumber++;
					newDir = false;;
				}
			}
			if(newDir)
			{
				std::vector< double > dir;
				dir.push_back(GradContainer->ElementAt(i)[0]);
				dir.push_back(GradContainer->ElementAt(i)[1]);
				dir.push_back(GradContainer->ElementAt(i)[2]);

				DiffusionDir diffusionDir;
				diffusionDir.gradientDir = dir;
				diffusionDir.repetitionNumber=1;

				DiffusionDirections.push_back(diffusionDir);
			}
		}
		else
		{
			std::vector< double > dir;
			dir.push_back(GradContainer->ElementAt(i)[0]);
			dir.push_back(GradContainer->ElementAt(i)[1]);
			dir.push_back(GradContainer->ElementAt(i)[2]);

			DiffusionDir diffusionDir;
			diffusionDir.gradientDir = dir;
			diffusionDir.repetitionNumber=1;

			DiffusionDirections.push_back(diffusionDir);
		}
	}

	//std::cout<<"DiffusionDirections.size(): " << DiffusionDirections.size() <<std::endl;

	std::vector<double> dirMode;
	dirMode.clear();

	this->baselineLeftNumber=0;
	for( unsigned int i=0; i<DiffusionDirections.size(); i++)
	{
		if( DiffusionDirections[i].gradientDir[0] == 0.0 &&
			DiffusionDirections[i].gradientDir[1] == 0.0 &&
			DiffusionDirections[i].gradientDir[2] == 0.0 ) 
		{
			this->baselineLeftNumber = DiffusionDirections[i].repetitionNumber;
			//std::cout<<"DiffusionDirections[i].repetitionNumber: " <<i<<"  "<<DiffusionDirections[i].repetitionNumber <<std::endl;
		}
		else
		{
			this->repetitionLeftNumber.push_back(DiffusionDirections[i].repetitionNumber);

			double modeSqr =	DiffusionDirections[i].gradientDir[0]*DiffusionDirections[i].gradientDir[0] +
				DiffusionDirections[i].gradientDir[1]*DiffusionDirections[i].gradientDir[1] +
				DiffusionDirections[i].gradientDir[2]*DiffusionDirections[i].gradientDir[2];

			// 	std::cout<<"modeSqr: " <<modeSqr <<std::endl;
			if( dirMode.size() > 0)
			{
				bool newDirMode = true;
				for(unsigned int j=0;j< dirMode.size();j++)
				{
					if( fabs(modeSqr-dirMode[j])<0.001)   // 1 DIFFERENCE for b value
					{
						newDirMode = false;	
						break;
					}
				}
				if( newDirMode && DiffusionDirections[i].repetitionNumber>0 )
				{
					dirMode.push_back(	modeSqr) ;
					// 					std::cout<<" if(newDirMode) dirMode.size(): " <<  dirMode.size() <<std::endl;
				}
			}
			else
			{
				if(DiffusionDirections[i].repetitionNumber>0)
					dirMode.push_back(	modeSqr) ;
				//std::cout<<" else dirMode.size(): " <<  dirMode.size() <<std::endl;
			}
		}
	}

	// 	std::cout<<" repetNum.size(): " <<  repetNum.size() <<std::endl;
	// 	std::cout<<" dirMode.size(): " <<  dirMode.size() <<std::endl;

	this->gradientDirLeftNumber = 0;
	this->gradientLeftNumber = 0;
	for(unsigned int i=0; i<this->repetitionLeftNumber.size(); i++)
	{
		this->gradientLeftNumber+=this->repetitionLeftNumber[i];
		if(this->repetitionLeftNumber[i]>0)
			this->gradientDirLeftNumber++;
	}
		
	this->bValueLeftNumber = dirMode.size();

	std::cout<<"Left DWI Diffusion: "	<<std::endl;
	std::cout<<"\tbaselineLeftNumber: "	<<baselineLeftNumber	<<std::endl;
	std::cout<<"\tbValueLeftNumber: "	<<bValueLeftNumber		<<std::endl;
	std::cout<<"\tgradientDirLeftNumber: "<<gradientDirLeftNumber	<<std::endl;

	if( reportfilename.length()>0)
	{
		std::ofstream outfile; 
		outfile.open( ReportFileName.c_str(), std::ios::app);
		outfile<<"--------------------------------"<<std::endl;
		outfile<<"Diffusion Gradient information:"<<std::endl;

		outfile<<"Left DWI Diffusion: "		<<std::endl;
		outfile<<"\tbaselineLeftNumber: "	<<baselineLeftNumber	<<std::endl;
		outfile<<"\tbValueLeftNumber: "		<<bValueLeftNumber		<<std::endl;
		outfile<<"\tgradientDirLeftNumber: "<<gradientDirLeftNumber	<<std::endl;

		for(unsigned int i=0;i< DiffusionDirections.size();i++)
		{
			std::cout<<"\t"<<i<<"\t[ " 
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<DiffusionDirections[i].gradientDir[0]<<", "
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<DiffusionDirections[i].gradientDir[1]<<", "
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<DiffusionDirections[i].gradientDir[2]<<" ]"
				<<"\t"<<DiffusionDirections[i].repetitionNumber <<std::endl;

			outfile<<"\t"<<i<<"\t[ " 
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<DiffusionDirections[i].gradientDir[0]<<", "
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<DiffusionDirections[i].gradientDir[1]<<", "
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<DiffusionDirections[i].gradientDir[2]<<" ]"
				<<"\t"<<DiffusionDirections[i].repetitionNumber <<std::endl;

		}
		outfile.close();
	}

	return;
}

void CIntensityMotionCheck::collectDiffusionStatistics()
{
	std::vector<DiffusionDir> DiffusionDirections;
	DiffusionDirections.clear();

	// 	std::cout<<"this->GetDiffusionProtocal().gradients.size(): " << this->GetDiffusionProtocal().gradients.size() <<std::endl;
	for( unsigned int i=0; i< this->GradientDirectionContainer->size();i++) 
	{
		if(DiffusionDirections.size()>0)
		{
			bool newDir = true;
			for(unsigned int j=0;j<DiffusionDirections.size();j++)
			{
				if( this->GradientDirectionContainer->ElementAt(i)[0] == DiffusionDirections[j].gradientDir[0] && 
					this->GradientDirectionContainer->ElementAt(i)[1] == DiffusionDirections[j].gradientDir[1] && 
					this->GradientDirectionContainer->ElementAt(i)[2] == DiffusionDirections[j].gradientDir[2] )
				{
					DiffusionDirections[j].repetitionNumber++;
					newDir = false;;
				}
			}
			if(newDir)
			{
				std::vector< double > dir;
				dir.push_back(this->GradientDirectionContainer->ElementAt(i)[0]);
				dir.push_back(this->GradientDirectionContainer->ElementAt(i)[1]);
				dir.push_back(this->GradientDirectionContainer->ElementAt(i)[2]);

				DiffusionDir diffusionDir;
				diffusionDir.gradientDir = dir;
				diffusionDir.repetitionNumber=1;

				DiffusionDirections.push_back(diffusionDir);
			}
		}
		else
		{
			std::vector< double > dir;
			dir.push_back(this->GradientDirectionContainer->ElementAt(i)[0]);
			dir.push_back(this->GradientDirectionContainer->ElementAt(i)[1]);
			dir.push_back(this->GradientDirectionContainer->ElementAt(i)[2]);

			DiffusionDir diffusionDir;
			diffusionDir.gradientDir = dir;
			diffusionDir.repetitionNumber=1;

			DiffusionDirections.push_back(diffusionDir);
		}
	}

//	std::cout<<" DiffusionDirections.size(): " << DiffusionDirections.size() <<std::endl;

	std::vector<int> repetNum;
	repetNum.clear();
	std::vector<double> dirMode;
	dirMode.clear();

	for( unsigned int i=0; i<DiffusionDirections.size(); i++)
	{
		if( DiffusionDirections[i].gradientDir[0] == 0.0 &&
			DiffusionDirections[i].gradientDir[1] == 0.0 &&
			DiffusionDirections[i].gradientDir[2] == 0.0 ) 
		{
			this->baselineNumber = DiffusionDirections[i].repetitionNumber;
//			std::cout<<" DiffusionDirections[i].repetitionNumber: " <<i<<"  "<<DiffusionDirections[i].repetitionNumber <<std::endl;
		}
		else
		{
			repetNum.push_back(DiffusionDirections[i].repetitionNumber);

			double modeSqr =	DiffusionDirections[i].gradientDir[0]*DiffusionDirections[i].gradientDir[0] +
				DiffusionDirections[i].gradientDir[1]*DiffusionDirections[i].gradientDir[1] +
				DiffusionDirections[i].gradientDir[2]*DiffusionDirections[i].gradientDir[2];

//			std::cout<<" modeSqr: " <<modeSqr <<std::endl;
			if( dirMode.size() > 0)
			{
				bool newDirMode = true;
				for(unsigned int j=0;j< dirMode.size();j++)
				{
					if( fabs(modeSqr-dirMode[j])<0.001)   // 1 DIFFERENCE for b value
					{
						newDirMode = false;	
						break;
					}
				}
				if(newDirMode)
				{
					dirMode.push_back(	modeSqr) ;
					// 					std::cout<<" if(newDirMode) dirMode.size(): " <<  dirMode.size() <<std::endl;
				}
			}
			else
			{
				dirMode.push_back(	modeSqr) ;
				// 				std::cout<<" else dirMode.size(): " <<  dirMode.size() <<std::endl;
			}
		}
	}

//	 	std::cout<<"  repetNum.size(): " <<  repetNum.size() <<std::endl;
//	 	std::cout<<"  dirMode.size(): " <<  dirMode.size() <<std::endl;

	this->gradientDirNumber = repetNum.size();
	this->bValueNumber = dirMode.size();

	if( repetNum.size() > 1 )
	{
		repetitionNumber = repetNum[0];
		for( unsigned int i=1; i<repetNum.size(); i++)
		{ 
			if( repetNum[i] != repetNum[0])
			{
				std::cout<<"DWI data error. Not all the gradient directions have same repetition. "<<std::endl;
				repetitionNumber = -1;			
			}
		}
	}
	else
	{
		std::cout<<" too less independent gradient dir detected in DWI "<<std::endl;
	}

	this->gradientNumber = this->GradientDirectionContainer->size()-this->baselineNumber;

// 	std::cout<<"DWI Diffusion: "		<<std::endl;
 //	std::cout<<"  baselineNumber: "		<<baselineNumber	<<std::endl;
 //	std::cout<<"  bValueNumber: "		<<bValueNumber		<<std::endl;
//	std::cout<<"  gradientDirNumber: "	<<gradientDirNumber	<<std::endl;
//	std::cout<<"  gradientNumber: "		<<gradientNumber	<<std::endl;
 //	std::cout<<"  repetitionNumber: "	<<repetitionNumber	<<std::endl;
	return ;
}


bool CIntensityMotionCheck::dtiestim()
{
 	if(!protocal->GetDTIProtocal().bCompute)
	{
		std::cout<< "DTI computing NOT set" <<std::endl;
		return true;
	}

// dtiestim
	std::string str;
	str.append(protocal->GetDTIProtocal().dtiestimCommand); 
	str.append(" ");

	std::string OutputDwiFileName;
	OutputDwiFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
	OutputDwiFileName.append( protocal->GetQCedDWIFileNameSuffix() );	

	str.append(OutputDwiFileName);
	str.append(" ");

	std::string OutputTensor;
	OutputTensor=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
	OutputTensor.append( protocal->GetDTIProtocal().tensorSuffix);
	str.append(OutputTensor); 
	
	if(protocal->GetDTIProtocal().method == Protocal::METHOD_WLS )
		str.append(" -m wls ");
	else if(protocal->GetDTIProtocal().method == Protocal::METHOD_ML)
		str.append(" -m ml ");
	else if(protocal->GetDTIProtocal().method == Protocal::METHOD_NLS)
		str.append(" -m nls ");
	else
		str.append(" -m lls ");

	if(protocal->GetDTIProtocal().mask.length()>0)
	{
		str.append(" -M ");
		str.append( protocal->GetDTIProtocal().mask );
	}

	str.append(" -t ");
	char buffer [10]; 
	sprintf( buffer, "%d", protocal->GetDTIProtocal().baselineThreshold );
	str.append( buffer );	

	if( protocal->GetDTIProtocal().bidwi)
	{
		str.append(" --idwi "); 
		std::string idwi;
		idwi=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		idwi.append(protocal->GetDTIProtocal().idwiSuffix);
		str.append(idwi);
	}

	if( protocal->GetDTIProtocal().bbaseline)
	{
		str.append(" --B0 "); 
		std::string baseline;
		baseline=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		baseline.append(protocal->GetDTIProtocal().baselineSuffix);
		str.append(baseline);
	}

	std::cout<< "dtiestim command: "<< str.c_str() << std::endl;
	system(str.c_str());
	return true;
}

bool CIntensityMotionCheck::DTIComputing()
{
	if(!protocal->GetDTIProtocal().bCompute)
	{
		std::cout<< "DTI computing NOT set" <<std::endl;
		return true;
	}

	dtiestim();
	dtiprocess();
	return true;
}

bool CIntensityMotionCheck::dtiprocess()
{
	std::string string;
	string.append(protocal->GetDTIProtocal().dtiprocessCommand); 
	string.append(" "); 

	std::string dtiprocessInput;
	dtiprocessInput=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
	dtiprocessInput.append(protocal->GetDTIProtocal().tensorSuffix);

	string.append(dtiprocessInput);	

	if( protocal->GetDTIProtocal().bfa)
	{
		string.append(" -f ");
		std::string fa;
		fa=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		fa.append(protocal->GetDTIProtocal().faSuffix);
		string.append(fa); 
	}

	if( protocal->GetDTIProtocal().bmd)
	{
		string.append(" -m "); 
		std::string md;
		md=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		md.append(protocal->GetDTIProtocal().mdSuffix);
		string.append(md); 
	}

	if( protocal->GetDTIProtocal().bcoloredfa)
	{
		string.append(" -c "); 
		std::string cfa;
		cfa=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		cfa.append(protocal->GetDTIProtocal().coloredfaSuffix);
		string.append(cfa); 
	}

	if( protocal->GetDTIProtocal().bfrobeniusnorm)
	{
		string.append(" --frobenius-norm-output "); 
		std::string fn;
		fn=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		fn.append(protocal->GetDTIProtocal().frobeniusnormSuffix);
		string.append(fn); 
	}

	std::cout<< "dtiprocess command: "<< string.c_str() << std::endl;
	system(string.c_str());

	return true;
}






//////////////////////////////
bool CIntensityMotionCheck::ImageCheck( )
{

	DwiImageType::Pointer dwi = DwiImageTemp;
	//First check
	bool returnValue = true;
	bool bReport = false;
	std::string ReportFileName;

	if( protocal->GetImageProtocal().reportFileNameSuffix.length()>0 )
	{
		ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		ReportFileName.append( protocal->GetImageProtocal().reportFileNameSuffix );			
	}

// 	std::cout << "DwiFileName: " << DwiFileName<<std::endl;
// 	std::cout << "ReportFileName: " << ReportFileName<<std::endl;

	std::ofstream outfile;

	if( protocal->GetImageProtocal().reportFileMode == 1 )
		outfile.open( ReportFileName.c_str(), std::ios_base::app);
	else
		outfile.open( ReportFileName.c_str());

	if( outfile ) 
		bReport = true;

	if( bReport )
	{
		outfile <<std::endl;
		outfile <<"================================"<<std::endl;
		outfile <<"  Image Information checking    "<<std::endl;
		outfile <<"================================"<<std::endl;
	}
	else
	{
		std::cout<< "Image information check report file open failed." << std::endl;
	}	

	if( !protocal->GetImageProtocal().bCheck)
	{
		std::cout<<"Image information check NOT set."<<std::endl;
		if(bReport)
			outfile<<"Image information check NOT set."<<std::endl;

		return true;
	}
	else
	{
		if( !dwi  )
		{
			std::cout<<"DWI image error."<<std::endl;
			bGetGridentDirections=false;
			return false;
		}
		//size
		if( protocal->GetImageProtocal().size[0] ==	dwi->GetLargestPossibleRegion().GetSize()[0] && 
			protocal->GetImageProtocal().size[1] ==	dwi->GetLargestPossibleRegion().GetSize()[1] && 
			protocal->GetImageProtocal().size[2] ==	dwi->GetLargestPossibleRegion().GetSize()[2] )
		{
			qcResult->GetImageInformationCheckResult().size = true;
			if(bReport)
				outfile<<"Image size Check: " << "\t\tOK"<<std::endl;
			std::cout<<"Image size Check: " << "\t\tOK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().size = false;
			if(bReport)
				outfile<<"Image size Check: " << "\t\tFAILED"<<std::endl;
			std::cout<<"Image size Check: " << "\t\tFAILED"<<std::endl;
			returnValue = false;
		}
		//origion
		if( protocal->GetImageProtocal().origin[0] ==	dwi->GetOrigin()[0]	&& 
			protocal->GetImageProtocal().origin[1] ==	dwi->GetOrigin()[1]	&& 
			protocal->GetImageProtocal().origin[2] ==	dwi->GetOrigin()[2]	 	)
		{
			qcResult->GetImageInformationCheckResult().origin = true;
			if(bReport)
				outfile<<"Image Origin Check: " << "\t\tOK"<<std::endl;
			std::cout<<"Image Origin Check: " << "\t\tOK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().origin = false;
			if(bReport)
				outfile<<"Image Origin Check: " << "\t\tFAILED"<<std::endl;
			std::cout<<"Image Origin Check: " << "\t\tFAILED"<<std::endl;
			returnValue = false;
		}
		//spacing
		//std::cout<<"spacing: "<< protocal->GetImageProtocal().spacing[0]<<" "<<protocal->GetImageProtocal().spacing[1]<<" "<<protocal->GetImageProtocal().spacing[2]<<std::endl;
		//std::cout<<"spacing: "<< DwiImage->GetSpacing()[0]<<" "<<DwiImage->GetSpacing()[1]<<" "<<DwiImage->GetSpacing()[2]<<std::endl;
		if( protocal->GetImageProtocal().spacing[0] ==	dwi->GetSpacing()[0]	&& 
			protocal->GetImageProtocal().spacing[1] ==	dwi->GetSpacing()[1]	&& 
			protocal->GetImageProtocal().spacing[2] ==	dwi->GetSpacing()[2]	 	)
		{
			qcResult->GetImageInformationCheckResult().spacing = true;
			if(bReport)
				outfile<<"Image Spacing Check: " << "\t\tOK"<<std::endl;
			std::cout<<"Image Spacing Check: " << "\t\tOK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().spacing = false;
			if(bReport)
				outfile<<"Image Spacing Check: " << "\t\tFAILED"<<std::endl;
			std::cout<<"Image Spacing Check: " << "\t\tFAILED"<<std::endl;
			returnValue = false;
		}
		// space direction
		vnl_matrix<double> imgf(3,3);
		imgf = dwi->GetDirection().GetVnlMatrix();

		if( protocal->GetImageProtocal().spacedirection[0][0] ==	imgf(0,0)	&& 
			protocal->GetImageProtocal().spacedirection[0][1] ==	imgf(0,1)	&&
			protocal->GetImageProtocal().spacedirection[0][2] ==	imgf(0,2)	&&
			protocal->GetImageProtocal().spacedirection[1][0] ==	imgf(1,0)	&&
			protocal->GetImageProtocal().spacedirection[1][1] ==	imgf(1,1)	&&
			protocal->GetImageProtocal().spacedirection[1][2] ==	imgf(1,2)	&&
			protocal->GetImageProtocal().spacedirection[2][0] ==	imgf(2,0)	&&
			protocal->GetImageProtocal().spacedirection[2][1] ==	imgf(2,1)	&&
			protocal->GetImageProtocal().spacedirection[2][2] ==	imgf(2,2)	    )		
		{
			qcResult->GetImageInformationCheckResult().spacedirection = true;
			if(bReport)
				outfile<<"Image spacedirection Check: " << "\tOK"<<std::endl;
			std::cout<<"Image spacedirection Check: " << "\tOK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().spacedirection = false;
			if(bReport)
				outfile<<"Image spacedirection Check: " << "\tFAILED"<<std::endl;
			std::cout<<"Image spacedirection Check: " << "\tFAILED"<<std::endl;
			returnValue = false;
		}

		// space
		itk::MetaDataDictionary imgMetaDictionary = dwi->GetMetaDataDictionary(); 
		std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
		std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
		std::string metaString;

		itk::ExposeMetaData<std::string> (imgMetaDictionary, "NRRD_space", metaString);

		int space;
		if(		 metaString.compare( "left-posterior-superior") ==0 )	space = Protocal::SPACE_LPS;
		else if( metaString.compare( "left-posterior-inferior") ==0 )	space = Protocal::SPACE_LPI;
		else if( metaString.compare( "left-anterior-superior" )==0 )	space = Protocal::SPACE_LAS;
		else if( metaString.compare( "left-anterior-inferior" )==0 )	space = Protocal::SPACE_LAI;
		else if( metaString.compare( "right-posterior-superior")==0 )	space = Protocal::SPACE_RPS;
		else if( metaString.compare( "right-posterior-inferior")==0 )	space = Protocal::SPACE_RPI;
		else if( metaString.compare( "right-anterior-superior" )==0 )	space = Protocal::SPACE_RAS;
		else if( metaString.compare( "right-anterior-inferior" )==0 )	space = Protocal::SPACE_RAI;
		else space = Protocal::SPACE_UNKNOWN;

		if( protocal->GetImageProtocal().space == space)
		{
			qcResult->GetImageInformationCheckResult().space = true;
			if(bReport)
				outfile<<"Image space Check: " << "\t\tOK"<<std::endl;
			std::cout<<"Image space Check: " << "\t\tOK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().space = false;
			if(bReport)
				outfile<<"Image space Check: " << "\t\tFAILED"<<std::endl;
			std::cout<<"Image space Check: " << "\t\tFAILED"<<std::endl;
			returnValue = false;
		}
	}

	if(bReport)
		outfile.close();	

	// then crop/pad
	
	if( protocal->GetImageProtocal().bCrop )
	{
		int *sizePara = NULL;
		sizePara = new int[3];
		sizePara[0]	= protocal->GetImageProtocal().size[0];
		sizePara[1]	= protocal->GetImageProtocal().size[1];
		sizePara[2]	= protocal->GetImageProtocal().size[2];

		Cropper	= CropperType::New();
		Cropper->SetInput( dwi ); 
		Cropper->SetSize(sizePara);

		if(bReport)
		{
			Cropper->SetReportFileName( ReportFileName ); //protocal->GetImageProtocal().reportFileNameSuffix);
			Cropper->SetReportFileMode( protocal->GetImageProtocal().reportFileMode);
		}
		Cropper->Update();

		DwiImageTemp = Cropper->GetOutput();

		if( protocal->GetImageProtocal().croppedDWIFileNameSuffix.length()>0 )
		{
			try
			{
				std::string CroppedFileName;
				CroppedFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
				CroppedFileName.append( protocal->GetImageProtocal().croppedDWIFileNameSuffix );

				DwiWriter->SetFileName( CroppedFileName );
				DwiWriter->SetInput( DwiImageTemp );
				DwiWriter->UseCompressionOn();
				DwiWriter->Update();
			}
			catch(itk::ExceptionObject & e)
			{
				std::cout<< e.GetDescription()<<std::endl;
			}
			std::cout<< "DONE."<< std::endl;
		}
	}
	else
	{
		DwiImageTemp = dwi;
	}

	return returnValue;
}

bool CIntensityMotionCheck::DiffusionCheck( )
{
	DwiImageType::Pointer dwi = DwiImageTemp;

	bool bReport = false;

	std::string ReportFileName;
	if( protocal->GetDiffusionProtocal().reportFileNameSuffix.length()>0 )
	{
		ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		ReportFileName.append( protocal->GetDiffusionProtocal().reportFileNameSuffix );			
	}		

	std::ofstream outfile;
	if( protocal->GetImageProtocal().reportFileMode == 1 )
		outfile.open( ReportFileName.c_str(), std::ios_base::app);
	else
		outfile.open( ReportFileName.c_str());
	if(outfile) 
		bReport = true;

	if(bReport) 
	{
		outfile <<std::endl;
		outfile <<"================================"<<std::endl;
		outfile <<" Diffusion Information checking "<<std::endl;
		outfile <<"================================"<<std::endl;
	}

	bool returnValte;

	if( !protocal->GetDiffusionProtocal().bCheck)
	{
		if(bReport)
			outfile<<"Diffusion information check NOT set."<<std::endl;
		std::cout<<"Diffusion information check NOT set."<<std::endl;
		return true;
	}
	else
	{
		if( !dwi )
		{
			std::cout<<"DWI error."<<std::endl;
			bGetGridentDirections=false;
			return false;
		}

		GradientDirectionContainerType::Pointer	GradContainer = GradientDirectionContainerType::New();
		double bValue;
		this->GetGridentDirections( dwi,bValue, GradContainer);

		if( fabs(protocal->GetDiffusionProtocal().bValue - bValue) < 0.0000001 )
		{
			qcResult->GetDiffusionInformationCheckResult().b = true;
			if(bReport)
				outfile<<"DWMRI_bValue Check: " << "\t\t\tOK"<<std::endl;
			std::cout<<"DWMRI_bValue check: " << "\t\t\tOK"<<std::endl;
		}
		else
		{
			qcResult->GetDiffusionInformationCheckResult().b = false;
			if(bReport)
			{
				outfile<<"Diffusion b-value Check:\t\tFAILED" <<std::endl;
				outfile<<"DWMRI_bValue\t\tmismatch with DWI = " << this->b0 
					<< "\t\t\t\tprotocol = " << protocal->GetDiffusionProtocal().bValue <<std::endl;
			}
			std::cout<<"Diffusion b-value Check:\t\tFAILED" <<std::endl;
			// 			std::cout <<"DWMRI_bValue\t\tmismatch with DWI = " << this->b0 
			// 				<< "\t\t\t\tprotocol = " << protocal->GetDiffusionProtocal().bValue <<std::endl;

			if( protocal->GetDiffusionProtocal().bUseDiffusionProtocal )
			{
				std::ostringstream ossMetaString;
				ossMetaString << protocal->GetDiffusionProtocal().bValue;
				itk::EncapsulateMetaData<std::string>( dwi->GetMetaDataDictionary(), "DWMRI_b-value", ossMetaString.str());
			}
			returnValte = false;
		}

		bool result = true;
		if( GradContainer->size() != protocal->GetDiffusionProtocal().gradients.size())
		{
			qcResult->GetDiffusionInformationCheckResult().gradient = false;
			if(bReport)
				outfile <<"Diffusion vector # mismatch with protocol = "
				<< protocal->GetDiffusionProtocal().gradients.size()
				<< " image = " << GradContainer->size() <<std::endl;

			std::cout	<<"Diffusion vector # mismatch with protocol = "
				<< protocal->GetDiffusionProtocal().gradients.size()
				<< " image = " << GradContainer->size() <<std::endl;

			result = false;
		}
		else // Diffusion vector # matched
		{
			qcResult->GetDiffusionInformationCheckResult().gradient = true;
			for(unsigned int i=0; i< GradContainer->size();i++)
			{
				if( fabs(protocal->GetDiffusionProtocal().gradients[i][0] - GradContainer->ElementAt(i)[0]) < 0.00001 &&
					fabs(protocal->GetDiffusionProtocal().gradients[i][1] - GradContainer->ElementAt(i)[1]) < 0.00001 &&
					fabs(protocal->GetDiffusionProtocal().gradients[i][2] - GradContainer->ElementAt(i)[2]) < 0.00001 )			
				{
					result = result && true;
				}
				else
				{
					if(bReport)
					{
						outfile	<<"DWMRI_gradient_" << std::setw(4) << std::setfill('0') << i<< "\tmismatch with DWI = [ " 
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)<< GradContainer->ElementAt(i)[0]<< " "
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)<< GradContainer->ElementAt(i)[1]<< " "
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)<< GradContainer->ElementAt(i)[2]<< " ] \tprotocol = [ "
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)<< protocal->GetDiffusionProtocal().gradients[i][0]<< " "
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)<< protocal->GetDiffusionProtocal().gradients[i][1]<< " "
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)<< protocal->GetDiffusionProtocal().gradients[i][2]<< " ]" << std::endl;
					}

					if( protocal->GetDiffusionProtocal().bUseDiffusionProtocal )
					{
						std::ostringstream ossMetaString, ossMetaKey;
						ossMetaKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << i;
						ossMetaString << std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
							<< protocal->GetDiffusionProtocal().gradients[i][0] << "    " 
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
							<< protocal->GetDiffusionProtocal().gradients[i][1] << "    " 
							<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
							<< protocal->GetDiffusionProtocal().gradients[i][2] ;

						itk::EncapsulateMetaData<std::string>( dwi->GetMetaDataDictionary(), ossMetaKey.str(),  ossMetaString.str());
					}

					qcResult->GetDiffusionInformationCheckResult().gradient = false;
					result = false;				
				}
			}
		}

		if( result )
		{
			qcResult->GetDiffusionInformationCheckResult().gradient = true;
			if(bReport)
				outfile<<"Diffusion gradient Check: \t\tOK"<<std::endl;
			std::cout<<"Diffusion gradient Check: \t\tOK"<<std::endl;
		}
		else
		{
			qcResult->GetDiffusionInformationCheckResult().gradient = false;
			if(bReport)
				outfile<<"Diffusion gradient Check: \t\tFAILED"<<std::endl;
			std::cout<<"Diffusion gradient Check: \t\tFAILED"<<std::endl;	
		}

		returnValte = returnValte && result;

		//  measurement frame 
		if( DwiImageTemp->GetMetaDataDictionary().HasKey("NRRD_measurement frame"))
		{
			// measurement frame
			vnl_matrix<double> mf(3,3);
			// imaging frame
			vnl_matrix<double> imgf(3,3);
			std::vector<std::vector<double> > nrrdmf;
			itk::ExposeMetaData<std::vector<std::vector<double> > >( dwi->GetMetaDataDictionary(),"NRRD_measurement frame",nrrdmf);

			imgf = DwiImageTemp->GetDirection().GetVnlMatrix();

			//Image frame
			for(unsigned int i = 0; i < 3; ++i)
			{
				for(unsigned int j = 0; j < 3; ++j)
				{
					mf(i,j) = nrrdmf[j][i];
					nrrdmf[j][i] = imgf(i,j);
				}
			}

			// Meausurement frame
			if( 
				protocal->GetDiffusionProtocal().measurementFrame[0][0] == mf(0,0) &&
				protocal->GetDiffusionProtocal().measurementFrame[0][1] == mf(0,1) &&
				protocal->GetDiffusionProtocal().measurementFrame[0][2] == mf(0,2) &&
				protocal->GetDiffusionProtocal().measurementFrame[1][0] == mf(1,0) &&
				protocal->GetDiffusionProtocal().measurementFrame[1][1] == mf(1,1) &&
				protocal->GetDiffusionProtocal().measurementFrame[1][2] == mf(1,2) &&
				protocal->GetDiffusionProtocal().measurementFrame[2][0] == mf(2,0) &&
				protocal->GetDiffusionProtocal().measurementFrame[2][1] == mf(2,1) &&
				protocal->GetDiffusionProtocal().measurementFrame[2][2] == mf(2,2)		)
			{
				qcResult->GetDiffusionInformationCheckResult().measurementFrame = true;
				if(bReport)
					outfile<<"Diffusion measurementFrame Check: \tOK"<<std::endl;
				std::cout<<"Diffusion measurementFrame Check: \tOK"<<std::endl;
			}
			else
			{
				returnValte = false;
				qcResult->GetDiffusionInformationCheckResult().measurementFrame = false;
				if(bReport)
					outfile<<"Diffusion measurementFrame Check: \tFAILED"<<std::endl;
				std::cout<<"Diffusion measurementFrame Check: \tFAILED"<<std::endl;

				if( protocal->GetDiffusionProtocal().bUseDiffusionProtocal )
				{
					nrrdmf[0][0] = protocal->GetDiffusionProtocal().measurementFrame[0][0];
					nrrdmf[0][1] = protocal->GetDiffusionProtocal().measurementFrame[0][1];
					nrrdmf[0][2] = protocal->GetDiffusionProtocal().measurementFrame[0][2] ;
					nrrdmf[1][0] = protocal->GetDiffusionProtocal().measurementFrame[1][0] ;
					nrrdmf[1][1] = protocal->GetDiffusionProtocal().measurementFrame[1][1] ;
					nrrdmf[1][2] = protocal->GetDiffusionProtocal().measurementFrame[1][2] ;
					nrrdmf[2][0] = protocal->GetDiffusionProtocal().measurementFrame[2][0] ;
					nrrdmf[2][1] = protocal->GetDiffusionProtocal().measurementFrame[2][1] ;
					nrrdmf[2][2] = protocal->GetDiffusionProtocal().measurementFrame[2][2] ;	
					itk::EncapsulateMetaData<std::vector<std::vector<double> > >( dwi->GetMetaDataDictionary(),"NRRD_measurement frame",nrrdmf);
				}
			}
		}

		if( !returnValte )
		{
			std::cout << "Mismatched informations was replaced with that from protocol." << std::endl;
			if(bReport)
				outfile << "Mismatched informations was replaced with that from protocol." << std::endl;
		}
	}

	if( bReport )
		outfile.close();

	// then save the updated DWI
	if( protocal->GetDiffusionProtocal().diffusionReplacedDWIFileNameSuffix.length()>0 )
	{
		std::string DWIFileName;
		DWIFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		DWIFileName.append( protocal->GetDiffusionProtocal().diffusionReplacedDWIFileNameSuffix );			
		try
		{
			std::cout<< "Saving diffusion information updated DWI: "<< DWIFileName << "...";
			DwiWriter->SetFileName( DWIFileName );
			DwiWriter->SetInput( DwiImageTemp);			
			DwiWriter->UseCompressionOn();
			DwiWriter->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;
			//return false;
		}
		std::cout<< "DONE"<<std::endl;
	}

	return returnValte;
}



bool CIntensityMotionCheck::SliceWiseCheck( )
{
	DwiImageType::Pointer dwi = DwiImageTemp;

	if( protocal->GetSliceCheckProtocal().bCheck )
	{
		std::string ReportFileName;
		if( protocal->GetSliceCheckProtocal().reportFileNameSuffix.length()>0 )
		{
			ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			ReportFileName.append( protocal->GetSliceCheckProtocal().reportFileNameSuffix );			
		}

		SliceChecker = SliceCheckerType::New();
		SliceChecker->SetInput( dwi );
		SliceChecker->SetCheckTimes( protocal->GetSliceCheckProtocal().checkTimes );
		SliceChecker->SetHeadSkipRatio( protocal->GetSliceCheckProtocal().headSkipSlicePercentage );
		SliceChecker->SetTailSkipRatio( protocal->GetSliceCheckProtocal().tailSkipSlicePercentage );
		SliceChecker->SetBaselineStdevTimes( protocal->GetSliceCheckProtocal().correlationDeviationThresholdbaseline );
		SliceChecker->SetGradientStdevTimes( protocal->GetSliceCheckProtocal().correlationDeviationThresholdgradient );
		SliceChecker->SetReportFileName( ReportFileName );
		SliceChecker->SetReportFileMode( protocal->GetSliceCheckProtocal().reportFileMode );

		try
		{
			SliceChecker->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;			
		}

		DwiImageTemp = SliceChecker->GetOutput();

		for( unsigned int i=0; i< SliceChecker->GetGradientDirectionContainer()->size(); i++)
		{
			for( unsigned int j=0; j< this->qcResult->GetIntensityMotionCheckResult().size(); j++)
			{
				if( SliceChecker->GetGradientDirectionContainer()->at(i)[0] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0] &&
					SliceChecker->GetGradientDirectionContainer()->at(i)[1] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1] &&
					SliceChecker->GetGradientDirectionContainer()->at(i)[2] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
				{
					if( this->qcResult->GetIntensityMotionCheckResult()[j].processing != QCResult::GRADIENT_INCLUDE)
					{
						std::cout<< "gradient " << i << "has been excluded!" <<std::endl;

					}
					else
					{
						if(	!SliceChecker->getQCResults()[i])
							this->qcResult->GetIntensityMotionCheckResult()[i].processing = QCResult::GRADIENT_EXCLUDE_SLICECHECK;
					}
				}
			}
		}


		if( protocal->GetSliceCheckProtocal().outputDWIFileNameSuffix.length()>0 )
		{
			std::string SliceWiseOutput;
			SliceWiseOutput=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			SliceWiseOutput.append( protocal->GetSliceCheckProtocal().outputDWIFileNameSuffix );			

			try
			{
				std::cout<< "Saving output of slice check: "<< SliceWiseOutput <<" ... ";
				DwiWriter->SetImageIO(NrrdImageIO);
				DwiWriter->SetFileName( SliceWiseOutput );
				DwiWriter->SetInput( DwiImageTemp );
				DwiWriter->UseCompressionOn();
				DwiWriter->Update();
			}
			catch(itk::ExceptionObject & e)
			{
				std::cout<< e.GetDescription()<<std::endl;
			}
			std::cout<< "DONE"<<std::endl;
		}
	}
	else
	{
		std::cout<<"Slice-wise check NOT set."<<std::endl;
	}

	return true;
}

bool CIntensityMotionCheck::InterlaceWiseCheck( )
{
	DwiImageType::Pointer dwi = DwiImageTemp;

	if( protocal->GetInterlaceCheckProtocal().bCheck )
	{
		std::string ReportFileName;
		if( protocal->GetInterlaceCheckProtocal().reportFileNameSuffix.length()>0 )
		{
			ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			ReportFileName.append( protocal->GetInterlaceCheckProtocal().reportFileNameSuffix );			
		}

		InterlaceChecker = InterlaceCheckerType::New();
		InterlaceChecker->SetInput( dwi );
		InterlaceChecker->SetCorrelationThresholdBaseline( protocal->GetInterlaceCheckProtocal().correlationThresholdBaseline );
		InterlaceChecker->SetCorrelationThresholdGradient( protocal->GetInterlaceCheckProtocal().correlationThresholdGradient );
		InterlaceChecker->SetCorrelationStedvTimesBaseline( protocal->GetInterlaceCheckProtocal().correlationDeviationBaseline );
		InterlaceChecker->SetCorrelationStdevTimesGradient( protocal->GetInterlaceCheckProtocal().correlationDeviationGradient );
		InterlaceChecker->SetTranslationThreshold( protocal->GetInterlaceCheckProtocal().translationThreshold );
		InterlaceChecker->SetRotationThreshold( protocal->GetInterlaceCheckProtocal().rotationThreshold );
		InterlaceChecker->SetReportFileName( ReportFileName );
		InterlaceChecker->SetReportFileMode( protocal->GetInterlaceCheckProtocal().reportFileMode );

		try
		{
			InterlaceChecker->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;
			return -1;
		}

		DwiImageTemp = InterlaceChecker->GetOutput();

		for( unsigned int i=0; i< InterlaceChecker->GetGradientDirectionContainer()->size(); i++)
		{
			for( unsigned int j=0; j< this->qcResult->GetIntensityMotionCheckResult().size(); j++)
			{
				if( InterlaceChecker->GetGradientDirectionContainer()->at(i)[0] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0] &&
					InterlaceChecker->GetGradientDirectionContainer()->at(i)[1] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1] &&
					InterlaceChecker->GetGradientDirectionContainer()->at(i)[2] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
				{
					if( this->qcResult->GetIntensityMotionCheckResult()[j].processing != QCResult::GRADIENT_INCLUDE)
					{
						std::cout<< "gradient " << i << "has been excluded!" <<std::endl;

					}
					else
					{
						if(	!InterlaceChecker->getQCResults()[i])
							this->qcResult->GetIntensityMotionCheckResult()[i].processing = QCResult::GRADIENT_EXCLUDE_INTERLACECHECK;
					}
				}
			}
		}

		if( protocal->GetInterlaceCheckProtocal().outputDWIFileNameSuffix.length()>0 )
		{
			std::string outputDWIFileName;
			outputDWIFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			outputDWIFileName.append( protocal->GetInterlaceCheckProtocal().outputDWIFileNameSuffix );			

			try
			{
				std::cout<< "Saving output of interlace check: "<< outputDWIFileName <<" ... ";
				DwiWriter->SetImageIO(NrrdImageIO);
				DwiWriter->SetFileName( outputDWIFileName );
				DwiWriter->SetInput( DwiImageTemp );
				DwiWriter->UseCompressionOn();
				DwiWriter->Update();
			}
			catch(itk::ExceptionObject & e)
			{
				std::cout<< e.GetDescription()<<std::endl;
				return -1;
			}
			std::cout<< "DONE"<<std::endl;
		}
	}
	else
	{
		std::cout<<"Interlace-wise check NOT set."<<std::endl;
	}

	return true;
}

bool CIntensityMotionCheck::BaselineAverage( )
{
	DwiImageType::Pointer dwi = DwiImageTemp;

	if( protocal->GetBaselineAverageProtocal().bAverage )
	{
		std::string ReportFileName;
		if( protocal->GetBaselineAverageProtocal().reportFileNameSuffix.length()>0 )
		{
			ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			ReportFileName.append( protocal->GetBaselineAverageProtocal().reportFileNameSuffix );			
		}

		BaselineAverager	= BaselineAveragerType::New();

		BaselineAverager->SetInput( dwi );
		BaselineAverager->SetReportFileName( ReportFileName );
		BaselineAverager->SetReportFileMode(protocal->GetBaselineAverageProtocal().reportFileMode);
		BaselineAverager->SetAverageMethod( protocal->GetBaselineAverageProtocal().averageMethod );
		BaselineAverager->SetStopThreshold( protocal->GetBaselineAverageProtocal().stopThreshold );

		try
		{
			BaselineAverager->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;
			return -1;
		}

		DwiImageTemp = BaselineAverager->GetOutput();

		for( unsigned int j=0; j< this->qcResult->GetIntensityMotionCheckResult().size(); j++)
		{
			if( 0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0] &&
				0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1] &&
				0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
			{
				if( this->qcResult->GetIntensityMotionCheckResult()[j].processing == QCResult::GRADIENT_INCLUDE)
				{
					this->qcResult->GetIntensityMotionCheckResult()[j].processing = QCResult::GRADIENT_BASELINE_AVERAGED;
				}
			}
		}

		if( protocal->GetBaselineAverageProtocal().outputDWIFileNameSuffix.length()>0 )
		{
			std::string outputDWIFileName;
			outputDWIFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			outputDWIFileName.append( protocal->GetBaselineAverageProtocal().outputDWIFileNameSuffix );			

			try
			{
				std::cout<< "Saving output of baseline average: "<< outputDWIFileName <<" ... ";
				DwiWriter->SetImageIO(NrrdImageIO);
				DwiWriter->SetFileName( outputDWIFileName );
				DwiWriter->SetInput( DwiImageTemp );
				DwiWriter->UseCompressionOn();
				DwiWriter->Update();
			}
			catch(itk::ExceptionObject & e)
			{
				std::cout<< e.GetDescription()<<std::endl;
				return -1;
			}
			std::cout<< "DONE"<<std::endl;
		}
	}
	else
	{
		std::cout<<"Baseline average NOT set."<<std::endl;
	}

	return true;
}


bool CIntensityMotionCheck::EddyMotionCorrect( )
{
	DwiImageType::Pointer dwi = DwiImageTemp;

	if( protocal->GetEddyMotionCorrectionProtocal().bCorrect )
	{
		std::string ReportFileName;
		if( protocal->GetEddyMotionCorrectionProtocal().reportFileNameSuffix.length()>0 )
		{
			ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			ReportFileName.append( protocal->GetEddyMotionCorrectionProtocal().reportFileNameSuffix );			
		}

		EddyMotionCorrector = EddyMotionCorrectorType::New();
		EddyMotionCorrector->SetInput( dwi);
		EddyMotionCorrector->SetReportFileName( ReportFileName );
		EddyMotionCorrector->SetReportFileMode( protocal->GetEddyMotionCorrectionProtocal().reportFileMode );

		EddyMotionCorrector->SetNumberOfBins(protocal->GetEddyMotionCorrectionProtocal().numberOfBins );
		EddyMotionCorrector->SetSamples( protocal->GetEddyMotionCorrectionProtocal().numberOfSamples );
		EddyMotionCorrector->SetTranslationScale( protocal->GetEddyMotionCorrectionProtocal().translationScale );
		EddyMotionCorrector->SetStepLength(protocal->GetEddyMotionCorrectionProtocal().stepLength );
		EddyMotionCorrector->SetFactor( protocal->GetEddyMotionCorrectionProtocal().relaxFactor );
		EddyMotionCorrector->SetMaxNumberOfIterations( protocal->GetEddyMotionCorrectionProtocal().maxNumberOfIterations );

		try
		{
			EddyMotionCorrector->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;			
		}

		this->DwiImageTemp = EddyMotionCorrector->GetOutput();

		for( unsigned int i=0; i< EddyMotionCorrector->GetOutputGradientDirectionContainer()->size(); i++)
		{
			for( unsigned int j=0; j< this->qcResult->GetIntensityMotionCheckResult().size(); j++)
			{
				if( 0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0] &&
					0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1] &&
					0.0 ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
				{
					this->qcResult->GetIntensityMotionCheckResult()[j].processing = QCResult::GRADIENT_BASELINE_AVERAGED;
					continue;
				}

				if( EddyMotionCorrector->GetFeedinGradientDirectionContainer()->at(i)[0] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0] &&
					EddyMotionCorrector->GetFeedinGradientDirectionContainer()->at(i)[1] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1] &&
					EddyMotionCorrector->GetFeedinGradientDirectionContainer()->at(i)[2] ==  this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
				{
					if( this->qcResult->GetIntensityMotionCheckResult()[j].processing > QCResult::GRADIENT_EDDY_MOTION_CORRECTED) //GRADIENT_EXCLUDE_SLICECHECK,
																														//GRADIENT_EXCLUDE_INTERLACECHECK,
																														//GRADIENT_EXCLUDE_GRADIENTCHECK,
																														//GRADIENT_EXCLUDE_MANUALLY,
					{
						std::cout<< "gradient " << i << "has been excluded!" <<std::endl;
					}
					else
					{
						this->qcResult->GetIntensityMotionCheckResult()[j].processing = QCResult::GRADIENT_EDDY_MOTION_CORRECTED;
						this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[0] = EddyMotionCorrector->GetOutputGradientDirectionContainer()->at(i)[0];
						this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[1] = EddyMotionCorrector->GetOutputGradientDirectionContainer()->at(i)[1];
						this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[2] = EddyMotionCorrector->GetOutputGradientDirectionContainer()->at(i)[2];
					}
				}
			}
		}

		if( protocal->GetGradientCheckProtocal().outputDWIFileNameSuffix.length()>0 )
		{
			std::string outputDWIFileName;
			outputDWIFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			outputDWIFileName.append( protocal->GetEddyMotionCorrectionProtocal().outputDWIFileNameSuffix );			

			try
			{
				std::cout<< "Saving output of eddy current motion correction: "<< outputDWIFileName <<" ... ";
				DwiWriter->SetImageIO(NrrdImageIO);
				DwiWriter->SetFileName( outputDWIFileName );
				DwiWriter->SetInput( DwiImageTemp);
				DwiWriter->UseCompressionOn();
				DwiWriter->Update();
			}
			catch(itk::ExceptionObject & e)
			{
				std::cout<< e.GetDescription()<<std::endl;
			}
			std::cout<< "DONE"<<std::endl;
		}
	}
	else
	{
		std::cout<<"Eddy-current and head motion correction NOT set."<<std::endl;
	}
	return true;
}


bool CIntensityMotionCheck::GradientWiseCheck( )
{
	DwiImageType::Pointer dwi = DwiImageTemp;

	if( protocal->GetGradientCheckProtocal().bCheck )
	{
		std::string ReportFileName;
		if( protocal->GetGradientCheckProtocal().reportFileNameSuffix.length()>0 )
		{
			ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			ReportFileName.append( protocal->GetGradientCheckProtocal().reportFileNameSuffix );			
		}

		GradientChecker = GradientCheckerType::New();
		GradientChecker->SetInput( dwi);
		GradientChecker->SetTranslationThreshold( protocal->GetGradientCheckProtocal().translationThreshold );
		GradientChecker->SetRotationThreshold( protocal->GetGradientCheckProtocal().rotationThreshold );
		GradientChecker->SetReportFileName( ReportFileName );
		GradientChecker->SetReportFileMode( protocal->GetGradientCheckProtocal().reportFileMode );

		try
		{
			GradientChecker->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;
			return -1;
		}

		DwiImageTemp = GradientChecker->GetOutput();

		for( unsigned int i=0; i< GradientChecker->GetGradientDirectionContainer()->size(); i++)
		{
			for( unsigned int j=0; j< this->qcResult->GetIntensityMotionCheckResult().size(); j++)
			{
				if( fabs(GradientChecker->GetGradientDirectionContainer()->at(i)[0] - this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[0]) < 0.000001 &&
					fabs(GradientChecker->GetGradientDirectionContainer()->at(i)[1] - this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[1]) < 0.000001 &&
					fabs(GradientChecker->GetGradientDirectionContainer()->at(i)[2] - this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[2]) < 0.000001    )
				{
					if( this->qcResult->GetIntensityMotionCheckResult()[j].processing > QCResult::GRADIENT_EDDY_MOTION_CORRECTED )
					{
						std::cout<< "gradient " << j << "has been excluded!" <<std::endl;

					}
					else
					{
						if(	!GradientChecker->getQCResults()[i])
							this->qcResult->GetIntensityMotionCheckResult()[j].processing = QCResult::GRADIENT_EXCLUDE_GRADIENTCHECK;
					}
				}
			}
		}

		if( protocal->GetGradientCheckProtocal().outputDWIFileNameSuffix.length()>0 )
		{
			std::string outputDWIFileName;
			outputDWIFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			outputDWIFileName.append( protocal->GetGradientCheckProtocal().outputDWIFileNameSuffix );			

			try
			{
				std::cout<< "Saving output of gradient check: "<< outputDWIFileName <<" ... ";
				DwiWriter->SetImageIO(NrrdImageIO);
				DwiWriter->SetFileName( outputDWIFileName );
				DwiWriter->SetInput( DwiImageTemp);
				DwiWriter->UseCompressionOn();
				DwiWriter->Update();
			}
			catch(itk::ExceptionObject & e)
			{
				std::cout<< e.GetDescription()<<std::endl;
				return -1;
			}
			std::cout<< "DONE"<<std::endl;
		}
	}
	else
	{
		std::cout<<"Gradient-wise check NOT set."<<std::endl;
	}
	return true;
}



bool CIntensityMotionCheck::SaveQCedDWI()
{
	DwiImageType::Pointer dwi = DwiImageTemp;

 	if( protocal->GetQCedDWIFileNameSuffix().length()>0 )
 	{
 		std::string outputDWIFileName;
 		outputDWIFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
 		outputDWIFileName.append( protocal->GetQCedDWIFileNameSuffix());	
 		try
 		{
 			DwiWriter->SetImageIO(NrrdImageIO);
 			DwiWriter->SetFileName( outputDWIFileName );
 			DwiWriter->SetInput( dwi );
 			DwiWriter->UseCompressionOn();
 			DwiWriter->Update();
 		}
 		catch(itk::ExceptionObject & e)
 		{
 			std::cout<< e.GetDescription()<<std::endl;
 			return false;
 		}
 	}
	return true;
}
void CIntensityMotionCheck::collectLeftDiffusionStatistics( int dumb)
{
	DwiImageType::Pointer dwi = DwiImageTemp;

	std::string reportfilename;
	reportfilename=DwiFileName.substr(0,DwiFileName.find_last_of('.') );

	if( protocal->GetReportFileNameSuffix().length() > 0)
		reportfilename.append( protocal->GetReportFileNameSuffix() );	
	else
		reportfilename.append( "_QC_CheckReports.txt");

	std::vector<DiffusionDir> DiffusionDirections;
	DiffusionDirections.clear();

	for( unsigned int i=0; i< this->GradientDirectionContainer->size();i++) 
	{
		if(DiffusionDirections.size()>0)
		{
			bool newDir = true;
			for(unsigned int j=0;j<DiffusionDirections.size();j++)
			{
				if( this->GradientDirectionContainer->ElementAt(i)[0] == DiffusionDirections[j].gradientDir[0] && 
					this->GradientDirectionContainer->ElementAt(i)[1] == DiffusionDirections[j].gradientDir[1] && 
					this->GradientDirectionContainer->ElementAt(i)[2] == DiffusionDirections[j].gradientDir[2] )
				{
					if(qcResult->GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_INCLUDE)
						DiffusionDirections[j].repetitionNumber++;
					newDir = false;;
				}
			}
			if(newDir)
			{
				std::vector< double > dir;
				dir.push_back(this->GradientDirectionContainer->ElementAt(i)[0]);
				dir.push_back(this->GradientDirectionContainer->ElementAt(i)[1]);
				dir.push_back(this->GradientDirectionContainer->ElementAt(i)[2]);

				DiffusionDir diffusionDir;
				diffusionDir.gradientDir = dir;
				if(qcResult->GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_INCLUDE)
					diffusionDir.repetitionNumber=1;
				else
					diffusionDir.repetitionNumber=0;

				DiffusionDirections.push_back(diffusionDir);
			}
		}
		else
		{
			std::vector< double > dir;
			dir.push_back(this->GradientDirectionContainer->ElementAt(i)[0]);
			dir.push_back(this->GradientDirectionContainer->ElementAt(i)[1]);
			dir.push_back(this->GradientDirectionContainer->ElementAt(i)[2]);

			DiffusionDir diffusionDir;
			diffusionDir.gradientDir = dir;
			if(qcResult->GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_INCLUDE)
				diffusionDir.repetitionNumber=1;
			else
				diffusionDir.repetitionNumber=0;

			DiffusionDirections.push_back(diffusionDir);
		}
	}

	// 	std::cout<<"DiffusionDirections.size(): " << DiffusionDirections.size() <<std::endl;

	//std::vector<int> repetNum;
	//repetNum.clear();
	std::vector<double> dirMode;
	dirMode.clear();

	this->baselineLeftNumber=0;
	for( unsigned int i=0; i<DiffusionDirections.size(); i++)
	{
		if( DiffusionDirections[i].gradientDir[0] == 0.0 &&
			DiffusionDirections[i].gradientDir[1] == 0.0 &&
			DiffusionDirections[i].gradientDir[2] == 0.0 ) 
		{
			this->baselineLeftNumber = DiffusionDirections[i].repetitionNumber;
			// std::cout<<"DiffusionDirections[i].repetitionNumber: " <<i<<"  "<<DiffusionDirections[i].repetitionNumber <<std::endl;
		}
		else
		{
			this->repetitionLeftNumber.push_back(DiffusionDirections[i].repetitionNumber);

			double modeSqr =	DiffusionDirections[i].gradientDir[0]*DiffusionDirections[i].gradientDir[0] +
				DiffusionDirections[i].gradientDir[1]*DiffusionDirections[i].gradientDir[1] +
				DiffusionDirections[i].gradientDir[2]*DiffusionDirections[i].gradientDir[2];

			// 	std::cout<<"modeSqr: " <<modeSqr <<std::endl;
			if( dirMode.size() > 0)
			{
				bool newDirMode = true;
				for(unsigned int j=0;j< dirMode.size();j++)
				{
					if( fabs(modeSqr-dirMode[j])<0.001)   // 1 DIFFERENCE for b value
					{
						newDirMode = false;	
						break;
					}
				}
				if( newDirMode && DiffusionDirections[i].repetitionNumber>0 )
				{
					dirMode.push_back(	modeSqr) ;
					// 					std::cout<<" if(newDirMode) dirMode.size(): " <<  dirMode.size() <<std::endl;
				}
			}
			else
			{
				if(DiffusionDirections[i].repetitionNumber>0)
					dirMode.push_back(	modeSqr) ;
				//std::cout<<" else dirMode.size(): " <<  dirMode.size() <<std::endl;
			}
		}
	}

	// 	std::cout<<" repetNum.size(): " <<  repetNum.size() <<std::endl;
	// 	std::cout<<" dirMode.size(): " <<  dirMode.size() <<std::endl;

	this->gradientDirLeftNumber = 0;
	this->gradientLeftNumber = 0;
	for(unsigned int i=0; i<this->repetitionLeftNumber.size(); i++)
	{
		this->gradientLeftNumber+=this->repetitionLeftNumber[i];
		if(this->repetitionLeftNumber[i]>0)
			this->gradientDirLeftNumber++;
	}
		
	this->bValueLeftNumber = dirMode.size();

	std::ofstream outfile; 
	outfile.open(ReportFileName.c_str(), std::ios::app);
	outfile<<"--------------------------------"<<std::endl;
	outfile<<"Diffusion Gradient information:"<<std::endl;

	std::cout<<"Left DWI Diffusion: "	<<std::endl;
	std::cout<<"\tbaselineLeftNumber: "	<<baselineLeftNumber	<<std::endl;
	std::cout<<"\tbValueLeftNumber: "	<<bValueLeftNumber		<<std::endl;
	std::cout<<"\tgradientDirLeftNumber: "<<gradientDirLeftNumber	<<std::endl;

	outfile<<"Left DWI Diffusion: "		<<std::endl;
	outfile<<"\tbaselineLeftNumber: "	<<baselineLeftNumber	<<std::endl;
	outfile<<"\tbValueLeftNumber: "		<<bValueLeftNumber		<<std::endl;
	outfile<<"\tgradientDirLeftNumber: "<<gradientDirLeftNumber	<<std::endl;

	for(unsigned int i=0;i< DiffusionDirections.size();i++)
	{
		std::cout<<"\t["	<<DiffusionDirections[i].gradientDir[0]<<", "
			<<DiffusionDirections[i].gradientDir[1]<<", "
			<<DiffusionDirections[i].gradientDir[2]<<"] "
			<<"repetitionLeftNumber: "	<<DiffusionDirections[i].repetitionNumber <<std::endl;
		outfile<<"\t["	<<DiffusionDirections[i].gradientDir[0]<<", "
			<<DiffusionDirections[i].gradientDir[1]<<", "
			<<DiffusionDirections[i].gradientDir[2]<<"] "
			<<"repetitionLeftNumber: "	<<DiffusionDirections[i].repetitionNumber <<std::endl;
	}

	outfile.close();
	return;
}
