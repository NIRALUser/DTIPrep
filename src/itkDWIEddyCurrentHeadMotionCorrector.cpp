/*=========================================================================

Program:   NeuroLib
Module:    $file: itkDWIEddyCurrentHeadMotionCorrector.cpp $
Language:  C++
Date:      $Date: 2009-11-26 21:52:35 $
Version:   $Revision: 1.9 $
Author:    Zhexing Liu (liuzhexing@gmail.com)

Copyright (c) NIRAL, UNC. All rights reserved.
See http://www.niral.unc.edu for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _itkDWIEddyCurrentHeadMotionCorrector_cpp
#define _itkDWIEddyCurrentHeadMotionCorrector_cpp

#include "itkDWIEddyCurrentHeadMotionCorrector.h"
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
	template <class TImageType>
	DWIEddyCurrentHeadMotionCorrector<TImageType>
		::DWIEddyCurrentHeadMotionCorrector()
	{
		m_NumberOfBins    = 24;
		m_Samples      = 100000;
		m_TranslationScale  = 0.001;
		m_StepLength    = 0.1;
		m_Factor      = 0.5;
		m_MaxNumberOfIterations = 500;

		b0 = -1.0;

		m_ReportFileName = "";
		m_ReportFileMode = DWIEddyCurrentHeadMotionCorrector::REPORT_FILE_NEW;

		bValues.clear();

		CorrectDoneOff();
	}

	/**
	*
	*/
	template <class TImageType>
	DWIEddyCurrentHeadMotionCorrector<TImageType>
		::~DWIEddyCurrentHeadMotionCorrector()
	{}

	/**
	* PrintSelf
	*/
	template <class TImageType>
	void
		DWIEddyCurrentHeadMotionCorrector<TImageType>
		::PrintSelf(std::ostream & os, Indent indent) const
	{
		Superclass::PrintSelf(os, indent);
		os << indent << "DWI Eddy-Current Head Motion Corrector" << std::endl;
	}

	/**
	*
	*/
	template <class TImageType>
	void
		DWIEddyCurrentHeadMotionCorrector<TImageType>
		::GenerateOutputInformation()
	{
		// call the superclass' implementation of this method
		Superclass::GenerateOutputInformation();

		// get pointers to the input and output
		InputImageConstPointer inputPtr = this->GetInput();
		OutputImagePointer     outputPtr = this->GetOutput();

		if ( !inputPtr || !outputPtr )
		{
			return;
		}

		// perform correction
		parseGradientDirections();
		collectDiffusionStatistics();
		correct();
		if ( ( this->corr->GetVectorLength() - 1 ) != this->re.GetGradients().size() )
		{
			std::cout << "ERROR: Gradient # and Diffusion Direction # mismatch!"
				<< std::endl;
			return;
		}
		collectLeftDiffusionStatistics();
		writeReport();
		CorrectDoneOn();

		outputPtr->SetVectorLength( this->corr->GetVectorLength() );

		itk::MetaDataDictionary outputMetaDictionary;

		itk::MetaDataDictionary imgMetaDictionary
			= inputPtr->GetMetaDataDictionary();
		std::vector<std::string> imgMetaKeys
			= imgMetaDictionary.GetKeys();
		std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
		std::string                              metaString;

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
		//  std::ostringstream ossKey;
		//  std::ostringstream ossMetaString;

		itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
			"DWMRI_gradient_0000",
			" 0.000000     0.000000     0.000000");

		for ( unsigned int i = 0; i < this->re.GetGradients().size(); i++ )
		{
			std::ostringstream ossKeyLocal;
			std::ostringstream ossMetaString;

			ossKeyLocal << "DWMRI_gradient_" << std::setw(4) << std::setfill('0')
				<< i + 1;
			ossMetaString << std::setw(9) << std::setiosflags(std::ios::fixed)
				<< std::setprecision(6) << std::setiosflags(std::ios::right)
				<< this->re.GetGradients().at(i)[0]  << "    "
				<< std::setw(9) << std::setiosflags(std::ios::fixed)
				<< std::setprecision(6) << std::setiosflags(std::ios::right)
				<< this->re.GetGradients().at(i)[1]  << "    "
				<< std::setw(9) << std::setiosflags(std::ios::fixed)
				<< std::setprecision(6) << std::setiosflags(std::ios::right)
				<< this->re.GetGradients().at(i)[2];

			// std::cout<<ossKeyLocal.str()<<ossMetaString.str()<<std::endl;
			itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
				ossKeyLocal.str(), ossMetaString.str() );
		}
		outputPtr->SetMetaDataDictionary( outputMetaDictionary );
	}

	/**
	*
	*/
	template <class TImageType>
	void
		DWIEddyCurrentHeadMotionCorrector<TImageType>
		::correct()
	{
		collectLeftDiffusionStatistics();

		int DWICount, BaselineCount;
		BaselineCount  = getBaselineLeftNumber();
		DWICount    = getGradientLeftNumber();
		//  std::cout <<"BaselineCount: "<<BaselineCount<<std::endl;
		//   std::cout <<"DWICount: "<<DWICount<<std::endl;

		// //////////////////////////////////////////////////////////////////////
		//   parse diffusion vectors
		/**/
		InputImageConstPointer inputPtr = this->GetInput();
		if ( !inputPtr )
		{
			return;
		}

		itk::MetaDataDictionary imgMetaDictionary
			= inputPtr->GetMetaDataDictionary();
		std::vector<std::string>           imgMetaKeys = imgMetaDictionary.GetKeys();
		std::vector<std::string>::iterator itKey = imgMetaKeys.begin();
		std::string                        metaString;

		vnl_vector_fixed<double, 3>               vect3d;
		std::vector<vnl_vector_fixed<double, 3> > DiffusionVectors;

		m_FeedinGradientDirectionContainer = GradientDirectionContainerType::New();
		m_OutputGradientDirectionContainer = GradientDirectionContainerType::New();

		// bool Baseline = true;
		int nBaseline = 0;

		std::vector<unsigned int> baselineIndicator( 0 );
		std::vector<unsigned int> diffusionIndicator( 0 );

		{
			int k = 0;
			for (; itKey != imgMetaKeys.end(); itKey++ )
			{
				ExposeMetaData(imgMetaDictionary, *itKey, metaString);
				const int pos = itKey->find("DWMRI_gradient");
				if ( pos == -1 )
				{
					continue;
				}
				std::cout  << metaString << std::endl;

				double x, y, z;
				std::sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
				vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
				if ( x == 0 && y == 0 && z == 0 )  // indicating baseline image
				{
					nBaseline++;
					baselineIndicator.push_back( k );
					k++;
					continue;
				}

				DiffusionVectors.push_back(vect3d);
				m_FeedinGradientDirectionContainer->push_back(vect3d);
				m_OutputGradientDirectionContainer->push_back(vect3d);

				diffusionIndicator.push_back(k);
				k++;
			}
		}
		unsigned int nMeasurement = DiffusionVectors.size();
		std::cout << "gradient size:" << nMeasurement
			<< "  baseline size: " << nBaseline << std::endl;

		// averaging baseline images
		VectorImageType::IndexType nrrdIdx;

		ScalarImageType::Pointer b0Image = ScalarImageType::New();
		b0Image->CopyInformation( inputPtr );
		b0Image->SetRegions( b0Image->GetLargestPossibleRegion() );
		b0Image->Allocate();
		itk::ImageRegionIteratorWithIndex<ScalarImageType> itB0(
			b0Image, b0Image->GetLargestPossibleRegion() );

		for ( itB0.GoToBegin(); !itB0.IsAtEnd(); ++itB0 )
		{
			for ( unsigned int index = 0; index < dim; index++ )
			{
				nrrdIdx[index] = itB0.GetIndex()[index];
			}

			double b = 0;
			if ( nBaseline == 0 ) // if baseline# == 0, using the first gradient as
				// reference for registration
			{
				b = static_cast<double>( inputPtr->GetPixel(nrrdIdx)[0] );
			}
			else
			{
				for ( int kLocal = 0; kLocal < nBaseline; kLocal++ )
				{
					b
						+= static_cast<double>( inputPtr->GetPixel(nrrdIdx)[baselineIndicator
						[
							kLocal]] );
				}
				b /= static_cast<double>( nBaseline );
			}
			itB0.Set( static_cast<float>( b ) );
		}

		this->re.SetGradients( DiffusionVectors );
		this->re.SetFixedImage( b0Image );

		// set baseline images
		for ( int kLocal = 0; kLocal < nBaseline; kLocal++ )
		{
			ScalarImageType::Pointer baseline = ScalarImageType::New();
			baseline->CopyInformation( inputPtr );
			baseline->SetRegions( baseline->GetLargestPossibleRegion() );
			baseline->Allocate();

			itk::ImageRegionIteratorWithIndex<ScalarImageType> itBaseLine(
				baseline,
				baseline->
				GetLargestPossibleRegion() );

			for ( itBaseLine.GoToBegin(); !itBaseLine.IsAtEnd(); ++itBaseLine )
			{
				for ( unsigned int j = 0; j < dim; j++ )
				{
					nrrdIdx[j] = itBaseLine.GetIndex()[j];
				}

				double b
					= static_cast<double>( inputPtr->GetPixel(nrrdIdx)[baselineIndicator[
						kLocal]] );
						itBaseLine.Set( static_cast<float>( b ) );
			}

			this->re.SetBaseLines( baseline );
		}

		// separate moving images
		for ( unsigned int kLocal = 0; kLocal < nMeasurement; kLocal++ )
		{
			ScalarImageType::Pointer dwi = ScalarImageType::New();
			dwi->CopyInformation( inputPtr );
			dwi->SetRegions( dwi->GetLargestPossibleRegion() );
			dwi->Allocate();
			itk::ImageRegionIteratorWithIndex<ScalarImageType> itDWI(
				dwi,
				dwi->
				GetLargestPossibleRegion() );

			for ( itDWI.GoToBegin(); !itDWI.IsAtEnd(); ++itDWI )
			{
				for ( unsigned int j = 0; j < dim; j++ )
				{
					nrrdIdx[j] = itDWI.GetIndex()[j];
				}

				double b
					= static_cast<double>( inputPtr->GetPixel(nrrdIdx)[diffusionIndicator[
						kLocal]] );
						itDWI.Set( static_cast<float>( b ) );
			}

			this->re.SetMovingImage( dwi );
		}

		this->re.SetNumberOfBins( this->GetNumberOfBins() );
		this->re.SetSamples( this->GetSamples() );
		this->re.SetTranslationScale( this->GetTranslationScale() );
		this->re.SetStepLength( this->GetStepLength() );
		this->re.SetFactor( this->GetFactor() );
		this->re.SetPrefix( "DWI_" );

		this->corr = this->re.Registration( );

		m_OutputGradientDirectionContainer = GradientDirectionContainerType::New();
		for ( unsigned int i = 0; i < this->re.GetGradients().size(); i++ )
		{
			vect3d[0] = this->re.GetGradients().at(i)[0];
			vect3d[1] = this->re.GetGradients().at(i)[1];
			vect3d[2] = this->re.GetGradients().at(i)[2];
			m_OutputGradientDirectionContainer->push_back(vect3d);
		}
		return;
	}

	/**
	*
	*/
	template <class TImageType>
	void
		DWIEddyCurrentHeadMotionCorrector<TImageType>
		::writeReport()
	{
		if ( this->GetReportFileName().length() == 0 )
		{
			return;
		}

		std::ofstream outfile;
		if ( GetReportFileMode() == DWIEddyCurrentHeadMotionCorrector::REPORT_FILE_APPEND )
		{
			outfile.open( GetReportFileName().c_str(), std::ios_base::app);
		}
		else
		{
			outfile.open( GetReportFileName().c_str() );
		}

		outfile << std::endl;
		outfile << "=============================================" << std::endl;
		outfile << "*  Eddy-Current and Head Motion Correcting  *" << std::endl;
		outfile << "=============================================" << std::endl;

		outfile << std::endl << "Parameters:" << std::endl;
		outfile << "  NumberOfBins: "      << m_NumberOfBins      << std::endl;
		outfile << "  m_Samples: "      << m_Samples        << std::endl;
		outfile << "  m_TranslationScale: "  << m_TranslationScale    << std::endl;
		outfile << "  m_StepLength: "      << m_StepLength        << std::endl;
		outfile << "  m_Factor: "        << m_Factor          << std::endl;
		outfile << "  m_MaxNumberOfIterations: " << m_MaxNumberOfIterations
			<< std::endl;

		collectLeftDiffusionStatistics();    // update
		int DWICount, BaselineCount;
		BaselineCount  = getBaselineNumber();
		DWICount    = getGradientNumber();

		outfile << std::endl << "======" << std::endl;
		outfile << " Input Diffusion Gradient information:" << std::endl;
		outfile << "\tbValueLeftNumber: "    << bValueLeftNumber    << std::endl;
		outfile << "\tbaselineLeftNumber: "  << baselineLeftNumber  << std::endl;
		outfile << "\tgradientDirLeftNumber: " << gradientDirLeftNumber  << std::endl;

		outfile << std::endl << "\t# " << "\tDirVector" << std::setw(34)
			<< std::setiosflags(std::ios::left) << "\tIncluded" << std::endl;
		for ( unsigned int i = 0; i < this->m_GradientDirectionContainer->size(); i++ )
		{
			outfile << "\t" << i << "\t[ "
				<< std::setw(9) << std::setiosflags(std::ios::fixed)
				<< std::setprecision(6) << std::setiosflags(std::ios::right)
				<< this->m_GradientDirectionContainer->at(i)[0] << ", "
				<< std::setw(9) << std::setiosflags(std::ios::fixed)
				<< std::setprecision(6) << std::setiosflags(std::ios::right)
				<< this->m_GradientDirectionContainer->at(i)[1] << ", "
				<< std::setw(9) << std::setiosflags(std::ios::fixed)
				<< std::setprecision(6) << std::setiosflags(std::ios::right)
				<< this->m_GradientDirectionContainer->at(i)[2] << " ] "
				<< "\t1" << std::endl;
		}

		outfile << std::endl << "Output Gradient Direction Histogram: " << std::endl;
		outfile << "\t# " << "\tDirVector" << std::setw(34) << std::setiosflags(
			std::ios::left) << "\tRepLeft" << std::endl;

		for ( unsigned int i = 0; i < this->DiffusionDirHistOutput.size(); i++ )
		{
			if ( GetReportFileName().length() > 0 )
			{
				outfile << "\t" << i << "\t[ "
					<< std::setw(9) << std::setiosflags(std::ios::fixed)
					<< std::setprecision(6) << std::setiosflags(std::ios::right)
					<< DiffusionDirHistOutput[i].gradientDir[0] << ", "
					<< std::setw(9) << std::setiosflags(std::ios::fixed)
					<< std::setprecision(6) << std::setiosflags(std::ios::right)
					<< DiffusionDirHistOutput[i].gradientDir[1] << ", "
					<< std::setw(9) << std::setiosflags(std::ios::fixed)
					<< std::setprecision(6) << std::setiosflags(std::ios::right)
					<< DiffusionDirHistOutput[i].gradientDir[2] << " ] "
					<< "\t" << DiffusionDirHistOutput[i].repetitionNumber
					<< std::endl;
			}
		}
		outfile.close();
		return;
	}

	/**
	*
	*/
	template <class TImageType>
	void
		DWIEddyCurrentHeadMotionCorrector<TImageType>
		::collectLeftDiffusionStatistics()
	{
		this->DiffusionDirHistOutput.clear();
		this->repetitionLeftNumber.clear();

		// set baseline
		std::vector<double> dir;
		dir.push_back( 0.0 );
		dir.push_back( 0.0 );
		dir.push_back( 0.0 );

		struDiffusionDir diffusionDir;
		diffusionDir.gradientDir = dir;
		diffusionDir.repetitionNumber = 1;

		DiffusionDirHistOutput.push_back(diffusionDir);

		for ( unsigned int i = 0; i < this->re.GetGradients().size(); i++ )
		{
			if ( DiffusionDirHistOutput.size() > 0 )
			{
				bool newDir = true;
				for ( unsigned int j = 0; j < DiffusionDirHistOutput.size(); j++ )
				{
					if ( this->re.GetGradients().at(i)[0] ==
						DiffusionDirHistOutput[j].gradientDir[0]
					&& this->re.GetGradients().at(i)[1] ==
						DiffusionDirHistOutput[j].gradientDir[1]
					&& this->re.GetGradients().at(i)[2] ==
						DiffusionDirHistOutput[j].gradientDir[2] )
					{
						DiffusionDirHistOutput[j].repetitionNumber++;
						newDir = false;
					}
				}
				if ( newDir )
				{
					std::vector<double> dirLocal;
					dirLocal.push_back( this->re.GetGradients().at(i)[0] );
					dirLocal.push_back( this->re.GetGradients().at(i)[1] );
					dirLocal.push_back( this->re.GetGradients().at(i)[2] );

					struDiffusionDir diffusionDirLocal;
					diffusionDirLocal.gradientDir = dirLocal;
					diffusionDirLocal.repetitionNumber = 1;

					DiffusionDirHistOutput.push_back(diffusionDirLocal);
				}
			}
			else
			{
				std::vector<double> dirLocal;
				dirLocal.push_back( this->re.GetGradients().at(i)[0] );
				dirLocal.push_back( this->re.GetGradients().at(i)[1] );
				dirLocal.push_back( this->re.GetGradients().at(i)[2] );

				struDiffusionDir diffusionDirLocal;
				diffusionDirLocal.gradientDir = dirLocal;
				diffusionDirLocal.repetitionNumber = 1;

				DiffusionDirHistOutput.push_back(diffusionDirLocal);
			}
		}

		std::vector<double> dirNorm;
		dirNorm.clear();

		this->baselineLeftNumber = 0;
		this->gradientLeftNumber = 0;
		// double modeTemp = 0.0;
		for ( unsigned int i = 0; i < DiffusionDirHistOutput.size(); i++ )
		{
			if ( DiffusionDirHistOutput[i].gradientDir[0] == 0.0
				&& DiffusionDirHistOutput[i].gradientDir[1] == 0.0
				&& DiffusionDirHistOutput[i].gradientDir[2] == 0.0 )
			{
				this->baselineLeftNumber = DiffusionDirHistOutput[i].repetitionNumber;
				// std::cout<<"DiffusionDirections[i].repetitionNumber: " <<i<<"
				//  "<<DiffusionDirections[i].repetitionNumber <<std::endl;
			}
			else
			{
				this->repetitionLeftNumber.push_back(
					DiffusionDirHistOutput[i].repetitionNumber);

				double normSqr
					= DiffusionDirHistOutput[i].gradientDir[0]
				* DiffusionDirHistOutput[i].gradientDir[0]
				+ DiffusionDirHistOutput[i].gradientDir[1]
				* DiffusionDirHistOutput[i].gradientDir[1]
				+ DiffusionDirHistOutput[i].gradientDir[2]
				* DiffusionDirHistOutput[i].gradientDir[2];

				//   std::cout<<"modeSqr: " <<modeSqr <<std::endl;
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
					if ( newDirMode && DiffusionDirHistOutput[i].repetitionNumber > 0 )
					{
						dirNorm.push_back(  normSqr);
						//           std::cout<<" if(newDirMode) dirMode.size(): " <<
						//  dirMode.size() <<std::endl;
					}
				}
				else
				{
					if ( DiffusionDirHistOutput[i].repetitionNumber > 0 )
					{
						dirNorm.push_back(  normSqr);
					}
					// std::cout<<" else dirMode.size(): " <<  dirMode.size() <<std::endl;
				}
			}
		}

		//   std::cout<<" repetNum.size(): " <<  repetNum.size() <<std::endl;
		//   std::cout<<" dirMode.size(): " <<  dirMode.size() <<std::endl;

		this->gradientDirLeftNumber = 0;
		this->gradientLeftNumber = 0;
		for ( unsigned int i = 0; i < this->repetitionLeftNumber.size(); i++ )
		{
			this->gradientLeftNumber += this->repetitionLeftNumber[i];
			if ( this->repetitionLeftNumber[i] > 0 )
			{
				this->gradientDirLeftNumber++;
			}
		}

		this->bValueLeftNumber = dirNorm.size();

		return;
	}

	/**
	*
	*/
	template <class TImageType>
	void
		DWIEddyCurrentHeadMotionCorrector<TImageType>
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
					std::vector<double> dir; //HACK: This should be a vnl_vector_fixed<double,3>
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
				std::vector<double> dir; //HACK: This should be a vnl_vector_fixed<double,3>
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

				//HACK:  If gradientDir where a vnl_fixed_vector<double,3> you could compute the norm of the vector
				const double normSqr =  DiffusionDirections[i].gradientDir[0]
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
				std::cout
					<< "Warning: Not all the gradient directions have same repetition. "
            << "GradientNumber= " << i << " " << repetNum[i] << " != " << repetNum[0]
					<< std::endl;
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
		DWIEddyCurrentHeadMotionCorrector<TImageType>
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
		//     for(int i=0;i< this->m_GradientDirectionContainer -> Size();i++ )
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
		DWIEddyCurrentHeadMotionCorrector<TImageType>
		::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
		int threadId )
	{
		itkDebugMacro(<< "Actually executing");

		// Get the input and output pointers
		InputImageConstPointer inputPtr = this->GetInput();
		OutputImagePointer     outputPtr = this->GetOutput();

		int gradientLeft = 0;
		gradientLeft = this->baselineLeftNumber + this->gradientLeftNumber;
		if ( gradientLeft == 0 )
		{
			std::cout << "0 gradient left" << std::endl;
			return;
		}

		// Define/declare an iterator that will walk the output region for this
		// thread.
		typedef ImageRegionIteratorWithIndex<TImageType> OutputIterator;

		OutputIterator outIt(outputPtr, outputRegionForThread);

		// Define a few indices that will be used to translate from an input pixel
		// to an output pixel
		typename TImageType::IndexType outputIndex;
		typename VectorImageType::IndexType inputIndex;    // corrected dwi

		// support progress methods/callbacks
		ProgressReporter progress( this, threadId,
			outputRegionForThread.GetNumberOfPixels() );

		typename TImageType::PixelType value;
		value.SetSize( this->corr->GetVectorLength() );

		// walk the output region, and sample the input image
		while ( !outIt.IsAtEnd() )
		{
			// determine the index of the output pixel
			outputIndex = outIt.GetIndex();

			// determine the input pixel location associated with this output pixel
			inputIndex = outputIndex;

			// int element = 0;
			for ( unsigned int i = 0; i < this->corr->GetVectorLength(); i++ )
			{
				value.SetElement( i,
					static_cast<unsigned int>( this->corr->GetPixel(inputIndex)[i] ) );
			}
			// copy the input pixel to the output
			outIt.Set( value);
			++outIt;

			progress.CompletedPixel();
		}
	}
}

#endif
