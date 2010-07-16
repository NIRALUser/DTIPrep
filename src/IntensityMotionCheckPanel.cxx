#include <QtGui>

#include "IntensityMotionCheckPanel.h"
#include "IntensityMotionCheck.h"
#include "ThreadIntensityMotionCheck.h"

#include "itkMetaDataDictionary.h"
#include "itkNrrdImageIO.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

// #include "itkQtAdaptor.h"
// #include "itkQtAdaptor.h"
// #include "itkQtLightIndicator.h"
// #include "itkQtProgressBar.h"

#include "XmlStreamReader.h"
#include "XmlStreamWriter.h"

#include "IntraGradientRigidRegistration.h"
#include "itkExtractImageFilter.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

IntensityMotionCheckPanel::IntensityMotionCheckPanel(QMainWindow *parentNew) :
QDockWidget(parentNew)
{
  setupUi(this);
  verticalLayout->setContentsMargins(0, 0, 0, 0);
  progressBar->setValue(0);

  m_DwiOriginalImage = NULL;
  protocol.clear();
  bDwiLoaded = false;
  bProtocol = false;

  bProtocolTreeEditable = false;
  pushButton_DefaultProtocol->setEnabled( 0 );
  pushButton_Save->setEnabled( 0 );
  pushButton_SaveProtocolAs->setEnabled( 0 );

  bResultTreeEditable = false;
  //   pushButton_SaveDWI->setEnabled( 0 );
  //   pushButton_SaveQCReport->setEnabled( 0 );
  //   pushButton_SaveQCReportAs->setEnabled( 0 );
  pushButton_SaveDWIAs->setEnabled( 0 );
  // pushButton_CreateDefaultProtocol->setEnabled( 0 );

  pushButton_RunPipeline->setEnabled( 0 );

  pushButton_DefaultQCResult->setEnabled( 0 );
  //   pushButton_OpenQCReport->setEnabled( 0 );

  QStringList labels;
  labels << tr("Parameter") << tr("Value");
  // treeWidget->header()->setResizeMode(QHeaderView::Stretch);
  treeWidget->setHeaderLabels(labels);

  ThreadIntensityMotionCheck = new CThreadIntensityMotionCheck;

  bGetGradientDirections = false;

  GradientDirectionContainer = GradientDirectionContainerType::New();

  // ThreadIntensityMotionCheck->SetProtocol( &protocol);
  // ThreadIntensityMotionCheck->SetQCResult(&qcResult);

  // connect(ThreadIntensityMotionCheck, SIGNAL(QQQ(int )), this,
  // SLOT(UpdateProgressBar(const int )) );

  connect( ThreadIntensityMotionCheck, SIGNAL( kkk(int ) ), this,
    SLOT( UpdateProgressBar(const int ) ) );

  connect( this->ThreadIntensityMotionCheck,
    SIGNAL( ResultUpdate() ),
    this,
    SLOT( ResultUpdate() ) );

  // connect( this->ThreadIntensityMotionCheck->IntensityMotionCheck,
  //  SIGNAL(Progress(int )),
  //  this,
  //  SLOT(UpdateProgressBar(const int ))  );
}

void IntensityMotionCheckPanel::on_treeWidget_DiffusionInformation_itemClicked(
  QTreeWidgetItem *item,
  int /* column */)
{
  std::string str = item->text(0).toStdString();

  if ( str.find("gradient") != std::string::npos )
  {
    emit currentGradient( 0, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    emit currentGradient( 1, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    emit currentGradient( 2, atoi( str.substr(str.length() - 4, 4).c_str() ) );
  }
}

void IntensityMotionCheckPanel::
on_treeWidget_DiffusionInformation_currentItemChanged(
  QTreeWidgetItem *current,
  QTreeWidgetItem * /* previous */)
{
  std::string str = current->text(0).toStdString();

  if ( str.find("gradient") != std::string::npos )
  {
    emit currentGradient( 0, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    emit currentGradient( 1, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    emit currentGradient( 2, atoi( str.substr(str.length() - 4, 4).c_str() ) );
  }
}

void IntensityMotionCheckPanel::on_treeWidget_Results_itemClicked(
  QTreeWidgetItem *item,
  int /* column */)
{
  std::string str = item->text(0).toStdString();

  if ( str.find("gradient") != std::string::npos )
  {
    emit currentGradient( 0, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    emit currentGradient( 1, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    emit currentGradient( 2, atoi( str.substr(str.length() - 4, 4).c_str() ) );
  }
}

void IntensityMotionCheckPanel::on_treeWidget_Results_currentItemChanged(
  QTreeWidgetItem *current,
  QTreeWidgetItem *previous)
{
  treeWidget_Results->closePersistentEditor(previous, 2); // does nothing if
  // none open

  std::string str = current->text(0).toStdString();
  if ( str.find("gradient") != std::string::npos )
  {
    emit currentGradient( 0, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    emit currentGradient( 1, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    emit currentGradient( 2, atoi( str.substr(str.length() - 4, 4).c_str() ) );
  }
}

void IntensityMotionCheckPanel::on_treeWidget_itemDoubleClicked(
  QTreeWidgetItem *item,
  int col)
{
  if ( col == 1 && bProtocolTreeEditable )
  {
    treeWidget->openPersistentEditor(item, col);
  }
}

void IntensityMotionCheckPanel::on_treeWidget_Results_itemDoubleClicked(
  QTreeWidgetItem *item,
  int column)
{
  if ( bResultTreeEditable && column == 2 && item->text(0).left(8) ==
    tr("gradient") )
  {
    treeWidget_Results->openPersistentEditor(item, column);
  }
}

void IntensityMotionCheckPanel::on_treeWidget_Results_itemChanged(
  QTreeWidgetItem *item,
  int /* column */)
{
  if ( bResultTreeEditable )
  {
    if ( item->text(2).toLower() == tr("exclude") )
    {
      this->GetQCResult().GetIntensityMotionCheckResult()[item->text(0).right(4)
        .toUInt()].
        processing = QCResult::GRADIENT_EXCLUDE_MANUALLY;
      // std::cout << "gradient "<< item->text(0).right(4).toUInt() << ":
      // GRADIENT_EXCLUDE" <<std::endl;
    }
    else
    {
      this->GetQCResult().GetIntensityMotionCheckResult()[item->text(0).right(4)
        .toUInt()].
        processing = QCResult::GRADIENT_INCLUDE;
      // std::cout << "gradient "<< item->text(0).right(4).toUInt() << ":
      // GRADIENT_INCLUDE" <<std::endl;
    }

    emit UpdateOutputDWIDiffusionVectorActors();
  }
}

void IntensityMotionCheckPanel::on_treeWidget_currentItemChanged(
  QTreeWidgetItem * /* current */,
  QTreeWidgetItem *previous)
{
  treeWidget->closePersistentEditor(previous, 1); // does nothing if none open
}

IntensityMotionCheckPanel::~IntensityMotionCheckPanel(void)
{
  delete ThreadIntensityMotionCheck;
}

void IntensityMotionCheckPanel::on_pushButton_RunPipeline_clicked( )
{
  // CIntensityMotionCheck
  //
  // IntensityMotionCheck(lineEdit_DWIFileName->text().toStdString());
  // IntensityMotionCheck.SetProtocol(&protocol);
  // IntensityMotionCheck.SetQCResult(&qcResult);
  // IntensityMotionCheck.GetImagesInformation();
  // IntensityMotionCheck.CheckByProtocol();
  if ( m_DwiOriginalImage->GetVectorLength() != GradientDirectionContainer->size() )
  {
    std::cout
      << "Bad DWI: mismatch between gradient image # and gradient vector #"
      << std::endl;
    QMessageBox::critical( this, tr("BAD DWI !"),
      tr("Bad DWI: mismatch between gradient image # and gradient vector # !") );
    return;
  }

  if ( !bProtocol )
  {
    std::cout << "Protocol NOT set. Load prorocol file first!" << std::endl;
    return;
  }

  if ( DwiFileName.length() == 0 )
  {
    std::cout << "DWI file name not set!" << std::endl;
    QMessageBox::critical( this, tr("Warning"), tr("DWI file name not set!") );
    return;
  }

  qcResult.Clear();
  // std::cout <<
  // "ThreadIntensityMotionCheck->SetFileName(lineEdit_DWIFileName->text().toStdString());"<<std::endl;
  ThreadIntensityMotionCheck->SetDwiFileName(DwiFileName); 
  ThreadIntensityMotionCheck->SetXmlFileName(lineEdit_Protocol->text().toStdString());
  ThreadIntensityMotionCheck->SetProtocol( &protocol);
  ThreadIntensityMotionCheck->SetQCResult(&qcResult);
  ThreadIntensityMotionCheck->start();

  bResultTreeEditable = false;
  pushButton_SaveDWIAs->setEnabled( 0 );
}

void IntensityMotionCheckPanel::UpdateProgressBar(int posNew )
{
  progressBar->setValue(posNew);
}

void IntensityMotionCheckPanel::SetFileName(QString nrrd )
{
  // lineEdit_DWIFileName->setText(nrrd);
  DwiFileName = nrrd.toStdString();
}

void IntensityMotionCheckPanel::on_toolButton_ProtocolFileOpen_clicked( )
{
  OpenXML();
  bProtocolTreeEditable = true;
  emit ProtocolChanged();
}

void IntensityMotionCheckPanel::OpenXML( )
{
  QString xmlFile = QFileDialog::getOpenFileName ( this, tr(
    "Select Protocol"), lineEdit_Protocol->text(), tr("xml Files (*.xml)") );

  if ( xmlFile.length() > 0 )
  {
    lineEdit_Protocol->setText(xmlFile);
  }
  else
  {
    return;
  }

  treeWidget->clear();
  protocol.clear();

  XmlStreamReader XmlReader(treeWidget);
  XmlReader.setProtocol( &protocol);
  XmlReader.readFile(xmlFile, XmlStreamReader::TreeWise);
  XmlReader.readFile(xmlFile, XmlStreamReader::ProtocolWise);

  protocol.collectDiffusionStatistics();
  bProtocol = true;

  pushButton_Save->setEnabled( 1 );
  pushButton_SaveProtocolAs->setEnabled( 1 );
  pushButton_RunPipeline->setEnabled(1);

  // protocol.printProtocols();
}

bool IntensityMotionCheckPanel::LoadDwiImage()
{
  // use with windows
  // std::string str;
  // str=DwiFileName.substr(0,DwiFileName.find_last_of('\\')+1);
  // std::cout<< str<<std::endl;
  // ::SetCurrentDirectory(str.c_str());


  if ( DwiFileName.length() == 0 )
  {
    std::cout << "Dwi file name not set" << std::endl;
    bDwiLoaded = false;
    return false;
  }
  else
  {
    itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
    DwiReaderType::Pointer DwiReader;
    DwiReader = DwiReaderType::New();
    try
    {
      DwiReader->SetImageIO(myNrrdImageIO);
      DwiReader->SetFileName(DwiFileName);
      std::cout << "Loading in IntensityMotionCheckPanel:" << DwiFileName
        << " ... ";
      DwiReader->Update();
    }
    catch ( itk::ExceptionObject & e )
    {
      std::cout << e.GetDescription() << std::endl;
      bDwiLoaded = false;
      return false;
    }
    std::cout << "Done " << std::endl;

    m_DwiOriginalImage = DwiReader->GetOutput();
    bDwiLoaded = true;

    std::cout << "Image Dimension"
      << m_DwiOriginalImage->GetLargestPossibleRegion().GetSize().GetSizeDimension()
      << ": ";
    std::cout << m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[0] << " ";
    std::cout << m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[1] << " ";
    std::cout << m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[2] << std::endl;

    std::cout << "Pixel Vector Length: " << m_DwiOriginalImage->GetVectorLength()
      << std::endl;

  }
  return bDwiLoaded;
}

bool IntensityMotionCheckPanel::GetGradientDirections( bool bDisplay)
{
  if ( !bDwiLoaded )
  {
    LoadDwiImage();
  }
  if ( !bDwiLoaded )
  {
    std::cout << "DWI load error, no Gradient Direction Loaded" << std::endl;
    bGetGradientDirections = false;
    return false;
  }

  itk::MetaDataDictionary imgMetaDictionary
    = this->m_DwiOriginalImage->GetMetaDataDictionary();                                            //
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string                              metaString;

  // int numberOfImages=0;
  TensorReconstructionImageFilterType::GradientDirectionType vect3d;

  GradientDirectionContainer->clear();

  for (; itKey != imgMetaKeys.end(); itKey++ )
  {
    // double x,y,z;
    itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
    if ( itKey->find("DWMRI_gradient") != std::string::npos )
    {
      std::istringstream iss(metaString);
      iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
      // sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
      // vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
      GradientDirectionContainer->push_back(vect3d);
    }
    else if ( itKey->find("DWMRI_b-value") != std::string::npos )
    {
      readb0 = true;
      b0 = atof( metaString.c_str() );
      // std::cout<<"b Value: "<<b0<<std::endl;
    }
  }

  if ( !readb0 )
  {
    std::cout << "BValue not specified in header file" << std::endl;
    return false;
  }
  if ( GradientDirectionContainer->Size() <= 6 )
  {
    std::cout << "Gradient Images Less than 7" << std::endl;
    bGetGradientDirections = false;
    return false;
  }

  if ( bDisplay )
  {
    std::cout << "b Value: " << b0 << std::endl;
    std::cout << "DWI image gradient count: " << m_DwiOriginalImage->GetVectorLength()
      << std::endl;

    for ( unsigned int i = 0; i < m_DwiOriginalImage->GetVectorLength(); i++ ) //
      // GradientDirectionContainer->Size()
    {
      //      std::cout<<"Gradient Direction "<<i<<": \t[";
      //      std::cout<<GradientDirectionContainer->at(i)[0]<<",\t";
      //      std::cout<<GradientDirectionContainer->at(i)[1]<<",\t";
      std::cout << GradientDirectionContainer->at(i)[2] << " ]" << std::endl;
    }
  }

  bGetGradientDirections = true;
  return true;
}

void IntensityMotionCheckPanel::on_treeWidget_itemChanged(QTreeWidgetItem * /*
                                      item
                                      */,
                                      int /* column */)
{
  // pushButton_Save->setEnabled(pushButton_Editable->isCheckable());
}

void IntensityMotionCheckPanel::on_pushButton_SaveProtocolAs_clicked( )
{
  QString xmlFile = QFileDialog::getSaveFileName( this, tr(
    "Save Protocol As"), lineEdit_Protocol->text(),  tr("xml Files (*.xml)") );

  if ( xmlFile.length() > 0 )
  {
    lineEdit_Protocol->setText(xmlFile);
    XmlStreamWriter XmlWriter(treeWidget);
    XmlWriter.writeXml(xmlFile);

    // treeWidget->clear();
    protocol.clear();

    XmlStreamReader XmlReader(treeWidget);
    XmlReader.setProtocol( &protocol);
    // XmlReader.readFile(xmlFile, XmlStreamReader::TreeWise);
    XmlReader.readFile(xmlFile, XmlStreamReader::ProtocolWise);
  }

  bProtocol = true;
}

void IntensityMotionCheckPanel::on_pushButton_Save_clicked( )
{
  if ( lineEdit_Protocol->text().length() > 0 )
  {
    XmlStreamWriter XmlWriter(treeWidget);
    XmlWriter.writeXml( lineEdit_Protocol->text() );

    // treeWidget->clear();
    protocol.clear();

    XmlStreamReader XmlReader(treeWidget);
    XmlReader.setProtocol( &protocol);
    // XmlReader.readFile(lineEdit_Protocol->text(), XmlStreamReader::TreeWise);
    XmlReader.readFile(lineEdit_Protocol->text(), XmlStreamReader::ProtocolWise);
  }
  else
  {
    QString xmlFile = QFileDialog::getSaveFileName( this, tr(
      "Save Protocol As"), lineEdit_Protocol->text(),  tr("xml Files (*.xml)") );
    if ( xmlFile.length() > 0 )
    {
      lineEdit_Protocol->setText(xmlFile);
      XmlStreamWriter XmlWriter(treeWidget);
      XmlWriter.writeXml(xmlFile);

      // treeWidget->clear();
      protocol.clear();

      XmlStreamReader XmlReader(treeWidget);
      XmlReader.setProtocol( &protocol);
      // XmlReader.readFile(xmlFile, XmlStreamReader::TreeWise);
      XmlReader.readFile(xmlFile, XmlStreamReader::ProtocolWise);
    }
  }

  emit ProtocolChanged();
  bProtocol = true;
}

void IntensityMotionCheckPanel::UpdatePanelDWI()
{
  this->qcResult.Clear();
  treeWidget_Results->clear();
  pushButton_DefaultQCResult->setEnabled( 1 );
  //   pushButton_OpenQCReport->setEnabled( 1 );

  treeWidget_DiffusionInformation->clear();
  pushButton_DefaultProtocol->setEnabled( 1 );
  //   pushButton_SaveDWI->setEnabled( 0 );
  pushButton_SaveDWIAs->setEnabled( 0 );

  lineEdit_SizeX->setText( QString::number(this->m_DwiOriginalImage->
    GetLargestPossibleRegion().GetSize()[0]) );
  lineEdit_SizeY->setText( QString::number(this->m_DwiOriginalImage->
    GetLargestPossibleRegion().GetSize()[1]) );
  lineEdit_SizeZ->setText( QString::number(this->m_DwiOriginalImage->
    GetLargestPossibleRegion().GetSize()[2]) );

  lineEdit_OriginX->setText( QString::number(this->m_DwiOriginalImage->GetOrigin()[0],
    'f') );
  lineEdit_OriginY->setText( QString::number(this->m_DwiOriginalImage->GetOrigin()[1],
    'f') );
  lineEdit_OriginZ->setText( QString::number(this->m_DwiOriginalImage->GetOrigin()[2],
    'f') );
  lineEdit_SpacingX->setText( QString::number(this->m_DwiOriginalImage->GetSpacing()[0],
    'f') );
  lineEdit_SpacingY->setText( QString::number(this->m_DwiOriginalImage->GetSpacing()[1],
    'f') );
  lineEdit_SpacingZ->setText( QString::number(this->m_DwiOriginalImage->GetSpacing()[2],
    'f') );

  GetGradientDirections(0);

  QTreeWidgetItem *bValue = new QTreeWidgetItem(treeWidget_DiffusionInformation);
  bValue->setText( 0, tr("DWMRI_b-value") );
  bValue->setText( 1, QString::number(b0, 'f', 0) );

  for ( unsigned int i = 0; i < GradientDirectionContainer->size(); i++ )
  {
    QTreeWidgetItem *gradient = new QTreeWidgetItem(
      treeWidget_DiffusionInformation);
    gradient->setText( 0,
      QString("DWMRI_gradient_%1").arg( i, 4, 10, QLatin1Char( '0' ) ) );
    gradient->setText(1, QString("%1 %2 %3")
      .arg(GradientDirectionContainer->ElementAt(i)[0], 10, 'f', 6)
      .arg(GradientDirectionContainer->ElementAt(i)[1], 10, 'f', 6)
      .arg(GradientDirectionContainer->ElementAt(i)[2], 10, 'f', 6)
      );
  }

  if ( m_DwiOriginalImage->GetVectorLength() != GradientDirectionContainer->size() )
  {
    std::cout
      << "Bad DWI: mismatch between gradient image # and gradient vector #"
      << std::endl;
  }
  itk::MetaDataDictionary imgMetaDictionary
    = m_DwiOriginalImage->GetMetaDataDictionary();
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string                              metaString;

  //  measurement frame
  if ( imgMetaDictionary.HasKey("NRRD_measurement frame") )
  {
    {
    // imaging frame
    const vnl_matrix_fixed<double, 3, 3> &imgf= m_DwiOriginalImage->GetDirection().GetVnlMatrix();

    // Image frame
    // std::cout << "Image frame: " << std::endl;
    // std::cout << imgf << std::endl;

    lineEdit_SpaceDir11->setText( QString::number(imgf(0, 0), 'f') );
    lineEdit_SpaceDir12->setText( QString::number(imgf(0, 1), 'f') );
    lineEdit_SpaceDir13->setText( QString::number(imgf(0, 2), 'f') );
    lineEdit_SpaceDir21->setText( QString::number(imgf(1, 0), 'f') );
    lineEdit_SpaceDir22->setText( QString::number(imgf(1, 1), 'f') );
    lineEdit_SpaceDir23->setText( QString::number(imgf(1, 2), 'f') );
    lineEdit_SpaceDir31->setText( QString::number(imgf(2, 0), 'f') );
    lineEdit_SpaceDir32->setText( QString::number(imgf(2, 1), 'f') );
    lineEdit_SpaceDir33->setText( QString::number(imgf(2, 2), 'f') );
    }
    {
    std::vector<std::vector<double> > nrrdmf;
    itk::ExposeMetaData<std::vector<std::vector<double> > >(
      imgMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);

    lineEdit_MeasurementFrame11->setText( QString::number(nrrdmf[0][0], 'f') );
    lineEdit_MeasurementFrame12->setText( QString::number(nrrdmf[0][1], 'f') );
    lineEdit_MeasurementFrame13->setText( QString::number(nrrdmf[0][2], 'f') );
    lineEdit_MeasurementFrame21->setText( QString::number(nrrdmf[1][0], 'f') );
    lineEdit_MeasurementFrame22->setText( QString::number(nrrdmf[1][1], 'f') );
    lineEdit_MeasurementFrame23->setText( QString::number(nrrdmf[1][2], 'f') );
    lineEdit_MeasurementFrame31->setText( QString::number(nrrdmf[2][0], 'f') );
    lineEdit_MeasurementFrame32->setText( QString::number(nrrdmf[2][1], 'f') );
    lineEdit_MeasurementFrame33->setText( QString::number(nrrdmf[2][2], 'f') );
    }
  }

  // space
  itk::ExposeMetaData<std::string>(imgMetaDictionary, "NRRD_space", metaString);
  //   std::cout<<"space: "<<metaString.c_str()<<std::endl;
  comboBox_Space->setCurrentIndex( comboBox_Space->findText ( QString::
    fromStdString( metaString), Qt::MatchExactly) );
}

void IntensityMotionCheckPanel::on_pushButton_DefaultProtocol_clicked( )
{
  //  CreateDefaultProtocol(); //old
  //  UpdateProtocolTree(); //old

  if ( m_DwiOriginalImage->GetVectorLength() != GradientDirectionContainer->size() )
  {
    std::cout
      << "Bad DWI: mismatch between gradient image # and gradient vector #"
      << std::endl;
    QMessageBox::critical( this, tr("BAD DWI !"),
      tr("Bad DWI: mismatch between gradient image # and gradient vector # !") );
    return;
  }

  DefaultProtocol();
  UpdateProtocolToTreeWidget();

  pushButton_RunPipeline->setEnabled(1);
  pushButton_Save->setEnabled( 1 );
  pushButton_SaveProtocolAs->setEnabled( 1 );

  emit ProtocolChanged();
  bProtocolTreeEditable = true;
}

// old
void IntensityMotionCheckPanel::DefaultProtocol()
{
  this->GetProtocol().clear();
  this->GetProtocol().initDTIProtocol();

  this->GetProtocol().GetQCOutputDirectory() = "";
  this->GetProtocol().GetQCedDWIFileNameSuffix() = "_QCed.nhdr";
  this->GetProtocol().GetReportFileNameSuffix() = "_QCReport.txt";
  this->GetProtocol().SetBadGradientPercentageTolerance(0.2);
  this->GetProtocol().SetReportType(0);

  // ***** image
  this->GetProtocol().GetImageProtocol().bCheck = true;

  // size
  this->GetProtocol().GetImageProtocol().size[0]
  = m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[0];
  this->GetProtocol().GetImageProtocol().size[1]
  = m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[1];
  this->GetProtocol().GetImageProtocol().size[2]
  = m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[2];

  // origin
  this->GetProtocol().GetImageProtocol().origin[0] = m_DwiOriginalImage->GetOrigin()[0];
  this->GetProtocol().GetImageProtocol().origin[1] = m_DwiOriginalImage->GetOrigin()[1];
  this->GetProtocol().GetImageProtocol().origin[2] = m_DwiOriginalImage->GetOrigin()[2];

  // spacing
  this->GetProtocol().GetImageProtocol().spacing[0] = m_DwiOriginalImage->GetSpacing()[0];
  this->GetProtocol().GetImageProtocol().spacing[1] = m_DwiOriginalImage->GetSpacing()[1];
  this->GetProtocol().GetImageProtocol().spacing[2] = m_DwiOriginalImage->GetSpacing()[2];

  // space
  itk::MetaDataDictionary imgMetaDictionary
    = m_DwiOriginalImage->GetMetaDataDictionary();
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string                              metaString;

  itk::ExposeMetaData<std::string>(imgMetaDictionary, "NRRD_space", metaString);
  if ( metaString == "left-anterior-inferior" )
  {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_LAI;
  }
  else if ( metaString == "left-anterior-superior" )
  {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_LAS;
  }
  else if ( metaString == "left-posterior-inferior" )
  {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_LPI;
  }
  else if ( metaString == "left-posterior-superior" )
  {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_LPS;
  }
  else if ( metaString == "right-anterior-inferior" )
  {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_RAI;
  }
  else if ( metaString == "right-anterior-superior" )
  {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_RAS;
  }
  else if ( metaString == "right-posterior-inferior" )
  {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_RPI;
  }
  else if ( metaString == "right-posterior-superior" )
  {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_RPS;
  }
  else
  {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_UNKNOWN;
  }

  this->GetProtocol().GetImageProtocol().bCrop = true;
  this->GetProtocol().GetImageProtocol().croppedDWIFileNameSuffix
    = "_CroppedDWI.nhdr";

  this->GetProtocol().GetImageProtocol().reportFileNameSuffix = "_QCReport.txt";
  this->GetProtocol().GetImageProtocol().reportFileMode = 1;

  this->GetProtocol().GetImageProtocol().bQuitOnCheckSpacingFailure = true;
  this->GetProtocol().GetImageProtocol().bQuitOnCheckSizeFailure = false;

  // ***** diffusion
  GetGradientDirections( 0 );

  this->GetProtocol().GetDiffusionProtocol().bCheck = true;
  this->GetProtocol().GetDiffusionProtocol().bValue = this->b0;

  for ( unsigned int i = 0; i < GradientDirectionContainer->size(); i++ )
  {
    vnl_vector_fixed<double, 3> vect;
    vect[0] = ( GradientDirectionContainer->ElementAt(i)[0] );
    vect[1] = ( GradientDirectionContainer->ElementAt(i)[1] );
    vect[2] = ( GradientDirectionContainer->ElementAt(i)[2] );

    this->GetProtocol().GetDiffusionProtocol().gradients.push_back(vect);
  }

  //  
  if ( imgMetaDictionary.HasKey("NRRD_measurement frame") )
  {
    {
    // imaging frame
    const vnl_matrix_fixed<double, 3, 3> &imgf= m_DwiOriginalImage->GetDirection().GetVnlMatrix();

    // Image frame
    this->GetProtocol().GetImageProtocol().spacedirection=m_DwiOriginalImage->GetDirection().GetVnlMatrix();
    this->GetProtocol().GetImageProtocol().spacedirection[0][0] = imgf(0, 0);
    this->GetProtocol().GetImageProtocol().spacedirection[0][1] = imgf(0, 1);
    this->GetProtocol().GetImageProtocol().spacedirection[0][2] = imgf(0, 2);
    this->GetProtocol().GetImageProtocol().spacedirection[1][0] = imgf(1, 0);
    this->GetProtocol().GetImageProtocol().spacedirection[1][1] = imgf(1, 1);
    this->GetProtocol().GetImageProtocol().spacedirection[1][2] = imgf(1, 2);
    this->GetProtocol().GetImageProtocol().spacedirection[2][0] = imgf(2, 0);
    this->GetProtocol().GetImageProtocol().spacedirection[2][1] = imgf(2, 1);
    this->GetProtocol().GetImageProtocol().spacedirection[2][2] = imgf(2, 2);
    }
    {
    std::vector<std::vector<double> > nrrdmf;
    itk::ExposeMetaData<std::vector<std::vector<double> > >(
      imgMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);

    // measurement frame

    for ( unsigned int i = 0; i < 3; ++i )
    {
      for ( unsigned int j = 0; j < 3; ++j )
      {
        // Meausurement frame
        this->GetProtocol().GetDiffusionProtocol().measurementFrame[i][j] = nrrdmf[i][j];
      }
    }
    std::cout << "DEBUG: QQQQQQQQQQQQ\n" <<  this->GetProtocol().GetDiffusionProtocol().measurementFrame << std::flush << std::endl;
    }
  }

  //HACK:  This breaks encapsulation of the function.  SetFunctions should be used!
  this->GetProtocol().GetDiffusionProtocol().bUseDiffusionProtocol = true;
  this->GetProtocol().GetDiffusionProtocol().diffusionReplacedDWIFileNameSuffix
    = "_DiffusionReplaced.nhdr";

  this->GetProtocol().GetDiffusionProtocol().reportFileNameSuffix
    = "_QCReport.txt";
  this->GetProtocol().GetDiffusionProtocol().reportFileMode = 1;
  this->GetProtocol().GetDiffusionProtocol().bQuitOnCheckFailure = true;

  // ***** slice check
  emit status("Estimating protocol parameter  ...");
  std::cout << "Estimating protocol parameter  ..." << std::endl;

  //  double sliceBaselineThreshold, sliceGradientThreshold,
  // devBaselineThreshold, devGradientThreshold;
  //  GetSliceProtocolParameters(
  //    0.10,
  //    0.10,
  //    sliceBaselineThreshold ,
  //    sliceGradientThreshold ,
  //    devBaselineThreshold,
  //    devGradientThreshold
  //    );
  //  std::cout << "sliceBaselineThreshold:
  // "<<sliceBaselineThreshold<<std::endl;
  //  std::cout << "sliceGradientThreshold:
  // "<<sliceGradientThreshold<<std::endl;
  //  std::cout << "devBaselineThreshold: "<<devBaselineThreshold<<std::endl;
  //  std::cout << "devGradientThreshold: "<<devGradientThreshold<<std::endl;
  this->GetProtocol().GetSliceCheckProtocol().bCheck = true;
  //  this->GetProtocol().GetSliceCheckProtocol().badGradientPercentageTolerance
  // = 0.2;
  this->GetProtocol().GetSliceCheckProtocol().bSubregionalCheck = false;
  this->GetProtocol().GetSliceCheckProtocol().subregionalCheckRelaxationFactor = 1.1;
  this->GetProtocol().GetSliceCheckProtocol().checkTimes = 0;
  this->GetProtocol().GetSliceCheckProtocol().headSkipSlicePercentage = 0.1;
  this->GetProtocol().GetSliceCheckProtocol().tailSkipSlicePercentage = 0.1;
  this->GetProtocol().GetSliceCheckProtocol().
    correlationDeviationThresholdbaseline = 3.00;
  this->GetProtocol().GetSliceCheckProtocol().
    correlationDeviationThresholdgradient = 3.50;                                           //
  //
  // 4.5
  //
  // for
  //
  // LeaveOneOutCheck?

  this->GetProtocol().GetSliceCheckProtocol().outputDWIFileNameSuffix = "";
  this->GetProtocol().GetSliceCheckProtocol().reportFileNameSuffix
    = "_QCReport.txt";
  this->GetProtocol().GetSliceCheckProtocol().reportFileMode = 1;
  this->GetProtocol().GetSliceCheckProtocol().excludedDWINrrdFileNameSuffix = "";
  this->GetProtocol().GetSliceCheckProtocol().bQuitOnCheckFailure = true;
  // ***** interlace check
  double interlaceBaselineThreshold, interlaceGradientThreshold,
    interlaceBaselineDev, interlaceGradientDev;
  GetInterlaceProtocolParameters(
    interlaceBaselineThreshold,
    interlaceGradientThreshold,
    interlaceBaselineDev,
    interlaceGradientDev
    );
  //  std::cout << "interlaceBaselineThreshold:
  // "<<interlaceBaselineThreshold<<std::endl;
  //  std::cout << "interlaceGradientThreshold:
  // "<<interlaceGradientThreshold<<std::endl;
  //  std::cout << "interlaceBaselineDev: "<<interlaceBaselineDev<<std::endl;
  //  std::cout << "interlaceGradientDev: "<<interlaceGradientDev<<std::endl;

  this->GetProtocol().GetInterlaceCheckProtocol().bCheck = true;
  //
  //
  //
  //
  // this->GetProtocol().GetInterlaceCheckProtocol().badGradientPercentageTolerance
  // = 0.2;
  this->GetProtocol().GetInterlaceCheckProtocol().correlationThresholdBaseline
    = interlaceBaselineThreshold * 0.95;
  this->GetProtocol().GetInterlaceCheckProtocol().correlationDeviationBaseline
    = 2.50;
  this->GetProtocol().GetInterlaceCheckProtocol().correlationThresholdGradient
    = interlaceGradientThreshold * 0.95;
  this->GetProtocol().GetInterlaceCheckProtocol().correlationDeviationGradient
    = 3.00;
  this->GetProtocol().GetInterlaceCheckProtocol().rotationThreshold = 0.5; //
  //
  // degree
  this->GetProtocol().GetInterlaceCheckProtocol().translationThreshold
    = ( this->GetProtocol().GetImageProtocol().spacing[0]
  +
    this->GetProtocol().GetImageProtocol().spacing[1]
  +
    this->GetProtocol().GetImageProtocol().spacing[2]   ) * 0.3333333333333;
  this->GetProtocol().GetInterlaceCheckProtocol().outputDWIFileNameSuffix = "";
  this->GetProtocol().GetInterlaceCheckProtocol().reportFileNameSuffix
    = "_QCReport.txt";
  this->GetProtocol().GetInterlaceCheckProtocol().reportFileMode = 1;
  this->GetProtocol().GetInterlaceCheckProtocol().excludedDWINrrdFileNameSuffix = "";
  this->GetProtocol().GetInterlaceCheckProtocol().bQuitOnCheckFailure = true;

  // ***** gradient check
  this->GetProtocol().GetGradientCheckProtocol().bCheck = true;
  //
  //
  //
  //
  // this->GetProtocol().GetGradientCheckProtocol().badGradientPercentageTolerance
  // = 0.2;
  this->GetProtocol().GetGradientCheckProtocol().rotationThreshold = 0.5; //
  // degree
  this->GetProtocol().GetGradientCheckProtocol().translationThreshold
    = (  this->GetProtocol().GetImageProtocol().spacing[0]
  +
    this->GetProtocol().GetImageProtocol().spacing[1]
  +
    this->GetProtocol().GetImageProtocol().spacing[2]   )
    * 0.3333333333333;
  this->GetProtocol().GetGradientCheckProtocol().outputDWIFileNameSuffix = "";
  this->GetProtocol().GetGradientCheckProtocol().reportFileNameSuffix
    = "_QCReport.txt";
  this->GetProtocol().GetGradientCheckProtocol().reportFileMode = 1;
  this->GetProtocol().GetGradientCheckProtocol().excludedDWINrrdFileNameSuffix = "";
  this->GetProtocol().GetGradientCheckProtocol().bQuitOnCheckFailure = true;

  // ***** baseline average
  this->GetProtocol().GetBaselineAverageProtocol().bAverage = true;

  this->GetProtocol().GetBaselineAverageProtocol().averageMethod = 1;
  this->GetProtocol().GetBaselineAverageProtocol().stopThreshold = 0.02;

  this->GetProtocol().GetBaselineAverageProtocol().outputDWIFileNameSuffix
    = "";
  this->GetProtocol().GetBaselineAverageProtocol().reportFileNameSuffix
    = "_QCReport.txt";

  // ***** Eddy motion correction
  this->GetProtocol().GetEddyMotionCorrectionProtocol().bCorrect = true;
  //   this->GetProtocol().GetEddyMotionCorrectionProtocol().EddyMotionCommand =
  // "/tools/bin_linux64/EddyMotionCorrector";
  //  this->GetProtocol().GetEddyMotionCorrectionProtocol().InputFileName =
  // "_QCOutput.nhdr";
  //  this->GetProtocol().GetEddyMotionCorrectionProtocol().OutputFileName =
  // "_EddyMotion_Output.nhdr";

  this->GetProtocol().GetEddyMotionCorrectionProtocol().numberOfBins    =  24;
  this->GetProtocol().GetEddyMotionCorrectionProtocol().numberOfSamples
    =  100000;
  this->GetProtocol().GetEddyMotionCorrectionProtocol().translationScale
    =  0.001;
  this->GetProtocol().GetEddyMotionCorrectionProtocol().stepLength    =  0.1;
  this->GetProtocol().GetEddyMotionCorrectionProtocol().relaxFactor    =  0.5;
  this->GetProtocol().GetEddyMotionCorrectionProtocol().maxNumberOfIterations
    =  500;

  this->GetProtocol().GetEddyMotionCorrectionProtocol().outputDWIFileNameSuffix
    = "";
  this->GetProtocol().GetEddyMotionCorrectionProtocol().reportFileNameSuffix
    = "_QCReport.txt";
  this->GetProtocol().GetEddyMotionCorrectionProtocol().reportFileMode = 1;

  // ***** DTI
  this->GetProtocol().GetDTIProtocol().bCompute = true;
  this->GetProtocol().GetDTIProtocol().dtiestimCommand
    = "/tools/bin_linux64/dtiestim";
  this->GetProtocol().GetDTIProtocol().dtiprocessCommand
    = "/tools/bin_linux64/dtiprocess";
  this->GetProtocol().GetDTIProtocol().method = Protocol::METHOD_WLS;
  this->GetProtocol().GetDTIProtocol().baselineThreshold = 50; //
  this->GetProtocol().GetDTIProtocol().mask = "";
  this->GetProtocol().GetDTIProtocol().tensorSuffix = "_DTI.nhdr";
  this->GetProtocol().GetDTIProtocol().bbaseline = true;
  this->GetProtocol().GetDTIProtocol().baselineSuffix = "_Baseline.nhdr";
  this->GetProtocol().GetDTIProtocol().bidwi = true;
  this->GetProtocol().GetDTIProtocol().idwiSuffix = "_IDWI.nhdr";
  this->GetProtocol().GetDTIProtocol().bfa = true;
  this->GetProtocol().GetDTIProtocol().faSuffix = "_FA.nhdr";
  this->GetProtocol().GetDTIProtocol().bmd = true;
  this->GetProtocol().GetDTIProtocol().mdSuffix = "_MD.nhdr";
  this->GetProtocol().GetDTIProtocol().bcoloredfa = true;
  this->GetProtocol().GetDTIProtocol().coloredfaSuffix = "_colorFA.nhdr";
  this->GetProtocol().GetDTIProtocol().bfrobeniusnorm = true;
  this->GetProtocol().GetDTIProtocol().frobeniusnormSuffix
    = "_frobeniusnorm.nhdr";

  this->GetProtocol().GetDTIProtocol().reportFileNameSuffix = "_QCReport.txt";
  this->GetProtocol().GetDTIProtocol().reportFileMode = 1;

  bProtocol = true;

  emit status("done");
  std::cout << "Estimating protocol parameter  ... done" << std::endl;
}

bool IntensityMotionCheckPanel::GetSliceProtocolParameters(
  double beginSkip,
  double endSkip,
  double & baselineCorrelationThreshold,
  double & gradientCorrelationThreshold,
  double & baselineCorrelationDeviationThreshold,
  double & gradientCorrelationDeviationThreshold
  )
{
  // emit Progress(j+1/m_DwiOriginalImage->GetVectorLength());//emit QQQ(10);

  if ( !bDwiLoaded  )
  {
    LoadDwiImage();
  }
  if ( !bDwiLoaded  )
  {
    std::cout << "DWI load error, no Gradient Images got" << std::endl;
    return false;
  }

  std::vector<std::vector<struIntra2DResults> > ResultsContainer;

  typedef itk::VectorIndexSelectionCastImageFilter<DwiImageType,
    GradientImageType> FilterType;
  FilterType::Pointer componentExtractor = FilterType::New();
  componentExtractor->SetInput(m_DwiOriginalImage);

  typedef itk::ExtractImageFilter<GradientImageType,
    SliceImageType> ExtractFilterType;
  ExtractFilterType::Pointer filter1 = ExtractFilterType::New();
  ExtractFilterType::Pointer filter2 = ExtractFilterType::New();

  for ( unsigned int j = 0; j < m_DwiOriginalImage->GetVectorLength(); j++ )
  {
    componentExtractor->SetIndex( j );
    componentExtractor->Update();

    GradientImageType::RegionType inputRegion
      = componentExtractor->GetOutput()->GetLargestPossibleRegion();
    GradientImageType::SizeType localSize = inputRegion.GetSize();
    localSize[2] = 0;

    GradientImageType::IndexType start1 = inputRegion.GetIndex();
    GradientImageType::IndexType start2 = inputRegion.GetIndex();

    std::vector<struIntra2DResults> Results;

    filter1->SetInput( componentExtractor->GetOutput() );
    filter2->SetInput( componentExtractor->GetOutput() );

    for ( unsigned int i = 1;
      i <
      componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()
      [2];
    i++ )
    {
      start1[2] = i - 1;
      start2[2] = i;

      GradientImageType::RegionType desiredRegion1;
      desiredRegion1.SetSize( localSize );
      desiredRegion1.SetIndex( start1 );
      filter1->SetExtractionRegion( desiredRegion1 );

      GradientImageType::RegionType desiredRegion2;
      desiredRegion2.SetSize( localSize );
      desiredRegion2.SetIndex( start2 );
      filter2->SetExtractionRegion( desiredRegion2 );

      filter1->Update();
      filter2->Update();

      CIntraGradientRigidRegistration IntraGradientRigidReg( filter1->GetOutput(
        ),
        filter2->GetOutput() );
      struIntra2DResults s2DResults = IntraGradientRigidReg.Run( 0 /* No
                                     registration
                                     */);
      Results.push_back(s2DResults);
    }
    ResultsContainer.push_back(Results);
  }

  GetGradientDirections(0);
  int DWICount, BaselineCount;

  DWICount = 0;
  BaselineCount = 0;
  for ( unsigned int i = 0; i < GradientDirectionContainer->size(); i++ )
  {
    if ( GradientDirectionContainer->ElementAt(i)[0] == 0.0
      && GradientDirectionContainer->ElementAt(i)[1] == 0.0
      && GradientDirectionContainer->ElementAt(i)[2] == 0.0 )
    {
      BaselineCount++;
    }
    else
    {
      DWICount++;
    }
  }

  std::cout << "BaselineCount: " << BaselineCount << std::endl;
  std::cout << "DWICount: " << DWICount << std::endl;

  std::vector<double> baselineCorrelationMean;
  std::vector<double> gradientCorrelationMean;

  std::vector<double> baselineCorrelationMin;
  std::vector<double> gradientCorrelationMin;

  std::vector<double> baselineCorrelationDev;
  std::vector<double> gradientCorrelationDev;

  for ( unsigned int j = 0; j < ResultsContainer[0].size(); j++ )
  {
    double baselinemean = 0.0, DWImean = 0.0, baselinedeviation = 0.0,
      DWIdeviation = 0.0, baselineMin = 1.0, gradientMin = 1.0;
    for ( unsigned int i = 0; i < ResultsContainer.size(); i++ )
    {
      if ( GradientDirectionContainer->at(i)[0] == 0.0
        && GradientDirectionContainer->at(i)[1] == 0.0
        && GradientDirectionContainer->at(i)[2] == 0.0 )
      {
        baselinemean += ResultsContainer[i][j].Correlation
          / (double)(BaselineCount);
        if ( ResultsContainer[i][j].Correlation < baselineMin )
        {
          baselineMin = ResultsContainer[i][j].Correlation;
        }
      }
      else
      {
        DWImean += ResultsContainer[i][j].Correlation / (double)(DWICount);
        if ( ResultsContainer[i][j].Correlation < gradientMin )
        {
          gradientMin = ResultsContainer[i][j].Correlation;
        }
      }
    }

    baselineCorrelationMean.push_back(baselinemean);
    gradientCorrelationMean.push_back(DWImean);

    baselineCorrelationMin.push_back(baselineMin);
    gradientCorrelationMin.push_back(gradientMin);

    for ( unsigned int i = 0; i < ResultsContainer.size(); i++ )
    {
      if ( GradientDirectionContainer->at(i)[0] == 0.0
        && GradientDirectionContainer->at(i)[1] == 0.0
        && GradientDirectionContainer->at(i)[2] == 0.0 )
      {
        if ( BaselineCount >= 1 )
        {
          baselinedeviation
            += ( ResultsContainer[i][j].Correlation
            - baselinemean )
            * ( ResultsContainer[i][j].Correlation
            - baselinemean ) / (double)(BaselineCount);
        }
        else
        {
          baselinedeviation = 0.0;
        }
      }
      else
      {
        DWIdeviation
          += ( ResultsContainer[i][j].Correlation
          - DWImean )
          * ( ResultsContainer[i][j].Correlation
          - DWImean ) / (double)(DWICount);
      }
    }

    baselineCorrelationDev.push_back( sqrt(baselinedeviation) );
    gradientCorrelationDev.push_back( sqrt(DWIdeviation) );
  }

  double minBaselineCorrelation = 1.0;
  double minGradientCorrelation = 1.0;

  double maxBaselineCorrelationDev = 0.0;
  double maxGradientCorrelationDev = 0.0;

  double maxBaselineCorrelationDevTime = 0.0;
  double maxGradientCorrelationDevTime = 0.0;

  int sliceNum = m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[2];
  for ( int i = 0 + (int)( sliceNum * beginSkip);
    i < sliceNum - (int)( sliceNum * endSkip);
    i++ )                                                                               //
    //
    // for(int
    //
    // j=0;j<ResultsContainer[0].size()-1;
    //
    // j++)
  {
    if ( baselineCorrelationMin[i] < minBaselineCorrelation )
    {
      minBaselineCorrelation = baselineCorrelationMin[i];
    }

    if ( gradientCorrelationMin[i] < minGradientCorrelation )
    {
      minGradientCorrelation = gradientCorrelationMin[i];
    }

    if ( baselineCorrelationDev[i] > maxBaselineCorrelationDev )
    {
      maxBaselineCorrelationDev = baselineCorrelationDev[i];
    }

    if ( gradientCorrelationDev[i] > maxGradientCorrelationDev )
    {
      maxGradientCorrelationDev = gradientCorrelationDev[i];
    }

    if ( ( baselineCorrelationMean[i]
    - baselineCorrelationMin[i] ) / baselineCorrelationDev[i] >
      maxBaselineCorrelationDevTime )
    {
      maxBaselineCorrelationDevTime
        = ( baselineCorrelationMean[i]
      - baselineCorrelationMin[i] ) / baselineCorrelationDev[i];
    }

    if ( ( gradientCorrelationMean[i]
    - gradientCorrelationMin[i] ) / gradientCorrelationDev[i] >
      maxGradientCorrelationDevTime )
    {
      maxGradientCorrelationDevTime
        = ( gradientCorrelationMean[i]
      - gradientCorrelationMin[i] ) / gradientCorrelationDev[i];
    }
  }

  baselineCorrelationThreshold = minBaselineCorrelation; //
  // minBaselineCorrelation
  // -
  //
  // maxBaselineCorrelationDev*2.0;
  gradientCorrelationThreshold = minGradientCorrelation; // -
  //
  // maxGradientCorrelationDev*2.0;

  baselineCorrelationDeviationThreshold =  maxBaselineCorrelationDevTime;
  gradientCorrelationDeviationThreshold =  maxGradientCorrelationDevTime;

  std::cout << "minBaselineCorrelation: " << minBaselineCorrelation
    << std::endl;
  std::cout << "minGradientCorrelation: " << minGradientCorrelation
    << std::endl;
  std::cout << "maxBaselineCorrelationDev: " << maxBaselineCorrelationDev
    << std::endl;
  std::cout << "maxGradientCorrelationDev: " << maxGradientCorrelationDev
    << std::endl;
  std::cout << "maxBaselineCorrelationDevTime: "
    << maxBaselineCorrelationDevTime << std::endl;
  std::cout << "maxGradientCorrelationDevTime: "
    << maxGradientCorrelationDevTime << std::endl;

  return true;
}

bool IntensityMotionCheckPanel::GetInterlaceProtocolParameters(
  double & correlationThresholdBaseline,
  double & correlationThresholdGradient,
  double & correlationBaselineDevTimes,
  double & correlationGradientDevTimes
  )
{
  if ( !bDwiLoaded )
  {
    LoadDwiImage();
  }
  if ( !bDwiLoaded )
  {
    std::cout << "DWI load error, no Gradient Images got" << std::endl;
    return false;
  }

  GetGradientDirections(0);

  std::vector<double> baselineCorrelation;
  std::vector<double> gradientCorrelation;

  typedef itk::VectorIndexSelectionCastImageFilter<DwiImageType,
    GradientImageType> FilterType;
  FilterType::Pointer componentExtractor = FilterType::New();

  componentExtractor->SetInput(m_DwiOriginalImage);

  GradientImageType::Pointer InterlaceOdd  = GradientImageType::New();
  GradientImageType::Pointer InterlaceEven = GradientImageType::New();

  componentExtractor->SetIndex( 0 );
  componentExtractor->Update();

  GradientImageType::RegionType region;
  GradientImageType::SizeType   sizeLocal;
  sizeLocal[0]
  = componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[0];
  sizeLocal[1]
  = componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[1];
  sizeLocal[2]
  = componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[2]
  / 2;
  region.SetSize( sizeLocal );

  const GradientImageType::SpacingType spacing = componentExtractor->GetOutput()->GetSpacing();

  InterlaceOdd->CopyInformation( componentExtractor->GetOutput() );
  InterlaceOdd->SetRegions( region );
  InterlaceOdd->Allocate();

  InterlaceEven->CopyInformation( componentExtractor->GetOutput() );
  InterlaceEven->SetRegions( region );
  InterlaceEven->Allocate();

  typedef itk::ImageRegionIteratorWithIndex<GradientImageType> IteratorType;
  IteratorType iterateOdd( InterlaceOdd, InterlaceOdd->GetLargestPossibleRegion() );
  IteratorType iterateEven( InterlaceEven,
    InterlaceEven->GetLargestPossibleRegion() );

  for ( unsigned int j = 0; j < m_DwiOriginalImage->GetVectorLength(); j++ )
  {
    componentExtractor->SetIndex( j );
    componentExtractor->Update();

    typedef itk::ImageRegionIteratorWithIndex<GradientImageType> IteratorType;
    IteratorType iterateGradient(
      componentExtractor->GetOutput(),
      componentExtractor->GetOutput()->GetLargestPossibleRegion() );

    iterateGradient.GoToBegin();
    iterateOdd.GoToBegin();
    iterateEven.GoToBegin();

    unsigned long count = 0;
    while ( !iterateGradient.IsAtEnd() )
    {
      if ( count < sizeLocal[0] * sizeLocal[1] * sizeLocal[2] * 2 )
      {
        if ( ( count / ( sizeLocal[0] * sizeLocal[1] ) ) % 2 == 0 )
        {
          iterateEven.Set( iterateGradient.Get() );
          ++iterateEven;
        }
        if ( ( count / ( sizeLocal[0] * sizeLocal[1] ) ) % 2 == 1 )
        {
          iterateOdd.Set( iterateGradient.Get() );
          ++iterateOdd;
        }
      }
      ++iterateGradient;
      ++count;
    }

    typedef itk::ImageRegionConstIterator<GradientImageType> citType;
    citType cit1( InterlaceOdd, InterlaceOdd->GetBufferedRegion() );
    citType cit2( InterlaceEven, InterlaceEven->GetBufferedRegion() );

    cit1.GoToBegin();
    cit2.GoToBegin();

    double Correlation;
    double sAB = 0.0, sA2 = 0.0, sB2 = 0.0;
    while ( !cit1.IsAtEnd() )
    {
      sAB += cit1.Get() * cit2.Get();
      sA2 += cit1.Get() * cit1.Get();
      sB2 += cit2.Get() * cit2.Get();
      ++cit1;
      ++cit2;
    }
    Correlation = sAB / sqrt(sA2 * sB2);

    if ( GradientDirectionContainer->at(j)[0] == 0.0
      && GradientDirectionContainer->at(j)[1] == 0.0
      && GradientDirectionContainer->at(j)[2] == 0.0 )
    {
      baselineCorrelation.push_back(Correlation);
    }
    else
    {
      gradientCorrelation.push_back(Correlation);
    }

    //     std::cout<<"Correlation: " << Correlation<< std::endl;:837: error: expected `;' before ¡®this¡¯

  }

  int DWICount, BaselineCount;

  DWICount = 0;
  BaselineCount = 0;
  for ( unsigned int i = 0; i < GradientDirectionContainer->size(); i++ )
  {
    if ( GradientDirectionContainer->ElementAt(i)[0] == 0.0
      && GradientDirectionContainer->ElementAt(i)[1] == 0.0
      && GradientDirectionContainer->ElementAt(i)[2] == 0.0 )
    {
      BaselineCount++;
    }
    else
    {
      DWICount++;
    }
  }

  std::cout << "BaselineCount: " << BaselineCount << std::endl;
  std::cout << "DWICount: " << DWICount << std::endl;

  double minBaselineCorrelation = 1.0;
  double minGradientCorrelation = 1.0;

  double meanBaselineCorrelation = 0.0;
  double meanGradientCorrelation = 0.0;

  double baselineCorrelationDev = 0.0;
  double gradientCorrelationDev = 0.0;

  for ( unsigned int i = 0; i < baselineCorrelation.size(); i++ )
  {
    if ( baselineCorrelation[i] < minBaselineCorrelation )
    {
      minBaselineCorrelation = baselineCorrelation[i];
    }
    meanBaselineCorrelation += baselineCorrelation[i]
    / baselineCorrelation.size();
  }

  for ( unsigned int i = 0; i < baselineCorrelation.size(); i++ )
  {
    baselineCorrelationDev
      += ( baselineCorrelation[i]
    - meanBaselineCorrelation )
      * ( baselineCorrelation[i]
    - meanBaselineCorrelation ) / baselineCorrelation.size();                                                                                             //
    //
    // meanBaselineCorrelation
    //
    // +=
    //
    // baselineCorrelation[i]/baselineCorrelation.size();
  }
  baselineCorrelationDev = sqrt(baselineCorrelationDev);

  for ( unsigned int i = 0; i < gradientCorrelation.size(); i++ )
  {
    if ( gradientCorrelation[i] < minGradientCorrelation )
    {
      minGradientCorrelation = gradientCorrelation[i];
    }
    meanGradientCorrelation += gradientCorrelation[i]
    / gradientCorrelation.size();
  }

  for ( unsigned int i = 0; i < gradientCorrelation.size(); i++ )
  {
    gradientCorrelationDev
      += ( gradientCorrelation[i]
    - meanGradientCorrelation )
      * ( gradientCorrelation[i]
    - meanGradientCorrelation ) / gradientCorrelation.size();                                                                                             //
    //
    // meanBaselineCorrelation
    //
    // +=
    //
    // baselineCorrelation[i]/baselineCorrelation.size();
  }
  gradientCorrelationDev = sqrt(gradientCorrelationDev);

  // return values
  correlationThresholdBaseline = minBaselineCorrelation;
  correlationThresholdGradient = minGradientCorrelation;

  double maxBaselineCorrelationDevTimes;
  double maxGradientCorrelationDevTimes;

  maxBaselineCorrelationDevTimes
    = ( meanBaselineCorrelation
    - minBaselineCorrelation ) / baselineCorrelationDev;
  maxGradientCorrelationDevTimes
    = ( meanGradientCorrelation
    - minGradientCorrelation ) / gradientCorrelationDev;
  // return values

  correlationBaselineDevTimes = maxBaselineCorrelationDevTimes;
  correlationGradientDevTimes = maxGradientCorrelationDevTimes;

  std::cout << "minBaselineCorrelation: " << minBaselineCorrelation
    << std::endl;
  std::cout << "minGradientCorrelation: " << minGradientCorrelation
    << std::endl;
  std::cout << "baselineCorrelationDev: " << baselineCorrelationDev
    << std::endl;
  std::cout << "gradientCorrelationDev: " << gradientCorrelationDev
    << std::endl;
  std::cout << "maxBaselineCorrelationDevTimes: "
    << maxBaselineCorrelationDevTimes << std::endl;
  std::cout << "maxGradientCorrelationDevTimes: "
    << maxGradientCorrelationDevTimes << std::endl;

  return true;
}

void IntensityMotionCheckPanel::UpdateProtocolToTreeWidget( )
{
  lineEdit_Protocol->clear();
  treeWidget->clear();

  QTreeWidgetItem *itemQCOutputDirectory = new QTreeWidgetItem(treeWidget);
  itemQCOutputDirectory->setText( 0, tr("QC_QCOutputDirectory") );
  itemQCOutputDirectory->setText( 1,
    QString::fromStdString( this->GetProtocol().GetQCOutputDirectory() ) );

  QTreeWidgetItem *itemQCedDWIFileNameSuffix = new QTreeWidgetItem(treeWidget);
  itemQCedDWIFileNameSuffix->setText( 0, tr("QC_QCedDWIFileNameSuffix") );
  itemQCedDWIFileNameSuffix->setText( 1,
    QString::fromStdString( this->GetProtocol().GetQCedDWIFileNameSuffix() ) );

  QTreeWidgetItem *itemReportFileNameSuffix = new QTreeWidgetItem(treeWidget);
  itemReportFileNameSuffix->setText( 0, tr("QC_reportFileNameSuffix") );
  itemReportFileNameSuffix->setText( 1,
    QString::fromStdString( this->GetProtocol().GetReportFileNameSuffix() ) );

  QTreeWidgetItem *itemBadGradientPercentageTolerance = new QTreeWidgetItem(
    treeWidget);
  itemBadGradientPercentageTolerance->setText( 0,
    tr("QC_badGradientPercentageTolerance") );
  itemBadGradientPercentageTolerance->setText( 1,
    QString::number(this->GetProtocol().GetBadGradientPercentageTolerance(),
    'f',
    4) );

  QTreeWidgetItem *itemReportType = new QTreeWidgetItem(treeWidget);
  itemReportType->setText( 0, tr("QC_reportType") );
  itemReportType->setText( 1,
    QString("%1").arg(this->GetProtocol().GetReportType(), 0,
    10) );

  // image
  QTreeWidgetItem *itemImageInformation = new QTreeWidgetItem(treeWidget);
  itemImageInformation->setText( 0, tr("IMAGE_bCheck") );
  if ( this->GetProtocol().GetImageProtocol().bCheck )
  {
    itemImageInformation->setText( 1, tr("Yes") );
  }
  else
  {
    itemImageInformation->setText( 1, tr("No") );
  }

  QTreeWidgetItem *itemSpace = new QTreeWidgetItem(itemImageInformation);
  itemSpace->setText( 0, tr("IMAGE_space") );
  switch ( this->GetProtocol().GetImageProtocol().space )
  {
  case Protocol::SPACE_LPS:
    itemSpace->setText( 1, tr("left-posterior-superior") );
    break;
  case Protocol::SPACE_LPI:
    itemSpace->setText( 1, tr("left-posterior-inferior") );
    break;
  case Protocol::SPACE_LAS:
    itemSpace->setText( 1, tr("left-anterior-superior") );
    break;
  case Protocol::SPACE_LAI:
    itemSpace->setText( 1, tr("left-anterior-inferior") );
    break;
  case Protocol::SPACE_RPS:
    itemSpace->setText( 1, tr("right-posterior-superior") );
    break;
  case Protocol::SPACE_RPI:
    itemSpace->setText( 1, tr("right-posterior-inferior") );
    break;
  case Protocol::SPACE_RAS:
    itemSpace->setText( 1, tr("right-anterior-superior") );
    break;
  case Protocol::SPACE_RAI:
    itemSpace->setText( 1, tr("right-anterior-inferior") );
    break;
  default:
    itemSpace->setText( 1, tr("SPACE_UNKNOWN") );
    break;
  }

  QTreeWidgetItem *itemSpaceDirections = new QTreeWidgetItem(
    itemImageInformation);
  itemSpaceDirections->setText( 0, tr("IMAGE_directions") );
  itemSpaceDirections->setText(1, QString("%1 %2 %3, %4 %5 %6, %7 %8 %9")
    .arg(this->GetProtocol().GetImageProtocol().spacedirection[0][0], 0, 'f', 6)
    .arg(this->GetProtocol().GetImageProtocol().spacedirection[1][0], 0, 'f', 6)
    .arg(this->GetProtocol().GetImageProtocol().spacedirection[2][0], 0, 'f', 6)
    .arg(this->GetProtocol().GetImageProtocol().spacedirection[0][1], 0, 'f', 6)
    .arg(this->GetProtocol().GetImageProtocol().spacedirection[1][1], 0, 'f', 6)
    .arg(this->GetProtocol().GetImageProtocol().spacedirection[2][1], 0, 'f', 6)
    .arg(this->GetProtocol().GetImageProtocol().spacedirection[0][2], 0, 'f', 6)
    .arg(this->GetProtocol().GetImageProtocol().spacedirection[1][2], 0, 'f', 6)
    .arg(this->GetProtocol().GetImageProtocol().spacedirection[2][2], 0, 'f', 6)
    );

  QTreeWidgetItem *itemSizes = new QTreeWidgetItem(itemImageInformation);
  itemSizes->setText( 0, tr("IMAGE_size") );
  itemSizes->setText(1, QString("%1, %2, %3")
    .arg(this->GetProtocol().GetImageProtocol().size[0], 0, 10)
    .arg(this->GetProtocol().GetImageProtocol().size[1], 0, 10)
    .arg(this->GetProtocol().GetImageProtocol().size[2], 0, 10)
    );

  QTreeWidgetItem *itemSpacing = new QTreeWidgetItem(itemImageInformation);
  itemSpacing->setText( 0, tr("IMAGE_spacing") );
  itemSpacing->setText(1, QString("%1, %2, %3")
    .arg(this->GetProtocol().GetImageProtocol().spacing[0], 0, 'f', 6)
    .arg(this->GetProtocol().GetImageProtocol().spacing[1], 0, 'f', 6)
    .arg(this->GetProtocol().GetImageProtocol().spacing[2], 0, 'f', 6)
    );

  QTreeWidgetItem *itemOrig = new QTreeWidgetItem(itemImageInformation);
  itemOrig->setText( 0, tr("IMAGE_origin") );
  itemOrig->setText(1, QString("%1, %2, %3")
    .arg(this->GetProtocol().GetImageProtocol().origin[0], 0, 'f', 6)
    .arg(this->GetProtocol().GetImageProtocol().origin[1], 0, 'f', 6)
    .arg(this->GetProtocol().GetImageProtocol().origin[2], 0, 'f', 6)
    );

  QTreeWidgetItem *itemCrop = new QTreeWidgetItem(itemImageInformation);
  itemCrop->setText( 0, tr("IMAGE_bCrop") );
  if ( this->GetProtocol().GetImageProtocol().bCrop )
  {
    itemCrop->setText( 1, tr("Yes") );
  }
  else
  {
    itemCrop->setText( 1, tr("No") );
  }

  QTreeWidgetItem *itemCroppedDWIFileNameSuffix = new QTreeWidgetItem(
    itemImageInformation);
  itemCroppedDWIFileNameSuffix->setText( 0, tr("IMAGE_croppedDWIFileNameSuffix") );
  itemCroppedDWIFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetImageProtocol().
    croppedDWIFileNameSuffix) );

  QTreeWidgetItem *itemImageReportFileNameSuffix = new QTreeWidgetItem(
    itemImageInformation);
  itemImageReportFileNameSuffix->setText( 0, tr("IMAGE_reportFileNameSuffix") );
  itemImageReportFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetImageProtocol().
    reportFileNameSuffix) );

  QTreeWidgetItem *itemImageReportFileMode = new QTreeWidgetItem(
    itemImageInformation);
  itemImageReportFileMode->setText( 0, tr("IMAGE_reportFileMode") );
  itemImageReportFileMode->setText( 1,
    QString("%1").arg(this->GetProtocol().GetImageProtocol().reportFileMode, 0,
    10) );

  QTreeWidgetItem *itembQuitOnCheckSpacingFailure = new QTreeWidgetItem(
    itemImageInformation);
  itembQuitOnCheckSpacingFailure->setText( 0, tr("IMAGE_bQuitOnCheckSpacingFailure") );
  if ( this->GetProtocol().GetImageProtocol().bQuitOnCheckSpacingFailure )
  {
    itembQuitOnCheckSpacingFailure->setText( 1, tr("Yes") );
  }
  else
  {
    itembQuitOnCheckSpacingFailure->setText( 1, tr("No") );
  }

  QTreeWidgetItem *itembQuitOnCheckSizeFailure = new QTreeWidgetItem(
    itemImageInformation);
  itembQuitOnCheckSizeFailure->setText( 0, tr("IMAGE_bQuitOnCheckSizeFailure") );
  if ( this->GetProtocol().GetImageProtocol().bQuitOnCheckSizeFailure )
  {
    itembQuitOnCheckSizeFailure->setText( 1, tr("Yes") );
  }
  else
  {
    itembQuitOnCheckSizeFailure->setText( 1, tr("No") );
  }

  // diffusion
  QTreeWidgetItem *itemDiffusionInformation = new QTreeWidgetItem(treeWidget);
  itemDiffusionInformation->setText( 0, tr("DIFFUSION_bCheck") );
  if ( this->GetProtocol().GetDiffusionProtocol().bCheck )
  {
    itemDiffusionInformation->setText( 1, tr("Yes") );
  }
  else
  {
    itemDiffusionInformation->setText( 1, tr("No") );
  }
  std::cout<<"PPPPPP\n"<< this->GetProtocol().GetDiffusionProtocol().measurementFrame <<
  "PPPPPPPP\n\n\n\n\n" << std::flush << std::endl;
  
  QTreeWidgetItem *itemMeasurementFrame = new QTreeWidgetItem(
    itemDiffusionInformation);
  itemMeasurementFrame->setText( 0, tr("DIFFUSION_measurementFrame") );
  itemMeasurementFrame->setText(1, QString("%1 %2 %3, %4 %5 %6, %7 %8 %9")
    .arg(this->GetProtocol().GetDiffusionProtocol().measurementFrame[0][0], 0, 'f', 6)
    .arg(this->GetProtocol().GetDiffusionProtocol().measurementFrame[0][1], 0, 'f', 6)
    .arg(this->GetProtocol().GetDiffusionProtocol().measurementFrame[0][2], 0, 'f', 6)
    .arg(this->GetProtocol().GetDiffusionProtocol().measurementFrame[1][0], 0, 'f', 6)
    .arg(this->GetProtocol().GetDiffusionProtocol().measurementFrame[1][1], 0, 'f', 6)
    .arg(this->GetProtocol().GetDiffusionProtocol().measurementFrame[1][2], 0, 'f', 6)
    .arg(this->GetProtocol().GetDiffusionProtocol().measurementFrame[2][0], 0, 'f', 6)
    .arg(this->GetProtocol().GetDiffusionProtocol().measurementFrame[2][1], 0, 'f', 6)
    .arg(this->GetProtocol().GetDiffusionProtocol().measurementFrame[2][2], 0, 'f', 6)
    );

  QTreeWidgetItem *itemBValue = new QTreeWidgetItem(itemDiffusionInformation);
  itemBValue->setText( 0, tr("DIFFUSION_DWMRI_bValue") );
  itemBValue->setText( 1,
    QString("%1").arg(this->GetProtocol().GetDiffusionProtocol().bValue, 0, 'f',
    4) );

  for ( unsigned int i = 0;
    i < this->GetProtocol().GetDiffusionProtocol().gradients.size();
    i++ )
  {
    QTreeWidgetItem *itemGradientDir = new QTreeWidgetItem(
      itemDiffusionInformation);
    itemGradientDir->setText( 0,
      QString("DIFFUSION_DWMRI_gradient_%1").arg( i, 4, 10, QLatin1Char( '0' ) ) );
    itemGradientDir->setText(1, QString("%1 %2 %3")
      .arg(this->GetProtocol().GetDiffusionProtocol().gradients[i][0], 0, 'f',
      6)
      .arg(this->GetProtocol().GetDiffusionProtocol().gradients[i][1], 0, 'f',
      6)
      .arg(this->GetProtocol().GetDiffusionProtocol().gradients[i][2], 0, 'f',
      6)
      );
  }

  QTreeWidgetItem *itembUseDiffusionProtocol = new QTreeWidgetItem(
    itemDiffusionInformation);
  itembUseDiffusionProtocol->setText( 0, tr("DIFFUSION_bUseDiffusionProtocol") );
  if ( this->GetProtocol().GetDiffusionProtocol().bUseDiffusionProtocol )
  {
    itembUseDiffusionProtocol->setText( 1, tr("Yes") );
  }
  else
  {
    itembUseDiffusionProtocol->setText( 1, tr("No") );
  }

  QTreeWidgetItem *itemDiffusionReplacedDWIFileNameSuffix = new QTreeWidgetItem(
    itemDiffusionInformation);
  itemDiffusionReplacedDWIFileNameSuffix->setText( 0,
    tr("DIFFUSION_diffusionReplacedDWIFileNameSuffix") );
  itemDiffusionReplacedDWIFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetDiffusionProtocol().
    diffusionReplacedDWIFileNameSuffix) );

  QTreeWidgetItem *itemDiffusionReportFileNameSuffix = new QTreeWidgetItem(
    itemDiffusionInformation);
  itemDiffusionReportFileNameSuffix->setText( 0,
    tr("DIFFUSION_reportFileNameSuffix") );
  itemDiffusionReportFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetDiffusionProtocol().
    reportFileNameSuffix) );

  QTreeWidgetItem *itemDiffusionReportFileMode = new QTreeWidgetItem(
    itemDiffusionInformation);
  itemDiffusionReportFileMode->setText( 0, tr("DIFFUSION_reportFileMode") );
  itemDiffusionReportFileMode->setText( 1,
    QString("%1").arg(this->GetProtocol().GetDiffusionProtocol().reportFileMode,
    0,
    10) );

  QTreeWidgetItem *itemDiffusionbQuitOnCheckFailure = new QTreeWidgetItem(
    itemDiffusionInformation);
  itemDiffusionbQuitOnCheckFailure->setText( 0, tr("DIFFUSION_bQuitOnCheckFailure") );
  if ( this->GetProtocol().GetDiffusionProtocol().bQuitOnCheckFailure )
  {
    itemDiffusionbQuitOnCheckFailure->setText( 1, tr("Yes") );
  }
  else
  {
    itemDiffusionbQuitOnCheckFailure->setText( 1, tr("No") );
  }


  // Slice Check
  QTreeWidgetItem *itemSliceCheck = new QTreeWidgetItem(treeWidget);
  itemSliceCheck->setText( 0, tr("SLICE_bCheck") );
  if ( this->GetProtocol().GetSliceCheckProtocol().bCheck )
  {
    itemSliceCheck->setText( 1, tr("Yes") );
  }
  else
  {
    itemSliceCheck->setText( 1, tr("No") );
  }

  QTreeWidgetItem *itembSubregionalCheck = new QTreeWidgetItem(itemSliceCheck);
  itembSubregionalCheck->setText( 0, tr("SLICE_bSubregionalCheck") );
  if ( this->GetProtocol().GetSliceCheckProtocol().bSubregionalCheck )
  {
    itembSubregionalCheck->setText( 1, tr("Yes") );
  }
  else
  {
    itembSubregionalCheck->setText( 1, tr("No") );
  }

  QTreeWidgetItem *itemSubregionalCheckRelaxationFactor = new QTreeWidgetItem(itemSliceCheck);
  itemSubregionalCheckRelaxationFactor->setText( 0, tr("SLICE_subregionalCheckRelaxationFactor") );
  itemSubregionalCheckRelaxationFactor->setText( 1,
    QString::number(this->GetProtocol().GetSliceCheckProtocol().
    subregionalCheckRelaxationFactor,  'f', 4) );

  //   QTreeWidgetItem *itemSliceBadGradientPercentageTolerance = new
  // QTreeWidgetItem(itemSliceCheck);
  //   itemSliceBadGradientPercentageTolerance->setText(0,
  // tr("SLICE_badGradientPercentageTolerance"));
  //
  //
  //
  //
  // itemSliceBadGradientPercentageTolerance->setText(1,QString::number(this->GetProtocol().GetSliceCheckProtocol().badGradientPercentageTolerance,
  //  'f', 4));

  QTreeWidgetItem *itemImageCheckTimes = new QTreeWidgetItem(itemSliceCheck);
  itemImageCheckTimes->setText( 0, tr("SLICE_checkTimes") );
  itemImageCheckTimes->setText( 1,
    QString("%1").arg(this->GetProtocol().GetSliceCheckProtocol().checkTimes, 0,
    10) );

  QTreeWidgetItem *itemBeginSkip = new QTreeWidgetItem(itemSliceCheck);
  itemBeginSkip->setText( 0, tr("SLICE_headSkipSlicePercentage") );
  itemBeginSkip->setText( 1,
    QString::number(this->GetProtocol().GetSliceCheckProtocol().
    headSkipSlicePercentage,  'f', 4) );

  QTreeWidgetItem *itemEndLeft = new QTreeWidgetItem(itemSliceCheck);
  itemEndLeft->setText( 0, tr("SLICE_tailSkipSlicePercentage") );
  itemEndLeft->setText( 1,
    QString::number(this->GetProtocol().GetSliceCheckProtocol().
    tailSkipSlicePercentage,  'f', 4) );

  QTreeWidgetItem *itemBaselineCorrelationDev = new QTreeWidgetItem(
    itemSliceCheck);
  itemBaselineCorrelationDev->setText( 0,
    tr("SLICE_correlationDeviationThresholdbaseline") );
  itemBaselineCorrelationDev->setText( 1,
    QString::number(this->GetProtocol().GetSliceCheckProtocol().
    correlationDeviationThresholdbaseline,  'f', 4) );

  QTreeWidgetItem *itemGradientCorrelationDev = new QTreeWidgetItem(
    itemSliceCheck);
  itemGradientCorrelationDev->setText( 0,
    tr("SLICE_correlationDeviationThresholdgradient") );
  itemGradientCorrelationDev->setText( 1,
    QString::number(this->GetProtocol().GetSliceCheckProtocol().
    correlationDeviationThresholdgradient, 'f', 4) );

  QTreeWidgetItem *itemSliceOutputDWIFileNameSuffix = new QTreeWidgetItem(
    itemSliceCheck);
  itemSliceOutputDWIFileNameSuffix->setText( 0,
    tr("SLICE_outputDWIFileNameSuffix") );
  itemSliceOutputDWIFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetSliceCheckProtocol().
    outputDWIFileNameSuffix) );

  QTreeWidgetItem *itemSliceReportFileNameSuffix = new QTreeWidgetItem(
    itemSliceCheck);
  itemSliceReportFileNameSuffix->setText( 0, tr("SLICE_reportFileNameSuffix") );
  itemSliceReportFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetSliceCheckProtocol().
    reportFileNameSuffix) );

  QTreeWidgetItem *itemSliceReportFileMode = new QTreeWidgetItem(itemSliceCheck);
  itemSliceReportFileMode->setText( 0, tr("SLICE_reportFileMode") );
  itemSliceReportFileMode->setText( 1,
    QString("%1").arg(this->GetProtocol().GetSliceCheckProtocol().
    reportFileMode,
    0, 10) );

  QTreeWidgetItem *itemSliceExcludedDWINrrdFileNameSuffix = new QTreeWidgetItem(
    itemSliceCheck);
  itemSliceExcludedDWINrrdFileNameSuffix->setText( 0, tr("SLICE_excludedDWINrrdFileNameSuffix") );
  itemSliceExcludedDWINrrdFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetSliceCheckProtocol().
    excludedDWINrrdFileNameSuffix) );

  QTreeWidgetItem *itemSlicebQuitOnCheckFailur = new QTreeWidgetItem(itemSliceCheck);
  itemSlicebQuitOnCheckFailur->setText( 0, tr("SLICE_bQuitOnCheckFailure") );
  if ( this->GetProtocol().GetSliceCheckProtocol().bQuitOnCheckFailure )
  {
    itemSlicebQuitOnCheckFailur->setText( 1, tr("Yes") );
  }
  else
  {
    itemSlicebQuitOnCheckFailur->setText( 1, tr("No") );
  }

  // interlace check
  QTreeWidgetItem *itemInterlaceCheck = new QTreeWidgetItem(treeWidget);
  itemInterlaceCheck->setText( 0, tr("INTERLACE_bCheck") );
  if ( this->GetProtocol().GetInterlaceCheckProtocol().bCheck )
  {
    itemInterlaceCheck->setText( 1, tr("Yes") );
  }
  else
  {
    itemInterlaceCheck->setText( 1, tr("No") );
  }

  //   QTreeWidgetItem *itemInterlaceBadGradientPercentageTolerance = new
  // QTreeWidgetItem(itemInterlaceCheck);
  //   itemInterlaceBadGradientPercentageTolerance->setText(0,
  // tr("INTERLACE_badGradientPercentageTolerance"));
  //
  //
  //
  //
  // itemInterlaceBadGradientPercentageTolerance->setText(1,QString::number(this->GetProtocol().GetInterlaceCheckProtocol().badGradientPercentageTolerance,
  //  'f', 4));

  QTreeWidgetItem *itemInterlaceCorrBaseline = new QTreeWidgetItem(
    itemInterlaceCheck);
  itemInterlaceCorrBaseline->setText( 0,
    tr("INTERLACE_correlationThresholdBaseline") );
  itemInterlaceCorrBaseline->setText( 1,
    QString::number(this->GetProtocol().GetInterlaceCheckProtocol().
    correlationThresholdBaseline, 'f', 4) );

  QTreeWidgetItem *itemInterlaceCorrGrad = new QTreeWidgetItem(
    itemInterlaceCheck);
  itemInterlaceCorrGrad->setText( 0,
    tr("INTERLACE_correlationThresholdGradient") );
  itemInterlaceCorrGrad->setText( 1,
    QString::number(this->GetProtocol().GetInterlaceCheckProtocol().
    correlationThresholdGradient, 'f', 4) );

  QTreeWidgetItem *itemInterlaceCorrDevBaseline = new QTreeWidgetItem(
    itemInterlaceCheck);
  itemInterlaceCorrDevBaseline->setText( 0,
    tr("INTERLACE_correlationDeviationBaseline") );
  itemInterlaceCorrDevBaseline->setText( 1,
    QString::number(this->GetProtocol().GetInterlaceCheckProtocol().
    correlationDeviationBaseline, 'f', 4) );

  QTreeWidgetItem *itemInterlaceCorrDevGrad = new QTreeWidgetItem(
    itemInterlaceCheck);
  itemInterlaceCorrDevGrad->setText( 0,
    tr("INTERLACE_correlationDeviationGradient") );
  itemInterlaceCorrDevGrad->setText( 1,
    QString::number(this->GetProtocol().GetInterlaceCheckProtocol().
    correlationDeviationGradient, 'f', 4) );

  QTreeWidgetItem *itemInterlaceTranslation = new QTreeWidgetItem(
    itemInterlaceCheck);
  itemInterlaceTranslation->setText( 0, tr("INTERLACE_translationThreshold") );
  itemInterlaceTranslation->setText( 1,
    QString::number(this->GetProtocol().GetInterlaceCheckProtocol().
    translationThreshold, 'f', 4) );

  QTreeWidgetItem *itemInterlaceRotation = new QTreeWidgetItem(
    itemInterlaceCheck);
  itemInterlaceRotation->setText( 0, tr("INTERLACE_rotationThreshold") );
  itemInterlaceRotation->setText( 1,
    QString::number(this->GetProtocol().GetInterlaceCheckProtocol().
    rotationThreshold, 'f', 4) );

  QTreeWidgetItem *itemInterlaceOutputDWIFileNameSuffix = new QTreeWidgetItem(
    itemInterlaceCheck);
  itemInterlaceOutputDWIFileNameSuffix->setText( 0,
    tr("INTERLACE_outputDWIFileNameSuffix") );
  itemInterlaceOutputDWIFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetInterlaceCheckProtocol().
    outputDWIFileNameSuffix) );

  QTreeWidgetItem *itemInterlaceReportFileNameSuffix = new QTreeWidgetItem(
    itemInterlaceCheck);
  itemInterlaceReportFileNameSuffix->setText( 0,
    tr("INTERLACE_reportFileNameSuffix") );
  itemInterlaceReportFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetInterlaceCheckProtocol().
    reportFileNameSuffix) );

  QTreeWidgetItem *itemInterlaceReportFileMode = new QTreeWidgetItem(
    itemInterlaceCheck);
  itemInterlaceReportFileMode->setText( 0, tr("INTERLACE_reportFileMode") );
  itemInterlaceReportFileMode->setText( 1,
    QString("%1").arg(this->GetProtocol().GetInterlaceCheckProtocol().
    reportFileMode, 0, 10) );

  QTreeWidgetItem *itemInterlaceExcludedDWINrrdFileNameSuffix = new QTreeWidgetItem(
    itemInterlaceCheck);
  itemInterlaceExcludedDWINrrdFileNameSuffix->setText( 0, tr("INTERLACE_excludedDWINrrdFileNameSuffix") );
  itemInterlaceExcludedDWINrrdFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetInterlaceCheckProtocol().
    excludedDWINrrdFileNameSuffix) );

  QTreeWidgetItem *itembInterlaceQuitOnCheckFailur = new QTreeWidgetItem(itemInterlaceCheck);
  itembInterlaceQuitOnCheckFailur->setText( 0, tr("INTERLACE_bQuitOnCheckFailure") );
  if ( this->GetProtocol().GetInterlaceCheckProtocol().bQuitOnCheckFailure )
  {
    itembInterlaceQuitOnCheckFailur->setText( 1, tr("Yes") );
  }
  else
  {
    itembInterlaceQuitOnCheckFailur->setText( 1, tr("No") );
  }

  // baseline average
  QTreeWidgetItem *itemBaselineAverage = new QTreeWidgetItem(treeWidget);
  itemBaselineAverage->setText( 0, tr("BASELINE_bAverage") );
  if ( this->GetProtocol().GetBaselineAverageProtocol().bAverage )
  {
    itemBaselineAverage->setText( 1, tr("Yes") );
  }
  else
  {
    itemBaselineAverage->setText( 1, tr("No") );
  }

  QTreeWidgetItem *itemBaselineAverageMethod = new QTreeWidgetItem(
    itemBaselineAverage);
  itemBaselineAverageMethod->setText( 0, tr("BASELINE_averageMethod") );
  itemBaselineAverageMethod->setText( 1,
    QString("%1").arg(this->GetProtocol().GetBaselineAverageProtocol().
    averageMethod,  0, 10) );

  QTreeWidgetItem *itemBaselineAverageStopThreshold = new QTreeWidgetItem(
    itemBaselineAverage);
  itemBaselineAverageStopThreshold->setText( 0, tr("BASELINE_stopThreshold") );
  itemBaselineAverageStopThreshold->setText( 1,
    QString::number(this->GetProtocol().GetBaselineAverageProtocol().
    stopThreshold,
    'f', 4) );

  QTreeWidgetItem *itemBaselineAverageOutputDWIFileNameSuffix
    = new QTreeWidgetItem(itemBaselineAverage);
  itemBaselineAverageOutputDWIFileNameSuffix->setText( 0,
    tr("BASELINE_outputDWIFileNameSuffix") );
  itemBaselineAverageOutputDWIFileNameSuffix->setText(
    1,
    QString::fromStdString(this->GetProtocol().GetBaselineAverageProtocol().
    outputDWIFileNameSuffix) );

  QTreeWidgetItem *itemBaselineAverageReportFileNameSuffix
    = new QTreeWidgetItem(itemBaselineAverage);
  itemBaselineAverageReportFileNameSuffix->setText( 0,
    tr("BASELINE_reportFileNameSuffix") );
  itemBaselineAverageReportFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetBaselineAverageProtocol().
    reportFileNameSuffix) );

  QTreeWidgetItem *itemBaselineReportFileMode = new QTreeWidgetItem(
    itemBaselineAverage);
  itemBaselineReportFileMode->setText( 0, tr("BASELINE_reportFileMode") );
  itemBaselineReportFileMode->setText( 1,
    QString("%1").arg(this->GetProtocol().GetBaselineAverageProtocol().
    reportFileMode, 0, 10) );

  // EddyMotion
  QTreeWidgetItem *itemEddyMotionCorrection = new QTreeWidgetItem(treeWidget);
  itemEddyMotionCorrection->setText( 0, tr("EDDYMOTION_bCorrect") );
  if ( this->GetProtocol().GetEddyMotionCorrectionProtocol().bCorrect )
  {
    itemEddyMotionCorrection->setText( 1, tr("Yes") );
  }
  else
  {
    itemEddyMotionCorrection->setText( 1, tr("No") );
  }

  //   QTreeWidgetItem *itemEddyMotionCommand = new
  // QTreeWidgetItem(itemEddyMotionCorrection);
  //   itemEddyMotionCommand->setText(0, tr("EDDYMOTION_command"));
  //
  //
  //
  //
  // itemEddyMotionCommand->setText(1,QString::fromStdString(this->GetProtocol().GetEddyMotionCorrectionProtocol().EddyMotionCommand));

  //  QTreeWidgetItem *itemEddyMotionInputFilename = new
  // QTreeWidgetItem(itemEddyMotionCorrection);
  //  itemEddyMotionInputFilename->setText(0, tr("EddyMotionInputFileName"));
  //
  //
  //
  // itemEddyMotionInputFilename->setText(1,QString::fromStdString(this->GetProtocol().GetEddyMotionCorrectionProtocol().inputFileName));

  //  QTreeWidgetItem *itemEddyMotionOutputFilename = new
  // QTreeWidgetItem(itemEddyMotionCorrection);
  //  itemEddyMotionOutputFilename->setText(0, tr("EddyMotionOutputFileName"));
  //
  //
  //
  // itemEddyMotionOutputFilename->setText(1,QString::fromStdString(this->GetProtocol().GetEddyMotionCorrectionProtocol().outputFileName));

  QTreeWidgetItem *itemNumberOfBins = new QTreeWidgetItem(
    itemEddyMotionCorrection);
  itemNumberOfBins->setText( 0, tr("EDDYMOTION_numberOfBins") );
  itemNumberOfBins->setText( 1,
    QString("%1").arg(this->GetProtocol().GetEddyMotionCorrectionProtocol().
    numberOfBins,  0, 10) );

  QTreeWidgetItem *itemNumberOfSamples = new QTreeWidgetItem(
    itemEddyMotionCorrection);
  itemNumberOfSamples->setText( 0, tr("EDDYMOTION_numberOfSamples") );
  itemNumberOfSamples->setText( 1,
    QString("%1").arg(this->GetProtocol().GetEddyMotionCorrectionProtocol().
    numberOfSamples,  0, 10) );

  QTreeWidgetItem *itemTranslationScale = new QTreeWidgetItem(
    itemEddyMotionCorrection);
  itemTranslationScale->setText( 0, tr("EDDYMOTION_translationScale") );
  itemTranslationScale->setText( 1,
    QString::number(this->GetProtocol().GetEddyMotionCorrectionProtocol().
    translationScale,  'f', 4) );

  QTreeWidgetItem *itemStepLength = new QTreeWidgetItem(
    itemEddyMotionCorrection);
  itemStepLength->setText( 0, tr("EDDYMOTION_stepLength") );
  itemStepLength->setText( 1,
    QString::number(this->GetProtocol().GetEddyMotionCorrectionProtocol().
    stepLength,  'f', 4) );

  QTreeWidgetItem *itemRelaxFactor = new QTreeWidgetItem(
    itemEddyMotionCorrection);
  itemRelaxFactor->setText( 0, tr("EDDYMOTION_relaxFactor") );
  itemRelaxFactor->setText( 1,
    QString::number(this->GetProtocol().GetEddyMotionCorrectionProtocol().
    relaxFactor,  'f', 4) );

  QTreeWidgetItem *itemMaxNumberOfIterations = new QTreeWidgetItem(
    itemEddyMotionCorrection);
  itemMaxNumberOfIterations->setText( 0, tr("EDDYMOTION_maxNumberOfIterations") );
  itemMaxNumberOfIterations->setText( 1,
    QString("%1").arg(this->GetProtocol().GetEddyMotionCorrectionProtocol().
    maxNumberOfIterations,  0, 10) );
  // ////////////////////////////////////////////////////////////////////////

  QTreeWidgetItem *itemEddyMotionOutputDWIFileNameSuffix = new QTreeWidgetItem(
    itemEddyMotionCorrection);
  itemEddyMotionOutputDWIFileNameSuffix->setText( 0,
    tr("EDDYMOTION_outputDWIFileNameSuffix") );
  itemEddyMotionOutputDWIFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetEddyMotionCorrectionProtocol()
    .
    outputDWIFileNameSuffix) );

  QTreeWidgetItem *itemEddyMotionReportFileNameSuffix = new QTreeWidgetItem(
    itemEddyMotionCorrection);
  itemEddyMotionReportFileNameSuffix->setText( 0,
    tr("EDDYMOTION_reportFileNameSuffix") );
  itemEddyMotionReportFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetEddyMotionCorrectionProtocol()
    .
    reportFileNameSuffix) );

  QTreeWidgetItem *itemEddyMotionReportFileMode = new QTreeWidgetItem(
    itemEddyMotionCorrection);
  itemEddyMotionReportFileMode->setText( 0, tr("EDDYMOTION_reportFileMode") );
  itemEddyMotionReportFileMode->setText( 1,
    QString("%1").arg(this->GetProtocol().GetEddyMotionCorrectionProtocol().
    reportFileMode, 0, 10) );

  // gradient wise check
  QTreeWidgetItem *itemGradientCheck = new QTreeWidgetItem(treeWidget);
  itemGradientCheck->setText( 0, tr("GRADIENT_bCheck") );
  if ( this->GetProtocol().GetGradientCheckProtocol().bCheck )
  {
    itemGradientCheck->setText( 1, tr("Yes") );
  }
  else
  {
    itemGradientCheck->setText( 1, tr("No") );
  }

  //   QTreeWidgetItem *itemGradientBadGradientPercentageTolerance = new
  // QTreeWidgetItem(itemGradientCheck);
  //   itemGradientBadGradientPercentageTolerance->setText(0,
  // tr("GRADIENT_badGradientPercentageTolerance"));
  //
  //
  //
  //
  // itemGradientBadGradientPercentageTolerance->setText(1,QString::number(this->GetProtocol().GetGradientCheckProtocol().badGradientPercentageTolerance,
  //  'f', 4));

  QTreeWidgetItem *itemGradientTranslation = new QTreeWidgetItem(
    itemGradientCheck);
  itemGradientTranslation->setText( 0, tr("GRADIENT_translationThrehshold") );
  itemGradientTranslation->setText( 1,
    QString::number(this->GetProtocol().GetGradientCheckProtocol().
    translationThreshold, 'f', 4) );

  QTreeWidgetItem *itemGradientRotation = new QTreeWidgetItem(itemGradientCheck);
  itemGradientRotation->setText( 0, tr("GRADIENT_rotationThreshold") );
  itemGradientRotation->setText( 1,
    QString::number(this->GetProtocol().GetGradientCheckProtocol().
    rotationThreshold, 'f', 4) );

  QTreeWidgetItem *itemGradientOutputDWIFileNameSuffix = new QTreeWidgetItem(
    itemGradientCheck);
  itemGradientOutputDWIFileNameSuffix->setText( 0,
    tr("GRADIENT_outputDWIFileNameSuffix") );
  itemGradientOutputDWIFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetGradientCheckProtocol().
    outputDWIFileNameSuffix) );

  QTreeWidgetItem *itemGradientReportFileNameSuffix = new QTreeWidgetItem(
    itemGradientCheck);
  itemGradientReportFileNameSuffix->setText( 0,
    tr("GRADIENT_reportFileNameSuffix") );
  itemGradientReportFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetGradientCheckProtocol().
    reportFileNameSuffix) );

  QTreeWidgetItem *itemGradientReportFileMode = new QTreeWidgetItem(
    itemGradientCheck);
  itemGradientReportFileMode->setText( 0, tr("GRADIENT_reportFileMode") );
  itemGradientReportFileMode->setText( 1,
    QString("%1").arg(this->GetProtocol().GetGradientCheckProtocol().
    reportFileMode, 0, 10) );

  QTreeWidgetItem *itemGradientExcludedDWINrrdFileNameSuffix = new QTreeWidgetItem(
    itemGradientCheck);
  itemGradientExcludedDWINrrdFileNameSuffix->setText( 0, tr("GRADIENT_excludedDWINrrdFileNameSuffix") );
  itemGradientExcludedDWINrrdFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetGradientCheckProtocol().
    excludedDWINrrdFileNameSuffix) );

  QTreeWidgetItem *itembGradientQuitOnCheckFailur = new QTreeWidgetItem(itemGradientCheck);
  itembGradientQuitOnCheckFailur->setText( 0, tr("GRADIENT_bQuitOnCheckFailure") );
  if ( this->GetProtocol().GetGradientCheckProtocol().bQuitOnCheckFailure )
  {
    itembGradientQuitOnCheckFailur->setText( 1, tr("Yes") );
  }
  else
  {
    itembGradientQuitOnCheckFailur->setText( 1, tr("No") );
  }


  // DTI Computing
  QTreeWidgetItem *itemDTIComputing = new QTreeWidgetItem(treeWidget);
  itemDTIComputing->setText( 0, tr("DTI_bCompute") );
  if ( this->GetProtocol().GetDTIProtocol().bCompute )
  {
    itemDTIComputing->setText( 1, tr("Yes") );
  }
  else
  {
    itemDTIComputing->setText( 1, tr("No") );
  }

  QTreeWidgetItem *itemDtiestimCommand = new QTreeWidgetItem(itemDTIComputing);
  itemDtiestimCommand->setText( 0, tr("DTI_dtiestimCommand") );
  itemDtiestimCommand->setText( 1,
    QString::fromStdString(this->GetProtocol().GetDTIProtocol().dtiestimCommand) );

  QTreeWidgetItem *itemDtiprocessCommand = new QTreeWidgetItem(itemDTIComputing);
  itemDtiprocessCommand->setText( 0, tr("DTI_dtiprocessCommand") );
  itemDtiprocessCommand->setText( 1,
    QString::fromStdString(this->GetProtocol().GetDTIProtocol().
    dtiprocessCommand) );

  QTreeWidgetItem *itemMethod = new QTreeWidgetItem(itemDTIComputing);
  itemMethod->setText( 0, tr("DTI_method") );
  switch ( this->GetProtocol().GetDTIProtocol().method )
  {
  case Protocol::METHOD_WLS:
    itemMethod->setText( 1, tr("wls") );
    break;
  case Protocol::METHOD_LLS:
    itemMethod->setText( 1, tr("lls") );
    break;
  case Protocol::METHOD_ML:
    itemMethod->setText( 1, tr("ml") );
    break;
  case Protocol::METHOD_NLS:
    itemMethod->setText( 1, tr("nls") );
    break;
  default:
    itemMethod->setText( 1, tr("lls") );
    break;
  }

  QTreeWidgetItem *itemBaselineThreshold = new QTreeWidgetItem(itemDTIComputing);
  itemBaselineThreshold->setText( 0, tr("DTI_baselineThreshold") );
  itemBaselineThreshold->setText( 1,
    QString::number(this->GetProtocol().GetDTIProtocol().baselineThreshold) );

  QTreeWidgetItem *itemMaskFile = new QTreeWidgetItem(itemDTIComputing);
  itemMaskFile->setText( 0, tr("DTI_maskFileName") );
  itemMaskFile->setText( 1,
    QString::fromStdString(this->GetProtocol().GetDTIProtocol().mask ) );

  QTreeWidgetItem *itemTensorFile = new QTreeWidgetItem(itemDTIComputing);
  itemTensorFile->setText( 0, tr("DTI_tensor") );
  itemTensorFile->setText( 1,
    QString::fromStdString(this->GetProtocol().GetDTIProtocol().tensorSuffix ) );

  // tensor scalar images
  QTreeWidgetItem *itemScalarBaseline = new QTreeWidgetItem(itemDTIComputing);
  itemScalarBaseline->setText( 0, tr("DTI_baseline") );
  if ( this->GetProtocol().GetDTIProtocol().bbaseline )
  {
    itemScalarBaseline->setText( 1, tr("Yes, ")
      + QString::fromStdString(this->GetProtocol().GetDTIProtocol().
      baselineSuffix ) );
  }
  else
  {
    itemScalarBaseline->setText( 1, tr("No, ")
      + QString::fromStdString(this->GetProtocol().GetDTIProtocol().
      baselineSuffix ) );
  }

  QTreeWidgetItem *itemScalarIDWI = new QTreeWidgetItem(itemDTIComputing);
  itemScalarIDWI->setText( 0, tr("DTI_idwi") );
  if ( this->GetProtocol().GetDTIProtocol().bidwi )
  {
    itemScalarIDWI->setText( 1, tr("Yes, ")
      + QString::fromStdString(this->GetProtocol().GetDTIProtocol().idwiSuffix ) );
  }
  else
  {
    itemScalarIDWI->setText( 1, tr("No, ")
      + QString::fromStdString(this->GetProtocol().GetDTIProtocol().idwiSuffix ) );
  }

  QTreeWidgetItem *itemScalarFA = new QTreeWidgetItem(itemDTIComputing);
  itemScalarFA->setText( 0, tr("DTI_fa") );
  if ( this->GetProtocol().GetDTIProtocol().bfa )
  {
    itemScalarFA->setText( 1, tr("Yes, ")
      + QString::fromStdString(this->GetProtocol().GetDTIProtocol().faSuffix ) );
  }
  else
  {
    itemScalarFA->setText( 1, tr("No, ")
      + QString::fromStdString(this->GetProtocol().GetDTIProtocol().faSuffix ) );
  }

  QTreeWidgetItem *itemScalarMD = new QTreeWidgetItem(itemDTIComputing);
  itemScalarMD->setText( 0, tr("DTI_md") );
  if ( this->GetProtocol().GetDTIProtocol().bmd )
  {
    itemScalarMD->setText( 1, tr("Yes, ")
      + QString::fromStdString(this->GetProtocol().GetDTIProtocol().mdSuffix ) );
  }
  else
  {
    itemScalarMD->setText( 1, tr("No, ")
      + QString::fromStdString(this->GetProtocol().GetDTIProtocol().mdSuffix ) );
  }

  QTreeWidgetItem *itemScalarcolorFA = new QTreeWidgetItem(itemDTIComputing);
  itemScalarcolorFA->setText( 0, tr("DTI_colorfa") );
  if ( this->GetProtocol().GetDTIProtocol().bcoloredfa )
  {
    itemScalarcolorFA->setText( 1, tr("Yes, ")
      + QString::fromStdString(this->GetProtocol().GetDTIProtocol().
      coloredfaSuffix ) );
  }
  else
  {
    itemScalarcolorFA->setText( 1, tr("No, ")
      + QString::fromStdString(this->GetProtocol().GetDTIProtocol().
      coloredfaSuffix ) );
  }

  QTreeWidgetItem *itemScalarFrobenius = new QTreeWidgetItem(itemDTIComputing);
  itemScalarFrobenius->setText( 0, tr("DTI_frobeniusnorm") );
  if ( this->GetProtocol().GetDTIProtocol().bfrobeniusnorm )
  {
    itemScalarFrobenius->setText( 1, tr("Yes, ")
      + QString::fromStdString(this->GetProtocol().GetDTIProtocol().
      frobeniusnormSuffix ) );
  }
  else
  {
    itemScalarFrobenius->setText( 1, tr("No, ")
      + QString::fromStdString(this->GetProtocol().GetDTIProtocol().
      frobeniusnormSuffix ) );
  }

  QTreeWidgetItem *itemDTIReportFileNameSuffix = new QTreeWidgetItem(
    itemDTIComputing);
  itemDTIReportFileNameSuffix->setText( 0, tr("DTI_reportFileNameSuffix") );
  itemDTIReportFileNameSuffix->setText( 1,
    QString::fromStdString(this->GetProtocol().GetDTIProtocol().
    reportFileNameSuffix) );

  QTreeWidgetItem *itemEDTIReportFileMode = new QTreeWidgetItem(
    itemDTIComputing);
  itemEDTIReportFileMode->setText( 0, tr("DTI_reportFileMode") );
  itemEDTIReportFileMode->setText( 1,
    QString("%1").arg(this->GetProtocol().GetDTIProtocol().reportFileMode, 0,
    10) );
}

void IntensityMotionCheckPanel::ResultUpdate()
{
  bResultTreeEditable = false;
  // std::cout <<"ResultUpdate()"<<std::endl;
  QStringList labels;
  labels << tr("type") << tr("result") << tr("processing");
  // treeWidget->header()->setResizeMode(QHeaderView::Stretch);
  treeWidget_Results->setHeaderLabels(labels);
  treeWidget_Results->clear();

  if ( protocol.GetImageProtocol().bCheck )
  {
    // ImageInformationCheckResult
    QTreeWidgetItem *itemImageInformation = new QTreeWidgetItem(
      treeWidget_Results);
    itemImageInformation->setText( 0, tr("ImageInformation") );

    QTreeWidgetItem *origin = new QTreeWidgetItem(itemImageInformation);
    origin->setText( 0, tr("origin") );
    if ( qcResult.GetImageInformationCheckResult().origin )
    {
      origin->setText( 1, tr("Pass") );
    }
    else
    {
      origin->setText( 1, tr("Failed") );
    }

    QTreeWidgetItem *sizeLocal = new QTreeWidgetItem(itemImageInformation);
    sizeLocal->setText( 0, tr("size") );
    if ( qcResult.GetImageInformationCheckResult().size )
    {
      sizeLocal->setText( 1, tr("Pass") );
    }
    else
    {
      sizeLocal->setText( 1, tr("Failed") );
    }

    QTreeWidgetItem *space = new QTreeWidgetItem(itemImageInformation);
    space->setText( 0, tr("space") );
    if ( qcResult.GetImageInformationCheckResult().space )
    {
      space->setText( 1, tr("Pass") );
    }
    else
    {
      space->setText( 1, tr("Failed") );
    }

    QTreeWidgetItem *spacedirection = new QTreeWidgetItem(itemImageInformation);
    spacedirection->setText( 0, tr("spacedirection") );
    if ( qcResult.GetImageInformationCheckResult().spacedirection )
    {
      spacedirection->setText( 1, tr("Pass") );
    }
    else
    {
      spacedirection->setText( 1, tr("Failed") );
    }

    QTreeWidgetItem *spacing = new QTreeWidgetItem(itemImageInformation);
    spacing->setText( 0, tr("spacing") );
    if ( qcResult.GetImageInformationCheckResult().spacing )
    {
      spacing->setText( 1, tr("Pass") );
    }
    else
    {
      spacing->setText( 1, tr("Failed") );
    }
  }

  if ( protocol.GetDiffusionProtocol().bCheck )
  {
    // DiffusionInformationCheckResult
    QTreeWidgetItem *itemDiffusionInformation = new QTreeWidgetItem(
      treeWidget_Results);
    itemDiffusionInformation->setText( 0, tr("DiffusionInformation") );

    QTreeWidgetItem *b = new QTreeWidgetItem(itemDiffusionInformation);
    b->setText( 0, tr("b value") );
    if ( qcResult.GetDiffusionInformationCheckResult().b )
    {
      b->setText( 1, tr("Pass") );
    }
    else
    {
      b->setText( 1, tr("Failed") );
    }

    QTreeWidgetItem *gradient = new QTreeWidgetItem(itemDiffusionInformation);
    gradient->setText( 0, tr("gradient") );
    if ( qcResult.GetDiffusionInformationCheckResult().gradient )
    {
      gradient->setText( 1, tr("Pass") );
    }
    else
    {
      gradient->setText( 1, tr("Failed") );
    }

    QTreeWidgetItem *measurementFrame = new QTreeWidgetItem(
      itemDiffusionInformation);
    measurementFrame->setText( 0, tr("measurementFrame") );
    if ( qcResult.GetDiffusionInformationCheckResult().measurementFrame )
    {
      measurementFrame->setText( 1, tr("Pass") );
    }
    else
    {
      measurementFrame->setText( 1, tr("Failed") );
    }
  }

  // itemIntensityMotionInformation
  QTreeWidgetItem *itemIntensityMotionInformation = new QTreeWidgetItem(
    treeWidget_Results);
  itemIntensityMotionInformation->setText( 0, tr("IntensityMotion") );

  for ( unsigned int i = 0;
    i < qcResult.GetIntensityMotionCheckResult().size();
    i++ )
  {
    // gradient
    QTreeWidgetItem *gradient = new QTreeWidgetItem(
      itemIntensityMotionInformation);

    // gradient->setText(0, tr("gradient ")+QString::number(i));
    gradient->setText( 0,
      QString("gradient_%1").arg( i, 4, 10, QLatin1Char( '0' ) ) );

    // std::cout<<"1:ResultUpdate()"<<std::endl;
    switch ( qcResult.GetIntensityMotionCheckResult()[i].processing )
    {
    case QCResult::GRADIENT_BASELINE_AVERAGED:
      gradient->setText( 2, tr("BASELINE_AVERAGED") );
      break;
    case QCResult::GRADIENT_EXCLUDE_SLICECHECK:
      gradient->setText( 2, tr("EXCLUDE_SLICECHECK") );
      break;
    case QCResult::GRADIENT_EXCLUDE_INTERLACECHECK:
      gradient->setText( 2, tr("EXCLUDE_INTERLACECHECK") );
      break;
    case QCResult::GRADIENT_EXCLUDE_GRADIENTCHECK:
      gradient->setText( 2, tr("EXCLUDE_GRADIENTCHECK") );
      break;
    case QCResult::GRADIENT_EXCLUDE_MANUALLY:
      gradient->setText( 2, tr("EXCLUDE") );
      break;
    case QCResult::GRADIENT_EDDY_MOTION_CORRECTED:
      gradient->setText( 2, tr("EDDY_MOTION_CORRECTED") );
      break;
    case QCResult::GRADIENT_INCLUDE:
    default:
      gradient->setText( 2, tr("INCLUDE") );
      break;
    }

    QTreeWidgetItem *itemOriginalGradientDir = new QTreeWidgetItem(gradient);
    itemOriginalGradientDir->setText( 0, tr("OriginalDir") );
    itemOriginalGradientDir->setText(1, QString("%1 %2 %3")
      .arg(qcResult.GetIntensityMotionCheckResult()[i].OriginalDir[0], 0, 'f',
      6)
      .arg(qcResult.GetIntensityMotionCheckResult()[i].OriginalDir[1], 0, 'f',
      6)
      .arg(qcResult.GetIntensityMotionCheckResult()[i].OriginalDir[2], 0, 'f',
      6)
      );

    QTreeWidgetItem *itemReplacedGradientDir = new QTreeWidgetItem(gradient);
    itemReplacedGradientDir->setText( 0, tr("ReplacedDir") );
    itemReplacedGradientDir->setText(1, QString("%1 %2 %3")
      .arg(qcResult.GetIntensityMotionCheckResult()[i].ReplacedDir[0], 0, 'f',
      6)
      .arg(qcResult.GetIntensityMotionCheckResult()[i].ReplacedDir[1], 0, 'f',
      6)
      .arg(qcResult.GetIntensityMotionCheckResult()[i].ReplacedDir[2], 0, 'f',
      6)
      );

    QTreeWidgetItem *itemCorrectedGradientDir = new QTreeWidgetItem(gradient);
    itemCorrectedGradientDir->setText( 0, tr("CorrectedDir") );
    itemCorrectedGradientDir->setText(1, QString("%1 %2 %3")
      .arg(qcResult.GetIntensityMotionCheckResult()[i].CorrectedDir[0], 0,
      'f', 6)
      .arg(qcResult.GetIntensityMotionCheckResult()[i].CorrectedDir[1], 0,
      'f', 6)
      .arg(qcResult.GetIntensityMotionCheckResult()[i].CorrectedDir[2], 0,
      'f', 6)
      );
  }

  bResultTreeEditable = false; // no edit to automatically generated results
  pushButton_SaveDWIAs->setEnabled( 0 );

  emit UpdateOutputDWIDiffusionVectorActors();

  return;
}

void IntensityMotionCheckPanel::GenerateCheckOutputImage( const std::string filename)
{
  if ( !bDwiLoaded  )
  {
    std::cout << "DWI load error, no Gradient Direction Loaded" << std::endl;
    bGetGradientDirections = false;
    return;
  }

  unsigned int gradientLeft = 0;
  for ( unsigned int i = 0;
    i < qcResult.GetIntensityMotionCheckResult().size();
    i++ )
  {
    if ( qcResult.GetIntensityMotionCheckResult()[i].processing ==
      QCResult::GRADIENT_INCLUDE )
    {
      gradientLeft++;
    }
  }

  std::cout << "gradientLeft: " << gradientLeft << std::endl;
  // std::cout <<
  //
  // "1.0-(float)((float)gradientLeft/(float)qcResult.GetGradientProcess().size()):
  // " <<
  //
  // 1.0-(float)((float)gradientLeft/(float)qcResult.GetGradientProcess().size())<<std::endl;
  // std::cout <<
  //
  // "this->protocol.GetIntensityMotionCheckProtocol().badGradientPercentageTolerance:
  // " <<
  //
  // this->protocol.GetIntensityMotionCheckProtocol().badGradientPercentageTolerance<<std::endl;

  if ( bProtocol )
  {
    if ( 1.0
      - (float)( (float)gradientLeft
      / (float)qcResult.GetIntensityMotionCheckResult().size() ) >=
      this->protocol.GetBadGradientPercentageTolerance() )
    {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, tr("Attention"),
        tr(
        "Bad gradients number is greater than that in protocol, save anyway?"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
      if ( reply == QMessageBox::No || reply == QMessageBox::Cancel )
      {
        return;
      }
    }
  }

  if ( gradientLeft == qcResult.GetIntensityMotionCheckResult().size() )
  {
    itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
    try
    {
      DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
      DwiWriter->SetImageIO(myNrrdImageIO);
      DwiWriter->SetFileName( filename );
      DwiWriter->SetInput( this->m_DwiOriginalImage );
      DwiWriter->UseCompressionOn();
      DwiWriter->Update();
    }
    catch ( itk::ExceptionObject & e )
    {
      std::cout << e.GetDescription() << std::endl;
      return;
    }
    return;
  }

  DwiImageType::Pointer newDwiImage = DwiImageType::New();
  newDwiImage->CopyInformation(m_DwiOriginalImage);
  newDwiImage->SetRegions( m_DwiOriginalImage->GetLargestPossibleRegion() );
  newDwiImage->Allocate();
  newDwiImage->SetVectorLength( gradientLeft);

  typedef itk::ImageRegionConstIteratorWithIndex<DwiImageType>
    ConstIteratorType;
  ConstIteratorType oit( m_DwiOriginalImage, m_DwiOriginalImage->GetLargestPossibleRegion() );
  typedef itk::ImageRegionIteratorWithIndex<DwiImageType> IteratorType;
  IteratorType nit( newDwiImage, newDwiImage->GetLargestPossibleRegion() );

  oit.GoToBegin();
  nit.GoToBegin();

  DwiImageType::PixelType value;
  value.SetSize( gradientLeft );

  while ( !oit.IsAtEnd() )
  {
    int element = 0;
    for ( unsigned int i = 0;
      i < qcResult.GetIntensityMotionCheckResult().size();
      i++ )
    {
      if ( qcResult.GetIntensityMotionCheckResult()[i].processing ==
        QCResult::GRADIENT_INCLUDE )
      {
        value.SetElement( element, oit.Get()[i] );
        element++;
      }
    }
    nit.Set(value);
    ++oit;
    ++nit;
  }

  itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
  try
  {
    DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
    DwiWriter->SetImageIO(myNrrdImageIO);
    DwiWriter->SetFileName( filename );
    DwiWriter->SetInput(newDwiImage);
    DwiWriter->UseCompressionOn();
    DwiWriter->Update();
  }
  catch ( itk::ExceptionObject & e )
  {
    std::cout << e.GetDescription() << std::endl;
    return;
  }

  // newDwiImage->Delete();

  itk::MetaDataDictionary imgMetaDictionary
    = m_DwiOriginalImage->GetMetaDataDictionary();                                            //
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string                              metaString;

  char *aryOut = new char[filename.length() + 1];
  strcpy ( aryOut, filename.c_str() );

  std::ofstream header;
  header.open(aryOut, std::ios_base::app);

  //  measurement frame
  if ( imgMetaDictionary.HasKey("NRRD_measurement frame") )
  {

    std::vector<std::vector<double> > nrrdmf;
    itk::ExposeMetaData<std::vector<std::vector<double> > >(
      imgMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);

    // Meausurement frame
    header  << "measurement frame: ("
      << nrrdmf[0][0] << ","
      << nrrdmf[0][1] << ","
      << nrrdmf[0][2] << ") ("
      << nrrdmf[1][0] << ","
      << nrrdmf[1][1] << ","
      << nrrdmf[1][2] << ") ("
      << nrrdmf[2][0] << ","
      << nrrdmf[2][1] << ","
      << nrrdmf[2][2] << ")"
      << std::endl;
  }

  for ( itKey = imgMetaKeys.begin(); itKey != imgMetaKeys.end(); itKey++ )
  {
    itk::ExposeMetaData(imgMetaDictionary, *itKey, metaString);
    const int posFind = itKey->find("modality");
    if ( posFind == -1 )
    {
      continue;
    }
    // std::cout  << metaString << std::endl;
    header << "modality:=" << metaString << std::endl;
  }

  GetGradientDirections();

  for ( itKey = imgMetaKeys.begin(); itKey != imgMetaKeys.end(); itKey++ )
  {
    itk::ExposeMetaData(imgMetaDictionary, *itKey, metaString);
    const int posFind = itKey->find("DWMRI_b-value");
    if ( posFind == -1 )
    {
      continue;
    }

    // std::cout  << metaString << std::endl;
    header << "DWMRI_b-value:=" << metaString << std::endl;
  }

  int newGradientNumber = 0;
  for ( unsigned int i = 0; i < GradientDirectionContainer->size(); i++ )
  {
    if ( qcResult.GetIntensityMotionCheckResult()[i].processing ==
      QCResult::GRADIENT_INCLUDE )
    {
      header  << "DWMRI_gradient_" << std::setw(4) << std::setfill('0')
        << newGradientNumber << ":="
        << GradientDirectionContainer->ElementAt(i)[0] << "   "
        << GradientDirectionContainer->ElementAt(i)[1] << "   "
        << GradientDirectionContainer->ElementAt(i)[2] << std::endl;
      ++newGradientNumber;
    }
  }

  header.flush();
  header.close();

  std::cout << " QC Image saved " << std::endl;
}

bool IntensityMotionCheckPanel::GetGradientDirections()
{
  if ( !bDwiLoaded )
  {
    LoadDwiImage();
  }
  if ( !bDwiLoaded )
  {
    std::cout << "DWI load error, no Gradient Direction Loaded" << std::endl;
    bGetGradientDirections = false;
    return false;
  }

  itk::MetaDataDictionary imgMetaDictionary
    = m_DwiOriginalImage->GetMetaDataDictionary();                                            //
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string                              metaString;

  // int numberOfImages=0;
  TensorReconstructionImageFilterType::GradientDirectionType vect3d;

  GradientDirectionContainer->clear();

  for (; itKey != imgMetaKeys.end(); itKey++ )
  {
    // double x,y,z;
    itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
    if ( itKey->find("DWMRI_gradient") != std::string::npos )
    {
      std::istringstream iss(metaString);
      iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
      // sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
      // vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
      GradientDirectionContainer->push_back(vect3d);
    }
    else if ( itKey->find("DWMRI_b-value") != std::string::npos )
    {
      readb0 = true;
      b0 = atof( metaString.c_str() );
      // std::cout<<"b Value: "<<b0<<std::endl;
    }
  }

  if ( !readb0 )
  {
    std::cout << "BValue not specified in header file" << std::endl;
    return false;
  }
  if ( GradientDirectionContainer->size() <= 6 )
  {
    std::cout << "Gradient Images Less than 7" << std::endl;
    bGetGradientDirections = false;
    return false;
  }

  std::cout << "b Value: " << b0 << std::endl;
  std::cout << "DWI image gradient count: " << m_DwiOriginalImage->GetVectorLength()
    << std::endl;

  for ( unsigned int i = 0; i < m_DwiOriginalImage->GetVectorLength(); i++ ) //
    // GradientDirectionContainer->Size()
  {
    //    std::cout<<"Gradient Direction "<<i<<": \t[";
    //    std::cout<<GradientDirectionContainer->at(i)[0]<<",\t";
    //    std::cout<<GradientDirectionContainer->at(i)[1]<<",\t";
    //    std::cout<<GradientDirectionContainer->at(i)[2]<<" ]"<<std::endl;
  }

  bGetGradientDirections = true;
  return true;
}

void IntensityMotionCheckPanel::on_pushButton_SaveDWIAs_clicked( )
{
  QString DWIFile = QFileDialog::getSaveFileName( this, tr(
    "Save Protocol As"), QString::fromStdString(DwiFileName),
    tr("nrrd Files (*.nhdr)") );

  if ( DWIFile.length() > 0 )
  {
    std::cout << "Save DWI into file: " << DWIFile.toStdString() << std::endl;
    this->GenerateCheckOutputImage( DWIFile.toStdString() );
  }
  else
  {
    std::cout << "DWI file name NOT set" << std::endl;
  }
}

void IntensityMotionCheckPanel::on_pushButton_DefaultQCResult_clicked( )
{
  if ( m_DwiOriginalImage->GetVectorLength() != GradientDirectionContainer->size() )
  {
    std::cout
      << "Bad DWI: mismatch between gradient image # and gradient vector #"
      << std::endl;
    QMessageBox::critical( this, tr("BAD DWI !"),
      tr("Bad DWI: mismatch between gradient image # and gradient vector # !") );
    return;
  }

  DefaultProcess( );
  ResultUpdate();

  bResultTreeEditable = true;
  pushButton_SaveDWIAs->setEnabled( 1 );

  emit UpdateOutputDWIDiffusionVectorActors();
}

void IntensityMotionCheckPanel::DefaultProcess( )
{
  this->qcResult.Clear();

  GradientIntensityMotionCheckResult IntensityMotionCheckResult;
  IntensityMotionCheckResult.processing = QCResult::GRADIENT_INCLUDE;

  for ( unsigned int i = 0; i < this->m_DwiOriginalImage->GetVectorLength(); i++ )
  {
    IntensityMotionCheckResult.OriginalDir[0]
    = this->GradientDirectionContainer->ElementAt(i)[0];
    IntensityMotionCheckResult.OriginalDir[1]
    = this->GradientDirectionContainer->ElementAt(i)[1];
    IntensityMotionCheckResult.OriginalDir[2]
    = this->GradientDirectionContainer->ElementAt(i)[2];

    IntensityMotionCheckResult.ReplacedDir[0]
    = this->GradientDirectionContainer->ElementAt(i)[0];
    IntensityMotionCheckResult.ReplacedDir[1]
    = this->GradientDirectionContainer->ElementAt(i)[1];
    IntensityMotionCheckResult.ReplacedDir[2]
    = this->GradientDirectionContainer->ElementAt(i)[2];

    IntensityMotionCheckResult.CorrectedDir[0]
    = this->GradientDirectionContainer->ElementAt(i)[0];
    IntensityMotionCheckResult.CorrectedDir[1]
    = this->GradientDirectionContainer->ElementAt(i)[1];
    IntensityMotionCheckResult.CorrectedDir[2]
    = this->GradientDirectionContainer->ElementAt(i)[2];

    qcResult.GetIntensityMotionCheckResult().push_back(
      IntensityMotionCheckResult);
  }

  protocol.clear();
  protocol.GetImageProtocol().     bCheck = true;
  protocol.GetDiffusionProtocol(). bCheck = true;

  protocol.GetSliceCheckProtocol().           bCheck = true;
  protocol.GetInterlaceCheckProtocol().       bCheck = true;
  protocol.GetGradientCheckProtocol().        bCheck = true;
  protocol.GetBaselineAverageProtocol().      bAverage = true;
  protocol.GetEddyMotionCorrectionProtocol(). bCorrect = true;
}
