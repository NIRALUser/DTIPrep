/*=========================================================================

Program:   NeuroLib
Module:    $file: itkDWIQCInterlaceChecker.cpp $
Language:  C++
Date:      $Date: 2009-09-24 15:12:36 $
Version:   $Revision: 1.6 $
Author:    Zhexing Liu (liuzhexing@gmail.com)

Copyright (c) NIRAL, UNC. All rights reserved.
See http://www.niral.unc.edu for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _itkDWIQCInterlaceChecker_cpp
#define _itkDWIQCInterlaceChecker_cpp

#include "itkDWIQCInterlaceChecker.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkExceptionObject.h"
#include "itkProgressReporter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkExtractImageFilter.h"
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_matrix_inverse.h"


#include "itkMeanSquaresImageToImageMetric.h"
#include "itkImageRegistrationMethod.h"
#include "itkMattesMutualInformationImageToImageMetric.h"

#include "itkLinearInterpolateImageFunction.h"
#include "itkVersorRigid3DTransform.h"
#include "itkCenteredTransformInitializer.h"
#include "itkVersorRigid3DTransformOptimizer.h"

#include <iostream>
#include <iomanip>
#include <fstream>

namespace itk
{
	/**
	*
	*/
	template<class TImageType>
	DWIQCInterlaceChecker< TImageType>
		::DWIQCInterlaceChecker()
	{
		m_CorrelationThresholdBaseline	= 0.95 ;
		m_CorrelationThresholdGradient	= 0.95 ;
		m_TranslationThreshold			= 2.00 ;
		m_RotationThreshold				= 0.50 ;

		m_CorrelationStedvTimesBaseline = 3.0;
		m_CorrelationStdevTimesGradient = 3.5;

		b0 = -1.0 ;

		m_ReportFileName = "";
		m_ReportFileMode = DWIQCInterlaceChecker::Report_New;

		bValues.clear();

		CheckDoneOff();

	}

	/**
	*
	*/
	template<class TImageType>
	DWIQCInterlaceChecker< TImageType>
		::~DWIQCInterlaceChecker()
	{
	}

	/**
	* PrintSelf
	*/
	template<class TImageType>
	void 
		DWIQCInterlaceChecker<TImageType>
		::PrintSelf(std::ostream& os, Indent indent) const
	{
		Superclass::PrintSelf(os,indent);
		os << indent << "DWI QC interlace-wise check filter: " << std::endl;
	}


	/**
	* initialize QC Resullts
	*/
	template<class TImageType>
	void 
		DWIQCInterlaceChecker<TImageType>
		::initializeQCResullts() 
	{
		for( unsigned int j = 0; j< this->GetInput()->GetVectorLength(); j++)
		{
			this->qcResults.push_back(1);
		}
	}

	/** 
	*
	*/
	template <class TImageType>
	void 
		DWIQCInterlaceChecker<TImageType>
		::GenerateOutputInformation()
	{
		// call the superclass' implementation of this method
		Superclass::GenerateOutputInformation();

		// get pointers to the input and output
		InputImageConstPointer   inputPtr = this->GetInput();
		OutputImagePointer      outputPtr = this->GetOutput();

		if ( !inputPtr || !outputPtr )
		{
			return;
		}
		
		// perform InterlaceWiseCheck
		//std::cout << "Interlace wise checking begins here. " <<std::endl;
		parseGridentDirections();
		collectDiffusionStatistics();
		initializeQCResullts() ;
		calculateCorrelationsAndMotions();
		check();
		collectLeftDiffusionStatistics();
		writeReport();
		CheckDoneOn();

		outputPtr->SetVectorLength( this->baselineLeftNumber + this-> gradientLeftNumber);

		itk::MetaDataDictionary outputMetaDictionary;

 		itk::MetaDataDictionary imgMetaDictionary = inputPtr->GetMetaDataDictionary(); 
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
			imgf = inputPtr->GetDirection().GetVnlMatrix();

			// Meausurement frame
			itk::EncapsulateMetaData<std::vector<std::vector<double> > >( outputMetaDictionary, "NRRD_measurement frame", nrrdmf);

		}

		// modality
		if(imgMetaDictionary.HasKey("modality"))
		{
			itk::ExposeMetaData(imgMetaDictionary, "modality", metaString);
			itk::EncapsulateMetaData<std::string>( outputMetaDictionary, "modality", metaString);
		}

// 		// thickness
// 		if(imgMetaDictionary.HasKey("NRRD_thicknesses[2]"))
// 		{
// 			double thickness;
// 			itk::ExposeMetaData<double>(imgMetaDictionary, "NRRD_thickness[2]", thickness);
// 			itk::EncapsulateMetaData<double>( outputMetaDictionary, "NRRD_thickness[2]", thickness);
// 		}

		// centerings
		if(imgMetaDictionary.HasKey("NRRD_centerings[0]"))
		{
			itk::ExposeMetaData(imgMetaDictionary, "NRRD_centerings[0]", metaString);
			itk::EncapsulateMetaData<std::string>( outputMetaDictionary, "NRRD_centerings[0]", metaString);
		}
		if(imgMetaDictionary.HasKey("NRRD_centerings[1]"))
		{
			itk::ExposeMetaData(imgMetaDictionary, "NRRD_centerings[1]", metaString);
			itk::EncapsulateMetaData<std::string>( outputMetaDictionary, "NRRD_centerings[1]", metaString);
		}
		if(imgMetaDictionary.HasKey("NRRD_centerings[2]"))
		{
			itk::ExposeMetaData(imgMetaDictionary, "NRRD_centerings[2]", metaString);
			itk::EncapsulateMetaData<std::string>( outputMetaDictionary, "NRRD_centerings[2]", metaString);
		}

		// b-value
		if(imgMetaDictionary.HasKey("DWMRI_b-value"))
		{
			itk::ExposeMetaData(imgMetaDictionary, "DWMRI_b-value", metaString);
			itk::EncapsulateMetaData<std::string>( outputMetaDictionary, "DWMRI_b-value", metaString);
		}

		// gradient vectors
		int temp=0;
		for(unsigned int i=0;i< this->m_GradientDirectionContainer -> Size();i++ )
		{
			if(this->qcResults[i])
			{
				std::ostringstream ossKey;
				ossKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << temp ;

				std::ostringstream ossMetaString;
				ossMetaString << std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
					<< this->m_GradientDirectionContainer->ElementAt(i)[0] << "    " 
					<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
					<< this->m_GradientDirectionContainer->ElementAt(i)[1] << "    " 
					<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
					<< this->m_GradientDirectionContainer->ElementAt(i)[2] ;

				//std::cout<<ossKey.str()<<ossMetaString.str()<<std::endl;
				itk::EncapsulateMetaData<std::string>( outputMetaDictionary, ossKey.str(), ossMetaString.str());
				++temp;
			}
		}
		outputPtr->SetMetaDataDictionary(outputMetaDictionary);
	}


	/** 
	*
	*/
	template <class TImageType>
	void  
		DWIQCInterlaceChecker<TImageType>
		::rigidRegistration(
		GradientImageType::Pointer odd, 
		GradientImageType::Pointer even,
		unsigned int	BinsNumber,
		double			SamplesPercent ,
		bool			/* ExplicitPDFDerivatives */,
		struInterlaceResults&  regResult
		)
	{
		//setup pipeline
		typedef itk::VersorRigid3DTransform< double >   TransformType;
		typedef itk::VersorRigid3DTransformOptimizer     OptimizerType;
		typedef itk::MattesMutualInformationImageToImageMetric< GradientImageType, GradientImageType >   MetricType;
		typedef itk::LinearInterpolateImageFunction< GradientImageType ,double >    InterpolatorType;
		typedef itk::ImageRegistrationMethod< GradientImageType,GradientImageType >    RegistrationType;

		typedef GradientImageType::SpacingType    SpacingType;
		typedef GradientImageType::PointType      OriginType;
		typedef GradientImageType::RegionType     RegionType;
		typedef GradientImageType::SizeType       SizeType;

		MetricType::Pointer				metric			= MetricType::New();
		OptimizerType::Pointer			optimizer		= OptimizerType::New();
		InterpolatorType::Pointer		interpolator	= InterpolatorType::New();
		RegistrationType::Pointer		registration	= RegistrationType::New();
		TransformType::Pointer			transform		= TransformType::New();

		GradientImageType::Pointer		fixedImage = odd;
		GradientImageType::Pointer		movingImage = even;

		unsigned int numberOfBins = BinsNumber;
		double  percentOfSamples = SamplesPercent;			// 1% ~ 20%
		//bool	useExplicitPDFDerivatives = ExplicitPDFDerivatives;

		registration->SetMetric(        metric        );
		registration->SetOptimizer(     optimizer     );
		registration->SetInterpolator(  interpolator  );
		registration->SetTransform(     transform	  );

		registration->SetFixedImage(   fixedImage );
		registration->SetMovingImage(  movingImage);
		
		// setup parameters
		registration->SetFixedImageRegion( fixedImage->GetBufferedRegion() );

		typedef itk::CenteredTransformInitializer< TransformType, GradientImageType, GradientImageType >  TransformInitializerType;
		TransformInitializerType::Pointer initializer = TransformInitializerType::New();

		initializer->SetTransform(   transform );
		initializer->SetFixedImage(  fixedImage );
		initializer->SetMovingImage( movingImage);
		initializer->MomentsOn();
		//initializer->GeometryOn();
		initializer->InitializeTransform();

		typedef TransformType::VersorType  VersorType;
		typedef VersorType::VectorType     VectorType;

		VersorType     rotation;
		VectorType     axis;

		axis[0] = 0.0;
		axis[1] = 0.0;
		axis[2] = 1.0;

		const double angle = 0;
		rotation.Set(  axis, angle  );
		transform->SetRotation( rotation );

		registration->SetInitialTransformParameters( transform->GetParameters() );

		typedef OptimizerType::ScalesType       OptimizerScalesType;
		OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );
		const double translationScale = 1.0 / 1000.0;

		optimizerScales[0] = 1.0;
		optimizerScales[1] = 1.0;
		optimizerScales[2] = 1.0;
		optimizerScales[3] = translationScale;
		optimizerScales[4] = translationScale;
		optimizerScales[5] = translationScale;

		optimizer->SetScales( optimizerScales );
		optimizer->SetMaximumStepLength( 0.2000  ); 
		optimizer->SetMinimumStepLength( 0.0001 );

		optimizer->SetNumberOfIterations( 1000 );
		metric->SetNumberOfHistogramBins( numberOfBins );

		int SampleSize = (int)(fixedImage->GetPixelContainer()->Size() * percentOfSamples);
		if( SampleSize> 100000 )	 metric->SetNumberOfSpatialSamples( SampleSize );
		else  metric->UseAllPixelsOn();

		// run the registration pipeline
		try 
		{ 
			registration->StartRegistration(); 
		} 
		catch( itk::ExceptionObject & err ) 
		{ 
			std::cerr << "ExceptionObject caught !" << std::endl; 
			std::cerr << err << std::endl; 
			return ;
		} 

		OptimizerType::ParametersType finalParameters =registration->GetLastTransformParameters();

		const double finalAngleX           = finalParameters[0];
		const double finalAngleY           = finalParameters[1];
		const double finalAngleZ           = finalParameters[2];
		const double finalTranslationX     = finalParameters[3];
		const double finalTranslationY     = finalParameters[4];
		const double finalTranslationZ     = finalParameters[5];

		//const unsigned int numberOfIterations = optimizer->GetCurrentIteration();
		const double bestValue = optimizer->GetValue();

		// Print out results
		const double finalAngleInDegreesX = finalAngleX * 45.0 / atan(1.0);
		const double finalAngleInDegreesY = finalAngleY * 45.0 / atan(1.0);
		const double finalAngleInDegreesZ = finalAngleZ * 45.0 / atan(1.0);

// 		std::cout << "Result = " << std::endl;
// 		std::cout << " AngleX (radians)   = " << finalAngleX  << std::endl;
// 		std::cout << " AngleX (degrees)   = " << finalAngleInDegreesX  << std::endl;
// 		std::cout << " AngleY (radians)   = " << finalAngleY  << std::endl;
// 		std::cout << " AngleY (degrees)   = " << finalAngleInDegreesY  << std::endl;
// 		std::cout << " AngleZ (radians)   = " << finalAngleZ  << std::endl;
// 		std::cout << " AngleZ (degrees)   = " << finalAngleInDegreesZ  << std::endl;
// 		std::cout << " Translation X = " << finalTranslationX  << std::endl;
// 		std::cout << " Translation Y = " << finalTranslationY  << std::endl;
// 		std::cout << " Translation Z = " << finalTranslationZ  << std::endl;
// 		std::cout << " Iterations    = " << numberOfIterations << std::endl;
// 		std::cout << " Metric value  = " << bestValue          << std::endl;

		regResult.AngleX = finalAngleInDegreesX;
		regResult.AngleY = finalAngleInDegreesY;
		regResult.AngleZ = finalAngleInDegreesZ;
		regResult.TranslationX=finalTranslationX;
		regResult.TranslationY=finalTranslationY;
		regResult.TranslationZ=finalTranslationZ;
		regResult.Metric=-bestValue;

		return;
	}
	
	/** 
	*
	*/
	template <class TImageType>
	double 
		DWIQCInterlaceChecker<TImageType>
		::computeCorrelation(GradientImageType::Pointer odd, GradientImageType::Pointer even)
	{
		typedef itk::ImageRegionConstIterator<GradientImageType>  citType;
		citType cit1(odd, odd ->GetBufferedRegion() );
		citType cit2(even,even->GetBufferedRegion() );

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

		return Correlation;
	}		
		
		
	/** 
	*
	*/
	template <class TImageType>
	void 
		DWIQCInterlaceChecker<TImageType>
		::calculateCorrelationsAndMotions()
	{
		std::cout<<"Interlace calculating .";
		InputImageConstPointer  inputPtr = this->GetInput();

		typedef itk::VectorIndexSelectionCastImageFilter< TImageType, GradientImageType > FilterType;
		typename FilterType::Pointer componentExtractor = FilterType::New();
		componentExtractor->SetInput(inputPtr);

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

		std::vector< double>	Results;
		for( unsigned int j = 0; j<inputPtr->GetVectorLength(); j++ )
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
						++iterateEven;
					}
					if( (count/(size[0]*size[1]))%2 == 1)
					{
						iterateOdd.Set(iterateGradient.Get());
						++iterateOdd;
					}
				}
				++iterateGradient;
				++count;			
			}

 			struInterlaceResults result;
 			rigidRegistration(InterlaceOdd,InterlaceEven,25, 0.1, 1, result );
 			result.Correlation = computeCorrelation(InterlaceOdd,InterlaceEven);
 
			this->ResultsContainer.push_back(result);			

			std::cout<<".";
			
// 			std::cout<<"===="<<std::endl;
// 			std::cout<<std::endl<<"  Gradient "<<j;
// 			std::cout<<"RotationAngles(degree): "<<this->ResultsContainer[j].AngleX<<" "<<this->ResultsContainer[j].AngleY<<" "<<this->ResultsContainer[j].AngleZ<<std::endl;
// 			std::cout<<"Translations(mm)      : "<<this->ResultsContainer[j].TranslationX<<" "<<this->ResultsContainer[j].TranslationY<<" "<<this->ResultsContainer[j].TranslationZ<<std::endl;
// 			std::cout<<"MutualInformation     : "<<this->ResultsContainer[j].Metric<<std::endl;
// 			std::cout<<"NormalizedCorrelation : "<<this->ResultsContainer[j].Correlation << std::endl;
		}

		std::cout<<" DONE"<<std::endl;
		return;
	}

	/** 
	*
	*/
	template <class TImageType>
	void 
		DWIQCInterlaceChecker<TImageType>
		::check()
	{
		std::cout<<"Interlace checking ...";
		// calculate the mean and stdev of baseline and gradients
		int DWICount, BaselineCount;
		BaselineCount	= getBaselineNumber();
		DWICount		= getGradientNumber();
	
//  		std::cout<<"BaselineCount: "<<BaselineCount<<std::endl;
//  		std::cout<<"DWICount: "<<DWICount<<std::endl;
// 		std::cout<<"getBValueNumber(): "<<getBValueNumber()<<std::endl;
// 		std::cout<<"getBaselineNumber(): "<<getBaselineNumber()<<std::endl;
// 		std::cout<<"getBValueLeftNumber(): "<<getBValueLeftNumber()<<std::endl;
// 		std::cout<<"getBaselineLeftNumber(): "<<getBaselineLeftNumber()<<std::endl;   

		this->interlaceBaselineMeans=0.0;
		this->interlaceBaselineDeviations=0.0;

		this->interlaceGradientMeans=0.0;
		this->interlaceGradientDeviations=0.0;

		this->interlaceGradientMeans=0.0;
		this->interlaceGradientDeviations=0.0;

		this->quardraticFittedMeans=0.0;
		this->quardraticFittedDeviations=0.0;

		std::vector< double > normalizedMetric;
		for (unsigned int i=0; i< this->ResultsContainer.size(); i++ )
		{
			normalizedMetric.push_back( -1.0);
		}

		if( getBValueNumber() >=3 || (getBValueNumber() ==2 && getBaselineNumber()>0)) // ensure a quardratic fit	
		{
// 			std::cout<<" multiple b valued DWI, do a quadratic-curve fitting between b-value and image correlation for each gradient "<<std::endl;
			vnl_matrix<double> bMatrix( getBaselineNumber()+getGradientNumber() , 3);
			vnl_matrix<double> correlationVector(getBaselineNumber()+getGradientNumber() , 1);
			//unsigned int matrixLineNumber = 0;
			for(unsigned int i=0; i<this->ResultsContainer.size(); i++) // for each gradient
			{
				bMatrix[i][0] = this->bValues[i] *this->bValues[i] ;						
				bMatrix[i][1] = this->bValues[i] ;						
				bMatrix[i][2] = 1.0;

				correlationVector[i][0] = this->ResultsContainer[i].Correlation;
			}
// 
     		vnl_matrix<double> coefficients(3 , 1);
// 			coefficients = vnl_matrix_inverse<double>(bMatrix.transpose()*bMatrix)*bMatrix.transpose()*correlationVector;
//  		std::cout<<"coefficients1: \n"<<coefficients<<std::endl;

			coefficients = vnl_matrix_inverse<double>(bMatrix.transpose()*bMatrix);
			coefficients = coefficients*bMatrix.transpose()*correlationVector;
// 			std::cout<<"coefficients2: \n"<<coefficients<<std::endl;

			for(unsigned int i=0; i<this->ResultsContainer.size(); i++) // for each gradient
			{
// 				std::cout<<"[i]: "<<i<<std::endl;
				normalizedMetric[i] = this->ResultsContainer[i].Correlation - (this->bValues[i] *this->bValues[i]*coefficients[0][0]+this->bValues[i]*coefficients[1][0]+coefficients[2][0]);
// 				std::cout<<"ResultsContainer[i].Correlation: "<< ResultsContainer[i].Correlation <<std::endl;
// 				std::cout<<"normalizedMetric[i]: "<< normalizedMetric[i] <<std::endl;
			}

			// to compute the mean and stdev after quardratic fitting
			for(unsigned int i=0; i< this->ResultsContainer.size(); i++)
			{
				this->quardraticFittedMeans += normalizedMetric[i]/ static_cast< double > ( DWICount+BaselineCount );
			}

			for(unsigned int i=0; i< this->ResultsContainer.size(); i++)
			{				
				if(DWICount>1)
					this->quardraticFittedDeviations+=( normalizedMetric[i]-quardraticFittedMeans)*( normalizedMetric[i]-quardraticFittedMeans)/(double)(DWICount+BaselineCount-1);
				else
					this->quardraticFittedDeviations=0.0; 				
			}
			quardraticFittedDeviations = sqrt(quardraticFittedDeviations);

// 			std::cout<<"quardraticFittedMeans: "<< quardraticFittedMeans <<std::endl;
// 			std::cout<<"quardraticFittedDeviations: "<< quardraticFittedDeviations <<std::endl;
		}
		else // single b value( baseline + bvalue or 2 different b values, [2 different b values not yet implemented])
		{
			for(unsigned int i=0; i< this->ResultsContainer.size(); i++)
			{
				if ( this->m_GradientDirectionContainer->at(i)[0] == 0.0 &&
					this->m_GradientDirectionContainer->at(i)[1] == 0.0 && 
					this->m_GradientDirectionContainer->at(i)[2] == 0.0    )
				{	//for interlace baseline correlation
					this->interlaceBaselineMeans += this->ResultsContainer[i].Correlation/(double)BaselineCount;			
				}
				else //for interlace gradient correlation
				{
					this->interlaceGradientMeans += this->ResultsContainer[i].Correlation/(double)DWICount;
				}
			}

			for(unsigned int i=0; i< this->ResultsContainer.size(); i++)
			{
				if ( this->m_GradientDirectionContainer->at(i)[0] == 0.0 && 
					this->m_GradientDirectionContainer->at(i)[1] == 0.0 && 
					this->m_GradientDirectionContainer->at(i)[2] == 0.0     )
				{
					if(BaselineCount>=1)
						this->interlaceBaselineDeviations += ( this->ResultsContainer[i].Correlation-interlaceBaselineMeans)*( this->ResultsContainer[i].Correlation-interlaceBaselineMeans)/(double)(BaselineCount);
					else
						this->interlaceBaselineDeviations = 0.0;
				}
				else
				{
					if(DWICount>=1)
						interlaceGradientDeviations+=( this->ResultsContainer[i].Correlation-interlaceGradientMeans)*( this->ResultsContainer[i].Correlation-interlaceGradientMeans)/(double)(DWICount);
					else
						interlaceGradientDeviations = 0.0;
				}
			}

			interlaceBaselineDeviations = sqrt(interlaceBaselineDeviations);
			interlaceGradientDeviations = sqrt(interlaceGradientDeviations);

			// 		std::cout<<"interlaceBaselineDeviations: "<<interlaceBaselineDeviations<<std::endl;
			// 		std::cout<<"interlaceGradientDeviations: "<<interlaceGradientDeviations<<std::endl;
			// 
			// 		std::cout<<"m_RotationThreshold: "<<m_RotationThreshold<<std::endl;
			// 		std::cout<<"m_TranslationThreshold: "<<m_TranslationThreshold<<std::endl;
			// 		std::cout<<"m_CorrelationThresholdBaseline: "<<m_CorrelationThresholdBaseline<<std::endl;
			// 		std::cout<<"m_CorrelationStedvTimesBaseline: "<<m_CorrelationStedvTimesBaseline<<std::endl;
			// 		std::cout<<"m_CorrelationThresholdGradient: "<<m_CorrelationThresholdGradient<<std::endl;
			// 		std::cout<<"m_CorrelationStdevTimesGradient: "<<m_CorrelationStdevTimesGradient<<std::endl;
			// 		std::cout<<"0.5*this->GetInput()->GetSpacing()[2]: "<<0.5*this->GetInput()->GetSpacing()[2]<<std::endl;

		}

		// really checking begins here
		if( getBValueNumber() >=3 || (getBValueNumber() ==2 && getBaselineNumber()>0)) // ensure a quardratic fit	
		{
// 			std::cout<<" multiple b valued DWI, do a quadratic-curve fitting between b-values and image correlation for each gradient "<<std::endl;
			for(unsigned int i=0;i< this->ResultsContainer.size();i++)
			{
				if( fabs(this->ResultsContainer[i].AngleX) > m_RotationThreshold		|| fabs(this->ResultsContainer[i].AngleY) > m_RotationThreshold ||
					fabs(this->ResultsContainer[i].AngleZ) > m_RotationThreshold		|| fabs(this->ResultsContainer[i].TranslationX)>m_TranslationThreshold || 
					fabs(this->ResultsContainer[i].TranslationY)>m_TranslationThreshold || fabs(this->ResultsContainer[i].TranslationZ-0.5*this->GetInput()->GetSpacing()[2])>m_TranslationThreshold||
					this->ResultsContainer[i].Correlation < m_CorrelationThresholdGradient || 
					normalizedMetric[i] < quardraticFittedMeans - quardraticFittedDeviations * m_CorrelationStdevTimesGradient) 
				{
// 					std::cout<<"normalizedMetric[i]: "<<normalizedMetric[i]<<std::endl;
// 					std::cout<<"quardraticFittedMeans: "<<quardraticFittedMeans<<std::endl;
// 					std::cout<<"quardraticFittedDeviations: "<<quardraticFittedDeviations<<std::endl;
// 					std::cout<<"m_CorrelationStdevTimesGradient: "<<m_CorrelationStdevTimesGradient<<std::endl;
					this->qcResults[i] = 0;
				}
			}
		}
		else // single b value( baseline + bvalue or 2 different b values, [2 different b values not yet implemented])
		{
			for(unsigned int i=0;i< this->ResultsContainer.size();i++)
			{
				if (this->m_GradientDirectionContainer->at(i)[0] == 0.0 && 
					this->m_GradientDirectionContainer->at(i)[1] == 0.0 && 
					this->m_GradientDirectionContainer->at(i)[2] == 0.0    )
				{	//baseline
					if( fabs(this->ResultsContainer[i].AngleX) > m_RotationThreshold		|| fabs(this->ResultsContainer[i].AngleY) > m_RotationThreshold						||
						fabs(this->ResultsContainer[i].AngleZ) > m_RotationThreshold		|| fabs(this->ResultsContainer[i].TranslationX)>m_TranslationThreshold					|| 
						fabs(this->ResultsContainer[i].TranslationY)>m_TranslationThreshold	|| fabs(this->ResultsContainer[i].TranslationZ - 0.5*this->GetInput()->GetSpacing()[2])>m_TranslationThreshold	||
						this->ResultsContainer[i].Correlation < m_CorrelationThresholdBaseline ||
						this->ResultsContainer[i].Correlation < interlaceBaselineMeans - interlaceBaselineDeviations * m_CorrelationStedvTimesBaseline ) 
					{
						this->qcResults[i] = 0;
					}
				}
				else // gradients
				{
					if( fabs(this->ResultsContainer[i].AngleX) > m_RotationThreshold		|| fabs(this->ResultsContainer[i].AngleY) > m_RotationThreshold ||
						fabs(this->ResultsContainer[i].AngleZ) > m_RotationThreshold		||fabs(this->ResultsContainer[i].TranslationX)>m_TranslationThreshold || 
						fabs(this->ResultsContainer[i].TranslationY)>m_TranslationThreshold || fabs(this->ResultsContainer[i].TranslationZ-0.5*this->GetInput()->GetSpacing()[2])>m_TranslationThreshold||
						this->ResultsContainer[i].Correlation < m_CorrelationThresholdGradient || 
						this->ResultsContainer[i].Correlation < interlaceGradientMeans - interlaceGradientDeviations * m_CorrelationStdevTimesGradient) 
					{
						this->qcResults[i] = 0;
					}
				}
			}
		}

		std::cout<<" DONE"<<std::endl;
		return;
	}

	/** 
	*
	*/
	template <class TImageType>
	void 
		DWIQCInterlaceChecker<TImageType>
		::writeReport()
	{
		if( this->GetReportFileName().length()==0 )
			return;

		std::ofstream outfile;
 		
		if( GetReportFileMode()== DWIQCInterlaceChecker::Report_Append )
			outfile.open( GetReportFileName().c_str(), std::ios_base::app);
		else
			outfile.open( GetReportFileName().c_str());

		outfile <<std::endl;
		outfile <<"================================"<<std::endl;
		outfile <<"     Interlace-wise checking    "<<std::endl;
		outfile <<"================================"<<std::endl;

		outfile<<"Parameters:"<<std::endl;
		outfile<<"  CorrelationThresholdBaseline: "		<< m_CorrelationThresholdBaseline	<<std::endl;
		outfile<<"  CorrelationThresholdGradient: "		<< m_CorrelationThresholdGradient	<<std::endl;
		outfile<<"  CorrelationStedvTimesBaseline: "	<< m_CorrelationStedvTimesBaseline	<<std::endl;
		outfile<<"  CorrelationStdevTimesGradient: "	<< m_CorrelationStdevTimesGradient	<<std::endl;
		outfile<<"  TranslationThreshold: "				<< m_TranslationThreshold			<<std::endl;
		outfile<<"  RotationThreshold: "				<< m_RotationThreshold				<<std::endl;

		outfile <<std::endl<<"======"<<std::endl;
		outfile <<"Interlace-wise motions and correlations: "<<std::endl<<std::endl;

		outfile <<"\t"<<std::setw(10)<<"Gradient#:\t"<<std::setw(10)<<"AngleX\t"<<std::setw(10)<<"AngleY\t"<<
			std::setw(10)<<"AngleZ\t"<<std::setw(10)<<"TranslationX\t"<<std::setw(10)<<"TranslationY\t"<<
			std::setw(10)<<"TranslationZ\t"<<std::setw(10)<<"Metric(MI)\t"<<std::setw(10)<<"Correlation"<<std::endl;

 		collectLeftDiffusionStatistics();  //update
		int DWICount, BaselineCount;
		BaselineCount	= getBaselineNumber();
		DWICount		= getGradientNumber();
		
		for(unsigned int i=0;i<this->ResultsContainer.size();i++)
		{
			outfile.precision(6);
			outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
			outfile<<"\t"<<std::setw(10)<<i;
			outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
			outfile<<this->ResultsContainer[i].AngleX;
			outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
			outfile<<this->ResultsContainer[i].AngleY;
			outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
			outfile<<this->ResultsContainer[i].AngleZ;
			outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
			outfile<<this->ResultsContainer[i].TranslationX;
			outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
			outfile<<this->ResultsContainer[i].TranslationY;
			outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
			outfile<<this->ResultsContainer[i].TranslationZ;
			outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
			outfile<<this->ResultsContainer[i].Metric;
			outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
			outfile<<this->ResultsContainer[i].Correlation;
			outfile<<std::endl;
			outfile.precision();
			outfile.setf(std::ios_base::unitbuf);		
		}

// 		outfile<< "interlaceBaselineDeviations: " << interlaceBaselineDeviations<<std::endl;
// 		outfile<< "interlaceBaselineMeans:      " << interlaceBaselineMeans<<std::endl;
// 
// 		outfile<< "interlaceGradientDeviations: " << interlaceGradientDeviations<<std::endl;
// 		outfile<< "interlaceGradientMeans:      " << interlaceGradientMeans<<std::endl;

		outfile <<std::endl<<"======"<<std::endl;
		outfile <<"Interlace-wise Check Artifacts:"<<std::endl;
		outfile <<"\t"<<std::setw(10)<<"Gradient#:\t"<<std::setw(10)<<"AngleX\t"<<std::setw(10)<<"AngleY\t"<<
			std::setw(10)<<"AngleZ\t"<<std::setw(10)<<"TranslationX\t"<<std::setw(10)<<"TranslationY\t"<<
			std::setw(10)<<"TranslationZ\t"<<std::setw(10)<<"Metric(MI)\t"<<std::setw(10)<<"Correlation"<<std::endl;

		for(unsigned int i=0;i<this->qcResults.size();i++)
		{
			if(!this->qcResults[i])
			{
				outfile.precision(6);
				outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
				outfile<<"\t"<<std::setw(10)<<i;
				outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
				outfile<<this->ResultsContainer[i].AngleX;
				outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
				outfile<<this->ResultsContainer[i].AngleY;
				outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
				outfile<<this->ResultsContainer[i].AngleZ;
				outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
				outfile<<this->ResultsContainer[i].TranslationX;
				outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
				outfile<<this->ResultsContainer[i].TranslationY;
				outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
				outfile<<this->ResultsContainer[i].TranslationZ;
				outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
				outfile<<this->ResultsContainer[i].Metric;
				outfile<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right);
				outfile<<this->ResultsContainer[i].Correlation;
				outfile<<std::endl;
				outfile.precision();
				outfile.setf(std::ios_base::unitbuf);	
			}
		}

		outfile <<std::endl<<"======"<<std::endl;
		outfile<<"Left Diffusion Gradient information:"<<std::endl;
		outfile<<"\tbValueLeftNumber: "		<<bValueLeftNumber		<<std::endl;
		outfile<<"\tbaselineLeftNumber: "	<<baselineLeftNumber	<<std::endl;
		outfile<<"\tgradientDirLeftNumber: "<<gradientDirLeftNumber	<<std::endl;

		outfile<<std::endl<<"\t# "<<"\tDirVector"<<std::setw(34)<<std::setiosflags(std::ios::left)<<"\tIncluded"<<std::endl;
		for(unsigned int i=0;i< this->m_GradientDirectionContainer->size();i++)
		{
			outfile<<"\t"<<i<<"\t[ " 
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<this->m_GradientDirectionContainer->at(i)[0]<<", "
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<this->m_GradientDirectionContainer->at(i)[1]<<", "
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<this->m_GradientDirectionContainer->at(i)[2]<<" ]"
				<<"\t"<<this->qcResults[i]<<std::endl;
		}

		outfile<<std::endl<<"Left Gradient Direction Histogram: "<<std::endl;
		outfile<<"\t# "<<"\tDirVector"<<std::setw(34)<<std::setiosflags(std::ios::left)<<"\tRepLeft"<<std::endl;
		
		for(unsigned int i=0;i< this->DiffusionDirHistOutput.size();i++)
		{
			if( GetReportFileName().length()>0)
				outfile<<"\t"<<i<<"\t[ " 
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<DiffusionDirHistOutput[i].gradientDir[0]<<", "
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<DiffusionDirHistOutput[i].gradientDir[1]<<", "
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<DiffusionDirHistOutput[i].gradientDir[2]<<" ]"
				<<"\t"<<DiffusionDirHistOutput[i].repetitionNumber <<std::endl;
		}
		outfile.close();

		return;
	}

	/** 
	*
	*/
	template <class TImageType>
	void 
		DWIQCInterlaceChecker<TImageType>
		::collectLeftDiffusionStatistics()
	{
		this->DiffusionDirHistOutput.clear();
		this->repetitionLeftNumber.clear();

		for( unsigned int i=0; i< this->m_GradientDirectionContainer->size();i++) 
		{
			if(DiffusionDirHistOutput.size()>0)
			{
				bool newDir = true;
				for(unsigned int j=0;j<DiffusionDirHistOutput.size();j++)
				{
					if( this->m_GradientDirectionContainer->ElementAt(i)[0] == DiffusionDirHistOutput[j].gradientDir[0] && 
						this->m_GradientDirectionContainer->ElementAt(i)[1] == DiffusionDirHistOutput[j].gradientDir[1] && 
						this->m_GradientDirectionContainer->ElementAt(i)[2] == DiffusionDirHistOutput[j].gradientDir[2] )
					{
						if( qcResults[i])
							DiffusionDirHistOutput[j].repetitionNumber++;
						newDir = false;;
					}
				}
				if(newDir)
				{
					std::vector< double > dir;
					dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[0]);
					dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[1]);
					dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[2]);

					struDiffusionDir diffusionDir;
					diffusionDir.gradientDir = dir;
					if(this->qcResults[i])
						diffusionDir.repetitionNumber=1;
					else
						diffusionDir.repetitionNumber=0;

					DiffusionDirHistOutput.push_back(diffusionDir);
				}
			}
			else
			{
				std::vector< double > dir;
				dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[0]);
				dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[1]);
				dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[2]);

				struDiffusionDir diffusionDir;
				diffusionDir.gradientDir = dir;
				if(this->qcResults[i])
					diffusionDir.repetitionNumber=1;
				else
					diffusionDir.repetitionNumber=0;

				DiffusionDirHistOutput.push_back(diffusionDir);
			}
		}

		std::vector<double> dirNorm;
		dirNorm.clear();

		this->baselineLeftNumber=0;
		this->gradientLeftNumber=0;
		//double modeTemp = 0.0;
		for( unsigned int i=0; i<DiffusionDirHistOutput.size(); i++)
		{
			if( DiffusionDirHistOutput[i].gradientDir[0] == 0.0 &&
				DiffusionDirHistOutput[i].gradientDir[1] == 0.0 &&
				DiffusionDirHistOutput[i].gradientDir[2] == 0.0 ) 
			{
				this->baselineLeftNumber = DiffusionDirHistOutput[i].repetitionNumber;
				// std::cout<<"DiffusionDirections[i].repetitionNumber: " <<i<<"  "<<DiffusionDirections[i].repetitionNumber <<std::endl;
			}
			else
			{
				this->repetitionLeftNumber.push_back(DiffusionDirHistOutput[i].repetitionNumber);

				double normSqr =
					DiffusionDirHistOutput[i].gradientDir[0]*DiffusionDirHistOutput[i].gradientDir[0] +
					DiffusionDirHistOutput[i].gradientDir[1]*DiffusionDirHistOutput[i].gradientDir[1] +
					DiffusionDirHistOutput[i].gradientDir[2]*DiffusionDirHistOutput[i].gradientDir[2];

				// 	std::cout<<"modeSqr: " <<modeSqr <<std::endl;
				if( dirNorm.size() > 0)
				{
					bool newDirMode = true;
					for(unsigned int j=0;j< dirNorm.size();j++)
					{
						if( fabs(normSqr-dirNorm[j])<0.001)   // 1 DIFFERENCE for b value
						{
							newDirMode = false;	
							break;
						}
					}
					if( newDirMode && DiffusionDirHistOutput[i].repetitionNumber>0 )
					{
						dirNorm.push_back(	normSqr) ;
						// 					std::cout<<" if(newDirMode) dirMode.size(): " <<  dirMode.size() <<std::endl;
					}
				}
				else
				{
					if(DiffusionDirHistOutput[i].repetitionNumber>0)
						dirNorm.push_back(	normSqr) ;
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

		this->bValueLeftNumber = dirNorm.size();
		
		return;
	}

	/** 
	*
	*/
	template <class TImageType>
	void 
		DWIQCInterlaceChecker<TImageType>
		::collectDiffusionStatistics()
	{
		std::vector<struDiffusionDir> DiffusionDirections;
		DiffusionDirections.clear();

		for( unsigned int i=0; i< this->m_GradientDirectionContainer->size();i++) 
		{
			if(DiffusionDirections.size()>0)
			{
				bool newDir = true;
				for(unsigned int j=0;j<DiffusionDirections.size();j++)
				{
					if( this->m_GradientDirectionContainer->ElementAt(i)[0] == DiffusionDirections[j].gradientDir[0] && 
						this->m_GradientDirectionContainer->ElementAt(i)[1] == DiffusionDirections[j].gradientDir[1] && 
						this->m_GradientDirectionContainer->ElementAt(i)[2] == DiffusionDirections[j].gradientDir[2] )
					{
						DiffusionDirections[j].repetitionNumber++;
						newDir = false;;
					}
				}
				if(newDir)
				{
					std::vector< double > dir;
					dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[0]);
					dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[1]);
					dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[2]);

					struDiffusionDir diffusionDir;
					diffusionDir.gradientDir = dir;
					diffusionDir.repetitionNumber=1;

					DiffusionDirections.push_back(diffusionDir);
				}
			}
			else
			{
				std::vector< double > dir;
				dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[0]);
				dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[1]);
				dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[2]);

				struDiffusionDir diffusionDir;
				diffusionDir.gradientDir = dir;
				diffusionDir.repetitionNumber=1;

				DiffusionDirections.push_back(diffusionDir);
			}
		}

		//std::cout<<"DiffusionDirections.size(): " << m_GradientDirectionContainer->size() <<std::endl;

		std::vector<int> repetNum;
		repetNum.clear();
		std::vector<double> dirNorm;
		dirNorm.clear();

		//double modeTemp = 0.0;
		for( unsigned int i=0; i<DiffusionDirections.size(); i++)
		{
			if( DiffusionDirections[i].gradientDir[0] == 0.0 &&
				DiffusionDirections[i].gradientDir[1] == 0.0 &&
				DiffusionDirections[i].gradientDir[2] == 0.0 ) 
			{
				this->baselineNumber = DiffusionDirections[i].repetitionNumber;
				//std::cout<<"DiffusionDirections[i].repetitionNumber: " <<i<<"  "<<DiffusionDirections[i].repetitionNumber <<std::endl;
			}
			else
			{
				repetNum.push_back(DiffusionDirections[i].repetitionNumber);

				double normSqr =	DiffusionDirections[i].gradientDir[0]*DiffusionDirections[i].gradientDir[0] +
					DiffusionDirections[i].gradientDir[1]*DiffusionDirections[i].gradientDir[1] +
					DiffusionDirections[i].gradientDir[2]*DiffusionDirections[i].gradientDir[2];

				//std::cout<<"normSqr: " <<normSqr <<std::endl;
				if( dirNorm.size() > 0)
				{
					bool newDirMode = true;
					for(unsigned int j=0;j< dirNorm.size();j++)
					{
						if( fabs(normSqr-dirNorm[j])<0.001)   // 1 DIFFERENCE for b value
						{
							newDirMode = false;	
							break;
						}
					}
					if(newDirMode)
					{
						dirNorm.push_back(	normSqr) ;
					}
				}
				else
				{
					dirNorm.push_back(	normSqr) ;
				}
			}
		}

		this->gradientDirNumber = repetNum.size();
		this->bValueNumber = dirNorm.size();

		repetitionNumber = repetNum[0];
		for( unsigned int i=1; i<repetNum.size(); i++)
		{ 
			if( repetNum[i] != repetNum[0])
			{
				std::cout<<"Warrning: Not all the gradient directions have same repetition. "<<std::endl;
				repetitionNumber = -1;			
			}
		}

		this->gradientNumber = this->m_GradientDirectionContainer->size()-this->baselineNumber;

		return;
	}


	/** 
	*
	*/
	template <class TImageType>
	void 
		DWIQCInterlaceChecker<TImageType>
		::parseGridentDirections()
	{
		InputImageConstPointer  inputPtr = this->GetInput();

		itk::MetaDataDictionary imgMetaDictionary = inputPtr->GetMetaDataDictionary();    //
		std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
		std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
		std::string metaString;

		m_GradientDirectionContainer = GradientDirectionContainerType::New();
		GradientDirectionType  vect3d;
		for ( ; itKey != imgMetaKeys.end(); itKey ++)
		{
			itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
			//std::cout<<*itKey<<"  "<<metaString<<std::endl;

			if (itKey->find("DWMRI_gradient") != std::string::npos)
			{ 
				std::istringstream iss(metaString);
				iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
				m_GradientDirectionContainer->push_back(vect3d);
			}
			else if (itKey->find("DWMRI_b-value") != std::string::npos)
			{
				b0 = atof(metaString.c_str());
			}
		}

		if( b0<0.0)
		{
			std::cout <<"BValue not specified in header file" <<std::endl;
			return ;
		}

		for(unsigned int i=0;i< this->m_GradientDirectionContainer -> Size();i++ )
		{
			bValues.push_back(	static_cast< int >((	this->m_GradientDirectionContainer->ElementAt(i)[0]*this->m_GradientDirectionContainer->ElementAt(i)[0] +
				this->m_GradientDirectionContainer->ElementAt(i)[1]*this->m_GradientDirectionContainer->ElementAt(i)[1] +
				this->m_GradientDirectionContainer->ElementAt(i)[2]*this->m_GradientDirectionContainer->ElementAt(i)[2] ) * b0 + 0.5) );
		}

		// 		std::cout << "b values:" << std::endl;
		// 		for(int i=0;i< this->m_GradientDirectionContainer -> Size();i++ )
		// 		{
		// 			std::cout << bValues[i] << std::endl;
		// 		}


		if( m_GradientDirectionContainer->size()<=6) 
		{
			std::cout<<"Gradient Images Less than 6" <<std::endl;
			return ;
		}
	}	
	
	/** 
	*
	*/
	template <class TImageType>
	void 
		DWIQCInterlaceChecker<TImageType>
		::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,int threadId )
	{

		itkDebugMacro(<<"Actually executing");

		// Get the input and output pointers
		InputImageConstPointer  inputPtr = this->GetInput();
		OutputImagePointer      outputPtr = this->GetOutput();

		int gradientLeft=0;
		gradientLeft = this->baselineLeftNumber + this-> gradientLeftNumber;
		if(gradientLeft==0)
		{
			std::cout<<"0 gradient left" <<std::endl;
			return;
		}
		
		// Define/declare an iterator that will walk the output region for this
		// thread.
		typedef ImageRegionIteratorWithIndex<TImageType> OutputIterator;

		OutputIterator outIt(outputPtr, outputRegionForThread);
		
		// Define a few indices that will be used to translate from an input pixel
		// to an output pixel
		typename TImageType::IndexType outputIndex;
		typename TImageType::IndexType  inputIndex;

		// support progress methods/callbacks
		ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

		typename TImageType::PixelType value ;
		value.SetSize( gradientLeft ) ;
		
		// walk the output region, and sample the input image
		while ( !outIt.IsAtEnd() ) 
		{
			// determine the index of the output pixel
			outputIndex = outIt.GetIndex();

			// determine the input pixel location associated with this output pixel
			inputIndex = outputIndex;
			
			int element = 0;
			for( unsigned int i = 0 ; i < this->qcResults.size(); i++ )
			{
				if(this->qcResults[i])
				{
					value.SetElement( element , inputPtr->GetPixel(inputIndex)[i] ) ;
					element++;
				}
			}
			// copy the input pixel to the output
			outIt.Set( value);
			++outIt;

			progress.CompletedPixel();
		}
	}

	/** 
	*
	*/
	template <class TImageType>
	typename TImageType::Pointer
		DWIQCInterlaceChecker<TImageType>
		::GetExcludedGradiennts()
	{		
		if(GetCheckDone())
		{
			// Get the input and output pointers
			InputImageConstPointer  inputPtr = this->GetInput();
			OutputImagePointer      outputPtr = this->GetOutput();

			int gradientLeft=0;
			gradientLeft = this->baselineLeftNumber + this-> gradientLeftNumber;
			if(gradientLeft==inputPtr->GetVectorLength())
			{
				std::cout<<"No gradient excluded" <<std::endl;
				return NULL;
			}

			// Define/declare an iterator that will walk the output region for this
			// thread.
			excludedDwiImage = TImageType::New(); 
			excludedDwiImage->CopyInformation(inputPtr);
			excludedDwiImage->SetRegions(inputPtr->GetLargestPossibleRegion());
			excludedDwiImage->SetVectorLength( inputPtr->GetVectorLength()-gradientLeft) ; 
			
			// meta data
			itk::MetaDataDictionary outputMetaDictionary;

			itk::MetaDataDictionary imgMetaDictionary = inputPtr->GetMetaDataDictionary(); 
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
				imgf = inputPtr->GetDirection().GetVnlMatrix();

				// Meausurement frame
				itk::EncapsulateMetaData<std::vector<std::vector<double> > >( outputMetaDictionary, "NRRD_measurement frame", nrrdmf);

			}

			// modality
			if(imgMetaDictionary.HasKey("modality"))
			{
				itk::ExposeMetaData(imgMetaDictionary, "modality", metaString);
				itk::EncapsulateMetaData<std::string>( outputMetaDictionary, "modality", metaString);
			}

			// 		// thickness
			// 		if(imgMetaDictionary.HasKey("NRRD_thicknesses[2]"))
			// 		{
			// 			double thickness;
			// 			itk::ExposeMetaData<double>(imgMetaDictionary, "NRRD_thickness[2]", thickness);
			// 			itk::EncapsulateMetaData<double>( outputMetaDictionary, "NRRD_thickness[2]", thickness);
			// 		}

			// centerings
			if(imgMetaDictionary.HasKey("NRRD_centerings[0]"))
			{
				itk::ExposeMetaData(imgMetaDictionary, "NRRD_centerings[0]", metaString);
				itk::EncapsulateMetaData<std::string>( outputMetaDictionary, "NRRD_centerings[0]", metaString);
			}
			if(imgMetaDictionary.HasKey("NRRD_centerings[1]"))
			{
				itk::ExposeMetaData(imgMetaDictionary, "NRRD_centerings[1]", metaString);
				itk::EncapsulateMetaData<std::string>( outputMetaDictionary, "NRRD_centerings[1]", metaString);
			}
			if(imgMetaDictionary.HasKey("NRRD_centerings[2]"))
			{
				itk::ExposeMetaData(imgMetaDictionary, "NRRD_centerings[2]", metaString);
				itk::EncapsulateMetaData<std::string>( outputMetaDictionary, "NRRD_centerings[2]", metaString);
			}

			// b-value
			if(imgMetaDictionary.HasKey("DWMRI_b-value"))
			{
				itk::ExposeMetaData(imgMetaDictionary, "DWMRI_b-value", metaString);
				itk::EncapsulateMetaData<std::string>( outputMetaDictionary, "DWMRI_b-value", metaString);
			}

			// gradient vectors
			int temp=0;
			for(int i=0;i< this->m_GradientDirectionContainer -> Size();i++ )
			{
				if(!this->qcResults[i])
				{
					std::ostringstream ossKey;
					ossKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << temp ;

					std::ostringstream ossMetaString;
					ossMetaString << std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
						<< this->m_GradientDirectionContainer->ElementAt(i)[0] << "    " 
						<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
						<< this->m_GradientDirectionContainer->ElementAt(i)[1] << "    " 
						<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
						<< this->m_GradientDirectionContainer->ElementAt(i)[2] ;

					//std::cout<<ossKey.str()<<ossMetaString.str()<<std::endl;
					itk::EncapsulateMetaData<std::string>( outputMetaDictionary, ossKey.str(), ossMetaString.str());
					++temp;
				}
			}
			excludedDwiImage->SetMetaDataDictionary(outputMetaDictionary);  //
			excludedDwiImage->Allocate();

			typedef ImageRegionIteratorWithIndex<TImageType> OutputIterator;
			OutputIterator outIt(excludedDwiImage, excludedDwiImage->GetLargestPossibleRegion());

			// Define a few indices that will be used to translate from an input pixel
			// to an output pixel
			typename TImageType::IndexType outputIndex;
			typename TImageType::IndexType  inputIndex;

			typename TImageType::PixelType value ;
			value.SetSize( inputPtr->GetVectorLength()-gradientLeft ) ;

			// walk the output region, and sample the input image
			while ( !outIt.IsAtEnd() ) 
			{
				// determine the index of the output pixel
				outputIndex = outIt.GetIndex();

				// determine the input pixel location associated with this output pixel
				inputIndex = outputIndex;

				int element = 0;
				for( int i = 0 ; i < this->qcResults.size(); i++ )
				{
					if(!this->qcResults[i])
					{
						value.SetElement( element , inputPtr->GetPixel(inputIndex)[i] ) ;
						element++;
					}
				}
				// copy the input pixel to the output
				outIt.Set( value);
				++outIt;
			}
			return excludedDwiImage;
		}

		return NULL;
	}

} // namespace itk

#endif

