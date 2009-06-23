#include "IntensityMotionCheckPanel.h"
#include "IntensityMotionCheck.h"
#include "ThreadIntensityMotionCheck.h"
#include <QtGui>

#include "itkMetaDataDictionary.h"
#include "itkNrrdImageIO.h"
//#include "itkGDCMImageIO.h"
//#include "itkGDCMSeriesFileNames.h"
//#include "itkImageSeriesReader.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

//#include "itkQtAdaptor.h"
//#include "itkQtAdaptor.h"
//#include "itkQtLightIndicator.h"
//#include "itkQtProgressBar.h"

#include "XmlStreamReader.h"
#include "XmlStreamWriter.h"

#include "IntraGradientRigidRegistration.h"
#include "itkExtractImageFilter.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

IntensityMotionCheckPanel::IntensityMotionCheckPanel(QMainWindow *parent):QDockWidget(parent)
{
	setupUi(this);
	verticalLayout->setContentsMargins(0,0,0,0);
	progressBar->setValue(0);

	DwiImage= NULL ;
	protocal.clear();
	bDwiLoaded = false;
	bProtocol = false;

	bProtocolTreeEditable = false;
	pushButton_DefaultProtocol->setEnabled( 0 );
	pushButton_Save->setEnabled( 0 );
	pushButton_SaveProtocolAs->setEnabled( 0 );


	bResultTreeEditable = false;
	pushButton_SaveDWI->setEnabled( 0 );
	pushButton_SaveQCReport->setEnabled( 0 );
	pushButton_SaveQCReportAs->setEnabled( 0 );
	pushButton_SaveDWIAs->setEnabled( 0 );
	//pushButton_CreateDefaultProtocol->setEnabled( 0 );

	pushButton_RunPipeline->setEnabled( 0 );

	pushButton_DefaultQCResult->setEnabled( 0 );
	pushButton_OpenQCReport->setEnabled( 0 );

	QStringList labels;
	labels << tr("Parameter") << tr("Value");
	//treeWidget->header()->setResizeMode(QHeaderView::Stretch);
	treeWidget->setHeaderLabels(labels);
			
	ThreadIntensityMotionCheck = new CThreadIntensityMotionCheck;

	bGetGridentDirections = false;

	GradientDirectionContainer = GradientDirectionContainerType::New();

	//ThreadIntensityMotionCheck->SetProtocal( &protocal);
	//ThreadIntensityMotionCheck->SetQCResult(&qcResult);

	//connect(ThreadIntensityMotionCheck, SIGNAL(QQQ(int )), this, SLOT(UpdateProgressBar(const int )) );

	connect(ThreadIntensityMotionCheck, SIGNAL(kkk(int )), this, SLOT(UpdateProgressBar(const int )) );

	connect( this->ThreadIntensityMotionCheck, 
		SIGNAL(ResultUpdate()),
		this,
		SLOT(ResultUpdate()) );

	//connect( this->ThreadIntensityMotionCheck->IntensityMotionCheck, 
	//	SIGNAL(Progress(int )),
	//	this,
	//	SLOT(UpdateProgressBar(const int ))  );

}

void IntensityMotionCheckPanel::on_treeWidget_itemDoubleClicked(QTreeWidgetItem* item,int col)
{
	if(col==1 && bProtocolTreeEditable)
		treeWidget->openPersistentEditor(item,col);
}

void  IntensityMotionCheckPanel::on_treeWidget_Results_itemDoubleClicked(QTreeWidgetItem * item, int column)
{
	if( bResultTreeEditable && column == 2 && item->text(0).left(8) == tr("gradient"))
		treeWidget_Results->openPersistentEditor(item,column);
}

void IntensityMotionCheckPanel::on_treeWidget_Results_currentItemChanged( QTreeWidgetItem *current,  QTreeWidgetItem *previous)
{
	treeWidget->closePersistentEditor(previous,2); // does nothing if none open
}

void IntensityMotionCheckPanel::on_treeWidget_Results_itemChanged(QTreeWidgetItem * item, int column) 
{
	if(bResultTreeEditable)
	{
		if( item->text(2) == tr("EXCLUDE"))
		{
			this->GetQCResult().GetGradientProcess()[item->text(0).right(4).toUInt()] = QCResult::GRADIENT_EXCLUDE;
			//qcResult.GetGradientProcess()[i] == QCResult::GRADIENT_EXCLUDE
			std::cout << "gradient "<< item->text(0).right(4).toUInt() << ": GRADIENT_EXCLUDE" <<std::endl;
		}
		else
		{
			this->GetQCResult().GetGradientProcess()[item->text(0).right(4).toUInt()] = QCResult::GRADIENT_INCLUDE;
			std::cout << "gradient "<< item->text(0).right(4).toUInt() << ": GRADIENT_INCLUDE" <<std::endl;
		}
	}
}

void IntensityMotionCheckPanel::on_treeWidget_currentItemChanged( QTreeWidgetItem *current,  QTreeWidgetItem *previous)
{
	treeWidget->closePersistentEditor(previous,1); // does nothing if none open
}

  
IntensityMotionCheckPanel::~IntensityMotionCheckPanel(void)
{
	delete ThreadIntensityMotionCheck;
}

void IntensityMotionCheckPanel::on_pushButton_RunPipeline_clicked( )
{
	//CIntensityMotionCheck IntensityMotionCheck(lineEdit_DWIFileName->text().toStdString());//,"report_54.txt");//
	//IntensityMotionCheck.SetProtocal(&protocal);
	//IntensityMotionCheck.SetQCResult(&qcResult);
	//IntensityMotionCheck.GetImagesInformation();
	//IntensityMotionCheck.CheckByProtocal();
	if(!bProtocol)
	{
		std::cout << "Protocol NOT set. Load prorocol file first!"<<std::endl;
		return;
	}

	if(DwiFileName.length()==0)
	{
		std::cout<<"DWI file name not set!"<<std::endl;
		QMessageBox::information( this, tr("Warning"), tr("DWI file name not set!"));

		return;
	}

	qcResult.Clear();
	//std::cout << "ThreadIntensityMotionCheck->SetFileName(lineEdit_DWIFileName->text().toStdString());"<<std::endl;
	ThreadIntensityMotionCheck->SetFileName(DwiFileName);//lineEdit_DWIFileName->text().toStdString()
	ThreadIntensityMotionCheck->SetProtocal( &protocal);
	ThreadIntensityMotionCheck->SetQCResult(&qcResult);	
	ThreadIntensityMotionCheck->start();

	bResultTreeEditable = false;
	pushButton_SaveDWI->setEnabled( 0 );
	pushButton_SaveDWIAs->setEnabled( 0 );
}


void IntensityMotionCheckPanel::UpdateProgressBar(int pos )
{
	 progressBar->setValue(pos);
}
void IntensityMotionCheckPanel::SetFileName(QString nrrd )
{
	//lineEdit_DWIFileName->setText(nrrd);
	DwiFileName = nrrd.toStdString();
}

void IntensityMotionCheckPanel::on_toolButton_ProtocalFileOpen_clicked( )
{
	//OpenXML();
	emit loadProtocol();
	pushButton_Save->setEnabled( 1 );
	pushButton_SaveProtocolAs->setEnabled( 1 );
	bProtocolTreeEditable = true;
	
}


void IntensityMotionCheckPanel::OpenXML( )
{
	QString xmlFile = QFileDialog::getOpenFileName ( this, tr("Select Protocal"),lineEdit_Protocal->text(), tr("xml Files (*.xml)") );
	 if(xmlFile.length()>0)
	 {
		lineEdit_Protocal->setText(xmlFile);
	 }
	 else
		 return;

	treeWidget->clear();
	protocal.clear();

	XmlStreamReader XmlReader(treeWidget);
	XmlReader.setProtocal( &protocal);
	XmlReader.readFile(xmlFile, XmlStreamReader::TreeWise);
	XmlReader.readFile(xmlFile, XmlStreamReader::ProtocalWise);

	protocal.collectDiffusionStatistics();
	bProtocol=true;

	//if(DwiFileName.length()>0)
	pushButton_RunPipeline->setEnabled(1);

	//protocal.print();
}

bool IntensityMotionCheckPanel::LoadDwiImage()
{
	
// use with windows
	//std::string str;
	//str=DwiFileName.substr(0,DwiFileName.find_last_of('\\')+1);
	//std::cout<< str<<std::endl;
	//::SetCurrentDirectory(str.c_str());
	
	itk::NrrdImageIO::Pointer  NrrdImageIO = itk::NrrdImageIO::New();

	if(DwiFileName.length()!=0)
	{
		try
		{
			DwiReader = DwiReaderType::New();
			DwiReader->SetImageIO(NrrdImageIO);
			DwiReader->SetFileName(DwiFileName);
			std::cout<< "Loading in IntensityMotionCheckPanel:"<<DwiFileName<<" ... ";
			DwiReader->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;
			bDwiLoaded=false;
			return false;
		}
	}
	else
	{
		std::cout<< "Dwi file name not set"<<std::endl;
		bDwiLoaded=false;
		return false;
	}
	std::cout<< "Done "<<std::endl;

	DwiImage = DwiReader->GetOutput();
	bDwiLoaded = true;  
	
	std::cout<<"Image Dimension"<< DwiImage->GetLargestPossibleRegion().GetSize().GetSizeDimension()<<": ";
	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[0]<<" ";
	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[1]<<" ";
	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[2]<<std::endl;

	std::cout<<"Pixel Vector Length: "<<DwiImage->GetVectorLength()<<std::endl;

	return bDwiLoaded;
}


bool IntensityMotionCheckPanel::GetGridentDirections( bool bDisplay)
{
	if(!bDwiLoaded) LoadDwiImage();
	if(!bDwiLoaded)
	{
		std::cout<<"DWI load error, no Gradient Direction Loaded"<<std::endl;
		bGetGridentDirections=false;
		return false;
	}		

	itk::MetaDataDictionary imgMetaDictionary = this->DwiImage->GetMetaDataDictionary();    //
	std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
	std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
	std::string metaString;

	//int numberOfImages=0;
	TensorReconstructionImageFilterType::GradientDirectionType vect3d;
	
	GradientDirectionContainer->clear();
	
	for ( ; itKey != imgMetaKeys.end(); itKey ++)
	{
		//double x,y,z;
		itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
		if (itKey->find("DWMRI_gradient") != std::string::npos)
		{ 
			std::istringstream iss(metaString);
			iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
			//sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
			//vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
			GradientDirectionContainer->push_back(vect3d);

		}
		else if (itKey->find("DWMRI_b-value") != std::string::npos)
		{
			readb0 = true;
			b0 = atof(metaString.c_str());
			//std::cout<<"b Value: "<<b0<<std::endl;
		}
	}

	if(!readb0)
	{
		std::cout<<"BValue not specified in header file" <<std::endl;
		return false;
	}
	if(GradientDirectionContainer->Size()<=6) 
	{
		std::cout<<"Gradient Images Less than 7" <<std::endl;
		bGetGridentDirections=false;
		return false;
	}

	if(bDisplay)
	{
		std::cout<<"b Value: "<<b0<<std::endl;
		std::cout<<"DWI image gradient count: "<<DwiImage->GetVectorLength()<<std::endl;

		for( unsigned int i = 0; i<DwiImage->GetVectorLength();i++ )//GradientDirectionContainer->Size()
		{
			std::cout<<"Gradient Direction "<<i<<": \t[";
			std::cout<<GradientDirectionContainer->at(i)[0]<<",\t";
			std::cout<<GradientDirectionContainer->at(i)[1]<<",\t";
			std::cout<<GradientDirectionContainer->at(i)[2]<<" ]"<<std::endl;
		}
	}

	bGetGridentDirections=true;
	return true;
}

/*
void IntensityMotionCheckPanel::on_comboBox_Protocal_currentIndexChanged(QString protocalName)
{
	//std::cout<<"current protocal: "<<protocal<<std::endl;
	treeWidget->clear();
	protocal.clear();

	if(protocalName!=tr("none"))
	{
		QString str;
		str.append( QCoreApplication::applicationDirPath() );
		str.append(tr("/"));
		str.append(protocalName);
		str.append(tr(".xml"));
		XmlStreamReader XmlReader(treeWidget);
		XmlReader.setProtocal( &protocal);
		XmlReader.readFile(str, XmlStreamReader::TreeWise);
		XmlReader.readFile(str, XmlStreamReader::ProtocalWise);
		
		protocal.print();
	}
}
*/

void IntensityMotionCheckPanel::on_treeWidget_itemChanged(QTreeWidgetItem * item, int column)
{
	//pushButton_Save->setEnabled(pushButton_Editable->isCheckable());
}

void IntensityMotionCheckPanel::on_pushButton_SaveQCReport_clicked( )
{
	if(lineEdit_QCReport->text().length()>0)
	{
	}
	else
	{
		QString reportFile = QFileDialog::getSaveFileName( this, tr("Save Results As"),lineEdit_QCReport->text(),  tr("txt Files (*.txt)") );
		if(reportFile.length()>0)
		{
			lineEdit_QCReport->setText(reportFile);
		}
	}
}

void IntensityMotionCheckPanel::on_pushButton_SaveQCReportAs_clicked( )
{
	QString reportFile = QFileDialog::getSaveFileName( this, tr("Save Results As"),lineEdit_QCReport->text(),  tr("txt Files (*.txt)") );
	if(reportFile.length()>0)
	{
		lineEdit_QCReport->setText(reportFile);
	}
}

void IntensityMotionCheckPanel::on_pushButton_SaveProtocolAs_clicked( )
{
	QString xmlFile = QFileDialog::getSaveFileName( this, tr("Save Protocal As"),lineEdit_Protocal->text(),  tr("xml Files (*.xml)") );
	if(xmlFile.length()>0)
	{
		lineEdit_Protocal->setText(xmlFile);
		XmlStreamWriter XmlWriter(treeWidget);
		XmlWriter.writeXml(xmlFile);

		//treeWidget->clear();
		protocal.clear();

		XmlStreamReader XmlReader(treeWidget);
		XmlReader.setProtocal( &protocal);
		//XmlReader.readFile(xmlFile, XmlStreamReader::TreeWise);
		XmlReader.readFile(xmlFile, XmlStreamReader::ProtocalWise);
	}

	bProtocol = true;
}

void IntensityMotionCheckPanel::on_pushButton_Save_clicked( )
{
	if(lineEdit_Protocal->text().length()>0)
	{
		XmlStreamWriter XmlWriter(treeWidget);
		XmlWriter.writeXml(lineEdit_Protocal->text());

		//treeWidget->clear();
		protocal.clear();

		XmlStreamReader XmlReader(treeWidget);
		XmlReader.setProtocal( &protocal);
		//XmlReader.readFile(lineEdit_Protocal->text(), XmlStreamReader::TreeWise);
		XmlReader.readFile(lineEdit_Protocal->text(), XmlStreamReader::ProtocalWise);
	}
	else
	{
		 QString xmlFile = QFileDialog::getSaveFileName( this, tr("Save Protocal As"),lineEdit_Protocal->text(),  tr("xml Files (*.xml)") );
		 if(xmlFile.length()>0)
		 {
			lineEdit_Protocal->setText(xmlFile);
			XmlStreamWriter XmlWriter(treeWidget);
			XmlWriter.writeXml(xmlFile);

			//treeWidget->clear();
			protocal.clear();

			XmlStreamReader XmlReader(treeWidget);
			XmlReader.setProtocal( &protocal);
			//XmlReader.readFile(xmlFile, XmlStreamReader::TreeWise);
			XmlReader.readFile(xmlFile, XmlStreamReader::ProtocalWise);

		 }
	}

	bProtocol = true;
}
/*
void IntensityMotionCheckPanel::on_pushButton_Identity_clicked( )
{
	lineEdit_MeasurementFrame11->setText(tr("1.0000"));
	lineEdit_MeasurementFrame12->setText(tr("0.0000"));
	lineEdit_MeasurementFrame13->setText(tr("0.0000"));

	lineEdit_MeasurementFrame21->setText(tr("0.0000"));
	lineEdit_MeasurementFrame22->setText(tr("1.0000"));
	lineEdit_MeasurementFrame23->setText(tr("0.0000"));

	lineEdit_MeasurementFrame31->setText(tr("0.0000"));
	lineEdit_MeasurementFrame32->setText(tr("0.0000"));
	lineEdit_MeasurementFrame33->setText(tr("1.0000"));
}

void IntensityMotionCheckPanel::on_pushButton_ExchangeXY_clicked( )
{
	double temp;

	temp = lineEdit_MeasurementFrame11->text().toDouble();
	lineEdit_MeasurementFrame11->setText( QString::number( lineEdit_MeasurementFrame12->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame12->setText( QString::number(temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame21->text().toDouble();
	lineEdit_MeasurementFrame21->setText( QString::number( lineEdit_MeasurementFrame22->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame22->setText( QString::number(temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame31->text().toDouble();
	lineEdit_MeasurementFrame31->setText( QString::number( lineEdit_MeasurementFrame32->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame32->setText( QString::number(temp,'f',4 ) );
}


void IntensityMotionCheckPanel::on_pushButton_ExchangeXZ_clicked( )
{
	double temp;

	temp = lineEdit_MeasurementFrame11->text().toDouble();
	lineEdit_MeasurementFrame11->setText( QString::number( lineEdit_MeasurementFrame13->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame13->setText( QString::number(temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame21->text().toDouble();
	lineEdit_MeasurementFrame21->setText( QString::number( lineEdit_MeasurementFrame23->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame23->setText( QString::number(temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame31->text().toDouble();
	lineEdit_MeasurementFrame31->setText( QString::number( lineEdit_MeasurementFrame33->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame33->setText( QString::number(temp,'f',4 ) );

}


void IntensityMotionCheckPanel::on_pushButton_ExchangeYZ_clicked( )
{
	double temp;

	temp = lineEdit_MeasurementFrame12->text().toDouble();
	lineEdit_MeasurementFrame12->setText( QString::number( lineEdit_MeasurementFrame13->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame13->setText( QString::number(temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame22->text().toDouble();
	lineEdit_MeasurementFrame22->setText( QString::number( lineEdit_MeasurementFrame23->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame23->setText( QString::number(temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame32->text().toDouble();
	lineEdit_MeasurementFrame32->setText( QString::number( lineEdit_MeasurementFrame33->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame33->setText( QString::number(temp,'f',4 ) );
}


void IntensityMotionCheckPanel::on_pushButton_FlipX_clicked( )
{
	double temp;

	temp = lineEdit_MeasurementFrame11->text().toDouble();
	lineEdit_MeasurementFrame11->setText( QString::number( -temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame21->text().toDouble();
	lineEdit_MeasurementFrame21->setText( QString::number( -temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame31->text().toDouble();
	lineEdit_MeasurementFrame31->setText( QString::number( -temp,'f',4 ) );
}


void IntensityMotionCheckPanel::on_pushButton_FlipY_clicked( )
{
	double temp;

	temp = lineEdit_MeasurementFrame12->text().toDouble();
	lineEdit_MeasurementFrame12->setText( QString::number( -temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame22->text().toDouble();
	lineEdit_MeasurementFrame22->setText( QString::number( -temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame32->text().toDouble();
	lineEdit_MeasurementFrame32->setText( QString::number( -temp,'f',4 ) );
}


void IntensityMotionCheckPanel::on_pushButton_FlipZ_clicked( )
{
	double temp;

	temp = lineEdit_MeasurementFrame13->text().toDouble();
	lineEdit_MeasurementFrame13->setText( QString::number( -temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame23->text().toDouble();
	lineEdit_MeasurementFrame23->setText( QString::number( -temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame33->text().toDouble();
	lineEdit_MeasurementFrame33->setText( QString::number( -temp,'f',4 ) );
}
*/
void IntensityMotionCheckPanel::UpdatePanelDWI()
{
	treeWidget_Results->clear();

	treeWidget_DiffusionInformation->clear();
	pushButton_SaveDWI->setEnabled( 0 );
	pushButton_SaveDWIAs->setEnabled( 0 );

	//pushButton_CreateDefaultProtocol->setEnabled( 1 );
	pushButton_DefaultQCResult->setEnabled( 1 );
	pushButton_OpenQCReport->setEnabled( 1 );

	this->qcResult.Clear();

	pushButton_DefaultProtocol->setEnabled( 1 );

// 	std::cout<<"tree widgets cleared"<<std::endl;

	lineEdit_SizeX->setText( QString::number(this->DwiImage->GetLargestPossibleRegion().GetSize()[0]) );
	lineEdit_SizeY->setText( QString::number(this->DwiImage->GetLargestPossibleRegion().GetSize()[1]));
	lineEdit_SizeZ->setText( QString::number(this->DwiImage->GetLargestPossibleRegion().GetSize()[2]));

	lineEdit_OriginX->setText( QString::number(this->DwiImage->GetOrigin()[0], 'f'));
	lineEdit_OriginY->setText( QString::number(this->DwiImage->GetOrigin()[1], 'f'));
	lineEdit_OriginZ->setText( QString::number(this->DwiImage->GetOrigin()[2], 'f'));

	lineEdit_SpacingX->setText( QString::number(this->DwiImage->GetSpacing()[0], 'f'));
	lineEdit_SpacingY->setText( QString::number(this->DwiImage->GetSpacing()[1], 'f'));
	lineEdit_SpacingZ->setText( QString::number(this->DwiImage->GetSpacing()[2], 'f'));

	GetGridentDirections(0);
	
	QTreeWidgetItem *bValue = new QTreeWidgetItem(treeWidget_DiffusionInformation);
	bValue->setText(0, tr("DWMRI_b-value"));
	bValue->setText(1, QString::number(b0, 'f', 0));

	for(unsigned int i=0; i< GradientDirectionContainer->size();i++)
	{
		QTreeWidgetItem *gradient = new QTreeWidgetItem(treeWidget_DiffusionInformation);
		gradient->setText(0, QString("DWMRI_gradient_%1").arg(i, 4, 10, QLatin1Char( '0' )));
		gradient->setText(1, QString("%1 %2 %3")
			.arg(GradientDirectionContainer->ElementAt(i)[0], 10, 'f', 6)
			.arg(GradientDirectionContainer->ElementAt(i)[1], 10, 'f', 6)
			.arg(GradientDirectionContainer->ElementAt(i)[2], 10, 'f', 6)
			);
	}

/////////////////////////////////////////////////

	itk::MetaDataDictionary imgMetaDictionary = DwiImage->GetMetaDataDictionary(); 
	std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
	std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
	std::string metaString;

  //  measurement frame 
  if(imgMetaDictionary.HasKey("NRRD_measurement frame"))
  {
    // measurement frame
    vnl_matrix<double> mf(3,3);
    // imaging frame
    vnl_matrix<double> imgf(3,3);
    std::vector<std::vector<double> > nrrdmf;
    itk::ExposeMetaData<std::vector<std::vector<double> > >(imgMetaDictionary,"NRRD_measurement frame",nrrdmf);

    imgf = DwiImage->GetDirection().GetVnlMatrix();
    
	//Image frame
//     std::cout << "Image frame: " << std::endl;
//     std::cout << imgf << std::endl;

	lineEdit_SpaceDir11->setText( QString::number(imgf(0,0), 'f'));
	lineEdit_SpaceDir12->setText( QString::number(imgf(0,1), 'f'));
	lineEdit_SpaceDir13->setText( QString::number(imgf(0,2), 'f'));
	lineEdit_SpaceDir21->setText( QString::number(imgf(1,0), 'f'));
	lineEdit_SpaceDir22->setText( QString::number(imgf(1,1), 'f'));
	lineEdit_SpaceDir23->setText( QString::number(imgf(1,2), 'f'));
	lineEdit_SpaceDir31->setText( QString::number(imgf(2,0), 'f'));
	lineEdit_SpaceDir32->setText( QString::number(imgf(2,1), 'f'));
	lineEdit_SpaceDir33->setText( QString::number(imgf(2,2), 'f'));

    for(unsigned int i = 0; i < 3; ++i)
    {
      for(unsigned int j = 0; j < 3; ++j)
      {
        mf(i,j) = nrrdmf[j][i];
        nrrdmf[j][i] = imgf(i,j);
      }
    }
  
	// Meausurement frame
// 	std::cout << "Meausurement frame: " << std::endl;
// 	std::cout << mf << std::endl;

	lineEdit_MeasurementFrame11->setText( QString::number(mf(0,0), 'f'));
	lineEdit_MeasurementFrame12->setText( QString::number(mf(0,1), 'f'));
	lineEdit_MeasurementFrame13->setText( QString::number(mf(0,2), 'f'));
	lineEdit_MeasurementFrame21->setText( QString::number(mf(1,0), 'f'));
	lineEdit_MeasurementFrame22->setText( QString::number(mf(1,1), 'f'));
	lineEdit_MeasurementFrame23->setText( QString::number(mf(1,2), 'f'));
	lineEdit_MeasurementFrame31->setText( QString::number(mf(2,0), 'f'));
	lineEdit_MeasurementFrame32->setText( QString::number(mf(2,1), 'f'));
	lineEdit_MeasurementFrame33->setText( QString::number(mf(2,2), 'f'));

 
    //itk::EncapsulateMetaData<std::vector<std::vector<double> > >(dict,NRRD_MEASUREMENT_KEY,nrrdmf);
  }

  // space
  itk::ExposeMetaData<std::string> (imgMetaDictionary, "NRRD_space", metaString);
  std::cout<<"space: "<<metaString.c_str()<<std::endl;
  comboBox_Space->setCurrentIndex(  comboBox_Space->findText ( QString::fromStdString( metaString), Qt::MatchExactly));
}

///////////////////////////////////
/*
void IntensityMotionCheckPanel::on_pushButton_InformationCheck_clicked( )
{

}

void IntensityMotionCheckPanel::on_pushButton_DWIInfoUpdateProtocal_clicked( )
{

}

*/
/*
void IntensityMotionCheckPanel::on_toolButton_ReportFile_clicked( )
{
	 QString ReportFile = QFileDialog::getSaveFileName ( this, tr("Save Report File As"), lineEdit_ReportFile->text() , tr("text Files (*.txt)") );
	 if (!ReportFile.isEmpty()) 
	 {
		 std::cout<<"Report File: "<<ReportFile.toStdString()<<std::endl;
		 this->lineEdit_ReportFile->setText(ReportFile);
	 }
	 return;
}

void IntensityMotionCheckPanel::on_toolButton_QCOutputNrrd_clicked( )
{
	 QString QCOutputNrrd = QFileDialog::getSaveFileName ( this, tr("Save EddyMotion Output As"), lineEdit_QCOutputNrrd->text() , tr("nrrd Files (*.nhdr)") );
	 if (!QCOutputNrrd.isEmpty()) 
	 {
		 std::cout<<"QCOutput Nrrd File: "<<QCOutputNrrd.toStdString()<<std::endl;
		 this->lineEdit_QCOutputNrrd->setText(QCOutputNrrd);
		 this->lineEdit_EddyMotionInput->setText(QCOutputNrrd);
	 }
	 return;
}

void IntensityMotionCheckPanel::on_pushButton_QCUpdateProtocol_clicked( )
{

}

void IntensityMotionCheckPanel::on_pushButton_QCCheck_clicked( )
{

}
*/
/*
void IntensityMotionCheckPanel::on_toolButton_EddyMotionCommand_clicked( )
{
	QString EddyMotionCommand= QFileDialog::getOpenFileName(this, tr("Set the dtiestim command"),lineEdit_EddyMotionCommand->text());
	
	 if (!EddyMotionCommand.isEmpty()) 
	 {
		 std::cout<<"EddyMotion  Command: "<<EddyMotionCommand.toStdString()<<std::endl;
		 this->lineEdit_EddyMotionCommand->setText(EddyMotionCommand);
	 }
}

void IntensityMotionCheckPanel::on_toolButton_EddyMotionInput_clicked( )
{
	QString EddyMotionInput= QFileDialog::getOpenFileName(this, tr("Set tEddyMotion Input"),lineEdit_EddyMotionInput->text(), tr("nrrd Files (*.nhdr)") );
	
	 if (!EddyMotionInput.isEmpty()) 
	 {
		 std::cout<<"EddyMotion Input: "<<EddyMotionInput.toStdString()<<std::endl;
		 this->lineEdit_EddyMotionInput->setText(EddyMotionInput);
	 }
}

void IntensityMotionCheckPanel::on_toolButton_EddyMotionOutput_clicked( )
{
	 QString EddyMotionOutput = QFileDialog::getSaveFileName ( this, tr("Save EddyMotion Output As"), lineEdit_EddyMotionOutput->text() , tr("nrrd Files (*.nhdr)") );
	 if (!EddyMotionOutput.isEmpty()) 
	 {
		 std::cout<<"EddyMotion Output File: "<<EddyMotionOutput.toStdString()<<std::endl;
		 this->lineEdit_EddyMotionOutput->setText(EddyMotionOutput);
	 }
	 return;
}

void IntensityMotionCheckPanel::on_pushButton_CorrectUpdateProtocol_clicked( )
{

}

void IntensityMotionCheckPanel::on_pushButton_EddyMotionCorrect_clicked( )
{

}


void IntensityMotionCheckPanel::on_toolButton_dtiestim_clicked( )
{
	QString dtiestimCommand= QFileDialog::getOpenFileName(this, tr("Set the dtiestim command"),lineEdit_dtiestim->text());
	
	 if (!dtiestimCommand.isEmpty()) 
	 {
		 std::cout<<"dtiestim command: "<<dtiestimCommand.toStdString()<<std::endl;
		 this->lineEdit_dtiestim->setText(dtiestimCommand);
	 }
}

void IntensityMotionCheckPanel::on_toolButton_dtiprocess_clicked( )
{
	QString dtiprocessCommand= QFileDialog::getOpenFileName(this, tr("Set the dtiprocess command"),lineEdit_dtiprocess->text());
	
	 if (!dtiprocessCommand.isEmpty()) 
	 {
		 std::cout<<"dtiprocess command: "<<dtiprocessCommand.toStdString()<<std::endl;
		 this->lineEdit_dtiprocess->setText(dtiprocessCommand);
	 }
}

void IntensityMotionCheckPanel::on_toolButton_MaskFile_clicked( )
{
	QString MaskFile= QFileDialog::getOpenFileName(this, tr("Set the Mask File"),lineEdit_MaskFile->text());
	
	 if (!MaskFile.isEmpty()) 
	 {
		 std::cout<<"Mask File: "<<MaskFile.toStdString()<<std::endl;
		 this->lineEdit_MaskFile->setText(MaskFile);
	 }

}

void IntensityMotionCheckPanel::on_toolButton_TensorFile_clicked( )
{
	 QString TensorFile = QFileDialog::getSaveFileName ( this, tr("Save Tensor Files As"), lineEdit_TensorFile->text() , tr("Tensor Files (*.nhdr)") );
	 if (!TensorFile.isEmpty()) 
	 {
		 std::cout<<"Tensor File: "<<TensorFile.toStdString()<<std::endl;
		 //nrrdPath=nrrFile;
	     this->lineEdit_TensorFile->setText(TensorFile);
	 }
	 return;
}


void IntensityMotionCheckPanel::on_pushButton_DTIUpdateProtocol_clicked( )
{

}

void IntensityMotionCheckPanel::on_pushButton_DTICompute_clicked( )
{

}
*/



void IntensityMotionCheckPanel::on_pushButton_DefaultProtocol_clicked( )
{
	CreateDefaultProtocol();

	pushButton_Save->setEnabled( 1 );
	pushButton_SaveProtocolAs->setEnabled( 1 );
	
	bProtocolTreeEditable = true;
}


//void IntensityMotionCheckPanel::on_pushButton_CreateDefaultProtocol_clicked( )
//{
//  CreateDefaultProtocol();
//}
void IntensityMotionCheckPanel::CreateDefaultProtocol()
{

	this->GetProtocal().clear();

	this->GetProtocal().GetReportFileName() = "_QCReport.txt"; //////////////////

	// image
	this->GetProtocal().GetImageProtocal().bCheck = true;

	this->GetProtocal().GetImageProtocal().size[0] = DwiImage->GetLargestPossibleRegion().GetSize()[0];
	this->GetProtocal().GetImageProtocal().size[1] = DwiImage->GetLargestPossibleRegion().GetSize()[1];
	this->GetProtocal().GetImageProtocal().size[2] = DwiImage->GetLargestPossibleRegion().GetSize()[2];

	this->GetProtocal().GetImageProtocal().origin[0] = DwiImage->GetOrigin()[0];
	this->GetProtocal().GetImageProtocal().origin[1] = DwiImage->GetOrigin()[1];
	this->GetProtocal().GetImageProtocal().origin[2] = DwiImage->GetOrigin()[2];

	this->GetProtocal().GetImageProtocal().spacing[0] = DwiImage->GetSpacing()[0];
	this->GetProtocal().GetImageProtocal().spacing[1] = DwiImage->GetSpacing()[1];
	this->GetProtocal().GetImageProtocal().spacing[2] = DwiImage->GetSpacing()[2];


	// diffusion
	GetGridentDirections( 0 );

	this->GetProtocal().GetDiffusionProtocal().bCheck = true;
	this->GetProtocal().GetDiffusionProtocal().b = this->b0;

	for(unsigned int i=0; i< GradientDirectionContainer->size();i++)
	{
		std::vector<double> vect;
		vect.push_back(GradientDirectionContainer->ElementAt(i)[0]);
		vect.push_back(GradientDirectionContainer->ElementAt(i)[1]);
		vect.push_back(GradientDirectionContainer->ElementAt(i)[2]);

		this->GetProtocal().GetDiffusionProtocal().gradients.push_back(vect);
	}

	itk::MetaDataDictionary imgMetaDictionary = DwiImage->GetMetaDataDictionary(); 
	std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
	std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
	std::string metaString;

	//  measurement frame 
	if(imgMetaDictionary.HasKey("NRRD_measurement frame"))
	{
		// measurement frame
		vnl_matrix<double> mf(3,3);
		// imaging frame
		vnl_matrix<double> imgf(3,3);
		std::vector<std::vector<double> > nrrdmf;
		itk::ExposeMetaData<std::vector<std::vector<double> > >(imgMetaDictionary,"NRRD_measurement frame",nrrdmf);

		imgf = DwiImage->GetDirection().GetVnlMatrix();

		//Image frame
		this->GetProtocal().GetImageProtocal().spacedirection[0][0] = imgf(0,0);
		this->GetProtocal().GetImageProtocal().spacedirection[0][1] = imgf(0,1);
		this->GetProtocal().GetImageProtocal().spacedirection[0][2] = imgf(0,2);
		this->GetProtocal().GetImageProtocal().spacedirection[1][0] = imgf(1,0);
		this->GetProtocal().GetImageProtocal().spacedirection[1][1] = imgf(1,1);
		this->GetProtocal().GetImageProtocal().spacedirection[1][2] = imgf(1,2);
		this->GetProtocal().GetImageProtocal().spacedirection[2][0] = imgf(2,0);
		this->GetProtocal().GetImageProtocal().spacedirection[2][1] = imgf(2,1);
		this->GetProtocal().GetImageProtocal().spacedirection[2][2] = imgf(2,2);
		

		for(unsigned int i = 0; i < 3; ++i)
		{
			for(unsigned int j = 0; j < 3; ++j)
			{
				mf(i,j) = nrrdmf[j][i];
				nrrdmf[j][i] = imgf(i,j);
			}
		}

		// Meausurement frame
		this->GetProtocal().GetDiffusionProtocal().measurementFrame[0][0] = mf(0,0);
		this->GetProtocal().GetDiffusionProtocal().measurementFrame[0][1] = mf(0,1);
		this->GetProtocal().GetDiffusionProtocal().measurementFrame[0][2] = mf(0,2);
		this->GetProtocal().GetDiffusionProtocal().measurementFrame[1][0] = mf(1,0);
		this->GetProtocal().GetDiffusionProtocal().measurementFrame[1][1] = mf(1,1);
		this->GetProtocal().GetDiffusionProtocal().measurementFrame[1][2] = mf(1,2);
		this->GetProtocal().GetDiffusionProtocal().measurementFrame[2][0] = mf(2,0);
		this->GetProtocal().GetDiffusionProtocal().measurementFrame[2][1] = mf(2,1);
		this->GetProtocal().GetDiffusionProtocal().measurementFrame[2][2] = mf(2,2);
	}
	// space
	itk::ExposeMetaData<std::string> (imgMetaDictionary, "NRRD_space", metaString);

	if( metaString == "left-anterior-inferior")
		this->GetProtocal().GetImageProtocal().space = Protocal::SPACE_LAI;
	else if( metaString =="left-anterior-superior")
		this->GetProtocal().GetImageProtocal().space = Protocal::SPACE_LAS;
	else if( metaString =="left-posterior-inferior")
		this->GetProtocal().GetImageProtocal().space = Protocal::SPACE_LPI;
	else if( metaString =="left-posterior-superior")
		this->GetProtocal().GetImageProtocal().space = Protocal::SPACE_LPS;
	else if( metaString =="right-anterior-inferior")
		this->GetProtocal().GetImageProtocal().space = Protocal::SPACE_RAI;
	else if( metaString =="right-anterior-superior")
		this->GetProtocal().GetImageProtocal().space = Protocal::SPACE_RAS;
	else if( metaString =="right-posterior-inferior")
		this->GetProtocal().GetImageProtocal().space = Protocal::SPACE_RPI;
	else if( metaString =="right-posterior-superior")
		this->GetProtocal().GetImageProtocal().space = Protocal::SPACE_RPS;
	else
		this->GetProtocal().GetImageProtocal().space = Protocal::SPACE_UNKNOWN;

	// intensity Motion check
	this->GetProtocal().GetIntensityMotionCheckProtocal().bCheck = true;
	this->GetProtocal().GetIntensityMotionCheckProtocal().OutputFileName = "_QCOutput.nhdr"; //////////////////
	this->GetProtocal().GetIntensityMotionCheckProtocal().badGradientPercentageTolerance = 0.1;

	emit status("Estimating protocol parameter  ...");
	std::cout<< "Estimating protocol parameter  ..." <<std::endl;

	double sliceBaselineThreshold, sliceGradientThreshold, devBaselineThreshold, devGradientThreshold;
	GetSliceProtocolParameters( 
		0.15, // get the 10% threshold and check with 15%: get 10%-90% image threshold and ckeck 15%-85% range
		0.15, // get the 10% threshold and check with 15%
		sliceBaselineThreshold ,  
		sliceGradientThreshold ,
		devBaselineThreshold, 
		devGradientThreshold
		);
	std::cout << "sliceBaselineThreshold: "<<sliceBaselineThreshold<<std::endl;
	std::cout << "sliceGradientThreshold: "<<sliceGradientThreshold<<std::endl;
	std::cout << "devBaselineThreshold: "<<devBaselineThreshold<<std::endl;
	std::cout << "devGradientThreshold: "<<devGradientThreshold<<std::endl;

	double interlaceBaselineThreshold, interlaceGradientThreshold, interlaceBaselineDev, interlaceGradientDev;
	GetInterlaceProtocolParameters(
		interlaceBaselineThreshold, 
		interlaceGradientThreshold,
		interlaceBaselineDev,
		interlaceGradientDev
		);
	std::cout << "interlaceBaselineThreshold: "<<interlaceBaselineThreshold<<std::endl;
	std::cout << "interlaceGradientThreshold: "<<interlaceGradientThreshold<<std::endl;
	std::cout << "interlaceBaselineDev: "<<interlaceBaselineDev<<std::endl;
	std::cout << "interlaceGradientDev: "<<interlaceGradientDev<<std::endl;

	emit status("done");
	std::cout<< "Estimating protocol parameter  ... done" <<std::endl;


	this->GetProtocal().GetIntensityMotionCheckProtocal().bSliceCheck = true;
	this->GetProtocal().GetIntensityMotionCheckProtocal().badSlicePercentageTolerance = 0.0;
	this->GetProtocal().GetIntensityMotionCheckProtocal().baselineCorrelationThreshold = sliceBaselineThreshold*0.90; //0.8; // to be optimized
	this->GetProtocal().GetIntensityMotionCheckProtocal().baselineCorrelationDeviationThreshold = 3.0; //devBaselineThreshold*1.1;//3.50;
	this->GetProtocal().GetIntensityMotionCheckProtocal().headSkipSlicePercentage = 0.15; 
	this->GetProtocal().GetIntensityMotionCheckProtocal().tailSkipSlicePercentage = 0.15;  
	this->GetProtocal().GetIntensityMotionCheckProtocal().sliceCorrelationThreshold = sliceGradientThreshold*0.90; //0.8;// to be optimized
	this->GetProtocal().GetIntensityMotionCheckProtocal().sliceCorrelationDeviationThreshold = 3.50; //devGradientThreshold*1.1;//3.50;//

	this->GetProtocal().GetIntensityMotionCheckProtocal().bInterlaceCheck = true;
	this->GetProtocal().GetIntensityMotionCheckProtocal().interlaceCorrelationThresholdBaseline = interlaceBaselineThreshold*0.9; //0.92;// to be optimized
	this->GetProtocal().GetIntensityMotionCheckProtocal().interlaceCorrelationDeviationBaseline = 3.00;//interlaceBaselineDev*1.05;//3.00;//
	this->GetProtocal().GetIntensityMotionCheckProtocal().interlaceCorrelationThresholdGradient = interlaceGradientThreshold*0.9; //0.90;//  to be optimized
	this->GetProtocal().GetIntensityMotionCheckProtocal().interlaceCorrelationDeviationGradient = 3.00;//interlaceGradientDev*1.05;//3.00;//
	this->GetProtocal().GetIntensityMotionCheckProtocal().interlaceRotationThreshold = 0.5;// degree
	this->GetProtocal().GetIntensityMotionCheckProtocal().interlaceTranslationThreshold = ( this->GetProtocal().GetImageProtocal().spacing[0] +
																							this->GetProtocal().GetImageProtocal().spacing[1] +
																							this->GetProtocal().GetImageProtocal().spacing[2] )*0.33333333333;//1 xoxel;//

	this->GetProtocal().GetIntensityMotionCheckProtocal().bGradientCheck = true;
	this->GetProtocal().GetIntensityMotionCheckProtocal().gradientRotationThreshold = 0.8; //degree
	this->GetProtocal().GetIntensityMotionCheckProtocal().gradientTranslationThreshold = (	this->GetProtocal().GetImageProtocal().spacing[0] +
																							this->GetProtocal().GetImageProtocal().spacing[1] +
																							this->GetProtocal().GetImageProtocal().spacing[2] )*0.33333333333*1.50;//3 voxel;//

	// Eddy motion correction
	this->GetProtocal().GetEddyMotionCorrectionProtocal().bCorrect = true;
	this->GetProtocal().GetEddyMotionCorrectionProtocal().EddyMotionCommand = "/tools/bin_linux64/EddyMotionCorrector";
	this->GetProtocal().GetEddyMotionCorrectionProtocal().InputFileName = "_QCOutput.nhdr";
	this->GetProtocal().GetEddyMotionCorrectionProtocal().OutputFileName = "_EddyMotion_Output.nhdr";

	// DTI
	this->GetProtocal().GetDTIProtocal().bCompute = true;
	this->GetProtocal().GetDTIProtocal().dtiestimCommand = "/tools/bin_linux64/dtiestim";
	this->GetProtocal().GetDTIProtocal().dtiprocessCommand = "/tools/bin_linux64/dtiprocess";
	this->GetProtocal().GetDTIProtocal().dtiPaddingCommand = "/tools/bin_linux64/CropDTI";
	this->GetProtocal().GetDTIProtocal().baselineThreshold = 50; //
	this->GetProtocal().GetDTIProtocal().mask = "";
	this->GetProtocal().GetDTIProtocal().method = Protocal::METHOD_WLS;
	this->GetProtocal().GetDTIProtocal().tensor = "_DTI.nhdr";
	this->GetProtocal().GetDTIProtocal().bPadding = true;
// 	this->GetProtocal().GetDTIProtocal().tensorPadded = "_DTI.nhdr";
// 	this->GetProtocal().GetDTIProtocal().paddingParameters[0] = 0;
// 	this->GetProtocal().GetDTIProtocal().paddingParameters[1] = 0;
// 	this->GetProtocal().GetDTIProtocal().paddingParameters[2] = 0;
// 	this->GetProtocal().GetDTIProtocal().paddingParameters[3] = 0;
// 	this->GetProtocal().GetDTIProtocal().paddingParameters[4] = 0;
// 	this->GetProtocal().GetDTIProtocal().paddingParameters[5] = 0;
	this->GetProtocal().GetDTIProtocal().bbaseline = true;
	this->GetProtocal().GetDTIProtocal().baseline = "_Baseline.nhdr";
	this->GetProtocal().GetDTIProtocal().bidwi = true;
	this->GetProtocal().GetDTIProtocal().idwi = "_IDWI.nhdr";
	this->GetProtocal().GetDTIProtocal().bfa = true;
	this->GetProtocal().GetDTIProtocal().fa = "_FA.nhdr";
	this->GetProtocal().GetDTIProtocal().bmd = true;
	this->GetProtocal().GetDTIProtocal().md = "_MD.nhdr";
	this->GetProtocal().GetDTIProtocal().bcoloredfa = true;
	this->GetProtocal().GetDTIProtocal().coloredfa = "_colorFA.nhdr";
	this->GetProtocal().GetDTIProtocal().bfrobeniusnorm = true;
	this->GetProtocal().GetDTIProtocal().frobeniusnorm = "_frobeniusnorm.nhdr";

	//////////////////////////////////////////////////////////////////////////
	UpdateProtocolTree();
	pushButton_RunPipeline->setEnabled(1);
	
	//on_pushButton_Save_clicked( );

	bProtocol = true;
}

bool IntensityMotionCheckPanel::GetSliceProtocolParameters(
									   double beginSkip, 
									   double endSkip, 
									   double &baselineCorrelationThreshold ,  
									   double &gradientCorrelationThreshold	,								   
									   double &baselineCorrelationDeviationThreshold ,  
									   double &gradientCorrelationDeviationThreshold									   
									   )
{
	//emit Progress(j+1/DwiImage->GetVectorLength());//emit QQQ(10);

	if(!bDwiLoaded  ) LoadDwiImage();
	if(!bDwiLoaded  )
	{
		std::cout<<"DWI load error, no Gradient Images got"<<std::endl;
		return false;
	}	

	std::vector< std::vector<struIntra2DResults> >		ResultsContainer;

	typedef itk::VectorIndexSelectionCastImageFilter<DwiImageType, GradientImageType > FilterType;
	FilterType::Pointer componentExtractor = FilterType::New();
	componentExtractor->SetInput(DwiImage);

	typedef itk::ExtractImageFilter< GradientImageType, SliceImageType > ExtractFilterType;
	ExtractFilterType::Pointer filter1 = ExtractFilterType::New();
	ExtractFilterType::Pointer filter2 = ExtractFilterType::New();

	for( unsigned int j = 0; j<DwiImage->GetVectorLength(); j++ )
	{
		componentExtractor->SetIndex( j );
		componentExtractor->Update();

		GradientImageType::RegionType inputRegion =componentExtractor->GetOutput()->GetLargestPossibleRegion();
		GradientImageType::SizeType size = inputRegion.GetSize();
		size[2] = 0;

		GradientImageType::IndexType start1 = inputRegion.GetIndex();
		GradientImageType::IndexType start2 = inputRegion.GetIndex();


		std::vector< struIntra2DResults >	Results;

		filter1->SetInput( componentExtractor->GetOutput() );
		filter2->SetInput( componentExtractor->GetOutput() );	

		for(unsigned int i=1; i<componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[2]; i++)
		{
			start1[2] = i-1;
			start2[2] = i;

			GradientImageType::RegionType desiredRegion1;
			desiredRegion1.SetSize( size );
			desiredRegion1.SetIndex( start1 );
			filter1->SetExtractionRegion( desiredRegion1 );

			GradientImageType::RegionType desiredRegion2;
			desiredRegion2.SetSize( size );
			desiredRegion2.SetIndex( start2 );
			filter2->SetExtractionRegion( desiredRegion2 );

			filter1->Update();
			filter2->Update();

			CIntraGradientRigidRegistration IntraGradientRigidReg(filter1->GetOutput(),filter2->GetOutput());
			struIntra2DResults s2DResults= IntraGradientRigidReg.Run( 0 /* No registration */ );
			Results.push_back(s2DResults);
		}
		ResultsContainer.push_back(Results);
	}


	GetGridentDirections(0);
	int DWICount, BaselineCount;

	DWICount=0;
	BaselineCount=0;
	for ( unsigned int i=0;i< GradientDirectionContainer->size() ;i++ )
	{
		if( GradientDirectionContainer->ElementAt(i)[0] == 0.0 && GradientDirectionContainer->ElementAt(i)[1] == 0.0 && GradientDirectionContainer->ElementAt(i)[2] == 0.0)
			BaselineCount++;
		else
			DWICount++;
	}

	std::cout<<"BaselineCount: "<<BaselineCount<<std::endl;
	std::cout<<"DWICount: "<<DWICount<<std::endl;

	std::vector<double> baselineCorrelationMean;
	std::vector<double> gradientCorrelationMean;

	std::vector<double> baselineCorrelationMin;
	std::vector<double> gradientCorrelationMin;

	std::vector<double> baselineCorrelationDev;
	std::vector<double> gradientCorrelationDev;

	for(unsigned int j=0;j<ResultsContainer[0].size();j++)
	{
		double baselinemean=0.0, DWImean=0.0, baselinedeviation=0.0, DWIdeviation=0.0, baselineMin=1.0, gradientMin=1.0;
		for(unsigned int i=0; i<ResultsContainer.size(); i++)
		{
			if ( GradientDirectionContainer->at(i)[0] == 0.0 && GradientDirectionContainer->at(i)[1] == 0.0 && GradientDirectionContainer->at(i)[2] == 0.0 )
			{
				baselinemean+=ResultsContainer[i][j].Correlation/(double)(BaselineCount);
				if( ResultsContainer[i][j].Correlation < baselineMin ) baselineMin = ResultsContainer[i][j].Correlation;
			}
			else
			{
				DWImean+=ResultsContainer[i][j].Correlation/(double)(DWICount);
				if( ResultsContainer[i][j].Correlation < gradientMin ) gradientMin = ResultsContainer[i][j].Correlation;
			}				
		}

		baselineCorrelationMean.push_back(baselinemean);
		gradientCorrelationMean.push_back(DWImean);

		baselineCorrelationMin.push_back(baselineMin);
		gradientCorrelationMin.push_back(gradientMin);

		for(unsigned int i=0; i<ResultsContainer.size(); i++)
		{
			if ( GradientDirectionContainer->at(i)[0] == 0.0 && GradientDirectionContainer->at(i)[1] == 0.0 && GradientDirectionContainer->at(i)[2] == 0.0)
			{
				if(BaselineCount>=1)
					baselinedeviation+=(ResultsContainer[i][j].Correlation-baselinemean)*(ResultsContainer[i][j].Correlation-baselinemean)/(double)(BaselineCount);
				else
					baselinedeviation=0.0;
			}
			else
				DWIdeviation+=(ResultsContainer[i][j].Correlation-DWImean)*(ResultsContainer[i][j].Correlation-DWImean)/(double)(DWICount);
		}

		baselineCorrelationDev.push_back(sqrt(baselinedeviation));
		gradientCorrelationDev.push_back(sqrt(DWIdeviation));
	}

	double minBaselineCorrelation = 1.0;
	double minGradientCorrelation = 1.0;

	double maxBaselineCorrelationDev = 0.0;
	double maxGradientCorrelationDev = 0.0;

	double maxBaselineCorrelationDevTime = 0.0;
	double maxGradientCorrelationDevTime = 0.0;

	int sliceNum = DwiImage->GetLargestPossibleRegion().GetSize()[2];
	for(int i=0 + (int)( sliceNum * beginSkip);i< sliceNum - (int)( sliceNum * endSkip); i++) //for(int j=0;j<ResultsContainer[0].size()-1; j++)
	{
		if( baselineCorrelationMin[i] < minBaselineCorrelation) 
			minBaselineCorrelation = baselineCorrelationMin[i];

		if( gradientCorrelationMin[i] < minGradientCorrelation) 
			minGradientCorrelation = gradientCorrelationMin[i];

		if( baselineCorrelationDev[i] > maxBaselineCorrelationDev) 
			maxBaselineCorrelationDev = baselineCorrelationDev[i];

		if( gradientCorrelationDev[i] > maxGradientCorrelationDev) 
			maxGradientCorrelationDev = gradientCorrelationDev[i];

		if( (baselineCorrelationMean[i]-baselineCorrelationMin[i])/baselineCorrelationDev[i] > maxBaselineCorrelationDevTime)
			maxBaselineCorrelationDevTime = (baselineCorrelationMean[i]-baselineCorrelationMin[i])/baselineCorrelationDev[i];

		if( (gradientCorrelationMean[i]-gradientCorrelationMin[i])/gradientCorrelationDev[i] > maxGradientCorrelationDevTime)
			maxGradientCorrelationDevTime = (gradientCorrelationMean[i]-gradientCorrelationMin[i])/gradientCorrelationDev[i];

	}

	baselineCorrelationThreshold = minBaselineCorrelation;//minBaselineCorrelation - maxBaselineCorrelationDev*2.0;
	gradientCorrelationThreshold = minGradientCorrelation;// - maxGradientCorrelationDev*2.0;

	baselineCorrelationDeviationThreshold =  maxBaselineCorrelationDevTime;
	gradientCorrelationDeviationThreshold =  maxGradientCorrelationDevTime;

	std::cout <<"minBaselineCorrelation: "<<minBaselineCorrelation<<std::endl;
	std::cout <<"minGradientCorrelation: "<<minGradientCorrelation<<std::endl;
	std::cout <<"maxBaselineCorrelationDev: "<<maxBaselineCorrelationDev<<std::endl;
	std::cout <<"maxGradientCorrelationDev: "<<maxGradientCorrelationDev<<std::endl;
	std::cout <<"maxBaselineCorrelationDevTime: "<<maxBaselineCorrelationDevTime<<std::endl;
	std::cout <<"maxGradientCorrelationDevTime: "<<maxGradientCorrelationDevTime<<std::endl;


	return true;
}


bool IntensityMotionCheckPanel::GetInterlaceProtocolParameters(
	double &correlationThresholdBaseline, 
	double &correlationThresholdGradient,
	double &correlationBaselineDevTimes, 
	double &correlationGradientDevTimes
	)
{
	if(!bDwiLoaded ) LoadDwiImage();
	if(!bDwiLoaded )
	{
		std::cout<<"DWI load error, no Gradient Images got"<<std::endl;
		return false;
	}	

	GetGridentDirections(0);

	std::vector< double >	baselineCorrelation;;
	std::vector< double >	gradientCorrelation;;

	typedef itk::VectorIndexSelectionCastImageFilter<DwiImageType, GradientImageType > FilterType;
	FilterType::Pointer componentExtractor = FilterType::New();

	componentExtractor->SetInput(DwiImage);

	GradientImageType::Pointer InterlaceOdd  = GradientImageType::New();
	GradientImageType::Pointer InterlaceEven = GradientImageType::New();

	componentExtractor->SetIndex( 0 );
	componentExtractor->Update();

	GradientImageType::RegionType region;
	GradientImageType::SizeType size;
	size[0]= componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[0];
	size[1]= componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[1];
	size[2]= componentExtractor->GetOutput()->GetLargestPossibleRegion().GetSize()[2]/2;
	region.SetSize( size );

	GradientImageType::SpacingType spacing;
	spacing = componentExtractor->GetOutput()->GetSpacing();

	InterlaceOdd->CopyInformation(componentExtractor->GetOutput());
	InterlaceEven->CopyInformation(componentExtractor->GetOutput());

	InterlaceOdd->SetRegions( region );
	InterlaceEven->SetRegions( region );

	InterlaceOdd->Allocate();
	InterlaceEven->Allocate();

	typedef itk::ImageRegionIteratorWithIndex< GradientImageType > IteratorType;
	IteratorType iterateOdd( InterlaceOdd, InterlaceOdd->GetLargestPossibleRegion() );
	IteratorType iterateEven( InterlaceEven, InterlaceEven->GetLargestPossibleRegion() );

	for( unsigned int j = 0; j<DwiImage->GetVectorLength(); j++ )
	{
		componentExtractor->SetIndex( j );
		componentExtractor->Update();

		typedef itk::ImageRegionIteratorWithIndex< GradientImageType > IteratorType;
		IteratorType iterateGradient( componentExtractor->GetOutput(), componentExtractor->GetOutput()->GetLargestPossibleRegion() );

		iterateGradient.GoToBegin();
		iterateOdd.GoToBegin();
		iterateEven.GoToBegin();

		unsigned long count=0;
		while (!iterateGradient.IsAtEnd())
		{
			if(count<size[0]*size[1]*size[2]*2)
			{
				if( (count/(size[0]*size[1]))%2 == 0)	
				{
					iterateEven.Set(iterateGradient.Get());
					++iterateEven;
				}
				if( (count/(size[0]*size[1]))%2 == 1)
				{
					iterateOdd.Set(iterateGradient.Get());
					++iterateOdd;
				}
			}
			++iterateGradient;
			++count;			
		}

		typedef itk::ImageRegionConstIterator<GradientImageType>  citType;
		citType cit1(InterlaceOdd, InterlaceOdd ->GetBufferedRegion() );
		citType cit2(InterlaceEven,InterlaceEven->GetBufferedRegion() );

		cit1.GoToBegin();
		cit2.GoToBegin();

		double Correlation;
		double sAB=0.0,sA2=0.0,sB2=0.0;
		while (!cit1.IsAtEnd())
		{
			sAB+=cit1.Get()*cit2.Get();
			sA2+=cit1.Get()*cit1.Get();
			sB2+=cit2.Get()*cit2.Get();
			++cit1;
			++cit2;
		}
		Correlation=sAB/sqrt(sA2*sB2);

		if ( GradientDirectionContainer->at(j)[0] == 0.0 && GradientDirectionContainer->at(j)[1] == 0.0 && GradientDirectionContainer->at(j)[2] == 0.0)
			baselineCorrelation.push_back(Correlation);
		else
			gradientCorrelation.push_back(Correlation);

// 		std::cout<<"Correlation: " << Correlation<< std::endl;
	}


	int DWICount, BaselineCount;

	DWICount=0;
	BaselineCount=0;
	for (unsigned int i=0;i< GradientDirectionContainer->size() ;i++ )
	{
		if( GradientDirectionContainer->ElementAt(i)[0] == 0.0 && GradientDirectionContainer->ElementAt(i)[1] == 0.0 && GradientDirectionContainer->ElementAt(i)[2] == 0.0)
			BaselineCount++;
		else
			DWICount++;
	}

	std::cout<<"BaselineCount: "<<BaselineCount<<std::endl;
	std::cout<<"DWICount: "<<DWICount<<std::endl;
	
	double minBaselineCorrelation = 1.0;
	double minGradientCorrelation = 1.0;

	double meanBaselineCorrelation = 0.0;
	double meanGradientCorrelation = 0.0;

	double baselineCorrelationDev = 0.0;
	double gradientCorrelationDev = 0.0;

	for(unsigned int i=0; i< baselineCorrelation.size(); i++)
	{
			if( baselineCorrelation[i] < minBaselineCorrelation )
				minBaselineCorrelation = baselineCorrelation[i];
			meanBaselineCorrelation += baselineCorrelation[i]/baselineCorrelation.size();
	}

	for(unsigned int i=0; i< baselineCorrelation.size(); i++)
	{
		baselineCorrelationDev += ( baselineCorrelation[i] - meanBaselineCorrelation )*( baselineCorrelation[i] -meanBaselineCorrelation )/baselineCorrelation.size() ;//meanBaselineCorrelation += baselineCorrelation[i]/baselineCorrelation.size();
	}
	baselineCorrelationDev = sqrt(baselineCorrelationDev);


	for(unsigned int i=0; i< gradientCorrelation.size(); i++)
	{
		if( gradientCorrelation[i] < minGradientCorrelation )
			minGradientCorrelation = gradientCorrelation[i];	
		meanGradientCorrelation += gradientCorrelation[i]/gradientCorrelation.size();
	}

	for(unsigned int i=0; i< gradientCorrelation.size(); i++)
	{
		gradientCorrelationDev += ( gradientCorrelation[i] - meanGradientCorrelation )*( gradientCorrelation[i] -meanGradientCorrelation )/gradientCorrelation.size() ;//meanBaselineCorrelation += baselineCorrelation[i]/baselineCorrelation.size();
	}
	gradientCorrelationDev = sqrt(gradientCorrelationDev);

 // return values
	correlationThresholdBaseline = minBaselineCorrelation; 
	correlationThresholdGradient = minGradientCorrelation;

	double maxBaselineCorrelationDevTimes ;
	double maxGradientCorrelationDevTimes ;

	maxBaselineCorrelationDevTimes = (meanBaselineCorrelation-minBaselineCorrelation)/baselineCorrelationDev;
	maxGradientCorrelationDevTimes = (meanGradientCorrelation-minGradientCorrelation)/gradientCorrelationDev;
 // return values

	correlationBaselineDevTimes = maxBaselineCorrelationDevTimes;
	correlationGradientDevTimes = maxGradientCorrelationDevTimes;

	std::cout<<"minBaselineCorrelation: "<<minBaselineCorrelation<<std::endl;
	std::cout<<"minGradientCorrelation: "<<minGradientCorrelation<<std::endl;
	std::cout<<"baselineCorrelationDev: "<<baselineCorrelationDev<<std::endl;
	std::cout<<"gradientCorrelationDev: "<<gradientCorrelationDev<<std::endl;
	std::cout<<"maxBaselineCorrelationDevTimes: "<<maxBaselineCorrelationDevTimes<<std::endl;
	std::cout<<"maxGradientCorrelationDevTimes: "<<maxGradientCorrelationDevTimes<<std::endl;

	return true;
}

void IntensityMotionCheckPanel::UpdateProtocolTree( )
{
	lineEdit_Protocal->clear();
	treeWidget->clear();

	//////////////////////////////////////////////////////////////////////////
	QTreeWidgetItem *itemReportFileName = new QTreeWidgetItem(treeWidget);
	itemReportFileName->setText(0, tr("ReportFileName"));
	itemReportFileName->setText(1, QString::fromStdString(this->GetProtocal().GetReportFileName()) );

	// image
	QTreeWidgetItem *itemImageInformation = new QTreeWidgetItem(treeWidget);
	itemImageInformation->setText(0, tr("Image Check"));
	if(this->GetProtocal().GetImageProtocal().bCheck)
		itemImageInformation->setText(1,tr("Yes"));
	else
		itemImageInformation->setText(1,tr("No"));

	QTreeWidgetItem *itemSpace = new QTreeWidgetItem(itemImageInformation);
	itemSpace->setText(0, tr("space"));
	switch(this->GetProtocal().GetImageProtocal().space)
	{
		case Protocal::SPACE_LPS:
			itemSpace->setText(1, tr("left-posterior-superior"));
			break;
		case Protocal::SPACE_LPI:
			itemSpace->setText(1, tr("left-posterior-inferior"));
			break;
		case Protocal::SPACE_LAS:
			itemSpace->setText(1, tr("left-anterior-superior"));
			break;
		case Protocal::SPACE_LAI:
			itemSpace->setText(1, tr("left-anterior-inferior"));
			break;
		case Protocal::SPACE_RPS:
			itemSpace->setText(1, tr("right-posterior-superior"));
			break;
		case Protocal::SPACE_RPI:
			itemSpace->setText(1, tr("right-posterior-inferior"));
			break;
		case Protocal::SPACE_RAS:
			itemSpace->setText(1, tr("right-anterior-superior"));
			break;
		case Protocal::SPACE_RAI:
			itemSpace->setText(1, tr("right-anterior-inferior"));
			break;
		default:
			itemSpace->setText(1, tr("SPACE_UNKNOWN"));
			break;
	}

	QTreeWidgetItem *itemSpaceDirections = new QTreeWidgetItem(itemImageInformation);
	itemSpaceDirections->setText(0, tr("space directions"));
	itemSpaceDirections->setText(1, QString("%1 %2 %3, %4 %5 %6, %7 %8 %9")
		.arg(this->GetProtocal().GetImageProtocal().spacedirection[0][0], 0, 'f', 6)
		.arg(this->GetProtocal().GetImageProtocal().spacedirection[0][1], 0, 'f', 6)
		.arg(this->GetProtocal().GetImageProtocal().spacedirection[0][2], 0, 'f', 6)
		.arg(this->GetProtocal().GetImageProtocal().spacedirection[1][0], 0, 'f', 6)
		.arg(this->GetProtocal().GetImageProtocal().spacedirection[1][1], 0, 'f', 6)
		.arg(this->GetProtocal().GetImageProtocal().spacedirection[1][2], 0, 'f', 6)
		.arg(this->GetProtocal().GetImageProtocal().spacedirection[2][0], 0, 'f', 6)
		.arg(this->GetProtocal().GetImageProtocal().spacedirection[2][1], 0, 'f', 6)
		.arg(this->GetProtocal().GetImageProtocal().spacedirection[2][2], 0, 'f', 6)
		);

	QTreeWidgetItem *itemSizes = new QTreeWidgetItem(itemImageInformation);
	itemSizes->setText(0, tr("sizes"));
	itemSizes->setText(1, QString("%1, %2, %3")
		.arg(this->GetProtocal().GetImageProtocal().size[0], 0,10)
		.arg(this->GetProtocal().GetImageProtocal().size[1], 0,10)
		.arg(this->GetProtocal().GetImageProtocal().size[2], 0,10)
		);

	QTreeWidgetItem *itemSpacing = new QTreeWidgetItem(itemImageInformation);
	itemSpacing->setText(0, tr("spacing"));
	itemSpacing->setText(1, QString("%1, %2, %3")
		.arg(this->GetProtocal().GetImageProtocal().spacing[0], 0, 'f', 6)
		.arg(this->GetProtocal().GetImageProtocal().spacing[1], 0, 'f', 6)
		.arg(this->GetProtocal().GetImageProtocal().spacing[2], 0, 'f', 6)
		);

	QTreeWidgetItem *itemOrig = new QTreeWidgetItem(itemImageInformation);
	itemOrig->setText(0, tr("origin"));
	itemOrig->setText(1, QString("%1, %2, %3")
		.arg(this->GetProtocal().GetImageProtocal().origin[0], 0, 'f', 6)
		.arg(this->GetProtocal().GetImageProtocal().origin[1], 0, 'f', 6)
		.arg(this->GetProtocal().GetImageProtocal().origin[2], 0, 'f', 6)
		);

	// diffusion
	QTreeWidgetItem *itemDiffusionInformation = new QTreeWidgetItem(treeWidget);
	itemDiffusionInformation->setText(0, tr("Diffusion Check"));
	if(this->GetProtocal().GetDiffusionProtocal().bCheck)
		itemDiffusionInformation->setText(1,tr("Yes"));
	else
		itemDiffusionInformation->setText(1,tr("No"));

	QTreeWidgetItem *itemMeasurementFrame = new QTreeWidgetItem(itemDiffusionInformation);
	itemMeasurementFrame->setText(0, tr("measurement frame"));
	itemMeasurementFrame->setText(1, QString("%1 %2 %3, %4 %5 %6, %7 %8 %9")
		.arg(this->GetProtocal().GetDiffusionProtocal().measurementFrame[0][0], 0, 'f', 6)
		.arg(this->GetProtocal().GetDiffusionProtocal().measurementFrame[0][1], 0, 'f', 6)
		.arg(this->GetProtocal().GetDiffusionProtocal().measurementFrame[0][2], 0, 'f', 6)
		.arg(this->GetProtocal().GetDiffusionProtocal().measurementFrame[1][0], 0, 'f', 6)
		.arg(this->GetProtocal().GetDiffusionProtocal().measurementFrame[1][1], 0, 'f', 6)
		.arg(this->GetProtocal().GetDiffusionProtocal().measurementFrame[1][2], 0, 'f', 6)
		.arg(this->GetProtocal().GetDiffusionProtocal().measurementFrame[2][0], 0, 'f', 6)
		.arg(this->GetProtocal().GetDiffusionProtocal().measurementFrame[2][1], 0, 'f', 6)
		.arg(this->GetProtocal().GetDiffusionProtocal().measurementFrame[2][2], 0, 'f', 6)
		);

	QTreeWidgetItem *itemBValue = new QTreeWidgetItem(itemDiffusionInformation);
	itemBValue->setText(0, tr("DWMRI_b-value"));
	itemBValue->setText(1,QString("%1").arg(this->GetProtocal().GetDiffusionProtocal().b, 0, 'f', 4));

	for(unsigned int i=0;i<this->GetProtocal().GetDiffusionProtocal().gradients.size();i++ )
	{
		QTreeWidgetItem *itemGradientDir = new QTreeWidgetItem(itemDiffusionInformation);
		itemGradientDir->setText(0, QString("DWMRI_gradient_%1").arg(i, 4, 10, QLatin1Char( '0' )));
		itemGradientDir->setText(1, QString("%1 %2 %3")
			.arg(this->GetProtocal().GetDiffusionProtocal().gradients[i][0], 0, 'f', 6)
			.arg(this->GetProtocal().GetDiffusionProtocal().gradients[i][1], 0, 'f', 6)
			.arg(this->GetProtocal().GetDiffusionProtocal().gradients[i][2], 0, 'f', 6)
			);
	}

	// QC
	QTreeWidgetItem *itemQCCheck = new QTreeWidgetItem(treeWidget);
	itemQCCheck->setText(0, tr("QC Check"));
	if(this->GetProtocal().GetIntensityMotionCheckProtocal().bCheck)
		itemQCCheck->setText(1,tr("Yes"));
	else
		itemQCCheck->setText(1,tr("No"));

	QTreeWidgetItem *itemQCOutputFilename = new QTreeWidgetItem(itemQCCheck);
	itemQCOutputFilename->setText(0, tr("QCOutputFileName"));
	itemQCOutputFilename->setText(1,QString::fromStdString(this->GetProtocal().GetIntensityMotionCheckProtocal().OutputFileName));

	QTreeWidgetItem *itemBadGradientTolerance = new QTreeWidgetItem(itemQCCheck);
	itemBadGradientTolerance->setText(0, tr("bad gradient percentage tolerance"));
	itemBadGradientTolerance->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().badGradientPercentageTolerance, 'f', 4));

	//slice wise QC
	QTreeWidgetItem *itemSliceWiseQCCheck = new QTreeWidgetItem(itemQCCheck);
	itemSliceWiseQCCheck->setText(0, tr("slice wise threshold"));
	if(this->GetProtocal().GetIntensityMotionCheckProtocal().bSliceCheck)
		itemSliceWiseQCCheck->setText(1,tr("Yes"));
	else
		itemSliceWiseQCCheck->setText(1,tr("No"));

	QTreeWidgetItem *itemBeginSkip = new QTreeWidgetItem(itemSliceWiseQCCheck);
	itemBeginSkip->setText(0, tr("head skip slice percentage"));
	itemBeginSkip->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().headSkipSlicePercentage,  'f', 4));

	QTreeWidgetItem *itemEndLeft = new QTreeWidgetItem(itemSliceWiseQCCheck);
	itemEndLeft->setText(0, tr("tail skip slice percentage"));
	itemEndLeft->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().tailSkipSlicePercentage,  'f', 4));

	QTreeWidgetItem *itemBaselineCorrelation = new QTreeWidgetItem(itemSliceWiseQCCheck);
	itemBaselineCorrelation->setText(0, tr("baseline correlation"));
	itemBaselineCorrelation->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().baselineCorrelationThreshold,  'f', 4));

	QTreeWidgetItem *itemBaselineCorrelationDev = new QTreeWidgetItem(itemSliceWiseQCCheck);
	itemBaselineCorrelationDev->setText(0, tr("baseline correlation deviation"));
	itemBaselineCorrelationDev->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().baselineCorrelationDeviationThreshold,  'f', 4));

	QTreeWidgetItem *itemSliceCorrelation = new QTreeWidgetItem(itemSliceWiseQCCheck);
	itemSliceCorrelation->setText(0, tr("slice correlation"));
	itemSliceCorrelation->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().sliceCorrelationThreshold, 'f', 4));

	QTreeWidgetItem *itemSliceCorrelationDev = new QTreeWidgetItem(itemSliceWiseQCCheck);
	itemSliceCorrelationDev->setText(0, tr("slice correlation deviation"));
	itemSliceCorrelationDev->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().sliceCorrelationDeviationThreshold, 'f', 4));
	
	QTreeWidgetItem *itemBadSliceToleration = new QTreeWidgetItem(itemSliceWiseQCCheck);
	itemBadSliceToleration->setText(0, tr("bad slice percentage tolerance"));
	itemBadSliceToleration->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().badSlicePercentageTolerance, 'f', 4));
	
	//interlace wise QC
	QTreeWidgetItem *itemInterlaceWiseQCCheck = new QTreeWidgetItem(itemQCCheck);
	itemInterlaceWiseQCCheck->setText(0, tr("interlace wise threshold"));
	if(this->GetProtocal().GetIntensityMotionCheckProtocal().bInterlaceCheck)
		itemInterlaceWiseQCCheck->setText(1,tr("Yes"));
	else
		itemInterlaceWiseQCCheck->setText(1,tr("No"));

	QTreeWidgetItem *itemInterlaceCorrBaseline = new QTreeWidgetItem(itemInterlaceWiseQCCheck);
	itemInterlaceCorrBaseline->setText(0, tr("interlace correlation baseline"));
	itemInterlaceCorrBaseline->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().interlaceCorrelationThresholdBaseline, 'f', 4));
	
	QTreeWidgetItem *itemInterlaceCorrDevBaseline = new QTreeWidgetItem(itemInterlaceWiseQCCheck);
	itemInterlaceCorrDevBaseline->setText(0, tr("interlace correlation deviation baseline"));
	itemInterlaceCorrDevBaseline->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().interlaceCorrelationDeviationBaseline, 'f', 4));

	QTreeWidgetItem *itemInterlaceCorrGrad = new QTreeWidgetItem(itemInterlaceWiseQCCheck);
	itemInterlaceCorrGrad->setText(0, tr("interlace correlation gradient"));
	itemInterlaceCorrGrad->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().interlaceCorrelationThresholdGradient, 'f', 4));

	QTreeWidgetItem *itemInterlaceCorrDevGrad = new QTreeWidgetItem(itemInterlaceWiseQCCheck);
	itemInterlaceCorrDevGrad->setText(0, tr("interlace correlation deviation gradient"));
	itemInterlaceCorrDevGrad->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().interlaceCorrelationDeviationGradient, 'f', 4));

	QTreeWidgetItem *itemInterlaceTranslation = new QTreeWidgetItem(itemInterlaceWiseQCCheck);
	itemInterlaceTranslation->setText(0, tr("interlace translation"));
	itemInterlaceTranslation->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().interlaceTranslationThreshold, 'f', 4));

	QTreeWidgetItem *itemInterlaceRotation = new QTreeWidgetItem(itemInterlaceWiseQCCheck);
	itemInterlaceRotation->setText(0, tr("interlace rotation"));
	itemInterlaceRotation->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().interlaceRotationThreshold, 'f', 4));
	
	//gradient wise QC
	QTreeWidgetItem *itemGradientWiseQCCheck = new QTreeWidgetItem(itemQCCheck);
	itemGradientWiseQCCheck->setText(0, tr("gradient wise threshold"));
	if(this->GetProtocal().GetIntensityMotionCheckProtocal().bGradientCheck)
		itemGradientWiseQCCheck->setText(1,tr("Yes"));
	else
		itemGradientWiseQCCheck->setText(1,tr("No"));

	QTreeWidgetItem *itemGradientTranslation = new QTreeWidgetItem(itemGradientWiseQCCheck);
	itemGradientTranslation->setText(0, tr("gradient translation"));
	itemGradientTranslation->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().gradientTranslationThreshold, 'f', 4));

	QTreeWidgetItem *itemGradientRotation = new QTreeWidgetItem(itemGradientWiseQCCheck);
	itemGradientRotation->setText(0, tr("gradient rotation"));
	itemGradientRotation->setText(1,QString::number(this->GetProtocal().GetIntensityMotionCheckProtocal().gradientRotationThreshold, 'f', 4));

	//EddyMotion
	QTreeWidgetItem *itemEddyMotionCorrection = new QTreeWidgetItem(treeWidget);
	itemEddyMotionCorrection->setText(0, tr("Eddy Motion Correction"));
	if(this->GetProtocal().GetEddyMotionCorrectionProtocal().bCorrect)
		itemEddyMotionCorrection->setText(1,tr("Yes"));
	else
		itemEddyMotionCorrection->setText(1,tr("No"));

	QTreeWidgetItem *itemEddyMotionCommand = new QTreeWidgetItem(itemEddyMotionCorrection);
	itemEddyMotionCommand->setText(0, tr("Eddy Motion Correction Command"));
	itemEddyMotionCommand->setText(1,QString::fromStdString(this->GetProtocal().GetEddyMotionCorrectionProtocal().EddyMotionCommand));

	QTreeWidgetItem *itemEddyMotionInputFilename = new QTreeWidgetItem(itemEddyMotionCorrection);
	itemEddyMotionInputFilename->setText(0, tr("EddyMotionInputFileName"));
	itemEddyMotionInputFilename->setText(1,QString::fromStdString(this->GetProtocal().GetEddyMotionCorrectionProtocal().InputFileName));

	QTreeWidgetItem *itemEddyMotionOutputFilename = new QTreeWidgetItem(itemEddyMotionCorrection);
	itemEddyMotionOutputFilename->setText(0, tr("EddyMotionOutputFileName"));
	itemEddyMotionOutputFilename->setText(1,QString::fromStdString(this->GetProtocal().GetEddyMotionCorrectionProtocal().OutputFileName));

	//DTI Computing
	QTreeWidgetItem *itemDTIComputing = new QTreeWidgetItem(treeWidget);
	itemDTIComputing->setText(0, tr("DTI Computing"));
	if(this->GetProtocal().GetDTIProtocal().bCompute)
		itemDTIComputing->setText(1,tr("Yes"));
	else
		itemDTIComputing->setText(1,tr("No"));

	QTreeWidgetItem *itemDtiestimCommand = new QTreeWidgetItem(itemDTIComputing);
	itemDtiestimCommand->setText(0, tr("dtiestim Command"));
	itemDtiestimCommand->setText(1,QString::fromStdString(this->GetProtocal().GetDTIProtocal().dtiestimCommand));

	QTreeWidgetItem *itemDtiprocessCommand = new QTreeWidgetItem(itemDTIComputing);
	itemDtiprocessCommand->setText(0, tr("dtiprocess Command"));
	itemDtiprocessCommand->setText(1,QString::fromStdString(this->GetProtocal().GetDTIProtocal().dtiprocessCommand));

	QTreeWidgetItem *itemMethod = new QTreeWidgetItem(itemDTIComputing);
	itemMethod->setText(0, tr("method"));
	switch(this->GetProtocal().GetDTIProtocal().method)
	{
	case Protocal::METHOD_WLS:
		itemMethod->setText(1, tr("wls"));
		break;
	case Protocal::METHOD_LLS:
		itemMethod->setText(1, tr("lls"));
		break;
	case Protocal::METHOD_ML:
		itemMethod->setText(1, tr("ml"));
		break;
	case Protocal::METHOD_NLS:
		itemMethod->setText(1, tr("nls"));
		break;
	default:
		itemMethod->setText(1, tr("lls"));
		break;
	}

	QTreeWidgetItem *itemBaselineThreshold = new QTreeWidgetItem(itemDTIComputing);
	itemBaselineThreshold->setText(0, tr("baseline threshold"));
	itemBaselineThreshold->setText(1,QString::number(this->GetProtocal().GetDTIProtocal().baselineThreshold));

	QTreeWidgetItem *itemMaskFile = new QTreeWidgetItem(itemDTIComputing);
	itemMaskFile->setText(0, tr("mask file"));
	itemMaskFile->setText(1,QString::fromStdString(this->GetProtocal().GetDTIProtocal().mask ));

	QTreeWidgetItem *itemTensorFile = new QTreeWidgetItem(itemDTIComputing);
	itemTensorFile->setText(0, tr("tensor file"));
	itemTensorFile->setText(1,QString::fromStdString(this->GetProtocal().GetDTIProtocal().tensor ));

	// padding
	QTreeWidgetItem *itemDtiPaddingCommand = new QTreeWidgetItem(itemDTIComputing);
	itemDtiPaddingCommand->setText(0, tr("CropDTI Command"));
	itemDtiPaddingCommand->setText(1,QString::fromStdString(this->GetProtocal().GetDTIProtocal().dtiPaddingCommand));

	QTreeWidgetItem *itemDtiPadding = new QTreeWidgetItem(itemDTIComputing);
	itemDtiPadding->setText(0, tr("tensor padding"));
	if(this->GetProtocal().GetDTIProtocal().bPadding)
		itemDtiPadding->setText(1,tr("Yes"));
	else
		itemDtiPadding->setText(1,tr("No"));

// 	QTreeWidgetItem *paddingParameters = new QTreeWidgetItem(itemDTIComputing);
// 	paddingParameters->setText(0, tr("tensor padding parameters"));
// 	paddingParameters->setText(1, QString("%1 %2 %3 %4 %5 %6")
// 		.arg(this->GetProtocal().GetDTIProtocal().paddingParameters[0], 0,10)
// 		.arg(this->GetProtocal().GetDTIProtocal().paddingParameters[1], 0,10)
// 		.arg(this->GetProtocal().GetDTIProtocal().paddingParameters[2], 0,10)
// 		.arg(this->GetProtocal().GetDTIProtocal().paddingParameters[3], 0,10)
// 		.arg(this->GetProtocal().GetDTIProtocal().paddingParameters[4], 0,10)
// 		.arg(this->GetProtocal().GetDTIProtocal().paddingParameters[5], 0,10)
// 		);

	// scalar images
	QTreeWidgetItem *itemScalarMeasures = new QTreeWidgetItem(itemDTIComputing);
	itemScalarMeasures->setText(0, tr("scalar measurements"));

	QTreeWidgetItem *itemScalarBaseline = new QTreeWidgetItem(itemScalarMeasures);
	itemScalarBaseline->setText(0, tr("baseline"));
	if(this->GetProtocal().GetDTIProtocal().bbaseline)
		itemScalarBaseline->setText(1,tr("Yes, ")+QString::fromStdString(this->GetProtocal().GetDTIProtocal().baseline ));
	else
		itemScalarBaseline->setText(1,tr("No, ")+QString::fromStdString(this->GetProtocal().GetDTIProtocal().baseline ));

	QTreeWidgetItem *itemScalarIDWI = new QTreeWidgetItem(itemScalarMeasures);
	itemScalarIDWI->setText(0, tr("idwi"));
	if(this->GetProtocal().GetDTIProtocal().bidwi)
		itemScalarIDWI->setText(1,tr("Yes, ")+QString::fromStdString(this->GetProtocal().GetDTIProtocal().idwi ));
	else
		itemScalarIDWI->setText(1,tr("No, ")+QString::fromStdString(this->GetProtocal().GetDTIProtocal().idwi ));

	QTreeWidgetItem *itemScalarFA = new QTreeWidgetItem(itemScalarMeasures);
	itemScalarFA->setText(0, tr("fa"));
	if(this->GetProtocal().GetDTIProtocal().bfa)
		itemScalarFA->setText(1,tr("Yes, ")+QString::fromStdString(this->GetProtocal().GetDTIProtocal().fa ));
	else
		itemScalarFA->setText(1,tr("No, ")+QString::fromStdString(this->GetProtocal().GetDTIProtocal().fa ));

	QTreeWidgetItem *itemScalarMD = new QTreeWidgetItem(itemScalarMeasures);
	itemScalarMD->setText(0, tr("md"));
	if(this->GetProtocal().GetDTIProtocal().bmd)
		itemScalarMD->setText(1,tr("Yes, ")+QString::fromStdString(this->GetProtocal().GetDTIProtocal().md ));
	else
		itemScalarMD->setText(1,tr("No, ")+QString::fromStdString(this->GetProtocal().GetDTIProtocal().md ));

	QTreeWidgetItem *itemScalarcolorFA = new QTreeWidgetItem(itemScalarMeasures);
	itemScalarcolorFA->setText(0, tr("colored fa"));
	if(this->GetProtocal().GetDTIProtocal().bcoloredfa)
		itemScalarcolorFA->setText(1,tr("Yes, ")+QString::fromStdString(this->GetProtocal().GetDTIProtocal().coloredfa ));
	else
		itemScalarcolorFA->setText(1,tr("No, ")+QString::fromStdString(this->GetProtocal().GetDTIProtocal().coloredfa ));

	QTreeWidgetItem *itemScalarFrobenius = new QTreeWidgetItem(itemScalarMeasures);
	itemScalarFrobenius->setText(0, tr("frobenius norm"));
	if(this->GetProtocal().GetDTIProtocal().bfrobeniusnorm)
		itemScalarFrobenius->setText(1,tr("Yes, ")+QString::fromStdString(this->GetProtocal().GetDTIProtocal().frobeniusnorm ));
	else
		itemScalarFrobenius->setText(1,tr("No, ")+QString::fromStdString(this->GetProtocal().GetDTIProtocal().frobeniusnorm ));


}


void IntensityMotionCheckPanel::ResultUpdate()
{
	bResultTreeEditable = false;
	//std::cout <<"ResultUpdate()"<<std::endl;
	QStringList labels;
	labels << tr("type") << tr("result")<<tr("processing");
	//treeWidget->header()->setResizeMode(QHeaderView::Stretch);
	treeWidget_Results->setHeaderLabels(labels);
	treeWidget_Results->clear();

	if(protocal.GetImageProtocal().bCheck)
	{
		// ImageInformationCheckResult
		QTreeWidgetItem *itemImageInformation = new QTreeWidgetItem(treeWidget_Results);
		itemImageInformation->setText(0, tr("ImageInformation"));

		QTreeWidgetItem *origin = new QTreeWidgetItem(itemImageInformation);
		origin->setText(0, tr("origin"));
		if(qcResult.GetImageInformationCheckResult().origin)
			origin->setText(1, tr("Pass"));
		else
			origin->setText(1, tr("Failed"));

		QTreeWidgetItem *size = new QTreeWidgetItem(itemImageInformation);
		size->setText(0, tr("size"));
		if(qcResult.GetImageInformationCheckResult().size)
			size->setText(1, tr("Pass"));
		else
			size->setText(1, tr("Failed"));

		QTreeWidgetItem *space = new QTreeWidgetItem(itemImageInformation);
		space->setText(0, tr("space"));
		if(qcResult.GetImageInformationCheckResult().space)
			space->setText(1, tr("Pass"));
		else
			space->setText(1, tr("Failed"));

		QTreeWidgetItem *spacedirection = new QTreeWidgetItem(itemImageInformation);
		spacedirection->setText(0, tr("spacedirection"));
		if(qcResult.GetImageInformationCheckResult().spacedirection)
			spacedirection->setText(1, tr("Pass"));
		else
			spacedirection->setText(1, tr("Failed"));

		QTreeWidgetItem *spacing = new QTreeWidgetItem(itemImageInformation);
		spacing->setText(0, tr("spacing"));
		if(qcResult.GetImageInformationCheckResult().spacing)
			spacing->setText(1, tr("Pass"));
		else
			spacing->setText(1, tr("Failed"));
	}

	if(protocal.GetDiffusionProtocal().bCheck)
	{
		// DiffusionInformationCheckResult
		QTreeWidgetItem *itemDiffusionInformation = new QTreeWidgetItem(treeWidget_Results);
		itemDiffusionInformation->setText(0, tr("DiffusionInformation"));

		QTreeWidgetItem *b = new QTreeWidgetItem(itemDiffusionInformation);
		b->setText(0, tr("b value"));
		if(qcResult.GetDiffusionInformationCheckResult().b)
			b->setText(1, tr("Pass"));
		else
			b->setText(1, tr("Failed"));

		QTreeWidgetItem *gradient = new QTreeWidgetItem(itemDiffusionInformation);
		gradient->setText(0, tr("gradient"));
		if(qcResult.GetDiffusionInformationCheckResult().gradient)
			gradient->setText(1, tr("Pass"));
		else
			gradient->setText(1, tr("Failed"));

		QTreeWidgetItem *measurementFrame = new QTreeWidgetItem(itemDiffusionInformation);
		measurementFrame->setText(0, tr("measurementFrame"));
		if(qcResult.GetDiffusionInformationCheckResult().measurementFrame)
			measurementFrame->setText(1, tr("Pass"));
		else
			measurementFrame->setText(1, tr("Failed"));
	}

	// QC MotionCheckResult
	if( protocal.GetIntensityMotionCheckProtocal().bCheck && (
		protocal.GetIntensityMotionCheckProtocal().bSliceCheck ||
		protocal.GetIntensityMotionCheckProtocal().bInterlaceCheck ||
		protocal.GetIntensityMotionCheckProtocal().bGradientCheck)     )
	{
		QTreeWidgetItem *itemIntensityMotionInformation = new QTreeWidgetItem(treeWidget_Results);
		itemIntensityMotionInformation->setText(0, tr("IntensityMotion"));


		//std::cout<< "in panel qcResult.GetGradientProcess().size(): "<< qcResult.GetGradientProcess().size() <<std::endl;
		for(unsigned int i=0;i<qcResult.GetIntensityMotionCheckResult().size();i++ )
		{

			// gradient 
			QTreeWidgetItem *gradient = new QTreeWidgetItem(itemIntensityMotionInformation);

			//gradient->setText(0, tr("gradient ")+QString::number(i));
			gradient->setText(0, QString("gradient_%1").arg(i, 4, 10, QLatin1Char( '0' )));

			//std::cout<<"1:ResultUpdate()"<<std::endl;
			if(qcResult.GetGradientProcess()[i] == QCResult::GRADIENT_EXCLUDE)
				gradient->setText(2, tr("EXCLUDE"));
			else if(qcResult.GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
				gradient->setText(2, tr(""));
			else
				;

			// slice wise
			//std::cout<<"2:ResultUpdate()"<<std::endl;
			QTreeWidgetItem *slice = new QTreeWidgetItem(gradient);
			slice->setText(0, tr("slice check"));
			if( !qcResult.GetIntensityMotionCheckResult()[i].bSliceCheckOK )
				slice->setText(1, tr("failed"));
			else
				slice->setText(1, tr("pass"));

			for(unsigned int j=0; j<qcResult.GetIntensityMotionCheckResult()[i].sliceCorrelation.size();j++)
			{
				QTreeWidgetItem *subslice = new QTreeWidgetItem(slice);
				subslice->setText(0,  tr(" slice Correlation ")+QString::number(j+1));
				subslice->setText(1, QString::number ( qcResult.GetIntensityMotionCheckResult()[i].sliceCorrelation[j], 'f', 3 ) );
			}
			//std::cout<<"3:ResultUpdate()"<<std::endl;
			// interlace wise
			QTreeWidgetItem *interlacewise = new QTreeWidgetItem(gradient);
			interlacewise->setText(0, tr("interlace check"));
			if( !qcResult.GetIntensityMotionCheckResult()[i].bInterlaceCheckOK )
				interlacewise->setText(1, tr("failed"));
			else
				interlacewise->setText(1, tr("pass"));

			QTreeWidgetItem *Correlation = new QTreeWidgetItem(interlacewise);
			Correlation->setText(0, tr("Correlation"));
			Correlation->setText(1,  QString::number ( qcResult.GetIntensityMotionCheckResult()[i].interlaceCorrelation, 'f', 3 ));

			QTreeWidgetItem *RotationX = new QTreeWidgetItem(interlacewise);
			RotationX->setText(0, tr("RotationX"));
			RotationX->setText(1,  QString::number ( qcResult.GetIntensityMotionCheckResult()[i].interlaceRotationX, 'f', 3 ));

			QTreeWidgetItem *RotationY = new QTreeWidgetItem(interlacewise);
			RotationY->setText(0, tr("RotationY"));
			RotationY->setText(1,  QString::number ( qcResult.GetIntensityMotionCheckResult()[i].interlaceRotationY, 'f', 3 ));

			QTreeWidgetItem *RotationZ = new QTreeWidgetItem(interlacewise);
			RotationZ->setText(0, tr("RotationZ"));
			RotationZ->setText(1,  QString::number ( qcResult.GetIntensityMotionCheckResult()[i].interlaceRotationZ, 'f', 3 ));

			QTreeWidgetItem *TranslationX = new QTreeWidgetItem(interlacewise);
			TranslationX->setText(0, tr("TranslationX"));
			TranslationX->setText(1,  QString::number ( qcResult.GetIntensityMotionCheckResult()[i].interlaceTranslationX, 'f', 3 ));

			QTreeWidgetItem *TranslationY = new QTreeWidgetItem(interlacewise);
			TranslationY->setText(0, tr("TranslationY"));
			TranslationY->setText(1,  QString::number ( qcResult.GetIntensityMotionCheckResult()[i].interlaceTranslationY, 'f', 3 ));

			QTreeWidgetItem *TranslationZ = new QTreeWidgetItem(interlacewise);
			TranslationZ->setText(0, tr("TranslationZ"));
			TranslationZ->setText(1,  QString::number ( qcResult.GetIntensityMotionCheckResult()[i].interlaceTranslationZ, 'f', 3 ));

			// gradient wise
			QTreeWidgetItem *gradientwise = new QTreeWidgetItem(gradient);
			gradientwise->setText(0, tr("gradient check"));
			if( !qcResult.GetIntensityMotionCheckResult()[i].bGradientCheckOK )
				gradientwise->setText(1, tr("failed"));
			else
				gradientwise->setText(1, tr("pass"));

			QTreeWidgetItem *gradientRotationX = new QTreeWidgetItem(gradientwise);
			gradientRotationX->setText(0, tr("RotationX"));
			gradientRotationX->setText(1,  QString::number ( qcResult.GetIntensityMotionCheckResult()[i].gradientRotationX, 'f', 3 ));

			QTreeWidgetItem *gradientRotationY = new QTreeWidgetItem(gradientwise);
			gradientRotationY->setText(0, tr("RotationY"));
			gradientRotationY->setText(1,  QString::number ( qcResult.GetIntensityMotionCheckResult()[i].gradientRotationY, 'f', 3 ));

			QTreeWidgetItem *gradientRotationZ = new QTreeWidgetItem(gradientwise);
			gradientRotationZ->setText(0, tr("RotationZ"));
			gradientRotationZ->setText(1,  QString::number ( qcResult.GetIntensityMotionCheckResult()[i].gradientRotationZ, 'f', 3 ));

			QTreeWidgetItem *gradientTranslationX = new QTreeWidgetItem(gradientwise);
			gradientTranslationX->setText(0, tr("TranslationX"));
			gradientTranslationX->setText(1,  QString::number ( qcResult.GetIntensityMotionCheckResult()[i].gradientTranslationX, 'f', 3 ));

			QTreeWidgetItem *gradientTranslationY = new QTreeWidgetItem(gradientwise);
			gradientTranslationY->setText(0, tr("TranslationY"));
			gradientTranslationY->setText(1,  QString::number ( qcResult.GetIntensityMotionCheckResult()[i].gradientTranslationY, 'f', 3 ));

			QTreeWidgetItem *gradientTranslationZ = new QTreeWidgetItem(gradientwise);
			gradientTranslationZ->setText(0, tr("TranslationZ"));
			gradientTranslationZ->setText(1,  QString::number ( qcResult.GetIntensityMotionCheckResult()[i].gradientTranslationZ, 'f', 3 ));
		}
	}
	//std::cout<<"4:ResultUpdate()"<<std::endl;

	bResultTreeEditable = true;
	pushButton_SaveDWI->setEnabled( 1 );
	pushButton_SaveDWIAs->setEnabled( 1 );

	return ;
}



void IntensityMotionCheckPanel::GenerateCheckOutputImage( std::string filename)
{
	if(!bDwiLoaded  ) LoadDwiImage();
	if(!bDwiLoaded  )
	{
		std::cout<<"DWI load error, no Gradient Direction Loaded"<<std::endl;
		bGetGridentDirections=false;
		return ;
	}

	unsigned int gradientLeft=0;
	for(unsigned int i=0;i< qcResult.GetGradientProcess().size();i++)
	{
		if(qcResult.GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
			gradientLeft++;
	}

	std::cout << "gradientLeft: " << gradientLeft<<std::endl;
	//std::cout << "1.0-(float)((float)gradientLeft/(float)qcResult.GetGradientProcess().size()): " << 1.0-(float)((float)gradientLeft/(float)qcResult.GetGradientProcess().size())<<std::endl;
	//std::cout << "this->protocal.GetIntensityMotionCheckProtocal().badGradientPercentageTolerance: " << this->protocal.GetIntensityMotionCheckProtocal().badGradientPercentageTolerance<<std::endl;

	if(bProtocol)
	{
		if( 1.0-(float)((float)gradientLeft/(float)qcResult.GetGradientProcess().size()) >= this->protocal.GetIntensityMotionCheckProtocal().badGradientPercentageTolerance)
		{
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(this, tr("Attention"),
				tr("Bad gradients number is greater than that in protocol, save anyway?"),
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			if ( reply == QMessageBox::No || reply == QMessageBox::Cancel)
				return;		
		}
	}

	if( gradientLeft == qcResult.GetGradientProcess().size())
	{
		itk::NrrdImageIO::Pointer  NrrdImageIO = itk::NrrdImageIO::New();
		try
		{
			DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
			DwiWriter->SetImageIO(NrrdImageIO);
			DwiWriter->SetFileName( filename );
			DwiWriter->SetInput(DwiReader->GetOutput());
			DwiWriter->UseCompressionOn();
			DwiWriter->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;
			return ;
		}
		return;
	}

	DwiImageType::Pointer newDwiImage = DwiImageType::New(); 
	newDwiImage->CopyInformation(DwiImage);
	newDwiImage->SetRegions(DwiImage->GetLargestPossibleRegion());
	newDwiImage->SetVectorLength( gradientLeft) ; 
	//	newDwiImage->SetMetaDataDictionary(imgMetaDictionary);
	newDwiImage->Allocate();

	typedef itk::ImageRegionConstIteratorWithIndex< DwiImageType > ConstIteratorType;
	ConstIteratorType oit( DwiImage, DwiImage->GetLargestPossibleRegion() );
	typedef itk::ImageRegionIteratorWithIndex< DwiImageType > IteratorType;
	IteratorType nit( newDwiImage, newDwiImage->GetLargestPossibleRegion() );

	oit.GoToBegin();
	nit.GoToBegin();

	DwiImageType::PixelType value ;
	value.SetSize( gradientLeft ) ;

	while (!oit.IsAtEnd())
	{
		int element = 0;
		for( unsigned int i = 0 ; i < qcResult.GetGradientProcess().size(); i++ )
		{
			if(qcResult.GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
			{
				value.SetElement( element , oit.Get()[i] ) ;
				element++;
			}
		}
		nit.Set(value);	
		++oit;
		++nit;
	}

	itk::NrrdImageIO::Pointer  NrrdImageIO = itk::NrrdImageIO::New();
	try
	{
		DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
		DwiWriter->SetImageIO(NrrdImageIO);
		DwiWriter->SetFileName( filename );
		DwiWriter->SetInput(newDwiImage);
		DwiWriter->UseCompressionOn();
		DwiWriter->Update();
	}
	catch(itk::ExceptionObject & e)
	{
		std::cout<< e.GetDescription()<<std::endl;
		return ;
	}

	//newDwiImage->Delete();

	itk::MetaDataDictionary imgMetaDictionary = DwiImage->GetMetaDataDictionary();    //
	std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
	std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
	std::string metaString;

	char *aryOut = new char[filename.length()+1];
	strcpy ( aryOut, filename.c_str() );

	std::ofstream header;
	header.open(aryOut, std::ios_base::app);

	//  measurement frame 
	if(imgMetaDictionary.HasKey("NRRD_measurement frame"))
	{
		// measurement frame
		vnl_matrix<double> mf(3,3);
		// imaging frame
		vnl_matrix<double> imgf(3,3);
		std::vector<std::vector<double> > nrrdmf;
		itk::ExposeMetaData<std::vector<std::vector<double> > >(imgMetaDictionary,"NRRD_measurement frame",nrrdmf);

		imgf = DwiImage->GetDirection().GetVnlMatrix();

		//Image frame
		//std::cout << "Image frame: " << std::endl;
		//std::cout << imgf << std::endl;

		for(unsigned int i = 0; i < 3; ++i)
		{
			for(unsigned int j = 0; j < 3; ++j)
			{
				mf(i,j) = nrrdmf[j][i];
				nrrdmf[j][i] = imgf(i,j);
			}
		}

		// Meausurement frame
		header	<< "measurement frame: (" 
			<< mf(0,0) << "," 
			<< mf(1,0) << "," 
			<< mf(2,0) << ") ("
			<< mf(0,1) << "," 
			<< mf(1,1) << "," 
			<< mf(2,1) << ") ("
			<< mf(0,2) << "," 
			<< mf(1,2) << "," 
			<< mf(2,2) << ")"
			<< std::endl;
	}

	for ( itKey=imgMetaKeys.begin(); itKey != imgMetaKeys.end(); itKey ++){
		int pos;

		itk::ExposeMetaData(imgMetaDictionary, *itKey, metaString);
		pos = itKey->find("modality");
		if (pos == -1){
			continue;
		}

		std::cout  << metaString << std::endl;    
		header << "modality:=" << metaString << std::endl;
	}

	GetGridentDirections();

	for ( itKey=imgMetaKeys.begin(); itKey != imgMetaKeys.end(); itKey ++){
		int pos;

		itk::ExposeMetaData(imgMetaDictionary, *itKey, metaString);
		pos = itKey->find("DWMRI_b-value");
		if (pos == -1){
			continue;
		}

		std::cout  << metaString << std::endl;    
		header << "DWMRI_b-value:=" << metaString << std::endl;
	}


	int temp=0;
	for(unsigned int i=0;i< GradientDirectionContainer->size();i++ )
	{
		if(qcResult.GetGradientProcess()[i] == QCResult::GRADIENT_INCLUDE)
		{
			header	<< "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << temp << ":=" 
				<< GradientDirectionContainer->ElementAt(i)[0] << "   " 
				<< GradientDirectionContainer->ElementAt(i)[1] << "   " 
				<< GradientDirectionContainer->ElementAt(i)[2] << std::endl;
			++temp;
		}
	}

	header.flush();
	header.close();

	std::cout<<" QC Image saved "<<std::endl;

}

bool IntensityMotionCheckPanel::GetGridentDirections()
{
	if(!bDwiLoaded ) LoadDwiImage();
	if(!bDwiLoaded )
	{
		std::cout<<"DWI load error, no Gradient Direction Loaded"<<std::endl;
		bGetGridentDirections=false;
		return false;
	}		

	itk::MetaDataDictionary imgMetaDictionary = DwiImage->GetMetaDataDictionary();    //
	std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
	std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
	std::string metaString;

	//int numberOfImages=0;
	TensorReconstructionImageFilterType::GradientDirectionType vect3d;

	GradientDirectionContainer->clear();

	for ( ; itKey != imgMetaKeys.end(); itKey ++)
	{
		//double x,y,z;
		itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
		if (itKey->find("DWMRI_gradient") != std::string::npos)
		{ 
			std::istringstream iss(metaString);
			iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
			//sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
			//vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
			GradientDirectionContainer->push_back(vect3d);
		}
		else if (itKey->find("DWMRI_b-value") != std::string::npos)
		{
			readb0 = true;
			b0 = atof(metaString.c_str());
			//std::cout<<"b Value: "<<b0<<std::endl;
		}
	}

	if(!readb0)
	{
		std::cout<<"BValue not specified in header file" <<std::endl;
		return false;
	}
	if(GradientDirectionContainer->size()<=6) 
	{
		std::cout<<"Gradient Images Less than 7" <<std::endl;
		bGetGridentDirections=false;
		return false;
	}

	std::cout<<"b Value: "<<b0<<std::endl;
	std::cout<<"DWI image gradient count: "<<DwiImage->GetVectorLength()<<std::endl;

	for( unsigned int i = 0; i<DwiImage->GetVectorLength();i++ )//GradientDirectionContainer->Size()
	{
		std::cout<<"Gradient Direction "<<i<<": \t[";
		std::cout<<GradientDirectionContainer->at(i)[0]<<",\t";
		std::cout<<GradientDirectionContainer->at(i)[1]<<",\t";
		std::cout<<GradientDirectionContainer->at(i)[2]<<" ]"<<std::endl;
	}

	bGetGridentDirections=true;
	return true;
}

void IntensityMotionCheckPanel::on_pushButton_SaveDWIAs_clicked( )
{
	QString DWIFile = QFileDialog::getSaveFileName( this, tr("Save Protocal As"),QString::fromStdString(DwiFileName),  tr("nrrd Files (*.nhdr)") );
	if(DWIFile.length()>0)
	{
		std::cout<<"Save DWI into file: "<<DWIFile.toStdString()<<std::endl;
		this->GenerateCheckOutputImage( DWIFile.toStdString());
	}
	else
	{
		std::cout<<"DWI file name NOT set"<<std::endl;
	}
}


void IntensityMotionCheckPanel::on_pushButton_SaveDWI_clicked( )
{
	if(!bProtocol)
	{
		std::cout<<"No protocol information"<<std::endl;
		QMessageBox::information( this, tr("Warning"), tr("No protocol information!"));

		return;
	}
	if(this->GetProtocal().GetIntensityMotionCheckProtocal().OutputFileName.length()>0)
	{
		std::cout<<"Save DWI into file: "<< this->GetProtocal().GetIntensityMotionCheckProtocal().OutputFileName <<std::endl;
		
		std::string OutputFileName;
		OutputFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );
		OutputFileName.append(this->GetProtocal().GetIntensityMotionCheckProtocal().OutputFileName);

		this->GenerateCheckOutputImage( OutputFileName);
	}
	else
	{
		std::cout<<"DWI file name NOT set in protocol"<<std::endl;
	}
}

void IntensityMotionCheckPanel::on_pushButton_DefaultQCResult_clicked( )
{
	DefaultProcess( );
	//pushButton_SaveQCReport->setEnabled(1);
	//pushButton_SaveQCReportAs->setEnabled(1);
}

void IntensityMotionCheckPanel::DefaultProcess( )
{
	this->qcResult.Clear();

	GradientIntensityMotionCheckResult  IntensityMotionCheckResult;

	IntensityMotionCheckResult.bGradientCheckOK = true;
	IntensityMotionCheckResult.gradientRotationX = 0.0;
	IntensityMotionCheckResult.gradientRotationY = 0.0;
	IntensityMotionCheckResult.gradientRotationZ = 0.0;
	IntensityMotionCheckResult.gradientTranslationX = 0.0;
	IntensityMotionCheckResult.gradientTranslationY = 0.0;
	IntensityMotionCheckResult.gradientTranslationZ = 0.0;

	IntensityMotionCheckResult.bInterlaceCheckOK = true;
	IntensityMotionCheckResult.interlaceCorrelation = 0.0;
	IntensityMotionCheckResult.interlaceRotationX = 0.0;
	IntensityMotionCheckResult.interlaceRotationY = 0.0;
	IntensityMotionCheckResult.interlaceRotationZ = 0.0;
	IntensityMotionCheckResult.interlaceTranslationX = 0.0;
	IntensityMotionCheckResult.interlaceTranslationY = 0.0;
	IntensityMotionCheckResult.interlaceTranslationZ = 0.0;

	IntensityMotionCheckResult.bSliceCheckOK = true;
	for(unsigned int i=0;i<this->DwiImage->GetLargestPossibleRegion().GetSize()[2];i++)
		IntensityMotionCheckResult.sliceCorrelation.push_back(0.0);

	for( unsigned int i=0;i< this->DwiImage->GetVectorLength(); i++)
	{
		this->qcResult.GetGradientProcess().push_back(QCResult::GRADIENT_INCLUDE);
		this->qcResult.GetIntensityMotionCheckResult().push_back(IntensityMotionCheckResult);
	}

	protocal.clear();
	protocal.GetImageProtocal().bCheck = true;
	protocal.GetDiffusionProtocal().bCheck = true;
	protocal.GetIntensityMotionCheckProtocal().bCheck = true;
	protocal.GetIntensityMotionCheckProtocal().bSliceCheck = true;
	protocal.GetIntensityMotionCheckProtocal().bInterlaceCheck = true;
	protocal.GetIntensityMotionCheckProtocal().bGradientCheck = true;

	ResultUpdate();

	pushButton_SaveDWIAs->setEnabled(1);
	pushButton_SaveDWI->setEnabled(1);
}

void IntensityMotionCheckPanel::on_pushButton_OpenQCReport_clicked( )
{
	OpenQCReport( );
	//pushButton_SaveQCReport->setEnabled(1);
	//pushButton_SaveQCReportAs->setEnabled(1);
}

void IntensityMotionCheckPanel::OpenQCReport( )
{
	std::string ReportFileName;
	ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );

	if( this->protocal.GetReportFileName().length() != 0)
		ReportFileName.append( this->protocal.GetReportFileName() );// "IntensityMotionCheckReports.txt"		
	else
		ReportFileName.append( "_QC_CheckReports.txt");

	QString reportFile = QFileDialog::getOpenFileName ( this, tr("Open QC Result"), QString::fromStdString(ReportFileName), tr("txt Files (*.txt)") );
	if( reportFile.length() <= 0 )
		return;

	this->qcResult.Clear();

	GradientIntensityMotionCheckResult  IntensityMotionCheckResult;

	IntensityMotionCheckResult.bGradientCheckOK = true;
	IntensityMotionCheckResult.gradientRotationX = 0.0;
	IntensityMotionCheckResult.gradientRotationY = 0.0;
	IntensityMotionCheckResult.gradientRotationZ = 0.0;
	IntensityMotionCheckResult.gradientTranslationX = 0.0;
	IntensityMotionCheckResult.gradientTranslationY = 0.0;
	IntensityMotionCheckResult.gradientTranslationZ = 0.0;

	IntensityMotionCheckResult.bInterlaceCheckOK = true;
	IntensityMotionCheckResult.interlaceCorrelation = 0.0;
	IntensityMotionCheckResult.interlaceRotationX = 0.0;
	IntensityMotionCheckResult.interlaceRotationY = 0.0;
	IntensityMotionCheckResult.interlaceRotationZ = 0.0;
	IntensityMotionCheckResult.interlaceTranslationX = 0.0;
	IntensityMotionCheckResult.interlaceTranslationY = 0.0;
	IntensityMotionCheckResult.interlaceTranslationZ = 0.0;

	IntensityMotionCheckResult.bSliceCheckOK = true;
	for(unsigned int i=0;i<this->DwiImage->GetLargestPossibleRegion().GetSize()[2];i++)
		IntensityMotionCheckResult.sliceCorrelation.push_back(0.0);

	for( unsigned int i=0;i< this->DwiImage->GetVectorLength(); i++)
	{
		this->qcResult.GetGradientProcess().push_back(QCResult::GRADIENT_INCLUDE);
		this->qcResult.GetIntensityMotionCheckResult().push_back(IntensityMotionCheckResult);
	}

	protocal.clear();
	protocal.GetImageProtocal().bCheck = true;
	protocal.GetDiffusionProtocal().bCheck = true;
	protocal.GetIntensityMotionCheckProtocal().bCheck = true;
	protocal.GetIntensityMotionCheckProtocal().bSliceCheck = true;
	protocal.GetIntensityMotionCheckProtocal().bInterlaceCheck = true;
	protocal.GetIntensityMotionCheckProtocal().bGradientCheck = true;

	////////////////////////////////////////////////////////////////////////////
	std::vector< int > GradientProcess;
	bool bAllIncluded = false;

	if(reportFile.length()>0)
	{
		std::ifstream infile; 
		infile.open( reportFile.toStdString().c_str());

		std::string strTemp;
		bool excludeStart=false, includeStart=false;
		std::string strExclude ("Gradient(s) excluded:");
		std::string strInclude ("Gradients included:");
		std::string strGradient ("Gradient");
		std::string strAllGradient ("No gradient data excluded");

		excludeStart=false;
		includeStart=false;

		while(!infile.eof())
		{
			std::getline ( infile, strTemp );
			std::cout<<"std::getline:"<<strTemp<<std::endl;

			size_t found;

			found=strTemp.find(strAllGradient);
			if (found!=std::string::npos)
			{
				//excludeStart=true;
				//includeStart=false;
				std::cout<<"No gradient data excluded."<<std::endl;
				bAllIncluded = true;
				break;
			}

			found=strTemp.find(strGradient);
			if (found == std::string::npos) 
			{
				excludeStart=false;
				includeStart=false;
				continue;
			}

			found=strTemp.find(strExclude);
			if (found!=std::string::npos)
			{
				excludeStart=true;
				includeStart=false;
				std::cout<<"std::getline:"<<strTemp<<std::endl;
				continue;
			}

			found=strTemp.find(strInclude);
			if (found!=std::string::npos)
			{
				excludeStart=false;
				includeStart=true;
				std::cout<<"std::getline:"<<strTemp<<std::endl;
				continue;
			}

			if(excludeStart)
			{
				strTemp=strTemp.substr(strTemp.find(strGradient)+strGradient.length()+1,strTemp.length()-strGradient.length()-1 );
				GradientProcess.push_back( -atoi(strTemp.c_str()));
				std::cout<<"strTemp.substr:"<<strTemp<<" = "<<-atoi(strTemp.c_str())<<std::endl;
			}

			if(includeStart)
			{
				strTemp=strTemp.substr(strTemp.find(strGradient)+strGradient.length()+1,strTemp.length()-strGradient.length()-1 );
				GradientProcess.push_back( atoi(strTemp.c_str()));
				std::cout<<"strTemp.substr:"<<strTemp<<" = "<<atoi(strTemp.c_str())<<std::endl;
			}
		}
	}

	if(bAllIncluded)
	{
		GradientProcess.clear();
		for( unsigned int i=0;i< this->DwiImage->GetVectorLength(); i++)
		{
			GradientProcess.push_back(i);
		}
	}

	std::cout<<"GradientProcess.size():"<<GradientProcess.size() <<std::endl;
	std::cout<<"this->qcResult.GetGradientProcess().size(): "<< this->qcResult.GetGradientProcess().size()<<std::endl;
	if( GradientProcess.size() != this->qcResult.GetGradientProcess().size())
	{

		QMessageBox::warning(this, tr("Warning"),tr("Gradient Numbers mismatch between DWI and QC Report file."));
		std::cout<<"Gradient Numbers mismatch between DWI and QC Report file."<<std::endl;
		return;
	}

	for(unsigned int i=0;i< GradientProcess.size();i++)
	{
		if(GradientProcess[i]< 0 ) 
			this->qcResult.GetGradientProcess()[-GradientProcess[i]]=QCResult::GRADIENT_EXCLUDE;
		else
			this->qcResult.GetGradientProcess()[GradientProcess[i]]=QCResult::GRADIENT_INCLUDE;
	}

	ResultUpdate();

	pushButton_SaveDWIAs->setEnabled(1);
	pushButton_SaveDWI->setEnabled(1);
}
