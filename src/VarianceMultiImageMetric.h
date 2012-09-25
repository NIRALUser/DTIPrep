/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkMutualInformationCongealingMetric.h,v $
  Language:  C++
  Date:      $Date: 2005/10/03 15:18:45 $
  Version:   $Revision: 1.39 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __VarianceMultiImageMetric_h
#define __VarianceMultiImageMetric_h

// user defined headers
#include "UnivariateEntropyMultiImageMetric.h"

namespace itk
{
/** \class VarianceMultiImageMetric
 * \brief Computes sum of variances along pixel stacks
 *
 * VarianceMultiImageMetric computes sum of variances
 * along pixel stacks. This corresponds to registering images
 * to a mean template image.
 *
 * This class is templated over the Image type.
 *
 * The images are set via methods SetImageArray(int, image.
 * This metric makes use of user specified Transform and
 * Interpolator arrays. The Transform is used to map points from a given fixed region to
 * the domain of each image. The Interpolator is used to evaluate the image
 * intensity at user specified geometric points.
 * The Transform and Interpolator arrays are set via methods SetTransformArray(int, transform) and
 * SetInterpolatorArray(int, interpolator).
 *
 * \warning This metric assumes that the images has already been
 * connected to the interpolator outside of this class.
 * MultiResolutionMultiImageRegistrationMethod can be used to handle the connections.
 *
 * The method GetValue() computes of the sum of variances
 * while method GetValueAndDerivative() computes
 * both the sum of univariate entropies and its derivatives with respect to the
 * transform parameters.
 *
 *
 *
 * A stochastic function evaluation is used to increase the computational efficiency.
 * At each iteration a random sample set is drawn from the image
 * and the objective function is evaluated on that sample set.
 * By default 100 samples points are used in each set.
 * Other values can be set via the SetNumberOfSpatialSamples() method.
 *
 *
 * \ingroup RegistrationMetrics
 */
template <class TImage>
class ITK_EXPORT VarianceMultiImageMetric :
  public UnivariateEntropyMultiImageMetric<TImage>
{
public:

  /** Standard class typedefs. */
  typedef VarianceMultiImageMetric                  Self;
  typedef UnivariateEntropyMultiImageMetric<TImage> Superclass;
  typedef SmartPointer<Self>                        Pointer;
  typedef SmartPointer<const Self>                  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(VarianceMultiImageMetric, UnivariateEntropyMultiImageMetric);

  /** Types inherited from Superclass. */
  typedef typename Superclass::TransformType         TransformType;
  typedef typename Superclass::TransformPointer      TransformPointer;
  typedef typename Superclass::TransformJacobianType TransformJacobianType;
  typedef typename Superclass::InterpolatorType      InterpolatorType;
  typedef typename Superclass::MeasureType           MeasureType;
  typedef typename Superclass::DerivativeType        DerivativeType;
  typedef typename Superclass::ParametersType        ParametersType;
  typedef typename Superclass::ParametersArray       ParametersArray;
  typedef typename Superclass::ImageType             ImageType;
  typedef typename Superclass::ImageConstPointer     ImageConstPointer;
  typedef typename Superclass::PixelType             ImagePixelType;
  typedef typename Superclass::RealType              RealType;
  typedef typename Superclass::ImagePointType        ImagePointType;
  struct ThreadStruct
    {
    ConstPointer Metric;
    };

  /** Initialize the Metric by making sure that all the components
   *  are present and plugged together correctly     */
  virtual void Initialize(void)
  throw ( ExceptionObject );

  /**  Get the metric value. */
  MeasureType GetValue( const ParametersType& parameters ) const;

  /**  Get the value and derivatives for single valued optimizers. */
  void GetValueAndDerivative( const ParametersType& parameters, MeasureType& Value, DerivativeType& Derivative ) const;

protected:
  VarianceMultiImageMetric();
  virtual ~VarianceMultiImageMetric()
  {
  };

  /** static members for multi-thread support */
  static ITK_THREAD_RETURN_TYPE ThreaderCallbackGetValueAndDerivative( void *arg );

  static ITK_THREAD_RETURN_TYPE ThreaderCallbackGetValue( void *arg );

  /** Methods added for supporting multi-threading GetValue */
  void GetThreadedValue( int threadID ) const;

  MeasureType AfterGetThreadedValue() const;

  /** Methods added for supporting multi-threading GetValueAndDerivative */
  void GetThreadedValueAndDerivative( int threadID ) const;

  void AfterGetThreadedValueAndDerivative(MeasureType & value, DerivativeType & derivative) const;

private:
  VarianceMultiImageMetric(const Self &); // purposely not implemented
  void operator=(const Self &);           // purposely not implemented

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "VarianceMultiImageMetric.hxx"
#endif

#endif
