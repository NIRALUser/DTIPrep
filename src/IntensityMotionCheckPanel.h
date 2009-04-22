
#ifndef INTENSITYMOTIONCHECKPANEL_H
#define INTENSITYMOTIONCHECKPANEL_H

#include <QtGui/QDockWidget>
#include <QtGui/QMainWindow>
#include "ui_IntensityMotionCheckPanel.h"

#include "itkImage.h"
#include "itkVectorImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiffusionTensor3DReconstructionImageFilter.h"

#include "Protocal.h"
#include "QCResult.h"

class CThreadIntensityMotionCheck;

class IntensityMotionCheckPanel : public QDockWidget, private Ui_IntensityMotionCheckPanel
{
	Q_OBJECT

public:
	IntensityMotionCheckPanel(QMainWindow *parent=0);
	~IntensityMotionCheckPanel(void);

	void SetFileName(QString nrrd );

signals:
	void status(const QString &);

	private slots:
		
		//void on_comboBox_Protocal_currentIndexChanged(QString protocalName);

		void on_treeWidget_itemDoubleClicked(QTreeWidgetItem * item, int column) ;
		void on_treeWidget_itemChanged(QTreeWidgetItem * item, int column) ;
		void on_treeWidget_currentItemChanged( QTreeWidgetItem *current,  QTreeWidgetItem *previous);

		void on_treeWidget_Results_itemDoubleClicked(QTreeWidgetItem * item, int column) ;
		void on_treeWidget_Results_currentItemChanged( QTreeWidgetItem *current,  QTreeWidgetItem *previous);
		void on_treeWidget_Results_itemChanged(QTreeWidgetItem * item, int column) ;

		void on_pushButton_Save_clicked( );
		void on_pushButton_DefaultProtocol_clicked( );
		void on_pushButton_SaveProtocolAs_clicked( );

		void on_toolButton_ProtocalFileOpen_clicked( );

		void on_pushButton_RunPipeline_clicked( );
		//void on_pushButton_CreateDefaultProtocol_clicked( );

		void on_pushButton_SaveDWIAs_clicked( );
		void on_pushButton_SaveDWI_clicked( );

		void on_pushButton_DefaultQCResult_clicked( );
		void on_pushButton_OpenQCReport_clicked( );
		void on_pushButton_SaveQCReport_clicked( );
		void on_pushButton_SaveQCReportAs_clicked( );

		//MRI Tabbed plane
		//	void on_pushButton_Identity_clicked( );
		//	void on_pushButton_ExchangeXY_clicked( );
		//	void on_pushButton_ExchangeXZ_clicked( );
		//	void on_pushButton_ExchangeYZ_clicked( );
		//	void on_pushButton_FlipX_clicked( );
		//	void on_pushButton_FlipY_clicked( );
		//	void on_pushButton_FlipZ_clicked( );

		//	void on_pushButton_InformationCheck_clicked( );
		//	void on_pushButton_DWIInfoUpdateProtocal_clicked( );

		//	void on_toolButton_ReportFile_clicked( );
		//	void on_toolButton_QCOutputNrrd_clicked( );
		//	void on_pushButton_QCUpdateProtocol_clicked( );
		//	void on_pushButton_QCCheck_clicked( );

		//	void on_toolButton_EddyMotionCommand_clicked( );
		//	void on_toolButton_EddyMotionInput_clicked( );
		//	void on_toolButton_EddyMotionOutput_clicked( );
		//	void on_pushButton_CorrectUpdateProtocol_clicked( );
		//	void on_pushButton_EddyMotionCorrect_clicked( );

		//	void on_toolButton_dtiestim_clicked( );
		//	void on_toolButton_dtiprocess_clicked( );
		//	void on_toolButton_MaskFile_clicked( );
		//	void on_toolButton_TensorFile_clicked( );

		//	void on_pushButton_DTIUpdateProtocol_clicked( );
		//	void on_pushButton_DTICompute_clicked( );




		//treeWidget
		//comboBox_Protocal

		void ResultUpdate();
		void UpdateProgressBar(const int pos);


private:
	CThreadIntensityMotionCheck  *ThreadIntensityMotionCheck;


public:

	bool GetSliceProtocolParameters( 
		double beginSkip, 
		double endSkip, 
		double &baselineCorrelationThreshold ,  
		double &gradientCorrelationThreshold ,
		double &baselineCorrelationDeviationThreshold ,  
		double &gradientCorrelationDeviationThreshold
		);

	bool GetInterlaceProtocolParameters(
		double &correlationThresholdBaseline, 
		double &correlationThresholdGradient,
		double &correlationBaselineDevTimes, 
		double &correlationGradientDevTimes
		);

	CThreadIntensityMotionCheck  *GetThreadIntensityMotionCheck(){ return  ThreadIntensityMotionCheck; };

	typedef unsigned short DwiPixelType;
	typedef itk::Image<DwiPixelType, 2>			SliceImageType;
	typedef itk::Image<DwiPixelType, 3>			GradientImageType;
	typedef itk::VectorImage<DwiPixelType, 3>	DwiImageType;    
	typedef itk::ImageFileReader<DwiImageType>	DwiReaderType;
	typedef itk::ImageFileWriter<DwiImageType>	DwiWriterType;


	typedef itk::DiffusionTensor3DReconstructionImageFilter< DwiPixelType, DwiPixelType, double >		TensorReconstructionImageFilterType;
	typedef	TensorReconstructionImageFilterType::GradientDirectionContainerType							GradientDirectionContainerType;

	//void SetFileName(std::string filename) {DwiFileName = filename; };
	bool GetGridentDirections( bool bDisplay);
	bool GetGridentDirections();

	bool LoadDwiImage();

	QCResult &GetQCResult(){ return qcResult;};
	Protocal &GetProtocal(){ return protocal;};

	void UpdatePanelDWI( );
	void UpdateProtocolTree( );

	void SetDWIImage(DwiImageType::Pointer DWIImage) { DwiImage = DWIImage; bDwiLoaded=true; };

	void GenerateCheckOutputImage( std::string filename);

	void DefaultProcess( );
	void OpenQCReport( );
	void OpenXML();

	void CreateDefaultProtocol();
private:

	bool bProtocol;
	Protocal protocal;
	QCResult qcResult;

	std::string DwiFileName;
	DwiReaderType::Pointer DwiReader;
	DwiImageType::Pointer DwiImage;

	bool bDwiLoaded;
	bool bGetGridentDirections;
	bool readb0 ;
	double b0 ;
	GradientDirectionContainerType::Pointer	   GradientDirectionContainer;

	bool bResultTreeEditable;
	bool bProtocolTreeEditable;

  
      //tree_widget->openPersistentEditor(item,col);
      //tree_widget->closePersistentEditor(item,col); 
	//QTreeWidget::itemDoubleClicked(QTreeWidgetItem*,int) 
	//void QTreeWidget::itemChanged(QTreeWidgetItem * item, int column) 
	//ovoid QTreeWidget::currentItemChanged( QTreeWidgetItem *current,  QTreeWidgetItem *previous);
};

#endif // INTENSITYMOTIONCHECKPANEL_H

