#include "IntensityMotionCheck.h"

#include "itkMetaDataDictionary.h"
#include "itkNrrdImageIO.h"
//#include "itkGDCMImageIO.h"
//#include "itkGDCMSeriesFileNames.h"
//#include "itkImageSeriesReader.h"
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
		ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		ReportFileName.append("IntensityMotionCheckReports.txt");
	}
	else
		ReportFileName = reportFile;


	DwiImage=NULL;
	bDwiLoaded=false;
	bGetGridentDirections=false;
	bGetGridentImages=false;
}


CIntensityMotionCheck::CIntensityMotionCheck(void)
{
	
	DwiImage=NULL;
	bDwiLoaded=false;
	bGetGridentDirections=false;
	bGetGridentImages=false;


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
			std::cout<< "Loading"<<DwiFileName<<" ... ";
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

	std::cout<<"Image size"<< DwiImage->GetLargestPossibleRegion().GetSize().GetSizeDimension()<<": ";
	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[0]<<" ";
	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[1]<<" ";
	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[2]<<std::endl;

	std::cout<<"Pixel Vector Length: "<<DwiImage->GetVectorLength()<<std::endl;

	// Create result
	GradientIntensityMotionCheckResult result;
	for(int i=1; i<DwiImage->GetLargestPossibleRegion().GetSize()[2]; i++)
	{
		result.sliceCorrelation.push_back(0.0);
	}
	//std::cout<<"result.sliceCorrelation.size(): "<< result.sliceCorrelation.size()<<std::endl;

	for( unsigned int j = 0; j<DwiImage->GetVectorLength(); j++ )
	{
		qcResult->GetIntensityMotionCheckResult().push_back(result);
		//std::cout<<"qcResult.GetIntensityMotionCheckResult().size(): "<<qcResult->GetIntensityMotionCheckResult().size()<<std::endl;
	}
	std::cout<<"qcResult.GetIntensityMotionCheckResult().size(): "<<qcResult->GetIntensityMotionCheckResult().size()<<std::endl;

	return true;
}


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
 
bool CIntensityMotionCheck::IntraCheck( )
{
	if(protocal->GetIntensityMotionCheckProtocal().bCheck &&  protocal->GetIntensityMotionCheckProtocal().bSliceCheck)
		return IntraCheck( 0, protocal->GetIntensityMotionCheckProtocal().sliceCorrelationThreshold, protocal->GetIntensityMotionCheckProtocal().sliceCorrelationDeviationThreshold );
	else
		return true;
}


bool CIntensityMotionCheck::IntraCheck(bool bRegister, double CorrelationThreshold ,  double CorrelationDeviationThreshold)
{
	if(!bDwiLoaded  ) LoadDwiImage();
	if(!bDwiLoaded  )
	{
		std::cout<<"DWI load error, no Gradient Images got"<<std::endl;
		return false;
	}	

	if(qcResult->GetGradientProcess().size()!=DwiImage->GetVectorLength())
	{
		qcResult->GetGradientProcess().clear();
		for(int j=0;j<DwiImage->GetVectorLength();j++)
			qcResult->GetGradientProcess().push_back(QCResult::GRADIENT_INCLUDE);
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
		
		//std::vector< double >	NormalizedCorrelation;
		std::vector< struIntra2DResults >	Results;

		filter1->SetInput( componentExtractor->GetOutput() );
		filter2->SetInput( componentExtractor->GetOutput() );	

		for(int i=1; i<componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[2]; i++)
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
			Results.push_back(IntraGradientRigidReg.Run( bRegister ));
		}
		ResultsContainer.push_back(Results);
	}

	// copy slice correlation to qcResult
	std::cout<<"ResultsContainer.size(): "<<ResultsContainer.size()<<std::endl;
	std::cout<<"qcResult->GetIntensityMotionCheckResult().size(): "<<qcResult->GetIntensityMotionCheckResult().size()<<std::endl;
	for(int i=0; i< ResultsContainer.size(); i++)
	{
		for(int j=0; j<ResultsContainer[i].size(); j++)
		{
			qcResult->GetIntensityMotionCheckResult()[i].sliceCorrelation[j] = ResultsContainer[i][j].Correlation;
		}
	}

	// print & save results
	std::ofstream outfile; 
	outfile.open(ReportFileName.c_str(),std::ios::app);
	outfile<<std::endl;
	outfile<<"==========================================================================="<<std::endl;
	outfile <<"Intra-gradient slice wise check with/without 2D rigid registration: "<<std::endl<<std::endl;

	/*
	for(int i=0;i<ResultsContainer.size();i++)
	{
		outfile <<"Gradient#: "<<i<<std::endl;
		outfile <<"\tRegistration\tAngle\tTranslationX\tTranslationY\tCorralation"<<std::endl;

		std::cout<<std::endl<<"Gradient "<<i<<std::endl;

		for(int j=0;j<ResultsContainer[i].size();j++)
		{
			std::cout.precision(6);
			std::cout.setf(std::ios_base::showpoint);
			std::cout<<std::setw(10)<<ResultsContainer[i][j].Correlation;
			std::cout.precision();
			std::cout.setf(std::ios_base::unitbuf);

			outfile <<"\t"<<j+1<<"-"<<j;
			outfile.precision(6);
			outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
			//outfile.fill();
			outfile <<"\t"<<std::setw(10)<<ResultsContainer[i][j].Angle;
			outfile <<"\t"<<std::setw(10)<<ResultsContainer[i][j].TranslationX;
			outfile <<"\t"<<std::setw(10)<<ResultsContainer[i][j].TranslationY;
			outfile <<"\t"<<std::setw(10)<<ResultsContainer[i][j].Correlation<<std::endl;
			outfile.precision();
			outfile.setf(std::ios_base::unitbuf);			
		}
		outfile<<std::endl;
	}
*/
	outfile <<"Slice Correlations"<<std::endl;
	for(int i=0; i<ResultsContainer.size(); i++)
	{
		outfile <<"\t"<<"Gradient"<<i;
	}
	outfile <<"\tmean"<<"\tdeviation"<<std::endl;

//	std::vector<double> means;
//	std::vector<double> deviations;

	for(int j=0;j<ResultsContainer[0].size();j++)
	{
		double mean=0.0, deviation=0.0;
		for(int i=0; i<ResultsContainer.size(); i++)
		{
			outfile.precision(6);
			outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
			outfile <<"\t"<<std::setw(10)<<ResultsContainer[i][j].Correlation;
			outfile.precision();
			outfile.setf(std::ios_base::unitbuf);

			if(i>0)
				mean+=ResultsContainer[i][j].Correlation/(double)(ResultsContainer.size()-1.0);
		}
		for(int i=1; i<ResultsContainer.size(); i++)
		{
			if(i>0)
				deviation+=(ResultsContainer[i][j].Correlation-mean)*(ResultsContainer[i][j].Correlation-mean)/(double)(ResultsContainer.size()-2.0);
		}
		outfile <<"\t"<<std::setw(10)<<mean;
		outfile <<"\t"<<std::setw(10)<<sqrt(deviation);
		outfile <<std::endl;

		means.push_back(mean);
		deviations.push_back(sqrt(deviation));

	}

	// calculate the means and deviations
/*
	for(int j=0;j<ResultsContainer[0].size();j++)
	{
		double mean=0.0, deviation=0.0;
		for(int i=1; i<ResultsContainer.size(); i++)
			mean+= ResultsContainer[i][j].Correlation/((float)ResultsContainer.size()-1.0);
		for(int i=1; i<ResultsContainer.size(); i++)
			deviation+=(ResultsContainer[i][j].Correlation-mean)*(ResultsContainer[i][j].Correlation-mean)/((float)ResultsContainer.size()-2.0);

		means.push_back(mean);
		deviations.push_back(sqrt(deviation));
	}
	
	outfile <<std::endl;
	for(int i=0;i<means.size();i++)
		outfile <<"\t"<<std::setw(10)<<means[i];
	outfile << std::endl;
	for(int i=0;i<deviations.size();i++)
		outfile <<"\t"<<std::setw(10)<<deviations[i];
	outfile << std::endl;

*/

	outfile <<std::endl<<"Intra-Gradient Slice Check Artifacts:"<<std::endl;
	outfile  <<"\t"<<std::setw(10)<<"Gradient"<<"\t"<<std::setw(10)<<"Slice"<<"\t"<<std::setw(10)<<"preCorrelation"<<"\t"<<std::setw(10)<<"afterCorrelation"<<std::endl;
	std::cout <<std::endl<<"Intra-Gradient Slice Check Artifacts:"<<std::endl;
	std::cout<<"\t"<<std::setw(10)<<"Gradient"<<"\t"<<std::setw(10)<<"Slice"<<"\t"<<std::setw(10)<<"preCorrelation"<<"\t"<<std::setw(10)<<"afterCorrelation"<<std::endl;

//first baseline
	int badcount=0;
	for(int j=0;j<ResultsContainer[0].size(); j++) //for(int j=0;j<ResultsContainer[0].size()-1; j++)
	{
		outfile.precision(6);
		outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
		if( ResultsContainer[0][j].Correlation < CorrelationThreshold )
		{
			outfile  <<"\t"<<std::setw(10)<<"0"<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[0][j].Correlation<<"\t"<<std::setw(10)<<ResultsContainer[0][j+1].Correlation<<std::endl;
			std::cout<<"\t"<<std::setw(10)<<"0"<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[0][j].Correlation<<"\t"<<std::setw(10)<<ResultsContainer[0][j+1].Correlation<<std::endl;
			badcount++;
		}
		outfile.precision();
		outfile.setf(std::ios_base::unitbuf);
	}

	if( badcount >= protocal->GetIntensityMotionCheckProtocal().badSlicePercentageTolerance * DwiImage->GetLargestPossibleRegion().GetSize()[2] )
		qcResult->GetGradientProcess()[0]=QCResult::GRADIENT_EXCLUDE;

// gradients
	for(int i=1;i<ResultsContainer.size();i++)
	{
		badcount=0;
		for(int j=0;j<ResultsContainer[0].size(); j++) //for(int j=0;j<ResultsContainer[0].size()-1; j++)
		{
			outfile.precision(6);
			outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
			//if( ResultsContainer[i][j].Correlation < CorrelationThreshold &&  ResultsContainer[i][j+1].Correlation < CorrelationThreshold)
			if( ResultsContainer[i][j].Correlation < CorrelationThreshold ||  abs(ResultsContainer[i][j].Correlation-means[j]) > deviations[j] * CorrelationDeviationThreshold)
			{
				outfile  <<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<"\t"<<std::setw(10)<<ResultsContainer[i][j+1].Correlation<<std::endl;
				std::cout<<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j].Correlation<<"\t"<<std::setw(10)<<ResultsContainer[i][j+1].Correlation<<std::endl;
				badcount++;
			}
			outfile.precision();
			outfile.setf(std::ios_base::unitbuf);
		}
		if( badcount >= protocal->GetIntensityMotionCheckProtocal().badSlicePercentageTolerance * DwiImage->GetLargestPossibleRegion().GetSize()[2] )
			qcResult->GetGradientProcess()[i]=QCResult::GRADIENT_EXCLUDE;
		
	}
	outfile.close();	
	return true;
}

bool CIntensityMotionCheck::InterlaceCheck()
{
	if(protocal->GetIntensityMotionCheckProtocal().bCheck &&  protocal->GetIntensityMotionCheckProtocal().bInterlaceCheck)
		return InterlaceCheck(	protocal->GetIntensityMotionCheckProtocal().interlaceRotationThreshold,
								protocal->GetIntensityMotionCheckProtocal().interlaceTranslationThreshold,
								protocal->GetIntensityMotionCheckProtocal().interlaceCorrelationThresholdBaseline,
								protocal->GetIntensityMotionCheckProtocal().interlaceCorrelationThresholdGradient	);
	else
		return true;

	//return InterlaceCheck(0.5, 1.0); //0.5 degree 1 mm
}


bool CIntensityMotionCheck::InterlaceCheck(double angleThreshold, double transThreshold, double correlationThresholdBaseline, double correlationThresholdGradient)
{
	if(!bDwiLoaded ) LoadDwiImage();
	if(!bDwiLoaded )
	{
		std::cout<<"DWI load error, no Gradient Images got"<<std::endl;
		return false;
	}	
	
	if(qcResult->GetGradientProcess().size()!=DwiImage->GetVectorLength())
	{
		qcResult->GetGradientProcess().clear();
		for(int j=0;j<DwiImage->GetVectorLength();j++)
			qcResult->GetGradientProcess().push_back(QCResult::GRADIENT_INCLUDE);
	}

	//std::vector< std::vector<struInterlaceResults> >	ResultsContainer;

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
	//std::cout<<"spacing  "<<spacing[0]<<"spacing  "<<spacing[1]<<"spacing  "<<spacing[2]<<std::endl;

	//spacing = DwiImage->GetSpacing();
	//std::cout<<"spacing  "<<spacing[0]<<"spacing  "<<spacing[1]<<"spacing  "<<spacing[2]<<std::endl;

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

		long count=0;
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


//////////////////////////////////////
		std::cout<<"Angles: "<<Results[j].AngleX<<" "<<Results[j].AngleY<<" "<<Results[j].AngleZ<<std::endl;
		std::cout<<"Trans : "<<Results[j].TranslationX<<" "<<Results[j].TranslationY<<" "<<Results[j].TranslationZ<<std::endl;
		std::cout<<"MI    : "<<Results[j].MutualInformation<<std::endl;
		std::cout<<"Correlation: " <<Results[j].Correlation << std::endl;
	}

	// copy interlace results to qcResult
	for(int i=0; i< Results.size(); i++)
	{
		qcResult->GetIntensityMotionCheckResult()[i].interlaceCorrelation	= Results[i].Correlation;	
		qcResult->GetIntensityMotionCheckResult()[i].interlaceRotationX		= Results[i].AngleX;	
		qcResult->GetIntensityMotionCheckResult()[i].interlaceRotationY		= Results[i].AngleY;	
		qcResult->GetIntensityMotionCheckResult()[i].interlaceRotationZ		= Results[i].AngleZ;	
		qcResult->GetIntensityMotionCheckResult()[i].interlaceTranslationX	= Results[i].TranslationX;	
		qcResult->GetIntensityMotionCheckResult()[i].interlaceTranslationY	= Results[i].TranslationY;	
		qcResult->GetIntensityMotionCheckResult()[i].interlaceTranslationZ	= Results[i].TranslationZ;	
	}


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

	for(int i=0;i<Results.size();i++)
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

	std::cout <<std::endl<<"Intra-Gradient Interlace Check Artifacts:"<<std::endl;
	outfile   <<std::endl<<"Intra-Gradient Interlace Check Artifacts:"<<std::endl;

	std::cout <<"\t"<<std::setw(10)<<"Gradient#:\t"<<std::setw(10)<<"AngleX\t"<<std::setw(10)<<"AngleY\t"<<
			std::setw(10)<<"AngleZ\t"<<std::setw(10)<<"TranslationX\t"<<std::setw(10)<<"TranslationY\t"<<
			std::setw(10)<<"TranslationZ\t"<<std::setw(10)<<"Metric(MI)\t"<<std::setw(10)<<"Correlation"<<std::endl;
	
	outfile <<"\t"<<std::setw(10)<<"Gradient#:\t"<<std::setw(10)<<"AngleX\t"<<std::setw(10)<<"AngleY\t"<<
			std::setw(10)<<"AngleZ\t"<<std::setw(10)<<"TranslationX\t"<<std::setw(10)<<"TranslationY\t"<<
			std::setw(10)<<"TranslationZ\t"<<std::setw(10)<<"Metric(MI)\t"<<std::setw(10)<<"Correlation"<<std::endl;


	for(int i=0;i<Results.size();i++)
	{
		if(i==0)
		{
			if(fabs(Results[i].AngleX) > angleThreshold || fabs(Results[i].AngleY) > angleThreshold ||
				fabs(Results[i].AngleZ) > angleThreshold	 ||fabs(Results[i].TranslationX)>transThreshold || 
				fabs(Results[i].TranslationY)>transThreshold || fabs(Results[i].TranslationZ-0.5*spacing[2])>transThreshold||
				Results[i].Correlation < correlationThresholdBaseline) 
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
			}
			//else
			//	qcResult->GetGradientProcess()[i]=(QCResult::GRADIENT_INCLUDE);

		}
		else
		{
			if(fabs(Results[i].AngleX) > angleThreshold || fabs(Results[i].AngleY) > angleThreshold ||
				fabs(Results[i].AngleZ) > angleThreshold	 ||fabs(Results[i].TranslationX)>transThreshold || 
				fabs(Results[i].TranslationY)>transThreshold || fabs(Results[i].TranslationZ-0.5*spacing[2])>transThreshold||
				Results[i].Correlation< correlationThresholdGradient) 
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
			}
			//else
			  //qcResult->GetGradientProcess()[i]=(QCResult::GRADIENT_INCLUDE);
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
		return true;

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

	if(qcResult->GetGradientProcess().size()!=DwiImage->GetVectorLength())
	{
		qcResult->GetGradientProcess().clear();
		for(int j=0;j<DwiImage->GetVectorLength();j++)
			qcResult->GetGradientProcess().push_back(QCResult::GRADIENT_INCLUDE);
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
	for(int i=0; i< Results.size(); i++)
	{
		qcResult->GetIntensityMotionCheckResult()[i].gradientRotationX		= Results[i].AngleX;	
		qcResult->GetIntensityMotionCheckResult()[i].gradientRotationY		= Results[i].AngleY;	
		qcResult->GetIntensityMotionCheckResult()[i].gradientRotationZ		= Results[i].AngleZ;	
		qcResult->GetIntensityMotionCheckResult()[i].gradientTranslationX	= Results[i].TranslationX;	
		qcResult->GetIntensityMotionCheckResult()[i].gradientTranslationY	= Results[i].TranslationY;
		qcResult->GetIntensityMotionCheckResult()[i].gradientTranslationZ	= Results[i].TranslationZ;
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

	for(int i=0;i<Results.size();i++)
	{
		std::cout.precision(6);
		std::cout.setf(std::ios_base::showpoint|std::ios_base::right) ;	
		std::cout<<"\tRegister "<<"-0";
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


	for(int i=0;i<Results.size();i++)
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

			qcResult->GetGradientProcess()[i]=(QCResult::GRADIENT_EXCLUDE);
		}
		//else
			//qcResult->GetGradientProcess()[i]=(QCResult::GRADIENT_INCLUDE);

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
	if(GradientDirectionContainer->Size()<=6) 
	{
		std::cout<<"Gradient Images Less than 7" <<std::endl;
		bGetGridentDirections=false;
		return false;
	}

	std::cout<<"b Value: "<<b0<<std::endl;
	std::cout<<"DWI image gradient count: "<<DwiImage->GetVectorLength()<<std::endl;

	for( unsigned int i = 0; i<DwiImage->GetVectorLength();i++ )//GradientDirectionContainer->Size()
	{
		std::cout<<"Gradient Direction "<<i<<": \t[";
		std::cout<<GradientDirectionContainer->at(i)[0]<<",\t";
		std::cout<<GradientDirectionContainer->at(i)[1]<<",\t";
		std::cout<<GradientDirectionContainer->at(i)[2]<<" ]"<<std::endl;
	}

	bGetGridentDirections=true;
	return true;
}

void CIntensityMotionCheck::GetImagesInformation()
{

///////////////////////////////
/*
space directions: (-1.9779,-0.0658816,0.289063) (-4.79479e-08,-1.95,-0.444431) (0.296476,-0.439521,1.92845) none
measurement frame: (1,0,0) (0,1,0) (0,0,1)

*/
/////////////////////////////
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

	std::cout<<"spacing: "<< spacing[0] <<" "<< spacing[1] <<" "<<spacing[2]<<std::endl;
	std::cout<<"origin: "<< origin[0] <<" "<< origin[1] <<" "<<origin[2]<<std::endl;
	std::cout<<"direction: "
			<< direction[0] <<" "<< direction[1] <<" "<<direction[2]
			<< direction[3] <<" "<< direction[4] <<" "<<direction[5]
			<< direction[6] <<" "<< direction[7] <<" "<<direction[8]	<<std::endl;

	int dimension = DwiImage->GetImageDimension();
	int componentNumber = DwiImage->GetNumberOfComponentsPerPixel();
	int vectorLength = DwiImage->GetVectorLength();
	
	std::cout<<"dimension: "<< dimension <<std::endl;
	std::cout<<"componentNumber: "<< componentNumber <<std::endl;
	std::cout<<"vectorLength: "<< vectorLength <<std::endl;

	int type=-1;
	int space;

	
	for ( ; itKey != imgMetaKeys.end(); itKey ++)
	{
		//double x,y,z;
		itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
		std::cout<<"itKey: "<< *itKey <<"    metaString: "<<metaString<<std::endl;

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

	std::ofstream outfile; 
	outfile.open(ReportFileName.c_str());//,std::ios::app);
	outfile<<"Image Information Check"<<std::endl;

	if( protocal->GetImageProtocal().bCheck)
	{
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
			outfile<<"Image size Check OK"<<std::endl;
			std::cout<<"Image size Check OK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().size = false;
			outfile<<" Image size Check FAILED"<<std::endl;
			std::cout<<" Image size Check FAILED"<<std::endl;
			return false;
		}
//origion
		if( protocal->GetImageProtocal().origin[0] ==	DwiImage->GetOrigin()[0]	&& 
			protocal->GetImageProtocal().origin[1] ==	DwiImage->GetOrigin()[1]	&& 
			protocal->GetImageProtocal().origin[2] ==	DwiImage->GetOrigin()[2]	 	)
		{
			qcResult->GetImageInformationCheckResult().origin = true;
			outfile<<"Image Origin Check OK"<<std::endl;
			std::cout<<"Image Origin Check OK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().origin = false;
			outfile<<" Image Origin Check FAILED"<<std::endl;
			std::cout<<" Image Origin Check FAILED"<<std::endl;
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
			outfile<<"Image Spacing Check OK"<<std::endl;
			std::cout<<"Image Spacing Check OK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().spacing = false;
			outfile<<" Image Spacing Check FAILED"<<std::endl;
			std::cout<<" Image Spacing Check FAILED"<<std::endl;
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
			outfile<<"Image spacedirection Check OK"<<std::endl;
			std::cout<<"Image spacedirection Check OK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().spacedirection = false;
			outfile<<" Image spacedirection Check FAILED"<<std::endl;
			std::cout<<" Image spacedirection Check FAILED"<<std::endl;
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
		else if( metaString.compare( "left-posterior-superior" )==0 )	space = Protocal::SPACE_LPS;
		else if( metaString.compare( "right-posterior-superior")==0 )	space = Protocal::SPACE_RPS;
		else if( metaString.compare( "right-posterior-inferior")==0 )	space = Protocal::SPACE_RPI;
		else if( metaString.compare( "right-anterior-superior" )==0 )	space = Protocal::SPACE_RAS;
		else if( metaString.compare( "right-anterior-inferior" )==0 )	space = Protocal::SPACE_RAI;
		else space = Protocal::SPACE_UNKNOWN;

		if( protocal->GetImageProtocal().space == space)
		{
			qcResult->GetImageInformationCheckResult().space = true;
			outfile<<"Image space Check OK"<<std::endl;
			std::cout<<"Image space Check OK"<<std::endl;
		}
		else
		{
			qcResult->GetImageInformationCheckResult().space = false;
			outfile<<" Image space Check FAILED"<<std::endl;
			std::cout<<" Image space Check FAILED"<<std::endl;
			return false;
		}
	}

	outfile.close();	
	return true;
}

bool CIntensityMotionCheck::DiffusionCheck()
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



	GetGridentDirections();

	if( protocal->GetDiffusionProtocal().b == b0)
	{
		qcResult->GetDiffusionInformationCheckResult().b = true;
		outfile<<"Diffusion b Check OK"<<std::endl;
		std::cout<<"Diffusion b Check OK"<<std::endl;
	}
	else
	{
		qcResult->GetDiffusionInformationCheckResult().b = false;
		outfile<<" Diffusion b Check FAILED"<<std::endl;
		std::cout<<" Diffusion b Check FAILED"<<std::endl;
		return false;
	}
	

	bool result = true;
	for(int i=0; i< GradientDirectionContainer->size();i++)
	{
		if( protocal->GetDiffusionProtocal().gradients[i][0] == GradientDirectionContainer->ElementAt(i)[0] &&
			protocal->GetDiffusionProtocal().gradients[i][1] == GradientDirectionContainer->ElementAt(i)[1] &&
			protocal->GetDiffusionProtocal().gradients[i][2] == GradientDirectionContainer->ElementAt(i)[2]   )			
		{
			result = true;
		}
		else
		{
			result = false;
			break;
		}
	}
	if( result)
	{
		qcResult->GetDiffusionInformationCheckResult().gradient = true;
		outfile<<"Diffusion gradient Check OK"<<std::endl;
		std::cout<<"Diffusion gradient Check OK"<<std::endl;
	}
	else
	{
		qcResult->GetDiffusionInformationCheckResult().gradient = false;
		outfile<<" Diffusion gradient Check FAILED"<<std::endl;
		std::cout<<" Diffusion gradient Check FAILED"<<std::endl;
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
		std::cout << "Image frame: " << std::endl;
		std::cout << imgf << std::endl;

		for(unsigned int i = 0; i < 3; ++i)
		{
			for(unsigned int j = 0; j < 3; ++j)
			{
				mf(i,j) = nrrdmf[j][i];
				nrrdmf[j][i] = imgf(i,j);
			}
		}

		// Meausurement frame
		std::cout << "Meausurement frame: " << std::endl;
		std::cout << mf << std::endl;

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
			outfile<<"Diffusion measurementFrame Check OK"<<std::endl;
			std::cout<<"Diffusion measurementFrame Check OK"<<std::endl;
		}
		else
		{
			qcResult->GetDiffusionInformationCheckResult().measurementFrame = false;
			outfile<<"Diffusion measurementFrame Check FAILED"<<std::endl;
			std::cout<<"Diffusion measurementFrame Check FAILED"<<std::endl;
		}

		//itk::EncapsulateMetaData<std::vector<std::vector<double> > >(dict,NRRD_MEASUREMENT_KEY,nrrdmf);
	}

	qcResult->GetDiffusionInformationCheckResult().gradient = true;
	qcResult->GetDiffusionInformationCheckResult().measurementFrame = true;

	outfile.close();	
	return true;

}

bool CIntensityMotionCheck::DTIComputing()
{
	if(!protocal->GetDTIProtocal().bCompute)
	{
		std::cout<< "DTI computing not set" <<std::endl;
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
	
// dtiprocess
	std::string string;
	string.append(protocal->GetDTIProtocal().dtiprocessCommand); 
	string.append(" "); 

	string.append(OutputTensor);	

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

void CIntensityMotionCheck::GenerateCheckOutputImage()
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

	for(int i=0;i< qcResult->GetGradientProcess().size();i++)
	{
		if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_EXCLUDE)
			outfile<<"Gradient "<< i <<" excluded "<<std::endl;
	}

	int gradientLeft=0;
	for(int i=0;i< qcResult->GetGradientProcess().size();i++)
	{
		if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
			gradientLeft++;
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
		int element = 0;
		for( int i = 0 ; i < qcResult->GetGradientProcess().size(); i++ )
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
		DwiWriter->Update();
	}
	catch(itk::ExceptionObject & e)
	{
		std::cout<< e.GetDescription()<<std::endl;
		return ;
	}

	//newDwiImage->Delete();

	outfile<<"QC output image: "<< OutputFileName <<std::endl;
	outfile<<"Gradients included: "<< std::endl;
	for(int i=0;i< qcResult->GetGradientProcess().size();i++)
	{
		if(qcResult->GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
			outfile<<"Gradient "<< i <<std::endl;
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


	int temp=0;
	for(int i=0;i< GradientDirectionContainer->size();i++ )
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

void CIntensityMotionCheck::EddyMotionCorrection()
{
	if(!protocal->GetEddyMotionCorrectionProtocal().bCorrect)
	{
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
	system("EddyMotionCorrection");
}

bool CIntensityMotionCheck::CheckByProtocal()
{
	std::cout<<"ImageCheck ... ";
	ImageCheck();
	std::cout<<"ImageCheck DONE"<<std::endl;

	std::cout<<"DiffusionCheck ... ";
	DiffusionCheck();
	std::cout<<"DiffusionCheck DONE"<<std::endl;

	std::cout<<"IntraCheck ... ";
	IntraCheck();
	std::cout<<"IntraCheck DONE"<<std::endl;

	std::cout<<"InterlaceCheck ... ";
	InterlaceCheck();
	std::cout<<"nterlaceCheck DONE"<<std::endl;

	std::cout<<"InterCheck ... ";
	InterCheck();
	std::cout<<"nterCheck DONE"<<std::endl;

	std::cout<<"GenerateCheckOutputImage ... ";
	GenerateCheckOutputImage();
	std::cout<<"GenerateCheckOutputImage DONE"<<std::endl;

	std::cout<<"EddyMotionCorrection ... ";
	EddyMotionCorrection();
	std::cout<<"EddyMotionCorrection DONE"<<std::endl;

	std::cout<<"DTIComputing ... ";
	DTIComputing();
	std::cout<<"DTIComputing DONE"<<std::endl;

	/*
	return (	ImageCheck()		&&
				DiffusionCheck()	&&
				IntraCheck()		&&
				InterlaceCheck()	&&
				InterCheck()		&&
				DTIComputing()			);*/
	return true;
}

