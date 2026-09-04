#include <ros/ros.h>
#include <sensor_msgs/Joy.h>
#include <geometry_msgs/Twist.h>

ros::Publisher g_pubVel;

uint32_t g_u32TimeoutCnt;
double g_vel_v, g_vel_w;
void JoyCB(const sensor_msgs::Joy::ConstPtr& _joy)
{
    g_vel_v=_joy->axes[1];
    g_vel_w=_joy->axes[3];

    //g_u32TimeoutCnt=0;
}



int main(int argc, char **argv)
{
  ros::init(argc, argv, "joy2vel");
  ros::NodeHandle nh;

  ros::Subscriber subJoy=nh.subscribe("/joy1",1000,JoyCB);
  g_pubVel = nh.advertise<geometry_msgs::Twist>("/cmd_vel",1000);

  ros::Rate loop_rate(10);

  while(ros::ok())
  {
    geometry_msgs::Twist msg;

    g_u32TimeoutCnt=0;
    if(g_u32TimeoutCnt<10)
    {
      msg.linear.x=g_vel_v;
      msg.angular.z=g_vel_w;
      g_u32TimeoutCnt++;
    }
    else
    {
      msg.linear.x=0;
      msg.angular.z=0;
    }

    g_pubVel.publish(msg);

    loop_rate.sleep();
    ros::spinOnce();
  }
}
