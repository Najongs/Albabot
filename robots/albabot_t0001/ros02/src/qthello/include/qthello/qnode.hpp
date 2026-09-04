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

#ifndef qthello_QNODE_HPP_
#define qthello_QNODE_HPP_

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

namespace qthello {

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

  cv::Mat m_srcImage;
  cv_bridge::CvImagePtr cv_ptr;
  cv_bridge::CvImagePtr cv_Thermalptr;

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
    void viewUpdate();
    void viewThermalUpdate();

private:
	int init_argc;
	char** init_argv;
	ros::Publisher chatter_publisher;
    QStringListModel logging_model;

  ros::Subscriber m_subCamera;
  ros::Subscriber m_subThermalCamera;
  ros::Subscriber m_subRobotInfo;

};

}  // namespace qthello

#endif /* qthello_QNODE_HPP_ */
