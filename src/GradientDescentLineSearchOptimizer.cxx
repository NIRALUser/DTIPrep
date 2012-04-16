/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkFRPROptimizer.cxx,v $
  Language:  C++
  Date:      $Date: 2006/06/06 18:50:29 $
  Version:   $Revision: 1.8 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _GradientDescentLineSearchOptimizer_cxx
#define _GradientDescentLineSearchOptimizer_cxx

#include "GradientDescentLineSearchOptimizer.h"

namespace itk
{

const double FRPR_TINY = 1e-20;

GradientDescentLineSearchOptimizer
::GradientDescentLineSearchOptimizer()
{
}

GradientDescentLineSearchOptimizer
::~GradientDescentLineSearchOptimizer()
{
}

void
GradientDescentLineSearchOptimizer
::GetValueAndDerivative(ParametersType p, double * val,
                        ParametersType * xi)
{
  this->m_CostFunction->GetValueAndDerivative(p, *val, *xi);
  if( this->GetMaximize() )
    {
    (*val) = -(*val);
    for( unsigned int i = 0; i < this->GetSpaceDimension(); i++ )
      {
      (*xi)[i] = -(*xi)[i];
      }
    }
}

void
GradientDescentLineSearchOptimizer
::LineOptimize(ParametersType * p, ParametersType xi, double * val)
{
  this->SetLine(*p, xi);
  double ax = 0.0;
  double fa = (*val);
  double xx = this->GetStepLength();
  double fx;
  double bx;
  double fb;

  ParametersType pp = (*p);   // ??????

  this->LineBracket(&ax, &xx, &bx, &fa, &fx, &fb);
  this->SetCurrentLinePoint(xx, fx);

  double extX = 0;
  double extVal = 0;

  this->BracketedLineOptimize(ax, xx, bx, fa, fx, fb, &extX, &extVal);
  this->SetCurrentLinePoint(extX, extVal);

  (*p) = this->GetCurrentPosition();
  (*val) = extVal;
}

void
GradientDescentLineSearchOptimizer
::StartOptimization()
{

  if( m_CostFunction.IsNull() )
    {
    return;
    }

  this->InvokeEvent( StartEvent() );
  this->SetStop(false);

  this->SetSpaceDimension(m_CostFunction->GetNumberOfParameters() );

  const unsigned int SpaceDimension = this->GetSpaceDimension();

  GradientDescentLineSearchOptimizer::ParametersType xi( SpaceDimension );

  GradientDescentLineSearchOptimizer::ParametersType p( SpaceDimension );
  p = this->GetInitialPosition();
  // std::cout<<"InitialPosition:"<<p<<std::endl;
  this->SetCurrentPosition(p);

  double fp;
  this->GetValueAndDerivative(p, &fp, &xi);
  for( unsigned int currentIteration = 0;
       currentIteration <= this->GetMaximumIteration();
       currentIteration++ )
    {
    this->SetCurrentIteration(currentIteration);

    double fret;
    fret = fp;
    this->LineOptimize(&p, xi, &fret);
    this->GetValueAndDerivative(p, &fp, &xi);
    this->SetCurrentPosition(p);
    this->InvokeEvent( IterationEvent() );
    }

  this->SetCurrentPosition(p);
  // std::cout<<"CurrentPosition: "<<this->GetCurrentPosition()<<std::endl;
  this->InvokeEvent( EndEvent() );
}

/**
 *
 */
void
GradientDescentLineSearchOptimizer
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

} // end of namespace itk
#endif
