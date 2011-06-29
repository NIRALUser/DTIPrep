#include "XmlStreamReader.h"
#include <QtGui>
#include <iostream>


XmlStreamReader::XmlStreamReader(QTreeWidget *tree)
{
  treeWidget = tree;
  protocol = NULL;
  QCRESULT = NULL;
}

XmlStreamReader::~XmlStreamReader(void)
{}

void XmlStreamReader::InitializeProtocolStringValues()
{
  // QC overall
  s_mapProtocolStringValues["QC_QCOutputDirectory"]
  = QC_QCOutputDirectory;
  s_mapProtocolStringValues["QC_QCedDWIFileNameSuffix"]
  = QC_QCedDWIFileNameSuffix;
  s_mapProtocolStringValues["QC_reportFileNameSuffix"]
  = QC_reportFileNameSuffix;
  s_mapProtocolStringValues["QC_badGradientPercentageTolerance"]
  = QC_badGradientPercentageTolerance;
  s_mapProtocolStringValues["QC_reportType"]
  = QC_reportType;

  // image
  s_mapProtocolStringValues["IMAGE_bCheck"]
  = IMAGE_bCheck;
  s_mapProtocolStringValues["IMAGE_type"]
  = IMAGE_type;
  s_mapProtocolStringValues["IMAGE_space"]
  = IMAGE_space;
  s_mapProtocolStringValues["IMAGE_directions"]
  = IMAGE_directions;
  s_mapProtocolStringValues["IMAGE_dimension"]
  = IMAGE_dimension;
  s_mapProtocolStringValues["IMAGE_size"]
  = IMAGE_size;
  s_mapProtocolStringValues["IMAGE_spacing"]
  = IMAGE_spacing;
  s_mapProtocolStringValues["IMAGE_origin"]
  = IMAGE_origin;
  s_mapProtocolStringValues["IMAGE_bCrop"]
  = IMAGE_bCrop;
  s_mapProtocolStringValues["IMAGE_croppedDWIFileNameSuffix"]
  = IMAGE_croppedDWIFileNameSuffix;
  s_mapProtocolStringValues["IMAGE_reportFileNameSuffix"]
  = IMAGE_reportFileNameSuffix;
  s_mapProtocolStringValues["IMAGE_reportFileMode"]
  = IMAGE_reportFileMode;
  s_mapProtocolStringValues["IMAGE_bQuitOnCheckSpacingFailure"]
  = IMAGE_bQuitOnCheckSpacingFailure;
  s_mapProtocolStringValues["IMAGE_bQuitOnCheckSizeFailure"]
  = IMAGE_bQuitOnCheckSizeFailure;

  // diffusion
  s_mapProtocolStringValues["DIFFUSION_bCheck"]
  = DIFFUSION_bCheck;
  s_mapProtocolStringValues["DIFFUSION_measurementFrame"]
  = DIFFUSION_measurementFrame;
  s_mapProtocolStringValues["DIFFUSION_DWMRI_bValue"]
  = DIFFUSION_DWMRI_bValue;
  s_mapProtocolStringValues["DIFFUSION_DWMRI_gradient"]
  = DIFFUSION_DWMRI_gradient;
  s_mapProtocolStringValues["DIFFUSION_bUseDiffusionProtocol"]
  = DIFFUSION_bUseDiffusionProtocol;
  s_mapProtocolStringValues["DIFFUSION_diffusionReplacedDWIFileNameSuffix"]
  = DIFFUSION_diffusionReplacedDWIFileNameSuffix;
  s_mapProtocolStringValues["DIFFUSION_reportFileNameSuffix"]
  = DIFFUSION_reportFileNameSuffix;
  s_mapProtocolStringValues["DIFFUSION_reportFileMode"]
  = DIFFUSION_reportFileMode;
  s_mapProtocolStringValues["DIFFUSION_bQuitOnCheckFailure"]
  = DIFFUSION_bQuitOnCheckFailure;

  // slice check
  s_mapProtocolStringValues["SLICE_bCheck"]
  = SLICE_bCheck;
  //   s_mapProtocolStringValues["SLICE_badGradientPercentageTolerance"]    =
  // SLICE_badGradientPercentageTolerance;
  s_mapProtocolStringValues["SLICE_bSubregionalCheck"]
  = SLICE_bSubregionalCheck;
  s_mapProtocolStringValues["SLICE_subregionalCheckRelaxationFactor"]
  = SLICE_subregionalCheckRelaxationFactor;
  s_mapProtocolStringValues["SLICE_checkTimes"]
  = SLICE_checkTimes;
  s_mapProtocolStringValues["SLICE_headSkipSlicePercentage"]
  = SLICE_headSkipSlicePercentage;
  s_mapProtocolStringValues["SLICE_tailSkipSlicePercentage"]
  = SLICE_tailSkipSlicePercentage;
  s_mapProtocolStringValues["SLICE_correlationDeviationThresholdbaseline"]
  = SLICE_correlationDeviationThresholdbaseline;
  s_mapProtocolStringValues["SLICE_correlationDeviationThresholdgradient"]
  = SLICE_correlationDeviationThresholdgradient;
  s_mapProtocolStringValues["SLICE_outputDWIFileNameSuffix"]
  = SLICE_outputDWIFileNameSuffix;
  s_mapProtocolStringValues["SLICE_reportFileNameSuffix"]
  = SLICE_reportFileNameSuffix;
  s_mapProtocolStringValues["SLICE_reportFileMode"]
  = SLICE_reportFileMode;
  s_mapProtocolStringValues["SLICE_excludedDWINrrdFileNameSuffix"]
  = SLICE_excludedDWINrrdFileNameSuffix;
  s_mapProtocolStringValues["SLICE_bQuitOnCheckFailure"]
  = SLICE_bQuitOnCheckFailure;

  // interlace check
  s_mapProtocolStringValues["INTERLACE_bCheck"]              = INTERLACE_bCheck;
  //   s_mapProtocolStringValues["INTERLACE_badGradientPercentageTolerance"]  =
  // INTERLACE_badGradientPercentageTolerance;
  s_mapProtocolStringValues["INTERLACE_correlationThresholdBaseline"]
  = INTERLACE_correlationThresholdBaseline;
  s_mapProtocolStringValues["INTERLACE_correlationThresholdGradient"]
  = INTERLACE_correlationThresholdGradient;
  s_mapProtocolStringValues["INTERLACE_correlationDeviationBaseline"]
  = INTERLACE_correlationDeviationBaseline;
  s_mapProtocolStringValues["INTERLACE_correlationDeviationGradient"]
  = INTERLACE_correlationDeviationGradient;
  s_mapProtocolStringValues["INTERLACE_translationThreshold"]
  = INTERLACE_translationThreshold;
  s_mapProtocolStringValues["INTERLACE_rotationThreshold"]
  = INTERLACE_rotationThreshold;
  s_mapProtocolStringValues["INTERLACE_outputDWIFileNameSuffix"]
  = INTERLACE_outputDWIFileNameSuffix;
  s_mapProtocolStringValues["INTERLACE_reportFileNameSuffix"]
  = INTERLACE_reportFileNameSuffix;
  s_mapProtocolStringValues["INTERLACE_reportFileMode"]
  = INTERLACE_reportFileMode;
  s_mapProtocolStringValues["INTERLACE_reportFileMode"]
  = INTERLACE_reportFileMode;
  s_mapProtocolStringValues["INTERLACE_excludedDWINrrdFileNameSuffix"]
  = INTERLACE_excludedDWINrrdFileNameSuffix;
  s_mapProtocolStringValues["INTERLACE_bQuitOnCheckFailure"]
  = INTERLACE_bQuitOnCheckFailure;

  // gradient check
  s_mapProtocolStringValues["GRADIENT_bCheck"]              = GRADIENT_bCheck;
  //   s_mapProtocolStringValues["GRADIENT_badGradientPercentageTolerance"]
  //= GRADIENT_badGradientPercentageTolerance;
  s_mapProtocolStringValues["GRADIENT_translationThrehshold"]
  = GRADIENT_translationThrehshold;
  s_mapProtocolStringValues["GRADIENT_rotationThreshold"]
  = GRADIENT_rotationThreshold;
  s_mapProtocolStringValues["GRADIENT_outputDWIFileNameSuffix"]
  = GRADIENT_outputDWIFileNameSuffix;
  s_mapProtocolStringValues["GRADIENT_reportFileNameSuffix"]
  = GRADIENT_reportFileNameSuffix;
  s_mapProtocolStringValues["GRADIENT_reportFileMode"]
  = GRADIENT_reportFileMode;
  s_mapProtocolStringValues["GRADIENT_excludedDWINrrdFileNameSuffix"]
  = GRADIENT_excludedDWINrrdFileNameSuffix;
  s_mapProtocolStringValues["GRADIENT_bQuitOnCheckFailure"]
  = GRADIENT_bQuitOnCheckFailure;

  // baseline average
  s_mapProtocolStringValues["BASELINE_bAverage"]
  = BASELINE_bAverage;
  s_mapProtocolStringValues["BASELINE_averageMethod"]
  = BASELINE_averageMethod;
  s_mapProtocolStringValues["BASELINE_stopThreshold"]
  = BASELINE_stopThreshold;
  s_mapProtocolStringValues["BASELINE_outputDWIFileNameSuffix"]
  = BASELINE_outputDWIFileNameSuffix;
  s_mapProtocolStringValues["BASELINE_reportFileNameSuffix"]
  = BASELINE_reportFileNameSuffix;
  s_mapProtocolStringValues["BASELINE_reportFileMode"]
  = BASELINE_reportFileMode;

  // eddy motion correction
  s_mapProtocolStringValues["EDDYMOTION_bCorrect"]
  = EDDYMOTION_bCorrect;

  //   s_mapProtocolStringValues["EDDYMOTION_command"]              =
  // EDDYMOTION_command;
  //   s_mapProtocolStringValues["EDDYMOTION_inputFileName"]          =
  // EDDYMOTION_inputFileName;
  //   s_mapProtocolStringValues["EDDYMOTION_outputFileName"]          =
  // EDDYMOTION_outputFileName;

  s_mapProtocolStringValues["EDDYMOTION_numberOfBins"]
  = EDDYMOTION_numberOfBins;
  s_mapProtocolStringValues["EDDYMOTION_numberOfSamples"]
  = EDDYMOTION_numberOfSamples;
  s_mapProtocolStringValues["EDDYMOTION_translationScale"]
  = EDDYMOTION_translationScale;
  s_mapProtocolStringValues["EDDYMOTION_stepLength"]
  = EDDYMOTION_stepLength;
  s_mapProtocolStringValues["EDDYMOTION_relaxFactor"]
  = EDDYMOTION_relaxFactor;
  s_mapProtocolStringValues["EDDYMOTION_maxNumberOfIterations"]
  = EDDYMOTION_maxNumberOfIterations;

  s_mapProtocolStringValues["EDDYMOTION_outputDWIFileNameSuffix"]
  = EDDYMOTION_outputDWIFileNameSuffix;
  s_mapProtocolStringValues["EDDYMOTION_reportFileNameSuffix"]
  = EDDYMOTION_reportFileNameSuffix;
  s_mapProtocolStringValues["EDDYMOTION_reportFileMode"]
  = EDDYMOTION_reportFileMode;

  // DTI computing
  s_mapProtocolStringValues["DTI_bCompute"]
  = DTI_bCompute;
  s_mapProtocolStringValues["DTI_dtiestimCommand"]
  = DTI_dtiestimCommand;
  s_mapProtocolStringValues["DTI_dtiprocessCommand"]
  = DTI_dtiprocessCommand;
  s_mapProtocolStringValues["DTI_method"]
  = DTI_method;
  s_mapProtocolStringValues["DTI_baselineThreshold"]
  = DTI_baselineThreshold;
  s_mapProtocolStringValues["DTI_maskFileName"]
  = DTI_maskFileName;
  s_mapProtocolStringValues["DTI_tensor"]
  = DTI_tensor;
  s_mapProtocolStringValues["DTI_baseline"]
  = DTI_baseline;
  s_mapProtocolStringValues["DTI_idwi"]
  = DTI_idwi;
  s_mapProtocolStringValues["DTI_fa"]
  = DTI_fa;
  s_mapProtocolStringValues["DTI_md"]
  = DTI_md;
  s_mapProtocolStringValues["DTI_colorfa"]
  = DTI_colorfa;
  s_mapProtocolStringValues["DTI_frobeniusnorm"]
  = DTI_frobeniusnorm;
  s_mapProtocolStringValues["DTI_reportFileNameSuffix"]
  = DTI_reportFileNameSuffix;
  s_mapProtocolStringValues["DTI_reportFileMode"]
  = DTI_reportFileMode;

  //  std::cout << "s_mapProtocolStringValues contains "
  //      << s_mapProtocolStringValues.size()
  //      << " entries." << std::endl;
}

void XmlStreamReader::InitializeQCRESULTStringValue()
{
 
 //ImageInfo
 s_mapQCRESULTStringValue[ "ImageInformation" ]
 = IMG_INFO;
 s_mapQCRESULTStringValue[ "origin" ]
 = IMG_ORIGIN;
 s_mapQCRESULTStringValue[ "size" ]
 = IMG_SIZE; 
 s_mapQCRESULTStringValue[ "space" ]
 = IMG_SPACE;
 s_mapQCRESULTStringValue[ "spacedirection" ]
 = IMG_SPACEDIRECTION;
 
 //DiffusionInformationCheckResult
 s_mapQCRESULTStringValue[ "b value" ]
 = DIFF_B;
 s_mapQCRESULTStringValue[ "gradient" ]
 = DIFF_GRADIENT;
 s_mapQCRESULTStringValue[ "measurementframe" ]
 = DIFF_MEASUREMENTFRAME;

 //DWICheck
 s_mapQCRESULTStringValue[ "SliceWiseCheck" ]
 = DWI_SWCk;
 s_mapQCRESULTStringValue[ "InterlaceWiseCheck" ]
 = DWI_IWCk;
 s_mapQCRESULTStringValue[ "GWCk" ]
 = DWI_GWCk;
 s_mapQCRESULTStringValue[ "SWCk" ]
 = DWI_SWCk;

 s_mapQCRESULTStringValue[ "Slice#" ]
 = DWI_SLICE;
 s_mapQCRESULTStringValue[ "Correlation" ]
 = DWI_CORRELATION;
 s_mapQCRESULTStringValue[ "Slice#" ]
 = DWI_SLICE;

 s_mapQCRESULTStringValue[ "InterlaceWiseCheckProcessing" ]
 = DWI_INTERLACEWISECHECK;
 s_mapQCRESULTStringValue[ "InterlaceAngleX" ]
 = DWI_INTERLACEX;
 s_mapQCRESULTStringValue[ "InterlaceAngleY" ]
 = DWI_INTERLACEY;
 s_mapQCRESULTStringValue[ "InterlaceAngleZ" ]
 = DWI_INTERLACEZ;
 s_mapQCRESULTStringValue[ "InterlaceTranslationX" ]
 = DWI_INTERLACE_TRX;
 s_mapQCRESULTStringValue[ "InterlaceTranslationY" ]
 = DWI_INTERLACE_TRY;
 s_mapQCRESULTStringValue[ "InterlaceTranslationZ" ]
 = DWI_INTERLACE_TRZ;
 s_mapQCRESULTStringValue[ "InterlaceMetric(MI)" ]
 = DWI_INTERLACE_MI;
 s_mapQCRESULTStringValue[ "InterlaceCorrelation_Baseline" ]
 = DWI_INTERLACE_CORRELATION;

s_mapQCRESULTStringValue[ "GradientWiseCheckProcessing" ]
 = DWI_GRADIENTWISECHECK;
s_mapQCRESULTStringValue[ "GradientAngleX" ]
 = DWI_GRADIENTX;
 s_mapQCRESULTStringValue[ "GradientAngleY" ]
 = DWI_GRADIENTY;
 s_mapQCRESULTStringValue[ "GradientAngleZ" ]
 = DWI_GRADIENTZ;
 s_mapQCRESULTStringValue[ "GradientTranslationX" ]
 = DWI_GRADIENT_TRX;
 s_mapQCRESULTStringValue[ "GradientTranslationY" ]
 = DWI_GRADIENT_TRY;
 s_mapQCRESULTStringValue[ "GradientTranslationZ" ]
 = DWI_GRADIENT_TRZ;
 s_mapQCRESULTStringValue[ "GradientMetric(MI)" ]
 = DWI_GRADIENT_MI;


/*
 s_mapQCResultStringValues[ "IntensityMotion" ]
 = IntensityMotion;
 s_mapQCResultStringValues[ "gradient_0000" ]
 = gradient_0000;
 s_mapQCResultStringValues[ "OriginalDir" ]
 = OriginalDir;
 s_mapQCResultStringValues[ "ReplacedDir" ]
 = ReplacedDir;
 s_mapQCResultStringValues[ "CorrectedDir" ]
 = CorrectedDir;
 s_mapQCResultStringValues[ "SliceWiseCheck" ]
 = SliceWiseCheck;
 s_mapQCResultStringValues[ "InterlaceWiseCheck" ]
 = InterlaceWiseCheck;
 s_mapQCResultStringValues[ "InterlaceAngleX" ]
 = InterlaceAngleX;
 s_mapQCResultStringValues[ "InterlaceAngleY" ]
 = InterlaceAngleY;
 s_mapQCResultStringValues[ "InterlaceAngleZ" ]
 = InterlaceAngleZ;
 s_mapQCResultStringValues[ "InterlaceTranslationX" ]
 = InterlaceTranslationX;
 s_mapQCResultStringValues[ "InterlaceTranslationY" ]
 = InterlaceTranslationY;
 s_mapQCResultStringValues[ "InterlaceTranslationZ" ]
 = InterlaceTranslationZ;
 s_mapQCResultStringValues[ "InterlaceMetric(MI)" ]
 = InterlaceMetric(MI);
 s_mapQCResultStringValues[ "InterlaceCorrelation_Baseline" ]
 = InterlaceCorrelation_Baseline;
 s_mapQCResultStringValues[ "GradientWiseCheck" ]
 = GradientWiseCheck;
 s_mapQCResultStringValues[ "GradientAngleX" ]
 = GradientWiseCheck;
 s_mapQCResultStringValues[ "GradientAngleY" ]
 = GradientWiseCheck;
 s_mapQCResultStringValues[ "GradientAngleZ" ]
 = GradientWiseCheck;
 s_mapQCResultStringValues[ "GradientTranslationX" ]
 = GradientWiseCheck;
 s_mapQCResultStringValues[ "GradientTranslationY" ]
 = GradientWiseCheck;
 s_mapQCResultStringValues[ "GradientTranslationZ" ]
 = GradientWiseCheck;
 s_mapQCResultStringValues[ "GradientMetric(MI)" ]
 = GradientWiseCheck;*/

 s_mapQCRESULTStringValue[ "INCLUDE" ] = INCLUDE ;
 s_mapQCRESULTStringValue[ "BASELINE_AVERAGED" ] = BASELINE_AVERAGED ;
 s_mapQCRESULTStringValue[ "EDDY_MOTION_CORRECTED" ] = EDDY_MOTION_CORRECTED ;
 s_mapQCRESULTStringValue[ "EXCLUDE_SLICECHECK" ] = EXCLUDE_SLICECHECK ;
 s_mapQCRESULTStringValue[ "EXCLUDE_INTERLACECHECK" ] = EXCLUDE_INTERLACECHECK ;
 s_mapQCRESULTStringValue[ "EXCLUDE_GRADIENTCHECK" ] = EXCLUDE_GRADIENTCHECK ;
 s_mapQCRESULTStringValue[ "EXCLUDE_MANUALLY" ] = EXCLUDE_MANUALLY ;
 
}

bool XmlStreamReader::readFile(const QString & fileName, int mode)
{
  if ( protocol )
  {
    protocol->clear();
  }

  QFile file(fileName);
  if ( !file.open(QFile::ReadOnly | QFile::Text) )
  {
    std::cerr << "Error: Cannot read file " << qPrintable(fileName)
      << ": " << qPrintable( file.errorString() )
      << std::endl;
    return false;
  }
  reader.setDevice(&file);

  reader.readNext();
  while ( !reader.atEnd() )
  {
    if ( reader.isStartElement() )
    {
      if ( reader.name() == "ProtocolSettings" )
      {
        readProtocolSettingsElement( mode );
      }
      else
      {
        reader.raiseError( QObject::tr("Not a ProtocolSettings file") );
      }
    }
    else
    {
      reader.readNext();
    }
  }

  file.close();
  if ( reader.hasError() )
  {
    std::cerr << "Error: Failed to parse file "
      << qPrintable(fileName) << ": "
      << qPrintable( reader.errorString() ) << std::endl;
    return false;
  }
  else if ( file.error() != QFile::NoError )
  {
    std::cerr << "Error: Cannot read file " << qPrintable(fileName)
      << ": " << qPrintable( file.errorString() )
      << std::endl;
    return false;
  }
  return true;
}

bool XmlStreamReader::readFile_QCResult(const QString & fileName, int mode)
{
  QFile file(fileName);
  if ( !file.open(QFile::ReadOnly | QFile::Text) )
  {
    std::cerr << "Error: Cannot read file " << qPrintable(fileName)
      << ": " << qPrintable( file.errorString() )
      << std::endl;
    return false;
  }

  QString errorStr;
  int errorLine;
  int errorColumn;

  QDomDocument doc;
  if ( !doc.setContent( &file, false, &errorStr, 
                  &errorLine, &errorColumn)){
     std::cerr<<"Error: Parse error at line" <<errorLine<<","
              <<"Column"<<errorColumn<<":"
              <<qPrintable(errorStr)<<std::endl;
     return false;
  }

  QDomElement root = doc.documentElement();
  if (root.tagName() != "QCResultSettings") {
     std::cerr<<"Error: Not a QCResult file"<<std::endl;
     return false;
  }
  
  InitializeQCRESULTStringValue();
  parseQCResultElement(root);
  return true;

  /*reader.setDevice(&file);

  reader.readNext();
  while ( !reader.atEnd() )
  {
    if ( reader.isStartElement() )
    {
      if ( reader.name() == "QCResultSettings" )
      {
        readElement_QCResult( mode );
      }
      else
      {
        reader.raiseError( QObject::tr("Not a ProtocolSettings file") );
      }
    }
    else
    {
      reader.readNext();
    }
  }

  file.close();
  if ( reader.hasError() )
  {
    std::cerr << "Error: Failed to parse file "
      << qPrintable(fileName) << ": "
      << qPrintable( reader.errorString() ) << std::endl;
    return false;
  }
  else if ( file.error() != QFile::NoError )
  {
    std::cerr << "Error: Cannot read file " << qPrintable(fileName)
      << ": " << qPrintable( file.errorString() )
      << std::endl;
    return false;
  }
  return true;
  */
}

void XmlStreamReader::parseQCResultElement( const QDomElement &element)
{
  
  QDomNode child = element.firstChild();
  GetImgInfoParsing(child.toElement());  //ImageInfo
  LoadQCResultFromImgInfoParsing();
  parametersQCResult.clear();
  

  child = child.nextSibling();
  if (child.isNull()){
      std::cerr<<"Improper xml QCResult file"<<std::endl;
  }
  GetDiffInfoParsing(child.toElement());   // DiffusionInformation
  LoadQCResultFromDiffInfoParsing();
  parametersQCResult.clear();


  child = child.nextSibling();
  if (child.isNull()){
      std::cerr<<"Inproper xml QCResult file"<<std::endl;
  }
  
  GetDWICheckParsing(child);   // DWI Check
  LoadQCResultFromDWICheckParsing();
 

}

void XmlStreamReader::GetImgInfoParsing(const QDomElement &element)
{

  // Image Information
  
  if(element.tagName()=="entry"){
         parseEntryElement_QCResult_ImgInfo( element, treeWidget->invisibleRootItem());
  }
  
  else {
        std::cerr<<"Error: Inproper xml QCResult file"<<std::endl;
        return;
  }

}

void XmlStreamReader::parseEntryElement_QCResult_ImgInfo( const QDomElement &element, QTreeWidgetItem *parent)
{

  QTreeWidgetItem * tree_item = new QTreeWidgetItem(parent);
  tree_item->setText(0, element.attribute("parameter"));

  ITEM item;
  item.parameter = element.attribute("parameter");
  parametersQCResult.push_back(item);


  QDomNode child = element.firstChild();
  while (!child.isNull()){
      if(child.toElement().tagName()=="entry"){
            parseEntryElement_QCResult_ImgInfo( child.toElement(), tree_item);
  }
  else if(child.toElement().tagName()=="value"){
            parseValueElement_QCResult_ImgInfo( child.toElement(),tree_item);
  }
  child = child.nextSibling();
  }

}

void XmlStreamReader::parseValueElement_QCResult_ImgInfo(const QDomElement &element, QTreeWidgetItem *parent)
{
  QBrush greenText (Qt::green);
  QBrush redText (Qt::red);

  QString page = element.text();
  QString allPages = parent->text(1);
  if (!allPages.isEmpty())
     allPages +=",";
  allPages +=page;
  parent->setText(1, allPages);
  parametersQCResult[ parametersQCResult.size() -1].value = allPages;
  if (element.tagName()=="green") 
     parent->setForeground(1,greenText);
  if (element.tagName()=="red")
     parent->setForeground(1,redText);
}


void XmlStreamReader::LoadQCResultFromImgInfoParsing()
{


  for ( int i=0; i<parametersQCResult.size(); i++)
  {
     
     switch (s_mapQCRESULTStringValue[parametersQCResult[i].parameter.toStdString()])
     {
     case IMG_INFO:
     QCRESULT->GetImageInformationCheckResult().info = parametersQCResult[i].value;
     break;
     case IMG_ORIGIN:
     {
     if(parametersQCResult[i].value == "Pass")
         QCRESULT->GetImageInformationCheckResult().origin = true;
     else
         QCRESULT->GetImageInformationCheckResult().origin = false;

     break;}
     case IMG_SIZE:
     {
     if(parametersQCResult[i].value == "Pass")
         QCRESULT->GetImageInformationCheckResult().size = true;
     else
         QCRESULT->GetImageInformationCheckResult().size = false;

     break;}
     case IMG_SPACE:
     {
     if(parametersQCResult[i].value == "Pass")
         QCRESULT->GetImageInformationCheckResult().space = true;
     else
         QCRESULT->GetImageInformationCheckResult().space = false;

     break;}
     case IMG_SPACEDIRECTION:
     {
     if(parametersQCResult[i].value == "Pass")
         QCRESULT->GetImageInformationCheckResult().spacedirection = true;
     else
         QCRESULT->GetImageInformationCheckResult().spacedirection = false;

     break;}
     default:
     break;
     }
  }
}

void XmlStreamReader::GetDiffInfoParsing(const QDomElement &element)
{

  // DiffusionInformation
  if(element.tagName()=="entry"){
         parseEntryElement_QCResult_ImgInfo( element, treeWidget->invisibleRootItem());     // Since the process is same as ImageInfo part we call parseEntryElement_QCResult_ImgInfo
  }
  
  else {
        std::cerr<<"Error: Inproper xml QCResult file"<<std::endl;
        return;
  }

}

void XmlStreamReader::LoadQCResultFromDiffInfoParsing()
{

for ( int i=0; i<parametersQCResult.size(); i++)
  {
     switch (s_mapQCRESULTStringValue[parametersQCResult[i].parameter.toStdString()])
     {
     case DIFF_B:
     {
     if(parametersQCResult[i].value == "Pass")
         QCRESULT->GetDiffusionInformationCheckResult().b = true;
     else
         QCRESULT->GetDiffusionInformationCheckResult().b = false;

     break;}
     case DIFF_GRADIENT:
     {
     if(parametersQCResult[i].value == "Pass")
         QCRESULT->GetDiffusionInformationCheckResult().gradient = true;
     else
         QCRESULT->GetDiffusionInformationCheckResult().gradient = false;

     break;}
     case DIFF_MEASUREMENTFRAME:
     {
     if(parametersQCResult[i].value == "Pass")
         QCRESULT->GetDiffusionInformationCheckResult().measurementFrame = true;
     else
         QCRESULT->GetDiffusionInformationCheckResult().measurementFrame = false;

     break;}
     default:
     break;
     }
  }
}

QDomNodeList XmlStreamReader::GetDWICheckParsing(const QDomNode &element)
{

  // DWI Check
  QTreeWidgetItem * tree_item_DWICheck = new QTreeWidgetItem(treeWidget->invisibleRootItem());
  tree_item_DWICheck->setText(0, element.toElement().attribute("parameter"));

  ITEM item;
  item.parameter = element.toElement().attribute("parameter");
  parametersQCResult.push_back(item);

  
  QDomNodeList childList = element.childNodes();   // gets all direct childs of DWICheck element
  if (element.toElement().tagName()=="entry")
  {
       parseEntryElement_QCResult_DWICheck( childList, tree_item_DWICheck);
  }
  else{
        std::cerr<<"Error: Inproper xml QCResult file"<<std::endl;
        
  }
  return childList;

}


void XmlStreamReader::parseEntryElement_QCResult_DWICheck(const QDomNodeList &childList,QTreeWidgetItem *parent)
{
  int Grd_num = 0;

  for ( int i=0; i< childList.size(); i++)
  {
   
   QDomNode child_d = childList.item( i );
  
   if ( child_d.toElement().attribute("parameter").left(9) == QObject::tr("gradient_"))
   {
      parametersQCResult_Gradient.clear();
      parseEntryElement_QCResult_GradientDWICheck( child_d.toElement(), parent );
      LoadQCResultFromDWICheckGradientParsing(Grd_num);

      Grd_num++;
   }
   else{
      QTreeWidgetItem * tree_item = new QTreeWidgetItem(parent);
      tree_item->setText(0, child_d.toElement().attribute("parameter"));

      ITEM item;
      item.parameter = child_d.toElement().attribute("parameter");
      parametersQCResult.push_back(item);

      QDomNode child = child_d.toElement().firstChild();
      
      if(child.toElement().tagName()=="value"){
                 parseValueElement_QCResult_ImgInfo( child.toElement(),tree_item);
      }
      else
         std::cerr<<"Error:Improper xml QCResult"<<std::endl;
      
      }

  
  } 
}

void XmlStreamReader::parseEntryElement_QCResult_GradientDWICheck(const QDomElement & element, QTreeWidgetItem *parent)
{
      QTreeWidgetItem * tree_item = new QTreeWidgetItem(parent);
      tree_item->setText(0, element.attribute("parameter"));
      
      ITEM item;
      item.parameter = element.attribute("parameter");
      parametersQCResult_Gradient.push_back(item);
   
      QDomNode child = element.firstChild();
      while (!child.isNull()){
          if(child.toElement().tagName()=="entry"){
                 parseEntryElement_QCResult_GradientDWICheck( child.toElement(), tree_item);
          }
          else if(child.toElement().tagName()=="value"|| child.toElement().tagName()=="processing" ||child.toElement().tagName()=="green"||child.toElement().tagName()=="red"){
                 parseValueElement_QCResult_GradientDWICheck( child.toElement(),tree_item);
          }
          child = child.nextSibling();
      }

}

void XmlStreamReader::parseValueElement_QCResult_GradientDWICheck(const QDomElement &element, QTreeWidgetItem *parent)
{
  QBrush greenText (Qt::green);
  QBrush redText (Qt::red);

  QString page = element.text();
  QString allPages = parent->text(1);
  if (!allPages.isEmpty())
     allPages +=",";
  allPages +=page;
  if (parametersQCResult_Gradient[parametersQCResult_Gradient.size() -1].parameter.left(9) ==
      QObject::tr("gradient_") && element.tagName()=="processing"){
       parent->setText(2, allPages);
       parametersQCResult_Gradient[ parametersQCResult_Gradient.size() -1].value = allPages;
  }
  if (parametersQCResult_Gradient[parametersQCResult_Gradient.size() -1].parameter.left(9) ==
      QObject::tr("gradient_") && element.tagName()=="value")
       parent->setText(3, allPages);

  else if (parametersQCResult_Gradient[parametersQCResult_Gradient.size() -1].parameter.left(9)!= QObject::tr("gradient_")){
       parent->setText(1, allPages);
       parametersQCResult_Gradient[ parametersQCResult_Gradient.size() -1].value = allPages;
  }
  if (element.tagName()=="green") 
     parent->setForeground(1,greenText);
  if (element.tagName()=="red")
     parent->setForeground(1,redText);
}

void XmlStreamReader::LoadQCResultFromDWICheckParsing()
{

for ( int i=0; i<parametersQCResult.size(); i++)
  {
     switch (s_mapQCRESULTStringValue[parametersQCResult[i].parameter.toStdString()])
     {
     case DWI_SWCk:
     {
     if(parametersQCResult[i].value == "Pass")
         QCRESULT->GetOverallQCResult().SWCk = true;
     else
         QCRESULT->GetOverallQCResult().SWCk = false;

     break;}
     case DWI_IWCk:
     {
     if(parametersQCResult[i].value == "Pass")
         QCRESULT->GetOverallQCResult().IWCk = true;
     else
         QCRESULT->GetOverallQCResult().IWCk = false;

     break;}
     case DWI_GWCk:
     {
     if(parametersQCResult[i].value == "Pass")
         QCRESULT->GetOverallQCResult().GWCk = true;
     else
         QCRESULT->GetOverallQCResult().GWCk = false;

     break;}
     default:
     break;
     }
  }
}

void XmlStreamReader::LoadQCResultFromDWICheckGradientParsing(int Grd_num)
{

  GradientIntensityMotionCheckResult GrdIntMotionChk;
  SliceWiseCheckResult SlcWiseCk;
  InterlaceWiseCheckResult IntWiseCk;
  GradientWiseCheckResult GrdWiseCheck;

  //status of the gradient
  GrdIntMotionChk.processing = s_mapQCRESULTStringValue[parametersQCResult_Gradient[0].value.toStdString()];
  //std::cout<<"gradient"<<Grd_num<<parametersQCResult_Gradient[0].value.toStdString().c_str()<<std::endl;
  //std::cout<<"gradient"<<Grd_num<<parametersQCResult_Gradient[1].value.toStdString().c_str()<<std::endl;
  //std::cout<<"gradient"<<Grd_num<<parametersQCResult_Gradient[parametersQCResult_Gradient.size()-1].value.toStdString().c_str()<<std::endl;
  //status of the Visual Checking
  if (parametersQCResult_Gradient[parametersQCResult_Gradient.size()-1].value == "Include")
     GrdIntMotionChk.VisualChecking = 0;
  if (parametersQCResult_Gradient[parametersQCResult_Gradient.size()-1].value == "Exclude")
     GrdIntMotionChk.VisualChecking = 6;
  if (parametersQCResult_Gradient[parametersQCResult_Gradient.size()-1].value == "NoChange")
     GrdIntMotionChk.VisualChecking = -1;

  QStringList values;
  for ( int i=1; i< parametersQCResult_Gradient.size(); i++)
  {
    if ( parametersQCResult_Gradient[i].parameter.left(11) ==
      QObject::tr("OriginalDir"))
      
    {
      values = parametersQCResult_Gradient[i].value.split(" ");
      int ii = 0;
      foreach (QString value, values)
      {
        GrdIntMotionChk.OriginalDir[ii] = ( value.toDouble() );
        ii++;
      }
      
    }
    if ( parametersQCResult_Gradient[i].parameter.left(11) ==
      QObject::tr("ReplacedDir"))
      
    {
      values = parametersQCResult_Gradient[i].value.split(" ");
      int ii = 0;
      foreach (QString value, values)
      {
        GrdIntMotionChk.ReplacedDir[ii] = ( value.toDouble() );
        ii++;
      }
    }
    if ( parametersQCResult_Gradient[i].parameter.left(12) ==
      QObject::tr("CorrectedDir"))
      
    {
      values = parametersQCResult_Gradient[i].value.split(" ");
      int ii = 0;
      foreach (QString value, values)
      {
        GrdIntMotionChk.CorrectedDir[ii] = ( value.toDouble() );
        //std::cout<<"CorrectedDir"<<ii<<" "<<GrdIntMotionChk.OriginalDir[ii]<<std::endl;
        ii++;
      }
    }
    
    switch (s_mapQCRESULTStringValue[parametersQCResult_Gradient[i].parameter.toStdString()])
    {
    case DWI_SLICE:
    {
       SlcWiseCk.GradientNum = Grd_num;
       SlcWiseCk.SliceNum = parametersQCResult_Gradient[i].value.toInt();
       SlcWiseCk.Correlation = parametersQCResult_Gradient[i+1].value.toDouble();
       QCRESULT->GetSliceWiseCheckResult().push_back(SlcWiseCk);
       break;
    }
    case DWI_SLICEWISECHECK:
    {
       QCRESULT->GetSliceWiseCheckProcessing().push_back(s_mapQCRESULTStringValue[parametersQCResult_Gradient[i].value.toStdString()]);
       break;
    }
    case DWI_INTERLACEWISECHECK:
    {
       IntWiseCk.InterlaceWiseCheckProcessing = s_mapQCRESULTStringValue[parametersQCResult_Gradient[i].value.toStdString()];
       break;
    }
    case DWI_INTERLACEX:
    {
       IntWiseCk.AngleX = parametersQCResult_Gradient[i].value.toDouble();
       break;
    }
    case DWI_INTERLACEY:
       IntWiseCk.AngleY = parametersQCResult_Gradient[i].value.toDouble();
       break;
    case DWI_INTERLACEZ:
       IntWiseCk.AngleZ = parametersQCResult_Gradient[i].value.toDouble();
       break;
    case DWI_INTERLACE_TRX:
       IntWiseCk.TranslationX = parametersQCResult_Gradient[i].value.toDouble();
       break;
    case DWI_INTERLACE_TRY:
       IntWiseCk.TranslationY = parametersQCResult_Gradient[i].value.toDouble();
       break;
    case DWI_INTERLACE_TRZ:
       IntWiseCk.TranslationZ = parametersQCResult_Gradient[i].value.toDouble();
       break;
    case DWI_INTERLACE_MI:
       IntWiseCk.Metric = parametersQCResult_Gradient[i].value.toDouble();
       break;
    case DWI_INTERLACE_CORRELATION:
       IntWiseCk.Correlation = parametersQCResult_Gradient[i].value.toDouble();
       break;
    case DWI_GRADIENTWISECHECK:
       GrdWiseCheck.GradientWiseCheckProcessing = s_mapQCRESULTStringValue[parametersQCResult_Gradient[i].value.toStdString()];
       break;
    case DWI_GRADIENTX:
       GrdWiseCheck.AngleX = parametersQCResult_Gradient[i].value.toDouble();
       break;
    case DWI_GRADIENTY:
       GrdWiseCheck.AngleY = parametersQCResult_Gradient[i].value.toDouble();
       break;
    case DWI_GRADIENTZ:
       GrdWiseCheck.AngleZ = parametersQCResult_Gradient[i].value.toDouble();
       break;
    case DWI_GRADIENT_TRX:
       GrdWiseCheck.TranslationX = parametersQCResult_Gradient[i].value.toDouble();
       break;
    case DWI_GRADIENT_TRY:
       GrdWiseCheck.TranslationY = parametersQCResult_Gradient[i].value.toDouble();
       break;
    case DWI_GRADIENT_TRZ:
       GrdWiseCheck.TranslationZ = parametersQCResult_Gradient[i].value.toDouble();
       break;
    case DWI_GRADIENT_MI:
       GrdWiseCheck.MutualInformation = parametersQCResult_Gradient[i].value.toDouble();
       break;  
    default:
       break;
  }
}
  QCRESULT->GetIntensityMotionCheckResult().push_back(GrdIntMotionChk);
  QCRESULT->GetInterlaceWiseCheckResult().push_back(IntWiseCk);
  QCRESULT->GetGradientWiseCheckResult().push_back(GrdWiseCheck);
  parametersQCResult_Gradient.clear();

}

void XmlStreamReader::readProtocolSettingsElement(int mode)
{
  reader.readNext();
  while ( !reader.atEnd() )
  {
    if ( reader.isEndElement() )
    {
      reader.readNext();
      break;
    }

    if ( reader.isStartElement() )
    {
      if ( reader.name() == "entry" )
      {
        if ( mode == XmlStreamReader::TreeWise )
        {
          readEntryElement( treeWidget->invisibleRootItem() );
        }
        else if ( mode == XmlStreamReader::ProtocolWise )
        {
          readEntryElement();
        }
        else
        {
          std::cout << "invalid setting reading mode" << std::endl;
          return;
        }
      }
      else
      {
        skipUnknownElement();
      }
    }
    else
    {
      reader.readNext();
    }
  }

  if ( mode == XmlStreamReader::ProtocolWise )
  {
    InitializeProtocolStringValues();
    parseXMLParametersToProtocol();
  }
}

void XmlStreamReader::readElement_QCResult(int mode)
{
  reader.readNext();
  while ( !reader.atEnd() )
  {
    if ( reader.isEndElement() )
    {
      reader.readNext();
      break;
    }

    if ( reader.isStartElement() )
    {
      if ( reader.name() == "entry" )
      {
        if ( mode == XmlStreamReader::TreeWise )
        {
          readEntryElement_QCResult( treeWidget->invisibleRootItem() );
        }
        else if ( mode == XmlStreamReader::QCResultlWise )
        {
          readEntryElement_QCResult();
        }
        else
        {
          std::cout << "invalid setting reading mode" << std::endl;
          return;
        }
      }
      else
      {
        skipUnknownElement();
      }
    }
    else
    {
      reader.readNext();
    }
  }

  if ( mode == XmlStreamReader::QCResultlWise )
  {
    InitializeQCRESULTStringValue();
    parseXMLParametersToQCResult();
  }
}

void XmlStreamReader::parseXMLParametersToQCResult()
{

  for ( unsigned int i = 0; i < parametersQCResult.size(); i++ )
  {
   
   if ( parametersQCResult[i].parameter.left(9) ==
      QObject::tr("gradient_") )
   {
    
    GradientIntensityMotionCheckResult Grd;
   // Grd.processing = paremeters[i].value.toInt();

   Grd.processing = s_mapQCRESULTStringValue[parametersQCResult[i].value.toStdString()]; 

   QCRESULT->GetIntensityMotionCheckResult().push_back( Grd );
      
        
   }

  }
}

void XmlStreamReader::parseXMLParametersToProtocol()
{
  int         temp;
  QStringList values;
  QStringList subvalues;

  for ( unsigned int i = 0; i < paremeters.size(); i++ )
  {
    // std::cout<<paremeters[i].parameter.toStdString()<<"
    //    "<<paremeters[i].value.toStdString()<<std::endl;
    if ( paremeters[i].parameter.left(24) ==
      QObject::tr("DIFFUSION_DWMRI_gradient") )
    {
      vnl_vector_fixed<double, 3> vect;
      values = paremeters[i].value.split(" ");
      {
        int ii = 0;
        foreach (QString value, values)
        {
          vect[ii] = ( value.toDouble() );
          ii++;
        }
      }
      protocol->GetDiffusionProtocol(). gradients.push_back(vect);
    }
    else
    {
      switch ( s_mapProtocolStringValues[paremeters[i].parameter.toStdString()] )
      {
        // QC overall
      case QC_QCOutputDirectory:
        protocol->GetQCOutputDirectory() =  paremeters[i].value.toStdString();
        break;
      case QC_QCedDWIFileNameSuffix:
        protocol->GetQCedDWIFileNameSuffix()
          =  paremeters[i].value.toStdString();
        break;
      case QC_reportFileNameSuffix:
        protocol->GetReportFileNameSuffix()
          =  paremeters[i].value.toStdString();
        break;
      case QC_badGradientPercentageTolerance:
        protocol->SetBadGradientPercentageTolerance(
          paremeters[i].value.toDouble() );
        break;
      case QC_reportType:
        protocol->SetReportType(
          paremeters[i].value.toInt() );
        break;

        // image
      case IMAGE_bCheck:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetImageProtocol(). bCheck = true;
        }
        else
        {
          protocol->GetImageProtocol(). bCheck = false;
        }
        break;
      case IMAGE_type:
        if ( paremeters[i].value.toStdString() == "short" )
        {
          protocol->GetImageProtocol(). type = Protocol::TYPE_SHORT;
        }
        else if ( paremeters[i].value.toStdString() == "unsigned short" )
        {
          protocol->GetImageProtocol(). type = Protocol::TYPE_USHORT;
        }
        else
        {
          protocol->GetImageProtocol(). type = Protocol::TYPE_UNKNOWN;
        }
        break;
      case IMAGE_space:
        if ( paremeters[i].value.toStdString() == "left-anterior-inferior" )
        {
          protocol->GetImageProtocol(). space = Protocol::SPACE_LAI;
        }
        else if ( paremeters[i].value.toStdString() ==
          "left-anterior-superior" )
        {
          protocol->GetImageProtocol(). space = Protocol::SPACE_LAS;
        }
        else if ( paremeters[i].value.toStdString() ==
          "left-posterior-inferior" )
        {
          protocol->GetImageProtocol(). space = Protocol::SPACE_LPI;
        }
        else if ( paremeters[i].value.toStdString() ==
          "left-posterior-superior" )
        {
          protocol->GetImageProtocol(). space = Protocol::SPACE_LPS;
        }
        else if ( paremeters[i].value.toStdString() ==
          "right-anterior-inferior" )
        {
          protocol->GetImageProtocol(). space = Protocol::SPACE_RAI;
        }
        else if ( paremeters[i].value.toStdString() ==
          "right-anterior-superior" )
        {
          protocol->GetImageProtocol(). space = Protocol::SPACE_RAS;
        }
        else if ( paremeters[i].value.toStdString() ==
          "right-posterior-inferior" )
        {
          protocol->GetImageProtocol(). space = Protocol::SPACE_RPI;
        }
        else if ( paremeters[i].value.toStdString() ==
          "right-posterior-superior" )
        {
          protocol->GetImageProtocol(). space = Protocol::SPACE_RPS;
        }
        else
        {
          protocol->GetImageProtocol(). space = Protocol::SPACE_UNKNOWN;
        }
        break;
      case IMAGE_directions:
        values = paremeters[i].value.split(", ");
        temp = 0;
        foreach (QString value, values)
        {
          subvalues = value.split(" ");
          foreach (QString subvalue, subvalues)
          {
            protocol->GetImageProtocol(). spacedirection[temp / 3][temp % 3] // row dominant
            = subvalue.toDouble();
            temp++;
          }
        }
          std::cout << protocol->GetImageProtocol(). spacedirection << std::endl;
        break;
      case IMAGE_dimension:
        protocol->GetImageProtocol().dimension = paremeters[i].value.toInt();
        break;
      case IMAGE_size:
        values = paremeters[i].value.split(", ");
        temp = 0;
        foreach (QString value, values)
        {
          protocol->GetImageProtocol(). size[temp] = value.toInt();
          temp++;
        }
        break;
      case IMAGE_spacing:
        values = paremeters[i].value.split(", ");
        temp = 0;
        foreach (QString value, values)
        {
          protocol->GetImageProtocol(). spacing[temp] = value.toDouble();
          temp++;
        }
        break;
      case IMAGE_origin:
        values = paremeters[i].value.split(", ");
        temp = 0;
        foreach (QString value, values)
        {
          protocol->GetImageProtocol(). origin[temp] = value.toDouble();
          temp++;
        }
        break;
      case IMAGE_bCrop:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetImageProtocol(). bCrop = true;
        }
        else
        {
          protocol->GetImageProtocol(). bCrop = false;
        }
        break;
      case IMAGE_croppedDWIFileNameSuffix:
        protocol->GetImageProtocol().croppedDWIFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case IMAGE_reportFileNameSuffix:
        protocol->GetImageProtocol().reportFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case IMAGE_reportFileMode:
        protocol->GetImageProtocol().reportFileMode
          =  paremeters[i].value.toInt();
        break;
      case IMAGE_bQuitOnCheckSpacingFailure:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetImageProtocol(). bQuitOnCheckSpacingFailure = true;
        } 
        else
        {
          protocol->GetImageProtocol(). bQuitOnCheckSpacingFailure = false;
        }       
        break;
      case IMAGE_bQuitOnCheckSizeFailure:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetImageProtocol(). bQuitOnCheckSizeFailure = true;
        } 
        else
        {
          protocol->GetImageProtocol(). bQuitOnCheckSizeFailure = false;
        }       
        break;

        // Diffusion
      case DIFFUSION_bCheck:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetDiffusionProtocol(). bCheck = true;
        }
        else
        {
          protocol->GetDiffusionProtocol(). bCheck = false;
        }
        break;
      case DIFFUSION_measurementFrame:
        values = paremeters[i].value.split(", ");
        temp = 0;
        foreach (QString value, values)
        {
          subvalues = value.split(" ");
          foreach (QString subvalue, subvalues)
          {
            // JTM - Second wrong 
            protocol->GetDiffusionProtocol(). measurementFrame[temp / 3][temp % 3] // row dominant
            = subvalue.toDouble();
            temp++;
          }
        }
          std::cout << protocol->GetDiffusionProtocol(). measurementFrame << std::endl;
        break;
      case DIFFUSION_DWMRI_bValue:
        protocol->GetDiffusionProtocol().bValue
          =  paremeters[i].value.toDouble();
        break;
        // case ev_DWMRI_gradient:
        //  break;
      case DIFFUSION_bUseDiffusionProtocol:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetDiffusionProtocol(). bUseDiffusionProtocol = true;
        }
        else
        {
          protocol->GetDiffusionProtocol(). bUseDiffusionProtocol = false;
        }
        break;
      case DIFFUSION_diffusionReplacedDWIFileNameSuffix:
        protocol->GetDiffusionProtocol().diffusionReplacedDWIFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case DIFFUSION_reportFileNameSuffix:
        protocol->GetDiffusionProtocol().reportFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case DIFFUSION_reportFileMode:
        protocol->GetDiffusionProtocol().reportFileMode
          =  paremeters[i].value.toInt();
        break;
      case DIFFUSION_bQuitOnCheckFailure:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetDiffusionProtocol(). bQuitOnCheckFailure = true;
        }
        else
        {
          protocol->GetDiffusionProtocol(). bQuitOnCheckFailure = false;
        }
        break;

        // slice Check
      case SLICE_bCheck:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetSliceCheckProtocol(). bCheck = true;
        }
        else
        {
          protocol->GetSliceCheckProtocol(). bCheck = false;
        }
        break;
        //       case SLICE_badGradientPercentageTolerance:
        //
        //
        //
        //
        //      protocol->GetSliceCheckProtocol().badGradientPercentageTolerance
        // =  paremeters[i].value.toDouble();
        //         break;
      case SLICE_checkTimes:
        protocol->GetSliceCheckProtocol().checkTimes
          =  paremeters[i].value.toInt();
        break;
      case SLICE_headSkipSlicePercentage:
        protocol->GetSliceCheckProtocol().headSkipSlicePercentage
          = paremeters[i].value.toDouble();
        break;
      case SLICE_tailSkipSlicePercentage:
        protocol->GetSliceCheckProtocol().tailSkipSlicePercentage
          = paremeters[i].value.toDouble();
        break;
      case SLICE_correlationDeviationThresholdbaseline:
        protocol->GetSliceCheckProtocol().
          correlationDeviationThresholdbaseline
          = paremeters[i].value.toDouble();
        break;
      case SLICE_correlationDeviationThresholdgradient:
        protocol->GetSliceCheckProtocol().
          correlationDeviationThresholdgradient
          = paremeters[i].value.toDouble();
        break;
      case SLICE_outputDWIFileNameSuffix:
        protocol->GetSliceCheckProtocol().outputDWIFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case SLICE_reportFileNameSuffix:
        protocol->GetSliceCheckProtocol().reportFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case SLICE_reportFileMode:
        protocol->GetSliceCheckProtocol().reportFileMode
          =  paremeters[i].value.toInt();
        break;
      case SLICE_bSubregionalCheck:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetSliceCheckProtocol(). bSubregionalCheck = true;
        }
        else
        {  
          protocol->GetSliceCheckProtocol(). bSubregionalCheck = false;
        }
        break;
      case SLICE_subregionalCheckRelaxationFactor:
        protocol->GetSliceCheckProtocol().subregionalCheckRelaxationFactor
          = paremeters[i].value.toDouble();
        break;
      case SLICE_excludedDWINrrdFileNameSuffix:
        protocol->GetSliceCheckProtocol().excludedDWINrrdFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case SLICE_bQuitOnCheckFailure:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetSliceCheckProtocol(). bQuitOnCheckFailure = true;
        }
        else
        {
          protocol->GetSliceCheckProtocol(). bQuitOnCheckFailure = false;
        }
        break;

        // interlace check
      case INTERLACE_bCheck:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetInterlaceCheckProtocol(). bCheck = true;
        }
        else
        {
          protocol->GetInterlaceCheckProtocol(). bCheck = false;
        }
        break;
        //case INTERLACE_badGradientPercentageTolerance:
        //  protocol->GetInterlaceCheckProtocol().badGradientPercentageTolerance
        //    =  paremeters[i].value.toDouble();
        // break;
      case INTERLACE_correlationThresholdBaseline:
        protocol->GetInterlaceCheckProtocol().correlationThresholdBaseline
          = paremeters[i].value.toDouble();
        break;
      case INTERLACE_correlationThresholdGradient:
        protocol->GetInterlaceCheckProtocol().correlationThresholdGradient
          = paremeters[i].value.toDouble();
        break;
      case INTERLACE_correlationDeviationBaseline:
        protocol->GetInterlaceCheckProtocol().correlationDeviationBaseline
          = paremeters[i].value.toDouble();
        break;
      case INTERLACE_correlationDeviationGradient:
        protocol->GetInterlaceCheckProtocol().correlationDeviationGradient
          = paremeters[i].value.toDouble();
        break;
      case INTERLACE_translationThreshold:
        protocol->GetInterlaceCheckProtocol().translationThreshold
          = paremeters[i].value.toDouble();
        break;
      case INTERLACE_rotationThreshold:
        protocol->GetInterlaceCheckProtocol().rotationThreshold
          = paremeters[i].value.toDouble();
        break;
      case INTERLACE_outputDWIFileNameSuffix:
        protocol->GetInterlaceCheckProtocol().outputDWIFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case INTERLACE_reportFileNameSuffix:
        protocol->GetInterlaceCheckProtocol().reportFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case INTERLACE_reportFileMode:
        protocol->GetInterlaceCheckProtocol().reportFileMode
          =  paremeters[i].value.toInt();
        break;
      case INTERLACE_excludedDWINrrdFileNameSuffix:
        protocol->GetInterlaceCheckProtocol().excludedDWINrrdFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case INTERLACE_bQuitOnCheckFailure:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetInterlaceCheckProtocol(). bQuitOnCheckFailure = true;
        }
        else
        {
          protocol->GetInterlaceCheckProtocol(). bQuitOnCheckFailure = false;
        }
        break;

        // gradient check
      case GRADIENT_bCheck:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetGradientCheckProtocol(). bCheck = true;
        }
        else
        {
          protocol->GetGradientCheckProtocol(). bCheck = false;
        }
        break;
        // case GRADIENT_badGradientPercentageTolerance:
        //   protocol->GetGradientCheckProtocol().badGradientPercentageTolerance
        // =  paremeters[i].value.toDouble();
        //         break;
      case GRADIENT_translationThrehshold:
        protocol->GetGradientCheckProtocol().translationThreshold
          = paremeters[i].value.toDouble();
        break;
      case GRADIENT_rotationThreshold:
        protocol->GetGradientCheckProtocol().rotationThreshold
          = paremeters[i].value.toDouble();
        break;
      case GRADIENT_outputDWIFileNameSuffix:
        protocol->GetGradientCheckProtocol().outputDWIFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case GRADIENT_reportFileNameSuffix:
        protocol->GetGradientCheckProtocol().reportFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case GRADIENT_reportFileMode:
        protocol->GetGradientCheckProtocol().reportFileMode
          =  paremeters[i].value.toInt();
        break;
      case GRADIENT_excludedDWINrrdFileNameSuffix:
        protocol->GetGradientCheckProtocol().excludedDWINrrdFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case GRADIENT_bQuitOnCheckFailure:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetGradientCheckProtocol(). bQuitOnCheckFailure = true;
        }
        else
        {
          protocol->GetGradientCheckProtocol(). bQuitOnCheckFailure = false;
        }
        break;

        // baseline average
      case BASELINE_bAverage:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetBaselineAverageProtocol(). bAverage = true;
        }
        else
        {
          protocol->GetBaselineAverageProtocol(). bAverage = false;
        }
        break;
      case BASELINE_averageMethod:
        protocol->GetBaselineAverageProtocol().averageMethod
          =  paremeters[i].value.toInt();
        break;
      case BASELINE_stopThreshold:
        protocol->GetBaselineAverageProtocol().stopThreshold
          = paremeters[i].value.toDouble();
        break;
      case BASELINE_outputDWIFileNameSuffix:
        protocol->GetBaselineAverageProtocol().outputDWIFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case BASELINE_reportFileNameSuffix:
        protocol->GetBaselineAverageProtocol().reportFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case BASELINE_reportFileMode:
        protocol->GetBaselineAverageProtocol().reportFileMode
          =  paremeters[i].value.toInt();
        break;

        // eddy current
      case EDDYMOTION_bCorrect:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetEddyMotionCorrectionProtocol(). bCorrect = true;
        }
        else
        {
          protocol->GetEddyMotionCorrectionProtocol(). bCorrect = false;
        }
        break;
        //       case EDDYMOTION_command:
        //         protocol->GetEddyMotionCorrectionProtocol().EddyMotionCommand
        // =  paremeters[i].value.toStdString();
        //         break;
        //       case EDDYMOTION_inputFileName:
        //         protocol->GetEddyMotionCorrectionProtocol().InputFileName =
        // paremeters[i].value.toStdString();
        //         break;
        //       case EDDYMOTION_outputFileName:
        //         protocol->GetEddyMotionCorrectionProtocol().OutputFileName =
        // paremeters[i].value.toStdString();
        //         break;

      case EDDYMOTION_numberOfBins:
        protocol->GetEddyMotionCorrectionProtocol().numberOfBins
          =  paremeters[i].value.toInt();
        break;
      case EDDYMOTION_numberOfSamples:
        protocol->GetEddyMotionCorrectionProtocol().numberOfSamples
          =  paremeters[i].value.toInt();
        break;
      case EDDYMOTION_translationScale:
        protocol->GetEddyMotionCorrectionProtocol().translationScale
          =  paremeters[i].value.toFloat();
        break;
      case EDDYMOTION_stepLength:
        protocol->GetEddyMotionCorrectionProtocol().stepLength
          =  paremeters[i].value.toFloat();
        break;
      case EDDYMOTION_relaxFactor:
        protocol->GetEddyMotionCorrectionProtocol().relaxFactor
          =  paremeters[i].value.toFloat();
        break;
      case EDDYMOTION_maxNumberOfIterations:
        protocol->GetEddyMotionCorrectionProtocol().maxNumberOfIterations
          =  paremeters[i].value.toInt();
        break;

      case EDDYMOTION_outputDWIFileNameSuffix:
        protocol->GetEddyMotionCorrectionProtocol().outputDWIFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case EDDYMOTION_reportFileNameSuffix:
        protocol->GetEddyMotionCorrectionProtocol().reportFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case EDDYMOTION_reportFileMode:
        protocol->GetEddyMotionCorrectionProtocol().reportFileMode
          =  paremeters[i].value.toInt();
        break;

        //  DTI
      case DTI_bCompute:
        if ( paremeters[i].value.toLower().toStdString() == "yes" )
        {
          protocol->GetDTIProtocol(). bCompute = true;
        }
        else
        {
          protocol->GetDTIProtocol(). bCompute = false;
        }
        break;
      case DTI_dtiestimCommand:
        protocol->GetDTIProtocol().dtiestimCommand
          =  paremeters[i].value.toStdString();
        break;
      case DTI_dtiprocessCommand:
        protocol->GetDTIProtocol().dtiprocessCommand
          =  paremeters[i].value.toStdString();
        break;
      case DTI_method:
        if ( paremeters[i].value.toStdString() == "wls" )
        {
          protocol->GetDTIProtocol(). method = Protocol::METHOD_WLS;
        }
        else if ( paremeters[i].value.toStdString() == "lls" )
        {
          protocol->GetDTIProtocol(). method = Protocol::METHOD_LLS;
        }
        else if ( paremeters[i].value.toStdString() == "ml" )
        {
          protocol->GetDTIProtocol(). method = Protocol::METHOD_ML;
        }
        else
        {
          protocol->GetDTIProtocol(). method = Protocol::METHOD_UNKNOWN;
        }
        break;
      case DTI_baselineThreshold:
        protocol->GetDTIProtocol().baselineThreshold
          =  paremeters[i].value.toInt();
        break;
      case DTI_maskFileName:
        protocol->GetDTIProtocol().mask =  paremeters[i].value.toStdString();
        break;
      case DTI_tensor:
        protocol->GetDTIProtocol().tensorSuffix
          = paremeters[i].value.toStdString();
        break;
      case DTI_fa:
        values = paremeters[i].value.split(", ");
        protocol->GetDTIProtocol(). faSuffix = values[1].toStdString();
        if ( values[0].toLower().toStdString() == "yes" )
        {
          protocol->GetDTIProtocol(). bfa = true;
        }
        else
        {
          protocol->GetDTIProtocol(). bfa = false;
        }
        break;
      case DTI_md:
        values = paremeters[i].value.split(", ");
        protocol->GetDTIProtocol(). mdSuffix = values[1].toStdString();
        if ( values[0].toLower().toStdString() == "yes" )
        {
          protocol->GetDTIProtocol(). bmd = true;
        }
        else
        {
          protocol->GetDTIProtocol(). bmd = false;
        }
        break;
      case DTI_colorfa:
        values = paremeters[i].value.split(", ");
        protocol->GetDTIProtocol(). coloredfaSuffix = values[1].toStdString();
        if ( values[0].toLower().toStdString() == "yes" )
        {
          protocol->GetDTIProtocol(). bcoloredfa = true;
        }
        else
        {
          protocol->GetDTIProtocol(). bcoloredfa = false;
        }
        break;
      case DTI_idwi:
        values = paremeters[i].value.split(", ");
        protocol->GetDTIProtocol(). idwiSuffix  = values[1].toStdString();
        if ( values[0].toLower().toStdString() == "yes" )
        {
          protocol->GetDTIProtocol(). bidwi = true;
        }
        else
        {
          protocol->GetDTIProtocol(). bidwi = false;
        }
        break;

      case DTI_frobeniusnorm:
        values = paremeters[i].value.split(", ");
        protocol->GetDTIProtocol(). frobeniusnormSuffix
          = values[1].toStdString();
        if ( values[0].toLower().toStdString() == "yes" )
        {
          protocol->GetDTIProtocol(). bfrobeniusnorm = true;
        }
        else
        {
          protocol->GetDTIProtocol(). bfrobeniusnorm = false;
        }
        break;

      case DTI_baseline:
        values = paremeters[i].value.split(", ");
        protocol->GetDTIProtocol(). baselineSuffix  = values[1].toStdString();
        if ( values[0].toLower().toStdString() == "yes" )
        {
          protocol->GetDTIProtocol(). bbaseline = true;
        }
        else
        {
          protocol->GetDTIProtocol(). bbaseline = false;
        }
        break;
      case DTI_reportFileNameSuffix:
        protocol->GetDTIProtocol().reportFileNameSuffix
          =  paremeters[i].value.toStdString();
        break;
      case DTI_reportFileMode:
        protocol->GetDTIProtocol().reportFileMode
          =  paremeters[i].value.toInt();
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

  item->setText( 0, reader.attributes().value("parameter").toString() );

  reader.readNext();
  while ( !reader.atEnd() )
  {
    if ( reader.isEndElement() )
    {
      reader.readNext();
      break;
    }

    if ( reader.isStartElement() )
    {
      if ( reader.name() == "entry" )
      {
        readEntryElement(item);
      }
      else if ( reader.name() == "value" )
      {
        readValueElement(item);
      }
      else
      {
        skipUnknownElement();
      }
    }
    else
    {
      reader.readNext();
    }
  }
}

void XmlStreamReader::readEntryElement_QCResult(QTreeWidgetItem *parent)
{
  QTreeWidgetItem *item = new QTreeWidgetItem(parent);

  item->setText( 0, reader.attributes().value("parameter").toString() );

  reader.readNext();
  while ( !reader.atEnd() )
  {
    if ( reader.isEndElement() )
    {
      if ( reader.name() == "entry" )
      {
	//stack->poplast();
	}
      reader.readNext();
      break;
    }

    if ( reader.isStartElement() )
    {
      if ( reader.name() == "entry" )
      {
  	//stack->push(reader.attributes().value("parameter").toString());
        readEntryElement_QCResult(item);
      }
      else if ( reader.name() == "value" )
      {
        readValueElement_QCResult(item);
      }
      else if ( reader.name()=="processing")
      {
        readProcessingElement_QCResult(item);
      }
      else if (reader.name() == "red" )
     {
       readRedElement_QCResult(item);
     }
     else if (reader.name() == "green" )
     {
       readGreenElement_QCResult(item);
     }
     else
      {
        skipUnknownElement();
      }
    }
    else
    {
      reader.readNext();
    }
  }
}

void XmlStreamReader::readValueElement_QCResult(QTreeWidgetItem *parent)
{
  QString page = reader.readElementText();

  if ( reader.isEndElement() )
  {
    reader.readNext();
  }

  QString allPages = parent->text(1);
  if ( !allPages.isEmpty() )
  {
    allPages += ", ";
  }
  allPages += page;

  parent->setText(1, allPages);
}

void XmlStreamReader::readProcessingElement_QCResult(QTreeWidgetItem *parent)
{
  QString page = reader.readElementText();

  if ( reader.isEndElement() )
  {
    reader.readNext();
  }

  parent->setText(2, page);
}

void XmlStreamReader::readRedElement_QCResult(QTreeWidgetItem *parent)
{
  QBrush redText (Qt::red);
  QString page = reader.readElementText();

  if ( reader.isEndElement() )
  {
    reader.readNext();
  }

  parent->setText(1, page);
  parent->setForeground(1,redText);
}

void XmlStreamReader::readGreenElement_QCResult(QTreeWidgetItem *parent)
{
  QBrush greenText (Qt::green);
  QString page = reader.readElementText();

  if ( reader.isEndElement() )
  {
    reader.readNext();
  }

  parent->setText(1, page);
  parent->setForeground(1,greenText);
}

void XmlStreamReader::readEntryElement()
{
  ITEM item;

  item.parameter = reader.attributes().value("parameter").toString();
  paremeters.push_back( item );

  reader.readNext();
  while ( !reader.atEnd() )
  {
    if ( reader.isEndElement() )
    {
      reader.readNext();
      break;
    }

    if ( reader.isStartElement() )
    {
      if ( reader.name() == "entry" )
      {
        readEntryElement();
      }
      else if ( reader.name() == "value" )
      {
        readValueElement();
      }
      else
      {
        skipUnknownElement();
      }
    }
    else
    {
      reader.readNext();
    }
  }
}

void XmlStreamReader::readEntryElement_QCResult()
{
  ITEM item;
  item.parameter = reader.attributes().value("parameter").toString();
  parametersQCResult.push_back( item );

  reader.readNext();
  while ( !reader.atEnd() )
  {
    if ( reader.isEndElement() )
    {
      reader.readNext();
      break;
    }

    if ( reader.isStartElement() )
    {
      if ( reader.name() == "entry" )
      {
        readEntryElement_QCResult();
      }
      else if ( reader.name() == "value" || reader.name() == "processing" || reader.name() == "green" || reader.name() == "red")
      {
        readValueElement_QCResult();
      }
      else
      {
        skipUnknownElement();
      }
    }
    else
    {
      reader.readNext();
    }
  }

}

void XmlStreamReader::readValueElement_QCResult()
{
  QString page = reader.readElementText();

  if ( reader.isEndElement() )
  {
    reader.readNext();
  }

  QString allPages = parametersQCResult[parametersQCResult.size() - 1].value;
  if ( !allPages.isEmpty() )
  {
    allPages += ", ";
  }
  allPages += page;

  parametersQCResult[parametersQCResult.size() - 1].value = allPages;
}

void XmlStreamReader::readValueElement()
{
  QString page = reader.readElementText();

  if ( reader.isEndElement() )
  {
    reader.readNext();
  }

  QString allPages = paremeters[paremeters.size() - 1].value;
  if ( !allPages.isEmpty() )
  {
    allPages += ", ";
  }
  allPages += page;

  paremeters[paremeters.size() - 1].value = allPages;
}

void XmlStreamReader::readValueElement(QTreeWidgetItem *parent)
{
  QString page = reader.readElementText();

  if ( reader.isEndElement() )
  {
    reader.readNext();
  }

  QString allPages = parent->text(1);
  if ( !allPages.isEmpty() )
  {
    allPages += ", ";
  }
  allPages += page;

  parent->setText(1, allPages);
}

void XmlStreamReader::skipUnknownElement()
{
  reader.readNext();
  while ( !reader.atEnd() )
  {
    if ( reader.isEndElement() )
    {
      reader.readNext();
      break;
    }

    if ( reader.isStartElement() )
    {
      skipUnknownElement();
    }
    else
    {
      reader.readNext();
    }
  }
 }


