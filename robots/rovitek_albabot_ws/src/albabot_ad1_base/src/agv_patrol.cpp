#include <ros/ros.h>
#include <stdio.h>
#include <sound_play/sound_play.h>
#include "albabot_msgs/RobotInfo.h"

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

enum playtype {PLAYTYPE_NEXT=0,
               PLAYTYPE_LEFT,
               PLAYTYPE_RIGHT,
               PLAYTYPE_TURN,
               PLAYTYPE_OFFLINE=4};


uint64_t g_u64Timerout1=0;
int g_nPlayCount=0;
int g_nPlayType=0;

std::string g_strPlaylist[5]=
    {"/opt/robot_alpha1/sound/Albabot_GuideVoice-01-excuseme.ogg",
     "/opt/robot_alpha1/sound/Albabot_GuideVoice-02-heresfood.ogg",
     "/opt/robot_alpha1/sound/Albabot_GuideVoice-03-arrivetrayreturn.ogg",
     "/opt/robot_alpha/sound/jejufoodshow.ogg",
     "/opt/robot_alpha/sound/jejufoodshow.ogg"
    };

uint8_t g_u8PreMission=AGV_MISSION_WAIT;


void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value)
{

  /*ROS_INFO("Enc:(%.8d, %.8d), Hall : %.4x, Obstacle : %d, AGV Status:%d, Mission : %d/%d, ",
           value->left_enc, value->right_enc,value->hall[0], value->sonic[0],value->agvStatus,
           value->agvCurrentMission,value->agvMissionCount);*/


  uint8_t curMission;
  curMission=value->agvCurrentMission;

  if(value->agvStatus==AGV_STATUS_OFFLINE)
  {
    g_nPlayCount=-1;
    g_nPlayType=PLAYTYPE_OFFLINE;
  }
  else
  {
    if(g_u8PreMission != curMission)
    {
      if(curMission==AGV_MISSION_NEXT)
      {
        g_nPlayCount=-1;
        g_nPlayType=PLAYTYPE_NEXT;
      }
      else if(curMission==AGV_MISSION_LEFT_90)
      {
        g_nPlayCount=1;
        g_nPlayType=PLAYTYPE_LEFT;
      }
      else if(curMission==AGV_MISSION_RIGHT_90)
      {
        g_nPlayCount=1;
        g_nPlayType=PLAYTYPE_RIGHT;
      }
      else if(curMission==AGV_MISSION_TURN)
      {
        g_nPlayCount=1;
        g_nPlayType=PLAYTYPE_TURN;
      }
      else
      {
        g_nPlayCount=0;
      }

      ROS_INFO("mission : %d, count : %d, %s",curMission, g_nPlayCount, g_strPlaylist[g_nPlayType].c_str() );
    }
  }


  g_u8PreMission=curMission;
  g_u64Timerout1=0;

}

double g_dbPeriod;
int main(int argc, char **argv)
{
  ros::init(argc, argv, "agv_patrol");
  ros::NodeHandle nh("~");
  ros::Subscriber robotInfo_sub_=nh.subscribe("/robot_info",1000, RobotInfoCB);


  nh.param<double>("period",g_dbPeriod,5.0);
  ROS_INFO("period:%f",g_dbPeriod);


  sound_play::SoundClient sc;

  ros::Time timestamp, last_time;
  double dt=0;
  timestamp=ros::Time::now();
  last_time=timestamp;

  ros::Rate loop_rate(20); //1.0/15.0);

  while(ros::ok())
  {
    timestamp=ros::Time::now();
    dt += timestamp.toSec()-last_time.toSec();
    last_time = timestamp;


    if(g_nPlayCount>0)
    {
      sc.playWave(g_strPlaylist[g_nPlayType],1.0f);
      g_nPlayCount--;
    }
    else if(g_nPlayCount==-1)
    {
      if(dt>g_dbPeriod)
      {
        sc.playWave(g_strPlaylist[g_nPlayType],1.0f);
        dt=0;
      }
    }
    ros::spinOnce();
    loop_rate.sleep();
  }

  sc.stopAll();
}
