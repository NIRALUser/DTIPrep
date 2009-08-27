#include "XmlStreamReader.h"
#include <QtGui>
#include <iostream>

XmlStreamReader::XmlStreamReader(QTreeWidget *tree)
{
	treeWidget = tree;
	protocal=NULL;
}

XmlStreamReader::~XmlStreamReader(void)
{
}

void XmlStreamReader::InitializeProtocolStringValues()
{
	//QC overall
	s_mapProtocolStringValues["QC_QCOutputDirectory"]						= QC_QCOutputDirectory;
	s_mapProtocolStringValues["QC_QCedDWIFileNameSuffix"]					= QC_QCedDWIFileNameSuffix;
	s_mapProtocolStringValues["QC_reportFileNameSuffix"]					= QC_reportFileNameSuffix;
	s_mapProtocolStringValues["QC_badGradientPercentageTolerance"]			= QC_badGradientPercentageTolerance;

	// image
	s_mapProtocolStringValues["IMAGE_bCheck"]								= IMAGE_bCheck;
	s_mapProtocolStringValues["IMAGE_type"]									= IMAGE_type;
	s_mapProtocolStringValues["IMAGE_space"]								= IMAGE_space;
	s_mapProtocolStringValues["IMAGE_directions"]							= IMAGE_directions;
	s_mapProtocolStringValues["IMAGE_dimension"]							= IMAGE_dimension;
	s_mapProtocolStringValues["IMAGE_size"]									= IMAGE_size;
	s_mapProtocolStringValues["IMAGE_spacing"]								= IMAGE_spacing;
	s_mapProtocolStringValues["IMAGE_origin"]								= IMAGE_origin;
	s_mapProtocolStringValues["IMAGE_bCrop"]								= IMAGE_bCrop;
	s_mapProtocolStringValues["IMAGE_croppedDWIFileNameSuffix"]				= IMAGE_croppedDWIFileNameSuffix;
	s_mapProtocolStringValues["IMAGE_reportFileNameSuffix"]					= IMAGE_reportFileNameSuffix;
	s_mapProtocolStringValues["IMAGE_reportFileMode"]						= IMAGE_reportFileMode;

	// diffusion
	s_mapProtocolStringValues["DIFFUSION_bCheck"]							= DIFFUSION_bCheck;
	s_mapProtocolStringValues["DIFFUSION_measurementFrame"]					= DIFFUSION_measurementFrame;
	s_mapProtocolStringValues["DIFFUSION_DWMRI_bValue"]						= DIFFUSION_DWMRI_bValue;
	s_mapProtocolStringValues["DIFFUSION_DWMRI_gradient"]					= DIFFUSION_DWMRI_gradient;
	s_mapProtocolStringValues["DIFFUSION_bUseDiffusionProtocal"]			= DIFFUSION_bUseDiffusionProtocal;
	s_mapProtocolStringValues["DIFFUSION_diffusionReplacedDWIFileNameSuffix"]= DIFFUSION_diffusionReplacedDWIFileNameSuffix;
	s_mapProtocolStringValues["DIFFUSION_reportFileNameSuffix"]				= DIFFUSION_reportFileNameSuffix;
	s_mapProtocolStringValues["DIFFUSION_reportFileMode"]					= DIFFUSION_reportFileMode;
	
	// slice check
	s_mapProtocolStringValues["SLICE_bCheck"]								= SLICE_bCheck;
// 	s_mapProtocolStringValues["SLICE_badGradientPercentageTolerance"]		= SLICE_badGradientPercentageTolerance;
	s_mapProtocolStringValues["SLICE_checkTimes"]							= SLICE_checkTimes;
	s_mapProtocolStringValues["SLICE_headSkipSlicePercentage"]				= SLICE_headSkipSlicePercentage;
	s_mapProtocolStringValues["SLICE_tailSkipSlicePercentage"]				= SLICE_tailSkipSlicePercentage;
	s_mapProtocolStringValues["SLICE_correlationDeviationThresholdbaseline"]= SLICE_correlationDeviationThresholdbaseline;
	s_mapProtocolStringValues["SLICE_correlationDeviationThresholdgradient"]= SLICE_correlationDeviationThresholdgradient;
	s_mapProtocolStringValues["SLICE_outputDWIFileNameSuffix"]				= SLICE_outputDWIFileNameSuffix;
	s_mapProtocolStringValues["SLICE_reportFileNameSuffix"]					= SLICE_reportFileNameSuffix;
	s_mapProtocolStringValues["SLICE_reportFileMode"]						= SLICE_reportFileMode;

	// interlace check
	s_mapProtocolStringValues["INTERLACE_bCheck"]							= INTERLACE_bCheck;
// 	s_mapProtocolStringValues["INTERLACE_badGradientPercentageTolerance"]	= INTERLACE_badGradientPercentageTolerance;
	s_mapProtocolStringValues["INTERLACE_correlationThresholdBaseline"]		= INTERLACE_correlationThresholdBaseline;
	s_mapProtocolStringValues["INTERLACE_correlationThresholdGradient"]		= INTERLACE_correlationThresholdGradient;
	s_mapProtocolStringValues["INTERLACE_correlationDeviationBaseline"]		= INTERLACE_correlationDeviationBaseline;
	s_mapProtocolStringValues["INTERLACE_correlationDeviationGradient"]		= INTERLACE_correlationDeviationGradient;
	s_mapProtocolStringValues["INTERLACE_translationThreshold"]				= INTERLACE_translationThreshold;
	s_mapProtocolStringValues["INTERLACE_rotationThreshold"]				= INTERLACE_rotationThreshold;
	s_mapProtocolStringValues["INTERLACE_outputDWIFileNameSuffix"]			= INTERLACE_outputDWIFileNameSuffix;
	s_mapProtocolStringValues["INTERLACE_reportFileNameSuffix"]				= INTERLACE_reportFileNameSuffix;
	s_mapProtocolStringValues["INTERLACE_reportFileMode"]					= INTERLACE_reportFileMode;

	// gradient check
	s_mapProtocolStringValues["GRADIENT_bCheck"]							= GRADIENT_bCheck; 
// 	s_mapProtocolStringValues["GRADIENT_badGradientPercentageTolerance"]	= GRADIENT_badGradientPercentageTolerance;
	s_mapProtocolStringValues["GRADIENT_translationThrehshold"]				= GRADIENT_translationThrehshold;
	s_mapProtocolStringValues["GRADIENT_rotationThreshold"]					= GRADIENT_rotationThreshold;
	s_mapProtocolStringValues["GRADIENT_outputDWIFileNameSuffix"]			= GRADIENT_outputDWIFileNameSuffix;
	s_mapProtocolStringValues["GRADIENT_reportFileNameSuffix"]				= GRADIENT_reportFileNameSuffix;
	s_mapProtocolStringValues["GRADIENT_reportFileMode"]					= GRADIENT_reportFileMode;

	// baseline average
	s_mapProtocolStringValues["BASELINE_bAverage"]							= BASELINE_bAverage;
	s_mapProtocolStringValues["BASELINE_averageMethod"]						= BASELINE_averageMethod;
	s_mapProtocolStringValues["BASELINE_stopThreshold"]						= BASELINE_stopThreshold;
	s_mapProtocolStringValues["BASELINE_outputDWIFileNameSuffix"]			= BASELINE_outputDWIFileNameSuffix;
	s_mapProtocolStringValues["BASELINE_reportFileNameSuffix"]				= BASELINE_reportFileNameSuffix;
	s_mapProtocolStringValues["BASELINE_reportFileMode"]					= BASELINE_reportFileMode;

	// eddy motion correction
	s_mapProtocolStringValues["EDDYMOTION_bCorrect"]						= EDDYMOTION_bCorrect;

// 	s_mapProtocolStringValues["EDDYMOTION_command"]							= EDDYMOTION_command;
// 	s_mapProtocolStringValues["EDDYMOTION_inputFileName"]					= EDDYMOTION_inputFileName;
// 	s_mapProtocolStringValues["EDDYMOTION_outputFileName"]					= EDDYMOTION_outputFileName;
	
	s_mapProtocolStringValues["EDDYMOTION_numberOfBins"]					= EDDYMOTION_numberOfBins;
	s_mapProtocolStringValues["EDDYMOTION_numberOfSamples"]					= EDDYMOTION_numberOfSamples	;
	s_mapProtocolStringValues["EDDYMOTION_translationScale"]				= EDDYMOTION_translationScale;
	s_mapProtocolStringValues["EDDYMOTION_stepLength"]						= EDDYMOTION_stepLength;
	s_mapProtocolStringValues["EDDYMOTION_relaxFactor"]						= EDDYMOTION_relaxFactor;
	s_mapProtocolStringValues["EDDYMOTION_maxNumberOfIterations"]			= EDDYMOTION_maxNumberOfIterations;

	s_mapProtocolStringValues["EDDYMOTION_outputDWIFileNameSuffix"]			= EDDYMOTION_outputDWIFileNameSuffix;
	s_mapProtocolStringValues["EDDYMOTION_reportFileNameSuffix"]			= EDDYMOTION_reportFileNameSuffix;
	s_mapProtocolStringValues["EDDYMOTION_reportFileMode"]					= EDDYMOTION_reportFileMode;

	// DTI computing
	s_mapProtocolStringValues["DTI_bCompute"]								= DTI_bCompute;
	s_mapProtocolStringValues["DTI_dtiestimCommand"]						= DTI_dtiestimCommand;
	s_mapProtocolStringValues["DTI_dtiprocessCommand"]						= DTI_dtiprocessCommand;
	s_mapProtocolStringValues["DTI_method"]									= DTI_method;
	s_mapProtocolStringValues["DTI_baselineThreshold"]						= DTI_baselineThreshold;
	s_mapProtocolStringValues["DTI_maskFileName"]							= DTI_maskFileName;
	s_mapProtocolStringValues["DTI_tensor"]									= DTI_tensor;
	s_mapProtocolStringValues["DTI_baseline"]								= DTI_baseline;
	s_mapProtocolStringValues["DTI_idwi"]									= DTI_idwi;
	s_mapProtocolStringValues["DTI_fa"]										= DTI_fa;
	s_mapProtocolStringValues["DTI_md"]										= DTI_md;
	s_mapProtocolStringValues["DTI_colorfa"]								= DTI_colorfa;
	s_mapProtocolStringValues["DTI_frobeniusnorm"]							= DTI_frobeniusnorm;
	s_mapProtocolStringValues["DTI_reportFileNameSuffix"]					= DTI_reportFileNameSuffix;
	s_mapProtocolStringValues["DTI_reportFileMode"]							= DTI_reportFileMode;

//	std::cout << "s_mapProtocolStringValues contains "
//			<< s_mapProtocolStringValues.size()
//			<< " entries." << std::endl;
}


bool XmlStreamReader::readFile(const QString &fileName, int mode)
{
	if(protocal)	protocal->clear();

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        std::cerr << "Error: Cannot read file " << qPrintable(fileName)
                  << ": " << qPrintable(file.errorString())
                  << std::endl;
        return false;
    }
    reader.setDevice(&file);

	reader.readNext();
	while (!reader.atEnd()) {
		if (reader.isStartElement()) {
			if (reader.name() == "ProtocalSettings") {
				readProtocalSettingsElement( mode );
			} else {
				reader.raiseError(QObject::tr("Not a ProtocalSettings file"));
			}
		} else {
			reader.readNext();
		}
	}

    file.close();
    if (reader.hasError()) {
        std::cerr << "Error: Failed to parse file "
                  << qPrintable(fileName) << ": "
                  << qPrintable(reader.errorString()) << std::endl;
        return false;
    } else if (file.error() != QFile::NoError) {
        std::cerr << "Error: Cannot read file " << qPrintable(fileName)
                  << ": " << qPrintable(file.errorString())
                  << std::endl;
        return false;
    }
    return true;
}

void XmlStreamReader::readProtocalSettingsElement(int mode)
{
    reader.readNext();
    while (!reader.atEnd()) {
        if (reader.isEndElement()) {
            reader.readNext();
            break;
        }

        if (reader.isStartElement()) {
            if (reader.name() == "entry") {
				if( mode== XmlStreamReader::TreeWise)
					readEntryElement(treeWidget->invisibleRootItem());
				else if(mode== XmlStreamReader::ProtocalWise){
					readEntryElement();					
				}
				else{
					std::cout<<"invalid setting reading mode"<<std::endl;
					return;
				}
            } else {
                skipUnknownElement();
            }
        } else {
            reader.readNext();
        }
    }

	if(mode== XmlStreamReader::ProtocalWise) 
	{
		InitializeProtocolStringValues();
		parseXMLParametersToProtocal();
	}
}

void XmlStreamReader::parseXMLParametersToProtocal()
{	
	int temp;
	QStringList values;
	QStringList subvalues;
	for(unsigned int i=0;i<paremeters.size();i++)
	{
		//std::cout<<paremeters[i].parameter.toStdString()<<"    "<<paremeters[i].value.toStdString()<<std::endl;
		if(paremeters[i].parameter.left(24)==QObject::tr("DIFFUSION_DWMRI_gradient"))
		{
			std::vector<double> vect;
			values = paremeters[i].value.split(" ");
			foreach (QString value, values)
			{
				vect.push_back( value.toDouble());
			}
			protocal->GetDiffusionProtocal().gradients.push_back(vect);
		}
		else
		{
			switch(s_mapProtocolStringValues[paremeters[i].parameter.toStdString()])
			{
			//QC overall
			case QC_QCOutputDirectory:
				protocal->GetQCOutputDirectory() =  paremeters[i].value.toStdString();
				break;
			case QC_QCedDWIFileNameSuffix:
				protocal->GetQCedDWIFileNameSuffix() =  paremeters[i].value.toStdString();
				break;
			case QC_reportFileNameSuffix:
				protocal->GetReportFileNameSuffix() =  paremeters[i].value.toStdString();
				break;
			case QC_badGradientPercentageTolerance:
				protocal->SetBadGradientPercentageTolerance(paremeters[i].value.toDouble()) ;
				break;

			// image
			case IMAGE_bCheck:
				if(paremeters[i].value.toLower().toStdString()=="yes")
					protocal->GetImageProtocal().bCheck = true;
				else
					protocal->GetImageProtocal().bCheck = false;
				break;
			case IMAGE_type:
				if(paremeters[i].value.toStdString()=="short")
					protocal->GetImageProtocal().type = Protocal::TYPE_SHORT;
				else if(paremeters[i].value.toStdString()=="unsigned short")
					protocal->GetImageProtocal().type = Protocal::TYPE_USHORT;
				else
					protocal->GetImageProtocal().type = Protocal::TYPE_UNKNOWN;
				break;
			case IMAGE_space:
				if(paremeters[i].value.toStdString()=="left-anterior-inferior")
					protocal->GetImageProtocal().space = Protocal::SPACE_LAI;
				else if(paremeters[i].value.toStdString()=="left-anterior-superior")
					protocal->GetImageProtocal().space = Protocal::SPACE_LAS;
				else if(paremeters[i].value.toStdString()=="left-posterior-inferior")
					protocal->GetImageProtocal().space = Protocal::SPACE_LPI;
				else if(paremeters[i].value.toStdString()=="left-posterior-superior")
					protocal->GetImageProtocal().space = Protocal::SPACE_LPS;
				else if(paremeters[i].value.toStdString()=="right-anterior-inferior")
					protocal->GetImageProtocal().space = Protocal::SPACE_RAI;
				else if(paremeters[i].value.toStdString()=="right-anterior-superior")
					protocal->GetImageProtocal().space = Protocal::SPACE_RAS;
				else if(paremeters[i].value.toStdString()=="right-posterior-inferior")
					protocal->GetImageProtocal().space = Protocal::SPACE_RPI;
				else if(paremeters[i].value.toStdString()=="right-posterior-superior")
					protocal->GetImageProtocal().space = Protocal::SPACE_RPS;
				else
					protocal->GetImageProtocal().space = Protocal::SPACE_UNKNOWN;
				break;
			case IMAGE_directions:
				values = paremeters[i].value.split(", ");
				temp=0;
				foreach (QString value, values)
				{
					subvalues = value.split(" ");
					foreach (QString subvalue, subvalues)
					{
						protocal->GetImageProtocal().spacedirection[temp/3][temp%3]= subvalue.toDouble();
						temp++;
					}
				}				
				break;
			case IMAGE_dimension:
				protocal->GetImageProtocal().dimension = paremeters[i].value.toInt();
				break;
			case IMAGE_size:
				values = paremeters[i].value.split(", ");
				temp=0;
				foreach (QString value, values)
				{
					protocal->GetImageProtocal().size[temp]= value.toInt();
					temp++;
				}				
				break;
			case IMAGE_spacing:
				values = paremeters[i].value.split(", ");
				temp=0;
				foreach (QString value, values)
				{
					protocal->GetImageProtocal().spacing[temp]= value.toDouble();
					temp++;
				}				
				break;
			case IMAGE_origin:
				values = paremeters[i].value.split(", ");
				temp=0;
				foreach (QString value, values)
				{
					protocal->GetImageProtocal().origin[temp]= value.toDouble();
					temp++;
				}				
				break;
			case IMAGE_bCrop:
				if(paremeters[i].value.toLower().toStdString()=="yes")
					protocal->GetImageProtocal().bCrop = true;
				else
					protocal->GetImageProtocal().bCrop = false;
				break;
			case IMAGE_croppedDWIFileNameSuffix:
				protocal->GetImageProtocal().croppedDWIFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case IMAGE_reportFileNameSuffix:
				protocal->GetImageProtocal().reportFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case IMAGE_reportFileMode:
				protocal->GetImageProtocal().reportFileMode =  paremeters[i].value.toInt();
				break;

			// Diffusion
			case DIFFUSION_bCheck:
				if(paremeters[i].value.toLower().toStdString()=="yes")
					protocal->GetDiffusionProtocal().bCheck = true;
				else
					protocal->GetDiffusionProtocal().bCheck = false;
				break;
			case DIFFUSION_measurementFrame:
				values = paremeters[i].value.split(", ");
				temp=0;
				foreach (QString value, values)
				{
					subvalues = value.split(" ");
					foreach (QString subvalue, subvalues)
					{
						protocal->GetDiffusionProtocal().measurementFrame[temp/3][temp%3]= subvalue.toDouble();
						temp++;
					}
				}				
				break;
			case DIFFUSION_DWMRI_bValue:
				protocal->GetDiffusionProtocal().bValue =  paremeters[i].value.toDouble();
				break;
			//case ev_DWMRI_gradient:
			//	break;
			case DIFFUSION_bUseDiffusionProtocal:
				if(paremeters[i].value.toLower().toStdString()=="yes")
					protocal->GetDiffusionProtocal().bUseDiffusionProtocal = true;
				else
					protocal->GetDiffusionProtocal().bUseDiffusionProtocal = false;
				break;
			case DIFFUSION_diffusionReplacedDWIFileNameSuffix:
				protocal->GetDiffusionProtocal().diffusionReplacedDWIFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case DIFFUSION_reportFileNameSuffix:
				protocal->GetDiffusionProtocal().reportFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case DIFFUSION_reportFileMode:
				protocal->GetDiffusionProtocal().reportFileMode =  paremeters[i].value.toInt();
				break;

			// slice Check
			case SLICE_bCheck:
				if(paremeters[i].value.toLower().toStdString()=="yes")
					protocal->GetSliceCheckProtocal().bCheck = true;
				else
					protocal->GetSliceCheckProtocal().bCheck = false;
				break;
// 			case SLICE_badGradientPercentageTolerance:
// 				protocal->GetSliceCheckProtocal().badGradientPercentageTolerance =  paremeters[i].value.toDouble();
// 				break;
			case SLICE_checkTimes:
				protocal->GetSliceCheckProtocal().checkTimes =  paremeters[i].value.toInt();
				break;
			case SLICE_headSkipSlicePercentage:
				protocal->GetSliceCheckProtocal().headSkipSlicePercentage = paremeters[i].value.toDouble(); 
				break;
			case SLICE_tailSkipSlicePercentage:
				protocal->GetSliceCheckProtocal().tailSkipSlicePercentage = paremeters[i].value.toDouble(); 
				break;
			case SLICE_correlationDeviationThresholdbaseline:
				protocal->GetSliceCheckProtocal().correlationDeviationThresholdbaseline = paremeters[i].value.toDouble(); 
				break;
			case SLICE_correlationDeviationThresholdgradient:
				protocal->GetSliceCheckProtocal().correlationDeviationThresholdgradient = paremeters[i].value.toDouble(); 
				break;			
			case SLICE_outputDWIFileNameSuffix:
				protocal->GetSliceCheckProtocal().outputDWIFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case SLICE_reportFileNameSuffix:
				protocal->GetSliceCheckProtocal().reportFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case SLICE_reportFileMode:
				protocal->GetSliceCheckProtocal().reportFileMode =  paremeters[i].value.toInt();
				break;

			// interlace check
			case INTERLACE_bCheck:
				if(paremeters[i].value.toLower().toStdString()=="yes")
					protocal->GetInterlaceCheckProtocal().bCheck = true;
				else
					protocal->GetInterlaceCheckProtocal().bCheck = false;
				break;
// 			case INTERLACE_badGradientPercentageTolerance:
// 				protocal->GetInterlaceCheckProtocal().badGradientPercentageTolerance =  paremeters[i].value.toDouble();
// 				break;
			case INTERLACE_correlationThresholdBaseline:
				protocal->GetInterlaceCheckProtocal().correlationThresholdBaseline = paremeters[i].value.toDouble(); 
				break;
			case INTERLACE_correlationThresholdGradient:
				protocal->GetInterlaceCheckProtocal().correlationThresholdGradient = paremeters[i].value.toDouble(); 
				break;
			case INTERLACE_correlationDeviationBaseline:
				protocal->GetInterlaceCheckProtocal().correlationDeviationBaseline = paremeters[i].value.toDouble(); 
				break;
			case INTERLACE_correlationDeviationGradient:
				protocal->GetInterlaceCheckProtocal().correlationDeviationGradient = paremeters[i].value.toDouble(); 
				break;
			case INTERLACE_translationThreshold:
				protocal->GetInterlaceCheckProtocal().translationThreshold = paremeters[i].value.toDouble(); 
				break;
			case INTERLACE_rotationThreshold:
				protocal->GetInterlaceCheckProtocal().rotationThreshold = paremeters[i].value.toDouble(); 
				break;
			case INTERLACE_outputDWIFileNameSuffix:
				protocal->GetInterlaceCheckProtocal().outputDWIFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case INTERLACE_reportFileNameSuffix:
				protocal->GetInterlaceCheckProtocal().reportFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case INTERLACE_reportFileMode:
				protocal->GetInterlaceCheckProtocal().reportFileMode =  paremeters[i].value.toInt();
				break;

			// gradient check
			case GRADIENT_bCheck:
				if(paremeters[i].value.toLower().toStdString()=="yes")
					protocal->GetGradientCheckProtocal().bCheck = true;
				else
					protocal->GetGradientCheckProtocal().bCheck = false;
				break;
// 			case GRADIENT_badGradientPercentageTolerance:
// 				protocal->GetGradientCheckProtocal().badGradientPercentageTolerance =  paremeters[i].value.toDouble();
// 				break;
			case GRADIENT_translationThrehshold:
				protocal->GetGradientCheckProtocal().translationThreshold = paremeters[i].value.toDouble(); 
				break;
			case GRADIENT_rotationThreshold:
				protocal->GetGradientCheckProtocal().rotationThreshold = paremeters[i].value.toDouble(); 
				break;
			case GRADIENT_outputDWIFileNameSuffix:
				protocal->GetGradientCheckProtocal().outputDWIFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case GRADIENT_reportFileNameSuffix:
				protocal->GetGradientCheckProtocal().reportFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case GRADIENT_reportFileMode:
				protocal->GetGradientCheckProtocal().reportFileMode =  paremeters[i].value.toInt();
				break;

			// baseline average
			case BASELINE_bAverage:
				if(paremeters[i].value.toLower().toStdString()=="yes")
					protocal->GetBaselineAverageProtocal().bAverage = true;
				else
					protocal->GetBaselineAverageProtocal().bAverage = false;
				break;
			case BASELINE_averageMethod:
				protocal->GetBaselineAverageProtocal().averageMethod=  paremeters[i].value.toInt();
				break;
			case BASELINE_stopThreshold:
				protocal->GetBaselineAverageProtocal().stopThreshold = paremeters[i].value.toDouble(); 
				break;
			case BASELINE_outputDWIFileNameSuffix:
				protocal->GetBaselineAverageProtocal().outputDWIFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case BASELINE_reportFileNameSuffix:
				protocal->GetBaselineAverageProtocal().reportFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case BASELINE_reportFileMode:
				protocal->GetBaselineAverageProtocal().reportFileMode =  paremeters[i].value.toInt();
				break;

			// eddy current
			case EDDYMOTION_bCorrect:
				if(paremeters[i].value.toLower().toStdString()=="yes")
					protocal->GetEddyMotionCorrectionProtocal().bCorrect = true;
				else
					protocal->GetEddyMotionCorrectionProtocal().bCorrect = false;
				break;
// 			case EDDYMOTION_command:
// 				protocal->GetEddyMotionCorrectionProtocal().EddyMotionCommand =  paremeters[i].value.toStdString();
// 				break;
// 			case EDDYMOTION_inputFileName:
// 				protocal->GetEddyMotionCorrectionProtocal().InputFileName = paremeters[i].value.toStdString(); 
// 				break;
// 			case EDDYMOTION_outputFileName:
// 				protocal->GetEddyMotionCorrectionProtocal().OutputFileName = paremeters[i].value.toStdString(); 
// 				break;

			case EDDYMOTION_numberOfBins:
				protocal->GetEddyMotionCorrectionProtocal().numberOfBins =  paremeters[i].value.toInt();
				break;
			case EDDYMOTION_numberOfSamples:
				protocal->GetEddyMotionCorrectionProtocal().numberOfSamples =  paremeters[i].value.toInt();
				break;
			case EDDYMOTION_translationScale:
				protocal->GetEddyMotionCorrectionProtocal().translationScale =  paremeters[i].value.toFloat();
				break;
			case EDDYMOTION_stepLength:
				protocal->GetEddyMotionCorrectionProtocal().stepLength =  paremeters[i].value.toFloat(); 
				break;
			case EDDYMOTION_relaxFactor:
				protocal->GetEddyMotionCorrectionProtocal().relaxFactor =  paremeters[i].value.toFloat(); 
				break;
			case EDDYMOTION_maxNumberOfIterations:
				protocal->GetEddyMotionCorrectionProtocal().maxNumberOfIterations =  paremeters[i].value.toInt();
				break;

			case EDDYMOTION_outputDWIFileNameSuffix:
				protocal->GetEddyMotionCorrectionProtocal().outputDWIFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case EDDYMOTION_reportFileNameSuffix:
				protocal->GetEddyMotionCorrectionProtocal().reportFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case EDDYMOTION_reportFileMode:
				protocal->GetEddyMotionCorrectionProtocal().reportFileMode =  paremeters[i].value.toInt();
				break;

			//  DTI
			case DTI_bCompute:
				if(paremeters[i].value.toLower().toStdString()=="yes")
					protocal->GetDTIProtocal().bCompute = true;
				else
					protocal->GetDTIProtocal().bCompute = false;
				break;
			case DTI_dtiestimCommand:
				protocal->GetDTIProtocal().dtiestimCommand =  paremeters[i].value.toStdString();
				break;
			case DTI_dtiprocessCommand:
				protocal->GetDTIProtocal().dtiprocessCommand =  paremeters[i].value.toStdString();
				break;
			case DTI_method:
				if(paremeters[i].value.toStdString()=="wls")
					protocal->GetDTIProtocal().method = Protocal::METHOD_WLS;
				else if(paremeters[i].value.toStdString()=="lls")
					protocal->GetDTIProtocal().method = Protocal::METHOD_LLS;
				else if(paremeters[i].value.toStdString()=="ml")
					protocal->GetDTIProtocal().method = Protocal::METHOD_ML;
				else
					protocal->GetDTIProtocal().method = Protocal::METHOD_UNKNOWN;
				break;
			case DTI_baselineThreshold:
				protocal->GetDTIProtocal().baselineThreshold=  paremeters[i].value.toInt();
				break;
			case DTI_maskFileName:
				protocal->GetDTIProtocal().mask =  paremeters[i].value.toStdString();
				break;
			case DTI_tensor:
				protocal->GetDTIProtocal().tensorSuffix = paremeters[i].value.toStdString();
				break;
			case DTI_fa:
				values = paremeters[i].value.split(", ");
				protocal->GetDTIProtocal().faSuffix = values[1].toStdString();
				if(values[0].toLower().toStdString()=="yes")
					protocal->GetDTIProtocal().bfa = true;
				else
					protocal->GetDTIProtocal().bfa = false;
				break;
			case DTI_md:
				values = paremeters[i].value.split(", ");
					protocal->GetDTIProtocal().mdSuffix = values[1].toStdString();
				if(values[0].toLower().toStdString()=="yes")
					protocal->GetDTIProtocal().bmd = true;
				else
					protocal->GetDTIProtocal().bmd = false;
				break;
			case DTI_colorfa:
				values = paremeters[i].value.split(", ");
				protocal->GetDTIProtocal().coloredfaSuffix = values[1].toStdString();
				if(values[0].toLower().toStdString()=="yes")
					protocal->GetDTIProtocal().bcoloredfa= true;
				else
					protocal->GetDTIProtocal().bcoloredfa= false;
				break;
			case DTI_idwi:
				values = paremeters[i].value.split(", ");
				protocal->GetDTIProtocal().idwiSuffix  = values[1].toStdString();
				if(values[0].toLower().toStdString()=="yes")
					protocal->GetDTIProtocal().bidwi = true;
				else
					protocal->GetDTIProtocal().bidwi = false;
				break;

			case DTI_frobeniusnorm:
				values = paremeters[i].value.split(", ");
				protocal->GetDTIProtocal().frobeniusnormSuffix = values[1].toStdString();
				if(values[0].toLower().toStdString()=="yes")
					protocal->GetDTIProtocal().bfrobeniusnorm = true;
				else
					protocal->GetDTIProtocal().bfrobeniusnorm = false;
				break;

			case DTI_baseline:
				values = paremeters[i].value.split(", ");
				protocal->GetDTIProtocal().baselineSuffix  = values[1].toStdString();
				if(values[0].toLower().toStdString()=="yes")
					protocal->GetDTIProtocal().bbaseline = true;
				else
					protocal->GetDTIProtocal().bbaseline = false;
				break;
			case DTI_reportFileNameSuffix:
				protocal->GetDTIProtocal().reportFileNameSuffix =  paremeters[i].value.toStdString();
				break;
			case DTI_reportFileMode:
				protocal->GetDTIProtocal().reportFileMode =  paremeters[i].value.toInt();
				break;

			case QC_Unknow:
			default:
				break;
			}
		}	
	}
}



void XmlStreamReader::readEntryElement(QTreeWidgetItem *parent)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, reader.attributes().value("parameter").toString());

    reader.readNext();
    while (!reader.atEnd()) {
        if (reader.isEndElement()) {
            reader.readNext();
            break;
        }

        if (reader.isStartElement()) {
            if (reader.name() == "entry") {
                readEntryElement(item);
            } else if (reader.name() == "value") {
                readValueElement(item);
            } else {
                skipUnknownElement();
            }
        } else {
            reader.readNext();
        }
    }
}

void XmlStreamReader::readEntryElement()
{
	ITEM item;

	item.parameter=reader.attributes().value("parameter").toString();
	paremeters.push_back( item );

    reader.readNext();
    while (!reader.atEnd()) {
        if (reader.isEndElement()) {
            reader.readNext();
            break;
        }

        if (reader.isStartElement()) {
            if (reader.name() == "entry") {
                readEntryElement();
            } else if (reader.name() == "value") {
                readValueElement();
            } else {
                skipUnknownElement();
            }
        } else {
            reader.readNext();
        }
    }
}
void XmlStreamReader::readValueElement()
{
    QString page = reader.readElementText();
    if (reader.isEndElement())
        reader.readNext();

	QString allPages =paremeters[paremeters.size()-1].value;
    if (!allPages.isEmpty())
        allPages += ", ";
    allPages += page;

	paremeters[paremeters.size()-1].value = allPages;
   
}

void XmlStreamReader::readValueElement(QTreeWidgetItem *parent)
{
    QString page = reader.readElementText();
    if (reader.isEndElement())
        reader.readNext();

    QString allPages = parent->text(1);
    if (!allPages.isEmpty())
        allPages += ", ";
    allPages += page;

    parent->setText(1, allPages);
}

void XmlStreamReader::skipUnknownElement()
{
    reader.readNext();
    while (!reader.atEnd()) {
        if (reader.isEndElement()) {
            reader.readNext();
            break;
        }

        if (reader.isStartElement()) {
            skipUnknownElement();
        } else {
            reader.readNext();
        }
    }
}
