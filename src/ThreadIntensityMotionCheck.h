#ifndef ThreadIntensityMotionCheck_H
#define ThreadIntensityMotionCheck_H

#include <QThread>

class Protocol;
class QCResult;

#include "IntensityMotionCheck.h"

class CThreadIntensityMotionCheck : public QThread
{
  Q_OBJECT
public:
  CThreadIntensityMotionCheck(QObject *parent = 0);
  ~CThreadIntensityMotionCheck();

  CIntensityMotionCheck * m_IntensityMotionCheck;    //pointer to the core function to show its long running for progressBar
  

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

signals:
  void allDone(const QString &);

  void ResultUpdate();

  void QQQ(int);

  void kkk( int );

  void StartProgressSignal();   // Signal for progressBar function
  void StopProgressSignal();   // Signal for progressBar function


protected:
  void run();
  
private:
  std::string DWINrrdFilename;
  std::string XmlFilename;

  Protocol    *protocol;
  QCResult    *qcResult;

};

#endif
