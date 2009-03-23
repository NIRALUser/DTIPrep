
#ifndef DIFFUSIONCHECKPANEL_H
#define DIFFUSIONCHECKPANEL_H

#include <QtGui/QDockWidget>
#include <QtGui/QMainWindow>
#include "ui_DiffusionCheckPanel.h"

class DiffusionCheckPanel : public QDockWidget, private Ui_DiffusionCheckPanel
{
    Q_OBJECT

public:
    DiffusionCheckPanel(QMainWindow *parent=0);
	~DiffusionCheckPanel(void);

private slots:

private:

};

#endif // DIFFUSIONCHECKPANEL_H

