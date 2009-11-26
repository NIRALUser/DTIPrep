/*=========================================================================

Program:   NeuroLib
Module:    $file: itkDWIQCSliceChecker.h $
Language:  C++
Date:      $Date: 2009-11-26 21:52:35 $
Version:   $Revision: 1.7 $
Author:    Zhexing Liu (liuzhexing@gmail.com)

Copyright (c) NIRAL, UNC. All rights reserved.
See http://www.niral.unc.edu for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _itkDWIQCSliceChecker_h
#define _itkDWIQCSliceChecker_h

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"
#include "itkVectorContainer.h"

namespace itk
{
/** \class DWIQCSliceChecker
* \brief DWI QC by Slice-wise Check.
*
* WIQCSliceChecker DWI QC by Slice-wise Check.
*
* \ingroup Multithreaded
* \ingroup Streamed
*/

template <class TImageType>
class ITK_EXPORT DWIQCSliceChecker :
  public ImageToImageFilter<TImageType, TImageType>
  {
public:

  typedef enum {
    Report_New = 0,
    Report_Append,
    } ReportFileMode;

  struct struDiffusionDir {
    std::vector<double> gradientDir;
    int repetitionNumber;
    };

  /** Standard class typedefs. */
  typedef DWIQCSliceChecker                          Self;
  typedef ImageToImageFilter<TImageType, TImageType> Superclass;
  typedef SmartPointer<Self>                         Pointer;
  typedef SmartPointer<const Self>                   ConstPointer;

  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(DWIQCSliceChecker, ImageToImageFilter);

  /** Typedef to images */
  typedef TImageType                                 OutputImageType;
  typedef TImageType                                 InputImageType;
  typedef typename OutputImageType::Pointer          OutputImagePointer;
  typedef typename InputImageType::ConstPointer      InputImageConstPointer;
  typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

  typedef unsigned short                             DwiPixelType;
  typedef itk::Image<DwiPixelType, 2>                SliceImageType;
  typedef itk::Image<DwiPixelType, 3>                GradientImageType;

  typedef vnl_vector_fixed<double, 3>                GradientDirectionType;

  /** Container to hold gradient directions of the 'n' DW measurements */
  typedef VectorContainer<unsigned int,
    GradientDirectionType> GradientDirectionContainerType;

  /** ImageDimension enumeration. */
  itkStaticConstMacro(ImageDimension, unsigned int, TImageType::ImageDimension );

  /** Get & Set the HeadSkipRatio. */
  itkGetConstMacro( HeadSkipRatio, float );
  itkSetMacro( HeadSkipRatio, float );

  /** Get & Set the TailSkipRatio */
  itkGetConstMacro( TailSkipRatio, float );
  itkSetMacro( TailSkipRatio, float );

  /** Get & Set the StdevTimes */
  itkGetConstMacro( BaselineStdevTimes, float );
  itkSetMacro( BaselineStdevTimes, float );

  /** Get & Set the StdevTimes */
  itkGetConstMacro( GradientStdevTimes, float );
  itkSetMacro( GradientStdevTimes, float );

  /** Get & Set the CheckTimes */
  itkGetConstMacro( CheckTimes, int );
  itkSetMacro( CheckTimes, int );

  /** Get & Set the check status */
  itkBooleanMacro(CheckDone);
  itkGetConstMacro(CheckDone, bool);
  itkSetMacro(CheckDone, bool);

  /** Get & Set the QuadFit indicator */
  itkBooleanMacro( QuadFit );
  itkGetConstMacro( QuadFit, bool );
  itkSetMacro( QuadFit, bool );

  /** Get & Set the smoothing indicator */
  itkBooleanMacro( Smoothing );
  itkGetConstMacro( Smoothing, bool );
  itkSetMacro( Smoothing, bool );

  /** Get & Set the SubRegionalCheck indicator */
  itkBooleanMacro( SubRegionalCheck );
  itkGetConstMacro( SubRegionalCheck, bool );
  itkSetMacro( SubRegionalCheck, bool );

  /** Get & Set the report file mode */
  itkGetConstMacro( ReportFileMode, int );
  itkSetMacro( ReportFileMode, int  );

  /** Get & Set the ReportFilename */
  itkGetConstMacro( ReportFileName, std::string );
  itkSetMacro( ReportFileName, std::string  );

  /** Get & Set the GaussianVariance */
  itkGetConstMacro( GaussianVariance, double );
  itkSetMacro( GaussianVariance, double  );

  /** Get & Set the MaxKernelWidth */
  itkGetConstMacro( MaxKernelWidth, double );
  itkSetMacro( MaxKernelWidth, double  );

  /** DWIQCSliceChecker produces an image which is a different vector length
    * than its input image. As such, DWIQCSliceChecker needs to provide
    * an implementation for GenerateOutputInformation() in order to inform
    * the pipeline execution model.The original documentation of this
    * method is below.
    * \sa ProcessObject::GenerateOutputInformaton() */
  virtual void GenerateOutputInformation();

protected:
  DWIQCSliceChecker();
  ~DWIQCSliceChecker();

  void PrintSelf(std::ostream & os, Indent indent) const;

  void ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
    int threadId );

private:
  DWIQCSliceChecker(const Self &);  // purposely not implemented
  void operator=(const Self &);     // purposely not implemented

  /** Gaussian smoothing parameters */
  double m_GaussianVariance;
  double m_MaxKernelWidth;

  /** check parameters */
  float m_HeadSkipRatio;
  float m_TailSkipRatio;
  float m_BaselineStdevTimes;
  float m_GradientStdevTimes;

  /** check times, ineratively while <=0  */
  int m_CheckTimes;

  /** indicate whether chech is done */
  bool m_CheckDone;

  /** report filename */
  std::string m_ReportFileName;

  /** report file mode */
  int m_ReportFileMode;

  /** excluded gradients filename */
  OutputImagePointer excludedDwiImage;

  /** input info */
  int baselineNumber;
  int bValueNumber;
  int gradientDirNumber;
  int repetitionNumber;
  int gradientNumber;

  /** output info */
  int              baselineLeftNumber;
  int              bValueLeftNumber;
  int              gradientDirLeftNumber;
  int              gradientLeftNumber;
  std::vector<int> repetitionLeftNumber;

  /** smoothing */
  bool m_Smoothing;

  /** quadratic fitting? */
  bool m_QuadFit;

  /** conduct subregional check */
  bool m_SubRegionalCheck;

  /** b value */
  double b0;

  /** container to hold gradient directions */
  GradientDirectionContainerType::Pointer m_GradientDirectionContainer;

  /** container to hold input gradient directions histogram */
  std::vector<struDiffusionDir> DiffusionDirHistInput;

  /** container to hold input b values */
  std::vector<double> bValues;

  /** container to hold output gradient directions histogram */
  std::vector<struDiffusionDir> DiffusionDirHistOutput;

  /** for all gradients  slice wise correlation */
  std::vector<double> gradientMeans;
  std::vector<double> gradientDeviations;

  /** for all baseline slice wise correlation */
  std::vector<double> baselineMeans;
  std::vector<double> baselineDeviations;

  /** for all multi-bValued gradient slice wise correlation(after quardatic
    fitting) */
  std::vector<double> quardraticFittedMeans;
  std::vector<double> quardraticFittedDeviations;

  /** initialize qcResullts */
  std::vector<std::vector<double> > ResultsContainer;    // starts from #1
                                                         // slice,
                                                         // "correlation<=0"
                                                         // means a "bad slice"

  std::vector<std::vector<double> > ResultsContainer0;    // starts from #1
                                                          // slice,
                                                          // "correlation<=0"
                                                          // means a "bad slice"
  std::vector<std::vector<double> > ResultsContainer1;    // starts from #1
                                                          // slice,
                                                          // "correlation<=0"
                                                          // means a "bad slice"
  std::vector<std::vector<double> > ResultsContainer2;    // starts from #1
                                                          // slice,
                                                          // "correlation<=0"
                                                          // means a "bad slice"
  std::vector<std::vector<double> > ResultsContainer3;    // starts from #1
                                                          // slice,
                                                          // "correlation<=0"
                                                          // means a "bad slice"
  std::vector<std::vector<double> > ResultsContainer4;    // starts from #1
                                                          // slice,
                                                          // "correlation<=0"
                                                          // means a "bad slice"
  // 0      1
  //     2
  // 3      4
  /** for all gradients  slice wise correlation */
  std::vector<double> gradientMeans0;
  std::vector<double> gradientDeviations0;
  std::vector<double> gradientMeans1;
  std::vector<double> gradientDeviations1;
  std::vector<double> gradientMeans2;
  std::vector<double> gradientDeviations2;
  std::vector<double> gradientMeans3;
  std::vector<double> gradientDeviations3;
  std::vector<double> gradientMeans4;
  std::vector<double> gradientDeviations4;

  std::vector<std::vector<double> > HistogramCorrelationContainer;    // starts
                                                                      // from #1
                                                                      // slice,
                                                                      //
                                                                      // "correlation<=0"
                                                                      // means a
                                                                      // "bad
                                                                      // slice"

  std::vector<bool>                 qcResults;
  std::vector<std::vector<double> > normalizedMetric;

  void parseGradientDirections();

  void collectDiffusionStatistics();

  void initializeQCResullts();

  void calculateCorrelations( bool smoothing );

  void calculateSubRegionalCorrelations();

  void check();

  void SubRegionalcheck();

  void LeaveOneOutcheck();

  void iterativeCheck();

  void collectLeftDiffusionStatistics();

  void writeReport();

  // calculate slice-wise histogram correlations
  void calculateSliceWiseHistogramCorrelations( bool smoothing,
    double GaussianVariance,
    double MaxKernelWidth );

public:
  OutputImagePointer GetExcludedGradiennts();

  inline std::vector<bool> getQCResults()
  {
    return qcResults;
  }

  inline GradientDirectionContainerType::Pointer  GetGradientDirectionContainer()
  {
    return m_GradientDirectionContainer;
  }

  inline int getBaselineNumber()
  {
    return baselineNumber;
  }

  inline int getBValueNumber()
  {
    return bValueNumber;
  }

  inline int getGradientDirNumber()
  {
    return gradientDirNumber;
  }

  inline int getRepetitionNumber()
  {
    return repetitionNumber;
  }

  inline int getGradientNumber()
  {
    return gradientNumber;
  }

  inline int getBaselineLeftNumber()
  {
    return baselineLeftNumber;
  }

  inline int getBValueLeftNumber()
  {
    return bValueLeftNumber;
  }

  inline int getGradientDirLeftNumber()
  {
    return gradientDirLeftNumber;
  }

  inline int getGradientLeftNumber()
  {
    return gradientLeftNumber;
  }

  inline std::vector<int> getRepetitionLeftNumber()
  {
    return repetitionLeftNumber;
  }
  };
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDWIQCSliceChecker.cpp"
#endif

#endif
