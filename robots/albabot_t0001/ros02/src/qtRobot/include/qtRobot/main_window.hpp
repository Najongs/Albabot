/**
 * @file /include/qthello/main_window.hpp
 *
 * @brief Qt based gui for qthello.
 *
 * @date November 2010
 **/
#ifndef qtRobot_MAIN_WINDOW_H
#define qtRobot_MAIN_WINDOW_H

/*****************************************************************************
** Includes
*****************************************************************************/

#include <QMainWindow>
#include "ui_main_window.h"
#include "qnode.hpp"
#include <QProcess>
/*****************************************************************************
** Namespace
*****************************************************************************/

namespace qtRobot {

class MainWindow : public QMainWindow {
Q_OBJECT

public:
	MainWindow(int argc, char** argv, QWidget *parent = 0);
	~MainWindow();

	void ReadSettings(); // Load up qt program settings at startup
	void WriteSettings(); // Save qt program settings when closing

	void closeEvent(QCloseEvent *event); // Overloaded function
	void showNoMasterMessage();

  bool eventFilter(QObject *watched, QEvent *event);

  QImage m_imgDisplay;

  QProcess *m_procVideo;
  QProcess *m_procAudio;

public Q_SLOTS:
    void updateLoggingView(); // no idea why this can't connect automatically

    void updateManagerReImageView();
    void updateThermalImageView();
    void updateHeadImageView();

private slots:
    void on_pushButton_call_agent_clicked();

    void on_pushButton_2_clicked();

private:
	Ui::MainWindowDesign ui;
	QNode qnode;
};

}  // namespace qtRobot

#endif // qtRobot_MAIN_WINDOW_H

