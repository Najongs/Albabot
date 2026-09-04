#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_listener.h>
#include <tf2_ros/static_transform_broadcaster.h>
#include <geometry_msgs/TransformStamped.h>
#include <tf2/LinearMath/Quaternion.h>


int main(int argc, char **argv)
{
  ros::init(argc, argv, "ad1_tf");
  ros::NodeHandle nh;

#if 0
  static tf2_ros::StaticTransformBroadcaster br1;
  static tf2_ros::StaticTransformBroadcaster br2;
  static tf2_ros::StaticTransformBroadcaster br3;
  static tf2_ros::StaticTransformBroadcaster br4;


  static geometry_msgs::TransformStamped tr1;
  static geometry_msgs::TransformStamped tr2;
  static geometry_msgs::TransformStamped tr3;
  static geometry_msgs::TransformStamped tr4;

  tf2::Quaternion quat;

  /*tr1.header.stamp = ros::Time::now();
  tr1.header.frame_id = "odom";
  tr1.child_frame_id = "base_footprint";
  tr1.transform.translation.x = 0;
  tr1.transform.translation.y = 0;
  tr1.transform.translation.z = 0.1;

  quat.setRPY(0.0, 0.0*(3.141592/180.0), 0.0*(3.141592/180.0));
  tr1.transform.rotation.x = quat.x();
  tr1.transform.rotation.y = quat.y();
  tr1.transform.rotation.z = quat.z();
  tr1.transform.rotation.w = quat.w();
  br1.sendTransform(tr1);*/


  tr2.header.stamp = ros::Time::now();
  tr2.header.frame_id = "base_footprint";
  tr2.child_frame_id = "base_link";
  tr2.transform.translation.x = 0;
  tr2.transform.translation.y = 0;
  tr2.transform.translation.z = 0.0;

  quat.setRPY(0.0, 0.0*(3.141592/180.0), 0.0*(3.141592/180.0));
  tr2.transform.rotation.x = quat.x();
  tr2.transform.rotation.y = quat.y();
  tr2.transform.rotation.z = quat.z();
  tr2.transform.rotation.w = quat.w();
  br2.sendTransform(tr2);

  tr3.header.stamp = ros::Time::now();
  tr3.header.frame_id = "/base_link";
  tr3.child_frame_id = "/base_scan";
  tr3.transform.translation.x = 0.208;
  tr3.transform.translation.y = 0;
  tr3.transform.translation.z = 0.075;

  quat.setRPY(0.0, 0.0*(3.141592/180.0), -90.0*(3.141592/180.0));
  tr3.transform.rotation.x = quat.x();
  tr3.transform.rotation.y = quat.y();
  tr3.transform.rotation.z = quat.z();
  tr3.transform.rotation.w = quat.w();
  br3.sendTransform(tr3);

#endif


  ros::Rate r(20);

  tf::TransformBroadcaster broadcaster;
  tf::Quaternion q;

  tf::TransformListener listener;
  while(nh.ok())
  {
    tf::StampedTransform transform;
    try{

      listener.lookupTransform("/base_footprint", "/odom",ros::Time(0), transform);
    }
    catch (tf::TransformException &ex) {
      ROS_ERROR("%s",ex.what());
      ros::Duration(1.0).sleep();
      continue;
    }

    broadcaster.sendTransform(
      tf::StampedTransform(
        tf::Transform(transform.getRotation(), transform.getOrigin()),
        ros::Time::now(),"base_footprint", "base_link"));


    q.setRPY(0.0, 0.0*(3.141592/180.0), -90.0*(3.141592/180.0));
    broadcaster.sendTransform(
      tf::StampedTransform(
        tf::Transform(q, tf::Vector3(0.0, 0.0, 0.0)),
        ros::Time::now(),"base_link", "usb_cam"));


    q.setRPY(0.0, 0.0*(3.141592/180.0), -90.0*(3.141592/180.0));
    broadcaster.sendTransform(
      tf::StampedTransform(
        tf::Transform(q, tf::Vector3(0.208, 0, 0.075)),
        ros::Time::now(),"base_link", "base_scan"));


    ros::spinOnce();
    r.sleep();
  }


  return 0;
}
