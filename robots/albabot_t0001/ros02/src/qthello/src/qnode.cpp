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
#include "../include/qthello/qnode.hpp"

/*****************************************************************************
** Namespaces
*****************************************************************************/

namespace qthello {

/*****************************************************************************
** Implementation
*****************************************************************************/

QNode::QNode(int argc, char** argv ) :
	init_argc(argc),
	init_argv(argv)
	{}

QNode::~QNode() {
    if(ros::isStarted()) {
      ros::shutdown(); // explicitly needed since we use ros::start();
      ros::waitForShutdown();
    }
	wait();
}

bool QNode::init() {
    ros::init(init_argc,init_argv,"qthello");
	if ( ! ros::master::check() ) {
		return false;
	}
	ros::start(); // explicitly needed since our nodehandle is going out of scope.
	ros::NodeHandle n;
	// Add your ros communications here.
	chatter_publisher = n.advertise<std_msgs::String>("chatter", 1000);
	start();
	return true;
}

void QNode::ThermalCB(const sensor_msgs::ImageConstPtr& _image)
{
  ROS_INFO("got image!");
  try
  {
    cv_Thermalptr = cv_bridge::toCvCopy(_image, sensor_msgs::image_encodings::RGB8);
    //m_srcImage=cv_bridge::toCvCopy(_image, sensor_msgs::image_encodings::BGR8);
  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("cv_bridge exception: %s", e.what());
    return;
  }

  Q_EMIT viewThermalUpdate();
}

void QNode::CameraCB(const sensor_msgs::ImageConstPtr& _image)
{
  ROS_INFO("got image!");
  try
  {
    cv_ptr = cv_bridge::toCvCopy(_image, sensor_msgs::image_encodings::RGB8);
    //m_srcImage=cv_bridge::toCvCopy(_image, sensor_msgs::image_encodings::BGR8);
  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("cv_bridge exception: %s", e.what());
    return;
  }

  Q_EMIT viewUpdate();
}


void QNode::RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value)
{
  uint16_t g_hallData;
  uint16_t g_pSonicData[8];

  g_hallData=value->hall[0];
  for(int i=0;i<8;i++)
    g_pSonicData[i]=value->sonic[i];

  ROS_INFO("%d, %d, %.4x, %d, %d, %d, %d, %d, %d, %d, %d",value->left_enc, value->right_enc,
              g_hallData, g_pSonicData[0], g_pSonicData[1], g_pSonicData[2], g_pSonicData[3],
              g_pSonicData[4], g_pSonicData[5], g_pSonicData[6], g_pSonicData[7]);


}

bool QNode::init(const std::string &master_url, const std::string &host_url) {
	std::map<std::string,std::string> remappings;
	remappings["__master"] = master_url;
	remappings["__hostname"] = host_url;
    ros::init(remappings,"qthello");
	if ( ! ros::master::check() ) {
		return false;
	}
	ros::start(); // explicitly needed since our nodehandle is going out of scope.
	ros::NodeHandle n;
	// Add your ros communications here.
	chatter_publisher = n.advertise<std_msgs::String>("chatter", 1000);

  m_subThermalCamera=n.subscribe("/usb_cam/image_raw",60,&QNode::ThermalCB,this);
  m_subCamera=n.subscribe("/camera/color/image_raw",60,&QNode::CameraCB,this);
  m_subRobotInfo=n.subscribe("/robot_info",100,&QNode::RobotInfoCB,this);




	start();
	return true;
}

void QNode::run() {
	ros::Rate loop_rate(1);
	int count = 0;

  ros::spin();
  /*while ( ros::ok() ) {

    std_msgs::String msg;
		std::stringstream ss;
		ss << "hello world " << count;
		msg.data = ss.str();
		chatter_publisher.publish(msg);
		log(Info,std::string("I sent: ")+msg.data);
		ros::spinOnce();
		loop_rate.sleep();
		++count;
  }*/
	std::cout << "Ros shutdown, proceeding to close the gui." << std::endl;
	Q_EMIT rosShutdown(); // used to signal the gui for a shutdown (useful to roslaunch)
}


void QNode::log( const LogLevel &level, const std::string &msg) {
	logging_model.insertRows(logging_model.rowCount(),1);
	std::stringstream logging_model_msg;
	switch ( level ) {
		case(Debug) : {
				ROS_DEBUG_STREAM(msg);
				logging_model_msg << "[DEBUG] [" << ros::Time::now() << "]: " << msg;
				break;
		}
		case(Info) : {
				ROS_INFO_STREAM(msg);
				logging_model_msg << "[INFO] [" << ros::Time::now() << "]: " << msg;
				break;
		}
		case(Warn) : {
				ROS_WARN_STREAM(msg);
				logging_model_msg << "[INFO] [" << ros::Time::now() << "]: " << msg;
				break;
		}
		case(Error) : {
				ROS_ERROR_STREAM(msg);
				logging_model_msg << "[ERROR] [" << ros::Time::now() << "]: " << msg;
				break;
		}
		case(Fatal) : {
				ROS_FATAL_STREAM(msg);
				logging_model_msg << "[FATAL] [" << ros::Time::now() << "]: " << msg;
				break;
		}
	}
	QVariant new_row(QString(logging_model_msg.str().c_str()));
	logging_model.setData(logging_model.index(logging_model.rowCount()-1),new_row);
	Q_EMIT loggingUpdated(); // used to readjust the scrollbar
}

}  // namespace qtros
