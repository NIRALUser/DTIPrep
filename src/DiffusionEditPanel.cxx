#include "DiffusionEditPanel.h"

DiffusionEditPanel::DiffusionEditPanel(QMainWindow *parent):QDockWidget(parent)
{
	 setupUi(this);
	 verticalLayout->setContentsMargins(0,0,0,0);
}

DiffusionEditPanel::~DiffusionEditPanel(void)
{
}
