/*
driving_m1.cpp
- 조이스틱의 조작값을 받아 AD1의 조이스틱에 의한 동작.
- AGV / Remote mode를 조이스틱으로 설정
- AGV mode시 next, right, left, turn, cancel 등의 단위 동작 수행이 가능함.
- Remot mode시 조이스틱 값에 의해 v,w 값의 모터 좌우 구동값으로 변환하여 mainCon으로 보냄.
*/

#include <ros/ros.h>
#include "albabot_msgs/CanMsg.h"
#include "albabot_msgs/RobotInfo.h"
#include "agvteleop.h"

// for ros pub/sub
ros::Publisher g_pubCommand;
ros::Publisher g_pubVel;

// for control time
ros::Time timestamp, last_time;
double dt=0;

// agv's max velocity
float g_fAgvVelocity=1.6;
// agv's driving mode
uint8_t g_u8Opmode=OPMODE_AGV;
// joystick type
uint8_t g_u8JoyType=0;

float g_fAgvKp,g_fAgvKi,g_fAgvKd,g_fAgvGearRatio,g_fAgvW2WDist,g_fAgvAcc;

void GetParam(ros::NodeHandle *pNh)
{
  // parameter
  int nTemp=0;
  float fTemp=0;
  pNh->param<int>("joy_type",nTemp,0);
  g_u8JoyType=(uint8_t)nTemp;
  pNh->param<float>("agv_velocity",fTemp,1.6);
  g_fAgvVelocity=fTemp;
  pNh->param<float>("agv_kp",fTemp,0.2);
  g_fAgvKp=fTemp;
  pNh->param<float>("agv_ki",fTemp,0.0);
  g_fAgvKi=fTemp;
  pNh->param<float>("agv_kd",fTemp,4.0);
  g_fAgvKd=fTemp;
  pNh->param<float>("agv_gearratio",fTemp,50);
  g_fAgvGearRatio=fTemp;
  pNh->param<float>("agv_w2wDist",fTemp,0.338);
  g_fAgvW2WDist=fTemp;
  pNh->param<float>("agv_acc",fTemp,0.2);
  g_fAgvAcc=fTemp;

}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "driving_m1");
  ros::NodeHandle nh("~");

  GetParam(&nh);

  AGVTeleop agvTeleop(g_u8JoyType);

  // subuscribe
  ros::Subscriber joy_sub=nh.subscribe("/joy", 1000, &AGVTeleop::JoyCB, &agvTeleop);

  g_pubVel = nh.advertise<std_msgs::Float32MultiArray>("/cmd_vel",1000);
  g_pubCommand = nh.advertise<albabot_msgs::CanMsg>("/AlbabotMessage",1000);

  //agvTeleop.InitRobot(&g_pubCommand);
  agvTeleop.InitRobot(&g_pubCommand,g_u8Opmode,g_fAgvVelocity,g_fAgvKp,g_fAgvKi,g_fAgvKd,
                      g_fAgvGearRatio,g_fAgvW2WDist,g_fAgvAcc);

  g_fAgvVelocity=agvTeleop.GetAgvVelocity();

  ros::Rate loop_rate(20); //1.0/15.0);

  while(ros::ok())
  {
    agvTeleop.Publish(&g_pubVel,&g_pubCommand);
    //g_u8Opmode=agvTeleop.GetOpMode();

    ros::spinOnce();
    loop_rate.sleep();
  }
}




