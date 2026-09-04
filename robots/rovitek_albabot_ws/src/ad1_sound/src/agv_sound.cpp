#include <ros/ros.h>
#include <sound_play/sound_play.h>
#include <std_msgs/Int32MultiArray.h>


double g_dbPeriod;
std::string g_strSoundHome;//="./albabot_ad1/sound/";
std::string g_strPlaylist[11]=
{    "Albabot_GuideVoice-01-excuseme.ogg",
     "Albabot_GuideVoice-02-heresfood.ogg",
     "Albabot_GuideVoice-03-arrivetrayreturn.ogg",
     "Albabot_GuideVoice-04-iamlost.ogg",
     "Albabot_GuideVoice-05-arrived.ogg",
     "Albabot_GuideVoice-06-needtocharge.ogg",
     "Albabot_GuideVoice-07-iamleaving.ogg",
     "Albabot_GuideVoice-08-pressnext.ogg",
     "Albabot_GuideVoice-09-leftturn.ogg",
     "Albabot_GuideVoice-10-rightturn.ogg",
     "Albabot_GuideVoice-11-u-turn.ogg"
};

int g_nPlayType=0;
int g_nPlayCount=0;


#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#include<iostream>

std::string GetCurrentWorkingDir( void ) {
  char buff[FILENAME_MAX];
  GetCurrentDir( buff, FILENAME_MAX );
  std::string current_working_dir(buff);
  return current_working_dir;
}



void SoundSelCb(const std_msgs::Int32MultiArray::ConstPtr& _msg)
{
  g_nPlayType=_msg->data[0];
  g_nPlayCount=_msg->data[1];

  ROS_INFO("%d,%d : %s", g_nPlayType, g_nPlayCount,GetCurrentWorkingDir().c_str());
          // (g_strSoundHome+g_strPlaylist[g_nPlayType-1]).c_str());
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "agv_sound");
  ros::NodeHandle nh("~");

  ros::Subscriber sc_pub=nh.subscribe("/sound_sel", 100, SoundSelCb);

  // parameter
  nh.param<double>("period",g_dbPeriod,5.0);
  ROS_INFO("period:%f",g_dbPeriod);
  nh.param<std::string>("sound_home",g_strSoundHome,"./albabot/sound/");
  ROS_INFO("sound_home:%s",g_strSoundHome.c_str());


  ROS_INFO("%s", GetCurrentWorkingDir().c_str());
  sound_play::SoundClient sc;

  ros::Time timestamp, last_time;
  double dt=0;


  ros::Rate loop_rate(20);  // 20hz ==

  while(ros::ok())
  {
    timestamp=ros::Time::now();
    dt += timestamp.toSec()-last_time.toSec();
    last_time = timestamp;

    if(g_nPlayType>=1 && g_nPlayType<=11)
    {
      if(g_nPlayCount>0)
      {
        g_nPlayCount--;
        sc.playWave((g_strSoundHome+g_strPlaylist[g_nPlayType-1]).c_str(),1.0);
      }
      else if(g_nPlayCount==-1 && dt>g_dbPeriod)
      {
        sc.playWave((g_strSoundHome+g_strPlaylist[g_nPlayType-1]).c_str(),1.0);
        dt=0;
      }
    }
    ros::spinOnce();
    loop_rate.sleep();
  }
  return 1;

}
