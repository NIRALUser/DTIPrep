/*=========================================================================

Program:   NeuroLib
Module:    $file: itkDWIBaselineAverger.cpp $
Language:  C++
Date:      $Date: 2009-11-26 21:52:35 $
Version:   $Revision: 1.12 $
Author:    Zhexing Liu (liuzhexing@gmail.com)

Copyright (c) NIRAL, UNC. All rights reserved.
See http://www.niral.unc.edu for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _itkDWIBaseLineAverager_cpp
#define _itkDWIBaseLineAverager_cpp

#include "itkImageFileWriter.h"

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

#include "UtilRegister.h"
#include "UtilBaselineOptimizedAverage.h"
#include "itkMultiImageRegistrationFilter.h"

#if 0 // HACK
#include "UtilGradientOptimizedAverage.h"
#endif

namespace itk
{
/**
 *
 */
template <class TVectorImageType>
DWIBaselineAverager<TVectorImageType>
::DWIBaselineAverager() :
  m_baselineNumber(0)     // Initialize to no baselines
{
  m_baselineNumber = 0;
  m_b0 = -1.0;
  m_ReportFileName = "";
  m_AverageMethod = DWIBaselineAverager::BaselineOptimized;
  m_StopCriteria = IntensityMeanSquareDiffBased;
  m_StopThreshold = 0.02;
  m_MaxIteration = 2;
  m_ReportFileMode = DWIBaselineAverager::REPORT_FILE_NEW;
  AverageDoneOff();
}

/**
 *
 */
template <class TVectorImageType>
DWIBaselineAverager<TVectorImageType>
::~DWIBaselineAverager()
{
}

/**
 * PrintSelf
 */
template <class TVectorImageType>
void
DWIBaselineAverager<TVectorImageType>
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "DWI baseline average filter: " << std::endl;
  // HACK:  This needs to be written out yet
  // TODO:  This needs to be filled out yet.
}

/**
 *
 */
template <class TVectorImageType>
void
DWIBaselineAverager<TVectorImageType>
::GenerateOutputInformation()
{
  // call the superclass' implementation of this method
  Superclass::GenerateOutputInformation();

  // get pointers to the input and output
  InputImageConstPointer inputPtr = this->GetInput();
  OutputImagePointer     outputPtr = this->GetOutput();
  if( !inputPtr || !outputPtr )
    {
    return;
    }

  // perform averaging
  this->parseGradientDirections();
  this->collectDiffusionStatistics();
  this->average();
  this->AverageDoneOn();
  this->writeReport();

  if( getBaselineNumber() == 0 )
    {
    outputPtr->SetVectorLength( this->m_gradientNumber );
    }
  else
    {
    outputPtr->SetVectorLength( 1 + this->m_gradientNumber );
    }

  itk::MetaDataDictionary outputMetaDictionary;

  itk::MetaDataDictionary imgMetaDictionary
    = inputPtr->GetMetaDataDictionary();
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string                              metaString;

  //  measurement frame
  if( imgMetaDictionary.HasKey("NRRD_measurement frame") )
    {
#if 0
    // measurement frame
    vnl_matrix_fixed<double, 3, 3> mf;
    // imaging frame
    vnl_matrix_fixed<double, 3, 3> imgf;
    imgf = inputPtr->GetDirection().GetVnlMatrix();
#endif

    std::vector<std::vector<double> > nrrdmf;
    itk::ExposeMetaData<std::vector<std::vector<double> > >(
      imgMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);

    // Meausurement frame
    itk::EncapsulateMetaData<std::vector<std::vector<double> > >(
      outputMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);
    }

  // modality
  if( imgMetaDictionary.HasKey("modality") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "modality", metaString);
    itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
                                           "modality",
                                           metaString);
    }

  // thickness
  if( imgMetaDictionary.HasKey("NRRD_thicknesses[2]") )
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
  if( imgMetaDictionary.HasKey("NRRD_centerings[0]") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "NRRD_centerings[0]", metaString);
    itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
                                           "NRRD_centerings[0]",
                                           metaString);
    }
  if( imgMetaDictionary.HasKey("NRRD_centerings[1]") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "NRRD_centerings[1]", metaString);
    itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
                                           "NRRD_centerings[1]",
                                           metaString);
    }
  if( imgMetaDictionary.HasKey("NRRD_centerings[2]") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "NRRD_centerings[2]", metaString);
    itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
                                           "NRRD_centerings[2]",
                                           metaString);
    }

  // b-value
  if( imgMetaDictionary.HasKey("DWMRI_b-value") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "DWMRI_b-value", metaString);
    itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
                                           "DWMRI_b-value",
                                           metaString);
    }

  if( getBaselineNumber() > 0 )
    {
      {
      // baseline dir vector
      std::ostringstream ossKey;
      ossKey << "DWMRI_gradient_0000";
      std::ostringstream ossMetaString;
      ossMetaString << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << 0.0 << "    "
                    << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << 0.0 << "    "
                    << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << 0.0;

      itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
                                             ossKey.str(), ossMetaString.str() );
      }

    // gradient dir vectors
    int temp = 1;
    for( unsigned int i = 0;
         i < this->m_GradientDirectionContainer->Size();
         i++ )
      {
      // Skip B0 gradients
      if( this->GradientDirectionIsB0Image(i) == true )
        {
        continue;
        }

      std::ostringstream ossLocalGradientKey;
      ossLocalGradientKey << "DWMRI_gradient_" << std::setw(4) << std::setfill(
        '0') << temp;

      std::ostringstream ossLocalGradientMetaString;
      ossLocalGradientMetaString << std::setw(9) << std::setiosflags(
        std::ios::fixed) << std::setprecision(6) << std::setiosflags(
        std::ios::right)
      << this->m_GradientDirectionContainer->
      ElementAt(i)[0] << "    "
      << std::setw(9) << std::setiosflags(
        std::ios::fixed) << std::setprecision(6) << std::setiosflags(
        std::ios::right)
      << this->m_GradientDirectionContainer->
      ElementAt(i)[1] << "    "
      << std::setw(9) << std::setiosflags(
        std::ios::fixed) << std::setprecision(6) << std::setiosflags(
        std::ios::right)
      << this->m_GradientDirectionContainer->
      ElementAt(i)[2];

      itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
                                             ossLocalGradientKey.str(),
                                             ossLocalGradientMetaString.str() );
      ++temp;
      }
    }
  else        // no baseline
    {
    for( unsigned int i = 0;
         i < this->m_GradientDirectionContainer->Size();
         i++ )
      {
      std::ostringstream ossKey;
      ossKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << i;

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

      itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
                                             ossKey.str(), ossMetaString.str() );
      }
    }
  outputPtr->SetMetaDataDictionary(outputMetaDictionary);
}

/**
 *
 */
template <class TVectorImageType>
void
DWIBaselineAverager<TVectorImageType>
::average()
{
  if( this->m_baselineNumber <= 0 )      // no baseline
    {
    std::cout << "No baseline included." << std::endl;
    m_averagedBaseline = NULL;
    this->computeIDWI();
    return;
    }
  else
    {
    InputImageConstPointer inputPtr = this->GetInput();
    m_averagedBaseline = floatImageType::New();
    m_averagedBaseline->CopyInformation(inputPtr);
    m_averagedBaseline->SetRegions( inputPtr->GetLargestPossibleRegion() );
    m_averagedBaseline->Allocate();
    m_averagedBaseline->FillBuffer(0);

    if( this->m_baselineNumber == 1 )        // single baseline: copy baseline into
      {
      DirectAverage();
      this->computeIDWI();
      }
    else        // multiple baseline images, need to be averaged
      {
      switch( this->m_AverageMethod )
        {
        case DWIBaselineAverager::BSplineOptimized:
          {
          std::cout << "Staring Direct Averaging." << std::endl;
          this->DirectAverage();
          std::cout << "Finishing Direct Averaging." << std::endl;
          // TODO:  We keep doing the same work over and over again with very little differences in the code
          //  It may be possible to extract the baseline images only once.
          //  This code could be made into a separate function so that it is not replicated.
          // std::cout<<"registering all baseline onto averaged .";
          InputImageConstPointer inputPtr = this->GetInput();
          // copy baseline images into baselineContainer
          std::vector<floatImageType::Pointer> baselineContainer;
            {
            unsigned int FoundBaselines = 0;
            for( unsigned int i = 0; i < inputPtr->GetVectorLength(); i++ )
              {

              if( this->GradientDirectionIsB0Image(i) == true )
                {
                Gradient_indx_Baselines.push_back( true );
                // TODO:  This should use the extract vector element filter.
                  {
                  std::cout << "Extracting element " << i << std::endl;
                  typedef typename itk::VectorIndexSelectionCastImageFilter<TVectorImageType,
                                                                            floatImageType> ExtractBaselineFilterType;
                  typename ExtractBaselineFilterType::Pointer componentExtractor = ExtractBaselineFilterType::New();
                  componentExtractor->SetInput(inputPtr);
                  componentExtractor->SetIndex( i );
                  try
                    {
                    componentExtractor->Update();
                    }
                  catch( itk::ExceptionObject & err )
                    {
                    std::cout << err.GetDescription() << std::endl;
                    throw;
                    }
                  typename floatImageType::Pointer baseline = componentExtractor->GetOutput();
                  baselineContainer.push_back(baseline);
                  }
                FoundBaselines++;
                }
              else if( this->GradientDirectionIsB0Image(i) == false )
                {
                Gradient_indx_Baselines.push_back( false );
                }
              }
            if( FoundBaselines != this->getBaselineNumber() )
              {
              std::cout << "WRONG NUMBER OF BASELINES FOUND: "
                        << this->getBaselineNumber() << " != " << FoundBaselines << std::endl;
              exit(-1);
              }
            }
          std::cout << "BSplineOptimized" << std::endl;
          MultiImageRegistrationFilter::Pointer registration = MultiImageRegistrationFilter::New();
          registration->SetImages(baselineContainer);
          // registration->SetOptAffineNumberOfIterations(10);
          // registration->SetOptBsplineNumberOfIterations(10);
          // registration->SetNumberOfSpatialSamplesBsplinePercentage(0.005);
          registration->Update();
          this->m_averagedBaseline = registration->GetOutput();

          this->computeIDWI();

          }
          break;
        case DWIBaselineAverager::BaselineOptimized:
          {
          std::cout << "Staring Direct Averaging." << std::endl;
          this->DirectAverage();
          std::cout << "Finishing Direct Averaging." << std::endl;
          // TODO:  We keep doing the same work over and over again with very little differences in the code
          //  It may be possible to extract the baseline images only once.
          //  This code could be made into a separate function so that it is not replicated.
          // std::cout<<"registering all baseline onto averaged .";
          InputImageConstPointer inputPtr = this->GetInput();
          // copy baseline images into baselineContainer
          std::vector<UnsignedImageType::Pointer> baselineContainer;
            {
            unsigned int FoundBaselines = 0;
            for( unsigned int i = 0; i < inputPtr->GetVectorLength(); i++ )
              {

              if( this->GradientDirectionIsB0Image(i) == true )
                {
                Gradient_indx_Baselines.push_back( true );
                // TODO:  This should use the extract vector element filter.
                  {
                  std::cout << "Extracting element " << i << std::endl;
                  typedef typename itk::VectorIndexSelectionCastImageFilter<TVectorImageType,
                                                                            UnsignedImageType>
                  ExtractBaselineFilterType;
                  typename ExtractBaselineFilterType::Pointer componentExtractor = ExtractBaselineFilterType::New();
                  componentExtractor->SetInput(inputPtr);
                  componentExtractor->SetIndex( i );
                  try
                    {
                    componentExtractor->Update();
                    }
                  catch( itk::ExceptionObject & err )
                    {
                    std::cout << err.GetDescription() << std::endl;
                    throw;
                    }
                  typename UnsignedImageType::Pointer baseline = componentExtractor->GetOutput();
                  baselineContainer.push_back(baseline);
                  }
                FoundBaselines++;
                }
              else if(  this->GradientDirectionIsB0Image(i) == false )
                {
                Gradient_indx_Baselines.push_back( false );
                }
              }
            if( FoundBaselines != this->getBaselineNumber() )
              {
              std::cout << "WRONG NUMBER OF BASELINES FOUND: "
                        << this->getBaselineNumber() << " != " << FoundBaselines << std::endl;
              exit(-1);
              return;
              }
            }
          std::cout << "BaselineOptimized" << std::endl;
          if( BaselineOptimizedAverage<UnsignedImageType>(baselineContainer, this->m_averagedBaseline,
                                                          this->m_StopCriteria, this->m_StopThreshold,
                                                          this->GetMaxIteration() ) == false )
            {
            std::cout << "Since BaselineOptimized is failed, DirectAverging is applied!" << std::endl;
            this->DirectAverage();
            }
          this->computeIDWI();
          }
          break;
#if 0     // HACK
        case DWIBaselineAverager::GradientOptamized:
          std::cout << "GradientOptamized" << std::endl;
          GradientOptimizedAverage();
          // this->computeIDWI();
          break;
#endif
        case DWIBaselineAverager::Direct:
        default:
          std::cout << "Direct" << std::endl;
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
template <class TVectorImageType>
void
DWIBaselineAverager<TVectorImageType>
::DirectAverage()
{
  if( !m_averagedBaseline )
    {
    std::cout << "WARNING:  No m_averagedBaseline" << std::endl;
    exit(-1);
    return;
    }
  // This has been re-written to make the algorithm easier to read.
  // There are some more optimizations that could occur to make this faster, but since it is currently working
  // no additional optimizations will be done now.
  // TODO:  Replace this averaging code with itk filters to make it faster and avoid using the pixelIndex.
    {
    // std::cout << "WARNING:  Averaging values together for baselines, without registration!" << std::endl;
    m_averagedBaseline->FillBuffer(0.0);
    typedef ImageRegionIteratorWithIndex<floatImageType> averagedBaselineIterator;
    averagedBaselineIterator aIt( m_averagedBaseline, m_averagedBaseline->GetLargestPossibleRegion() );
    InputImageConstPointer   inputPtr = this->GetInput();
    unsigned int             FoundBaselines = 0;
    for( unsigned int i = 0; i < inputPtr->GetVectorLength(); i++ )
      {
      if( this->GradientDirectionIsB0Image(i) == true )
        {
        aIt.GoToBegin();
        while( !aIt.IsAtEnd() )
          {
          aIt.Set(aIt.Get() + inputPtr->GetPixel(aIt.GetIndex() )[i]);
          ++aIt;
          }

        FoundBaselines++;
        }
      }
    if( this->getBaselineNumber() != FoundBaselines )
      {
      std::cout << "ERROR:  Number of baselines found does not match previous estimates!"
                << this->getBaselineNumber() << " != " << FoundBaselines << std::endl;
      exit(-1);
      return;
      }
      {
      const float InvFoundBaselines = 1.0 / static_cast<float>(FoundBaselines);
      aIt.GoToBegin();
      while( !aIt.IsAtEnd() )
        {
        aIt.Set(aIt.Get() * InvFoundBaselines);
        ++aIt;
        }
      }
    }
}

/**
 *
 */
template <class TVectorImageType>
void
DWIBaselineAverager<TVectorImageType>
::writeReport()
{
  if( GetReportFileName().length() == 0 )
    {
    return;
    }

  std::ofstream outfile;
  if( GetReportFileMode() == DWIBaselineAverager::REPORT_FILE_APPEND )
    {
    outfile.open( GetReportFileName().c_str(), std::ios_base::app);
    }
  else
    {
    outfile.open( GetReportFileName().c_str() );
    }
  // int DWICount, BaselineCount;
  int temp = 0;

  switch( m_ReportType )
    {
    case DWIBaselineAverager::REPORT_TYPE_SIMPLE:
      outfile << std::endl;
      outfile << "================================" << std::endl;
      outfile << "       Baseline averaging       " << std::endl;
      outfile << "================================" << std::endl;

      outfile << std::endl;

      switch( this->m_AverageMethod )
        {
        case DWIBaselineAverager::Direct:
          outfile
          << "Average Method: [DWIBaselineAverager::Direct]--Direct arithmetic averaging "
          << std::endl;
          break;
        case DWIBaselineAverager::BaselineOptimized:
          outfile
          << "Average Method: [DWIBaselineAverager::BaselineOptimized]--Averaging Optimized based on baseline images"
          << std::endl;
          break;
        case DWIBaselineAverager::GradientOptamized:
          outfile
          <<
          "Average Method: [DWIBaselineAverager::GradientOptamized]--Averaging Optimized based on DW gradients images "
          << std::endl;
          break;
        default:
          outfile
          << "Average Method: UNKNOWN--Using direct arithmetic averaging "
          << std::endl;
          break;
        }

      outfile << "Stop threshold: " << this->m_StopThreshold << std::endl;

      outfile << std::endl << "======" << std::endl;
      outfile << "Output Diffusion Gradient direction information:" << std::endl;

      outfile << std::endl << "\t#" << "\tDirVector" << std::endl;

      temp = 0;
      if( getBaselineNumber() > 0 )
        {
        outfile << "\t" << temp << "\t[ "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << 0.0 << ", "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << 0.0 << ", "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << 0.0 << " ]"
                << std::endl;
        temp = 1;
        }
      for( unsigned int i = 0; i < this->m_GradientDirectionContainer->size(); i++ )
        {
        if( this->GradientDirectionIsB0Image(i) == true )
          {
          continue;
          }

        outfile << "\t" << temp << "\t[ "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[0] << ", "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[1] << ", "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[2] << " ]"
                << std::endl;
        temp++;
        }

      outfile.close();
      break;

    case DWIBaselineAverager::REPORT_TYPE_VERBOSE:
      if( GetReportFileName().length() == 0 )
        {
        return;
        }

      if( GetReportFileMode() == DWIBaselineAverager::REPORT_FILE_APPEND )
        {
        outfile.open( GetReportFileName().c_str(), std::ios_base::app);
        }
      else
        {
        outfile.open( GetReportFileName().c_str() );
        }

      outfile << std::endl;
      outfile << "================================" << std::endl;
      outfile << "       Baseline averaging       " << std::endl;
      outfile << "================================" << std::endl;

      outfile << std::endl;

      switch( this->m_AverageMethod )
        {
        case DWIBaselineAverager::Direct:
          outfile
          << "Average Method: [DWIBaselineAverager::Direct]--Direct arithmetic averaging "
          << std::endl;
          break;
        case DWIBaselineAverager::BaselineOptimized:
          outfile
          << "Average Method: [DWIBaselineAverager::BaselineOptimized]--Averaging Optimized based on baseline images"
          << std::endl;
          break;
        case DWIBaselineAverager::GradientOptamized:
          outfile
          <<
          "Average Method: [DWIBaselineAverager::GradientOptamized]--Averaging Optimized based on DW gradients images "
          << std::endl;
          break;
        default:
          outfile
          << "Average Method: UNKNOWN--Using direct arithmetic averaging "
          << std::endl;
          break;
        }

      outfile << "Stop threshold: " << this->m_StopThreshold << std::endl;

      outfile << std::endl << "======" << std::endl;
      outfile << "Input Diffusion Gradient information:" << std::endl;
      outfile << "\tbValueNumber: "    << m_bValueNumber    << std::endl;
      outfile << "\tbaselineNumber: "  << m_baselineNumber  << std::endl;
      outfile << "\tgradientDirNumber: " << m_gradientDirNumber  << std::endl;

      outfile << std::endl << "\t#" << "\tDirVector" << std::endl;
      for( unsigned int i = 0; i < this->m_GradientDirectionContainer->size(); i++ )
        {
        outfile << "\t" << i << "\t[ "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[0] << ", "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[1] << ", "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[2] << " ]"
                << std::endl;
        }

      outfile << std::endl << "======" << std::endl;
      outfile << "Output Diffusion Gradient direction information:" << std::endl;

      outfile << std::endl << "\t#" << "\tDirVector" << std::endl;

      temp = 0;
      if( getBaselineNumber() > 0 )
        {
        outfile << "\t" << temp << "\t[ "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << 0.0 << ", "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << 0.0 << ", "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << 0.0 << " ]"
                << std::endl;
        temp = 1;
        }
      for( unsigned int i = 0; i < this->m_GradientDirectionContainer->size(); i++ )
        {
        if( this->GradientDirectionIsB0Image(i) == true )
          {
          continue;
          }

        outfile << "\t" << temp << "\t[ "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[0] << ", "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[1] << ", "
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[2] << " ]"
                << std::endl;
        temp++;
        }

      outfile.close();
      break;

    case DWIBaselineAverager::REPORT_TYPE_EASY_PARSE:
      if( GetReportFileName().length() == 0 )
        {
        return;
        }

      if( GetReportFileMode() == DWIBaselineAverager::REPORT_FILE_APPEND )
        {
        outfile.open( GetReportFileName().c_str(), std::ios_base::app);
        }
      else
        {
        outfile.open( GetReportFileName().c_str() );
        }

      outfile << std::endl;

      switch( this->m_AverageMethod )
        {
        case DWIBaselineAverager::Direct:
          outfile << "Baseline_Avg\t"
                  << "Average Method Direct "
                  << std::endl;
          break;
        case DWIBaselineAverager::BaselineOptimized:
          outfile << "Baseline_Avg\t"
                  << "Average Method BaselineOptimized"
                  << std::endl;
          break;
        case DWIBaselineAverager::GradientOptamized:
          outfile << "Baseline_Avg\t"
                  << "Average Method GradientOptamized"
                  << std::endl;
          break;
        default:
          outfile << "Baseline_Avg\t"
                  << "Average Method UNKNOWN"
                  << std::endl;
          break;
        }

      outfile << "Baseline_Avg\tStop threshold " << this->m_StopThreshold << std::endl;

      outfile << "Baseline_Avg\tbValueNumber "    << m_bValueNumber    << std::endl;
      outfile << "Baseline_Avg\tbaselineNumber "  << m_baselineNumber  << std::endl;
      outfile << "Baseline_Avg\tgradientDirNumber " << m_gradientDirNumber  << std::endl;

      outfile << std::endl << "Pre_baseline_avg\tGradientNum " << "\tx\ty\tz" << std::endl;
      for( unsigned int i = 0; i < this->m_GradientDirectionContainer->size(); i++ )
        {
        outfile << "Pre_baseline_avg\t" << i << "\t"
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[0] << "\t"
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[1] << "\t"
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[2]
                << std::endl;
        }

      //  outfile << std::endl << "======" << std::endl;
      //  outfile << "Output Diffusion Gradient direction information:" << std::endl;

      outfile << std::endl << "Post_baseline_avg\tGradientNum " << "\tx\ty\tz" << std::endl;

      temp = 0;
      if( getBaselineNumber() > 0 )
        {
        outfile << "Post_baseline_avg\t" << temp << "\t"
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << 0.0 << "\t"
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << 0.0 << "\t"
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << 0.0 << std::endl;
        temp = 1;
        }
      for( unsigned int i = 0; i < this->m_GradientDirectionContainer->size(); i++ )
        {
        if( this->GradientDirectionIsB0Image(i) == true )
          {
          continue;
          }

        outfile << "Post_baseline_avg\t" << temp << "\t"
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[0] << "\t"
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[1] << "\t"
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << m_GradientDirectionContainer->ElementAt(i)[2]
                << std::endl;
        temp++;
        }

      outfile.close();
      break;

    case DWIBaselineAverager::REPORT_TYPE_NO:
    default:
      break;
    }

  return;
}

/**
 *
 */
template <class TVectorImageType>
void
DWIBaselineAverager<TVectorImageType>
::collectDiffusionStatistics()
{
  std::vector<struDiffusionDir> DiffusionDirections;
  DiffusionDirections.clear();
  for( unsigned int i = 0; i < this->m_GradientDirectionContainer->size(); i++ )
    {
      {
      bool newDir = true;
      for( unsigned int j = 0; j < DiffusionDirections.size(); j++ )
        {
        // HACK:   Should do a better comparison,  comparing floating point numbers is not reliable
        if( vcl_abs(this->m_GradientDirectionContainer->ElementAt(i)[0] - DiffusionDirections[j].gradientDir[0]) <
            NearZeroSmallNumber
            && vcl_abs(this->m_GradientDirectionContainer->ElementAt(i)[1] - DiffusionDirections[j].gradientDir[1]) <
            NearZeroSmallNumber
            && vcl_abs(this->m_GradientDirectionContainer->ElementAt(i)[2] - DiffusionDirections[j].gradientDir[2]) <
            NearZeroSmallNumber )
          {
          DiffusionDirections[j].m_repetitionNumber++;
          newDir = false;
          }
        }
      if( newDir )
        {
        std::vector<double> dir;
        dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[0]);
        dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[1]);
        dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[2]);

        struDiffusionDir diffusionDir;
        diffusionDir.gradientDir = dir;
        diffusionDir.m_repetitionNumber = 1;

        DiffusionDirections.push_back(diffusionDir);
        }
      }
    }

  // std::cout<<"DiffusionDirections.size(): " <<
  // m_GradientDirectionContainer->size() <<std::endl;

  std::vector<int> repetNum;
  repetNum.clear();
  std::vector<double> dirNorm;
  dirNorm.clear();
  // double modeTemp = 0.0;
  for( unsigned int i = 0; i < DiffusionDirections.size(); i++ )
    {
    if(
      vcl_abs(DiffusionDirections[i].gradientDir[0]) < NearZeroSmallNumber
      && vcl_abs(DiffusionDirections[i].gradientDir[1]) < NearZeroSmallNumber
      && vcl_abs(DiffusionDirections[i].gradientDir[2]) < NearZeroSmallNumber
      )
      {
      this->m_baselineNumber = DiffusionDirections[i].m_repetitionNumber;

      }
    else
      {
      repetNum.push_back(DiffusionDirections[i].m_repetitionNumber);

      const double normSqr =
        +DiffusionDirections[i].gradientDir[0]
        * DiffusionDirections[i].gradientDir[0]
        + DiffusionDirections[i].gradientDir[1]
        * DiffusionDirections[i].gradientDir[1]
        + DiffusionDirections[i].gradientDir[2]
        * DiffusionDirections[i].gradientDir[2];

      // std::cout<<"normSqr: " <<normSqr <<std::endl;
      if( dirNorm.size() > 0 )
        {
        bool newDirMode = true;
        for( unsigned int j = 0; j < dirNorm.size(); j++ )
          {
          if( vcl_abs(normSqr - dirNorm[j]) < 0.001 )      // 1 DIFFERENCE for b value
            {
            newDirMode = false;
            break;
            }
          }
        if( newDirMode )
          {
          dirNorm.push_back( normSqr );
          }
        }
      else
        {
        dirNorm.push_back( normSqr );
        }
      }
    }

  this->m_gradientDirNumber = repetNum.size();
  this->m_bValueNumber = dirNorm.size();

  this->m_gradientNumber = this->m_GradientDirectionContainer->size() - this->m_baselineNumber;

#if 0
  std::cout << "DWI Diffusion Information: "    << std::endl;
  std::cout << "  m_baselineNumber: "    << m_baselineNumber  << std::endl;
  std::cout << "  m_bValueNumber: "    << m_bValueNumber    << std::endl;
  std::cout << "  m_gradientDirNumber: "  << m_gradientDirNumber  << std::endl;
  std::cout << "  m_gradientNumber: "    << m_gradientNumber  << std::endl;

#endif
  return;
}

/**
 *
 */
template <class TVectorImageType>
void
DWIBaselineAverager<TVectorImageType>
::parseGradientDirections()
{
  typename TVectorImageType::ConstPointer inputPtr = this->GetInput();
  itk::MetaDataDictionary imgMetaDictionary = inputPtr->GetMetaDataDictionary();          //

  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  m_GradientDirectionContainer = GradientDirectionContainerType::New();
  for(
    std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
    itKey != imgMetaKeys.end(); itKey++ )
    {
    std::string metaString;
    itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
    // std::cout<<*itKey<<"  "<<metaString<<std::endl;

    if( itKey->find("DWMRI_gradient") != std::string::npos )
      {
      GradientDirectionType vect3d;
      std::istringstream    iss(metaString);
      iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
      m_GradientDirectionContainer->push_back(vect3d);
      }
    else if( itKey->find("DWMRI_b-value") != std::string::npos )
      {
      this->m_b0 = atof( metaString.c_str() );
      }
    }

  if( this->m_b0 < 0.0 )
    {
    std::cout << "BValue not specified in header file" << std::endl;
    exit(-1);    // HACK:  Attempting to debug
    return;
    }
  for( unsigned int i = 0; i < this->m_GradientDirectionContainer->Size(); i++ )
    {
    m_bValues.push_back( static_cast<int>(
                           (
                             +this->m_GradientDirectionContainer->ElementAt(i)[0]
                             * this->m_GradientDirectionContainer->ElementAt(i)[0]
                             + this->m_GradientDirectionContainer->ElementAt(i)[1]
                             * this->m_GradientDirectionContainer->ElementAt(i)[1]
                             + this->m_GradientDirectionContainer->ElementAt(i)[2]
                             * this->m_GradientDirectionContainer->ElementAt(i)[2]
                           ) * m_b0 + 0.5
                           )
                         );
    }

  //     std::cout << "b values:" << std::endl;
  //     for(unsigned int i=0;i< this->m_GradientDirectionContainer ->
  // Size();i++ )
  //     {
  //       std::cout << m_bValues[i] << std::endl;
  //     }

  if( m_GradientDirectionContainer->size() <= 6 )
    {
    std::cout << "Gradient Images Less than 6" << std::endl;
    exit(-1);    // HACK:  Attempting to debug
    return;
    }
}

/**
 *
 */
template <class TVectorImageType>
void
DWIBaselineAverager<TVectorImageType>
::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                       int threadId )
{
  itkDebugMacro(<< "Actually executing");

  // Get the input and output pointers
  InputImageConstPointer inputPtr = this->GetInput();
  OutputImagePointer     outputPtr = this->GetOutput();

  // Define/declare an iterator that will walk the output region for this
  // thread.
  typedef ImageRegionIteratorWithIndex<TVectorImageType> OutputIterator;

  OutputIterator outIt(outputPtr, outputRegionForThread);
  outIt.GoToBegin();

  // Define a few indices that will be used to translate from an input pixel
  // to an output pixel
  typename TVectorImageType::IndexType outputIndex;
  typename TVectorImageType::IndexType inputIndex;

  // support progress methods/callbacks
  ProgressReporter progress( this, threadId,
                             outputRegionForThread.GetNumberOfPixels() );

  typename TVectorImageType::PixelType value;
  if( getBaselineNumber() == 0 || getBaselineNumber() == 1 )      // copy input to
  // output
    {
    value.SetSize( inputPtr->GetVectorLength() );
    while( !outIt.IsAtEnd() )
      {
      // determine the index of the output pixel
      outputIndex = outIt.GetIndex();

      // determine the input pixel location associated with this output pixel
      inputIndex = outputIndex;
      outIt.Set( inputPtr->GetPixel(inputIndex) );
      ++outIt;

      progress.CompletedPixel();
      }
    }
  else       // combine m_averagedBaseline and DWIs
    {
    value.SetSize( this->m_gradientNumber + 1 );
    while( !outIt.IsAtEnd() )
      {
      // determine the index of the output pixel
      outputIndex = outIt.GetIndex();

      // determine the input pixel location associated with this output pixel
      inputIndex = outputIndex;

      value.SetElement( 0, m_averagedBaseline->GetPixel(inputIndex) );

      int element = 1;
      for( unsigned int i = 0; i < inputPtr->GetVectorLength(); i++ )
        {
        if( this->GradientDirectionIsB0Image(i) == true )
          {
          continue;
          }
        else
          {
          value.SetElement( element, inputPtr->GetPixel(inputIndex)[i] );
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
template <class TVectorImageType>
void
DWIBaselineAverager<TVectorImageType>
::computeIDWI()
{
  InputImageConstPointer inputPtr = this->GetInput();

  idwi = UnsignedImageType::New();
  idwi->CopyInformation(inputPtr);
  idwi->SetRegions( inputPtr->GetLargestPossibleRegion() );
  idwi->Allocate();
  idwi->FillBuffer(0);

  typedef ImageRegionIteratorWithIndex<UnsignedImageType> idwiIterator;
  idwiIterator idwiIt( idwi, idwi->GetLargestPossibleRegion() );
  idwiIt.GoToBegin();

  while( !idwiIt.IsAtEnd() )
    {
    // determine the index of the output pixel
    const typename UnsignedImageType::IndexType pixelIndex = idwiIt.GetIndex();
    double pixelValue = 1.0;
    for( unsigned int i = 0; i < inputPtr->GetVectorLength(); i++ )
      {
      if( this->GradientDirectionIsB0Image(i) == false )
        {
        pixelValue *= inputPtr->GetPixel(pixelIndex)[i];
        }
      }
    idwiIt.Set( static_cast<unsigned int>(
                  vcl_pow( pixelValue, 1.0 / this->getGradientNumber() ) ) );
    ++idwiIt;
    }
}

}
#endif
