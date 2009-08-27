/*=========================================================================

Program:   NeuroLib
Module:    $file: itkDWIBaselineAverger.h $
Language:  C++
Date:      $Date: 2009-08-27 01:39:28 $
Version:   $Revision: 1.2 $
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

namespace itk
{
	/** \class DWIBaselineAverager
	* \brief DWI Baseline Averaging.
	*
	* DWIBaselineAverager DWI DWI Baseline Averaging.
	*
	* \ingroup Multithreaded
	* \ingroup Streamed
	*/

	template<class TImageType>
	class ITK_EXPORT DWIBaselineAverager : 
		public ImageToImageFilter< TImageType, TImageType>
	{

	public:
		struct struDiffusionDir
		{
			std::vector< double > gradientDir;
			int repetitionNumber;
		};

		typedef enum
		{
			Direct = 0,
			BaselineOptimized,
			GradientOptamized,
		} AverageMethod;

		typedef enum
		{
			IntensityMeanSquareDiffBased= 0,
			MetricSumBased ,                // not being used anymore
			TotalTransformationBased,		// not being used anymore
		} Stop_Criteria;

		typedef struct  RigidRegResult
		{ 
			double		AngleX; // in degrees
			double		AngleY; // in degrees
			double		AngleZ; // in degrees
			double		TranslationX;
			double		TranslationY;
			double		TranslationZ;
			double		MutualInformation; //-Metrix
		} struRigidRegResult,  *pstruRigidRegResult;

		typedef enum
		{
			Report_New = 0,
			Report_Append,
		} ReportFileMode;


		/** Standard class typedefs. */
		typedef DWIBaselineAverager							 Self;
		typedef ImageToImageFilter< TImageType, TImageType>  Superclass;
		typedef SmartPointer<Self>							Pointer;
		typedef SmartPointer<const Self>					ConstPointer;

		itkNewMacro(Self);

		/** Run-time type information (and related methods). */
		itkTypeMacro(DWIBaselineAverager, ImageToImageFilter);

		/** Typedef to images */
		typedef TImageType									OutputImageType;
		typedef TImageType									InputImageType;

		typedef typename OutputImageType::Pointer           OutputImagePointer;
		typedef typename InputImageType::ConstPointer       InputImageConstPointer;
		typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;

		typedef unsigned short						DwiPixelType;
		typedef itk::Image<DwiPixelType, 2>			SliceImageType;
		typedef itk::Image<double, 3>				doubleImageType;

		typedef typename doubleImageType::Pointer	doubleImagePointerType;

		typedef itk::Image<unsigned int, 3>					UnsignedImageType;
		typedef typename UnsignedImageType::Pointer         UnsignedImageTypePointer;

		typedef vnl_vector_fixed< double, 3 >       GradientDirectionType;

		/** Container to hold gradient directions of the 'n' DW measurements */
		typedef VectorContainer< unsigned int, GradientDirectionType >   GradientDirectionContainerType;
		
		/** ImageDimension enumeration. */
		itkStaticConstMacro(ImageDimension, unsigned int, TImageType::ImageDimension );

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
		itkGetConstMacro( StopCriteria, int );
		itkSetMacro( StopCriteria, int  );

		/** Get & Set the Stop Threshold */
		itkGetConstMacro( StopThreshold, float );
		itkSetMacro( StopThreshold, float  );

		/** Get & Set the Stop Threshold */
		itkGetConstMacro( MaxIteration, unsigned int );
		itkSetMacro( MaxIteration, unsigned int  );
		
		/** Get & Set the report file mode */
		itkGetConstMacro( ReportFileMode, int );
		itkSetMacro( ReportFileMode, int  );

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

		void PrintSelf(std::ostream& os, Indent indent) const;
		void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,int threadId );  

	private:
		DWIBaselineAverager(const Self&);	//purposely not implemented
		void operator=(const Self&);		//purposely not implemented

		/** averaged baseline */
		UnsignedImageTypePointer averagedBaseline;

		/** temp baseline image  */
		UnsignedImageTypePointer tempBaseline;

		/** temp idwi */
		UnsignedImageTypePointer idwi;

		/** temp idwi image  */
		doubleImagePointerType tempIDWI;

		/** Report File Mode */
		int m_ReportFileMode;

		/** average method */
		int m_AverageMethod;

		/** Stop Criteria */
		int m_StopCriteria;

		/** Stop threshold */
		float m_StopThreshold;

		/** Max ineration */
		unsigned int m_MaxIteration;

		/** indicate whether average is done */
		bool m_AverageDone;

		/** report filename */
		std::string m_ReportFileName ;

		/** baseline filename */
		std::string m_BaselineFileName ;

		/** idwi filename */
		std::string m_IdwiFileName ;

		/** input info */
		int baselineNumber;
		int bValueNumber;
		int gradientDirNumber;
		int repetitionNumber;
		int gradientNumber;
		
		/** output info */
		int baselineLeftNumber;
		int bValueLeftNumber;
		int gradientDirLeftNumber;
		int gradientLeftNumber;
		std::vector<int> repetitionLeftNumber;

		/** b value */
		double b0 ;

		/** container to hold input b values */
		std::vector<double> bValues;

		/** container to hold gradient directions */
		GradientDirectionContainerType::Pointer  m_GradientDirectionContainer;

		/** container to hold input gradient directions histogram */
		std::vector<struDiffusionDir> DiffusionDirHistInput;

		/** container to hold baseline image wise registration */
		std::vector< std::vector<struRigidRegResult> > BaselineToBaselineReg;

		/** container to hold gradient to base wise registration */
		std::vector< std::vector<struRigidRegResult> > GradientToBaselineReg;

		void parseGridentDirections();
		void collectDiffusionStatistics();
		void average();
		void writeReport();

		void BaselineOptimizedAverage();
		void GradientOptamizedAverage();
		void DirectAverage();

		void computeIDWI();

		void rigidRegistration(
			UnsignedImageType::Pointer fixed, 
			UnsignedImageType::Pointer moving, 
			unsigned int BinsNumber,
			double  SamplesPercent ,
			bool	ExplicitPDFDerivatives,
			struRigidRegResult&  regResult,
			int baselineORidwi // 0: baseline; otherwise: idwi
			);

	public:

		inline GradientDirectionContainerType::Pointer  GetGradientDirectionContainer()
			{ return m_GradientDirectionContainer; };

		UnsignedImageTypePointer GetBaseline();
		UnsignedImageTypePointer GetIDWI();

		inline int getBaselineNumber()		{   return baselineNumber;};
		inline int getBValueNumber()		{   return bValueNumber;};
		inline int getGradientDirNumber()	{   return gradientDirNumber;};
		inline int getRepetitionNumber()	{   return repetitionNumber;};
		inline int getGradientNumber()		{   return gradientNumber;};
	};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDWIBaselineAverager.cpp"
#endif

#endif
