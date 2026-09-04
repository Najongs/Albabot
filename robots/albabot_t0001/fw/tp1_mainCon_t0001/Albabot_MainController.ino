#include <ros.h>
#include <ros/time.h>
#include <geometry_msgs/Twist.h>
#include <tf/tf.h>
#include <tf/transform_broadcaster.h>
#include <std_msgs/UInt16.h>
#include <std_msgs/UInt8.h>
#include <std_msgs/UInt64MultiArray.h>
#include <std_msgs/Int64MultiArray.h>
#include <std_msgs/Float32MultiArray.h>
#include <nav_msgs/Odometry.h>
#include <sensor_msgs/Range.h>

#include <albabot_msgs/CanMsg.h>
#include <albabot_msgs/Command.h>
#include <albabot_msgs/RobotInfo.h>

#include <string>
#include "define.h"
#include "BLDCMotor2.h"
#include <HardwareCAN.h>
#include "AlbabotAGV.h"

//using albabot_ad2_pget_messagekg::command_service;
//using albabot_msgs::can_msg;

#define ALRAM_PIN_IG1       PA2
#define ALRAM_PIN_RL        PC8
#define ALRAM_PIN_BL1       PB10
#define ALRAM_PIN_BL2       PB11

void SetAlarm(u8 value);
void InitAlarm();


// ROS : handle
ros::NodeHandle  nh;

// ROS : publish => odometry
double g_fPosX = 0.0;
double g_fPosY = 0.0;
double g_fPosZ1 = 0.0;
double g_fPosZ2 = 0.0;

double vx=0.0, vy=0.0, vth=0.0;
double g_fTheta = 0.0;


//std_msgs::Float32MultiArray encValue_msg;
//ros::Publisher encValue_pub("enc_value", &encValue_msg);

// ROS service
void Obstacle_cb( const std_msgs::Float32MultiArray& msg); // callback 
void GetMessageCB(const albabot_msgs::CanMsg &msg);
/*ros::ServiceServer<albabot_msgs::Command::Request, albabot_msgs::Command::Response> 
        srvServerRxMessage("AlbabotMessage",&GetMessageCB);*/

ros::Subscriber<albabot_msgs::CanMsg> sub_Command("AlbabotMessage", GetMessageCB);        




//void rx_message_cb(const command_service::Request &req, command_service::Response &res);
//ros::ServiceServer<command_service::Request, command_service::Response> serverRXMessage("get_message",&rx_message_cb);

// ROS : publish => robot status : hall2can(2ch), sonic1 mc (4 sonics, 1ch), sonic2 mc (4 sonics, 1ch)
//uint64_t g_pRobotStatusData[3];
//std_msgs::UInt64MultiArray robot_status_msg;
//ros::Publisher pub_robot_status("robot_status", &robot_status_msg);

albabot_msgs::RobotInfo robotInfoMsg;
ros::Publisher pub_robot_info("robot_info", &robotInfoMsg);

//std_msgs::Int64MultiArray encValue_msg;
//ros::Publisher pub_encValue("encValue_status", &encValue_msg);


// ROS : subscribe => set bldc motor' velocity
void Velocity_cb( const std_msgs::Float32MultiArray& cmd_msg); // callback 
void Obstacle_cb( const std_msgs::Float32MultiArray& msg); // callback 

ros::Subscriber<std_msgs::Float32MultiArray> sub_velocity("motor_vel", Velocity_cb);
ros::Subscriber<std_msgs::Float32MultiArray> sub_obstacle("obstacle", Obstacle_cb);

// Albabot AD2~(AGV 기능)
CAlbabotAGV g_AlbabotAgv;
//AGV_PARAM g_AGVParam;

u8 g_u8OpMode=OPMODE_REMOTE;
void RemoteModeOperation();
void AGVModeOperation();

// 모터드라이버 구동 클래스
CBLDCMotor2 g_MainController;
float g_pVelocity[2]={0.0,0.0};
float g_pMotorOutput[2]={0.0,0.0};

int64_t g_i64EncValue[2]={0,0};

uint16_t g_pHall2CANData[2];
uint16_t g_pSonic2CANData[8]={100,100,100,100,};

// lift controller 
float g_fLiftVel1;
float g_fLiftVel2;

int64_t g_pEncoderVal[4]; // Encoder pos
float g_pDeltaPos[2];     // delta Pos

// 내부 타이머
HardwareTimer g_timer4(4);     

// for CAN communication 
HardwareCAN g_CANBus(CAN1_BASE);
CAN_TX_MBX CANsend(CanMsg *pmsg);

// init function
void InitRobot(void);
void InitTimer(void);
void StartROS(void);

ros::Time current_time, last_time;


u32 g_u32HeartbeatCount=0;
u8 g_u8IsOperating=0;


void setup()
{  
  InitTimer();
  InitCAN();
  InitRobot();
  startRos();
}

void loop()
{ 
  if(g_u8OpMode==OPMODE_AGV)
    AGVModeOperation();
  else if(g_u8OpMode==OPMODE_REMOTE)
    RemoteModeOperation();
}


void AGVModeOperation()
{
  static int ncount=0;
  // Velocity_cb함수가 호출되면 g_u32HeartbeatCount를 0으로 초기화. 
  // 속도값이 전송되지 않으면 동작을 멈춘다.
  float fVel[2];

  float tempV,tempW;
  
  // 모터의 회전속도 읽기
  //g_MainController.GetEncoder(0.01,g_pEncoderVal,g_pDeltaPos,NULL);
  g_MainController.SetMaxEncCount2(g_AlbabotAgv.GetParam_GearRatio()*20.0);//g_AGVParam.fGearRatio*20.0);
  g_MainController.SetWheelRadius2(g_AlbabotAgv.GetParam_WheelRadius());//g_AGVParam.fWheelRadius);
  g_MainController.GetEncoder(0.01,g_i64EncValue,g_pDeltaPos,NULL);
  //g_pDeltaPos[0]=g_pDeltaPos[1];    // cl1
  tempV=(g_pDeltaPos[0]+g_pDeltaPos[1])/2.0;
  tempW=(g_pDeltaPos[0]-g_pDeltaPos[1])/g_AlbabotAgv.GetParam_W2WDist(); //g_AGVParam.fDistWheel2Wheel;
  g_AlbabotAgv.AddAccumDist(tempV);
  g_AlbabotAgv.AddAccumAng(tempW);


  //g_pSonic2CANData[0]=100;

  g_AlbabotAgv.SetSensorData(g_pHall2CANData,g_pSonic2CANData);   
  g_AlbabotAgv.ProcessAGV(tempV,tempW,&(g_pMotorOutput[0]));

  fVel[0]=g_pMotorOutput[0];
  fVel[1]=g_pMotorOutput[1];

  // 모터 회전속도 및 가속도 값 설정
  //g_MainController.SetPWMValue(fVel[0],fVel[1],1.0,1.0);
  g_MainController.SetPWMValue(fVel[0],fVel[1],g_AlbabotAgv.GetParam_AccelL(),g_AlbabotAgv.GetParam_AccelR());

  if(ncount<5)
    ncount++;
  else
    ncount=0;

  if(ncount==0)
  {
    
    TransmitMessage();
    nh.spinOnce();  
  }

  delay(10);
  
}

void RemoteModeOperation()
{
  float fVel[2];  

  float dbTemp[2];

  g_MainController.GetEncoder(0.01,g_i64EncValue,dbTemp,NULL);

  // 속도값을 주기적으로 전송받고 있으며, 이 값이 수신되지 않는 시간을 타임아웃으로 사용
  if(g_u32HeartbeatCount>100) // 50ms 주기일 경우(20=1초)
    g_u8IsOperating=0;        // 동작 중지
  else
  {
    g_u32HeartbeatCount++;
    g_u8IsOperating=1;        // 동작
  }

  if(g_u8IsOperating==1)
  {
    fVel[0]=g_pMotorOutput[0];
    fVel[1]=g_pMotorOutput[1];
  }  
  else if(g_u8IsOperating==0)
  {
    fVel[0]=0;
    fVel[1]=0;
  } 

  //fVel[0]=g_pMotorOutput[0];
  //fVel[1]=g_pMotorOutput[1];
  // 모터 회전속도 및 가속도 값 설정
  g_MainController.SetPWMValue(fVel[0],fVel[1],0.2,0.2);

  
  TransmitMessage();
  nh.spinOnce();  
  delay(50);    
}

// loop함수에서 주기적으로 처리하는 루틴함수
// publish, sensor 요청 등
void TransmitMessage()
{
  double vel_v, vel_w;
  double delta_x, delta_y;
  //float dbTemp[2];

  //g_MainController.GetEncoder(0.01,g_i64EncValue,dbTemp,NULL);
  
  robotInfoMsg.left_enc=g_i64EncValue[0];
  robotInfoMsg.right_enc=g_i64EncValue[1];
  robotInfoMsg.hall_length=2;
  robotInfoMsg.hall=g_pHall2CANData;
  robotInfoMsg.sonic_length=8;
  robotInfoMsg.sonic=g_pSonic2CANData;
  robotInfoMsg.batVoltage=g_MainController.GetBatVol(0);
  robotInfoMsg.gio=0x0000;  
  robotInfoMsg.agvMissionCount=(u16)(g_AlbabotAgv.GetMissionCount());
  robotInfoMsg.agvStatus=g_AlbabotAgv.GetAGVStatus();
  robotInfoMsg.agvCurrentMission=g_AlbabotAgv.GetCurMission();
  robotInfoMsg.agvDirection=(u8)(g_AlbabotAgv.GetDirection());
  pub_robot_info.publish(&robotInfoMsg);

  // 6. request sonic value
  ReqSonicSensor();
}

void ReqSonicSensor(void)
{
  CanMsg canmsg;  
  FLOAT_DATA floatData;
  
  canmsg.IDE = CAN_ID_STD;
  canmsg.RTR = CAN_RTR_REMOTE;
  canmsg.DLC=0;

  canmsg.ID=0x20; 
  //canmsg.Data[0]=0x11;  // set pos  
  //canmsg.Data[0]=0x21;  // set vel  
  /*floatData.fData=pPos[0];
  for(int i=0;i<4;i++)
    canmsg.Data[i+1]=floatData.u8Data[i];  */
  CANsend(&canmsg); 
}

void SetMode_Hall2CAN(uint32_t id, uint8_t mode)
{
  // id = 0x10(left), 0x11(right)
  // mode = // 6:Calibration ,0:100Hz
  CanMsg hall2CAN;
  hall2CAN.IDE = CAN_ID_STD;
  hall2CAN.RTR = CAN_RTR_DATA;
  hall2CAN.DLC=8;
  hall2CAN.ID=id;
  hall2CAN.Data[0]=0xff;
  hall2CAN.Data[1]=0x00;
  hall2CAN.Data[2]=0x00;
  hall2CAN.Data[3]=0x00;
  hall2CAN.Data[4]=0x00;
  hall2CAN.Data[5]=0x00;
  hall2CAN.Data[6]=0x00;
  hall2CAN.Data[7]=mode;  // 6:Calibration ,0:100Hz
  CANsend(&hall2CAN);
}

u8 g_pDataRes[8];

void ROSServiceRes_OK(uint32_t id, uint8_t cmdtype, uint8_t command, void *pData)//, albabot_msgs::CommandResponse *pResponse)
{  
  albabot_msgs::Command::Response *pResponse;
  pResponse=(albabot_msgs::Command::Response*)pData; 
  pResponse->can_msg.id=id;
  pResponse->can_msg.dlc=3;
  pResponse->can_msg.data_length=3;
  g_pDataRes[0]=cmdtype;
  g_pDataRes[1]=command;
  g_pDataRes[2]=0x01;   //0x01 : ok
  pResponse->can_msg.data=g_pDataRes;
  pResponse->result=1;
}

void ROSServiceRes_Float(float val, const void* pReq, void *pRes)
{
  FLOAT_DATA floatData;

  albabot_msgs::Command::Response *pResponse;
  albabot_msgs::Command::Request *pRequest;
  
  pResponse=(albabot_msgs::Command::Response*)pRes; 
  pRequest=(albabot_msgs::Command::Request*)pReq; 

  pResponse->can_msg.dlc=6;
  floatData.fData=val;
  g_pDataRes[0]=pRequest->can_msg.data[0];
  g_pDataRes[1]=pRequest->can_msg.data[1];
  for(int i=0;i<4;i++)
    g_pDataRes[2+i]=floatData.u8Data[i];
  pResponse->can_msg.data_length=6;                    
  pResponse->can_msg.data=g_pDataRes;
}

void ROSServiceRes_int(int val, const void* pReq, void *pRes)
{
  FLOAT_DATA floatData;

  albabot_msgs::Command::Response *pResponse;
  albabot_msgs::Command::Request *pRequest;
  
  pResponse=(albabot_msgs::Command::Response*)pRes; 
  pRequest=(albabot_msgs::Command::Request*)pReq; 

  pResponse->can_msg.dlc=6;
  floatData.nData=val;
  g_pDataRes[0]=pRequest->can_msg.data[0];
  g_pDataRes[1]=pRequest->can_msg.data[1];
  for(int i=0;i<4;i++)
    g_pDataRes[2+i]=floatData.u8Data[i];
  pResponse->can_msg.data_length=6;                    
  pResponse->can_msg.data=g_pDataRes;
}

void ROSServiceRes_U16(uint16_t val, const void* pReq, void *pRes)
{
  FLOAT_DATA floatData;

  albabot_msgs::Command::Response *pResponse;
  albabot_msgs::Command::Request *pRequest;
  
  pResponse=(albabot_msgs::Command::Response*)pRes; 
  pRequest=(albabot_msgs::Command::Request*)pReq; 

  pResponse->can_msg.dlc=4;
  floatData.fData=val;
  g_pDataRes[0]=pRequest->can_msg.data[0];
  g_pDataRes[1]=pRequest->can_msg.data[1];
  g_pDataRes[2]=val>>8;
  g_pDataRes[3]=val;

  pResponse->can_msg.data_length=4;                    
  pResponse->can_msg.data=g_pDataRes;
}

void GetMessageCB(const albabot_msgs::CanMsg &msg)
//void rx_message_cb(const command_service::Request &req, command_service::Response &res)
{
  FLOAT_DATA floatData;
  uint32_t u32Temp;
  uint8_t u8Temp;
  uint16_t u16Temp;
  float fTemp;

  u8 command;
  u8 cmdtype;

  cmdtype=msg.data[0]; 
  command=msg.data[1];

  

  if(cmdtype==0x00)
  {
    switch(command)
    { 
      case 0x00:
        g_AlbabotAgv.SetCurMission(g_AlbabotAgv.GetMission(0));
        g_AlbabotAgv.SetMissionCount(0);
        //ROSServiceRes_OK(msg.id, cmdtype, command, &res);
        break;
      case 0x01:
        g_AlbabotAgv.SetMissionCount(0);
        g_AlbabotAgv.ClearMissionPlan();
        g_AlbabotAgv.SetMission(0,AGV_MISSION_NEXT);
        g_AlbabotAgv.SetCurMission(g_AlbabotAgv.GetMission(0));
        //ROSServiceRes_OK(msg.id, cmdtype, command, &res);
        break;
      case 0x02:
        g_AlbabotAgv.SetMissionCount(0);
        g_AlbabotAgv.ClearMissionPlan();
        g_AlbabotAgv.SetMission(0,AGV_MISSION_LEFT_90);
        g_AlbabotAgv.SetCurMission(g_AlbabotAgv.GetMission(0));
        g_AlbabotAgv.SetAccumAng(0.0);
        //ROSServiceRes_OK(msg.id, cmdtype, command, &res);
        break;
      case 0x03:
        g_AlbabotAgv.SetMissionCount(0);
        g_AlbabotAgv.ClearMissionPlan();
        g_AlbabotAgv.SetMission(0,AGV_MISSION_RIGHT_90);
        g_AlbabotAgv.SetCurMission(g_AlbabotAgv.GetMission(0));
        g_AlbabotAgv.SetAccumAng(0.0);
        //ROSServiceRes_OK(msg.id, cmdtype, command, &res);
        break;
      case 0x04:
        g_AlbabotAgv.SetMissionCount(0);
        g_AlbabotAgv.ClearMissionPlan();
        g_AlbabotAgv.SetMission(0,AGV_MISSION_TURN);
        g_AlbabotAgv.SetCurMission(g_AlbabotAgv.GetMission(0));
        g_AlbabotAgv.SetAccumAng(0.0);
        //ROSServiceRes_OK(msg.id, cmdtype, command, &res);
        break;
      case 0x05:
        g_AlbabotAgv.SetMissionCount(0);
        g_AlbabotAgv.ClearMissionPlan();
        g_AlbabotAgv.SetMission(0,AGV_MISSION_BACK);
        g_AlbabotAgv.SetCurMission(g_AlbabotAgv.GetMission(0));
        //ROSServiceRes_OK(msg.id, cmdtype, command, &res);
        break;
      case 0x30:    // Hall2can Calibration
        SetMode_Hall2CAN((uint32_t)msg.data[2],msg.data[3]);        
        //ROSServiceRes_OK(msg.id, cmdtype, command, &res);
        break;      
      case 0x31:    // Set Robot Operating Mode 
        if(msg.data[2]==0x00)
          g_u8OpMode=OPMODE_REMOTE;
        else if(msg.data[2]==0x01)
          g_u8OpMode=OPMODE_AGV;
        //ROSServiceRes_OK(msg.id, cmdtype, command, &res);
        break;
      case 0x40:    // 구동모듈 이동속도 : (0~3.3)  
        if(msg.dlc==6)
        {             
          for(int i=0;i<4;i++)
          {
            floatData.u8Data[i]=msg.data[2+i];
          }
          //g_AGVParam.fVelocityV=floatData.fData;
          g_AlbabotAgv.SetParam_VelV(floatData.fData);        
        }        
        //ROSServiceRes_Float(floatData.fData,&req, &res);                
        break;
      case 0x41:    // 구동모듈 회전속도 : (0~3.3)        
        if(msg.dlc==6)
        {
          for(int i=0;i<4;i++)
          {
            floatData.u8Data[i]=msg.data[2+i];
          }
          //g_AGVParam.fVelocityW=floatData.fData;
          g_AlbabotAgv.SetParam_VelW(floatData.fData);
        }        
        //ROSServiceRes_Float(floatData.fData,&req, &res);          
        break;
      case 0x42:    // Accelermeter 
        if(msg.dlc==6)
        {
          for(int i=0;i<4;i++)
          {
            floatData.u8Data[i]=msg.data[2+i];
          }
          //g_AGVParam.fAccelLimitL=floatData.fData;
          //g_AGVParam.fAccelLimitR=floatData.fData;
          g_AlbabotAgv.SetParam_Accel(floatData.fData);
        }
        //ROSServiceRes_Float(floatData.fData,&req, &res);  
        break;
      case 0x43:
        if(msg.dlc==6)
        {
          for(int i=0;i<4;i++)
          floatData.u8Data[i]=msg.data[2+i];
          //g_AGVParam.fBrakeDist=floatData.fData;
          g_AlbabotAgv.SetParam_BrakingDist(floatData.fData);
        }
        //ROSServiceRes_Float(floatData.fData,&req, &res);                  
        break;   
          
      case 0x44:    // 구동모듈 휠 반지름 : (m)        
        if(msg.dlc==6)
        {
          for(int i=0;i<4;i++)
          {
            floatData.u8Data[i]=msg.data[2+i];
          }
          //g_AGVParam.fWheelRadius=floatData.fData;
          g_AlbabotAgv.SetParam_WheelRadius(floatData.fData);
          //g_MainController.SetWheelRadius2(g_AGVParam.fWheelRadius);
        }
        //ROSServiceRes_Float(floatData.fData,&req, &res);       
        break; 
      // 구동모듈 기어비
      case 0x45:  
        if(msg.dlc==6)
        {
          for(int i=0;i<4;i++)
          {
            floatData.u8Data[i]=msg.data[2+i];
          }
          //g_AGVParam.fGearRatio=floatData.fData;
          g_AlbabotAgv.SetParam_GearRatio(floatData.fData);
          //g_MainController.SetMaxEncCount2(g_AGVParam.fGearRatio*10);
        }
        //ROSServiceRes_Float(floatData.fData,&req, &res);     
        
        break;  
      // wheel to wheel distance
      case 0x46:      
        if(msg.dlc==6)
        {
          for(int i=0;i<4;i++)
          {
            floatData.u8Data[i]=msg.data[2+i];
          }
          //g_AGVParam.fDistWheel2Wheel=floatData.fData;
          g_AlbabotAgv.SetParam_W2WDist(floatData.fData);
        }
        //ROSServiceRes_Float(floatData.fData,&req, &res);      
        break;      
      // 구동 미션 지연시간. 
      case 0x47:  
        if(msg.dlc==6)
        {
          for(int i=0;i<4;i++)
            floatData.u8Data[i] = msg.data[2+i];
          //g_AGVParam.nDelayModeCount=floatData.nData;//msg.data[2]*256+msg.data[3];
          g_AlbabotAgv.SetParam_DelayCount(floatData.nData);
        }
        //ROSServiceRes_int(floatData.nData, &req , &res);
        break; 
          
      case 0x48:      // 구동부 PID gain : Kp  
        if(msg.dlc==6)
        {
          for(int i=0;i<4;i++)
          {
            floatData.u8Data[i]=msg.data[2+i];
          }
          //g_AGVParam.kp=floatData.fData;
          g_AlbabotAgv.SetParam_Kp(floatData.fData);
        }
        ///ROSServiceRes_Float(floatData.fData,&req, &res);          
        
        break;
          
      case 0x49:      // 구동부 PID gain : Ki
        if(msg.dlc==6)
        {
          for(int i=0;i<4;i++)
          {
            floatData.u8Data[i]=msg.data[2+i];
          }
          //g_AGVParam.ki=floatData.fData;
          g_AlbabotAgv.SetParam_Ki(floatData.fData);
        }
        //ROSServiceRes_Float(floatData.fData,&req, &res);          
        break;          
      case 0x4a:      // 구동부 PID gain : Kd
        if(msg.dlc==6)
        {
          for(int i=0;i<4;i++)
          {
            floatData.u8Data[i]=msg.data[2+i];
          }
          //g_AGVParam.kd=floatData.fData;
          g_AlbabotAgv.SetParam_Kd(floatData.fData);
        }
        //ROSServiceRes_Float(floatData.fData,&req, &res);          
        break;
      case 0x78:
        if(msg.dlc==3)
        {        
          g_AlbabotAgv.SetDirection(msg.data[2]);        
        }
        //ROSServiceRes_int(g_AlbabotAgv.GetDirection(), &req, &res);
        break;
      case 0x80:  // path 생성 요청
        break;
      case 0x61:	// 전압값              
        u16Temp=g_MainController.GetBatVol(0);
        //ROSServiceRes_U16(u16Temp,&req,&res);
        break;
      case 0x82:	// 온도
        /*res.result.result.dlc=4;
        res.result.result.data.clear();
        res.result.result.data.push_back(0);
        res.result.result.data.push_back(0x82);
        res.result.result.data.push_back(12);
        res.result.result.data.push_back(34);*/
        break;
      case 0x83:	// 습도
        /*res.result.result.dlc=4;
        res.result.result.data.clear();
        res.result.result.data.push_back(0);
        res.result.result.data.push_back(0x83);
        res.result.result.data.push_back(56);
        res.result.result.data.push_back(78);*/
        break;
      case 0x84:  // AGVStatus                          
        //res.can_msg.id=msg.id;
        //res.can_msg.dlc=8;
        g_pDataRes[0]=(0x00);  
        g_pDataRes[1]=(0x84);  
        g_pDataRes[2]=(g_AlbabotAgv.GetAGVStatus());
        u16Temp=g_AlbabotAgv.GetMissionCount();
        g_pDataRes[3]=((uint8_t)(u16Temp>>8));
        g_pDataRes[4]=((uint8_t)(u16Temp));
        g_pDataRes[5]=(g_AlbabotAgv.GetCurMission());
        g_pDataRes[6]=(g_AlbabotAgv.GetDirection());
        g_pDataRes[7]=(0);    // 장애물 발생 유무
        //res.can_msg.data=g_pDataRes;             
        break;
      case 0x85:  //Set Alarm
      if(msg.dlc==3)
        {        
          SetAlarm(msg.data[2]);        
        }
        
        break;
      case 0xff:    // clear mission
        g_AlbabotAgv.SetMissionCount(0);
        g_AlbabotAgv.ClearMissionPlan();
        g_AlbabotAgv.SetMission(0,AGV_MISSION_WAIT);
        g_AlbabotAgv.SetCurMission(g_AlbabotAgv.GetMission(0));
        g_AlbabotAgv.SetAGVStatus(0x00);
        //ROSServiceRes_OK(msg.id, cmdtype, command, &res);
        break;
      default:
        break;
    }
  }
  else
  {
    g_AlbabotAgv.SetMissionPlan((u8*)&(msg.data[1]),
            (cmdtype-1)*7,(cmdtype-1)*7+msg.dlc-2);
    g_AlbabotAgv.SetMissionCount(0);

    //ROSServiceRes_OK(msg.id, cmdtype, command, &res);
  }
}

//상위제어기로 부터 모터의 전진 / 회전 속도값 메시지를 받아 처리하는 콜밸함수
// opmode = AGV : linear.x(left motor vel), linear.y(right motor vel) / unit 0~3.3V
// opmode = Remote : linear.x (전진속도), angular.z(회전속도)
void Velocity_cb( const std_msgs::Float32MultiArray& cmd_msg)
{     
  float vel1=0, vel2=0; // motor vel.(1:left motor, 2: right motor)  
  g_pMotorOutput[0]=cmd_msg.data[0];
  g_pMotorOutput[1]=cmd_msg.data[1];
  g_MainController.ToggleStatusLED1();
  g_u32HeartbeatCount=0;
}

void Obstacle_cb( const std_msgs::Float32MultiArray& msg)
{     
  float vel1=0, vel2=0; // motor vel.(1:left motor, 2: right motor)  
  /*g_pMotorOutput[0]=cmd_msg.data[0];
  g_pMotorOutput[1]=cmd_msg.data[1];
  g_MainController.ToggleStatusLED1();
  g_u32HeartbeatCount=0;*/

  g_pSonic2CANData[0]=(uint16)(msg.data[0]*100);

}


void startRos()
{
  g_MainController.EnableMotor(0,0);
  nh.initNode();

  /*while (!nh.connected())
  {
    nh.spinOnce();
  }*/

  

  nh.subscribe(sub_velocity);
  nh.subscribe(sub_obstacle);
  //nh.advertise(pub_robot_status);
  //nh.advertise(pub_encValue);
  nh.advertise(pub_robot_info);
  //nh.advertiseService(srvServerRxMessage);
  nh.subscribe(sub_Command);

  g_MainController.EnableMotor(1,1);
}

void InitRobot(void)
{
  InitAlarm();

  // 1. AGV parameter 설정
  //g_AlbabotAgv.SetInitAGVParam(1);  // AD1
  g_AlbabotAgv.SetInitAGVParam(2);  // ST1
  //g_AlbabotAgv.GetAGVParam(&g_AGVParam);

  // 2. 모터드라이버 설정
  g_MainController.Init();
  g_MainController.SetMaxEncCount2(g_AlbabotAgv.GetParam_GearRatio()*20.0);//g_AGVParam.fGearRatio*20.0);
  g_MainController.SetWheelRadius2(g_AlbabotAgv.GetParam_WheelRadius());//g_AGVParam.fWheelRadius);
  
  //g_MainController.SetDriverMode(DRIVER_MODE_MIDDLE); // AD1
  g_MainController.SetDriverMode(DRIVER_MODE_MIDDLE); // ST1


  /*g_MainController.SetDriverMode(DRIVER_MODE_LOW); // AD1

  for(int i=0;i<10;i++)
  {
    g_MainController.SetPWMValue(0.05,0.05,0.1,0.1);
    delay(5000);
    g_MainController.SetPWMValue(0.0,0.0,0.1,0.1);
    delay(1000);
    g_MainController.SetPWMValue(-0.05,-0.05,0.1,0.1);
    delay(5000);
  } */ 


  // 3. Hall2CAN 초기화
  CanMsg hall2CAN;
  hall2CAN.IDE = CAN_ID_STD;
  hall2CAN.RTR = CAN_RTR_DATA;
  hall2CAN.DLC=8;
  hall2CAN.ID=0x10;
  hall2CAN.Data[0]=0xff;    // 모드 설정임을 표시
  hall2CAN.Data[1]=0x00;
  hall2CAN.Data[2]=0x00;
  hall2CAN.Data[3]=0x00;
  hall2CAN.Data[4]=0x00;
  hall2CAN.Data[5]=0x00;
  hall2CAN.Data[6]=0x00;
  hall2CAN.Data[7]=0x00;    //0:100Hz, 1:50Hz, 2:10Hz, 3:5Hz, 4:1Hz, 5:trigger, 6:Calibration
  CANsend(&hall2CAN);

  float dbTemp[2];

  g_i64EncValue[0]=0.0;
  g_i64EncValue[1]=0.0;


  uint8_t pathplan[2048];

  for(int i=0;i<500;i++)
  {
    
    pathplan[i*4+0]=AGV_MISSION_NEXT;
    pathplan[i*4+1]=AGV_MISSION_DELAY;
    pathplan[i*4+2];AGV_MISSION_TURN;
    pathplan[i*4+3]=AGV_MISSION_DELAY;
  }

  g_AlbabotAgv.SetMissionPlan(pathplan,500*4);


}



void InitAlarm()
{
    pinMode(ALRAM_PIN_IG1, OUTPUT);
    pinMode(ALRAM_PIN_RL, OUTPUT);
    pinMode(ALRAM_PIN_BL1, OUTPUT);
    pinMode(ALRAM_PIN_BL2, OUTPUT); 
}

void SetAlarm(u8 value)
{
    if((value & 0x01) == 0x01)
      digitalWrite(ALRAM_PIN_IG1,1);
    else
      digitalWrite(ALRAM_PIN_IG1,0);
    
    if((value & 0x02) == 0x02)
      digitalWrite(ALRAM_PIN_RL,1);
    else
      digitalWrite(ALRAM_PIN_RL,0);

    if((value & 0x04) == 0x04)
      digitalWrite(ALRAM_PIN_BL1,1);
    else
      digitalWrite(ALRAM_PIN_BL1,0);

    if((value & 0x08) == 0x08)
      digitalWrite(ALRAM_PIN_BL2,1);
    else
      digitalWrite(ALRAM_PIN_BL2,0);
}

void InitTimer(void)
{
  // 3. 내부타이머 초기화
  g_timer4.setMode(TIMER_CH1, TIMER_OUTPUT_COMPARE);
  g_timer4.pause();
  g_timer4.setPeriod(100);
  g_timer4.setChannel1Mode(TIMER_OUTPUT_COMPARE);
  g_timer4.setCompare(TIMER_CH1, 1);
  g_timer4.attachCompare1Interrupt(handler_timer4);
  g_timer4.refresh();
  g_timer4.resume();
}


void handler_timer4(void) 
{  
  static int nCount=0;
  
  CAN1_Loop();
  if(nCount<999)
    nCount++;
  else
  {
    nCount=0;
  }  
}

void InitCAN(void)
{
  CAN_STATUS Stat ;
  g_CANBus.map(CAN_GPIO_PB8_PB9);       // This setting is already wired in the Olimexino-STM32 board
  Stat = g_CANBus.begin(CAN_SPEED_1000, CAN_MODE_NORMAL);    // Other speeds go from 125 kbps to 1000 kbps. CAN allows even more choices.

  g_CANBus.filter(0, 0x10, 0x7ff);    // hall2can  
  g_CANBus.filter(1, 0x11, 0x7ff);    // hall2can 2
  g_CANBus.filter(2, 0x20, 0x7ff);    // sonic2can_if
  g_CANBus.filter(3, 0x21, 0x7ff);    // sonic2can_if
  
  g_CANBus.set_irq_mode();            // Use irq mode (recommended), so the handling of incoming messages
                                      // will be performed at ease in a task or in the loop. The software fifo is 16 cells long, 
                                      // allowing at least 15 ms before processing the fifo is needed at 125 kbps
  Stat = g_CANBus.status();
  if (Stat != CAN_OK)
    /* Your own error processing here */ ;   // Initialization failed
}

void CAN1_Loop(void)
{
  int Pr = 0 ;
  int i ;  
  CanMsg *r_msg;  
  while ((r_msg = g_CANBus.recv()) != NULL)
  {  
    u32 u32CanID=r_msg->ID;
    switch(u32CanID)
    {
      case 0x10:
        //g_pRobotStatusData[0]=(uint64_t)((r_msg->Data[2]<<8) + r_msg->Data[3]);
        g_pHall2CANData[0]=(uint16_t)((r_msg->Data[2]<<8) + r_msg->Data[3]);
        break;
      case 0x11:
        //g_pRobotStatusData[0]=(uint64_t)((r_msg->Data[2]<<8) + r_msg->Data[3]);
        g_pHall2CANData[1]=(uint16_t)((r_msg->Data[2]<<8) + r_msg->Data[3]);
        break;
      case 0x20:
        g_pSonic2CANData[0]=(uint16_t)((r_msg->Data[1]<<8) + r_msg->Data[0]);
        g_pSonic2CANData[1]=(uint16_t)((r_msg->Data[3]<<8) + r_msg->Data[2]);
        g_pSonic2CANData[2]=(uint16_t)((r_msg->Data[5]<<8) + r_msg->Data[4]);
        g_pSonic2CANData[3]=(uint16_t)((r_msg->Data[7]<<8) + r_msg->Data[6]);


        break;
      /*case 0x21:
        g_pSonic2CANData[4]=(uint16_t)((r_msg->Data[1]<<8) + r_msg->Data[0]);
        g_pSonic2CANData[5]=(uint16_t)((r_msg->Data[3]<<8) + r_msg->Data[2]);
        g_pSonic2CANData[6]=(uint16_t)((r_msg->Data[5]<<8) + r_msg->Data[4]);
        g_pSonic2CANData[7]=(uint16_t)((r_msg->Data[7]<<8) + r_msg->Data[6]);
        */


        break;
      
      default:
        break;
    }  

    g_CANBus.free();

  }
}

CAN_TX_MBX CANsend(CanMsg *pmsg)
{
  CAN_TX_MBX mbx;
  do 
  {
    mbx = g_CANBus.send(pmsg) ;
  }
  while(mbx == CAN_TX_NO_MBX) ;       
  return mbx ;
}




