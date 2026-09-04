#include <ros/ros.h>
#include <sound_play/sound_play.h>
#include "sensor_msgs/Range.h"
#include "albabot_msgs/RobotInfo.h"


void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value)
{

  ROS_INFO("Enc:(%.8d, %.8d), Hall : %.4x, Obstacle : %d, AGV Status:%d, Mission : %d/%d, ",
           value->left_enc, value->right_enc,value->hall[0], value->sonic[0],value->agvStatus,
           value->agvCurrentMission,value->agvMissionCount);
}


int main(int argc, char **argv)
{
  ros::init(argc, argv, "ad1_sound");
  ros::NodeHandle nh;
  ros::Subscriber robotInfo_sub_=nh.subscribe("robot_info",1000, RobotInfoCB);


  sound_play::SoundClient sc;
  //sound_play::Sound s1=sc.waveSound("/home/swpark/mywork/rovitek_albabot_ws/src/albabot_ad1_pkg/sound/jejufoodshow.ogg", 1.0f);

  sleep(1);  //soundplay_node.py 실행될 때까지 필요한 시간

  //sc.repeat("/home/swpark/mywork/rovitek_albabot_ws/src/albabot_ad1_pkg/sound/jejufoodshow.ogg", 1.0f);



  //sc.playWave("/home/swpark/mywork/rovitek_albabot_ws/src/albabot_ad1_pkg/sound/jejufoodshow.ogg", 1.0f);

  ros::Rate loop_rate(1.0/15.0);

  while(ros::ok())
  {
    //s1.repeat();
    //sc.playWave("/home/swpark/mywork/rovitek_albabot_ws/src/albabot_ad1_pkg/sound/jejufoodshow.ogg", 1.0f);
    sc.playWave("/opt/robot_alpha/sound/jejufoodshow.ogg", 1.0f);
    ros::spinOnce();
    loop_rate.sleep();

    //sleep(30);
    //
  }
  //sc.stopAll();
  //s1.stop();

  ROS_INFO("Hello world!");
}
