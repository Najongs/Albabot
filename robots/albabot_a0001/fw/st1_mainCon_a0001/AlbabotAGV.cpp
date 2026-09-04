#include "AlbabotAGV.h"


/* 함수 : 생성자
   각종 멤버변수 초기화
*/
CAlbabotAGV::CAlbabotAGV(void)
{
  m_u16Hall2CANData1=0x00;          // Hall2CAN Data (전면)
  m_u16Hall2CANData2=0x00;          // Hall2CAN Data (후면) 
  m_u8AGVStatus=0;                  // AGV status
  m_fAccumAng=0;	                  // relative turning angle(radian)
  m_fAccumDist=0;	                  // relative braking distance(m)
  m_u8CurMission=AGV_MISSION_DELAY; // AGV모드에서 로봇의 현재 임무
  m_u32MissionCount=0;              // AGV모드에서 로봇의 현재 수행중인 임무의 카운터
  m_fDestMotorVel[0]=0.0;           // 모터의 최종출력 값(좌)
  m_fDestMotorVel[1]=0.0;           // 모터의 최종출력 값(우) 
  m_nCurDirection=1;                // 로봇의 현재 방향
  m_nObstacleDelayCount=0;          // 장애물이 인식되었을 때 대기해야할 카운터
  g_u8RobotType=_ROBOT_MODEL_AD1;   // 로봇의 모델

  // 2020.4.29일 수정 : 초음파센서 임계치 강제 적용(30cm)
  u16 pThr[8];
  for(int i=0;i<8;i++)
    pThr[i]=0;        // 0 : 안쓰겠단 말이지.
  SetSonicThreshold(pThr);          // 초음파센서의 장애물 인식 최소 거리 적용
}


/* 함수 : GetCurMission()
   현재 수행중인 임무를 리턴한다. 
   NEXT, LEFT TURN, RIGHT TURN, U TURN, STOP, WAIT 
*/
u8 CAlbabotAGV::GetCurMission()
{
    return m_u8CurMission;
}

/* 함수 : GetMagneticCenteroid()
   Hall2CAN 센서가 인식한 센서의 위치 0~15
*/
u8 CAlbabotAGV::GetMagneticCenteroid()
{
    return m_u8MagneticPos;
}

u8 CAlbabotAGV::GetAGVStatus()
{
    return m_u8AGVStatus;
}

void CAlbabotAGV::SetAGVStatus(u8 status)
{
  m_u8AGVStatus=status;
}
void CAlbabotAGV::SetSensorData(u16 *pHall2can, u16 *pSonic)
{
  uint16_t temp=0x0000;
  uint16_t minSonic=500;
  
	if(g_u8RobotType==_ROBOT_MODEL_AD1)
    {
      m_pU16SonicData[0]=minSonic;

      for(int i=0;i<8;i++)
      {
        if(pSonic[i]!=0)
        {
          if(minSonic>pSonic[i])
          {
            minSonic=pSonic[i];
            m_pU16SonicData[0]=minSonic;
          }  
        }  
      }

      m_u16Hall2CANData1=pHall2can[0];

      for(int i=0;i<16;i++)
      {

          if((pHall2can[1]>>i) & 0x0001 ==0x0001)
          {
            temp |= (0x0001<<i);                         
          }
          else
          {
            temp &= ~(0x0001<<i);
          }
      }      
      m_u16Hall2CANData2=temp;
      //m_u16Hall2CANData2=pHall2can[1];

    }	
	else if(g_u8RobotType==_ROBOT_MODEL_CL1)
	{
		// 0000 0111 1110 0000	// 0x07e0
		// 0011 1111 1111 1100  // 0x3ffc
    if( ((pHall2can[1] & 0xe000) == 0xe000)   )
		{
			m_u16Hall2CANData1=0x3ffc;
		}
		else 
		{
			m_u16Hall2CANData1=0x7e0;
		}	
    
		if( ((pHall2can[0] & 0xe000) == 0xe000)  )
		{
			m_u16Hall2CANData2=0x3ffc;
		}
		else 
		{
			m_u16Hall2CANData2=0x7e0;
		}	

    if( ((pHall2can[0] & 0xe000) == 0xe000) && ((pHall2can[1] & 0xe000) == 0xe000))
    {
      m_u16Hall2CANData1=0x0000;
      m_u16Hall2CANData2=0x0000;

    }
	}
}

void CAlbabotAGV::SetSonicThreshold(u16 *pThr)
{
  for(int i=0;i<8;i++)
  {
    m_pU16Obstacle_thr[i]=pThr[i];
  }  
}

void CAlbabotAGV::SetMissionPlan(u8 *pData, u16 length)
{
    //memset(m_pMissionPlan,AGV_MISSION_WAIT,sizeof(u8)*MAX_MISSION_COUNT);

    if(length<MAX_MISSION_COUNT)
        memcpy(m_pMissionPlan,pData,sizeof(u8)*length);
}


void CAlbabotAGV::SetMissionPlan(u8 *pData, u16 startIndex, u16 endIndex)
{
    //memset(m_pMissionPlan,AGV_MISSION_WAIT,sizeof(u8)*MAX_MISSION_COUNT);
    u16 length=endIndex-startIndex+1;
    if(length<MAX_MISSION_COUNT)      
        memcpy((u8*)&(m_pMissionPlan[startIndex]),pData,sizeof(u8)*length);
}
void CAlbabotAGV::SetMission(uint32_t index, u8 mission)
{
  m_pMissionPlan[index]=mission;
}
void CAlbabotAGV::SetCurMission(u8 status)
{
    m_u8CurMission=status;
}

void CAlbabotAGV::ProcessAGV(double posX, double posY, double theta, float *p_Output)
{
  DrivingAGV();


  p_Output[0]=m_fDestMotorVel[0];
  p_Output[1]=m_fDestMotorVel[1];
}

void CAlbabotAGV::ProcessAGV(double vel_v, double vel_w, float *p_Output)
{

    DrivingAGV();

    if(m_nObstacleDelayCount>0)
    {
      m_nObstacleDelayCount--;  
    }
    
    
    if(m_nObstacleDelayCount==0)
    {
      p_Output[0]=m_fDestMotorVel[0];
      p_Output[1]=m_fDestMotorVel[1];
    }
    else
    {
      m_fDestMotorVel[0]=0;
      m_fDestMotorVel[1]=0;

    }

    p_Output[0]=m_fDestMotorVel[0];
    p_Output[1]=m_fDestMotorVel[1];
    
}

void CAlbabotAGV::SetParam_VelV(float val)
{
  m_AGVParam.fVelocityV=val;
  m_fInitVelocityV=m_AGVParam.fVelocityV;
}

void CAlbabotAGV::SetParam_VelW(float val)
{
  m_AGVParam.fVelocityW=val;
}

void CAlbabotAGV::SetParam_Accel(float val)
{
  m_AGVParam.fAccelLimitL=val;
  m_AGVParam.fAccelLimitR=val;
}

void CAlbabotAGV::SetParam_BrakingDist(float val)
{
  m_AGVParam.fBrakeDist=val;
}
void CAlbabotAGV::SetParam_WheelRadius(float val)
{
  m_AGVParam.fWheelRadius=val;  
}

void CAlbabotAGV::SetParam_GearRatio(float val)
{
  m_AGVParam.fGearRatio=val;
}
void CAlbabotAGV::SetParam_W2WDist(float val)
{
  m_AGVParam.fDistWheel2Wheel=val;
}

void CAlbabotAGV::SetParam_DelayCount(int val)
{
  m_AGVParam.nDelayModeCount=val;
}
void CAlbabotAGV::SetParam_Kp(float val)
{
  m_AGVParam.kp=val;
}
void CAlbabotAGV::SetParam_Ki(float val)
{
  m_AGVParam.ki=val;
}
void CAlbabotAGV::SetParam_Kd(float val)
{
  m_AGVParam.kd=val;
}

float CAlbabotAGV::GetParam_VelV()
{
  return m_AGVParam.fVelocityV;
}

float CAlbabotAGV::GetParam_VelW()
{
  return m_AGVParam.fVelocityW;
}

float CAlbabotAGV::GetParam_AccelL()
{  
  return m_AGVParam.fAccelLimitL;
}

float CAlbabotAGV::GetParam_AccelR()
{  
  return m_AGVParam.fAccelLimitR;
}

float CAlbabotAGV::GetParam_BrakingDist()
{
  return m_AGVParam.fBrakeDist;
}
float CAlbabotAGV::GetParam_WheelRadius()
{
  return m_AGVParam.fWheelRadius;  
}

float CAlbabotAGV::GetParam_GearRatio()
{
  return m_AGVParam.fGearRatio;
}
float CAlbabotAGV::GetParam_W2WDist()
{
  return m_AGVParam.fDistWheel2Wheel;
}

int CAlbabotAGV::GetParam_DelayCount()
{
  return m_AGVParam.nDelayModeCount;
}
float CAlbabotAGV::GetParam_Kp()
{
  return m_AGVParam.kp;
}
float CAlbabotAGV::GetParam_Ki()
{
  return m_AGVParam.ki;
}
float CAlbabotAGV::GetParam_Kd()
{
  return m_AGVParam.kd;
}


/*void CAlbabotAGV::SetAGVParam(AGV_PARAM* pParam)
{
    memcpy((AGV_PARAM*)&m_AGVParam,pParam, sizeof(AGV_PARAM));
}*/

/*void CAlbabotAGV::GetAGVParam(AGV_PARAM* pParam)
{
    memcpy(pParam,(AGV_PARAM*)&m_AGVParam, sizeof(AGV_PARAM));
}*/

void CAlbabotAGV::SetInitAGVParam(int nMode)
{
    //AGV_PARAM *pParam = (AGV_PARAM*)&m_AGVParam;
    //g_u8RobotType=_ROBOT_MODEL_AD1;

	switch(nMode)
	{
	// 0 : nainbot black (gear ratio => 100:1)
	// Motor Driver Mode : DRIVER_MODE_HIGH mode
	case 0:     // winery cart
		//BLDCMotor2_SetDriverMode(DRIVER_MODE_HIGH);
    m_AGVParam.fVelocityV=0.38;		//voltage : 0.93V
    m_AGVParam.fVelocityW=0.250;			//0.65;////0.5;//0.35;
		m_AGVParam.fAccelLimitL=0.05;
		m_AGVParam.fAccelLimitR=0.05;
    m_AGVParam.kp=0.30;
		m_AGVParam.ki=0.00;
		m_AGVParam.kd=0.05;
    m_AGVParam.nDelayModeCount=50;	// 1sec
		m_AGVParam.fBrakeDist=0.100;		// �����Ÿ�100.0mm
		m_AGVParam.fGearRatio=30;		// ���� 100 : 1
		m_AGVParam.fWheelRadius=0.100;		// �� ������ : 100mm
		m_AGVParam.fDistWheel2Wheel=0.338;//0.343;//0.328;	// �� Wheel ������ �Ÿ� : 328MM
		m_AGVParam.fBalanceVal=1;	// �¿� ������ ȸ���뷱���� ���߱� ���� ��. //0.91(ȸ���ӵ� 0.65) //0.88(ȸ���ӵ� 0.5) // 0.83(ȸ���ӵ� 0.35)

		break;
	// 1 : nainbot black (gear ration => 30:1)
	case 1:       // No Load for Test
		//g_MainController.SetDriverMode(DRIVER_MODE_MIDDLE);
    m_AGVParam.fVelocityV=0.9;		//voltage : 0.93V
    m_AGVParam.fVelocityW=0.65;			//0.65;////0.5;//0.35;
		m_AGVParam.fAccelLimitL=3.3;
		m_AGVParam.fAccelLimitR=3.3;
    m_AGVParam.kp=0.30;
		m_AGVParam.ki=0.0;
		m_AGVParam.kd=0.35;
    m_AGVParam.nDelayModeCount=50;	// 1sec
		m_AGVParam.fBrakeDist=0.100;		// �����Ÿ�100.0mm
		m_AGVParam.fGearRatio=50;		// ���� 100 : 1
		m_AGVParam.fWheelRadius=0.100;		// �� ������ : 100mm
		m_AGVParam.fDistWheel2Wheel=0.328;//0.328;	// �� Wheel ������ �Ÿ� : 328MM
		m_AGVParam.fBalanceVal=1;	// �¿� ������ ȸ���뷱���� ���߱� ���� ��. //0.91(ȸ���ӵ� 0.65) //0.88(ȸ���ӵ� 0.5) // 0.83(ȸ���ӵ� 0.35)
		break;
  case 2:
		//g_MainController.SetDriverMode(DRIVER_MODE_HIGH);
    m_AGVParam.fVelocityV=0.8;		//voltage : 0.93V
    m_AGVParam.fVelocityW=0.4;			//0.65;////0.5;//0.35;
		m_AGVParam.fAccelLimitL=3.3;
		m_AGVParam.fAccelLimitR=3.3;
    m_AGVParam.kp=0.3;
		m_AGVParam.ki=0.00;
		m_AGVParam.kd=1.5;
    m_AGVParam.nDelayModeCount=50;	// 1sec
		m_AGVParam.fBrakeDist=0.100;		// �����Ÿ�100.0mm
		m_AGVParam.fGearRatio=30;		// ���� 50 : 1
		m_AGVParam.fWheelRadius=0.100;		// �� ������ : 100mm
		m_AGVParam.fDistWheel2Wheel=0.375;//0.328;	// �� Wheel ������ �Ÿ� : 328MM
		m_AGVParam.fBalanceVal=1;	// �¿� ������ ȸ���뷱���� ���߱� ���� ��. //0.91(ȸ���ӵ� 0.65) //0.88(ȸ���ӵ� 0.5) // 0.83(ȸ���ӵ� 0.35)
		break;
	case 3:
		//BLDCMotor2_SetDriverMode(DRIVER_MODE_MIDDLE);
    m_AGVParam.fVelocityV=0.5;		//voltage : 0.93V
    m_AGVParam.fVelocityW=0.25;			//0.65;////0.5;//0.35;
		m_AGVParam.fAccelLimitL=0.05;
		m_AGVParam.fAccelLimitR=0.05;
		m_AGVParam.kp=0.28;
		m_AGVParam.ki=0.0;
		m_AGVParam.kd=0.0;
    m_AGVParam.nDelayModeCount=50;	// 1sec
		m_AGVParam.fBrakeDist=0.100;		// unit : m
    m_AGVParam.fGearRatio=100;		// ���� 100 : 1
		m_AGVParam.fWheelRadius=0.1;		// unit : m
		m_AGVParam.fDistWheel2Wheel=0.328;	// unit : m
		m_AGVParam.fBalanceVal=1.0;//0.89;	// �¿� ������ ȸ���뷱���� ���߱� ���� ��. //0.91(ȸ���ӵ� 0.65) //0.88(ȸ���ӵ� 0.5) // 0.83(ȸ���ӵ� 0.35)

			break;
	default:
    m_AGVParam.fVelocityV=0.45;		//voltage : 0.93V
    m_AGVParam.fVelocityW=0.250;			//0.65;////0.5;//0.35;
		m_AGVParam.fAccelLimitL=0.05;
		m_AGVParam.fAccelLimitR=0.05;
    m_AGVParam.kp=0.45;
		m_AGVParam.ki=0.00;
		m_AGVParam.kd=0.25;
    m_AGVParam.nDelayModeCount=50;	// 1sec
		m_AGVParam.fBrakeDist=0.100;		// �����Ÿ�100.0mm
		m_AGVParam.fGearRatio=50;		// ���� 100 : 1
		m_AGVParam.fWheelRadius=0.100;		// �� ������ : 100mm
		m_AGVParam.fDistWheel2Wheel=0.328;//0.328;	// �� Wheel ������ �Ÿ� : 328MM
		m_AGVParam.fBalanceVal=1;	// �¿� ������ ȸ���뷱���� ���߱� ���� ��. //0.91(ȸ���ӵ� 0.65) //0.88(ȸ���ӵ� 0.5) // 0.83(ȸ���ӵ� 0.35)


		break;

	}


  m_fInitVelocityV=m_AGVParam.fVelocityV;
}

int CAlbabotAGV::GetDirection(void)
{
    return m_nCurDirection;
}

void CAlbabotAGV::SetDirection(int nToDir)
{
    m_nCurDirection=nToDir;
}
void CAlbabotAGV::CalcDirection(u8 curMission)
{
    int toDoDir;

    switch(curMission)
    {
    case AGV_MISSION_NEXT:
        toDoDir=0;
        break;
    case AGV_MISSION_TURN:
        toDoDir=2;
        break;
    case AGV_MISSION_WAIT:
        toDoDir=0;
        break;
    case AGV_MISSION_LEFT_90:
        toDoDir=-1;
        break;
    case AGV_MISSION_RIGHT_90:
        toDoDir=1;
        break;
    case AGV_MISSION_DELAY:
        toDoDir=0;
        break;
    case AGV_MISSION_BACK:
        toDoDir=0;
        break;
    default:
        toDoDir=0;
        break;
    }

    m_nCurDirection+=toDoDir;

    if(m_nCurDirection>4)
        m_nCurDirection=m_nCurDirection%4;
    else if(m_nCurDirection<1)
        m_nCurDirection+=4;
}

void CAlbabotAGV::DrivingAGV()
{
    static u32 u32DelayMissionCount=0;
	  static u8 u8OnBraking=0;
	  u8 u8CenterPos;
    float fScale=0;
    

    switch(m_u8CurMission)
    {
	  case AGV_MISSION_WAIT:
        u8CenterPos=GetAGVStatus_WAITMission();
        m_fDestMotorVel[0]=0.0;
        m_fDestMotorVel[1]=0.0;
        u32DelayMissionCount=0;

        m_fAccumAng=0.0;
        m_fAccumDist=0.0;
        u8OnBraking=0;

        AGV_PID_Controller2(0.02,u8CenterPos,m_u8CurMission);

        break;

    case AGV_MISSION_NEXT:
        // 1. brake 조건이 아닐 경우 다음 상태를 체크
        if(u8OnBraking==0)
            u8CenterPos=GetAGVStatus_NEXTMission(&m_u8AGVStatus);
        // 2. 현재상태가 노드인 경우 : 제동
        if(m_u8AGVStatus==0x01) // node
        { 
            if(u8OnBraking==0)
              m_fAccumDist=0;

            // 다음 미션이 NEXT이면 제동일 경우 일반 주행라인 위에 있도록 설정해서 원래 속도로 통과함. 
            
            if(m_u32MissionCount<MAX_MISSION_COUNT-1 &&
                    m_pMissionPlan[m_u32MissionCount+1]==AGV_MISSION_NEXT)        
            {
                /*if(m_pMissionPlan[m_u32MissionCount+1]==AGV_MISSION_NEXT)
                {
                  ++m_u32MissionCount;
                  m_u8CurMission=m_pMissionPlan[m_u32MissionCount];
                  m_u8AGVStatus=0x00; // on line                  
                }
                else
                {*/
                  ++m_u32MissionCount;
                  m_u8CurMission=m_pMissionPlan[m_u32MissionCount];
                  m_u8AGVStatus=0x00; // on line
                  //m_u8PrepareBraking=1;
                  //m_AGVParam.fVelocityV=1.9;
                //}
                
            }          
            else
            {
                // 2.2 braking process
                if(fabs(m_fAccumDist)>m_AGVParam.fBrakeDist)  // 2.2.1 제동거리이동 종료(정지)
                {
                    CalcDirection(m_u8CurMission);

                    m_fAccumAng=0.0;
                    m_fAccumDist=0.0;

                    ++m_u32MissionCount;
                    m_u8CurMission=m_pMissionPlan[m_u32MissionCount];
                    m_u8AGVStatus=0x00;

                    m_fDestMotorVel[0]=0;
                    m_fDestMotorVel[1]=0;

                    u8OnBraking=0;
                    //m_u8PrepareBraking=0;
                    m_AGVParam.fVelocityV=m_fInitVelocityV;

                    
                }
                else  // 2.2.2 check braking distance
                {
                    //m_fAccumDist+=m_fCurDist;
                    u8OnBraking=1;
                    // 2.1 agv status 가 노드위에 있을 경우 감속을 위한 속도값 설정
                    float temp=fabs(m_fAccumDist)/m_AGVParam.fBrakeDist;
                    if(temp>=1.0)
                    {
                      m_fDestMotorVel[0]=-(m_AGVParam.fVelocityV * (0.0));
                      m_fDestMotorVel[1]=-(m_AGVParam.fVelocityV* (0.0));
                    }
                    else
                    {
                      //m_fDestMotorVel[0]=-(m_AGVParam.fVelocityV*(1.0-temp));
                      //m_fDestMotorVel[1]=-(m_AGVParam.fVelocityV*(1.0-temp));
                      m_fDestMotorVel[0]=-(m_AGVParam.fVelocityV*(1.0-temp))/2.0;
                      m_fDestMotorVel[1]=-(m_AGVParam.fVelocityV*(1.0-temp))/2.0;
                    }

                    // 3.1 장애물 체크
                    /*if( (m_pU16SonicData[0]>0 && (m_pU16SonicData[0]< m_pU16Obstacle_thr[0])) ||
                        (m_pU16SonicData[1]>0 && (m_pU16SonicData[1]< m_pU16Obstacle_thr[1])) ||
                        (m_pU16SonicData[2]>0 && (m_pU16SonicData[2]< m_pU16Obstacle_thr[2])) ||
                        (m_pU16SonicData[3]>0 && (m_pU16SonicData[3]< m_pU16Obstacle_thr[3])) )
                    {
                      m_fDestMotorVel[0]=0;
                      m_fDestMotorVel[1]=0;

                      m_nObstacleDelayCount=100;
                    }*/

                    
                    if(m_pU16SonicData[0]<m_AGVParam.fBrakeDist*100.0)
                    {
                      m_fDestMotorVel[0]=0;
                      m_fDestMotorVel[1]=0;

                    }  
                }
            }
        }
        // 3. 현재상태가 라인위인 경우 : 주행
        else if(m_u8AGVStatus==0x00 || m_u8AGVStatus==0xf0) // online
        {
            float fTemp;
            float fTemp1,fTemp2;
            
            
            // 3.1 장애물 체크
            /*if( (m_pU16SonicData[0]>0 && (m_pU16SonicData[0]< m_pU16Obstacle_thr[0])) ||
                (m_pU16SonicData[1]>0 && (m_pU16SonicData[1]< m_pU16Obstacle_thr[1])) ||
                (m_pU16SonicData[2]>0 && (m_pU16SonicData[2]< m_pU16Obstacle_thr[2])) ||
                (m_pU16SonicData[3]>0 && (m_pU16SonicData[3]< m_pU16Obstacle_thr[3])) )
            {
               //m_fDestMotorVel[0]=0;
               //m_fDestMotorVel[1]=0;
               m_nObstacleDelayCount=100;
            }*/
            if(m_pU16SonicData[0]<50)
            {
              m_fDestMotorVel[0]=0;
              m_fDestMotorVel[1]=0;
              m_nObstacleDelayCount=100;
            }
            else
            {
              // 주행 알고리즘
              fTemp=AGV_PID_Controller2(0.05,u8CenterPos,m_u8CurMission);
              //fTemp=AGV_PID_Controller(0.02,u8CenterPos,m_u8CurMission);

              /*if(u8CenterPos<7-1)
                {
                  fScale=0.7;//u8CenterPos/7;
                  //fTemp=2.0;                  
                //fScale=0.1+u8CenterPos/10.0;
                }
              else if(u8CenterPos>10+1)
              {
                fScale=0.7;//(17-u8CenterPos)/7;
                //fTemp=-2.0;
                //fScale=0.1+(17-u8CenterPos)/10.0;
              }
              else
                fScale=1.0;*/
              
              fScale=1.0;

#if 1
              if(fTemp>1.0)
              {
                fTemp1=1.0;
                fTemp2=0.0;
              }
              else if(fTemp<-1.0)
              {
                fTemp1=0.0;
                fTemp2=1.0;    
              }
              else if(fTemp>0)
              {
                fTemp1=1.0;                
                fTemp2=1.0-fTemp;
              }
              else if(fTemp<0)
              {
                
                fTemp1=1.0+fTemp;                
                fTemp2=1.0;
              }
              else 
              {
                fTemp1=1.0;
                fTemp2=1.0;
              }
#endif
              
              float fOptacleScale=1.0;
              //기존
              /*if(m_pU16SonicData[0]<50 && m_pU16SonicData[0]>0)
              {
                fOptacleScale=0.0;
              }
              else
              {
                fOptacleScale=m_pU16SonicData[0]/50.0;
              }*/

              if(fOptacleScale>1.0)
              {

                m_fDestMotorVel[0]=-(m_AGVParam.fVelocityV + fTemp);
                m_fDestMotorVel[1]=-(m_AGVParam.fVelocityV - fTemp);
              }
              else
              {
                m_fDestMotorVel[0]=-((m_AGVParam.fVelocityV+fTemp)*fOptacleScale);
                m_fDestMotorVel[1]=-((m_AGVParam.fVelocityV-fTemp)*fOptacleScale);
              }

              
              // 제주 전시장용 ----start
              /*if(m_pU16SonicData[0]<50 && m_pU16SonicData[0]>0)
              {
                fOptacleScale=0.0;
              }
            
              if(fOptacleScale>1.0)
              {
                m_fDestMotorVel[0]=-(m_AGVParam.fVelocityV*fScale*fTemp1);
                m_fDestMotorVel[1]=-(m_AGVParam.fVelocityV*fScale*fTemp2);
              }
              else
              {
                m_fDestMotorVel[0]=-(m_AGVParam.fVelocityV*fOptacleScale*fScale*fTemp1);
                m_fDestMotorVel[1]=-(m_AGVParam.fVelocityV*fOptacleScale*fScale*fTemp2);
              }*/
              // 제주 전시장용 ----end
              
            }
            
        }
        // 4. 현재상태가 탈선인 경우
        else if(m_u8AGVStatus==0xff)  // offline
        {
            // 4.1 모터 정지
            m_fDestMotorVel[0]=0;
            m_fDestMotorVel[1]=0;

            // 4.2 기존 미션 초기화
            m_u8CurMission=AGV_MISSION_WAIT;
            m_u32MissionCount=0;
            memset(m_pMissionPlan,0,MAX_MISSION_COUNT);

            m_fAccumAng=0.0;
            m_fAccumDist=0.0;
            u8OnBraking=0;
        }
        // 5. Delay mission에 사용되는 카운터 초기화
        u32DelayMissionCount=0;
        break;

    case AGV_MISSION_BACK:

        // 1. brake 조건이 아닐 경우 다음 상태를 체크
        if(u8OnBraking==0)
            u8CenterPos=GetAGVStatus_BACKMission(&m_u8AGVStatus);
        // 2. 현재상태가 노드인 경우 : 제동
        if(m_u8AGVStatus==0x01) // node
        {
            if(u8OnBraking==0)
              m_fAccumDist=0;

            if(m_u32MissionCount<MAX_MISSION_COUNT-1 &&
                    m_pMissionPlan[m_u32MissionCount+1]==AGV_MISSION_BACK)
            {
                //m_u8AGVStatus=0x00;
                ++m_u32MissionCount;
                m_u8CurMission=m_pMissionPlan[m_u32MissionCount];
                m_u8AGVStatus=0x00;
            }
            else
            {
                // 2.2 braking process
                if(fabs(m_fAccumDist)>m_AGVParam.fBrakeDist)  // 2.2.1 제동거리이동 종료(정지)
                {
                    CalcDirection(m_u8CurMission);



                    m_fAccumAng=0.0;
                    m_fAccumDist=0.0;

                    ++m_u32MissionCount;
                    m_u8CurMission=m_pMissionPlan[m_u32MissionCount];
                    m_u8AGVStatus=0x00;

                    m_fDestMotorVel[0]=0;
                    m_fDestMotorVel[1]=0;

                    u8OnBraking=0;
                }
                else  // 2.2.2 check braking distance
                {
                    //m_fAccumDist+=m_fCurDist;
                    u8OnBraking=1;
                    // 2.1 agv status 가 노드위에 있을 경우 감속을 위한 속도값 설정
                    float temp=fabs(m_fAccumDist)/m_AGVParam.fBrakeDist;
                    if(temp>1.0)
                    {
                      m_fDestMotorVel[0]=(m_AGVParam.fVelocityV*0);
                      m_fDestMotorVel[1]=(m_AGVParam.fVelocityV*0);
                    }
                    else
                    {
                      m_fDestMotorVel[0]=(m_AGVParam.fVelocityV*(1.0-temp));
                      m_fDestMotorVel[1]=(m_AGVParam.fVelocityV*(1.0-temp));
                    }
                }
            }


        }
        // 3. 현재상태가 라인위인 경우 : 주행
        else if(m_u8AGVStatus==0x00 || m_u8AGVStatus==0xf0) // online
        {
            // 3.1 주행 알고리즘
            float fTemp;            
            fTemp=AGV_PID_Controller(0.02,u8CenterPos,m_u8CurMission);
            m_fDestMotorVel[0]=(m_AGVParam.fVelocityV+fTemp);
            m_fDestMotorVel[1]=(m_AGVParam.fVelocityV-fTemp);  

            /*float fTemp1, fTemp2;
            AGV_PID_Controller2(u8CenterPos,&fTemp1, &fTemp2);
            m_fDestMotorVel[0]=(m_AGVParam.fVelocityV*fTemp1);
            m_fDestMotorVel[1]=(m_AGVParam.fVelocityV*fTemp2);*/
                      
        // 3.2 장애물 체크
        }
        // 4. 현재상태가 탈선인 경우
        else if(m_u8AGVStatus==0xff)  // offline
        {
            // 4.1 모터 정지
            m_fDestMotorVel[0]=0;
            m_fDestMotorVel[1]=0;

            // 4.2 기존 미션 초기화
            m_u8CurMission=AGV_MISSION_WAIT;
            m_u32MissionCount=0;
            memset(m_pMissionPlan,0,MAX_MISSION_COUNT);
        }
        // 5. Delay mission에 사용되는 카운터 초기화
        u32DelayMissionCount=0;
        break;
    case AGV_MISSION_LEFT_90:
        u8CenterPos=GetAGVStatus_Turn90Mission(&m_u8AGVStatus);
        if(fabs((m_fAccumAng/3.141592)*180)>70.0)
        {
          fScale=0.3;
        }
        else
        {
          fScale=1.0;
        }


        m_fDestMotorVel[0]=m_AGVParam.fVelocityW*fScale;
        m_fDestMotorVel[1]=-1.0*m_AGVParam.fVelocityW*fScale;

        if(m_u8AGVStatus==0x01)// || g_u8AGVStatus==0xff)
        {
            CalcDirection(m_u8CurMission);

            ++m_u32MissionCount;
            m_u8CurMission=m_pMissionPlan[m_u32MissionCount];
            m_u8AGVStatus=0x00;

            m_fDestMotorVel[0]=0;
            m_fDestMotorVel[1]=0;
            m_fAccumAng=0.0;
            m_fAccumDist=0.0;
            
        }
        // 5. Delay mission에 사용되는 카운터 초기화
        u32DelayMissionCount=0;
        break;

    case AGV_MISSION_RIGHT_90:

        u8CenterPos=GetAGVStatus_Turn90Mission(&m_u8AGVStatus);
        if(fabs((m_fAccumAng/3.141592)*180)>70.0)
        {
          fScale=0.3;
        }
        else
        {
          fScale=1.0;
        }
        m_fDestMotorVel[0]=-1.0*m_AGVParam.fVelocityW*fScale;
        m_fDestMotorVel[1]=m_AGVParam.fVelocityW*fScale;

        if(m_u8AGVStatus==0x01)// || g_u8AGVStatus==0xff)
        {
            CalcDirection(m_u8CurMission);
          ++m_u32MissionCount;
          m_u8CurMission=m_pMissionPlan[m_u32MissionCount];
          m_u8AGVStatus=0x00;

          m_fDestMotorVel[0]=0;
          m_fDestMotorVel[1]=0;
          m_fAccumAng=0.0;
        m_fAccumDist=0.0;
        }

        // 5. Delay mission에 사용되는 카운터 초기화
        u32DelayMissionCount=0;
        break; 

    case AGV_MISSION_TURN:
        u8CenterPos=GetAGVStatus_Turn180Mission(&m_u8AGVStatus);
        if(fabs((m_fAccumAng/3.141592)*180)>160.0)
        {
          fScale=0.3;
        }
        else
        {
          fScale=1.0;
        }
        m_fDestMotorVel[0]=-1.0*m_AGVParam.fVelocityW*fScale;
        m_fDestMotorVel[1]=m_AGVParam.fVelocityW*fScale;


        if(m_u8AGVStatus==0x01)// || g_u8AGVStatus==0xff)
        {
                CalcDirection(m_u8CurMission);

          ++m_u32MissionCount;
          m_u8CurMission=m_pMissionPlan[m_u32MissionCount];
          m_u8AGVStatus=0x00;

          m_fDestMotorVel[0]=0;
          m_fDestMotorVel[1]=0;
          m_fAccumAng=0.0;
            m_fAccumDist=0.0;
        }
        // 5. Delay mission에 사용되는 카운터 초기화
        u32DelayMissionCount=0;
        break; 

    case AGV_MISSION_DELAY:
        m_fDestMotorVel[0]=0;
		    m_fDestMotorVel[1]=0;

        m_fAccumAng=0.0;
        m_fAccumDist=0.0;


        if(u32DelayMissionCount<m_AGVParam.nDelayModeCount)		// systick (20ms) * u32DelayMissionCount
        {
          u32DelayMissionCount++;
        }
        else
        {
                CalcDirection(m_u8CurMission);

          ++m_u32MissionCount;
          m_u8CurMission=m_pMissionPlan[m_u32MissionCount];
          u32DelayMissionCount=0;
        }
        break; 

        AGV_PID_Controller2(0.02,u8CenterPos,m_u8CurMission);

    default:
        m_fDestMotorVel[0]=0;
		    m_fDestMotorVel[1]=0;
        break;
  }
}

u32 CAlbabotAGV::GetLineSensorStatus(u16 u16SensorData)
{
	u8 u8FlagFound=0;   // ���༱ ������ ã�Ҵٰ� ǥ��(0:����. 1:1��, 2:2��, ..)
	u8 u8CountOne=0;
	u32 fTempSum=0;

	u32 u32Result=0x000000;

	for(int k=0;k<16;k++)
	{
		if(u8FlagFound==0)
		{
			if( (u16SensorData >> k ) & 0x0001 == 0x0001)             // Hall2CAN bit�� 1�� ���
			{
				fTempSum+=(k+1);
				++u8CountOne;
			}
			else                                                                // Hall2CAN bit�� 0�� ���
			{
				/* 히스테리시스 구간 문제로 처리하지 않음. 2019. 12.3
        if(u8CountOne<3)                // ���ӵ� 1�� ���� 2�� �̸��� ��� ������� ����
				{
					fTempSum=0;
					u8CountOne=0;
				}
				else                            // �׷��� ���� ��� ���༱�� ������ ã����.
				{
					u8FlagFound=1;
				}
        */
			}
		}
	}

	if(u8CountOne<3)		// 탈선
	{
		u32Result = 0x80000000;
	}
	else
	{
		if(u8CountOne<10)	// 주행선 위에 있음. 
		{
      u32Result = (u32)(fTempSum/u8CountOne)+1;
		}
		else				      // 노드
		{
      u32Result = 0x40000000 + (u32)(fTempSum/u8CountOne)+1;
    }
	}

  //ROS_INFO("status[%d], pos[%d]", (u8)(u32Result>>24), (u8)u32Result);

	if(u32Result & 0x80000000 == 0x80000000)
		m_u8MagneticPos=0;
	else if(u32Result & 0x40000000 == 0x40000000)
		m_u8MagneticPos=0xff;
	else
		m_u8MagneticPos=(u8)u32Result;    
    
  
  m_u8MagneticPos=(u8)u32Result;

	return u32Result;
}


u8 CAlbabotAGV::GetAGVStatus_WAITMission(void)
{
	static u8 NodeCount=0;
	u32 u32Hall2CANResult;
	u8 pData[8];

	u32Hall2CANResult=GetLineSensorStatus(m_u16Hall2CANData1);
	pData[0]=u32Hall2CANResult>>24;
	//pData[1]=u32Hall2CANResult>>16;
	//pData[2]=u32Hall2CANResult>>8;
	pData[3]=u32Hall2CANResult;

	return pData[3];
}


u8 CAlbabotAGV::GetAGVStatus_NEXTMission(u8* pAGVStatus)
{
	static u8 NodeCount=0;
	u32 u32Hall2CANResult;
	u8 pData[8];

	u32Hall2CANResult=GetLineSensorStatus(m_u16Hall2CANData1);
	pData[0]=u32Hall2CANResult>>24;   // 0x80(offline), 0x40(node), 0x00(online)
	//pData[1]=u32Hall2CANResult>>16;
	//pData[2]=u32Hall2CANResult>>8;
	pData[3]=u32Hall2CANResult;

  // check which current status is node
  if(NodeCount>2 && pData[0]==0) //
	{
		*pAGVStatus=0x01;   // agv status : node
		NodeCount=0;
	}

	//////////////////////////////////////////////
  // check node! 
	if(pData[0]==0x80)		// offline
	{
		*pAGVStatus=0xff;   // agv status : offline
		NodeCount=0;
	}
	else if(pData[0]==0x40)	// node
	{
		if(NodeCount<10)
			NodeCount++;
	}
	else
	{
		NodeCount=0;
	}
	//////////////////////////////////////////////



	return pData[3];
}

u8 CAlbabotAGV::GetAGVStatus_BACKMission(u8* pAGVStatus)
{
	static u8 NodeCount=0;
	u32 u32Hall2CANResult;
	u8 pData[8];

	u32Hall2CANResult=GetLineSensorStatus(m_u16Hall2CANData2);
	pData[0]=u32Hall2CANResult>>24;   // 0x80(offline), 0x40(node), 0x00(online)
	//pData[1]=u32Hall2CANResult>>16;
	//pData[2]=u32Hall2CANResult>>8;
	pData[3]=u32Hall2CANResult;

  // check which current status is node
  if(NodeCount>2 && pData[0]==0) //
	{
		*pAGVStatus=0x01;   // agv status : node
		NodeCount=0;
	}

	//////////////////////////////////////////////
  // check node! 
	if(pData[0]==0x80)		// offline
	{
		*pAGVStatus=0xff;   // agv status : offline
		NodeCount=0;
	}
	else if(pData[0]==0x40)	// node
	{
		if(NodeCount<10)
			NodeCount++;
	}
	else
	{
		NodeCount=0;
	}
	//////////////////////////////////////////////



	return pData[3];
}


u8 CAlbabotAGV::GetAGVStatus_Turn90Mission(u8* pAGVStatus)
{
	static u8 TimeoutCount=0;
	static u8 OfflineCount=0;
	u32 u32Hall2CANResult;
	u8 pData[8];

	u32Hall2CANResult=GetLineSensorStatus(m_u16Hall2CANData1);
	pData[0]=u32Hall2CANResult>>24;
	//pData[1]=u32Hall2CANResult>>16;
	//pData[2]=u32Hall2CANResult>>8;
	pData[3]=u32Hall2CANResult;


  if(fabs(m_fAccumAng)>((89.0/180.0)*3.1415) ) // && fabs(m_fAccumAng)<((91.0/180.0)*3.1415))
	//if((fDistP1>fDistP2 && fDistP1<fDistP3) || (fDistM1<fDistM2 && fDistM1>fDistM3))
	//if(TimeoutCount>200-1 && pData[0]==0 && pData[3]>7 && pData[3]<10)
	{
		*pAGVStatus=0x01;   // agv status : node
		TimeoutCount=0;
    m_fAccumAng=0;
	}
	else
	{
    //m_fAccumAng+=m_fCurAng;
	}


	//////////////////////////////////////////////
	if(TimeoutCount<200)
	{
		TimeoutCount++;
	}

	return pData[3];
}

u8 CAlbabotAGV::GetAGVStatus_Turn180Mission(u8* pAGVStatus)
{
	static u32 TimeoutCount=0;
	static u8 OfflineCount=0;
	u32 u32Hall2CANResult;
	u8 pData[8];

	u32Hall2CANResult=GetLineSensorStatus(m_u16Hall2CANData1);
	pData[0]=u32Hall2CANResult>>24;
	//pData[1]=u32Hall2CANResult>>16;
	//pData[2]=u32Hall2CANResult>>8;
	pData[3]=u32Hall2CANResult;

  if(fabs(m_fAccumAng)>((179.0/180.0)*3.1415) ) // && fabs(m_fAccumAng)<((181.0/180.0)*3.1415))
	//if((fDistP1>fDistP2 && fDistP1<fDistP3) || (fDistM1<fDistM2 && fDistM1>fDistM3))
	//if(TimeoutCount>200-1 && pData[0]==0 && pData[3]>7 && pData[3]<10)
	{
		*pAGVStatus=0x01;
		TimeoutCount=0;
    m_fAccumAng=0;
	}
	else
	{
    //m_fAccumAng+=m_fCurAng;
	}

	//////////////////////////////////////////////
	if(TimeoutCount<400)
		TimeoutCount++;

	return pData[3];
}



float CAlbabotAGV::AGV_PID_Controller2(float dt, u8 CurPos,u8 opmode)
{

  //8*sin

  /*float pNormPos[16]={7.85/8.0,7.39/8.0,6.65/8.0,5.66/8.0,4.44/8.0,3.06/8.0,1.56/8.0,0,
                      0,-1.56/8.0,-3.06/8.0,-4.44/8.0,-5.66/8.0,-6.65/8.0,-7.39/8.0,-7.85/8.0};*/
  float pNormPos[16]={0.4,0.4,0.4,0.4,0.4,0.3,0.2,0.1,                    
                      -0.1,-0.2,-0.3,-0.4,-0.4,-0.4,-0.4,-0.4};
  /*float pNormPos[16]={0.97,0.93,0.86,0.75,0.59,0.39,0.20,0,
                      0,-0.20,-0.39,-0.59,-0.75,-0.86,-0.93,-0.97};                      */
  /*float pNormPos[16]={1.00,0.99,0.96,0.86,0.68,0.39,0.12,0.00,
                      0.00,-0.12,-0.39,-0.68,-0.86,-0.96,-0.99,-1.00};                           */

	float kp=m_AGVParam.kp, ki=m_AGVParam.ki, kd=m_AGVParam.kd;
	float fMaxRPM=m_AGVParam.fVelocityV;

	float fCorrection = 0.0;
	float fError = 0.0;
	float fTs=dt;

	float fTemp=0.0;

	static float fControlP=0.0;
	static float fControlI=0.0;
	static float fControlD=0.0;
	static float fPreError=0.0;
  static float fErrorSum=0.0;

  if(opmode!=AGV_MISSION_NEXT && opmode!=AGV_MISSION_BACK)
	{
    fControlP=0.0;
		fControlI=0.0;
		fControlD=0.0;
		fPreError=0.0;
    fErrorSum=0.0;
	}
	else
	{

    if(CurPos>0)
      fError=pNormPos[CurPos-1];
  

		fControlP=kp*fError;
		fControlI=ki*fErrorSum;
		fControlD=kd*(fError-fPreError);

		fPreError=fError;
		fErrorSum+=fError;
	}

	fCorrection=fControlP+fControlI+fControlD;
  fTemp=fCorrection;

	return fTemp;
}

  

float CAlbabotAGV::AGV_PID_Controller(float dt, u8 CurPos,u8 opmode)
{
	float kp=m_AGVParam.kp, ki=m_AGVParam.ki, kd=m_AGVParam.kd;
	float fMaxRPM=m_AGVParam.fVelocityV;

	float fCorrection = 0.0;
	float fError = 0.0;
	float fTs=dt;

	float fTemp=0.0;

	static float fControlP=0.0;
	static float fControlI=0.0;
	static float fControlD=0.0;
	static float fPreError=0.0;
  static float fErrorSum=0.0;

  if(opmode!=AGV_MISSION_NEXT && opmode!=AGV_MISSION_BACK)
	{
    fControlP=0.0;
		fControlI=0.0;
		fControlD=0.0;
		fPreError=0.0;
    fErrorSum=0.0;
	}
	else
	{
    
    if(CurPos<8)
      fError=1.0*(8.0-(float)CurPos)/7.0;
    else if(CurPos>9)
      fError=-1.0*((float)CurPos - 9.0)/7.0;
  

		fControlP=kp*fError;
		fControlI=ki*fErrorSum*fTs;
		fControlD=kd*(fError-fPreError)*fTs;
		fPreError=fError;
		fErrorSum+=fError;
	}

	fCorrection=fControlP+fControlI+fControlD;
  fTemp=fCorrection;

	return fTemp;
}

void CAlbabotAGV::AddAccumDist(float fData)
{
  m_fAccumDist+=fData;
}

void CAlbabotAGV::AddAccumAng(float fData)
{
  m_fAccumAng+=fData;
}

void CAlbabotAGV::SetMissionCount(uint32_t count)
{
  m_u32MissionCount=count;
}

uint32_t CAlbabotAGV::GetMissionCount()
{
  return m_u32MissionCount;
}

u8 CAlbabotAGV::GetMission(uint32_t index)
{
  return m_pMissionPlan[index];
}

void CAlbabotAGV::ClearMissionPlan()
{
  memset(m_pMissionPlan,0,MAX_MISSION_COUNT);
}

void CAlbabotAGV::SetAccumDist(float fData)
{
  m_fAccumDist=fData;
}

void CAlbabotAGV::SetAccumAng(float fData)
{
  m_fAccumAng=fData;
}

float CAlbabotAGV::GetAccumDist(void)
{
    return m_fAccumDist;
}

float CAlbabotAGV::GetAccumAng(void)
{
    return m_fAccumAng;
}
