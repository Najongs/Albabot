#include <ros/ros.h>
#include <std_msgs/String.h>
#include <sensor_msgs/Joy.h>
#include <std_msgs/Float32MultiArray.h>
#include "albabot_msgs/CanMsg.h"
#include "albabot_msgs/Command.h"

#define DISTANCE_WHEEL_TO_WHEEL   0.328

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

double g_joy_scale, g_joy_v, g_joy_w;
int g_TimeoutCnt=0;

ros::ServiceClient g_ServiceClient;
albabot_msgs::Command g_Command;
ros::Publisher g_command_pub;

// callbac : joystick
void JoyCB(const sensor_msgs::Joy::ConstPtr& _joy)
{
  // kiro joystick
  g_joy_scale=((1+_joy->axes[2])/2.0) * 3.3;
  g_joy_v = 1.0*g_joy_scale*_joy->axes[1];
  g_joy_w = 1.0*g_joy_scale*_joy->axes[3];

  // 무선조이스틱

  //g_Command.request.can_msg=msg;
  //blnIsCmd=g_ServiceClient.call(g_Command);

  /*ROS_INFO("result : %d, data = %d, %d, %d",g_Command.response.result,g_Command.response.can_msg.data[0],
          g_Command.response.can_msg.data[1],
          g_Command.response.can_msg.data[2]);*/

  ROS_INFO("result : %f, %f",g_joy_v,g_joy_w);


  g_TimeoutCnt=0;
}

bool SetMode()
{
    FLOAT_DATA float_data;
    albabot_msgs::CanMsg msg;
    bool blnIsCmd=false;
    msg.id=0x100;   // 현재버전에서는 의미없는 값임.
    msg.dlc=3;
    msg.data.push_back(0x00);
    msg.data.push_back(0x31);
    msg.data.push_back(0x00); //0x00 : remote / 0x01 : AGV mode

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

int main(int argc, char **argv)
{
    ros::init(argc, argv, "alpha1_joy");
    ros::NodeHandle nh;

    //g_ServiceClient = nh.serviceClient<albabot_msgs::Command>("AlbabotMessage");
    albabot_msgs::CanMsg msg;

    ros::Subscriber joy_sub=nh.subscribe("joy", 1000, JoyCB);
    ros::Publisher vel_pub = nh.advertise<std_msgs::Float32MultiArray>("cmd_vel",1000);
    g_command_pub = nh.advertise<albabot_msgs::CanMsg>("AlbabotMessage",1000);


    sleep(5);

    if(SetMode())
        ROS_INFO("Mode : OK");
    else
        ROS_INFO("Mode : Fail");

    // ros 주기 설정
    ros::Rate loop_rate(10);

    while (ros::ok())
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
      //SetMode();

      /*
      _FLOAT_DATA floatData;
      floatData.fData=100.0;
      ROS_INFO("%.2X,%.2X,%.2X,%.2X",floatData.u8Data[0],floatData.u8Data[1],
          floatData.u8Data[2],floatData.u8Data[3]);*/

      ros::spinOnce();
      loop_rate.sleep();
    }

}
