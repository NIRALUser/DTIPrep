/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkTensorPrincipalEigenvectorImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2009/08/26 14:24:01 $
  Version:   $Revision: 1.7 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkTensorPrincipalEigenvectorImageFilter_h
#define __itkTensorPrincipalEigenvectorImageFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "itkCovariantVector.h"
#include <vnl/vnl_double_3.h>
#include <vnl/algo/vnl_symmetric_eigensystem.h>

namespace itk
{

// This functor class invokes the computation of fractional anisotropy from
// every pixel.
namespace Functor
{

template <typename TInput, typename VectorPixelValueType>
class TensorPrincipalEigenvectorFunction
{
public:
  typedef typename TInput::RealValueType           RealValueType;
  typedef typename TInput::EigenVectorsMatrixType  EigenVectorsType;
  typedef typename TInput::EigenValuesArrayType    EigenValuesType;
  typedef CovariantVector<VectorPixelValueType, 3> PixelType;

  TensorPrincipalEigenvectorFunction()
  {
  }

  ~TensorPrincipalEigenvectorFunction()
  {
  }

  bool operator!=( const TensorPrincipalEigenvectorFunction & ) const
  {
    return false;
  }

  bool operator==( const TensorPrincipalEigenvectorFunction & other ) const
  {
    return !(*this != other);
  }

  PixelType operator()( const TInput & x ) const
  {
    double lambdas[3];

    vnl_symmetric_eigensystem_compute_eigenvals(x[0], x[1], x[2],
                                                x[3], x[4],
                                                x[5],
                                                lambdas[0],
                                                lambdas[1],
                                                lambdas[2]);

    // Substract eigenvalue * I from x
    TInput y(x);
    y[0] -= lambdas[2];
    y[3] -= lambdas[2];
    y[5] -= lambdas[2];

    // const double epsilon = 1.0e-16;
    // Largest eigenvalue is distinct
    //      if(lambdas[2] - lambdas[1] >= 0)
    //      {
    // Cross-product of any two rows of y
    const vnl_double_3 a(y[0], y[1], y[2]);
    const vnl_double_3 b(y[1], y[3], y[4]);
    const vnl_double_3 c(y[2], y[4], y[5]);
    // Find two largest magnitude vectors as one
    // row may have been nulled by the substraction
    const vnl_double_3 evec = (vnl_cross_3d(a, b)
                               + vnl_cross_3d(a, c)
                               + vnl_cross_3d(b, c) ).normalize();

    return PixelType(evec.data_block() );
    //      }
    // Largest two are duplicated but distinct from smallest
//       else if(lambdas[1] - lambdas[0] > epsilon)
//       {
//       }
    // Space is isotropic
    //     else
    //     {
    //       throw itk::ExceptionObject("Tracked into region with undefined largest eigenvector");
    //     }

  }

};

}  // end namespace functor

/** \class TensorPrincipalEigenvectorImageFilter
 * \brief Computes the Principal Eigenvector on a voxelwise basis
 *
 * \sa TensorRelativeAnisotropyImageFilter
 * \sa TensorFractionalAnisotropyImageFilter
 * \sa DiffusionTensor3D
 *
 * \ingroup IntensityImageFilters  Multithreaded  TensorObjects
 *
 */
template <typename TInputImage,
          typename TOutputImage>
class ITK_EXPORT TensorPrincipalEigenvectorImageFilter :
  public
  UnaryFunctorImageFilter<TInputImage, TOutputImage,
                          Functor::TensorPrincipalEigenvectorFunction<
                            typename TInputImage::PixelType,
                            typename TOutputImage::PixelType::ValueType> >
{
public:
  /** Standard class typedefs. */
  typedef TensorPrincipalEigenvectorImageFilter Self;
  typedef UnaryFunctorImageFilter<TInputImage, TOutputImage,
                                  Functor::TensorPrincipalEigenvectorFunction<
                                    typename TInputImage::PixelType,
                                    typename TOutputImage::PixelType::ValueType> >  Superclass;

  typedef SmartPointer<Self>       Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  typedef typename Superclass::OutputImageType OutputImageType;
  typedef typename TOutputImage::PixelType     OutputPixelType;
  typedef typename TInputImage::PixelType      InputPixelType;
  typedef typename InputPixelType::ValueType   InputValueType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Print internal ivars */
  void PrintSelf(std::ostream& os, Indent indent) const
  {
    this->Superclass::PrintSelf( os, indent );
  }

protected:
  TensorPrincipalEigenvectorImageFilter()
  {
  };
  virtual ~TensorPrincipalEigenvectorImageFilter()
  {
  };
private:
  TensorPrincipalEigenvectorImageFilter(const Self &); // purposely not implemented
  void operator=(const Self &);                        // purposely not implemented

};

} // end namespace itk

#endif
