/**
 * @file /include/agv_simple_test/qnode.hpp
 *
 * @brief Communications central!
 *
 * @date February 2011
 **/
/*****************************************************************************
** Ifdefs
*****************************************************************************/

#ifndef QNODE_H
#define QNODE_H

/*****************************************************************************
** Includes
*****************************************************************************/

// To workaround boost/qt4 problems that won't be bugfixed. Refer to
//    https://bugreports.qt.io/browse/QTBUG-22829
#ifndef Q_MOC_RUN
#include <ros/ros.h>
#include <ros/network.h>
#include <std_msgs/String.h>

// Add... Compressed image
#include "image_transport/subscriber_plugin.h"
#include "image_transport/simple_subscriber_plugin.h"
#include <sensor_msgs/CompressedImage.h>
#include <dynamic_reconfigure/server.h>
#include <compressed_image_transport/CompressedSubscriberConfig.h>
//...

#include <image_transport/image_transport.h>
#include <compressed_image_transport/compressed_subscriber.h>
#include <compressed_image_transport/compression_common.h>

#include <sensor_msgs/image_encodings.h>
#include <geometry_msgs/Point.h>
#include <cv_bridge/cv_bridge.h>
//#include <cv_camera/capture.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <limits>
#include <vector>
#endif

#include <string>
#include <QThread>
#include <QStringListModel>


/*****************************************************************************
** Namespaces
*****************************************************************************/

namespace qtgui_user {

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

Q_SIGNALS:

    void rosShutdown();
    void sendImage(cv::Mat img);

public:
    typedef boost::function<void(const sensor_msgs::ImageConstPtr&)>Callback;
    //typedef boost::function<void(const sensor_msgs::CompressedImagePtr&)>Callback;

    cv::Mat srcImgShow, srcImgShow1;
    cv::Mat rstImgShow, rstImgShow1;
    int m_nPosX, m_nPosY;

private:
	int init_argc;
	char** init_argv;

  QStringListModel logging_model;

  image_transport::Subscriber sub_srcImage;
  image_transport::Subscriber sub_rstImage;

  void srcImageCallback(const sensor_msgs::ImageConstPtr& msg);
  void srcCompressedImageCallback(const sensor_msgs::CompressedImagePtr& msg);
};

}  // namespace Koins

#endif /* Koins_QNODE_HPP_ */
