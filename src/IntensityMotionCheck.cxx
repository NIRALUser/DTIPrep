#include "IntensityMotionCheck.h"

#include "itkNrrdImageIO.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

#include "IntraGradientRigidRegistration.h"
#include "itkExtractImageFilter.h"
#include "RigidRegistration.h"

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
	bDwiLoaded=true;

	GetGridentDirections();
	if( bGetGridentDirections )
		collectDiffusionStatistics();
	else
	{
		std::cout<< "Diffusion information read error"<<std::endl;
		return false;
	}

	std::cout<<"Image size"<< DwiImage->GetLargestPossibleRegion().GetSize().GetSizeDimension()<<": ";
	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[0]<<" ";
	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[1]<<" ";
	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[2]<<std::endl;

	this->numGradients = DwiImage->GetVectorLength();
	std::cout<<"Pixel Vector Length: "<<DwiImage->GetVectorLength()<<std::endl;

	// Create result
	GradientIntensityMotionCheckResult result;
	
	result.bSliceCheckOK = true;
	for(unsigned int i=1; i<DwiImage->GetLargestPossibleRegion().GetSize()[2]; i++)
	{
		result.sliceCorrelation.push_back(0.0);
	}
	//std::cout<<"result.sliceCorrelation.size(): "<< result.sliceCorrelation.size()<<std::endl;

	result.bInterlaceCheckOK	= true;
	result.interlaceCorrelation	= 0.0;
	result.interlaceRotationX	= 0.0;
	result.interlaceRotationY	= 0.0;
	result.interlaceRotationZ	= 0.0;
	result.interlaceTranslationX= 0.0;
	result.interlaceTranslationY= 0.0;
	result.interlaceTranslationZ= 0.0;

	result.bGradientCheckOK		= true;
	result.gradientRotationX	= 0.0;
	result.gradientRotationY	= 0.0;
	result.gradientRotationZ	= 0.0;
	result.gradientTranslationX	= 0.0;
	result.gradientTranslationY	= 0.0;
	result.gradientTranslationZ	= 0.0;

	qcResult->Clear();

	for( unsigned int j = 0; j<DwiImage->GetVectorLength(); j++ )
	{
		qcResult->GetIntensityMotionCheckResult().push_back(result);
		qcResult->GetGradientProcess().push_back(QCResult::GRADIENT_INCLUDE);
	}
	//std::cout<<"qcResult.GetIntensityMotionCheckResult().size(): "<<qcResult->GetIntensityMotionCheckResult().size()<<std::endl;
	return true;
}

/*
bool CIntensityMotionCheck::GetGridentImages()
{
	if( !bDwiLoaded  ) LoadDwiImage();
	if( !bDwiLoaded  )
	{
		std::cout<<"DWI load error, no Gradient Images got"<<std::endl;
		bGetGridentImages=false;
		return false;
	}	

	GradientImageContainer.clear();
	typedef itk::VectorIndexSelectionCastImageFilter<DwiImageType, GradientImageType > FilterType;
	FilterType::Pointer componentExtractor = FilterType::New();
	componentExtractor->SetInput(DwiImage);

	std::cout<<"gradient images extrating ... ";
	for( unsigned int i = 0; i<DwiImage->GetVectorLength();i++ )
	{
		componentExtractor->SetIndex( i );
		componentExtractor->Update();
		GradientImageContainer.push_back( componentExtractor->GetOutput() );
		std::cout<<" "<<i;
	}

	std::cout<<std::endl<<"Totally "<<GradientImageContainer.size()<<" gradient images extrated"<<std::endl;
	for(int i=0;i<GradientImageContainer.size();i++)
	{
		//std::cout<<"Dimensions: "<< GradientImageContainer[i]->GetLargestPossibleRegion().GetSize().GetSizeDimension() <<std::endl;
		std::cout<<"  "<< GradientImageContainer[i]->GetLargestPossibleRegion().GetSize()[0] <<"\t";
		std::cout<< GradientImageContainer[i]->GetLargestPossibleRegion().GetSize()[1] <<"\t";
		std::cout<< GradientImageContainer[i]->GetLargestPossibleRegion().GetSize()[2] <<std::endl;
	}

		
	bGetGridentImages=true;
	return true;
}
*/
bool CIntensityMotionCheck::IntraCheck( )
{
	if(protocal->GetIntensityMotionCheckProtocal().bCheck &&  protocal->GetIntensityMotionCheckProtocal().bSliceCheck)
		return IntraCheck( 
		0, 
		protocal->GetIntensityMotionCheckProtocal().headSkipSlicePercentage,
		protocal->GetIntensityMotionCheckProtocal().tailSkipSlicePercentage,
		protocal->GetIntensityMotionCheckProtocal().baselineCorrelationThreshold,
		protocal->GetIntensityMotionCheckProtocal().baselineCorrelationDeviationThreshold,
		protocal->GetIntensityMotionCheckProtocal().sliceCorrelationThreshold, 
		protocal->GetIntensityMotionCheckProtocal().sliceCorrelationDeviationThreshold 
		);
	else
	{
		std::cout<<"Intra-gradient slice-wise check NOT set."<<std::endl;
		return true;
	}
}


bool CIntensityMotionCheck::IntraCheck(
									   bool bRegister,  
									   double beginSkip, 
									   double endSkip, 
									   double baselineCorrelationThreshold ,  
									   double baselineCorrelationDeviationThreshold, 
									   double CorrelationThreshold ,  
									   double CorrelationDeviationThreshold	
									   )
{
	if(!bDwiLoaded  ) LoadDwiImage();
	if(!bDwiLoaded  )
	{
		std::cout<<"DWI load error, no Gradient Images got"<<std::endl;
		return false;
	}	

	std::vector< std::vector<struIntra2DResults> >		ResultsContainer;

	typedef itk::VectorIndexSelectionCastImageFilter<DwiImageType, GradientImageType > FilterType;
	FilterType::Pointer componentExtractor = FilterType::New();
	componentExtractor->SetInput(DwiImage);

	typedef itk::ExtractImageFilter< GradientImageType, SliceImageType > ExtractFilterType;
	ExtractFilterType::Pointer filter1 = ExtractFilterType::New();
	ExtractFilterType::Pointer filter2 = ExtractFilterType::New();

	for( unsigned int j = 0; j<DwiImage->GetVectorLength(); j++ )
	{
		componentExtractor->SetIndex( j );
		componentExtractor->Update();

		GradientImageType::RegionType inputRegion =componentExtractor->GetOutput()->GetLargestPossibleRegion();
		GradientImageType::SizeType size = inputRegion.GetSize();
		size[2] = 0;

		GradientImageType::IndexType start1 = inputRegion.GetIndex();
		GradientImageType::IndexType start2 = inputRegion.GetIndex();

		std::cout<<std::endl;
		std::cout<<"Gradient "<<j<<std::endl;
		
		std::vector< struIntra2DResults >	Results;

		filter1->SetInput( componentExtractor->GetOutput() );
		filter2->SetInput( componentExtractor->GetOutput() );	

// 		std::vector< struIntra2DResults >	 corrEven;
// 		std::vector< struIntra2DResults >	 corrOdd;
// 
// 		for(int i=2; i<componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[2]; i+=2)
// 		{
// 			if(bRegister)
// 				std::cout<<"Total slice #: "<<componentExtractor->GetOutput()-> GetLargestPossibleRegion().GetSize()[2]<<",  Registering slice "<< i <<" to slice "<<i-1<<std::endl;
// 			else
// 				std::cout<<"Total slice #: "<<componentExtractor->GetOutput()-> GetLargestPossibleRegion().GetSize()[2]<<",  Metric for slice"<< i <<" to slice "<<i-1<<std::endl;
// 
// 			start1[2] = i-2;
// 			start2[2] = i;
// 
// 			GradientImageType::RegionType desiredRegion1;
// 			desiredRegion1.SetSize( size );
// 			desiredRegion1.SetIndex( start1 );
// 			filter1->SetExtractionRegion( desiredRegion1 );
// 
// 			GradientImageType::RegionType desiredRegion2;
// 			desiredRegion2.SetSize( size );
// 			desiredRegion2.SetIndex( start2 );
// 			filter2->SetExtractionRegion( desiredRegion2 );
// 
// 			filter1->Update();
// 			filter2->Update();
// 
// 			CIntraGradientRigidRegistration IntraGradientRigidReg(filter1->GetOutput(),filter2->GetOutput());
// 			struIntra2DResults s2DResults= IntraGradientRigidReg.Run( bRegister );
// 			corrEven.push_back(s2DResults);
// 		}
// 
// 		std::ofstream ofile; 
// 		ofile.open("/home/zhliu/corrForWang_EVEN.txt",  std::ios::app);
// 		for(int i=0;i<corrEven.size();i++ )
// 			ofile<<corrEven[i].Correlation<<std::endl;
// 
// 		ofile<<std::endl;
// 		ofile.close();
// 
// 		for(int i=3; i<componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[2]; i+=2)
// 		{
// 			if(bRegister)
// 				std::cout<<"Total slice #: "<<componentExtractor->GetOutput()-> GetLargestPossibleRegion().GetSize()[2]<<",  Registering slice "<< i <<" to slice "<<i-1<<std::endl;
// 			else
// 				std::cout<<"Total slice #: "<<componentExtractor->GetOutput()-> GetLargestPossibleRegion().GetSize()[2]<<",  Metric for slice"<< i <<" to slice "<<i-1<<std::endl;
// 
// 			start1[2] = i-2;
// 			start2[2] = i;
// 
// 			GradientImageType::RegionType desiredRegion1;
// 			desiredRegion1.SetSize( size );
// 			desiredRegion1.SetIndex( start1 );
// 			filter1->SetExtractionRegion( desiredRegion1 );
// 
// 			GradientImageType::RegionType desiredRegion2;
// 			desiredRegion2.SetSize( size );
// 			desiredRegion2.SetIndex( start2 );
// 			filter2->SetExtractionRegion( desiredRegion2 );
// 
// 			filter1->Update();
// 			filter2->Update();
// 
// 			CIntraGradientRigidRegistration IntraGradientRigidReg(filter1->GetOutput(),filter2->GetOutput());
// 			struIntra2DResults s2DResults= IntraGradientRigidReg.Run( bRegister );
// 			corrOdd.push_back(s2DResults);
// 		}
// 
// 		//std::ofstream ofile; 
// 		ofile.open("/home/zhliu/corrForWang_ODD.txt",  std::ios::app);
// 		for(int i=0;i<corrOdd.size();i++ )
// 			ofile<<corrOdd[i].Correlation<<std::endl;
// 
// 		ofile<<std::endl;
// 		ofile.close();


		for(unsigned int i=1; i<componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[2]; i++)
		{
			if(bRegister)
				std::cout<<"Total slice #: "<<componentExtractor->GetOutput()-> GetLargestPossibleRegion().GetSize()[2]<<",  Registering slice "<< i <<" to slice "<<i-1<<std::endl;
			else
				std::cout<<"Total slice #: "<<componentExtractor->GetOutput()-> GetLargestPossibleRegion().GetSize()[2]<<",  Metric for slice"<< i <<" to slice "<<i-1<<std::endl;

			start1[2] = i-1;
			start2[2] = i;

			GradientImageType::RegionType desiredRegion1;
			desiredRegion1.SetSize( size );
			desiredRegion1.SetIndex( start1 );
			filter1->SetExtractionRegion( desiredRegion1 );

			GradientImageType::RegionType desiredRegion2;
			desiredRegion2.SetSize( size );
			desiredRegion2.SetIndex( start2 );
			filter2->SetExtractionRegion( desiredRegion2 );

			filter1->Update();
			filter2->Update();

			CIntraGradientRigidRegistration IntraGradientRigidReg(filter1->GetOutput(),filter2->GetOutput());
			struIntra2DResults s2DResults= IntraGradientRigidReg.Run( bRegister );
			Results.push_back(s2DResults);
		}
		ResultsContainer.push_back(Results);
		//emit Progress(j+1/DwiImage->GetVectorLength());//emit QQQ(10);
	}

// copy slice correlation to qcResult
	for(unsigned int i=0; i< ResultsContainer.size(); i++)
	{
		for(unsigned int j=0; j<ResultsContainer[i].size(); j++)
		{
			qcResult->GetIntensityMotionCheckResult()[i].sliceCorrelation[j] = ResultsContainer[i][j].Correlation;
		}
	}

//print & save results
	std::ofstream outfile; 
	outfile.open(ReportFileName.c_str(),std::ios::app);
	outfile<<std::endl;
	outfile<<"==========================================================================="<<std::endl;
	outfile <<"Intra-gradient slice wise check with/without 2D rigid registration: "<<std::endl<<std::endl;

	outfile <<"Slice Correlations"<<std::endl;
	for(unsigned int i=0; i<ResultsContainer.size(); i++)
	{
		outfile <<"\t"<<"Gradient"<<i;
	}
	outfile <<"\tbaselineMean"<<"\tbaselineDeviation"<<"\tmean"<<"\tdeviation"<<std::endl;

	int DWICount, BaselineCount;
	BaselineCount	= baselineNumber;
	DWICount		= ResultsContainer.size()-baselineNumber;//gradientDirNumber * repitionNumber;

	std::cout <<"BaselineCount: "<<BaselineCount<<std::endl;
	std::cout <<"DWICount: "<<DWICount<<std::endl;

	for(unsigned int j=0;j<ResultsContainer[0].size();j++)
	{
		double baselinemean=0.0, DWImean=0.0, baselinedeviation=0.0, DWIdeviation=0.0;
		for(unsigned int i=0; i<ResultsContainer.size(); i++)
		{
			outfile.precision(6);
			outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
			outfile <<"\t"<<std::setw(10)<<ResultsContainer[i][j].Correlation;
			outfile.precision();
			outfile.setf(std::ios_base::unitbuf);
			if ( GradientDirectionContainer->at(i)[0] == 0.0 && GradientDirectionContainer->at(i)[1] == 0.0 && GradientDirectionContainer->at(i)[2] == 0.0 )
			{
				baselinemean+=ResultsContainer[i][j].Correlation/(double)(BaselineCount);
			}
			else
			{
				DWImean+=ResultsContainer[i][j].Correlation/(double)(DWICount);
			}				
		}

		for(unsigned int i=0; i<ResultsContainer.size(); i++)
		{
			if ( GradientDirectionContainer->at(i)[0] == 0.0 && GradientDirectionContainer->at(i)[1] == 0.0 && GradientDirectionContainer->at(i)[2] == 0.0)
			{
				if(BaselineCount>=1)
					baselinedeviation+=(ResultsContainer[i][j].Correlation-baselinemean)*(ResultsContainer[i][j].Correlation-baselinemean)/(double)(BaselineCount);
				else
					baselinedeviation=0.0;
			}
			else
			{
				if(DWICount>=1)
					DWIdeviation+=(ResultsContainer[i][j].Correlation-DWImean)*(ResultsContainer[i][j].Correlation-DWImean)/(double)(DWICount);
				else
					DWIdeviation=0.0;
			}
		}

		outfile <<"\t"<<std::setw(10)<<baselinemean;
		outfile <<"\t"<<std::setw(10)<<sqrt(baselinedeviation);
		outfile <<"\t"<<std::setw(10)<<DWImean;
		outfile <<"\t"<<std::setw(10)<<sqrt(DWIdeviation);
		outfile <<std::endl;

		this->baselineMeans.push_back(baselinemean);
		this->baselineDeviations.push_back(sqrt(baselinedeviation));

		this->means.push_back(DWImean);
		this->deviations.push_back(sqrt(DWIdeviation));
	}

// 	double baselineMeanStdev=0.0, gradientMeanStdev=0.0;
// 	unsigned int effectiveSliceNumber=0;
// 
// 	for(int j = 0 + (int)(DwiImage->GetLargestPossibleRegion().GetSize()[2]*beginSkip);j < ResultsContainer[0].size() - (int)(DwiImage->GetLargestPossibleRegion().GetSize()[2]*endSkip); j++ ) 
// 	{
// 		baselineMeanStdev += this->baselineDeviations[j];		
// 		gradientMeanStdev += this->deviations[j];	
// 		effectiveSliceNumber++;
// 	}
// 
// 	baselineMeanStdev = baselineMeanStdev/(double)effectiveSliceNumber;		
// 	gradientMeanStdev = gradientMeanStdev/(double)effectiveSliceNumber;	

// 	std::cout<<"effectiveSliceNumber: "<<effectiveSliceNumber<<std::endl;
// 	std::cout<<"baselineMeanStdev: "<<baselineMeanStdev<<std::endl;
// 	std::cout<<"gradientMeanStdev: "<<gradientMeanStdev<<std::endl;

	outfile <<std::endl<<"Intra-Gradient Slice Check Artifacts:"<<std::endl;
	outfile  <<"\t"<<std::setw(10)<<"Gradient"<<"\t"<<std::setw(10)<<"Slice"<<"\t"<<std::setw(10)<<"Correlation"<<std::endl;

	std::cout <<std::endl<<"Intra-Gradient Slice Check Artifacts:"<<std::endl;
	std::cout<<"\t"<<std::setw(10)<<"Gradient"<<"\t"<<std::setw(10)<<"Slice"<<"\t"<<std::setw(10)<<"Correlation"<<std::endl;
	
	int badcount=0;
	int baselineBadcount=0;

	for(unsigned int i=0;i<ResultsContainer.size();i++)
	{
		baselineBadcount = 0;
		badcount=0;
		if ( GradientDirectionContainer->at(i)[0] == 0.0 && GradientDirectionContainer->at(i)[1] == 0.0 && GradientDirectionContainer->at(i)[2] == 0.0 )
		{	
			//baseline
			for(unsigned int j = 0 + (int)(DwiImage->GetLargestPossibleRegion().GetSize()[2]*beginSkip);j < ResultsContainer[0].size() - (int)(DwiImage->GetLargestPossibleRegion().GetSize()[2]*endSkip); j++ ) 
			{
				outfile.precision(6);
				outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;

				double MeanStdev=0.0;
				unsigned int effectiveSliceNumber=0;

				for(unsigned int k = (j-3>0? j-3:0); k < (j+3<DwiImage->GetLargestPossibleRegion().GetSize()[2]? j+3:DwiImage->GetLargestPossibleRegion().GetSize()[2]) ; k++ ) 
				{
					MeanStdev += this->baselineDeviations[k];		
					effectiveSliceNumber++;
				}
				MeanStdev = MeanStdev/(double)effectiveSliceNumber;	

				double stddev=0.0;
				if( baselineDeviations[j]< MeanStdev )
					stddev = baselineDeviations[j];
				else
					stddev = MeanStdev;
				
				if(getBValueNumber()>1)
				{
					if( ResultsContainer[i][j].Correlation < baselineCorrelationThreshold ||  ResultsContainer[i][j].Correlation < baselineMeans[j] - baselineDeviations[j] * baselineCorrelationDeviationThreshold)
					{
						outfile  <<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<std::endl;
						std::cout<<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<std::endl;
						baselineBadcount++;
					}
				}
				else
				{	
					if( ResultsContainer[i][j].Correlation < baselineCorrelationThreshold ||  ResultsContainer[i][j].Correlation < baselineMeans[j] - stddev * baselineCorrelationDeviationThreshold)
					{
						outfile  <<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<std::endl;
						std::cout<<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<std::endl;
						baselineBadcount++;
					}
				}
// 				if( ResultsContainer[i][j].Correlation < baselineCorrelationThreshold ||  ResultsContainer[i][j].Correlation < baselineMeans[j] - baselineDeviations[j] * baselineCorrelationDeviationThreshold)
// 				//if( ResultsContainer[i][j].Correlation < baselineCorrelationThreshold ||  ResultsContainer[i][j].Correlation < baselineMeans[j] - stddev * baselineCorrelationDeviationThreshold)
// 				{
// 					outfile  <<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<std::endl;
// 					std::cout<<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<std::endl;
// 					baselineBadcount++;
// 				}
				outfile.precision();
				outfile.setf(std::ios_base::unitbuf);
			}

			if( baselineBadcount > (int)(protocal->GetIntensityMotionCheckProtocal().badSlicePercentageTolerance * DwiImage->GetLargestPossibleRegion().GetSize()[2]) )
			{
				qcResult->GetGradientProcess()[i] = QCResult::GRADIENT_EXCLUDE;
				qcResult->GetIntensityMotionCheckResult()[i].bSliceCheckOK = false;
			}
		}
		else // gradients
		{			
			for(unsigned int j=0 + (int)(DwiImage->GetLargestPossibleRegion().GetSize()[2]*beginSkip);j<ResultsContainer[0].size() - (int)(DwiImage->GetLargestPossibleRegion().GetSize()[2]*endSkip); j++) //for(int j=0;j<ResultsContainer[0].size()-1; j++)
			{
				outfile.precision(6);
				outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	

				double MeanStdev=0.0;
				unsigned int effectiveSliceNumber=0;

				for(unsigned int k = (j-3>0? j-3:0); k < ( j+3 < DwiImage->GetLargestPossibleRegion().GetSize()[2]? j+3:DwiImage->GetLargestPossibleRegion().GetSize()[2]) ; k++ ) 
				{
					MeanStdev += this->deviations[k];		
					effectiveSliceNumber++;
				}
				MeanStdev = MeanStdev/(double)effectiveSliceNumber;	

				double stddev=0.0;
				if( deviations[j]< MeanStdev )
					stddev = deviations[j];
				else
					stddev = MeanStdev;

				if(getBValueNumber()>1)
				{
					if( ResultsContainer[i][j].Correlation < CorrelationThreshold ||  ResultsContainer[i][j].Correlation < means[j] - deviations[j] * CorrelationDeviationThreshold) // ok
					{
						outfile  <<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<std::endl;
						std::cout<<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<std::endl;
						badcount++;
					}
				}
				else
				{
					if( ResultsContainer[i][j].Correlation < CorrelationThreshold ||  ResultsContainer[i][j].Correlation < means[j] - stddev * CorrelationDeviationThreshold) // ok
					{
						outfile  <<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<std::endl;
						std::cout<<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<std::endl;
						badcount++;
					}
				}
				outfile.precision();
				outfile.setf(std::ios_base::unitbuf);
			}
			if( badcount > (int)(protocal->GetIntensityMotionCheckProtocal().badSlicePercentageTolerance * DwiImage->GetLargestPossibleRegion().GetSize()[2]) )
			{
				qcResult->GetGradientProcess()[i]=QCResult::GRADIENT_EXCLUDE;
				qcResult->GetIntensityMotionCheckResult()[i].bSliceCheckOK = false;
			}
		}		
	}
	outfile.close();	
	return true;
}

void CIntensityMotionCheck::PrintResult()
{
/*	// slice wise check results
	std::ofstream outfile; 
	outfile.open(ReportFileName.c_str(),std::ios::app);
	outfile<<std::endl;
	outfile<<"==========================================================================="<<std::endl;
	outfile <<"Intra-gradient slice wise check with/without 2D rigid registration: "<<std::endl<<std::endl;

	outfile <<"Slice Correlations"<<std::endl;
	for(int i=0; i< qcResult->GetIntensityMotionCheckResult().size(); i++)
	{
		outfile <<"\t"<<"Gradient"<<i;
	}
	outfile <<"\tbaselineMean"<<"\tbaselineDeviation"<<"\tmean"<<"\tdeviation"<<std::endl;


	for(int j=0;j<qcResult->GetIntensityMotionCheckResult()[0].sliceCorrelation.size();j++)
	{
		for(int i=0; i<qcResult->GetIntensityMotionCheckResult().size(); i++)
		{
			outfile.precision(6);
			outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
			outfile <<"\t"<<std::setw(10)<<qcResult->GetIntensityMotionCheckResult()[i].sliceCorrelation[j];
			outfile.precision();
			outfile.setf(std::ios_base::unitbuf);
		}

		outfile <<"\t"<<std::setw(10)<< this->baselineMeans[j];
		outfile <<"\t"<<std::setw(10)<<sqrt(this->baselineDeviations[j]);
		outfile <<"\t"<<std::setw(10)<<this->means;
		outfile <<"\t"<<std::setw(10)<<sqrt(this->deviations);
		outfile <<std::endl;
	}

	outfile <<std::endl<<"Intra-Gradient Slice Check Artifacts:"<<std::endl;
	outfile  <<"\t"<<std::setw(10)<<"Gradient"<<"\t"<<std::setw(10)<<"Slice"<<"\t"<<std::setw(10)<<"Correlation"<<std::endl;
	std::cout <<std::endl<<"Intra-Gradient Slice Check Artifacts:"<<std::endl;
	std::cout<<"\t"<<std::setw(10)<<"Gradient"<<"\t"<<std::setw(10)<<"Slice"<<"\t"<<std::setw(10)<<"Correlation"<<std::endl;

	int badcount=0;
	int baselineBadcount=0;

	for(int i=0;i<ResultsContainer.size();i++)
	{
		baselineBadcount = 0;
		badcount=0;
		if ( GradientDirectionContainer->at(i)[0] == 0.0 && GradientDirectionContainer->at(i)[1] == 0.0 && GradientDirectionContainer->at(i)[2] == 0.0 )
		{	
			//baseline
			for(int j = 0 + (int)(DwiImage->GetLargestPossibleRegion().GetSize()[2]*beginSkip);j < ResultsContainer[0].size() - (int)(DwiImage->GetLargestPossibleRegion().GetSize()[2]*endSkip); j++ ) 
			{
				outfile.precision(6);
				outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
				if( ResultsContainer[i][j].Correlation < baselineCorrelationThreshold ||  ResultsContainer[i][j].Correlation < baselineMeans[j] - baselineDeviations[j] * baselineCorrelationDeviationThreshold)
				{
					outfile  <<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<std::endl;
					std::cout<<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<std::endl;
					baselineBadcount++;
				}
				outfile.precision();
				outfile.setf(std::ios_base::unitbuf);
			}

			if( baselineBadcount > (int)(protocal->GetIntensityMotionCheckProtocal().badSlicePercentageTolerance * DwiImage->GetLargestPossibleRegion().GetSize()[2]) )
			{
				qcResult->GetGradientProcess()[i]=QCResult::GRADIENT_EXCLUDE;
				qcResult->GetIntensityMotionCheckResult()[i].bSliceCheckOK = false;
			}
		}
		else // gradients
		{			
			for(int j=0 + (int)(DwiImage->GetLargestPossibleRegion().GetSize()[2]*beginSkip);j<ResultsContainer[0].size() - (int)(DwiImage->GetLargestPossibleRegion().GetSize()[2]*endSkip); j++) //for(int j=0;j<ResultsContainer[0].size()-1; j++)
			{
				outfile.precision(6);
				outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
				//if( ResultsContainer[i][j].Correlation < CorrelationThreshold &&  ResultsContainer[i][j+1].Correlation < CorrelationThreshold)
				if( ResultsContainer[i][j].Correlation < CorrelationThreshold ||  ResultsContainer[i][j].Correlation < means[j] - deviations[j] * CorrelationDeviationThreshold)
				{
					outfile  <<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<std::endl;
					std::cout<<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<std::endl;
					badcount++;
				}
				outfile.precision();
				outfile.setf(std::ios_base::unitbuf);
			}
			if( badcount > (int)(protocal->GetIntensityMotionCheckProtocal().badSlicePercentageTolerance * DwiImage->GetLargestPossibleRegion().GetSize()[2]) )
			{
				qcResult->GetGradientProcess()[i]=QCResult::GRADIENT_EXCLUDE;
				qcResult->GetIntensityMotionCheckResult()[i].bSliceCheckOK = false;
			}
		}		
	}



	outfile.close();	
*/

}


bool CIntensityMotionCheck::InterlaceCheck()
{
	if(protocal->GetIntensityMotionCheckProtocal().bCheck &&  protocal->GetIntensityMotionCheckProtocal().bInterlaceCheck)
		return InterlaceCheck(	protocal->GetIntensityMotionCheckProtocal().interlaceRotationThreshold,
								protocal->GetIntensityMotionCheckProtocal().interlaceTranslationThreshold,
								protocal->GetIntensityMotionCheckProtocal().interlaceCorrelationThresholdBaseline,
								protocal->GetIntensityMotionCheckProtocal().interlaceCorrelationThresholdGradient,
								protocal->GetIntensityMotionCheckProtocal().interlaceCorrelationDeviationBaseline,
								protocal->GetIntensityMotionCheckProtocal().interlaceCorrelationDeviationGradient
								);
	else
	{
		std::cout<<"Intra-gradient interlace-wise check NOT set."<<std::endl;
		return true;
	}
}


bool CIntensityMotionCheck::InterlaceCheck(
	double angleThreshold, 
	double transThreshold, 
	double correlationThresholdBaseline, 
	double correlationThresholdGradient,
	double corrBaselineDev,	
	double corrGradientDev)
{
	if(!bDwiLoaded ) LoadDwiImage();
	if(!bDwiLoaded )
	{
		std::cout<<"DWI load error, no Gradient Images got"<<std::endl;
		return false;
	}	
	
	typedef itk::VectorIndexSelectionCastImageFilter<DwiImageType, GradientImageType > FilterType;
	FilterType::Pointer componentExtractor = FilterType::New();

	componentExtractor->SetInput(DwiImage);

	GradientImageType::Pointer InterlaceOdd  = GradientImageType::New();
	GradientImageType::Pointer InterlaceEven = GradientImageType::New();

	componentExtractor->SetIndex( 0 );
	componentExtractor->Update();
	
	GradientImageType::RegionType region;
	GradientImageType::SizeType size;
	size[0]= componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[0];
	size[1]= componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[1];
	size[2]= componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[2]/2;
	region.SetSize( size );

	GradientImageType::SpacingType spacing;
	spacing = componentExtractor->GetOutput()->GetSpacing();

	InterlaceOdd->CopyInformation(componentExtractor->GetOutput());
	InterlaceEven->CopyInformation(componentExtractor->GetOutput());

	InterlaceOdd->SetRegions( region );
	InterlaceEven->SetRegions( region );

	InterlaceOdd->Allocate();
	InterlaceEven->Allocate();

	typedef itk::ImageRegionIteratorWithIndex< GradientImageType > IteratorType;
	IteratorType iterateOdd( InterlaceOdd, InterlaceOdd->GetLargestPossibleRegion() );
	IteratorType iterateEven( InterlaceEven, InterlaceEven->GetLargestPossibleRegion() );

	std::vector< struInterlaceResults >	Results;
	for( unsigned int j = 0; j<DwiImage->GetVectorLength(); j++ )
	{
		componentExtractor->SetIndex( j );
		componentExtractor->Update();

		typedef itk::ImageRegionIteratorWithIndex< GradientImageType > IteratorType;
		IteratorType iterateGradient( componentExtractor->GetOutput(), componentExtractor->GetOutput()->GetLargestPossibleRegion() );

		iterateGradient.GoToBegin();
		iterateOdd.GoToBegin();
		iterateEven.GoToBegin();

		unsigned long count=0;
		while (!iterateGradient.IsAtEnd())
		{
			if(count<size[0]*size[1]*size[2]*2)
			{
				if( (count/(size[0]*size[1]))%2 == 0)	
				{
					iterateEven.Set(iterateGradient.Get());
					//std::cout<<"\tgradient "<<j<<"\tcreate even "<<count<<std::endl;
					++iterateEven;
				}
				if( (count/(size[0]*size[1]))%2 == 1)
				{
					iterateOdd.Set(iterateGradient.Get());
					//std::cout<<"\tgradient "<<j<<"\tcreate odd "<<count<<std::endl;
					++iterateOdd;
				}
			}
			++iterateGradient;
			++count;			
		}

		std::cout<<"Gradient "<<j<<std::endl;

		CRigidRegistration RigidRegistration(InterlaceOdd,InterlaceEven,25, 0.1, 1 );
		Results.push_back(RigidRegistration.Run());

//////////////////////////////////////////////////////
// to calculate correlation between interlaces  /////
//////////////////////////////////////////////////////
		
		typedef itk::ImageRegionConstIterator<GradientImageType>  citType;
		citType cit1(InterlaceOdd, InterlaceOdd ->GetBufferedRegion() );
		citType cit2(InterlaceEven,InterlaceEven->GetBufferedRegion() );

		cit1.GoToBegin();
		cit2.GoToBegin();

		double Correlation;
		double sAB=0.0,sA2=0.0,sB2=0.0;
		while (!cit1.IsAtEnd())
		{
			sAB+=cit1.Get()*cit2.Get();
			sA2+=cit1.Get()*cit1.Get();
			sB2+=cit2.Get()*cit2.Get();
			++cit1;
			++cit2;
		}
		Correlation=sAB/sqrt(sA2*sB2);
		Results[j].Correlation=Correlation;

		std::cout<<"Angles: "<<Results[j].AngleX<<" "<<Results[j].AngleY<<" "<<Results[j].AngleZ<<std::endl;
		std::cout<<"Trans : "<<Results[j].TranslationX<<" "<<Results[j].TranslationY<<" "<<Results[j].TranslationZ<<std::endl;
		std::cout<<"MI    : "<<Results[j].MutualInformation<<std::endl;
		std::cout<<"Correlation: " <<Results[j].Correlation << std::endl;
	}

	// copy interlace results to qcResult
	for(unsigned int i=0; i< Results.size(); i++)
	{
		qcResult->GetIntensityMotionCheckResult()[i].interlaceCorrelation	= Results[i].Correlation;	
		qcResult->GetIntensityMotionCheckResult()[i].interlaceRotationX		= Results[i].AngleX;	
		qcResult->GetIntensityMotionCheckResult()[i].interlaceRotationY		= Results[i].AngleY;	
		qcResult->GetIntensityMotionCheckResult()[i].interlaceRotationZ		= Results[i].AngleZ;	
		qcResult->GetIntensityMotionCheckResult()[i].interlaceTranslationX	= Results[i].TranslationX;	
		qcResult->GetIntensityMotionCheckResult()[i].interlaceTranslationY	= Results[i].TranslationY;	
		qcResult->GetIntensityMotionCheckResult()[i].interlaceTranslationZ	= Results[i].TranslationZ;	
	}

// 	if(!bGetGridentDirections)
// 		GetGridentDirections();
	int DWICount, BaselineCount;

	BaselineCount= baselineNumber;
	DWICount=GradientDirectionContainer->size()-baselineNumber;
// 	for ( int i=0;i< GradientDirectionContainer->size() ;i++ )
// 	{
// 		if( GradientDirectionContainer->ElementAt(i)[0] == 0.0 && GradientDirectionContainer->ElementAt(i)[1] == 0.0 && GradientDirectionContainer->ElementAt(i)[2] == 0.0)
// 			BaselineCount++;
// 		else
// 			DWICount++;
// 	}

	std::cout<<"BaselineCount: "<<BaselineCount<<std::endl;
	std::cout<<"DWICount: "<<DWICount<<std::endl;

	interlaceBaselineMeans=0.0;
	interlaceBaselineDeviations=0.0;
	interlaceGradientMeans=0.0;
	interlaceGradientDeviations=0.0;

	for(unsigned int i=0; i< Results.size(); i++)
	{
		if ( GradientDirectionContainer->at(i)[0] == 0.0 && GradientDirectionContainer->at(i)[1] == 0.0 && GradientDirectionContainer->at(i)[2] == 0.0 )
		{	//for interlace baseline correlation
			interlaceBaselineMeans += Results[i].Correlation/(double)BaselineCount;			
		}
		else //for interlace gradient correlation
		{
			interlaceGradientMeans += Results[i].Correlation/(double)DWICount;
		}
	}

	for(unsigned int i=0; i<Results.size(); i++)
	{
		if ( GradientDirectionContainer->at(i)[0] == 0.0 && GradientDirectionContainer->at(i)[1] == 0.0 && GradientDirectionContainer->at(i)[2] == 0.0)
		{
			if(BaselineCount>=1)
				interlaceBaselineDeviations += ( Results[i].Correlation-interlaceBaselineMeans)*( Results[i].Correlation-interlaceBaselineMeans)/(double)(BaselineCount);
			else
				interlaceBaselineDeviations = 0.0;
		}
		else
		{
			if(DWICount>=1)
				interlaceGradientDeviations+=( Results[i].Correlation-interlaceGradientMeans)*( Results[i].Correlation-interlaceGradientMeans)/(double)(DWICount);
			else
				interlaceGradientDeviations = 0.0;
		}
	}

	interlaceBaselineDeviations = sqrt(interlaceBaselineDeviations);
	interlaceGradientDeviations = sqrt(interlaceGradientDeviations);

	// print & save results
	std::ofstream outfile; 
	outfile.open(ReportFileName.c_str(),std::ios::app);
	outfile<<std::endl;
	outfile<<"=================================================================================================="<<std::endl;
	outfile  <<"Intra-gradient interlace INTENSITY and MOTION check with MI-based 3D rigid registration: "<<std::endl;
	std::cout<<"Intra-gradient interlace INTENSITY and MOTION check with MI-based 3D rigid registration: "<<std::endl;

	std::cout <<"\t"<<std::setw(10)<<"Gradient#:\t"<<std::setw(10)<<"AngleX\t"<<std::setw(10)<<"AngleY\t"<<
			std::setw(10)<<"AngleZ\t"<<std::setw(10)<<"TranslationX\t"<<std::setw(10)<<"TranslationY\t"<<
			std::setw(10)<<"TranslationZ\t"<<std::setw(10)<<"Metric(MI)\t"<<std::setw(10)<<"Correlation"<<std::endl;
	
	outfile <<"\t"<<std::setw(10)<<"Gradient#:\t"<<std::setw(10)<<"AngleX\t"<<std::setw(10)<<"AngleY\t"<<
			std::setw(10)<<"AngleZ\t"<<std::setw(10)<<"TranslationX\t"<<std::setw(10)<<"TranslationY\t"<<
			std::setw(10)<<"TranslationZ\t"<<std::setw(10)<<"Metric(MI)\t"<<std::setw(10)<<"Correlation"<<std::endl;

	for(unsigned int i=0;i<Results.size();i++)
	{
		std::cout.precision(6);
		std::cout.setf(std::ios_base::showpoint|std::ios_base::right) ;	
		std::cout<<"\t"<<std::setw(10)<<i;
		std::cout<<"\t"<<std::setw(10)<<Results[i].AngleX;
		std::cout<<"\t"<<std::setw(10)<<Results[i].AngleY;
		std::cout<<"\t"<<std::setw(10)<<Results[i].AngleZ;
		std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationX;
		std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationY;
		std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationZ;
		std::cout<<"\t"<<std::setw(10)<<Results[i].MutualInformation;
		std::cout<<"\t"<<std::setw(10)<<Results[i].Correlation;
		std::cout<<std::endl;
		std::cout.precision();
		std::cout.setf(std::ios_base::unitbuf);

		outfile.precision(6);
		outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
		outfile<<"\t"<<std::setw(10)<<i;
		outfile<<"\t"<<std::setw(10)<<Results[i].AngleX;
		outfile<<"\t"<<std::setw(10)<<Results[i].AngleY;
		outfile<<"\t"<<std::setw(10)<<Results[i].AngleZ;
		outfile<<"\t"<<std::setw(10)<<Results[i].TranslationX;
		outfile<<"\t"<<std::setw(10)<<Results[i].TranslationY;
		outfile<<"\t"<<std::setw(10)<<Results[i].TranslationZ;
		outfile<<"\t"<<std::setw(10)<<Results[i].MutualInformation;
		outfile<<"\t"<<std::setw(10)<<Results[i].Correlation;
		outfile<<std::endl;
		outfile.precision();
		outfile.setf(std::ios_base::unitbuf);		
	}

	outfile<< "interlaceBaselineDeviations: " << interlaceBaselineDeviations<<std::endl;
	outfile<< "interlaceBaselineMeans: " << interlaceBaselineMeans<<std::endl;

	outfile<< "interlaceGradientDeviations: " << interlaceGradientDeviations<<std::endl;
	outfile<< "interlaceGradientMeans: " << interlaceGradientMeans<<std::endl;

	std::cout<< "interlaceBaselineDeviations: " << interlaceBaselineDeviations<<std::endl;
	std::cout<< "interlaceBaselineMeans: " << interlaceBaselineMeans<<std::endl;

	std::cout<< "interlaceGradientDeviations: " << interlaceGradientDeviations<<std::endl;
	std::cout<< "interlaceGradientMeans: " << interlaceGradientMeans<<std::endl;

	std::cout <<std::endl<<"Intra-Gradient Interlace Check Artifacts:"<<std::endl;
	outfile   <<std::endl<<"Intra-Gradient Interlace Check Artifacts:"<<std::endl;

	std::cout <<"\t"<<std::setw(10)<<"Gradient#:\t"<<std::setw(10)<<"AngleX\t"<<std::setw(10)<<"AngleY\t"<<
			std::setw(10)<<"AngleZ\t"<<std::setw(10)<<"TranslationX\t"<<std::setw(10)<<"TranslationY\t"<<
			std::setw(10)<<"TranslationZ\t"<<std::setw(10)<<"Metric(MI)\t"<<std::setw(10)<<"Correlation"<<std::endl;
	
	outfile <<"\t"<<std::setw(10)<<"Gradient#:\t"<<std::setw(10)<<"AngleX\t"<<std::setw(10)<<"AngleY\t"<<
			std::setw(10)<<"AngleZ\t"<<std::setw(10)<<"TranslationX\t"<<std::setw(10)<<"TranslationY\t"<<
			std::setw(10)<<"TranslationZ\t"<<std::setw(10)<<"Metric(MI)\t"<<std::setw(10)<<"Correlation"<<std::endl;


	for(unsigned int i=0;i<Results.size();i++)
	{
		if ( GradientDirectionContainer->at(i)[0] == 0.0 && GradientDirectionContainer->at(i)[1] == 0.0 && GradientDirectionContainer->at(i)[2] == 0.0)
		{	//baseline
			if( fabs(Results[i].AngleX) > angleThreshold	|| fabs(Results[i].AngleY) > angleThreshold ||
				fabs(Results[i].AngleZ) > angleThreshold	|| fabs(Results[i].TranslationX)>transThreshold || 
				fabs(Results[i].TranslationY)>transThreshold|| fabs(Results[i].TranslationZ-0.5*spacing[2])>transThreshold||
				Results[i].Correlation < correlationThresholdBaseline ||
				Results[i].Correlation < interlaceBaselineMeans - interlaceBaselineDeviations * corrBaselineDev ) 
			{
				std::cout.precision(6);
				std::cout.setf(std::ios_base::showpoint|std::ios_base::right) ;	
				std::cout<<"\t"<<std::setw(10)<<i;
				std::cout<<"\t"<<std::setw(10)<<Results[i].AngleX;
				std::cout<<"\t"<<std::setw(10)<<Results[i].AngleY;
				std::cout<<"\t"<<std::setw(10)<<Results[i].AngleZ;
				std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationX;
				std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationY;
				std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationZ;
				std::cout<<"\t"<<std::setw(10)<<Results[i].MutualInformation;
				std::cout<<"\t"<<std::setw(10)<<Results[i].Correlation;
				std::cout<<std::endl;
				std::cout.precision();
				std::cout.setf(std::ios_base::unitbuf);

				outfile.precision(6);
				outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
				outfile<<"\t"<<std::setw(10)<<i;
				outfile<<"\t"<<std::setw(10)<<Results[i].AngleX;
				outfile<<"\t"<<std::setw(10)<<Results[i].AngleY;
				outfile<<"\t"<<std::setw(10)<<Results[i].AngleZ;
				outfile<<"\t"<<std::setw(10)<<Results[i].TranslationX;
				outfile<<"\t"<<std::setw(10)<<Results[i].TranslationY;
				outfile<<"\t"<<std::setw(10)<<Results[i].TranslationZ;
				outfile<<"\t"<<std::setw(10)<<Results[i].MutualInformation;
				outfile<<"\t"<<std::setw(10)<<Results[i].Correlation;
				outfile<<std::endl;
				outfile.precision();
				outfile.setf(std::ios_base::unitbuf);	

				qcResult->GetGradientProcess()[i]=(QCResult::GRADIENT_EXCLUDE);
				qcResult->GetIntensityMotionCheckResult()[i].bInterlaceCheckOK = false;
			}
		}
		else // gradients
		{
			if( this->bValueNumber>1) // multiple b values
			{
				if(fabs(Results[i].AngleX) > angleThreshold || fabs(Results[i].AngleY) > angleThreshold ||
					fabs(Results[i].AngleZ) > angleThreshold	 ||fabs(Results[i].TranslationX)>transThreshold || 
					fabs(Results[i].TranslationY)>transThreshold || fabs(Results[i].TranslationZ-0.5*spacing[2])>transThreshold||
					Results[i].Correlation < correlationThresholdGradient) 
				{
					std::cout.precision(6);
					std::cout.setf(std::ios_base::showpoint|std::ios_base::right) ;	
					std::cout<<"\t"<<std::setw(10)<<i;
					std::cout<<"\t"<<std::setw(10)<<Results[i].AngleX;
					std::cout<<"\t"<<std::setw(10)<<Results[i].AngleY;
					std::cout<<"\t"<<std::setw(10)<<Results[i].AngleZ;
					std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationX;
					std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationY;
					std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationZ;
					std::cout<<"\t"<<std::setw(10)<<Results[i].MutualInformation;
					std::cout<<"\t"<<std::setw(10)<<Results[i].Correlation;
					std::cout<<std::endl;
					std::cout.precision();
					std::cout.setf(std::ios_base::unitbuf);

					outfile.precision(6);
					outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
					outfile<<"\t"<<std::setw(10)<<i;
					outfile<<"\t"<<std::setw(10)<<Results[i].AngleX;
					outfile<<"\t"<<std::setw(10)<<Results[i].AngleY;
					outfile<<"\t"<<std::setw(10)<<Results[i].AngleZ;
					outfile<<"\t"<<std::setw(10)<<Results[i].TranslationX;
					outfile<<"\t"<<std::setw(10)<<Results[i].TranslationY;
					outfile<<"\t"<<std::setw(10)<<Results[i].TranslationZ;
					outfile<<"\t"<<std::setw(10)<<Results[i].MutualInformation;
					outfile<<"\t"<<std::setw(10)<<Results[i].Correlation;
					outfile<<std::endl;
					outfile.precision();
					outfile.setf(std::ios_base::unitbuf);	

					qcResult->GetGradientProcess()[i]=(QCResult::GRADIENT_EXCLUDE);
					qcResult->GetIntensityMotionCheckResult()[i].bInterlaceCheckOK = false;
				}
			}
			else // single b value
			{
				if(fabs(Results[i].AngleX) > angleThreshold || fabs(Results[i].AngleY) > angleThreshold ||
					fabs(Results[i].AngleZ) > angleThreshold	 ||fabs(Results[i].TranslationX)>transThreshold || 
					fabs(Results[i].TranslationY)>transThreshold || fabs(Results[i].TranslationZ-0.5*spacing[2])>transThreshold||
					Results[i].Correlation < correlationThresholdGradient || 
					Results[i].Correlation < interlaceGradientMeans - interlaceGradientDeviations * corrGradientDev) 
				{
					std::cout.precision(6);
					std::cout.setf(std::ios_base::showpoint|std::ios_base::right) ;	
					std::cout<<"\t"<<std::setw(10)<<i;
					std::cout<<"\t"<<std::setw(10)<<Results[i].AngleX;
					std::cout<<"\t"<<std::setw(10)<<Results[i].AngleY;
					std::cout<<"\t"<<std::setw(10)<<Results[i].AngleZ;
					std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationX;
					std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationY;
					std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationZ;
					std::cout<<"\t"<<std::setw(10)<<Results[i].MutualInformation;
					std::cout<<"\t"<<std::setw(10)<<Results[i].Correlation;
					std::cout<<std::endl;
					std::cout.precision();
					std::cout.setf(std::ios_base::unitbuf);

					outfile.precision(6);
					outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
					outfile<<"\t"<<std::setw(10)<<i;
					outfile<<"\t"<<std::setw(10)<<Results[i].AngleX;
					outfile<<"\t"<<std::setw(10)<<Results[i].AngleY;
					outfile<<"\t"<<std::setw(10)<<Results[i].AngleZ;
					outfile<<"\t"<<std::setw(10)<<Results[i].TranslationX;
					outfile<<"\t"<<std::setw(10)<<Results[i].TranslationY;
					outfile<<"\t"<<std::setw(10)<<Results[i].TranslationZ;
					outfile<<"\t"<<std::setw(10)<<Results[i].MutualInformation;
					outfile<<"\t"<<std::setw(10)<<Results[i].Correlation;
					outfile<<std::endl;
					outfile.precision();
					outfile.setf(std::ios_base::unitbuf);	

					qcResult->GetGradientProcess()[i]=(QCResult::GRADIENT_EXCLUDE);
					qcResult->GetIntensityMotionCheckResult()[i].bInterlaceCheckOK = false;
				}
			}
		}
	}
	outfile.close();	
	
	return true;
}

bool CIntensityMotionCheck::InterCheck()
{
	if(protocal->GetIntensityMotionCheckProtocal().bCheck &&  protocal->GetIntensityMotionCheckProtocal().bGradientCheck)
		return InterCheck(	25, 0.1, 1,
							protocal->GetIntensityMotionCheckProtocal().gradientRotationThreshold,
							protocal->GetIntensityMotionCheckProtocal().gradientTranslationThreshold	);
	else
	{
		std::cout<<"Inter-gradient gradient-wise check NOT set."<<std::endl;
		return true;
	}

	//return InterCheck(25, 0.1, 1, 0.5, 1.0);
}

bool CIntensityMotionCheck::InterCheck(unsigned int BinNumb, double PercentagePixel, bool UseExplicitPDFDerivatives, double angleThreshold,double transThreshold )
{
	std::cout<<"Intergradient Image checking by rigid registration to baseline image"<<std::endl;
	if(!bDwiLoaded  ) LoadDwiImage();
	if(!bDwiLoaded  )
	{
		std::cout<<"DWI load error, no Gradient Images got"<<std::endl;
		return false;
	}	

	//std::vector< std::vector<struInterlaceResults> >	ResultsContainer;

	typedef itk::VectorIndexSelectionCastImageFilter<DwiImageType, GradientImageType > FilterType;
	FilterType::Pointer componentExtractor = FilterType::New();
	componentExtractor->SetInput(DwiImage);

	FilterType::Pointer componentExtractor1 = FilterType::New();
	componentExtractor1->SetInput(DwiImage);

	componentExtractor->SetIndex(0);
	componentExtractor->Update();

	std::vector< struInterlaceResults >	Results;
	for( unsigned int j = 1; j<DwiImage->GetVectorLength(); j++ )
	{
		componentExtractor1->SetIndex( j );
		componentExtractor1->Update();

		std::cout<<std::endl<<"Register Gradient "<<j<<" to Baseline Image ..."<<std::endl;

		//CRigidRegistration RigidRegistration(componentExtractor->GetOutput(),componentExtractor1->GetOutput() );
		CRigidRegistration RigidRegistration(componentExtractor->GetOutput(),componentExtractor1->GetOutput(),BinNumb,PercentagePixel,UseExplicitPDFDerivatives );
		Results.push_back(RigidRegistration.Run());

		std::cout<<"Angles: "<<Results[j-1].AngleX<<" "<<Results[j-1].AngleY<<" "<<Results[j-1].AngleZ<<std::endl;
		std::cout<<"Trans : "<<Results[j-1].TranslationX<<" "<<Results[j-1].TranslationY<<" "<<Results[j-1].TranslationZ<<std::endl;
		std::cout<<"MI    : "<<Results[j-1].MutualInformation<<std::endl;
	}

	// copy gradient results to qcResult
	for(unsigned int i=0; i< Results.size(); i++)
	{
		qcResult->GetIntensityMotionCheckResult()[i+1].gradientRotationX	= Results[i].AngleX;	
		qcResult->GetIntensityMotionCheckResult()[i+1].gradientRotationY	= Results[i].AngleY;	
		qcResult->GetIntensityMotionCheckResult()[i+1].gradientRotationZ	= Results[i].AngleZ;	
		qcResult->GetIntensityMotionCheckResult()[i+1].gradientTranslationX	= Results[i].TranslationX;	
		qcResult->GetIntensityMotionCheckResult()[i+1].gradientTranslationY	= Results[i].TranslationY;
		qcResult->GetIntensityMotionCheckResult()[i+1].gradientTranslationZ	= Results[i].TranslationZ;
	}


	//////////////////////////////////////
	// print & save results
	std::ofstream outfile; 
	outfile.open(ReportFileName.c_str(),std::ios::app);
	outfile<<std::endl;
	outfile<<"==========================================================================="<<std::endl;
	outfile  <<"Inter-gradient volume MOTION check with MI-based 3D rigid registration: "<<std::endl;
	std::cout<<"Inter-gradient volume MOTION check with MI-based 3D rigid registration: "<<std::endl;

	std::cout <<"\t"<<std::setw(10)<<"Register:\t"<<std::setw(10)<<"AngleX\t"<<std::setw(10)<<"AngleY\t"<<
			std::setw(10)<<"AngleZ\t"<<std::setw(10)<<"TranslationX\t"<<std::setw(10)<<"TranslationY\t"<<
			std::setw(10)<<"TranslationZ\t"<<std::setw(10)<<"Metric(MI)"<<std::endl;
	
	outfile <<"\t"<<std::setw(10)<<"Register#:\t"<<std::setw(10)<<"AngleX\t"<<std::setw(10)<<"AngleY\t"<<
			std::setw(10)<<"AngleZ\t"<<std::setw(10)<<"TranslationX\t"<<std::setw(10)<<"TranslationY\t"<<
			std::setw(10)<<"TranslationZ\t"<<std::setw(10)<<"Metric(MI)"<<std::endl;

	for(unsigned int i=0;i<Results.size();i++)
	{
		std::cout.precision(6);
		std::cout.setf(std::ios_base::showpoint|std::ios_base::right) ;	
		std::cout<<"\tRegister "<<i+1<<"-0";
		std::cout<<"\t"<<std::setw(10)<<Results[i].AngleX;
		std::cout<<"\t"<<std::setw(10)<<Results[i].AngleY;
		std::cout<<"\t"<<std::setw(10)<<Results[i].AngleZ;
		std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationX;
		std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationY;
		std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationZ;
		std::cout<<"\t"<<std::setw(10)<<Results[i].MutualInformation;
		std::cout<<std::endl;
		std::cout.precision();
		std::cout.setf(std::ios_base::unitbuf);

		outfile.precision(6);
		outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
		outfile<<"\tRegister "<<i+1<<"-0";
		outfile<<"\t"<<std::setw(10)<<Results[i].AngleX;
		outfile<<"\t"<<std::setw(10)<<Results[i].AngleY;
		outfile<<"\t"<<std::setw(10)<<Results[i].AngleZ;
		outfile<<"\t"<<std::setw(10)<<Results[i].TranslationX;
		outfile<<"\t"<<std::setw(10)<<Results[i].TranslationY;
		outfile<<"\t"<<std::setw(10)<<Results[i].TranslationZ;
		outfile<<"\t"<<std::setw(10)<<Results[i].MutualInformation;
		outfile<<std::endl;
		outfile.precision();
		outfile.setf(std::ios_base::unitbuf);		
	}

	std::cout <<std::endl<<"Inter-gradient check Artifacts:"<<std::endl;
	outfile   <<std::endl<<"Inter-gradient check Artifacts::"<<std::endl;

	std::cout <<"\t"<<std::setw(10)<<"Gradient#:\t"<<std::setw(10)<<"AngleX\t"<<std::setw(10)<<"AngleY\t"<<
			std::setw(10)<<"AngleZ\t"<<std::setw(10)<<"TranslationX\t"<<std::setw(10)<<"TranslationY\t"<<
			std::setw(10)<<"TranslationZ\t"<<std::setw(10)<<"Metric(MI)"<<std::endl;
	
	outfile <<"\t"<<std::setw(10)<<"Gradient#:\t"<<std::setw(10)<<"AngleX\t"<<std::setw(10)<<"AngleY\t"<<
			std::setw(10)<<"AngleZ\t"<<std::setw(10)<<"TranslationX\t"<<std::setw(10)<<"TranslationY\t"<<
			std::setw(10)<<"TranslationZ\t"<<std::setw(10)<<"Metric(MI)"<<std::endl;


	for(unsigned int i=0;i<Results.size();i++)
	{
		if( fabs(Results[i].AngleX) > angleThreshold || fabs(Results[i].AngleY) > angleThreshold ||
			fabs(Results[i].AngleZ) > angleThreshold ||fabs(Results[i].TranslationX)>transThreshold ||
			fabs(Results[i].TranslationY)>transThreshold || fabs(Results[i].TranslationZ)>transThreshold)
		{
			std::cout.precision(6);
			std::cout.setf(std::ios_base::showpoint|std::ios_base::right) ;	
			std::cout<<"\t"<<std::setw(10)<<i+1;
			std::cout<<"\t"<<std::setw(10)<<Results[i].AngleX;
			std::cout<<"\t"<<std::setw(10)<<Results[i].AngleY;
			std::cout<<"\t"<<std::setw(10)<<Results[i].AngleZ;
			std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationX;
			std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationY;
			std::cout<<"\t"<<std::setw(10)<<Results[i].TranslationZ;
			std::cout<<"\t"<<std::setw(10)<<Results[i].MutualInformation;
			std::cout<<std::endl;
			std::cout.precision();
			std::cout.setf(std::ios_base::unitbuf);

			outfile.precision(6);
			outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
			outfile<<"\t"<<std::setw(10)<<i+1;
			outfile<<"\t"<<std::setw(10)<<Results[i].AngleX;
			outfile<<"\t"<<std::setw(10)<<Results[i].AngleY;
			outfile<<"\t"<<std::setw(10)<<Results[i].AngleZ;
			outfile<<"\t"<<std::setw(10)<<Results[i].TranslationX;
			outfile<<"\t"<<std::setw(10)<<Results[i].TranslationY;
			outfile<<"\t"<<std::setw(10)<<Results[i].TranslationZ;
			outfile<<"\t"<<std::setw(10)<<Results[i].MutualInformation;
			outfile<<std::endl;
			outfile.precision();
			outfile.setf(std::ios_base::unitbuf);		

			qcResult->GetGradientProcess()[i+1]=(QCResult::GRADIENT_EXCLUDE);
			qcResult->GetIntensityMotionCheckResult()[i+1].bGradientCheckOK = false;
		}
	}

	outfile.close();	
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
		bGetGridentDirections=false;
		return false;
	}

	bGetGridentDirections=true;
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

	//int numberOfImages=0;
	TensorReconstructionImageFilterType::GradientDirectionType vect3d;
	
	GradientDirectionContainerType::Pointer GradientContainer = GradientDirectionContainerType::New();
	GradientContainer->clear();

	//short sizes[4];

	DwiImageType::SpacingType	spacing =  DwiImage->GetSpacing();
	DwiImageType::PointType		origin	=  DwiImage->GetOrigin();
	DwiImageType::DirectionType direction = DwiImage->GetDirection();

// 	std::cout<<"spacing: "<< spacing[0] <<" "<< spacing[1] <<" "<<spacing[2]<<std::endl;
// 	std::cout<<"origin: "<< origin[0] <<" "<< origin[1] <<" "<<origin[2]<<std::endl;
// 	std::cout<<"direction: "
// 			<< direction[0] <<" "<< direction[1] <<" "<<direction[2]
// 			<< direction[3] <<" "<< direction[4] <<" "<<direction[5]
// 			<< direction[6] <<" "<< direction[7] <<" "<<direction[8]	<<std::endl;

	//int dimension = DwiImage->GetImageDimension();
	//int componentNumber = DwiImage->GetNumberOfComponentsPerPixel();
	//int vectorLength = DwiImage->GetVectorLength();
	
// 	std::cout<<"dimension: "<< dimension <<std::endl;
// 	std::cout<<"componentNumber: "<< componentNumber <<std::endl;
// 	std::cout<<"vectorLength: "<< vectorLength <<std::endl;

	//int type=-1;
	int space;

	
	for ( ; itKey != imgMetaKeys.end(); itKey ++)
	{
		//double x,y,z;
		itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
// 		std::cout<<"itKey: "<< *itKey <<"    metaString: "<<metaString<<std::endl;

		if (itKey->find("DWMRI_gradient") != std::string::npos)
		{ 
			std::istringstream iss(metaString);
			iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
			//sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
			//vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
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
			std::cout<<"modality: "<<metaString<<std::endl;
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



bool CIntensityMotionCheck::ImageCheck()
{

	if( !protocal->GetImageProtocal().bCheck)
	{
		std::cout<<"Image information check NOT set."<<std::endl;
		return true;
	}
	else
	{
		std::ofstream outfile; 
		outfile.open(ReportFileName.c_str());//,std::ios::app);
		outfile<<"Image Information Check"<<std::endl;

		if(!bDwiLoaded  ) LoadDwiImage();
		if(!bDwiLoaded  )
		{
			std::cout<<"DWI load error, no Gradient Direction Loaded"<<std::endl;
			bGetGridentDirections=false;
			return false;
		}
//size
		if( protocal->GetImageProtocal().size[0] ==	DwiImage->GetLargestPossibleRegion().GetSize()[0] && 
			protocal->GetImageProtocal().size[1] ==	DwiImage->GetLargestPossibleRegion().GetSize()[1] && 
			protocal->GetImageProtocal().size[2] ==	DwiImage->GetLargestPossibleRegion().GetSize()[2] )
		{
			qcResult->GetImageInformationCheckResult().size = true;
			outfile<<"  Image size Check OK"<<std::endl;
			std::cout<<"  Image size Check OK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().size = false;
			outfile<<"  Image size Check FAILED"<<std::endl;
			std::cout<<"  Image size Check FAILED"<<std::endl;
			return false;
		}
//origion
		if( protocal->GetImageProtocal().origin[0] ==	DwiImage->GetOrigin()[0]	&& 
			protocal->GetImageProtocal().origin[1] ==	DwiImage->GetOrigin()[1]	&& 
			protocal->GetImageProtocal().origin[2] ==	DwiImage->GetOrigin()[2]	 	)
		{
			qcResult->GetImageInformationCheckResult().origin = true;
			outfile<<"  Image Origin Check OK"<<std::endl;
			std::cout<<"  Image Origin Check OK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().origin = false;
			outfile<<"  Image Origin Check FAILED"<<std::endl;
			std::cout<<"  Image Origin Check FAILED"<<std::endl;
			return false;
		}
//spacing
		//std::cout<<"spacing: "<< protocal->GetImageProtocal().spacing[0]<<" "<<protocal->GetImageProtocal().spacing[1]<<" "<<protocal->GetImageProtocal().spacing[2]<<std::endl;
		//std::cout<<"spacing: "<< DwiImage->GetSpacing()[0]<<" "<<DwiImage->GetSpacing()[1]<<" "<<DwiImage->GetSpacing()[2]<<std::endl;
		if( protocal->GetImageProtocal().spacing[0] ==	DwiImage->GetSpacing()[0]	&& 
			protocal->GetImageProtocal().spacing[1] ==	DwiImage->GetSpacing()[1]	&& 
			protocal->GetImageProtocal().spacing[2] ==	DwiImage->GetSpacing()[2]	 	)
		{
			qcResult->GetImageInformationCheckResult().spacing = true;
			outfile<<"  Image Spacing Check OK"<<std::endl;
			std::cout<<"  Image Spacing Check OK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().spacing = false;
			outfile<<"  Image Spacing Check FAILED"<<std::endl;
			std::cout<<"  Image Spacing Check FAILED"<<std::endl;
			return false;
		}
// space direction
		vnl_matrix<double> imgf(3,3);
		imgf = DwiImage->GetDirection().GetVnlMatrix();

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
			outfile<<"  Image spacedirection Check OK"<<std::endl;
			std::cout<<"  Image spacedirection Check OK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().spacedirection = false;
			outfile<<"  Image spacedirection Check FAILED"<<std::endl;
			std::cout<<"  Image spacedirection Check FAILED"<<std::endl;
			return false;
		}

  // space
		itk::MetaDataDictionary imgMetaDictionary = DwiImage->GetMetaDataDictionary(); 
		std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
		std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
		std::string metaString;

		itk::ExposeMetaData<std::string> (imgMetaDictionary, "NRRD_space", metaString);
		std::cout<<"space: "<<metaString.c_str()<<std::endl;
		
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
			outfile<<"  Image space Check OK"<<std::endl;
			std::cout<<"  Image space Check OK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().space = false;
			outfile<<"  Image space Check FAILED"<<std::endl;
			std::cout<<"  Image space Check FAILED"<<std::endl;
			return false;
		}
		outfile.close();	
	}

	return true;
}

bool CIntensityMotionCheck::DiffusionCheck()
{

	if( !protocal->GetDiffusionProtocal().bCheck)
	{
		std::cout<<"Diffusion information check NOT set."<<std::endl;
		return true;
	}
	else
	{
		if(!bDwiLoaded  ) LoadDwiImage();
		if(!bDwiLoaded  )
		{
			std::cout<<"DWI load error, no Gradient Direction Loaded"<<std::endl;
			bGetGridentDirections=false;
			return false;
		}	

		std::ofstream outfile; 
		outfile.open(ReportFileName.c_str(), std::ios::app);
		outfile<<"==============================================================="<<std::endl;
		outfile<<"Diffusion Information Check"<<std::endl;

		if(!bGetGridentDirections)
			GetGridentDirections();

		if( fabs(protocal->GetDiffusionProtocal().b - this->b0) < 0.0000001 )
		{
			qcResult->GetDiffusionInformationCheckResult().b = true;
			outfile<<"  Diffusion b Check OK"<<std::endl;
			std::cout<<"  Diffusion b Check OK"<<std::endl;
		}
		else
		{
			qcResult->GetDiffusionInformationCheckResult().b = false;
			outfile<<"  Diffusion b Check FAILED: protocol "<< protocal->GetDiffusionProtocal().b<< "   image: " << this->b0<<std::endl;
			std::cout<<"  Diffusion b Check FAILED: protocol "<< protocal->GetDiffusionProtocal().b<< "   image: " << this->b0<<std::endl;
			return false;
		}


		bool result = true;
		for(unsigned int i=0; i< GradientDirectionContainer->size();i++)
		{
			if( fabs(protocal->GetDiffusionProtocal().gradients[i][0] - GradientDirectionContainer->ElementAt(i)[0]) < 0.0000001 &&
				fabs(protocal->GetDiffusionProtocal().gradients[i][1] - GradientDirectionContainer->ElementAt(i)[1]) < 0.0000001 &&
				fabs(protocal->GetDiffusionProtocal().gradients[i][2] - GradientDirectionContainer->ElementAt(i)[2]) < 0.0000001 )			
			{
				result = true;
			}
			else
			{
				std::cout<<i<< ": " <<std::endl;
				std::cout<<protocal->GetDiffusionProtocal().gradients[i][0]<<"    "<< GradientDirectionContainer->ElementAt(i)[0]<<std::endl;
				std::cout<<protocal->GetDiffusionProtocal().gradients[i][1]<<"    "<< GradientDirectionContainer->ElementAt(i)[1]<<std::endl;
				std::cout<<protocal->GetDiffusionProtocal().gradients[i][2]<<"    "<< GradientDirectionContainer->ElementAt(i)[2]<<std::endl;
				result = false;
				break;
			}
		}
		if( result)
		{
			qcResult->GetDiffusionInformationCheckResult().gradient = true;
			outfile<<"  Diffusion gradient Check OK"<<std::endl;
			std::cout<<"  Diffusion gradient Check OK"<<std::endl;
		}
		else
		{
			qcResult->GetDiffusionInformationCheckResult().gradient = false;
			outfile<<"  Diffusion gradient Check FAILED"<<std::endl;
			std::cout<<"  Diffusion gradient Check FAILED"<<std::endl;
			return false;
		}


		/////////////////////////////////////////////////


		itk::MetaDataDictionary imgMetaDictionary = DwiImage->GetMetaDataDictionary(); 
		std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
		std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
		std::string metaString;

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
// 			std::cout << "Image frame: " << std::endl;
// 			std::cout << imgf << std::endl;

			for(unsigned int i = 0; i < 3; ++i)
			{
				for(unsigned int j = 0; j < 3; ++j)
				{
					mf(i,j) = nrrdmf[j][i];
					nrrdmf[j][i] = imgf(i,j);
				}
			}

			// Meausurement frame
// 			std::cout << "Meausurement frame: " << std::endl;
// 			std::cout << mf << std::endl;

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
				outfile<<"  Diffusion measurementFrame Check OK"<<std::endl;
				std::cout<<"  Diffusion measurementFrame Check OK"<<std::endl;
			}
			else
			{
				qcResult->GetDiffusionInformationCheckResult().measurementFrame = false;
				outfile<<"  Diffusion measurementFrame Check FAILED"<<std::endl;
				std::cout<<"  Diffusion measurementFrame Check FAILED"<<std::endl;
			}

			//itk::EncapsulateMetaData<std::vector<std::vector<double> > >(dict,NRRD_MEASUREMENT_KEY,nrrdmf);
		}

		qcResult->GetDiffusionInformationCheckResult().gradient = true;
		qcResult->GetDiffusionInformationCheckResult().measurementFrame = true;

		outfile.close();	
	}

	return true;

}


bool CIntensityMotionCheck::CropDTI()
{ 

	if(!protocal->GetDTIProtocal().bPadding)
	{
		std::cout<<"tensor padding not set"<<std::endl;
		return true;
	}

	std::string str;
	str.append(protocal->GetDTIProtocal().dtiPaddingCommand); 
	str.append(" ");

	str.append(DwiFileName.substr(0,DwiFileName.find_last_of('.') ));
	str.append(protocal->GetDTIProtocal().tensor);

	str.append(" -o ");

	str.append(DwiFileName.substr(0,DwiFileName.find_last_of('.') ));
	str.append(protocal->GetDTIProtocal().tensor);

	str.append(" -size ");

	std::stringstream out;
	out << protocal->GetImageProtocal().size[0]<<","
		<< protocal->GetImageProtocal().size[1]<<","
		<< protocal->GetImageProtocal().size[2];		

	str.append(out.str());

	std::cout<< "DTICrop command: "<< str.c_str() << std::endl;
	system(str.c_str());
	

	return true;
}

bool CIntensityMotionCheck::dtiestim()
{
	if(!protocal->GetDTIProtocal().bCompute)
	{
		std::cout<< "DTI computing NOT set" <<std::endl;
		return true;
	}

	if(!bDwiLoaded  ) LoadDwiImage();
	if(!bDwiLoaded  )
	{
		std::cout<<"DWI load error, no Gradient Direction Loaded"<<std::endl;
		bGetGridentDirections=false;
		return false;
	}
// dtiestim
	std::string str;
	str.append(protocal->GetDTIProtocal().dtiestimCommand); 
	str.append(" ");

	std::string OutputDwiFileName;
	if(protocal->GetEddyMotionCorrectionProtocal().bCorrect)
	{
		OutputDwiFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		OutputDwiFileName.append(protocal->GetEddyMotionCorrectionProtocal().OutputFileName);	
	}
	else
	{
		OutputDwiFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		OutputDwiFileName.append(protocal->GetIntensityMotionCheckProtocal().OutputFileName);	
	}

	str.append(OutputDwiFileName);
	str.append(" ");

	std::string OutputTensor;
	OutputTensor=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
	OutputTensor.append(protocal->GetDTIProtocal().tensor);
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
	str.append(buffer);
	
	

	if( protocal->GetDTIProtocal().bidwi)
	{
		str.append(" --idwi "); 
		std::string idwi;
		idwi=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		idwi.append(protocal->GetDTIProtocal().idwi);
		str.append(idwi);
	}

	if( protocal->GetDTIProtocal().bbaseline)
	{
		str.append(" --B0 "); 
		std::string baseline;
		baseline=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		baseline.append(protocal->GetDTIProtocal().baseline);
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
	CropDTI();	
	dtiprocess();
	return true;
}

// dtiprocess
bool CIntensityMotionCheck::dtiprocess()
{
	std::string string;
	string.append(protocal->GetDTIProtocal().dtiprocessCommand); 
	string.append(" "); 

	std::string dtiprocessInput;
	dtiprocessInput=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
	dtiprocessInput.append(protocal->GetDTIProtocal().tensor);

	string.append(dtiprocessInput);	

	if( protocal->GetDTIProtocal().bfa)
	{
		string.append(" -f ");
		std::string fa;
		fa=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		fa.append(protocal->GetDTIProtocal().fa);
		string.append(fa); 
	}

	if( protocal->GetDTIProtocal().bmd)
	{
		string.append(" -m "); 
		std::string md;
		md=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		md.append(protocal->GetDTIProtocal().md);
		string.append(md); 
	}

	if( protocal->GetDTIProtocal().bcoloredfa)
	{
		string.append(" -c "); 
		std::string cfa;
		cfa=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		cfa.append(protocal->GetDTIProtocal().coloredfa);
		string.append(cfa); 
	}

	if( protocal->GetDTIProtocal().bfrobeniusnorm)
	{
		string.append(" --frobenius-norm-output "); 
		std::string fn;
		fn=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		fn.append(protocal->GetDTIProtocal().frobeniusnorm);
		string.append(fn); 
	}

	std::cout<< "dtiprocess command: "<< string.c_str() << std::endl;
	system(string.c_str());

	return true;
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
		for(unsigned int i=0;i< qcResult->GetGradientProcess().size();i++)
		{
			if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
				gradientLeft++;
		}

		std::cout << "gradientLeft: " << gradientLeft<<std::endl;

		if( gradientLeft == 0)
		{
			//outfile<<"No gradient data left."<<std::endl;
			std::cout<<"No gradient data left."<<std::endl;
			return;
		}

		if( gradientLeft == qcResult->GetGradientProcess().size())
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
			for( unsigned int i = 0 ; i < qcResult->GetGradientProcess().size(); i++ )
			{
				if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
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
			if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
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

void CIntensityMotionCheck::GenerateCheckOutputImage()
{
	if( protocal->GetIntensityMotionCheckProtocal().bCheck && (
		protocal->GetIntensityMotionCheckProtocal().bSliceCheck ||
		protocal->GetIntensityMotionCheckProtocal().bInterlaceCheck ||
		protocal->GetIntensityMotionCheckProtocal().bGradientCheck)     )
	{

		if(!bDwiLoaded  ) LoadDwiImage();
		if(!bDwiLoaded  )
		{
			std::cout<<"DWI load error, no Gradient Direction Loaded"<<std::endl;
			bGetGridentDirections=false;
			return ;
		}

		std::ofstream outfile; 
		outfile.open(ReportFileName.c_str(), std::ios::app);
		outfile<<"==============================================================="<<std::endl;
		outfile<<"QC Check Results"<<std::endl;

		outfile<<"  Gradient(s) excluded:"<<std::endl;
		for(unsigned int i=0;i< qcResult->GetGradientProcess().size();i++)
		{
			if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_EXCLUDE)
				outfile<<"    Gradient "<< i <<std::endl;
		}

		unsigned int gradientLeft=0;
		for(unsigned int i=0;i< qcResult->GetGradientProcess().size();i++)
		{
			if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
				gradientLeft++;
		}

		if( gradientLeft == 0)
		{
			outfile<<"No gradient data left."<<std::endl;
			std::cout<<"No gradient data left."<<std::endl;
			return;
		}

		if( gradientLeft == qcResult->GetGradientProcess().size())
		{
			std::string OutputFileName;
			OutputFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			OutputFileName.append(protocal->GetIntensityMotionCheckProtocal().OutputFileName);
			std::cout<<"No gradient data excluded."<<std::endl;
			std::cout<<"OutputFileName: "<<OutputFileName <<std::endl;
			outfile<<"No gradient data excluded."<<std::endl;
			outfile<<"OutputFileName: "<<OutputFileName <<std::endl;

			itk::NrrdImageIO::Pointer  NrrdImageIO = itk::NrrdImageIO::New();
			try
			{
				DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
				DwiWriter->SetImageIO(NrrdImageIO);
				DwiWriter->SetFileName( OutputFileName );
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
			for( unsigned int i = 0 ; i < qcResult->GetGradientProcess().size(); i++ )
			{
				if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
				{
					value.SetElement( element , oit.Get()[i] ) ;
					element++;
				}
			}
			nit.Set(value);	
			++oit;
			++nit;
		}

		std::string OutputFileName;
		OutputFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		OutputFileName.append(protocal->GetIntensityMotionCheckProtocal().OutputFileName);
		std::cout<<"OutputFileName: "<<OutputFileName <<std::endl;

		itk::NrrdImageIO::Pointer  NrrdImageIO = itk::NrrdImageIO::New();
		try
		{
			DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
			DwiWriter->SetImageIO(NrrdImageIO);
			DwiWriter->SetFileName( OutputFileName );
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

		outfile<<"  QC output image: "<< OutputFileName <<std::endl;
		outfile<<"  Gradients included: "<< std::endl;
		for(unsigned int i=0;i< qcResult->GetGradientProcess().size();i++)
		{
			if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
				outfile<<"    Gradient "<< i <<std::endl;
		}

		outfile.close();



		itk::MetaDataDictionary imgMetaDictionary = DwiImage->GetMetaDataDictionary();    //
		std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
		std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
		std::string metaString;

		char *aryOut = new char[OutputFileName.length()+1];
		strcpy ( aryOut, OutputFileName.c_str() );

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
			if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
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
	else
		std::cout<<" NO QC check procesure and NO QC Image saved "<<std::endl;
}

void CIntensityMotionCheck::EddyMotionCorrection()
{
	if(!protocal->GetEddyMotionCorrectionProtocal().bCorrect)
	{
		std::cout<< "EddyMotionCorrection NOT set" <<std::endl;
		return ;
	}

	std::string string;
	string.append(protocal->GetEddyMotionCorrectionProtocal().EddyMotionCommand); 
	string.append(" ");

	string.append(" -i ");
	std::string input;
	input=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
	input.append(protocal->GetEddyMotionCorrectionProtocal().InputFileName);
	string.append(input); 

	string.append(" -o ");
	std::string output;
	output=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
	output.append(protocal->GetEddyMotionCorrectionProtocal().OutputFileName);
	string.append(output); 

	std::cout<<"EddyMotionCorrection command: "<<string<<std::endl;
	system(string.c_str());
}

unsigned char CIntensityMotionCheck::CheckByProtocal()
{
	unsigned char result=0;   // ZYXEDCBA: 					
					// X QC; Too many bad gradient directions found!
					// Y QC; Single b-value DWI without a b0/baseline!
					// Z QC: Gradient direction # is less than 6!
					// A:ImageCheck() B:DiffusionCheck() C: IntraCheck()  D:InterlaceCheck()  E: InterCheck()

	std::cout<<"ImageCheck ... "<<std::endl;
	if(!ImageCheck())
		result = result | 1;
	std::cout<<"ImageCheck DONE "<<std::endl;

	std::cout<<"DiffusionCheck ... "<<std::endl;
	if(!DiffusionCheck())
		result = result | 2;
	std::cout<<"DiffusionCheck DONE "<<std::endl;

	std::cout<<"IntraCheck ... "<<std::endl;
	if(!IntraCheck())
		result = result | 4;
	std::cout<<"IntraCheck DONE "<<std::endl;

	std::cout<<"InterlaceCheck ... "<<std::endl;
	if(!InterlaceCheck())
		result = result | 8;
	std::cout<<"nterlaceCheck DONE"<<std::endl;

	std::cout<<"InterCheck ... "<<std::endl;
	if(!InterCheck())
		result = result | 16;
	std::cout<<"nterCheck DONE"<<std::endl;

	unsigned int baselineLeft=0;
	unsigned int gradientLeft=0;
	for(unsigned int i=0;i< qcResult->GetGradientProcess().size();i++)
	{
		if ( GradientDirectionContainer->at(i)[0] == 0.0 && GradientDirectionContainer->at(i)[1] == 0.0 && GradientDirectionContainer->at(i)[2] == 0.0)
		//baseline
		{
			if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
				baselineLeft++;
		}
		else //gradient
		{
			if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
				gradientLeft++;
		}
	}

	//this->gradientLeftNumber = gradientLeft;

	std::cout <<"baselineLeft: "<<baselineLeft<<std::endl;
	std::cout <<"gradientLeft: "<<gradientLeft<<std::endl;

	collectLeftDiffusionStatistics();

	unsigned char ValidateResult;
	ValidateResult=validateLeftDiffusionStatistics();
	// 00000CBA:  
	// A: Gradient direction # is less than 6! 
	// B: Single b-value DWI without a b0/baseline!
	// C: Too many bad gradient directions found!
	// 0: valid

	//std::cout << "CheckByProtocal(): ValidateResult: "<< ValidateResult << std::endl;
// 	switch(ValidateResult)
// 	{
// 	case 1:
// 		break;
// 		std::cout<<"Gradient direction # is less than 6! Bad DWI! Should be excluded!"<<std::endl;
// 		result = result | 128;	
// 	case 2:
// 		std::cout<<"Single b-value DWI without a b0/baseline! Bad DWI! Should be excluded!"<<std::endl;
// 		result = result | 64;	
// 		break;
// 	case 4:
// 		std::cout<<"Too many bad gradient directions found! Bad DWI! Should be excluded!"<<std::endl;
// 		result = result | 32;	
// 		break;
// 	default:
// 		break;
// 	}

	if( ValidateResult & 1)
	{	
		std::cout<<"Gradient direction # is less than 6! Bad DWI! Should be excluded!"<<std::endl;
		result = result | 128;	
		//return result;
	}

	if( ValidateResult & 2)
	{	
		std::cout<<"Single b-value DWI without a b0/baseline! Bad DWI! Should be excluded!"<<std::endl;
		result = result | 64;	
		//return result;
	}

	if( ValidateResult & 4)
	{	
		std::cout<<"Too many bad gradient directions found! Bad DWI! Should be excluded!"<<std::endl;
		result = result | 32;	
		//return result;
	}

	if(ValidateResult>0)
	{
		std::cout << "CheckByProtocal(): result: "<< ValidateResult << std::endl;
		return result;
	}

	std::cout<<"GenerateCheckOutputImage ... "<<std::endl;
	GenerateCheckOutputImage();
	std::cout<<"GenerateCheckOutputImage DONE"<<std::endl;

	std::cout<<"EddyMotionCorrection ... "<<std::endl;
	EddyMotionCorrection();
	std::cout<<"EddyMotionCorrection DONE"<<std::endl;

	std::cout<<"DTIComputing ... "<<std::endl;
	DTIComputing();
	std::cout<<"DTIComputing DONE"<<std::endl;

	//std::cout << "CheckByProtocal(): result: "<< ValidateResult << std::endl;
	return result;
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
	std::ofstream outfile; 
	outfile.open(ReportFileName.c_str(), std::ios::app);
	outfile<<"==============================================================="<<std::endl;
	outfile<<"QC result summary:"<<std::endl;

	unsigned char ret = 0;

	if(this->gradientDirLeftNumber<6)
	{
		std::cout<<"\tGradient direction # is less than 6!"<<std::endl;
		outfile<<"\tGradient direction # is less than 6!"<<std::endl;
		ret = ret| 1;
	}

	if(this->baselineLeftNumber==0 && this->bValueLeftNumber==1)
	{
		std::cout<<"\tSingle b-value DWI without a b0/baseline!"<<std::endl;
		outfile<<"\tSingle b-value DWI without a b0/baseline!"<<std::endl;
		ret = ret| 2;
	}

	if( ((this->gradientDirNumber)-(this->gradientDirLeftNumber)) > protocal->GetIntensityMotionCheckProtocal().badGradientPercentageTolerance* (this->gradientDirNumber))
	{
		std::cout<<"\tToo many bad gradient directions found! "<<std::endl;
		outfile  <<"\tToo many bad gradient directions found! "<<std::endl;
		ret = ret| 4;
	}

	//std::cout<<"validateDiffusionStatistics(): ret "<<ret<<std::endl;
	outfile.close();

	return ret;
}

void CIntensityMotionCheck::collectLeftDiffusionStatistics()
{
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
					if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
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
				if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
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
			if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
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
	outfile<<"==============================================================="<<std::endl;
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
	return ;

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

	// 	std::cout<<"DiffusionDirections.size(): " << DiffusionDirections.size() <<std::endl;

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
			// 			std::cout<<"DiffusionDirections[i].repetitionNumber: " <<i<<"  "<<DiffusionDirections[i].repetitionNumber <<std::endl;
		}
		else
		{
			repetNum.push_back(DiffusionDirections[i].repetitionNumber);

			double modeSqr =	DiffusionDirections[i].gradientDir[0]*DiffusionDirections[i].gradientDir[0] +
				DiffusionDirections[i].gradientDir[1]*DiffusionDirections[i].gradientDir[1] +
				DiffusionDirections[i].gradientDir[2]*DiffusionDirections[i].gradientDir[2];

			// 			std::cout<<"modeSqr: " <<modeSqr <<std::endl;
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

	// 	std::cout<<" repetNum.size(): " <<  repetNum.size() <<std::endl;
	// 	std::cout<<" dirMode.size(): " <<  dirMode.size() <<std::endl;

	this->gradientDirNumber = repetNum.size();
	this->bValueNumber = dirMode.size();

	repetitionNumber = repetNum[0];
	for( unsigned int i=1; i<repetNum.size(); i++)
	{ 
		if( repetNum[i] != repetNum[0])
		{
			std::cout<<"Protocol error. Not all the gradient directions have same repetition. "<<std::endl;
			repetitionNumber = -1;			
		}
	}

	this->gradientNumber = this->GradientDirectionContainer->size()-this->baselineNumber;

	std::cout<<"DWI Diffusion: "		<<std::endl;
	std::cout<<"  baselineNumber: "		<<baselineNumber	<<std::endl;
	std::cout<<"  bValueNumber: "		<<bValueNumber		<<std::endl;
	std::cout<<"  gradientDirNumber: "	<<gradientDirNumber	<<std::endl;
	std::cout<<"  gradientNumber: "		<<gradientNumber	<<std::endl;
	std::cout<<"  repetitionNumber: "	<<repetitionNumber	<<std::endl;

	return ;
}


