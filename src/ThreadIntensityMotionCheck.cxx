
#include "ThreadIntensityMotionCheck.h"
#include "IntensityMotionCheck.h"
#include <string>
#include <math.h>

CThreadIntensityMotionCheck::CThreadIntensityMotionCheck(QObject *parent)
     : QThread(parent)
{
  	//IntensityMotionCheck = new CIntensityMotionCheck("D:\\Liuzx\\DTIdeveloping\	\DataForTest\\DTI03\\DTI3_128x128x79x7.nhdr", "report.txt");//
	DWINrrdFilename="";


}

CThreadIntensityMotionCheck::~CThreadIntensityMotionCheck()
{
}

void CThreadIntensityMotionCheck::run()
{ 
	if(DWINrrdFilename.length()==0)
	{
		std::cout<<"DWI file name not set!"<<std::endl;
		return;
	}

	emit allDone("checking ...");

	CIntensityMotionCheck IntensityMotionCheck(DWINrrdFilename);//,"report.txt");//
	IntensityMotionCheck.SetProtocal( protocal);
	IntensityMotionCheck.SetQCResult( qcResult);
	IntensityMotionCheck.GetImagesInformation();
	IntensityMotionCheck.CheckByProtocal();

	emit allDone("check ended");
	emit ResultUpdate();
}

