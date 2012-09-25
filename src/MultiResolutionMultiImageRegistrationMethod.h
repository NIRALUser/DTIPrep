/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkMultiResolutionMultiImageRegistrationMethod.h,v $
  Language:  C++
  Date:      $Date: 2003/09/10 14:28:35 $
  Version:   $Revision: 1.8 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __MultiResolutionMultiImageRegistrationMethod_h
#define __MultiResolutionMultiImageRegistrationMethod_h

#include "itkProcessObject.h"
#include "itkImageToImageMetric.h"
#include "itkSingleValuedNonLinearOptimizer.h"
#include "itkMultiResolutionPyramidImageFilter.h"
#include "itkNumericTraits.h"

// user defined headers
#include "VarianceMultiImageMetric.h"
#include "UserMacro.h"
#include <vector>

namespace itk
{

/** \class MultiResolutionMultiImageRegistrationMethod
 * \brief Base class for multi-resolution image registration methods
 *
 * This class provides a generic interface for multi-resolution
 * registration using components of the registration framework.
 * See documentation for ImageRegistrationMethod for a description
 * of the registration framework components.
 *
 * The registration process is initiated by method Update().
 * The user must set the parameters of each component before calling
 * this method.
 *
 * The number of resolution level to process can be set via
 * SetNumberOfLevels(). At each resolution level, the user specified
 * registration components are used to register downsampled version of the
 * images by computing the transform parameters that will map one image onto
 * the other image.
 *
 * The downsampled images are provided by user specified
 * MultiResolutionPyramidImageFilters. User must specify the schedule
 * for each pyramid externally prior to calling Update().
 *
 * \warning If there is discrepancy between the number of level requested
 * and a pyramid schedule. The pyramid schedule will be overriden
 * with a default one.
 *
 * Before each resolution level an IterationEvent is invoked providing an
 * opportunity for a user interface to change any of the components,
 * change component parameters, or stop the registration.
 *
 * This class is templated over the image type.
 *
 * \sa ImageRegistrationMethod
 * \ingroup RegistrationFilters
 */
template <typename TImage>
class ITK_EXPORT MultiResolutionMultiImageRegistrationMethod : public ProcessObject
{
public:
  /** Standard class typedefs. */
  typedef MultiResolutionMultiImageRegistrationMethod Self;
  typedef ProcessObject                               Superclass;
  typedef SmartPointer<Self>                          Pointer;
  typedef SmartPointer<const Self>                    ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(MultiResolutionMultiImageRegistrationMethod, ProcessObject);

  /**  Type of the Fixed image. */
  typedef          TImage                  ImageType;
  typedef typename ImageType::ConstPointer ImageConstPointer;
  typedef        vector<ImageConstPointer> ImageArrayPointer;
  typedef typename ImageType::RegionType   ImageRegionType;

  /**  Type of the metric. */
  typedef MultiImageMetric<ImageType>  MetricType;
  typedef typename MetricType::Pointer MetricPointer;

  /**  Type of the Transform . */
  typedef typename MetricType::TransformType TransformType;
  typedef typename TransformType::Pointer    TransformPointer;
  typedef  vector<TransformPointer>          TransformPointerArray;

  /**  Type of the Interpolator. */
  typedef typename MetricType::InterpolatorType InterpolatorType;
  typedef typename InterpolatorType::Pointer    InterpolatorPointer;
  typedef  vector<InterpolatorPointer>          InterpolatorPointerArray;

  /**  Type of the optimizer. */
  typedef SingleValuedNonLinearOptimizer OptimizerType;

  /** Type of the Fixed image multiresolution pyramid. */
  typedef MultiResolutionPyramidImageFilter<ImageType,
                                            ImageType>
  ImagePyramidType;
  typedef typename ImagePyramidType::Pointer ImagePyramidPointer;
  typedef  vector<ImagePyramidPointer>       ImagePyramidPointerArray;

  /** Typedef for image masks */
  typedef typename  MetricType::ImageMaskType     ImageMaskType;
  typedef typename  MetricType::ImageMaskPointer  ImageMaskPointer;
  typedef typename  std::vector<ImageMaskPointer> ImageMaskPointerArray;

  /** Type used for representing point components  */
  typedef typename MetricType::CoordinateRepresentationType CoordinateRepresentationType;

  /** Type of the Transformation parameters This is the same type used to
   *  represent the search space of the optimization algorithm */
  typedef  typename MetricType::TransformParametersType ParametersType;

  /** Method that initiates the registration. */
  void StartRegistration();

  /** Method to stop the registration. */
  void StopRegistration();

  /** Set/Get the Optimizer. */
  itkSetObjectMacro( Optimizer,  OptimizerType );
  itkGetObjectMacro( Optimizer,  OptimizerType );

  /** Set/Get the Metric. */
  itkSetObjectMacro( Metric, MetricType );
  itkGetObjectMacro( Metric, MetricType );

  /** Set/Get the FixedImageRegion. */
  itkSetMacro( FixedImageRegion, ImageRegionType );
  itkGetConstReferenceMacro( FixedImageRegion, ImageRegionType );

  /** Set/Get the Transfrom. */
  UserSetObjectMacro( TransformArray, TransformType );
  UserGetObjectMacro( TransformArray, TransformType );

  /** Set/Get the Interpolator. */
  UserSetObjectMacro( InterpolatorArray, InterpolatorType );
  UserGetObjectMacro( InterpolatorArray, InterpolatorType );

  /** Set/Get the Fixed image pyramid. */
  UserSetObjectMacro( ImagePyramidArray, ImagePyramidType );
  UserGetObjectMacro( ImagePyramidArray, ImagePyramidType );

  /** Set/Get image mask array */
  UserSetObjectMacro( ImageMaskArray, ImageMaskType );
  UserGetObjectMacro( ImageMaskArray, ImageMaskType );

  /** Set/Get the number of multi-resolution levels. */
  itkSetClampMacro( NumberOfLevels, unsigned long, 1,
                    NumericTraits<unsigned long>::max() );
  itkGetMacro( NumberOfLevels, unsigned long );

  /** Get the current resolution level being processed. */
  itkGetMacro( CurrentLevel, unsigned long );

  /** Set/Get the length of the parameters vector. The length of the parameters
      array should be set before assigning individual parameters */
  virtual void SetTransformParametersLength( int N );

  virtual int  GetTransformParametersLength();

  /** Set/Get the initial transformation parameters. */
  // itkSetMacro( InitialTransformParameters, ParametersType );
  itkGetConstReferenceMacro( InitialTransformParameters, ParametersType );
  virtual void SetInitialTransformParameters( int i, const ParametersType & param );

  virtual void SetInitialTransformParameters( const ParametersType & param );

  /** Set/Get the initial transformation parameters of the next resolution
   level to be processed. The default is the last set of parameters of
   the last resolution level. */
  // itkSetMacro( InitialTransformParametersOfNextLevel, ParametersType );
  itkGetConstReferenceMacro( InitialTransformParametersOfNextLevel, ParametersType );
  virtual void SetInitialTransformParametersOfNextLevel( const ParametersType & param, int i );

  virtual void SetInitialTransformParametersOfNextLevel( const ParametersType & param );

  /** Get the last transformation parameters visited by
   * the optimizer. */
  itkGetConstReferenceMacro( LastTransformParameters, ParametersType );

  /** Set the number of images */
  void SetNumberOfImages(int N);

  /** write output images and extract their slices after each resolution */
protected:
  MultiResolutionMultiImageRegistrationMethod();
  virtual ~MultiResolutionMultiImageRegistrationMethod()
  {
  };
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** Initialize by setting the interconnects between the components.
      This method is executed at every level of the pyramid with the
      values corresponding to this resolution
   */
  void Initialize()
  throw (ExceptionObject);

  /** Compute the size of the fixed region for each level of the pyramid. */
  void PreparePyramids( void );

  /** Set the current level to be processed */
  itkSetMacro( CurrentLevel, unsigned long );
private:
  MultiResolutionMultiImageRegistrationMethod(const Self &); // purposely not implemented
  void operator=(const Self &);                              // purposely not implemented

  MetricPointer          m_Metric;
  OptimizerType::Pointer m_Optimizer;

  TransformPointerArray    m_TransformArray;
  InterpolatorPointerArray m_InterpolatorArray;

  ImagePyramidPointerArray m_ImagePyramidArray;

  ParametersType m_InitialTransformParameters;
  ParametersType m_InitialTransformParametersOfNextLevel;
  ParametersType m_LastTransformParameters;

  ImageRegionType              m_FixedImageRegion;
  std::vector<ImageRegionType> m_FixedImageRegionPyramid;

  ImageMaskPointerArray m_ImageMaskArray;
  unsigned long         m_NumberOfLevels;
  unsigned long         m_CurrentLevel;

  bool m_Stop;

  int m_NumberOfImages;

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "MultiResolutionMultiImageRegistrationMethod.hxx"
#endif

#endif
