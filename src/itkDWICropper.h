/*=========================================================================

Program:   NeuroLib
Module:    $file: itkDWICropper.h $
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

#ifndef _itkDWICropper_h
#define _itkDWICropper_h

#include "itkImageToImageFilter.h"
#include "vnl/vnl_vector_fixed.h"
#include "itkVectorImage.h"
#include "itkVectorContainer.h"

namespace itk
{
  /** \class DWICropper
  * \brief DWI image size crop/pad filter.
  *
  * DWICropper DWI image size crop/pad filter.
  *
  * \ingroup Multithreaded
  * \ingroup Streamed
  */

  template <class TImageType>
  class ITK_EXPORT DWICropper :
    public ImageToImageFilter<TImageType, TImageType>
  {
  public:
    struct struDiffusionDir {
      std::vector<double> gradientDir;
      int repetitionNumber;
    };

    typedef enum {
      REPORT_FILE_NEW = 0,
      REPORT_FILE_APPEND,
    } ReportFileMode;

    typedef enum {
      REPORT_TYPE_NO = -1,
      REPORT_TYPE_SIMPLE,
      REPORT_TYPE_VERBOSE,
      REPORT_TYPE_EASY_PARSE,
    } ReportType;

    /** Standard class typedefs. */
    typedef DWICropper                                 Self;
    typedef ImageToImageFilter<TImageType, TImageType> Superclass;
    typedef SmartPointer<Self>                         Pointer;
    typedef SmartPointer<const Self>                   ConstPointer;

    itkNewMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(DWICropper, ImageToImageFilter);

    /** Typedef to images */
    typedef TImageType                                 OutputImageType;
    typedef TImageType                                 InputImageType;

    typedef typename OutputImageType::Pointer          OutputImagePointer;
    typedef typename InputImageType::ConstPointer      InputImageConstPointer;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    typedef unsigned short                             DwiPixelType;
    typedef vnl_vector_fixed<double, 3>                GradientDirectionType;

    /** Container to hold gradient directions of the 'n' DW measurements */
    typedef VectorContainer<unsigned int,
      GradientDirectionType> GradientDirectionContainerType;

    /** ImageDimension enumeration. */
    itkStaticConstMacro(ImageDimension, unsigned int, TImageType::ImageDimension );

    /** Get & Set the ReportFilename */
    itkGetConstMacro( ReportFileName, std::string );
    itkSetMacro( ReportFileName, std::string  );

    /** Get & Set the report file mode */
    itkGetConstMacro( ReportFileMode, int );
    itkSetMacro( ReportFileMode, int  );

    /** Get & Set the report type */
    itkGetConstMacro( ReportType, int );
    itkSetMacro( ReportType, int  );

    /** DWICropper produces an image which is a padded/cropped version of the input.
    * As such, DWICropper needs to provide
    * an implementation for GenerateOutputInformation() in order to inform
    * the pipeline execution model.The original documentation of this
    * method is below.
    * \sa ProcessObject::GenerateOutputInformaton() */

    virtual void GenerateOutputInformation();

    /**
    * DWICropper needs a smaller input requested region than
    * output requested region.  As such, PadImageFilter needs to
    * provide an implementation for GenerateInputRequestedRegion() in
    * order to inform the pipeline execution model.
    *
    * \sa ProcessObject::GenerateInputRequestedRegion()
    */
    virtual void GenerateInputRequestedRegion();

  protected:
    DWICropper();
    ~DWICropper();

    void PrintSelf(std::ostream & os, Indent indent) const;

    void ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
      int threadId );

  private:
    DWICropper(const Self &);     // purposely not implemented
    void operator=(const Self &); // purposely not implemented

    /** region para */
    int *region;   // region[0]: index i, region[1]:size i;
        // region[2]: index j, region[3]:size j;
        // region[4]: index k, region[5]:size k;

    /** size para */
    int *size;

    /** Report File Mode */
    int m_ReportFileMode;

    /** report type */
    int m_ReportType;


    /** report filename */
    std::string m_ReportFileName;

    /** input info */
    int baselineNumber;
    int bValueNumber;
    int gradientDirNumber;
    int repetitionNumber;
    int gradientNumber;

    /** b value */
    double b0;

    /** container to hold input b values */
    std::vector<double> bValues;

    /** container to hold gradient directions */
    GradientDirectionContainerType::Pointer m_GradientDirectionContainer;

    /** container to hold input gradient directions histogram */
    std::vector<struDiffusionDir> DiffusionDirHistInput;

    void parseGradientDirections();

    void collectDiffusionStatistics();

    void writeReport();

    void doPadding();

  public:
    inline int * GetSize()
    {
      return size;
    }

    inline int * GetRegion()
    {
      return region;
    }

    inline void SetSize( int *s )
    {
      size = s;
    }

    inline void SetRegion( int *r )
    {
      region = r;
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
  };
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDWICropper.cpp"
#endif

#endif
