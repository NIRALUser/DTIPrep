/*=========================================================================

Program:   NeuroLib
Module:    $file: itkDWIQCSliceChecker.cpp $
Language:  C++
Date:      $Date: 2009-08-27 01:39:40 $
Version:   $Revision: 1.2 $
Author:    Zhexing Liu (liuzhexing@gmail.com)

Copyright (c) NIRAL, UNC. All rights reserved.
See http://www.niral.unc.edu for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _itkDWIQCSliceChecker_cpp
#define _itkDWIQCSliceChecker_cpp

#include "itkDWIQCSliceChecker.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkExceptionObject.h"
#include "itkProgressReporter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkExtractImageFilter.h"
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_matrix_inverse.h"

#include <iostream>
#include <iomanip>
#include <fstream>

namespace itk
{
	/**
	*
	*/
	template<class TImageType>
	DWIQCSliceChecker< TImageType>
		::DWIQCSliceChecker()
	{
		m_CheckTimes = 0;

		m_HeadSkipRatio = 0.1;
		m_TailSkipRatio = 0.1;
		m_BaselineStdevTimes = 3.5;
		m_GradientStdevTimes = 3.5;

		b0 = -1.0 ;

		m_ReportFileName = "";
		m_ReportFileMode = DWIQCSliceChecker::Report_New;

		bValues.clear();

		CheckDoneOff();
	}

	/**
	*
	*/
	template<class TImageType>
	DWIQCSliceChecker< TImageType>
		::~DWIQCSliceChecker()
	{
	}

	/**
	* PrintSelf
	*/
	template<class TImageType>
	void 
		DWIQCSliceChecker<TImageType>
		::PrintSelf(std::ostream& os, Indent indent) const
	{
		Superclass::PrintSelf(os,indent);
		os << indent << "DWI QC slice-wise check filter: " << std::endl;
	}

	/**
	* initialize QC Resullts
	*/
	template<class TImageType>
	void 
		DWIQCSliceChecker<TImageType>
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
		DWIQCSliceChecker<TImageType>
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

		// perform SliceWiseCheck
		//std::cout << "Slice wise checking begins here. " <<std::endl;

		parseGridentDirections();
		collectDiffusionStatistics();
		initializeQCResullts() ;
		calculateCorrelations();
		iterativeCheck();
		//validateLeftDiffusionStatistics();
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

		// thickness
		if(imgMetaDictionary.HasKey("NRRD_thicknesses[2]"))
		{
			double thickness;
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

		// gradient vectors
		int temp=0;
		for(int i=0;i< this->m_GradientDirectionContainer -> Size();i++ )
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
		DWIQCSliceChecker<TImageType>
		::calculateCorrelations()
	{
		std::cout<<"Slice calculating .";

		InputImageConstPointer  inputPtr = this->GetInput();

		typedef itk::VectorIndexSelectionCastImageFilter< TImageType, GradientImageType > FilterType;
		typename FilterType::Pointer componentExtractor = FilterType::New();
		componentExtractor->SetInput(inputPtr);

		typedef itk::ExtractImageFilter< GradientImageType, SliceImageType > ExtractFilterType;
		typename ExtractFilterType::Pointer filter1 = ExtractFilterType::New();
		typename ExtractFilterType::Pointer filter2 = ExtractFilterType::New();

		for( unsigned int j = 0; j<inputPtr->GetVectorLength(); j++ )
		{
			componentExtractor->SetIndex( j );
			componentExtractor->Update();

			GradientImageType::RegionType inputRegion =componentExtractor->GetOutput()->GetLargestPossibleRegion();
			GradientImageType::SizeType size = inputRegion.GetSize();
			size[2] = 0;

			GradientImageType::IndexType start1 = inputRegion.GetIndex();
			GradientImageType::IndexType start2 = inputRegion.GetIndex();

			//std::cout<<"Gradient "<<j<<std::endl;

			std::vector< double > Results;
			filter1->SetInput( componentExtractor->GetOutput() );
			filter2->SetInput( componentExtractor->GetOutput() );	

			for(int i=1; i<componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[2]; i++)
			{
				start1[2] = i-1;
				start2[2] = i;

				typename GradientImageType::RegionType desiredRegion1;
				desiredRegion1.SetSize( size );
				desiredRegion1.SetIndex( start1 );
				filter1->SetExtractionRegion( desiredRegion1 );

				typename GradientImageType::RegionType desiredRegion2;
				desiredRegion2.SetSize( size );
				desiredRegion2.SetIndex( start2 );
				filter2->SetExtractionRegion( desiredRegion2 );

				filter1->Update();
				filter2->Update();

				typedef itk::ImageRegionConstIterator<SliceImageType>  citType;
				citType cit1( filter1->GetOutput(), filter1->GetOutput()->GetLargestPossibleRegion() );
				citType cit2( filter2->GetOutput(), filter2->GetOutput()->GetLargestPossibleRegion() );
				cit1.GoToBegin();
				cit2.GoToBegin();

				double correlation;
				double sAB=0.0,sA2=0.0,sB2=0.0;
				while (!cit1.IsAtEnd())
				{
					sAB += (cit1.Get()) * (cit2.Get());
					sA2 += (cit1.Get()) * (cit1.Get());
					sB2 += (cit2.Get()) * (cit2.Get());
					++cit1;
					++cit2;
				}
				if( sA2*sB2 == 0 )
				{
					//if(sA2==sB2)
						correlation = 1.0;
					//else
					//	correlation = 0.0;
				}
				else
				{
					correlation = sAB/sqrt(sA2*sB2);
				}
				Results.push_back(correlation);
			}			
			this->ResultsContainer.push_back(Results);			
			std::cout<<".";
		}
		std::cout<<" DONE"<<std::endl;
		return;

	}

	/** 
	*
	*/
	template <class TImageType>
	void 
		DWIQCSliceChecker<TImageType>
		::report()
	{
		bool bOutput = 0;
		std::ofstream outfile; 
		if( this->GetReportFileName().length()>0 )
		{

			if( GetReportFileMode()== DWIQCSliceChecker::Report_Append )
				outfile.open( GetReportFileName().c_str(), std::ios_base::app);
			else
				outfile.open( GetReportFileName().c_str());

			outfile <<"Slice-wise correlations: "<<std::endl<<std::endl;
			for(int i=0; i<this->ResultsContainer.size(); i++)
			{
				outfile <<"\t"<<"Gradient"<<i;
			}
			outfile <<"\tbaselineMean"<<"\tbaselineDeviation"<<"\tmean"<<"\tdeviation"<<std::endl;

			bOutput = 1;
		}

		int DWICount, BaselineCount;
		BaselineCount	= getBaselineNumber();
		DWICount		= getGradientNumber();

		std::cout <<"BaselineCount: "<<BaselineCount<<std::endl;
		std::cout <<"DWICount: "<<DWICount<<std::endl;

		for(int j=0;j<ResultsContainer[0].size();j++)
		{
			double baselinemean=0.0, DWImean=0.0, baselinedeviation=0.0, DWIdeviation=0.0;
			for(int i=0; i<ResultsContainer.size(); i++)
			{
				if(bOutput)
				{
					outfile.precision(6);
					outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;	
					outfile <<"\t"<<std::setw(10)<<ResultsContainer[i][j];
					outfile.precision();
					outfile.setf(std::ios_base::unitbuf);
				}
				if ( this->m_GradientDirectionContainer->at(i)[0] == 0.0 && 
					this->m_GradientDirectionContainer->at(i)[1] == 0.0 && 
					this->m_GradientDirectionContainer->at(i)[2] == 0.0    )
				{
					baselinemean+=this->ResultsContainer[i][j]/(double)(BaselineCount);
				}
				else
				{
					DWImean+=this->ResultsContainer[i][j]/(double)(DWICount);
				}				
			}

			for(int i=0; i<this->ResultsContainer.size(); i++)
			{
				if ( this->m_GradientDirectionContainer->at(i)[0] == 0.0 && 
					this->m_GradientDirectionContainer->at(i)[1] == 0.0 && 
					this->m_GradientDirectionContainer->at(i)[2] == 0.0    )
				{
					if(BaselineCount>=1)
						baselinedeviation+=(ResultsContainer[i][j]-baselinemean)*(ResultsContainer[i][j]-baselinemean)/(double)(BaselineCount);
					else
						baselinedeviation=0.0;
				}
				else
				{
					if(DWICount>=1)
						DWIdeviation+=(ResultsContainer[i][j]-DWImean)*(ResultsContainer[i][j]-DWImean)/(double)(DWICount);
					else
						DWIdeviation=0.0;
				}
			}

			if(bOutput)
			{
				outfile <<"\t"<<std::setw(10)<<baselinemean;
				outfile <<"\t"<<std::setw(10)<<sqrt(baselinedeviation);
				outfile <<"\t"<<std::setw(10)<<DWImean;
				outfile <<"\t"<<std::setw(10)<<sqrt(DWIdeviation);
				outfile <<std::endl;
			}

			this->baselineMeans.push_back(baselinemean);
			this->baselineDeviations.push_back(sqrt(baselinedeviation));

			this->gradientMeans.push_back(DWImean);
			this->gradientDeviations.push_back(sqrt(DWIdeviation));
		}

		if(bOutput)
		{
			outfile <<std::endl<<"Slice-wise Check Artifacts:"<<std::endl;
			outfile <<"\t"<<std::setw(10)<<"Gradient"<<"\t"<<std::setw(10)<<"Slice"<<"\t"<<std::setw(10)<<"Correlation"<<std::endl;
		}

		std::cout <<std::endl<<"Slice-wise Check Artifacts:"<<std::endl;
		std::cout<<"\t"<<std::setw(10)<<"Gradient"<<"\t"<<std::setw(10)<<"Slice"<<"\t"<<std::setw(10)<<"Correlation"<<std::endl;

		int badcount=0;
		int baselineBadcount=0;

		InputImageConstPointer  inputPtr = this->GetInput();
		for(int i=0;i<ResultsContainer.size();i++)
		{
			baselineBadcount = 0;
			badcount=0;
			if ( this->m_GradientDirectionContainer->at(i)[0] == 0.0 &&
				this->m_GradientDirectionContainer->at(i)[1] == 0.0 && 
				this->m_GradientDirectionContainer->at(i)[2] == 0.0     )
			{	
				//baseline
				for(int j = 0 + (int)(inputPtr->GetLargestPossibleRegion().GetSize()[2] * this->m_HeadSkipRatio);
					j < ResultsContainer[0].size() - (int)( inputPtr->GetLargestPossibleRegion().GetSize()[2]*this->m_TailSkipRatio); j++ ) 
				{
					if(bOutput)
					{
						outfile.precision(6);
						outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;
					}

					if( this->ResultsContainer[i][j] < baselineMeans[j] - baselineDeviations[j] * m_BaselineStdevTimes)
					{
						if(bOutput)
						{
							outfile  <<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j]<<std::endl;
						}
						std::cout<<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j]<<std::endl;
						this->ResultsContainer[i][j] = -this->ResultsContainer[i][j]; // to indicate a bad slice
						baselineBadcount++;
					}
					if(bOutput)
					{
						outfile.precision();
						outfile.setf(std::ios_base::unitbuf);
					}
				}
				if( baselineBadcount > 0 )
				{
					this->qcResults[i] = 0;	
				}
			}
			else // gradients
			{			
				for(int j=0 + (int)(inputPtr->GetLargestPossibleRegion().GetSize()[2]*this->m_HeadSkipRatio);
					j<ResultsContainer[0].size() - (int)(inputPtr->GetLargestPossibleRegion().GetSize()[2]*this->m_TailSkipRatio); j++) 
				{
					if(bOutput)
					{
						outfile.precision(6);
						outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;
					}

					if( ResultsContainer[i][j] < this->gradientMeans[j] - this->gradientDeviations[j] * this->m_GradientStdevTimes) // ok
					{
						if(bOutput)
						{
							outfile  <<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j]<<std::endl;
						}
						std::cout<<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<ResultsContainer[i][j]<<std::endl;
						this->ResultsContainer[i][j] = -this->ResultsContainer[i][j]; // to indicate a bad slice
						badcount++;
					}
					if(bOutput)
					{
						outfile.precision();
						outfile.setf(std::ios_base::unitbuf);
					}
				}
				if( badcount > 0 )
				{
					this->qcResults[i] = 0;
				}
			}		
		}
		if(bOutput)
		{
			outfile.close();
		}
		return;
	}

	/** 
	*
	*/
	template <class TImageType>
	void 
		DWIQCSliceChecker<TImageType>
		::check()
	{
		collectLeftDiffusionStatistics();

		int DWICount, BaselineCount;
		BaselineCount	= getBaselineLeftNumber();
		DWICount		= getGradientLeftNumber();

		this->baselineMeans.clear();
		this->baselineDeviations.clear();

		this->gradientMeans.clear();
		this->gradientDeviations.clear();

		this->quardraticFittedMeans.clear();
		this->quardraticFittedDeviations.clear();

		std::vector< std::vector<double> > normalizedMetric;
		for (int i=0; i<ResultsContainer.size(); i++ )
		{
			std::vector<double> temp;
			for (int j=0; j<ResultsContainer[0].size(); j++ )
			{
				temp.push_back(-1.0);
			}
			normalizedMetric.push_back(temp);
		}

		if( getBValueLeftNumber() >=3 || (getBValueLeftNumber() ==2 && getBaselineLeftNumber()>0)) // ensure a quardratic fit
		// multiple b valued DWI, do a quadratic-curve fitting between b-value and image correlation at each slice position
		{

			for(int j=0;j<ResultsContainer[0].size();j++) // for each slice
			{
				vnl_matrix<double> bMatrix( getBaselineLeftNumber()+getGradientLeftNumber() , 3);
				vnl_matrix<double> correlationVector(getBaselineLeftNumber()+getGradientLeftNumber() , 1);
				int matrixLineNumber = 0;
				for(int i=0; i<this->ResultsContainer.size(); i++) // for each gradient
				{
					if(this->qcResults[i])
					{
						bMatrix[matrixLineNumber][0] = this->bValues[i] *this->bValues[i] ;						
						bMatrix[matrixLineNumber][1] = this->bValues[i] ;						
						bMatrix[matrixLineNumber][2] = 1.0;						

						correlationVector[matrixLineNumber][0] = this->ResultsContainer[i][j];

						matrixLineNumber++;
					}
				}

				vnl_matrix<double> coefficients(3 , 1);
				coefficients = vnl_matrix_inverse<double>(bMatrix.transpose()*bMatrix);
				coefficients = coefficients*bMatrix.transpose()*correlationVector;

				for(int i=0; i<this->ResultsContainer.size(); i++) // for each gradient
				{
					if(this->qcResults[i])
					{
						normalizedMetric[i][j] = this->ResultsContainer[i][j] - (this->bValues[i] *this->bValues[i]*coefficients[0][0]+this->bValues[i]*coefficients[1][0]+coefficients[2][0]);
					}
				}
			}

			// to calculate the mean and stdev of the quardratic fitted correlations
			for(int j=0;j<ResultsContainer[0].size();j++)
			{
				double mean=0.0, deviation=0.0; 
				for(int i=0; i<this->ResultsContainer.size(); i++)
				{
					if(this->qcResults[i])
					{
						mean += normalizedMetric[i][j];
					}
				}
				
				mean = mean/static_cast<double>(DWICount+BaselineCount);
				for(int i=0; i<this->ResultsContainer.size(); i++)
				{
					if(this->qcResults[i])
					{
						if( DWICount+BaselineCount > 1)
							deviation+=(normalizedMetric[i][j]-mean)*(normalizedMetric[i][j]-mean)/(double)(DWICount+BaselineCount-1);
						else
							deviation=0.0;
					}
				}

				this->quardraticFittedMeans.push_back(mean);
				this->quardraticFittedDeviations.push_back(sqrt(deviation));

// 				std::cout<<"slice #: "<<j<<std::endl;
// 				std::cout<<" mean: "<<mean<<std::endl;
// 				std::cout<<" deviation: "<<sqrt(deviation)<<std::endl;
// 				std::cout<<" DWICount+BaselineCount: "<<DWICount+BaselineCount<<std::endl;

			}
		}
		else
		// single b value( baseline + bvalue or 2 different b values, [2 different b values not yet implemented])
		{
// 			std::cout<<" single b value( baseline + bvalue or 2 different b values, [2 different b values not yet implemented]"<<std::endl;
// 			std::cout<<" performing single b value checking computing"<<std::endl;

			for(int j=0;j<ResultsContainer[0].size();j++)
			{
				double baselinemean=0.0, DWImean=0.0, baselinedeviation=0.0, DWIdeviation=0.0; 
				for(int i=0; i<this->ResultsContainer.size(); i++)
				{
					if(this->qcResults[i])
					{
						if ( this->m_GradientDirectionContainer->at(i)[0] == 0.0 && 
							 this->m_GradientDirectionContainer->at(i)[1] == 0.0 && 
							 this->m_GradientDirectionContainer->at(i)[2] == 0.0     )
						{
							baselinemean+=this->ResultsContainer[i][j]/(double)(BaselineCount);
						}
						else
						{
							DWImean+=this->ResultsContainer[i][j]/(double)(DWICount);
						}
					}
				}

				for(int i=0; i<this->ResultsContainer.size(); i++)
				{
					if(this->qcResults[i])
					{
						if ( this->m_GradientDirectionContainer->at(i)[0] == 0.0 && 
							 this->m_GradientDirectionContainer->at(i)[1] == 0.0 && 
							 this->m_GradientDirectionContainer->at(i)[2] == 0.0     )
						{
							if(BaselineCount>=1)
								baselinedeviation+=(ResultsContainer[i][j]-baselinemean)*(ResultsContainer[i][j]-baselinemean)/(double)(BaselineCount);
							else
								baselinedeviation=0.0;
						}
						else
						{
							if(DWICount>=1)
								DWIdeviation+=(ResultsContainer[i][j]-DWImean)*(ResultsContainer[i][j]-DWImean)/(double)(DWICount);
							else
								DWIdeviation=0.0;
						}
					}
				}

				this->baselineMeans.push_back(baselinemean);
				this->baselineDeviations.push_back(sqrt(baselinedeviation));

				this->gradientMeans.push_back(DWImean);
				this->gradientDeviations.push_back(sqrt(DWIdeviation));
			}
		}


		int badcount=0;
		int baselineBadcount=0;

		InputImageConstPointer  inputPtr = this->GetInput();

// checking begins here
		if( getBValueLeftNumber() >=3 || (getBValueLeftNumber() ==2 && getBaselineLeftNumber()>0)) // ensure a quardratic fit
			// multiple b valued DWI, do a quadratic-curve fitting between b-value and image correlation at each slice position
		{
// 			std::cout<<" performing multiple b value checking"<<std::endl;
			for(int i=0;i<ResultsContainer.size();i++)
			{
				baselineBadcount = 0;
				badcount=0;
				if(this->qcResults[i]) // only check the left gradients
				{
					for(int j=0 + (int)(inputPtr->GetLargestPossibleRegion().GetSize()[2]*this->m_HeadSkipRatio);
						j<ResultsContainer[0].size() - (int)(inputPtr->GetLargestPossibleRegion().GetSize()[2]*this->m_TailSkipRatio); j++) 
					{
						////// to avoid report too many "good" as bad
						double MeanStdev=0.0;
						unsigned int effectiveSliceNumber=0;

						for(unsigned int k = (j-3>0? j-3:0); k < (j+3<inputPtr->GetLargestPossibleRegion().GetSize()[2]? j+3:inputPtr->GetLargestPossibleRegion().GetSize()[2]) ; k++ ) 
						{
							MeanStdev += this->quardraticFittedDeviations[k];		
							effectiveSliceNumber++;
						}
						MeanStdev = MeanStdev/(double)effectiveSliceNumber;	

						double stddev=0.0;
						if( this->quardraticFittedDeviations[j]< MeanStdev )
							stddev = MeanStdev;
						else
							stddev = this->quardraticFittedDeviations[j];
						///////////////////////////////////////////
//						if( normalizedMetric[i][j] < this->quardraticFittedMeans[j] - this->quardraticFittedDeviations[j] * this->m_GradientStdevTimes)
						if( normalizedMetric[i][j] < this->quardraticFittedMeans[j] - stddev * this->m_GradientStdevTimes)
						{
							this->ResultsContainer[i][j] = -this->ResultsContainer[i][j]; // to indicate a bad slice
							badcount++;
						}
					}
					if( badcount > 0 )
						this->qcResults[i] = 0;
				}
			}
		}
		else // single b value( baseline + bvalue or 2 different b values, [2 different b values not yet implemented])
		{
// 			std::cout<<" performing single b value checking"<<std::endl;

			for(int i=0;i<ResultsContainer.size();i++)
			{
				baselineBadcount = 0;
				badcount=0;
				if(this->qcResults[i]) // only check the left gradients
				{
					if ( this->m_GradientDirectionContainer->at(i)[0] == 0.0 &&
						this->m_GradientDirectionContainer->at(i)[1] == 0.0 && 
						this->m_GradientDirectionContainer->at(i)[2] == 0.0     )
					{	
						//baseline
						for(int j = 0 + (int)(inputPtr->GetLargestPossibleRegion().GetSize()[2] * this->m_HeadSkipRatio);
							j < ResultsContainer[0].size() - (int)( inputPtr->GetLargestPossibleRegion().GetSize()[2]*this->m_TailSkipRatio); j++ ) 
						{
							////// to avoid report too many "good" as bad
							double MeanStdev=0.0;
							unsigned int effectiveSliceNumber=0;

							for(unsigned int k = (j-3>0? j-3:0); k < (j+3<inputPtr->GetLargestPossibleRegion().GetSize()[2]? j+3:inputPtr->GetLargestPossibleRegion().GetSize()[2]) ; k++ ) 
							{
								MeanStdev += baselineDeviations[k];		
								effectiveSliceNumber++;
							}
							MeanStdev = MeanStdev/(double)effectiveSliceNumber;	

							double stddev=0.0;
							if( baselineDeviations[j]< MeanStdev )
								stddev = MeanStdev;
							else
								stddev = baselineDeviations[j];
							///////////////////////////////////////////
// 							if( this->ResultsContainer[i][j] < baselineMeans[j] - baselineDeviations[j] * m_BaselineStdevTimes)
							if( this->ResultsContainer[i][j] < baselineMeans[j] - stddev * m_BaselineStdevTimes)
							{
								this->ResultsContainer[i][j] = -this->ResultsContainer[i][j]; // to indicate a bad slice
								baselineBadcount++;
							}
						}
						if( baselineBadcount > 0 )
							this->qcResults[i] = 0;	
					}
					else // gradients
					{			
						for(int j=0 + (int)(inputPtr->GetLargestPossibleRegion().GetSize()[2]*this->m_HeadSkipRatio);
							j<ResultsContainer[0].size() - (int)(inputPtr->GetLargestPossibleRegion().GetSize()[2]*this->m_TailSkipRatio); j++) 
						{
							////// to avoid report too many "good" as bad
							double MeanStdev=0.0;
							unsigned int effectiveSliceNumber=0;

							for(unsigned int k = (j-3>0? j-3:0); k < (j+3<inputPtr->GetLargestPossibleRegion().GetSize()[2]? j+3:inputPtr->GetLargestPossibleRegion().GetSize()[2]) ; k++ ) 
							{
								MeanStdev += this->gradientDeviations[k];		
								effectiveSliceNumber++;
							}
							MeanStdev = MeanStdev/(double)effectiveSliceNumber;	

							double stddev=0.0;
							if( this->gradientDeviations[j]< MeanStdev )
								stddev = MeanStdev;
							else
								stddev = this->gradientDeviations[j];
							///////////////////////////////////////////
// 							if( ResultsContainer[i][j] < this->gradientMeans[j] - this->gradientDeviations[j] * this->m_GradientStdevTimes) // ok
							if( ResultsContainer[i][j] < this->gradientMeans[j] - stddev * this->m_GradientStdevTimes) // ok
							{
								this->ResultsContainer[i][j] = -this->ResultsContainer[i][j]; // to indicate a bad slice
								badcount++;
							}
						}
						if( badcount > 0 )
							this->qcResults[i] = 0;
					}
				}
			}
		}
		return;
	}

	/** 
	*
	*/
	template <class TImageType>
	void 
		DWIQCSliceChecker<TImageType>
		::iterativeCheck()
	{
		std::cout<<"slice checking ... ";
		if( this->m_CheckTimes>0)
		{
			for(int i=0;i<this->m_CheckTimes; i++)
			{
				std::cout<<" count: "<<i+1<<"...";
				check();
			}
		}
		else
		{
			collectLeftDiffusionStatistics();
			int DWICount, BaselineCount;
			int i=0;
			do 
			{
				std::cout<<"  count: "<<i+1<<"...";
				BaselineCount	= getBaselineLeftNumber();
				DWICount		= getGradientLeftNumber();
				// 				std::cout <<"BaselineCount: "<<BaselineCount<<std::endl;
				// 				std::cout <<"DWICount: "<<DWICount<<std::endl;

				check();
				collectLeftDiffusionStatistics();
				i++;
			} while ( BaselineCount!=getBaselineLeftNumber() || DWICount!=getGradientLeftNumber());
		}

		std::cout<<" DONE "<<std::endl;
		return;
	}

	/** 
	*
	*/
	template <class TImageType>
	void 
		DWIQCSliceChecker<TImageType>
		::writeReport()
	{
		if( this->GetReportFileName().length()==0 )
			return;

		std::ofstream outfile;
		if( GetReportFileMode()== DWIQCSliceChecker::Report_Append )
			outfile.open( GetReportFileName().c_str(), std::ios_base::app);
		else
			outfile.open( GetReportFileName().c_str());

		outfile <<std::endl;
		outfile <<"================================"<<std::endl;
		outfile <<"       Slice-wise checking      "<<std::endl;
		outfile <<"================================"<<std::endl;

		outfile <<"Parameters:"<<std::endl;

		outfile<<"  CheckTimes: "			<< m_CheckTimes			<<std::endl;
		outfile<<"  HeadSkipRatio: "		<< m_HeadSkipRatio		<<std::endl;
		outfile<<"  TailSkipRatio: "		<< m_TailSkipRatio		<<std::endl;
		outfile<<"  BaselineStdevTimes: "	<< m_BaselineStdevTimes <<std::endl;
		outfile<<"  GradientStdevTimes: "	<< m_GradientStdevTimes <<std::endl;

		outfile <<std::endl<<"======"<<std::endl;
		outfile <<"Slice-wise correlations: "<<std::endl<<std::endl;
		for(int i=0; i<this->ResultsContainer.size(); i++)
		{
			outfile <<"\t"<<"Gradient"<<i;
		}
		outfile <<std::endl;
// 		outfile <<"\tbaselineMean"<<"\tbaselineDeviation"<<"\tmean"<<"\tdeviation"<<std::endl;

		collectLeftDiffusionStatistics();  //update
		int DWICount, BaselineCount;
		BaselineCount	= getBaselineNumber();
		DWICount		= getGradientNumber();

		for(int j=0;j<this->ResultsContainer[0].size();j++)
		{
			double baselinemean=0.0, DWImean=0.0, baselinedeviation=0.0, DWIdeviation=0.0;
			for(int i=0; i<ResultsContainer.size(); i++)
			{

				outfile <<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
					    <<fabs(ResultsContainer[i][j]);
			}
			outfile <<std::endl;
		}

		outfile <<std::endl<<"======"<<std::endl;
		outfile <<"Slice-wise Check Artifacts:"<<std::endl;
		outfile <<"\t"<<std::setw(10)<<"Gradient#"<<"\t"<<std::setw(10)<<"Slice#"<<"\t"<<std::setw(10)<<"Correlation"<<std::endl;

		for(int i=0;i<this->ResultsContainer.size();i++)
		{
			for(int j=0; j<ResultsContainer[0].size(); j++ ) 
			{
				outfile.precision(6);
				outfile.setf(std::ios_base::showpoint|std::ios_base::right) ;

				if( this->ResultsContainer[i][j] <=0 )
					outfile	<<"\t"<<std::setw(9)<<i
							<<"\t"<<std::setw(9)<<j+1
							<<"\t"<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
							<<-ResultsContainer[i][j]<<std::endl;
					//outfile  <<"\t"<<std::setw(10)<<i<<"\t"<<std::setw(10)<<j+1<<"\t"<<-ResultsContainer[i][j]<<std::endl;
			}
		}

		outfile <<std::endl<<"======"<<std::endl;
		outfile<<"Left Diffusion Gradient information:"<<std::endl;
		outfile<<"\tbValueLeftNumber: "		<<bValueLeftNumber		<<std::endl;
		outfile<<"\tbaselineLeftNumber: "	<<baselineLeftNumber	<<std::endl;
		outfile<<"\tgradientDirLeftNumber: "<<gradientDirLeftNumber	<<std::endl;

		outfile<<std::endl<<"\t# "<<"\tDirVector"<<std::setw(34)<<std::setiosflags(std::ios::left)<<"\tIncluded"<<std::endl;
		for(int i=0;i< this->m_GradientDirectionContainer->size();i++)
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

		for(int i=0;i< this->DiffusionDirHistOutput.size();i++)
		{
			if(GetReportFileName().length()>0)
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
		DWIQCSliceChecker<TImageType>
		::collectLeftDiffusionStatistics()
	{
		this->DiffusionDirHistOutput.clear();
		this->repetitionLeftNumber.clear();

		for( int i=0; i< this->m_GradientDirectionContainer->size();i++) 
		{
			if(DiffusionDirHistOutput.size()>0)
			{
				bool newDir = true;
				for(int j=0;j<DiffusionDirHistOutput.size();j++)
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
		double modeTemp = 0.0;
		for( int i=0; i<DiffusionDirHistOutput.size(); i++)
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
					for(int j=0;j< dirNorm.size();j++)
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
		for(int i=0; i<this->repetitionLeftNumber.size(); i++)
		{
			this->gradientLeftNumber+=this->repetitionLeftNumber[i];
			if(this->repetitionLeftNumber[i]>0)
				this->gradientDirLeftNumber++;
		}

		this->bValueLeftNumber = dirNorm.size();

		// 		std::ofstream outfile; 
		// 		if(reportFilename.length()>0)
		// 		{
		// 			outfile.open(reportFilename.c_str(), std::ios::app);
		// 			outfile<<"====="<<std::endl;
		// 			outfile<<"Left Diffusion Gradient information:"<<std::endl;
		// 			outfile<<"\tbValueLeftNumber: "		<<bValueLeftNumber		<<std::endl;
		// 			outfile<<"\tbaselineLeftNumber: "	<<baselineLeftNumber	<<std::endl;
		// 			outfile<<"\tgradientDirLeftNumber: "<<gradientDirLeftNumber	<<std::endl;
		// 		}
		// 
		// 		std::cout<<"Left Diffusion Gradient information:"<<std::endl;
		// 		std::cout<<"\tbValueLeftNumber: "	   <<bValueLeftNumber		<<std::endl;
		// 		std::cout<<"\tbaselineLeftNumber: "	   <<baselineLeftNumber	<<std::endl;
		// 		std::cout<<"\tgradientDirLeftNumber: " <<gradientDirLeftNumber	<<std::endl;
		// 
		// 		std::cout<<std::endl<<"\t# "<<"\t"<<std::setw(34)<<"DirVector"<<"\tIncluded"<<std::endl;
		// 		outfile  <<std::endl<<"\t# "<<"\t"<<std::setw(34)<<"DirVector"<<"\tIncluded"<<std::endl;
		// 		for(int i=0;i< this->m_GradientDirectionContainer->size();i++)
		// 		{
		// 			outfile<<"\t"<<i<<"\t[ " 
		// 				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
		// 				<<this->m_GradientDirectionContainer->at(i)[0]<<", "
		// 				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
		// 				<<this->m_GradientDirectionContainer->at(i)[1]<<", "
		// 				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
		// 				<<this->m_GradientDirectionContainer->at(i)[2]<<" ]: "
		// 				<<"\t"<<this->qcResults[i]<<std::endl;
		// 			std::cout<<"\t"<<i<<"\t[ " 
		// 				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
		// 				<<this->m_GradientDirectionContainer->at(i)[0]<<", "
		// 				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
		// 				<<this->m_GradientDirectionContainer->at(i)[1]<<", "
		// 				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
		// 				<<this->m_GradientDirectionContainer->at(i)[2]<<" ]: "
		// 				<<"\t"<<this->qcResults[i]<<std::endl;
		// 		}
		// 
		// 		std::cout<<std::endl<<"Left Gradient Direction Histogram: "<<std::endl;
		// 		std::cout<<"\t# "<<"\t"<<std::setw(34)<<"DirVector"<<"\tRepLeft#"<<std::endl;
		// 		outfile  <<std::endl<<"Left Gradient Direction Histogram: "<<std::endl;
		// 		outfile  <<"\t# "<<"\t"<<std::setw(34)<<"DirVector"<<"\tRepLeft#"<<std::endl;
		// 		for(int i=0;i< DiffusionDirHistOutput.size();i++)
		// 		{
		// 			std::cout<<"\t"<<i<<"\t[ " 
		// 				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
		// 				<<DiffusionDirHistOutput[i].gradientDir[0]<<", "
		// 				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
		// 				<<DiffusionDirHistOutput[i].gradientDir[1]<<", "
		// 				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
		// 				<<DiffusionDirHistOutput[i].gradientDir[2]<<"] "
		// 				<<"\t"<<DiffusionDirHistOutput[i].repetitionNumber <<std::endl;
		// 
		// 			if(reportFilename.length()>0)
		// 				outfile<<"\t"<<i<<"\t[ " 
		// 				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
		// 				<<DiffusionDirHistOutput[i].gradientDir[0]<<", "
		// 				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
		// 				<<DiffusionDirHistOutput[i].gradientDir[1]<<", "
		// 				<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
		// 				<<DiffusionDirHistOutput[i].gradientDir[2]<<"] "
		// 				<<"\t"<<DiffusionDirHistOutput[i].repetitionNumber <<std::endl;
		// 		}
		// 
		// 		outfile.close();

		return;
	}

	/** 
	*
	*/
	template <class TImageType>
	void 
		DWIQCSliceChecker<TImageType>
		::collectDiffusionStatistics()
	{
		std::vector<struDiffusionDir> DiffusionDirections;
		DiffusionDirections.clear();

		for( int i=0; i< this->m_GradientDirectionContainer->size();i++) 
		{
			if(DiffusionDirections.size()>0)
			{
				bool newDir = true;
				for(int j=0;j<DiffusionDirections.size();j++)
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

		double modeTemp = 0.0;
		for( int i=0; i<DiffusionDirections.size(); i++)
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
					for(int j=0;j< dirNorm.size();j++)
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
		for( int i=1; i<repetNum.size(); i++)
		{ 
			if( repetNum[i] != repetNum[0])
			{
				std::cout<<"Protocol error. Not all the gradient directions have same repetition. "<<std::endl;
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
	template <class TImageType>
	void 
		DWIQCSliceChecker<TImageType>
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

		for(int i=0;i< this->m_GradientDirectionContainer -> Size();i++ )
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
		DWIQCSliceChecker<TImageType>
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
			for( int i = 0 ; i < this->qcResults.size(); i++ )
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
		DWIQCSliceChecker<TImageType>
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



	/** 
	*
	*/

}

#endif

