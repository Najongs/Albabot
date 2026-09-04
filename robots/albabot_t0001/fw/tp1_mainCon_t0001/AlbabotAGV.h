#ifndef _ALBABOTAGV__H_
#define _ALBABOTAGV__H_

#include <Arduino.h>
#include "define.h"

#define MAX_MISSION_COUNT				2048//1024

#define	AGV_MISSION_WAIT				0
#define	AGV_MISSION_NEXT				1
#define	AGV_MISSION_LEFT_90				2
#define	AGV_MISSION_RIGHT_90			3
#define AGV_MISSION_DELAY				4
#define AGV_MISSION_BACK				5
#define	AGV_MISSION_TURN				6

typedef struct _AGV_PARAM
{
	float fVelocityV;		// AGV ���� �ִ�ӵ�
	float fVelocityW;		// AGV ȸ�� �ӵ�

	int nDelayModeCount;	// AGV�� AGV_MISSION_DELAY �̼��� ���� ī����, 1=10ms

	float kp;
	float ki;
	float kd;

	float fAccelLimitL;		// ������(0.03V) (Anlog output value : volt)
	float fAccelLimitR;		// ������(0.03V) (Anlog output value : volt)

	float fBrakeDist;

	float fGearRatio;
	float fWheelRadius;
	float fDistWheel2Wheel;

	float fBalanceVal;

}AGV_PARAM;

class CAlbabotAGV
{
    public:   
        CAlbabotAGV(void);
        void SetMissionPlan(u8 *pData, u16 length);
        void SetMissionPlan(u8 *pData, u16 startIndex, u16 endIndex);
        void SetMission(uint32_t index, u8 mission);
        void SetCurMission(u8 status);

        void AddAccumDist(float fData);
        void AddAccumAng(float fData);
        void SetAccumDist(float fData);
        void SetAccumAng(float fData);
        float GetAccumDist(void);
        float GetAccumAng(void);

        void SetSonicThreshold(u16 *pThr);


        void CalcDirection(u8 curMission);
        int GetDirection(void);
        void SetDirection(int nToDir);

        void SetMissionCount(uint32_t count);
        u8 GetMission(uint32_t index);
        uint32_t GetMissionCount();
        u8 GetCurMission();
        void ClearMissionPlan();

        void ProcessAGV(double posX, double posY, double theta, float *p_Output);
        void ProcessAGV(double vel_v, double vel_w, float *p_Output);
        void SetInitAGVParam(int nMode);
        void SetSensorData(u16 *pHall2can, u16 *pSonic);

        u8 GetAGVStatus();
        void SetAGVStatus(u8 status);
        u8 GetMagneticCenteroid();

        //void GetAGVParam(AGV_PARAM* pParam);
        //void SetAGVParam(AGV_PARAM* pParam);


        void SetParam_VelV(float val);
        void SetParam_VelW(float val);
        void SetParam_Accel(float val);
        void SetParam_BrakingDist(float val);
        void SetParam_WheelRadius(float val);
        void SetParam_GearRatio(float val);
        void SetParam_W2WDist(float val);
        void SetParam_DelayCount(int val);
        void SetParam_Kp(float val);
        void SetParam_Ki(float val);
        void SetParam_Kd(float val);

        float GetParam_VelV();
        float GetParam_VelW();
        float GetParam_AccelL();
        float GetParam_AccelR();
        float GetParam_BrakingDist();
        float GetParam_WheelRadius();
        float GetParam_GearRatio();
        float GetParam_W2WDist();
        int GetParam_DelayCount();
        float GetParam_Kp();
        float GetParam_Ki();
        float GetParam_Kd();




        u16 m_u16TempData;


    private:
        u8 m_u8MagneticPos;         //상위제어기의 요청에 대한 응답 정보(Hall2can 중심위치)
        u16 m_u16Hall2CANData1;     // Hall2CAN Data
        u16 m_u16Hall2CANData2;     // Hall2CAN Data
        u16 m_pU16SonicData[8];
        u8 m_u8AGVStatus;         // AGV status
        u16 m_pU16Obstacle_thr[8];

        int m_nObstacleDelayCount;
        
        int m_nCurDirection;

        u8 m_u8CurMission;
        u32 m_u32MissionCount;
        u8 m_pMissionPlan[MAX_MISSION_COUNT];
        AGV_PARAM m_AGVParam;
        float m_fDestMotorVel[2];
        float m_fCurAng;
        float m_fCurDist;
        float m_fAccumAng;	// relative turning angle(radian)
        float m_fAccumDist;	// relative braking distance(m)

        float m_fInitVelocityV;

        u8 g_u8RobotType;

        void DrivingAGV();
        u32 GetLineSensorStatus(u16 u16SensorData);
        u8 GetAGVStatus_WAITMission(void);
        u8 GetAGVStatus_NEXTMission(u8* pAGVStatus);
        u8 GetAGVStatus_BACKMission(u8* pAGVStatus);
        u8 GetAGVStatus_Turn90Mission(u8* pAGVStatus);
        u8 GetAGVStatus_Turn180Mission(u8* pAGVStatus);

        float AGV_PID_Controller(float dt, u8 CurPos,u8 opmode);
        float AGV_PID_Controller2(float dt, u8 CurPos,u8 opmode);
};



#endif

