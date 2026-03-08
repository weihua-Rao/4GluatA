#ifndef timer_io_H
#define timer_io_H
//#include "PhoneCode.h"

//#include "UserConf.h"
//#ifdef 	timer_io_c
//#define timer_io_ext  
//#else
//#define timer_io_ext extern
//#endif

//#define FOSC 9600UL
//u16 T0_1MS = 0;			  //1M秒计数
//extern u16 UartDelay;
//extern bit	UartInt;
//#define T1MS (65536-FOSC/10030)      //1T模式
//#define T1MS (65536-FOSC/12/60000) //12T模式
//extern xdata u8 CheckNum;
//extern void Hex2Ascii(u8 dat);
//extern void Uart1Sends(u8 *puts);
//extern void Uart1BYTE(unsigned char temp);
//extern union Tr_IO ProtState;
//char xdata Trcnt1 = 0,Trcnt2 = 0,Trcnt3 = 0,Trcnt4 = 0,Trcnt5 = 0,Trcnt6 = 0,Trcnt7 = 0,Trcnt8 = 0;
//extern long unsigned int xdata EventBlack;


//extern uint8_t CheckNum;
extern uint8_t SingeDelay;
//extern unsigned char Pevent[];
void TIM_Configuration(void);
void ADC_Initial(ADC_Module* ADCx);
uint16_t ADC_GetData(ADC_Module* ADCx, uint8_t ADC_Channel);
#endif
