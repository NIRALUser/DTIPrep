
#include "ThreadIntensityMotionCheck.h"
#include "IntensityMotionCheck.h"
#include <string>
#include <math.h>
#include <QtGui>

CThreadIntensityMotionCheck::CThreadIntensityMotionCheck(QObject *parent)
     : QThread(parent)
{
	DWINrrdFilename="";

	//IntensityMotionCheck = new CIntensityMotionCheck;//
	//IntensityMotionCheck->SetProtocal( protocal);
	//IntensityMotionCheck->SetQCResult( qcResult);



}

CThreadIntensityMotionCheck::~CThreadIntensityMotionCheck()
{
	//delete IntensityMotionCheck;
}

void CThreadIntensityMotionCheck::run()
{ 
	if(DWINrrdFilename.length()==0)
	{
		std::cout<<"DWI file name not set!"<<std::endl;
		return;
	}

	emit allDone("checking ...");

	qcResult->Clear();
	CIntensityMotionCheck IntensityMotionCheck(DWINrrdFilename);//,"report.txt");//
	IntensityMotionCheck.SetProtocal( protocal);
	IntensityMotionCheck.SetQCResult( qcResult);
	//IntensityMotionCheck.SetFileName(DWINrrdFilename);//,"report.txt");//
	IntensityMotionCheck.GetImagesInformation();

	std::cout<<"begin check"<<std::endl;
	unsigned short  result = IntensityMotionCheck.CheckByProtocal();
	
	unsigned short out;	
	out = result;
	
	std::cout << "===========================================================" << std::endl;
	out = result;
	out = out<<8;
	out = out>>15;
	if( out)
		std::cout << "QC FAILURE: too many gradients excluded:" << std::endl;

	out = result;
	out = out>>8;
	std::cout << "Gradient Included: "<< out<<std::endl;

	out = result;
	out = out<<15;
	out = out>>15;
	if( out )
		std::cout << "Image information check: FAILURE" << std::endl;
	else 
		std::cout << "Image information check: PASS" << std::endl;

	out = result;
	out = out<<14;
	out = out>>15;
	if( out )
		std::cout << "Diffusion information check: FAILURE" << std::endl;
	else 
		std::cout << "Diffusion information check: PASS" << std::endl;

	out = result;
	out = out<<13;
	out = out>>15;
	if( out  )
		std::cout << "Slice-wise check: FAILURE" << std::endl;
	else 
		std::cout << "Slice-wise check: PASS" << std::endl;

	out = result;
	out = out<<12;
	out = out>>15;
	if( out  )
		std::cout << "Interlace-wise check: FAILURE"<<  std::endl;
	else 
		std::cout << "Interlace-wise check: PASS" << std::endl;

	out = result;
	out = out<<11;
	out = out>>15;
	if( out )
		std::cout << "Gradient-wise check: FAILURE" << std::endl;
	else 
		std::cout << "Gradient-wise check: PASS" << std::endl;


	//emit kkk(100);//emit QQQ(10);

	emit allDone("check ended");
	emit ResultUpdate();
}

