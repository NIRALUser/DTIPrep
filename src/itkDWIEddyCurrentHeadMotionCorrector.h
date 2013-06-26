/*=========================================================================

Program:   NeuroLib
Module:    $file: itkDWIEddyCurrentHeadMotionCorrector.h $
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

#ifndef _itkDWIEddyCurrentHeadMotionCorrector_h
#define _itkDWIEddyCurrentHeadMotionCorrector_h

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"
#include "itkVectorContainer.h"
#if (ITK_VERSION_MAJOR < 3)
typedef int ThreadIdType;
#else
#include "itkIntTypes.h"
#endif

#include "itkDWIHeadMotionEddyCurrentCorrection.h" // correct code from Ran Tao

namespace itk
{
/** \class DWIEddyCurrentHeadMotionCorrector
* \brief DWI QC by DWIEddyCurrentHeadMotionCorrector.
*
* DWIEddyCurrentHeadMotionCorrector DWI QC by correcting artifacts from eddycurrent and head motions.
*
* \ingroup Multithreaded
* \ingroup Streamed
*/

template <class TImageType>
class DWIEddyCurrentHeadMotionCorrector :
  public ImageToImageFilter<TImageType, TImageType>
{
public:

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

  struct struDiffusionDir
    {
    std::vector<double> gradientDir;   // HACK:  If gradientDir where a vnl_fixed_vector<double,3> you could compute the
                                       // norm of the vector
    int repetitionNumber;
    };

  /** Standard class typedefs. */
  typedef DWIEddyCurrentHeadMotionCorrector          Self;
  typedef ImageToImageFilter<TImageType, TImageType> Superclass;
  typedef SmartPointer<Self>                         Pointer;
  typedef SmartPointer<const Self>                   ConstPointer;

  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro( DWIEddyCurrentHeadMotionCorrector, ImageToImageFilter);

  /** Typedef to images */
  typedef TImageType                                 OutputImageType;
  typedef TImageType                                 InputImageType;
  typedef typename OutputImageType::Pointer          OutputImagePointer;
  typedef typename InputImageType::ConstPointer      InputImageConstPointer;
  typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

  static const unsigned int dim = 3;
  typedef unsigned short                      DwiPixelType;
  typedef itk::Image<DwiPixelType, dim>       GradientImageType;
  typedef vnl_vector_fixed<double, dim>       GradientDirectionType;
  typedef itk::VectorImage<DwiPixelType, dim> DwiImageType;

  /** Container to hold gradient directions of the 'n' DW measurements */
  typedef VectorContainer<unsigned int,
                          GradientDirectionType> GradientDirectionContainerType;

  /** ImageDimension enumeration. */
  itkStaticConstMacro( ImageDimension, unsigned int, TImageType::ImageDimension );

  /** Get & Set the numberOfBins. */
  itkGetConstMacro( NumberOfBins, int );
  itkSetMacro( NumberOfBins, int );

  /** Get & Set the samples */
  itkGetConstMacro( Samples, int );
  itkSetMacro( Samples, int );

  /** Get & Set the translationScale */
  itkGetConstMacro( TranslationScale, float );
  itkSetMacro( TranslationScale, float );

  /** Get & Set the stepLength */
  itkGetConstMacro( StepLength, float );
  itkSetMacro( StepLength, float );

  /** Get & Set the factor */
  itkGetConstMacro( Factor, float );
  itkSetMacro( Factor, float );

  /** Get & Set the maxNumberOfIterations */
  itkGetConstMacro( MaxNumberOfIterations, int );
  itkSetMacro( MaxNumberOfIterations, int );

  /** Get & Set the check status */
  itkBooleanMacro( CorrectDone);
  itkGetConstMacro( CorrectDone, bool);
  itkSetMacro( CorrectDone, bool);

  /** Get & Set the report file mode */
  itkGetConstMacro( ReportFileMode, int );
  itkSetMacro( ReportFileMode, int  );

  /** Get & Set the ReportFilename */
  itkGetConstMacro( ReportFileName, std::string );
  itkSetMacro( ReportFileName, std::string  );

  /** Get & Set the report type */
  itkGetConstMacro( ReportType, int );
  itkSetMacro( ReportType, int  );

  /** DWIEddyCurrentHeadMotionCorrector produces an image which corrects the eddy-motion and head motion artifacts and updates the diffusion wieghting. As such, DWIEddyCurrentHeadMotionCorrector needs to provide
  * an implementation for GenerateOutputInformation() in order to set the correct mete
  * information.The original documentation of this method is below.
  * \sa ProcessObject::GenerateOutputInformaton() */
  virtual void GenerateOutputInformation();

protected:
  DWIEddyCurrentHeadMotionCorrector();
  ~DWIEddyCurrentHeadMotionCorrector();

  void PrintSelf(std::ostream & os, Indent indent) const;

  virtual void ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId );

private:
  DWIEddyCurrentHeadMotionCorrector(const Self &);       // purposely not
  // implemented
  void operator=(const Self &);                          // purposely not

  // implemented

  // /////////////////////////////////////////////////////////
  DWIHeadMotionEddyCurrentCorrection re;      // correction class

  /** parameters */
  int   m_NumberOfBins;
  int   m_Samples;
  float m_TranslationScale;
  float m_StepLength;
  float m_Factor;
  int   m_MaxNumberOfIterations;

  /** indicate whether correct is done */
  bool m_CorrectDone;

  /** report filename */
  std::string m_ReportFileName;

  /** report file mode */
  int m_ReportFileMode;

  /** report type */
  int m_ReportType;

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

  /** b value */
  double b0;

  /** container to hold gradient directions */
  typename GradientDirectionContainerType::Pointer m_GradientDirectionContainer;
  typename GradientDirectionContainerType::Pointer
  m_FeedinGradientDirectionContainer;
  typename GradientDirectionContainerType::Pointer
  m_OutputGradientDirectionContainer;

  /** container to hold input gradient directions histogram */
  std::vector<struDiffusionDir> DiffusionDirHistInput;

  /** container to hold input b values */
  std::vector<double> bValues;

  /** container to hold output gradient directions histogram */
  std::vector<struDiffusionDir> DiffusionDirHistOutput;

  void parseGradientDirections();

  void collectDiffusionStatistics();

  void correct();

  void collectLeftDiffusionStatistics();

  void writeReport();

public:
  inline typename GradientDirectionContainerType::Pointer GetGradientDirectionContainer()
  {
    return m_GradientDirectionContainer;
  }

  inline typename GradientDirectionContainerType::Pointer GetFeedinGradientDirectionContainer()
  {
    return m_FeedinGradientDirectionContainer;
  }

  inline typename GradientDirectionContainerType::Pointer GetOutputGradientDirectionContainer()
  {
    return m_OutputGradientDirectionContainer;
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

  typedef itk::Image<float, 3>       ScalarImageType;
  typedef itk::VectorImage<float, 3> VectorImageType;
private:
  VectorImageType::Pointer corr;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDWIEddyCurrentHeadMotionCorrector.hxx"
#endif

#endif
