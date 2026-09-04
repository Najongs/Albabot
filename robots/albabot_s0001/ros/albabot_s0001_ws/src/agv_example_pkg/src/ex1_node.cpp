#include <ros/ros.h>
#include "albabot_msgs/CanMsg.h"
#include "albabot_msgs/RobotInfo.h"

ros::Publisher g_pubCommand;


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

    g_pubCommand = nh.advertise<albabot_msgs::CanMsg>("/AlbabotMessage",1000);


    ClearCommand();
    SetRobotOpMode(0x01);

    ros::Rate loop_rate(10);
    int nCount=0;
    while(ros::ok())
    {
        if(nCount==0)
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
        }


        ros::spinOnce();
        loop_rate.sleep();


    }


}
