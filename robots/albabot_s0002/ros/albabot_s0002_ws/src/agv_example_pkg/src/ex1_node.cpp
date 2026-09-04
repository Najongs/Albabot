#include <ros/ros.h>
#include "albabot_msgs/CanMsg.h"
#include "albabot_msgs/RobotInfo.h"

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



ros::Publisher g_pubCommand;

void SetRobotValue(uint8_t cmd, float fData);
void SetRobotOpMode(uint8_t opmode);
void ClearCommand();

void GetParam(ros::NodeHandle *pNh);

float g_fAgvGearRatio;
float g_fAgvW2WDist;

void GetParam(ros::NodeHandle *pNh)
{
  // parameter
  int nTemp=0;
  float fTemp=0;

  pNh->param<float>("agv_gearratio",fTemp,30);
  g_fAgvGearRatio=fTemp;
  pNh->param<float>("agv_w2wDist",fTemp,0.338);
  g_fAgvW2WDist=fTemp;

}

void InitRobot()
{
  albabot_msgs::CanMsg msg;

  // 모드 설정(1:AGV)
  //SetRobotOpMode(0x01);

  // 기존 명령어 취소
  //ClearCommand();

  // velocity
  //SetRobotValue(0x40,1.6);
  //kp
  //SetRobotValue(0x48,0.2);
  //ki
  //SetRobotValue(0x49,0.0);
  //kd
  //SetRobotValue(0x4a,4.0);

  //gear ratio
  SetRobotValue(0x45,g_fAgvGearRatio);

  // wheel to wheel dist.
  SetRobotValue(0x46,g_fAgvW2WDist);

}


void SetRobotValue(uint8_t cmd, float fData)
{
    albabot_msgs::CanMsg msg;

    // velocity
    FLOAT_DATA float_data;
    float_data.fData=fData;
    msg.dlc=6;
    msg.data.push_back(0x00);
    msg.data.push_back(cmd);
    msg.data.push_back(float_data.u8Data[0]);
    msg.data.push_back(float_data.u8Data[1]);
    msg.data.push_back(float_data.u8Data[2]);
    msg.data.push_back(float_data.u8Data[3]);
    g_pubCommand.publish(msg);
    ros::spinOnce();
    usleep(1000*1000);
    msg.data.clear();
}

///////////////////////////////////////////////////////////////////
void SetRobotOpMode(uint8_t opmode)
{
    albabot_msgs::CanMsg msg;

    // 모드 설정
    msg.id=0x100;
    msg.dlc=3;
    msg.data.push_back(0x00);
    msg.data.push_back(0x31);
    msg.data.push_back(opmode); //0x00 : remote / 0x01 : AGV mode
    g_pubCommand.publish(msg);
    ros::spinOnce();
    usleep(1000*1000);
    //msg.data.clear();
}

void ClearCommand()
{
    albabot_msgs::CanMsg msg;

    // 기존 명령어 취소
    msg.id=0x100;
    msg.dlc=2;
    msg.data.push_back(0x00);
    msg.data.push_back(0xff);
    g_pubCommand.publish(msg);
    ros::spinOnce();
    sleep(1);
    msg.data.clear();
}
/////////////////////////////////////////////////////////////////////

// cmd 0x00 : start
// cmd 0x01 : next
// cmd 0x02 : left
// cmd 0x03 : right
// cmd 0x04 : turn
// protocol 문서 참고
void SetCommand(uint8_t cmd)
{
    albabot_msgs::CanMsg msg;

    msg.id=0x100;
    msg.dlc=2;
    msg.data.push_back(0x00);
    msg.data.push_back(cmd);
    g_pubCommand.publish(msg);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "ex1_node");
    ros::NodeHandle nh;

    GetParam(&nh);

    g_pubCommand = nh.advertise<albabot_msgs::CanMsg>("/AlbabotMessage",1000);


    InitRobot();

    ros::Rate loop_rate(10);
    int nCount=0;
    while(ros::ok())
    {
        /*if(nCount==0)
        {
            SetCommand(0x02);
            ROS_INFO("Left");
        }
        else if(nCount==50)
        {
            SetCommand(0x03);
            ROS_INFO("Right");
        }

        if(nCount<100-1)
        {
            nCount++;
        }
        else {
            nCount=0;
        }*/


        ros::spinOnce();
        loop_rate.sleep();


    }


}
