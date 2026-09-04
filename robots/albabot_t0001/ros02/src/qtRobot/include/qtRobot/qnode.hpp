/**
 * @file /include/qthello/qnode.hpp
 *
 * @brief Communications central!
 *
 * @date Nov. 2020
 **/
/*****************************************************************************
** Ifdefs
*****************************************************************************/

#ifndef qtRobot_QNODE_HPP_
#define qtRobot_QNODE_HPP_

/*****************************************************************************
** Includes
*****************************************************************************/

// To workaround boost/qt4 problems that won't be bugfixed. Refer to
//    https://bugreports.qt.io/browse/QTBUG-22829
#ifndef Q_MOC_RUN
#include <ros/ros.h>
#include "albabot_msgs/RobotInfo.h"

#endif
#include <string>
#include <QThread>
#include <QStringListModel>


#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <cv_bridge/cv_bridge.h>


/*****************************************************************************
** Namespaces
*****************************************************************************/

namespace qtRobot {

/*****************************************************************************
** Class
*****************************************************************************/

class QNode : public QThread {
    Q_OBJECT
public:
	QNode(int argc, char** argv );
	virtual ~QNode();
	bool init();
	bool init(const std::string &master_url, const std::string &host_url);
	void run();

  void CameraCB(const sensor_msgs::ImageConstPtr& _image);
  void ThermalCB(const sensor_msgs::ImageConstPtr& _image);
  void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value);

  void HeadCameraCB(const sensor_msgs::ImageConstPtr& _image);
  void ManagerCameraReCB(const sensor_msgs::ImageConstPtr& _image);
  void ThermalCameraCB(const sensor_msgs::ImageConstPtr& _image);



  cv_bridge::CvImagePtr cv_ptrManagerRe;
  cv_bridge::CvImagePtr cv_ptrThermal;
  cv_bridge::CvImagePtr cv_ptrHead;


	/*********************
	** Logging
	**********************/
	enum LogLevel {
	         Debug,
	         Info,
	         Warn,
	         Error,
	         Fatal
	 };

	QStringListModel* loggingModel() { return &logging_model; }
	void log( const LogLevel &level, const std::string &msg);

Q_SIGNALS:
	void loggingUpdated();
    void rosShutdown();


    void emitUpdateManagerReImage();
    void emitUpdateHeadImage();
    void emitUpdateThermalImage();


private:
	int init_argc;
	char** init_argv;
	ros::Publisher chatter_publisher;
    QStringListModel logging_model;


  ros::Subscriber m_subThermalCamera;
  ros::Subscriber m_subHeadCamera;
  ros::Subscriber m_subManagerReCamera;

  ros::Subscriber m_subRobotInfo;



};

}  // namespace qthello

#endif /* qtRobot_QNODE_HPP_ */
