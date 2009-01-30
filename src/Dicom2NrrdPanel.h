
#include <QtGui/QDockWidget>
#include <QtGui/QMainWindow>
#include "ui_Dicom2NrrdPanel.h"

#include "ThreadDicomToNrrd.h"

class Dicom2NrrdPanel : public QDockWidget, private Ui_Dicom2NrrdPanel
{
    Q_OBJECT

public:
    Dicom2NrrdPanel(QMainWindow *parent=0);
    ~Dicom2NrrdPanel();

	CThreadDicomToNrrd ThreadDicomToNrrd;
protected:
 
private slots:
	void on_dicomDirectoryBrowseButton_clicked();
	void on_nrrdFileBrowseButton_clicked();

	void on_toolButton_DicomToNrrdConverterCommand_clicked();
//	void on_buttonBox_accepted();
//	void on_buttonBox_rejected();
	void on_pushButton_Convert_clicked();
	void UpdateProgressBar(int pos );

private:

//	bool ShowDicomSeries(QString path);


};
