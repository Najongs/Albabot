#include <ros/ros.h>
#include <std_msgs/Byte.h>
#include <geometry_msgs/Pose2D.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include "agv_define.h"
#include "albabot_msgs/RobotInfo.h"
#include "agvmapper.h"

visualization_msgs::Marker GenMark(uint8_t id, double x, double y);
visualization_msgs::Marker GenMarkText(uint8_t id, double x, double y);
visualization_msgs::MarkerArray markArr;
ros::Publisher g_pubMarker;

ros::Time timestamp, last_time;
double dt=0;

void PublishMarker(ros::Time current_time);

AgvMapper *pAgvMapper;

void PoseSetterCB(const std_msgs::Byte::ConstPtr &msg)
{
  uint8_t tempData=0;
  tempData=msg->data;

  ROS_INFO("byte:%d",msg->data);

  switch(tempData)
  {
    case 0:
      pAgvMapper->AddNode();
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
  ros::init(argc, argv, "node_gen");
  ros::NodeHandle nh;

  pAgvMapper=new AgvMapper(&nh);

  ros::Subscriber robotInfo_sub_=nh.subscribe("/robot_info", 1000, &AgvMapper::RobotInfoCB, pAgvMapper);
  ros::Subscriber subSet=nh.subscribe("/setter",1000, PoseSetterCB);
  g_pubMarker = nh.advertise<visualization_msgs::MarkerArray>("/markers",100);

  //albabot_msgs::RobotInfoConstPtr rxMsg = ros::topic::waitForMessage<albabot_msgs::RobotInfo>("/robot_info");

  std::list<MAP_INFO>::iterator iter;
  for(iter=pAgvMapper->m_listNodeData.begin();iter!=pAgvMapper->m_listNodeData.end();++iter)
  {
    markArr.markers.push_back(GenMark(iter->id,iter->x,iter->y));
    markArr.markers.push_back(GenMarkText(iter->id,iter->x,iter->y));
  }

  ros::Rate loop_rate(10);

  while(ros::ok())
  {
    timestamp=ros::Time::now();
    PublishMarker(timestamp);
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
  text.pose.position.x = x+0.1;
  text.pose.position.y = y+0.1;
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

void PublishMarker(ros::Time current_time)
{
  g_pubMarker.publish(markArr);
}
