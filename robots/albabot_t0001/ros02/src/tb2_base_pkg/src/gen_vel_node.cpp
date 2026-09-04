#include <ros/ros.h>
#include <std_msgs/Float32MultiArray.h>
#include <geometry_msgs/Twist.h>
#include <sensor_msgs/Joy.h>

ros::Publisher g_pubVel;
uint32_t g_u32TimeoutCnt=0;

uint8_t g_u8InputType=0;//autonomous(0), manaul(1)


float g_vel_v;
float g_vel_w;

void CmdVelCB(const geometry_msgs::Twist::ConstPtr& value)
{
    float v,w;
    v=value->linear.x;
    w=value->angular.z;

    g_vel_v=v;
    g_vel_w=w;

    //g_u32TimeoutCnt=0;

}


void JoyCB(const sensor_msgs::Joy::ConstPtr& _joy)
{
  // kiro joystick
  //m_dbJoyScale=((1+_joy->axes[2])/2.0) * 3.3;
  //m_dbJoyV = 1.0*m_dbJoyScale*_joy->axes[1];
  //m_dbJoyW = 1.0*m_dbJoyScale*_joy->axes[3];

    g_vel_v=_joy->axes[1];
    g_vel_w=_joy->axes[3];

    g_u32TimeoutCnt=0;
}



int main(int argc, char **argv)
{
    ros::init(argc, argv, "gen_vel_node");
    ros::NodeHandle nh;

    ros::Subscriber subVel=nh.subscribe("/cmd_vel",1000, CmdVelCB);
    ros::Subscriber subJoy=nh.subscribe("/joy",1000, JoyCB);

    g_pubVel = nh.advertise<std_msgs::Float32MultiArray>("/motor_vel",1000);

    ros::Rate loop_rate(5);
    while(ros::ok())
    {
        /*if(g_u32TimeoutCnt<10)
        {
            g_u32TimeoutCnt++;
        }
        else
        {
            g_vel_v=0;
            g_vel_w=0;
        }*/

        std_msgs::Float32MultiArray vel;
        float vel1=0.0,vel2=0.0;
        float scale=1.5;

        vel1=-1.0*(g_vel_v*3.0-scale*g_vel_w)*(3.3/3.3);
        vel2=-1.0*(g_vel_v*3.0+scale*g_vel_w)*(3.3/3.3);

        vel.data.push_back(vel1);
        vel.data.push_back(vel2);

        g_pubVel.publish(vel);
        ROS_INFO("vel(%f, %f), motor(%f, %f)", g_vel_v,g_vel_w,vel1,vel2);


        ros::spinOnce();
        loop_rate.sleep();

    }
}
