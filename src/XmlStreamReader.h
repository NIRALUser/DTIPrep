#pragma once

#include <QIcon>
#include <QXmlStreamReader>

#include <QtXml/QDomElement>
#include <QtXml/QDomNode>
#include <QtXml/QDomDocument>
#include <QtXml>

#include "Protocol.h"
#include "QCResult.h"

class QTreeWidget;
class QString;
class QTreeWidgetItem;

#include <map>
#include <string>
#include <iostream>

enum ProtocolStringValue
  {
  // QC overal
  QC_Unknow = 0,
  QC_QCOutputDirectory,
  QC_QCedDWIFileNameSuffix,
  QC_reportFileNameSuffix,
  QC_badGradientPercentageTolerance,
  QC_reportType, // -1: no; 0:simple, 1:verbose, 2: iowa

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

  IMAGE_bQuitOnCheckSpacingFailure,
  IMAGE_bQuitOnCheckSizeFailure,

  // diffusion
  DIFFUSION_bCheck,
  DIFFUSION_DWMRI_bValue,
  DIFFUSION_DWMRI_gradient,
  DIFFUSION_measurementFrame,
  DIFFUSION_bUseDiffusionProtocol,
  DIFFUSION_diffusionReplacedDWIFileNameSuffix,
  DIFFUSION_reportFileNameSuffix,
  DIFFUSION_reportFileMode,

  DIFFUSION_bQuitOnCheckFailure,

  // Denoising 1
  DENOISING_bCheck,
  DENOISING_Path,
  DENOISING_ParameterSet,
  DENOISING_NumIter,
  DENOISING_Est_Radius,
  DENOISING_Filter_Radius,
  DENOISING_Min_VoxelNum_Filter,
  DENOISING_Min_VoxelNum_Est,
  DENOISING_MinNoiseSTD,
  DENOISING_MaxNoiseSTD,
  DENOISING_HistogramResolution,
  DENOISING_AbsoluteValue,

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

  SLICE_bSubregionalCheck,
  SLICE_subregionalCheckRelaxationFactor,
  SLICE_excludedDWINrrdFileNameSuffix,
  SLICE_bQuitOnCheckFailure,

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
  INTERLACE_excludedDWINrrdFileNameSuffix,
  INTERLACE_bQuitOnCheckFailure,

  // gradient check
  GRADIENT_bCheck,
  //   GRADIENT_badGradientPercentageTolerance,
  GRADIENT_translationThrehshold,
  GRADIENT_rotationThreshold,
  GRADIENT_outputDWIFileNameSuffix,
  GRADIENT_reportFileNameSuffix,
  GRADIENT_reportFileMode,
  GRADIENT_excludedDWINrrdFileNameSuffix,
  GRADIENT_bQuitOnCheckFailure,

  // Denoising 2
  JOINDENOISING_bCheck,
  JOINDENOISING_Path,
  JOINDENOISING_ParameterSet,
  JOINDENOISING_NumNeighborGradients,
  JOINDENOISING_Est_Radius,
  JOINDENOISING_Filter_Radius,

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

enum QCRESULTStringValue
  {

  INCLUDE = 0,
  BASELINE_AVERAGED,
  EDDY_MOTION_CORRECTED,
  EXCLUDE_SLICECHECK,
  EXCLUDE_INTERLACECHECK,
  EXCLUDE_GRADIENTCHECK,
  EXCLUDE_MANUALLY,

  IMG_INFO,
  IMG_ORIGIN,
  IMG_SIZE,
  IMG_SPACE,
  IMG_SPACEDIRECTION,

  DIFF_B,
  DIFF_GRADIENT,
  DIFF_MEASUREMENTFRAME,

  DWI_SWCk,
  DWI_IWCk,
  DWI_GWCk,

  DWI_SLICEWISECHECK,
  DWI_SLICE,
  DWI_CORRELATION,

  DWI_INTERLACEWISECHECK,
  DWI_INTERLACEX,
  DWI_INTERLACEY,
  DWI_INTERLACEZ,
  DWI_INTERLACE_TRX,
  DWI_INTERLACE_TRY,
  DWI_INTERLACE_TRZ,
  DWI_INTERLACE_MI,
  DWI_INTERLACE_CORRELATION,

  DWI_GRADIENTWISECHECK,
  DWI_GRADIENTX,
  DWI_GRADIENTY,
  DWI_GRADIENTZ,
  DWI_GRADIENT_TRX,
  DWI_GRADIENT_TRY,
  DWI_GRADIENT_TRz,
  DWI_GRADIENT_TRZ,
  DWI_GRADIENT_MI,
  DWI_QC_Index
  };

class XmlStreamReader
{
public:
  XmlStreamReader(QTreeWidget *tree);
  ~XmlStreamReader(void);

  enum { TreeWise = 0, ProtocolWise, QCResultlWise };

  enum { IMAGE = 0, DIFFUSION, QC, CORRECTION, DTICOMPUTING, };

  // Map to associate the strings with the enum values
  std::map<std::string, int> s_mapProtocolStringValues;
  std::map<std::string, int> s_mapQCRESULTStringValue;

  void InitializeProtocolStringValues();

  void InitializeQCRESULTStringValue();

  void setProtocol( Protocol  *p )
  {
    protocol = p;
  }

  void setQCRESULT( QCResult * q )
  {
    QCRESULT = q;
  }

  bool readFile(const QString & fileName, int mode);

  bool readFile_QCResult(const QString & fileName, int mode);   // Reading QCResult in xml format

  void parseQCResultElement( const QDomElement & element);

  void GetImgInfoParsing(const QDomElement & element);

  void parseEntryElement_QCResult_ImgInfo( const QDomElement & element, QTreeWidgetItem *parent);

  void parseValueElement_QCResult_ImgInfo(const QDomElement & element, QTreeWidgetItem *parent);

  void LoadQCResultFromImgInfoParsing();

  void GetDiffInfoParsing(const QDomElement & element);

  void LoadQCResultFromDiffInfoParsing();

  void LoadQCResultFromDWICheckParsing();

  void parseValueElement_QCResult_GradientDWICheck(const QDomElement & element, QTreeWidgetItem *parent);

  void parseEntryElement_QCResult_DWICheck(const QDomNodeList & childList, QTreeWidgetItem *parent);

  void parseEntryElement_QCResult_GradientDWICheck(const QDomElement & element, QTreeWidgetItem *parent);

  void LoadQCResultFromDWICheckGradientParsing(int grd_num);

  QDomNodeList GetDWICheckParsing(const QDomNode & element);

  struct ITEM
    {
    QString parameter;
    QString value;
    };

  std::vector<ITEM> paremeters;
  std::vector<ITEM> parametersQCResult;
  std::vector<ITEM> parametersQCResult_Gradient;  // Contains all information about each the gradient
private:
  void readProtocolSettingsElement(int mode);

  void readElement_QCResult(int mode);

  void readEntryElement(QTreeWidgetItem *parent);

  void readEntryElement_QCResult(QTreeWidgetItem *parent);

  void readValueElement(QTreeWidgetItem *parent);

  void readValueElement_QCResult(QTreeWidgetItem *parent);

  void readProcessingElement_QCResult(QTreeWidgetItem *parent);

  void readGreenElement_QCResult(QTreeWidgetItem * parent);

  void readRedElement_QCResult(QTreeWidgetItem * parent);

  void readEntryElement();

  void readEntryElement_QCResult();

  void readValueElement();

  void readValueElement_QCResult();

  void skipUnknownElement();

  void parseXMLParametersToProtocol();

  void parseXMLParametersToQCResult();

  QTreeWidget *    treeWidget;
  QTreeWidgetItem * m_tree_item_DWICheck;
  Protocol *       protocol;
  QCResult *       QCRESULT;
  QXmlStreamReader reader;
};
