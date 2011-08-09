#include "FurtherQCThread.h"

#include <string>
#include <math.h>
#include <QtGui>
#include <QThread>
#include <QFont>



CFurtherQCThread::CFurtherQCThread(QObject *parentLocal) : QThread(parentLocal)
{
  	DWINrrdFilename = "";
  	XmlFilename = "";
  	protocol = NULL;
  	qcResult = NULL;
  	mf_IntensityMotionCheck = new CIntensityMotionCheck;
	dwi = NULL;
}

CFurtherQCThread::~CFurtherQCThread()
{}

void CFurtherQCThread::run()
{
  
  if ( !protocol )
  {
    std::cout << "protocol not set (in Further QC)!" << std::endl;
    return;
  }

  if ( !qcResult )
  {
    std::cout << "qcResult not set (in Further QC)!" << std::endl;
    return;
  }

  std::cout << "Further QC thread begins here " << std::endl;
  
  mf_IntensityMotionCheck->SetXmlFileName(XmlFilename);
  mf_IntensityMotionCheck->SetProtocol_FurtherQC( protocol);
  mf_IntensityMotionCheck->SetQCResult( qcResult);
  mf_IntensityMotionCheck->LoadDwiImage_FurtherQC( this->Getdwi() );
  std::cout << " Test Further 1 " << std::endl;
  emit f_StartProgressSignal(); 	// start showing progress bar
  mf_IntensityMotionCheck->RunPipelineByProtocol_FurtherQC();


  /*printf("result of PIPELINE = %d",result);

  unsigned char out = result;
  std::cout << "--------------------------------" << std::endl;

  out = result;
  out = out >> 7;
  if ( out )
  {
    std::cout << "QC FAILURE: too many gradients excluded!" << std::endl;
  }

  out = result;
  out = out << 1;
  out = out >> 7;
  if ( out )
  {
    std::cout << "QC FAILURE: Single b-value DWI without a b0/baseline!"
      << std::endl;
  }

  out = result;
  out = out << 2;
  out = out >> 7;
  if ( out )
  {
    std::cout << "QC FAILURE: Gradient direction # is less than 6!"
      << std::endl;
  }

  out = result;
  out = out << 7;
  out = out >> 7;
  if ( out  )
  {
    std::cout << "Image information check:\tFAILURE" << std::endl;
  }
  else
  {
    std::cout << "Image information check:\tPASS" << std::endl;
  }

  out = result;
  out = out << 6;
  out = out >> 7;
  if ( out )
  {
    std::cout << "Diffusion information check:\tFAILURE" << std::endl;
  }
  else
  {
    std::cout << "Diffusion information check:\tPASS" << std::endl;
  }

  out = result;
  out = out << 5;
  out = out >> 7;
  if ( out  )
  {
    std::cout << "Slice-wise check:\t\tFAILURE" << std::endl;
  }
  else
  {
    std::cout << "Slice-wise check:\t\tPASS" << std::endl;
  }

  out = result;
  out = out << 4;
  out = out >> 7;
  if ( out  )
  {
    std::cout << "Interlace-wise check:\t\tFAILURE" <<  std::endl;
  }
  else
  {
    std::cout << "Interlace-wise check:\t\tPASS" << std::endl;
  }

  out = result;
  out = out << 3;
  out = out >> 7;
  if ( out )
  {
    std::cout << "Gradient-wise check:\t\tFAILURE" << std::endl;
  }
  else
  {
    std::cout << "Gradient-wise check:\t\tPASS" << std::endl;
  }
  */
  
  emit f_StopProgressSignal();  // hiding progress bar

  //emit allDone("Checking Thread ended");
  emit ResultUpdate();
}

