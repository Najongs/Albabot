#include <ros/ros.h>
#include "albabot_msgs/RobotInfo.h"
#include <std_msgs/Byte.h>

double g_dbDistance=0;
float g_dbAngle=0.0;
double g_dbPosX;
double g_dbPosY;
int8_t g_u8Opmode=1;

int64_t g_i64PreEncL=0;
int64_t g_i64PreEncR=0;


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


int g_nIndexX=0;
int g_nIndexY=0;

uint8_t g_u8PreAgvStatus=0;
uint8_t g_u8PreMission=AGV_MISSION_WAIT;
uint32_t g_u32PreMissionCount=0;


#include <vector>
#include <bits/stdc++.h>

typedef std::pair<int,int> IndexPair;
typedef std::pair<double,double> CalPair;
std::list<std::pair<IndexPair,CalPair>> g_listCalData;

void AddNode();
void CaliPosition();

void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value)
{
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

    CaliPosition();

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


  ROS_INFO("pos[%d]:%d, %d => %.3lf, %.3lf",g_listCalData.size(), g_nIndexX, g_nIndexY, g_dbPosX, g_dbPosY);


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
      g_dbAngle+=((diff1-diff2)/0.328);

    }
    else if(value->agvCurrentMission==AGV_MISSION_LEFT_90)
    {
      g_dbDistance=0;
      double diff1, diff2;
      diff1=2.0*0.1*((value->left_enc-g_i64PreEncL)/(50.0*20))*3.141592;
      diff2=2.0*0.1*((value->right_enc-g_i64PreEncR)/(50.0*20))*3.141592;
      g_dbAngle+=((diff1-diff2)/0.328);
    }
    else if(value->agvCurrentMission==AGV_MISSION_RIGHT_90)
    {
      g_dbDistance=0;
      diff1=2.0*0.1*((value->left_enc-g_i64PreEncL)/(50.0*20))*3.141592;
      diff2=2.0*0.1*((value->right_enc-g_i64PreEncR)/(50.0*20))*3.141592;
      g_dbAngle+=((diff1-diff2)/0.328);
    }
    else if(value->agvCurrentMission==AGV_MISSION_NEXT)
    {
      g_dbAngle=(value->agvDirection-1)*(3.141592/2.0);

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
      g_dbDistance=0;
      g_dbAngle=(value->agvDirection-1)*(3.141592/2.0);
    }
  }
  else if(g_u8Opmode==0)
  {
    diff1=2.0*0.1*((value->left_enc-g_i64PreEncL)/(50.0*20))*3.141592;
    diff2=2.0*0.1*((value->right_enc-g_i64PreEncR)/(50.0*20))*3.141592;
    g_dbAngle+=((diff1-diff2)/0.328);

    diff=((float)(value->left_enc-g_i64PreEncL)+(float)(value->right_enc-g_i64PreEncR))/2.0;
    diff1=(((0.2*3.141592)/(50.0*20.0))*diff);
    g_dbPosX+=(diff1*cos(g_dbAngle));
    g_dbPosY+=(diff1*sin(g_dbAngle));
  }
  //ROS_INFO("Distance : %.3f, %.2f",g_dbDistance,g_fAngle);//(g_fAngle/3.141592)*180.0);

  g_i64PreEncL=value->left_enc;
  g_i64PreEncR=value->right_enc;
}

void CaliPosition()
{
  /*std::pair<IndexPair,CalPair> data;
  data.first.first=g_nIndexX;
  data.first.second=g_nIndexY;
  data.second.first=g_dbPosX;
  data.second.second=g_dbPosY;
  g_listCalData.push_back(data);*/

  std::list<std::pair<IndexPair,CalPair>>::iterator iter;
  for(iter=g_listCalData.begin();iter!=g_listCalData.end();++iter)
  {
    if(g_nIndexX==iter->first.first && g_nIndexY==iter->first.second)
    {
      g_dbPosX=iter->second.first;
      g_dbPosY=iter->second.second;
    }
  }
}

void AddNode()
{
  std::list<std::pair<IndexPair,CalPair>>::iterator iter;
  if(g_listCalData.size()==0)
  {
    std::pair<IndexPair,CalPair> data;
    data.first.first=g_nIndexX;
    data.first.second=g_nIndexY;
    data.second.first=g_dbPosX;
    data.second.second=g_dbPosY;
    g_listCalData.push_back(data);
  }
  else
  {
    for(iter=g_listCalData.begin();iter!=g_listCalData.end();++iter)
    {
      if(g_nIndexX==iter->first.first && g_nIndexY==iter->first.second)
      {

      }
      else
      {
        std::pair<IndexPair,CalPair> data;
        data.first.first=g_nIndexX;
        data.first.second=g_nIndexY;
        data.second.first=g_dbPosX;
        data.second.second=g_dbPosY;
        g_listCalData.push_back(data);
      }
    }
  }
}

void DelNode()
{

  if(g_listCalData.size()>0)
  {
    std::list<std::pair<IndexPair,CalPair>>::iterator iter;
    for(iter=g_listCalData.begin();iter!=g_listCalData.end();++iter)
    {
      if(g_nIndexX==iter->first.first && g_nIndexY==iter->first.second)
      {
        g_listCalData.erase(iter);
        break;
      }
    }
  }
}


#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

void SaveMap(std::string strFileName)
{
  FILE *fp;
  char pStrTemp[1024];
  char pStrDir[1024];
  sprintf(pStrDir,"%s/.ros/map/",getenv("HOME"));
  mkdir(pStrDir,755); // rwxr-xr-x
  sprintf(pStrTemp,"%s/.ros/map/%s.txt",getenv("HOME"),strFileName.c_str());
  fp=fopen(pStrTemp,"w");

  std::list<std::pair<IndexPair,CalPair>>::iterator iter;
  for(iter=g_listCalData.begin();iter!=g_listCalData.end();++iter)
  {
    fprintf(fp,"%d,%d,%.3lf,%.3lf\n",iter->first.first,iter->first.second,
            iter->second.first,iter->second.second);
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

  if(fp==NULL)
   {
    ROS_INFO("bye!");
    return;
  }

  while(!feof(fp))
  {
    std::pair<IndexPair,CalPair> data;
    char pStrLine[2048];
    char pStrTemp[64];

    if(fgets(pStrLine,2048-1,fp)==NULL)
      break;


    int numElem=sscanf(pStrLine,"%d,%d,%lf,%lf",&(data.first.first),&(data.first.second),
                       &(data.second.first),&(data.second.second));

    if(numElem==4)
    {
      g_listCalData.push_back(data);

      ROS_INFO("calibaration point : [%d,%d]=>%.3lf,%.3lf",(data.first.first),(data.first.second),
               (data.second.first),(data.second.second));
    }
  }
  fclose(fp);
}

void PoseSetterCB(const std_msgs::Byte::ConstPtr &msg)
{
  uint8_t tempData=0;
  tempData=msg->data;

  switch(tempData)
  {
    case 0:
      AddNode();
      break;
    case 1:
      DelNode();
      break;
    case 2:
      SaveMap("map");
      //ROS_INFO("%lf, %lf, %lf",g_dbPosX,g_dbPosY,g_dbAngle);
      break;
    default:
      break;
  }
}

#include <image_transport/image_transport.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>



int main(int argc, char **argv)
{
  ros::init(argc, argv, "agv_odom");
  ros::NodeHandle nh;

  ros::Subscriber robotInfo_sub_=nh.subscribe("/robot_info",1000, RobotInfoCB);
  ros::Subscriber subSet=nh.subscribe("/setter",10, PoseSetterCB);

  LoadMap();

  ros::spin();
}

