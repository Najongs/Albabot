/**
 * @file /src/main_window.cpp
 *
 * @brief Implementation for the qt gui.
 *
 * @date February 2011
 **/
/*****************************************************************************
** Includes
*****************************************************************************/

#include <QtGui>
#include <QMessageBox>
#include <iostream>
#include "../include/qthello/main_window.hpp"

#include <QDebug>

/*****************************************************************************
** Namespaces
*****************************************************************************/

namespace qthello {

using namespace Qt;

/*****************************************************************************
** Implementation [MainWindow]
*****************************************************************************/



MainWindow::MainWindow(int argc, char** argv, QWidget *parent)
	: QMainWindow(parent)
	, qnode(argc,argv)
{
	ui.setupUi(this); // Calling this incidentally connects all ui's triggers to on_...() callbacks in this class.
    QObject::connect(ui.actionAbout_Qt, SIGNAL(triggered(bool)), qApp, SLOT(aboutQt())); // qApp is a global variable for the application

    //ReadSettings();
	setWindowIcon(QIcon(":/images/icon.png"));


  //ui.tab_manager->setCurrentIndex(0); // ensure the first tab is showing - qt-designer should have this already hardwired, but often loses it (settings?).
    QObject::connect(&qnode, SIGNAL(rosShutdown()), this, SLOT(close()));

  //ui.view_logging->setModel(qnode.loggingModel());
    QObject::connect(&qnode, SIGNAL(loggingUpdated()), this, SLOT(updateLoggingView()));
    QObject::connect(&qnode, SIGNAL(viewUpdate()), this, SLOT(updateImageView()));
    QObject::connect(&qnode, SIGNAL(viewThermalUpdate()), this, SLOT(updateThermalImageView()));

    qnode.init("http://192.168.1.103:11311","192.168.1.103");

    ui.display_image->move(0,0);
    ui.display_ThermalImage->move(640,0);

}

MainWindow::~MainWindow() {}



void MainWindow::updateLoggingView() {
        //ui.view_logging->scrollToBottom();
}

void MainWindow::updateThermalImageView()
{
  //qDebug() << "Main image!!!!! => " << qnode.cv_ptr->image.rows << " / " << qnode.cv_ptr->image.cols;

  QImage imdisplay((uchar*)qnode.cv_Thermalptr->image.data, qnode.cv_Thermalptr->image.cols, qnode.cv_Thermalptr->image.rows,
                   qnode.cv_Thermalptr->image.step, QImage::Format_RGB888);

  ui.display_ThermalImage->setPixmap(QPixmap::fromImage(imdisplay));
  //ui.display_ThermalImage->resize(qnode.cv_Thermalptr->image.cols,qnode.cv_Thermalptr->image.rows);
  ui.display_ThermalImage->resize(160,120);
}

void MainWindow::updateImageView()
{
  //qDebug() << "Main image!!!!! => " << qnode.cv_ptr->image.rows << " / " << qnode.cv_ptr->image.cols;

  QImage imdisplay((uchar*)qnode.cv_ptr->image.data, qnode.cv_ptr->image.cols, qnode.cv_ptr->image.rows,
                   qnode.cv_ptr->image.step, QImage::Format_RGB888);

  ui.display_image->setPixmap(QPixmap::fromImage(imdisplay));
  ui.display_image->resize(qnode.cv_ptr->image.cols,qnode.cv_ptr->image.rows);
}


void MainWindow::closeEvent(QCloseEvent *event)
{
  //WriteSettings();
	QMainWindow::closeEvent(event);
}

}  // namespace qtros





