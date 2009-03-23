
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

	bool GetGridentDirections();
	bool GetGridentImages();

	bool CheckByProtocal();

	bool ImageCheck();
	bool DiffusionCheck();
	bool IntraCheck( );
	bool InterlaceCheck();
	bool InterCheck();
	void GenerateCheckOutputImage();
	void EddyMotionCorrection();
	bool DTIComputing();

	bool IntraCheck( bool bRegister, double CorrelationThreshold ,  double CorrelationDeviationThreshold);
	bool InterlaceCheck(double angleThreshold, double transThreshold, double correlationThresholdBaseline, double correlationThresholdGradient);
	bool InterCheck(unsigned int BinNumb, double PercentagePixel, bool UseExplicitPDFDerivatives, double angleThreshold,double transThreshold);

	void SetProtocal(Protocal *p) { protocal = p;};
	void SetQCResult(QCResult *r) { qcResult = r;};

//signals:
 //   void Progress( int );

	
private:
	bool LoadDwiImage();
	bool bDwiLoaded;
	bool bGetGridentDirections;
	bool bGetGridentImages;

	std::string DwiFileName;
	std::string ReportFileName;

//	struSettings IntraSettings;
//	struSettings InterSettings;

	DwiImageType::Pointer DwiImage;    
	DwiReaderType::Pointer DwiReader;

	std::vector<GradientImageType::Pointer>    GradientImageContainer;
	GradientDirectionContainerType::Pointer	   GradientDirectionContainer;

	//for all gradients (baseline excluded) slice wise correlation
	std::vector<double> means;
	std::vector<double> deviations;


	//std::vector<struIntra2DResults>    Results;
	//std::vector<struIntra2DResults>    ResultsContainer;

	Protocal *protocal;
	QCResult *qcResult;

	bool readb0 ;
	double b0 ;

};
