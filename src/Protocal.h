#pragma once

//#include <iomanip>
//#include <iostream>
//#include <fstream>
//#include <string>
//#include <math.h>

#include <string>
#include <vector>

struct ImageProtocal
{
	bool bCheck;
	int type;
	int space;
	int dimension;
	int size[4];
	double origin[3];
	double spacing[3];
	double spacedirection[3][3];
};

struct DiffusionProtocal
{
	bool bCheck;
	double b;
	std::vector< std::vector<double> > gradients;
	double measurementFrame[3][3];
};

struct IntensityMotionCheckProtocal
{
	bool bCheck;

	std::string OutputFileName;

	bool bSliceCheck;
	double sliceCorrelationThreshold;
	double sliceCorrelationDeviationThreshold;
	double badSlicePercentageTolerance;


	bool bInterlaceCheck;
	double interlaceCorrelationThresholdBaseline;
	double interlaceCorrelationThresholdGradient;
	double interlaceTranslationThreshold;
	double interlaceRotationThreshold;

	bool bGradientCheck;
	double gradientTranslationThreshold;
	double gradientRotationThreshold;
};

struct EddyMotionCorrectionProtocal
{
	bool bCorrect;
	std::string EddyMotionCommand;
	std::string InputFileName;
	std::string OutputFileName;
};

struct DTIProtocal
{
	bool bCompute;
	std::string dtiestimCommand;
	std::string dtiprocessCommand;

	int method;
	int baselineThreshold;
	std::string mask;

	std::string tensor;

	std::string fa;
	std::string md;
	std::string coloredfa;
	std::string baseline;
	std::string frobeniusnorm;
	std::string idwi;

	bool  bfa;
	bool  bmd;
	bool  bcoloredfa;
	bool  bbaseline;
	bool  bfrobeniusnorm;
	bool  bidwi;

	std::vector< std::vector<std::string> > measures;
};


class Protocal
{
public:
	Protocal(void);
	~Protocal(void);

	enum {
			TYPE_SHORT = 0,
			TYPE_USHORT,
			TYPE_UNKNOWN,
	};

	enum {
			SPACE_LAI = 0,
			SPACE_LAS,
			SPACE_LPI,
			SPACE_LPS,
			SPACE_RAI,
			SPACE_RAS,
			SPACE_RPI,
			SPACE_RPS,
			SPACE_UNKNOWN,

	};

	enum {	METHOD_WLS = 0,
			METHOD_LLS,
			METHOD_ML,
			METHOD_NLS,
			METHOD_UNKNOWN,
	};

	void clear();
	void print();
	//void fromTreeWidget(QTreeWidget tree);
	void fromXMLFile(std::string xml);

	std::string									&GetReportFileName(){				return  ReportFileName;};

	struct ImageProtocal						&GetImageProtocal(){				return  imageProtocal;};
	struct DiffusionProtocal					&GetDiffusionProtocal(){			return 	diffusionProtocal;};
	struct IntensityMotionCheckProtocal			&GetIntensityMotionCheckProtocal(){	return 	intensityMotionCheckProtocal;};
	struct EddyMotionCorrectionProtocal			&GetEddyMotionCorrectionProtocal(){	return 	eddyMotionCorrectionProtocal;};
	struct DTIProtocal							&GetDTIProtocal() {					return 	dTIProtocal;};
private:

	std::string ReportFileName;

	ImageProtocal					imageProtocal;
	DiffusionProtocal				diffusionProtocal;
	IntensityMotionCheckProtocal	intensityMotionCheckProtocal;
	EddyMotionCorrectionProtocal	eddyMotionCorrectionProtocal;
	DTIProtocal						dTIProtocal;
	

};
