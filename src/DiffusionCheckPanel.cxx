#include "DiffusionCheckPanel.h"

DiffusionCheckPanel::DiffusionCheckPanel(QMainWindow *parent):QDockWidget(parent)
{
	 setupUi(this);
	 verticalLayout->setContentsMargins(0,0,0,0);
}

DiffusionCheckPanel::~DiffusionCheckPanel(void)
{
}
