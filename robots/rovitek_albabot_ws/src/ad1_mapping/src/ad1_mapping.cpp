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

#include <visualization_msgs/MarkerArray.h>
visualization_msgs::Marker GenMark(uint8_t id, double x, double y);
visualization_msgs::Marker GenMarkText(uint8_t id, double x, double y);
visualization_msgs::MarkerArray markArr;

ros::Publisher g_pubOdom;
ros::Publisher g_pubMarker;

ros::Time timestamp, last_time;
double dt=0;

void PublishOdom(ros::Time current_time, double dt, geometry_msgs::Pose2D pose);
void PublishMarker(ros::Time current_time);

AgvMapper *pAgvMapper;

uint8_t g_u8SetFlag=0;
void PoseSetterCB(const std_msgs::Byte::ConstPtr &msg)
{
  uint8_t tempData=0;
  tempData=msg->data;

  ROS_INFO("byte:%d",msg->data);

  switch(tempData)
  {
    case 0:
      pAgvMapper->AddNode();
      g_u8SetFlag=1;

      //GenMark(0,pAgvMapper->GetPose2D().x,pAgvMapper->GetPose2D().y);
      markArr.markers.push_back(GenMark(pAgvMapper->GetCurID(),pAgvMapper->GetPose2D().x,pAgvMapper->GetPose2D().y));
      markArr.markers.push_back(GenMarkText(pAgvMapper->GetCurID(),pAgvMapper->GetPose2D().x,pAgvMapper->GetPose2D().y));

      break;
    case 1:
      pAgvMapper->DelNode();
      break;
    case 2:
      pAgvMapper->SaveMap("map");
      //ROS_INFO("%lf, %lf, %lf",g_dbPosX,g_dbPosY,g_dbAngle);
      break;
    case 3:
      pAgvMapper->ClearNode();
      //ROS_INFO("Node Num : %d",pAgvMapper->m_listCalData.size());
      break;
    default:
      break;
  }
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "ad1_mapping");
  ros::NodeHandle nh;

  pAgvMapper=new AgvMapper(&nh);

  ros::Subscriber robotInfo_sub_=nh.subscribe("/robot_info", 1000, &AgvMapper::RobotInfoCB, pAgvMapper);
  ros::Subscriber subSet=nh.subscribe("/setter",1000, PoseSetterCB);

  g_pubOdom = nh.advertise<nav_msgs::Odometry>("/alba_odom", 50);
  g_pubMarker = nh.advertise<visualization_msgs::MarkerArray>("/markers",100);


  albabot_msgs::RobotInfoConstPtr rxMsg = ros::topic::waitForMessage<albabot_msgs::RobotInfo>("/robot_info");

  //printf("curDir = %d\n", rxMsg->agvDirection);

  geometry_msgs::Pose2D curPose;
  pAgvMapper->SetCurPose(0,0,3,rxMsg->agvDirection);
  pAgvMapper->LoadMap();

  std::list<MAP_INFO>::iterator iter;
  for(iter=pAgvMapper->m_listNodeData.begin();iter!=pAgvMapper->m_listNodeData.end();++iter)
  {
    markArr.markers.push_back(GenMark(iter->id,iter->x,iter->y));
    markArr.markers.push_back(GenMarkText(iter->id,iter->x,iter->y));
  }


  ros::Rate loop_rate(10);
  timestamp=ros::Time::now();
  last_time=timestamp;

  //PublishMarker(timestamp);

  dt=0;
  while(ros::ok())
  {
    timestamp=ros::Time::now();
    dt += timestamp.toSec()-last_time.toSec();
    last_time = timestamp;

    curPose=pAgvMapper->GetPose2D();
    //curPose.theta+=3.141592/2;
    PublishOdom(timestamp,dt,curPose);

    //if(g_u8SetFlag==1)
    //{
      //g_u8SetFlag=0;
      PublishMarker(timestamp);
    //}

    ROS_INFO("Num:%d,%.3lf,%.3lf",pAgvMapper->m_listNodeData.size(),curPose.x,curPose.y);
/*
    ///printf("----------Num : %d ----------\n", pAgvMapper->m_listCalData.size());
    double x,y;
    int i=0;
    //ROS_INFO("Num:%d",pAgvMapper->m_listCalData.size());

    std::list<std::pair<IndexPair,CalPair>>::iterator iter;
    for(iter=pAgvMapper->m_listCalData.begin();iter!=pAgvMapper->m_listCalData.end();++iter)
    {
      x=iter->second.first;
      y=iter->second.second;
      //printf("node %d : %.3lf, %.3lf\n",++i,x,y);

    }

*/
    //PublishMarker(timestamp);

    ros::spinOnce();
    loop_rate.sleep();
  }

  delete pAgvMapper;

  return 1;
}


visualization_msgs::Marker GenMarkText(uint8_t id, double x, double y)
{
  visualization_msgs::Marker text;
  text.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
  text.action = visualization_msgs::Marker::ADD;
  text.header.frame_id = "/odom";
  text.color.r = text.color.g = text.color.b = text.color.a = 1.0f;
  text.id = id+1000;
  text.scale.x = text.scale.y = text.scale.z = 0.1;
  text.pose.position.x = x+0.2;
  text.pose.position.y = y+0.2;
  text.pose.position.z = 0.1;
  text.pose.orientation.x=0;
  text.pose.orientation.y=0.707;
  text.pose.orientation.z=0;
  text.pose.orientation.w=0.707;
  text.ns = "text";
  text.text = std::to_string(id);

  return text;
}

visualization_msgs::Marker GenMark(uint8_t id, double x, double y)
{
  visualization_msgs::Marker marker;
  marker.header.frame_id="/odom";
  marker.header.stamp=ros::Time::now();
  marker.id=pAgvMapper->GetCurID();
  marker.ns="basic_shapes";
  marker.type=visualization_msgs::Marker::CUBE;
  marker.action = visualization_msgs::Marker::ADD;

  marker.pose.position.x = x;
  marker.pose.position.y = y;
  marker.pose.position.z = 0;
  marker.pose.orientation.x = 0.0;
  marker.pose.orientation.y = 0.0;
  marker.pose.orientation.z = 0.0;
  marker.pose.orientation.w = 1.0;

  marker.scale.x = 0.05;
  marker.scale.y = 0.05;
  marker.scale.z = 0.01;
  marker.color.r = 0.0f;
  marker.color.g = 1.0f;
  marker.color.b = 0.0f;
  marker.color.a = 1.0;


  return marker;
}


void PublishMarkers()
{
  /*visualization_msgs::MarkerArray markArr;


  markArr.markers.push_back(mark);*/

}


void PublishMarker(ros::Time current_time)
{
  /*geometry_msgs::Pose2D curPose;
  curPose=pAgvMapper->GetPose2D();

  visualization_msgs::Marker marker;
  marker.header.frame_id="/odom";
  marker.header.stamp=current_time;
  marker.id=0;
  marker.ns="basic_shapes";
  marker.type=visualization_msgs::Marker::CUBE;
  marker.action = visualization_msgs::Marker::ADD;

  marker.pose.position.x = curPose.x;
  marker.pose.position.y = curPose.y;
  marker.pose.position.z = 0;
  marker.pose.orientation.x = 0.0;
  marker.pose.orientation.y = 0.0;
  marker.pose.orientation.z = 0.0;
  marker.pose.orientation.w = 1.0;

  marker.scale.x = 0.2;
  marker.scale.y = 0.2;
  marker.scale.z = 0.01;
  marker.color.r = 0.0f;
  marker.color.g = 1.0f;
  marker.color.b = 0.0f;
  marker.color.a = 1.0;*/



  g_pubMarker.publish(markArr);
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
