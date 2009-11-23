/*=========================================================================

Program:   NeuroLib
Module:    $file: itkDWIBaselineAverger.cpp $
Language:  C++
Date:      $Date: 2009-11-23 14:14:23 $
Version:   $Revision: 1.10 $
Author:    Zhexing Liu (liuzhexing@gmail.com)

Copyright (c) NIRAL, UNC. All rights reserved.
See http://www.niral.unc.edu for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _itkDWIBaseLineAverager_cpp
#define _itkDWIBaseLineAverager_cpp

#include "itkDWIBaselineAverager.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkExceptionObject.h"
#include "itkProgressReporter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkExtractImageFilter.h"
#include "vnl/vnl_matrix.h"

#include <iostream>
#include <iomanip>
#include <fstream>

#include "itkCastImageFilter.h"
#include "itkImageRegistrationMethod.h"
#include "itkMattesMutualInformationImageToImageMetric.h"

#include "itkLinearInterpolateImageFunction.h"
#include "itkVersorRigid3DTransform.h"
#include "itkCenteredTransformInitializer.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkResampleImageFilter.h"

namespace itk
{
	/**
	*
	*/
	template<class TImageType>
	DWIBaselineAverager< TImageType>
		::DWIBaselineAverager()
	{
		b0 = -1.0 ;
		m_ReportFileName = "";
		m_AverageMethod = DWIBaselineAverager::BaselineOptimized;
		m_StopCriteria = DWIBaselineAverager::IntensityMeanSquareDiffBased;
		m_StopThreshold = 0.02;
		m_MaxIteration = 500;
		m_ReportFileMode = DWIBaselineAverager::Report_New;
		AverageDoneOff();
	}

	/**
	*
	*/
	template<class TImageType>
	DWIBaselineAverager< TImageType>
		::~DWIBaselineAverager()
	{
	}

	/**
	* PrintSelf
	*/
	template<class TImageType>
	void 
		DWIBaselineAverager< TImageType>
		::PrintSelf(std::ostream& os, Indent indent) const
	{
		Superclass::PrintSelf(os,indent);
		os << indent << "DWI baseline average filter: " << std::endl;
	}


	/** 
	*
	*/
	template<class TImageType>
	void 
		DWIBaselineAverager< TImageType>
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

		// perform averaging
		parseGridentDirections();
		collectDiffusionStatistics();
		average();	
		AverageDoneOn();
		writeReport();

		if( getBaselineNumber()==0 ) 
			outputPtr->SetVectorLength( this-> gradientNumber );
		else
			outputPtr->SetVectorLength( 1+ this-> gradientNumber );

		itk::MetaDataDictionary outputMetaDictionary;

		itk::MetaDataDictionary imgMetaDictionary = inputPtr->GetMetaDataDictionary(); 
		std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
		std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
		std::string metaString;

		//  measurement frame 
		if(imgMetaDictionary.HasKey("NRRD_measurement frame"))
		{
#if 0
			// measurement frame
			vnl_matrix_fixed<double,3,3> mf;
			// imaging frame
			vnl_matrix_fixed<double,3,3> imgf;
			imgf = inputPtr->GetDirection().GetVnlMatrix();
#endif

			std::vector<std::vector<double> > nrrdmf;
			itk::ExposeMetaData<std::vector<std::vector<double> > >(imgMetaDictionary,"NRRD_measurement frame",nrrdmf);

			// Meausurement frame
			itk::EncapsulateMetaData<std::vector<std::vector<double> > >( outputMetaDictionary, "NRRD_measurement frame", nrrdmf);

		}

		// modality
		if(imgMetaDictionary.HasKey("modality"))
		{
			itk::ExposeMetaData(imgMetaDictionary, "modality", metaString);
			itk::EncapsulateMetaData<std::string>( outputMetaDictionary, "modality", metaString);
		}

		// thickness
		if(imgMetaDictionary.HasKey("NRRD_thicknesses[2]"))
		{
			double thickness=-12345;
			itk::ExposeMetaData<double>(imgMetaDictionary, "NRRD_thickness[2]", thickness);
			itk::EncapsulateMetaData<double>( outputMetaDictionary, "NRRD_thickness[2]", thickness);
  			itk::EncapsulateMetaData<double>( outputMetaDictionary, "NRRD_thickness[0]", -1);
  			itk::EncapsulateMetaData<double>( outputMetaDictionary, "NRRD_thickness[1]", -1);
// 			itk::EncapsulateMetaData<std::string>( outputMetaDictionary, "NRRD_thickness[2]", "NaN");
		}

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

		if( getBaselineNumber() > 1 ) 
		{
      {
      // baseline dir vector
      std::ostringstream ossKey;
      ossKey << "DWMRI_gradient_0000";
      std::ostringstream ossMetaString;
      ossMetaString << std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
        << 0.0 << "    " 
        << std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
        << 0.0 << "    " 
        << std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
        << 0.0 ;

      itk::EncapsulateMetaData<std::string>( outputMetaDictionary, ossKey.str(), ossMetaString.str());
      }

			// gradient dir vectors
			int temp=1;
			for(unsigned int i=0;i< this->m_GradientDirectionContainer -> Size();i++ )
			{
      //Skip B0 gradients
				if( 1e-7 >=	this->m_GradientDirectionContainer->ElementAt(i)[0] &&
					  1e-7 >=	this->m_GradientDirectionContainer->ElementAt(i)[1] &&
					  1e-7 >=	this->m_GradientDirectionContainer->ElementAt(i)[2]    )
          {
					continue;
          }

				std::ostringstream ossLocalGradientKey;
				ossLocalGradientKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << temp ;

				std::ostringstream ossLocalGradientMetaString;
				ossLocalGradientMetaString << std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
					<< this->m_GradientDirectionContainer->ElementAt(i)[0] << "    " 
					<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
					<< this->m_GradientDirectionContainer->ElementAt(i)[1] << "    " 
					<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
					<< this->m_GradientDirectionContainer->ElementAt(i)[2] ;

				itk::EncapsulateMetaData<std::string>( outputMetaDictionary, ossLocalGradientKey.str(), ossLocalGradientMetaString.str());
				++temp;
			}
		}
		else  // no baseline
		{
			for(unsigned int i=0;i< this->m_GradientDirectionContainer -> Size();i++ )
			{
				std::ostringstream ossKey;
				ossKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << i ;

				std::ostringstream ossMetaString;
				ossMetaString << std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
					<< this->m_GradientDirectionContainer->ElementAt(i)[0] << "    " 
					<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
					<< this->m_GradientDirectionContainer->ElementAt(i)[1] << "    " 
					<< std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
					<< this->m_GradientDirectionContainer->ElementAt(i)[2] ;

				itk::EncapsulateMetaData<std::string>( outputMetaDictionary, ossKey.str(), ossMetaString.str());
			}			
		}
		outputPtr->SetMetaDataDictionary(outputMetaDictionary);
	}

	/** 
	*
	*/
	template<class TImageType>
	void 
		DWIBaselineAverager< TImageType>
		::average()
	{

		if(this->baselineNumber<=0) // no baseline
		{
			std::cout<<"No baseline included."<<std::endl;
			averagedBaseline = NULL;
			this->computeIDWI();
			return ;
		}
		else
		{
			InputImageConstPointer  inputPtr = this->GetInput();

			averagedBaseline = UnsignedImageType::New();
			averagedBaseline->CopyInformation(inputPtr);
			averagedBaseline->SetRegions(inputPtr->GetLargestPossibleRegion());
			averagedBaseline->Allocate();
			averagedBaseline->FillBuffer(0);

			if( this->baselineNumber == 1 )	// single baseline: copy baseline into averagedBaseline for getBaseline()
			{		
				DirectAverage();
				this->computeIDWI();
			}
			else  // multiple baseline images, need to be averaged
			{			
				switch(this->m_AverageMethod)
				{
				case DWIBaselineAverager::BaselineOptimized:
					//std::cout << "BaselineOptimized" << std::endl;
					BaselineOptimizedAverage();
					this->computeIDWI();
					break;
				case DWIBaselineAverager::GradientOptamized:
					//std::cout << "GradientOptamized" << std::endl;
					GradientOptamizedAverage();
					//this->computeIDWI(); 
					break;
				case DWIBaselineAverager::Direct:
				default:
					//std::cout << "Direct" << std::endl;
					DirectAverage();
					this->computeIDWI();
					break;
				}
			}
		}
		return;
	}

	/** 
	*
	*/
	template<class TImageType>
	void 
		DWIBaselineAverager< TImageType>
		::GradientOptamizedAverage()
	{
		// 1: create idwi
		// 2 register all gradients into idwi
		// iteratively do  1 and 2 until idwi doesn't change any more(by threshold)

		// register all baselines into the final idwi then average.
		InputImageConstPointer  inputPtr = this->GetInput();

		typedef itk::VectorIndexSelectionCastImageFilter< TImageType, UnsignedImageType > FilterType;
		typename FilterType::Pointer componentExtractor = FilterType::New();
		componentExtractor->SetInput(inputPtr);

		// first do direct averaging idwi
		this->computeIDWI(); // init idwi with the original DW gradients

		//resampledGradient: used in registration(), containing the resampled image after transformation from registration.
		tempIDWI = doubleImageType::New();
		tempIDWI->CopyInformation(inputPtr);
		tempIDWI->SetRegions(inputPtr->GetLargestPossibleRegion());
		tempIDWI->Allocate();
		tempIDWI->FillBuffer(1);

		// iteratively register each DW gradient onto the idwi 
		std::vector< struRigidRegResult >	Results;
		struRigidRegResult result;
		result.AngleX = 0.0;
		result.AngleY = 0.0;
		result.AngleZ = 0.0;
		result.TranslationX = 0.0;
		result.TranslationY = 0.0;
		result.TranslationZ = 0.0;
		result.MutualInformation = 0.0;

		for( unsigned int i = 0; i< this->m_GradientDirectionContainer->Size(); i++ )
		{
			if( 0.0 !=	this->m_GradientDirectionContainer->ElementAt(i)[0] ||
				0.0 !=	this->m_GradientDirectionContainer->ElementAt(i)[1] ||
				0.0 !=	this->m_GradientDirectionContainer->ElementAt(i)[2]    )
			Results.push_back(result);
		}

		bool bRegister = true;
		unsigned int inertationCount = 0;
		std::cout<<"Gradient optamized averaging baseline ...";
		do 
		{
			inertationCount++;
			std::cout<<" iteration: "<< inertationCount<<"...";		

			double oldMISum = 0.0;
			double newMISum = 0.0;

			double oldTransSum = 0.0;
			double newTransSum = 0.0;

			double oldRotateSum = 0.0;
			double newRotateSum = 0.0;

			int gradientNum = 0;
			for( unsigned int i = 0; i< this-> m_GradientDirectionContainer->Size(); i++ )
			{
				if( 0.0 !=	this->m_GradientDirectionContainer->ElementAt(i)[0] ||
					0.0 !=	this->m_GradientDirectionContainer->ElementAt(i)[1] ||
					0.0 !=	this->m_GradientDirectionContainer->ElementAt(i)[2]    )
				{
					componentExtractor->SetIndex( i );
					componentExtractor->Update();


					// register and replace the DW gradient volumes with the idwi
// 					std::cout<<"Registering DW gradient #: "<<i<<" to current idwi ... ";
					struRigidRegResult resultLocal;
					rigidRegistration( this->idwi,componentExtractor->GetOutput()  , 25, 0.1, 1, resultLocal, 1 /* DW gradient */);
					std::cout<<" done "<<std::endl;

					Results.at(gradientNum) = resultLocal;
					gradientNum++;

// 					std::cout<<"DW gradient #: "<<i<<std::endl;
// 					std::cout<<"Angles: "<<resultLocal.AngleX<<" "<<resultLocal.AngleY<<" "<<resultLocal.AngleZ<<std::endl;
// 					std::cout<<"Trans : "<<resultLocal.TranslationX<<" "<<resultLocal.TranslationY<<" "<<resultLocal.TranslationZ<<std::endl;
// 					std::cout<<"MI    : "<<resultLocal.MutualInformation<<std::endl;		
				}
			}

			if( m_StopCriteria == DWIBaselineAverager::TotalTransformationBased )
			{
// 				std::cout<<"Stop_Criteria: TotalTransformationBased"<<std::endl;
// 				std::cout<<"StopThreshold: "<<m_StopThreshold<<std::endl;

				if( (fabs( newTransSum  - oldTransSum  )  < m_StopThreshold ) && (fabs( newRotateSum - oldRotateSum )< m_StopThreshold )) //0.01
					bRegister = false;	
			}

			if( m_StopCriteria == DWIBaselineAverager::MetricSumBased )
			{
// 				std::cout<<"Stop_Criteria: MetricSumBased"<<std::endl;
// 				std::cout<<"StopThreshold: "<<m_StopThreshold<<std::endl;

				if( fabs(newMISum-oldMISum)  < m_StopThreshold ) //0.001
					bRegister = false;		
			}

			//compute the idwi with registered DW gradient 
			typedef ImageRegionIteratorWithIndex< UnsignedImageType > idwiIterator;
			idwiIterator idwiIt(idwi, idwi->GetLargestPossibleRegion());
			idwiIt.GoToBegin();

			typename UnsignedImageType::IndexType pixelIndex;
			double meanSquarediff =0.0;
			double meanIntensity =0.0;
			unsigned short pixelValue;
			while ( !idwiIt.IsAtEnd() ) 
			{
				pixelIndex = idwiIt.GetIndex();
				pixelValue = idwiIt.Get();
				meanIntensity += pixelValue;
				idwiIt.Set( pow( tempIDWI->GetPixel(pixelIndex), 1.0/this->getGradientNumber()));
				meanSquarediff += (pixelValue - idwiIt.Get())*(pixelValue - idwiIt.Get());
				tempIDWI->SetPixel(pixelIndex, 1);
				++idwiIt;
			}
			tempIDWI->FillBuffer(1);

			UnsignedImageType::SizeType sizes;
			sizes = averagedBaseline->GetLargestPossibleRegion().GetSize();
			long voxelCount  = sizes[0]*sizes[1]*sizes[2];
			meanSquarediff = sqrt(meanSquarediff/voxelCount);
			meanIntensity = meanIntensity/voxelCount;
			double ratio = meanSquarediff/meanIntensity;

// 			std::cout<<"voxelCount                    : "<<voxelCount<<std::endl;
// 			std::cout<<"meanVoxelIntensity            : "<<meanIntensity<<std::endl;
// 			std::cout<<"meanVoxelSquareRootDifference : "<<meanSquarediff<<std::endl;
// 			std::cout<<"meanVoxelSquareRootDifference/meanVoxelIntensity: "<<ratio<<std::endl;

			if( m_StopCriteria == DWIBaselineAverager::IntensityMeanSquareDiffBased )
			{
// 				std::cout<<"Stop_Criteria: IntensityMeanSquareDiffBased"<<std::endl;
// 				std::cout<<"StopThreshold: "<<m_StopThreshold<<std::endl;

				if( ratio < m_StopThreshold ) //0.02
          {
					bRegister = false;
          }
			}

		} while ( bRegister && inertationCount<= GetMaxIteration() );

		// registered all the baseline into idwi, then average
		//////////////////////////////////////////////////////////////////////////
		tempBaseline = UnsignedImageType::New();
		tempBaseline->CopyInformation(inputPtr);
		tempBaseline->SetRegions(inputPtr->GetLargestPossibleRegion());
		tempBaseline->Allocate();
		tempBaseline->FillBuffer(0);

		for( unsigned int i = 0; i< this-> m_GradientDirectionContainer->Size(); i++ )
		{
			if( 0.0 ==	this->m_GradientDirectionContainer->ElementAt(i)[0] &&
				0.0 ==	this->m_GradientDirectionContainer->ElementAt(i)[1] &&
				0.0 ==	this->m_GradientDirectionContainer->ElementAt(i)[2]    )
			{
				componentExtractor->SetIndex( i );
				componentExtractor->Update();

				struRigidRegResult resultLocal;

				// register and replace the DW gradient volumes with the idwi
// 				std::cout<<"Registering baseline #: "<<i<<" to idwi ... ";
				rigidRegistration( this->idwi, componentExtractor->GetOutput(), 25, 0.1, 1, resultLocal, 0 /* baseline */);
// 				std::cout<<" done "<<std::endl;
			}
		}

		//average the registered baselines into averagedBaseline
		typedef ImageRegionIteratorWithIndex<UnsignedImageType> averagedBaselineIterator;
		averagedBaselineIterator aIt(averagedBaseline, averagedBaseline->GetLargestPossibleRegion());
		aIt.GoToBegin();

		typename UnsignedImageType::IndexType pixelIndex;
		unsigned short pixelValue;
		while ( !aIt.IsAtEnd() ) 
		{
			pixelIndex = aIt.GetIndex();
			pixelValue = aIt.Get();
			aIt.Set( static_cast<unsigned short>( tempBaseline->GetPixel(pixelIndex)/ (static_cast<double>(this->getGradientNumber()))));
			++aIt;
		}

// 		std::cout<<"DW gradient based optimized done"<<std::endl;
	}



	/** 
	*
	*/
	template<class TImageType>
	void 
		DWIBaselineAverager< TImageType>
		::BaselineOptimizedAverage()
	{
		//std::cout<<"registering all baseline onto averaged .";

		InputImageConstPointer  inputPtr = this->GetInput();

		// first do direct averaging baselines
		this->DirectAverage();

		tempBaseline = UnsignedImageType::New();
		tempBaseline->CopyInformation(inputPtr);
		tempBaseline->SetRegions(inputPtr->GetLargestPossibleRegion());
		tempBaseline->Allocate();
		tempBaseline->FillBuffer(0);

		// copy baseline images into baselineContainer
		std::vector< UnsignedImageType::Pointer > baselineContainer;
		for( unsigned int i = 0; i< inputPtr->GetVectorLength(); i++ )
		{
			if( this-> m_GradientDirectionContainer->ElementAt(i)[0] == 0.0 &&
				this-> m_GradientDirectionContainer->ElementAt(i)[1] == 0.0 && 
				this-> m_GradientDirectionContainer->ElementAt(i)[2] == 0.0     )
			{
				UnsignedImageType::Pointer baseline;
				baseline = UnsignedImageType::New();
				baseline->CopyInformation(inputPtr);
				baseline->SetRegions(inputPtr->GetLargestPossibleRegion());
				baseline->Allocate();
				baseline->FillBuffer(0);

				typedef ImageRegionIteratorWithIndex<UnsignedImageType> baselineIterator;
				baselineIterator aIt(baseline, baseline->GetLargestPossibleRegion());
				aIt.GoToBegin();

				typename UnsignedImageType::IndexType pixelIndex;
				while ( !aIt.IsAtEnd() ) 
				{
					// determine the index of the output pixel
					pixelIndex = aIt.GetIndex();
					unsigned short pixelValue = inputPtr->GetPixel(pixelIndex)[i];					
					aIt.Set( pixelValue );
					++aIt;
				}
				baselineContainer.push_back(baseline);
			}
		}

		// iteratively register each baseline onto the averaged baseline
		std::vector< struRigidRegResult >	Results;
		struRigidRegResult result;
		result.AngleX = 0.0;
		result.AngleY = 0.0;
		result.AngleZ = 0.0;
		result.TranslationX = 0.0;
		result.TranslationY = 0.0;
		result.TranslationZ = 0.0;
		result.MutualInformation = 0.0;

		for( unsigned int i = 0; i< baselineContainer.size(); i++ )
		{
			Results.push_back(result);
		}
		bool bRegister = true;
		unsigned int inertationCount = 0;

		std::cout<<"Baseline optimized averaging baseline ...";
		do 
		{
			inertationCount++;
			std::cout<<" iteration: "<< inertationCount<< "...";		

			double oldMISum = 0.0;
			double newMISum = 0.0;

			double oldTransSum = 0.0;
			double newTransSum = 0.0;

			double oldRotateSum = 0.0;
			double newRotateSum = 0.0;

			for( unsigned int i = 0; i<baselineContainer.size(); i++ )
			{
				struRigidRegResult resultLocal;
				
				// register and replace the baseline volumes with the transformed ones
// 				std::cout<<"Registering baseline #: "<<i<<" to current averaged baseline ... ";
				rigidRegistration( this->averagedBaseline, baselineContainer[i], 25, 0.1, 1, resultLocal, 0 /* baseline */);
// 				std::cout<<" done "<<std::endl;
				
				oldMISum += Results.at(i).MutualInformation;
				newMISum += resultLocal.MutualInformation;

				oldTransSum  += (fabs(Results.at(i).TranslationX) + fabs(Results.at(i).TranslationY) + fabs(Results.at(i).TranslationZ))/3.0;
				newTransSum  += (fabs(resultLocal.TranslationX) + fabs(resultLocal.TranslationY) + fabs(resultLocal.TranslationZ))/3.0;

				oldRotateSum += (fabs(Results.at(i).AngleX) + fabs(Results.at(i).AngleY) + fabs(Results.at(i).AngleZ))/3.0;
				newRotateSum += (fabs(resultLocal.AngleX) + fabs(resultLocal.AngleY) + fabs(resultLocal.AngleZ))/3.0;

				Results.at(i) = resultLocal;

// 				std::cout<<"baseline #: "<<i<<std::endl;
// 				std::cout<<"Angles: "<<resultLocal.AngleX<<" "<<resultLocal.AngleY<<" "<<resultLocal.AngleZ<<std::endl;
// 				std::cout<<"Trans : "<<resultLocal.TranslationX<<" "<<resultLocal.TranslationY<<" "<<resultLocal.TranslationZ<<std::endl;
//  			std::cout<<"MI    : "<<resultLocal.MutualInformation<<std::endl;		
			}


// 			std::cout<<"oldTransSum:  "<<oldTransSum<<std::endl;
// 			std::cout<<"newTransSum:  "<<newTransSum<<std::endl;
// 			std::cout<<"oldRotateSum: "<<oldRotateSum<<std::endl;
// 			std::cout<<"newRotateSum: "<<newRotateSum<<std::endl;
// 			std::cout<<"|newTransSum  - oldTransSum |: "<< fabs( newTransSum  - oldTransSum  )<<std::endl;
// 			std::cout<<"|newRotateSum - oldRotateSum|: "<< fabs( newRotateSum - oldRotateSum )<<std::endl;

			if( m_StopCriteria == DWIBaselineAverager::TotalTransformationBased )
			{
// 				std::cout<<"Stop_Criteria: TotalTransformationBased"<<std::endl;
// 				std::cout<<"StopThreshold: "<<m_StopThreshold<<std::endl;

				if( (fabs( newTransSum  - oldTransSum  )  < m_StopThreshold ) && (fabs( newRotateSum - oldRotateSum )< m_StopThreshold )) //0.01
					bRegister = false;	
			}

// 			std::cout<<"oldMISum: "<<oldMISum<<std::endl;
// 			std::cout<<"newMISum: "<<newMISum<<std::endl;
// 			std::cout<<"|newMISum-oldMISum|: "<<fabs(newMISum-oldMISum)<<std::endl;

			if( m_StopCriteria == DWIBaselineAverager::MetricSumBased )
			{
// 				std::cout<<"Stop_Criteria: MetricSumBased"<<std::endl;
// 				std::cout<<"StopThreshold: "<<m_StopThreshold<<std::endl;

				if( fabs(newMISum-oldMISum)  < m_StopThreshold ) //0.001
					bRegister = false;		
			}

			//average the registered baselines into averagedBaseline
			typedef ImageRegionIteratorWithIndex<UnsignedImageType> averagedBaselineIterator;
			averagedBaselineIterator aIt(averagedBaseline, averagedBaseline->GetLargestPossibleRegion());
			aIt.GoToBegin();

			typename UnsignedImageType::IndexType pixelIndex;
			double meanSquarediff =0.0;
			double meanIntensity =0.0;
			unsigned short pixelValue;
			while ( !aIt.IsAtEnd() ) 
			{
				pixelIndex = aIt.GetIndex();
				pixelValue = aIt.Get();
				meanIntensity += pixelValue;
				aIt.Set( static_cast<unsigned short>( tempBaseline->GetPixel(pixelIndex)/ (static_cast<double>(baselineContainer.size()))));
				meanSquarediff += (pixelValue - aIt.Get())*(pixelValue - aIt.Get());
				tempBaseline->SetPixel(pixelIndex, 0);
				++aIt;
			}
			tempBaseline->FillBuffer(0);

			UnsignedImageType::SizeType sizes;
			sizes = averagedBaseline->GetLargestPossibleRegion().GetSize();
			long voxelCount  = sizes[0]*sizes[1]*sizes[2];
			meanSquarediff = sqrt(meanSquarediff/voxelCount);
			meanIntensity = meanIntensity/voxelCount;
			double ratio = meanSquarediff/meanIntensity;

// 			std::cout<<"voxelCount                    : "<<voxelCount<<std::endl;
// 			std::cout<<"meanVoxelIntensity            : "<<meanIntensity<<std::endl;
// 			std::cout<<"meanVoxelSquareRootDifference : "<<meanSquarediff<<std::endl;
// 			std::cout<<"meanVoxelSquareRootDifference/meanVoxelIntensity: "<<ratio<<std::endl;

			if( m_StopCriteria == DWIBaselineAverager::IntensityMeanSquareDiffBased )
			{
// 				std::cout<<"Stop_Criteria: IntensityMeanSquareDiffBased"<<std::endl;
// 				std::cout<<"StopThreshold: "<<m_StopThreshold<<std::endl;

				if( ratio < m_StopThreshold ) //0.01
          {
          bRegister = false;
          }
			}

		} while ( bRegister && inertationCount<= GetMaxIteration()   );

 		std::cout<<"DONE"<<std::endl;
	}

	/** 
	*
	*/
	template<class TImageType>
	void 
		DWIBaselineAverager< TImageType>
		::DirectAverage()
	{
		if( !averagedBaseline)
			return;

		typedef ImageRegionIteratorWithIndex<UnsignedImageType> averagedBaselineIterator;
		averagedBaselineIterator aIt(averagedBaseline, averagedBaseline->GetLargestPossibleRegion());
		aIt.GoToBegin();

		InputImageConstPointer  inputPtr = this->GetInput();
		typename UnsignedImageType::IndexType pixelIndex;
		double pixelValue ;
		while ( !aIt.IsAtEnd() ) 
		{
			// determine the index of the output pixel
			pixelIndex = aIt.GetIndex();
			pixelValue = 0.0;
			for(unsigned int i = 0 ; i < inputPtr->GetVectorLength(); i++ )
			{
				if( this->m_GradientDirectionContainer->ElementAt(i)[0] == 0.0 &&
					this->m_GradientDirectionContainer->ElementAt(i)[1] == 0.0 && 
					this->m_GradientDirectionContainer->ElementAt(i)[2] == 0.0     )
				{
					//std::cout<<" GetPixel(pixelIndex): "<< inputPtr->GetPixel(pixelIndex)[i] <<std::endl;
					pixelValue += inputPtr->GetPixel(pixelIndex)[i]/(static_cast<double>(getBaselineNumber()));					
				}
			}
			aIt.Set( static_cast<unsigned short>(pixelValue) );
			++aIt;
		}		
	}

	/** 
	*
	*/
	template<class TImageType>
	void 
		DWIBaselineAverager< TImageType>
		::writeReport()
	{
		if( GetReportFileName().length()==0 )
			return;

		std::ofstream outfile;

		if( GetReportFileMode()== DWIBaselineAverager::Report_Append )
			outfile.open( GetReportFileName().c_str(), std::ios_base::app);
		else
			outfile.open( GetReportFileName().c_str());

		outfile <<std::endl;
		outfile <<"================================"<<std::endl;
		outfile <<"       Baseline averaging       "<<std::endl;
		outfile <<"================================"<<std::endl;

		outfile <<std::endl;

		switch(this->m_AverageMethod)
		{
		case DWIBaselineAverager::Direct:
			outfile <<"Average Method: [DWIBaselineAverager::Direct]--Direct arithmetic averaging "<<std::endl;
			break;
		case DWIBaselineAverager::BaselineOptimized:
			outfile <<"Average Method: [DWIBaselineAverager::BaselineOptimized]--Averaging Optimized based on baseline images"<<std::endl;
			break;
		case DWIBaselineAverager::GradientOptamized:
			outfile <<"Average Method: [DWIBaselineAverager::GradientOptamized]--Averaging Optimized based on DW gradients images "<<std::endl;
			break;
		default:
			outfile <<"Average Method: UNKNOWN--Using direct arithmetic averaging "<<std::endl;
			break;
		}

		outfile <<"Stop threshold: " << this->m_StopThreshold << std::endl;

		outfile <<std::endl<<"======"<<std::endl;
		outfile<<"Input Diffusion Gradient information:" <<std::endl;
		outfile<<"\tbValueNumber: "		<<bValueNumber		<<std::endl;
		outfile<<"\tbaselineNumber: "	<<baselineNumber	<<std::endl;
		outfile<<"\tgradientDirNumber: "<<gradientDirNumber	<<std::endl;
		outfile<<"\trepetitionNumber: " <<repetitionNumber	<<std::endl;

		outfile<<std::endl<<"\t# "<<"\tDirVector"<<std::endl;
		for(unsigned int i=0;i< this->m_GradientDirectionContainer->size();i++)
		{
			outfile<<"\t"<<i<<"\t[ " 
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<m_GradientDirectionContainer->ElementAt(i)[0]<<", "
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<m_GradientDirectionContainer->ElementAt(i)[1]<<", "
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<m_GradientDirectionContainer->ElementAt(i)[2]<<" ]"
				<<std::endl;
		}

		outfile <<std::endl<<"======"<<std::endl;
		outfile<<"Output Diffusion Gradient direction information:"<<std::endl;

		outfile<<std::endl<<"\t# "<<"\tDirVector"<<std::endl;

		int temp=0;
		if(getBaselineNumber()>0)
		{
			outfile<<"\t"<<temp<<"\t[ " 
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<0.0<<", "
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<0.0<<", "
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<0.0<<" ]"
				<<std::endl;
			temp=1;
		}

		for(unsigned int i=0;i< this->m_GradientDirectionContainer->size();i++)
		{

			if( 0.0 ==	this->m_GradientDirectionContainer->ElementAt(i)[0] &&
				0.0 ==	this->m_GradientDirectionContainer->ElementAt(i)[1] &&
				0.0 ==	this->m_GradientDirectionContainer->ElementAt(i)[2]    )
				continue;

			outfile<<"\t"<<temp<<"\t[ " 
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<m_GradientDirectionContainer->ElementAt(i)[0]<<", "
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<m_GradientDirectionContainer->ElementAt(i)[1]<<", "
				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
				<<m_GradientDirectionContainer->ElementAt(i)[2]<<" ]"
				<<std::endl;
			temp++;
		}

		outfile.close();
		return;
	}

	/** 
	*
	*/
	template<class TImageType>
	void 
		DWIBaselineAverager< TImageType>
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
				std::cout<<"Warring: Not all the gradient directions have same repetition. "<<std::endl;
				repetitionNumber = -1;			
			}
		}

		this->gradientNumber = this->m_GradientDirectionContainer->size()-this->baselineNumber;

		// 		std::cout<<"DWI Diffusion Information: "		<<std::endl;
		// 		std::cout<<"  baselineNumber: "		<<baselineNumber	<<std::endl;
		// 		std::cout<<"  bValueNumber: "		<<bValueNumber		<<std::endl;
		// 		std::cout<<"  gradientDirNumber: "	<<gradientDirNumber	<<std::endl;
		// 		std::cout<<"  gradientNumber: "		<<gradientNumber	<<std::endl;
		// 		std::cout<<"  repetitionNumber: "	<<repetitionNumber	<<std::endl;
		return;
	}


	/** 
	*
	*/
	template<class TImageType>
	void 
		DWIBaselineAverager< TImageType>
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
		// 		for(unsigned int i=0;i< this->m_GradientDirectionContainer -> Size();i++ )
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
	template<class TImageType>
	void 
		DWIBaselineAverager< TImageType>
		::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,int threadId )
	{


		itkDebugMacro(<<"Actually executing");

		// Get the input and output pointers
		InputImageConstPointer  inputPtr = this->GetInput();
		OutputImagePointer      outputPtr = this->GetOutput();

		// Define/declare an iterator that will walk the output region for this
		// thread.
		typedef ImageRegionIteratorWithIndex<TImageType> OutputIterator;

		OutputIterator outIt(outputPtr, outputRegionForThread);
		outIt.GoToBegin();

		// Define a few indices that will be used to translate from an input pixel
		// to an output pixel
		typename TImageType::IndexType outputIndex;
		typename TImageType::IndexType  inputIndex;

		// support progress methods/callbacks
		ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

		typename TImageType::PixelType value ;
		if( getBaselineNumber()==0 || getBaselineNumber()==1 ) //copy input to output
		{
			value.SetSize( inputPtr->GetVectorLength() ) ;
			while ( !outIt.IsAtEnd() ) 
			{
				// determine the index of the output pixel
				outputIndex = outIt.GetIndex();

				// determine the input pixel location associated with this output pixel
				inputIndex = outputIndex;
				outIt.Set( inputPtr->GetPixel(inputIndex));
				++outIt;

				progress.CompletedPixel();
			}			
		}
		else // combine averagedBaseline and DWIs
		{
			value.SetSize( this->gradientNumber+1 ) ;
			while ( !outIt.IsAtEnd() ) 
			{
				// determine the index of the output pixel
				outputIndex = outIt.GetIndex();

				// determine the input pixel location associated with this output pixel
				inputIndex = outputIndex;

				value.SetElement(0, averagedBaseline->GetPixel(inputIndex));

				int element = 1;
				for( unsigned int i = 0 ; i < inputPtr->GetVectorLength(); i++ )
				{
					if( this->m_GradientDirectionContainer->ElementAt(i)[0] == 0.0 &&
						this->m_GradientDirectionContainer->ElementAt(i)[1] == 0.0 && 
						this->m_GradientDirectionContainer->ElementAt(i)[2] == 0.0     )
						continue;
					else
					{
						value.SetElement( element , inputPtr->GetPixel(inputIndex)[i] ) ;
						element++;
					}
				}

				outIt.Set( value);
				++outIt;

				progress.CompletedPixel();
			}
		}
	}

	/** 
	*
	*/
	template<class TImageType>
	void  
		DWIBaselineAverager< TImageType>
		::rigidRegistration(
		UnsignedImageType::Pointer fixed, 
		UnsignedImageType::Pointer moving,
		unsigned int	BinsNumber,
		double			SamplesPercent,
		bool			/* ExplicitPDFDerivatives */,
		struRigidRegResult&  regResult,
		int baselineORidwi // 0: baseline; otherwise: idwi
		)
	{
		//setup pipeline
		typedef itk::VersorRigid3DTransform< double >   TransformType;
		typedef itk::VersorRigid3DTransformOptimizer     OptimizerType;
		typedef itk::MattesMutualInformationImageToImageMetric< UnsignedImageType, UnsignedImageType >   MetricType;
		typedef itk::LinearInterpolateImageFunction< UnsignedImageType ,double >    InterpolatorType;
		typedef itk::ImageRegistrationMethod< UnsignedImageType,UnsignedImageType >    RegistrationType;

		typedef UnsignedImageType::SpacingType    SpacingType;
		typedef UnsignedImageType::PointType      OriginType;
		typedef UnsignedImageType::RegionType     RegionType;
		typedef UnsignedImageType::SizeType       SizeType;

		MetricType::Pointer				metric			= MetricType::New();
		OptimizerType::Pointer			optimizer		= OptimizerType::New();
		InterpolatorType::Pointer		interpolator	= InterpolatorType::New();
		RegistrationType::Pointer		registration	= RegistrationType::New();
		TransformType::Pointer			transform		= TransformType::New();

		UnsignedImageType::Pointer		fixedImage = fixed;
		UnsignedImageType::Pointer		movingImage = moving;

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

		typedef itk::CenteredTransformInitializer< TransformType, UnsignedImageType, UnsignedImageType >  TransformInitializerType;
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

		//std::cout<<"UseNumberOfSpatialSamples: "<<(int)(fixedImage->GetPixelContainer()->Size() * percentOfSamples)<<std::endl;
		int SampleSize = (int)(fixedImage->GetPixelContainer()->Size() * percentOfSamples);
		if( SampleSize> 100000 )	 metric->SetNumberOfSpatialSamples( SampleSize );
		else  metric->UseAllPixelsOn();
		//metric->SetUseExplicitPDFDerivatives( useExplicitPDFDerivatives ); //true for small # of parameters; false for big # of transform paramrters

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
		regResult.MutualInformation=-bestValue;
		
		// resample the input baselines
		typedef itk::ResampleImageFilter< UnsignedImageType,UnsignedImageType > ResampleFilterType;
		ResampleFilterType::Pointer resampler = ResampleFilterType::New();
		resampler->SetInput( movingImage );
		resampler->SetTransform( registration->GetOutput()->Get() );
		resampler->SetSize( movingImage->GetLargestPossibleRegion().GetSize() );
		resampler->SetOutputOrigin( movingImage->GetOrigin() );
		resampler->SetOutputSpacing( movingImage->GetSpacing() );
		resampler->SetDefaultPixelValue( 0 );
		resampler->Update();

		UnsignedImageType::Pointer transformedImage =  UnsignedImageType::New();
		transformedImage =  resampler->GetOutput();

		typedef ImageRegionIteratorWithIndex<UnsignedImageType> transformedImageIterator;
		transformedImageIterator transIt(transformedImage, transformedImage->GetLargestPossibleRegion());
		transIt.GoToBegin();
		typename UnsignedImageType::IndexType pixelIndex;
		while ( !transIt.IsAtEnd() ) 
		{					
			pixelIndex = transIt.GetIndex();
			if(baselineORidwi==0) //baseline
				tempBaseline->SetPixel( pixelIndex, tempBaseline->GetPixel( pixelIndex ) + (int)(transIt.Get()+0.5));
			else // DW gradient
				tempIDWI->SetPixel( pixelIndex, tempIDWI->GetPixel( pixelIndex ) * transIt.Get());

			//movingImage->SetPixel( pixelIndex, transIt.Get()); // replace the input image: this will result in aggressive smoothing
			++transIt;
		}
		return;
	}


	/** 
	*
	*/
	template<class TImageType>
	void  
		DWIBaselineAverager< TImageType>
		::computeIDWI()
	{
		InputImageConstPointer  inputPtr = this->GetInput();

		idwi = UnsignedImageType::New();
		idwi->CopyInformation(inputPtr);
		idwi->SetRegions(inputPtr->GetLargestPossibleRegion());
		idwi->Allocate();
		idwi->FillBuffer(0);

		typedef ImageRegionIteratorWithIndex<UnsignedImageType> idwiIterator;
		idwiIterator idwiIt(idwi, idwi->GetLargestPossibleRegion());
		idwiIt.GoToBegin();

		typename UnsignedImageType::IndexType pixelIndex;
		double pixelValue ;
		while ( !idwiIt.IsAtEnd() ) 
		{
			// determine the index of the output pixel
			pixelIndex = idwiIt.GetIndex();
			pixelValue = 1.0;
			for(unsigned int i = 0 ; i < inputPtr->GetVectorLength(); i++ )
			{
				if( this->m_GradientDirectionContainer->ElementAt(i)[0] != 0.0 ||
					this->m_GradientDirectionContainer->ElementAt(i)[1] != 0.0 || 
					this->m_GradientDirectionContainer->ElementAt(i)[2] != 0.0     )
				{
					pixelValue *= inputPtr->GetPixel(pixelIndex)[i];					
				}
			}
			idwiIt.Set( static_cast<unsigned int>(pow( pixelValue, 1.0/this->getGradientNumber())) );
			++idwiIt;
		}
	}

	/** 
	*
	*/
	template<class TImageType>
	itk::Image<unsigned int, 3>::Pointer
		DWIBaselineAverager< TImageType>
		::GetBaseline()
	{	
		if(GetAverageDone())
		{
			if(averagedBaseline)
				return averagedBaseline;
			else
			{
				std::cout<<"No baseline found."<<std::endl;
				return NULL;
			}
		}
		else
		{
			std::cout<<"You should update this filter first."<<std::endl;
			return NULL;
		}
	}


	/** 
	*
	*/
	template<class TImageType>
	itk::Image<unsigned int, 3>::Pointer
		DWIBaselineAverager< TImageType>
		::GetIDWI()
	{
		if(GetAverageDone())
		{
			if( idwi)
				return idwi;
			else
			{
				std::cout<<"No idwi created."<<std::endl;
				return NULL;
			}
		}
		else
		{
			std::cout<<"You should update this filter first."<<std::endl;
			return NULL;
		}
	}
}
#endif

