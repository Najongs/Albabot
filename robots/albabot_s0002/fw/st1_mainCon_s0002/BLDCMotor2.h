#ifndef _BLDCMOTOR2__H_
#define _BLDCMOTOR2__H_

#include <Arduino.h>
#include "define.h"

#include "../generic_stm32f103r/board/board.h"
#include "libmaple/dac.h"





class CBLDCMotor2
{
    public:
        void Init(void);
        void EnableMotor(u8 u8Enable, u8 u8Enable2);
        void SetDirection(u8 u8Dir, u8 u8Dir2);
        void SetDriverMode(u8 u8Mode);
        void ConfigEncoder(void);
        //void GetOdom(float dt, float *rpm, int *pos, ALBABOT_PARAM *pParam);
        //float GetRPM(float ds, float dt);
        void SetPWMValue(float fLeft, float fRight, float fPWMAccelLimitLeft, float fPWMAccelLimitRight);
        void GetVelocity(float *pVel);
        void GetEncoder(float dt, int64_t *pos, float *pDeltaPos, float *pDeltTheta);
        
        void SetMaxEncCount2(float value);        
        void SetWheelRadius2(float value);

        void ToggleStatusLED1();
        void ToggleStatusLED2();
        void SetStatusLED1(bool status);
        void SetStatusLED2(bool status);

        u16 GetBatVol(u8 u8Ch);
        
        //HardwareTimer g_timerEnc1(8); // enc1
        //HardwareTimer g_timerEnc2(2); // 


    private:
        float g_fWheelRadius;
        float g_fMaxEncoderCount;

        //dac_dev *DAC;
};



#endif

