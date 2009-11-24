#include "ThreadDicomSeriesRead.h"
#include <iostream>
#include <string>
#include <math.h>

CThreadDicomSeriesRead::CThreadDicomSeriesRead(QObject *parent) :
  QThread(parent)
    {}

CThreadDicomSeriesRead::~CThreadDicomSeriesRead()
    {}

void CThreadDicomSeriesRead::run()
{
  emit status("transform begings");

  for ( int i = 0; i < 10000; i++ )
    {
    emit kkk( ( i + 1 ) / 100 );
    std::cout << i << std::endl;
    }

  emit status("Transform ends");
}
