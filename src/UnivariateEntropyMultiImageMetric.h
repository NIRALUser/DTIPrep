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
#ifndef __UnivariateEntropyMultiImageMetric_h
#define __UnivariateEntropyMultiImageMetric_h

#include "itkCovariantVector.h"
#include "itkPoint.h"

#include "itkIndex.h"
#include "itkKernelFunction.h"
#include "itkCentralDifferenceImageFunction.h"

#include "itkGradientImageFilter.h"
#include "itkImageRandomNonRepeatingConstIteratorWithIndex.h"

#include "itkImageRegionIterator.h"

// user defined headers
#include <vector>
#include "MultiImageMetric.h"

namespace itk
{

/** \class UnivariateEntropyImageToImageMetric
 * \brief Computes sum of one dimensional entropies along pixel stacks
 *
 * UnivariateEntropyImageToImageMetric computes sum of one dimensional
 * entropies along pixel stacks.
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
 * The method GetValue() computes of the sum of univariate entropies
 * while method GetValueAndDerivative() computes
 * both the sum of univariate entropies and its derivatives with respect to the
 * transform parameters.
 *
 * The calculations are based on the method of Viola and Wells
 * where the probability density distributions are estimated using
 * Parzen windows.
 *
 * By default a Gaussian kernel is used in the density estimation
 *
 * A stochastic function evaluation is used to increase the computational efficiency.
 * At each iteration a random sample set is drawn from the image
 * and the objective function is evaluated on that sample set.
 * By default 100 samples points are used in each set.
 * Other values can be set via the SetNumberOfSpatialSamples() method.
 *
 * Quality of the density estimate depends on the choice of the
 * kernel's standard deviation. Optimal choice will depend on the images.
 * In our experiments, we have found that a
 * standard deviation of 10 percent of the range of the intensity values works well.
 * The variance can be set via method SetImageStandardDeviation().
 *
 *
 * \ingroup RegistrationMetrics
 */
template <class TFixedImage>
class ITK_EXPORT UnivariateEntropyMultiImageMetric :
  public MultiImageMetric<TFixedImage>
{
public:

  /** Standard class typedefs. */
  typedef UnivariateEntropyMultiImageMetric Self;
  typedef MultiImageMetric<TFixedImage>     Superclass;
  typedef SmartPointer<Self>                Pointer;
  typedef SmartPointer<const Self>          ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(UnivariateEntropyMultiImageMetric, MultiImageMetric);

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
  typedef typename Superclass::PixelType             PixelType;
  typedef typename Superclass::RealType              RealType;

  struct ThreadStruct
    {
    ConstPointer Metric;
    };

  /** Index and Point typedef support. */
  typedef typename ImageType::IndexType           ImageIndexType;
  typedef typename ImageIndexType::IndexValueType ImageIndexValueType;
  typedef typename TransformType::InputPointType  ImagePointType;

  /** Enum of the moving image dimension. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      ImageType::ImageDimension);

  /** Get the derivatives of the match measure. */
  void GetDerivative(const ParametersType& parameters, DerivativeType & Derivative ) const;

  /**  Get the metric value. */
  MeasureType GetValue( const ParametersType& parameters ) const;

  /**  Get the value and derivatives for single valued optimizers. */
  void GetValueAndDerivative( const ParametersType& parameters, MeasureType& Value, DerivativeType& Derivative ) const;

  /** Set/Get the moving image intensitiy standard deviation. This defines
   * the kernel bandwidth used in the joint probability distribution
   * calculation. Default value is 10.0 which works well for image intensities
   * having a range of 0-255 */
  itkSetClampMacro( ImageStandardDeviation, double,
                    NumericTraits<double>::NonpositiveMin(), NumericTraits<double>::max() );
  itkGetConstReferenceMacro( ImageStandardDeviation, double );

  /** Initialize the Metric by making sure that all the components
   *  are present and plugged together correctly     */
  virtual void Initialize(void)
  throw ( ExceptionObject );

  /** Finalize the Metric by making sure that there is no
   *  memory leak     */
  virtual void Finalize(void);

  /** Define the bspline tranform type for regularization
   *  For Regularization BsplineTransfromPointer must be explicitly
   *  provided */
  typedef typename Superclass::BSplineTransformType        BSplineTransformType;
  typedef typename Superclass::BSplineTransformTypePointer BSplineTransformTypePointer;

  /** Set/Get the width of kernel used for BSpline smoothing. */
  itkSetMacro( GaussianFilterKernelWidth, double);
  itkGetMacro( GaussianFilterKernelWidth, double);

  /** Set/Get the BSpline regularization flag. */
  itkSetMacro( BSplineRegularizationFlag, bool);
  itkGetMacro( BSplineRegularizationFlag, bool);
protected:
  UnivariateEntropyMultiImageMetric();
  virtual ~UnivariateEntropyMultiImageMetric();

  UnivariateEntropyMultiImageMetric(const Self &); // purposely not implemented
  void operator=(const Self &);                    // purposely not implemented

  void PrintSelf(std::ostream& os, Indent indent) const;

  /** Methods added for supporting multi-threading GetValue */
  void GetThreadedValue( int threadID ) const;

  void BeforeGetThreadedValue(const ParametersType & parameters) const;

  MeasureType AfterGetThreadedValue() const;

  /** Methods added for supporting multi-threading GetValueAndDerivative */
  void GetThreadedValueAndDerivative( int threadID ) const;

  void BeforeGetThreadedValueAndDerivative(const ParametersType & parameters) const;

  void AfterGetThreadedValueAndDerivative(MeasureType & value, DerivativeType & derivative) const;

  /** Static members for multi-threading */
  static ITK_THREAD_RETURN_TYPE ThreaderCallbackGetValueAndDerivative( void *arg );

  static ITK_THREAD_RETURN_TYPE ThreaderCallbackGetValue( void *arg );

  static ITK_THREAD_RETURN_TYPE ThreaderCallbackSampleFixedImageDomain( void *arg );

  /** SpatialSampleContainer typedef support. */
  typedef typename Superclass::SpatialSample SpatialSample;
  typedef std::vector<SpatialSample>         SpatialSampleContainer;

  /** Uniformly select samples from the fixed image buffer. */
  void SampleFixedImageDomain() const;

  void ThreadedSampleFixedImageDomain( int threadID ) const;

  typedef CovariantVector<RealType, ImageDimension> CovarientType;

  /** Add the derivative update to the current images parameters at a given point and image derivative*/
  void UpdateSingleImageParameters( DerivativeType & inputDerivative, const SpatialSample& sample,
                                    const RealType& weight, const int& imageNumber,
                                    const int& threadID) const;

  typedef typename Superclass::CoordinateRepresentationType CoordinateRepresentationType;
  typedef CentralDifferenceImageFunction<ImageType,
                                         CoordinateRepresentationType>
  DerivativeFunctionType;

  mutable std::vector<typename DerivativeFunctionType::Pointer> m_DerivativeCalculator;
  mutable std::vector<std::vector<DerivativeType> >             m_DerivativesArray;

  double                                        m_ImageStandardDeviation;
  std::vector<typename KernelFunction::Pointer> m_KernelFunction;

  bool                    m_ReseedIterator;
  int                     m_RandomSeed;
  bool                    m_UseMask;
  bool                    m_BSplineRegularizationFlag;
  mutable Array<RealType> m_value;

  mutable std::vector<ParametersType> m_TransformParametersArray;

  typedef ImageRandomNonRepeatingConstIteratorWithIndex<ImageType> RandomIterator;
  mutable std::vector<RandomIterator *> m_RandIterArray;

  // Bspline optimization
  int                          numberOfWeights;
  mutable Array<unsigned long> bsplineIndexes;
  long unsigned int            m_NumberOfParametersPerdimension;

  // Bspline regularization, kernel width
  double m_GaussianFilterKernelWidth;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "UnivariateEntropyMultiImageMetric.hxx"
#endif

#endif
