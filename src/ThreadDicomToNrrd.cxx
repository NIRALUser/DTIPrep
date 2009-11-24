#include "ThreadDicomToNrrd.h"

#include <string>
#include <iostream>

CThreadDicomToNrrd::CThreadDicomToNrrd(QObject *parent) :
  QThread(parent)
    {}

CThreadDicomToNrrd::~CThreadDicomToNrrd()
    {}

void CThreadDicomToNrrd::run()
{
  emit allDone("DicomToNrrd transforming ...");

  QString str;

  str += DicomToNrrdCmd;
  str += QString( tr("  ") );
  str += DicomDir;
  str += QString( tr("  ") );
  str += NrrdFileName;

  // for(int i=0;i< 10000;i++)
  // {
  //  emit kkk((i+1)/100);
  //  std::cout<<i<<std::endl;
  // }

  system( const_cast<char *>( str.toStdString().c_str() ) );

  emit allDone("DicomToNrrd Transform ended");
  emit QQQ();
}
