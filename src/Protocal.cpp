#include "Protocal.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

Protocal::Protocal(void)
{
	initProtocals();
}

Protocal::~Protocal(void)
{
}

// init protocols
void Protocal::initProtocals()
{
	this->QCOutputDirectory = "";
	this->QCedDWIFileNameSuffix = "_QCed.nhdr"; 
	this->reportFileNameSuffix = "_QCReport.txt"; 
	this->badGradientPercentageTolerance = 0.2;

	initImageProtocal();
	initDiffusionProtocal();
	initSliceCheckProtocal();
	initInterlaceCheckProtocal();
	initBaselineAverageProtocal();
	initEddyMotionCorrectionProtocal();
	initGradientCheckProtocal();
	initDTIProtocal();
}

void Protocal::initImageProtocal()
{
	imageProtocal.bCheck = true;
	imageProtocal.type = Protocal::TYPE_USHORT;
	imageProtocal.space = Protocal::SPACE_LPS;

	imageProtocal.dimension = 3;
	imageProtocal.size[0]	= 128;		
	imageProtocal.size[1]	= 128;
	imageProtocal.size[2]	= 65;

	imageProtocal.origin[0]	= 0.0;
	imageProtocal.origin[1]	= 0.0;
	imageProtocal.origin[2]	= 0.0;

	imageProtocal.spacing[0]= 2.00;
	imageProtocal.spacing[1]= 2.00;
	imageProtocal.spacing[2]= 2.00;

	imageProtocal.spacedirection[0][0] = 1.0;
	imageProtocal.spacedirection[0][1] = 0.0;
	imageProtocal.spacedirection[0][2] = 0.0;
	imageProtocal.spacedirection[1][0] = 0.0;
	imageProtocal.spacedirection[1][1] = 1.0;
	imageProtocal.spacedirection[1][2] = 0.0;
	imageProtocal.spacedirection[2][0] = 0.0;
	imageProtocal.spacedirection[2][1] = 0.0;
	imageProtocal.spacedirection[2][2] = 1.0;

	imageProtocal.bCrop = true;
	imageProtocal.croppedDWIFileNameSuffix = "_CroppedDWI.nhdr";

	imageProtocal.reportFileNameSuffix = "_Report.txt";
	imageProtocal.reportFileMode = 1; // 0: new   1: append
}

void Protocal::initDiffusionProtocal()
{
	diffusionProtocal.bCheck = true;
	diffusionProtocal.bUseDiffusionProtocal = true;

	diffusionProtocal.diffusionReplacedDWIFileNameSuffix = "_DiffusionReplaced.nhdr";
	diffusionProtocal.reportFileNameSuffix = "_Report.txt";
	diffusionProtocal.reportFileMode = 1; // 0: new   1: append
}

void Protocal::initSliceCheckProtocal()
{
	sliceCheckProtocal.bCheck = true;

	sliceCheckProtocal.checkTimes = 0;
	sliceCheckProtocal.headSkipSlicePercentage = 0.1;
	sliceCheckProtocal.tailSkipSlicePercentage = 0.1;
	sliceCheckProtocal.correlationDeviationThresholdbaseline = 3.0;
	sliceCheckProtocal.correlationDeviationThresholdgradient = 3.5;

	sliceCheckProtocal.outputDWIFileNameSuffix = "";
	sliceCheckProtocal.reportFileNameSuffix = "_Report.txt";
	sliceCheckProtocal.reportFileMode = 1; // 0: new   1: append
}

void Protocal::initInterlaceCheckProtocal()
{
	interlaceCheckProtocal.bCheck = true;

	interlaceCheckProtocal.correlationThresholdBaseline = 0.9;
	interlaceCheckProtocal.correlationThresholdGradient = 0.9;
	interlaceCheckProtocal.correlationDeviationBaseline = 2.5;
	interlaceCheckProtocal.correlationDeviationGradient = 3.0;
	interlaceCheckProtocal.translationThreshold = 2.0;
	interlaceCheckProtocal.rotationThreshold = 0.5;

	interlaceCheckProtocal.outputDWIFileNameSuffix = "";
	interlaceCheckProtocal.reportFileNameSuffix = "_Report.txt";
	interlaceCheckProtocal.reportFileMode = 1; // 0: new   1: append
}

void Protocal::initGradientCheckProtocal()
{
	gradientCheckProtocal.bCheck = true;

	gradientCheckProtocal.translationThreshold = 2.0;
	gradientCheckProtocal.rotationThreshold = 0.5;

	gradientCheckProtocal.outputDWIFileNameSuffix = "";
	gradientCheckProtocal.reportFileNameSuffix = "_Report.txt";
	gradientCheckProtocal.reportFileMode = 1; // 0: new   1: append
}

void Protocal::initBaselineAverageProtocal()
{
	baselineAverageProtocal.bAverage = true;

	baselineAverageProtocal.averageMethod = 1; // baseline optimized
	baselineAverageProtocal.stopThreshold = 0.02;

	baselineAverageProtocal.outputDWIFileNameSuffix = "";
	baselineAverageProtocal.reportFileNameSuffix = "_Report.txt";
	baselineAverageProtocal.reportFileMode = 1; // 0: new   1: append
}

void Protocal::initEddyMotionCorrectionProtocal()
{
	eddyMotionCorrectionProtocal.bCorrect = true;

	eddyMotionCorrectionProtocal.numberOfBins		= 24;
	eddyMotionCorrectionProtocal.numberOfSamples	= 100000;
	eddyMotionCorrectionProtocal.translationScale	= 0.001;
	eddyMotionCorrectionProtocal.stepLength			= 0.1;
	eddyMotionCorrectionProtocal.relaxFactor		= 0.5;
	eddyMotionCorrectionProtocal.maxNumberOfIterations	= 500;

	eddyMotionCorrectionProtocal.outputDWIFileNameSuffix = "";
	eddyMotionCorrectionProtocal.reportFileNameSuffix = "_Report.txt";
	eddyMotionCorrectionProtocal.reportFileMode = 1; // 0: new   1: append
}
void Protocal::initDTIProtocal()
{
	dTIProtocal.bCompute = true;
	dTIProtocal.dtiestimCommand = "dtiestim";
	dTIProtocal.dtiprocessCommand = "dtiprocess";

	dTIProtocal.method = Protocal::METHOD_WLS;
	dTIProtocal.baselineThreshold = 50; //for neoates
	dTIProtocal.mask = "";

	dTIProtocal.tensorSuffix = "_tensor.nrrd";

	dTIProtocal.faSuffix = "_fa.nrrd";
	dTIProtocal.mdSuffix = "_md.nrrd";
	dTIProtocal.coloredfaSuffix = "_coloredfa.nrrd";
	dTIProtocal.baselineSuffix = "_baseline.nrrd";
	dTIProtocal.frobeniusnormSuffix = "_frobeniusnorm.nrrd";
	dTIProtocal.idwiSuffix = "_idwi.nrrd";

	dTIProtocal.bfa = true;
	dTIProtocal.bmd = true;
	dTIProtocal.bcoloredfa = true;
	dTIProtocal.bbaseline = true;
	dTIProtocal.bfrobeniusnorm = true;
	dTIProtocal.bidwi = true;

	dTIProtocal.reportFileNameSuffix = "_Report.txt";
	dTIProtocal.reportFileMode = 1; // 0: new   1: append
}

// print protocols
void Protocal::printProtocals()
{
	std::cout<<"================================"<<std::endl;
	std::cout<<"          QC Protocal           "<<std::endl;
	std::cout<<"================================"<<std::endl;
	
	std::cout<<"\tQCOutputDirectory: "<< GetQCOutputDirectory() << std::endl;
	std::cout<<"\tQCedDWIFileNameSuffix: "<< GetQCedDWIFileNameSuffix() << std::endl;
	std::cout<<"\tReportFileNameSuffix: "<< GetReportFileNameSuffix() << std::endl;
	std::cout<<"\tBadGradientPercentageTolerance: "<< GetBadGradientPercentageTolerance() << std::endl;

	printImageProtocal();
	printDiffusionProtocal();
	printSliceCheckProtocal();
	printInterlaceCheckProtocal();
	printBaselineAverageProtocal();
	printEddyMotionCorrectionProtocal();
	printGradientCheckProtocal();
	printDTIProtocal();
}

void Protocal::printImageProtocal()
{
	std::cout<<"================================"<<std::endl;
	std::cout<<"         Image Protocal         "<<std::endl;
	std::cout<<"================================"<<std::endl;

	if( GetImageProtocal().bCheck )
		std::cout<<"\tbCheck: Yes"<<std::endl;
	else
		std::cout<<"\tbCheck: No"<<std::endl;
	
	switch( GetImageProtocal().type )
	{
	case Protocal::TYPE_SHORT:
		std::cout<<"\ttype: short"<<std::endl;
		break;
	case Protocal::TYPE_USHORT:
		std::cout<<"\ttype: unsigned short"<<std::endl;
		break;
	case Protocal::TYPE_UNKNOWN:
	default:
		std::cout<<"\ttype: UNKNOWN"<<std::endl;
		break;
	}
	
	switch( GetImageProtocal().space )
	{
	case Protocal::SPACE_LAI:
		std::cout<<"\tspace: left-anterior-inferior"<<std::endl;
		break;
	case Protocal::SPACE_LAS:
		std::cout<<"\tspace: left-anterior-superior"<<std::endl;
		break;
	case Protocal::SPACE_LPI:
		std::cout<<"\tspace: left-posterior-inferior"<<std::endl;
		break;
	case Protocal::SPACE_LPS:
		std::cout<<"\tspace: left-posterior-superior"<<std::endl;
		break;
	case Protocal::SPACE_RAI:
		std::cout<<"\tspace: right-anterior-inferior"<<std::endl;
		break;
	case Protocal::SPACE_RAS:
		std::cout<<"\tspace: right-anterior-superior"<<std::endl;
		break;
	case Protocal::SPACE_RPI:
		std::cout<<"\tspace: right-posterior-inferior"<<std::endl;
		break;
	case Protocal::SPACE_RPS:
		std::cout<<"\tspace: right-posterior-superior"<<std::endl;
		break;
	case Protocal::SPACE_UNKNOWN:
	default:
		std::cout<<"\tspace: UNKNOWN"<<std::endl;
		break;
	}

	std::cout<<"\tspace directions: ";
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			std::cout<< GetImageProtocal().spacedirection[i][j]<<" ";
	std::cout<<std::endl;

	std::cout<<"\tdimension: "	<< GetImageProtocal().dimension<<std::endl;

	std::cout<<"\tsize: "		<< GetImageProtocal().size[0]<<" "
								<< GetImageProtocal().size[1]<<" "
								<< GetImageProtocal().size[2]<<" "
								<< GetImageProtocal().size[3]<<std::endl;

	std::cout<<"\tspacing: "	<< GetImageProtocal().spacing[0]<<" "
								<< GetImageProtocal().spacing[1]<<" "
								<< GetImageProtocal().spacing[2]<<std::endl;

	std::cout<<"\torigin: "		<< GetImageProtocal().origin[0]<<" "
								<< GetImageProtocal().origin[1]<<" "
								<< GetImageProtocal().origin[2]<<std::endl;

	if( GetImageProtocal().bCrop )
		std::cout<<"\tbCrop: Yes"<<std::endl;
	else
		std::cout<<"\tbCrop: No"<<std::endl;

	std::cout<<"\tcroppedDWIFileNameSuffix: "<<  GetImageProtocal().croppedDWIFileNameSuffix <<std::endl;

	std::cout<<"\treportFileNameSuffix: "<<  GetImageProtocal().reportFileNameSuffix <<std::endl;

	if( GetImageProtocal().reportFileMode == 0 ) 
		std::cout<<"\treportFileMode: new"<<std::endl;

	if( GetImageProtocal().reportFileMode == 1 )  
		std::cout<<"\treportFileMode: append"<<std::endl;

	std::cout<<std::endl;
}

void Protocal::printDiffusionProtocal()
{
	std::cout<<"================================"<<std::endl;
	std::cout<<"       Diffusion Protocal       "<<std::endl;
	std::cout<<"================================"<<std::endl;

	if( GetDiffusionProtocal().bCheck )
		std::cout<<"\tbCheck: Yes"<<std::endl;
	else
		std::cout<<"\tbCheck: No"<<std::endl;

	std::cout<<"\tmeasurementFrame: ";
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			std::cout<< GetDiffusionProtocal().measurementFrame[i][j]<<" ";
	std::cout<<std::endl;

	std::cout<<"\tDWMRI_b-value: "<< GetDiffusionProtocal().bValue<<std::endl;

	for( unsigned int i=0;i< GetDiffusionProtocal().gradients.size();i++ )
	{
		std::vector<double> vect;
		vect  = GetDiffusionProtocal().gradients[i];
		//std::cout<<"\tDWMRI_gradient_"<<i<<": "<<vect[0]<<" "<<vect[1]<<" "<<vect[2]<<std::endl;

		std::cout<<"\tDWMRI_gradient_"<<i<<"\t[ " 
			<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
			<<vect[0]<<", "
			<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
			<<vect[1]<<", "
			<<std::setw(9)<<std::setiosflags(std::ios::fixed)<< std::setprecision(6)<<std::setiosflags(std::ios::right)
			<<vect[2]<<" ]"
			<<std::endl;
	}


	if( GetDiffusionProtocal().bUseDiffusionProtocal )
		std::cout<<"\tbuseDiffusionProtocal: Yes"<<std::endl;
	else
		std::cout<<"\tuseDiffusionProtocal: No"<<std::endl;

	std::cout<<"\tdiffusionReplacedDWIFileNameSuffix: "<<  GetDiffusionProtocal().diffusionReplacedDWIFileNameSuffix <<std::endl;

	std::cout<<"\treportFileNameSuffix: "<<  GetDiffusionProtocal().reportFileNameSuffix <<std::endl;

	if( GetDiffusionProtocal().reportFileMode == 0 ) 
		std::cout<<"\treportFileMode: new"<<std::endl;

	if( GetDiffusionProtocal().reportFileMode == 1 )  
		std::cout<<"\treportFileMode: append"<<std::endl;

	std::cout<<std::endl;
}

void Protocal::printSliceCheckProtocal()
{
	std::cout<<"================================"<<std::endl;
	std::cout<<"      Slice Check Protocal      "<<std::endl;
	std::cout<<"================================"<<std::endl;

	if( GetSliceCheckProtocal().bCheck )
		std::cout<<"\tbCheck: Yes"<<std::endl;
	else
		std::cout<<"\tbCheck: No"<<std::endl;

	std::cout<<"\tcheckTimes: "<< GetSliceCheckProtocal().checkTimes<<std::endl;

// 	std::cout<<"\tbadGradientPercentageTolerance: "<< GetSliceCheckProtocal().badGradientPercentageTolerance<<std::endl;

	std::cout<<"\theadSkipSlicePercentage: "<< GetSliceCheckProtocal().headSkipSlicePercentage <<std::endl;
	std::cout<<"\ttailSkipSlicePercentage: "<< GetSliceCheckProtocal().tailSkipSlicePercentage<<std::endl;

	std::cout<<"\tcorrelationDeviationThresholdbaseline: "<< GetSliceCheckProtocal().correlationDeviationThresholdbaseline<<std::endl;
	std::cout<<"\tcorrelationDeviationThresholdgradient: "<< GetSliceCheckProtocal().correlationDeviationThresholdgradient<<std::endl;

	std::cout<<"\toutputDWIFileNameSuffix: "<<  GetSliceCheckProtocal().outputDWIFileNameSuffix <<std::endl;
	std::cout<<"\treportFileNameSuffix: "<<  GetSliceCheckProtocal().reportFileNameSuffix <<std::endl;

	if( GetSliceCheckProtocal().reportFileMode == 0 ) 
		std::cout<<"\treportFileMode: new"<<std::endl;

	if( GetSliceCheckProtocal().reportFileMode == 1 )  
		std::cout<<"\treportFileMode: append"<<std::endl;

	std::cout<<std::endl;
}

void Protocal::printInterlaceCheckProtocal()
{
	std::cout<<"================================"<<std::endl;
	std::cout<<"    Interlace Check Protocal    "<<std::endl;
	std::cout<<"================================"<<std::endl;

	if( GetInterlaceCheckProtocal().bCheck )
		std::cout<<"\tbCheck: Yes"<<std::endl;
	else
		std::cout<<"\tbCheck: No"<<std::endl;

// 	std::cout<<"\tbadGradientPercentageTolerance: "<< GetInterlaceCheckProtocal().badGradientPercentageTolerance<<std::endl;

	std::cout<<"\tcorrelationThresholdBaseline: "<< GetInterlaceCheckProtocal().correlationThresholdBaseline<<std::endl;
	std::cout<<"\tcorrelationDeviationBaseline: "<< GetInterlaceCheckProtocal().correlationDeviationBaseline<<std::endl;
	std::cout<<"\tcorrelationThresholdGradient: "<< GetInterlaceCheckProtocal().correlationThresholdGradient<<std::endl;
	std::cout<<"\tcorrelationDeviationGradient: "<< GetInterlaceCheckProtocal().correlationDeviationGradient<<std::endl;
	std::cout<<"\ttranslationThreshold: "<< GetInterlaceCheckProtocal().translationThreshold<<std::endl;
	std::cout<<"\trotationThreshold: "<< GetInterlaceCheckProtocal().rotationThreshold<<std::endl;

	std::cout<<"\toutputDWIFileNameSuffix: "<<  GetInterlaceCheckProtocal().outputDWIFileNameSuffix <<std::endl;
	std::cout<<"\treportFileNameSuffix: "<<  GetInterlaceCheckProtocal().reportFileNameSuffix <<std::endl;

	if( GetInterlaceCheckProtocal().reportFileMode == 0 ) 
		std::cout<<"\treportFileMode: new"<<std::endl;

	if( GetInterlaceCheckProtocal().reportFileMode == 1 )  
		std::cout<<"\treportFileMode: append"<<std::endl;

	std::cout<<std::endl;
}

void Protocal::printGradientCheckProtocal()
{
	std::cout<<"================================"<<std::endl;
	std::cout<<"     Gradient Check Protocal    "<<std::endl;
	std::cout<<"================================"<<std::endl;

	if( GetGradientCheckProtocal().bCheck )
		std::cout<<"\tbCheck: Yes"<<std::endl;
	else
		std::cout<<"\tbCheck: No"<<std::endl;

// 	std::cout<<"\tbadGradientPercentageTolerance: "<< GetGradientCheckProtocal().badGradientPercentageTolerance<<std::endl;

	std::cout<<"\tgradientTranslationThreshold: "<< GetGradientCheckProtocal().translationThreshold<<std::endl;
	std::cout<<"\tgradientRotationThreshold: "<< GetGradientCheckProtocal().rotationThreshold<<std::endl;

	std::cout<<"\toutputDWIFileNameSuffix: "<<  GetGradientCheckProtocal().outputDWIFileNameSuffix <<std::endl;
	std::cout<<"\treportFileNameSuffix: "<<  GetGradientCheckProtocal().reportFileNameSuffix <<std::endl;

	if( GetGradientCheckProtocal().reportFileMode == 0 ) 
		std::cout<<"\treportFileMode: new"<<std::endl;

	if( GetGradientCheckProtocal().reportFileMode == 1 )  
		std::cout<<"\treportFileMode: append"<<std::endl;

	std::cout<<std::endl;
}

void Protocal::printBaselineAverageProtocal()
{
	std::cout<<"================================"<<std::endl;
	std::cout<<"    Baseline Average Protocal   "<<std::endl;
	std::cout<<"================================"<<std::endl;

	if( GetBaselineAverageProtocal().bAverage )
		std::cout<<"\tbAverage: Yes"<<std::endl;
	else
		std::cout<<"\tbAverage: No"<<std::endl;

	switch( GetBaselineAverageProtocal().averageMethod )
	{
	case 0:
		std::cout<<"\taverageMethod: direct"<<std::endl;
		break;
	case 1:
		std::cout<<"\taverageMethod: baselines optimized"<<std::endl;
		break;
	case 2:
		std::cout<<"\taverageMethod: gradients optimized"<<std::endl;
		break;
	default:
		break;
	}

	std::cout<<"\tstopThreshold: "<< GetBaselineAverageProtocal().stopThreshold <<std::endl;

	std::cout<<"\toutputDWIFileNameSuffix: "<<  GetBaselineAverageProtocal().outputDWIFileNameSuffix <<std::endl;
	std::cout<<"\treportFileNameSuffix: "<<  GetBaselineAverageProtocal().reportFileNameSuffix <<std::endl;

	if( GetBaselineAverageProtocal().reportFileMode == 0 ) 
		std::cout<<"\treportFileMode: new"<<std::endl;

	if( GetBaselineAverageProtocal().reportFileMode == 1 )  
		std::cout<<"\treportFileMode: append"<<std::endl;

	std::cout<<std::endl;
}

void Protocal::printEddyMotionCorrectionProtocal()
{
	std::cout<<"================================"<<std::endl;
	std::cout<<" Eddy-Motion Correction Protocal"<<std::endl;
	std::cout<<"================================"<<std::endl;

	if( GetEddyMotionCorrectionProtocal().bCorrect )
		std::cout<<"\tbCorrect: Yes"<<std::endl;
	else
		std::cout<<"\tbCorrect: No"<<std::endl;

// 	std::cout<<"\tEddyCurrentCommand: "<< GetEddyMotionCorrectionProtocal().EddyMotionCommand<< std::endl;
//	std::cout<<"\tInputFileName: "<< GetEddyMotionCorrectionProtocal().InputFileName<< std::endl;
//	std::cout<<"\tOutputFileName: "<< GetEddyMotionCorrectionProtocal().OutputFileName<< std::endl;

	std::cout<<"\tnumberOfBins: "		<< GetEddyMotionCorrectionProtocal().numberOfBins << std::endl;
	std::cout<<"\tnumberOfSamples: "	<< GetEddyMotionCorrectionProtocal().numberOfSamples << std::endl;
	std::cout<<"\ttranslationScale: "	<< GetEddyMotionCorrectionProtocal().translationScale << std::endl;
	std::cout<<"\tstepLength: "			<< GetEddyMotionCorrectionProtocal().stepLength << std::endl;
	std::cout<<"\trelaxFactor: "		<< GetEddyMotionCorrectionProtocal().relaxFactor << std::endl;
	std::cout<<"\tmaxNumberOfIterations: "<< GetEddyMotionCorrectionProtocal().maxNumberOfIterations << std::endl;

	std::cout<<"\toutputDWIFileNameSuffix: "<<  GetEddyMotionCorrectionProtocal().outputDWIFileNameSuffix <<std::endl;
	std::cout<<"\treportFileNameSuffix: "<<  GetEddyMotionCorrectionProtocal().reportFileNameSuffix <<std::endl;

	if( GetEddyMotionCorrectionProtocal().reportFileMode == 0 ) 
		std::cout<<"\treportFileMode: new"<<std::endl;

	if( GetEddyMotionCorrectionProtocal().reportFileMode == 1 )  
		std::cout<<"\treportFileMode: append"<<std::endl;

	std::cout<<std::endl;
}

void Protocal::printDTIProtocal()
{
	std::cout<<"================================"<<std::endl;
	std::cout<<"     DTI Computing Protocal     "<<std::endl;
	std::cout<<"================================"<<std::endl;

	if( GetDTIProtocal().bCompute )
		std::cout<<"\tbCompute: Yes"<<std::endl;
	else
		std::cout<<"\tbCompute: No"<<std::endl;

	std::cout<<"\tdtiestimCommand: "<< GetDTIProtocal().dtiestimCommand << std::endl;
	std::cout<<"\tdtiprocessCommand: "<< GetDTIProtocal().dtiprocessCommand<< std::endl;

	switch( GetDTIProtocal().method )
	{
	case Protocal::METHOD_WLS:
		std::cout<<"\tmethod: wls"<<std::endl;
		break;
	case Protocal::METHOD_LLS:
		std::cout<<"\tmethod: lls"<<std::endl;
		break;
	case Protocal::METHOD_ML:
		std::cout<<"\tmethod: ml"<<std::endl;
		break;
	case Protocal::METHOD_UNKNOWN:
	default:
		std::cout<<"\tmethod: UNKNOWN"<<std::endl;
		break;
	}

	std::cout<<"\ttensorSuffix: "<<GetDTIProtocal().tensorSuffix<<std::endl;

	std::cout<<"\tbaselineThreshold: "<< GetDTIProtocal().baselineThreshold<<std::endl;
	std::cout<<"\tmask: "<< GetDTIProtocal().mask <<std::endl;

	if( GetDTIProtocal().bfa )
		std::cout<<"\tbfa: Yes"<<std::endl;
	else
		std::cout<<"\tbfa: No"<<std::endl;
	std::cout<<"\tfaSuffix: "<<GetDTIProtocal().faSuffix<<std::endl;

	if( GetDTIProtocal().bmd )
		std::cout<<"\tbmd: Yes"<<std::endl;
	else
		std::cout<<"\tbmd: No"<<std::endl;
	std::cout<<"\tmdSuffix: "<<GetDTIProtocal().mdSuffix<<std::endl;

	if( GetDTIProtocal().bcoloredfa )
		std::cout<<"\tbcoloredfa: Yes"<<std::endl;
	else
		std::cout<<"\tbcoloredfa: No"<<std::endl;
	std::cout<<"\tcoloredfaSuffix: "<<GetDTIProtocal().coloredfaSuffix<<std::endl;

	if( GetDTIProtocal().bbaseline )
		std::cout<<"\tbbaseline: Yes"<<std::endl;
	else
		std::cout<<"\tbbaseline: No"<<std::endl;
	std::cout<<"\tbaselineSuffix: "<<GetDTIProtocal().baselineSuffix<<std::endl;

	if( GetDTIProtocal().bfrobeniusnorm )
		std::cout<<"\tbfrobeniusnorm: Yes"<<std::endl;
	else
		std::cout<<"\tbfrobeniusnorm: No"<<std::endl;
	std::cout<<"\tfrobeniusnormSuffix: "<<GetDTIProtocal().frobeniusnormSuffix<<std::endl;

	if( GetDTIProtocal().bidwi )
		std::cout<<"\tbidwi: Yes"<<std::endl;
	else
		std::cout<<"\tbidwi: No"<<std::endl;
	std::cout<<"\tidwiSuffix: "<<GetDTIProtocal().idwiSuffix<<std::endl;

	if( GetDTIProtocal().reportFileMode == 0 ) 
		std::cout<<"\treportFileMode: new"<<std::endl;

	if( GetDTIProtocal().reportFileMode == 1 )  
		std::cout<<"\treportFileMode: append"<<std::endl;

	std::cout<<std::endl;
}



void Protocal::clear()
{
	GetDiffusionProtocal().gradients.clear();

	GetImageProtocal().bCheck					= false;
	GetDiffusionProtocal().bCheck				= false;
	GetSliceCheckProtocal().bCheck				= false;
	GetInterlaceCheckProtocal().bCheck			= false;
	GetBaselineAverageProtocal().bAverage		= false;
	GetEddyMotionCorrectionProtocal().bCorrect	= false;
	GetGradientCheckProtocal().bCheck			= false;
	GetDTIProtocal().bCompute					= false;

	baselineNumber		= 1;
	bValueNumber		= 1;
	repetitionNumber	= 1;
	gradientDirNumber	= 0;
}

void Protocal::collectDiffusionStatistics()
{
	std::vector<DiffusionDir> DiffusionDirections;
	DiffusionDirections.clear();

	for( unsigned int i=0; i<this->GetDiffusionProtocal().gradients.size();i++)
	{
		if(DiffusionDirections.size()>0)
		{
			bool newDir = true;
			for(unsigned int j=0;j<DiffusionDirections.size();j++)
			{
				if( this->GetDiffusionProtocal().gradients[i][0] == DiffusionDirections[j].gradientDir[0] && 
					this->GetDiffusionProtocal().gradients[i][1] == DiffusionDirections[j].gradientDir[1] && 
					this->GetDiffusionProtocal().gradients[i][2] == DiffusionDirections[j].gradientDir[2] )
				{
					DiffusionDirections[j].repetitionNumber++;
					newDir = false;;
				}
			}
			if(newDir)
			{
				std::vector< double > dir;
				dir.push_back(this->GetDiffusionProtocal().gradients[i][0]);
				dir.push_back(this->GetDiffusionProtocal().gradients[i][1]);
				dir.push_back(this->GetDiffusionProtocal().gradients[i][2]);

				DiffusionDir diffusionDir;
				diffusionDir.gradientDir = dir;
				diffusionDir.repetitionNumber=1;

				DiffusionDirections.push_back(diffusionDir);
			}
		}
		else
		{
			std::vector< double > dir;
			dir.push_back(this->GetDiffusionProtocal().gradients[i][0]);
			dir.push_back(this->GetDiffusionProtocal().gradients[i][1]);
			dir.push_back(this->GetDiffusionProtocal().gradients[i][2]);

			DiffusionDir diffusionDir;
			diffusionDir.gradientDir = dir;
			diffusionDir.repetitionNumber=1;

			DiffusionDirections.push_back(diffusionDir);
		}
	}

	std::vector<int> repetNum;
	repetNum.clear();
	std::vector<double> dirMode;
	dirMode.clear();

	for( unsigned int i=0; i<DiffusionDirections.size(); i++)
	{
		if( DiffusionDirections[i].gradientDir[0] == 0.0 &&
			DiffusionDirections[i].gradientDir[1] == 0.0 &&
			DiffusionDirections[i].gradientDir[2] == 0.0 ) 
		{
			this->baselineNumber = DiffusionDirections[i].repetitionNumber;
		}
		else
		{
			repetNum.push_back(DiffusionDirections[i].repetitionNumber);

			double modeSqr =	DiffusionDirections[i].gradientDir[0]*DiffusionDirections[i].gradientDir[0] +
								DiffusionDirections[i].gradientDir[1]*DiffusionDirections[i].gradientDir[1] +
								DiffusionDirections[i].gradientDir[2]*DiffusionDirections[i].gradientDir[2];

			if( dirMode.size() > 0)
			{
				bool newDirMode = true;
				for(unsigned int j=0;j< dirMode.size();j++)
				{
					if( fabs(modeSqr-dirMode[j])<0.001)   // 1 DIFFERENCE for b value
					{
						newDirMode = false;	
						break;
					}
				}
				if(newDirMode)
				{
					dirMode.push_back(	modeSqr) ;
				}
			}
			else
			{
				dirMode.push_back(	modeSqr) ;
			}
		}
	}

// 	std::cout<<" repetNum.size(): " <<  repetNum.size() <<std::endl;
// 	std::cout<<" dirMode.size(): " <<  dirMode.size() <<std::endl;

	this->gradientDirNumber = repetNum.size();
	this->bValueNumber = dirMode.size();

	repetitionNumber = repetNum[0];
	for( unsigned int i=1; i<repetNum.size(); i++)
	{ 
		if( repetNum[i] != repetNum[0])
		{
			std::cout<<"Protocol error. Not all the gradient directions have same repetition. "<<std::endl;
			repetitionNumber = -1;			
		}
	}

// 	std::cout<<"Protocol Diffusion: "	<<std::endl;
// 	std::cout<<"  baselineNumber: "	<<baselineNumber	<<std::endl;
// 	std::cout<<"  bValueNumber: "		<<bValueNumber		<<std::endl;
// 	std::cout<<"  gradientDirNumber: "<<gradientDirNumber	<<std::endl;
// 	std::cout<<"  repetitionNumber: "	<<repetitionNumber	<<std::endl;

	return ;
}
