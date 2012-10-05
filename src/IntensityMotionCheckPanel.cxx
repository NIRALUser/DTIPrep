#include <QtGui>
#include <QThread>
#include <QFont>
#include <qdom.h>

#include "IntensityMotionCheckPanel.h"
// #include "IntensityMotionCheck.h"
// #include "ThreadIntensityMotionCheck.h"

#include "itkMetaDataDictionary.h"
#include "itkNrrdImageIO.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include <itksys/SystemTools.hxx>

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

// Defining Checking bits
#define ImageCheckBit 1
#define DiffusionCheckBit 2
#define SliceWiseCheckBit 4
#define InterlaceWiseCheckBit 8
#define GradientWiseCheckBit 16
#define BrainMaskBit 32
#define DominantDirectionDetectBit 64

IntensityMotionCheckPanel::IntensityMotionCheckPanel(QMainWindow *parentNew) :
  QDockWidget(parentNew)
{
  setupUi(this);
  verticalLayout->setContentsMargins(0, 0, 0, 0);
  qRegisterMetaType<QString>();
  // Setting max and min to zero to behave as a busy indicator
  this->progressBar2->setMinimum(0);
  this->progressBar2->setMaximum(0);
  this->progressBar2->hide(); // because we only want to show the progressBar when a connection is activated
  connect(&myIntensityThread, SIGNAL(StartProgressSignal() ), this, SLOT(StartProgressSlot() ), Qt::QueuedConnection);
  connect(&myIntensityThread, SIGNAL(StopProgressSignal() ), this, SLOT(StopProgressSlot() ), Qt::QueuedConnection);

  // Setting max and min to zero to behave as a busy indicator
  this->f_progressBar->setMinimum(0);
  this->f_progressBar->setMaximum(0);
  this->f_progressBar->hide(); // because we only want to show the progressBar when a connection is activated
  connect(&myFurtherQCThread, SIGNAL(f_StartProgressSignal() ), this, SLOT(
            f_StartProgressSlot() ), Qt::QueuedConnection);
  connect(&myFurtherQCThread, SIGNAL(f_StopProgressSignal() ), this, SLOT(f_StopProgressSlot() ), Qt::QueuedConnection);

  m_DwiOriginalImage = NULL;
  protocol.clear();
  bDwiLoaded = false;
  bDwi_VisualCheckLoad = false;
  bProtocol = false;

  bCancel_QC = false;

  bLoadDefaultQC = false;

  bProtocolTreeEditable = false;
  pushButton_DefaultProtocol->setEnabled( 0 );
  pushButton_Save->setEnabled( 0 );
  pushButton_SaveProtocolAs->setEnabled( 0 );

  bResultTreeEditable = false;
  //   pushButton_SaveDWI->setEnabled( 0 );
  //   pushButton_SaveQCReport->setEnabled( 0 );
  //   pushButton_SaveQCReportAs->setEnabled( 0 );

  pushButton_SaveVisualChecking->setEnabled( 1 );
  // pushButton_SaveDWIAs->setEnabled( 0 );
  // pushButton_CreateDefaultProtocol->setEnabled( 0 );

  pushButton_RunPipeline->setEnabled( 0 );

  pushButton_DefaultQCResult->setEnabled( 0 );

  //   pushButton_OpenQCReport->setEnabled( 0 );

  QStringList labels;
  labels << tr("Parameter") << tr("Value");
  // treeWidget->header()->setResizeMode(QHeaderView::Stretch);
  treeWidget->setHeaderLabels(labels);

  // QStringList labels_Result;
  // labels_Result << tr("Type") << tr("Result") << tr("Processing");  //Lables of Widget of QCResults tab
  // treeWidget_Results->setHeaderlables(labels_Results);

  bGetGradientDirections = false;

  GradientDirectionContainer = GradientDirectionContainerType::New();

  connect( &myIntensityThread,
           SIGNAL( ResultUpdate() ),
           this,
           SLOT( ResultUpdate() ) );

  connect( &myFurtherQCThread,
           SIGNAL( ResultUpdate() ),
           this,
           SLOT( ResultUpdate() ) );

  connect( &myFurtherQCThread,
           SIGNAL( ResultUpdate() ),
           this,
           SLOT( ResultUpdate() ) );

  connect( &myIntensityThread,
           SIGNAL( Building_Mapping_XML() ),
           this,
           SLOT( Building_Mapping_XML() ) );

  connect( &myIntensityThread,
           SIGNAL( LoadQCedDWI( QString) ),
           this,
           SLOT( LoadQCedDWI( QString ) ) );

  connect( &myIntensityThread,
           SIGNAL( QCedResultUpdate() ),
           this,
           SLOT( QCedResultUpdate() ) );

  connect( &myIntensityThread,
           SIGNAL( Set_VCStatus() ),
           this,
           SLOT( Set_VCStatus() ) );

  connect( &myIntensityThread,
           SIGNAL( Set_Original_ForcedConformance_Mapping() ),
           this,
           SLOT( Set_Original_ForcedConformance_Mapping() ) );

  connect( &myIntensityThread,
           SIGNAL( Set_QCedDWI() ),
           this,
           SLOT( Set_QCedDWI() ) );

  connect( this, SIGNAL( Set_init_Path_Signal() ), this, SLOT( Set_init_Path() ) );

}

IntensityMotionCheckPanel::~IntensityMotionCheckPanel()
{
}

void IntensityMotionCheckPanel::StartProgressSlot()
{
  this->progressBar2->show();    // To show progressBar when StartProgressSignal emitted
}

void IntensityMotionCheckPanel::StopProgressSlot()
{
  this->progressBar2->hide();    // To hide progressBar when StopProgressSignal emitted
}

void IntensityMotionCheckPanel::f_StartProgressSlot()
{
  this->f_progressBar->show();    // To show progressBar when f_StartProgressSignal emitted
}

void IntensityMotionCheckPanel::f_StopProgressSlot()
{
  this->f_progressBar->hide();    // To hide progressBar when f_StopProgressSignal emitted
}

void IntensityMotionCheckPanel::on_treeWidget_DiffusionInformation_itemClicked(
  QTreeWidgetItem *item,
  int /* column */)
{
  std::string str = item->text(0).toStdString();

  if( str.find("gradient") != std::string::npos )
    {
    emit currentGradient( 0, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    emit currentGradient( 1, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    emit currentGradient( 2, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    }
}

void IntensityMotionCheckPanel::on_treeWidget_DiffusionInformation_currentItemChanged(
  QTreeWidgetItem *current,
  QTreeWidgetItem * /* previous */)
{
  std::string str = current->text(0).toStdString();

  if( str.find("gradient") != std::string::npos )
    {
    emit currentGradient( 0, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    emit currentGradient( 1, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    emit currentGradient( 2, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    }
}

void IntensityMotionCheckPanel::on_treeWidget_Results_itemDoubleClicked(
  QTreeWidgetItem *item,
  int /* column */)
{
  if( item == NULL )
    {
    return;
    }

  if( item->text(0).left(9) == tr("gradient_") )
    {
    std::string str = item->text(0).toStdString();
    emit        currentGradient( 0, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    emit        currentGradient( 1, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    emit        currentGradient( 2, atoi( str.substr(str.length() - 4, 4).c_str() ) );
    }

  if( item->text(0).left(10) == tr("VC_Status_") )
    {
    std::string str = item->text(0).toStdString();
    emit        currentGradientChanged_VC( atoi( str.substr(str.length() - 4, 4).c_str() ) );
    }

}

void IntensityMotionCheckPanel::on_treeWidget_Results_currentItemChanged(
  QTreeWidgetItem *current,
  QTreeWidgetItem *previous)
{
  treeWidget_Results->closePersistentEditor(previous, 2); // does nothing if
  // none open
  if( current == NULL )
    {
    return;
    }

  std::string str = current->text(0).toStdString();
  // if ( str.find("gradient") != std::string::npos )
  // {
  //   emit currentGradient( 0, atoi( str.substr(str.length() - 4, 4).c_str() ) );
  //   emit currentGradient( 1, atoi( str.substr(str.length() - 4, 4).c_str() ) );
  //   emit currentGradient( 2, atoi( str.substr(str.length() - 4, 4).c_str() ) );
  // }
}

void IntensityMotionCheckPanel::on_treeWidget_itemDoubleClicked(
  QTreeWidgetItem *item,
  int col)
{

  if( col == 1 && bProtocolTreeEditable )
    {
    treeWidget->openPersistentEditor(item, col);
    }

  if( item == NULL )
    {
    return;
    }

  if( item->text(0).left(22) == tr("BRAINMASK_method") )
    {
    std::string str = item->text(0).toStdString();

    QString str_brainmask = QString("Please select brain masking approach." );
    QString str_brainmask2 = QString("Please select path." );
    QString str_brainmask3 = QString("No path was selected." );

    QMessageBox msgBox;
    QMessageBox msgBox2;
    QMessageBox msgBox3;

    msgBox.setText( str_brainmask );
    QPushButton * FSL = msgBox.addButton( tr("FSL_bet"), QMessageBox::ActionRole);
    QPushButton * Slicer = msgBox.addButton( tr("Slicer"), QMessageBox::ActionRole);
    QPushButton * Option = msgBox.addButton( tr("User mask"), QMessageBox::ActionRole);
    QPushButton * Cancel = msgBox.addButton( tr("Cancel"), QMessageBox::ActionRole);
    msgBox.exec();

    if( msgBox.clickedButton() == FSL )
      {
      std::cout << "Protocol Tree child 14 " << std::endl;
      this->GetProtocol().GetBrainMaskProtocol().BrainMask_Method = 0;
      // std::cout << "Protocol Tree child 14 " <<
      // this->GetTreeWidgetProtocol()->topLevelItem(14)->child(1)->text(1).toStdString().c_str() << std::endl;

      // Set BrainMask parameters in the protocol
      this->GetTreeWidgetProtocol()->topLevelItem(14)->child(0)->setText( 1,
                                                                          QString("%1").arg(this->GetProtocol().
                                                                                            GetBrainMaskProtocol().
                                                                                            BrainMask_Method, 0, 10) );

      // msgBox2.setText( str_brainmask2 );
      // msgBox2.exec();

      // Finding systempath and running the brain masking method
      this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_FSL = lineEdit_FSL->text().toStdString();

      if( lineEdit_FSL->text() == NULL )
        {
        msgBox3.setText( str_brainmask3 );
        msgBox3.exec();
        std::cerr << "No system path was selected. " << std::endl;
        return;
        }

     

      }
    if( msgBox.clickedButton() == Slicer )
      {
      this->GetProtocol().GetBrainMaskProtocol().BrainMask_Method = 1;
      this->GetTreeWidgetProtocol()->topLevelItem(14)->child(0)->setText( 1,
                                                                          QString("%1").arg(this->GetProtocol().
                                                                                            GetBrainMaskProtocol().
                                                                                            BrainMask_Method, 0, 10) );
      std::cout << "brainMask Test New:" << "Slicer" << std::endl;
      }
    if( msgBox.clickedButton() == Option )
      {
      this->GetProtocol().GetBrainMaskProtocol().BrainMask_Method = 2;
      this->GetTreeWidgetProtocol()->topLevelItem(14)->child(0)->setText( 1,
                                                                          QString("%1").arg(this->GetProtocol().
                                                                                            GetBrainMaskProtocol().
                                                                                            BrainMask_Method, 0, 10) );
      std::cout << "brainMask Test New:" << "Option" << std::endl;

      /*QString Mask_image = QFileDialog::getOpenFileName( this, tr(
                                                           "Open nrrd masked brain image"), QDir::currentPath(),
                                                         tr("Nrrd Files (*.nhdr *.nrrd)") );

      /if( Mask_image.length() > 0 )
        {
        this->GetProtocol().GetBrainMaskProtocol().BrainMask_Image = Mask_image.toStdString();
        std::cout << "brain mask Option " << this->GetProtocol().GetBrainMaskProtocol().BrainMask_Image << std::endl;
        }
      else
        {
        std::cout << "No masked brain is selected." << std::endl;
        return;
        }
		*/
      }
    if( msgBox.clickedButton() == Cancel )
      {
      std::cout << "brainMask Test New:" << "Cancel" << std::endl;
      return;
      }

    }
}

/*void IntensityMotionCheckPanel::on_treeWidget_Results_itemDoubleClicked(
  QTreeWidgetItem *item,
  int column)
{
  if ( bResultTreeEditable && column == 2 && item->text(0).left(8) ==
    tr("gradient") )
  {
    treeWidget_Results->openPersistentEditor(item, column);
  }
}*/

void IntensityMotionCheckPanel::on_treeWidget_Results_itemChanged(
  QTreeWidgetItem *item,
  int /* column */)
{
  if( bResultTreeEditable )
    {
    if( item->text(2).toLower() == tr("exclude") )
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

void IntensityMotionCheckPanel::on_pushButton_RunPipeline_clicked()
{
  // CIntensityMotionCheck
  // IntensityMotionCheck(lineEdit_->text().toStdString());
  // IntensityMotionCheck.SetProtocol(&protocol);
  // IntensityMotionCheck.SetQCResult(&qcResult);
  // IntensityMotionCheck.GetImagesInformation();
  // IntensityMotionCheck.CheckByProtocol();

  bLoadDefaultQC = false;
  if( m_DwiOriginalImage->GetVectorLength() != GradientDirectionContainer->size() )
    {
    std::cout
    << "Bad DWI: mismatch between gradient image #and gradient vector #"
    << std::endl;
    QMessageBox::critical( this, tr("BAD DWI !"),
                           tr("Bad DWI: mismatch between gradient image #and gradient vector #!") );
    return;
    }

  if( !bProtocol )
    {
    std::cout << "Protocol NOT set. Load prorocol file first!" << std::endl;
    return;
    }

  if( DwiFileName.length() == 0 )
    {
    std::cout << "DWI file name not set!" << std::endl;
    QMessageBox::critical( this, tr("Warning"), tr("DWI file name not set!") );
    return;
    }
  
  if (this->GetProtocol().GetBrainMaskProtocol().BrainMask_Method == 2 && this->GetProtocol().GetBrainMaskProtocol().BrainMask_Image.empty() && this->GetProtocol().GetBrainMaskProtocol().bMask == true)
  {
	  QString m_user_option = "The brain mask procedure needs brain mask image is loaded:";
	  QMessageBox m_msgBox;
	  m_msgBox.setText( m_user_option );
	  QPushButton * Option = m_msgBox.addButton( tr("Load"), QMessageBox::ActionRole);
	  QPushButton * Cancel = m_msgBox.addButton( tr("Cancel"), QMessageBox::ActionRole);
	  m_msgBox.exec();
	  
	  if( m_msgBox.clickedButton() == Option )
	  {
	      
	      QString Mask_image = QFileDialog::getOpenFileName( this, tr(
	                                                             "Open nrrd masked brain image"), QDir::currentPath(),
	                                                           tr("Nrrd Files (*.nhdr *.nrrd)") );

	      if( Mask_image.length() > 0 )
	      {
	          this->GetProtocol().GetBrainMaskProtocol().BrainMask_Image = Mask_image.toStdString();
	          //std::cout << "brain mask Option " << this->GetProtocol().GetBrainMaskProtocol().BrainMask_Image << std::endl;
	      }
	      else
	      {
	          std::cout << "No masked brain is selected." << std::endl;
	          return;
	      }

	   }
	   if( m_msgBox.clickedButton() == Cancel )
	   {
	        std::cout << "brainMask Test New:" << "Cancel" << std::endl;
	        return;
	   }

  }

  treeWidget_Results->clear();
  qcResult.Clear();
  // "ThreadIntensityMotionCheck->SetFileName(lineEdit_DWIFileName->text().toStdString());"<<std::endl;
  myIntensityThread.SetDwiFileName(DwiFileName);
  myIntensityThread.SetXmlFileName(lineEdit_Protocol->text().toStdString() );
  myIntensityThread.SetProtocol( &protocol);
  myIntensityThread.SetQCResult(&qcResult);
  myIntensityThread.start();
  result = qcResult.Get_result();
  // ResultUpdate();
  printf( "result from Runpipeline bottom = %d", result);

  bResultTreeEditable = false;
  // pushButton_SaveDWIAs->setEnabled( 0 );

}

void IntensityMotionCheckPanel::SetFileName(QString nrrd )
{
  // lineEdit_DWIFileName->setText(nrrd);
  DwiFileName = nrrd.toStdString();
}

void IntensityMotionCheckPanel::SetName( QString nrrd_path )
{
  DwiFilePath = nrrd_path;
  DwiName = nrrd_path.section('/', -1); // set only dwi file name to DwiName
}

void IntensityMotionCheckPanel::on_toolButton_ProtocolFileOpen_clicked()
{
  OpenXML();

  bProtocolTreeEditable = true;
  emit ProtocolChanged();
}

void IntensityMotionCheckPanel::on_toolButton_ResultFileOpen_clicked()
{
  pushButton_SaveVisualChecking->setEnabled( 1 );
  bMatchNameQCResult_DwiFile = false;
  OpenXML_ResultFile();
  emit SignalActivateSphere(); // Activate the "actionIncluded" bottom

  // bProtocolTreeEditable = true;
  // emit ProtocolChanged();
}

void IntensityMotionCheckPanel::SetVisualCheckingStatus( int index, int local_status )
{

  // Set Visual Status with the assumption that user is not able to exclude Baseline
  const int pro =
    this->GetQCResult().GetIntensityMotionCheckResult()[t_Original_ForcedConformance_Mapping[index].index_original[0]].
    processing;

  // std::cout << "index: " << index << "pro: " << pro << std::endl;
  for( unsigned int j = 0; j < VC_Status.size(); j++ )
    {
    if( index == VC_Status[j].index )
      {
      VC_Status[j].VC_status = local_status;

      }

    }

  if( local_status == 0 )
    {
    this->GetQTreeWidgetResult()->topLevelItem(0)->child( index )->setText( 1, tr("INCLUDE_MANUALLY") );
    this->GetQTreeWidgetResult()->topLevelItem(0)->child( index )->child( 1 )->child( 0 )->setText( 1, tr("Include") );
    this->GetQCResult().GetIntensityMotionCheckResult()[t_Original_ForcedConformance_Mapping[index].index_original[0]].
    VisualChecking = 0;
    }
  if( local_status == 6 )
    {
    this->GetQTreeWidgetResult()->topLevelItem(0)->child( index )->setText(1, tr("EXCLUDE_MANUALLY") );
    this->GetQTreeWidgetResult()->topLevelItem(0)->child( index )->child( 1 )->child( 0 )->setText( 1, tr("Exclude") );
    this->GetQCResult().GetIntensityMotionCheckResult()[t_Original_ForcedConformance_Mapping[index].index_original[0]].
    VisualChecking = 6;
    }
  if( local_status == -1 )
    {
    this->GetQTreeWidgetResult()->topLevelItem(0)->child( index )->child( 1 )->child( 0 )->setText( 1, tr("NoChange") );
    this->GetQTreeWidgetResult()->topLevelItem(0)->child( index )->setText(1, tr("") );
    this->GetQCResult().GetIntensityMotionCheckResult()[t_Original_ForcedConformance_Mapping[index].index_original[0]].
    VisualChecking = -1;
    }

  if( pro <= 2 && local_status >= 3 )
    {

    pushButton_SaveVisualChecking->setEnabled( 1 );
    }

  if( pro >= 3 && local_status <= 2 && local_status > -1 )
    {

    pushButton_SaveVisualChecking->setEnabled( 1 );
    }
}

void IntensityMotionCheckPanel::OpenXML_ResultFile()
{

  QString xmlResultFile =
    QFileDialog::getOpenFileName(this, tr("Select QC Report Result"), lineEdit_Result->text(), tr("xml Files (*.xml)") );

  if( xmlResultFile.length() > 0 )
    {
    lineEdit_Result->setText(xmlResultFile);
    }
  else
    {
    return;
    }

  // if ( !(xmlResultFile.contains("_XMLQCResult.xml", Qt::CaseSensitive) ) )	// *  * Should be changed so that makes
  // cerr if protocol xml file
  // {
  // std::cerr << "No QCed XML File Loaded." << std::endl;
  // return;
  // }

  treeWidget_Results->clear();

  qcResult.Clear();

  XmlStreamReader XmlReader(treeWidget_Results);

  XmlReader.setQCRESULT( &qcResult);

  // XmlReader.readFile_QCResult(xmlResultFile, XmlStreamReader::TreeWise);
  XmlReader.readFile_QCResult(xmlResultFile, XmlStreamReader::QCResultlWise);

  // std::cout << " this->GetQCResult().GetIntensityMotionCheckResult().size()" <<
  // this->GetQCResult().GetIntensityMotionCheckResult().size() << std::endl;

  //
  //
  // ..........................................................................................................................................
  // Check weather user wants to visualize the entire QCed result or only passed result
  //
  // ...........................................................................................................................................
  QString     Grad1 = "Warning!";
  QString     Grad2 = "Do you want to do visual checking?";
  QMessageBox msgBox;
  msgBox.setWindowTitle( Grad1 );
  msgBox.setText( Grad2 );
  QPushButton * Passed_QCedResult = msgBox.addButton( tr("Yes"), QMessageBox::ActionRole);
  QPushButton * Cancel = msgBox.addButton( tr("No"), QMessageBox::ActionRole);

  msgBox.exec();

  if( msgBox.clickedButton() == Cancel )
    {
    emit UpdateOutputDWIDiffusionVectorActors();
    emit LoadQCResult(true);

    emit SignalActivateSphere(); // Activate "actionIncluded" bottom
    return;
    }

  if( msgBox.clickedButton() == Passed_QCedResult )
    {
    //
    // ............................................................................................................................................
    // loading Original_ForcedConformance_Mapping
    //
    // ............................................................................................................................................
    if( !bDwiLoaded )
      {
      std::cerr << "No DWI Image Loaded for Visual Checking. Please Load QCed Image. " << std::endl;
      return;
      }
    t_Original_ForcedConformance_Mapping.clear();
    Set_Original_ForcedConformance_Mapping();

    // std::cout << "t_Original_ForcedConformance_Mapping" << t_Original_ForcedConformance_Mapping.size() << std::endl;

    //
    // ............................................................................................................................................
    // loading VC_Status
    //
    // ............................................................................................................................................
    Clear_VC_Status();
    // std::cout << "t_Original_ForcedConformance_Mapping.size()" << t_Original_ForcedConformance_Mapping.size() <<
    // std::endl;
    for( unsigned int ind = 0; ind < t_Original_ForcedConformance_Mapping.size(); ind++  )
      {
      VC_STATUS vc;
      vc.index = ind;
      std::cout << "t_Original_ForcedConformance_Mapping[ind].index_original[0]"
                << t_Original_ForcedConformance_Mapping[ind].index_original[0] << " "
                << "t_Original_ForcedConformance_Mapping[ind].index_ForcedConformance"
                << t_Original_ForcedConformance_Mapping[ind].index_ForcedConformance << std::endl;
      if( ind == 0 )
        {
        for( unsigned int k = 0; k < t_Original_ForcedConformance_Mapping[0].index_original.size(); k++ )
          {
          vc.VC_status =
            this->GetQCResult().GetIntensityMotionCheckResult()[t_Original_ForcedConformance_Mapping[ind].
                                                                index_original[k
                                                                ]].VisualChecking;
          }
        }

      else
        {
        vc.VC_status =
          this->GetQCResult().GetIntensityMotionCheckResult()[t_Original_ForcedConformance_Mapping[ind].index_original[
                                                                0]]
          .VisualChecking;
        }
      VC_Status.push_back( vc );
      // std::cout << "VC_Status" << ind  << this->GetQCResult().GetIntensityMotionCheckResult()[
      // t_Original_ForcedConformance_Mapping[ind].index_original[0] ].VisualChecking << std::endl;

      }

    QCedResultUpdate();

    emit UpdateOutputDWIDiffusionVectorActors();
    emit LoadQCResult(true);

    emit SignalActivateSphere(); // Activate "actionIncluded" bottom

    return;
    }
  // std::cout<<qcResult.GetSliceWiseCheckResult()[1].GradientNum<<"GradientNum"<<std::endl;
  // std::cout<<qcResult.GetSliceWiseCheckResult()[1].SliceNum<<"SliceNum"<<std::endl;
  // std::cout<<qcResult.GetSliceWiseCheckResult()[1].Correlation<<"Correlation"<<std::endl;
  // std::cout<<qcResult.GetSliceWiseCheckProcessing()[50]<<"GradientWiseCheck"<<std::endl;

  // HACK Mahshid: Unused code
  emit UpdateOutputDWIDiffusionVectorActors();

  emit LoadQCResult(true);

  if( bDwiLoaded )
    {
    Match_DwiQC();   // Checking matching between Dwi file and proper QCResult file
    if( bMatch_DwiQC == false )
      {
      Match_NameDwiQC(); // Checking matching names between Dwi file and QCResult information
      }
    bMatchNameQCResult_DwiFile = true;
    }
  //

}

void IntensityMotionCheckPanel::Match_DwiQC()
{
  // Checking whether the number of gradients of Dwi file and QCResult are same
  bMatch_DwiQC = false;
  if( this->GetQCResult().GetIntensityMotionCheckResult().size() != GradientDirectionContainer->size() )
    {
    bMatch_DwiQC = true;
    QString     Grad1 = QString( "IMPORTANT ERROR" );
    QString     Grad2 = QString( "The dwi file and QCResult have different number of gradients" );
    QMessageBox msgBox;
    msgBox.setWindowTitle( Grad1 );
    msgBox.setText( Grad2 );
    QPushButton * Ok = msgBox.addButton( tr("Ok"), QMessageBox::ActionRole);
    QPushButton * LoadNewQC = msgBox.addButton( tr("Load new QCResult"), QMessageBox::ActionRole);

    msgBox.exec();

    if( msgBox.clickedButton() == Ok )
      {
      // treeWidget_Results->clear();
      // qcResult.Clear();
      emit LoadQCResult(true);
      // bCancel_QC = true;
      return;
      }
    if( msgBox.clickedButton() == LoadNewQC )
      {
      OpenXML_ResultFile();
      }
    }

}

void IntensityMotionCheckPanel::Match_NameDwiQC()
{
  // checking whether name loaded dwi file is matched with QCReport
  if( bMatchNameQCResult_DwiFile == false && DwiName.toStdString() !=
      this->GetQTreeWidgetResult()->topLevelItem(0)->child(0)->text( 1 ).toStdString() )
    {
    QString     Grad1 = QString( "WARNING" );
    QString     Grad2 = QString( "The Dwi file name is not matched with the QCResult information" );
    QMessageBox msgBox;
    msgBox.setWindowTitle( Grad1 );
    msgBox.setText( Grad2 );
    QPushButton * Ok = msgBox.addButton( tr("Ok"), QMessageBox::ActionRole);
    QPushButton * LoadNewQC = msgBox.addButton( tr("Load new QCResult"), QMessageBox::ActionRole);
    QPushButton * LoadDwi = msgBox.addButton( tr("Load new Dwi"), QMessageBox::ActionRole);

    msgBox.exec();

    if( msgBox.clickedButton() == Ok )
      {
      emit LoadQCResult(true);
      return;
      }
    if( msgBox.clickedButton() == LoadNewQC )
      {
      on_toolButton_ResultFileOpen_clicked();
      // emit SignalRemoveDwiFile();
      }
    if( msgBox.clickedButton() == LoadDwi )
      {
      emit SignalLoadDwiFile();
      }
    }

}

void IntensityMotionCheckPanel::OpenXML()
{
  QString xmlFile = QFileDialog::getOpenFileName( this, tr(
                                                    "Select Protocol"), lineEdit_Protocol->text(),
                                                  tr("xml Files (*.xml)") );

  if( xmlFile.length() > 0 )
    {
    lineEdit_Protocol->setText(xmlFile);
    }
  else
    {
    return;
    }

  // if ( (xmlFile.contains("_XMLQCResult.xml", Qt::CaseSensitive) ) )		// *  * Should be changed so that makes cerr
  // if protocol xml file
  // {
  // std::cerr << "No Protocol File Loaded." << std::endl;
  // return;
  // }

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

  if( DwiFileName.length() == 0 )
    {
    std::cout << "Dwi file name not set" << std::endl;
    bDwiLoaded = false;
    return false;
    }
  else
    {
    itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
    DwiReaderType::Pointer    DwiReader;
    DwiReader = DwiReaderType::New();
    try
      {
      DwiReader->SetImageIO(myNrrdImageIO);
      DwiReader->SetFileName(DwiFileName);
      std::cout << "Loading in IntensityMotionCheckPanel:" << DwiFileName
                << " ... ";
      DwiReader->Update();
      }
    catch( itk::ExceptionObject & e )
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
  if( !bDwiLoaded )
    {
    LoadDwiImage();
    }
  if( !bDwiLoaded )
    {
    std::cout << "DWI load error, no Gradient Direction Loaded" << std::endl;
    bGetGradientDirections = false;
    return false;
    }

  itk::MetaDataDictionary imgMetaDictionary
    = this->m_DwiOriginalImage->GetMetaDataDictionary();
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string                              metaString;

  // int numberOfImages=0;
  TensorReconstructionImageFilterType::GradientDirectionType vect3d;

  GradientDirectionContainer->clear();
  for( ; itKey != imgMetaKeys.end(); itKey++ )
    {
    // double x,y,z;
    itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
    if( itKey->find("DWMRI_gradient") != std::string::npos )
      {
      std::istringstream iss(metaString);
      iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
      // sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
      // vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
      GradientDirectionContainer->push_back(vect3d);
      }
    else if( itKey->find("DWMRI_b-value") != std::string::npos )
      {
      readb0 = true;
      b0 = atof( metaString.c_str() );
      // std::cout<<"b Value: "<<b0<<std::endl;
      }
    }

  if( !readb0 )
    {
    std::cout << "BValue not specified in header file" << std::endl;
    return false;
    }
  if( GradientDirectionContainer->Size() <= 6 )
    {
    std::cout << "Gradient Images Less than 7" << std::endl;
    bGetGradientDirections = false;
    return false;
    }

  if( bDisplay )
    {
    std::cout << "b Value: " << b0 << std::endl;
    std::cout << "DWI image gradient count: " << m_DwiOriginalImage->GetVectorLength()
              << std::endl;
    for( unsigned int i = 0; i < m_DwiOriginalImage->GetVectorLength(); i++ )  //
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
                                      */                  ,
                                                          int /* column */)
{
  // pushButton_Save->setEnabled(pushButton_Editable->isCheckable());
}

void IntensityMotionCheckPanel::on_pushButton_SaveProtocolAs_clicked()
{
  QString xmlFile = QFileDialog::getSaveFileName( this, tr(
                                                    "Save Protocol As"), lineEdit_Protocol->text(),
                                                  tr("xml Files (*.xml)") );

  if( xmlFile.length() > 0 )
    {
    lineEdit_Protocol->setText(xmlFile);
    XmlStreamWriter XmlWriter(treeWidget);
    XmlWriter.setProtocol(&protocol);
    XmlWriter.writeXml_Protocol(xmlFile);

    // treeWidget->clear();
    protocol.clear();

    XmlStreamReader XmlReader(treeWidget);
    XmlReader.setProtocol( &protocol);
    // XmlReader.readFile(xmlFile, XmlStreamReader::TreeWise);
    XmlReader.readFile(xmlFile, XmlStreamReader::ProtocolWise);
    }

  bProtocol = true;
}

void IntensityMotionCheckPanel::on_pushButton_Save_clicked()
{
  if( lineEdit_Protocol->text().length() > 0 )
    {
    XmlStreamWriter XmlWriter(treeWidget);
    XmlWriter.setProtocol(&protocol);
    XmlWriter.writeXml_Protocol( lineEdit_Protocol->text() );

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
                                                      "Save Protocol As"), lineEdit_Protocol->text(),
                                                    tr("xml Files (*.xml)") );
    if( xmlFile.length() > 0 )
      {
      lineEdit_Protocol->setText(xmlFile);
      XmlStreamWriter XmlWriter(treeWidget);
      XmlWriter.setProtocol(&protocol);
      XmlWriter.writeXml_Protocol(xmlFile);

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
  // pushButton_SaveDWIAs->setEnabled( 0 );

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
  for( unsigned int i = 0; i < GradientDirectionContainer->size(); i++ )
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

  if( m_DwiOriginalImage->GetVectorLength() != GradientDirectionContainer->size() )
    {
    std::cout
    << "Bad DWI: mismatch between gradient image #and gradient vector #"
    << std::endl;
    }
  itk::MetaDataDictionary imgMetaDictionary
    = m_DwiOriginalImage->GetMetaDataDictionary();
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();

  std::string metaString;

  //  measurement frame
  if( imgMetaDictionary.HasKey("NRRD_measurement frame") )
    {
      {
      // imaging frame
      const vnl_matrix_fixed<double, 3, 3> & imgf = m_DwiOriginalImage->GetDirection().GetVnlMatrix();

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
  comboBox_Space->setCurrentIndex( comboBox_Space->findText( QString::
                                                             fromStdString( metaString), Qt::MatchExactly) );
}

void IntensityMotionCheckPanel::on_pushButton_DefaultProtocol_clicked()
{
  //  CreateDefaultProtocol(); //old
  //  UpdateProtocolTree(); //old

  if( m_DwiOriginalImage->GetVectorLength() != GradientDirectionContainer->size() )
    {
    std::cout
    << "Bad DWI: mismatch between gradient image #and gradient vector #"
    << std::endl;
    QMessageBox::critical( this, tr("BAD DWI !"),
                           tr("Bad DWI: mismatch between gradient image #and gradient vector #!") );
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
  this->GetProtocol().GetQCedDWIFileNameSuffix() = "_QCed.nrrd";
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

  std::string metaString;

  itk::ExposeMetaData<std::string>(imgMetaDictionary, "NRRD_space", metaString);
  if( metaString == "left-anterior-inferior" )
    {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_LAI;
    }
  else if( metaString == "left-anterior-superior" )
    {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_LAS;
    }
  else if( metaString == "left-posterior-inferior" )
    {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_LPI;
    }
  else if( metaString == "left-posterior-superior" )
    {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_LPS;
    }
  else if( metaString == "right-anterior-inferior" )
    {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_RAI;
    }
  else if( metaString == "right-anterior-superior" )
    {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_RAS;
    }
  else if( metaString == "right-posterior-inferior" )
    {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_RPI;
    }
  else if( metaString == "right-posterior-superior" )
    {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_RPS;
    }
  else
    {
    this->GetProtocol().GetImageProtocol().space = Protocol::SPACE_UNKNOWN;
    }

  this->GetProtocol().GetImageProtocol().bCrop = true;
  this->GetProtocol().GetImageProtocol().croppedDWIFileNameSuffix
    = "_CroppedDWI.nrrd";

  this->GetProtocol().GetImageProtocol().reportFileNameSuffix = "_QCReport.txt";
  this->GetProtocol().GetImageProtocol().reportFileMode = 1;

  this->GetProtocol().GetImageProtocol().bQuitOnCheckSpacingFailure = false;
  this->GetProtocol().GetImageProtocol().bQuitOnCheckSizeFailure = false;

  // ***** diffusion
  GetGradientDirections( 0 );

  this->GetProtocol().GetDiffusionProtocol().bCheck = true;
  this->GetProtocol().GetDiffusionProtocol().bValue = this->b0;
  for( unsigned int i = 0; i < GradientDirectionContainer->size(); i++ )
    {
    vnl_vector_fixed<double, 3> vect;
    vect[0] = ( GradientDirectionContainer->ElementAt(i)[0] );
    vect[1] = ( GradientDirectionContainer->ElementAt(i)[1] );
    vect[2] = ( GradientDirectionContainer->ElementAt(i)[2] );

    this->GetProtocol().GetDiffusionProtocol().gradients.push_back(vect);
    }

  //
  if( imgMetaDictionary.HasKey("NRRD_measurement frame") )
    {
      {
      // imaging frame
      const vnl_matrix_fixed<double, 3, 3> & imgf = m_DwiOriginalImage->GetDirection().GetVnlMatrix();

      std::cout << "imgf(0, 0)" << imgf(0, 0) << std::endl;
      std::cout << "imgf(0, 1)" << imgf(0, 1) << std::endl;
      std::cout << "imgf(0, 2)" << imgf(0, 2) << std::endl;
      std::cout << "imgf(1, 0)" << imgf(1, 0) << std::endl;
      std::cout << "imgf(1, 1)" << imgf(1, 1) << std::endl;
      std::cout << "imgf(1, 2)" << imgf(1, 2) << std::endl;
      std::cout << "imgf(2, 0)" << imgf(2, 0) << std::endl;
      std::cout << "imgf(2, 1)" << imgf(2, 1) << std::endl;
      std::cout << "imgf(2, 2)" << imgf(2, 2) << std::endl;
      
      
      // Image frame
      this->GetProtocol().GetImageProtocol().spacedirection = m_DwiOriginalImage->GetDirection().GetVnlMatrix();
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
      for( unsigned int i = 0; i < 3; ++i )
        {
        for( unsigned int j = 0; j < 3; ++j )
          {
          // Meausurement frame
          this->GetProtocol().GetDiffusionProtocol().measurementFrame[i][j] = nrrdmf[i][j];
          std::cout << "nrrdmf[i][j]" << nrrdmf[i][j] << std::endl;
          }
        }
      std::cout << this->GetProtocol().GetDiffusionProtocol().measurementFrame << std::flush << std::endl;
      }
    }

  // HACK:  This breaks encapsulation of the function.  SetFunctions should be used!
  this->GetProtocol().GetDiffusionProtocol().bUseDiffusionProtocol = false;
  this->GetProtocol().GetDiffusionProtocol().diffusionReplacedDWIFileNameSuffix
    = "_DiffusionReplaced.nrrd";

  this->GetProtocol().GetDiffusionProtocol().reportFileNameSuffix
    = "_QCReport.txt";
  this->GetProtocol().GetDiffusionProtocol().reportFileMode = 1;
  this->GetProtocol().GetDiffusionProtocol().bQuitOnCheckFailure = false;

  // ***** Denoising

  this->GetProtocol().initDenoisingLMMSE();

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
  correlationDeviationThresholdgradient = 3.50;                                             //
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
  this->GetProtocol().GetSliceCheckProtocol().bQuitOnCheckFailure = false;
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
  this->GetProtocol().GetInterlaceCheckProtocol().bQuitOnCheckFailure = false;

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
  this->GetProtocol().GetGradientCheckProtocol().bQuitOnCheckFailure = false;

  // ***** baseline average
  this->GetProtocol().GetBaselineAverageProtocol().bAverage = true;

  this->GetProtocol().GetBaselineAverageProtocol().averageMethod = 1;
  this->GetProtocol().GetBaselineAverageProtocol().stopThreshold = 0.02;

  this->GetProtocol().GetBaselineAverageProtocol().outputDWIFileNameSuffix
    = "";
  this->GetProtocol().GetBaselineAverageProtocol().reportFileNameSuffix
    = "_QCReport.txt";

  this->GetProtocol().GetBaselineAverageProtocol().reportFileMode = 1;

  // ***** Eddy motion correction
  this->GetProtocol().GetEddyMotionCorrectionProtocol().bCorrect = true;
  //   this->GetProtocol().GetEddyMotionCorrectionProtocol().EddyMotionCommand =
  // "/tools/bin_linux64/EddyMotionCorrector";
  //  this->GetProtocol().GetEddyMotionCorrectionProtocol().InputFileName =
  // "_QCOutput.nrrd";
  //  this->GetProtocol().GetEddyMotionCorrectionProtocol().OutputFileName =
  // "_EddyMotion_Output.nrrd";

  this->GetProtocol().GetEddyMotionCorrectionProtocol().numberOfIterations    =  1000;
  this->GetProtocol().GetEddyMotionCorrectionProtocol().numberOfSamples
    =  100000;
  this->GetProtocol().GetEddyMotionCorrectionProtocol().translationScale
    =  1000.0;
  this->GetProtocol().GetEddyMotionCorrectionProtocol().maxStepLength    =  0.2;
  this->GetProtocol().GetEddyMotionCorrectionProtocol().minStepLength    =  0.0001;
  this->GetProtocol().GetEddyMotionCorrectionProtocol().relaxFactor    =  0.5;
  // this->GetProtocol().GetEddyMotionCorrectionProtocol().maxNumberOfIterations
  //  =  500;

  this->GetProtocol().GetEddyMotionCorrectionProtocol().outputDWIFileNameSuffix
    = "";
  this->GetProtocol().GetEddyMotionCorrectionProtocol().reportFileNameSuffix
    = "_QCReport.txt";
  this->GetProtocol().GetEddyMotionCorrectionProtocol().reportFileMode = 1;

  // Denoising JointLMMSE
  this->GetProtocol().initDenoisingJointLMMSE();

  // Brain Mask
  this->GetProtocol().initBrainMaskProtocol();
  this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_FSL = lineEdit_FSL->text().toStdString();  //
                                                                                                             // ???????????????????????????????????????????????????????????????????????????????????????????
  this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_Slicer = lineEdit_Slicer->text().toStdString();
  this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_convertITK = lineEdit_convertitk->text().toStdString();
  this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_imagemath = lineEdit_imagemath->text().toStdString();
  std::cout << "Test 2 Brain mask " << this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_FSL << std::endl;
  emit Set_init_Path_Signal();

  // Dominant directional artifact detector ( entropy tool )
  this->GetProtocol().initDominantDirectional_Detector();

  // ***** DTI
  this->GetProtocol().GetDTIProtocol().bCompute = true;
  this->GetProtocol().GetDTIProtocol().dtiestimCommand
    = "/tools/bin_linux64/dtiestim";
  this->GetProtocol().GetDTIProtocol().dtiprocessCommand
    = "/tools/bin_linux64/dtiprocess";
  this->GetProtocol().GetDTIProtocol().method = Protocol::METHOD_WLS;
  this->GetProtocol().GetDTIProtocol().baselineThreshold = 50; //
//  this->GetProtocol().GetDTIProtocol().mask = "";
  this->GetProtocol().GetDTIProtocol().tensorSuffix = "_DTI.nrrd";
  this->GetProtocol().GetDTIProtocol().bbaseline = true;
  this->GetProtocol().GetDTIProtocol().baselineSuffix = "_Baseline.nrrd";
  this->GetProtocol().GetDTIProtocol().bidwi = true;
  this->GetProtocol().GetDTIProtocol().idwiSuffix = "_IDWI.nrrd";
  this->GetProtocol().GetDTIProtocol().bfa = true;
  this->GetProtocol().GetDTIProtocol().faSuffix = "_FA.nrrd";
  this->GetProtocol().GetDTIProtocol().bmd = true;
  this->GetProtocol().GetDTIProtocol().mdSuffix = "_MD.nrrd";
  this->GetProtocol().GetDTIProtocol().bcoloredfa = true;
  this->GetProtocol().GetDTIProtocol().coloredfaSuffix = "_colorFA.nrrd";
  this->GetProtocol().GetDTIProtocol().bfrobeniusnorm = true;
  this->GetProtocol().GetDTIProtocol().frobeniusnormSuffix
    = "_frobeniusnorm.nrrd";

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

  if( !bDwiLoaded  )
    {
    LoadDwiImage();
    }
  if( !bDwiLoaded  )
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
  for( unsigned int j = 0; j < m_DwiOriginalImage->GetVectorLength(); j++ )
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
    for( unsigned int i = 1;
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
                                     */                          );
      Results.push_back(s2DResults);
      }
    ResultsContainer.push_back(Results);
    }

  GetGradientDirections(0);
  int DWICount, BaselineCount;

  DWICount = 0;
  BaselineCount = 0;
  for( unsigned int i = 0; i < GradientDirectionContainer->size(); i++ )
    {
    if( GradientDirectionContainer->ElementAt(i)[0] == 0.0
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
  for( unsigned int j = 0; j < ResultsContainer[0].size(); j++ )
    {
    double baselinemean = 0.0, DWImean = 0.0, baselinedeviation = 0.0,
      DWIdeviation = 0.0, baselineMin = 1.0, gradientMin = 1.0;
    for( unsigned int i = 0; i < ResultsContainer.size(); i++ )
      {
      if( GradientDirectionContainer->at(i)[0] == 0.0
          && GradientDirectionContainer->at(i)[1] == 0.0
          && GradientDirectionContainer->at(i)[2] == 0.0 )
        {
        baselinemean += ResultsContainer[i][j].Correlation
          / (double)(BaselineCount);
        if( ResultsContainer[i][j].Correlation < baselineMin )
          {
          baselineMin = ResultsContainer[i][j].Correlation;
          }
        }
      else
        {
        DWImean += ResultsContainer[i][j].Correlation / (double)(DWICount);
        if( ResultsContainer[i][j].Correlation < gradientMin )
          {
          gradientMin = ResultsContainer[i][j].Correlation;
          }
        }
      }

    baselineCorrelationMean.push_back(baselinemean);
    gradientCorrelationMean.push_back(DWImean);

    baselineCorrelationMin.push_back(baselineMin);
    gradientCorrelationMin.push_back(gradientMin);
    for( unsigned int i = 0; i < ResultsContainer.size(); i++ )
      {
      if( GradientDirectionContainer->at(i)[0] == 0.0
          && GradientDirectionContainer->at(i)[1] == 0.0
          && GradientDirectionContainer->at(i)[2] == 0.0 )
        {
        if( BaselineCount >= 1 )
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
  for( int i = 0 + (int)( sliceNum * beginSkip);
       i < sliceNum - (int)( sliceNum * endSkip);
       i++ )                                                                                //
  //
  // for(int
  //
  // j=0;j<ResultsContainer[0].size()-1;
  //
  // j++)
    {
    if( baselineCorrelationMin[i] < minBaselineCorrelation )
      {
      minBaselineCorrelation = baselineCorrelationMin[i];
      }

    if( gradientCorrelationMin[i] < minGradientCorrelation )
      {
      minGradientCorrelation = gradientCorrelationMin[i];
      }

    if( baselineCorrelationDev[i] > maxBaselineCorrelationDev )
      {
      maxBaselineCorrelationDev = baselineCorrelationDev[i];
      }

    if( gradientCorrelationDev[i] > maxGradientCorrelationDev )
      {
      maxGradientCorrelationDev = gradientCorrelationDev[i];
      }

    if( ( baselineCorrelationMean[i]
          - baselineCorrelationMin[i] ) / baselineCorrelationDev[i] >
        maxBaselineCorrelationDevTime )
      {
      maxBaselineCorrelationDevTime
        = ( baselineCorrelationMean[i]
            - baselineCorrelationMin[i] ) / baselineCorrelationDev[i];
      }

    if( ( gradientCorrelationMean[i]
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
  if( !bDwiLoaded )
    {
    LoadDwiImage();
    }
  if( !bDwiLoaded )
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
  for( unsigned int j = 0; j < m_DwiOriginalImage->GetVectorLength(); j++ )
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
    while( !iterateGradient.IsAtEnd() )
      {
      if( count < sizeLocal[0] * sizeLocal[1] * sizeLocal[2] * 2 )
        {
        if( ( count / ( sizeLocal[0] * sizeLocal[1] ) ) % 2 == 0 )
          {
          iterateEven.Set( iterateGradient.Get() );
          ++iterateEven;
          }
        if( ( count / ( sizeLocal[0] * sizeLocal[1] ) ) % 2 == 1 )
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
    while( !cit1.IsAtEnd() )
      {
      sAB += cit1.Get() * cit2.Get();
      sA2 += cit1.Get() * cit1.Get();
      sB2 += cit2.Get() * cit2.Get();
      ++cit1;
      ++cit2;
      }

    Correlation = sAB / sqrt(sA2 * sB2);

    if( GradientDirectionContainer->at(j)[0] == 0.0
        && GradientDirectionContainer->at(j)[1] == 0.0
        && GradientDirectionContainer->at(j)[2] == 0.0 )
      {
      baselineCorrelation.push_back(Correlation);
      }
    else
      {
      gradientCorrelation.push_back(Correlation);
      }

    //     std::cout<<"Correlation: " << Correlation<< std::endl;:837: error: expected `;' before ��this��

    }

  int DWICount, BaselineCount;

  DWICount = 0;
  BaselineCount = 0;
  for( unsigned int i = 0; i < GradientDirectionContainer->size(); i++ )
    {
    if( GradientDirectionContainer->ElementAt(i)[0] == 0.0
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
  for( unsigned int i = 0; i < baselineCorrelation.size(); i++ )
    {
    if( baselineCorrelation[i] < minBaselineCorrelation )
      {
      minBaselineCorrelation = baselineCorrelation[i];
      }
    meanBaselineCorrelation += baselineCorrelation[i]
      / baselineCorrelation.size();
    }
  for( unsigned int i = 0; i < baselineCorrelation.size(); i++ )
    {
    baselineCorrelationDev
      += ( baselineCorrelation[i]
           - meanBaselineCorrelation )
        * ( baselineCorrelation[i]
            - meanBaselineCorrelation ) / baselineCorrelation.size();                                                                                           //
    //
    // meanBaselineCorrelation
    //
    // +=
    //
    // baselineCorrelation[i]/baselineCorrelation.size();
    }
  baselineCorrelationDev = sqrt(baselineCorrelationDev);
  for( unsigned int i = 0; i < gradientCorrelation.size(); i++ )
    {
    if( gradientCorrelation[i] < minGradientCorrelation )
      {
      minGradientCorrelation = gradientCorrelation[i];
      }
    meanGradientCorrelation += gradientCorrelation[i]
      / gradientCorrelation.size();
    }
  for( unsigned int i = 0; i < gradientCorrelation.size(); i++ )
    {
    gradientCorrelationDev
      += ( gradientCorrelation[i]
           - meanGradientCorrelation )
        * ( gradientCorrelation[i]
            - meanGradientCorrelation ) / gradientCorrelation.size();                                                                                           //
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

void IntensityMotionCheckPanel::UpdateProtocolToTreeWidget()
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
  if( this->GetProtocol().GetImageProtocol().bCheck )
    {
    itemImageInformation->setText( 1, tr("Yes") );
    }
  else
    {
    itemImageInformation->setText( 1, tr("No") );
    }

  QTreeWidgetItem *itemSpace = new QTreeWidgetItem(itemImageInformation);
  itemSpace->setText( 0, tr("IMAGE_space") );

  switch( this->GetProtocol().GetImageProtocol().space )
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
                               .arg(this->GetProtocol().GetImageProtocol().spacedirection[0][1], 0, 'f', 6)
                               .arg(this->GetProtocol().GetImageProtocol().spacedirection[0][2], 0, 'f', 6)
                               .arg(this->GetProtocol().GetImageProtocol().spacedirection[1][0], 0, 'f', 6)
                               .arg(this->GetProtocol().GetImageProtocol().spacedirection[1][1], 0, 'f', 6)
                               .arg(this->GetProtocol().GetImageProtocol().spacedirection[1][2], 0, 'f', 6)
                               .arg(this->GetProtocol().GetImageProtocol().spacedirection[2][0], 0, 'f', 6)
                               .arg(this->GetProtocol().GetImageProtocol().spacedirection[2][1], 0, 'f', 6)
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
  if( this->GetProtocol().GetImageProtocol().bCrop )
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
  if( this->GetProtocol().GetImageProtocol().bQuitOnCheckSpacingFailure )
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
  if( this->GetProtocol().GetImageProtocol().bQuitOnCheckSizeFailure )
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
  if( this->GetProtocol().GetDiffusionProtocol().bCheck )
    {
    itemDiffusionInformation->setText( 1, tr("Yes") );
    }
  else
    {
    itemDiffusionInformation->setText( 1, tr("No") );
    }
  std::cout << this->GetProtocol().GetDiffusionProtocol().measurementFrame << std::flush << std::endl;

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
  for( unsigned int i = 0;
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
  if( this->GetProtocol().GetDiffusionProtocol().bUseDiffusionProtocol )
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
  if( this->GetProtocol().GetDiffusionProtocol().bQuitOnCheckFailure )
    {
    itemDiffusionbQuitOnCheckFailure->setText( 1, tr("Yes") );
    }
  else
    {
    itemDiffusionbQuitOnCheckFailure->setText( 1, tr("No") );
    }

  // Denoising method1: "Recian LMMSE Image Filter" adopted from Slicer3
  QTreeWidgetItem * item_DenoiseLMMSE = new QTreeWidgetItem(treeWidget);
  item_DenoiseLMMSE->setText( 0, tr("DENOISING_bCheck") );
  if( this->GetProtocol().GetDenoisingLMMSEProtocol().bCheck )
    {
    item_DenoiseLMMSE->setText( 1, tr("Yes") );
    }
  else
    {
    item_DenoiseLMMSE->setText( 1, tr("No") );
    }

  QTreeWidgetItem * item_PathCommand = new QTreeWidgetItem(item_DenoiseLMMSE);
  item_PathCommand->setText( 0, tr("DENOISING_Path") );
  item_PathCommand->setText( 1, QString::fromStdString(this->GetProtocol().GetDenoisingLMMSEProtocol().LMMSECommand) );

  QTreeWidgetItem * item_ParameterSet = new QTreeWidgetItem(item_DenoiseLMMSE);
  item_ParameterSet->setText( 0, tr("DENOISING_ParameterSet") );
  item_ParameterSet->setText( 1, QString::fromStdString(this->GetProtocol().GetDenoisingLMMSEProtocol().ParameterSet) );

  QTreeWidgetItem * item_NumItr = new QTreeWidgetItem(item_DenoiseLMMSE);
  item_NumItr->setText( 0, tr("DENOISING_NumIter") );
  item_NumItr->setText( 1, QString("%1").arg( this->GetProtocol().GetDenoisingLMMSEProtocol().NumIter, 0, 10 ) );

  QTreeWidgetItem * item_EstRadius = new QTreeWidgetItem(item_DenoiseLMMSE);
  item_EstRadius->setText( 0, tr("DENOISING_Est_Radius") );
  item_EstRadius->setText( 1,
                           QString("%1, %2, %3").arg( this->GetProtocol().GetDenoisingLMMSEProtocol().Est_Radius[0], 0,
                                                      10 ).arg( this->GetProtocol().GetDenoisingLMMSEProtocol().
                                                                Est_Radius[1], 0,
                                                                10 ).arg( this->GetProtocol().GetDenoisingLMMSEProtocol()
                                                                          .Est_Radius[2], 0, 10 ) );

  QTreeWidgetItem * item_FilterRadius = new QTreeWidgetItem(item_DenoiseLMMSE);
  item_FilterRadius->setText( 0, tr("DENOISING_Filter_Radius") );
  item_FilterRadius->setText( 1,
                              QString("%1, %2, %3").arg( this->GetProtocol().GetDenoisingLMMSEProtocol().Filter_Radius[
                                                           0], 0,
                                                         10 ).arg( this->GetProtocol().GetDenoisingLMMSEProtocol().
                                                                   Filter_Radius[1], 0,
                                                                   10 ).arg( this->GetProtocol().
                                                                             GetDenoisingLMMSEProtocol().Filter_Radius[
                                                                               2], 0, 10 ) );

  QTreeWidgetItem * item_MinNumVoxelFilter = new QTreeWidgetItem(item_DenoiseLMMSE);
  item_MinNumVoxelFilter->setText( 0, tr("DENOISING_Min_VoxelNum_Filter") );
  item_MinNumVoxelFilter->setText( 1,
                                   QString("%1").arg( this->GetProtocol().GetDenoisingLMMSEProtocol().
                                                      Min_VoxelNum_Filter, 0, 10 ) );

  QTreeWidgetItem * item_MinNumVoxelEstimation = new QTreeWidgetItem(item_DenoiseLMMSE);
  item_MinNumVoxelEstimation->setText( 0, tr("DENOISING_Min_VoxelNum_Est") );
  item_MinNumVoxelEstimation->setText( 1,
                                       QString("%1").arg( this->GetProtocol().GetDenoisingLMMSEProtocol().
                                                          Min_VoxelNum_Est, 0, 10 ) );

  QTreeWidgetItem * item_MinNoiseSTD = new QTreeWidgetItem(item_DenoiseLMMSE);
  item_MinNoiseSTD->setText( 0, tr("DENOISING_MinNoiseSTD") );
  item_MinNoiseSTD->setText( 1, QString("%1").arg( this->GetProtocol().GetDenoisingLMMSEProtocol().MinNoiseSTD, 0, 10 ) );

  QTreeWidgetItem * item_MaxNoiseSTD = new QTreeWidgetItem(item_DenoiseLMMSE);
  item_MaxNoiseSTD->setText( 0, tr("DENOISING_MaxNoiseSTD") );
  item_MaxNoiseSTD->setText( 1, QString("%1").arg( this->GetProtocol().GetDenoisingLMMSEProtocol().MaxNoiseSTD, 0, 10 ) );

  QTreeWidgetItem * item_HistogramResolutionFactor = new QTreeWidgetItem(item_DenoiseLMMSE);
  item_HistogramResolutionFactor->setText( 0, tr("DENOISING_HistogramResolution") );
  item_HistogramResolutionFactor->setText( 1,
                                           QString("%1").arg( this->GetProtocol().GetDenoisingLMMSEProtocol().
                                                              HistogramResolution, 0, 'f', 6 ) );

  QTreeWidgetItem * item_uav = new QTreeWidgetItem(item_DenoiseLMMSE);
  item_uav->setText( 0, tr("DENOISING_AbsoluteValue") );
  if( this->GetProtocol().GetDenoisingLMMSEProtocol().AbsoluteValue )
    {
    item_uav->setText( 1, tr("Yes") );
    }
  else
    {
    item_uav->setText( 1, tr("No") );
    }

  // Slice Check
  QTreeWidgetItem *itemSliceCheck = new QTreeWidgetItem(treeWidget);
  itemSliceCheck->setText( 0, tr("SLICE_bCheck") );
  if( this->GetProtocol().GetSliceCheckProtocol().bCheck )
    {
    itemSliceCheck->setText( 1, tr("Yes") );
    }
  else
    {
    itemSliceCheck->setText( 1, tr("No") );
    }

  QTreeWidgetItem *itembSubregionalCheck = new QTreeWidgetItem(itemSliceCheck);
  itembSubregionalCheck->setText( 0, tr("SLICE_bSubregionalCheck") );
  if( this->GetProtocol().GetSliceCheckProtocol().bSubregionalCheck )
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
  if( this->GetProtocol().GetSliceCheckProtocol().bQuitOnCheckFailure )
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
  if( this->GetProtocol().GetInterlaceCheckProtocol().bCheck )
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
                                                 QString::fromStdString(this->GetProtocol().GetInterlaceCheckProtocol()
                                                                        .
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
                                                       QString::fromStdString(this->GetProtocol().
                                                                              GetInterlaceCheckProtocol().
                                                                              excludedDWINrrdFileNameSuffix) );

  QTreeWidgetItem *itembInterlaceQuitOnCheckFailur = new QTreeWidgetItem(itemInterlaceCheck);
  itembInterlaceQuitOnCheckFailur->setText( 0, tr("INTERLACE_bQuitOnCheckFailure") );
  if( this->GetProtocol().GetInterlaceCheckProtocol().bQuitOnCheckFailure )
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
  if( this->GetProtocol().GetBaselineAverageProtocol().bAverage )
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
                                                    QString::fromStdString(this->GetProtocol().
                                                                           GetBaselineAverageProtocol().
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
  if( this->GetProtocol().GetEddyMotionCorrectionProtocol().bCorrect )
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
  //
  //
  // itemEddyMotionCommand->setText(1,QString::fromStdString(this->GetProtocol().GetEddyMotionCorrectionProtocol().EddyMotionCommand));

  //  QTreeWidgetItem *itemEddyMotionInputFilename = new
  // QTreeWidgetItem(itemEddyMotionCorrection);
  //  itemEddyMotionInputFilename->setText(0, tr("EddyMotionInputFileName"));
  //
  //
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
  //
  //
  // itemEddyMotionOutputFilename->setText(1,QString::fromStdString(this->GetProtocol().GetEddyMotionCorrectionProtocol().outputFileName));

  QTreeWidgetItem *itemNumberOfBins = new QTreeWidgetItem(
      itemEddyMotionCorrection);
  itemNumberOfBins->setText( 0, tr("EDDYMOTION_numberOfIterations") );
  itemNumberOfBins->setText( 1,
                             QString("%1").arg(this->GetProtocol().GetEddyMotionCorrectionProtocol().
                                               numberOfIterations,  0, 10) );

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

  QTreeWidgetItem *itemMaxStepLength = new QTreeWidgetItem(
      itemEddyMotionCorrection);
  itemMaxStepLength->setText( 0, tr("EDDYMOTION_maxStepLength") );
  itemMaxStepLength->setText( 1,
                              QString::number(this->GetProtocol().GetEddyMotionCorrectionProtocol().
                                              maxStepLength,  'f', 5) );

  QTreeWidgetItem *itemMinStepLength = new QTreeWidgetItem(
      itemEddyMotionCorrection);
  itemMinStepLength->setText( 0, tr("EDDYMOTION_minStepLength") );
  itemMinStepLength->setText( 1,
                              QString::number(this->GetProtocol().GetEddyMotionCorrectionProtocol().
                                              minStepLength,  'f', 5) );

  QTreeWidgetItem *itemRelaxFactor = new QTreeWidgetItem(
      itemEddyMotionCorrection);
  itemRelaxFactor->setText( 0, tr("EDDYMOTION_relaxFactor") );
  itemRelaxFactor->setText( 1,
                            QString::number(this->GetProtocol().GetEddyMotionCorrectionProtocol().
                                            relaxFactor,  'f', 4) );

  // QTreeWidgetItem *itemMaxNumberOfIterations = new QTreeWidgetItem(
  //     itemEddyMotionCorrection);
//  itemMaxNumberOfIterations->setText( 0, tr("EDDYMOTION_maxNumberOfIterations") );
// itemMaxNumberOfIterations->setText( 1,
  //  QString("%1").arg(this->GetProtocol().GetEddyMotionCorrectionProtocol().
  //   maxNumberOfIterations,  0, 10) );
  // ////////////////////////////////////////////////////////////////////////

  QTreeWidgetItem *itemEddyMotionOutputDWIFileNameSuffix = new QTreeWidgetItem(
      itemEddyMotionCorrection);
  itemEddyMotionOutputDWIFileNameSuffix->setText( 0,
                                                  tr("EDDYMOTION_outputDWIFileNameSuffix") );
  itemEddyMotionOutputDWIFileNameSuffix->setText( 1,
                                                  QString::fromStdString(this->GetProtocol().
                                                                         GetEddyMotionCorrectionProtocol()
                                                                         .
                                                                         outputDWIFileNameSuffix) );

  QTreeWidgetItem *itemEddyMotionReportFileNameSuffix = new QTreeWidgetItem(
      itemEddyMotionCorrection);
  itemEddyMotionReportFileNameSuffix->setText( 0,
                                               tr("EDDYMOTION_reportFileNameSuffix") );
  itemEddyMotionReportFileNameSuffix->setText( 1,
                                               QString::fromStdString(this->GetProtocol().
                                                                      GetEddyMotionCorrectionProtocol()
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
  if( this->GetProtocol().GetGradientCheckProtocol().bCheck )
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
                                                      QString::fromStdString(this->GetProtocol().
                                                                             GetGradientCheckProtocol().
                                                                             excludedDWINrrdFileNameSuffix) );

  QTreeWidgetItem *itembGradientQuitOnCheckFailur = new QTreeWidgetItem(itemGradientCheck);
  itembGradientQuitOnCheckFailur->setText( 0, tr("GRADIENT_bQuitOnCheckFailure") );
  if( this->GetProtocol().GetGradientCheckProtocol().bQuitOnCheckFailure )
    {
    itembGradientQuitOnCheckFailur->setText( 1, tr("Yes") );
    }
  else
    {
    itembGradientQuitOnCheckFailur->setText( 1, tr("No") );
    }

  // Denoising method2: "Joint LMMSE Image Filter" adopted from Slicer3

  QTreeWidgetItem * item_JointLMMSE = new QTreeWidgetItem(treeWidget);
  item_JointLMMSE->setText( 0, tr("JOINDENOISING_bCheck") );
  if( this->GetProtocol().GetDenoisingJointLMMSE().bCheck )
    {
    item_JointLMMSE->setText( 1, tr("Yes") );
    }
  else
    {
    item_JointLMMSE->setText( 1, tr("No") );
    }

  QTreeWidgetItem * item_JointLMMSE_Command = new QTreeWidgetItem( item_JointLMMSE );
  item_JointLMMSE_Command->setText( 0, tr("JOINDENOISING_Path") );
  item_JointLMMSE_Command->setText( 1,
                                    QString::fromStdString(this->GetProtocol().GetDenoisingJointLMMSE().
                                                           JointLMMSECommand) );

  QTreeWidgetItem * item_JointLMMSE_ParameterSet = new QTreeWidgetItem( item_JointLMMSE );
  item_JointLMMSE_ParameterSet->setText( 0, tr("JOINDENOISING_ParameterSet") );
  item_JointLMMSE_ParameterSet->setText( 1,
                                         QString::fromStdString(this->GetProtocol().GetDenoisingJointLMMSE().
                                                                ParameterSet) );

  QTreeWidgetItem * item_JointLMMSE_NumNeighborGradients = new QTreeWidgetItem( item_JointLMMSE );
  item_JointLMMSE_NumNeighborGradients->setText( 0, tr("JOINDENOISING_NumNeighborGradients") );
  item_JointLMMSE_NumNeighborGradients->setText( 1,
                                                 QString("%1").arg(this->GetProtocol().GetDenoisingJointLMMSE().
                                                                   NumNeighborGradients, 0, 10) );

  QTreeWidgetItem * item_JointLMMSE_EstRadius = new QTreeWidgetItem(item_JointLMMSE);
  item_JointLMMSE_EstRadius->setText( 0, tr("JOINDENOISING_Est_Radius") );
  item_JointLMMSE_EstRadius->setText( 1,
                                      QString("%1, %2, %3").arg( this->GetProtocol().GetDenoisingJointLMMSE().
                                                                 Est_Radius[0], 0,
                                                                 10 ).arg( this->GetProtocol().GetDenoisingJointLMMSE()
                                                                           .Est_Radius[1], 0,
                                                                           10 ).arg( this->GetProtocol().
                                                                                     GetDenoisingJointLMMSE().
                                                                                     Est_Radius[2], 0, 10 ) );

  QTreeWidgetItem * item_JointLMMSE_FilterRadius = new QTreeWidgetItem(item_JointLMMSE);
  item_JointLMMSE_FilterRadius->setText( 0, tr("JOINDENOISING_Filter_Radius") );
  item_JointLMMSE_FilterRadius->setText( 1,
                                         QString("%1, %2, %3").arg( this->GetProtocol().GetDenoisingJointLMMSE().
                                                                    Filter_Radius[0], 0,
                                                                    10 ).arg( this->GetProtocol().
                                                                              GetDenoisingJointLMMSE().Filter_Radius[1],
                                                                              0,
                                                                              10 ).arg( this->GetProtocol().
                                                                                        GetDenoisingJointLMMSE().
                                                                                        Filter_Radius[2], 0, 10 ) );

  // Creating brain mask

  QTreeWidgetItem * item_brainmask_bCheck = new QTreeWidgetItem(treeWidget);
  item_brainmask_bCheck->setText( 0, tr("BRAINMASK_bCheck") );
  if( this->GetProtocol().GetBrainMaskProtocol().bMask )
    {
    item_brainmask_bCheck->setText( 1, tr("Yes") );
    }
  else
    {
    item_brainmask_bCheck->setText( 1, tr("No") );
    }

  QTreeWidgetItem * item_brainmask_Method = new QTreeWidgetItem(item_brainmask_bCheck);
  item_brainmask_Method->setText( 0, tr("BRAINMASK_method") );

  item_brainmask_Method->setText( 1, QString("%1").arg(this->GetProtocol().GetBrainMaskProtocol().
                                                       BrainMask_Method, 0, 10) );

  QTreeWidgetItem * item_brainmask_SystemPath_FSL = new QTreeWidgetItem(item_brainmask_bCheck);
  item_brainmask_SystemPath_FSL->setText( 0, tr("BRAINMASK_SystemPath_FSL") );
  item_brainmask_SystemPath_FSL->setText( 1,
                                          QString::fromStdString( this->GetProtocol().GetBrainMaskProtocol().
                                                                  BrainMask_SystemPath_FSL ) );

  QTreeWidgetItem * item_brainmask_SystemPath_convitk = new QTreeWidgetItem(item_brainmask_bCheck);
  item_brainmask_SystemPath_convitk->setText( 0, tr("BRAINMASK_SystemPath_convertITK") );
  item_brainmask_SystemPath_convitk->setText( 1,
                                              QString::fromStdString( this->GetProtocol().GetBrainMaskProtocol().
                                                                      BrainMask_SystemPath_convertITK ) );

  QTreeWidgetItem * item_brainmask_SystemPath_imagemath = new QTreeWidgetItem(item_brainmask_bCheck);
  item_brainmask_SystemPath_imagemath->setText( 0, tr("BRAINMASK_SystemPath_imagemath") );
  item_brainmask_SystemPath_imagemath->setText( 1,
                                                QString::fromStdString( this->GetProtocol().GetBrainMaskProtocol().
                                                                        BrainMask_SystemPath_imagemath ) );

  QTreeWidgetItem * item_brainmask_SystemPath_Slicer = new QTreeWidgetItem(item_brainmask_bCheck);
  item_brainmask_SystemPath_Slicer->setText( 0, tr("BRAINMASK_SystemPath_Slicer") );
  item_brainmask_SystemPath_Slicer->setText( 1,
                                             QString::fromStdString( this->GetProtocol().GetBrainMaskProtocol().
                                                                     BrainMask_SystemPath_Slicer ) );

  QTreeWidgetItem * item_brainmask_MaskedImage = new QTreeWidgetItem(item_brainmask_bCheck);
  item_brainmask_MaskedImage->setText( 0, tr("BRAINMASK_MaskedImage") );
  item_brainmask_MaskedImage->setText( 1,
                                       QString::fromStdString( this->GetProtocol().GetBrainMaskProtocol().
                                                               BrainMask_Image ) );

  QTreeWidgetItem * item_brainmask_reportSuffix = new QTreeWidgetItem(item_brainmask_bCheck);
  item_brainmask_reportSuffix->setText( 0, tr("BRAINMASK_reportFileNameSuffix") );
  item_brainmask_reportSuffix->setText( 1,
                                        QString::fromStdString( this->GetProtocol().GetBrainMaskProtocol().
                                                                reportFileNameSuffix ) );

  QTreeWidgetItem * item_brainmask_reportMode = new QTreeWidgetItem(item_brainmask_bCheck);
  item_brainmask_reportMode->setText( 0, tr("BRAINMASK_reportFileMode") );
  item_brainmask_reportMode->setText( 1, QString("%1").arg( this->GetProtocol().GetBrainMaskProtocol().reportFileMode ) );

  QTreeWidgetItem * item_brainmask_bQuitOnCheckFailure = new QTreeWidgetItem(item_brainmask_bCheck);
  item_brainmask_bQuitOnCheckFailure->setText( 0, tr("BRAINMASK_bQuitOnCheckFailure") );
  if( this->GetProtocol().GetBrainMaskProtocol().bQuitOnCheckFailure )
    {
    item_brainmask_bQuitOnCheckFailure->setText( 1, tr( "Yes") );
    }
  else
    {
    item_brainmask_bQuitOnCheckFailure->setText( 1, tr( "No") );
    }

  /*ComboBoxItem * cmb3 = new ComboBoxItem(item_brainmask, 1);

  QTreeWidgetItem * item_brainmask_method = new QTreeWidgetItem(item_brainmask);
  item_brainmask_method->setText( 0 , tr ("BrainMask_method") );
    QTreeWidgetItem * item_brainmask_method2 = new QTreeWidgetItem(item_brainmask);
    item_brainmask_method2->setText( 0 , tr ("BrainMask_method2") );
  ComboBoxItem * cmb = new ComboBoxItem(item_brainmask_method, 1);
  ComboBoxItem * cmb2 = new ComboBoxItem(item_brainmask_method2, 1);
    cmb2->addItem("One");
    cmb2->addItem("Two");
    cmb2->addItem("Three");
    cmb2->setVisible(true);
    cmb2->setAutoFillBackground(true);
    cmb3->addItem("One");
    cmb3->addItem("Two");
    cmb3->addItem("Three");
    cmb3->setVisible(true);
    cmb3->setAutoFillBackground(true);


  cmb->addItem("One");
  cmb->addItem("Two");
  cmb->setVisible(true);
  cmb->setAutoFillBackground(true);


  treeWidget->setItemWidget(item_brainmask_method, 1 , cmb);

    treeWidget->setItemWidget(item_brainmask_method2, 1 , cmb2);

    treeWidget->setItemWidget(item_brainmask, 1 , cmb3);


  std::cout << "test brain mask " << item_brainmask_method->text(1).toStdString() << std::endl;
  */

  // Dominant directional artifact checking
  QTreeWidgetItem * item_dominantDirectional = new QTreeWidgetItem(treeWidget);
  item_dominantDirectional->setText( 0, tr("DOMINANTDIRECTION_bCheck") );
  if( this->GetProtocol().GetDominantDirectional_Detector().bCheck )
    {
    item_dominantDirectional->setText( 1, tr("Yes") );
    }
  else
    {
    item_dominantDirectional->setText( 1, tr("No") );
    }

  QTreeWidgetItem * item_dominantDirectional_mean = new QTreeWidgetItem(item_dominantDirectional);
  item_dominantDirectional_mean->setText( 0, tr("DOMINANTDIRECTION_Mean") );
  item_dominantDirectional_mean->setText( 1, QString("%1").arg( this->GetProtocol().GetDominantDirectional_Detector().
                                                                Mean, 0, 'f', 6 ) );

  QTreeWidgetItem * item_dominantDirectional_std = new QTreeWidgetItem(item_dominantDirectional);
  item_dominantDirectional_std->setText( 0, tr("DOMINANTDIRECTION_Deviation") );
  item_dominantDirectional_std->setText( 1, QString("%1").arg( this->GetProtocol().GetDominantDirectional_Detector().
                                                               Deviation, 0, 'f', 6 ) );

  QTreeWidgetItem * item_dominantDirectional_threshold1 = new QTreeWidgetItem(item_dominantDirectional);
  item_dominantDirectional_threshold1->setText( 0, tr("DOMINANTDIRECTION_z-Threshold1") );
  item_dominantDirectional_threshold1->setText( 1,
                                                QString("%1").arg( this->GetProtocol().GetDominantDirectional_Detector()
                                                                   .
                                                                   Threshold_Acceptance, 0, 'f', 6 ) );

  QTreeWidgetItem * item_dominantDirectional_threshold2 = new QTreeWidgetItem(item_dominantDirectional);
  item_dominantDirectional_threshold2->setText( 0, tr("DOMINANTDIRECTION_z-Threshold2") );
  item_dominantDirectional_threshold2->setText( 1,
                                                QString("%1").arg( this->GetProtocol().GetDominantDirectional_Detector()
                                                                   .
                                                                   Threshold_Suspicion_Unacceptance, 0, 'f', 6 ) );

  QTreeWidgetItem * item_dominantDirectional_reportFileNameSuffix = new QTreeWidgetItem(item_dominantDirectional);
  item_dominantDirectional_reportFileNameSuffix->setText( 0, tr("DOMINANTDIRECTION_reportFileNameSuffix") );
  item_dominantDirectional_reportFileNameSuffix->setText( 1,
                                                          QString::fromStdString( this->GetProtocol().
                                                                                  GetDominantDirectional_Detector().
                                                                                  reportFileNameSuffix) );

  QTreeWidgetItem * item_dominantDirectional_reportFileMode = new QTreeWidgetItem(item_dominantDirectional);
  item_dominantDirectional_reportFileMode->setText( 0, tr("DOMINANTDIRECTION_reportFileMode") );
  item_dominantDirectional_reportFileMode->setText( 1,
                                                    QString("%1").arg( this->GetProtocol().
                                                                       GetDominantDirectional_Detector().
                                                                       reportFileMode, 0, 10 ) );

  QTreeWidgetItem * item_dominantDirectional_bQuitOnCheckFailure = new QTreeWidgetItem(item_dominantDirectional);
  item_dominantDirectional_bQuitOnCheckFailure->setText( 0, tr("DOMINANTDIRECTION_bQuitOnCheckFailure") );
  if( this->GetProtocol().GetDominantDirectional_Detector().bQuitOnCheckFailure )
    {
    item_dominantDirectional_bQuitOnCheckFailure->setText( 1, tr("Yes") );
    }
  else
    {
    item_dominantDirectional_bQuitOnCheckFailure->setText( 1, tr("No") );
    }

  // DTI Computing
  QTreeWidgetItem *itemDTIComputing = new QTreeWidgetItem(treeWidget);
  itemDTIComputing->setText( 0, tr("DTI_bCompute") );
  if( this->GetProtocol().GetDTIProtocol().bCompute )
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

  switch( this->GetProtocol().GetDTIProtocol().method )
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

  // QTreeWidgetItem *itemMaskFile = new QTreeWidgetItem(itemDTIComputing);
  // itemMaskFile->setText( 0, tr("DTI_maskFileName") );
  // itemMaskFile->setText( 1,
  //                       QString::fromStdString(this->GetProtocol().GetDTIProtocol().mask ) );

  QTreeWidgetItem *itemTensorFile = new QTreeWidgetItem(itemDTIComputing);
  itemTensorFile->setText( 0, tr("DTI_tensor") );
  itemTensorFile->setText( 1,
                           QString::fromStdString(this->GetProtocol().GetDTIProtocol().tensorSuffix ) );

  // tensor scalar images
  QTreeWidgetItem *itemScalarBaseline = new QTreeWidgetItem(itemDTIComputing);
  itemScalarBaseline->setText( 0, tr("DTI_baseline") );
  if( this->GetProtocol().GetDTIProtocol().bbaseline )
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
  if( this->GetProtocol().GetDTIProtocol().bidwi )
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
  if( this->GetProtocol().GetDTIProtocol().bfa )
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
  if( this->GetProtocol().GetDTIProtocol().bmd )
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
  if( this->GetProtocol().GetDTIProtocol().bcoloredfa )
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
  if( this->GetProtocol().GetDTIProtocol().bfrobeniusnorm )
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

void IntensityMotionCheckPanel::f_overallSliceWiseCheck()
{
  // computing the overall results of the SliceWise check
  int num_SliceWiseCheckExc = 0;
  int _r_SliceWiseCkeck = 0;

  for( unsigned int i = 0; i < qcResult.GetIntensityMotionCheckResult().size();
       i++ )
    {

    if( qcResult.GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_EXCLUDE_SLICECHECK )
      {
      num_SliceWiseCheckExc++;
      }
    }
  _r_SliceWiseCkeck = num_SliceWiseCheckExc / qcResult.GetIntensityMotionCheckResult().size();

  if( _r_SliceWiseCkeck > this->GetProtocol().GetBadGradientPercentageTolerance() )
    {
    qcResult.GetOverallQCResult(). SWCk = false;
    }
  else
    {
    qcResult.GetOverallQCResult(). SWCk = true;
    }

}

void IntensityMotionCheckPanel::f_overallInterlaceWiseCheck()
{
  // computing the overall results of the InterlaceWise check
  int num_SliceWiseCheckExc = 0;
  int num_InterlaceWiseCheckExc = 0;
  int _r_InterlaceWiseCheck = 0;

  for( unsigned int i = 0; i < qcResult.GetIntensityMotionCheckResult().size();
       i++ )
    {

    if( qcResult.GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_EXCLUDE_SLICECHECK )
      {
      num_SliceWiseCheckExc++;
      }
    if( qcResult.GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_EXCLUDE_INTERLACECHECK )
      {
      num_InterlaceWiseCheckExc++;
      }
    }
  _r_InterlaceWiseCheck = (num_InterlaceWiseCheckExc + num_SliceWiseCheckExc)
    / qcResult.GetIntensityMotionCheckResult().size();
  if( _r_InterlaceWiseCheck > this->GetProtocol().GetBadGradientPercentageTolerance() )
    {
    qcResult.GetOverallQCResult(). IWCk = false;
    }
  else
    {
    qcResult.GetOverallQCResult(). IWCk = true;
    }

}

void IntensityMotionCheckPanel::f_overallGradientWiseCheck()
{
  // computing the overall results of the GradientWise check
  int num_SliceWiseCheckExc = 0;
  int num_InterlaceWiseCheckExc = 0;
  int num_GradientWiseCheckExc = 0;

  for( unsigned int i = 0; i < qcResult.GetIntensityMotionCheckResult().size(); i++ )
    {
    if( qcResult.GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_EXCLUDE_SLICECHECK )
      {
      num_SliceWiseCheckExc++;
      }
    if( qcResult.GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_EXCLUDE_INTERLACECHECK )
      {
      num_InterlaceWiseCheckExc++;
      }
    if( qcResult.GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_EXCLUDE_GRADIENTCHECK )
      {
      num_GradientWiseCheckExc++;
      }
    }

  // const double _r_GradWiseCheck = (num_GradientWiseCheckExc + num_SliceWiseCheckExc + num_InterlaceWiseCheckExc +
  // B0BadAvreage)/ qcResult.GetIntensityMotionCheckResult().size();

  const double _r_GradWiseCheck = (num_GradientWiseCheckExc + num_SliceWiseCheckExc + num_InterlaceWiseCheckExc)
    / qcResult.GetIntensityMotionCheckResult().size();

  if( _r_GradWiseCheck > this->GetProtocol().GetBadGradientPercentageTolerance() )
    {
    qcResult.GetOverallQCResult(). GWCk = false;
    }
  else
    {
    qcResult.GetOverallQCResult(). GWCk = true;
    }
}

void IntensityMotionCheckPanel::Set_VCStatus()
{

  // Updating VC_Status
  Clear_VC_Status();
  for( unsigned int ind = 0;
       ind < myIntensityThread.m_IntensityMotionCheck->get_Original_ForcedConformance_Mapping().size(); ind++  )
    {
    VC_STATUS vc;
    vc.index = ind;
    if( ind == 0 )
      {
      for( unsigned int k = 0;
           k <
           myIntensityThread.m_IntensityMotionCheck->get_Original_ForcedConformance_Mapping()[0].index_original.size();
           k++ )
        {
        vc.VC_status =
          this->GetQCResult().GetIntensityMotionCheckResult()[myIntensityThread.m_IntensityMotionCheck->
                                                              get_Original_ForcedConformance_Mapping()
                                                              [ind].index_original[k]].VisualChecking;
        }
      }

    else
      {
      vc.VC_status =
        this->GetQCResult().GetIntensityMotionCheckResult()[myIntensityThread.m_IntensityMotionCheck->
                                                            get_Original_ForcedConformance_Mapping()
                                                            [ind].index_original[0]].VisualChecking;
      }
    VC_Status.push_back( vc );
    // std::cout << "VC_Status" << ind  << vc.VC_status << std::endl;

    }

}

void IntensityMotionCheckPanel::Set_Original_ForcedConformance_Mapping()
{
  // std::cout << "GetDwiOutputImage()->GetVectorLength()" << GetDwiOutputImage()->GetVectorLength() << std::endl;
  for( unsigned int i = 0; i < GetDwiOutputImage()->GetVectorLength(); i++ )
    {
    m_Original_ForcedConformance_Mapping item;
    item.index_ForcedConformance = i;
    std::vector<int> Bing;
    for( unsigned int k = 0; k < qcResult.GetIntensityMotionCheckResult().size(); k++ )
      {
      if( static_cast<unsigned int>(qcResult.GetIntensityMotionCheckResult()[k].QCIndex) == i )
        {
        Bing.push_back( k );
        // std::cout << "i: " << i << "qcResult.GetIntensityMotionCheckResult()[k].QCIndex" <<
        // qcResult.GetIntensityMotionCheckResult()[k].QCIndex << std::endl;
        }
      }
    item.index_original = Bing;
    // std::cout << "Bing" << Bing[0] << std::endl;
    t_Original_ForcedConformance_Mapping.push_back( item );
    }

}

void IntensityMotionCheckPanel::Set_QCedDWI()
{
  SetDwiOutputImage( myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage() );

}

void IntensityMotionCheckPanel::Set_init_Path()
{
  lineEdit_FSL->setText(QString::fromStdString(this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_FSL) );
}

/*void IntensityMotionCheckPanel::Update_ProtocolPath()
{
  this->GetTreeWidgetProtocol()->topLevelItem(14)->child(1)->setText( 1, lineEdit_FSL->text() );
}
*/

void IntensityMotionCheckPanel::QCedResultUpdate()
{

  // creating QCResult tree include "gradients info + visual checking process" :"
  treeWidget_Results->clear();

  // gradient
  QTreeWidgetItem *itemIntensityMotionInformation = new QTreeWidgetItem( treeWidget_Results );
  itemIntensityMotionInformation->setText( 0, tr("DWI Check") );

  // Obtaing directions of gradients of QCed dwi
  //
  // ..............................................................................................................................................
  itk::MetaDataDictionary                  imgMetaDictionary = GetDwiOutputImage()->GetMetaDataDictionary();
  std::vector<std::string>                 imgMetaKeys = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string                              metaString;

  std::ofstream outfile;
  // outfile.open( myIntensityThread.m_IntensityMotionCheck->GetReportFileName().c_str(), std::ios_base::app |
  // std::ios_base::out);
  // outfile << "================================" << std::endl;
  // outfile << "Included Gradients" << std::endl;
  // outfile << "================================" << std::endl;

  // gradient vectors
  TensorReconstructionImageFilterType::GradientDirectionType vect3d_T;
  GradientDirectionContainer_ConformanceImg = GradientDirectionContainerType::New();
  int index_included_dwi = 0;
  // outfile << std::endl << "\t#" << "\tDirVector" << std::endl;
  for( ; itKey != imgMetaKeys.end(); itKey++ )
    {
    // double x,y,z;
    itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
    if( itKey->find("DWMRI_gradient") != std::string::npos )
      {
      std::istringstream iss(metaString);
      iss >> vect3d_T[0] >> vect3d_T[1] >> vect3d_T[2];
      // outfile << "\t" << index_included_dwi << "\t[ "
      //                << std::setw(9) << std::setiosflags(std::ios::fixed)
      //                << std::setprecision(6) << std::setiosflags(std::ios::right)
      //                << vect3d_T[0] << ", "
      //               << std::setw(9) << std::setiosflags(std::ios::fixed)
      //                << std::setprecision(6) << std::setiosflags(std::ios::right)
      //                << vect3d_T[1] << ", "
      //                << std::setw(9) << std::setiosflags(std::ios::fixed)
      //                << std::setprecision(6) << std::setiosflags(std::ios::right)
      //                << vect3d_T[2] << " ]"
      //                << std::endl;
      std::cout << "gradients dir: " << vect3d_T[0] << " " << vect3d_T[1] << " " << vect3d_T[2] << std::endl;
      index_included_dwi++;
      GradientDirectionContainer_ConformanceImg->push_back(vect3d_T);
      }
    }
  //
  // ..................................................................................................................................................
  for( unsigned int i = 0; i < t_Original_ForcedConformance_Mapping.size(); i++ )
    {

    QTreeWidgetItem *gradient = new QTreeWidgetItem(
        itemIntensityMotionInformation);

    gradient->setText( 0, QString("gradient_%1").arg( i, 4, 10, QLatin1Char( '0' ) ) );

    QTreeWidgetItem *itemGradientDir = new QTreeWidgetItem(gradient);
    itemGradientDir->setText( 0, tr("Dir") );
    itemGradientDir->setText(1, QString("%1 %2 %3")
                             .arg(GradientDirectionContainer_ConformanceImg->ElementAt(i)[0], 0,
                                  'f', 6)
                             .arg(GradientDirectionContainer_ConformanceImg->ElementAt(i)[1], 0,
                                  'f', 6)
                             .arg(GradientDirectionContainer_ConformanceImg->ElementAt(i)[2], 0,
                                  'f', 6)
                             );

    QTreeWidgetItem * itemVisualCheck = new QTreeWidgetItem(gradient);  // item for visual gradient checking
    itemVisualCheck->setText(0, tr("Visual Check") );
    QTreeWidgetItem * itemVisualCheck_Satus = new QTreeWidgetItem(itemVisualCheck);
    itemVisualCheck_Satus->setText( 0, QString("VC_Status_%1").arg( i, 4, 10, QLatin1Char( '0' ) ) );

    if( i == 0 )
      {
      for( unsigned int k = 0; k < t_Original_ForcedConformance_Mapping[0].index_original.size(); k++ )
        {
        if( qcResult.GetIntensityMotionCheckResult()[(t_Original_ForcedConformance_Mapping[i].index_original)[k]].
            VisualChecking == 0 )
          {
          itemVisualCheck_Satus->setText( 1, tr("Include" ) );
          }
        if( qcResult.GetIntensityMotionCheckResult()[(t_Original_ForcedConformance_Mapping[i].index_original)[k]].
            VisualChecking == 6 )
          {
          itemVisualCheck_Satus->setText( 1, tr("Exclude" ) );
          }
        if( qcResult.GetIntensityMotionCheckResult()[(t_Original_ForcedConformance_Mapping[i].index_original)[k]].
            VisualChecking == -1 )
          {
          itemVisualCheck_Satus->setText( 1, tr("NoChange" ) );
          }
        QTreeWidgetItem * itemQCIndex = new QTreeWidgetItem(gradient); // item for showing the mapping to the QCed dwi
                                                                       // image
        itemQCIndex->setText(0, tr("Original_Index") );
        itemQCIndex->setText(1, QString("%1").arg( (t_Original_ForcedConformance_Mapping[0].index_original)[k]) );
        }
      }
    else
      {
      if( qcResult.GetIntensityMotionCheckResult()[(t_Original_ForcedConformance_Mapping[i].index_original)[0]].
          VisualChecking == 0 )
        {
        itemVisualCheck_Satus->setText( 1, tr("Include" ) );
        }
      if( qcResult.GetIntensityMotionCheckResult()[(t_Original_ForcedConformance_Mapping[i].index_original)[0]].
          VisualChecking == 6 )
        {
        itemVisualCheck_Satus->setText( 1, tr("Exclude" ) );
        }
      if( qcResult.GetIntensityMotionCheckResult()[(t_Original_ForcedConformance_Mapping[i].index_original)[0]].
          VisualChecking == -1 )
        {
        itemVisualCheck_Satus->setText( 1, tr("NoChange" ) );
        }
      QTreeWidgetItem * itemQCIndex = new QTreeWidgetItem(gradient); // item for showing the mapping to the QCed dwi
                                                                     // image
      itemQCIndex->setText(0, tr("Original_Index") );
      itemQCIndex->setText(1, QString("%1").arg( (t_Original_ForcedConformance_Mapping[i].index_original)[0] ) );
      }

    }

  // outfile.close();
}

void IntensityMotionCheckPanel::ResultUpdate()
{
  // creating the entire QCResult tree :)

  bResultTreeEditable = false;
  treeWidget_Results->clear();
  QTreeWidgetItem *itemImageInformation = new QTreeWidgetItem(
      treeWidget_Results);
  itemImageInformation->setText( 0, tr("ImageInformation") );
  QTreeWidgetItem *itemDiffusionInformation = new QTreeWidgetItem(
      treeWidget_Results);
  itemDiffusionInformation->setText( 0, tr("DiffusionInformation") );
  QTreeWidgetItem *itemIntensityMotionInformation = new QTreeWidgetItem(
      treeWidget_Results);
  itemIntensityMotionInformation->setText( 0, tr("DWI Check") );

  // ImageInformationCheckResult
  QTreeWidgetItem *FileName = new QTreeWidgetItem( itemImageInformation );
  FileName->setText( 0, tr("file name") );
  FileName->setText( 1, DwiName );
  qcResult.GetImageInformationCheckResult(). info = DwiName;
  if( protocol.GetImageProtocol().bCheck )
    {
    if( ( this->GetProtocol().GetImageProtocol().bQuitOnCheckSizeFailure &&
          ( (qcResult.Get_result()  & ImageCheckBit) !=  0) ) ||
        (    this->GetProtocol().GetImageProtocol().bQuitOnCheckSpacingFailure &&
             ( (qcResult.Get_result()  & ImageCheckBit) !=  0) ) )
      {
      itemImageInformation->setText( 1, tr("Fail Pipeline Terminated") );
      itemImageInformation->setText( 2, tr("Finish QC Processing") );
      return;
      }

    QTreeWidgetItem *origin = new QTreeWidgetItem(itemImageInformation);
    origin->setText( 0, tr("origin") );
    if( qcResult.GetImageInformationCheckResult().origin )
      {
      origin->setText( 1, tr("Pass") );
      }
    else
      {
      origin->setText( 1, tr("Failed") );
      }

    QTreeWidgetItem *sizeLocal = new QTreeWidgetItem(itemImageInformation);
    sizeLocal->setText( 0, tr("size") );
    if( qcResult.GetImageInformationCheckResult().size )
      {
      sizeLocal->setText( 1, tr("Pass") );
      }
    else
      {
      sizeLocal->setText( 1, tr("Failed") );
      }
    QTreeWidgetItem *space = new QTreeWidgetItem(itemImageInformation);
    space->setText( 0, tr("space") );
    if( qcResult.GetImageInformationCheckResult().space )
      {
      space->setText( 1, tr("Pass") );
      }
    else
      {
      space->setText( 1, tr("Failed") );
      }
    QTreeWidgetItem *spacedirection = new QTreeWidgetItem(itemImageInformation);
    spacedirection->setText( 0, tr("spacedirection") );
    if( qcResult.GetImageInformationCheckResult().spacedirection )
      {
      spacedirection->setText( 1, tr("Pass") );
      }
    else
      {
      spacedirection->setText( 1, tr("Failed") );
      }

    QTreeWidgetItem *spacing = new QTreeWidgetItem(itemImageInformation);
    spacing->setText( 0, tr("spacing") );
    if( qcResult.GetImageInformationCheckResult().spacing )
      {
      spacing->setText( 1, tr("Pass") );
      }
    else
      {
      spacing->setText( 1, tr("Failed") );
      }

    }

  else if( !this->GetProtocol().GetImageProtocol().bCheck )
    {
    itemImageInformation->setText( 1, tr("Info NOT check") );
    }

  if( protocol.GetDiffusionProtocol().bCheck )
    {
    // DiffusionInformationCheckResult
    if( this->GetProtocol().GetDiffusionProtocol().bQuitOnCheckFailure &&
        ( (qcResult.Get_result()  & DiffusionCheckBit) !=  0) )
      {
      itemDiffusionInformation->setText( 1, tr("Fail Pipeline Terminated") );
      itemDiffusionInformation->setText( 2, tr("Finish QC Processing") );
      return;
      }

    QTreeWidgetItem *b = new QTreeWidgetItem(itemDiffusionInformation);
    b->setText( 0, tr("b value") );
    if( qcResult.GetDiffusionInformationCheckResult().b )
      {
      b->setText( 1, tr("Pass") );
      }
    else
      {
      b->setText( 1, tr("Failed") );
      }

    QTreeWidgetItem *gradient = new QTreeWidgetItem(itemDiffusionInformation);
    gradient->setText( 0, tr("gradient") );
    if( qcResult.GetDiffusionInformationCheckResult().gradient )
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
    if( qcResult.GetDiffusionInformationCheckResult().measurementFrame )
      {
      measurementFrame->setText( 1, tr("Pass") );
      }
    else
      {
      measurementFrame->setText( 1, tr("Failed") );
      }

    }
  else if( !this->GetProtocol().GetDiffusionProtocol().bCheck )
    {
    itemDiffusionInformation->setText( 1, tr("Info NOT check") );
    }

  // itemIntensityMotionInformation
  QTreeWidgetItem *overallSliceWiseCheck = new QTreeWidgetItem(itemIntensityMotionInformation);
  QTreeWidgetItem *overallInterlaceWiseCheck = new QTreeWidgetItem(itemIntensityMotionInformation);
  QTreeWidgetItem *overallGradientWiseCheck = new QTreeWidgetItem(itemIntensityMotionInformation);
  overallSliceWiseCheck->setText( 0, tr("SliceWiseCheck") );
  overallInterlaceWiseCheck->setText( 0, tr("InterlaceWiseCheck") );
  overallGradientWiseCheck->setText( 0, tr("GradientWiseCheck") );

  if( this->GetProtocol().GetSliceCheckProtocol().bCheck )    // Check protocol whether run SliceWiseChecking
    {
    if( (qcResult.Get_result()  & SliceWiseCheckBit) == SliceWiseCheckBit )
      {
      if( this->GetProtocol().GetSliceCheckProtocol().bQuitOnCheckFailure )
        {
        overallSliceWiseCheck->setText( 1, tr("Fail Pipeline Termination") );
        overallInterlaceWiseCheck->setText( 1, tr("NA") );
        overallGradientWiseCheck->setText( 1, tr("NA") );
        }
      else
        {
        f_overallSliceWiseCheck();
        if( qcResult.GetOverallQCResult().SWCk == 0 )
          {
          overallSliceWiseCheck->setText( 1, tr("Fail") );
          }
        else
          {
          overallSliceWiseCheck->setText( 1, tr("Pass") );
          }
        }
      }
    else
      {
      f_overallSliceWiseCheck();

      if( qcResult.GetOverallQCResult().SWCk == 0 )
        {
        overallSliceWiseCheck->setText( 1, tr("Fail") );
        }
      else
        {
        overallSliceWiseCheck->setText( 1, tr("Pass") );
        }
      }

    }
  else if( !this->GetProtocol().GetSliceCheckProtocol().bCheck )
    {
    overallSliceWiseCheck->setText( 1, tr("Not Set") );
    }

  if( !( (qcResult.Get_result()  & SliceWiseCheckBit) == SliceWiseCheckBit &&
         this->GetProtocol().GetSliceCheckProtocol().bQuitOnCheckFailure) &&
      this->GetProtocol().GetInterlaceCheckProtocol().bCheck )
    {

    if( (qcResult.Get_result() & InterlaceWiseCheckBit) == 0 )
      {
      f_overallInterlaceWiseCheck();
      if( qcResult.GetOverallQCResult().IWCk == 0 )
        {
        overallInterlaceWiseCheck->setText( 1, tr("Fail") );
        }
      else
        {
        overallInterlaceWiseCheck->setText( 1, tr("Pass") );
        }
      }
    else
      {
      if( this->GetProtocol().GetInterlaceCheckProtocol().bQuitOnCheckFailure )
        {
        overallInterlaceWiseCheck->setText( 1, tr("Fail Pipeline Termination") );
        overallGradientWiseCheck->setText( 1, tr("NA") );
        }
      else
        {
        f_overallInterlaceWiseCheck();
        if( qcResult.GetOverallQCResult().IWCk == 0 )
          {
          overallInterlaceWiseCheck->setText( 1, tr("Fail") );
          }
        else
          {
          overallInterlaceWiseCheck->setText( 1, tr("Pass") );
          }
        }
      }
    }
  else if( !this->GetProtocol().GetInterlaceCheckProtocol().bCheck )
    {
    overallInterlaceWiseCheck->setText( 1, tr("Not Set") );
    }

  if( ( ( (!qcResult.Get_result()  & InterlaceWiseCheckBit) == InterlaceWiseCheckBit ) ||
        (!(this->GetProtocol().GetSliceCheckProtocol().bQuitOnCheckFailure ||
           this->GetProtocol().GetInterlaceCheckProtocol().bQuitOnCheckFailure) ) ) &&
      this->GetProtocol().GetGradientCheckProtocol().bCheck )
    {
    if( (qcResult.Get_result() & GradientWiseCheckBit) == 0 )
      {
      f_overallGradientWiseCheck();
      if( qcResult.GetOverallQCResult().GWCk == 0 )
        {
        overallGradientWiseCheck->setText( 1, tr("Fail") );
        }
      else
        {
        overallGradientWiseCheck->setText( 1, tr("Pass") );
        }
      }
    else
      {
      if( this->GetProtocol().GetGradientCheckProtocol().bQuitOnCheckFailure )
        {
        overallGradientWiseCheck->setText( 1, tr("Fail Pipeline Termination") );
        }
      else
        {
        f_overallGradientWiseCheck();
        if( qcResult.GetOverallQCResult().GWCk == 0 )
          {
          overallGradientWiseCheck->setText( 1, tr("Fail") );
          }
        else
          {
          overallGradientWiseCheck->setText( 1, tr("Pass") );
          }
        }
      }
    }
  else if( !this->GetProtocol().GetGradientCheckProtocol().bCheck )
    {
    overallGradientWiseCheck->setText( 1, tr("Not Set") );
    }
  for( unsigned int i = 0;
       i < qcResult.GetIntensityMotionCheckResult().size();
       i++ )
    {

    // gradient
    bool EXCLUDE_SliceWiseCheck = false;
    bool EXCLUDE_InterlaceWiseCheck = false;
    bool EXCLUDE_GreadientWiseCheck = false;

    QTreeWidgetItem *gradient = new QTreeWidgetItem(
        itemIntensityMotionInformation);

    // gradient->setText(0, tr("gradient ")+QString::number(i));
    gradient->setText( 0,
                       QString("gradient_%1").arg( i, 4, 10, QLatin1Char( '0' ) ) );

    // std::cout<<"1:ResultUpdate()"<<std::endl;
    switch( qcResult.GetIntensityMotionCheckResult()[i].processing )
      {
      case QCResult::GRADIENT_BASELINE_AVERAGED:
        gradient->setText( 2, tr("BASELINE_AVERAGED") );
        break;
      case QCResult::GRADIENT_EXCLUDE_SLICECHECK:
        {
        gradient->setText( 2, tr("EXCLUDE_SLICECHECK") );
        EXCLUDE_SliceWiseCheck = true;
        }
        break;
      case QCResult::GRADIENT_EXCLUDE_INTERLACECHECK:
        {
        gradient->setText( 2, tr("EXCLUDE_INTERLACECHECK") );
        EXCLUDE_InterlaceWiseCheck = true;
        }
        break;
      case QCResult::GRADIENT_EXCLUDE_GRADIENTCHECK:
        {
        gradient->setText( 2, tr("EXCLUDE_GRADIENTCHECK") );
        EXCLUDE_GreadientWiseCheck = true;
        }
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

    QTreeWidgetItem * itemSliceWiseCheck = new QTreeWidgetItem(gradient);
    itemSliceWiseCheck->setText( 0, tr("SliceWiseCheck") );

    QTreeWidgetItem * itemInterlaceWiseCheck = new QTreeWidgetItem(gradient);
    itemInterlaceWiseCheck->setText( 0, tr("InterlaceWiseCheck") );

    QTreeWidgetItem * itemGradientWiseCheck = new QTreeWidgetItem(gradient);
    itemGradientWiseCheck->setText( 0, tr("GradientWiseCheck") );

    if( bLoadDefaultQC == false )
      {
      // if ( (myIntensityThread.Get_result() & 4) == 0 ){
      if( this->GetProtocol().GetSliceCheckProtocol().bCheck )
        {
        if( EXCLUDE_SliceWiseCheck == true )
          {
          itemSliceWiseCheck->setText( 2, tr("EXCLUDE") );
          for( unsigned int S_index = 0; S_index < qcResult.GetSliceWiseCheckResult().size(); S_index++ )
            {
            if( i == static_cast<unsigned int>(qcResult.GetSliceWiseCheckResult()[S_index].GradientNum) )
              {
              QTreeWidgetItem * itemSliceNum = new QTreeWidgetItem(itemSliceWiseCheck);
              itemSliceNum->setText(0, tr("Slice#") );
              itemSliceNum->setText(1, QString("%1").arg(qcResult.GetSliceWiseCheckResult()[S_index].SliceNum) );
              QTreeWidgetItem * itemCorrelation = new QTreeWidgetItem(itemSliceWiseCheck);
              itemCorrelation->setText(0, tr("Correlation") );
              itemCorrelation->setText(1, QString("%1").arg(qcResult.GetSliceWiseCheckResult()[S_index].Correlation) );
              }

            }
          }
        else
          {
          itemSliceWiseCheck->setText( 2, tr("INCLUDE") );
          }
        }
      if( !( (qcResult.Get_result()  & SliceWiseCheckBit) == SliceWiseCheckBit &&
             this->GetProtocol().GetSliceCheckProtocol().bQuitOnCheckFailure) )
        {
        if( this->GetProtocol().GetInterlaceCheckProtocol().bCheck )
          {
          if( EXCLUDE_InterlaceWiseCheck == true )
            {
            itemInterlaceWiseCheck->setText( 2, tr("EXCLUDE") );
            }
          else
          if( EXCLUDE_SliceWiseCheck == true )
            {
            itemInterlaceWiseCheck->setText( 2, tr("NA") );
            }
          else
            {
            itemInterlaceWiseCheck->setText( 2, tr("INCLUDE") );
            }
          QTreeWidgetItem * itemInterlaceAngleX = new QTreeWidgetItem(itemInterlaceWiseCheck);
          itemInterlaceAngleX->setText( 0, tr("InterlaceAngleX") );
          itemInterlaceAngleX->setText(1, QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].AngleX) );

          QTreeWidgetItem * itemInterlaceAngleY = new QTreeWidgetItem(itemInterlaceWiseCheck);
          itemInterlaceAngleY->setText( 0, tr("InterlaceAngleY") );
          itemInterlaceAngleY->setText(1, QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].AngleY) );
          QTreeWidgetItem * itemInterlaceAngleZ = new QTreeWidgetItem(itemInterlaceWiseCheck);
          itemInterlaceAngleZ->setText( 0, tr("InterlaceAngleZ") );
          itemInterlaceAngleZ->setText(1, QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].AngleZ) );
          QTreeWidgetItem * itemInterlaceTranslationX = new QTreeWidgetItem(itemInterlaceWiseCheck);
          itemInterlaceTranslationX->setText( 0, tr("InterlaceTranslationX") );
          itemInterlaceTranslationX->setText(1,
                                             QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].TranslationX) );
          QTreeWidgetItem * itemInterlaceTranslationY = new QTreeWidgetItem(itemInterlaceWiseCheck);
          itemInterlaceTranslationY->setText( 0, tr("InterlaceTranslationY") );
          itemInterlaceTranslationY->setText(1,
                                             QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].TranslationY) );
          QTreeWidgetItem * itemInterlaceTranslationZ = new QTreeWidgetItem(itemInterlaceWiseCheck);
          itemInterlaceTranslationZ->setText( 0, tr("InterlaceTranslationZ") );
          itemInterlaceTranslationZ->setText(1,
                                             QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].TranslationZ) );
          QTreeWidgetItem * itemInterlaceMetric = new QTreeWidgetItem(itemInterlaceWiseCheck);
          itemInterlaceMetric->setText( 0, tr("InterlaceMetric(MI)") );
          itemInterlaceMetric->setText(1, QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].Metric) );
          QTreeWidgetItem * itemInterlaceCorrelation = new QTreeWidgetItem(itemInterlaceWiseCheck);

          if( i == 0 ) // baseline
            {
            itemInterlaceCorrelation->setText( 0, tr("InterlaceCorrelation_Baseline") );
            }
          else
            {
            itemInterlaceCorrelation->setText( 0, tr("InterlaceCorrelation") );
            }

          itemInterlaceCorrelation->setText(1, QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].Correlation) );
          }
        if( ( (!( (qcResult.Get_result()  & InterlaceWiseCheckBit) == InterlaceWiseCheckBit) ) ||
              (!(this->GetProtocol().GetSliceCheckProtocol().bQuitOnCheckFailure ||
                 this->GetProtocol().GetInterlaceCheckProtocol().bQuitOnCheckFailure) ) ) )
          {
          if( this->GetProtocol().GetGradientCheckProtocol().bCheck )
            {
            if( EXCLUDE_GreadientWiseCheck == true )
              {
              itemGradientWiseCheck->setText( 2, tr("EXCLUDE") );
              }
            else
            if( EXCLUDE_SliceWiseCheck == true || EXCLUDE_InterlaceWiseCheck == true )
              {
              itemGradientWiseCheck->setText( 2, tr("NA") );
              }
            else
              {
              itemGradientWiseCheck->setText( 2, tr("INCLUDE") );
              }
            QTreeWidgetItem * itemGradientAngleX = new QTreeWidgetItem(itemGradientWiseCheck);
            itemGradientAngleX->setText( 0, tr("GradientAngleX") );
            itemGradientAngleX->setText(1, QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].AngleX) );
            QTreeWidgetItem * itemGradientAngleY = new QTreeWidgetItem(itemGradientWiseCheck);
            itemGradientAngleY->setText( 0, tr("GradientAngleY") );
            itemGradientAngleY->setText(1, QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].AngleY) );
            QTreeWidgetItem * itemGradientAngleZ = new QTreeWidgetItem(itemGradientWiseCheck);
            itemGradientAngleZ->setText( 0, tr("GradientAngleZ") );
            itemGradientAngleZ->setText(1, QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].AngleZ) );
            QTreeWidgetItem * itemGradientTranslationX = new QTreeWidgetItem(itemGradientWiseCheck);
            itemGradientTranslationX->setText( 0, tr("GradientTranslationX") );
            itemGradientTranslationX->setText(1,
                                              QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].TranslationX) );
            QTreeWidgetItem * itemGradientTranslationY = new QTreeWidgetItem(itemGradientWiseCheck);
            itemGradientTranslationY->setText( 0, tr("GradientTranslationY") );
            itemGradientTranslationY->setText(1,
                                              QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].TranslationY) );
            QTreeWidgetItem * itemGradientTranslationZ = new QTreeWidgetItem(itemGradientWiseCheck);
            itemGradientTranslationZ->setText( 0, tr("GradientTranslationZ") );
            itemGradientTranslationZ->setText(1,
                                              QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].TranslationZ) );
            QTreeWidgetItem * itemGradientMetric = new QTreeWidgetItem(itemGradientWiseCheck);
            itemGradientMetric->setText( 0, tr("GradientMetric(MI)") );
            itemGradientMetric->setText(1, QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].MutualInformation) );
            }
          }
        else
          {
          itemGradientWiseCheck->setText( 2, tr("NA") );
          }
        }
      // }
      else
        {
        itemInterlaceWiseCheck->setText( 2, tr("NA") );
        itemGradientWiseCheck->setText( 2, tr("NA") );
        }

      QTreeWidgetItem * itemQCIndex = new QTreeWidgetItem(gradient); // item for showing the mapping to the QCed dwi
                                                                     // image
      itemQCIndex->setText(0, tr("QC_Index") );
      itemQCIndex->setText(1, QString("%1").arg(qcResult.GetIntensityMotionCheckResult()[i].QCIndex) );

      QTreeWidgetItem * itemVisualCheck = new QTreeWidgetItem(gradient); // item for visual gradient checking
      itemVisualCheck->setText(0, tr("Visual Check") );
      QTreeWidgetItem * itemVisualCheck_Satus = new QTreeWidgetItem(itemVisualCheck);
      itemVisualCheck_Satus->setText( 0, QString("VC_Status_%1").arg( i, 4, 10, QLatin1Char( '0' ) ) );
      if( qcResult.GetIntensityMotionCheckResult()[i].VisualChecking == 0 )
        {
        itemVisualCheck_Satus->setText( 1, tr("Include" ) );
        }
      if( qcResult.GetIntensityMotionCheckResult()[i].VisualChecking == 6 )
        {
        itemVisualCheck_Satus->setText( 1, tr("Exclude" ) );
        }
      if( qcResult.GetIntensityMotionCheckResult()[i].VisualChecking == -1 )
        {
        itemVisualCheck_Satus->setText( 1, tr("NoChange" ) );
        }

      }

    if( bLoadDefaultQC == true )
      {
      QTreeWidgetItem * itemQCIndex = new QTreeWidgetItem(gradient); // item for showing the mapping to the QCed dwi
                                                                     // image
      itemQCIndex->setText(0, tr("QC_Index") );
      itemQCIndex->setText(1, QString("%1").arg(qcResult.GetIntensityMotionCheckResult()[i].QCIndex) );

      QTreeWidgetItem * itemVisualCheck = new QTreeWidgetItem(gradient); // item for visual gradient checking
      itemVisualCheck->setText(0, tr("Visual Check") );
      QTreeWidgetItem * itemVisualCheck_Satus = new QTreeWidgetItem(itemVisualCheck);
      itemVisualCheck_Satus->setText( 0, QString("VC_Status_%1").arg( i, 4, 10, QLatin1Char( '0' ) ) );
      itemVisualCheck_Satus->setText( 1, tr("NoChange" ) );
      }

    }

  QTreeWidgetItem *itemBrainMask = new QTreeWidgetItem(
      treeWidget_Results);
  itemBrainMask->setText(0, tr("BRAIN_MASK") );

  if( ( this->GetProtocol().GetBrainMaskProtocol().bQuitOnCheckFailure &&
        ( (qcResult.Get_result()  & BrainMaskBit) !=  0) ) )
    {
    itemBrainMask->setText( 1, tr("Fail Pipeline Terminated") );
    itemBrainMask->setText( 2, tr("Finish QC Processing") );
    return;
    }

  if( !this->GetProtocol().GetBrainMaskProtocol().bMask )
    {
    itemBrainMask->setText(1, tr("Not Set") );
    }
  else
    {
    if( qcResult.GetOverallQCResult().BMCK == true )
      {
      itemBrainMask->setText(1, tr("Pass") );
      }
    else
      {
      itemBrainMask->setText(1, tr("Fail") );
      }

    }

  std::cout << "QTree Result " << qcResult.GetOverallQCResult().BMCK << std::endl;
  std::cout << "QTree Result " << qcResult.GetOverallQCResult().DDDCK << std::endl;

  // QTreeWidgetItem *itemDominantDirectionDetec = new QTreeWidgetItem(
  //      treeWidget_Results);
  QTreeWidgetItem *itemDominantDirectionDetec = new QTreeWidgetItem(
      treeWidget_Results);
  itemDominantDirectionDetec->setText(0, tr("Dominant_Direction_Detector") );

  if( ( this->GetProtocol().GetDominantDirectional_Detector().bQuitOnCheckFailure &&
        ( (qcResult.Get_result()  & DominantDirectionDetectBit) !=  0) ) )
    {
    itemDominantDirectionDetec->setText( 1, tr("Fail Pipeline Terminated") );
    itemDominantDirectionDetec->setText( 2, tr("Finish QC Processing") );
    return;
    }

  if( !this->GetProtocol().GetDominantDirectional_Detector().bCheck )
    {
    itemDominantDirectionDetec->setText(1, tr("Not Set") );
    }
  else
    {

    if( qcResult.GetOverallQCResult().DDDCK == true )
      {
      itemDominantDirectionDetec->setText(1, tr("Pass") );
      QTreeWidgetItem *itemDominantDirectionDetec_Z_score = new QTreeWidgetItem(
          itemDominantDirectionDetec);
      itemDominantDirectionDetec_Z_score->setText(0, tr("DOMINANT_DIRECTION_Z_SCORE") );
      itemDominantDirectionDetec_Z_score->setText(1, QString("%1").arg(qcResult.GetDominantDirection_Detector().z_score) );

      QTreeWidgetItem *itemDominantDirectionDetec_Entropy = new QTreeWidgetItem(
          itemDominantDirectionDetec);
      itemDominantDirectionDetec_Entropy->setText(0, tr("DOMINANT_DIRECTION_ENTROPY") );
      itemDominantDirectionDetec_Entropy->setText(1,
                                                  QString("%1").arg(qcResult.GetDominantDirection_Detector().
                                                                    entropy_value) );

      QTreeWidgetItem *itemDominantDirectionDetec_Result = new QTreeWidgetItem(
          itemDominantDirectionDetec);
      itemDominantDirectionDetec_Result->setText(0, tr("DOMINANT_DIRECTION_RESULT") );

      if( qcResult.GetDominantDirection_Detector().detection_result == 2 )
        {
        itemDominantDirectionDetec_Result->setText( 1, tr("Reject") );
        }

      if( qcResult.GetDominantDirection_Detector().detection_result == 1 )
        {
        itemDominantDirectionDetec_Result->setText( 1, tr("Suspicious") );
        }

      if( qcResult.GetDominantDirection_Detector().detection_result == 0 )
        {
        itemDominantDirectionDetec_Result->setText( 1, tr("Accept") );
        }
      }
    else
      {
      itemDominantDirectionDetec->setText(1, tr("Fail") );
      }

    }

  // QTreeWidgetItem * itemSliceWiseCheck = new QTreeWidgetItem(treeWidget_Results);
  // itemSliceWiseCheck->setText( 0, tr("SLiceWiseCheck"));

  // for (int index=0; index< qcResult.GetSliceWiseCheckResult().size(); index ++)
  // {
  // QTreeWidgetItem * itemGradientNum = new QTreeWidgetItem(itemSliceWiseCheck);
  // itemGradientNum->setText(0, tr("Gradien#"));
  // itemGradientNum->setForeground(1,greenText);
  // itemGradientNum->setText(1,QString("%1").arg(qcResult.GetSliceWiseCheckResult()[index].GradientNum));
  // QTreeWidgetItem * itemSliceNum = new QTreeWidgetItem(itemSliceWiseCheck);
  // itemSliceNum->setText(0, tr("Slice#"));
  // itemSliceNum->setText(1,QString("%1").arg(qcResult.GetSliceWiseCheckResult()[index].SliceNum));
  // QTreeWidgetItem * itemCorrelation = new QTreeWidgetItem(itemSliceWiseCheck);
  // itemCorrelation->setText(0, tr("Correlation"));
  // itemCorrelation->setText(1,QString("%1").arg(qcResult.GetSliceWiseCheckResult()[index].Correlation));
  // }

  bResultTreeEditable = false; // no edit to automatically generated results
  // pushButton_SaveDWIAs->setEnabled( 0 );

  emit UpdateOutputDWIDiffusionVectorActors();
  emit LoadQCResult(true);

  if( bLoadDefaultQC )
    {

    QString     Grad2 = QString(" Do you want to save QCed result.");
    QMessageBox msgBox;
    msgBox.setText( Grad2 );
    QPushButton * YES = msgBox.addButton( tr("Yes"), QMessageBox::ActionRole);
    QPushButton * NO = msgBox.addButton( tr("No"), QMessageBox::ActionRole);
    msgBox.exec();

    if( msgBox.clickedButton() == YES )
      {

      SavingTreeWidgetResult_XmlFile();

      emit SignalActivateSphere(); // Activate "actionIncluded" bottom
      pushButton_SaveVisualChecking->setEnabled( 1 );
      return;

      }
    if( msgBox.clickedButton() == NO )
      {
      return;
      }

    // SavingTreeWidgetResult_XmlFile_Default();
    }

  SavingTreeWidgetResult_XmlFile();
  // }
  emit SignalActivateSphere(); // Activate "actionIncluded" bottom
  pushButton_SaveVisualChecking->setEnabled( 1 );
  return;
}

void IntensityMotionCheckPanel::SavingTreeWidgetResult_XmlFile_Default()     // Saving the treeWidget_Results in the xml
                                                                             // file format
{
  // QString Result_xmlFile = QFileDialog::getSaveFileName( this, tr(
  // "Save Result As"), lineEdit_Result->text(),  tr("xml Files (*.xml)") );
  QString Result_xmlFile;

  if( protocol.GetQCOutputDirectory().length() > 0 )
    {
    QString str_QCOutputDirectory = QString( protocol.GetQCOutputDirectory().c_str() );
    bool    found_SeparateChar = str_QCOutputDirectory.contains("/");
    if( !found_SeparateChar ) // "/" does not exist in the protocol->GetQCOutputDirectory() and interpreted as the
                              // relative path and creates the folder
      {
      QString Full_path = DwiFilePath;
      QString Full_name = DwiFilePath.section('/', -1);
      Full_path.remove(Full_name);
      Full_path.append( "/" );
      Full_path.append( QString( protocol.GetQCOutputDirectory().c_str() ) );
      if( !QDir( Full_path ).exists() )
        {
        QDir().mkdir( Full_path );
        }
      Full_path.append( "/" );
      Full_path.append( Full_name.section('.', -2, 0) );
      Full_path.append(QString(tr("_XMLQCResult_Default.xml") ) );
      Result_xmlFile = Full_path;
      }
    else
      {
      QString Full_name = DwiFilePath.section('/', -1);
      QString Full_path;
      Full_path.append( QString( protocol.GetQCOutputDirectory().c_str() ) );
      if( !QDir( Full_path ).exists() )
        {
        QDir().mkdir( Full_path );
        }
      Full_path.append( "/" );
      Full_path.append( Full_name );
      Full_path.append(QString(tr("_XMLQCResult_Default.xml") ) );
      Result_xmlFile = Full_path;
      }
    }

  else
    {
    Result_xmlFile = DwiFilePath.section('.', -2, 0);
    Result_xmlFile.remove("_QCed");
    Result_xmlFile.append(QString(tr("_XMLQCResult_Default.xml") ) );
    }

  if( Result_xmlFile.length() > 0 )
    {
    lineEdit_Result->setText(Result_xmlFile);
    XmlStreamWriter XmlWriter(treeWidget_Results);
    XmlWriter.setProtocol(&protocol);
    XmlWriter.writeXml(Result_xmlFile);

    }

}

void IntensityMotionCheckPanel::SavingTreeWidgetResult_XmlFile()     // Saving the treeWidget_Results in the xml file
                                                                     // format
{

  QString Result_xmlFile;

  if( protocol.GetQCOutputDirectory().length() > 0 )
    {

    QString str_QCOutputDirectory = QString(protocol.GetQCOutputDirectory().c_str() );
    bool    found_SeparateChar = str_QCOutputDirectory.contains("/");
    if( !found_SeparateChar ) // "/" does not exist in the protocol->GetQCOutputDirectory() and interpreted as the
                              // relative path and creates the folder
      {

      QString Full_path = DwiFilePath;
      QString Full_name = DwiFilePath.section('/', -1);

      Full_path.remove(Full_name);
      Full_path.append( "/" );
      Full_path.append( QString( protocol.GetQCOutputDirectory().c_str() ) );

      if( !QDir( Full_path ).exists() )
        {
        QDir().mkdir( Full_path );
        }
      Full_path.append( "/" );
      Full_path.append( Full_name.section('.', -2, 0) );
      Full_path.append(QString(tr("_XMLQCResult.xml") ) );
      Result_xmlFile = Full_path;

      }

    else
      {
      QString Full_name = DwiFilePath.section('/', -1);
      QString Full_path;
      Full_path.append( QString( protocol.GetQCOutputDirectory().c_str() ) );
      if( !QDir( Full_path ).exists() )
        {
        QDir().mkdir( Full_path );
        }
      Full_path.append( "/" );
      Full_path.append( Full_name.section('.', -2, 0) );
      Full_path.append(QString(tr("_XMLQCResult.xml") ) );
      Result_xmlFile = Full_path;

      }
    }

  else
    {
    Result_xmlFile = DwiFilePath.section('.', -2, 0);
    Result_xmlFile.remove("_QCed");
    Result_xmlFile.append(QString(tr("_XMLQCResult.xml") ) );
    }

  if( Result_xmlFile.length() > 0 )
    {
    lineEdit_Result->setText(Result_xmlFile);
    XmlStreamWriter XmlWriter(treeWidget_Results);
    XmlWriter.setProtocol(&protocol);
    XmlWriter.writeXml(Result_xmlFile);

    }

}

void IntensityMotionCheckPanel::GenerateCheckOutputImage( DwiImageType::Pointer dwi, const std::string filename)
{

  if( !bDwiLoaded  )
    {
    std::cout << "DWI load error, no Gradient Direction Loaded" << std::endl;
    bGetGradientDirections = false;
    return;
    }

  unsigned int gradientLeft = 0;
  for( unsigned int i = 0;
       i < qcResult.GetIntensityMotionCheckResult().size();
       i++ )
    {
    // Finding the included gradients after QC and Visual Checking
    if( ( (qcResult.GetIntensityMotionCheckResult()[i].processing ==
           QCResult::GRADIENT_INCLUDE || qcResult.GetIntensityMotionCheckResult()[i].processing ==
           QCResult::GRADIENT_BASELINE_AVERAGED || qcResult.GetIntensityMotionCheckResult()[i].processing ==
           QCResult::GRADIENT_EDDY_MOTION_CORRECTED) && qcResult.GetIntensityMotionCheckResult()[i].VisualChecking !=
          QCResult::GRADIENT_EXCLUDE_MANUALLY ) || qcResult.GetIntensityMotionCheckResult()[i].VisualChecking ==
        QCResult::GRADIENT_INCLUDE )
      {
      gradientLeft++;
      }
    }

  std::cout << "gradientLeft: " << gradientLeft << std::endl;

  if( bProtocol )
    {
    if( 1.0
        - (float)( (float)gradientLeft
                   / (float)qcResult.GetIntensityMotionCheckResult().size() ) >=
        this->protocol.GetBadGradientPercentageTolerance() )
      {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, tr("Attention"),
                                    tr(
                                      "Bad gradients number is greater than that in protocol, save anyway?"),
                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
      if( reply == QMessageBox::No || reply == QMessageBox::Cancel )
        {
        return;
        }
      }
    }

  if( gradientLeft == qcResult.GetIntensityMotionCheckResult().size() )
    {
    itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
    try
      {
      DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
      DwiWriter->SetImageIO(myNrrdImageIO);
      DwiWriter->SetFileName( filename );
      DwiWriter->UseInputMetaDataDictionaryOn();
      DwiWriter->SetInput( this->m_DwiOriginalImage );
      DwiWriter->UseCompressionOn();
      DwiWriter->Update();
      }
    catch( itk::ExceptionObject & e )
      {
      std::cout << e.GetDescription() << std::endl;
      return;
      }
    return;
    }

  DwiImageType::Pointer newDwiImage = DwiImageType::New();
  newDwiImage->CopyInformation(dwi);
  newDwiImage->SetRegions( dwi->GetLargestPossibleRegion() );
  newDwiImage->Allocate();
  newDwiImage->SetVectorLength( gradientLeft);

  typedef itk::ImageRegionConstIteratorWithIndex<DwiImageType>
  ConstIteratorType;
  ConstIteratorType oit( dwi, dwi->GetLargestPossibleRegion() );
  typedef itk::ImageRegionIteratorWithIndex<DwiImageType> IteratorType;
  IteratorType nit( newDwiImage, newDwiImage->GetLargestPossibleRegion() );

  oit.GoToBegin();
  nit.GoToBegin();

  DwiImageType::PixelType value;
  value.SetSize( gradientLeft );

  while( !oit.IsAtEnd() )
    {
    int element = 0;
    for( unsigned int i = 0;
         i < qcResult.GetIntensityMotionCheckResult().size();
         i++ )
      {
      if( ( (qcResult.GetIntensityMotionCheckResult()[i].processing ==
             QCResult::GRADIENT_INCLUDE || qcResult.GetIntensityMotionCheckResult()[i].processing ==
             QCResult::GRADIENT_BASELINE_AVERAGED || qcResult.GetIntensityMotionCheckResult()[i].processing ==
             QCResult::GRADIENT_EDDY_MOTION_CORRECTED) && qcResult.GetIntensityMotionCheckResult()[i].VisualChecking !=
            QCResult::GRADIENT_EXCLUDE_MANUALLY ) || qcResult.GetIntensityMotionCheckResult()[i].VisualChecking ==
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

  // wrting MetaDataDictionary of the output dwi image from input metaDataDictionary information
  itk::MetaDataDictionary output_imgMetaDictionary;  // output dwi image dictionary

  itk::MetaDataDictionary  imgMetaDictionary = dwi->GetMetaDataDictionary();
  std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();

  std::string metaString;

  if( imgMetaDictionary.HasKey("NRRD_measurement frame") )
    {
    // Meausurement frame
    std::vector<std::vector<double> > nrrdmf;
    itk::ExposeMetaData<std::vector<std::vector<double> > >(
      imgMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);
    itk::EncapsulateMetaData<std::vector<std::vector<double> > >(
      output_imgMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);
    }

  // modality
  if( imgMetaDictionary.HasKey("modality") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "modality", metaString);
    itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                           "modality",
                                           metaString);
    }

  // b-value
  if( imgMetaDictionary.HasKey("DWMRI_b-value") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "DWMRI_b-value", metaString);
    itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                           "DWMRI_b-value",
                                           metaString);
    }

  // gradient vectors
  int temp = 0;
  for( unsigned int i = 0; i < GradientDirectionContainer->size(); i++ )
    {

    if( ( (qcResult.GetIntensityMotionCheckResult()[i].processing ==
           QCResult::GRADIENT_INCLUDE || qcResult.GetIntensityMotionCheckResult()[i].processing ==
           QCResult::GRADIENT_BASELINE_AVERAGED || qcResult.GetIntensityMotionCheckResult()[i].processing ==
           QCResult::GRADIENT_EDDY_MOTION_CORRECTED) && qcResult.GetIntensityMotionCheckResult()[i].VisualChecking !=
          QCResult::GRADIENT_EXCLUDE_MANUALLY ) || qcResult.GetIntensityMotionCheckResult()[i].VisualChecking ==
        QCResult::GRADIENT_INCLUDE )
      {
      std::ostringstream ossKey;
      ossKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << temp;

      std::ostringstream ossMetaString;
      ossMetaString << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer->ElementAt(i)[0]
                    << "    "
                    << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer->ElementAt(i)[1]
                    << "    "
                    << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer->ElementAt(i)[2];

      // std::cout<<ossKey.str()<<ossMetaString.str()<<std::endl;
      itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                             ossKey.str(), ossMetaString.str() );
      ++temp;
      }
    }

  newDwiImage->SetMetaDataDictionary(output_imgMetaDictionary);

  SetDwiOutputImage(newDwiImage);

  itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
  try
    {
    DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
    DwiWriter->SetImageIO(myNrrdImageIO);
    DwiWriter->SetFileName( filename );
    DwiWriter->UseInputMetaDataDictionaryOn();
    DwiWriter->SetInput(newDwiImage);
    DwiWriter->UseCompressionOn();
    DwiWriter->Update();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cout << e.GetDescription() << std::endl;
    return;
    }
  std::cout << "QC Savd" << std::endl;
}

void IntensityMotionCheckPanel::GenerateCheckOutputImage( const std::string filename)
{
  if( !bDwiLoaded  )
    {
    std::cout << "DWI load error, no Gradient Direction Loaded" << std::endl;
    bGetGradientDirections = false;
    return;
    }

  unsigned int gradientLeft = 0;
  for( unsigned int i = 0;
       i < qcResult.GetIntensityMotionCheckResult().size();
       i++ )
    {
    // Finding the included gradients after QC and Visual Checking
    if( ( (qcResult.GetIntensityMotionCheckResult()[i].processing ==
           QCResult::GRADIENT_INCLUDE || qcResult.GetIntensityMotionCheckResult()[i].processing ==
           QCResult::GRADIENT_BASELINE_AVERAGED || qcResult.GetIntensityMotionCheckResult()[i].processing ==
           QCResult::GRADIENT_EDDY_MOTION_CORRECTED) && qcResult.GetIntensityMotionCheckResult()[i].VisualChecking !=
          QCResult::GRADIENT_EXCLUDE_MANUALLY ) || qcResult.GetIntensityMotionCheckResult()[i].VisualChecking ==
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

  if( bProtocol )
    {
    if( 1.0
        - (float)( (float)gradientLeft
                   / (float)qcResult.GetIntensityMotionCheckResult().size() ) >=
        this->protocol.GetBadGradientPercentageTolerance() )
      {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, tr("Attention"),
                                    tr(
                                      "Bad gradients number is greater than that in protocol, save anyway?"),
                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
      if( reply == QMessageBox::No || reply == QMessageBox::Cancel )
        {
        return;
        }
      }
    }

  if( gradientLeft == qcResult.GetIntensityMotionCheckResult().size() )
    {
    itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
    try
      {
      DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
      DwiWriter->SetImageIO(myNrrdImageIO);
      DwiWriter->SetFileName( filename );
      DwiWriter->UseInputMetaDataDictionaryOn();
      DwiWriter->SetInput( this->m_DwiOriginalImage );
      DwiWriter->UseCompressionOn();
      DwiWriter->Update();
      }
    catch( itk::ExceptionObject & e )
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

  while( !oit.IsAtEnd() )
    {
    int element = 0;
    for( unsigned int i = 0;
         i < qcResult.GetIntensityMotionCheckResult().size();
         i++ )
      {
      if( ( (qcResult.GetIntensityMotionCheckResult()[i].processing ==
             QCResult::GRADIENT_INCLUDE || qcResult.GetIntensityMotionCheckResult()[i].processing ==
             QCResult::GRADIENT_BASELINE_AVERAGED || qcResult.GetIntensityMotionCheckResult()[i].processing ==
             QCResult::GRADIENT_EDDY_MOTION_CORRECTED) && qcResult.GetIntensityMotionCheckResult()[i].VisualChecking !=
            QCResult::GRADIENT_EXCLUDE_MANUALLY ) || qcResult.GetIntensityMotionCheckResult()[i].VisualChecking ==
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

  // wrting MetaDataDictionary of the output dwi image from input metaDataDictionary information
  itk::MetaDataDictionary output_imgMetaDictionary;  // output dwi image dictionary

  itk::MetaDataDictionary  imgMetaDictionary = m_DwiOriginalImage->GetMetaDataDictionary();
  std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();

  std::string metaString;

  if( imgMetaDictionary.HasKey("NRRD_measurement frame") )
    {
    // Meausurement frame
    std::vector<std::vector<double> > nrrdmf;
    itk::ExposeMetaData<std::vector<std::vector<double> > >(
      imgMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);
    itk::EncapsulateMetaData<std::vector<std::vector<double> > >(
      output_imgMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);
    }

  // modality
  if( imgMetaDictionary.HasKey("modality") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "modality", metaString);
    itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                           "modality",
                                           metaString);
    }

  // b-value
  if( imgMetaDictionary.HasKey("DWMRI_b-value") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "DWMRI_b-value", metaString);
    itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                           "DWMRI_b-value",
                                           metaString);
    }

  // gradient vectors
  int temp = 0;
  for( unsigned int i = 0; i < GradientDirectionContainer->size(); i++ )
    {

    if( ( (qcResult.GetIntensityMotionCheckResult()[i].processing ==
           QCResult::GRADIENT_INCLUDE || qcResult.GetIntensityMotionCheckResult()[i].processing ==
           QCResult::GRADIENT_BASELINE_AVERAGED || qcResult.GetIntensityMotionCheckResult()[i].processing ==
           QCResult::GRADIENT_EDDY_MOTION_CORRECTED) && qcResult.GetIntensityMotionCheckResult()[i].VisualChecking !=
          QCResult::GRADIENT_EXCLUDE_MANUALLY ) || qcResult.GetIntensityMotionCheckResult()[i].VisualChecking ==
        QCResult::GRADIENT_INCLUDE )
      {
      std::ostringstream ossKey;
      ossKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << temp;

      std::ostringstream ossMetaString;
      ossMetaString << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer->ElementAt(i)[0]
                    << "    "
                    << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer->ElementAt(i)[1]
                    << "    "
                    << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer->ElementAt(i)[2];

      // std::cout<<ossKey.str()<<ossMetaString.str()<<std::endl;
      itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                             ossKey.str(), ossMetaString.str() );
      ++temp;
      }
    }

  newDwiImage->SetMetaDataDictionary(output_imgMetaDictionary);

  SetDwiOutputImage(newDwiImage);

  itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
  try
    {
    DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
    DwiWriter->SetImageIO(myNrrdImageIO);
    DwiWriter->SetFileName( filename );
    DwiWriter->UseInputMetaDataDictionaryOn();
    DwiWriter->SetInput(newDwiImage);
    DwiWriter->UseCompressionOn();
    DwiWriter->Update();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cout << e.GetDescription() << std::endl;
    return;
    }

  // newDwiImage->Delete();

  // --------------------------------------------- Generating Meta Data Dictionary of Output Image ------------
  /*
  itk::MetaDataDictionary imgMetaDictionary
    = m_DwiOriginalImage->GetMetaDataDictionary();                                            //
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey;
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
      QCResult::GRADIENT_INCLUDE || qcResult.GetIntensityMotionCheckResult()[i].processing ==
      QCResult::GRADIENT_BASELINE_AVERAGED || qcResult.GetIntensityMotionCheckResult()[i].processing ==
      QCResult::GRADIENT_EDDY_MOTION_CORRECTED )
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
  //------------------------------------------------------------------------------------------------------------
  */
  std::cout << " QC Image saved " << std::endl;

}

bool IntensityMotionCheckPanel::GetGradientDirections()
{
  if( !bDwiLoaded )
    {
    LoadDwiImage();
    }
  if( !bDwiLoaded )
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
  for( ; itKey != imgMetaKeys.end(); itKey++ )
    {
    // double x,y,z;
    itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
    if( itKey->find("DWMRI_gradient") != std::string::npos )
      {
      std::istringstream iss(metaString);
      iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
      // sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
      // vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
      GradientDirectionContainer->push_back(vect3d);
      }
    else if( itKey->find("DWMRI_b-value") != std::string::npos )
      {
      readb0 = true;
      b0 = atof( metaString.c_str() );
      // std::cout<<"b Value: "<<b0<<std::endl;
      }
    }

  if( !readb0 )
    {
    std::cout << "BValue not specified in header file" << std::endl;
    return false;
    }
  if( GradientDirectionContainer->size() <= 6 )
    {
    std::cout << "Gradient Images Less than 7" << std::endl;
    bGetGradientDirections = false;
    return false;
    }

  std::cout << "b Value: " << b0 << std::endl;
  std::cout << "DWI image gradient count: " << m_DwiOriginalImage->GetVectorLength()
            << std::endl;
  for( unsigned int i = 0; i < m_DwiOriginalImage->GetVectorLength(); i++ )  //
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

void IntensityMotionCheckPanel::on_pushButton_SaveVisualChecking_clicked()
{

  int num_Includegradient_VC = false;

  std::cout << "VC_Status.size(): " << VC_Status.size() << std::endl;
  for( unsigned int i = 0; i < VC_Status.size(); i++ )
    {
    std::cout << "index:" << VC_Status[i].index << " " << " VC_Status: " << VC_Status[i].VC_status << std::endl;
    }

  // for( unsigned int i=0; i< VC_Status.size(); i++)
  // {
  //   if (VC_Status[i].VC_status == 0 && this->GetQCResult().GetIntensityMotionCheckResult()[ VC_Status[i].index
  // ].processing >= 3 ) // check if the gradient (index th) changed to Include in Visual Checking step but its QCResult
  // has been Excluded
  //   {
  //      num_Includegradient_VC = true;
  //  index_listVCIncluded.push_back( VC_Status[i].index );
  //    }
  // }

//  for ( unsigned int i = 0; i< VC_Status.size() ; i++)
//  {
//     if (VC_Status[i].VC_status == QCResult::GRADIENT_EXCLUDE_MANUALLY)
//     {
//	index_listVCExcluded.push_back( VC_Status[i].index );
//     }
// }

  if( num_Includegradient_VC )
    {
    QString Grad2 = QString(
        "There are some gradients which have changed their status from Exclude to Include after Visual Checking. Do you want to return the automatic QC (recommended)?  If say No, it means that the gradients with changed status will not be saved.");
    QMessageBox msgBox;
    msgBox.setText( Grad2 );
    QPushButton * YES = msgBox.addButton( tr("Yes"), QMessageBox::ActionRole);
    QPushButton * NO = msgBox.addButton( tr("No"), QMessageBox::ActionRole);
    QPushButton * Cancel = msgBox.addButton( tr("Cancel"), QMessageBox::ActionRole);
    msgBox.exec();

    if( msgBox.clickedButton() == YES )
      {
      if( !bProtocol )
        {
        QString     Grad3 = QString( "Please choose the protocol" );
        QMessageBox msgBox3;
        msgBox3.setText( Grad3 );
        QPushButton * OK = msgBox3.addButton( tr("OK"), QMessageBox::ActionRole );
        msgBox3.exec();
        if( msgBox3.clickedButton() == OK )
          {
          on_toolButton_ProtocolFileOpen_clicked();
          }
        }

      if( bProtocol ) // Doing QC using current protocol
        {

        this->GenerateOutput_VisualCheckingResult();
        if( bDwi_VisualCheckLoad )
          {
          qcResult.Clear();
          treeWidget_Results->clear();
          myFurtherQCThread.Setdwi( this->GetDwiOutputImage() );
          myFurtherQCThread.SetXmlFileName(lineEdit_Protocol->text().toStdString() );
          myFurtherQCThread.SetProtocol( &protocol);
          myFurtherQCThread.SetQCResult(&qcResult);
          // myFurtherQCThread.Set_result();
          myFurtherQCThread.start();
          // result = myFurtherQCThread.Get_result();

          // myIntensityThread.m_IntensityMotionCheck->Setm_DwiForcedConformanceImage( GetDwiOutputImage() );
          /*myIntensityThread.m_IntensityMotionCheck->SetDwiFileName(DwiFileName);
          myIntensityThread.m_IntensityMotionCheck->SetXmlFileName(lineEdit_Protocol->text().toStdString());

          myIntensityThread.m_IntensityMotionCheck->SetProtocol( & protocol);
          myIntensityThread.m_IntensityMotionCheck->SetQCResult( & qcResult);

          myIntensityThread.m_IntensityMotionCheck->Setm_DwiForcedConformanceImage(m_DwiOriginalImage);

          myIntensityThread.m_IntensityMotionCheck->ImageCheck(myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage());
          myIntensityThread.m_IntensityMotionCheck->DiffusionCheck(myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage());

          GenerateCheckOutputImage(myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage());

          // exclude the "excluded gradients"
          myIntensityThread.m_IntensityMotionCheck->Setm_DwiForcedConformanceImage(GetDwiOutputImage());
          myIntensityThread.m_IntensityMotionCheck->BaselineAverage(myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage());
          myIntensityThread.m_IntensityMotionCheck->EddyMotionCorrectIowa(myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage());
          myIntensityThread.m_IntensityMotionCheck->GradientWiseCheck(myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage());
          // SaveDwiForcedConformanceImage ???????
          myIntensityThread.m_IntensityMotionCheck->DTIComputing();
          ResultUpdate();
          */
          }
        }
      }
    else if( msgBox.clickedButton() == NO )
      {
      SaveVisualCheckingResult();

      }
    else if( msgBox.clickedButton() == Cancel )
      {
      return;
      }
    }
  else if( !num_Includegradient_VC )
    {
    SaveVisualCheckingResult();
    }

}

void IntensityMotionCheckPanel::SaveVisualCheckingResult()
{

  pushButton_SaveVisualChecking->setEnabled( 1 );
  // Saving QC Result in associated xml file
  ResultUpdate();
  QCedResultUpdate();
  // if (bLoadDefaultQC)
  // {
  //   SavingTreeWidgetResult_XmlFile_Default();
  // }
  // else
  // {
  //     SavingTreeWidgetResult_XmlFile();
  // }

  // Saving Output

  QString VisualChecking_DwiFileName;
  if( protocol.GetQCOutputDirectory().length() > 0 )
    {

    QString str_QCOutputDirectory = QString(protocol.GetQCOutputDirectory().c_str() );
    bool    found_SeparateChar = str_QCOutputDirectory.contains("/");
    if( !found_SeparateChar ) // "/" does not exist in the protocol->GetQCOutputDirectory() and interpreted as the
                              // relative path and creates the folder
      {
      QString Full_path = DwiFilePath;
      QString Full_name = DwiFilePath.section('/', -1);

      Full_path.remove(Full_name);
      Full_path.append( "/" );
      Full_path.append( QString( protocol.GetQCOutputDirectory().c_str() ) );

      if( !QDir( Full_path ).exists() )
        {
        QDir().mkdir( Full_path );
        }

      Full_path.append( "/" );
      Full_path.append( Full_name.section('.', -2, 0) );
      Full_path.remove("_QCed");
      Full_path.append(QString(tr("_VC.nrrd") ) );
      VisualChecking_DwiFileName = Full_path;
      }
    else
      {
      QString Full_name = DwiFilePath.section('/', -1);
      QString Full_path;
      Full_path.append( QString( protocol.GetQCOutputDirectory().c_str() ) );
      if( !QDir( Full_path ).exists() )
        {
        QDir().mkdir( Full_path );
        }
      Full_path.append( "/" );
      Full_path.append( Full_name );
      Full_path.remove("_QCed");
      Full_path.append(QString(tr("_VC.nrrd") ) );
      VisualChecking_DwiFileName = Full_path;

      }
    }

  else
    {
    VisualChecking_DwiFileName = DwiFilePath.section('.', -2, 0);
    VisualChecking_DwiFileName.remove("_QCed");
    VisualChecking_DwiFileName.append(QString(tr("_VC.nrrd") ) );
    }

  QString DWIFile = QFileDialog::getSaveFileName( this, tr(
                                                    "Save Visual Checking DWI File As"), VisualChecking_DwiFileName,
                                                  tr("nrrd Files (*.nrrd)") );

  if( DWIFile.length() > 0 )
    {
    std::cout << "Save Visual Checking DWI into file: " << DWIFile.toStdString() << std::endl;
    }
  else
    {
    std::cout << "Visual Checking DWI file name NOT set" << std::endl;
    }

  GenerateOutput_VisualCheckingResult2( DWIFile.toStdString() );

  // VC_Status.clear();
  emit UpdateOutputDWIDiffusionVectorActors();

}

bool IntensityMotionCheckPanel::Search_index( int index, std::vector<int> list_index )
{
  for( unsigned int i = 0; i < list_index.size(); i++ )
    {
    if( index == list_index[i] )
      {
      return true;
      }
    }
  return false;
}

void IntensityMotionCheckPanel::GenerateOutput_VisualCheckingResult2( std::string filename )
{

  // Saving QCed dwi image that visually checked
  int num_gradeints_left = 0;

  for( unsigned int i = 0; i < VC_Status.size(); i++ )
    {
    if( VC_Status[i].VC_status == -1 || VC_Status[i].VC_status == 0 )
      {
      num_gradeints_left++;
      }

    }

  std::cout << "num_gradients_left: " << num_gradeints_left << std::endl;

  // Check wether the number of excluded gradeints does not exceed the threshold mentioned in protocol
  if( bProtocol )
    {
    if( 1.0
        - (float)( (float)num_gradeints_left
                   / (float)qcResult.GetIntensityMotionCheckResult().size() ) >=
        this->protocol.GetBadGradientPercentageTolerance() )
      {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, tr("Attention"),
                                    tr(
                                      "Bad gradients number is greater than that in protocol, save anyway?"),
                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
      if( reply == QMessageBox::No || reply == QMessageBox::Cancel )
        {
        bDwi_VisualCheckLoad = false;
        return;
        }
      }
    }

  if( num_gradeints_left == static_cast<int>(GetDwiOutputImage()->GetVectorLength() ) )
    {
    itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
    try
      {
      DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
      DwiWriter->SetImageIO(myNrrdImageIO);
      DwiWriter->SetFileName( filename );
      DwiWriter->UseInputMetaDataDictionaryOn();
      DwiWriter->SetInput( GetDwiOutputImage() );
      DwiWriter->UseCompressionOn();
      DwiWriter->Update();
      }
    catch( itk::ExceptionObject & e )
      {
      std::cout << e.GetDescription() << std::endl;
      bDwi_VisualCheckLoad = false;
      return;
      }
    bDwi_VisualCheckLoad = true;
    return;
    }

  DwiImageType::Pointer newDwiImage = DwiImageType::New();
  newDwiImage->CopyInformation(GetDwiOutputImage() );
  newDwiImage->SetRegions( GetDwiOutputImage()->GetLargestPossibleRegion() );
  newDwiImage->Allocate();
  newDwiImage->SetVectorLength( num_gradeints_left);

  typedef itk::ImageRegionConstIteratorWithIndex<DwiImageType>
  ConstIteratorType;
  ConstIteratorType oit( GetDwiOutputImage(), GetDwiOutputImage()->GetLargestPossibleRegion() );
  typedef itk::ImageRegionIteratorWithIndex<DwiImageType> IteratorType;
  IteratorType nit( newDwiImage, newDwiImage->GetLargestPossibleRegion() );

  oit.GoToBegin();
  nit.GoToBegin();

  DwiImageType::PixelType value;
  value.SetSize( num_gradeints_left );

  while( !oit.IsAtEnd() )
    {
    int element = 0;
    for( unsigned int i = 0; i < GetDwiOutputImage()->GetVectorLength(); i++ )
      {
      if( VC_Status[i].VC_status == -1 || VC_Status[i].VC_status == 0 )
        {
        value.SetElement( element, oit.Get()[i] );
        element++;
        }
      }
    nit.Set(value);
    ++oit;
    ++nit;
    }

  // wrting MetaDataDictionary of the output dwi image from input metaDataDictionary information
  itk::MetaDataDictionary output_imgMetaDictionary;  // output dwi image dictionary

  itk::MetaDataDictionary  imgMetaDictionary = GetDwiOutputImage()->GetMetaDataDictionary();
  std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();

  std::string metaString;

  if( imgMetaDictionary.HasKey("NRRD_measurement frame") )
    {
    // Meausurement frame
    std::vector<std::vector<double> > nrrdmf;
    itk::ExposeMetaData<std::vector<std::vector<double> > >(
      imgMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);
    itk::EncapsulateMetaData<std::vector<std::vector<double> > >(
      output_imgMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);
    }

  // modality
  if( imgMetaDictionary.HasKey("modality") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "modality", metaString);
    itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                           "modality",
                                           metaString);
    }

  // b-value
  if( imgMetaDictionary.HasKey("DWMRI_b-value") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "DWMRI_b-value", metaString);
    itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                           "DWMRI_b-value",
                                           metaString);
    }

  // gradient vectors
  // TensorReconstructionImageFilterType::GradientDirectionType vect3d_T;

  // GradientDirectionContainer_ConformanceImg = GradientDirectionContainerType::New();

  // for (; itKey != imgMetaKeys.end(); itKey++ )
  // {
  //  // double x,y,z;
  //  itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
  //  if ( itKey->find("DWMRI_gradient") != std::string::npos )
  //  {
  //    std::istringstream iss(metaString);
  //    iss >> vect3d_T[0] >> vect3d_T[1] >> vect3d_T[2];
  //     GradientDirectionContainer_ConformanceImg->push_back(vect3d_T);
  //  }
  // }
  int temp = 0;
  for( unsigned int i = 0; i < GetDwiOutputImage()->GetVectorLength(); i++ )
    {

    if( VC_Status[i].VC_status == -1 || VC_Status[i].VC_status == 0 )
      {
      std::ostringstream ossKey;
      ossKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << temp;

      std::ostringstream ossMetaString;
      ossMetaString << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer_ConformanceImg->ElementAt(i)[0]
                    << "    "
                    << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer_ConformanceImg->ElementAt(i)[1]
                    << "    "
                    << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer_ConformanceImg->ElementAt(i)[2];

      // std::cout<<ossKey.str()<<ossMetaString.str()<<std::endl;
      itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                             ossKey.str(), ossMetaString.str() );
      ++temp;
      }
    }

  newDwiImage->SetMetaDataDictionary(output_imgMetaDictionary);

  itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
  try
    {
    DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
    DwiWriter->SetImageIO(myNrrdImageIO);
    DwiWriter->SetFileName( filename );
    DwiWriter->UseInputMetaDataDictionaryOn();
    DwiWriter->SetInput(newDwiImage);
    DwiWriter->UseCompressionOn();
    DwiWriter->Update();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cout << e.GetDescription() << std::endl;
    bDwi_VisualCheckLoad = false;
    return;
    }
  std::cout << "QC dwi Savd" << std::endl;
  bDwi_VisualCheckLoad = true;
  return;

}

void IntensityMotionCheckPanel::GenerateOutput_VisualCheckingResult( std::string filename )
{
  /*std::vector<int> list_index_original;  // list of original indices in dwi ???? ( Problem )
  for ( unsigned int j = 0; j< qcResult.GetOriginal_ForcedConformance_Map().size() ; j++ )
  {
    if (j == 0)
    {
       for ( unsigned int k = 0; k< (qcResult.GetOriginal_ForcedConformance_Map()[j].index_original).size() ; k++ )
    list_index_original.push_back( (qcResult.GetOriginal_ForcedConformance_Map()[j].index_original)[k] ); // indices for Baseline
    }
    else
       list_index_original.push_back( (qcResult.GetOriginal_ForcedConformance_Map()[j].index_original)[0] );
  }

  for ( unsigned int i =0 ; i < list_index_original.size() ; i++ )
  {
  std::cout << "list_index_original: " << list_index_original[i] << std::endl;
  }*/
  for( unsigned int i = 0; i < index_listVCExcluded.size(); i++ )
    {
    std::cout << "index_listVCExcluded: " << index_listVCExcluded[i] << std::endl;
    }
  for( unsigned int i = 0; i < index_listVCIncluded.size(); i++ )
    {
    std::cout << "index_listVCIncluded: " << index_listVCIncluded[i] << std::endl;
    }

  unsigned int gradientLeft = 0;
  /*for ( unsigned int i = 0; i < qcResult.GetIntensityMotionCheckResult().size();  i++ )
  {
    // Finding the included gradients after QC and Visual Checking
    if (  Search_index( i, index_listVCExcluded ) == false && ( Search_index( i, list_index_original ) == true ||  Search_index( i, index_listVCIncluded ) == true ) )
    {
      std::cout << "Leftgradient : " << i << std::endl;
      gradientLeft++;
    }
  }*/
  /*for ( unsigned int i = 0; i < qcResult.GetIntensityMotionCheckResult().size();  i++ )
  {
    // Finding the included gradients after QC and Visual Checking
    if (  Search_index( i, index_listVCExcluded ) == false && ( (qcResult.GetIntensityMotionCheckResult()[i].processing == 0 || qcResult.GetIntensityMotionCheckResult()[i].processing == 1 || qcResult.GetIntensityMotionCheckResult()[i].processing == 2) ||  Search_index( i, index_listVCIncluded ) == true ) )
    {
      std::cout << "Leftgradient : " << i << std::endl;
      gradientLeft++;
    }
  }*/
  // std::cout << "" << "index_listVCExcluded.size()" << index_listVCExcluded.size() << std::endl;
  // std::cout << "myIntensityThread.m_IntensityMotionCheck->get_Original_ForcedConformance_Mapping().size() " <<
  // t_Original_ForcedConformance_Mapping.size() << std::endl;
  for( unsigned int i = 0; i < t_Original_ForcedConformance_Mapping.size();  i++ )
    {
    // Finding the included gradients after Visual Checking

    if(  Search_index( t_Original_ForcedConformance_Mapping[i].index_original[0],
                       index_listVCExcluded ) == false ||
         Search_index( t_Original_ForcedConformance_Mapping[i].index_original[0], index_listVCIncluded ) == true )
      {
      std::cout << "Leftgradient : " << i << std::endl;
      gradientLeft++;
      }
    }

  std::cout << "gradientLeft: " << gradientLeft << std::endl;

  if( bProtocol )
    {
    if( 1.0
        - (float)( (float)gradientLeft
                   / (float)qcResult.GetIntensityMotionCheckResult().size() ) >=
        this->protocol.GetBadGradientPercentageTolerance() )
      {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, tr("Attention"),
                                    tr(
                                      "Bad gradients number is greater than that in protocol, save anyway?"),
                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
      if( reply == QMessageBox::No || reply == QMessageBox::Cancel )
        {
        bDwi_VisualCheckLoad = false;
        return;
        }
      }
    }

  if( gradientLeft == myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage()->GetVectorLength() )
    {
    itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
    try
      {
      DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
      DwiWriter->SetImageIO(myNrrdImageIO);
      DwiWriter->SetFileName( filename );
      DwiWriter->UseInputMetaDataDictionaryOn();
      DwiWriter->SetInput( myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage() );
      DwiWriter->UseCompressionOn();
      DwiWriter->Update();
      }
    catch( itk::ExceptionObject & e )
      {
      std::cout << e.GetDescription() << std::endl;
      bDwi_VisualCheckLoad = false;
      return;
      }
    bDwi_VisualCheckLoad = true;  // ????
    return;
    }

  DwiImageType::Pointer newDwiImage = DwiImageType::New();
  newDwiImage->CopyInformation(myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage() );
  newDwiImage->SetRegions(
    myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage()->GetLargestPossibleRegion() );
  newDwiImage->Allocate();
  newDwiImage->SetVectorLength( gradientLeft);

  typedef itk::ImageRegionConstIteratorWithIndex<DwiImageType>
  ConstIteratorType;
  ConstIteratorType oit(
    myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage(),
    myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage()->GetLargestPossibleRegion() );
  typedef itk::ImageRegionIteratorWithIndex<DwiImageType> IteratorType;
  IteratorType nit( newDwiImage, newDwiImage->GetLargestPossibleRegion() );

  oit.GoToBegin();
  nit.GoToBegin();

  DwiImageType::PixelType value;
  value.SetSize( gradientLeft );

  while( !oit.IsAtEnd() )
    {
    int element = 0;
    for( unsigned int i = 0;
         i < myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage()->GetVectorLength(); i++ )
      {
      if( Search_index( t_Original_ForcedConformance_Mapping[i].index_original[0],
                        index_listVCExcluded ) == false ||
          Search_index( t_Original_ForcedConformance_Mapping[i].index_original[0], index_listVCIncluded ) == true )
        {
        value.SetElement( element, oit.Get()[i] );
        element++;
        }
      }
    nit.Set(value);
    ++oit;
    ++nit;
    }

  // wrting MetaDataDictionary of the output dwi image from input metaDataDictionary information
  itk::MetaDataDictionary output_imgMetaDictionary;  // output dwi image dictionary

  itk::MetaDataDictionary imgMetaDictionary =
    myIntensityThread.m_IntensityMotionCheck->Getm_DwiForcedConformanceImage()->GetMetaDataDictionary();
  std::vector<std::string>                 imgMetaKeys = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string                              metaString;

  if( imgMetaDictionary.HasKey("NRRD_measurement frame") )
    {
    // Meausurement frame
    std::vector<std::vector<double> > nrrdmf;
    itk::ExposeMetaData<std::vector<std::vector<double> > >(
      imgMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);
    itk::EncapsulateMetaData<std::vector<std::vector<double> > >(
      output_imgMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);
    }

  // modality
  if( imgMetaDictionary.HasKey("modality") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "modality", metaString);
    itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                           "modality",
                                           metaString);
    }

  // b-value
  if( imgMetaDictionary.HasKey("DWMRI_b-value") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "DWMRI_b-value", metaString);
    itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                           "DWMRI_b-value",
                                           metaString);
    }

  // gradient vectors
  TensorReconstructionImageFilterType::GradientDirectionType vect3d_T;

  GradientDirectionContainer_ConformanceImg = GradientDirectionContainerType::New();
  for( ; itKey != imgMetaKeys.end(); itKey++ )
    {
    // double x,y,z;
    itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
    if( itKey->find("DWMRI_gradient") != std::string::npos )
      {
      std::istringstream iss(metaString);
      iss >> vect3d_T[0] >> vect3d_T[1] >> vect3d_T[2];
      GradientDirectionContainer_ConformanceImg->push_back(vect3d_T);
      }
    }

  int temp = 0;
  for( unsigned int i = 0; i < t_Original_ForcedConformance_Mapping.size(); i++ )
    {

    if( Search_index( t_Original_ForcedConformance_Mapping[i].index_original[0],
                      index_listVCExcluded ) == false ||
        Search_index( t_Original_ForcedConformance_Mapping[i].index_original[0], index_listVCIncluded ) == true )
      {
      std::ostringstream ossKey;
      ossKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << temp;

      std::ostringstream ossMetaString;
      ossMetaString << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer_ConformanceImg->ElementAt(i)[0]
                    << "    "
                    << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer_ConformanceImg->ElementAt(i)[1]
                    << "    "
                    << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer_ConformanceImg->ElementAt(i)[2];

      // std::cout<<ossKey.str()<<ossMetaString.str()<<std::endl;
      itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                             ossKey.str(), ossMetaString.str() );
      ++temp;
      }
    }

  newDwiImage->SetMetaDataDictionary(output_imgMetaDictionary);

  SetDwiOutputImage(newDwiImage);

  itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
  try
    {
    DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
    DwiWriter->SetImageIO(myNrrdImageIO);
    DwiWriter->SetFileName( filename );
    DwiWriter->UseInputMetaDataDictionaryOn();
    DwiWriter->SetInput(newDwiImage);
    DwiWriter->UseCompressionOn();
    DwiWriter->Update();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cout << e.GetDescription() << std::endl;
    bDwi_VisualCheckLoad = false;
    return;
    }
  std::cout << "QC dwi Savd" << std::endl;
  bDwi_VisualCheckLoad = true;
  return;
}

/*void IntensityMotionCheckPanel::on_pushButton_SaveDWIAs_clicked( )
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
}*/

/*void IntensityMotionCheckPanel::on_pushButton_DefaultQCResult_clicked()
{
  if ( m_DwiOriginalImage->GetVectorLength() != GradientDirectionContainer->size() )
  {
    std::cout
      << "Bad DWI: mismatch between gradient image #and gradient vector #"
      << std::endl;
    QMessageBox::critical( this, tr("BAD DWI !"),
      tr("Bad DWI: mismatch between gradient image #and gradient vector #!") );
    return;
  }

}*/

void IntensityMotionCheckPanel::GenerateOutput_VisualCheckingResult()
{

  std::vector<int> list_index_original;  // list of original indices in dwi
  for( unsigned int j = 0; j < myIntensityThread.m_IntensityMotionCheck->get_Original_ForcedConformance_Mapping().size();
       j++ )
    {
    if( j == 0 )
      {
      for( unsigned int k = 0;
           k <
           (myIntensityThread.m_IntensityMotionCheck->get_Original_ForcedConformance_Mapping()[j].index_original).size();
           k++ )
        {
        list_index_original.push_back( (myIntensityThread.m_IntensityMotionCheck->
                                        get_Original_ForcedConformance_Mapping()[j].index_original)[k] );
        // indices for Baseline
        }
      }
    else
      {
      list_index_original.push_back( (myIntensityThread.m_IntensityMotionCheck->get_Original_ForcedConformance_Mapping()
                                      [j].index_original)[0] );
      }
    }
  for( unsigned int i = 0; i < list_index_original.size(); i++ )
    {
    std::cout << "list_index_original: " << list_index_original[i] << std::endl;
    }
  for( unsigned int i = 0; i < index_listVCExcluded.size(); i++ )
    {
    std::cout << "index_listVCExcluded: " << index_listVCExcluded[i] << std::endl;
    }
  for( unsigned int i = 0; i < index_listVCIncluded.size(); i++ )
    {
    std::cout << "index_listVCIncluded: " << index_listVCIncluded[i] << std::endl;
    }

  unsigned int gradientLeft = 0;
  for( unsigned int i = 0; i < qcResult.GetIntensityMotionCheckResult().size();  i++ )
    {
    // Finding the included gradients after QC and Visual Checking
    if(  Search_index( i,
                       index_listVCExcluded ) == false &&
         ( (qcResult.GetIntensityMotionCheckResult()[i].processing == 0 ||
            qcResult.GetIntensityMotionCheckResult()[i].processing == 1 ||
            qcResult.GetIntensityMotionCheckResult()[i].processing == 2) ||
           Search_index( i, index_listVCIncluded ) == true ) )
      {
      std::cout << "Leftgradient : " << i << std::endl;
      gradientLeft++;
      }
    }

  std::cout << "gradientLeft: " << gradientLeft << std::endl;

  if( bProtocol )
    {
    if( 1.0
        - (float)( (float)gradientLeft
                   / (float)qcResult.GetIntensityMotionCheckResult().size() ) >=
        this->protocol.GetBadGradientPercentageTolerance() )
      {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, tr("Attention"),
                                    tr(
                                      "Bad gradients number is greater than that in protocol, save anyway?"),
                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
      if( reply == QMessageBox::No || reply == QMessageBox::Cancel )
        {
        bDwi_VisualCheckLoad = false;
        return;
        }
      }
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

  while( !oit.IsAtEnd() )
    {
    int element = 0;
    for( unsigned int i = 0; i < m_DwiOriginalImage->GetVectorLength(); i++ )
      {
      if( Search_index( i,
                        index_listVCExcluded ) == false &&
          ( (qcResult.GetIntensityMotionCheckResult()[i].processing == 0 ||
             qcResult.GetIntensityMotionCheckResult()[i].processing == 1 ||
             qcResult.GetIntensityMotionCheckResult()[i].processing == 2) ||
            Search_index( i, index_listVCIncluded ) == true ) )
        {
        value.SetElement( element, oit.Get()[i] );
        element++;
        }
      }
    nit.Set(value);
    ++oit;
    ++nit;
    }

  // wrting MetaDataDictionary of the output dwi image from input metaDataDictionary information
  itk::MetaDataDictionary output_imgMetaDictionary;  // output dwi image dictionary

  itk::MetaDataDictionary  imgMetaDictionary = m_DwiOriginalImage->GetMetaDataDictionary();
  std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();

  std::string metaString;

  if( imgMetaDictionary.HasKey("NRRD_measurement frame") )
    {
    // Meausurement frame
    std::vector<std::vector<double> > nrrdmf;
    itk::ExposeMetaData<std::vector<std::vector<double> > >(
      imgMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);
    itk::EncapsulateMetaData<std::vector<std::vector<double> > >(
      output_imgMetaDictionary,
      "NRRD_measurement frame",
      nrrdmf);
    }

  // modality
  if( imgMetaDictionary.HasKey("modality") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "modality", metaString);
    itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                           "modality",
                                           metaString);
    }

  // b-value
  if( imgMetaDictionary.HasKey("DWMRI_b-value") )
    {
    itk::ExposeMetaData(imgMetaDictionary, "DWMRI_b-value", metaString);
    itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                           "DWMRI_b-value",
                                           metaString);
    }

  // gradient vectors
  int temp = 0;
  for( unsigned int i = 0; i < GradientDirectionContainer->size(); i++ )
    {

    if( Search_index( i,
                      index_listVCExcluded ) == false &&
        ( (qcResult.GetIntensityMotionCheckResult()[i].processing == 0 ||
           qcResult.GetIntensityMotionCheckResult()[i].processing == 1 ||
           qcResult.GetIntensityMotionCheckResult()[i].processing == 2) ||
          Search_index( i, index_listVCIncluded ) == true ) )
      {
      std::ostringstream ossKey;
      ossKey << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << temp;

      std::ostringstream ossMetaString;
      ossMetaString << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer->ElementAt(i)[0]
                    << "    "
                    << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer->ElementAt(i)[1]
                    << "    "
                    << std::setw(9) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(6) << std::setiosflags(std::ios::right)
                    << GradientDirectionContainer->ElementAt(i)[2];

      // std::cout<<ossKey.str()<<ossMetaString.str()<<std::endl;
      itk::EncapsulateMetaData<std::string>( output_imgMetaDictionary,
                                             ossKey.str(), ossMetaString.str() );
      ++temp;
      }
    }

  newDwiImage->SetMetaDataDictionary(output_imgMetaDictionary);

  SetDwiOutputImage(newDwiImage);

  bDwi_VisualCheckLoad = true;
  return;

}

void IntensityMotionCheckPanel::on_pushButton_DefaultQCResult_clicked()
{
  bLoadDefaultQC = true;
  if( m_DwiOriginalImage->GetVectorLength() != GradientDirectionContainer->size() )
    {
    std::cout
    << "Bad DWI: mismatch between gradient image #and gradient vector #"
    << std::endl;
    QMessageBox::critical( this, tr("BAD DWI !"),
                           tr("Bad DWI: mismatch between gradient image #and gradient vector #!") );
    return;
    }

  DefaultProcess();
  ResultUpdate();

  bResultTreeEditable = true;
  pushButton_SaveVisualChecking->setEnabled( 1 );

  emit UpdateOutputDWIDiffusionVectorActors();
}

void IntensityMotionCheckPanel::DefaultProcess()
{
  this->GetQCResult().Clear();

  GradientIntensityMotionCheckResult IntensityMotionCR;
  IntensityMotionCR.processing = QCResult::GRADIENT_INCLUDE;
  IntensityMotionCR.VisualChecking = -1;
  for( unsigned int i = 0; i < this->m_DwiOriginalImage->GetVectorLength(); i++ )
    {
    IntensityMotionCR.OriginalDir[0]
      = this->GradientDirectionContainer->ElementAt(i)[0];
    IntensityMotionCR.OriginalDir[1]
      = this->GradientDirectionContainer->ElementAt(i)[1];
    IntensityMotionCR.OriginalDir[2]
      = this->GradientDirectionContainer->ElementAt(i)[2];

    IntensityMotionCR.ReplacedDir[0]
      = this->GradientDirectionContainer->ElementAt(i)[0];
    IntensityMotionCR.ReplacedDir[1]
      = this->GradientDirectionContainer->ElementAt(i)[1];
    IntensityMotionCR.ReplacedDir[2]
      = this->GradientDirectionContainer->ElementAt(i)[2];

    IntensityMotionCR.CorrectedDir[0]
      = this->GradientDirectionContainer->ElementAt(i)[0];
    IntensityMotionCR.CorrectedDir[1]
      = this->GradientDirectionContainer->ElementAt(i)[1];
    IntensityMotionCR.CorrectedDir[2]
      = this->GradientDirectionContainer->ElementAt(i)[2];

    IntensityMotionCR.QCIndex = i;

    this->GetQCResult().GetIntensityMotionCheckResult().push_back(
      IntensityMotionCR);

    SetProcessingQCResult(this->GetQCResult().GetIntensityMotionCheckResult()[i].processing, QCResult::GRADIENT_INCLUDE);

    }

  ImageInformationCheckResult ImageInformationCR;
  ImageInformationCR.origin = true;
  ImageInformationCR.size = true;
  ImageInformationCR.spacing = true;
  ImageInformationCR.space = true;
  ImageInformationCR.spacedirection = true;
  qcResult.GetImageInformationCheckResult() = ImageInformationCR;

  DiffusionInformationCheckResult DiffusionInformationCR;
  DiffusionInformationCR.b = true;
  DiffusionInformationCR.gradient = true;
  DiffusionInformationCR.measurementFrame = true;
  qcResult.GetDiffusionInformationCheckResult() = DiffusionInformationCR;

}

void IntensityMotionCheckPanel::Clear_VC_Status()
{
  VC_Status.clear();
}

void IntensityMotionCheckPanel::LoadQCedDWI( QString qcdwiname )
{
  emit SignalLoadQCedDWI( qcdwiname );
}

void IntensityMotionCheckPanel::Building_Mapping_XML()
{
  // This function builds the xml file from m_Original_ForcedConformance_Mapping which contains mapping information
  // between the original dwi and the conformance dwi
  QString Mapping_xmlFile;

  Mapping_xmlFile.append(DwiFileName.c_str() );
  Mapping_xmlFile.append(QString(tr("_MappingXML.xml") ) );

  QFile file_Mapping(Mapping_xmlFile);

  if( !file_Mapping.open(QFile::WriteOnly | QFile::Text) )
    {
    std::cerr << "Error: Cannot write file "
              << qPrintable(Mapping_xmlFile) << ": "
              << qPrintable( file_Mapping.errorString() ) << std::endl;
    }

  QXmlStreamWriter xmlWriter(&file_Mapping);
  xmlWriter.setAutoFormatting(true);
  xmlWriter.writeStartDocument();
  xmlWriter.writeStartElement("MappingSettings");
// std::cout << "Size of myIntensityThread.m_IntensityMotionCheck->get_Original_ForcedConformance_Mapping(): "<<
// myIntensityThread.m_IntensityMotionCheck->get_Original_ForcedConformance_Mapping().size() << std::endl;
  for( unsigned int i = 0; i < myIntensityThread.m_IntensityMotionCheck->get_Original_ForcedConformance_Mapping().size();
       i++ )
    {

    // gradient

    xmlWriter.writeStartElement("entry");
    xmlWriter.writeAttribute( "parameter",  QString("Conformanced_gradients_%1").arg( i, 4, 10, QLatin1Char( '0' ) ) );
    for( unsigned int j = 0;
         j < myIntensityThread.m_IntensityMotionCheck->get_Original_ForcedConformance_Mapping()[i].index_original.size();
         j++ )
      {
      xmlWriter.writeTextElement("Original_id",
                                 QString("%1").arg( (myIntensityThread.m_IntensityMotionCheck->
                                                     get_Original_ForcedConformance_Mapping()[i]).
                                                    index_original[j]) );

      }
    xmlWriter.writeEndElement();

    }

  xmlWriter.writeEndElement();
  xmlWriter.writeEndDocument();
  file_Mapping.close();

}

/*bool IntensityMotionCheckPanel::OpenQCedDWI()
{
  QString QCedDWINrrdFile = QFileDialog::getOpenFileName ( this, tr(
          "Open QCed nrrd DWI"), QDir::currentPath(), tr("Nrrd Files (*.nhdr *.nrrd)") );

    if ( QCedDWINrrdFile.length() > 0 )
      {
      itk::NrrdImageIO::Pointer NrrdImageIO = itk::NrrdImageIO::New();
      QCedDwiReader = DwiReaderType::New();
      QCedDwiReader->SetImageIO(NrrdImageIO);

    try
        {
        QCedDwiReader->SetFileName( QCedDWINrrdFile.toStdString() );

        QCedDwiReader->Update();
        }
        catch ( itk::ExceptionObject & e )
        {
        std::cout << e.GetDescription() << std::endl;
        return false;
        }
        }
        else
        {
        std::cout << "QCedDwi file name not set" << std::endl;
        return false;
        }
        std::cout << "done " << std::endl;
  SetDwiOutputImage(QCedDwiReader->GetOutput());
  return true;

}*/

bool IntensityMotionCheckPanel::OpenMappingXML()
{
  QString xmlFile = QFileDialog::getOpenFileName( this, tr(
                                                    "Select Mapping file"), lineEdit_Protocol->text(),
                                                  tr("xml Files (*.xml)") );

  if( xmlFile.length() <= 0 )
    {
    std::cerr << "Error: Failed in opening file." << std::endl;
    return false;
    }

  QDomDocument doc("mydocument");
  QFile        file(xmlFile);

  if( !file.open(QFile::ReadOnly | QFile::Text) )
    {
    std::cerr << "Error: Failed in reading file." << qPrintable(xmlFile)
              << ": " << qPrintable( file.errorString() )
              << std::endl;
    return false;
    }
  if( !doc.setContent(&file) )
    {
    file.close();
    return false;
    }
  file.close();

  t_Original_ForcedConformance_Mapping.clear();

  QDomElement docElement = doc.documentElement();   // MappingSettings tag

  QDomNode n = docElement.firstChild();
  int      conformanced_id = 0;
  while( !n.isNull() )
    {
    m_Original_ForcedConformance_Mapping m_item;
    m_item.index_ForcedConformance = conformanced_id;

    QStringList values;
    QDomNode    m = n.firstChild(); // Original_id tag
    values = m.toElement().text().split(" ");
    foreach(QString value, values)
      {
      m_item.index_original.push_back(value.toInt() );
      }
    t_Original_ForcedConformance_Mapping.push_back(m_item);
    n = n.nextSibling();
    conformanced_id++;
    }
  for( unsigned int i = 0; i < t_Original_ForcedConformance_Mapping.size(); i++ )
    {
    // std::cout<< "Testing Mapping XML " << t_Original_ForcedConformance_Mapping[i].index_original[0] << std::endl;
    }

  return true;

}

void IntensityMotionCheckPanel::on_pushButton_Pathdefault_clicked()
{
  std::cout << "Test mahshid " << std::endl;
  std::string program;
  std::string notFound;

  // The protocol paths are bet2,DiffusionWeightedVolumeMasking,dtiestim,dtiprocess and denoisingfilter.
  // They are saved in protocol in BrainMaskProtocol and DTIProtocol structures.

  // BrainMask: FSL_bet
  program = itksys::SystemTools::FindProgram("bet2");
  std::cout << "Test mahshid " << program.c_str() << std::endl;
  if( program.empty() )
    {
    if( lineEdit_FSL->text().isEmpty() )
      {
      notFound = notFound + "> bet2\n";
      }
    }
  else
    {
    lineEdit_FSL->setText(QString::fromStdString(program) );
    this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_FSL = program;
    if( !bProtocol )
      {
      std::string text_er = "No protocol has been loaded. Please Load Protocol.";
      QMessageBox::warning(this, "Protocol missing", QString(text_er.c_str() ) );
      return;
      }
    this->GetTreeWidgetProtocol()->topLevelItem(14)->child(1)->setText( 1, QString::fromStdString(program) );
    }

  // convertitk
  program = itksys::SystemTools::FindProgram("convertITKformats");
  std::cout << "Test mahshid " << program.c_str() << std::endl;
  if( program.empty() )
    {
    if( lineEdit_convertitk->text().isEmpty() )
      {
      notFound = notFound + "> convertITKformats\n";
      }
    }
  else
    {
    lineEdit_convertitk->setText(QString::fromStdString(program) );
    this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_convertITK = program;
    this->GetTreeWidgetProtocol()->topLevelItem(14)->child(2)->setText( 1, QString::fromStdString(program) );
    }

  // convertitk
  program = itksys::SystemTools::FindProgram("ImageMath");
  std::cout << "Test mahshid " << program.c_str() << std::endl;
  if( program.empty() )
    {
    if( lineEdit_imagemath->text().isEmpty() )
      {
      notFound = notFound + "> ImageMath\n";
      }
    }
  else
    {
    lineEdit_imagemath->setText(QString::fromStdString(program) );
    this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_imagemath = program;
    this->GetTreeWidgetProtocol()->topLevelItem(14)->child(3)->setText( 1, QString::fromStdString(program) );
    }

  // BrainMask: Slicer/DiffusionWeightedVolumeMasking
  program = itksys::SystemTools::FindProgram("DiffusionWeightedVolumeMasking");
  if( program.empty() )
    {
    if( lineEdit_Slicer->text().isEmpty() )
      {
      notFound = notFound + "> DiffusionWeightedVolumeMasking\n";
      }
    }
  else
    {
    lineEdit_Slicer->setText(QString::fromStdString(program) );
    this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_Slicer = program;
    this->GetTreeWidgetProtocol()->topLevelItem(14)->child(4)->setText( 1, QString::fromStdString(program) );
    }

  // DTI estimation: dtiestim and dtiprocess
  program = itksys::SystemTools::FindProgram("dtiestim");
  if( program.empty() )
    {
    if( lineEdit_dtiestim->text().isEmpty() )
      {
      notFound = notFound + "> dtiestim\n";
      }
    }
  else
    {
    lineEdit_dtiestim->setText(QString::fromStdString(program) );
    this->GetProtocol().GetDTIProtocol().dtiestimCommand = program;
    this->GetTreeWidgetProtocol()->topLevelItem(16)->child(0)->setText( 1, QString::fromStdString(program) );
    }

  program = itksys::SystemTools::FindProgram("dtiprocess");
  if( program.empty() )
    {
    if( lineEdit_dtiprocess->text().isEmpty() )
      {
      notFound = notFound + "> dtiprocess\n";
      }
    }
  else
    {
    lineEdit_dtiprocess->setText(QString::fromStdString(program) );
    this->GetProtocol().GetDTIProtocol().dtiprocessCommand = program;
    this->GetTreeWidgetProtocol()->topLevelItem(16)->child(1)->setText( 1, QString::fromStdString(program) );
    }

  if( !notFound.empty() )
    {
    std::string text = "The following programs have not been found.\nPlease enter the path manually:\n" + notFound;
    QMessageBox::warning(this, "Program missing", QString(text.c_str() ) );

    }

}

void IntensityMotionCheckPanel::on_pushButton_FSL_clicked()
{
  if( !bProtocol )
    {
    std::string text_er = "No protocol has been loaded. Please Load Protocol.";
    QMessageBox::warning(this, "Protocol missing", QString(text_er.c_str() ) );
    return;
    }

  QString executable_file = QFileDialog::getOpenFileName( this, tr(
                                                            "Open program executable file"), QDir::currentPath(),
                                                          tr("Executable Files") );
  if( executable_file.length() == 0 )
    {
    std::string text = "No file is set";
    QMessageBox::warning(this, "Program missing", QString(text.c_str() ) );
    return;
    }

  lineEdit_FSL->setText(executable_file);
  this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_FSL = executable_file.toStdString();
  this->GetTreeWidgetProtocol()->topLevelItem(14)->child(1)->setText( 1, executable_file );

  return;
}

void IntensityMotionCheckPanel::on_pushButton_Slicer_clicked()
{
  if( !bProtocol )
    {
    std::string text_er = "No protocol has been loaded. Please Load Protocol.";
    QMessageBox::warning(this, "Protocol missing", QString(text_er.c_str() ) );
    return;
    }

  QString executable_file = QFileDialog::getOpenFileName( this, tr(
                                                            "Open program executable file"), QDir::currentPath(),
                                                          tr("Executable Files") );
  if( executable_file.length() == 0 )
    {
    std::string text = "No file is set";
    QMessageBox::warning(this, "Program missing", QString(text.c_str() ) );
    return;
    }

  lineEdit_Slicer->setText(executable_file);
  this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_Slicer = executable_file.toStdString();

  this->GetTreeWidgetProtocol()->topLevelItem(14)->child(4)->setText( 1, executable_file );

  return;
}

void IntensityMotionCheckPanel::on_pushButton_dtiestim_clicked()
{
  if( !bProtocol )
    {
    std::string text_er = "No protocol has been loaded. Please Load Protocol.";
    QMessageBox::warning(this, "Protocol missing", QString(text_er.c_str() ) );
    return;
    }

  QString executable_file = QFileDialog::getOpenFileName( this, tr(
                                                            "Open program executable file"), QDir::currentPath(),
                                                          tr("Executable Files") );
  if( executable_file.length() == 0 )
    {
    std::string text = "No file is set";
    QMessageBox::warning(this, "Program missing", QString(text.c_str() ) );
    return;
    }

  lineEdit_dtiestim->setText(executable_file);
  this->GetProtocol().GetDTIProtocol().dtiestimCommand = executable_file.toStdString();

  this->GetTreeWidgetProtocol()->topLevelItem(16)->child(0)->setText( 1, executable_file );

  return;
}

void IntensityMotionCheckPanel::on_pushButton_dtiprocess_clicked()
{
  if( !bProtocol )
    {
    std::string text_er = "No protocol has been loaded. Please Load Protocol.";
    QMessageBox::warning(this, "Protocol missing", QString(text_er.c_str() ) );
    return;
    }

  QString executable_file = QFileDialog::getOpenFileName( this, tr(
                                                            "Open program executable file"), QDir::currentPath(),
                                                          tr("Executable Files") );
  if( executable_file.length() == 0 )
    {
    std::string text = "No file is set";
    QMessageBox::warning(this, "Program missing", QString(text.c_str() ) );
    return;
    }

  lineEdit_dtiprocess->setText(executable_file);
  this->GetProtocol().GetDTIProtocol().dtiprocessCommand = executable_file.toStdString();

  this->GetTreeWidgetProtocol()->topLevelItem(16)->child(1)->setText( 1, executable_file );

  return;
}

void IntensityMotionCheckPanel::on_pushButton_convertitk_clicked()
{
  if( !bProtocol )
    {
    std::string text_er = "No protocol has been loaded. Please Load Protocol.";
    QMessageBox::warning(this, "Protocol missing", QString(text_er.c_str() ) );
    return;
    }
  QString executable_file = QFileDialog::getOpenFileName( this, tr(
                                                            "Open program executable file"), QDir::currentPath(),
                                                          tr("Executable Files") );
  if( executable_file.length() == 0 )
    {
    std::string text = "No file is set";
    QMessageBox::warning(this, "Program missing", QString(text.c_str() ) );
    return;
    }

  lineEdit_convertitk->setText(executable_file);
  this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_convertITK = executable_file.toStdString();

  this->GetTreeWidgetProtocol()->topLevelItem(14)->child(2)->setText( 1, executable_file );

  return;
}

void IntensityMotionCheckPanel::on_pushButton_imagemath_clicked()
{

  if( !bProtocol )
    {
    std::string text_er = "No protocol has been loaded. Please Load Protocol.";
    QMessageBox::warning(this, "Protocol missing", QString(text_er.c_str() ) );
    return;
    }

  QString executable_file = QFileDialog::getOpenFileName( this, tr(
                                                            "Open program executable file"), QDir::currentPath(),
                                                          tr("Executable Files") );
  if( executable_file.length() == 0 )
    {
    std::string text = "No file is set";
    QMessageBox::warning(this, "Program missing", QString(text.c_str() ) );
    return;
    }

  lineEdit_imagemath->setText(executable_file);
  this->GetProtocol().GetBrainMaskProtocol().BrainMask_SystemPath_imagemath = executable_file.toStdString();

  this->GetTreeWidgetProtocol()->topLevelItem(14)->child(3)->setText( 1, executable_file );

  return;
}

/*void IntensityMotionCheckPanel::DefaultProcess( )
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
}*/
