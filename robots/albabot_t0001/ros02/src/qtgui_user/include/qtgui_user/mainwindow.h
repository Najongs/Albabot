#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <QString>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QListWidget>
#include <QVector>
#include <QFile>
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <math.h>
#include <qthread.h>

#include "rviz/visualization_manager.h"
#include "inrenderpanel.h"
#include "rviz/display.h"
#include "rviz/visualization_frame.h"
#include "rviz/tool.h"
#include "rviz/tool_manager.h"
#include "rviz/yaml_config_reader.h"
#include "rviz/yaml_config_writer.h"

#include <qwt/qwt.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_symbol.h>
#include <qwt/qwt_legend.h>

#include "qnode.h"

#define BUFFER_SIZE 360*4

extern int m_nData1[BUFFER_SIZE], m_nData2[BUFFER_SIZE];
extern int m_nDataCnt;

using namespace qtgui_user;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(int argc, char** argv, QWidget *parent = 0);
  ~MainWindow();

  void showNoMasterMessage();
  void init_qwt();
  void DrawData(MainWindow *w);
  void initVar();
public Q_SLOTS:
  void onTimer();

private slots:
  void rvizPanelDoubleClick(int,int);
  void rvizPanelPress(int,int);
  void rvizPanelRelease(int,int);
  void closeEvent(QCloseEvent *event); // Overloaded function
  void dispImage(cv::Mat img);

private:
  Ui::MainWindow *ui;
  QNode qnode;

  rviz::VisualizationManager* manager_;
  rviz::ToolManager* tool_manager_;
  rviz::Tool* pos_2d_init;
  InRenderPanel* render_panel_;
  rviz::Display* grid_;
  rviz::Display * map_;
  rviz::Display * robotmodel_;  
  rviz::Display * odom_;
  rviz::Display * arrow_;
  rviz::Display * marker_;
  rviz::Display * markerArray_;
  rviz::Display * textMarkerArray_;
  rviz::Display * global_map_;
  rviz::Display * planner_plan_;
  rviz::Display * laser_scan_;
  rviz::Display * tf_;
  rviz::Display * global_path_;
  rviz::Display * local_path_;

public:


  QTime g_time;
  QTimer *m_pTimer;
  pthread_t m_p_thread;
  int m_nthr_id;
  bool m_bln_Thread_flag;

  bool m_blnRosConnectFlag;

  QwtPlotCurve *qwtPlotCurve_Data1;
  QwtPlotCurve *qwtPlotCurve_Data2;

};

#endif // MAINWINDOW_H
