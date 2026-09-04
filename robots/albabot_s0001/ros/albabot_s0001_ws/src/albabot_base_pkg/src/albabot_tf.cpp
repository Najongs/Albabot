#include "ros/ros.h"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_ros/transform_broadcaster.h"
#include "geometry_msgs/TransformStamped.h"
#include "geometry_msgs/PoseStamped.h"

#include "tf/tf.h"
#include "tf/transform_broadcaster.h"

bool g_blnIsFirstIn=false;
double g_PosX=0, g_PosY=0, g_Theta=0;

geometry_msgs::Pose g_Pose;



tf::Transform transform;
tf::Quaternion q;
ros::Time timestamp;

int main(int argc, char **argv)
{
  ros::init(argc, argv, "albabot_tf");
  ros::NodeHandle nh;

  tf2_ros::TransformBroadcaster tf_br;

  ros::Rate loop_rate(10);
  // loop
  while( ros::ok() )
  {
    timestamp=ros::Time::now();
    static tf::TransformBroadcaster br;

    transform.setOrigin(tf::Vector3(0,0,0.1));
    q.setRPY(0,0.0,0.0);
    transform.setRotation(q);
    br.sendTransform(tf::StampedTransform(transform, timestamp, "base_footprint", "base_link"));

    transform.setOrigin(tf::Vector3(0.208,0.0,0.075));
    q.setRPY(0,0.0,90.0*(3.141592/180.0));
    transform.setRotation(q);
    //static tf::TransformBroadcaster br2;
    br.sendTransform(tf::StampedTransform(transform, timestamp, "base_link", "base_scan"));

    loop_rate.sleep();

  }

  return 0;
}
