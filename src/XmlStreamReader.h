#pragma once


#include <QIcon>
#include <QXmlStreamReader>
#include "Protocal.h"

class QTreeWidget;
class QString;
class QTreeWidgetItem;

#include <map>
#include <string>
#include <iostream>


enum StringValue 
{
	ev_Unknow = 0,
	ev_ReportFileName,
	ev_Image,
	ev_type,
	ev_space,
	ev_directions,
	ev_dimension,
	ev_sizes,
	ev_spacing,
	ev_origin,
	ev_Diffusion,
	ev_measurementFrame,
	ev_bvalue,
	ev_DWMRI_gradient,
	ev_QCCheck,
	ev_QCOutputFileName,
	ev_SliceWiseCheck,  //SliceWiseCheck
	ev_sliceCorrelationThreshold,
	ev_sliceCorrelationDeviationThreshold,
	ev_badSlicePercentageTolerance,
	ev_InterlaceWiseCheck, //InterlaceWiseCheck
	ev_interlaceCorrelationThresholdBaseline,
	ev_interlaceCorrelationThresholdGradient,
	ev_interlaceTranslationThreshold,
	ev_interlaceRotationThreshold,
	ev_GradientWiseCheck,  //GradientWiseCheck
	ev_gradientTranslationThrehshold,
	ev_gradientRotationThreshold,
	ev_EddyMotionCorrection,
	ev_EddyMotionInputFileName,
	ev_EddyMotionOutputFileName,
	ev_DTIComputing,
	ev_DTIMethod,
	ev_baselineThreshold,
	ev_maskFIle,
	ev_tensor,
	ev_fa,
	ev_md,
	ev_colorfa,
	ev_baseline,
	ev_idwi,
	ev_frobeniusnorm,
	ev_EddyMotionCommand,
	ev_dtiestimCommand,
	ev_dtiprocessCommand,


};


class XmlStreamReader
{
public:
    XmlStreamReader(QTreeWidget *tree);
	~XmlStreamReader(void);

	enum{TreeWise = 0, ProtocalWise};

	enum{IMAGE = 0, DIFFUSION, QC, CORRECTION, DTICOMPUTING,};


	// Map to associate the strings with the enum values
	std::map<std::string, int> s_mapStringValues;

	void Initialize()
	{
		s_mapStringValues["ReportFileName"]		= ev_ReportFileName;
		s_mapStringValues["Image"]				= ev_Image;
		s_mapStringValues["type"]				= ev_type;
		s_mapStringValues["space"]				= ev_space;
		s_mapStringValues["space directions"]	= ev_directions;
		s_mapStringValues["dimension"]			= ev_dimension;
		s_mapStringValues["sizes"]				= ev_sizes;
		s_mapStringValues["spacing"]			= ev_spacing;
		s_mapStringValues["origin"]				= ev_origin;

		s_mapStringValues["Diffusion"]			= ev_Diffusion;
		s_mapStringValues["measurement frame"]	= ev_measurementFrame;
		s_mapStringValues["DWMRI_b-value"]		= ev_bvalue;
		s_mapStringValues["DWMRI_gradient"]		= ev_DWMRI_gradient;

		s_mapStringValues["QC Check"]				= ev_QCCheck;
		s_mapStringValues["QCOutputFileName"]		= ev_QCOutputFileName;

		s_mapStringValues["slice wise threshold"]	= ev_SliceWiseCheck;
		s_mapStringValues["slice correlation"]		= ev_sliceCorrelationThreshold;
		s_mapStringValues["slice correlation deviation"]= ev_sliceCorrelationDeviationThreshold;
		s_mapStringValues["bad slice percentage tolerance"]= ev_badSlicePercentageTolerance;

		s_mapStringValues["interlace wise threshold"]= ev_InterlaceWiseCheck;
		s_mapStringValues["interlace correlation baseline"]	= ev_interlaceCorrelationThresholdBaseline;
		s_mapStringValues["interlace correlation gradient"]	= ev_interlaceCorrelationThresholdGradient;
		s_mapStringValues["interlace rotation"]		= ev_interlaceRotationThreshold;
		s_mapStringValues["interlace translation"]	= ev_interlaceTranslationThreshold;

		s_mapStringValues["gradient wise threshold"]= ev_GradientWiseCheck;
		s_mapStringValues["gradient translation"]	= ev_gradientTranslationThrehshold;
		s_mapStringValues["gradient rotation"]		= ev_gradientRotationThreshold;

		s_mapStringValues["Eddy Motion Correction"]  = ev_EddyMotionCorrection;
		s_mapStringValues["Eddy Motion Correction Command"]  = ev_EddyMotionCommand;
		s_mapStringValues["EddyMotionInputFileName"] = ev_EddyMotionInputFileName;
		s_mapStringValues["EddyMotionOutputFileName"] = ev_EddyMotionOutputFileName;

		s_mapStringValues["DTI Computing"]			= ev_DTIComputing;
		s_mapStringValues["dtiestim Command"]		= ev_dtiestimCommand;
		s_mapStringValues["dtiprocess Command"]		= ev_dtiprocessCommand;
		s_mapStringValues["method"]					= ev_DTIMethod;
		s_mapStringValues["baseline threshold"]		= ev_baselineThreshold;
		s_mapStringValues["mask file"]				= ev_maskFIle;
		s_mapStringValues["tensor file"]			= ev_tensor;
		s_mapStringValues["fa"]						= ev_fa;
		s_mapStringValues["md"]						= ev_md;
		s_mapStringValues["colored fa"]				= ev_colorfa;
		s_mapStringValues["baseline"]				= ev_baseline;
		s_mapStringValues["idwi"]					= ev_idwi;
		s_mapStringValues["frobenius norm"]			= ev_frobeniusnorm;

		std::cout << "s_mapStringValues contains "
			<< s_mapStringValues.size()
			<< " entries." << std::endl;
	}



	void setProtocal( Protocal	*p ){ protocal=p; };
	bool readFile(const QString &fileName, int mode);

	struct ITEM
	{
		QString parameter;
		QString value;
	};

	std::vector<ITEM> paremeters;


private:
    void readProtocalSettingsElement(int mode);
    void readEntryElement(QTreeWidgetItem *parent);
    void readValueElement(QTreeWidgetItem *parent);
    void readEntryElement();
    void readValueElement();
    void skipUnknownElement();
	void parseparametersToProtocal();

    QTreeWidget *treeWidget;
	Protocal	*protocal;
    QXmlStreamReader reader;



};
