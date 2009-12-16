#include "Protocol.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

Protocol::Protocol(void)
{
	initProtocols();
}

Protocol::~Protocol(void)
{}

// init protocols
void Protocol::initProtocols()
{
	this->QCOutputDirectory = "";
	this->QCedDWIFileNameSuffix = "_QCed.nhdr";
	this->reportFileNameSuffix = "_QCReport.txt";
	this->m_BadGradientPercentageTolerance = 0.2;
	this->m_ReportType = REPORT_TYPE_VERBOSE;

	initImageProtocol();
	initDiffusionProtocol();
	initSliceCheckProtocol();
	initInterlaceCheckProtocol();
	initBaselineAverageProtocol();
	initEddyMotionCorrectionProtocol();
	initGradientCheckProtocol();
	initDTIProtocol();
}

void Protocol::initImageProtocol()
{
	imageProtocol.bCheck = true;
	imageProtocol.type = Protocol::TYPE_USHORT;
	imageProtocol.space = Protocol::SPACE_LPS;

	imageProtocol.dimension = 3;
	imageProtocol.size[0]  = 128;
	imageProtocol.size[1]  = 128;
	imageProtocol.size[2]  = 65;

	imageProtocol.origin[0]  = 0.0;
	imageProtocol.origin[1]  = 0.0;
	imageProtocol.origin[2]  = 0.0;

	imageProtocol.spacing[0] = 2.00;
	imageProtocol.spacing[1] = 2.00;
	imageProtocol.spacing[2] = 2.00;

	imageProtocol.spacedirection[0][0] = 1.0;
	imageProtocol.spacedirection[0][1] = 0.0;
	imageProtocol.spacedirection[0][2] = 0.0;
	imageProtocol.spacedirection[1][0] = 0.0;
	imageProtocol.spacedirection[1][1] = 1.0;
	imageProtocol.spacedirection[1][2] = 0.0;
	imageProtocol.spacedirection[2][0] = 0.0;
	imageProtocol.spacedirection[2][1] = 0.0;
	imageProtocol.spacedirection[2][2] = 1.0;

	imageProtocol.bCrop = true;
	imageProtocol.croppedDWIFileNameSuffix = "_CroppedDWI.nhdr";

	imageProtocol.reportFileNameSuffix = "_Report.txt";
	imageProtocol.reportFileMode = 1; // 0: new   1: append

	imageProtocol.bQuitOnCheckSpacingFailure = true;
	imageProtocol.bQuitOnCheckSizeFailure = false;
}

void Protocol::initDiffusionProtocol()
{
	diffusionProtocol.bCheck = true;
	diffusionProtocol.bUseDiffusionProtocol = false;

	diffusionProtocol.diffusionReplacedDWIFileNameSuffix
		= "_DiffusionReplaced.nhdr";
	diffusionProtocol.reportFileNameSuffix = "_Report.txt";
	diffusionProtocol.reportFileMode = 1; // 0: new   1: append

	diffusionProtocol.bQuitOnCheckFailure = true;
}

void Protocol::initSliceCheckProtocol()
{
	sliceCheckProtocol.bCheck = true;

	sliceCheckProtocol.bSubregionalCheck = false;
	sliceCheckProtocol.subregionalCheckRelaxationFactor = 1.1;

	sliceCheckProtocol.checkTimes = 0;
	sliceCheckProtocol.headSkipSlicePercentage = 0.1;
	sliceCheckProtocol.tailSkipSlicePercentage = 0.1;
	sliceCheckProtocol.correlationDeviationThresholdbaseline = 3.0;
	sliceCheckProtocol.correlationDeviationThresholdgradient = 3.5;

	sliceCheckProtocol.outputDWIFileNameSuffix = "";
	sliceCheckProtocol.reportFileNameSuffix = "_Report.txt";
	sliceCheckProtocol.reportFileMode = 1; // 0: new   1: append

	std::string excludedDWINrrdFileNameSuffix = "_SliceWiseExcluded.nrrd";
	sliceCheckProtocol.bQuitOnCheckFailure = true;
}

void Protocol::initInterlaceCheckProtocol()
{
	interlaceCheckProtocol.bCheck = true;

	interlaceCheckProtocol.correlationThresholdBaseline = 0.9;
	interlaceCheckProtocol.correlationThresholdGradient = 0.9;
	interlaceCheckProtocol.correlationDeviationBaseline = 2.5;
	interlaceCheckProtocol.correlationDeviationGradient = 3.0;
	interlaceCheckProtocol.translationThreshold = 2.0;
	interlaceCheckProtocol.rotationThreshold = 0.5;

	interlaceCheckProtocol.outputDWIFileNameSuffix = "";
	interlaceCheckProtocol.reportFileNameSuffix = "_Report.txt";
	interlaceCheckProtocol.reportFileMode = 1; // 0: new   1: append

	std::string excludedDWINrrdFileNameSuffix = "_InterlaceWiseExcluded.nrrd";
	interlaceCheckProtocol.bQuitOnCheckFailure = true;
}

void Protocol::initGradientCheckProtocol()
{
	gradientCheckProtocol.bCheck = true;

	gradientCheckProtocol.translationThreshold = 2.0;
	gradientCheckProtocol.rotationThreshold = 0.5;

	gradientCheckProtocol.outputDWIFileNameSuffix = "";
	gradientCheckProtocol.reportFileNameSuffix = "_Report.txt";
	gradientCheckProtocol.reportFileMode = 1; // 0: new   1: append

	std::string excludedDWINrrdFileNameSuffix = "_GradientWiseExcluded.nrrd";
	gradientCheckProtocol.bQuitOnCheckFailure = true;
}

void Protocol::initBaselineAverageProtocol()
{
	baselineAverageProtocol.bAverage = true;

	baselineAverageProtocol.averageMethod = 1; // baseline optimized
	baselineAverageProtocol.stopThreshold = 0.02;

	baselineAverageProtocol.outputDWIFileNameSuffix = "";
	baselineAverageProtocol.reportFileNameSuffix = "_Report.txt";
	baselineAverageProtocol.reportFileMode = 1; // 0: new   1: append
}

void Protocol::initEddyMotionCorrectionProtocol()
{
	eddyMotionCorrectionProtocol.bCorrect = true;

	eddyMotionCorrectionProtocol.numberOfBins    = 24;
	eddyMotionCorrectionProtocol.numberOfSamples  = 100000;
	eddyMotionCorrectionProtocol.translationScale  = 0.001;
	eddyMotionCorrectionProtocol.stepLength      = 0.1;
	eddyMotionCorrectionProtocol.relaxFactor    = 0.5;
	eddyMotionCorrectionProtocol.maxNumberOfIterations  = 500;

	eddyMotionCorrectionProtocol.outputDWIFileNameSuffix = "";
	eddyMotionCorrectionProtocol.reportFileNameSuffix = "_Report.txt";
	eddyMotionCorrectionProtocol.reportFileMode = 1; // 0: new   1: append
}

void Protocol::initDTIProtocol()
{
	dTIProtocol.bCompute = true;
	dTIProtocol.dtiestimCommand = "dtiestim";
	dTIProtocol.dtiprocessCommand = "dtiprocess";

	dTIProtocol.method = Protocol::METHOD_WLS;
	dTIProtocol.baselineThreshold = 50; // for neoates
	dTIProtocol.mask = "";

	dTIProtocol.tensorSuffix = "_tensor.nrrd";

	dTIProtocol.faSuffix = "_fa.nrrd";
	dTIProtocol.mdSuffix = "_md.nrrd";
	dTIProtocol.coloredfaSuffix = "_coloredfa.nrrd";
	dTIProtocol.baselineSuffix = "_baseline.nrrd";
	dTIProtocol.frobeniusnormSuffix = "_frobeniusnorm.nrrd";
	dTIProtocol.idwiSuffix = "_idwi.nrrd";

	dTIProtocol.bfa = true;
	dTIProtocol.bmd = true;
	dTIProtocol.bcoloredfa = true;
	dTIProtocol.bbaseline = true;
	dTIProtocol.bfrobeniusnorm = true;
	dTIProtocol.bidwi = true;

	dTIProtocol.reportFileNameSuffix = "_Report.txt";
	dTIProtocol.reportFileMode = 1; // 0: new   1: append
}

// print protocols
void Protocol::printProtocols()
{
	std::cout << "================================" << std::endl;
	std::cout << "          QC Protocol           " << std::endl;
	std::cout << "================================" << std::endl;

	std::cout << "\tQCOutputDirectory: " << GetQCOutputDirectory() << std::endl;
	std::cout << "\tQCedDWIFileNameSuffix: " << GetQCedDWIFileNameSuffix()
		<< std::endl;
	std::cout << "\tReportFileNameSuffix: " << GetReportFileNameSuffix()
		<< std::endl;
	std::cout << "\tBadGradientPercentageTolerance: "
		<< GetBadGradientPercentageTolerance() << std::endl;

	switch(GetReportType())
	{
	case REPORT_TYPE_SIMPLE:
		std::cout << "\treportType: SIMPLE"<< std::endl;
		break;
	case REPORT_TYPE_EASY_PARSE:
		std::cout << "\treportType: EASY_PARSE"<< std::endl;
		break;
	case REPORT_TYPE_VERBOSE:
	default:
		std::cout << "\treportType: VERBOSE"<< std::endl;
		break;
	}

	printImageProtocol();
	printDiffusionProtocol();
	printSliceCheckProtocol();
	printInterlaceCheckProtocol();
	printBaselineAverageProtocol();
	printEddyMotionCorrectionProtocol();
	printGradientCheckProtocol();
	printDTIProtocol();
}

void Protocol::printImageProtocol()
{
	std::cout << "================================" << std::endl;
	std::cout << "         Image Protocol         " << std::endl;
	std::cout << "================================" << std::endl;

	if ( GetImageProtocol().bCheck )
	{
		std::cout << "\tbCheck: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbCheck: No" << std::endl;
	}

	switch ( GetImageProtocol().type )
	{
	case Protocol::TYPE_SHORT:
		std::cout << "\ttype: short" << std::endl;
		break;
	case Protocol::TYPE_USHORT:
		std::cout << "\ttype: unsigned short" << std::endl;
		break;
	case Protocol::TYPE_UNKNOWN:
	default:
		std::cout << "\ttype: UNKNOWN" << std::endl;
		break;
	}

	switch ( GetImageProtocol().space )
	{
	case Protocol::SPACE_LAI:
		std::cout << "\tspace: left-anterior-inferior" << std::endl;
		break;
	case Protocol::SPACE_LAS:
		std::cout << "\tspace: left-anterior-superior" << std::endl;
		break;
	case Protocol::SPACE_LPI:
		std::cout << "\tspace: left-posterior-inferior" << std::endl;
		break;
	case Protocol::SPACE_LPS:
		std::cout << "\tspace: left-posterior-superior" << std::endl;
		break;
	case Protocol::SPACE_RAI:
		std::cout << "\tspace: right-anterior-inferior" << std::endl;
		break;
	case Protocol::SPACE_RAS:
		std::cout << "\tspace: right-anterior-superior" << std::endl;
		break;
	case Protocol::SPACE_RPI:
		std::cout << "\tspace: right-posterior-inferior" << std::endl;
		break;
	case Protocol::SPACE_RPS:
		std::cout << "\tspace: right-posterior-superior" << std::endl;
		break;
	case Protocol::SPACE_UNKNOWN:
	default:
		std::cout << "\tspace: UNKNOWN" << std::endl;
		break;
	}

	std::cout << "\tspace directions: ";
	for ( int i = 0; i < 3; i++ )
	{
		for ( int j = 0; j < 3; j++ )
		{
			std::cout << GetImageProtocol().spacedirection[i][j] << " ";
		}
	}
	std::cout << std::endl;

	std::cout << "\tdimension: "  << GetImageProtocol().dimension << std::endl;

	std::cout << "\tsize: "    << GetImageProtocol().size[0] << " "
		<< GetImageProtocol().size[1] << " "
		<< GetImageProtocol().size[2] << " "
		<< GetImageProtocol().size[3] << std::endl;

	std::cout << "\tspacing: "  << GetImageProtocol().spacing[0] << " "
		<< GetImageProtocol().spacing[1] << " "
		<< GetImageProtocol().spacing[2] << std::endl;

	std::cout << "\torigin: "    << GetImageProtocol().origin[0] << " "
		<< GetImageProtocol().origin[1] << " "
		<< GetImageProtocol().origin[2] << std::endl;

	if ( GetImageProtocol().bCrop )
	{
		std::cout << "\tbCrop: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbCrop: No" << std::endl;
	}

	std::cout << "\tcroppedDWIFileNameSuffix: "
		<<  GetImageProtocol().croppedDWIFileNameSuffix << std::endl;

	std::cout << "\treportFileNameSuffix: "
		<<  GetImageProtocol().reportFileNameSuffix << std::endl;

	if ( GetImageProtocol().reportFileMode == 0 )
	{
		std::cout << "\treportFileMode: new" << std::endl;
	}

	if ( GetImageProtocol().reportFileMode == 1 )
	{
		std::cout << "\treportFileMode: append" << std::endl;
	}

	if ( GetImageProtocol().bQuitOnCheckSpacingFailure)
	{
		std::cout << "\tbQuitOnCheckSpacingFailure: true" << std::endl;
	}
	else
	{
		std::cout << "\tbQuitOnCheckSpacingFailure: false" << std::endl;
	}

	if ( GetImageProtocol().bQuitOnCheckSizeFailure)
	{
		std::cout << "\tbQuitOnCheckSizeFailure: true" << std::endl;
	}
	else
	{
		std::cout << "\tbQuitOnCheckSizeFailure: false" << std::endl;
	} 

	std::cout << std::endl;
}

void Protocol::printDiffusionProtocol()
{
	std::cout << "================================" << std::endl;
	std::cout << "       Diffusion Protocol       " << std::endl;
	std::cout << "================================" << std::endl;

	if ( GetDiffusionProtocol().bCheck )
	{
		std::cout << "\tbCheck: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbCheck: No" << std::endl;
	}

	std::cout << "\tmeasurementFrame: ";
	for ( int i = 0; i < 3; i++ )
	{
		for ( int j = 0; j < 3; j++ )
		{
			std::cout << GetDiffusionProtocol().measurementFrame[i][j] << " ";
		}
	}
	std::cout << std::endl;

	std::cout << "\tDWMRI_b-value: " << GetDiffusionProtocol().bValue
		<< std::endl;

	for ( unsigned int i = 0; i < GetDiffusionProtocol().gradients.size(); i++ )
	{
		vnl_vector_fixed<double, 3> vect;
		vect  = GetDiffusionProtocol().gradients[i];
		// std::cout<<"\tDWMRI_gradient_"<<i<<": "<<vect[0]<<" "<<vect[1]<<"
		// "<<vect[2]<<std::endl;

		std::cout << "\tDWMRI_gradient_" << i << "\t[ "
			<< std::setw(9) << std::setiosflags(std::ios::fixed)
			<< std::setprecision(6) << std::setiosflags(std::ios::right)
			<< vect[0] << ", "
			<< std::setw(9) << std::setiosflags(std::ios::fixed)
			<< std::setprecision(6) << std::setiosflags(std::ios::right)
			<< vect[1] << ", "
			<< std::setw(9) << std::setiosflags(std::ios::fixed)
			<< std::setprecision(6) << std::setiosflags(std::ios::right)
			<< vect[2] << " ]"
			<< std::endl;
	}

	if ( GetDiffusionProtocol().bUseDiffusionProtocol )
	{
		std::cout << "\tbuseDiffusionProtocol: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tuseDiffusionProtocol: No" << std::endl;
	}

	std::cout << "\tdiffusionReplacedDWIFileNameSuffix: "
		<<  GetDiffusionProtocol().diffusionReplacedDWIFileNameSuffix
		<< std::endl;

	std::cout << "\treportFileNameSuffix: "
		<<  GetDiffusionProtocol().reportFileNameSuffix << std::endl;

	if ( GetDiffusionProtocol().reportFileMode == 0 )
	{
		std::cout << "\treportFileMode: new" << std::endl;
	}

	if ( GetDiffusionProtocol().reportFileMode == 1 )
	{
		std::cout << "\treportFileMode: append" << std::endl;
	}

	if ( GetDiffusionProtocol().bQuitOnCheckFailure)
	{
		std::cout << "\tbQuitOnCheckFailure: true" << std::endl;
	}
	else
	{
		std::cout << "\tbQuitOnCheckFailure: false" << std::endl;
	} 

	std::cout << std::endl;
}

void Protocol::printSliceCheckProtocol()
{
	std::cout << "================================" << std::endl;
	std::cout << "      Slice Check Protocol      " << std::endl;
	std::cout << "================================" << std::endl;

	if ( GetSliceCheckProtocol().bCheck )
	{
		std::cout << "\tbCheck: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbCheck: No" << std::endl;
	}

	if ( GetSliceCheckProtocol().bSubregionalCheck )
	{
		std::cout << "\tbSubregionalCheck: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbSubregionalCheck: No" << std::endl;
	}

	std::cout << "\tsubregionalCheckRelaxationFactor: " 
		<< GetSliceCheckProtocol().subregionalCheckRelaxationFactor
		<< std::endl;

	std::cout << "\tcheckTimes: " << GetSliceCheckProtocol().checkTimes
		<< std::endl;

	//   std::cout<<"\tbadGradientPercentageTolerance: "<<
	// GetSliceCheckProtocol().m_BadGradientPercentageTolerance<<std::endl;

	std::cout << "\theadSkipSlicePercentage: "
		<< GetSliceCheckProtocol().headSkipSlicePercentage << std::endl;
	std::cout << "\ttailSkipSlicePercentage: "
		<< GetSliceCheckProtocol().tailSkipSlicePercentage << std::endl;

	std::cout << "\tcorrelationDeviationThresholdbaseline: "
		<< GetSliceCheckProtocol().correlationDeviationThresholdbaseline
		<< std::endl;
	std::cout << "\tcorrelationDeviationThresholdgradient: "
		<< GetSliceCheckProtocol().correlationDeviationThresholdgradient
		<< std::endl;

	std::cout << "\toutputDWIFileNameSuffix: "
		<<  GetSliceCheckProtocol().outputDWIFileNameSuffix << std::endl;
	std::cout << "\treportFileNameSuffix: "
		<<  GetSliceCheckProtocol().reportFileNameSuffix << std::endl;

	if ( GetSliceCheckProtocol().reportFileMode == 0 )
	{
		std::cout << "\treportFileMode: new" << std::endl;
	}

	if ( GetSliceCheckProtocol().reportFileMode == 1 )
	{
		std::cout << "\treportFileMode: append" << std::endl;
	}

	std::cout << "\texcludedDWINrrdFileNameSuffix: "
		<<  GetSliceCheckProtocol().excludedDWINrrdFileNameSuffix << std::endl;

	if ( GetSliceCheckProtocol().bQuitOnCheckFailure)
	{
		std::cout << "\tbQuitOnCheckFailure: true" << std::endl;
	}
	else
	{
		std::cout << "\tbQuitOnCheckFailure: false" << std::endl;
	} 

	std::cout << std::endl;
}

void Protocol::printInterlaceCheckProtocol()
{
	std::cout << "================================" << std::endl;
	std::cout << "    Interlace Check Protocol    " << std::endl;
	std::cout << "================================" << std::endl;

	if ( GetInterlaceCheckProtocol().bCheck )
	{
		std::cout << "\tbCheck: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbCheck: No" << std::endl;
	}

	//   std::cout<<"\tbadGradientPercentageTolerance: "<<
	// GetInterlaceCheckProtocol().m_BadGradientPercentageTolerance<<std::endl;

	std::cout << "\tcorrelationThresholdBaseline: "
		<< GetInterlaceCheckProtocol().correlationThresholdBaseline
		<< std::endl;
	std::cout << "\tcorrelationDeviationBaseline: "
		<< GetInterlaceCheckProtocol().correlationDeviationBaseline
		<< std::endl;
	std::cout << "\tcorrelationThresholdGradient: "
		<< GetInterlaceCheckProtocol().correlationThresholdGradient
		<< std::endl;
	std::cout << "\tcorrelationDeviationGradient: "
		<< GetInterlaceCheckProtocol().correlationDeviationGradient
		<< std::endl;
	std::cout << "\ttranslationThreshold: "
		<< GetInterlaceCheckProtocol().translationThreshold << std::endl;
	std::cout << "\trotationThreshold: "
		<< GetInterlaceCheckProtocol().rotationThreshold << std::endl;

	std::cout << "\toutputDWIFileNameSuffix: "
		<<  GetInterlaceCheckProtocol().outputDWIFileNameSuffix
		<< std::endl;
	std::cout << "\treportFileNameSuffix: "
		<<  GetInterlaceCheckProtocol().reportFileNameSuffix << std::endl;

	if ( GetInterlaceCheckProtocol().reportFileMode == 0 )
	{
		std::cout << "\treportFileMode: new" << std::endl;
	}

	if ( GetInterlaceCheckProtocol().reportFileMode == 1 )
	{
		std::cout << "\treportFileMode: append" << std::endl;
	}

	std::cout << "\texcludedDWINrrdFileNameSuffix: "
		<<  GetInterlaceCheckProtocol().excludedDWINrrdFileNameSuffix << std::endl;

	if ( GetInterlaceCheckProtocol().bQuitOnCheckFailure)
	{
		std::cout << "\tbQuitOnCheckFailure: true" << std::endl;
	}
	else
	{
		std::cout << "\tbQuitOnCheckFailure: false" << std::endl;
	} 

	std::cout << std::endl;
}

void Protocol::printGradientCheckProtocol()
{
	std::cout << "================================" << std::endl;
	std::cout << "     Gradient Check Protocol    " << std::endl;
	std::cout << "================================" << std::endl;

	if ( GetGradientCheckProtocol().bCheck )
	{
		std::cout << "\tbCheck: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbCheck: No" << std::endl;
	}

	//   std::cout<<"\tbadGradientPercentageTolerance: "<<
	// GetGradientCheckProtocol().m_BadGradientPercentageTolerance<<std::endl;

	std::cout << "\tgradientTranslationThreshold: "
		<< GetGradientCheckProtocol().translationThreshold << std::endl;
	std::cout << "\tgradientRotationThreshold: "
		<< GetGradientCheckProtocol().rotationThreshold << std::endl;

	std::cout << "\toutputDWIFileNameSuffix: "
		<<  GetGradientCheckProtocol().outputDWIFileNameSuffix << std::endl;
	std::cout << "\treportFileNameSuffix: "
		<<  GetGradientCheckProtocol().reportFileNameSuffix << std::endl;

	if ( GetGradientCheckProtocol().reportFileMode == 0 )
	{
		std::cout << "\treportFileMode: new" << std::endl;
	}

	if ( GetGradientCheckProtocol().reportFileMode == 1 )
	{
		std::cout << "\treportFileMode: append" << std::endl;
	}

	std::cout << "\texcludedDWINrrdFileNameSuffix: "
		<<  GetGradientCheckProtocol().excludedDWINrrdFileNameSuffix << std::endl;

	if ( GetGradientCheckProtocol().bQuitOnCheckFailure)
	{
		std::cout << "\tbQuitOnCheckFailure: true" << std::endl;
	}
	else
	{
		std::cout << "\tbQuitOnCheckFailure: false" << std::endl;
	} 

	std::cout << std::endl;
}

void Protocol::printBaselineAverageProtocol()
{
	std::cout << "================================" << std::endl;
	std::cout << "    Baseline Average Protocol   " << std::endl;
	std::cout << "================================" << std::endl;

	if ( GetBaselineAverageProtocol().bAverage )
	{
		std::cout << "\tbAverage: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbAverage: No" << std::endl;
	}

	switch ( GetBaselineAverageProtocol().averageMethod )
	{
	case 0:
		std::cout << "\taverageMethod: direct" << std::endl;
		break;
	case 1:
		std::cout << "\taverageMethod: baselines optimized" << std::endl;
		break;
	case 2:
		std::cout << "\taverageMethod: gradients optimized" << std::endl;
		break;
	default:
		break;
	}

	std::cout << "\tstopThreshold: "
		<< GetBaselineAverageProtocol().stopThreshold << std::endl;

	std::cout << "\toutputDWIFileNameSuffix: "
		<<  GetBaselineAverageProtocol().outputDWIFileNameSuffix
		<< std::endl;
	std::cout << "\treportFileNameSuffix: "
		<<  GetBaselineAverageProtocol().reportFileNameSuffix << std::endl;

	if ( GetBaselineAverageProtocol().reportFileMode == 0 )
	{
		std::cout << "\treportFileMode: new" << std::endl;
	}

	if ( GetBaselineAverageProtocol().reportFileMode == 1 )
	{
		std::cout << "\treportFileMode: append" << std::endl;
	}

	std::cout << std::endl;
}

void Protocol::printEddyMotionCorrectionProtocol()
{
	std::cout << "================================" << std::endl;
	std::cout << " Eddy-Motion Correction Protocol" << std::endl;
	std::cout << "================================" << std::endl;

	if ( GetEddyMotionCorrectionProtocol().bCorrect )
	{
		std::cout << "\tbCorrect: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbCorrect: No" << std::endl;
	}

	//   std::cout<<"\tEddyCurrentCommand: "<<
	// GetEddyMotionCorrectionProtocol().EddyMotionCommand<< std::endl;
	//  std::cout<<"\tInputFileName: "<<
	// GetEddyMotionCorrectionProtocol().InputFileName<< std::endl;
	//  std::cout<<"\tOutputFileName: "<<
	// GetEddyMotionCorrectionProtocol().OutputFileName<< std::endl;

	std::cout << "\tnumberOfBins: "
		<< GetEddyMotionCorrectionProtocol().numberOfBins << std::endl;
	std::cout << "\tnumberOfSamples: "
		<< GetEddyMotionCorrectionProtocol().numberOfSamples << std::endl;
	std::cout << "\ttranslationScale: "
		<< GetEddyMotionCorrectionProtocol().translationScale << std::endl;
	std::cout << "\tstepLength: "
		<< GetEddyMotionCorrectionProtocol().stepLength << std::endl;
	std::cout << "\trelaxFactor: "
		<< GetEddyMotionCorrectionProtocol().relaxFactor << std::endl;
	std::cout << "\tmaxNumberOfIterations: "
		<< GetEddyMotionCorrectionProtocol().maxNumberOfIterations
		<< std::endl;

	std::cout << "\toutputDWIFileNameSuffix: "
		<<  GetEddyMotionCorrectionProtocol().outputDWIFileNameSuffix
		<< std::endl;
	std::cout << "\treportFileNameSuffix: "
		<<  GetEddyMotionCorrectionProtocol().reportFileNameSuffix
		<< std::endl;

	if ( GetEddyMotionCorrectionProtocol().reportFileMode == 0 )
	{
		std::cout << "\treportFileMode: new" << std::endl;
	}

	if ( GetEddyMotionCorrectionProtocol().reportFileMode == 1 )
	{
		std::cout << "\treportFileMode: append" << std::endl;
	}

	std::cout << std::endl;
}

void Protocol::printDTIProtocol()
{
	std::cout << "================================" << std::endl;
	std::cout << "     DTI Computing Protocol     " << std::endl;
	std::cout << "================================" << std::endl;

	if ( GetDTIProtocol().bCompute )
	{
		std::cout << "\tbCompute: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbCompute: No" << std::endl;
	}

	std::cout << "\tdtiestimCommand: " << GetDTIProtocol().dtiestimCommand
		<< std::endl;
	std::cout << "\tdtiprocessCommand: " << GetDTIProtocol().dtiprocessCommand
		<< std::endl;

	switch ( GetDTIProtocol().method )
	{
	case Protocol::METHOD_WLS:
		std::cout << "\tmethod: wls" << std::endl;
		break;
	case Protocol::METHOD_LLS:
		std::cout << "\tmethod: lls" << std::endl;
		break;
	case Protocol::METHOD_ML:
		std::cout << "\tmethod: ml" << std::endl;
		break;
	case Protocol::METHOD_UNKNOWN:
	default:
		std::cout << "\tmethod: UNKNOWN" << std::endl;
		break;
	}

	std::cout << "\ttensorSuffix: " << GetDTIProtocol().tensorSuffix << std::endl;

	std::cout << "\tbaselineThreshold: " << GetDTIProtocol().baselineThreshold
		<< std::endl;
	std::cout << "\tmask: " << GetDTIProtocol().mask << std::endl;

	if ( GetDTIProtocol().bfa )
	{
		std::cout << "\tbfa: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbfa: No" << std::endl;
	}
	std::cout << "\tfaSuffix: " << GetDTIProtocol().faSuffix << std::endl;

	if ( GetDTIProtocol().bmd )
	{
		std::cout << "\tbmd: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbmd: No" << std::endl;
	}
	std::cout << "\tmdSuffix: " << GetDTIProtocol().mdSuffix << std::endl;

	if ( GetDTIProtocol().bcoloredfa )
	{
		std::cout << "\tbcoloredfa: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbcoloredfa: No" << std::endl;
	}
	std::cout << "\tcoloredfaSuffix: " << GetDTIProtocol().coloredfaSuffix
		<< std::endl;

	if ( GetDTIProtocol().bbaseline )
	{
		std::cout << "\tbbaseline: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbbaseline: No" << std::endl;
	}
	std::cout << "\tbaselineSuffix: " << GetDTIProtocol().baselineSuffix
		<< std::endl;

	if ( GetDTIProtocol().bfrobeniusnorm )
	{
		std::cout << "\tbfrobeniusnorm: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbfrobeniusnorm: No" << std::endl;
	}
	std::cout << "\tfrobeniusnormSuffix: "
		<< GetDTIProtocol().frobeniusnormSuffix << std::endl;

	if ( GetDTIProtocol().bidwi )
	{
		std::cout << "\tbidwi: Yes" << std::endl;
	}
	else
	{
		std::cout << "\tbidwi: No" << std::endl;
	}
	std::cout << "\tidwiSuffix: " << GetDTIProtocol().idwiSuffix << std::endl;

	if ( GetDTIProtocol().reportFileMode == 0 )
	{
		std::cout << "\treportFileMode: new" << std::endl;
	}

	if ( GetDTIProtocol().reportFileMode == 1 )
	{
		std::cout << "\treportFileMode: append" << std::endl;
	}

	std::cout << std::endl;
}

void Protocol::clear()
{
	GetDiffusionProtocol().gradients.clear();

	GetImageProtocol().bCheck          = false;
	GetDiffusionProtocol().bCheck        = false;
	GetSliceCheckProtocol().bCheck        = false;
	GetInterlaceCheckProtocol().bCheck      = false;
	GetBaselineAverageProtocol().bAverage    = false;
	GetEddyMotionCorrectionProtocol().bCorrect  = false;
	GetGradientCheckProtocol().bCheck      = false;
	GetDTIProtocol().bCompute          = false;

	baselineNumber    = 1;
	bValueNumber    = 1;
	repetitionNumber  = 1;
	gradientDirNumber  = 0;
}

void Protocol::collectDiffusionStatistics()
{
	std::vector<DiffusionDir> DiffusionDirections;
	DiffusionDirections.clear();

	for ( unsigned int i = 0;
		i < this->GetDiffusionProtocol().gradients.size();
		i++ )
	{
		if ( DiffusionDirections.size() > 0 )
		{
			bool newDir = true;
			for ( unsigned int j = 0; j < DiffusionDirections.size(); j++ )
			{
				if ( this->GetDiffusionProtocol().gradients[i][0] ==
					DiffusionDirections[j].gradientDir[0]
				&& this->GetDiffusionProtocol().gradients[i][1] ==
					DiffusionDirections[j].gradientDir[1]
				&& this->GetDiffusionProtocol().gradients[i][2] ==
					DiffusionDirections[j].gradientDir[2] )
				{
					DiffusionDirections[j].repetitionNumber++;
					newDir = false;
				}
			}
			if ( newDir )
			{
				std::vector<double> dir;
				dir.push_back(this->GetDiffusionProtocol().gradients[i][0]);
				dir.push_back(this->GetDiffusionProtocol().gradients[i][1]);
				dir.push_back(this->GetDiffusionProtocol().gradients[i][2]);

				DiffusionDir diffusionDir;
				diffusionDir.gradientDir = dir;
				diffusionDir.repetitionNumber = 1;

				DiffusionDirections.push_back(diffusionDir);
			}
		}
		else
		{
			std::vector<double> dir;
			dir.push_back(this->GetDiffusionProtocol().gradients[i][0]);
			dir.push_back(this->GetDiffusionProtocol().gradients[i][1]);
			dir.push_back(this->GetDiffusionProtocol().gradients[i][2]);

			DiffusionDir diffusionDir;
			diffusionDir.gradientDir = dir;
			diffusionDir.repetitionNumber = 1;

			DiffusionDirections.push_back(diffusionDir);
		}
	}

	std::vector<int> repetNum;
	repetNum.clear();
	std::vector<double> dirMode;
	dirMode.clear();

	for ( unsigned int i = 0; i < DiffusionDirections.size(); i++ )
	{
		if ( DiffusionDirections[i].gradientDir[0] == 0.0
			&& DiffusionDirections[i].gradientDir[1] == 0.0
			&& DiffusionDirections[i].gradientDir[2] == 0.0 )
		{
			this->baselineNumber = DiffusionDirections[i].repetitionNumber;
		}
		else
		{
			repetNum.push_back(DiffusionDirections[i].repetitionNumber);

			double modeSqr =  DiffusionDirections[i].gradientDir[0]
			* DiffusionDirections[i].gradientDir[0]
			+ DiffusionDirections[i].gradientDir[1]
			* DiffusionDirections[i].gradientDir[1]
			+ DiffusionDirections[i].gradientDir[2]
			* DiffusionDirections[i].gradientDir[2];

			if ( dirMode.size() > 0 )
			{
				bool newDirMode = true;
				for ( unsigned int j = 0; j < dirMode.size(); j++ )
				{
					if ( fabs(modeSqr - dirMode[j]) < 0.001 )   // 1 DIFFERENCE for b
						// value
					{
						newDirMode = false;
						break;
					}
				}
				if ( newDirMode )
				{
					dirMode.push_back(  modeSqr);
				}
			}
			else
			{
				dirMode.push_back(  modeSqr);
			}
		}
	}

	//   std::cout<<" repetNum.size(): " <<  repetNum.size() <<std::endl;
	//   std::cout<<" dirMode.size(): " <<  dirMode.size() <<std::endl;

	this->gradientDirNumber = repetNum.size();
	this->bValueNumber = dirMode.size();

	repetitionNumber = repetNum[0];
	for ( unsigned int i = 1; i < repetNum.size(); i++ )
	{
		if ( repetNum[i] != repetNum[0] )
		{
			std::cout
				<< "Warrning: Not all the gradient directions have same repetition. "
				<< std::endl;
			repetitionNumber = -1;
		}
	}

	//   std::cout<<"Protocol Diffusion: "  <<std::endl;
	//   std::cout<<"  baselineNumber: "  <<baselineNumber  <<std::endl;
	//   std::cout<<"  bValueNumber: "    <<bValueNumber    <<std::endl;
	//   std::cout<<"  gradientDirNumber: "<<gradientDirNumber  <<std::endl;
	//   std::cout<<"  repetitionNumber: "  <<repetitionNumber  <<std::endl;

	return;
}

void Protocol::Save( std::string xml)
{

	std::string xmlFileName = xml;
	std::ofstream outfile;

	if( xmlFileName.substr(xmlFileName.rfind('.')).compare("xml") !=0 )
	{
		xmlFileName.append(".xml");
	}
	outfile.open(xmlFileName.c_str());

	outfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	outfile << "<ProtocolSettings>" << std::endl;
	outfile << "    <entry parameter=\"QC_QCOutputDirectory\"></entry>" << std::endl;
	outfile << "    <entry parameter=\"QC_QCedDWIFileNameSuffix\">" << std::endl;
	outfile << "        <value>" << GetQCedDWIFileNameSuffix() << "</value>" << std::endl;
	outfile << "    </entry>" << std::endl;
	outfile << "    <entry parameter=\"QC_reportFileNameSuffix\">" << std::endl;
	outfile << "        <value>" << GetReportFileNameSuffix() << "</value>" << std::endl;
	outfile << "    </entry>" << std::endl;
	outfile << "    <entry parameter=\"QC_badGradientPercentageTolerance\">" << std::endl;
	outfile << "        <value>" << GetBadGradientPercentageTolerance() << "</value>" << std::endl;
	outfile << "    </entry>" << std::endl;
	outfile << "    <entry parameter=\"QC_reportType\">" << std::endl;
	outfile << "        <value>" << GetReportType() << "</value>" << std::endl;
	outfile << "    </entry>" << std::endl;

	// image
	outfile << "    <entry parameter=\"IMAGE_bCheck\">" << std::endl;
	if( GetImageProtocol().bCheck )
		outfile << "        <value>Yes</value>" << std::endl;
	else
		outfile << "        <value>No</value>" << std::endl;

	outfile << "        <entry parameter=\"IMAGE_space\">" << std::endl;
	switch(GetImageProtocol().space)
	{
	case SPACE_LAI:
		outfile << "            <value> left-anterior-inferior</value>" << std::endl;
		break;
	case SPACE_LAS:
		outfile << "            <value> left-anterior-superior</value>" << std::endl;
		break;
	case SPACE_LPI:
		outfile << "            <value> left-posterior-inferior</value>" << std::endl;
		break;
	case SPACE_LPS:
		outfile << "            <value> left-posterior-superior</value>" << std::endl;
		break;
	case SPACE_RAI:
		outfile << "            <value> right-anterior-inferior</value>" << std::endl;
		break;
	case SPACE_RAS:
		outfile << "            <value> right-anterior-superior</value>" << std::endl;
		break;
	case SPACE_RPI:
		outfile << "            <value> right-posterior-inferior</value>" << std::endl;
		break;
	case SPACE_RPS:
		outfile << "            <value> right-posterior-superior</value>" << std::endl;
		break;
	}  
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"IMAGE_directions\">" << std::endl;
	outfile << "            <value>" 
		<< GetImageProtocol().spacedirection[0][0] << " " 
		<< GetImageProtocol().spacedirection[0][1] << " " 
		<< GetImageProtocol().spacedirection[0][2] << "</value>" << std::endl;
	outfile << "            <value>"
		<< GetImageProtocol().spacedirection[1][0] << " " 
		<< GetImageProtocol().spacedirection[1][1] << " " 
		<< GetImageProtocol().spacedirection[1][2] << "</value>" << std::endl;
	outfile << "            <value>"
		<< GetImageProtocol().spacedirection[2][0] << " " 
		<< GetImageProtocol().spacedirection[2][1] << " " 
		<< GetImageProtocol().spacedirection[2][2] << "</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"IMAGE_size\">" << std::endl;
	outfile << "            <value>" << GetImageProtocol().size[0] << "</value>" << std::endl;
	outfile << "            <value>" << GetImageProtocol().size[1] << "</value>" << std::endl;
	outfile << "            <value>" << GetImageProtocol().size[2] << "</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"IMAGE_spacing\">" << std::endl;
	outfile << "            <value>" << GetImageProtocol().spacing[0] << "</value>" << std::endl;
	outfile << "            <value>" << GetImageProtocol().spacing[0] << "</value>" << std::endl;
	outfile << "            <value>" << GetImageProtocol().spacing[0] << "</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"IMAGE_origin\">" << std::endl;
	outfile << "            <value>" << GetImageProtocol().origin[0] << "</value>" << std::endl;
	outfile << "            <value>" << GetImageProtocol().origin[0] << "</value>" << std::endl;
	outfile << "            <value>" << GetImageProtocol().origin[0] << "</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"IMAGE_bCrop\">" << std::endl;
	if( GetImageProtocol().bCrop )
		outfile << "            <value>Yes</value>" << std::endl;
	else
		outfile << "            <value>No</value>" << std::endl;

	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"IMAGE_croppedDWIFileNameSuffix\">" << std::endl;
	outfile << "            <value>" << GetImageProtocol().croppedDWIFileNameSuffix << "</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"IMAGE_reportFileNameSuffix\">" << std::endl;
	outfile << "            <value>" << GetImageProtocol().reportFileNameSuffix << "</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"IMAGE_reportFileMode\">" << std::endl;
	outfile << "            <value>" << GetImageProtocol().reportFileMode << "</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"IMAGE_bQuitOnCheckSpacingFailure\">" << std::endl;
	if( GetImageProtocol().bQuitOnCheckSpacingFailure )
		outfile << "            <value>Yes</value>" << std::endl;
	else
		outfile << "            <value>No</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"IMAGE_bQuitOnCheckSizeFailure\">" << std::endl;
	if( GetImageProtocol().bQuitOnCheckSizeFailure )
		outfile << "            <value>Yes</value>" << std::endl;
	else
		outfile << "            <value>No</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "    </entry>" << std::endl;

	// diffusion
	outfile << "    <entry parameter=\"DIFFUSION_bCheck\">" << std::endl;
	if( GetDiffusionProtocol().bCheck )
		outfile << "        <value>Yes</value>" << std::endl;
	else
		outfile << "        <value>No</value>" << std::endl;

	outfile << "        <entry parameter=\"DIFFUSION_measurementFrame\">" << std::endl;
	outfile << "            <value>" 
		<< GetDiffusionProtocol().measurementFrame[0][0] << " " 
		<< GetDiffusionProtocol().measurementFrame[0][1] << " " 
		<< GetDiffusionProtocol().measurementFrame[0][2] << "</value>" << std::endl;
	outfile << "            <value>"
		<< GetDiffusionProtocol().measurementFrame[1][0] << " " 
		<< GetDiffusionProtocol().measurementFrame[1][1] << " " 
		<< GetDiffusionProtocol().measurementFrame[1][2] << "</value>" << std::endl;
	outfile << "            <value>"
		<< GetDiffusionProtocol().measurementFrame[2][0] << " " 
		<< GetDiffusionProtocol().measurementFrame[2][1] << " " 
		<< GetDiffusionProtocol().measurementFrame[2][2] << "</value>" << std::endl;

	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DIFFUSION_DWMRI_bValue\">" << std::endl;
	outfile << "            <value>" << GetDiffusionProtocol().bValue << "</value>" << std::endl;
	outfile << "        </entry>" << std::endl;

	for(unsigned int i=0; i< GetDiffusionProtocol().gradients.size();i++  )
	{
		outfile << "        <entry parameter=\"DIFFUSION_DWMRI_gradient_" 
			<< std::setw(4) << std::setfill('0') << i 
			<< "\">" << std::endl;
		outfile << "            <value>" 
			<< GetDiffusionProtocol().gradients[i][0] << " "
			<< GetDiffusionProtocol().gradients[i][1] << " "
			<< GetDiffusionProtocol().gradients[i][2]
		<< "</value>" << std::endl;
		outfile << "        </entry>" << std::endl;
	}



	outfile << "        <entry parameter=\"DIFFUSION_bUseDiffusionProtocol\">" << std::endl;
	outfile << "            <value>Yes</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DIFFUSION_diffusionReplacedDWIFileNameSuffix\">" << std::endl;
	outfile << "            <value>_DiffusionReplaced.nhdr</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DIFFUSION_reportFileNameSuffix\">" << std::endl;
	outfile << "            <value>_QCReport.txt</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DIFFUSION_reportFileMode\">" << std::endl;
	outfile << "            <value>1</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DIFFUSION_bQuitOnCheckFailure\">" << std::endl;
	outfile << "            <value>Yes</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "    </entry>" << std::endl;
	outfile << "    <entry parameter=\"SLICE_bCheck\">" << std::endl;
	outfile << "        <value>Yes</value>" << std::endl;
	outfile << "        <entry parameter=\"SLICE_bSubregionalCheck\">" << std::endl;
	outfile << "            <value>No</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"SLICE_subregionalCheckRelaxationFactor\">" << std::endl;
	outfile << "            <value>1.1000</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"SLICE_checkTimes\">" << std::endl;
	outfile << "            <value>0</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"SLICE_headSkipSlicePercentage\">" << std::endl;
	outfile << "            <value>0.1000</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"SLICE_tailSkipSlicePercentage\">" << std::endl;
	outfile << "            <value>0.1000</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"SLICE_correlationDeviationThresholdbaseline\">" << std::endl;
	outfile << "            <value>3.0000</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"SLICE_correlationDeviationThresholdgradient\">" << std::endl;
	outfile << "            <value>3.5000</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"SLICE_outputDWIFileNameSuffix\"></entry>" << std::endl;
	outfile << "        <entry parameter=\"SLICE_reportFileNameSuffix\">" << std::endl;
	outfile << "            <value>_QCReport.txt</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"SLICE_reportFileMode\">" << std::endl;
	outfile << "            <value>1</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"SLICE_excludedDWINrrdFileNameSuffix\">" << std::endl;
	outfile << "            <value>_slice_exclude.nhdr</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"SLICE_bQuitOnCheckFailure\">" << std::endl;
	outfile << "            <value>Yes</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "    </entry>" << std::endl;
	outfile << "    <entry parameter=\"INTERLACE_bCheck\">" << std::endl;
	outfile << "        <value>Yes</value>" << std::endl;
	outfile << "        <entry parameter=\"INTERLACE_correlationThresholdBaseline\">" << std::endl;
	outfile << "            <value>0.8734</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"INTERLACE_correlationThresholdGradient\">" << std::endl;
	outfile << "            <value>0.8971</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"INTERLACE_correlationDeviationBaseline\">" << std::endl;
	outfile << "            <value>2.5000</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"INTERLACE_correlationDeviationGradient\">" << std::endl;
	outfile << "            <value>3.0000</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"INTERLACE_translationThreshold\">" << std::endl;
	outfile << "            <value>2.5000</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"INTERLACE_rotationThreshold\">" << std::endl;
	outfile << "            <value>0.5000</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"INTERLACE_outputDWIFileNameSuffix\"></entry>" << std::endl;
	outfile << "        <entry parameter=\"INTERLACE_reportFileNameSuffix\">" << std::endl;
	outfile << "            <value>_QCReport.txt</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"INTERLACE_reportFileMode\">" << std::endl;
	outfile << "            <value>1</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"INTERLACE_excludedDWINrrdFileNameSuffix\">" << std::endl;
	outfile << "            <value>_interlace_exclude.nhdr</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"INTERLACE_bQuitOnCheckFailure\">" << std::endl;
	outfile << "            <value>Yes</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "    </entry>" << std::endl;
	outfile << "    <entry parameter=\"BASELINE_bAverage\">" << std::endl;
	outfile << "        <value>Yes</value>" << std::endl;
	outfile << "        <entry parameter=\"BASELINE_averageMethod\">" << std::endl;
	outfile << "            <value>1</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"BASELINE_stopThreshold\">" << std::endl;
	outfile << "            <value>0.0200</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"BASELINE_outputDWIFileNameSuffix\"></entry>" << std::endl;
	outfile << "        <entry parameter=\"BASELINE_reportFileNameSuffix\">" << std::endl;
	outfile << "            <value>_QCReport.txt</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "       <entry parameter=\"BASELINE_reportFileMode\">" << std::endl;
	outfile << "            <value>1</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "    </entry>" << std::endl;
	outfile << "    <entry parameter=\"EDDYMOTION_bCorrect\">" << std::endl;
	outfile << "        <value>Yes</value>" << std::endl;
	outfile << "        <entry parameter=\"EDDYMOTION_numberOfBins\">" << std::endl;
	outfile << "            <value>24</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"EDDYMOTION_numberOfSamples\">" << std::endl;
	outfile << "            <value>100000</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"EDDYMOTION_translationScale\">" << std::endl;
	outfile << "            <value>0.0010</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"EDDYMOTION_stepLength\">" << std::endl;
	outfile << "            <value>0.1000</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"EDDYMOTION_relaxFactor\">" << std::endl;
	outfile << "            <value>0.5000</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"EDDYMOTION_maxNumberOfIterations\">" << std::endl;
	outfile << "            <value>500</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"EDDYMOTION_outputDWIFileNameSuffix\"></entry>" << std::endl;
	outfile << "        <entry parameter=\"EDDYMOTION_reportFileNameSuffix\">" << std::endl;
	outfile << "            <value>_QCReport.txt</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"EDDYMOTION_reportFileMode\">" << std::endl;
	outfile << "            <value>1</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "    </entry>" << std::endl;
	outfile << "    <entry parameter=\"GRADIENT_bCheck\">" << std::endl;
	outfile << "        <value>Yes</value>" << std::endl;
	outfile << "        <entry parameter=\"GRADIENT_translationThrehshold\">" << std::endl;
	outfile << "            <value>2.5000</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"GRADIENT_rotationThreshold\">" << std::endl;
	outfile << "            <value>0.5000</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"GRADIENT_outputDWIFileNameSuffix\"></entry>" << std::endl;
	outfile << "        <entry parameter=\"GRADIENT_reportFileNameSuffix\">" << std::endl;
	outfile << "            <value>_QCReport.txt</value>"  << std::endl;
	outfile << "        </entry>"  << std::endl;
	outfile << "        <entry parameter=\"GRADIENT_reportFileMode\">" << std::endl;
	outfile << "            <value>1</value>" << std::endl;
	outfile << "        </entry>"  << std::endl;
	outfile << "        <entry parameter=\"GRADIENT_excludedDWINrrdFileNameSuffix\">" << std::endl;
	outfile << "            <value>_gradient_exclude.nhdr</value>"  << std::endl;
	outfile << "        </entry>"  << std::endl;
	outfile << "        <entry parameter=\"GRADIENT_bQuitOnCheckFailure\">" << std::endl;
	outfile << "            <value>Yes</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "    </entry>" << std::endl;
	outfile << "    <entry parameter=\"DTI_bCompute\">" << std::endl;
	outfile << "        <value>Yes</value>" << std::endl;
	outfile << "        <entry parameter=\"DTI_dtiestimCommand\">" << std::endl;
	outfile << "            <value>/tools/bin_linux64/dtiestim</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DTI_dtiprocessCommand\">" << std::endl;
	outfile << "            <value>/tools/bin_linux64/dtiprocess</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DTI_method\">" << std::endl;
	outfile << "            <value>wls</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DTI_baselineThreshold\">" << std::endl;
	outfile << "            <value>50</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DTI_maskFileName\"></entry>" << std::endl;
	outfile << "        <entry parameter=\"DTI_tensor\">" << std::endl;
	outfile << "            <value>_DTI.nhdr</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DTI_baseline"<< std::endl;
	outfile << "            <value>Yes</value>" << std::endl;
	outfile << "            <value>_Baseline.nhdr</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DTI_idwi\">" << std::endl;
	outfile << "            <value>Yes</value>" << std::endl;
	outfile << "            <value>_IDWI.nhdr</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DTI_fa\">" << std::endl;
	outfile << "            <value>Yes</value>" << std::endl;
	outfile << "            <value>_FA.nhdr</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DTI_md\">" << std::endl;
	outfile << "            <value>Yes</value>" << std::endl;
	outfile << "            <value>_MD.nhdr</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DTI_colorfa\">" << std::endl;
	outfile << "            <value>Yes</value>" << std::endl;
	outfile << "            <value>_colorFA.nhdr</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DTI_frobeniusnorm\">" << std::endl;
	outfile << "            <value>Yes</value>" << std::endl;
	outfile << "            <value>_frobeniusnorm.nhdr</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DTI_reportFileNameSuffix\">" << std::endl;
	outfile << "            <value>_QCReport.txt</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "        <entry parameter=\"DTI_reportFileMode\">" << std::endl;
	outfile << "            <value>1</value>" << std::endl;
	outfile << "        </entry>" << std::endl;
	outfile << "    </entry>" << std::endl;
	outfile << "</ProtocolSettings>" << std::endl;

	outfile.close();
}

