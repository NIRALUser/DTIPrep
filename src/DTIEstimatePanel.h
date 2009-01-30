
#ifndef DTIESTIMATEPANEL_H
#define DTIESTIMATEPANEL_H

#include <QtGui/QDockWidget>
#include <QtGui/QMainWindow>
#include "ui_DTIEstimatePanel.h"

class DTIEstimatePanel : public QDockWidget, private Ui_DTIEstimatePanel
{
    Q_OBJECT

public:
    DTIEstimatePanel(QMainWindow *parent=0);
	~DTIEstimatePanel(void);

private slots:

private:

};

#endif // DTIESTIMATEPANEL_H

