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
#include "../include/qtRobot/main_window.hpp"

#include <QDebug>

/*****************************************************************************
** Namespaces
*****************************************************************************/

namespace qtRobot {

using namespace Qt;


MainWindow::MainWindow(int argc, char** argv, QWidget *parent)
	: QMainWindow(parent)
	, qnode(argc,argv)
{
	ui.setupUi(this); // Calling this incidentally connects all ui's triggers to on_...() callbacks in this class.
    QObject::connect(ui.actionAbout_Qt, SIGNAL(triggered(bool)), qApp, SLOT(aboutQt())); // qApp is a global variable for the application

	setWindowIcon(QIcon(":/images/icon.png"));

    QObject::connect(&qnode, SIGNAL(rosShutdown()), this, SLOT(close()));
    QObject::connect(&qnode, SIGNAL(loggingUpdated()), this, SLOT(updateLoggingView()));


    QObject::connect(&qnode, SIGNAL(emitUpdateHeadImage()), this, SLOT(updateHeadImageView()));
    QObject::connect(&qnode, SIGNAL(emitUpdateThermalImage()), this, SLOT(updateThermalImageView()));
    QObject::connect(&qnode, SIGNAL(emitUpdateManagerReImage()), this, SLOT(updateManagerReImageView()));


    qnode.init("http://172.16.200.158:11311","172.16.200.158");


    ui.labelDisplay1->move(160,100);
    ui.labelDisplay2->move(160+640,100+0);
    ui.labelDisplay3->move(160+640,100+240);

    ui.labelDisplay1->installEventFilter(this);

    //QImage m_imgDisplay(640,480,QImage::Format_RGB888);

    m_procAudio=NULL;
    m_procVideo=NULL;
    m_procAudio=new QProcess(this);
    m_procVideo=new QProcess(this);

}

MainWindow::~MainWindow() {}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
  if((watched == ui.labelDisplay1) && event->type() == QEvent::MouseButtonPress)
  {
    qDebug() << "touched";
    ui.labelDisplay2->setPixmap(QPixmap::fromImage(m_imgDisplay).scaled(160,120,Qt::IgnoreAspectRatio));

  }
}

void MainWindow::updateLoggingView() {
        //ui.view_logging->scrollToBottom();
}



void MainWindow::updateThermalImageView()
{

    cv::flip(qnode.cv_ptrThermal->image,qnode.cv_ptrThermal->image,-1);


  QImage imdisplay((uchar*)qnode.cv_ptrThermal->image.data, qnode.cv_ptrThermal->image.cols, qnode.cv_ptrThermal->image.rows,
                   qnode.cv_ptrThermal->image.step, QImage::Format_RGB888);

  //m_imgDisplay=imdisplay;

  //ui.labelDisplay3->setPixmap(QPixmap::fromImage(imdisplay).scaled(160,120,Qt::IgnoreAspectRatio));
  ui.labelDisplay3->setPixmap(QPixmap::fromImage(imdisplay).scaled(320,240,Qt::IgnoreAspectRatio));

  //ui.display_ThermalImage->setPixmap(QPixmap::fromImage(imdisplay).scaled(160,120,Qt::IgnoreAspectRatio));
  //ui.display_ThermalImage->resize(qnode.cv_Thermalptr->image.cols,qnode.cv_Thermalptr->image.rows);
  //ui.display_ThermalImage->resize(160,120);



}

void MainWindow::updateHeadImageView()
{
  //qDebug() << "Main image!!!!! => " << qnode.cv_ptr->image.rows << " / " << qnode.cv_ptr->image.cols;

  QImage imdisplay((uchar*)qnode.cv_ptrHead->image.data, qnode.cv_ptrHead->image.cols, qnode.cv_ptrHead->image.rows,
                   qnode.cv_ptrHead->image.step, QImage::Format_RGB888);

  //ui.labelDisplay2->setPixmap(QPixmap::fromImage(imdisplay).scaled(160,120,Qt::IgnoreAspectRatio));
  ui.labelDisplay2->setPixmap(QPixmap::fromImage(imdisplay).scaled(320,240,Qt::IgnoreAspectRatio));


}

void MainWindow::updateManagerReImageView()
{
  //qDebug() << "Main image!!!!! => " << qnode.cv_ptr->image.rows << " / " << qnode.cv_ptr->image.cols;

  QImage imdisplay((uchar*)qnode.cv_ptrManagerRe->image.data, qnode.cv_ptrManagerRe->image.cols, qnode.cv_ptrManagerRe->image.rows,
                   qnode.cv_ptrManagerRe->image.step, QImage::Format_RGB888);

  //ui.labelDisplay1->setPixmap(QPixmap::fromImage(imdisplay));
  //ui.labelDisplay1->resize(qnode.cv_ptrManagerRe->image.cols,qnode.cv_ptrManagerRe->image.rows);

  ui.labelDisplay1->setPixmap(QPixmap::fromImage(imdisplay).scaled(640,480,Qt::IgnoreAspectRatio));
}


void MainWindow::closeEvent(QCloseEvent *event)
{
  //WriteSettings();
	QMainWindow::closeEvent(event);
}


void MainWindow::on_pushButton_call_agent_clicked()
{
    static int i=0;

    //if(i==0)
    //{
      if(m_procAudio->isOpen())
      {
        m_procAudio->close();
      }
      //m_procAudio->start("roslaunch audio_capture capture.launch ns:=robot format:=mp3");

      m_procAudio->start("roslaunch tb2_base_pkg start_media.launch");

        //QProcess ps;
        //ps.start("roslaunch tb2_base_pkg start_media.launch");

    /*}
    else {
      if(m_procAudio->isOpen())
      {
        m_procAudio->close();
      }

      /*m_procAudio->start("rosnode kill /robot/audio_capture");
      m_procAudio->start("rosnode kill /manager/audio_play");*/

      //QProcess ps,ps1;
      //start("rosnode kill /robot/audio_capture");
      //ps1.start("rosnode kill /manager/audio_play");

    //}
    //i=1-i;
    qDebug()<<"Park" << i;
}

void MainWindow::on_pushButton_2_clicked()
{
    if(m_procVideo->isOpen())
    {
      m_procVideo->close();
    }
    //m_procVideo->start("rosnode kill /robot/audio_capture /manager/audio_play");
    m_procVideo->start("rosnode kill /manager/audio_play");


    //ps.start("rosnode kill /robot/audio_capture");
    //ps1.start("rosnode kill /manager/audio_play");


#if 0
    static int i=0;

    if(i==0)
    {
      if(m_procVideo->isOpen())
      {
        m_procVideo->close();
      }
      //m_procVideo->start("rosrun image_transport republish compressed in:=/manager_cam/image_raw raw out:=/manager_cam_re/image_raw");
      m_procVideo->start("roslaunch tb2_base_pkg start_media.launch");

    }
    else {
      if(m_procVideo->isOpen())
      {
        m_procVideo->close();
      }
      //m_procVideo->start("rosnode kill /img_republish");

    }
    i=1-i;

    /*
  <node pkg="image_transport" type="republish" name="republish2"
      args="compressed in:=/manager_cam/color/image_raw raw out:=/manager_cam_re/image_raw"/>


  m_proc->start("roslaunch audio_capture capture.launch ns:=robot format:=mp3");*/
#endif

}


}  // namespace qtRobot



