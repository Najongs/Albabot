
#include "define.h"
#include "BLDCMotor2.h"
//#include "generic_stm32f103r/board/board.h"
//#include "generic_stm32f103r/board/board.h"
//#include "/home/swpark/Arduino/hardware/Arduino_STM32-Phonog-patch-1/STM32F1/variants/generic_stm32f103r/board/board.h"
//#include "../generic_stm32f103r/board/board.h"
#include "libmaple/dac.h"

//HardwareTimer *g_timerEnc1;
//HardwareTimer *g_timerEnc2;

HardwareTimer g_timerEnc1(8);
HardwareTimer g_timerEnc2(2);


//u16 g_ints1=0;
//u16 g_ints2=0;


//dac_dev *DAC;

// Init() : 모터 구동을 위한 각종 제어신호, 출력, 엔코더 포트 설정
void CBLDCMotor2::Init(void)
{
  //g_timerEnc1=new HardwareTimer(8);
  //g_timerEnc2=new HardwareTimer(2);

  pinMode(MOTOR_CH1_ENABLE_PIN, OUTPUT);
  pinMode(MOTOR_CH2_ENABLE_PIN, OUTPUT);

  pinMode(MOTOR_CH1_DIR_PIN, OUTPUT);
  pinMode(MOTOR_CH2_DIR_PIN, OUTPUT);

  pinMode(MOTOR_CH1_MODE1_PIN, OUTPUT);
  pinMode(MOTOR_CH1_MODE2_PIN, OUTPUT);

  pinMode(MOTOR_CH2_MODE1_PIN, OUTPUT);
  pinMode(MOTOR_CH2_MODE2_PIN, OUTPUT);

  pinMode(ENC1_CH_A_PIN, INPUT_PULLUP);  //channel A
  pinMode(ENC1_CH_B_PIN, INPUT_PULLUP);  //channel B

  pinMode(ENC2_CH_A_PIN, INPUT_PULLUP);  //channel A
  pinMode(ENC2_CH_B_PIN, INPUT_PULLUP);  //channel B

  pinMode(LED1_STATUS_PIN, OUTPUT);
  pinMode(LED2_STATUS_PIN, OUTPUT);

  pinMode(MOTOR_OUTPUT1_PIN, OUTPUT);
  pinMode(MOTOR_OUTPUT2_PIN, OUTPUT);

  //SetDriverMode(DRIVER_MODE_HIGH);
  SetDriverMode(DRIVER_MODE_MIDDLE);
  //SetDriverMode(DRIVER_MODE_OPENLOOP);
  dac_init(DAC, DAC_CH1 | DAC_CH2);  
  SetPWMValue(0,0,0,0);  
  ConfigEncoder();
}

// SetStatusLED1 : 보드의 상태 LED1 설정
void CBLDCMotor2::SetStatusLED1(bool status)
{
  digitalWrite(LED1_STATUS_PIN,status);
}

// SetStatusLED1 : 보드의 상태 LED2 설정
void CBLDCMotor2::SetStatusLED2(bool status)
{
  digitalWrite(LED2_STATUS_PIN,status);
}

// SetStatusLED1 : 보드의 상태 LED1 토글
void CBLDCMotor2::ToggleStatusLED1()
{
  digitalWrite(LED1_STATUS_PIN, 1-digitalRead(LED1_STATUS_PIN));
}

// SetStatusLED1 : 보드의 상태 LED2 토글
void CBLDCMotor2::ToggleStatusLED2()
{
  digitalWrite(LED2_STATUS_PIN, 1-digitalRead(LED2_STATUS_PIN));
}

//
void CBLDCMotor2::SetWheelRadius2(float value)
{
  g_fWheelRadius=value;
}
void CBLDCMotor2::SetMaxEncCount2(float value)
{
  g_fMaxEncoderCount=value;
}

u16 CBLDCMotor2::GetBatVol(u8 u8Ch)
{
  u16 batVol=0;
  if(u8Ch==0)
  {
    batVol=analogRead(PC0);
  }
  else if(u8Ch==1)
  {
    batVol=analogRead(PC1);
  }

  return batVol;
}
void CBLDCMotor2::GetEncoder(float dt, int64_t *pos, float *pDeltaPos, float *pDeltTheta)
{
  static u16 encPreCount=0;
	static u16 encPreCount2=0;
	short diff1, diff2;
	int64_t prePos;
	float fRPM;
  float fDiff1,fDiff2;
	u16 encCount;
  //float fWheelRadius=g_fWheelRadius;
  //float fEncMax=g_fMaxEncoderCount;

  // motor 1
	prePos=pos[0];
	encCount=g_timerEnc1.getCount();//g_ints1;//g_timerEnc1->getCount();
	diff1=(short)(encCount-encPreCount);
	encPreCount=encCount;
	pos[0]=prePos+diff1;  // encoder가 2배 차이남. 포트 바꿔야 함. 
	fDiff1=pos[0]-prePos;
  
  // motor 2
	prePos=pos[1];
	encCount=g_timerEnc2.getCount();//g_ints2;//g_timerEnc2->getCount();
	diff2=(short)(encPreCount2-encCount);
	encPreCount2=encCount;
	pos[1]=(prePos+diff2);
	fDiff2=pos[1]-prePos; 

  //pDeltaPos[0]=fDiff1;
  //pDeltaPos[1]=fDiff2;

  pDeltaPos[0]=2.0*g_fWheelRadius*((fDiff1/g_fMaxEncoderCount))*3.141592;
  pDeltaPos[1]=2.0*g_fWheelRadius*((fDiff2/g_fMaxEncoderCount))*3.141592;
  
}




void CBLDCMotor2::GetVelocity(float *pVel)
{
  static int pos[2]={0,0};

  static u16 encPreCount=0;
	static u16 encPreCount2=0;
	short diff1, diff2;
	int prePos;
	float fRPM;
  float fDiff1,fDiff2;
	u16 encCount;
  float fWheelRadius=g_fWheelRadius;
  float fEncMax=g_fMaxEncoderCount;

	prePos=pos[0];
	encCount=g_timerEnc1.getCount();//g_ints1;//g_timerEnc1->getCount();
	diff1=(short)(encCount-encPreCount);
	encPreCount=encCount;
	pos[0]=prePos+diff1;  // encoder가 2배 차이남. 포트 바꿔야 함. 
	fDiff1=pos[0]-prePos;
  
	prePos=pos[1];
	encCount=g_timerEnc2.getCount();//g_ints2;//g_timerEnc2->getCount();
	diff2=(short)(encPreCount2-encCount);
	encPreCount2=encCount;
	pos[1]=(prePos+diff2);
	fDiff2=pos[1]-prePos;  
  
  pVel[0]=2.0*fWheelRadius*((fDiff1/fEncMax))*3.141592;
  pVel[1]=2.0*fWheelRadius*((fDiff2/fEncMax))*3.141592;

  //pVel[0]=(float)diff1;
  //pVel[1]=(float)diff2;


}

#if 0
float CBLDCMotor2::GetRPM(float ds, float dt)
{
	float fGearRatio=100;//=g_ControllerInfo.gear_ratio;
	float fNumPole=10;		// Bldc motor pole number
	float fRPM;
	float fppr=fNumPole*2.0*fGearRatio; // Pulse per Revolution
	fRPM=(60.0 * (1.0/dt) * ds) / fppr;

	return fRPM;
}

void CBLDCMotor2::GetOdom(float dt, float *rpm, int *pos, ALBABOT_PARAM *pParam)
{
    static u16 encPreCount=0;
	static u16 encPreCount2=0;
	short diff1, diff2;
	int prePos;
	float fRPM;
    float fDiff1,fDiff2;
	u16 encCount;
    float fEncMax=(pParam->u16GearRatio * 20);


	prePos=pos[0];
	encCount=g_timerEnc1->getCount();
	diff1=(short)(encCount-encPreCount);
	encPreCount=encCount;
	pos[0]=prePos+diff1;
	fDiff1=pos[0]-prePos;

	prePos=pos[1];
	encCount=g_timerEnc2->getCount();
	diff2=(short)(encCount-encPreCount2);
	encPreCount2=encCount;
	pos[1]=(prePos+diff2);
	fDiff2=pos[1]-prePos;

	float delS_l=2.0*pParam->fWheelRadius*((fDiff1/fEncMax))*3.141592;
	float delS_r=2.0*pParam->fWheelRadius*((-fDiff2/fEncMax))*3.141592;

	rpm[0]=(delS_r+delS_l)/(2.0); // m/s   //fDiff1;//150.0*(fDiff1-fDiff2)/(2.0*dt);
	rpm[1]=(delS_r-delS_l)/(pParam->fDistWheel2Wheel); // rad/s //fDiff2;//150.0*(fDiff1+fDiff2)/(2.0*dt);

	// 200*(360*fDiff1/2000) * PI  / L*

	//pos[0]=(u16)(fDiff1*1000);
	//pos[1]=(u16)(fDiff2*-1000);

}
#endif

void CBLDCMotor2::EnableMotor(u8 u8Enable, u8 u8Enable2)
{
  if(u8Enable==1)
  {
    digitalWrite(MOTOR_CH1_ENABLE_PIN, HIGH);
  }
  else
  {
    digitalWrite(MOTOR_CH1_ENABLE_PIN, LOW);
  }


  if(u8Enable2==1)
    {        
        digitalWrite(MOTOR_CH2_ENABLE_PIN, HIGH);
    }
    else
    {
      digitalWrite(MOTOR_CH2_ENABLE_PIN, LOW);
    }
}

void CBLDCMotor2::  SetDirection(u8 u8Dir, u8 u8Dir2 )
{
  if(u8Dir==1)
    digitalWrite(MOTOR_CH1_DIR_PIN, HIGH);
  else
    digitalWrite(MOTOR_CH1_DIR_PIN, LOW);

  if(u8Dir2==1)
    digitalWrite(MOTOR_CH2_DIR_PIN, HIGH);
  else
    digitalWrite(MOTOR_CH2_DIR_PIN, LOW);

}

void CBLDCMotor2::SetDriverMode(u8 u8Mode)
{
  // 0,0 (open loop control)
  // 1,0 (저속)
  // 0,1 (중속)
  // 1,1 (고속)

  switch(u8Mode)
  {
  case DRIVER_MODE_OPENLOOP:
    digitalWrite(MOTOR_CH1_MODE1_PIN, LOW);
    digitalWrite(MOTOR_CH1_MODE2_PIN, LOW);
    digitalWrite(MOTOR_CH2_MODE1_PIN, LOW);
    digitalWrite(MOTOR_CH2_MODE2_PIN, LOW);    
    break;
  case DRIVER_MODE_HIGH:
    digitalWrite(MOTOR_CH1_MODE1_PIN, HIGH);
    digitalWrite(MOTOR_CH1_MODE2_PIN, HIGH);
    digitalWrite(MOTOR_CH2_MODE1_PIN, HIGH);
    digitalWrite(MOTOR_CH2_MODE2_PIN, HIGH);   
    break;
  case DRIVER_MODE_MIDDLE:
    digitalWrite(MOTOR_CH1_MODE1_PIN, LOW);
    digitalWrite(MOTOR_CH1_MODE2_PIN, HIGH);
    digitalWrite(MOTOR_CH2_MODE1_PIN, LOW);
    digitalWrite(MOTOR_CH2_MODE2_PIN, HIGH);   
    break;
  case DRIVER_MODE_LOW:
    digitalWrite(MOTOR_CH1_MODE1_PIN, HIGH);
    digitalWrite(MOTOR_CH1_MODE2_PIN, LOW);
    digitalWrite(MOTOR_CH2_MODE1_PIN, HIGH);
    digitalWrite(MOTOR_CH2_MODE2_PIN, LOW);   
    break;
  default:
    break;
  }

    EnableMotor(0,0);
    delay(50);
    EnableMotor(1,1);
    delay(50);
    

}


/*void Func_Encoder1()
{
  
  if (g_timerEnc1.getDirection())
  {
    g_ints1--;
  } 
  else
  {
    g_ints1++;
  }
}*/

/*void Func_Encoder2()
{
  if (g_timerEnc2.getDirection())
  {
    g_ints2--;
  } 
  else
  {
    g_ints2++;
  }
}*/

void CBLDCMotor2::ConfigEncoder(void)
{
  /*g_timerEnc1->setMode(1,TIMER_ENCODER); //set mode, the channel is not used when in this mode.
  g_timerEnc1->pause(); //stop...
  g_timerEnc1->setPrescaleFactor(1); //normal for encoder to have the lowest or no prescaler.
  g_timerEnc1->setOverflow(0xffff);    //use this to match the number of pulse per revolution of the encoder. Most industrial use 1024 single channel steps.
  g_timerEnc1->setCount(0);          //reset the counter.
  g_timerEnc1->setEdgeCounting(TIMER_SMCR_SMS_ENCODER3); //or TIMER_SMCR_SMS_ENCODER1 or TIMER_SMCR_SMS_ENCODER2. This uses both channels to count and ascertain direction.
  g_timerEnc1->attachInterrupt(ENC1_CH_A_PIN, Func_Encoder1); //channel doesn't mean much here either.
  g_timerEnc1->resume();                 //start the encoder...

  g_timerEnc2->setMode(1,TIMER_ENCODER); //set mode, the channel is not used when in this mode.
  g_timerEnc2->pause(); //stop...
  g_timerEnc2->setPrescaleFactor(2); //normal for encoder to have the lowest or no prescaler.
  g_timerEnc2->setOverflow(0xffff);    //use this to match the number of pulse per revolution of the encoder. Most industrial use 1024 single channel steps.
  g_timerEnc2->setCount(0);          //reset the counter.
  g_timerEnc2->setEdgeCounting(TIMER_SMCR_SMS_ENCODER3); //or TIMER_SMCR_SMS_ENCODER1 or TIMER_SMCR_SMS_ENCODER2. This uses both channels to count and ascertain direction.
  g_timerEnc2->attachInterrupt(ENC2_CH_A_PIN, Func_Encoder2); //channel doesn't mean much here either.
  g_timerEnc2->resume();                 //start the encoder...
  */

  g_timerEnc1.setMode(TIMER_CH1,TIMER_ENCODER); //set mode, the channel is not used when in this mode.
  g_timerEnc1.pause(); //stop...
  g_timerEnc1.setPrescaleFactor(1); //normal for encoder to have the lowest or no prescaler.
  g_timerEnc1.setOverflow(0xffff);    //use this to match the number of pulse per revolution of the encoder. Most industrial use 1024 single channel steps.
  g_timerEnc1.setEdgeCounting(TIMER_SMCR_SMS_ENCODER3); //or TIMER_SMCR_SMS_ENCODER1 or TIMER_SMCR_SMS_ENCODER2. This uses both channels to count and ascertain direction.
  g_timerEnc1.refresh();
  //g_timerEnc1.attachInterrupt(0, Func_Encoder1); //channel doesn't mean much here either.
  g_timerEnc1.resume();                 //start the encoder...
  

  g_timerEnc2.setMode(TIMER_CH1,TIMER_ENCODER); //set mode, the channel is not used when in this mode.
  //g_timerEnc2.pause(); //stop...
  g_timerEnc2.setPrescaleFactor(1); //normal for encoder to have the lowest or no prescaler.
  g_timerEnc2.setOverflow(0xffff);    //use this to match the number of pulse per revolution of the encoder. Most industrial use 1024 single channel steps.
  g_timerEnc2.setEdgeCounting(TIMER_SMCR_SMS_ENCODER3); //or TIMER_SMCR_SMS_ENCODER1 or TIMER_SMCR_SMS_ENCODER2. This uses both channels to count and ascertain direction.
  g_timerEnc2.refresh();
  //g_timerEnc2.attachInterrupt(0, Func_Encoder2); //channel doesn't mean much here either.
  g_timerEnc2.resume();                 //start the encoder...
  

}

void CBLDCMotor2::SetPWMValue(float fLeft, float fRight, float fPWMAccelLimitLeft, float fPWMAccelLimitRight)
{
	u8 pTXCANData[8];
	//2채널보드용 변수
	u8 CH1EN,CH2EN,CH1Dir,CH2Dir;
	u16 CH1DAC,CH2DAC;

	static float fPreLeft=0.0;
	static float fPreRight=0.0;
	//fPWMAccelLimit=g_AGVParam.fAccelLimit;
	float fAccelL=fPWMAccelLimitLeft;
	float fAccelR=fPWMAccelLimitRight;


  // 방향전환 시 가속도 값 최소로 만듦

  if((fLeft>0 && fPreLeft<0) || (fLeft<0 && fPreLeft>0))
    fAccelL=0.025;

  if((fRight>0 && fPreRight<0) || (fRight<0 && fPreRight>0))
    fAccelR=0.025;

    
  if(fPWMAccelLimitLeft==0)
	{
		fAccelL=3.3;
	}

	if(fPWMAccelLimitRight==0)
	{
		fAccelR=3.3;
	}

  
	if((fLeft - fPreLeft) > fAccelL)			//가속
	{
		fPreLeft = fPreLeft + fAccelL;
	}
	else if((fLeft - fPreLeft) < -fAccelL)	// 감속
	{
		fPreLeft = fPreLeft - fAccelL;
	}
	else
	{
		fPreLeft = fLeft;							//
	}


	if((fRight - fPreRight) > fAccelR)
	{
		fPreRight = fPreRight + fAccelR;
	}
	else if((fRight - fPreRight) < -fAccelR)
	{
		fPreRight = fPreRight - fAccelR;
	}
	else
	{
		fPreRight = fRight;
	}

  if(fPreLeft>3.3)
    fPreLeft=3.3;
  else if(fPreLeft<-3.3)
    fPreLeft=-3.3;
  
  if(fPreRight>3.3)
    fPreRight=3.3;
  else if(fPreRight<-3.3)
    fPreRight=-3.3;


	if(fabs(fPreLeft)<0.00005)	CH1EN=0;
	else					CH1EN=1;

	if(fabs(fPreRight)<0.00005)	CH2EN=0;
	else					CH2EN=1;

	if(fPreLeft>0)				CH1Dir=0;
	else					CH1Dir=1;

	if(fPreRight<0)			CH2Dir=0;
	else					CH2Dir=1;



	if(fPreLeft<0)		CH1DAC=-(fPreLeft / 3.3) * 4095;
	else				CH1DAC=(fPreLeft / 3.3) * 4095;


	if(fPreRight<0)	CH2DAC=-(fPreRight / 3.3) * 4095;
	else			CH2DAC=(fPreRight / 3.3) * 4095;


	SetDirection(CH1Dir,CH2Dir);
	EnableMotor(CH1EN,CH2EN);

  dac_write_channel(DAC, DAC_CH1, (u16)(CH1DAC*1.0));//g_RobotParam.fBalanceVal));
  dac_write_channel(DAC, DAC_CH2, (u16)(CH2DAC*1.0));

    
}
