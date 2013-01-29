/*=========================================================================

Program:   NeuroLib
Module:    $file: itkDWIQCGradientChecker.cpp $
Language:  C++
Date:      $Date: 2009-11-26 21:52:35 $
Version:   $Revision: 1.10 $
Author:    Zhexing Liu (liuzhexing@gmail.com)

Copyright (c) NIRAL, UNC. All rights reserved.
See http://www.niral.unc.edu for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _itkDWIQCGradientChecker_hxx
#define _itkDWIQCGradientChecker_hxx

#include "itkDWIQCGradientChecker.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkExceptionObject.h"
#include "itkProgressReporter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkExtractImageFilter.h"
#include "vnl/vnl_matrix.h"

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
template <class TImageType>
DWIQCGradientChecker<TImageType>
::DWIQCGradientChecker()
{
  m_TranslationThreshold  = 2.00;
  m_RotationThreshold    = 0.50;

  b0 = -1.0;
  m_ReportFileName = "";
  m_ReportFileMode = DWIQCGradientChecker::REPORT_FILE_NEW;
  m_ReferenceIndex = 0;
  bValues.clear();

  m_ExcludeGradientsWithLargeMotionArtifacts = true;

  CheckDoneOff();
}

/**
*
*/
template <class TImageType>
DWIQCGradientChecker<TImageType>
::~DWIQCGradientChecker()
{
}

/**
* PrintSelf
*/
template <class TImageType>
void
DWIQCGradientChecker<TImageType>
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "DWI QC gradient-wise check filter: " << std::endl;
}

/**
* initialize QC Resullts
*/
template <class TImageType>
void
DWIQCGradientChecker<TImageType>
::initializeQCResullts()
{
  for( unsigned int j = 0; j < this->GetInput()->GetVectorLength(); j++ )
    {
    this->qcResults.push_back(1);
    }
}

/**
*
*/
template <class TImageType>
void
DWIQCGradientChecker<TImageType>
::rigidRegistration(
  GradientImageType::Pointer fixed,
  GradientImageType::Pointer moving,
  unsigned int BinsNumber,
  double SamplesPercent,
  bool /* ExplicitPDFDerivatives */,
  GradientResult &  regResult
  )
{
  // setup pipeline
  typedef itk::VersorRigid3DTransform<double>
  TransformType;
  typedef itk::VersorRigid3DTransformOptimizer
  OptimizerType;
  typedef itk::MattesMutualInformationImageToImageMetric<GradientImageType,
                                                         GradientImageType> MetricType;
  typedef itk::LinearInterpolateImageFunction<GradientImageType,
                                              double>            InterpolatorType;
  typedef itk::ImageRegistrationMethod<GradientImageType,
                                       GradientImageType> RegistrationType;

  typedef GradientImageType::SpacingType
  SpacingType;
  typedef GradientImageType::PointType
  OriginType;
  typedef GradientImageType::RegionType
  RegionType;
  typedef GradientImageType::SizeType
  SizeType;

  MetricType::Pointer       metric      = MetricType::New();
  OptimizerType::Pointer    optimizer    = OptimizerType::New();
  InterpolatorType::Pointer interpolator  = InterpolatorType::New();
  RegistrationType::Pointer registration  = RegistrationType::New();
  TransformType::Pointer    transform    = TransformType::New();

  GradientImageType::Pointer fixedImage = fixed;
  GradientImageType::Pointer movingImage = moving;

  unsigned int numberOfBins = BinsNumber;
  double       percentOfSamples = SamplesPercent;     // 1% ~ 20%
  // bool  useExplicitPDFDerivatives = ExplicitPDFDerivatives;

  registration->SetMetric(        metric        );
  registration->SetOptimizer(     optimizer     );
  registration->SetInterpolator(  interpolator  );
  registration->SetTransform(     transform    );

  registration->SetFixedImage(   fixedImage );
  registration->SetMovingImage(  movingImage);

  // setup parameters
  registration->SetFixedImageRegion( fixedImage->GetBufferedRegion() );

  typedef itk::CenteredTransformInitializer<TransformType, GradientImageType,
                                            GradientImageType> TransformInitializerType;
  TransformInitializerType::Pointer initializer = TransformInitializerType::New();

  initializer->SetTransform(   transform );
  initializer->SetFixedImage(  fixedImage );
  initializer->SetMovingImage( movingImage);
  initializer->MomentsOn();
  // initializer->GeometryOn();
  initializer->InitializeTransform();

  typedef TransformType::VersorType VersorType;
  typedef VersorType::VectorType    VectorType;

  VersorType rotation;
  VectorType axis;

  axis[0] = 0.0;
  axis[1] = 0.0;
  axis[2] = 1.0;

  const double angle = 0;
  rotation.Set(  axis, angle  );
  transform->SetRotation( rotation );

  registration->SetInitialTransformParameters( transform->GetParameters() );

  typedef OptimizerType::ScalesType OptimizerScalesType;
  OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );
  const double        translationScale = 1.0 / 1000.0;

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

  int SampleSize
    = (int)(fixedImage->GetPixelContainer()->Size() * percentOfSamples);
  if( SampleSize > 100000 )
    {
    metric->SetNumberOfSpatialSamples( SampleSize );
    }
  else
    {
    metric->UseAllPixelsOn();
    }

  // metric->SetUseExplicitPDFDerivatives( useExplicitPDFDerivatives ); //true
  // for small #of parameters; false for big #of transform paramrters

  // run the registration pipeline
  try
    {
    registration->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    return;
    }

  OptimizerType::ParametersType finalParameters
    = registration->GetLastTransformParameters();

  const double finalAngleX           = finalParameters[0];
  const double finalAngleY           = finalParameters[1];
  const double finalAngleZ           = finalParameters[2];
  const double finalTranslationX     = finalParameters[3];
  const double finalTranslationY     = finalParameters[4];
  const double finalTranslationZ     = finalParameters[5];

  // const unsigned int numberOfIterations = optimizer->GetCurrentIteration();
  const double bestValue = optimizer->GetValue();

  // Print out results
  const double finalAngleInDegreesX = finalAngleX * 45.0 / atan(1.0);
  const double finalAngleInDegreesY = finalAngleY * 45.0 / atan(1.0);
  const double finalAngleInDegreesZ = finalAngleZ * 45.0 / atan(1.0);

  //     std::cout << "Result = " << std::endl;
  //     std::cout << " AngleX (radians)   = " << finalAngleX  << std::endl;
  //     std::cout << " AngleX (degrees)   = " << finalAngleInDegreesX  <<
  // std::endl;
  //     std::cout << " AngleY (radians)   = " << finalAngleY  << std::endl;
  //     std::cout << " AngleY (degrees)   = " << finalAngleInDegreesY  <<
  // std::endl;
  //     std::cout << " AngleZ (radians)   = " << finalAngleZ  << std::endl;
  //     std::cout << " AngleZ (degrees)   = " << finalAngleInDegreesZ  <<
  // std::endl;
  //     std::cout << " Translation X = " << finalTranslationX  << std::endl;
  //     std::cout << " Translation Y = " << finalTranslationY  << std::endl;
  //     std::cout << " Translation Z = " << finalTranslationZ  << std::endl;
  //     std::cout << " Iterations    = " << numberOfIterations << std::endl;
  //     std::cout << " Metric value  = " << bestValue          << std::endl;

  regResult.AngleX = finalAngleInDegreesX;
  regResult.AngleY = finalAngleInDegreesY;
  regResult.AngleZ = finalAngleInDegreesZ;
  regResult.TranslationX = finalTranslationX;
  regResult.TranslationY = finalTranslationY;
  regResult.TranslationZ = finalTranslationZ;
  regResult.MutualInformation = -bestValue;

  return;
}

/**
*
*/
template <class TImageType>
void
DWIQCGradientChecker<TImageType>
::calculate()
{
  std::cout << "Gradient calculating " << std::endl;
  InputImageConstPointer inputPtr = this->GetInput();

  typedef itk::VectorIndexSelectionCastImageFilter<TImageType,
                                                   GradientImageType> FilterType;
  typename FilterType::Pointer componentExtractor = FilterType::New();
  componentExtractor->SetInput(inputPtr);

  typename FilterType::Pointer componentExtractor1 = FilterType::New();
  componentExtractor1->SetInput(inputPtr);

  // set the first baseline or first gradient as a reference if no baseline
  // found
  this->m_ReferenceIndex = 0;
  for( unsigned int i = 0; i < ResultsContainer.size(); i++ )
    {
    if( this->m_GradientDirectionContainer->at(i)[0] == 0.0
        && this->m_GradientDirectionContainer->at(i)[1] == 0.0
        && this->m_GradientDirectionContainer->at(i)[2] == 0.0    )
      {
      m_ReferenceIndex = i;
      break;
      }
    }
  componentExtractor->SetIndex(m_ReferenceIndex);
  componentExtractor->Update();

  std::vector<struGradientResult> Results;
  for( unsigned int j = 0; j < inputPtr->GetVectorLength(); j++ )
    {
    struGradientResult result;

    if( m_ReferenceIndex == j )
      {
      result.AngleX = 0.0;
      result.AngleY = 0.0;
      result.AngleZ = 0.0;
      result.TranslationX = 0.0;
      result.TranslationY = 0.0;
      result.TranslationZ = 0.0;
      result.MutualInformation = 1.0;

      this->ResultsContainer.push_back(result);
      continue;
      }

    componentExtractor1->SetIndex( j );
    componentExtractor1->Update();

    std::cout << "Register Gradient " << j << " to Baseline or first Image ..." << std::endl;

    rigidRegistration(
      componentExtractor->GetOutput(),
      componentExtractor1->GetOutput(), 25, 0.1, 1, result );
    this->ResultsContainer.push_back(result);

    }

  std::cout << " DONE" << std::endl;
  return;
}

/**
*
*/
template <class TImageType>
void
DWIQCGradientChecker<TImageType>
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

  // perform SliceWiseCheck
  parseGradientDirections();
  collectDiffusionStatistics();
  initializeQCResullts();
  calculate();
  DoCheck();
  collectLeftDiffusionStatistics();
  writeReport();
  CheckDoneOn();

  if( m_ExcludeGradientsWithLargeMotionArtifacts )
    {
    outputPtr->SetVectorLength(
      this->baselineLeftNumber + this->gradientLeftNumber);
    }
  else
    {
    outputPtr->SetVectorLength( qcResults.size() );
    }

  itk::MetaDataDictionary outputMetaDictionary;

  itk::MetaDataDictionary imgMetaDictionary
    = inputPtr->GetMetaDataDictionary();
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::string metaString;

  //  measurement frame
  if( imgMetaDictionary.HasKey("NRRD_measurement frame") )
    {
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
  if( imgMetaDictionary.HasKey("modality") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "modality", metaString);
    itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
                                           "modality",
                                           metaString);
    }

  //     // thickness
  //     if(imgMetaDictionary.HasKey("NRRD_thicknesses[2]"))
  //     {
  //       double thickness;
  //       itk::ExposeMetaData<double>(imgMetaDictionary, "NRRD_thickness[2]",
  // thickness);
  //       itk::EncapsulateMetaData<double>( outputMetaDictionary,
  // "NRRD_thickness[2]", thickness);
  //     }

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

  // gradient vectors
  int temp = 0;
  for( unsigned int i = 0; i < this->m_GradientDirectionContainer->Size(); i++ )
    {
    if( m_ExcludeGradientsWithLargeMotionArtifacts )
      {
      if( this->qcResults[i] )
        {
        std::ostringstream ossKey;
        ossKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0')
               << temp;

        std::ostringstream ossMetaString;
        ossMetaString << std::setprecision(17)
                      << this->m_GradientDirectionContainer->ElementAt(i)[0]
                      << "    "
                      << this->m_GradientDirectionContainer->ElementAt(i)[1]
                      << "    "
                      << this->m_GradientDirectionContainer->ElementAt(i)[2];

        // std::cout<<ossKey.str()<<ossMetaString.str()<<std::endl;
        itk::EncapsulateMetaData<std::string>( outputMetaDictionary, ossKey.str(
                                                 ), ossMetaString.str() );
        ++temp;
        }
      }
    else
      {
      std::ostringstream ossKey;
      ossKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << temp;

      std::ostringstream ossMetaString;
      ossMetaString << std::setprecision(17)
                    << this->m_GradientDirectionContainer->ElementAt(i)[0]
                    << "    "
                    << this->m_GradientDirectionContainer->ElementAt(i)[1]
                    << "    "
                    << this->m_GradientDirectionContainer->ElementAt(i)[2];

      itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
                                             ossKey.str(), ossMetaString.str() );
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
DWIQCGradientChecker<TImageType>
::DoCheck()
{
  std::cout << "Gradient checking ...";
  //     int DWICount, BaselineCount;
  //     BaselineCount  = getBaselineNumber();
  //     DWICount    = getGradientNumber();
  //     std::cout<<"BaselineCount: "<<BaselineCount<<std::endl;
  //     std::cout<<"DWICount: "<<DWICount<<std::endl;
  //     std::cout<<"m_RotationThreshold: "<<m_RotationThreshold<<std::endl;
  //     std::cout<<"m_TranslationThreshold:
  // "<<m_TranslationThreshold<<std::endl;
  for( unsigned int i = 0; i < this->ResultsContainer.size(); i++ )
    {

    if( fabs(this->ResultsContainer[i].AngleX) > m_RotationThreshold
        || fabs(this->ResultsContainer[i].AngleY) > m_RotationThreshold
        || fabs(this->ResultsContainer[i].AngleZ) > m_RotationThreshold
        || fabs(this->ResultsContainer[i].TranslationX) > m_TranslationThreshold
        || fabs(this->ResultsContainer[i].TranslationY) > m_TranslationThreshold
        || fabs(this->ResultsContainer[i].TranslationZ) > m_TranslationThreshold     )
      {
      this->qcResults[i] = 0;
      }
    }

  std::cout << " DONE" << std::endl;
  return;
}

/**
*
*/
template <class TImageType>
void
DWIQCGradientChecker<TImageType>
::writeReport()
{

  if( this->m_ReportFileName.length() == 0 )
    {
    return;
    }

  std::ofstream outfile;
  // int DWICount;
  // int BaselineCount;

  if( GetReportFileMode() == DWIQCGradientChecker::REPORT_FILE_APPEND )
    {
    outfile.open( GetReportFileName().c_str(), std::ios_base::app);
    }
  else
    {
    outfile.open( GetReportFileName().c_str() );
    }

  switch( m_ReportType )
    {
    case DWIQCGradientChecker::REPORT_TYPE_SIMPLE:
      outfile << std::endl;
      outfile << "================================" << std::endl;
      outfile << "    Gradient-wise checking      " << std::endl;
      outfile << "================================" << std::endl;

      outfile << std::endl << "Parameters:" << std::endl;

      outfile << "  TranslationThreshold: "    << m_TranslationThreshold
              << std::endl;
      outfile << "  RotationThreshold: "    << m_RotationThreshold    << std::endl;

      outfile << std::endl << "Inter-gradient check Artifacts::" << std::endl;
      outfile << "\t" << std::setw(10) << "Gradient#:\t" << std::setw(10)
              << "AngleX\t" << std::setw(10) << "AngleY\t"
              << std::setw(10) << "AngleZ\t" << std::setw(10)
              << "TranslationX\t" << std::setw(10) << "TranslationY\t"
              << std::setw(10) << "TranslationZ\t" << std::setw(10)
              << "Metric(MI)" << std::endl;

      std::cout << "DWIGradientResultsContainer" << ResultsContainer.size() << std::endl;
      for( unsigned int i = 0; i < this->ResultsContainer.size(); i++ )
        {
        if( !this->qcResults[i] )
          {
          outfile.precision(6);
          outfile.setf(std::ios_base::showpoint | std::ios_base::right);
          outfile << "\t" << std::setw(10) << i;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].AngleX;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].AngleY;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].AngleZ;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].TranslationX;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].TranslationY;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].TranslationZ;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].MutualInformation;
          outfile << std::endl;
          outfile.precision();
          outfile.setf(std::ios_base::unitbuf);
          }
        }
      break;

    case DWIQCGradientChecker::REPORT_TYPE_VERBOSE:
      outfile << std::endl;
      outfile << "================================" << std::endl;
      outfile << "    Gradient-wise checking      " << std::endl;
      outfile << "================================" << std::endl;

      outfile << std::endl << "Parameters:" << std::endl;

      outfile << "  TranslationThreshold: "    << m_TranslationThreshold
              << std::endl;
      outfile << "  RotationThreshold: "    << m_RotationThreshold    << std::endl;

      outfile << std::endl << "======" << std::endl;
      outfile << "Gradient-wise motion check with MI-based 3D rigid registration: "
              << std::endl;
      outfile << "\t" << std::setw(10) << "Register#:\t" << std::setw(10)
              << "AngleX\t" << std::setw(10) << "AngleY\t"
              << std::setw(10) << "AngleZ\t" << std::setw(10)
              << "TranslationX\t" << std::setw(10) << "TranslationY\t"
              << std::setw(10) << "TranslationZ\t" << std::setw(10)
              << "Metric(MI)" << std::endl;

      collectLeftDiffusionStatistics();    // update
      for( unsigned int i = 0; i < this->ResultsContainer.size(); i++ )
        {
        if( m_ReferenceIndex == i )
          {
          continue;
          }
        outfile.precision(6);
        outfile.setf(std::ios_base::showpoint | std::ios_base::right);
        outfile << "\tRegister " << i << "-" << m_ReferenceIndex;
        outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right);
        outfile << this->ResultsContainer[i].AngleX;
        outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right);
        outfile << this->ResultsContainer[i].AngleY;
        outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right);
        outfile << this->ResultsContainer[i].AngleZ;
        outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right);
        outfile << this->ResultsContainer[i].TranslationX;
        outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right);
        outfile << this->ResultsContainer[i].TranslationY;
        outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right);
        outfile << this->ResultsContainer[i].TranslationZ;
        outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right);
        outfile << this->ResultsContainer[i].MutualInformation;
        outfile << std::endl;
        outfile.precision();
        outfile.setf(std::ios_base::unitbuf);
        }

      outfile << std::endl << "Inter-gradient check Artifacts::" << std::endl;
      outfile << "\t" << std::setw(10) << "Gradient#:\t" << std::setw(10)
              << "AngleX\t" << std::setw(10) << "AngleY\t"
              << std::setw(10) << "AngleZ\t" << std::setw(10)
              << "TranslationX\t" << std::setw(10) << "TranslationY\t"
              << std::setw(10) << "TranslationZ\t" << std::setw(10)
              << "Metric(MI)" << std::endl;
      for( unsigned int i = 0; i < this->ResultsContainer.size(); i++ )
        {
        if( !this->qcResults[i] )
          {
          outfile.precision(6);
          outfile.setf(std::ios_base::showpoint | std::ios_base::right);
          outfile << "\t" << std::setw(10) << i;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].AngleX;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].AngleY;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].AngleZ;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].TranslationX;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].TranslationY;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].TranslationZ;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].MutualInformation;
          outfile << std::endl;
          outfile.precision();
          outfile.setf(std::ios_base::unitbuf);
          }
        }

      outfile << std::endl << "======" << std::endl;
      outfile << "Left Diffusion Gradient information:" << std::endl;
      outfile << "\tbValueLeftNumber: "    << bValueLeftNumber    << std::endl;
      outfile << "\tbaselineLeftNumber: "  << baselineLeftNumber  << std::endl;
      outfile << "\tgradientDirLeftNumber: " << gradientDirLeftNumber  << std::endl;

      outfile << std::endl << "\t#" << "\tDirVector" << std::setw(34)
              << std::setiosflags(std::ios::left) << "\tIncluded" << std::endl;
      for( unsigned int i = 0; i < this->m_GradientDirectionContainer->size(); i++ )
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
                << "\t" << this->qcResults[i] << std::endl;
        }

      outfile << std::endl << "Left Gradient Direction Histogram: " << std::endl;
      outfile << "\t#" << "\tDirVector" << std::setw(34)
              << std::setiosflags(std::ios::left) << "\tRepLeft" << std::endl;
      for( unsigned int i = 0; i < this->DiffusionDirHistOutput.size(); i++ )
        {
        if( m_ReportFileName.length() > 0 )
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
                  << DiffusionDirHistOutput[i].gradientDir[2] << " ]"
                  << "\t" << DiffusionDirHistOutput[i].repetitionNumber
                  << std::endl;
          }
        }
      break;

    case DWIQCGradientChecker::REPORT_TYPE_EASY_PARSE:
      outfile << std::endl;
      outfile << "Gradientwise_checking  TranslationThreshold "    << m_TranslationThreshold
              << std::endl;
      outfile << "Gradientwise_checking  RotationThreshold "    << m_RotationThreshold    << std::endl;

      outfile << std::endl;
      outfile << "Gradientwise_motion_check\t"
              << std::setw(10) << "RegisterGradientNum\t" << std::setw(10)
              << "AngleX\t" << std::setw(10) << "AngleY\t"
              << std::setw(10) << "AngleZ\t" << std::setw(10)
              << "TranslationX\t" << std::setw(10) << "TranslationY\t"
              << std::setw(10) << "TranslationZ\t" << std::setw(10)
              << "Metric(MI)" << std::endl;

      collectLeftDiffusionStatistics();    // update
      for( unsigned int i = 0; i < this->ResultsContainer.size(); i++ )
        {
        if( m_ReferenceIndex == i )
          {
          continue;
          }
        outfile.precision(6);
        outfile.setf(std::ios_base::showpoint | std::ios_base::right);
        outfile << "Gradientwise_motion_check\tRegister " << i << "-" << m_ReferenceIndex;
        outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right);
        outfile << this->ResultsContainer[i].AngleX;
        outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right);
        outfile << this->ResultsContainer[i].AngleY;
        outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right);
        outfile << this->ResultsContainer[i].AngleZ;
        outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right);
        outfile << this->ResultsContainer[i].TranslationX;
        outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right);
        outfile << this->ResultsContainer[i].TranslationY;
        outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right);
        outfile << this->ResultsContainer[i].TranslationZ;
        outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right);
        outfile << this->ResultsContainer[i].MutualInformation;
        outfile << std::endl;
        outfile.precision();
        outfile.setf(std::ios_base::unitbuf);
        }

      outfile << std::endl << "Gradientwise_check_artifacts\t"
              << std::setw(10) << "GradientNum\t" << std::setw(10)
              << "AngleX\t" << std::setw(10) << "AngleY\t"
              << std::setw(10) << "AngleZ\t" << std::setw(10)
              << "TranslationX\t" << std::setw(10) << "TranslationY\t"
              << std::setw(10) << "TranslationZ\t" << std::setw(10)
              << "Metric(MI)" << std::endl;
      for( unsigned int i = 0; i < this->ResultsContainer.size(); i++ )
        {
        if( !this->qcResults[i] )
          {
          outfile.precision(6);
          outfile.setf(std::ios_base::showpoint | std::ios_base::right);
          outfile << "Gradientwise_check_artifacts\t" << std::setw(10) << i;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].AngleX;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].AngleY;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].AngleZ;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].TranslationX;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].TranslationY;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].TranslationZ;
          outfile << "\t" << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right);
          outfile << this->ResultsContainer[i].MutualInformation;
          outfile << std::endl;
          outfile.precision();
          outfile.setf(std::ios_base::unitbuf);
          }
        }

      outfile <<  std::endl << "Post_gradientwise\tbValueLeftNumber "
              << bValueLeftNumber    << std::endl;
      outfile << "Post_gradientwise\tbaselineLeftNumber "
              << baselineLeftNumber  << std::endl;
      outfile << "Post_gradientwise\tgradientDirLeftNumber "
              << gradientDirLeftNumber  << std::endl;

      outfile << std::endl << "Post_gradientwise_diff_grad\tGradientNum "
              << "\tx\ty\tz" << std::setw(34)
              << std::setiosflags(std::ios::left) << "\tIncluded" << std::endl;
      for( unsigned int i = 0; i < this->m_GradientDirectionContainer->size(); i++ )
        {
        outfile << "Post_gradientwise_diff_grad\t" << i << "\t"
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << this->m_GradientDirectionContainer->at(i)[0] << "\t"
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << this->m_GradientDirectionContainer->at(i)[1] << "\t"
                << std::setw(9) << std::setiosflags(std::ios::fixed)
                << std::setprecision(6) << std::setiosflags(std::ios::right)
                << this->m_GradientDirectionContainer->at(i)[2]
                << "\t" << this->qcResults[i] << std::endl;
        }

      outfile << std::endl << "Post_gradientwise_grad_dir_histogram\tGradientNum "
              << "\tx\ty\tz" << std::setw(34)
              << std::setiosflags(std::ios::left) << "\tRepLeft" << std::endl;
      for( unsigned int i = 0; i < this->DiffusionDirHistOutput.size(); i++ )
        {
        if( m_ReportFileName.length() > 0 )
          {
          outfile << "Post_gradientwise_grad_dir_histogram\t" << i << "\t"
                  << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right)
                  << DiffusionDirHistOutput[i].gradientDir[0] << "\t"
                  << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right)
                  << DiffusionDirHistOutput[i].gradientDir[1] << "\t"
                  << std::setw(9) << std::setiosflags(std::ios::fixed)
                  << std::setprecision(6) << std::setiosflags(std::ios::right)
                  << DiffusionDirHistOutput[i].gradientDir[2]
                  << "\t" << DiffusionDirHistOutput[i].repetitionNumber
                  << std::endl;
          }
        }

      break;

    case DWIQCGradientChecker::REPORT_TYPE_NO:
    default:
      break;
    }

  outfile.close();

  return;
}

/**
*
*/
template <class TImageType>
void
DWIQCGradientChecker<TImageType>
::collectLeftDiffusionStatistics()
{
  this->DiffusionDirHistOutput.clear();
  this->repetitionLeftNumber.clear();
  for( unsigned int i = 0; i < this->m_GradientDirectionContainer->size(); i++ )
    {
    if( DiffusionDirHistOutput.size() > 0 )
      {
      bool newDir = true;
      for( unsigned int j = 0; j < DiffusionDirHistOutput.size(); j++ )
        {
        if( this->m_GradientDirectionContainer->ElementAt(i)[0] ==
            DiffusionDirHistOutput[j].gradientDir[0]
            && this->m_GradientDirectionContainer->ElementAt(i)[1] ==
            DiffusionDirHistOutput[j].gradientDir[1]
            && this->m_GradientDirectionContainer->ElementAt(i)[2] ==
            DiffusionDirHistOutput[j].gradientDir[2] )
          {
          if( qcResults[i] )
            {
            DiffusionDirHistOutput[j].repetitionNumber++;
            }
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
        if( this->qcResults[i] )
          {
          diffusionDir.repetitionNumber = 1;
          }
        else
          {
          diffusionDir.repetitionNumber = 0;
          }

        DiffusionDirHistOutput.push_back(diffusionDir);
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
      if( this->qcResults[i] )
        {
        diffusionDir.repetitionNumber = 1;
        }
      else
        {
        diffusionDir.repetitionNumber = 0;
        }

      DiffusionDirHistOutput.push_back(diffusionDir);
      }
    }

  std::vector<double> dirNorm;
  dirNorm.clear();

  this->baselineLeftNumber = 0;
  this->gradientLeftNumber = 0;
  // double modeTemp = 0.0;
  for( unsigned int i = 0; i < DiffusionDirHistOutput.size(); i++ )
    {
    if( DiffusionDirHistOutput[i].gradientDir[0] == 0.0
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
      if( dirNorm.size() > 0 )
        {
        bool newDirMode = true;
        for( unsigned int j = 0; j < dirNorm.size(); j++ )
          {
          if( fabs(normSqr - dirNorm[j]) < 0.001 )    // 1 DIFFERENCE for b value
            {
            newDirMode = false;
            break;
            }
          }
        if( newDirMode && DiffusionDirHistOutput[i].repetitionNumber > 0 )
          {
          dirNorm.push_back(  normSqr);
          //           std::cout<<" if(newDirMode) dirMode.size(): " <<
          //  dirMode.size() <<std::endl;
          }
        }
      else
        {
        if( DiffusionDirHistOutput[i].repetitionNumber > 0 )
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
  for( unsigned int i = 0; i < this->repetitionLeftNumber.size(); i++ )
    {
    this->gradientLeftNumber += this->repetitionLeftNumber[i];
    if( this->repetitionLeftNumber[i] > 0 )
      {
      this->gradientDirLeftNumber++;
      }
    }

  this->bValueLeftNumber = dirNorm.size();

  //     std::ofstream outfile;
  //     if(reportFilename.length()>0)
  //     {
  //       outfile.open(reportFilename.c_str(), std::ios::app);
  //       outfile<<"====="<<std::endl;
  //       outfile<<"Left Diffusion Gradient information:"<<std::endl;
  //       outfile<<"\tbValueLeftNumber: "    <<bValueLeftNumber    <<std::endl;
  //       outfile<<"\tbaselineLeftNumber: "  <<baselineLeftNumber  <<std::endl;
  //       outfile<<"\tgradientDirLeftNumber: "<<gradientDirLeftNumber
  //  <<std::endl;
  //     }
  //
  //     std::cout<<"Left Diffusion Gradient information:"<<std::endl;
  //     std::cout<<"\tbValueLeftNumber: "     <<bValueLeftNumber
  //    <<std::endl;
  //     std::cout<<"\tbaselineLeftNumber: "     <<baselineLeftNumber
  //  <<std::endl;
  //     std::cout<<"\tgradientDirLeftNumber: " <<gradientDirLeftNumber
  //  <<std::endl;
  //
  //     std::cout<<std::endl<<"\t#
  // "<<"\t"<<std::setw(34)<<"DirVector"<<"\tIncluded"<<std::endl;
  //     outfile  <<std::endl<<"\t#
  // "<<"\t"<<std::setw(34)<<"DirVector"<<"\tIncluded"<<std::endl;
  //     for(unsigned int i=0;i< this->m_GradientDirectionContainer->size();i++)
  //     {
  //       outfile<<"\t"<<i<<"\t[ "
  //         <<std::setw(9)<<std::setiosflags(std::ios::fixed)<<
  // std::setprecision(6)<<std::setiosflags(std::ios::right)
  //         <<this->m_GradientDirectionContainer->at(i)[0]<<", "
  //         <<std::setw(9)<<std::setiosflags(std::ios::fixed)<<
  // std::setprecision(6)<<std::setiosflags(std::ios::right)
  //         <<this->m_GradientDirectionContainer->at(i)[1]<<", "
  //         <<std::setw(9)<<std::setiosflags(std::ios::fixed)<<
  // std::setprecision(6)<<std::setiosflags(std::ios::right)
  //         <<this->m_GradientDirectionContainer->at(i)[2]<<" ]: "
  //         <<"\t"<<this->qcResults[i]<<std::endl;
  //       std::cout<<"\t"<<i<<"\t[ "
  //         <<std::setw(9)<<std::setiosflags(std::ios::fixed)<<
  // std::setprecision(6)<<std::setiosflags(std::ios::right)
  //         <<this->m_GradientDirectionContainer->at(i)[0]<<", "
  //         <<std::setw(9)<<std::setiosflags(std::ios::fixed)<<
  // std::setprecision(6)<<std::setiosflags(std::ios::right)
  //         <<this->m_GradientDirectionContainer->at(i)[1]<<", "
  //         <<std::setw(9)<<std::setiosflags(std::ios::fixed)<<
  // std::setprecision(6)<<std::setiosflags(std::ios::right)
  //         <<this->m_GradientDirectionContainer->at(i)[2]<<" ]: "
  //         <<"\t"<<this->qcResults[i]<<std::endl;
  //     }
  //
  //     std::cout<<std::endl<<"Left Gradient Direction Histogram: "<<std::endl;
  //     std::cout<<"\t#
  // "<<"\t"<<std::setw(34)<<"DirVector"<<"\tRepLeft#"<<std::endl;
  //     outfile  <<std::endl<<"Left Gradient Direction Histogram: "<<std::endl;
  //     outfile  <<"\t#
  // "<<"\t"<<std::setw(34)<<"DirVector"<<"\tRepLeft#"<<std::endl;
  //     for(unsigned int i=0;i< DiffusionDirHistOutput.size();i++)
  //     {
  //       std::cout<<"\t"<<i<<"\t[ "
  //         <<std::setw(9)<<std::setiosflags(std::ios::fixed)<<
  // std::setprecision(6)<<std::setiosflags(std::ios::right)
  //         <<DiffusionDirHistOutput[i].gradientDir[0]<<", "
  //         <<std::setw(9)<<std::setiosflags(std::ios::fixed)<<
  // std::setprecision(6)<<std::setiosflags(std::ios::right)
  //         <<DiffusionDirHistOutput[i].gradientDir[1]<<", "
  //         <<std::setw(9)<<std::setiosflags(std::ios::fixed)<<
  // std::setprecision(6)<<std::setiosflags(std::ios::right)
  //         <<DiffusionDirHistOutput[i].gradientDir[2]<<"] "
  //         <<"\t"<<DiffusionDirHistOutput[i].repetitionNumber <<std::endl;
  //
  //       if(reportFilename.length()>0)
  //         outfile<<"\t"<<i<<"\t[ "
  //         <<std::setw(9)<<std::setiosflags(std::ios::fixed)<<
  // std::setprecision(6)<<std::setiosflags(std::ios::right)
  //         <<DiffusionDirHistOutput[i].gradientDir[0]<<", "
  //         <<std::setw(9)<<std::setiosflags(std::ios::fixed)<<
  // std::setprecision(6)<<std::setiosflags(std::ios::right)
  //         <<DiffusionDirHistOutput[i].gradientDir[1]<<", "
  //         <<std::setw(9)<<std::setiosflags(std::ios::fixed)<<
  // std::setprecision(6)<<std::setiosflags(std::ios::right)
  //         <<DiffusionDirHistOutput[i].gradientDir[2]<<"] "
  //         <<"\t"<<DiffusionDirHistOutput[i].repetitionNumber <<std::endl;
  //     }
  //
  //     outfile.close();

  return;
}

/**
*
*/
template <class TImageType>
void
DWIQCGradientChecker<TImageType>
::collectDiffusionStatistics()
{
  std::vector<struDiffusionDir> DiffusionDirections;
  DiffusionDirections.clear();
  for( unsigned int i = 0; i < this->m_GradientDirectionContainer->size(); i++ )
    {
    if( DiffusionDirections.size() > 0 )
      {
      bool newDir = true;
      for( unsigned int j = 0; j < DiffusionDirections.size(); j++ )
        {
        if( this->m_GradientDirectionContainer->ElementAt(i)[0] ==
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
      if( newDir )
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
  for( unsigned int i = 0; i < DiffusionDirections.size(); i++ )
    {
    if( DiffusionDirections[i].gradientDir[0] == 0.0
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
      if( dirNorm.size() > 0 )
        {
        bool newDirMode = true;
        for( unsigned int j = 0; j < dirNorm.size(); j++ )
          {
          if( fabs(normSqr - dirNorm[j]) < 0.001 )    // 1 DIFFERENCE for b value
            {
            newDirMode = false;
            break;
            }
          }
        if( newDirMode )
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
  for( unsigned int i = 1; i < repetNum.size(); i++ )
    {
    if( repetNum[i] != repetNum[0] )
      {
      // std::cout
      //  << "Warning: Not all the gradient directions have same repetition. "
      // << "GradientNumber= " << i << " " << repetNum[i] << " != " << repetNum[0]
      //  << std::endl;
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
DWIQCGradientChecker<TImageType>
::parseGradientDirections()
{
  InputImageConstPointer inputPtr = this->GetInput();

  itk::MetaDataDictionary imgMetaDictionary = inputPtr->GetMetaDataDictionary();        //

  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string                              metaString;

  m_GradientDirectionContainer = GradientDirectionContainerType::New();
  GradientDirectionType vect3d;
  for( ; itKey != imgMetaKeys.end(); itKey++ )
    {
    itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
    // std::cout<<*itKey<<"  "<<metaString<<std::endl;

    if( itKey->find("DWMRI_gradient") != std::string::npos )
      {
      std::istringstream iss(metaString);
      iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
      m_GradientDirectionContainer->push_back(vect3d);
      }
    else if( itKey->find("DWMRI_b-value") != std::string::npos )
      {
      b0 = atof( metaString.c_str() );
      }
    }

  if( b0 < 0.0 )
    {
    std::cout << "BValue not specified in header file" << std::endl;
    return;
    }
  for( unsigned int i = 0; i < this->m_GradientDirectionContainer->Size(); i++ )
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

  if( m_GradientDirectionContainer->size() <= 6 )
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
DWIQCGradientChecker<TImageType>
::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                       ThreadIdType threadId )
{
  itkDebugMacro(<< "Actually executing");

  // Get the input and output pointers
  InputImageConstPointer inputPtr = this->GetInput();
  OutputImagePointer     outputPtr = this->GetOutput();

  int gradientLeft = 0;
  gradientLeft = this->baselineLeftNumber + this->gradientLeftNumber;
  if( gradientLeft == 0 )
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
  typename TImageType::IndexType inputIndex;

  // support progress methods/callbacks
  ProgressReporter progress( this, threadId,
                             outputRegionForThread.GetNumberOfPixels() );

  typename TImageType::PixelType value;
  value.SetSize( gradientLeft );

  // walk the output region, and sample the input image
  while( !outIt.IsAtEnd() )
    {
    // determine the index of the output pixel
    outputIndex = outIt.GetIndex();

    // determine the input pixel location associated with this output pixel
    inputIndex = outputIndex;

    int element = 0;
    for( unsigned int i = 0; i < this->qcResults.size(); i++ )
      {
      if( m_ExcludeGradientsWithLargeMotionArtifacts )
        {
        if( this->qcResults[i] )
          {
          value.SetElement( element, inputPtr->GetPixel(inputIndex)[i] );
          element++;
          }
        }
      else
        {
        value.SetElement( i, inputPtr->GetPixel(inputIndex)[i] );
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
DWIQCGradientChecker<TImageType>
::GetExcludedGradients()
{
  if( GetCheckDone() )
    {
    // Get the input and output pointers
    InputImageConstPointer inputPtr = this->GetInput();
    OutputImagePointer     outputPtr = this->GetOutput();

    unsigned int gradientLeft = this->baselineLeftNumber + this->gradientLeftNumber;
    if( gradientLeft == inputPtr->GetVectorLength() )
      {
      std::cout << "No gradient excluded" << std::endl;
      return NULL;
      }

    // Define/declare an iterator that will walk the output region for this
    // thread.
    // meta data
    itk::MetaDataDictionary outputMetaDictionary;

    itk::MetaDataDictionary imgMetaDictionary
      = inputPtr->GetMetaDataDictionary();
    std::vector<std::string> imgMetaKeys
      = imgMetaDictionary.GetKeys();

    std::string metaString;

    //  measurement frame
    vnl_matrix_fixed<double, 3, 3> mf;
    if( imgMetaDictionary.HasKey("NRRD_measurement frame") )
      {

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
    if( imgMetaDictionary.HasKey("modality") )
      {
      itk::ExposeMetaData(imgMetaDictionary, "modality", metaString);
      itk::EncapsulateMetaData<std::string>( outputMetaDictionary,
                                             "modality",
                                             metaString);
      }

    //     // thickness
    //     if(imgMetaDictionary.HasKey("NRRD_thicknesses[2]"))
    //     {
    //       double thickness;
    //       itk::ExposeMetaData<double>(imgMetaDictionary, "NRRD_thickness[2]",
    // thickness);
    //       itk::EncapsulateMetaData<double>( outputMetaDictionary,
    // "NRRD_thickness[2]", thickness);
    //     }

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

    // gradient vectors
    int temp = 0;
    for( unsigned int i = 0;
         i < this->m_GradientDirectionContainer->Size();
         i++ )
      {
      if( !this->qcResults[i] )
        {
        std::ostringstream ossKey;
        ossKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0')
               << temp;

        std::ostringstream ossMetaString;
        ossMetaString << std::setprecision(17)
                      << this->m_GradientDirectionContainer->ElementAt(i)[0]
                      << "    "
                      << this->m_GradientDirectionContainer->ElementAt(i)[1]
                      << "    "
                      << this->m_GradientDirectionContainer->ElementAt(i)[2];

        // std::cout<<ossKey.str()<<ossMetaString.str()<<std::endl;
        itk::EncapsulateMetaData<std::string>( outputMetaDictionary, ossKey.str(
                                                 ), ossMetaString.str() );
        ++temp;
        }
      }
    excludedDwiImage = TImageType::New();
    excludedDwiImage->CopyInformation(inputPtr);
    excludedDwiImage->SetRegions( inputPtr->GetLargestPossibleRegion() );
    excludedDwiImage->SetVectorLength(inputPtr->GetVectorLength() - gradientLeft);
    excludedDwiImage->Allocate();
    excludedDwiImage->SetMetaDataDictionary(outputMetaDictionary);      //

    typedef ImageRegionIteratorWithIndex<TImageType> OutputIterator;
    OutputIterator outIt( excludedDwiImage,
                          excludedDwiImage->GetLargestPossibleRegion() );

    // Define a few indices that will be used to translate from an input pixel
    // to an output pixel

    typename TImageType::PixelType value;
    value.SetSize( inputPtr->GetVectorLength() - gradientLeft );

    // walk the output region, and sample the input image
    while( !outIt.IsAtEnd() )
      {
      // determine the index of the output pixel
      const typename TImageType::IndexType outputIndex = outIt.GetIndex();
      // determine the input pixel location associated with this output pixel
      const typename TImageType::IndexType inputIndex = outputIndex;

      int element = 0;
      for( unsigned int i = 0; i < this->qcResults.size(); i++ )
        {
        if( !this->qcResults[i] )
          {
          value.SetElement( element, inputPtr->GetPixel(inputIndex)[i] );
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
