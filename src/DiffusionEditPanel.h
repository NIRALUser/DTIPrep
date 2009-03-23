
#ifndef DIFFUSIONEDITPANEL_H
#define DIFFUSIONEDITPANEL_H

#include <QtGui/QDockWidget>
#include <QtGui/QMainWindow>
#include "ui_DiffusionEditPanel.h"

class DiffusionEditPanel : public QDockWidget, private Ui_DiffusionEditPanel
{
    Q_OBJECT

public:
    DiffusionEditPanel(QMainWindow *parent=0);
	~DiffusionEditPanel(void);

private slots:

private:

};

#endif // DIFFUSIONEDITPANEL_H

