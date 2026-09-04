#include <ros/ros.h>
#include <stdio.h>
#include <sound_play/sound_play.h>
#include "sensor_msgs/Joy.h"
#include "std_msgs/Float32MultiArray.h"

#include "albabot_msgs/CanMsg.h"
#include "albabot_msgs/Command.h"
#include "albabot_msgs/RobotInfo.h"

#define DISTANCE_WHEEL_TO_WHEEL 0.328

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

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;

typedef union _FLOAT_DATA
{
  u32 u32Data;
  float fData;
  u8 u8Data[4];
    u16 u16Data[2];
  int nData;
}FLOAT_DATA;

enum playtype {PLAYTYPE_NEXT=0,
               PLAYTYPE_LEFT,
               PLAYTYPE_RIGHT,
               PLAYTYPE_TURN,
               PLAYTYPE_OFFLINE=4};


uint64_t g_u64Timerout1=0;
int g_nPlayCount=0;
int g_nPlayType=0;


std::string g_strPlaylist[5]=
    {"/opt/robot_alpha1/sound/jejufoodshow.ogg",
     "/opt/robot_alpha1/sound/Albabot_GuideVoice-01-excuseme.ogg",
     "/opt/robot_alpha1/sound/Albabot_GuideVoice-02-heresfood.ogg",
     "/opt/robot_alpha1/sound/Albabot_GuideVoice-03-arrivetrayreturn.ogg",
     "/opt/robot_alpha1/sound/jejufoodshow.ogg"

    };

uint8_t g_u8PreMission=AGV_MISSION_WAIT;
uint8_t g_u8Mode=0; // remote;
ros::Publisher g_command_pub;

double g_joy_scale, g_joy_v, g_joy_w;
int g_TimeoutCnt=0;
bool g_blnIsProc=false;

uint8_t g_u8PreModeBtn=0;
uint8_t g_u8ModeBtn=0;

uint8_t g_u8PreLeftBtn=0;
uint8_t g_u8PreRightBtn=0;

uint8_t g_u8LeftBtn=0;
uint8_t g_u8RightBtn=0;

void CheckBtnStatus(void);

uint8_t g_u8SendCount=0;

void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value)
{

  /*ROS_INFO("Enc:(%.8d, %.8d), Hall : %.4x, Obstacle : %d, AGV Status:%d, Mission : %d/%d, ",
           value->left_enc, value->right_enc,value->hall[0], value->sonic[0],value->agvStatus,
           value->agvCurrentMission,value->agvMissionCount);*/


  uint8_t curMission;
  curMission=value->agvCurrentMission;
  if(g_u8Mode==1)
  {
    if(value->agvStatus==AGV_STATUS_OFFLINE)
    {
      g_nPlayCount=-1;
      g_nPlayType=PLAYTYPE_OFFLINE;
    }
    else
    {
      albabot_msgs::CanMsg msg;
      msg.id=0x100;

      if(curMission==AGV_MISSION_WAIT)
      {
        if(g_u8SendCount==0)
        {
          /*msg.dlc=2;
          msg.data.push_back(0x00);
          msg.data.push_back(0xff);
          g_command_pub.publish(msg);*/
          g_u8SendCount=1;
        }
        else if(g_u8SendCount==1)
        {
          msg.dlc=5;
          msg.data.push_back(0x01); //type
          msg.data.push_back(AGV_MISSION_NEXT); //path 0x01:NEXT, 0x05
          msg.data.push_back(AGV_MISSION_DELAY);
          msg.data.push_back(AGV_MISSION_TURN);
          msg.data.push_back(AGV_MISSION_WAIT);
          g_command_pub.publish(msg);
          g_u8SendCount=2;
          }
      }
      if(g_u8SendCount==2)
      {
        g_nPlayCount=-1;
        g_nPlayType=PLAYTYPE_NEXT;

        g_u8SendCount=3;
      }
      else if(g_u8SendCount==3)
      {
        msg.dlc=2;
        msg.data.push_back(0x00);
        msg.data.push_back(0x00);
        g_command_pub.publish(msg);
        g_u8SendCount=0;

      }

      //}

      /*if(g_u8PreMission != curMission)
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
      }*/
    }
  }

  g_u8PreMission=curMission;
  g_u64Timerout1=0;
}

bool SetMode(uint8_t mode)
{
    FLOAT_DATA float_data;
    albabot_msgs::CanMsg msg;
    bool blnIsCmd=false;
    msg.id=0x100;   // 현재버전에서는 의미없는 값임.
    msg.dlc=3;
    msg.data.push_back(0x00);
    msg.data.push_back(0x31);
    msg.data.push_back(mode); //0x00 : remote / 0x01 : AGV mode

    blnIsCmd=true;
    g_command_pub.publish(msg);



    /*
    // 서비스 요청
    g_Command.request.can_msg=msg;
    blnIsCmd=g_ServiceClient.call(g_Command);

    ROS_INFO("result : %d, data = %d, %d, %d",g_Command.response.result,g_Command.response.can_msg.data[0],
            g_Command.response.can_msg.data[1],
            g_Command.response.can_msg.data[2]);
    */

    return blnIsCmd;

}

void CheckBtnStatus(void)
{
  albabot_msgs::CanMsg msg;

  if(g_blnIsProc==false)
  {
    if(g_u8PreModeBtn==1 && g_u8ModeBtn==0)
    {
      g_blnIsProc=true;
      g_u8Mode=1-g_u8Mode;
      SetMode(g_u8Mode);
      ROS_INFO("Mode : %d", g_u8Mode);
      g_blnIsProc=false;
    }
    else if(g_u8PreLeftBtn==1 && g_u8LeftBtn==0)
    {
      g_blnIsProc=true;
      msg.id=0x100;   // 현재버전에서는 의미없는 값임.
      msg.dlc=2;
      msg.data.push_back(0x00);
      msg.data.push_back(0xff);
      g_blnIsProc=false;
      g_command_pub.publish(msg);

    }
    else if(g_u8PreRightBtn==1 && g_u8RightBtn==0)
    {
      g_blnIsProc=true;
      msg.id=0x100;   // 현재버전에서는 의미없는 값임.
      msg.dlc=2;
      msg.data.push_back(0x00);
      msg.data.push_back(0x00);
      g_blnIsProc=false;
      g_command_pub.publish(msg);
    }
  }

  g_u8PreModeBtn  =g_u8ModeBtn;
  g_u8PreLeftBtn  =g_u8LeftBtn;
  g_u8PreRightBtn  =g_u8RightBtn;
}


void JoyCB(const sensor_msgs::Joy::ConstPtr& _joy)
{
  // kiro joystick
  g_joy_scale=((1+_joy->axes[2])/2.0) * 3.3;
  g_joy_v = 1.0*g_joy_scale*_joy->axes[1];
  g_joy_w = 1.0*g_joy_scale*_joy->axes[3];

  // xbox joystick
  /*g_joy_scale=((1+_joy->axes[2])/2.0) * 3.3;
  g_joy_v = 1.0*g_joy_scale*_joy->axes[1];
  g_joy_w = 1.0*g_joy_scale*_joy->axes[0];*/

  g_u8ModeBtn=_joy->buttons[4];
  g_u8RightBtn=_joy->buttons[1];
  g_u8LeftBtn=_joy->buttons[2];

  g_TimeoutCnt=0;
}


double g_dbPeriod;
int main(int argc, char **argv)
{
  ros::init(argc, argv, "agv_patrol2");
  ros::NodeHandle nh("~");
  ros::Subscriber robotInfo_sub_=nh.subscribe("/robot_info",1000, RobotInfoCB);
  ros::Publisher vel_pub = nh.advertise<std_msgs::Float32MultiArray>("/cmd_vel",1000);
  ros::Subscriber joy_sub=nh.subscribe("/joy", 1000, JoyCB);
  g_command_pub = nh.advertise<albabot_msgs::CanMsg>("/AlbabotMessage",1000);


  nh.param<double>("period",g_dbPeriod,5.0);
  ROS_INFO("period:%f",g_dbPeriod);


  sound_play::SoundClient sc;

  ros::Time timestamp, last_time;
  double dt=0;
  timestamp=ros::Time::now();
  last_time=timestamp;

  ros::Rate loop_rate(20); //1.0/15.0);

  sleep(5);
  g_u8Mode=1;
  SetMode(g_u8Mode);
  ros::spinOnce();
  sleep(5);
  albabot_msgs::CanMsg msg;
  msg.id=0x100;
  msg.dlc=2;
  msg.data.push_back(0x00);
  msg.data.push_back(0xff);
  g_command_pub.publish(msg);
  ros::spinOnce();
  sleep(5);


  while(ros::ok())
  {
    timestamp=ros::Time::now();
    dt += timestamp.toSec()-last_time.toSec();
    last_time = timestamp;

    CheckBtnStatus();

    if(g_u8Mode==1)
    {
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
    }
    else if(g_u8Mode==0)
    {
      double vel1=0, vel2=0;
      std_msgs::Float32MultiArray vel;
      double _v,_w;

      if(g_TimeoutCnt<10)   // 1000ms
      {
        g_TimeoutCnt++;
      }

      if(g_TimeoutCnt>10)
      {
        _v=0.0;
        _w=0.0;
      }
      else
      {
        _v=g_joy_v;
        _w=g_joy_w;
      }

      vel1=-1.0*(_v-DISTANCE_WHEEL_TO_WHEEL*_w )*(3.3/3.3);
      vel2=-1.0*(_v+DISTANCE_WHEEL_TO_WHEEL*_w )*(3.3/3.3);

      vel.data.push_back(vel1);
      vel.data.push_back(vel2);

      vel_pub.publish(vel);
    }

    ros::spinOnce();
    loop_rate.sleep();
  }

  sc.stopAll();
}
