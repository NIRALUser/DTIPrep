/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkMultiResolutionMultiImageRegistrationMethod.txx,v $
  Language:  C++
  Date:      $Date: 2006/03/19 04:36:55 $
  Version:   $Revision: 1.13 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _MultiResolutionMultiImageRegistrationMethod_cxx
#define _MultiResolutionMultiImageRegistrationMethod_cxx

#include "MultiResolutionMultiImageRegistrationMethod.h"
#include "itkRecursiveMultiResolutionPyramidImageFilter.h"

namespace itk
{

/*
 * Constructor
 */
template <typename ImageType>
MultiResolutionMultiImageRegistrationMethod<ImageType>
::MultiResolutionMultiImageRegistrationMethod()
{

  m_TransformArray.resize(0);
  m_InterpolatorArray.resize(0);
  m_Metric       = 0; // has to be provided by the user.
  m_Optimizer    = 0; // has to be provided by the user.
  m_NumberOfImages = 0;

  m_NumberOfLevels = 1;
  m_CurrentLevel = 0;

  m_Stop = false;

  m_InitialTransformParameters = ParametersType(1);
  m_InitialTransformParametersOfNextLevel = ParametersType(1);
  m_LastTransformParameters = ParametersType(1);

  m_InitialTransformParameters.Fill( 0.0f );
  m_InitialTransformParametersOfNextLevel.Fill( 0.0f );
  m_LastTransformParameters.Fill( 0.0f );

}

template <typename ImageType>
void MultiResolutionMultiImageRegistrationMethod<ImageType>
::SetNumberOfImages(int N)
{

  m_TransformArray.resize(N);
  m_InterpolatorArray.resize(N);
  m_ImagePyramidArray.resize(N);
  m_ImageMaskArray.resize(N);
  for( int i = m_NumberOfImages; i < N; i++ )
    {
    m_TransformArray[i] = 0;
    m_InterpolatorArray[i] = 0;
    m_ImagePyramidArray[i] = 0;
    m_ImageMaskArray[i] = 0;
    }

  m_NumberOfImages  = N;

}

/*
 * Initialize by setting the interconnects between components.
 */
template <typename ImageType>
void
MultiResolutionMultiImageRegistrationMethod<ImageType>
::Initialize()
throw (ExceptionObject)
{
  for( int i = 0; i < m_NumberOfImages; i++ )
    {

    if( !m_TransformArray[i] )
      {
      itkExceptionMacro(<< "Transform " << i << " is not present");
      }

    if( !m_InterpolatorArray[i] )
      {
      itkExceptionMacro(<< "Interpolator " << i << " is not present");
      }

    }

  // Sanity checks
  if( !m_Metric )
    {
    itkExceptionMacro(<< "Metric is not present" );
    }

  if( !m_Optimizer )
    {
    itkExceptionMacro(<< "Optimizer is not present" );
    }

  /* Setup the metric */
  m_Metric->SetNumberOfImages(m_NumberOfImages);
  for( int i = 0; i < m_NumberOfImages; i++ )
    {
    m_Metric->SetImageArray(i, m_ImagePyramidArray[i]->GetOutput(m_CurrentLevel) );
    m_Metric->SetInterpolatorArray(i, m_InterpolatorArray[i] );
    m_Metric->SetTransformArray(i, m_TransformArray[i] );
    // Connect the mask
    if( m_ImageMaskArray[i] )
      {
      m_Metric->SetImageMaskArray(i, m_ImageMaskArray[i] );
      }

    }

  // Setup the metric
  m_Metric->SetFixedImageRegion( m_FixedImageRegionPyramid[m_CurrentLevel] );
  m_Metric->Initialize();

  // Setup the optimizer
  m_Optimizer->SetCostFunction( m_Metric );
  m_Optimizer->SetInitialPosition( m_InitialTransformParametersOfNextLevel );

}

/*
 * Stop the Registration Process
 */
template <typename ImageType>
void
MultiResolutionMultiImageRegistrationMethod<ImageType>
::StopRegistration( void )
{
  m_Stop = true;
}

/*
 * Stop the Registration Process
 */
template <typename ImageType>
void
MultiResolutionMultiImageRegistrationMethod<ImageType>
::PreparePyramids( void )
{
  for( int i = 0; i < m_NumberOfImages; i++ )
    {

    if( !m_TransformArray[i] )
      {
      itkExceptionMacro(<< "Transform " << i << " is not present");
      }

    if( !m_InterpolatorArray[i] )
      {
      itkExceptionMacro(<< "Interpolator " << i << " is not present");
      }
    if( !m_ImagePyramidArray[i] )
      {
      itkExceptionMacro(<< "Image pyramid " << i << "is not present");
      }
    }

  // Assume ImagePyramidArray is allocated correctly
  if( m_NumberOfLevels > m_ImagePyramidArray[0]->GetNumberOfLevels() )
    {
    itkExceptionMacro(
      << "Image Pyramid does not have sufficient levels, increase the number of levels of the pyramid filter");
    }

  m_InitialTransformParametersOfNextLevel = m_InitialTransformParameters;

  if( m_InitialTransformParametersOfNextLevel.Size() !=
      m_TransformArray[0]->GetNumberOfParameters() * m_NumberOfImages )
    {
    itkExceptionMacro(<< "Size mismatch between initial parameter and transform");
    }

  // Assume the image pyramid is present
  typedef typename ImageRegionType::SizeType      SizeType;
  typedef typename ImageRegionType::IndexType     IndexType;
  typedef typename ImagePyramidType::ScheduleType ScheduleType;

  ScheduleType schedule = m_ImagePyramidArray[0]->GetSchedule();

  SizeType  inputSize  = m_FixedImageRegion.GetSize();
  IndexType inputStart = m_FixedImageRegion.GetIndex();

  m_FixedImageRegionPyramid.reserve( m_NumberOfLevels );
  m_FixedImageRegionPyramid.resize( m_NumberOfLevels );
  // Compute the FixedImageRegion corresponding to each level of the
  // pyramid. This uses the same algorithm of the ShrinkImageFilter
  // since the regions should be compatible.
  for( unsigned int level = 0; level < m_ImagePyramidArray[0]->GetNumberOfLevels(); level++ )
    {
    SizeType  size;
    IndexType start;
    for( unsigned int dim = 0; dim < ImageType::ImageDimension; dim++ )
      {
      const float scaleFactor = static_cast<float>( schedule[level][dim] );

      size[dim] = static_cast<typename SizeType::SizeValueType>(
          vcl_floor(static_cast<float>( inputSize[dim] ) / scaleFactor ) );
      if( size[dim] < 1 )
        {
        size[dim] = 1;
        }

      start[dim] = static_cast<typename IndexType::IndexValueType>(
          vcl_ceil(static_cast<float>( inputStart[dim] ) / scaleFactor ) );
      }
    m_FixedImageRegionPyramid[level].SetSize( size );
    m_FixedImageRegionPyramid[level].SetIndex( start );
    }

}

/*
 * Starts the Registration Process
 */
template <typename ImageType>
void
MultiResolutionMultiImageRegistrationMethod<ImageType>
::StartRegistration( void )
{

  m_Stop = false;

  this->PreparePyramids();
  // ImagePyramidArray might have more levels than required
  // there do not necessarily begin from zero
  for( m_CurrentLevel = m_ImagePyramidArray[0]->GetNumberOfLevels() - m_NumberOfLevels;
       m_CurrentLevel < m_ImagePyramidArray[0]->GetNumberOfLevels();
       m_CurrentLevel++ )
    {

    // Check if there has been a stop request
    if( m_Stop )
      {
      break;
      }

    // Invoke an iteration event.
    // This allows a UI to reset any of the components between
    // resolution level.
    m_Metric->SetFixedImageRegion( m_FixedImageRegionPyramid[m_CurrentLevel] );   // To print total number of pixels
    this->InvokeEvent( IterationEvent() );

    try
      {

      // initialize the interconnects between components
      this->Initialize();
      }
    catch( ExceptionObject& err )
      {
      m_LastTransformParameters = ParametersType(1);
      m_LastTransformParameters.Fill( 0.0f );

      // pass exception to caller
      throw err;
      }
    std::cout << "Level: " << m_CurrentLevel << std::endl;
    try
      {
      // do the optimization
      m_Optimizer->StartOptimization();
      }
    catch( ExceptionObject& err )
      {
      // An error has occurred in the optimization.
      // Update the parameters
      m_LastTransformParameters = m_Optimizer->GetCurrentPosition();

      // Pass exception to caller
      throw err;
      }

    // finalize the metric
    m_Metric->Finalize();

    // get the results
    ParametersType current(m_TransformArray[0]->GetNumberOfParameters() );
    m_LastTransformParameters = m_Optimizer->GetCurrentPosition();
    for( int i = 0; i < m_NumberOfImages; i++ )
      {
      for( unsigned int j = 0; j < m_TransformArray[i]->GetNumberOfParameters(); j++ )
        {
        current[j] = m_LastTransformParameters[i * m_TransformArray[i]->GetNumberOfParameters() + j];
        }
      m_TransformArray[i]->SetParametersByValue( current );
      }

    // setup the initial parameters for next level
    if( m_CurrentLevel < m_NumberOfLevels - 1 )
      {
      m_InitialTransformParametersOfNextLevel =
        m_LastTransformParameters;
      }
    }

}

/*
 * PrintSelf
 */
template <typename ImageType>
void
MultiResolutionMultiImageRegistrationMethod<ImageType>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "Metric: " << m_Metric.GetPointer() << std::endl;
  os << indent << "Optimizer: " << m_Optimizer.GetPointer() << std::endl;
  for( int i = 0; i < m_NumberOfImages; i++ )
    {
    os << indent << "Transform: " << i << " " << m_TransformArray[i].GetPointer() << std::endl;
    os << indent << "Interpolator: " << i << " " << m_InterpolatorArray[i].GetPointer() << std::endl;
    os << indent << "FixedImagePyramid: " << i << " ";
    os << m_ImagePyramidArray[i].GetPointer() << std::endl;
    }

  os << indent << "NumberOfLevels: ";
  os << m_NumberOfLevels << std::endl;

  os << indent << "CurrentLevel: ";
  os << m_CurrentLevel << std::endl;

  os << indent << "InitialTransformParameters: ";
  os << m_InitialTransformParameters << std::endl;
  os << indent << "InitialTransformParametersOfNextLevel: ";
  os << m_InitialTransformParametersOfNextLevel << std::endl;
  os << indent << "LastTransformParameters: ";
  os << m_LastTransformParameters << std::endl;
  os << indent << "ImageRegion: ";
  os << m_FixedImageRegion << std::endl;
  for( unsigned int level = 0; level < m_FixedImageRegionPyramid.size(); level++ )
    {
    os << indent << "ImageRegion at level " << level << ": ";
    os << m_FixedImageRegionPyramid[level] << std::endl;
    }

}

/*
 * Set i'th initial transform parameters
 */
template <typename ImageType>
void
MultiResolutionMultiImageRegistrationMethod<ImageType>
::SetInitialTransformParameters( int i, const ParametersType & param )
{
  for( unsigned int j = 0; j < param.Size(); j++ )
    {
    m_InitialTransformParameters[i * param.Size() + j] = param[j];
    }
  this->Modified();
}

/*
 * Set the initial transform parameters
 */
template <typename ImageType>
void
MultiResolutionMultiImageRegistrationMethod<ImageType>
::SetInitialTransformParameters( const ParametersType & param )
{
  m_InitialTransformParameters = param;
  this->Modified();
}

/*
 * Set i'th next transform parameters
 */
template <typename ImageType>
void
MultiResolutionMultiImageRegistrationMethod<ImageType>
::SetInitialTransformParametersOfNextLevel( const ParametersType & param, int i )
{
  for( unsigned int j = 0; j < param.Size(); j++ )
    {
    m_InitialTransformParametersOfNextLevel[i * param.Size() + j] = param[j];
    }
  this->Modified();
}

/*
 * Set the next transform parameters
 */
template <typename ImageType>
void
MultiResolutionMultiImageRegistrationMethod<ImageType>
::SetInitialTransformParametersOfNextLevel( const ParametersType & param )
{

  m_InitialTransformParametersOfNextLevel = param;

  this->Modified();
}

/*
 * Set the length of the parameters vector
 */
template <typename ImageType>
void
MultiResolutionMultiImageRegistrationMethod<ImageType>
::SetTransformParametersLength( int N )
{

  m_InitialTransformParametersOfNextLevel.SetSize(N);
  m_InitialTransformParameters.SetSize(N);
  m_LastTransformParameters.SetSize(N);

  this->Modified();
}

/*
 * Get the length of the parameters vector
 */
template <typename ImageType>
int
MultiResolutionMultiImageRegistrationMethod<ImageType>
::GetTransformParametersLength()
{

  return m_InitialTransformParameters.GetSize();

}

} // end namespace itk

#endif
