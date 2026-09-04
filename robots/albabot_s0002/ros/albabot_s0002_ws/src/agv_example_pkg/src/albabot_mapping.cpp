#include <ros/ros.h>
#include <nav_msgs/OccupancyGrid.h>
#include <nav_msgs/GridCells.h>
#include <nav_msgs/Odometry.h>
#include <tf/transform_broadcaster.h>
#include "albabot_msgs/CanMsg.h"
#include "albabot_msgs/RobotInfo.h"

ros::Time timestamp, last_time;
double dt=0;
int64_t g_i64PreEncL;
int64_t g_i64PreEncR;
double g_dbPosX=0;
double g_dbPosY=0;
float g_fAngle=0;

nav_msgs::OccupancyGrid g_gridMap;
nav_msgs::GridCells g_gridCels;
ros::Publisher g_pubMap;
ros::Publisher g_pubOdom;

void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value);


int main(int argc, char **argv)
{
    ros::init(argc, argv, "albabot_mapping");
    ros::NodeHandle nh;

    g_gridMap.info.width=10.0/0.01;
    g_gridMap.info.height=10.0/0.01;
    g_gridMap.info.resolution=0.01;
    g_gridMap.info.origin.position.x=0.0;
    g_gridMap.info.origin.position.y=0.0;
    g_gridMap.info.origin.position.z=0;

    tf::Quaternion q;
    q.setRPY(0,0,0);
    g_gridMap.info.origin.orientation.x=q.x();
    g_gridMap.info.origin.orientation.x=q.y();
    g_gridMap.info.origin.orientation.x=q.z();
    g_gridMap.info.origin.orientation.x=q.w();
    g_gridMap.data.resize(g_gridMap.info.width*g_gridMap.info.height);
    g_gridMap.data.assign(g_gridMap.info.width*g_gridMap.info.height,100);

    g_gridCels.header.frame_id="/map";
    g_gridCels.cell_width=10.0/0.01;
    g_gridCels.cell_height=10.0/0.01;
    g_gridCels.cells.resize(g_gridCels.cell_width*g_gridCels.cell_height);

    // subuscribe
    ros::Subscriber robotInfo_sub_=nh.subscribe("/robot_info",1000, RobotInfoCB);
    g_pubMap = nh.advertise<nav_msgs::OccupancyGrid>("/coverage_map",1);
    g_pubOdom = nh.advertise<nav_msgs::Odometry>("/alba_odom", 50);


    /*ros::ServiceServer srvSetNode = nh.advertiseService("/setnode", SetNodeCb);
    ros::ServiceServer srvClearNode = nh.advertiseService("/clearnode", ClearNodeCb);
    ros::ServiceServer srvSaveMap = nh.advertiseService("/savemap", SaveMapCb);*/



    // platform controller 의 데이터 전송 상태 확인을 위해 메시지를 기다린다.
    albabot_msgs::RobotInfoConstPtr rxMsg = ros::topic::waitForMessage<albabot_msgs::RobotInfo>("/robot_info");
    g_i64PreEncL=rxMsg->left_enc;
    g_i64PreEncR=rxMsg->right_enc;

    //ros::Rate loop_rate(20); //1.0/15.0);

    timestamp=ros::Time::now();
    last_time=timestamp;
    dt=0;

    ros::spin();

    /*
    while(ros::ok())
    {





      ros::spinOnce();
      loop_rate.sleep();
    }
    */


}


tf::Transform transform;
tf::Quaternion q;


ros::Publisher odom_pub;

void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value)
{
    double v,w,dt;

    timestamp=ros::Time::now();
    dt = timestamp.toSec()-last_time.toSec();
    last_time = timestamp;

    float diff;
    double diff1, diff2;

    diff1=2.0*0.1*((value->left_enc-g_i64PreEncL)/(30.0*20))*3.141592;
    diff2=2.0*0.1*((value->right_enc-g_i64PreEncR)/(30.0*20))*3.141592;
    g_fAngle+=((diff2-diff1)/0.375);

    diff=((float)(value->left_enc-g_i64PreEncL)+(float)(value->right_enc-g_i64PreEncR))/2.0;
    diff1=(((0.2*3.141592)/(30.0*20.0))*diff);
    g_dbPosX+=(diff1*cos(g_fAngle));
    g_dbPosY+=(diff1*sin(g_fAngle));
    //ROS_INFO("Distance : %.3f, %.2f",g_fDistance,g_fAngle);//(g_fAngle/3.141592)*180.0);

    g_i64PreEncL=value->left_enc;
    g_i64PreEncR=value->right_enc;

    nav_msgs::Odometry odom_msg;

    q.setRPY(0,0.0,g_fAngle);
    odom_msg.header.stamp = timestamp;
    odom_msg.header.frame_id = "odom";
    //set the position
    odom_msg.pose.pose.position.x = g_dbPosX;
    odom_msg.pose.pose.position.y = g_dbPosY;
    odom_msg.pose.pose.position.z = 0.0;
    odom_msg.pose.pose.orientation.x = q.x();
    odom_msg.pose.pose.orientation.y = q.y();
    odom_msg.pose.pose.orientation.z = q.z();
    odom_msg.pose.pose.orientation.w = q.w();
    //set the velocity
    odom_msg.child_frame_id = "base_footprint";
    odom_msg.twist.twist.linear.x = v/dt;
    odom_msg.twist.twist.linear.y = 0;
    odom_msg.twist.twist.angular.z = w/dt;

    g_pubOdom.publish(odom_msg);


    static tf::TransformBroadcaster br;

    transform.setOrigin(tf::Vector3(g_dbPosX,g_dbPosY,0));
    //q.setRPY(0,0.0,g_Theta);
    transform.setRotation(q);
    br.sendTransform(tf::StampedTransform(transform, timestamp, "odom", "base_footprint"));

    transform.setOrigin(tf::Vector3(5,-5,0));
    q.setRPY(0.0,0.0,0.0);
    transform.setRotation(q);
    br.sendTransform(tf::StampedTransform(transform, timestamp, "map", "odom"));


    int idxX, idxY;
    double resol=g_gridMap.info.resolution;

    idxX=(int)(g_gridMap.info.width/2+((g_dbPosX+resol/2.0)/resol));
    idxY=(int)(g_gridMap.info.height/2+((-1*g_dbPosY+resol/2.0)/resol));
    //idxX=(int)(((g_dbPosX+resol/2.0)/resol));
    //idxY=(int)(((g_dbPosY+resol/2.0)/resol));

    for(int i=0;i<3;i++)
    {
      for(int j=0;j<3;j++)
      {
        g_gridMap.data[(idxY+i-2)*g_gridMap.info.width+(idxX+j-2)]=0;
      }
    }
    //g_gridMap.data[(idxY)*g_gridMap.info.width+(idxX)]=0;


    g_gridMap.header.stamp= ros::Time::now();
    g_pubMap.publish(g_gridMap);


}
