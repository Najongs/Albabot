/*
  tutorial02_node
    1. ROS service를 이용하여 로봇의 모드를 리모트로 변경
    2. 조이스틱으로 부터 값을 받아 로봇의 좌우 바퀴 속도값 계산 및 publish

*/
#include <ros/ros.h>
#include "std_msgs/String.h"
#include "sensor_msgs/Joy.h"
#include "std_msgs/Float32MultiArray.h"

#include "albabot_msgs/CanMsg.h"
#include "albabot_msgs/Command.h"

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

typedef struct _ALBABOT_PARAM
{
  float fMaxV;
  float fMaxW;
}ALBABOT_PARAM;

#define DISTANCE_WHEEL_TO_WHEEL   0.328

double g_joy_scale, g_joy_v, g_joy_w;
int g_TimeoutCnt=0;

//ros::ServiceClient g_ServiceClient;
//albabot_msgs::Command g_Command;
ros::Publisher g_command_pub;


uint8_t g_u8Mode=0x00;

uint8_t g_u8PreNextBtn=0;
uint8_t g_u8PreLeftBtn=0;
uint8_t g_u8PreRightBtn=0;
uint8_t g_u8PreTurnBtn=0;
uint8_t g_u8PreModeBtn=0;
uint8_t g_u8PreSpeedUpBtn=0;

uint8_t g_u8NextBtn=0;
uint8_t g_u8LeftBtn=0;
uint8_t g_u8RightBtn=0;
uint8_t g_u8TurnBtn=0;
uint8_t g_u8ModeBtn=0;
uint8_t g_u8SpeedUpBtn=0;

float g_fCurSpeed=1.9;
bool g_blnIsProc=false;

ALBABOT_PARAM GetParameter(ros::NodeHandle *n);
void JoyCB(const sensor_msgs::Joy::ConstPtr& _joy);
bool SendCommand(uint8_t cmd);
void CheckBtnStatus(void);

int main(int argc, char **argv)
{
  ros::init(argc, argv, "joy_agv_teleop");
  ros::NodeHandle nh;

  //g_ServiceClient = nh.serviceClient<albabot_msgs::Command>("AlbabotMessage");
  //albabot_msgs::CanMsg msg;
  g_command_pub = nh.advertise<albabot_msgs::CanMsg>("AlbabotMessage",1000);

  ros::Publisher vel_pub = nh.advertise<std_msgs::Float32MultiArray>("cmd_vel",1000);
  ros::Subscriber joy_sub=nh.subscribe("joy", 1000, JoyCB);

  // 로봇의 동작모드를 리모트로 변경(프로토콜 문서 참조)
  g_blnIsProc=true;
  if(SendCommand(0x31)) // change mode
  {
    ROS_INFO("SendCommadn : OK");
  }
  else
  {
    ROS_INFO("SendCommadn : Fail");
  }
  g_blnIsProc=false;

  // ros 주기 설정
  ros::Rate loop_rate(10);

  while (ros::ok())
  {
    double vel1=0, vel2=0;
    std_msgs::Float32MultiArray vel;
    double _v,_w;

    if(g_TimeoutCnt<10)
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

    CheckBtnStatus();

    if((1-g_u8Mode)==0)
      vel_pub.publish(vel);

    /*
    _FLOAT_DATA floatData;
    floatData.fData=100.0;
    ROS_INFO("%.2X,%.2X,%.2X,%.2X",floatData.u8Data[0],floatData.u8Data[1],
        floatData.u8Data[2],floatData.u8Data[3]);*/

    ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;
}


bool SendCommand(uint8_t cmd)
{
  FLOAT_DATA float_data;
  albabot_msgs::CanMsg msg;
  bool blnIsCmd=false;
  bool bResult=false;
  msg.id=0x100;   // 현재버전에서는 의미없는 값임.

  switch(cmd)
  {
  case 0x00:
    msg.dlc=2;
    msg.data.push_back(0x00);
    msg.data.push_back(cmd);
    blnIsCmd=true;
    break;
  case 0xff:
    msg.dlc=2;
    msg.data.push_back(0x00);
    msg.data.push_back(cmd);
    blnIsCmd=true;
    break;
  case 0x01:  // Next
    msg.dlc=2;
    msg.data.push_back(0x00);
    msg.data.push_back(cmd);
    blnIsCmd=true;
    /*msg.dlc=5;
    msg.data.push_back(0x01); //type
    msg.data.push_back(0x01); //path 0x01:NEXT, 0x05
    msg.data.push_back(0x01);
    msg.data.push_back(0x01);
    msg.data.push_back(0x00);

    blnIsCmd=true;*/
    break;
  case 0x02:
    msg.dlc=2;
    msg.data.push_back(0x00);
    msg.data.push_back(cmd);
    blnIsCmd=true;
    break;
  case 0x03:
    msg.dlc=2;
    msg.data.push_back(0x00);
    msg.data.push_back(cmd);
    blnIsCmd=true;
    break;
  case 0x04:
    msg.dlc=2;
    msg.data.push_back(0x00);
    msg.data.push_back(cmd);
    blnIsCmd=true;
    /*msg.dlc=5;
    msg.data.push_back(0x01); //type
    msg.data.push_back(0x05); //path 0x01:NEXT, 0x05
    msg.data.push_back(0x05);
    msg.data.push_back(0x05);
    msg.data.push_back(0x00);
    blnIsCmd=true;*/
    break;
  case 0x31:
    msg.dlc=3;
    msg.data.push_back(0x00);
    msg.data.push_back(cmd);
    msg.data.push_back(g_u8Mode); //0x00 : remote / 0x01 : AGV mode
    g_u8Mode=1-g_u8Mode;
    blnIsCmd=true;
    break;
  case 0x40:
    float_data.fData=g_fCurSpeed;
    msg.dlc=6;
    msg.data.push_back(0x00);
    msg.data.push_back(cmd);
    msg.data.push_back(float_data.u8Data[0]);
    msg.data.push_back(float_data.u8Data[1]);
    msg.data.push_back(float_data.u8Data[2]);
    msg.data.push_back(float_data.u8Data[3]);
    blnIsCmd=true;
    break;
  case 0x41:  //회전속
    float_data.fData=g_fCurSpeed*0.3;
    msg.dlc=6;
    msg.data.push_back(0x00);
    msg.data.push_back(cmd);
    msg.data.push_back(float_data.u8Data[0]);
    msg.data.push_back(float_data.u8Data[1]);
    msg.data.push_back(float_data.u8Data[2]);
    msg.data.push_back(float_data.u8Data[3]);
    blnIsCmd=true;
    break;
  default:
    break;
  }

  if(blnIsCmd)
  {
    // 서비스 요청
    //g_Command.request.can_msg=msg;
    //bResult=g_ServiceClient.call(g_Command);
    g_command_pub.publish(msg);
  }
  bResult=true;
  return bResult;

}
ALBABOT_PARAM GetParameter(ros::NodeHandle *n)
{
  /*ALBABOT_PARAM albabotParam;
  std::string strParam;
  std::string strTemp;

  if (n->searchParam("cc_ip_addr",strParam))
  {
    n->getParam(strParam, albabotParam.IP);
  }
  else
  {
      ROS_INFO("CC IP address 파라미터를 찾을 수 없습니다.");
  }*/
}
void CheckBtnStatus(void)
{
  if(g_blnIsProc==false)
  {
  if(g_u8PreModeBtn==1 && g_u8ModeBtn==0)
  {
    g_blnIsProc=true;
    if(SendCommand(0x31))
    {
      ROS_INFO("Mode : OK");

    }
    else
    {
      ROS_INFO("Mode : Fail");
    }
    g_blnIsProc=false;
  }
  else if(g_u8PreNextBtn==1 && g_u8NextBtn==0)
  {
    g_blnIsProc=true;
    if(SendCommand(0xff))
    {
      ROS_INFO("clear : OK");
    }
    else
    {
      ROS_INFO("clear : Fail");
    }
    g_blnIsProc=false;

    g_blnIsProc=true;
    if(SendCommand(0x01))
    {
      ROS_INFO("Next : OK");
    }
    else
    {
      ROS_INFO("Next : Fail");
    }
    g_blnIsProc=false;

    if(SendCommand(0x00))
    {
      ROS_INFO("start : OK");
    }
    else
    {
      ROS_INFO("start : Fail");
    }
    g_blnIsProc=false;
  }
  else if(g_u8PreTurnBtn==1 && g_u8TurnBtn==0)
  {
    g_blnIsProc=true;
    if(SendCommand(0xff))
    {
      ROS_INFO("clear : OK");
    }
    else
    {
      ROS_INFO("clear : Fail");
    }
    g_blnIsProc=false;

    g_blnIsProc=true;
    if(SendCommand(0x04))
    {
      ROS_INFO("back : OK");
    }
    else
    {
      ROS_INFO("back : Fail");
    }
    g_blnIsProc=false;

    if(SendCommand(0x00))
    {
      ROS_INFO("start : OK");
    }
    else
    {
      ROS_INFO("start : Fail");
    }
    g_blnIsProc=false;
  }
  else if(g_u8PreLeftBtn==1 && g_u8LeftBtn==0)
  {
    g_blnIsProc=true;
    if(SendCommand(0x02))
    {
      ROS_INFO("Left: OK");
    }
    else
    {
      ROS_INFO("Left: Fail");
    }
    g_blnIsProc=false;
  }
  else if(g_u8PreRightBtn==1 && g_u8RightBtn==0)
  {
    g_blnIsProc=true;
    if(SendCommand(0x03))
    {
      ROS_INFO("Right: OK");
    }
    else
    {
      ROS_INFO("Right : Fail");
    }
    g_blnIsProc=false;
  }
  else if(g_u8PreSpeedUpBtn==0 && g_u8SpeedUpBtn==2)
  {
    g_blnIsProc=true;
    if(g_fCurSpeed<=3.2)
      g_fCurSpeed+=0.1;
    if(SendCommand(0x40))
    {
      ROS_INFO("up speed[%f]: OK",g_fCurSpeed);
    }
    else
    {
      ROS_INFO("up speed : Fail");
    }
    if(SendCommand(0x41))
    {
      ROS_INFO("up w speed[%f]: OK",g_fCurSpeed);
    }
    else
    {
      ROS_INFO("up w speed: Fail");
    }
    g_blnIsProc=false;
  }
  else if(g_u8PreSpeedUpBtn==0 && g_u8SpeedUpBtn==1)
  {
    g_blnIsProc=true;
    if(g_fCurSpeed>=0.1)
      g_fCurSpeed-=0.1;
    if(SendCommand(0x40))
    {
      ROS_INFO("down v speed[%f]: OK",g_fCurSpeed);
    }
    else
    {
      ROS_INFO("down v speed : Fail");
    }
    if(SendCommand(0x41))
    {
      ROS_INFO("down w speed[%f]: OK",g_fCurSpeed);
    }
    else
    {
      ROS_INFO("down w speed: Fail");
    }
    g_blnIsProc=false;
  }
  }
  g_u8PreNextBtn  =g_u8NextBtn;
  g_u8PreLeftBtn  =g_u8LeftBtn;
  g_u8PreRightBtn =g_u8RightBtn;
  g_u8PreTurnBtn  =g_u8TurnBtn;
  g_u8PreModeBtn  =g_u8ModeBtn;
  g_u8PreSpeedUpBtn = g_u8SpeedUpBtn;
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
  g_u8NextBtn=_joy->buttons[3];
  g_u8TurnBtn=_joy->buttons[0];
  g_u8LeftBtn=_joy->buttons[2];
  g_u8RightBtn=_joy->buttons[1];

  if(_joy->axes[5] > 0.5)
    g_u8SpeedUpBtn=2;
  else if(_joy->axes[5]<-0.5)
    g_u8SpeedUpBtn=1;
  else
    g_u8SpeedUpBtn=0;

  g_TimeoutCnt=0;
}
