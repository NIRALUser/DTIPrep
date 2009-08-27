#pragma once

#include <string>
#include <vector>

struct ImageProtocal
{
	bool bCheck;
	int type;
	int space;
	int dimension;
	unsigned int size[4];
	double origin[3];
	double spacing[3];
	double spacedirection[3][3];

	bool bCrop;
	std::string croppedDWIFileNameSuffix;

	std::string reportFileNameSuffix;
	int reportFileMode; // 0: new   1: append
};

struct DiffusionProtocal
{
	bool bCheck;
	double bValue;
	std::vector< std::vector<double> > gradients;
	double measurementFrame[3][3];

	bool bUseDiffusionProtocal;

	std::string diffusionReplacedDWIFileNameSuffix;
	std::string reportFileNameSuffix;
	int reportFileMode; // 0: new   1: append
};

struct SliceCheckProtocal
{
	bool bCheck;

	int checkTimes;
	double headSkipSlicePercentage;
	double tailSkipSlicePercentage;
	double correlationDeviationThresholdbaseline;
	double correlationDeviationThresholdgradient;

	std::string outputDWIFileNameSuffix;
	std::string reportFileNameSuffix;
	int reportFileMode; // 0: new   1: append
};

struct InterlaceCheckProtocal
{
	bool bCheck;

	double correlationThresholdBaseline;
	double correlationThresholdGradient;
	double correlationDeviationBaseline;
	double correlationDeviationGradient;
	double translationThreshold;
	double rotationThreshold;

	std::string outputDWIFileNameSuffix;
	std::string reportFileNameSuffix;
	int reportFileMode; // 0: new   1: append
};

struct GradientCheckProtocal
{
	bool bCheck;

	double translationThreshold;
	double rotationThreshold;

	std::string outputDWIFileNameSuffix;
	std::string reportFileNameSuffix;
	int reportFileMode; // 0: new   1: append
};

struct BaselineAverageProtocal
{
	bool bAverage;

	int averageMethod;
	double stopThreshold;

	std::string outputDWIFileNameSuffix;
	std::string reportFileNameSuffix;
	int reportFileMode; // 0: new   1: append
};

struct EddyMotionCorrectionProtocal
{
	bool bCorrect;

	int		numberOfBins;
	int		numberOfSamples	;
	double	translationScale;
	double	stepLength;
	double	relaxFactor;
	int		maxNumberOfIterations;

	std::string outputDWIFileNameSuffix;
	std::string reportFileNameSuffix;
	int reportFileMode; // 0: new   1: append
};

struct DTIProtocal
{
	bool bCompute;
	std::string dtiestimCommand;
	std::string dtiprocessCommand;

	int method;
	int baselineThreshold;
	std::string mask;
	std::string tensorSuffix;

	std::string idwiSuffix;
	std::string baselineSuffix;
	std::string faSuffix;
	std::string mdSuffix;
	std::string coloredfaSuffix;
	std::string frobeniusnormSuffix;

	bool  bidwi;
	bool  bbaseline;
	bool  bfa;
	bool  bmd;
	bool  bcoloredfa;
	bool  bfrobeniusnorm;

	std::string reportFileNameSuffix;
	int reportFileMode; // 0: new   1: append
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

	struct DiffusionDir
	{
		std::vector< double > gradientDir;
		int repetitionNumber;
	};

	void initProtocals();
	void initImageProtocal();
	void initDiffusionProtocal();
	void initSliceCheckProtocal();
	void initInterlaceCheckProtocal();
	void initGradientCheckProtocal();
	void initBaselineAverageProtocal();
	void initEddyMotionCorrectionProtocal();
	void initDTIProtocal();

	void printProtocals();
	void printImageProtocal();
	void printDiffusionProtocal();
	void printSliceCheckProtocal();
	void printInterlaceCheckProtocal();
	void printGradientCheckProtocal();
	void printBaselineAverageProtocal();
	void printEddyMotionCorrectionProtocal();
	void printDTIProtocal();

	void clear();
	
	void collectDiffusionStatistics();
	int getBaselineNumber()		{  return baselineNumber;};
	int getBValueNumber()		{  return bValueNumber;};
	int getgradientDirNumber()	{  return gradientDirNumber;};
	int getRepetitionNumber()	{  return repetitionNumber;};

	struct ImageProtocal						&GetImageProtocal(){				return  imageProtocal;};
	struct DiffusionProtocal					&GetDiffusionProtocal(){			return 	diffusionProtocal;};
	struct SliceCheckProtocal					&GetSliceCheckProtocal(){			return  sliceCheckProtocal;};
	struct InterlaceCheckProtocal				&GetInterlaceCheckProtocal(){		return  interlaceCheckProtocal;};
	struct GradientCheckProtocal				&GetGradientCheckProtocal(){		return  gradientCheckProtocal;};
	struct BaselineAverageProtocal				&GetBaselineAverageProtocal(){		return  baselineAverageProtocal;};
	struct EddyMotionCorrectionProtocal			&GetEddyMotionCorrectionProtocal(){	return 	eddyMotionCorrectionProtocal;};
	struct DTIProtocal							&GetDTIProtocal() {					return 	dTIProtocal;};

	std::string &GetQCOutputDirectory()		{	return QCOutputDirectory;};
	std::string &GetQCedDWIFileNameSuffix() {	return QCedDWIFileNameSuffix;};
	std::string &GetReportFileNameSuffix()	{	return reportFileNameSuffix;};
	double GetBadGradientPercentageTolerance(){ return badGradientPercentageTolerance;};
	void SetBadGradientPercentageTolerance(double tor){  badGradientPercentageTolerance = tor;};

private:

	std::string QCOutputDirectory;
	std::string QCedDWIFileNameSuffix;
	std::string reportFileNameSuffix;
	double badGradientPercentageTolerance;

	int baselineNumber;
	int bValueNumber;
	int gradientDirNumber;
	int repetitionNumber;

	ImageProtocal					imageProtocal;
	DiffusionProtocal				diffusionProtocal;
	SliceCheckProtocal				sliceCheckProtocal;
	InterlaceCheckProtocal			interlaceCheckProtocal;
	BaselineAverageProtocal			baselineAverageProtocal;
	EddyMotionCorrectionProtocal	eddyMotionCorrectionProtocal;
	GradientCheckProtocal			gradientCheckProtocal;
	DTIProtocal						dTIProtocal;	

};
