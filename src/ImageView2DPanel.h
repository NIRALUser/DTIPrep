
#ifndef IMAGEVIEW2DPANEL_H
#define IMAGEVIEW2DPANEL_H

#include <QtGui/QDockWidget>
#include <QtGui/QMainWindow>
#include "ui_ImageView2DPanel.h"

class vtkCylinderSource;
class vtkPolyDataMapper;
class vtkExodusReader;
class vtkDataSetMapper;
class vtkActor;
class vtkRenderer;
class vtkImageViewer2;

class ImageView2DPanel  : public QDockWidget, private Ui_ImageView2DPanel
{
    Q_OBJECT

public:
    ImageView2DPanel(QString title, QMainWindow *parent=0);
	~ImageView2DPanel(void);

private slots:

private:
    vtkCylinderSource* source;
    vtkPolyDataMapper* mapper;
    vtkActor* actor;
    vtkRenderer* ren;

    vtkImageViewer2 *ImageViewer2;

};

#endif // IMAGEVIEW2DPANEL_H

