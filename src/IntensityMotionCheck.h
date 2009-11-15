
#include "itkImage.h"
#include "itkVectorImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiffusionTensor3DReconstructionImageFilter.h"

#include "Protocol.h"
#include "QCResult.h"

#include <iostream>
#include <string>

#include "itkNrrdImageIO.h"

#include "itkDWICropper.h"
#include "itkDWIQCSliceChecker.h"
#include "itkDWIQCInterlaceChecker.h"
#include "itkDWIBaselineAverager.h"
#include "itkDWIQCGradientChecker.h"

#include "itkDWIEddyCurrentHeadMotionCorrector.h" //eddy-motion Utah
#include "itkVectorImageRegisterAffineFilter.h" // eddy-motion IOWA


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

	typedef itk::DWICropper<DwiImageType>							CropperType;
	typedef itk::DWIQCSliceChecker<DwiImageType>					SliceCheckerType;
	typedef itk::DWIQCInterlaceChecker<DwiImageType>				InterlaceCheckerType;
	typedef itk::DWIBaselineAverager<DwiImageType>					BaselineAveragerType;
	typedef itk::DWIQCGradientChecker<DwiImageType>					GradientCheckerType;

	typedef itk::DWIEddyCurrentHeadMotionCorrector<DwiImageType>	EddyMotionCorrectorType; //eddy-motion Utah
	typedef itk::VectorImageRegisterAffineFilter<DwiImageType, DwiImageType >	EddyMotionCorrectorTypeIowa;  //eddy-motion Iowa

	DwiWriterType::Pointer		DwiWriter;
	itk::NrrdImageIO::Pointer	NrrdImageIO;

	CropperType::Pointer				Cropper	;
	SliceCheckerType::Pointer			SliceChecker;
	InterlaceCheckerType::Pointer		InterlaceChecker;
	BaselineAveragerType::Pointer		BaselineAverager;
	GradientCheckerType::Pointer		GradientChecker;
	EddyMotionCorrectorType::Pointer	EddyMotionCorrector; //eddy-motion Utah
	EddyMotionCorrectorTypeIowa::Pointer	EddyMotionCorrectorIowa; //eddy-motion Iowa

	void SetFileName(std::string filename) {DwiFileName = filename; };
	
	void GetImagesInformation();
	unsigned int GetGradientsNumber() { return numGradients;};
	
	bool GetGridentDirections();
	bool GetGridentDirections( DwiImageType::Pointer dwi,double &bValue, GradientDirectionContainerType::Pointer	GradDireContainer);

	unsigned char  CheckByProtocol(); //old

	bool ImageCheck( DwiImageType::Pointer dwi );
	bool DiffusionCheck( DwiImageType::Pointer dwi );
	bool SliceWiseCheck( DwiImageType::Pointer dwi );
	bool InterlaceWiseCheck( DwiImageType::Pointer dwi );
	bool BaselineAverage( DwiImageType::Pointer dwi );
 	bool EddyMotionCorrect( DwiImageType::Pointer dwi );
	bool EddyMotionCorrectIowa( DwiImageType::Pointer dwi );
	bool GradientWiseCheck( DwiImageType::Pointer dwi );
	bool SaveQCedDWI(DwiImageType::Pointer dwi);
	void collectLeftDiffusionStatistics( DwiImageType::Pointer dwi, std::string reportfilename );
// 	bool DTIComputing(std::string input);

	bool ImageCheck();
	bool DiffusionCheck();
	bool SliceWiseCheck();
	bool InterlaceWiseCheck();
	bool BaselineAverage();
 	bool EddyMotionCorrect();
	bool GradientWiseCheck();
	void collectLeftDiffusionStatistics( int dumb);
	bool SaveQCedDWI();

	bool dtiestim();
	bool DTIComputing();
	bool dtiprocess();


	void GenerateCheckOutputImage( std::string filename);

	void SetProtocol(Protocol *p) { protocol = p;};
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

	unsigned char  RunPipelineByProtocol();

//signals:
 //   void Progress( int );


public:
	inline DwiImageType::Pointer GetDwiImage(){ return DwiImage;};
	inline DwiImageType::Pointer GetDwiImageTemp(){ return DwiImageTemp;};
	inline GradientDirectionContainerType::Pointer GetGradientDirectionContainer(){ return GradientDirectionContainer;};
	inline bool GetDwiLoadStatus(){ return bDwiLoaded;};
	inline std::string GetDwiFileName(){ return DwiFileName;};
	bool LoadDwiImage();
private:
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

	std::string DwiFileName;
	std::string ReportFileName;

	DwiImageType::Pointer DwiImageTemp;    

	DwiImageType::Pointer DwiImage;    
	DwiReaderType::Pointer DwiReader;

	unsigned int numGradients;

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


	Protocol *protocol;
	QCResult *qcResult;

	bool readb0 ;
	double b0 ;

};
