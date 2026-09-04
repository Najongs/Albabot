#include <ros/ros.h>
#include "albabot_ad1_a/setnode.h"
#include "albabot_ad1_a/clearnode.h"
#include "albabot_ad1_a/savemap.h"
#include <vector>
#include "albabot_msgs/CanMsg.h"
#include "albabot_msgs/RobotInfo.h"
#include <std_msgs/Byte.h>
#include <nav_msgs/Odometry.h>
#include <tf/transform_broadcaster.h>
#include "agvteleop.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


#define	AGV_MISSION_WAIT				0
#define	AGV_MISSION_NEXT				1
#define	AGV_MISSION_LEFT_90				2
#define	AGV_MISSION_RIGHT_90			3
#define AGV_MISSION_DELAY				4
#define AGV_MISSION_BACK				5
#define	AGV_MISSION_TURN				6

#define AGV_STATUS_ONLINE       0
#define AGV_STATUS_NODE         1
#define AGV_STATUS_OFFLINE      0xff

typedef struct _MAP_INFO
{
  int id;
  double x;
  double y;
  double th;
  int link;
  char strInfo[64];
}MAP_INFO;

ros::Publisher g_pubOdom;
ros::Publisher g_pubCommand;
ros::Publisher g_pubVel;
//uint8_t g_u8Opmode=1;

void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value);
void JoyButtonCB(const std_msgs::Byte::ConstPtr& value);
void InitRobot();
void PublishOdom(ros::Time current_time, double dt);
void SaveMap(std::string strFileName);
bool ClearNodeCb(albabot_ad1_a::clearnode::Request &req, albabot_ad1_a::clearnode::Response &res);
void LoadMap();
bool SaveMapCb(albabot_ad1_a::savemap::Request &req, albabot_ad1_a::savemap::Response &res);
bool SetNodeCb(albabot_ad1_a::setnode::Request &req, albabot_ad1_a::setnode::Response &res);
void NodeAdd(MAP_INFO map_info);



std::list<MAP_INFO> g_mapInfo;
std::list<albabot_msgs::CanMsg> g_ListCanMsg;
ros::Time timestamp, last_time;
double dt=0;
float g_fAgvVelocity=1.6;
float g_fDistance=0;
int64_t g_i64PreEncL;
int64_t g_i64PreEncR;
double g_dbPosX=0;
double g_dbPosY=0;
float g_fAngle=0;
uint8_t g_u8Opmode=OPMODE_AGV;

#include <nav_msgs/OccupancyGrid.h>
#include <nav_msgs/GridCells.h>
nav_msgs::OccupancyGrid g_gridMap;
nav_msgs::GridCells g_gridCels;
ros::Publisher g_pubMap;

int main(int argc, char **argv)
{
  ros::init(argc, argv, "agv_mapping");
  ros::NodeHandle nh;

  AGVTeleop agvTeleop;

  g_gridMap.info.width=40.0/0.2;
  g_gridMap.info.height=40.0/0.2;
  g_gridMap.info.resolution=0.2;
  g_gridMap.info.origin.position.x=0;
  g_gridMap.info.origin.position.y=0;
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
  g_gridCels.cell_width=40.0/0.2;
  g_gridCels.cell_height=40.0/0.2;
  g_gridCels.cells.resize(g_gridCels.cell_width*g_gridCels.cell_height);



  // subuscribe
  ros::Subscriber robotInfo_sub_=nh.subscribe("/robot_info",1000, RobotInfoCB);
  ros::Subscriber joy_sub=nh.subscribe("joy", 1000, &AGVTeleop::JoyCB, &agvTeleop);
  g_pubVel = nh.advertise<std_msgs::Float32MultiArray>("cmd_vel",1000);
  g_pubCommand = nh.advertise<albabot_msgs::CanMsg>("/AlbabotMessage",1000);
  g_pubOdom = nh.advertise<nav_msgs::Odometry>("/alba_odom", 50);
  g_pubMap = nh.advertise<nav_msgs::OccupancyGrid>("/coverage_map",1);
  //g_pubMap = nh.advertise<nav_msgs::GridCells>("/coverage_map",1);

  ros::ServiceServer srvSetNode = nh.advertiseService("/setnode", SetNodeCb);
  ros::ServiceServer srvClearNode = nh.advertiseService("/clearnode", ClearNodeCb);
  ros::ServiceServer srvSaveMap = nh.advertiseService("/savemap", SaveMapCb);

  MAP_INFO map_info;
  map_info.id=0;
  map_info.x=0;
  map_info.y=0;
  map_info.th=0;
  map_info.link=1;
  //map_info.strInfo="Home";
  sprintf(map_info.strInfo,"Home");

  //NodeAdd(map_info);
  //SaveMap();
  //LoadMap();

  // platform controller 의 데이터 전송 상태 확인을 위해 메시지를 기다린다.
  albabot_msgs::RobotInfoConstPtr rxMsg = ros::topic::waitForMessage<albabot_msgs::RobotInfo>("/robot_info");
  g_i64PreEncL=rxMsg->left_enc;
  g_i64PreEncR=rxMsg->right_enc;

  InitRobot();

  //agvTeleop.SetAgvVelocity(g_fAgvVelocity);

  g_fAgvVelocity=agvTeleop.GetAgvVelocity();

  ros::Rate loop_rate(20); //1.0/15.0);

  timestamp=ros::Time::now();
  last_time=timestamp;
  dt=0;

  while(ros::ok())
  {
    timestamp=ros::Time::now();
    dt += timestamp.toSec()-last_time.toSec();
    last_time = timestamp;

    PublishOdom(timestamp,dt);

    /*int idxX, idxY;
    double resol=g_gridMap.info.resolution;

    idxX=(int)(g_gridMap.info.width/2+((g_dbPosX+resol/2.0)/resol));
    idxY=(int)(g_gridMap.info.height/2+((g_dbPosY+resol/2.0)/resol));*/

    /*for(int i=0;i<3;i++)
    {
      for(int j=0;j<3;j++)
      {
        g_gridMap.data[(idxY+i-1)*g_gridMap.info.width+(idxX+j-1)]=0;
      }
    }*/
    //g_gridMap.data[(idxY)*g_gridMap.info.width+(idxX)]=0;


    g_gridMap.header.stamp= ros::Time::now();
    g_pubMap.publish(g_gridMap);

    agvTeleop.Publish(&g_pubVel,&g_pubCommand);
    g_u8Opmode=agvTeleop.GetOpMode();

    ros::spinOnce();
    loop_rate.sleep();
  }
}



void PublishOdom(ros::Time current_time, double dt)
{
  static tf::TransformBroadcaster g_odomBr;
  //since all odometry is 6DOF we'll need a quaternion created from yaw
  geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw(g_fAngle);

  //first, we'll publish the transform over tf
  geometry_msgs::TransformStamped odom_trans;
  odom_trans.header.stamp = current_time;
  odom_trans.header.frame_id = "odom";
  odom_trans.child_frame_id = "base_link";

  odom_trans.transform.translation.x = g_dbPosX;
  odom_trans.transform.translation.y = g_dbPosY;
  odom_trans.transform.translation.z = 0.0;
  odom_trans.transform.rotation = odom_quat;

  //send the transform
  g_odomBr.sendTransform(odom_trans);

  //next, we'll publish the odometry message over ROS
  nav_msgs::Odometry odom;
  odom.header.stamp = current_time;
  odom.header.frame_id = "odom";

  //set the position
  odom.pose.pose.position.x = g_dbPosX;
  odom.pose.pose.position.y = g_dbPosY;
  odom.pose.pose.position.z = 0.0;
  odom.pose.pose.orientation = odom_quat;

  //set the velocity
  odom.child_frame_id = "base_link";
  odom.twist.twist.linear.x = g_dbPosX/dt;
  odom.twist.twist.linear.y = g_dbPosY/dt;
  odom.twist.twist.angular.z = g_fAngle/dt;//((g_fAngle/180) * 3.14159)/dt;

  //publish the message
  g_pubOdom.publish(odom);
}


void InitRobot()
{
  albabot_msgs::CanMsg msg;

  g_u8Opmode=OPMODE_AGV;
  // 모드 설정
  msg.id=0x100;
  msg.dlc=3;
  msg.data.push_back(0x00);
  msg.data.push_back(0x31);
  msg.data.push_back(0x01); //0x00 : remote / 0x01 : AGV mode
  g_pubCommand.publish(msg);
  ros::spinOnce();
  //sleep(1);
  msg.data.clear();



  // 기존 명령어 취소
  msg.id=0x100;
  msg.dlc=2;
  msg.data.push_back(0x00);
  msg.data.push_back(0xff);
  g_pubCommand.publish(msg);
  ros::spinOnce();
  sleep(1);
  msg.data.clear();

  FLOAT_DATA float_data;
  float_data.fData=g_fAgvVelocity;
  msg.dlc=6;
  msg.data.push_back(0x00);
  msg.data.push_back(0x40);
  msg.data.push_back(float_data.u8Data[0]);
  msg.data.push_back(float_data.u8Data[1]);
  msg.data.push_back(float_data.u8Data[2]);
  msg.data.push_back(float_data.u8Data[3]);
  g_pubCommand.publish(msg);
  ros::spinOnce();
  sleep(1);
  msg.data.clear();

  //kp
  float_data.fData=0.2;
  msg.dlc=6;
  msg.data.push_back(0x00);
  msg.data.push_back(0x48);
  msg.data.push_back(float_data.u8Data[0]);
  msg.data.push_back(float_data.u8Data[1]);
  msg.data.push_back(float_data.u8Data[2]);
  msg.data.push_back(float_data.u8Data[3]);
  g_pubCommand.publish(msg);
  ros::spinOnce();
  sleep(1);
  msg.data.clear();

  //ki
  float_data.fData=0.0;
  msg.dlc=6;
  msg.data.push_back(0x00);
  msg.data.push_back(0x49);
  msg.data.push_back(float_data.u8Data[0]);
  msg.data.push_back(float_data.u8Data[1]);
  msg.data.push_back(float_data.u8Data[2]);
  msg.data.push_back(float_data.u8Data[3]);
  g_pubCommand.publish(msg);
  ros::spinOnce();
  sleep(1);
  msg.data.clear();

  //kd
  float_data.fData=4.0;
  msg.dlc=6;
  msg.data.push_back(0x00);
  msg.data.push_back(0x4a);
  msg.data.push_back(float_data.u8Data[0]);
  msg.data.push_back(float_data.u8Data[1]);
  msg.data.push_back(float_data.u8Data[2]);
  msg.data.push_back(float_data.u8Data[3]);
  g_pubCommand.publish(msg);
  ros::spinOnce();
  sleep(1);
  msg.data.clear();

}
void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value)
{
  float diff;
  double diff1, diff2;
  if(g_u8Opmode==1)       // agv mode
  {
    if(value->agvCurrentMission==AGV_MISSION_TURN)
    {
      g_fDistance=0;

      diff1=2.0*0.1*((value->left_enc-g_i64PreEncL)/(50.0*20))*3.141592;
      diff2=2.0*0.1*((value->right_enc-g_i64PreEncR)/(50.0*20))*3.141592;
      g_fAngle+=((diff1-diff2)/0.328);

    }
    else if(value->agvCurrentMission==AGV_MISSION_LEFT_90)
    {
      g_fDistance=0;
      double diff1, diff2;
      diff1=2.0*0.1*((value->left_enc-g_i64PreEncL)/(50.0*20))*3.141592;
      diff2=2.0*0.1*((value->right_enc-g_i64PreEncR)/(50.0*20))*3.141592;
      g_fAngle+=((diff1-diff2)/0.328);
    }
    else if(value->agvCurrentMission==AGV_MISSION_RIGHT_90)
    {
      g_fDistance=0;
      diff1=2.0*0.1*((value->left_enc-g_i64PreEncL)/(50.0*20))*3.141592;
      diff2=2.0*0.1*((value->right_enc-g_i64PreEncR)/(50.0*20))*3.141592;
      g_fAngle+=((diff1-diff2)/0.328);
    }
    else if(value->agvCurrentMission==AGV_MISSION_NEXT)
    {
      g_fAngle=(value->agvDirection-1)*(3.141592/2.0);

      diff=((float)(value->left_enc-g_i64PreEncL)+(float)(value->right_enc-g_i64PreEncR))/2.0;
      g_fDistance+=(((0.2*3.141592)/(50.0*20.0))*diff);

      if(value->agvDirection==1)
      {
        g_dbPosX+=(((0.2*3.141592)/(50.0*20.0))*diff);//g_fDistance;
      }
      else if(value->agvDirection==2)
      {
        g_dbPosY-=(((0.2*3.141592)/(50.0*20.0))*diff);//g_fDistance;
      }
      else if(value->agvDirection==3)
      {
        g_dbPosX-=(((0.2*3.141592)/(50.0*20.0))*diff);//g_fDistance;
      }
      else if(value->agvDirection==4)
      {
        g_dbPosY+=(((0.2*3.141592)/(50.0*20.0))*diff);//g_fDistance;
      }
    }
    else if(value->agvCurrentMission==AGV_MISSION_WAIT)
    {
      g_fDistance=0;
      g_fAngle=(value->agvDirection-1)*(3.141592/2.0);
    }
  }
  else if(g_u8Opmode==0)
  {
    diff1=2.0*0.1*((value->left_enc-g_i64PreEncL)/(50.0*20))*3.141592;
    diff2=2.0*0.1*((value->right_enc-g_i64PreEncR)/(50.0*20))*3.141592;
    g_fAngle+=((diff1-diff2)/0.328);

    diff=((float)(value->left_enc-g_i64PreEncL)+(float)(value->right_enc-g_i64PreEncR))/2.0;
    diff1=(((0.2*3.141592)/(50.0*20.0))*diff);
    g_dbPosX+=(diff1*cos(g_fAngle));
    g_dbPosY+=(diff1*sin(g_fAngle));
  }
  //ROS_INFO("Distance : %.3f, %.2f",g_fDistance,g_fAngle);//(g_fAngle/3.141592)*180.0);

  g_i64PreEncL=value->left_enc;
  g_i64PreEncR=value->right_enc;
}

void SaveMap(std::string strFileName)
{
  FILE *fp;
  char pStrTemp[1024];
  char pStrDir[1024];
  sprintf(pStrDir,"%s/.ros/map/",getenv("HOME"));
  mkdir(pStrDir,755); // rwxr-xr-x
  sprintf(pStrTemp,"%s/.ros/map/%s.txt",getenv("HOME"),strFileName.c_str());
  fp=fopen(pStrTemp,"w");


  std::list<MAP_INFO>::iterator iter;

  for(iter=g_mapInfo.begin();iter != g_mapInfo.end(); iter++)
  {
    fprintf(fp,"%d,%.3f,%.3f,%d,%s\n",iter->id,iter->x,iter->y,iter->link,iter->strInfo);
  }

  fclose(fp);
}


void LoadMap()
{
  FILE* fp;
  char pFileName[1024];
  sprintf(pFileName,"%s/.ros/map/map.txt",getenv("HOME"));
  fp=fopen(pFileName,"r");

  ROS_INFO("file name : %s",pFileName);

  while(!feof(fp))
  {
    MAP_INFO mapInfo;
    char pStrLine[2048];
    char pStrTemp[64];

    if(fgets(pStrLine,2048-1,fp)==NULL)
      break;


    int numElem=sscanf(pStrLine,"%d,%lf,%lf,%d,%s",&(mapInfo.id),&(mapInfo.x),&(mapInfo.y),
           &(mapInfo.link),mapInfo.strInfo);

    if(numElem==5)
    {
      g_mapInfo.push_back(mapInfo);

      ROS_INFO("map_info : %d,%.3lf,%.3lf,%d,%s",mapInfo.id, mapInfo.x, mapInfo.y,
               mapInfo.link, mapInfo.strInfo);
    }
  }
  fclose(fp);
}


bool SaveMapCb(albabot_ad1_a::savemap::Request &req, albabot_ad1_a::savemap::Response &res)
{
  ROS_INFO("Save map!!");
  SaveMap(req.filename);
  res.result=1;
  return true;
}
bool ClearNodeCb(albabot_ad1_a::clearnode::Request &req, albabot_ad1_a::clearnode::Response &res)
{

  ROS_INFO("ClearNode!!");
  res.result=1;
  return true;
}
bool SetNodeCb(albabot_ad1_a::setnode::Request &req, albabot_ad1_a::setnode::Response &res)
{
  /*MAP_INFO map_info;
  map_info.id=req.id;
  map_info.x=req.x;
  map_info.y=req.y;
  map_info.link=req.link;
  sprintf(map_info.strInfo,req.name.c_str());

  NodeAdd(map_info);*/


  //ROS_INFO("SetNode[%d] : %d",g_mapInfo.size(), req.id);
  ROS_INFO("SetNode : [%lf,%lf],[%f]",g_dbPosX, g_dbPosY,g_fAngle);
  res.result=1;
  return true;
}

int g_preX=0;
int g_preY=0;

void NodeAdd(MAP_INFO map_info)
{
  //g_mapInfo.push_back(map_info);

  /*int idxX, idxY;
  double resol=g_gridMap.info.resolution;

  idxX=(int)(g_gridMap.info.width/2+((g_dbPosX+resol/2.0)/resol));
  idxY=(int)(g_gridMap.info.height/2+((g_dbPosY+resol/2.0)/resol));

  if(abs(idxX-g_preX)<abs(idxY-g_preY))
  {

  }
  g_gridMap.data[(idxY)*g_gridMap.info.width+(idxX)]=0;

  g_preX=idxX;
  g_preX=idxY;*/

}

