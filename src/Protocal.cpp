#include "Protocal.h"
#include <iostream>
#include <math.h>


Protocal::Protocal(void)
{
	imageProtocal.bCheck = true;
	intensityMotionCheckProtocal.headSkipSlicePercentage = 0;
	intensityMotionCheckProtocal.tailSkipSlicePercentage = 0;

	//dTIProtocal.paddingParameters[0] = 0;
	//dTIProtocal.paddingParameters[1] = 0;
	//dTIProtocal.paddingParameters[2] = 0;
	//dTIProtocal.paddingParameters[3] = 0;
	//dTIProtocal.paddingParameters[4] = 0;
	//dTIProtocal.paddingParameters[5] = 0;

	dTIProtocal.bPadding = false;

	baselineNumber		= 0;
	bValueNumber		= 1;
	repetitionNumber	= 1;
	gradientDirNumber	= 0;

	//imageProtocal.size[0] = 128;
	//imageProtocal.size[1] = 128;
	//imageProtocal.size[2] = 79;
	//imageProtocal.size[3] = 7;
}

Protocal::~Protocal(void)
{
}

void Protocal::print()
{
	std::cout<<"======================================"<<std::endl;
	std::cout<<"QC Protocal"<<std::endl;

	std::cout<<" -ReportFileName "<< this->GetReportFileName() <<std::endl;

	// Image Part
	std::cout<<" -Image"<<std::endl;
	
	if(this->GetImageProtocal().bCheck)
		std::cout<<"    "<<"Check: Yes"<<std::endl;
	else
		std::cout<<"    "<<"Check: No"<<std::endl;
/*
	switch(imageProtocal.type)
	{
	case Protocal::TYPE_SHORT:
		std::cout<<"    "<<"type: short"<<std::endl;
		break;
	case Protocal::TYPE_USHORT:
		std::cout<<"    "<<"type: unsigned short"<<std::endl;
		break;
	case Protocal::TYPE_UNKNOWN:
	default:
		std::cout<<"    "<<"type: UNKNOWN"<<std::endl;
		break;
	}
*/
	switch(imageProtocal.space)
	{
	case Protocal::SPACE_LAI:
		std::cout<<"    "<<"space: left-anterior-inferior"<<std::endl;
		break;
	case Protocal::SPACE_LAS:
		std::cout<<"    "<<"space: left-anterior-superior"<<std::endl;
		break;
	case Protocal::SPACE_LPI:
		std::cout<<"    "<<"space: left-posterior-inferior"<<std::endl;
		break;
	case Protocal::SPACE_LPS:
		std::cout<<"    "<<"space: left-posterior-superior"<<std::endl;
		break;
	case Protocal::SPACE_RAI:
		std::cout<<"    "<<"space: right-anterior-inferior"<<std::endl;
		break;
	case Protocal::SPACE_RAS:
		std::cout<<"    "<<"space: right-anterior-superior"<<std::endl;
		break;
	case Protocal::SPACE_RPI:
		std::cout<<"    "<<"space: right-posterior-inferior"<<std::endl;
		break;
	case Protocal::SPACE_RPS:
		std::cout<<"    "<<"space: right-posterior-superior"<<std::endl;
		break;
	case Protocal::SPACE_UNKNOWN:
	default:
		std::cout<<"    "<<"space: UNKNOWN"<<std::endl;
		break;
	}

	std::cout<<"    "<<"space directions: ";
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			std::cout<<imageProtocal.spacedirection[i][j]<<" ";
	std::cout<<std::endl;

	std::cout<<"    "<<"dimension: "<<imageProtocal.dimension<<std::endl;
	std::cout<<"    "<<"size: "<<imageProtocal.size[0]<<" "<<imageProtocal.size[1]<<" "<<imageProtocal.size[2]<<" "<<imageProtocal.size[3]<<std::endl;
	std::cout<<"    "<<"spacing: "<<imageProtocal.spacing[0]<<" "<<imageProtocal.spacing[1]<<" "<<imageProtocal.spacing[2]<<std::endl;
	std::cout<<"    "<<"origin: "<<imageProtocal.origin[0]<<" "<<imageProtocal.origin[1]<<" "<<imageProtocal.origin[2]<<std::endl;
	
	// Diffusion Part
	std::cout<<" -Diffusion"<<std::endl;
	
	if(diffusionProtocal.bCheck)
		std::cout<<"    "<<"Check: Yes"<<std::endl;
	else
		std::cout<<"    "<<"Check: No"<<std::endl;

	std::cout<<"    "<<"measurement frame: ";
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			std::cout<<diffusionProtocal.measurementFrame[i][j]<<" ";
	std::cout<<std::endl;
	std::cout<<"    "<<"DWMRI_b-value: "<<diffusionProtocal.b<<std::endl;
	for(unsigned int i=0;i<diffusionProtocal.gradients.size();i++)
	{
		std::vector<double> vect;
		vect  = diffusionProtocal.gradients[i];
		std::cout<<"    "<<"DWMRI_gradient_"<<i<<": "<<vect[0]<<" "
			<<vect[1]<<" "
			<<vect[2]<<std::endl;
	}

	// Intensity Motion part
	std::cout<<" -IntensityMotion"<<std::endl;
	std::cout<<"    "<<"OutputFileName: "<< intensityMotionCheckProtocal.OutputFileName << std::endl;
	if(intensityMotionCheckProtocal.bCheck)
		std::cout<<"    "<<"Check: Yes"<<std::endl;
	else
		std::cout<<"    "<<"Check: No"<<std::endl;

	std::cout<<"    "<<"badGradientPercentageTolerance: "<<intensityMotionCheckProtocal.badGradientPercentageTolerance<<std::endl;

	if(intensityMotionCheckProtocal.bSliceCheck)
		std::cout<<"    -"<<"SliceCheck: Yes"<<std::endl;
	else
		std::cout<<"    -"<<"SliceCheck: No"<<std::endl;
	
	std::cout<<"        "<<"headSkipSlicePercentage: "<<intensityMotionCheckProtocal.headSkipSlicePercentage<<std::endl;
	std::cout<<"        "<<"tailSkipSlicePercentage: "<<intensityMotionCheckProtocal.tailSkipSlicePercentage<<std::endl;
	
	std::cout<<"        "<<"baselineCorrelationThreshold: "<<intensityMotionCheckProtocal.baselineCorrelationThreshold<<std::endl;
	std::cout<<"        "<<"baselineCorrelationDeviationThreshold: "<<intensityMotionCheckProtocal.baselineCorrelationDeviationThreshold<<std::endl;

	std::cout<<"        "<<"sliceCorrelationThreshold: "<<intensityMotionCheckProtocal.sliceCorrelationThreshold<<std::endl;
	std::cout<<"        "<<"sliceCorrelationDeviationThreshold: "<<intensityMotionCheckProtocal.sliceCorrelationDeviationThreshold<<std::endl;
	std::cout<<"        "<<"badSlicePercentageTolerance: "<<intensityMotionCheckProtocal.badSlicePercentageTolerance<<std::endl;

	if(intensityMotionCheckProtocal.bInterlaceCheck)
		std::cout<<"    -"<<"InterlaceCheck: Yes"<<std::endl;
	else
		std::cout<<"    -"<<"InterlaceCheck: No"<<std::endl;
	std::cout<<"        "<<"interlaceCorrelationThresholdBaseline: "<<intensityMotionCheckProtocal.interlaceCorrelationThresholdBaseline<<std::endl;
	std::cout<<"        "<<"interlaceCorrelationDeviationBaseline: "<<intensityMotionCheckProtocal.interlaceCorrelationDeviationBaseline<<std::endl;
	std::cout<<"        "<<"interlaceCorrelationThresholdGradient: "<<intensityMotionCheckProtocal.interlaceCorrelationThresholdGradient<<std::endl;
	std::cout<<"        "<<"interlaceCorrelationDeviationGradient: "<<intensityMotionCheckProtocal.interlaceCorrelationDeviationGradient<<std::endl;
	std::cout<<"        "<<"interlaceTranslationThreshold: "<<intensityMotionCheckProtocal.interlaceTranslationThreshold<<std::endl;
	std::cout<<"        "<<"interlaceRotationThreshold: "<<intensityMotionCheckProtocal.interlaceRotationThreshold<<std::endl;

	if(intensityMotionCheckProtocal.bGradientCheck)
		std::cout<<"    -"<<"GradientCheck: Yes"<<std::endl;
	else
		std::cout<<"    -"<<"GradientCheck: No"<<std::endl;
	std::cout<<"        "<<"gradientTranslationThreshold: "<<intensityMotionCheckProtocal.gradientTranslationThreshold<<std::endl;
	std::cout<<"        "<<"gradientRotationThreshold: "<<intensityMotionCheckProtocal.gradientRotationThreshold<<std::endl;

	//EddyMotion correct part
	std::cout<<" -EddyMotion correct"<<std::endl;
	if( eddyMotionCorrectionProtocal.bCorrect)
		std::cout<<"    "<<"Correct: Yes"<<std::endl;
	else
		std::cout<<"    "<<"Correct: No"<<std::endl;
	std::cout<<"    "<<"EddyCurrentCommand: "<< eddyMotionCorrectionProtocal.EddyMotionCommand<< std::endl;
	std::cout<<"    "<<"InputFileName: "<< eddyMotionCorrectionProtocal.InputFileName<< std::endl;
	std::cout<<"    "<<"OutputFileName: "<< eddyMotionCorrectionProtocal.OutputFileName<< std::endl;

	//DTI Computing part
	std::cout<<" -DTI"<<std::endl;
	
	if(dTIProtocal.bCompute)
		std::cout<<"    "<<"Compute: Yes"<<std::endl;
	else
		std::cout<<"    "<<"Compute: No"<<std::endl;

	std::cout<<"    "<<"dtiestimCommand: "<< dTIProtocal.dtiestimCommand << std::endl;
	std::cout<<"    "<<"dtiprocessCommand: "<< dTIProtocal.dtiprocessCommand<< std::endl;
	std::cout<<"    "<<"CropDTICommand: "<< dTIProtocal.dtiPaddingCommand<< std::endl;

	switch(dTIProtocal.method)
	{
	case Protocal::METHOD_WLS:
		std::cout<<"    "<<"method: wls"<<std::endl;
		break;
	case Protocal::METHOD_LLS:
		std::cout<<"    "<<"method: lls"<<std::endl;
		break;
	case Protocal::METHOD_ML:
		std::cout<<"    "<<"method: ml"<<std::endl;
		break;
	case Protocal::METHOD_UNKNOWN:
	default:
		std::cout<<"    "<<"method: UNKNOWN"<<std::endl;
		break;
	}
	std::cout<<"    "<<"tensor: "<<dTIProtocal.tensor<<std::endl;

	if(dTIProtocal.bPadding)
		std::cout<<"    "<<"padding: Yes"<<std::endl;
	else
		std::cout<<"    "<<"padding: No"<<std::endl;

// 	std::cout<<"    "<<"padded tensor: "<<dTIProtocal.tensorPadded<<std::endl;
// 	std::cout<<"    "<<"padding paremeters: "
// 		<<dTIProtocal.paddingParameters[0]<<" "<<dTIProtocal.paddingParameters[1]<<" "<<dTIProtocal.paddingParameters[2]<<" "
// 		<<dTIProtocal.paddingParameters[3]<<" "<<dTIProtocal.paddingParameters[4]<<" "<<dTIProtocal.paddingParameters[5]<<std::endl;

	std::cout<<"    "<<"baselineThreshold: "<<dTIProtocal.baselineThreshold<<std::endl;
	std::cout<<"    "<<"mask: "<<dTIProtocal.mask<<std::endl;
	std::cout<<"    "<<"baseline: "<<dTIProtocal.baseline<<std::endl;
	std::cout<<"    "<<"idwi: "<<dTIProtocal.idwi<<std::endl;
	std::cout<<"    "<<"fa: "<<dTIProtocal.fa<<std::endl;
	std::cout<<"    "<<"md: "<<dTIProtocal.md<<std::endl;
	std::cout<<"    "<<"coloredfa: "<<dTIProtocal.coloredfa<<std::endl;
	std::cout<<"    "<<"frobeniusnorm: "<<dTIProtocal.frobeniusnorm<<std::endl;

	std::cout<<"======================================"<<std::endl;
/*
	intensityMotionCheckProtocal;
	IntensityMotionCheckProtocal	intensityMotionCheckProtocal;
	EddyMotionCorrectionProtocal	eddyMotionCorrectionProtocal;
	DTIProtocal						dTIProtocal;

*/
}
	//void fromTreeWidget(QTreeWidget tree);
void Protocal::fromXMLFile(std::string xml)
{

}

void Protocal::clear()
{
	this->GetDiffusionProtocal().gradients.clear();

	this->GetImageProtocal().bCheck = false;

	this->GetDiffusionProtocal().bCheck = false;

	this->GetIntensityMotionCheckProtocal().bCheck = false;
	this->GetIntensityMotionCheckProtocal().bGradientCheck = false;
	this->GetIntensityMotionCheckProtocal().bSliceCheck = false;
	this->GetIntensityMotionCheckProtocal().bInterlaceCheck = false;

	baselineNumber		= 0;
	bValueNumber		= 1;
	repetitionNumber	= 1;
	gradientDirNumber	= 0;
}

void Protocal::collectDiffusionStatistics()
{

	std::vector<DiffusionDir> DiffusionDirections;
	DiffusionDirections.clear();

// 	std::cout<<"this->GetDiffusionProtocal().gradients.size(): " << this->GetDiffusionProtocal().gradients.size() <<std::endl;
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

// 	std::cout<<"DiffusionDirections.size(): " << DiffusionDirections.size() <<std::endl;

	std::vector<int> repetNum;
	repetNum.clear();
	std::vector<double> dirMode;
	dirMode.clear();

	double modeTemp = 0.0;
	for( unsigned int i=0; i<DiffusionDirections.size(); i++)
	{
		if( DiffusionDirections[i].gradientDir[0] == 0.0 &&
			DiffusionDirections[i].gradientDir[1] == 0.0 &&
			DiffusionDirections[i].gradientDir[2] == 0.0 ) 
		{
			this->baselineNumber = DiffusionDirections[i].repetitionNumber;
// 			std::cout<<"DiffusionDirections[i].repetitionNumber: " <<i<<"  "<<DiffusionDirections[i].repetitionNumber <<std::endl;
		}
		else
		{
			repetNum.push_back(DiffusionDirections[i].repetitionNumber);

			double modeSqr =	DiffusionDirections[i].gradientDir[0]*DiffusionDirections[i].gradientDir[0] +
								DiffusionDirections[i].gradientDir[1]*DiffusionDirections[i].gradientDir[1] +
								DiffusionDirections[i].gradientDir[2]*DiffusionDirections[i].gradientDir[2];

// 			std::cout<<"modeSqr: " <<modeSqr <<std::endl;
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
// 					std::cout<<" if(newDirMode) dirMode.size(): " <<  dirMode.size() <<std::endl;
				}
			}
			else
			{
				dirMode.push_back(	modeSqr) ;
// 				std::cout<<" else dirMode.size(): " <<  dirMode.size() <<std::endl;
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

	std::cout<<"Protocol Diffusion: "	<<std::endl;
	std::cout<<"  baselineNumber: "	<<baselineNumber	<<std::endl;
	std::cout<<"  bValueNumber: "		<<bValueNumber		<<std::endl;
	std::cout<<"  gradientDirNumber: "<<gradientDirNumber	<<std::endl;
	std::cout<<"  repetitionNumber: "	<<repetitionNumber	<<std::endl;

	return ;
}
