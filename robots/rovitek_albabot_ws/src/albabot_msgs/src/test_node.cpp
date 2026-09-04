#include <ros/ros.h>
#include "albabot_msgs/CanMsg.h"
#include "std_msgs/Int16.h"
#include "albabot_msgs/Command.h"


albabot_msgs::CanMsg g_msg;

bool testSrvServerCB(albabot_msgs::Command::Request &Req, albabot_msgs::Command::Response &Res)
{
  ROS_INFO("%d, %d", Req.can_msg.id, Req.can_msg.dlc);



  Res.result=2;

  //sleep(5);

  return true;
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "test_node");
  ros::NodeHandle nh;

  ros::Publisher test_pub = nh.advertise<albabot_msgs::CanMsg>("test_msgs",1000);
  ros::ServiceServer test_srvServer = nh.advertiseService("test_srvServer",testSrvServerCB);


  ROS_INFO("H1");

  g_msg.id=0x123;
  g_msg.dlc=8;
  for(int i=0;i<g_msg.dlc;i++)
  {
    g_msg.data.push_back(i);
  }
  ros::Rate loop_rate(10);
  while(ros::ok())
  {
    test_pub.publish(g_msg);
    ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;

}
