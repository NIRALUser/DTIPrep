/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkGradientSteepestDescentBaseOptimizer.cxx,v $
  Language:  C++
  Date:      $Date: 2009-11-24 12:27:56 $
  Version:   $Revision: 1.3 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkGradientSteepestDescentBaseOptimizer_txx
#define _itkGradientSteepestDescentBaseOptimizer_txx

#include "itkGradientSteepestDescentBaseOptimizer.h"
#include "itkSingleValuedNonLinearOptimizer.h"
#include "itkCommand.h"
#include "itkEventObject.h"
#include "vnl/vnl_math.h"

namespace itk
{
/**
 * Constructor
 */
GradientSteepestDescentBaseOptimizer
::GradientSteepestDescentBaseOptimizer()
{
  itkDebugMacro("Constructor");
  m_MaximumStepLength = 1.0;
  m_MinimumStepLength = 1e-3;
  m_GradientMagnitudeTolerance = 1e-4;
  m_NumberOfIterations = 100;
  m_CurrentIteration   =   0;
  m_Value = 0;
  m_PreviousValue = 0;  // add by Ran
  m_BestValue = 0.0;    // add by Ran

  m_Maximize = false;
  m_CostFunction = 0;
  m_CurrentStepLength   =   0;
  m_StopCondition = MaximumNumberOfIterations;
  m_Gradient.Fill( 0.0f );
  m_PreviousGradient.Fill( 0.0f );
  m_RelaxationFactor = 0.5;
}

/**
 * Start the optimization
 */
void
GradientSteepestDescentBaseOptimizer
::StartOptimization( void )
{
  itkDebugMacro("StartOptimization");

  m_CurrentStepLength         = m_MaximumStepLength;
  m_CurrentIteration          = 0;

  const unsigned int spaceDimension
    = m_CostFunction->GetNumberOfParameters();

  m_Gradient = DerivativeType( spaceDimension );
  m_PreviousGradient = DerivativeType( spaceDimension );
  m_Position = ParametersType( spaceDimension );          // add by Ran
  m_PreviousPosition = ParametersType( spaceDimension );  // add by Ran
  m_BestPosition = ParametersType( spaceDimension );      // add by Ran
  m_Gradient.Fill( 0.0f );
  m_PreviousGradient.Fill( 0.0f );
  m_PreviousPosition.Fill( 0.0f );  // add by Ran
  m_Position.Fill( 0.0f );          // add by Ran
  m_BestPosition.Fill( 0.0f );      // add by Ran
  this->SetCurrentPosition( GetInitialPosition() );
  this->ResumeOptimization();
}

/**
 * Resume the optimization
 */
void
GradientSteepestDescentBaseOptimizer
::ResumeOptimization( void )
{
  itkDebugMacro("ResumeOptimization");

  m_Stop = false;

  this->InvokeEvent( itk::StartEvent() );

  while( !m_Stop )
    {
    m_PreviousGradient = m_Gradient;
    m_PreviousValue = m_Value;  // add by Ran

    const unsigned int spaceDimension
      = m_CostFunction->GetNumberOfParameters();

    ParametersType currentPosition = this->GetCurrentPosition();
    for( unsigned int j = 0; j < spaceDimension; j++ )
      {
      m_PreviousPosition[j] = currentPosition[j];
      }

    if( m_Stop )
      {
      break;
      }

    try
      {
      m_CostFunction->GetValueAndDerivative(
        this->GetCurrentPosition(), m_Value, m_Gradient );
      }
    catch( itk::ExceptionObject & excp )
      {
      m_StopCondition = CostFunctionError;
      this->StopOptimization();
      throw excp;
      }

    if( m_Stop )
      {
      break;
      }

    this->AdvanceOneStep();

    m_CurrentIteration++;

    if( m_CurrentIteration == m_NumberOfIterations )
      {
      m_StopCondition = MaximumNumberOfIterations;
      this->StopOptimization();
      break;
      }
    }
}

/**
 * Stop optimization
 */
void
GradientSteepestDescentBaseOptimizer
::StopOptimization( void )
{
  itkDebugMacro("StopOptimization");

  m_Stop = true;
  this->InvokeEvent( itk::EndEvent() );
}

/**
 * Advance one Step following the gradient direction
 */
void
GradientSteepestDescentBaseOptimizer
::AdvanceOneStep( void )
{
  itkDebugMacro("AdvanceOneStep");

  const unsigned int spaceDimension
    = m_CostFunction->GetNumberOfParameters();

  DerivativeType transformedGradient( spaceDimension );
  DerivativeType previousTransformedGradient( spaceDimension );
  ScalesType     scales = this->GetScales();

  if( m_RelaxationFactor < 0.0 )
    {
    itkExceptionMacro(
      << "Relaxation factor must be positive. Current value is "
      << m_RelaxationFactor );
    return;
    }

  if( m_RelaxationFactor >= 1.0 )
    {
    itkExceptionMacro(
      << "Relaxation factor must less than 1.0. Current value is "
      << m_RelaxationFactor );
    return;
    }

  // Make sure the scales have been set properly
  if( scales.size() != spaceDimension )
    {
    itkExceptionMacro(<< "The size of Scales is "
                      << scales.size()
                      <<
                      ", but the NumberOfParameters for the CostFunction is "
                      << spaceDimension
                      << ".");
    }
  for( unsigned int i = 0;  i < spaceDimension; i++ )
    {
    transformedGradient[i]  = m_Gradient[i] / scales[i];
    previousTransformedGradient[i]
      = m_PreviousGradient[i] / scales[i];
    }

  double magnitudeSquare = 0;
  for( unsigned int dim = 0; dim < spaceDimension; dim++ )
    {
    const double weighted = transformedGradient[dim];
    magnitudeSquare += weighted * weighted;
    }

  const double gradientMagnitude = vcl_sqrt( magnitudeSquare );

  if( gradientMagnitude < m_GradientMagnitudeTolerance )
    {
    m_StopCondition = GradientMagnitudeTolerance;
    this->StopOptimization();
    return;
    }

  if( this->m_Maximize )
    {
    if( m_BestValue < m_Value )
      {
      m_BestValue = m_Value;
      m_Position = this->GetCurrentPosition();
      for( unsigned int i = 0; i < spaceDimension; i++ )
        {
        m_BestPosition[i] = m_Position[i];
        }
      }
    else
      {
      m_CurrentStepLength *= m_RelaxationFactor;
      this->SetCurrentPosition( m_BestPosition );
      }
    }
  else
    {
    if( m_BestValue > m_Value )
      {
      m_BestValue = m_Value;
      m_Position = this->GetCurrentPosition();
      for( unsigned int i = 0; i < spaceDimension; i++ )
        {
        m_BestPosition[i] = m_Position[i];
        }
      }
    else
      {
      m_CurrentStepLength *= m_RelaxationFactor;
      this->SetCurrentPosition( m_BestPosition );
      }
    }

  if( m_CurrentStepLength < m_MinimumStepLength )
    {
    m_StopCondition = StepTooSmall;
    this->StopOptimization();
    return;
    }

  double direction;
  if( this->m_Maximize )
    {
    direction = 1.0;
    }
  else
    {
    direction = -1.0;
    }

  const double factor
    = direction * m_CurrentStepLength / gradientMagnitude;

  // This method StepAlongGradient() will
  // be overloaded in non-vector spaces
  this->StepAlongGradient( factor, transformedGradient );

  this->InvokeEvent( itk::IterationEvent() );
}

void
GradientSteepestDescentBaseOptimizer
::PrintSelf( std::ostream & os, itk::Indent indent ) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "MaximumStepLength: "
     << m_MaximumStepLength << std::endl;
  os << indent << "MinimumStepLength: "
     << m_MinimumStepLength << std::endl;
  os << indent << "RelaxationFactor: "
     << m_RelaxationFactor << std::endl;
  os << indent << "GradientMagnitudeTolerance: "
     << m_GradientMagnitudeTolerance << std::endl;
  os << indent << "NumberOfIterations: "
     << m_NumberOfIterations << std::endl;
  os << indent << "CurrentIteration: "
     << m_CurrentIteration   << std::endl;
  os << indent << "Value: "
     << m_Value << std::endl;
  os << indent << "Maximize: "
     << m_Maximize << std::endl;
  if( m_CostFunction )
    {
    os << indent << "CostFunction: "
       << &m_CostFunction << std::endl;
    }
  else
    {
    os << indent << "CostFunction: "
       << "(None)" << std::endl;
    }
  os << indent << "CurrentStepLength: "
     << m_CurrentStepLength << std::endl;
  os << indent << "StopCondition: "
     << m_StopCondition << std::endl;
  os << indent << "Gradient: "
     << m_Gradient << std::endl;
}

} // end namespace itk

#endif
