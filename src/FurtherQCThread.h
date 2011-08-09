#ifndef FurtherQCThread_H
#define FurtherQCThread_H

#include <QThread>
#include "IntensityMotionCheck.h"

//This Class is used to apply further QC after including some gradients in visual checking step which were exclude before

class QCResult;
class Protocol;

class CFurtherQCThread : public QThread
{
	Q_OBJECT
	public:
		CFurtherQCThread(QObject * parent=0);
		~CFurtherQCThread();

		typedef unsigned short                     DwiPixelType;
		typedef itk::VectorImage<DwiPixelType, 3>  DwiImageType;


		CIntensityMotionCheck * mf_IntensityMotionCheck;    //pointer to the core function to show its long running for progressBar

		void SetDwiFileName(std::string filename)
  		{
		DWINrrdFilename = filename;
		}
		
		void SetXmlFileName(std::string filename)
		{
		XmlFilename = filename;
		}
		
		void SetProtocol( Protocol *p)
		{
		protocol = p;
		}
		
		void SetQCResult( QCResult *r)
		{
		qcResult = r;
		}
		
		void Set_result(unsigned char r)
		{
		result = r;
		} 
		
		unsigned char & Get_result()
		{
		return result;
		} 
 
		void GenerateCheckOutputImage( DwiImageType::Pointer dwi);	// generating the nrrd file from input dwi which all its excluded gradients are taken away

		inline DwiImageType::Pointer Getdwi() const
		{
		return dwi;
		}
		
		inline void Setdwi( const DwiImageType::Pointer DWI)
		{
		this->dwi = DWI;
		} 

	signals:
		//void allDone(const QString &);
		
		void ResultUpdate();
		
		//void QQQ(int);
		
		//void kkk( int );
		
		void f_StartProgressSignal();   // Signal for progressBar function
		void f_StopProgressSignal();   // Signal for progressBar function
		
		
	protected:
		void run();
		
	private:
		std::string DWINrrdFilename;
		std::string XmlFilename;
		
		Protocol    *protocol;
		QCResult    *qcResult;
		DwiImageType::Pointer dwi;
		
		unsigned char result; // the result of RunPipelineByProtocol

		
};

#endif //FurtherQCThread_H
