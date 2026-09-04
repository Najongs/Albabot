/**
 * @file /include/qthello/main_window.hpp
 *
 * @brief Qt based gui for qthello.
 *
 * @date November 2010
 **/
#ifndef qthello_MAIN_WINDOW_H
#define qthello_MAIN_WINDOW_H

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

namespace qthello {

class MainWindow : public QMainWindow {
Q_OBJECT

public:
	MainWindow(int argc, char** argv, QWidget *parent = 0);
	~MainWindow();

	void ReadSettings(); // Load up qt program settings at startup
	void WriteSettings(); // Save qt program settings when closing

	void closeEvent(QCloseEvent *event); // Overloaded function
	void showNoMasterMessage();

  QProcess *m_proc;

public Q_SLOTS:
  void updateLoggingView(); // no idea why this can't connect automatically

      void updateImageView();
      void updateThermalImageView();

private slots:
    //void on_pushButton_clicked();

      //void on_pushButton_clicked();

private:
	Ui::MainWindowDesign ui;
	QNode qnode;
};

}  // namespace qthello

#endif // qthello_MAIN_WINDOW_H
