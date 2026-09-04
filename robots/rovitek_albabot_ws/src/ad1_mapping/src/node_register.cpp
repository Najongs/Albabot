#include <ros/ros.h>
#include <std_msgs/ByteMultiArray.h>
#include "albabot_msgs/CanMsg.h"

typedef unsigned char   u8;

u8 g_u8NodeStart;
u8 g_u8NodeEnd;

ros::Publisher g_pubCommand;

void NodepairCB(const std_msgs::ByteMultiArray::ConstPtr& msg);

void NodepairCB(const std_msgs::ByteMultiArray::ConstPtr& msg)
{
  if(msg->data.size()==2)
  {
    g_u8NodeStart=msg->data[0];
    g_u8NodeEnd=msg->data[1];

    ROS_INFO("Node%d -> Node%d",g_u8NodeStart,g_u8NodeEnd);
    // Next 명령
    albabot_msgs::CanMsg canmsg;
    canmsg.id=0x00;
    canmsg.dlc=2;
    canmsg.data.push_back(0x00);
    canmsg.data.push_back(0x01);
    g_pubCommand.publish(canmsg);
  }
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "node_register");
  ros::NodeHandle nh;

  ros::Subscriber sub_NodePair=nh.subscribe("/node_pair", 1000, NodepairCB);
  g_pubCommand = nh.advertise<albabot_msgs::CanMsg>("/AlbabotMessage",1000);
  ros::spin();
}
