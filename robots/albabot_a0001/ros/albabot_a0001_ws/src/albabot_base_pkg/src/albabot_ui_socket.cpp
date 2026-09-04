#include <ros/ros.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "albabot_msgs/RobotInfo.h"
#include <netinet/in.h>

#include "albabot_msgs/CanMsg.h"
//#include "albabot_msgs/Command.h"

bool Parse(char *pParsingBuf, uint32_t length, albabot_msgs::CanMsg *pMsg);
bool SetPathPlan(albabot_msgs::CanMsg *msg);
bool ProcessCommand(albabot_msgs::CanMsg *msg);


int g_fdSocket, g_fdNewSocket, g_nPortNum; //Socket file descriptors and port number
socklen_t g_Clilen; //object clilen of type socklen_t
char buffer[1024]; //buffer array of size 256
struct sockaddr_in g_ServerAddr, g_ClientAddr; ///two objects to store client and server address

//ros::ServiceClient g_ServiceClient;
//albabot_msgs::Command g_Command;

ros::Publisher g_command_pub;

uint8_t g_AgvStatus[5];

albabot_msgs::RobotInfo g_RobotInfo;
uint16_t g_hallData[1];
uint16_t g_sonicData[8];

bool g_blnIsProc=false;

void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value)
{
  g_AgvStatus[0]=value->agvStatus;
  g_AgvStatus[1]=value->agvMissionCount>>8;
  g_AgvStatus[2]=value->agvMissionCount;
  g_AgvStatus[3]=value->agvCurrentMission;
  g_AgvStatus[4]=value->agvDirection;

  g_RobotInfo.hall[0]=value->hall[0];
  for(int i=0;i<8;i++)
    g_RobotInfo.sonic[i]=value->sonic[i];
  g_RobotInfo.left_enc=value->left_enc;
  g_RobotInfo.right_enc=value->right_enc;
  g_RobotInfo.batVoltage=value->batVoltage;
  g_RobotInfo.agvStatus=value->agvStatus;
  g_RobotInfo.agvMissionCount=value->agvMissionCount;
  g_RobotInfo.agvCurrentMission=value->agvCurrentMission;
  g_RobotInfo.agvDirection=value->agvDirection;

  //memcpy(&g_RobotInfo, value, sizeof(albabot_msgs::RobotInfo));
  //ROS_INFO("%d,%d,%d,%d", value->agvStatus, value->agvMissionCount, value->agvCurrentMission, value->agvDirection);
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "albabot_ui_socket");
  ros::NodeHandle nh;
  bool blnIsConnected=false;

  // robot info 초기화
  g_RobotInfo.hall.push_back(0);
  g_RobotInfo.sonic.push_back(0);
  g_RobotInfo.sonic.push_back(0);
  g_RobotInfo.sonic.push_back(0);
  g_RobotInfo.sonic.push_back(0);
  g_RobotInfo.sonic.push_back(0);
  g_RobotInfo.sonic.push_back(0);
  g_RobotInfo.sonic.push_back(0);
  g_RobotInfo.sonic.push_back(0);
  //g_RobotInfo.sonic=g_sonicData;



  g_nPortNum = 3456;
  g_fdSocket = socket(AF_INET, SOCK_STREAM, 0);

  if (g_fdSocket < 0)
      ROS_INFO("ERROR opening socket");
  int enable = 1;

  if (setsockopt(g_fdSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
      ROS_INFO("setsockopt(SO_REUSEADDR) failed");

  bzero((char *) &g_ServerAddr, sizeof(g_ServerAddr));
  g_ServerAddr.sin_family = AF_INET;
  g_ServerAddr.sin_addr.s_addr = INADDR_ANY;
  g_ServerAddr.sin_port = htons(g_nPortNum);


  if (bind(g_fdSocket, (struct sockaddr *) &g_ServerAddr,
            sizeof(g_ServerAddr)) < 0)
            ROS_INFO("ERROR on binding");
  listen(g_fdSocket,5);
  g_Clilen = sizeof(g_ClientAddr);
  g_fdNewSocket = accept(g_fdSocket,(struct sockaddr *) &g_ClientAddr,&g_Clilen);
  if (g_fdNewSocket < 0)
  {
    ROS_INFO("ERROR on accept");
  }
  else {
    blnIsConnected=true;
    ROS_INFO("Connected!!!");
  }

    ros::Subscriber robotInfo_pub_=nh.subscribe("robot_info",1000, RobotInfoCB);
    g_command_pub = nh.advertise<albabot_msgs::CanMsg>("AlbabotMessage",1000);
    //g_ServiceClient = nh.serviceClient<albabot_msgs::Command>("AlbabotMessage");
    albabot_msgs::CanMsg msg;

    ros::Rate loop_rate(100);
    int nCount=0;

  while(ros::ok())
  {
    int n;
    //char result='0';
    bzero(buffer,1024);
    n = read(g_fdNewSocket,buffer,1024);
    if(n>0 && g_blnIsProc==false)
    {

      if(Parse(buffer,n,&msg) == true)
      {
        if(msg.data[0]==0x00)
        {
          g_blnIsProc=true;
          ProcessCommand(&msg);
          g_blnIsProc=false;
        }
        else
        {
          g_blnIsProc=true;
          SetPathPlan(&msg);
          g_blnIsProc=false;
        }
        msg.data.clear();
        nCount=0;
      }
    }
    else
    {
      if(nCount<600)
        nCount++;
      else
      {
        if(blnIsConnected==true)
        {

          // 기존 클라이언트 연결 해제
          close(g_fdNewSocket);
          blnIsConnected=false;
          ROS_INFO("disconnected!!");

          // 새 클라이언트 연결 대기 및 연결
          listen(g_fdSocket,5);
          g_Clilen = sizeof(g_ClientAddr);
          g_fdNewSocket = accept(g_fdSocket,(struct sockaddr *) &g_ClientAddr,&g_Clilen);
          if (g_fdNewSocket < 0)
          {
            ROS_INFO("ERROR on accept");
          }
          else {
            blnIsConnected=true;
            ROS_INFO("Connected!!!");
          }
        }

      }

    }
    loop_rate.sleep();
    ros::spinOnce();
  }

  if(close(g_fdSocket))
  {
    ROS_INFO("good bye");
  }
}

bool SetPathPlan(albabot_msgs::CanMsg *msg)
{
  bool result=false;
  char strTemp[9];
  char ss[2];
  std::string strTx="";


  albabot_msgs::CanMsg temp_msg;
  temp_msg=*msg;

  g_command_pub.publish(temp_msg);

  result=true;  
  strTx.push_back(';');
  sprintf(strTemp,"%.8x%.1x",temp_msg.id,temp_msg.dlc);
  strTx.append(strTemp);
  for(int i=0;i<temp_msg.dlc;i++)
  {
    sprintf(ss,"%.2x",temp_msg.data[i]);
    strTx.append(ss);
  }
  strTx.push_back('\r');
  write(g_fdNewSocket,strTx.c_str(),strTx.length());


  /*if(g_ServiceClient.call(g_Command))
  {
    result=true;
    strTx.push_back(';');
    sprintf(strTemp,"%.8x%.1x",g_Command.response.can_msg.id,g_Command.response.can_msg.dlc);
    strTx.append(strTemp);
    for(int i=0;i<g_Command.response.can_msg.dlc;i++)
    {
      sprintf(ss,"%.2x",g_Command.response.can_msg.data[i]);
      strTx.append(ss);
    }
    strTx.push_back('\r');
    write(g_fdNewSocket,strTx.c_str(),strTx.length());
  }
  else
  {
    strTx.push_back(';');
    sprintf(strTemp,"%.8x2%.2x00",g_Command.request.can_msg.id,g_Command.request.can_msg.data[0]);
    strTx.append(strTemp);
    strTx.push_back('\r');
    write(g_fdNewSocket,strTx.c_str(),strTx.length());
    result=false;
  }*/

  return result;
}

bool ProcessCommand(albabot_msgs::CanMsg *msg)
{
  uint32_t id=0x100;
  uint8_t dlc=8;
  uint8_t pData[8];
  char tempSS[256];

  //uint8_t dlcEx=256;
  //uint8_t pDataEx[256];

  albabot_msgs::CanMsg temp_msg;
  temp_msg=*msg;


  char strTemp[9];
  char ss[2];
  bool result=false;
  uint8_t u8Cmd=msg->data[1];

  std::string strTx="";

  switch(u8Cmd)
  {
  case 0x00:  // cmd : Start Mission
  case 0xff:  // cmd : Cancel Mission
  case 0x01:  // cmd : 수동(Next go)
  case 0x02:  // cmd : 수동(left turn)
  case 0x03:  // cmd : 수동(right turn)
  case 0x04:  // cmd : 수동(u-turn)
  case 0x05:  // cmd : 수동(back)
  case 0x30:  // cmd : hall2can calibration
  case 0x31:  // cmd : opMode
  case 0x78:  // set : AGV Dir
  case 0x40:  // set : AGV 이동속도
  case 0x41:  // set : AGV 회전속도
  case 0x42:  // set : AGV 가속도
  case 0x43:  // set : 제동거리 (m)
  case 0x44:  // set : 휠반지름 (m)
  case 0x45:  // set : 감속비
  case 0x46:  // set : 휠 간 거리 (m)
  case 0x47:  // set : dleay count ( 1=10ms)
  case 0x48:  // set : Gain p
  case 0x49:  // set : Gain i
  case 0x4a:  // set : Gain d
    ROS_INFO("ProcessCommand");

    g_command_pub.publish(temp_msg);

    //*msg=g_Command.response.can_msg;
    ROS_INFO("after service");

    strTx.push_back(';');
    temp_msg.dlc=3;
    temp_msg.data[2]=1;
    sprintf(strTemp,"%.8x%.1x",temp_msg.id,temp_msg.dlc);
    strTx.append(strTemp);
    for(int i=0;i<temp_msg.dlc;i++)
    {
      sprintf(ss,"%.2x",temp_msg.data[i]);
      strTx.append(ss);
    }
    strTx.push_back('\r');
    write(g_fdNewSocket,strTx.c_str(),strTx.length());



    break; 
  case 0x60:  // 로봇 ID 요청
    break;
  case 0x61:  // 배터리 요청
    break;
  case 0x62:  // 온도 요청
    break;
  case 0x63:  // 습도 요청
    break;
  case 0x64:  // AGV Status 요청
    strTx.push_back(';');
    sprintf(strTemp,"%.8x%.1x",id,dlc);
    strTx.append(strTemp);
    pData[0]=0x00;
    pData[1]=0x84;
    pData[2]=g_AgvStatus[0];
    pData[3]=g_AgvStatus[1];
    pData[4]=g_AgvStatus[2];
    pData[5]=g_AgvStatus[3];
    pData[6]=g_AgvStatus[4];
    pData[7]=0;
    for(int i=0;i<8;i++)
    {
      sprintf(ss,"%.2x",pData[i]);
      strTx.append(ss);
    }
    strTx.push_back('\r');
    write(g_fdNewSocket,strTx.c_str(),strTx.length());
    ROS_INFO("%s",strTx.c_str());

    break;
  case 0x65:
    //sprintf(tempSS,"+%d,%d,%x",g_RobotInfo.left_enc,g_RobotInfo.right_enc,g_RobotInfo.hall[0]);

    sprintf(tempSS,"+%lld,%lld,%x,%d,%d,%d,%d,%d,%d,%d,%d,%d,%x,%d,%d,%d,%d\r",
            g_RobotInfo.left_enc, g_RobotInfo.right_enc,
            g_RobotInfo.hall[0],
            g_RobotInfo.sonic[0],g_RobotInfo.sonic[1],g_RobotInfo.sonic[2],g_RobotInfo.sonic[3],
            g_RobotInfo.sonic[4],g_RobotInfo.sonic[5],g_RobotInfo.sonic[6],g_RobotInfo.sonic[7],
            g_RobotInfo.batVoltage, g_RobotInfo.gio, g_RobotInfo.agvMissionCount,
            g_RobotInfo.agvStatus,g_RobotInfo.agvCurrentMission, g_RobotInfo.agvDirection);
    
    

    strTx.append(tempSS);
    //ROS_INFO("%s",strTx.c_str());
   // strTx.push_back('\r');
    ROS_INFO("%s",tempSS);

    write(g_fdNewSocket,strTx.c_str(),strTx.length());
    break;
  default:
    break;
  }

  return result;
}

// 수신된 문자열을 통해 can message를 읽음.
bool Parse(char *pParsingBuf, uint32_t length, albabot_msgs::CanMsg *pMsg)
{
  bool result=false;
  char pTemp[2];
  char pTemp2[8];
  int nOffset=8;
  uint8_t u8DLC;

  // 메시지의 ID를 초기화. 이전값을 제거해줌.
  pMsg->id=0;

  // Get ID, DLC
  if(pParsingBuf[0]==';' && length>9)
  {
    sprintf(pTemp2,"%.8s",(char*)&(pParsingBuf[1]));
    pMsg->id=(uint32_t)strtoul(pTemp2,NULL,16);

    if(pParsingBuf[9]>='0' && pParsingBuf[9]<='8')
    {
      u8DLC=(uint8_t)(pParsingBuf[9]-'0');

      if((length == (u8DLC*2+11)) && pParsingBuf[u8DLC*2+10]=='\r')
      {
        if(u8DLC!=0)
        {
          for(int i=0;i<u8DLC;i++)
          {
            pTemp[0]=pParsingBuf[10+i*2];
            pTemp[1]=pParsingBuf[11+i*2];
            pMsg->data.push_back((uint8_t)strtoul(pTemp,NULL,16));
            //pMsg->data[i]=(uint8_t)strtoul(pTemp,NULL,16);
            ROS_INFO("%d: %c%c = %x",i,pTemp[0], pTemp[1], pMsg->data[i]);
          }
        }
        pMsg->dlc=u8DLC;

        ROS_INFO("RXData(%d) : id(%x), cmdType(%x), cmd(%x), dlc(%d)",length,pMsg->id,pMsg->data[0], pMsg->data[1], u8DLC);

        result=true;
      }
    }
  }
  return result;
}



