
#ifndef EDDYMOTIONCORRECTPANEL_H
#define EDDYMOTIONCORRECTPANEL_H

#include <QtGui/QDockWidget>
#include <QtGui/QMainWindow>
#include "ui_EddyMotionCorrectPanel.h"

class EddyMotionCorrectPanel : public QDockWidget, private Ui_EddyMotionCorrectPanel
{
    Q_OBJECT

public:
    EddyMotionCorrectPanel(QMainWindow *parent=0);
	~EddyMotionCorrectPanel(void);

private slots:

private:

};

#endif // EDDYMOTIONCORRECTPANEL_H

