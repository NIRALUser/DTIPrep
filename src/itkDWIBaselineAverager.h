/*=========================================================================

Program:   NeuroLib
Module:    $file: itkDWIBaselineAverger.h $
Language:  C++
Date:      $Date: 2009-11-26 21:52:35 $
Version:   $Revision: 1.4 $
Author:    Zhexing Liu (liuzhexing@gmail.com)

Copyright (c) NIRAL, UNC. All rights reserved.
See http://www.niral.unc.edu for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _itkDWIBaseLineAverager_h
#define _itkDWIBaseLineAverager_h

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"
#include "itkVectorContainer.h"
#include "itkVersorRigid3DTransform.h"

#if (ITK_VERSION_MAJOR < 4)
typedef int ThreadIdType;
#else
#include "itkIntTypes.h"
#endif

#define NearZeroSmallNumber 1e-7

namespace itk
{

typedef enum
  {
  IntensityMeanSquareDiffBased = 0,
  MetricSumBased,                       // not being used anymore
  TotalTransformationBased,             // not being used anymore
  } StopCriteriaEnum;

/** \class DWIBaselineAverager
* \brief DWI Baseline Averaging.
*
* DWIBaselineAverager DWI DWI Baseline Averaging.
*
* \ingroup Multithreaded
* \ingroup Streamed
*/
template <class TVectorImageType>
class ITK_EXPORT DWIBaselineAverager :
  public ImageToImageFilter<TVectorImageType, TVectorImageType>
{
public:
  struct struDiffusionDir
    {
    std::vector<double> gradientDir;
    int m_repetitionNumber;
    };

  typedef enum
    {
    Direct = 0,
    BaselineOptimized,
    GradientOptamized,
    BSplineOptimized,
    } AverageMethod;

  typedef enum
    {
    REPORT_FILE_NEW = 0,
    REPORT_FILE_APPEND,
    } ReportFileMode;

  typedef enum
    {
    REPORT_TYPE_NO = -1,
    REPORT_TYPE_SIMPLE,
    REPORT_TYPE_VERBOSE,
    REPORT_TYPE_EASY_PARSE,
    } ReportType;

  /** Standard class typedefs. */
  typedef DWIBaselineAverager                                    Self;
  typedef ImageToImageFilter<TVectorImageType, TVectorImageType> Superclass;
  typedef SmartPointer<Self>                                     Pointer;
  typedef SmartPointer<const Self>                               ConstPointer;

  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(DWIBaselineAverager, ImageToImageFilter);

  /** Typedef to images */
  typedef TVectorImageType OutputImageType;

  typedef typename OutputImageType::Pointer          OutputImagePointer;
  typedef typename TVectorImageType::ConstPointer    InputImageConstPointer;
  typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

  typedef unsigned short              DwiPixelType;
  typedef itk::Image<DwiPixelType, 2> SliceImageType;
  // HACK:  Made this float instead of double
  typedef itk::Image<float, 3> floatImageType;

  typedef typename floatImageType::Pointer floatImagePointerType;

  typedef itk::Image<unsigned int, 3>         UnsignedImageType;
  typedef typename UnsignedImageType::Pointer UnsignedImageTypePointer;

  typedef vnl_vector_fixed<double, 3> GradientDirectionType;

  /** Container to hold gradient directions of the 'n' DW measurements */
  typedef VectorContainer<unsigned int,
                          GradientDirectionType> GradientDirectionContainerType;

  /** ImageDimension enumeration. */
  itkStaticConstMacro(ImageDimension, unsigned int, TVectorImageType::ImageDimension );

  /** Get & Set the Average Method */
  itkGetConstMacro( AverageMethod, int );
  itkSetMacro( AverageMethod, int );

  /** Get & Set the ReportFilename */
  itkGetConstMacro( ReportFileName, std::string );
  itkSetMacro( ReportFileName, std::string  );

  /** Get & Set the m_BaselineFileName */
  itkGetConstMacro( BaselineFileName, std::string );
  itkSetMacro( BaselineFileName, std::string  );

  /** Get & Set the m_IdwiFileName */
  itkGetConstMacro( IdwiFileName, std::string );
  itkSetMacro( IdwiFileName, std::string  );

  /** Get & Set the Stop Criteria */
  itkGetConstMacro( StopCriteria, StopCriteriaEnum );
  itkSetMacro( StopCriteria, StopCriteriaEnum);

  /** Get & Set the Stop Threshold */
  itkGetConstMacro( StopThreshold, float );
  itkSetMacro( StopThreshold, float  );

  /** Get & Set the Stop Threshold */
  itkGetConstMacro( MaxIteration, unsigned int );
  itkSetMacro( MaxIteration, unsigned int  );

  /** Get & Set the report file mode */
  itkGetConstMacro( ReportFileMode, int );
  itkSetMacro( ReportFileMode, int  );

  /** Get & Set the report type */
  itkGetConstMacro( ReportType, int );
  itkSetMacro( ReportType, int  );

  /** Get & Set the check status */
  itkBooleanMacro(AverageDone);
  itkGetConstMacro(AverageDone, bool);
  itkSetMacro(AverageDone, bool);

  /** DWIBaselineAverager produces an image which contains a averaged baseline volumes if any.
  * As such, DWIBaselineAverager needs to provide
  * an implementation for GenerateOutputInformation() in order to inform
  * the pipeline execution model.The original documentation of this
  * method is below.
  * \sa ProcessObject::GenerateOutputInformaton() */
  virtual void GenerateOutputInformation();

protected:
  DWIBaselineAverager();
  ~DWIBaselineAverager();

  void PrintSelf(std::ostream & os, Indent indent) const;

  virtual void ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId );

private:
  DWIBaselineAverager(const Self &);     // purposely not implemented
  void operator=(const Self &);          // purposely not implemented

  /** averaged baseline */
  floatImagePointerType m_averagedBaseline;

  /** temp baseline image  */
//    UnsignedImageTypePointer m_tempBaseline;

  /** temp idwi */
  UnsignedImageTypePointer idwi;

  /** temp idwi image  */
  floatImagePointerType m_tempIDWI;

  /** Report File Mode */
  int m_ReportFileMode;

  /** report type */
  int m_ReportType;

  /** average method */
  int m_AverageMethod;

  /** Stop Criteria */
  StopCriteriaEnum m_StopCriteria;

  /** Stop threshold */
  float m_StopThreshold;

  /** Max ineration */
  unsigned int m_MaxIteration;

  /** indicate whether average is done */
  bool m_AverageDone;

  /** report filename */
  std::string m_ReportFileName;

  /** baseline filename */
  std::string m_BaselineFileName;

  /** idwi filename */
  std::string m_IdwiFileName;

  /** input info */
  unsigned int m_baselineNumber;
  int          m_bValueNumber;
  int          m_gradientDirNumber;
  int          m_gradientNumber;

  /** output info */
  // HACK:  Not used int              m_baselineLeftNumber;
  // HACK:  Not used int              m_bValueLeftNumber;
  // HACK:  Not used int              m_gradientDirLeftNumber;
  // HACK:  Not used int              m_gradientLeftNumber;
  // HACK:  Not used std::vector<int> m_repetitionLeftNumber;

  /** b value */
  double m_b0;

  /** container to hold input b values */
  std::vector<double> m_bValues;

  /** container to hold gradient directions */
  GradientDirectionContainerType::Pointer m_GradientDirectionContainer;

  /** container to hold input gradient directions histogram */
  // std::vector<struDiffusionDir> DiffusionDirHistInput;

  /** container to hold baseline image wise registration */
  // std::vector<std::vector<struRigidRegResult> > BaselineToBaselineReg;

  /** container to hold gradient to base wise registration */
  // std::vector<std::vector<struRigidRegResult> > GradientToBaselineReg;

  void parseGradientDirections();

  void collectDiffusionStatistics();

  void average();

  void writeReport();

//    void BaselineOptimizedAverage();

  void GradientOptimizedAverage();

  void DirectAverage();

  void computeIDWI();

  /** container to hold status of gradient, 1 if is baseline and 0 if is not */
  std::vector<bool> Gradient_indx_Baselines;
public:

  inline GradientDirectionContainerType::Pointer  GetGradientDirectionContainer()
  {
    return m_GradientDirectionContainer;
  }

  inline unsigned int getBaselineNumber()
  {
    return m_baselineNumber;
  }

  inline int getBValueNumber()
  {
    return m_bValueNumber;
  }

  inline int getGradientDirNumber()
  {
    return m_gradientDirNumber;
  }

  inline int getGradientNumber()
  {
    return m_gradientNumber;
  }

  bool GradientDirectionIsB0Image(const unsigned int DirectionIndex) const
  {
    const GradientDirectionType & currentGradient = this->m_GradientDirectionContainer->ElementAt(DirectionIndex);

    return
      vcl_abs(currentGradient[0]) < NearZeroSmallNumber
      && vcl_abs(currentGradient[1]) < NearZeroSmallNumber
      && vcl_abs(currentGradient[2]) < NearZeroSmallNumber
    ;
  }

  inline std::vector<bool> getGradient_indx_Baselines()
  {
    return Gradient_indx_Baselines;
  }

};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDWIBaselineAverager.hxx"
#endif

#endif
