#include <ros/ros.h>
#include "sensor_msgs/Joy.h"
#include <std_msgs/Byte.h>
#include "albabot_msgs/CanMsg.h"
#include "albabot_msgs/RobotInfo.h"
#include <vector>

int g_nBtnIdxNext;
int g_nBtnIdxRight;
int g_nBtnIdxLeft;
int g_nBtnIdxTurn;
int g_nBtnIdxMode;
int g_nBtnIdxCancel;

int g_nAxisIdxV;
int g_nAxisIdxW;
int g_nAxisIdxScale;
int g_nAxisIdxAuxV;
int g_nAxisIdxAuxW;

ros::Publisher g_pubCommand;
ros::Publisher g_pubJoyBtn;

float g_joy_scale;
float g_joy_v;
float g_joy_w;

float g_joy_aux_scale;
float g_joy_aux_v;
float g_joy_aux_w;

uint8_t g_u8ModeBtn  ;
uint8_t g_u8NextBtn  ;
uint8_t g_u8RightBtn ;
uint8_t g_u8LeftBtn  ;
uint8_t g_u8TurnBtn  ;
uint8_t g_u8CancelBtn;

uint8_t g_u8PreModeBtn  ;
uint8_t g_u8PreNextBtn  ;
uint8_t g_u8PreRightBtn ;
uint8_t g_u8PreLeftBtn  ;
uint8_t g_u8PreTurnBtn  ;
uint8_t g_u8PreCancelBtn;

int g_TimeoutCnt;

void JoyCB(const sensor_msgs::Joy::ConstPtr& _joy);
void CheckButton();

int main(int argc, char **argv)
{
  ros::init(argc, argv, "agv_joystick");
  ros::NodeHandle nh("~");

  // get param
  nh.param<int>("next_btn_idx",g_nBtnIdxNext,3);
  nh.param<int>("mode_btn_idx",g_nBtnIdxMode,4);
  nh.param<int>("left_btn_idx",g_nBtnIdxLeft,2);
  nh.param<int>("right_btn_idx",g_nBtnIdxRight,1);
  nh.param<int>("turn_btn_idx",g_nBtnIdxTurn,0);
  nh.param<int>("cancel_btn_idx",g_nBtnIdxCancel,5);

  nh.param<int>("v_axis_idx",g_nAxisIdxV,1);
  nh.param<int>("w_axis_idx",g_nAxisIdxW,3);
  nh.param<int>("scale_idx",g_nAxisIdxScale,2);
  nh.param<int>("aux_v_axis_idx",g_nAxisIdxAuxV,5);
  nh.param<int>("aux_w_axis_idx",g_nAxisIdxAuxW,4);

  //ROS_INFO("period:%f",g_dbPeriod);

  ros::Subscriber joy_sub=nh.subscribe("/joy", 1000, JoyCB);
  //g_pubCommand = nh.advertise<albabot_msgs::CanMsg>("/AlbabotMessage",1000);
  g_pubJoyBtn=nh.advertise<std_msgs::Byte>("joyButton",100);

  ros::spin();

  return 1;

}


void JoyCB(const sensor_msgs::Joy::ConstPtr& _joy)
{
  // kiro joystick
  g_joy_scale=((1+_joy->axes[g_nAxisIdxScale])/2.0) * 3.3;
  g_joy_v = 1.0*g_joy_scale*_joy->axes[g_nAxisIdxV];
  g_joy_w = 1.0*g_joy_scale*_joy->axes[g_nAxisIdxW];

  g_joy_aux_scale=1.0;//((1+_joy->axes[g_nAxisIdxAuxScale])/2.0) * 3.3;
  g_joy_aux_v = 1.0*g_joy_aux_scale*_joy->axes[g_nAxisIdxAuxV];
  g_joy_aux_w = 1.0*g_joy_aux_scale*_joy->axes[g_nAxisIdxAuxW];
  // xbox joystick
  /*g_joy_scale=((1+_joy->axes[2])/2.0) * 3.3;
  g_joy_v = 1.0*g_joy_scale*_joy->axes[1];
  g_joy_w = 1.0*g_joy_scale*_joy->axes[0];*/

  g_u8ModeBtn=_joy->buttons[g_nBtnIdxMode];
  g_u8NextBtn=_joy->buttons[g_nBtnIdxNext];
  g_u8RightBtn=_joy->buttons[g_nBtnIdxRight];
  g_u8LeftBtn=_joy->buttons[g_nBtnIdxLeft];
  g_u8TurnBtn=_joy->buttons[g_nBtnIdxTurn];
  g_u8CancelBtn=_joy->buttons[g_nBtnIdxCancel];
  g_TimeoutCnt=0;

  CheckButton();

  g_u8PreModeBtn  = g_u8ModeBtn  ;
  g_u8PreNextBtn  = g_u8NextBtn  ;
  g_u8PreRightBtn = g_u8RightBtn ;
  g_u8PreLeftBtn  = g_u8LeftBtn  ;
  g_u8PreTurnBtn  = g_u8TurnBtn  ;
  g_u8PreCancelBtn= g_u8CancelBtn;

}

void CheckButton()
{
  uint8_t bBtnClicked=0x00;
  std_msgs::Byte txData;
  if(g_u8PreCancelBtn==1 && g_u8CancelBtn==0)
  {
    bBtnClicked=0x01;
  }
  else if(g_u8PreNextBtn==1 && g_u8NextBtn==0)
  {
    bBtnClicked=0x02;
  }
  else if(g_u8PreLeftBtn==1 && g_u8LeftBtn==0)
  {
    bBtnClicked=0x03;
  }
  else if(g_u8PreRightBtn==1 && g_u8RightBtn==0)
  {
    bBtnClicked=0x04;
  }
  else if(g_u8PreTurnBtn==1 && g_u8TurnBtn==0)
  {
    bBtnClicked=0x05;
  }
  else if(g_u8PreModeBtn==1 && g_u8ModeBtn==0)
  {
    bBtnClicked=0x06;
  }

  txData.data=bBtnClicked;
  g_pubJoyBtn.publish(txData);

  ROS_INFO("Btn : %d",bBtnClicked);

}
