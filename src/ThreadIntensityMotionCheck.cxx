
#include "ThreadIntensityMotionCheck.h"
#include "IntensityMotionCheck.h"
#include <string>
#include <math.h>

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
//	emit kkk(5);//emit QQQ(10);
	IntensityMotionCheck.SetProtocal( protocal);
//	emit kkk(10);//emit QQQ(10);
	IntensityMotionCheck.SetQCResult( qcResult);
//	emit kkk(15);//emit QQQ(10);
	IntensityMotionCheck.SetFileName(DWINrrdFilename);//,"report.txt");//
	IntensityMotionCheck.GetImagesInformation();
	//emit kkk(60);//emit QQQ(10);
	std::cout<<"begin check"<<std::endl;
	IntensityMotionCheck.CheckByProtocal();
	//emit kkk(100);//emit QQQ(10);

	emit allDone("check ended");
	emit ResultUpdate();
}

