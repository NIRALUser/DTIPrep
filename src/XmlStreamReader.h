#pragma once

#include <QIcon>
#include <QXmlStreamReader>
#include "Protocol.h"

class QTreeWidget;
class QString;
class QTreeWidgetItem;

#include <map>
#include <string>
#include <iostream>

enum ProtocolStringValue {
  // QC overal
  QC_Unknow = 0,
  QC_QCOutputDirectory,
  QC_QCedDWIFileNameSuffix,
  QC_reportFileNameSuffix,
  QC_badGradientPercentageTolerance,

  // image
  IMAGE_bCheck,
  IMAGE_type,
  IMAGE_space,
  IMAGE_dimension,
  IMAGE_size,
  IMAGE_origin,
  IMAGE_spacing,
  IMAGE_directions,
  IMAGE_bCrop,
  IMAGE_croppedDWIFileNameSuffix,
  IMAGE_reportFileNameSuffix,
  IMAGE_reportFileMode,

  // diffusion
  DIFFUSION_bCheck,
  DIFFUSION_DWMRI_bValue,
  DIFFUSION_DWMRI_gradient,
  DIFFUSION_measurementFrame,
  DIFFUSION_bUseDiffusionProtocol,
  DIFFUSION_diffusionReplacedDWIFileNameSuffix,
  DIFFUSION_reportFileNameSuffix,
  DIFFUSION_reportFileMode,

  // slice check
  SLICE_bCheck,
  //   SLICE_badGradientPercentageTolerance,
  SLICE_checkTimes,
  SLICE_headSkipSlicePercentage,
  SLICE_tailSkipSlicePercentage,
  SLICE_correlationDeviationThresholdbaseline,
  SLICE_correlationDeviationThresholdgradient,
  SLICE_outputDWIFileNameSuffix,
  SLICE_reportFileNameSuffix,
  SLICE_reportFileMode,

  // interlace check
  INTERLACE_bCheck,
  //   INTERLACE_badGradientPercentageTolerance,
  INTERLACE_correlationThresholdBaseline,
  INTERLACE_correlationThresholdGradient,
  INTERLACE_correlationDeviationBaseline,
  INTERLACE_correlationDeviationGradient,
  INTERLACE_translationThreshold,
  INTERLACE_rotationThreshold,
  INTERLACE_outputDWIFileNameSuffix,
  INTERLACE_reportFileNameSuffix,
  INTERLACE_reportFileMode,

  // gradient check
  GRADIENT_bCheck,
  //   GRADIENT_badGradientPercentageTolerance,
  GRADIENT_translationThrehshold,
  GRADIENT_rotationThreshold,
  GRADIENT_outputDWIFileNameSuffix,
  GRADIENT_reportFileNameSuffix,
  GRADIENT_reportFileMode,

  // baseline average
  BASELINE_bAverage,
  BASELINE_averageMethod,
  BASELINE_stopThreshold,
  BASELINE_outputDWIFileNameSuffix,
  BASELINE_reportFileNameSuffix,
  BASELINE_reportFileMode,

  // eddy motion correction
  EDDYMOTION_bCorrect,
  //   EDDYMOTION_command,
  //   EDDYMOTION_inputFileName,
  //   EDDYMOTION_outputFileName,

  EDDYMOTION_numberOfBins,
  EDDYMOTION_numberOfSamples,
  EDDYMOTION_translationScale,
  EDDYMOTION_stepLength,
  EDDYMOTION_relaxFactor,
  EDDYMOTION_maxNumberOfIterations,

  EDDYMOTION_outputDWIFileNameSuffix,
  EDDYMOTION_reportFileNameSuffix,
  EDDYMOTION_reportFileMode,

  // DTI computing
  DTI_bCompute,
  DTI_dtiestimCommand,
  DTI_dtiprocessCommand,
  DTI_method,
  DTI_baselineThreshold,
  DTI_maskFileName,
  DTI_tensor,
  DTI_baseline,
  DTI_idwi,
  DTI_fa,
  DTI_md,
  DTI_colorfa,
  DTI_frobeniusnorm,
  DTI_reportFileNameSuffix,
  DTI_reportFileMode,
  };

class XmlStreamReader
  {
public:
  XmlStreamReader(QTreeWidget *tree);
  ~XmlStreamReader(void);

  enum { TreeWise = 0, ProtocolWise};

  enum { IMAGE = 0, DIFFUSION, QC, CORRECTION, DTICOMPUTING, };

  // Map to associate the strings with the enum values
  std::map<std::string, int> s_mapProtocolStringValues;

  void InitializeProtocolStringValues();

  void setProtocol( Protocol  *p )
  {
    protocol = p;
  }

  bool readFile(const QString & fileName, int mode);

  struct ITEM {
    QString parameter;
    QString value;
    };

  std::vector<ITEM> paremeters;
private:
  void readProtocolSettingsElement(int mode);

  void readEntryElement(QTreeWidgetItem *parent);

  void readValueElement(QTreeWidgetItem *parent);

  void readEntryElement();

  void readValueElement();

  void skipUnknownElement();

  void parseXMLParametersToProtocol();

  QTreeWidget      *treeWidget;
  Protocol         *protocol;
  QXmlStreamReader reader;
  };
