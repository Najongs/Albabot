#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <std_msgs/Float32MultiArray.h>

using namespace std;

ros::Publisher obstacle_pub;

float g_fMinDistance=1000.0;
void ScanCB(const sensor_msgs::LaserScan::ConstPtr& scan)
{
  double phi = scan->angle_min - scan->angle_increment;

  /*double r = scan->ranges[90];

  phi=scan->angle_increment*90;
  ROS_INFO("data : %f,%f => (%f,%f)",r, phi, cos(phi)*r,sin(phi)*r);
*/

  double dbMin=1000.0;

  for(int i=30;i<150;i++)
  {
    phi=scan->angle_increment*i;
    double r=scan->ranges[i];
    double yPos=sin(phi)*r;
    double xPos=cos(phi)*r;

    if(fabs(xPos)<0.2 && fabs(yPos)<2.0)
    {
      if(yPos<dbMin)
        dbMin=yPos;
      //ROS_INFO("data[%d] : %f,%f => (%f,%f)",i,r, phi, xPos,yPos);
    }
  }


  if(dbMin>2.0)
    dbMin=2.0;
  else
    dbMin*=1.0;

  g_fMinDistance=dbMin;

  ROS_INFO("min distance : %f",dbMin);

  //ROS_INFO("data : %f", value->ranges[0]);
}


int main(int argc, char **argv)
{
  ros::init(argc, argv, "albabot_lidar");
  ros::NodeHandle nh;

  obstacle_pub = nh.advertise<std_msgs::Float32MultiArray>("/obstacle",1000);
  ros::Subscriber scan_pub_=nh.subscribe("/scan",1000, ScanCB);
  ros::Rate loop_rate(10);


  while(ros::ok())
  {
    std_msgs::Float32MultiArray val;
    val.data.push_back(g_fMinDistance);
    obstacle_pub.publish(val);



    ros::spinOnce();
    loop_rate.sleep();
  }


  ROS_INFO("Hello world!");
}
