#ifndef AGVTELEOP_H
#define AGVTELEOP_H

#include <ros/ros.h>
#include <sensor_msgs/Joy.h>
#include "albabot_msgs/CanMsg.h"
#include <std_msgs/Float32MultiArray.h>

#define OPMODE_REMOTE 0
#define OPMODE_AGV    1

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


class AGVTeleop
{
public:
  AGVTeleop();
  AGVTeleop(uint8_t joyType);
  AGVTeleop(ros::NodeHandle* nodehandle);
  void InitRobot(ros::Publisher *pubCmd, uint8_t opmode=0x01,
                            float agvVelocity=1.6, float kp=0.2, float ki=0.0, float kd=4.0,
                            float gearratio=50.0, float w2wDist=0.338, float acc=3.3);
  // callback
  void JoyCB(const sensor_msgs::Joy::ConstPtr& _joy);
  // joystick to velocity
  std_msgs::Float32MultiArray GetVelocity();
  // set agv op mode;
  void SetOpMode(uint8_t opMode);
  // get agv op mode;
  uint8_t GetOpMode();
  // check joystick button
  uint8_t CheckButton();
  void Publish(ros::Publisher *pubVel, ros::Publisher *pubCmd);
  float GetAgvVelocity();
private:

  ros::NodeHandle nh_;
  uint8_t m_u8OpMode;
  uint32_t m_u32TimeoutCnt;
  double m_dbJoyScale;
  double m_dbJoyV;
  double m_dbJoyW;

  float m_fAgvSpeed;

  uint8_t m_u8JoyType;

  // joystick button
  uint8_t m_u8PreCancelBtn=0;
  uint8_t m_u8PreNextBtn=0;
  uint8_t m_u8PreLeftBtn=0;
  uint8_t m_u8PreRightBtn=0;
  uint8_t m_u8PreTurnBtn=0;
  uint8_t m_u8PreModeBtn=0;
  uint8_t m_u8PreSpeedUpBtn=0;

  uint8_t m_u8CancelBtn=0;
  uint8_t m_u8NextBtn=0;
  uint8_t m_u8LeftBtn=0;
  uint8_t m_u8RightBtn=0;
  uint8_t m_u8TurnBtn=0;
  uint8_t m_u8ModeBtn=0;
  uint8_t m_u8SpeedUpBtn=0;

};


#endif // AGVTELEOP_H
