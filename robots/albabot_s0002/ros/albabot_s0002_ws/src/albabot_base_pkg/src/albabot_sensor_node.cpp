#include <ros/ros.h>
#include "tf2/LinearMath/Quaternion.h"
#include "std_msgs/Int64MultiArray.h"
#include "std_msgs/UInt64MultiArray.h"
#include "geometry_msgs/PoseStamped.h"
#include "sensor_msgs/Range.h"
#include "albabot_msgs/RobotInfo.h"

#include "tf/tf.h"
#include "tf/transform_broadcaster.h"

#include "nav_msgs/Odometry.h"


//#define DISTANCE_WHEEL_TO_WHEEL   0.328


bool g_blnIsFirstIn=false;
double g_PosX=0, g_PosY=0, g_Theta=0;
uint16_t g_hallData;
uint16_t g_sonicData[8];

float g_fAgvGearRatio,g_fAgvW2WDist;

void GetParam(ros::NodeHandle *pNh);

void getOdometry(int64_t leftVal, int64_t rightVal, double* pVel_v, double* pVel_w)
{
  // 모터의 각 바퀴 회전속도로 부터 전진속도 및 회전속도 계산
  double v,w;
  double GearRatio=g_fAgvGearRatio, Pole=10;
  double vel1,vel2;

  vel1=(3.141592*2*0.1)*(leftVal/(GearRatio*Pole*2));
  //vel2=-1.0*(3.141592*2*0.1)*(rightVal/(GearRatio*Pole));
  vel2=1.0*(3.141592*2*0.1)*(rightVal/(GearRatio*Pole*2));

  v=(vel1+vel2)/2;
  w=(vel2-vel1)/(1.0*g_fAgvW2WDist);

  double delta_x, delta_y;

  delta_x = cos(g_Theta)*v * 1.0;//cos(g_fTheta)*vel_v * 1.0;
  delta_y = sin(g_Theta)*v * 1.0;


  g_PosX += delta_x;
  g_PosY += delta_y;
  g_Theta += (w*1);

  *pVel_v=v;
  *pVel_w=w;
}

tf::Transform transform;
tf::Quaternion q;

ros::Time last_time;
ros::Time timestamp;

ros::Publisher odom_pub;

void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value)
{

  int64_t leftEnc_, rightEnc_;

  static int64_t pre_left=0;
  static int64_t pre_right=0;

  double v,w,dt;

  timestamp=ros::Time::now();
  dt = timestamp.toSec()-last_time.toSec();
  last_time = timestamp;

  if(g_blnIsFirstIn == false)
  {

    g_PosX=0;
    g_PosY=0;
    g_Theta=0;

    pre_left=value->left_enc;
    pre_right=value->right_enc;
    g_blnIsFirstIn=true;
  }
  else
  {
    leftEnc_=value->left_enc-pre_left;
    rightEnc_=(value->right_enc-pre_right);

    pre_left=value->left_enc;
    pre_right=(value->right_enc);

    getOdometry(leftEnc_, rightEnc_,&v,&w);

    nav_msgs::Odometry odom_msg;

    q.setRPY(0,0.0,g_Theta);
    odom_msg.header.stamp = timestamp;
    odom_msg.header.frame_id = "odom";
    //set the position
    odom_msg.pose.pose.position.x = g_PosX;
    odom_msg.pose.pose.position.y = g_PosY;
    odom_msg.pose.pose.position.z = 0.0;
    odom_msg.pose.pose.orientation.x = q.x();
    odom_msg.pose.pose.orientation.y = q.y();
    odom_msg.pose.pose.orientation.z = q.z();
    odom_msg.pose.pose.orientation.w = q.w();
    //set the velocity
    odom_msg.child_frame_id = "base_footprint";
    odom_msg.twist.twist.linear.x = v/dt;
    odom_msg.twist.twist.linear.y = 0;
    odom_msg.twist.twist.angular.z = w/dt;

    odom_pub.publish(odom_msg);

    static tf::TransformBroadcaster br;

    transform.setOrigin(tf::Vector3(g_PosX,g_PosY,0));
    //q.setRPY(0,0.0,g_Theta);
    transform.setRotation(q);
    br.sendTransform(tf::StampedTransform(transform, timestamp, "odom", "base_footprint"));

  }

  g_hallData=value->hall[0];
  for(int i=0;i<8;i++)
    g_sonicData[i]=value->sonic[i];

  ROS_INFO("%d, %d, %.4x, %d, %d, %d, %d, %d, %d, %d, %d",value->left_enc, value->right_enc,
              g_hallData, g_sonicData[0], g_sonicData[1], g_sonicData[2], g_sonicData[3],
              g_sonicData[4], g_sonicData[5], g_sonicData[6], g_sonicData[7]);
}

void GetParam(ros::NodeHandle *pNh)
{
  // parameter
  int nTemp=0;
  float fTemp=0;

  pNh->param<float>("agv_gearratio",fTemp,30);
  g_fAgvGearRatio=fTemp;
  pNh->param<float>("agv_w2wDist",fTemp,0.375);
  g_fAgvW2WDist=fTemp;

}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "albabot_sensor_node");
  ros::NodeHandle nh;

  GetParam(&nh);

  //ros::Subscriber encValue_pub_=nh.subscribe("encValue_status",1000, EncoderValueCB);
  ros::Subscriber robotInfo_pub_=nh.subscribe("robot_info",1000, RobotInfoCB);

  odom_pub = nh.advertise<nav_msgs::Odometry>("robot_pose",1000);

  // publish rate 20hz
  ros::Rate loop_rate(20);
  // loop
  while( ros::ok() )
  {
    

    /*sensor_msgs::Range sonar_msg;

    sonar_msg.header.stamp=ros::Time::now();
    sonar_msg.header.frame_id="base_sonic";
    sonar_msg.radiation_type=sensor_msgs::Range::ULTRASOUND;
    sonar_msg.field_of_view=(30.0/180.0)*3.141592;
    sonar_msg.range=g_sonicData[1];
    sonar1_pub.publish(sonar_msg);

    sonar_msg.range=g_sonicData[3];
    sonar2_pub.publish(sonar_msg);*/

    ros::spinOnce();
    loop_rate.sleep();
  }
  return 0;

}
