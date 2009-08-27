/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkGradientSteepestDescentOptimizer.h,v $
  Language:  C++
  Date:      $Date: 2009-08-27 01:35:37 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkGradientSteepestDescentOptimizer_h
#define __itkGradientSteepestDescentOptimizer_h

#include "itkGradientSteepestDescentBaseOptimizer.h"

namespace itk
{
  
/** \class GradientSteepestDescentOptimizer
 * \brief Implement a gradient descent optimizer
 *
 * \ingroup Numerics  Optimizers
 *
 */
class ITK_EXPORT GradientSteepestDescentOptimizer : 
    public GradientSteepestDescentBaseOptimizer
{
public:
  /** Standard class typedefs. */
  typedef GradientSteepestDescentOptimizer         Self;
  typedef GradientSteepestDescentBaseOptimizer     Superclass;
  typedef SmartPointer<Self>                          Pointer;
  typedef SmartPointer<const Self>                    ConstPointer;
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro( GradientSteepestDescentOptimizer, 
                GradientSteepestDescentBaseOptimizer );

  /** Cost function typedefs. */
  typedef Superclass::CostFunctionType        CostFunctionType;
  typedef CostFunctionType::Pointer           CostFunctionPointer;
  

protected:
  GradientSteepestDescentOptimizer() {};
  virtual ~GradientSteepestDescentOptimizer() {};

  /** Advance one step along the corrected gradient taking into
   * account the steplength represented by factor.
   * This method is invoked by AdvanceOneStep. It is expected
   * to be overrided by optimization methods in non-vector spaces
   * \sa AdvanceOneStep */
  virtual void StepAlongGradient( 
    double factor, 
    const DerivativeType & transformedGradient );


private:
  GradientSteepestDescentOptimizer(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};

} // end namespace itk



#endif



