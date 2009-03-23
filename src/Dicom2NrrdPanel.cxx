#include "Dicom2NrrdPanel.h"
#include <QtGui>
#include <iostream>

#include "itkGDCMSeriesFileNames.h"

Dicom2NrrdPanel::Dicom2NrrdPanel(QMainWindow *parent):QDockWidget(parent)
{
  setupUi(this);
  verticalLayout->setContentsMargins(0,0,0,0);
  progressBar->setValue(0);

  connect( &ThreadDicomToNrrd, SIGNAL( kkk( int )),this, SLOT( UpdateProgressBar( int )));

}

Dicom2NrrdPanel::~Dicom2NrrdPanel()
{

}

void Dicom2NrrdPanel::UpdateProgressBar(int pos )
{
	 progressBar->setValue(pos);
}

void Dicom2NrrdPanel::on_dicomDirectoryBrowseButton_clicked( )
{
	//QString fileName = QFileDialog::getOpenFileName(this, tr("Select the Dicom Directory"),tr("c:\\"), QFileDialog::ShowDirsOnly);
	 QString dicomDir= QFileDialog::getExistingDirectory(this, tr("Select the Dicom Directory"),dicomDirectoryEdit->text(), QFileDialog::ShowDirsOnly);
	
	 if (!dicomDir.isEmpty()) 
	 {
		 std::cout<<dicomDir.toStdString()<<std::endl;
		 //dicomDirectory=dicomDir;
		 this->dicomDirectoryEdit->setText(dicomDir);
	 }

	 //ShowDicomSeries(dicomDir);

	 return;
	
	//fileName = QFileDialog::getOpenFileName(this,
    //   tr("Open Image"), "/home/jana", tr("Image Files (*.png *.jpg *.bmp)"));

}

void Dicom2NrrdPanel::on_toolButton_DicomToNrrdConverterCommand_clicked()
{
	QString DicomToNrrdConverterCommand= QFileDialog::getOpenFileName(this, tr("Set the DicomToNrrdConverter command"),lineEdit_DicomToNrrdConverterCommand->text());
	
	 if (!DicomToNrrdConverterCommand.isEmpty()) 
	 {
		 std::cout<<DicomToNrrdConverterCommand.toStdString()<<std::endl;
		 //dicomDirectory=dicomDir;
		 this->lineEdit_DicomToNrrdConverterCommand->setText(DicomToNrrdConverterCommand);
	 }

	 //ShowDicomSeries(dicomDir);

	 return;
}

void Dicom2NrrdPanel::on_nrrdFileBrowseButton_clicked( )
{

	 QString nrrFile = QFileDialog::getSaveFileName ( this, tr("nrrd File Save As"), nrrdFileName->text() , tr("Image Files (*.nhdr)") );
	// QString nrrFile = QFileDialog::getSaveFileName ( this, tr("nrrd File Save As"), nrrdFileName->text() , tr("Image Files (*.nhdr *.nrrd)") );
	//QString getSaveFileName ( QWidget * parent = 0, const QString & caption = QString(), const QString & dir = QString(), const QString & filter = QString(), QString * selectedFilter = 0, Options options = 0 )	 
	 if (!nrrFile.isEmpty()) 
	 {
		 std::cout<<nrrFile.toStdString()<<std::endl;
		 //nrrdPath=nrrFile;
	     this->nrrdFileName->setText(nrrFile);
	 }
	 return;	
}

void Dicom2NrrdPanel::on_pushButton_Convert_clicked()
{

	if (lineEdit_DicomToNrrdConverterCommand->text().isEmpty()) 
	 {
		 std::cout<<QString(tr("DicomToNrrdConverter Command Empty!")).toStdString()<<std::endl;
		 return;
	 }

	if (dicomDirectoryEdit->text().isEmpty()) 
	 {
		 std::cout<<QString(tr("Dicom Directory Empty!")).toStdString()<<std::endl;
		 return;
	 }

	 if(nrrdFileName->text().isEmpty())
	 {
		 std::cout<<QString(tr("nrrd File Name Empty!")).toStdString()<<std::endl;
		 return;
	 }
	 
	std::cout<<QString(tr("Dicom to Nrrd converting ...")).toStdString()<<std::endl;

	//ThreadDicomToNrrd.D.DicomToNrrdCmd= QString(tr("/tools/devel/linux/Slicer3_linux/Slicer3-build/lib/Slicer3/Plugins/DicomToNrrdConverter "));
	ThreadDicomToNrrd.DicomToNrrdCmd= lineEdit_DicomToNrrdConverterCommand->text();
	ThreadDicomToNrrd.DicomDir=dicomDirectoryEdit->text();
	ThreadDicomToNrrd.NrrdFileName=nrrdFileName->text();

	QString str;
	str.append(lineEdit_DicomToNrrdConverterCommand->text());
	str.append( QString(tr(" ")));
	str.append(dicomDirectoryEdit->text());
	str.append( QString(tr(" ")));
	str.append(nrrdFileName->text());	
	std::cout<<str.toStdString()<<std::endl;

	ThreadDicomToNrrd.start(QThread::LowPriority);
}

/*
void Dicom2NrrdPanel::on_buttonBox_accepted()
{

	if (dicomDirectoryEdit->text().isEmpty()) 
	 {
		 std::cout<<QString(tr("Dicom Directory Empty!")).toStdString()<<std::endl;
		 return;
	 }
	 if(nrrdFileName->text().isEmpty())
	 {
		 std::cout<<QString(tr("nrrd File Name Empty!")).toStdString()<<std::endl;
		 return;
	 }
	 
	std::cout<<QString(tr("Dicom to Nrrd converting ...")).toStdString()<<std::endl;

	QString str,str1,str2;

	str +=	QString(tr("/tools/devel/linux/Slicer3_linux/Slicer3-build/lib/Slicer3/Plugins/DicomToNrrdConverter ")) ;
	str +=	dicomDirectoryEdit->text();
	
	str1 =	nrrdFileName->text().section('/', -1);
	str2 =	nrrdFileName->text().left(nrrdFileName->text().length()-str1.length());
	str+=QString(tr("  "));
	str+=str2;
	str+=QString(tr("  "));
	str+=str1;


	ThreadDicomToNrrd.DicomToNrrdCmd= QString(tr("/tools/devel/linux/Slicer3_linux/Slicer3-build/lib/Slicer3/Plugins/DicomToNrrdConverter "));
	ThreadDicomToNrrd.DicomDir=dicomDirectoryEdit->text();
	ThreadDicomToNrrd.NrrdDir=str2;
	ThreadDicomToNrrd.NrrdFileName=str1;

	ThreadDicomToNrrd.start(QThread::LowPriority);

	 //system(const_cast<char *>(str.toStdString().c_str())); 
}


void Dicom2NrrdPanel::on_buttonBox_rejected()
{
	//this->hide();

	 ThreadDicomToNrrd.start();

	//if (ThreadIntensityMotionCheck.isRunning()) {
    //   ThreadIntensityMotionCheck.stop();
    //} else {
    //    ThreadIntensityMotionCheck.start();
   // }

	return;
}

*/
#include "itkPadImageFilter.h"


/*
bool Dicom2NrrdPanel::ShowDicomSeries(QString path)
{
		// In this example, we also call the \code{SetUseSeriesDetails(true)} function
		// that tells the GDCMSereiesFileNames object to use additional DICOM 
		// information to distinguish unique volumes within the directory.  This is
		// useful, for example, if a DICOM device assigns the same SeriesID to 
		// a scout scan and its 3D volume; by using additional DICOM information
		// the scout scan will not be included as part of the 3D volume.  Note that
		// \code{SetUseSeriesDetails(true)} must be called prior to calling
		// \code{SetDirectory()}. By default \code{SetUseSeriesDetails(true)} will use
		// the following DICOM tags to sub-refine a set of files into multiple series:
		// * 0020 0011 Series Number
		// * 0018 0024 Sequence Name
		// * 0018 0050 Slice Thickness
		// * 0028 0010 Rows
		// * 0028 0011 Columns
		// If this is not enough for your specific case you can always add some more
		// restrictions using the \code{AddSeriesRestriction()} method. In this example we will use
		// the DICOM Tag: 0008 0021 DA 1 Series Date, to sub-refine each series. The format
		// for passing the argument is a string containing first the group then the element
		// of the DICOM tag, separed by a pipe (|) sign.


		if(dicomDirectoryEdit->text().isEmpty()) return 0;

		dicomSeriesTree->clear();
		typedef itk::GDCMSeriesFileNames NamesGeneratorType;
		NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();

		nameGenerator->SetUseSeriesDetails( true );
		nameGenerator->AddSeriesRestriction("0008|0021" );//0008 0021 DA 1 Series Date,
		nameGenerator->SetDirectory(dicomDirectoryEdit->text().toStdString() );

		typedef std::vector< std::string >    SeriesIdContainer;
		try
		{
			const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
			SeriesIdContainer::const_iterator seriesItr = seriesUID.begin();
			SeriesIdContainer::const_iterator seriesEnd = seriesUID.end();

			unsigned short seriesNo=1;

			if(seriesItr == seriesEnd)  
			{
				QStringList strList(tr("No Series in the Directory"));
				QTreeWidgetItem *item= new QTreeWidgetItem( dicomSeriesTree,strList , QTreeWidgetItem::Type );
				dicomSeriesTree-> addTopLevelItem ( item );
			}
			else
			{
				while( seriesItr != seriesEnd )
				{
					//QStringList strList(tr(seriesItr->c_str()));
					
					QStringList strList(tr("Series ")+QString::number(seriesNo));
					strList+=QString(tr(" [  ,  ,  ]"));

					QTreeWidgetItem *item= new QTreeWidgetItem( dicomSeriesTree,strList , QTreeWidgetItem::Type );
					dicomSeriesTree-> addTopLevelItem ( item );

					seriesItr++;
					seriesNo++;
				};
			}

		}
		catch (itk::ExceptionObject &err)
		{
			std::cout<<err.GetDescription()<<std::endl;
			return 0;
		}
		
	return 1;
}
*/
