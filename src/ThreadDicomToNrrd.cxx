#include "ThreadDicomToNrrd.h"

#include <string>
#include <iostream>
#include <stdlib.h>
#include <libgen.h>

CThreadDicomToNrrd::CThreadDicomToNrrd(QObject *parentLocal) :
  QThread(parentLocal)
{
}

CThreadDicomToNrrd::~CThreadDicomToNrrd()
{
}

void CThreadDicomToNrrd::run()
{
  emit allDone("DicomToNrrd transforming ...");

  QString str;

  std::string outputVolumeFullName = std::string(NrrdFileName.toStdString() );
  std::string outputVolumeName =  basename(const_cast<char *>(outputVolumeFullName.c_str() ) );
  std::string outputDirName =  dirname(const_cast<char *>(outputVolumeFullName.c_str() ) );

  str.append( DicomToNrrdCmd );

  str.append( QString( tr(" --inputDicomDirectory ") ) );
  str.append( DicomDir );
  str.append( QString( tr(" --outputVolume ") ) );
  str.append( outputVolumeName.c_str() );
  str.append( QString( tr("  --outputDirectory ") ) );
  str.append( outputDirName.c_str() );

  emit StartProgressSignal_D2N();

  system( const_cast<char *>( str.toStdString().c_str() ) );

  emit StopProgressSignal_D2N();

  emit allDone("DicomToNrrd Transform ended");

}
