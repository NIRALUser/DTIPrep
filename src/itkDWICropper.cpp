/*=========================================================================

Program:   NeuroLib
Module:    $file: itkDWICropper.cpp $
Language:  C++
Date:      $Date: 2009-11-26 21:52:35 $
Version:   $Revision: 1.8 $
Author:    Zhexing Liu (liuzhexing@gmail.com)

Copyright (c) NIRAL, UNC. All rights reserved.
See http://www.niral.unc.edu for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkDWICropper_cpp
#define _itkDWICropper_cpp

#include "itkDWICropper.h"

#include "itkImageRegionIteratorWithIndex.h"
#include "itkExceptionObject.h"
#include "itkProgressReporter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "vnl/vnl_matrix.h"
#include "itkResampleImageFilter.h"

#include <iostream>
#include <iomanip>
#include <fstream>

namespace itk
{
	/**
	*
	*/
	template <class TImageType>
	DWICropper<TImageType>
		::DWICropper()
	{
		region = NULL; 
		size = NULL;
	}

	/**
	*
	*/
	template <class TImageType>
	DWICropper<TImageType>
		::~DWICropper()
	{
		if ( size )
		{
			delete region;
		}
	}

	/**
	* PrintSelf
	*/
	template <class TImageType>
	void
		DWICropper<TImageType>
		::PrintSelf(std::ostream & os, Indent indent) const
	{
		Superclass::PrintSelf(os, indent);
		os << indent << "DWI image cropping/padding filter: " << std::endl;
	}

	/**
	*
	*/
	template <class TImageType>
	void
		DWICropper<TImageType>
		::GenerateOutputInformation()
	{
		// call the superclass' implementation of this method
		Superclass::GenerateOutputInformation();

		// get pointers to the input and output
		InputImageConstPointer inputPtr  = this->GetInput();
		OutputImagePointer     outputPtr = this->GetOutput();

		if ( !inputPtr || !outputPtr )
		{
			return;
		}

		parseGradientDirections();
		collectDiffusionStatistics();

		typename InputImageType::RegionType imageOriginalRegion
			= inputPtr->GetLargestPossibleRegion();
		typename InputImageType::SizeType imageOriginalSize
			= imageOriginalRegion.GetSize();

		typename OutputImageType::RegionType outputRegion;
		typename OutputImageType::IndexType outputIndex;
		typename OutputImageType::SizeType outputSize;

		if ( size )
		{
			region = new int[6];

			region[0] = ( (int)imageOriginalSize[0] - size[0] ) / 2;
			region[1] = ( (int)imageOriginalSize[1] - size[1] ) / 2;
			region[2] = ( (int)imageOriginalSize[2] - size[2] ) / 2;

			region[3] = size[0];
			region[4] = size[1];
			region[5] = size[2];

			//       for(unsigned int i=0;i<6;i++)
			//         std::cout<<"region["<<i<<"]: "<<region[i]<<std::endl;
		}
		outputIndex[0] = 0;
		outputIndex[1] = 0;
		outputIndex[2] = 0;

		outputSize[0] = region[3];
		outputSize[1] = region[4];
		outputSize[2] = region[5];

		outputRegion.SetIndex(outputIndex);
		outputRegion.SetSize( outputSize);

		outputPtr->SetLargestPossibleRegion(outputRegion);
		outputPtr->SetVectorLength( inputPtr->GetVectorLength() );

		itk::MetaDataDictionary outputMetaDictionary;

		itk::MetaDataDictionary imgMetaDictionary
			= inputPtr->GetMetaDataDictionary();
		std::vector<std::string> imgMetaKeys
			= imgMetaDictionary.GetKeys();
		std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
		std::string  metaString;

		//  measurement frame
		if ( imgMetaDictionary.HasKey("NRRD_measurement frame") )
		{
#if 0
			// measurement frame
			vnl_matrix_fixed<double, 3, 3> mf;
			// imaging frame
			vnl_matrix_fixed<double, 3, 3> imgf;
			imgf = inputPtr->GetDirection().GetVnlMatrix();
#endif
			// Meausurement frame
			std::vector<std::vector<double> > nrrdmf;
			itk::ExposeMetaData<std::vector<std::vector<double> > >(
				imgMetaDictionary,
				"NRRD_measurement frame",
				nrrdmf);
			itk::EncapsulateMetaData<std::vector<std::vector<double> > >(
				outputMetaDictionary,
				"NRRD_measurement frame",
				nrrdmf);
		}

		// modality
		if ( imgMetaDictionary.HasKey("modality") )
		{
			itk::ExposeMetaData(imgMetaDictionary, "modality", metaString);
			itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
				"modality",
				metaString);
		}

		// thickness
		if ( imgMetaDictionary.HasKey("NRRD_thicknesses[2]") )
		{
			double thickness = -12345;
			itk::ExposeMetaData<double>(imgMetaDictionary,
				"NRRD_thickness[2]",
				thickness);
			itk::EncapsulateMetaData<double>( outputMetaDictionary,
				"NRRD_thickness[2]",
				thickness);
			itk::EncapsulateMetaData<double>( outputMetaDictionary,
				"NRRD_thickness[0]",
				-1);
			itk::EncapsulateMetaData<double>( outputMetaDictionary,
				"NRRD_thickness[1]",
				-1);
			//       itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
			// "NRRD_thickness[2]", "NaN");
		}

		// centerings
		if ( imgMetaDictionary.HasKey("NRRD_centerings[0]") )
		{
			itk::ExposeMetaData(imgMetaDictionary, "NRRD_centerings[0]", metaString);
			itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
				"NRRD_centerings[0]",
				metaString);
		}
		if ( imgMetaDictionary.HasKey("NRRD_centerings[1]") )
		{
			itk::ExposeMetaData(imgMetaDictionary, "NRRD_centerings[1]", metaString);
			itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
				"NRRD_centerings[1]",
				metaString);
		}
		if ( imgMetaDictionary.HasKey("NRRD_centerings[2]") )
		{
			itk::ExposeMetaData(imgMetaDictionary, "NRRD_centerings[2]", metaString);
			itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
				"NRRD_centerings[2]",
				metaString);
		}

		// b-value
		if ( imgMetaDictionary.HasKey("DWMRI_b-value") )
		{
			itk::ExposeMetaData(imgMetaDictionary, "DWMRI_b-value", metaString);
			itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
				"DWMRI_b-value",
				metaString);
		}

		// gradient vectors
		int temp = 0;
		for ( unsigned int i = 0; i < this->m_GradientDirectionContainer->Size(); i++ )
		{
			std::ostringstream ossKey;
			ossKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << temp;

			std::ostringstream ossMetaString;
			ossMetaString << std::setw(9) << std::setiosflags(std::ios::fixed)
				<< std::setprecision(6) << std::setiosflags(std::ios::right)
				<< this->m_GradientDirectionContainer->ElementAt(i)[0]
			<< "    "
				<< std::setw(9) << std::setiosflags(std::ios::fixed)
				<< std::setprecision(6) << std::setiosflags(std::ios::right)
				<< this->m_GradientDirectionContainer->ElementAt(i)[1]
			<< "    "
				<< std::setw(9) << std::setiosflags(std::ios::fixed)
				<< std::setprecision(6) << std::setiosflags(std::ios::right)
				<< this->m_GradientDirectionContainer->ElementAt(i)[2];

			// std::cout<<ossKey.str()<<ossMetaString.str()<<std::endl;
			itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
				ossKey.str(), ossMetaString.str() );
			++temp;
		}
		outputPtr->SetMetaDataDictionary(outputMetaDictionary);

		writeReport();
	}

	/**
	*
	*/
	template <class TImageType>
	void
		DWICropper<TImageType>
		::writeReport()
	{
		if ( GetReportFileName().length() == 0 )
		{
			return;
		}

		std::ofstream outfile;

		if ( GetReportFileMode() == DWICropper::REPORT_FILE_APPEND )
		{
			outfile.open( GetReportFileName().c_str(), std::ios_base::app);
		}
		else
		{
			outfile.open( GetReportFileName().c_str() );
		}

		outfile << std::endl;
		outfile << "================================" << std::endl;
		outfile << "     DWI Cropping/Padding       " << std::endl;
		outfile << "================================" << std::endl;

		typename TImageType::ConstPointer inputPtr  = this->GetInput();
		typename TImageType::Pointer outputPtr = this->GetOutput();

		if ( !inputPtr || !outputPtr )
		{
			return;
		}

		typename InputImageType::RegionType imageOriginalRegion
			= inputPtr->GetLargestPossibleRegion();
		typename InputImageType::SizeType imageOriginalSize
			= imageOriginalRegion.GetSize();

		outfile << "input  size: [ " << imageOriginalSize[0] << ", "
			<< imageOriginalSize[1] << ", " << imageOriginalSize[2] << " ] "
			<< std::endl;
		outfile << "output size: [ " << region[3] << ", " << region[4] << ", "
			<< region[5] << " ] " << std::endl;
		outfile << "start index: [ " << region[0] << ", " << region[1] << ", "
			<< region[2] << " ] " << std::endl;

		outfile.close();
		return;
	}

	/**
	*
	*/
	template <class TImageType>
	void
		DWICropper<TImageType>
		::collectDiffusionStatistics()
	{
		std::vector<struDiffusionDir> DiffusionDirections;
		DiffusionDirections.clear();

		for ( unsigned int i = 0; i < this->m_GradientDirectionContainer->size(); i++ )
		{
			if ( DiffusionDirections.size() > 0 )
			{
				bool newDir = true;
				for ( unsigned int j = 0; j < DiffusionDirections.size(); j++ )
				{
					if ( this->m_GradientDirectionContainer->ElementAt(i)[0] ==
						DiffusionDirections[j].gradientDir[0]
					&& this->m_GradientDirectionContainer->ElementAt(i)[1] ==
						DiffusionDirections[j].gradientDir[1]
					&& this->m_GradientDirectionContainer->ElementAt(i)[2] ==
						DiffusionDirections[j].gradientDir[2] )
					{
						DiffusionDirections[j].repetitionNumber++;
						newDir = false;
					}
				}
				if ( newDir )
				{
					std::vector<double> dir;
					dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[0]);
					dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[1]);
					dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[2]);

					struDiffusionDir diffusionDir;
					diffusionDir.gradientDir = dir;
					diffusionDir.repetitionNumber = 1;

					DiffusionDirections.push_back(diffusionDir);
				}
			}
			else
			{
				std::vector<double> dir;
				dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[0]);
				dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[1]);
				dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[2]);

				struDiffusionDir diffusionDir;
				diffusionDir.gradientDir = dir;
				diffusionDir.repetitionNumber = 1;

				DiffusionDirections.push_back(diffusionDir);
			}
		}

		// std::cout<<"DiffusionDirections.size(): " <<
		// m_GradientDirectionContainer->size() <<std::endl;

		std::vector<int> repetNum;
		repetNum.clear();
		std::vector<double> dirNorm;
		dirNorm.clear();

		// double modeTemp = 0.0;
		for ( unsigned int i = 0; i < DiffusionDirections.size(); i++ )
		{
			if ( DiffusionDirections[i].gradientDir[0] == 0.0
				&& DiffusionDirections[i].gradientDir[1] == 0.0
				&& DiffusionDirections[i].gradientDir[2] == 0.0 )
			{
				this->baselineNumber = DiffusionDirections[i].repetitionNumber;
				// std::cout<<"DiffusionDirections[i].repetitionNumber: " <<i<<"
				//  "<<DiffusionDirections[i].repetitionNumber <<std::endl;
			}
			else
			{
				repetNum.push_back(DiffusionDirections[i].repetitionNumber);

				double normSqr =  DiffusionDirections[i].gradientDir[0]
				* DiffusionDirections[i].gradientDir[0]
				+ DiffusionDirections[i].gradientDir[1]
				* DiffusionDirections[i].gradientDir[1]
				+ DiffusionDirections[i].gradientDir[2]
				* DiffusionDirections[i].gradientDir[2];

				// std::cout<<"normSqr: " <<normSqr <<std::endl;
				if ( dirNorm.size() > 0 )
				{
					bool newDirMode = true;
					for ( unsigned int j = 0; j < dirNorm.size(); j++ )
					{
						if ( fabs(normSqr - dirNorm[j]) < 0.001 ) // 1 DIFFERENCE for b value
						{
							newDirMode = false;
							break;
						}
					}
					if ( newDirMode )
					{
						dirNorm.push_back(  normSqr);
					}
				}
				else
				{
					dirNorm.push_back(  normSqr);
				}
			}
		}

		this->gradientDirNumber = repetNum.size();
		this->bValueNumber = dirNorm.size();

		repetitionNumber = repetNum[0];
		for ( unsigned int i = 1; i < repetNum.size(); i++ )
		{
			if ( repetNum[i] != repetNum[0] )
			{
				//std::cout
				//	<< "Warning: Not all the gradient directions have same repetition. "
            			//<< "GradientNumber= " << i << " " << repetNum[i] << " != " << repetNum[0]
				//	<< std::endl;
				repetitionNumber = -1;
			}
		}

		this->gradientNumber = this->m_GradientDirectionContainer->size()
			- this->baselineNumber;

		//     std::cout<<"DWI Diffusion Information: "    <<std::endl;
		//     std::cout<<"  baselineNumber: "    <<baselineNumber  <<std::endl;
		//     std::cout<<"  bValueNumber: "    <<bValueNumber    <<std::endl;
		//     std::cout<<"  gradientDirNumber: "  <<gradientDirNumber  <<std::endl;
		//     std::cout<<"  gradientNumber: "    <<gradientNumber  <<std::endl;
		//     std::cout<<"  repetitionNumber: "  <<repetitionNumber  <<std::endl;
		return;
	}

	/**
	*
	*/
	template <class TImageType>
	void
		DWICropper<TImageType>
		::parseGradientDirections()
	{
		InputImageConstPointer inputPtr = this->GetInput();

		itk::MetaDataDictionary imgMetaDictionary = inputPtr->GetMetaDataDictionary();      //

		std::vector<std::string> imgMetaKeys
			= imgMetaDictionary.GetKeys();
		std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
		std::string                              metaString;

		m_GradientDirectionContainer = GradientDirectionContainerType::New();
		GradientDirectionType vect3d;
		for (; itKey != imgMetaKeys.end(); itKey++ )
		{
			itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
			// std::cout<<*itKey<<"  "<<metaString<<std::endl;

			if ( itKey->find("DWMRI_gradient") != std::string::npos )
			{
				std::istringstream iss(metaString);
				iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
				m_GradientDirectionContainer->push_back(vect3d);
			}
			else if ( itKey->find("DWMRI_b-value") != std::string::npos )
			{
				b0 = atof( metaString.c_str() );
			}
		}

		if ( b0 < 0.0 )
		{
			std::cout << "BValue not specified in header file" << std::endl;
			return;
		}

		for ( unsigned int i = 0; i < this->m_GradientDirectionContainer->Size(); i++ )
		{
			bValues.push_back( static_cast<int>( (  this->m_GradientDirectionContainer
				->ElementAt(i)[0]
			* this->
				m_GradientDirectionContainer->
				ElementAt(i)[0]
			+ this->
				m_GradientDirectionContainer->
				ElementAt(i)[1]
			* this->
				m_GradientDirectionContainer->
				ElementAt(i)[1]
			+ this->
				m_GradientDirectionContainer->
				ElementAt(i)[2]
			* this->
				m_GradientDirectionContainer->
				ElementAt(i)[2] ) * b0 + 0.5 ) );
		}

		//     std::cout << "b values:" << std::endl;
		//     for(unsigned int i=0;i< this->m_GradientDirectionContainer ->
		// Size();i++ )
		//     {
		//       std::cout << bValues[i] << std::endl;
		//     }

		if ( m_GradientDirectionContainer->size() <= 6 )
		{
			std::cout << "Gradient Images Less than 6" << std::endl;
			return;
		}
	}

	/**
	*
	*/
	template <class TImageType>
	void
		DWICropper<TImageType>
		::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
		int threadId )
	{
		itkDebugMacro(<< "Actually executing");

		// Get the input and output pointers
		InputImageConstPointer inputPtr = this->GetInput();
		OutputImagePointer     outputPtr = this->GetOutput();

		typedef ImageRegionConstIterator<TImageType> constDWIIterator;
		typedef ImageRegionIterator<TImageType>      DWIIterator;

		DWIIterator iOutput( outputPtr, outputRegionForThread);
		iOutput.GoToBegin();

		// support progress methods/callbacks
		ProgressReporter progress( this, threadId,
			outputRegionForThread.GetNumberOfPixels() );

		typename TImageType::IndexType index;
		typename TImageType::PixelType voxelvalue;
		voxelvalue.SetSize( inputPtr->GetVectorLength() );
		for ( unsigned int i = 0; i < inputPtr->GetVectorLength(); i++ )
		{
			voxelvalue.SetElement( i, 0 );
		}

		typename TImageType::RegionType imageOriginalRegion
			= inputPtr->GetLargestPossibleRegion();
		while ( !iOutput.IsAtEnd() )
		{
			index = iOutput.GetIndex();
			index[0] += region[0];
			index[1] += region[1];
			index[2] += region[2];
			if ( imageOriginalRegion.IsInside(index) )
			{
				iOutput.Set( inputPtr->GetPixel(index) );
			}
			else
			{
				iOutput.Set( voxelvalue );
			}
			++iOutput;

			progress.CompletedPixel();
		}
	}

	template <class TImageType>
	void
		DWICropper<TImageType>
		::GenerateInputRequestedRegion()
	{
		//     long sizeTemp;
		//
		//     // call the superclass' implementation of this method
		//     // Superclass::GenerateInputRequestedRegion();
		//
		//     // get pointers to the input and output
		//     typename Superclass::InputImagePointer  inputPtr =
		//       const_cast< TImageType * >( this->GetInput() );
		//     typename Superclass::OutputImagePointer outputPtr = this->GetOutput();
		//
		//     if ( !inputPtr || !outputPtr )
		//     {
		//       return;
		//     }
		//
		//     // we need to compute the input requested region (size and start index)
		//     unsigned int i;
		//     const typename TImageType::SizeType& outputRequestedRegionSize
		//       = outputPtr->GetRequestedRegion().GetSize();
		//     const typename TImageType::IndexType& outputRequestedRegionStartIndex
		//       = outputPtr->GetRequestedRegion().GetIndex();
		//     const typename TImageType::SizeType& inputWholeRegionSize
		//       = inputPtr->GetLargestPossibleRegion().GetSize();
		//     const typename TImageType::IndexType& inputWholeRegionStartIndex
		//       = inputPtr->GetLargestPossibleRegion().GetIndex();
		//
		//     typename TImageType::SizeType  inputRequestedRegionSize;
		//     typename TImageType::IndexType inputRequestedRegionStartIndex;
		//
		//
		//
		//     for (i = 0; i < 3; i++) // TImageType::ImageDimension
		//     {
		//       if (outputRequestedRegionStartIndex[i] <=
		// inputWholeRegionStartIndex[i])
		//       {
		//         inputRequestedRegionStartIndex[i] = inputWholeRegionStartIndex[i];
		//       }
		//       else
		//       {
		//         inputRequestedRegionStartIndex[i] =
		// outputRequestedRegionStartIndex[i];
		//       }
		//
		//       if
		// ((inputWholeRegionStartIndex[i]+static_cast<long>(inputWholeRegionSize[i]))
		// <=
		//
		//
		//
		//
		//
		//
		//
		//
		//
		//
		// (outputRequestedRegionStartIndex[i]+static_cast<long>(outputRequestedRegionSize[i])))
		//       {
		//         sizeTemp = static_cast<long>(inputWholeRegionSize[i])
		//           + inputWholeRegionStartIndex[i] -
		// inputRequestedRegionStartIndex[i];
		//       }
		//       else
		//       {
		//         sizeTemp = static_cast<long>(outputRequestedRegionSize[i])
		//           + outputRequestedRegionStartIndex[i] -
		// inputRequestedRegionStartIndex[i];
		//       }
		//
		//       //
		//       // The previous statements correctly handle overlapped regions where
		//       // at least some of the pixels from the input image end up reflected
		//       // in the output.  When there is no overlap, the size will be
		// negative.
		//       // In that case we arbitrarily pick the start of the input region
		//       // as the start of the output region and zero for the size.
		//       //
		//       if (sizeTemp < 0)
		//       {
		//         inputRequestedRegionSize[i] = 0;
		//         inputRequestedRegionStartIndex[i] = inputWholeRegionStartIndex[i];
		//       } else {
		//         inputRequestedRegionSize[i] = sizeTemp;
		//       }
		//
		//     }
		//
		//     typename TImageType::RegionType inputRequestedRegion;
		//     inputRequestedRegion.SetSize( inputRequestedRegionSize );
		//     inputRequestedRegion.SetIndex( inputRequestedRegionStartIndex );
		//
		//     inputPtr->SetRequestedRegion( inputRequestedRegion );
	}
}
#endif
