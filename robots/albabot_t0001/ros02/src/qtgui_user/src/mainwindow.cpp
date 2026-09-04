//#include "mainwindow.h"
#include "../include/qtgui_user/mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "rviz/new_object_dialog.h"

int m_nData1[BUFFER_SIZE], m_nData2[BUFFER_SIZE];
int m_nDataCnt;

// Thread Function
void *t_function(void *mdata)
{
  pid_t pid;
  pthread_t tid;
  pid = getpid();
  tid = pthread_self();
  printf ("\nRaspberry Pi Up_threadART Start : kmh\n");
  MainWindow *w = (MainWindow*)mdata;

  while(w->m_bln_Thread_flag)
  {
    w->DrawData(w);
  }
}

MainWindow::MainWindow(int argc, char** argv, QWidget *parent) :
  QMainWindow(parent),qnode(argc,argv) ,
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  initVar();

  //===================================================== qwt Init =============================================================//
  //init_qwt();

  //connect(&qnode, SIGNAL(rosShutdown()), this, SLOT(close()));

  //====================================================== qthread =============================================================//
  //m_bln_Thread_flag = true;
  //m_nthr_id = pthread_create(&m_p_thread, NULL, t_function, this);
  //usleep(1000);

  //====================================================== Timer ===============================================================//
//  m_pTimer = new QTimer(this);
//  connect(m_pTimer, SIGNAL(timeout()), this, SLOT(onTimer()));

//  m_pTimer->start(100);
//  g_time.start();


  //=============================================== ROS Initialize =================================================================//
  QMessageBox msgBox;

  if ( !qnode.init() )
  {
    m_blnRosConnectFlag = false;
    showNoMasterMessage();
  }
  else
  {
    m_blnRosConnectFlag = true;
  }

  sleep(1);

  qRegisterMetaType<cv::Mat>("cv::Mat");
  connect(&qnode, SIGNAL(sendImage(cv::Mat)),this, SLOT(dispImage(cv::Mat)));
}

MainWindow::~MainWindow()
{
  if(m_pTimer->isActive())
      m_pTimer->stop();

  delete ui;
}

void MainWindow::initVar()
{
  m_blnRosConnectFlag = false;

  memset(m_nData1, 0x00, sizeof(int)*BUFFER_SIZE);
  memset(m_nData2, 0x00, sizeof(int)*BUFFER_SIZE);

  m_nDataCnt = 0;
}

void MainWindow::showNoMasterMessage()
{
    QMessageBox msgBox;
    msgBox.setText("Couldn't find the ros master.");
    msgBox.exec();
    //close();
}

void MainWindow::init_qwt()
{
//  // Channel 1
//  ui->qwtPlot->setTitle( "Sensor Data" );
//  //ui->qwtPlot->setCanvasBackground( Qt::white );
//  ui->qwtPlot->setCanvasBackground( Qt::gray );
//  ui->qwtPlot->setAxisScale( QwtPlot::yLeft, -50.0, 500.0);
//  ui->qwtPlot->insertLegend( new QwtLegend() );

//  QwtPlotGrid *grid = new QwtPlotGrid();
//  grid->attach( ui->qwtPlot );

//  // Reference
//  qwtPlotCurve_Data1 = new QwtPlotCurve();
//  qwtPlotCurve_Data1->setTitle( "Reference" );
//  qwtPlotCurve_Data1->setPen( Qt::red, 2 );
//  qwtPlotCurve_Data1->setRenderHint( QwtPlotItem::RenderAntialiased, true );

//  QwtSymbol *symbol1 = new QwtSymbol( QwtSymbol::Ellipse,
//      QBrush( Qt::red ), QPen( Qt::red, 1 ), QSize( 3, 3 ) );
//  qwtPlotCurve_Data1->setSymbol( symbol1 );

//  // Data
//  qwtPlotCurve_Data2 = new QwtPlotCurve();
//  qwtPlotCurve_Data2->setTitle( "Sensor Data" );
//  qwtPlotCurve_Data2->setPen( Qt::green, 2 );
//  qwtPlotCurve_Data2->setRenderHint( QwtPlotItem::RenderAntialiased, true );

//  QwtSymbol *symbol2 = new QwtSymbol( QwtSymbol::Ellipse,
//      QBrush( Qt::green ), QPen( Qt::green, 1 ), QSize( 4, 4 ) );
//  qwtPlotCurve_Data2->setSymbol( symbol2 );

//  //---------------------------------------------- SET ---------------------------------------------//
//  qwtPlotCurve_Data1->attach( ui->qwtPlot );
//  qwtPlotCurve_Data2->attach( ui->qwtPlot );
//  ui->qwtPlot->show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  QMainWindow::closeEvent(event);
}

void MainWindow::dispImage(cv::Mat img)
{
//  ROS_INFO("dispImage start");
//  QImage image= QImage((uchar*) qnode.srcImgShow1.data,
//                        qnode.srcImgShow1.cols,
//                        qnode.srcImgShow1.rows,
//                        qnode.srcImgShow1.step,
//                        QImage::Format_RGB888);

  QImage image= QImage((uchar*) img.data,
                        img.cols,
                        img.rows,
                        img.step,
                        QImage::Format_RGB888);

  ui->lbView->setScaledContents(true);
  ui->lbView->setPixmap(QPixmap::fromImage(image));

//  ROS_INFO("dispImage end");
}

void MainWindow::rvizPanelDoubleClick(int nx,int ny)
{
  qDebug()<<"Double Click : "<<nx<<","<<ny<<endl;
}

void MainWindow::rvizPanelPress(int nx,int ny)
{
  qDebug()<<"Press : "<<nx<<","<<ny<<endl;
}

void MainWindow::rvizPanelRelease(int nx,int ny)
{
  qDebug()<<"Release : "<<nx<<","<<ny<<endl;
}

void MainWindow::DrawData(MainWindow *w)
{
  int nTmp;
  static int degree = 0;
  int result;
  double rad;

  //nTmp = qrand()%500;
  rad = 3.141592*degree / 180.0;
  result = 300.0 - (int)(sin(rad)*200.0);

  if(result >= 500)
  {
    result = 500;
    qDebug()<<"Over"<<endl;
  }

  //qDebug()<<"result:" <<result;

  degree+=1;
  if(degree >= 360)
    degree = 0;

  //m_nData[BUFFER_SIZE-1] = nTmp;
  //m_nDataCnt++;
  m_nData1[BUFFER_SIZE-1] = 300;
  m_nData2[BUFFER_SIZE-1] = result;

  for(int i = 0; i < BUFFER_SIZE-1; i++)
  {
    m_nData1[i] = m_nData1[i+1];
    m_nData2[i] = m_nData2[i+1];
    //if(i == BUFFER_SIZE-2)
    //  qDebug()<<"m_nData[i]:" <<m_nData[i];
  }
}

void MainWindow::onTimer()
{
  static int cnt = 0;

  if(m_blnRosConnectFlag)
  {
    //qDebug("onTimer start");
    //ROS_INFO("onTimer start");

//    QImage image1= QImage((uchar*) qnode.srcImgShow1.data,
//                          qnode.srcImgShow1.cols,
//                          qnode.srcImgShow1.rows,
//                          qnode.srcImgShow1.step,
//                          QImage::Format_RGB888);

//    ui->lbView->setScaledContents(true);
//    ui->lbView->setPixmap(QPixmap::fromImage(image1));

    // qwt draw
//    if(cnt%5 == 0)
//    {

//      QPolygonF points1;
//      QPolygonF points2;

//      for(int i = 0 ; i < BUFFER_SIZE ; i++)
//      {
//          points1 << QPointF( i, m_nData1[i]);
//          points2 << QPointF( i, m_nData2[i]);
//      }

//      qwtPlotCurve_Data1->setSamples( points1 );
//      qwtPlotCurve_Data2->setSamples( points2 );
//      ui->qwtPlot->replot();
//    }
//    cnt++;
  }
}
