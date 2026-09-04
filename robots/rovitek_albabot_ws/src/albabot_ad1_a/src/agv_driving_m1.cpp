#include <ros/ros.h>
#include <std_msgs/Byte.h>
#include <sound_play/sound_play.h>
#include <std_msgs/String.h>
#include <nav_msgs/Odometry.h>
#include <tf/transform_broadcaster.h>
#include "albabot_msgs/CanMsg.h"
#include "albabot_msgs/RobotInfo.h"
#include <vector>

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


void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value);
void JoyButtonCB(const std_msgs::Byte::ConstPtr& value);
void InitRobot();


uint64_t g_u64Timerout1=0;
int g_nPlayCount=0;
int g_nPlayType=0;

std::string g_strSoundHome;//"/opt/albabot_ad1_a/sound/";
std::string g_strPlaylist[5]=
    {"jejufoodshow.ogg",
     "Albabot_GuideVoice-09-leftturn.ogg",
     "Albabot_GuideVoice-10-rightturn.ogg",
     "Albabot_GuideVoice-11-u-turn.ogg",
     "Albabot_GuideVoice-04-iamlost.ogg"
    };

ros::Publisher g_pubOdom;
ros::Publisher g_pubCommand;



uint8_t g_u8Opmode=1;   // remote=0, agv=1;
double g_dbPeriod;
//std::vector<albabot_msgs::CanMsg> g_ListCanMsg;
std::list<albabot_msgs::CanMsg> g_ListCanMsg;


float g_fDistance=0;
int64_t g_i64PreEncL;
int64_t g_i64PreEncR;

u8 g_u8FlagDecel=0;
double g_dbPosX=0;
double g_dbPosY=0;
float g_fAngle=0;

void PublishOdom(ros::Time current_time, double dt);

int main(int argc, char **argv)
{
  ros::init(argc, argv, "agv_driving_m1");
  ros::NodeHandle nh("~");

  // subuscribe
  ros::Subscriber robotInfo_sub_=nh.subscribe("/robot_info",1000, RobotInfoCB);
  ros::Subscriber joyButton_sub_=nh.subscribe("/joyButton",1000,JoyButtonCB);
  g_pubCommand = nh.advertise<albabot_msgs::CanMsg>("/AlbabotMessage",1000);
  g_pubOdom = nh.advertise<nav_msgs::Odometry>("/alba_odom", 50);

  // parameter
  nh.param<double>("period",g_dbPeriod,5.0);
  ROS_INFO("period:%f",g_dbPeriod);
  nh.param<std::string>("sound_home",g_strSoundHome,"/opt/albabot/sound/");
  ROS_INFO("sound_home:%s",g_strSoundHome.c_str());

  sound_play::SoundClient sc;
  ros::Time timestamp, last_time;
  double dt=0;

  // platfor controller 의 데이터 전송 상태 확인을 위해 메시지를 기다린다.
  albabot_msgs::RobotInfoConstPtr rxMsg = ros::topic::waitForMessage<albabot_msgs::RobotInfo>("/robot_info");
  g_i64PreEncL=rxMsg->left_enc;
  g_i64PreEncR=rxMsg->right_enc;
  ROS_INFO("message OK!");

  InitRobot();


  /*for(int i=0;i<10;i++)
  {
    albabot_msgs::CanMsg msg;
    msg.id=i;
    msg.dlc=1;
    msg.data.push_back(i);

    g_ListCanMsg.push_back(msg);
  }*/

  ros::Rate loop_rate(50); //1.0/15.0);

  timestamp=ros::Time::now();
  last_time=timestamp;
  dt=0;



  while(ros::ok())
  {
    timestamp=ros::Time::now();
    dt += timestamp.toSec()-last_time.toSec();
    last_time = timestamp;

    PublishOdom(timestamp,dt);



    /*if(g_nPlayCount>0)
    {
      sc.playWave(g_strSoundHome+g_strPlaylist[g_nPlayType],1.0f);
      g_nPlayCount--;
    }
    else if(g_nPlayCount==-1)
    {
      if(dt>g_dbPeriod)
      {
        sc.playWave(g_strSoundHome+g_strPlaylist[g_nPlayType],1.0f);
        dt=0;
      }
    }*/

    if(g_ListCanMsg.size()>0)
    {
      albabot_msgs::CanMsg firstMsg;
      firstMsg=g_ListCanMsg.front();
      g_pubCommand.publish(firstMsg);
      g_ListCanMsg.pop_front();
    }

    ros::spinOnce();
    loop_rate.sleep();
  }

  return 1;

}

#if 1
void PublishOdom(ros::Time current_time, double dt)
{
  static tf::TransformBroadcaster g_odomBr;
  //since all odometry is 6DOF we'll need a quaternion created from yaw
      geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw((g_fAngle/180) * 3.141592);

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
      odom.twist.twist.angular.z = ((g_fAngle/180) * 3.14159)/dt;

      //publish the message
      g_pubOdom.publish(odom);
}
#endif


void InitRobot()
{
  albabot_msgs::CanMsg msg;

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

  g_u8Opmode=1;

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
  float_data.fData=1.6;
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

void JoyButtonCB(const std_msgs::Byte::ConstPtr& value)
{
  uint8_t data;
  data=value->data;

  if(data == 0)
  {
    // No operation
  }
  else if(data == 1)
  {
    // cancel
    albabot_msgs::CanMsg msg;
    msg.id=0x100;
    msg.dlc=2;
    msg.data.push_back(0x00);
    msg.data.push_back(0xff);
    g_ListCanMsg.push_back(msg);
  }
}



void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value)
{
  g_fAngle=(value->agvDirection-1)*90.0;
  float diff;
  if(g_u8Opmode==1)       // agv mode
  {
    if(value->agvStatus==AGV_STATUS_OFFLINE)
    {
      g_nPlayCount=-1;
      g_nPlayType=PLAYTYPE_OFFLINE;

      g_fDistance=0;
      g_u8FlagDecel=0;
    }
    else
    {
      if(value->agvCurrentMission==AGV_MISSION_TURN)
      {
        g_nPlayCount=1;
        g_nPlayType=PLAYTYPE_TURN;
        g_fDistance=0;
        g_u8FlagDecel=0;
      }
      else if(value->agvCurrentMission==AGV_MISSION_LEFT_90)
      {
        g_nPlayCount=1;
        g_nPlayType=PLAYTYPE_LEFT;
        g_fDistance=0;
        g_u8FlagDecel=0;

      }
      else if(value->agvCurrentMission==AGV_MISSION_RIGHT_90)
      {
        g_nPlayCount=1;
        g_nPlayType=PLAYTYPE_RIGHT;
        g_fDistance=0;
        g_u8FlagDecel=0;
      }
      else if(value->agvCurrentMission==AGV_MISSION_NEXT)
      {
        g_nPlayCount=-1;
        g_nPlayType=PLAYTYPE_NEXT;

        diff=((float)(value->left_enc-g_i64PreEncL)+(float)(value->right_enc-g_i64PreEncR))/2.0;
        g_fDistance+=(((0.2*3.141592)/(50.0*20.0))*diff);

        if(value->agvDirection==1)
        {
          g_dbPosX+=g_fDistance;
        }
        else if(value->agvDirection==2)
        {
          g_dbPosY-=g_fDistance;
        }
        else if(value->agvDirection==3)
        {
          g_dbPosX-=g_fDistance;
        }
        else if(value->agvDirection==4)
        {
          g_dbPosY+=g_fDistance;
        }

        FLOAT_DATA float_data;
        albabot_msgs::CanMsg msg;

        if(g_u8FlagDecel==0)
        {
          if(g_fDistance<1.0)
          {
            float_data.fData=1.0;
            msg.dlc=6;
            msg.data.push_back(0x00);
            msg.data.push_back(0x40);
            msg.data.push_back(float_data.u8Data[0]);
            msg.data.push_back(float_data.u8Data[1]);
            msg.data.push_back(float_data.u8Data[2]);
            msg.data.push_back(float_data.u8Data[3]);
            g_ListCanMsg.push_back(msg);
            msg.data.clear();
            g_u8FlagDecel=1;
          }
        }
        else if(g_u8FlagDecel==1)
        {
          if(g_fDistance>=1.0)
          {
            float_data.fData=3.0;
            msg.dlc=6;
            msg.data.push_back(0x00);
            msg.data.push_back(0x40);
            msg.data.push_back(float_data.u8Data[0]);
            msg.data.push_back(float_data.u8Data[1]);
            msg.data.push_back(float_data.u8Data[2]);
            msg.data.push_back(float_data.u8Data[3]);
            g_ListCanMsg.push_back(msg);
            msg.data.clear();
            g_u8FlagDecel=2;
          }
        }
        else if(g_u8FlagDecel==2)
        {
          //if(g_fDistance>6.0)
          if(g_fDistance>10.0)
          //if(g_fDistance>1.0)
          {
            float_data.fData=1.0;
            msg.dlc=6;
            msg.data.push_back(0x00);
            msg.data.push_back(0x40);
            msg.data.push_back(float_data.u8Data[0]);
            msg.data.push_back(float_data.u8Data[1]);
            msg.data.push_back(float_data.u8Data[2]);
            msg.data.push_back(float_data.u8Data[3]);
            g_ListCanMsg.push_back(msg);
            msg.data.clear();
            //g_u8FlagDecel=0;
          }
        }



      }
      else if(value->agvCurrentMission==AGV_MISSION_WAIT)
      {
        albabot_msgs::CanMsg msg;
        msg.dlc=5;
        msg.data.push_back(0x01); //type
        msg.data.push_back(AGV_MISSION_NEXT); //path 0x01:NEXT, 0x05
        msg.data.push_back(AGV_MISSION_DELAY);
        msg.data.push_back(AGV_MISSION_TURN);
        msg.data.push_back(AGV_MISSION_WAIT);
        g_ListCanMsg.push_back(msg);
        msg.data.clear();

        msg.dlc=2;
        msg.data.push_back(0x00);
        msg.data.push_back(0x00);
        g_ListCanMsg.push_back(msg);
        msg.data.clear();

        g_fDistance=0;
        g_u8FlagDecel=0;
      }
    }
  }

  ROS_INFO("Distance : %.3f, %f",g_fDistance,diff);

  g_i64PreEncL=value->left_enc;
  g_i64PreEncR=value->right_enc;
}

