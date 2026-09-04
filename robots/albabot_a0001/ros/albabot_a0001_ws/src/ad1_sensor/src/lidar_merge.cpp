#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <string>

sensor_msgs::LaserScan g_scan_msg1;
sensor_msgs::LaserScan g_scan_msg2;
ros::Publisher g_pubScan;

bool g_scan1IsFlag=false;

void ScanCB1(const sensor_msgs::LaserScan::ConstPtr& scan)
{
    g_scan_msg1=*scan;
    g_scan1IsFlag=true;
}

void ScanCB2(const sensor_msgs::LaserScan::ConstPtr& scan)
{
    g_scan_msg2=*scan;

    if(g_scan1IsFlag==true)
    {
        sensor_msgs::LaserScan scan_msg;
        std::string frame_id="/scan";

        scan_msg.header.stamp = ros::Time::now();
        scan_msg.header.frame_id=frame_id;
        scan_msg.angle_min =-3.141592;
        scan_msg.angle_max = 3.141592;
        scan_msg.angle_increment = 1*(3.141592/180.0);
        scan_msg.range_min = 0.01;
        scan_msg.range_max = 5.0;
        int size = (scan_msg.angle_max - scan_msg.angle_min)/ scan_msg.angle_increment;

        scan_msg.ranges.resize(size);
        ROS_INFO("scan2: %d,%.8f,%.8f",size,scan_msg.header.stamp.sec, scan_msg.header.stamp.nsec);

        //-180~0
        for(int i=0; i < size/2; i++)
        {
            scan_msg.ranges[i] = g_scan_msg1.ranges[i];
        }

        //0~180
        for(int i=size/2; i < size; i++)
        {
            scan_msg.ranges[i] = scan->ranges[i-size/2];
        }
        g_pubScan.publish(scan_msg);

        g_scan1IsFlag=false;
    }
}



int main(int argc, char **argv)
{
    ros::init(argc, argv, "lidar_merge");
    ros::NodeHandle nh;

    g_pubScan = nh.advertise<sensor_msgs::LaserScan>("/m_scan", 50);
    ros::Subscriber g_subScan1_=nh.subscribe("/scan1",1000, ScanCB1);
    ros::Subscriber g_subScan2=nh.subscribe("/scan2",1000, ScanCB2);

    ros::spin();
}
