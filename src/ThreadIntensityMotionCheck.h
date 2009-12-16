#ifndef ThreadIntensityMotionCheck_H
#define ThreadIntensityMotionCheck_H

#include <QThread>

class Protocol;
class QCResult;
class CIntensityMotionCheck;

class CThreadIntensityMotionCheck : public QThread
{
	Q_OBJECT
public:
	CThreadIntensityMotionCheck(QObject *parent = 0);
	~CThreadIntensityMotionCheck();

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

protected:
	void run();

private:
	std::string DWINrrdFilename;
	std::string XmlFilename;

	Protocol    *protocol;
	QCResult    *qcResult;
public:
	// CIntensityMotionCheck* IntensityMotionCheck;
};

#endif
