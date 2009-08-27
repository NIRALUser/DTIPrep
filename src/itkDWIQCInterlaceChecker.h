/*=========================================================================

Program:   NeuroLib
Module:    $file: itkDWIQCInterlaceChecker.h $
Language:  C++
Date:      $Date: 2009-08-27 01:39:40 $
Version:   $Revision: 1.2 $
Author:    Zhexing Liu (liuzhexing@gmail.com)

Copyright (c) NIRAL, UNC. All rights reserved.
See http://www.niral.unc.edu for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _itkDWIQCInterlaceChecker_h
#define _itkDWIQCInterlaceChecker_h

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"
#include "itkVectorContainer.h"

namespace itk
{
	/** \class DWIQCInterlaceChecker
	* \brief DWI QC by Interlace-wise Check.
	*
	* itkDWIQCInterlaceChecker DWI QC by Interlace-wise Check.
	*
	* \ingroup Multithreaded
	* \ingroup Streamed
	*/

	template<class TImageType>
	class ITK_EXPORT DWIQCInterlaceChecker : 
		public ImageToImageFilter< TImageType, TImageType>
	{

	public:

		typedef enum
		{
			Report_New = 0,
			Report_Append,
		} ReportFileMode;

		struct struDiffusionDir
		{
			std::vector< double > gradientDir;
			int repetitionNumber;
		};

		typedef struct  InterlaceResults
		{ 
			bool		bRegister;
			double		AngleX; // in degrees
			double		AngleY; // in degrees
			double		AngleZ; // in degrees
			double		TranslationX;
			double		TranslationY;
			double		TranslationZ;
			double		Metric;  //MutualInformation;	
			double      Correlation;		// graylevel correlation
		} struInterlaceResults,  *pstruInterlaceResults;

		/** Standard class typedefs. */
		typedef DWIQCInterlaceChecker						 Self;
		typedef ImageToImageFilter< TImageType, TImageType>  Superclass;
		typedef SmartPointer<Self>							Pointer;
		typedef SmartPointer<const Self>					ConstPointer;

		itkNewMacro(Self);

		/** Run-time type information (and related methods). */
		itkTypeMacro(DWIQCInterlaceChecker, ImageToImageFilter);

		/** Typedef to images */
		typedef TImageType									OutputImageType;
		typedef TImageType									InputImageType;
		typedef typename OutputImageType::Pointer           OutputImagePointer;
		typedef typename InputImageType::ConstPointer       InputImageConstPointer;
		typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;

		typedef unsigned short						DwiPixelType;
		typedef itk::Image<DwiPixelType, 3>			GradientImageType;

		typedef vnl_vector_fixed< double, 3 >       GradientDirectionType;

		/** Container to hold gradient directions of the 'n' DW measurements */
		typedef VectorContainer< unsigned int, GradientDirectionType >   GradientDirectionContainerType;
		
		/** ImageDimension enumeration. */
		itkStaticConstMacro(ImageDimension, unsigned int, TImageType::ImageDimension );

		/** Get & Set the CorrelationThresholdBaseline. */
		itkGetConstMacro( CorrelationThresholdBaseline, float );
		itkSetMacro( CorrelationThresholdBaseline, float );

		/** Get & Set the CorrelationThresholdGradient */
		itkGetConstMacro( CorrelationThresholdGradient, float );
		itkSetMacro( CorrelationThresholdGradient, float );

		/** Get & Set the TranslationThreshold */
		itkGetConstMacro( TranslationThreshold, float );
		itkSetMacro( TranslationThreshold, float );

		/** Get & Set the RotationThreshold */
		itkGetConstMacro( RotationThreshold, float );
		itkSetMacro( RotationThreshold, float );

		/** Get & Set the CorrelationStedvTimesBaseline */
		itkGetConstMacro( CorrelationStedvTimesBaseline, float );
		itkSetMacro( CorrelationStedvTimesBaseline, float );
		
		/** Get & Set the CorrelationStdevTimesGradient */
		itkGetConstMacro( CorrelationStdevTimesGradient, float );
		itkSetMacro( CorrelationStdevTimesGradient, float );
	
		/** Get & Set the check status */
		itkBooleanMacro(CheckDone);
		itkGetConstMacro(CheckDone, bool);
		itkSetMacro(CheckDone, bool);

		/** Get & Set the report file mode */
		itkGetConstMacro( ReportFileMode, int );
		itkSetMacro( ReportFileMode, int  );

		/** Get & Set the ReportFilename */
		itkGetConstMacro( ReportFileName, std::string );
		itkSetMacro( ReportFileName, std::string  );


		/** DWIQCInterlaceChecker produces an image which is a different vector length
			* than its input image. As such, DWIQCInterlaceChecker needs to provide
			* an implementation for GenerateOutputInformation() in order to inform
			* the pipeline execution model.The original documentation of this
			* method is below.
			* \sa ProcessObject::GenerateOutputInformaton() */
		virtual void GenerateOutputInformation();
		
	protected:
		DWIQCInterlaceChecker();
		~DWIQCInterlaceChecker();

		void PrintSelf(std::ostream& os, Indent indent) const;
		void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,int threadId );  

	private:
		DWIQCInterlaceChecker(const Self&); //purposely not implemented
		void operator=(const Self&);    //purposely not implemented

		//for interlace baseline correlation
		double interlaceBaselineMeans;
		double interlaceBaselineDeviations;

		//for interlace gradient correlation
		double interlaceGradientMeans;
		double interlaceGradientDeviations;

		// for all multi-bValued gradient-wise correlation(after quardatic fitting) 
		double quardraticFittedMeans;
		double quardraticFittedDeviations;

		/** check parameters */
		float m_CorrelationThresholdBaseline ;
		float m_CorrelationThresholdGradient ;
		float m_CorrelationStedvTimesBaseline ;
		float m_CorrelationStdevTimesGradient ;
		float m_TranslationThreshold ;
		float m_RotationThreshold ;
		
		/** indicate whether check is done */
		bool m_CheckDone;

		/** excluded gradients filename */
		OutputImagePointer      excludedDwiImage;

		/** report filename */
		std::string m_ReportFileName ;

		/** report file mode */
		int m_ReportFileMode ;

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

		/** container to hold output gradient directions histogram */
		std::vector<struDiffusionDir> DiffusionDirHistOutput;

		/** initialize qcResullts */
		std::vector< struInterlaceResults >	ResultsContainer;
		std::vector<bool> qcResults;							

		void parseGridentDirections();
		void collectDiffusionStatistics();
		void initializeQCResullts();
		void rigidRegistration(
							GradientImageType::Pointer odd, 
							GradientImageType::Pointer even, 
							unsigned int BinsNumber,
							double  SamplesPercent ,
							bool	ExplicitPDFDerivatives,
							struInterlaceResults&  regResult);

		double computeCorrelation(GradientImageType::Pointer odd, GradientImageType::Pointer even);
		void calculateCorrelationsAndMotions();
		void check();
		void collectLeftDiffusionStatistics();
		void writeReport();


	public:
		OutputImagePointer GetExcludedGradiennts();
		inline std::vector<bool> getQCResults() { return qcResults; };
		inline GradientDirectionContainerType::Pointer  GetGradientDirectionContainer()
		{ return m_GradientDirectionContainer; };


		inline int getBaselineNumber()		{   return baselineNumber;};
		inline int getBValueNumber()		{   return bValueNumber;};
		inline int getGradientDirNumber()	{   return gradientDirNumber;};
		inline int getRepetitionNumber()	{   return repetitionNumber;};
		inline int getGradientNumber()		{   return gradientNumber;};
		//bool validateDiffusionStatistics();

		inline int getBaselineLeftNumber()		{   return baselineLeftNumber;};
		inline int getBValueLeftNumber()		{   return bValueLeftNumber;};
		inline int getGradientDirLeftNumber()	{   return gradientDirLeftNumber;};
		inline int getGradientLeftNumber()		{   return gradientLeftNumber;};
		inline std::vector<int> getRepetitionLeftNumber()	{   return repetitionLeftNumber;};
		//unsigned char  validateLeftDiffusionStatistics();
		// 00000CBA:  
		// A: Gradient direction # is less than 6! 
		// B: Single b-value DWI without a b0/baseline!
		// C: Too many bad gradient directions found!
		// 0: valid
	};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDWIQCInterlaceChecker.cpp"
#endif

#endif
