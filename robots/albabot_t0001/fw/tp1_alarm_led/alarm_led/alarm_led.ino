#include <HardwareCAN.h>
#include <Adafruit_NeoPixel.h>

// 데이터 타입 정의 
typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;


#define PIN_OUT1   PB15//PA7
#define PIN_OUT2   PA7

// 내부 타이머
HardwareTimer g_timer4(4);     


// for CAN communication 
HardwareCAN g_CANBus(CAN1_BASE);
CAN_TX_MBX CANsend(CanMsg *pmsg);
void InitCAN(void);
void CAN1_Loop(void);


void InitTimer(void);

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(16, PIN_OUT1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(16, PIN_OUT2, NEO_GRB + NEO_KHZ800);


void setup() {

    //InitTimer();
    //InitCAN();

    // Set up the built-in LED pin as an output:
    pinMode(PIN_OUT1, OUTPUT);
    pinMode(PIN_OUT2, OUTPUT);

    strip1.begin();
    strip1.show();
}


void loop() {
  //아래의 순서대로 NeoPixel을 반복한다.
  colorWipe(strip1.Color(255, 0, 0), 50); //빨간색 출력
  colorWipe(strip1.Color(0, 255, 0), 50); //녹색 출력
  colorWipe(strip1.Color(0, 0, 255), 50); //파란색 출력

  theaterChase(strip1.Color(127, 127, 127), 50); //흰색 출력
  theaterChase(strip1.Color(127,   0,   0), 50); //빨간색 출력
  theaterChase(strip1.Color(  0,   0, 127), 50); //파란색 출력

  //화려하게 다양한 색 출력
  rainbow(20);
  rainbowCycle(20);
  theaterChaseRainbow(50);
}

//NeoPixel에 달린 LED를 각각 주어진 인자값 색으로 채워나가는 함수
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip1.numPixels(); i++) {
      strip1.setPixelColor(i, c);
      strip1.show();
      delay(wait);
  }
}

//모든 LED를 출력가능한 모든색으로 한번씩 보여주는 동작을 한번하는 함수
void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip1.numPixels(); i++) {
      strip1.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip1.show();
    delay(wait);
  }
}

//NeoPixel에 달린 LED를 각각 다른색으로 시작하여 다양한색으로 5번 반복한다
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { 
    for(i=0; i< strip1.numPixels(); i++) {
      strip1.setPixelColor(i, Wheel(((i * 256 / strip1.numPixels()) + j) & 255));
    }
    strip1.show();
    delay(wait);
  }
}

//입력한 색으로 LED를 깜빡거리며 표현한다
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip1.numPixels(); i=i+3) {
        strip1.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip1.show();
     
      delay(wait);
     
      for (int i=0; i < strip1.numPixels(); i=i+3) {
        strip1.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//LED를 다양한색으로 표현하며 깜빡거린다
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     //256가지의 색을 표현
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip1.numPixels(); i=i+3) {
          strip1.setPixelColor(i+q, Wheel( (i+j) % 255));
        }
        strip1.show();
       
        delay(wait);
       
        for (int i=0; i < strip1.numPixels(); i=i+3) {
          strip1.setPixelColor(i+q, 0); 
        }
    }
  }
}

//255가지의 색을 나타내는 함수
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip1.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip1.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip1.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
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

  g_CANBus.filter(0, 0x100, 0x7ff);   // alarm led1
  
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
      case 0x100:
        //g_pRobotStatusData[0]=(uint64_t)((r_msg->Data[2]<<8) + r_msg->Data[3]);
        //g_pHall2CANData[0]=(uint16_t)((r_msg->Data[2]<<8) + r_msg->Data[3]);
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

