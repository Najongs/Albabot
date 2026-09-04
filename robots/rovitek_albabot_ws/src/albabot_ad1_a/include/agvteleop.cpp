#include "agvteleop.h"

AGVTeleop::AGVTeleop()
{
  m_u8OpMode=OPMODE_AGV;
  m_u32TimeoutCnt=0;
  m_fAgvSpeed=1.6;
}

AGVTeleop::AGVTeleop(ros::NodeHandle* nodehandle):nh_(*nodehandle)
{
  m_u8OpMode=OPMODE_AGV;
  m_u32TimeoutCnt=0;
  m_fAgvSpeed=1.6;
}

void AGVTeleop::SetOpMode(uint8_t opMode)
{
  m_u8OpMode=opMode;
}

uint8_t AGVTeleop::GetOpMode()
{
  return m_u8OpMode;
}

#include "albabot_msgs/CanMsg.h"

void AGVTeleop::Publish(ros::Publisher *pubVel, ros::Publisher *pubCmd)
{

  std_msgs::Float32MultiArray vel;
  albabot_msgs::CanMsg msg;
  FLOAT_DATA float_data;
  uint8_t u8Status=0;

  vel=GetVelocity();
  if(m_u8OpMode==OPMODE_REMOTE)
  {
    ROS_INFO("vel : %f, %f", vel.data[0],vel.data[1]);
    pubVel->publish(vel);
  }

  u8Status=CheckButton();
  switch(u8Status)
  {
    case 0:   // No operation
      break;
    case 0x31:   // Op Mode
      m_u8OpMode=1-m_u8OpMode;
      msg.id=0x100;
      msg.dlc=3;
      msg.data.push_back(0x00);
      msg.data.push_back(0x31);
      msg.data.push_back(m_u8OpMode);
      pubCmd->publish(msg);

      ROS_INFO("Cur Mode : %d",m_u8OpMode);
      break;
    case 0x01:  // next
      msg.id=0x100;
      msg.dlc=2;
      msg.data.push_back(0x00);
      msg.data.push_back(0x01);
      pubCmd->publish(msg);
      break;
    case 0x02:  // left turn
      msg.id=0x100;
      msg.dlc=2;
      msg.data.push_back(0x00);
      msg.data.push_back(0x02);
      pubCmd->publish(msg);
      break;
    case 0x03:  // left turn
      msg.id=0x100;
      msg.dlc=2;
      msg.data.push_back(0x00);
      msg.data.push_back(0x03);
      pubCmd->publish(msg);
      break;
    case 0x04:  // u-turn
      msg.id=0x100;
      msg.dlc=2;
      msg.data.push_back(0x00);
      msg.data.push_back(0x04);
      pubCmd->publish(msg);
      break;
    case 0x40:
      float_data.fData=m_fAgvSpeed;
      msg.dlc=6;
      msg.data.push_back(0x00);
      msg.data.push_back(0x40);
      msg.data.push_back(float_data.u8Data[0]);
      msg.data.push_back(float_data.u8Data[1]);
      msg.data.push_back(float_data.u8Data[2]);
      msg.data.push_back(float_data.u8Data[3]);
      pubCmd->publish(msg);
      break;
    default:
      break;
  }

}

float AGVTeleop::GetAgvVelocity()
{
  return m_fAgvSpeed;
}



std_msgs::Float32MultiArray AGVTeleop::GetVelocity()
{
  double dbWheel2WheelDistance=0.328;
  double vel1=0, vel2=0;
  std_msgs::Float32MultiArray vel;
  double _v,_w;

  if(m_u32TimeoutCnt<10)   // 1000ms
  {
    m_u32TimeoutCnt++;
  }

  if(m_u32TimeoutCnt>10)
  {
    _v=0.0;
    _w=0.0;
  }
  else
  {
    _v=m_dbJoyV;
    _w=m_dbJoyW;
  }

  vel1=-1.0*(_v-dbWheel2WheelDistance*_w )*(3.3/3.3);
  vel2=-1.0*(_v+dbWheel2WheelDistance*_w )*(3.3/3.3);

  vel.data.push_back(vel1);
  vel.data.push_back(vel2);

  return vel;
}

void AGVTeleop::JoyCB(const sensor_msgs::Joy::ConstPtr& _joy)
{
  // kiro joystick
  m_dbJoyScale=((1+_joy->axes[2])/2.0) * 3.3;
  m_dbJoyV = 1.0*m_dbJoyScale*_joy->axes[1];
  m_dbJoyW = 1.0*m_dbJoyScale*_joy->axes[3];

  // 무선조이스틱

  //g_Command.request.can_msg=msg;
  //blnIsCmd=g_ServiceClient.call(g_Command);

  /*ROS_INFO("result : %d, data = %d, %d, %d",g_Command.response.result,g_Command.response.can_msg.data[0],
          g_Command.response.can_msg.data[1],
          g_Command.response.can_msg.data[2]);*/

  //ROS_INFO("result : %f, %f",g_joy_v,g_joy_w);

  m_u8ModeBtn=_joy->buttons[4];
  m_u8NextBtn=_joy->buttons[3];
  m_u8TurnBtn=_joy->buttons[0];
  m_u8LeftBtn=_joy->buttons[2];
  m_u8RightBtn=_joy->buttons[1];

  if(_joy->axes[5] > 0.5)
    m_u8SpeedUpBtn=2;
  else if(_joy->axes[5]<-0.5)
    m_u8SpeedUpBtn=1;
  else
    m_u8SpeedUpBtn=0;

  m_u32TimeoutCnt=0;
}


uint8_t AGVTeleop::CheckButton(void)
{
  uint8_t u8Status=0;
  if(m_u8PreModeBtn==1 && m_u8ModeBtn==0)
  {
    u8Status=0x31;
    ROS_INFO("Mode : OK");
  }
  else if(m_u8PreNextBtn==1 && m_u8NextBtn==0)
  {
    u8Status=0x01;
    ROS_INFO("Next : OK");
  }
  else if(m_u8PreTurnBtn==1 && m_u8TurnBtn==0)
  {
    u8Status=0x04;
    ROS_INFO("u-turn : OK");
  }
  else if(m_u8PreLeftBtn==1 && m_u8LeftBtn==0)
  {
    u8Status=0x02;
    ROS_INFO("Left: OK");
  }
  else if(m_u8PreRightBtn==1 && m_u8RightBtn==0)
  {
    u8Status=0x03;
    ROS_INFO("Right: OK");
  }
  else if(m_u8PreSpeedUpBtn==0 && m_u8SpeedUpBtn==2)
  {
    if(m_fAgvSpeed<=3.2)
      m_fAgvSpeed+=0.1;
    else {
      m_fAgvSpeed=3.3;
    }
    u8Status=0x40;
    ROS_INFO("up speed[%f]: OK",m_fAgvSpeed);

  }
  else if(m_u8PreSpeedUpBtn==0 && m_u8SpeedUpBtn==1)
  {
    if(m_fAgvSpeed>=0.1)
      m_fAgvSpeed-=0.1;
    else {
      m_fAgvSpeed=0.0;
    }
    u8Status=0x40;
    ROS_INFO("down speed[%f]: OK",m_fAgvSpeed);
  }
  m_u8PreNextBtn  =m_u8NextBtn;
  m_u8PreLeftBtn  =m_u8LeftBtn;
  m_u8PreRightBtn =m_u8RightBtn;
  m_u8PreTurnBtn  =m_u8TurnBtn;
  m_u8PreModeBtn  =m_u8ModeBtn;
  m_u8PreSpeedUpBtn = m_u8SpeedUpBtn;

  return u8Status;
}
