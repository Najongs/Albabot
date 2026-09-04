/**
 * @file /src/qnode.cpp
 *
 * @brief Ros communication central!
 *
 * @date February 2011
 **/

/*****************************************************************************
** Includes
*****************************************************************************/

#include <ros/ros.h>
#include <ros/network.h>
#include <string>
#include <std_msgs/String.h>
#include <sstream>
#include "qnode.h"

/*****************************************************************************
** Namespaces
*****************************************************************************/
namespace enc = sensor_msgs::image_encodings;

namespace qtgui_user {

/*****************************************************************************
** Implementation
*****************************************************************************/

QNode::QNode(int argc, char** argv ) :
	init_argc(argc),
  init_argv(argv)
{

}

QNode::~QNode()
{
  if(ros::isStarted())
  {
    ros::shutdown(); // explicitly needed since we use ros::start();
    ros::waitForShutdown();
  }
	wait();
}

bool QNode::init()
{
  ros::init(init_argc,init_argv,"Koins");

  if ( ! ros::master::check() )
  {
		return false;
	}

  ros::start(); // explicitly needed since our nodehandle is going out of scope.
	ros::NodeHandle n;

  image_transport::ImageTransport it(n);  
  //sub_srcImage = it.subscribe("/camera/image_color", 20, &QNode::srcImageCallback, this);
  sub_srcImage = it.subscribe("/camera/image_raw", 1000, &QNode::srcImageCallback, this);

  //sub_srcImage = it.subscribe("/camera/image_color/compressed", 20, boost::bind(&QNode::srcCompressedImageCallback,_1,1),
  //                            ros::VoidPtr(),image_transport::TransportHints("compressed"));

  //ros::Subscriber sub = n.subscribe("/camera/image_color/compressed", 20, &QNode::srcCompressedImageCallback, this);
  //ros::Subscriber sub = n.subscribe("/camera/image_raw/theora", 1, &QNode::srcCompressedImageCallback, this);


//  sub_srcImage = it.subscribe(
//      "/camera/image_color/compressed",
//      1,
//      boost::bind(&QNode::srcCompressedImageCallback, _1, 1),
//      ros::VoidPtr(),                       // THIS ADDED
//      image_transport::TransportHints::TransportHints("compressed")
//  );


  //sub_srcImage = it.subscribe("/usb_cam/image_raw", 20, &QNode::srcImageCallback, this);
  //sub_srcImage = it.subscribe("/usb_cam/image_raw/compressed", 20, &QNode::srcImageCallback, this);
  //ros::Subscriber sub = n.subscribe("/usb_cam/image_raw/compressed", 20, &QNode::srcCompressedImageCallback, this);

  //ros::spin();
  start();

	return true;
}

bool QNode::init(const std::string &master_url, const std::string &host_url)
{
  std::map<std::string,std::string> remappings;

  remappings["__master"] = master_url;
  remappings["__hostname"] = host_url;

  ros::init(remappings,"Koins");

  if ( ! ros::master::check() )
  {
    return false;
  }

  ros::start(); // explicitly needed since our nodehandle is going out of scope.
  ros::NodeHandle n;

  image_transport::ImageTransport it(n);
  //sub_srcImage = it.subscribe("/camera/image_color", 20, &QNode::srcImageCallback, this);
  sub_srcImage = it.subscribe("/camera/image_raw", 1000, &QNode::srcImageCallback, this);

  //sub_srcImage = it.subscribe("/usb_cam/image_raw", 20, &QNode::srcImageCallback,this);

  //sub_srcImage = it.subscribe("/usb_cam/image_raw/compressed", 10, &QNode::srcCompressedImageCallback,this);
  //ros::Subscriber sub = n.subscribe("/usb_cam/image_raw/compressed", 10, &QNode::srcCompressedImageCallback,this);

  start();
  return true;
}

void QNode::run()
{
  ros::Rate loop_rate(10);
  //int count = 0;
  //std_msgs::String msg;
  //std::stringstream ss;

  while ( ros::ok() )
  {
    //ss << "hello world " << count;
    //msg.data = ss.str();

    //chatter_publisher.publish(msg);
    //log(Info, std::string("I sent: ")+msg.data);

    //loop_rate.sleep();

    ros::spinOnce();
    //ROS_INFO("run");
    //++count;
  }

  std::cout << "Ros shutdown, proceeding to close the gui." << std::endl;
  Q_EMIT rosShutdown(); // used to signal the gui for a shutdown (useful to roslaunch)
}

void QNode::srcCompressedImageCallback(const sensor_msgs::CompressedImagePtr& msg)
{
  //srcImgShow = cv::imdecode(cv::Mat(msg->data),1);
  //srcImgShow = cv::imdecode(cv::Mat(msg->data),CV_LOAD_IMAGE_COLOR);
  //srcImgShow.copyTo(srcImgShow1);
  //cv::imshow("view", srcImgShow);
  //cv::waitKey(10);

  cv_bridge::CvImagePtr cvImg = cv_bridge::toCvCopy(msg, "rgb8");
  srcImgShow = cvImg->image;

  ROS_INFO("srcCompressedImageCallback");
  emit sendImage(srcImgShow);
}

void QNode::srcImageCallback(const sensor_msgs::ImageConstPtr& msg)
{
//=============================== source image1 (compressed test) ============================================//

//  cv_bridge::CvImagePtr cvImg = cv_bridge::toCvCopy(msg, "rgb8");
//  srcImgShow = cvImg->image;
//  srcImgShow.copyTo(srcImgShow1);
//  ROS_INFO("source image1 (compressed test)");
//  emit sendImage(srcImgShow1);

//=============================== source image2 (compressed test) ============================================//
  srcImgShow =cv_bridge::toCvShare(msg, "rgb8")->image;
  //srcImgShow.copyTo(srcImgShow1);
  ROS_INFO("source image2 (srcImageCallback test)");
 // cv::imshow("view", srcImgShow);
 //cv::waitKey(10);
  emit sendImage(srcImgShow);
//========================================= Binarization 1 ====================================================//
//  cv::Mat imgRGB[3];
//  cv::split(srcImgShow,imgRGB);
//  cv::threshold(imgRGB[0],imgRGB[0],100,255.0,cv::THRESH_BINARY);
//  cvtColor(imgRGB[0], srcImgShow1,CV_GRAY2RGB);

//========================================= Binarization 2 ====================================================//
//  cv::Mat proImg;
//  cv::Mat binImg;

//  std::vector <cv::Mat>mv;
//  cv::split(srcImgShow, mv);
//  proImg = mv[0];

//  cv::threshold(proImg, binImg, 100, 255.0, cv::THRESH_BINARY);
//  //cv::imshow("result image", binImg);                                                                   // ok
//  cvtColor(binImg, srcImgShow1,CV_GRAY2RGB);
}

}  // namespace Koins
