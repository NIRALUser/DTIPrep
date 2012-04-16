/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkFRPROptimizer.h,v $
  Language:  C++
  Date:      $Date: 2006/05/25 13:41:42 $
  Version:   $Revision: 1.2 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __GradientDescentLineSearchOptimizer_h
#define __GradientDescentLineSearchOptimizer_h

#include <itkVector.h>
#include <itkMatrix.h>
#include "itkPowellOptimizer.h"

namespace itk
{

/** \class GradientDescentLineSearchOptimizer
 * \brief Implements gradient descent optimization combined with line search .
 *
 * This optimizer needs a cost function.
 * This optimizer needs to be able to compute partial derivatives of the
 *    cost function with respect to each parameter.
 *
 * The SetStepLength determines the initial distance to step in a line direction
 * when bounding the minimum (using bracketing triple spaced using a
 * derivative-based search strategy).
 *
 * The StepTolerance terminates optimization when the parameter values are
 * known to be within this (scaled) distance of the local extreme.
 *
 * The ValueTolerance terminates optimization when the cost function values at
 * the current parameters and at the local extreme are likely (within a second
 * order approximation) to be within this is tolerance.
 *
 * \ingroup Numerics Optimizers
 *
 */

class ITK_EXPORT GradientDescentLineSearchOptimizer :
  public PowellOptimizer
{
public:
  /** Standard "Self" typedef. */
  typedef GradientDescentLineSearchOptimizer Self;
  typedef PowellOptimizer                    Superclass;
  typedef SmartPointer<Self>                 Pointer;
  typedef SmartPointer<const Self>           ConstPointer;

  typedef SingleValuedNonLinearOptimizer::ParametersType
  ParametersType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(GradientDescentLineSearchOptimizer, PowellOptimizer );

  /** Type of the Cost Function   */
  typedef  SingleValuedCostFunction  CostFunctionType;
  typedef  CostFunctionType::Pointer CostFunctionPointer;

  /** Start optimization. */
  void StartOptimization();

protected:
  GradientDescentLineSearchOptimizer();
  virtual ~GradientDescentLineSearchOptimizer();

  void PrintSelf(std::ostream& os, Indent indent) const;

  virtual void GetValueAndDerivative(ParametersType p, double * val, ParametersType * xi);

  virtual void   LineOptimize(ParametersType * p, ParametersType xi, double * val );

private:
  GradientDescentLineSearchOptimizer(const GradientDescentLineSearchOptimizer &); // not implemented

};  // end of class

} // end of namespace itk

#endif
