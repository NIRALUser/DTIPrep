#ifndef ThreadEddyMotionCorrect_H
#define ThreadEddyMotionCorrect_H


#include <QThread>

class CThreadEddyMotionCorrect : public QThread
{
    Q_OBJECT

public:
    CThreadEddyMotionCorrect(QObject *parent = 0);
    ~CThreadEddyMotionCorrect();



signals:
    void kkk( int );
	void allDone(const QString &);

protected:
    void run();

private:
};

#endif
