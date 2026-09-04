#include <ros/ros.h>
#include <sensor_msgs/Joy.h>
#include <geometry_msgs/Twist.h>

double g_joy_scale, g_joy_v, g_joy_w;
int g_TimeoutCnt=0;

geometry_msgs::Twist cmdvel_;
ros::Publisher pub_;


// callbac : joystick
void JoyCB(const sensor_msgs::Joy::ConstPtr& _joy)
{
  // kiro joystick
  g_joy_scale=((1+_joy->axes[2])/2.0) * 3.3;
  g_joy_v = 1.0*g_joy_scale*_joy->axes[1];
  g_joy_w = 1.0*g_joy_scale*_joy->axes[3];

  cmdvel_.linear.x = g_joy_v;
  cmdvel_.angular.z = g_joy_w;
  pub_.publish(cmdvel_);
}


int main(int argc, char **argv)
{
  ros::init(argc, argv, "joystick");
  ros::NodeHandle nh;

  ros::Subscriber joy_sub=nh.subscribe("joy", 1000, JoyCB);
  pub_ = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1);
  ROS_INFO("Hello world!");

  ros::spin();

  return 1;
}
