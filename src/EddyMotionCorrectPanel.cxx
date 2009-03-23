#include "EddyMotionCorrectPanel.h"

EddyMotionCorrectPanel::EddyMotionCorrectPanel(QMainWindow *parent):QDockWidget(parent)
{
	 setupUi(this);
	 verticalLayout->setContentsMargins(0,0,0,0);
}

EddyMotionCorrectPanel::~EddyMotionCorrectPanel(void)
{
}
