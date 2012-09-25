#include "ThreadEddyMotionCorrect.h"

#include <iostream>
#include <string>
#include <math.h>

CThreadEddyMotionCorrect::CThreadEddyMotionCorrect(QObject *parentLocal) :
  QThread(parentLocal)
{
}

CThreadEddyMotionCorrect::~CThreadEddyMotionCorrect()
{
}

void CThreadEddyMotionCorrect::run()
{
  emit allDone("transform begings");

  for( int i = 0; i < 10000; i++ )
    {
    emit kkk( ( i + 1 ) / 100 );
    std::cout << i << std::endl;
    }

  emit allDone("Transform ends");
}
