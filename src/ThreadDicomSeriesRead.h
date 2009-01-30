#ifndef ThreadDicomSeriesRead_H
#define ThreadDicomSeriesRead_H

#include <QThread>

class CThreadDicomSeriesRead : public QThread
{
    Q_OBJECT

public:
    CThreadDicomSeriesRead(QObject *parent = 0);
    ~CThreadDicomSeriesRead();


signals:
    void kkk( int );
	void status(const QString &);

protected:
    void run();

private:
 
 
};

#endif
