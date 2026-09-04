#include <ros/ros.h>

void ScanCB(const sensor_msgs::LaserScan::ConstPtr& scan)
{
  double phi = scan->angle_min - scan->angle_increment;

  double r=scan->ran

  double dbMin=1000.0;

  //for(int i=30;i<150;i++)
  for(int i=180;i<360;i++)  // 라이다 180도 회전되어 설치됨.
  {
    phi=scan->angle_increment*i;
    double r=scan->ranges[i];
    double yPos=sin(phi)*r;
    double xPos=cos(phi)*r;

    if(fabs(xPos)<0.3 && fabs(yPos)<2.0)
    {
      if(yPos<dbMin)
        dbMin=yPos;
      //ROS_INFO("data[%d] : %f,%f => (%f,%f)",i,r, phi, xPos,yPos);
    }
  }

}


int main(int argc, char **argv)
{
  ros::init(argc, argv, "lidar_obstacle");
  ros::NodeHandle nh;

  ros::Subscriber scan_pub_=nh.subscribe("/scan",1000, ScanCB);

  ROS_INFO("Hello world!");
}
