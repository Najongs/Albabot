#ifndef _DEFINE__H_
#define _DEFINE__H_

// 로봇 모델 정의 파일 
// 로봇 모델에 따라 센서, 동작 등이 달라 거기에 적합한 값 또는 동작을 수행하기 위해 설정
// 로봇 모델 정의가 첫번째 임. 
#define _ROBOT_MODEL_CL1    0
#define _ROBOT_MODEL_AD1    1
#define _ROBOT_MODEL_AD2    2
#define _ROBOT_MODEL_AD3    3
#define _ROBOT_MODEL_ST1    4

// 로봇의 동작모드 정의
#define OPMODE_AGV          0
#define OPMODE_REMOTE       1

// 데이터 타입 정의 
typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;

typedef union _FLOAT_DATA
{
	u32 u32Data;
	float fData;
	u8 u8Data[4];
    u16 u16Data[2];
	int nData;    
}FLOAT_DATA;

typedef struct _ALBABOT_CL1_PARAM
{
    float lift1_kp;         
	float lift1_ki;
	float lift1_kd;

    float lift2_kp;         
	float lift2_ki;
	float lift2_kd;
}ALBABOT_CL1_PARAM;

// board pinout 정의
#define LED1_STATUS_PIN             PB12
#define LED2_STATUS_PIN             PC9

#define MOTOR_CH1_ENABLE_PIN        PA3
#define MOTOR_CH2_ENABLE_PIN        PB0

#define MOTOTR_CH1_SPEED_3V3_PIN    PA4
#define MOTOTR_CH2_SPEED_3V3_PIN    PA5

#define MOTOR_CH1_DIR_PIN           PA6
#define MOTOR_CH2_DIR_PIN           PB1

#define MOTOR_CH1_MODE1_PIN         PC11
#define MOTOR_CH1_MODE2_PIN         PC10

#define MOTOR_CH2_MODE1_PIN         PD2
#define MOTOR_CH2_MODE2_PIN         PC12

#define DRIVER_MODE_OPENLOOP        0
#define DRIVER_MODE_LOW             1
#define DRIVER_MODE_MIDDLE          2
#define DRIVER_MODE_HIGH            3

#define ENC1_CH_A_PIN   PC6
#define ENC1_CH_B_PIN   PC7

#define ENC2_CH_A_PIN   PA0
#define ENC2_CH_B_PIN   PA1

#define MOTOR_OUTPUT1_PIN           PA4
#define MOTOR_OUTPUT2_PIN           PA5


#endif