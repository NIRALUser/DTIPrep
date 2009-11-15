#pragma once

#include <string>
#include <vector>
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"

struct ImageProtocol
{
	bool bCheck;
	int type;
	int space;
	int dimension;
  vnl_vector_fixed<unsigned int,4> size;
	vnl_vector_fixed<double,3> origin;
	vnl_vector_fixed<double,3> spacing;
	vnl_matrix_fixed<double,3,3> spacedirection;

	bool bCrop;
	std::string croppedDWIFileNameSuffix;

	std::string reportFileNameSuffix;
	int reportFileMode; // 0: new   1: append
};

struct DiffusionProtocol
{
	bool bCheck;
	double bValue;
	std::vector< std::vector<double> > gradients;
	double measurementFrame[3][3];

	bool bUseDiffusionProtocol;

	std::string diffusionReplacedDWIFileNameSuffix;
	std::string reportFileNameSuffix;
	int reportFileMode; // 0: new   1: append
};

struct SliceCheckProtocol
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

struct InterlaceCheckProtocol
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

struct GradientCheckProtocol
{
	bool bCheck;

	double translationThreshold;
	double rotationThreshold;

	std::string outputDWIFileNameSuffix;
	std::string reportFileNameSuffix;
	int reportFileMode; // 0: new   1: append
};

struct BaselineAverageProtocol
{
	bool bAverage;

	int averageMethod;
	double stopThreshold;

	std::string outputDWIFileNameSuffix;
	std::string reportFileNameSuffix;
	int reportFileMode; // 0: new   1: append
};

struct EddyMotionCorrectionProtocol
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

struct DTIProtocol
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

class Protocol
{
public:
	Protocol(void);
	~Protocol(void);

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

	void initProtocols();
	void initImageProtocol();
	void initDiffusionProtocol();
	void initSliceCheckProtocol();
	void initInterlaceCheckProtocol();
	void initGradientCheckProtocol();
	void initBaselineAverageProtocol();
	void initEddyMotionCorrectionProtocol();
	void initDTIProtocol();

	void printProtocols();
	void printImageProtocol();
	void printDiffusionProtocol();
	void printSliceCheckProtocol();
	void printInterlaceCheckProtocol();
	void printGradientCheckProtocol();
	void printBaselineAverageProtocol();
	void printEddyMotionCorrectionProtocol();
	void printDTIProtocol();

	void clear();
	
	void collectDiffusionStatistics();
	int getBaselineNumber()		{  return baselineNumber;};
	int getBValueNumber()		{  return bValueNumber;};
	int getgradientDirNumber()	{  return gradientDirNumber;};
	int getRepetitionNumber()	{  return repetitionNumber;};

	struct ImageProtocol						&GetImageProtocol(){				return  imageProtocol;};
	struct DiffusionProtocol					&GetDiffusionProtocol(){			return 	diffusionProtocol;};
	struct SliceCheckProtocol					&GetSliceCheckProtocol(){			return  sliceCheckProtocol;};
	struct InterlaceCheckProtocol				&GetInterlaceCheckProtocol(){		return  interlaceCheckProtocol;};
	struct GradientCheckProtocol				&GetGradientCheckProtocol(){		return  gradientCheckProtocol;};
	struct BaselineAverageProtocol				&GetBaselineAverageProtocol(){		return  baselineAverageProtocol;};
	struct EddyMotionCorrectionProtocol			&GetEddyMotionCorrectionProtocol(){	return 	eddyMotionCorrectionProtocol;};
	struct DTIProtocol							&GetDTIProtocol() {					return 	dTIProtocol;};

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

	ImageProtocol					imageProtocol;
	DiffusionProtocol				diffusionProtocol;
	SliceCheckProtocol				sliceCheckProtocol;
	InterlaceCheckProtocol			interlaceCheckProtocol;
	BaselineAverageProtocol			baselineAverageProtocol;
	EddyMotionCorrectionProtocol	eddyMotionCorrectionProtocol;
	GradientCheckProtocol			gradientCheckProtocol;
	DTIProtocol						dTIProtocol;	

};
