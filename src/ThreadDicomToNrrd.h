#ifndef ThreadDicomToNrrd_H
#define ThreadDicomToNrrd_H

#include <QThread>

class CThreadDicomToNrrd : public QThread
{
    Q_OBJECT

public:
    CThreadDicomToNrrd(QObject *parent = 0);
    ~CThreadDicomToNrrd();

	QString DicomToNrrdCmd;
	QString DicomDir;
	QString NrrdDir;
	QString NrrdFileName;

signals:
    void kkk( int );
	void allDone(const QString &);
	void QQQ();


protected:
    void run();

private:
 
};

#endif
