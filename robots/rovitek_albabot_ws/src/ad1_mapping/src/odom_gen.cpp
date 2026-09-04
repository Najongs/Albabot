#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include <geometry_msgs/Quaternion.h>
#include <nav_msgs/Odometry.h>
#include <visualization_msgs/Marker.h>
#include <std_msgs/Byte.h>

#include "agv_define.h"
#include "albabot_msgs/RobotInfo.h"
#include "agvmapper.h"

#include <geometry_msgs/Pose2D.h>


ros::Publisher g_pubOdom;

ros::Time timestamp, last_time;
double dt=0;

void PublishOdom(ros::Time current_time, double dt, geometry_msgs::Pose2D pose);

AgvMapper *pAgvMapper;


int main(int argc, char **argv)
{
  ros::init(argc, argv, "odom_gen");
  ros::NodeHandle nh;

  pAgvMapper=new AgvMapper(&nh);

  ros::Subscriber robotInfo_sub_=nh.subscribe("/robot_info", 1000, &AgvMapper::RobotInfoCB, pAgvMapper);
  g_pubOdom = nh.advertise<nav_msgs::Odometry>("/alba_odom", 50);

  albabot_msgs::RobotInfoConstPtr rxMsg = ros::topic::waitForMessage<albabot_msgs::RobotInfo>("/robot_info");

  geometry_msgs::Pose2D curPose;
  pAgvMapper->SetCurPose(0,0,3,rxMsg->agvDirection);


  ros::Rate loop_rate(10);
  timestamp=ros::Time::now();
  last_time=timestamp;

  while(ros::ok())
  {
    timestamp=ros::Time::now();
    curPose=pAgvMapper->GetPose2D();
    PublishOdom(timestamp,dt,curPose);
    ros::spinOnce();
    loop_rate.sleep();
  }

  delete pAgvMapper;

  return 1;
}

void PublishOdom(ros::Time current_time, double dt, geometry_msgs::Pose2D pose)
{
  static tf::TransformBroadcaster g_odomBr;
  //since all odometry is 6DOF we'll need a quaternion created from yaw
  geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw(pose.theta);

  //first, we'll publish the transform over tf
  geometry_msgs::TransformStamped odom_trans;
  odom_trans.header.stamp = current_time;
  odom_trans.header.frame_id = "odom";
  odom_trans.child_frame_id = "base_link";

  odom_trans.transform.translation.x = pose.x;
  odom_trans.transform.translation.y = pose.y;
  odom_trans.transform.translation.z = 0.0;
  odom_trans.transform.rotation = odom_quat;

  //tf nav_msgsROS_INFO("pulish odom");
  //send the transform
  g_odomBr.sendTransform(odom_trans);

  //next, we'll publish the odometry message over ROS
  nav_msgs::Odometry odom;
  odom.header.stamp = current_time;
  odom.header.frame_id = "odom";

  //set the position
  odom.pose.pose.position.x = pose.x;
  odom.pose.pose.position.y = pose.y;
  odom.pose.pose.position.z = 0.0;
  odom.pose.pose.orientation = odom_quat;

  //set the velocity
  odom.child_frame_id = "base_link";
  odom.twist.twist.linear.x = pose.x/dt;
  odom.twist.twist.linear.y = pose.y/dt;
  odom.twist.twist.angular.z = pose.theta/dt;//((g_fAngle/180) * 3.14159)/dt;

  //publish the message
  g_pubOdom.publish(odom);
}
