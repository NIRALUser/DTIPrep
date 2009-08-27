/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkGradientSteepestDescentBaseOptimizer.h,v $
  Language:  C++
  Date:      $Date: 2009-08-27 01:35:37 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkGradientSteepestDescentBaseOptimizer_h
#define __itkGradientSteepestDescentBaseOptimizer_h

#include "itkSingleValuedNonLinearOptimizer.h"

namespace itk
{
  
/** \class GradientSteepestDescentBaseOptimizer
 * \brief Implement a gradient descent optimizer
 *
 * \ingroup Numerics Optimizers
 */

class GradientSteepestDescentBaseOptimizer : 
    public SingleValuedNonLinearOptimizer
{
public:
  /** Standard "Self" typedef. */
  typedef GradientSteepestDescentBaseOptimizer      Self;
  typedef itk::SingleValuedNonLinearOptimizer               Superclass;
  typedef itk::SmartPointer<Self>                           Pointer;
  typedef itk::SmartPointer<const Self>                     ConstPointer;
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro( GradientSteepestDescentBaseOptimizer, 
                itk::SingleValuedNonLinearOptimizer );
  

  /** Codes of stopping conditions. */
  typedef enum {
    GradientMagnitudeTolerance = 1,
    StepTooSmall = 2,
    ImageNotAvailable = 3,
    CostFunctionError = 4,
    MaximumNumberOfIterations = 5
  } StopConditionType;

  /** Specify whether to minimize or maximize the cost function. */
  itkSetMacro( Maximize, bool );
  itkGetConstReferenceMacro( Maximize, bool );
  itkBooleanMacro( Maximize );
  bool GetMinimize( ) const
  { return !m_Maximize; }
  void SetMinimize(bool v)
  { this->SetMaximize(!v); }
  void    MinimizeOn(void) 
  { SetMaximize( false ); }
  void    MinimizeOff(void) 
  { SetMaximize( true ); }

  /** Start optimization. */
  void    StartOptimization( void );

  /** Resume previously stopped optimization with current parameters.
   * \sa StopOptimization */
  void    ResumeOptimization( void );

  /** Stop optimization.
   * \sa ResumeOptimization */
  void    StopOptimization( void );

  /** Set/Get parameters to control the optimization process. */
  itkSetMacro( MaximumStepLength, double );
  itkSetMacro( MinimumStepLength, double );
  itkSetMacro( RelaxationFactor, double );
  itkSetMacro( NumberOfIterations, unsigned long );
  itkSetMacro( GradientMagnitudeTolerance, double );
  itkGetConstReferenceMacro( CurrentStepLength, double);
  itkGetConstReferenceMacro( MaximumStepLength, double );
  itkGetConstReferenceMacro( MinimumStepLength, double );
  itkGetConstReferenceMacro( RelaxationFactor, double );
  itkGetConstReferenceMacro( NumberOfIterations, unsigned long );
  itkGetConstReferenceMacro( GradientMagnitudeTolerance, double );
  itkGetConstMacro( CurrentIteration, unsigned int );
  itkGetConstReferenceMacro( StopCondition, StopConditionType );
  itkGetConstReferenceMacro( Value, MeasureType );
  itkGetConstReferenceMacro( Gradient, DerivativeType );
  
  itkGetConstReferenceMacro( BestValue, MeasureType );
  itkGetConstReferenceMacro( BestPosition,  ParametersType );

protected:
  GradientSteepestDescentBaseOptimizer();
  virtual ~GradientSteepestDescentBaseOptimizer() {};
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /** Advance one step following the gradient direction
   * This method verifies if a change in direction is required
   * and if a reduction in steplength is required. */
  virtual void AdvanceOneStep( void );

  /** Advance one step along the corrected gradient taking into
   * account the steplength represented by factor.
   * This method is invoked by AdvanceOneStep. It is expected
   * to be overrided by optimization methods in non-vector spaces
   * \sa AdvanceOneStep */
  virtual void StepAlongGradient( 
    double,
    const DerivativeType&)
  {
    itk::ExceptionObject ex;
    ex.SetLocation(__FILE__);
    ex.SetDescription("This method MUST be overloaded in derived classes");
    throw ex;
  }


private:  
  GradientSteepestDescentBaseOptimizer(const Self&); //purposely not implemented
  void operator=(const Self&);//purposely not implemented

protected:
  DerivativeType                m_Gradient; 
  DerivativeType                m_PreviousGradient; 

  bool                          m_Stop;
  bool                          m_Maximize;
  MeasureType                   m_Value;
  MeasureType                   m_PreviousValue;//add by Ran
  MeasureType                   m_BestValue;//add by Ran
  ParametersType		m_Position;	//add by Ran
  ParametersType		m_PreviousPosition;	//add by Ran
  ParametersType		m_BestPosition;	//add by Ran
  double                        m_GradientMagnitudeTolerance;
  double                        m_MaximumStepLength;
  double                        m_MinimumStepLength;
  double                        m_CurrentStepLength;
  double                        m_RelaxationFactor;
  StopConditionType             m_StopCondition;
  unsigned long                 m_NumberOfIterations;
  unsigned long                 m_CurrentIteration;


};

} // end namespace itk



#endif



