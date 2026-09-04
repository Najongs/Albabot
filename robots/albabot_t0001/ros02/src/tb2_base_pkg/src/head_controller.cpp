#include <ros/ros.h>
#include <std_msgs/String.h>
#include <serial/serial.h>
#include <std_msgs/Float64MultiArray.h>
#include <sensor_msgs/Joy.h>

serial::Serial ser;
ros::Publisher read_pub;

uint8_t g_u8RobotId;
uint16_t g_u16Motor1Center=512;
uint16_t g_u16Motor2Center=456;
std::string g_strPort;

double g_joyVal1=0;
double g_joyVa12=0;

void JoyCB(const sensor_msgs::Joy::ConstPtr& _joy)
{
  // kiro joystick
  //m_dbJoyScale=((1+_joy->axes[2])/2.0) * 3.3;
  //m_dbJoyV = 1.0*m_dbJoyScale*_joy->axes[1];
  //m_dbJoyW = 1.0*m_dbJoyScale*_joy->axes[3];


    g_joyVal1=_joy->axes[6];
    g_joyVa12=_joy->axes[7];




    /*g_vel_v=_joy->axes[1];
    g_vel_w=_joy->axes[3];

    g_u32TimeoutCnt=0;*/
}

void rx_callback(const std_msgs::String::ConstPtr& msg){
    ROS_INFO_STREAM("Writing to serial port" << msg->data);
    //ser.write(msg->data);
}


void Parser(std::string str)
{
  std::string strSub;
  str.size();

  std_msgs::Float64MultiArray msg;

  if(str.at(0)=='$' && str.at(1)=='1')
  {
    strSub=str.substr(2,str.size()-2);
    double pData[6];
    int numElem=sscanf(strSub.c_str(),"%lf %lf %lf %lf %lf %lf",&pData[0],&pData[1],&pData[2],&pData[3],&pData[4],&pData[5]);
    //printf("%d : %s\n",numElem,strSub.c_str());
    if(numElem==6)
    {
      for(int i=0;i<numElem;i++)
        msg.data.push_back(pData[i]);

      printf("%lf,%lf,%lf,%lf,%lf,%lf\n",pData[0],pData[1],pData[2],pData[3],pData[4],pData[5]);
      read_pub.publish(msg);
    }
  }
}


void SendPacket(uint8_t id,uint8_t cmd, uint8_t *pParam, uint8_t length)
{
  uint8_t txData[64];
  uint8_t checksum=0;
  checksum+=id;
  checksum+=(length+2);
  checksum+=cmd;

  txData[0]=0xff;
  txData[1]=0xff;
  txData[2]=id;
  txData[3]=length+2;
  txData[4]=cmd;

  for(int i=0;i<length;i++)
  {
    txData[i+5]=pParam[i];
    checksum+=pParam[i];
  }

  txData[length+5]=~checksum;

  ser.write(txData,length+6);

}






void GetParam(ros::NodeHandle *pNh)
{
  // parameter
  int nTemp=0;

  pNh->param<int>("dxl1_centroid",nTemp,490);
  g_u16Motor1Center=(uint16_t)nTemp;

  pNh->param<int>("dxl2_centroid",nTemp,450);
  g_u16Motor2Center=(uint16_t)nTemp;

  pNh->param<std::string>("port",g_strPort,"/dev/ttyUSB-Head");



}


void SetPosition(uint8_t id, uint16_t pos)
{


    uint8_t txData[3];
    txData[0]=0x1e;
    txData[1]=pos;
    txData[2]=(pos)>>8;
    SendPacket(id,0x04,txData,3);
    SendPacket(id,0x05,txData,0);


}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "head_controller");
  ros::NodeHandle nh;

  ros::Subscriber rx_sub = nh.subscribe("rx_status", 1000, rx_callback);
  ros::Subscriber subJoy=nh.subscribe("/joy",1000, JoyCB);

  GetParam(&nh);

  //read_pub = nh.advertise<std_msgs::Float64MultiArray>("cart_info", 1000);


  try
  {
      ser.setPort("/dev/ttyUSB-Head");
      ser.setBaudrate(57600);
      serial::Timeout to = serial::Timeout::simpleTimeout(1000);
      ser.setTimeout(to);
      ser.open();

      SetPosition(1,g_u16Motor1Center);
      SetPosition(2,g_u16Motor2Center);

  }
  catch (serial::IOException& e)
  {
      ROS_ERROR_STREAM("Unable to open port ");
      return -1;
  }

  if(ser.isOpen()){
      ROS_INFO_STREAM("Serial Port initialized");
  }else{
      return -1;
  }


  ros::Rate loop_rate(5);


  uint8_t flag=1;
  uint16_t vel=100;
  int i;
  while(ros::ok())
  {
    flag=1-flag;


    if(g_joyVa12<-0.5)
    {
        if(g_u16Motor2Center>=(450-50))
            g_u16Motor2Center--;
    }
    else if(g_joyVa12>0.5)
    {
        if(g_u16Motor2Center<=(450+50))
           g_u16Motor2Center++;

    }

    if(g_joyVal1<-0.5)
    {
        if(g_u16Motor1Center>=(490-60))
            g_u16Motor1Center--;
    }
    else if(g_joyVal1>0.5)
    {
        if(g_u16Motor1Center<=(490+60))
           g_u16Motor1Center++;

    }

    if(flag==1)
        SetPosition(1,g_u16Motor1Center);
    else
        SetPosition(2,g_u16Motor2Center);

    ROS_INFO("head : %d, %d",g_u16Motor1Center,g_u16Motor2Center);

    //ROS_INFO("ID2 : %d",vel);


    /*flag=1-flag;
    uint8_t txData[2];
    txData[0]=0x19;
    txData[1]=flag;
    txPacket(0x01,0x03,txData,2);*/
    ros::spinOnce();
    loop_rate.sleep();
  }
}

