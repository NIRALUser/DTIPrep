
#include "itkImage.h"
#include "itkVectorImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiffusionTensor3DReconstructionImageFilter.h"

#include "Protocal.h"
#include "QCResult.h"

#include <iostream>
#include <string>
//#include <QObject>
class CIntensityMotionCheck //: public QObject
{
	//Q_OBJECT

public:
	CIntensityMotionCheck(std::string filename,std::string ReportFileName="");
	CIntensityMotionCheck(void);
	~CIntensityMotionCheck(void);

	struct DiffusionDir
	{
		std::vector< double > gradientDir;
		int repetitionNumber;
	};


	typedef unsigned short DwiPixelType;
	typedef itk::Image<DwiPixelType, 2>			SliceImageType;
	typedef itk::Image<DwiPixelType, 3>			GradientImageType;
	typedef itk::VectorImage<DwiPixelType, 3>	DwiImageType;    
	typedef itk::ImageFileReader<DwiImageType>	DwiReaderType;
	typedef itk::ImageFileWriter<DwiImageType>	DwiWriterType;

	typedef itk::DiffusionTensor3DReconstructionImageFilter< DwiPixelType, DwiPixelType, double >		TensorReconstructionImageFilterType;
	typedef	TensorReconstructionImageFilterType::GradientDirectionContainerType							GradientDirectionContainerType;

	void SetFileName(std::string filename) {DwiFileName = filename; };
	
	void GetImagesInformation();
	unsigned int GetGradientsNumber() { return numGradients;};

	bool GetGridentDirections();
	//bool GetGridentImages();

	unsigned char  CheckByProtocal();

	bool ImageCheck();
	bool DiffusionCheck();
	bool IntraCheck( );
	bool InterlaceCheck();
	bool InterCheck();
	void GenerateCheckOutputImage();
	void GenerateCheckOutputImage( std::string filename);
	void EddyMotionCorrection();

	bool dtiestim();
	bool CropDTI();
	bool dtiprocess();
	bool DTIComputing();

	bool IntraCheck( 
		bool bRegister, 
		double beginSkip, 
		double endSkip, 
		double baselineCorrelationThreshold ,  
		double baselineCorrelationDeviationThreshold, 
		double CorrelationThreshold ,  
		double CorrelationDeviationThreshold
		);

	bool InterlaceCheck(
		double angleThreshold, 
		double transThreshold, 
		double correlationThresholdBaseline, 
		double correlationThresholdGradient,
		double corrBaselineDev,	
		double corrGradientDev
		);


	bool InterCheck(
		unsigned int BinNumb, 
		double PercentagePixel, 
		bool UseExplicitPDFDerivatives, 
		double angleThreshold,
		double transThreshold
		);

	void SetProtocal(Protocal *p) 
	{ 
		protocal = p;

		if( DwiFileName.length() != 0 && ReportFileName.length() == 0 )
		{
			ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
			if( protocal->GetReportFileName().length() != 0)
				ReportFileName.append( protocal->GetReportFileName() );// "IntensityMotionCheckReports.txt"		
			else
				ReportFileName.append( "_QC_CheckReports.txt");
		}
	};
	void SetQCResult(QCResult *r) { qcResult = r;};

	QCResult *GetQCResult() { return qcResult;};

	void collectDiffusionStatistics();
	int getBaselineNumber()		{   return baselineNumber;};
	int getBValueNumber()		{   return bValueNumber;};
	int getGradientDirNumber()	{   return gradientDirNumber;};
	int getRepetitionNumber()	{   return repetitionNumber;};
	int getGradientNumber()		{   return gradientNumber;};
	bool validateDiffusionStatistics();

	void collectLeftDiffusionStatistics();
	int getBaselineLeftNumber()		{   return baselineLeftNumber;};
	int getBValueLeftNumber()		{   return bValueLeftNumber;};
	int getGradientDirLeftNumber()	{   return gradientDirLeftNumber;};
	int getGradientLeftNumber()		{   return gradientLeftNumber;};
	std::vector<int> getRepetitionLeftNumber()	{   return repetitionLeftNumber;};
	unsigned char  validateLeftDiffusionStatistics();	// 00000CBA:  
														// A: Gradient direction # is less than 6! 
														// B: Single b-value DWI without a b0/baseline!
														// C: Too many bad gradient directions found!
														// 0: valid


	void PrintResult();

//signals:
 //   void Progress( int );

	
private:
	bool LoadDwiImage();
	bool bDwiLoaded;

	int baselineNumber;
	int bValueNumber;
	int gradientDirNumber;
	int repetitionNumber;
	int gradientNumber;

	int baselineLeftNumber;
	int bValueLeftNumber;
	int gradientDirLeftNumber;
	int gradientLeftNumber;
	std::vector<int> repetitionLeftNumber;


	bool bGetGridentDirections;
//	bool bGetGridentImages;

	std::string DwiFileName;
	std::string ReportFileName;

//	struSettings IntraSettings;
//	struSettings InterSettings;

	DwiImageType::Pointer DwiImage;    
	DwiReaderType::Pointer DwiReader;

	unsigned int numGradients;

	//std::vector<GradientImageType::Pointer>    GradientImageContainer;
	GradientDirectionContainerType::Pointer	   GradientDirectionContainer;

	//for all gradients  slice wise correlation
	std::vector<double> means;
	std::vector<double> deviations;

	//for all baseline slice wise correlation
	std::vector<double> baselineMeans;
	std::vector<double> baselineDeviations;

	//for interlace baseline correlation
	double interlaceBaselineMeans;
	double interlaceBaselineDeviations;

	//for interlace gradient correlation
	double interlaceGradientMeans;
	double interlaceGradientDeviations;

	//std::vector<struIntra2DResults>    Results;
	//std::vector<struIntra2DResults>    ResultsContainer;

	Protocal *protocal;
	QCResult *qcResult;

	bool readb0 ;
	double b0 ;

};
