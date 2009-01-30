#ifndef ThreadIntensityMotionCheck_H
#define ThreadIntensityMotionCheck_H


#include <QThread>

class Protocal;
class QCResult;
class CIntensityMotionCheck;

class CThreadIntensityMotionCheck : public QThread
{
    Q_OBJECT

public:
    CThreadIntensityMotionCheck(QObject *parent = 0);
    ~CThreadIntensityMotionCheck();

    void SetFileName(std::string filename){ DWINrrdFilename = filename;};
	void SetProtocal( Protocal *p) { protocal = p;};
	void SetQCResult( QCResult *r) { qcResult = r;};
signals:
    void allDone(const QString &);
    void ResultUpdate();
    void QQQ();
    void kkk( int );


protected:
    void run();

private:
   std::string DWINrrdFilename;
   Protocal *protocal;
   QCResult *qcResult;
   //CIntensityMotionCheck* IntensityMotionCheck;
};

#endif
