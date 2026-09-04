#include <ros/ros.h>
#include <std_msgs/Byte.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/Pose2D.h>
#include "albabot_msgs/RobotInfo.h"
#include <tf/transform_broadcaster.h>


#define	AGV_MISSION_WAIT          0
#define	AGV_MISSION_NEXT          1
#define	AGV_MISSION_LEFT_90				2
#define	AGV_MISSION_RIGHT_90			3
#define AGV_MISSION_DELAY         4
#define AGV_MISSION_BACK          5
#define	AGV_MISSION_TURN          6

#define AGV_STATUS_ONLINE         0
#define AGV_STATUS_NODE           1
#define AGV_STATUS_OFFLINE        0xff

std::string g_strOdomFrame;
std::string g_strBaselinkFrame;



void GetParam(ros::NodeHandle *pNh);
void PublishOdom(geometry_msgs::Pose2D pose);

double g_dbDistance=0;
float g_dbAngle=0.0;
double g_dbPosX;
double g_dbPosY;
int8_t g_u8Opmode=1;

int64_t g_i64PreEncL=0;
int64_t g_i64PreEncR=0;

uint8_t g_u8PreAgvStatus=0;
uint8_t g_u8PreMission=AGV_MISSION_WAIT;
uint32_t g_u32PreMissionCount=0;

int g_nIndexX=0;
int g_nIndexY=0;

ros::Publisher g_pubOdom;
geometry_msgs::Pose2D g_curPose;

ros::Time timestamp, last_time;
double dt=0;

void GetParam(ros::NodeHandle *pNh)
{
  // parameter
  int nTemp=0;
  float fTemp=0;
  std::string strTemp;

  pNh->param<std::string>("odom_frame",strTemp,"odom");
  g_strOdomFrame=strTemp;
  pNh->param<std::string>("baselink_frame",strTemp,"odom");
  g_strOdomFrame=strTemp;

}

float g_mapResolution=0.01; // 1cm

void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value)
{
  int nIndexOffset=0;
  
  nIndexOffset=(int)(g_dbDistance/g_mapResolution);

  
  //preAgvstatus(1=node) & curAgvStatus(0=line) => next driving
  if(g_u8PreAgvStatus==1 && value->agvStatus==0)
  {
    if(value->agvDirection==1)
      g_nIndexY++;
    else if(value->agvDirection==2)
      g_nIndexX++;
    else if(value->agvDirection==3)
      g_nIndexY--;
    else if(value->agvDirection==4)
      g_nIndexX--;

    //CaliPosition();

  }
  else if(value->agvCurrentMission==AGV_MISSION_NEXT && g_u8PreMission!=AGV_MISSION_NEXT)
  {
    if(value->agvDirection==1)
      g_nIndexY++;
    else if(value->agvDirection==2)
      g_nIndexX++;
    else if(value->agvDirection==3)
      g_nIndexY--;
    else if(value->agvDirection==4)
      g_nIndexX--;
  }
  else if(value->agvCurrentMission==AGV_MISSION_NEXT &&
          g_u8PreMission==AGV_MISSION_NEXT &&
          g_u32PreMissionCount != value->agvMissionCount)
  {
    if(value->agvDirection==1)
      g_nIndexY++;
    else if(value->agvDirection==2)
      g_nIndexX++;
    else if(value->agvDirection==3)
      g_nIndexY--;
    else if(value->agvDirection==4)
      g_nIndexX--;
  }
  
  if(value->agvCurrentMission==AGV_MISSION_NEXT)
  {
    if(value->agvDirection==1)
      g_nIndexY++;
    else if(value->agvDirection==2)
      g_nIndexX++;
    else if(value->agvDirection==3)
      g_nIndexY--;
    else if(value->agvDirection==4)
      g_nIndexX--;
  }
  //ROS_INFO("pos[%d]:%d, %d => %.3lf, %.3lf",g_listCalData.size(), g_nIndexX, g_nIndexY, g_dbPosX, g_dbPosY);

  g_u8PreAgvStatus=value->agvStatus;
  g_u8PreMission=value->agvCurrentMission;
  g_u32PreMissionCount=value->agvMissionCount;

  float diff;
  double diff1, diff2;
  if(g_u8Opmode==1)       // agv mode
  {
    if(value->agvCurrentMission==AGV_MISSION_TURN)
    {
      g_dbDistance=0;

      diff1=2.0*0.1*((value->left_enc-g_i64PreEncL)/(50.0*20))*3.141592;
      diff2=2.0*0.1*((value->right_enc-g_i64PreEncR)/(50.0*20))*3.141592;
      g_dbAngle+=((diff2-diff1)/0.328);

    }
    else if(value->agvCurrentMission==AGV_MISSION_LEFT_90)
    {
      g_dbDistance=0;
      double diff1, diff2;
      diff1=2.0*0.1*((value->left_enc-g_i64PreEncL)/(50.0*20))*3.141592;
      diff2=2.0*0.1*((value->right_enc-g_i64PreEncR)/(50.0*20))*3.141592;
      g_dbAngle+=((diff2-diff1)/0.328);
    }
    else if(value->agvCurrentMission==AGV_MISSION_RIGHT_90)
    {
      g_dbDistance=0;
      diff1=2.0*0.1*((value->left_enc-g_i64PreEncL)/(50.0*20))*3.141592;
      diff2=2.0*0.1*((value->right_enc-g_i64PreEncR)/(50.0*20))*3.141592;
      g_dbAngle+=((diff2-diff1)/0.328);
    }
    else if(value->agvCurrentMission==AGV_MISSION_NEXT)
    {
      g_dbAngle=(value->agvDirection-1)*(-3.141592/2.0);

      diff=((float)(value->left_enc-g_i64PreEncL)+(float)(value->right_enc-g_i64PreEncR))/2.0;
      g_dbDistance+=(((0.2*3.141592)/(50.0*20.0))*diff);

      if(value->agvDirection==1)
      {
        g_dbPosX+=(((0.2*3.141592)/(50.0*20.0))*diff);//g_dbDistance;
      }
      else if(value->agvDirection==2)
      {
        g_dbPosY-=(((0.2*3.141592)/(50.0*20.0))*diff);//g_dbDistance;
      }
      else if(value->agvDirection==3)
      {
        g_dbPosX-=(((0.2*3.141592)/(50.0*20.0))*diff);//g_dbDistance;
      }
      else if(value->agvDirection==4)
      {
        g_dbPosY+=(((0.2*3.141592)/(50.0*20.0))*diff);//g_dbDistance;
      }
    }
    else if(value->agvCurrentMission==AGV_MISSION_WAIT)
    {
      //g_dbDistance=0;
      g_dbAngle=(value->agvDirection-1)*(-3.141592/2.0);
    }
  }
  else if(g_u8Opmode==0)
  {
    diff1=2.0*0.1*((value->left_enc-g_i64PreEncL)/(50.0*20))*3.141592;
    diff2=2.0*0.1*((value->right_enc-g_i64PreEncR)/(50.0*20))*3.141592;
    g_dbAngle+=((diff2-diff1)/0.328);

    diff=((float)(value->left_enc-g_i64PreEncL)+(float)(value->right_enc-g_i64PreEncR))/2.0;
    diff1=(((0.2*3.141592)/(50.0*20.0))*diff);
    g_dbPosX+=(diff1*cos(g_dbAngle));
    g_dbPosY+=(diff1*sin(g_dbAngle));
  }
  ROS_INFO("Pos(%.3f,%.3f), Distance(%.3f), Angle(%.2f)",g_dbPosX,g_dbPosY,g_dbDistance,g_dbAngle);//(g_fAngle/3.141592)*180.0);


  g_curPose.x=g_dbPosX;
  g_curPose.y=g_dbPosY;
  g_curPose.theta=g_dbAngle;


  g_i64PreEncL=value->left_enc;
  g_i64PreEncR=value->right_enc;

  PublishOdom(g_curPose);
}

void PublishOdom(geometry_msgs::Pose2D pose)
{
  static ros::Time last_time=ros::Time::now();
  double dt=0;
  ros::Time current_time;

  current_time=ros::Time::now();
  dt=(last_time-current_time).toSec();
  last_time=current_time;

  static tf::TransformBroadcaster g_odomBr;
  //since all odometry is 6DOF we'll need a quaternion created from yaw
  geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw(pose.theta);

  //first, we'll publish the transform over tf
  geometry_msgs::TransformStamped odom_trans;
  odom_trans.header.stamp = current_time;
  odom_trans.header.frame_id = "odom";
  odom_trans.child_frame_id = "base_link";

  odom_trans.transform.translation.x = pose.x;
  odom_trans.transform.translation.y = pose.y;
  odom_trans.transform.translation.z = 0.0;
  odom_trans.transform.rotation = odom_quat;

  //tf nav_msgsROS_INFO("pulish odom");
  //send the transform
  g_odomBr.sendTransform(odom_trans);

  //next, we'll publish the odometry message over ROS
  nav_msgs::Odometry odom;
  odom.header.stamp = current_time;
  odom.header.frame_id = g_strOdomFrame;

  //set the position
  odom.pose.pose.position.x = pose.x;
  odom.pose.pose.position.y = pose.y;
  odom.pose.pose.position.z = 0.0;
  odom.pose.pose.orientation = odom_quat;

  //set the velocity
  odom.child_frame_id = g_strBaselinkFrame;
  odom.twist.twist.linear.x = pose.x/dt;
  odom.twist.twist.linear.y = pose.y/dt;
  odom.twist.twist.angular.z = pose.theta/dt;//((g_fAngle/180) * 3.14159)/dt;

  //publish the message
  g_pubOdom.publish(odom);
}


int main(int argc, char **argv)
{
  ros::init(argc, argv, "agv_odom");
  ros::NodeHandle nh;
  g_u8Opmode=1;

  GetParam(&nh);

  ros::Subscriber robotInfo_sub_=nh.subscribe("/robot_info",1000, RobotInfoCB);
  g_pubOdom = nh.advertise<nav_msgs::Odometry>("/alba_odom", 50);
  albabot_msgs::RobotInfoConstPtr rxMsg = ros::topic::waitForMessage<albabot_msgs::RobotInfo>("/robot_info");



  ros::spin();
}
