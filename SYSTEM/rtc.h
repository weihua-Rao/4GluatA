#ifndef __RTC_H
#define __RTC_H	
#include "n32g45x.h"
#include "n32g45x_it.h" 
#include "syslib.h"

//#define RTC_IT_OW            ((uint16_t)0x0004)  /*!< Overflow interrupt */
//#define RTC_IT_ALR           ((uint16_t)0x0002)  /*!< Alarm interrupt */
//#define RTC_IT_SEC           ((uint16_t)0x0001)  /*!< Second interrupt */
//#define IS_RTC_IT(IT) ((((IT) & (uint16_t)0xFFF8) == 0x00) && ((IT) != 0x00))
//#define IS_RTC_GET_IT(IT) (((IT) == RTC_IT_OW) || ((IT) == RTC_IT_ALR) || \
                           ((IT) == RTC_IT_SEC))

/** @defgroup RCC_Flag 
  * @{
  */
//#define RCC_FLAG_HSIRDY                  ((uint8_t)0x21)
//#define RCC_FLAG_HSERDY                  ((uint8_t)0x31)
//#define RCC_FLAG_PLLRDY                  ((uint8_t)0x39)
//#define RCC_FLAG_LSERDY                  ((uint8_t)0x41)
//#define RCC_FLAG_LSIRDY                  ((uint8_t)0x61)
//#define RCC_FLAG_PINRST                  ((uint8_t)0x7A)
//#define RCC_FLAG_PORRST                  ((uint8_t)0x7B)
//#define RCC_FLAG_SFTRST                  ((uint8_t)0x7C)
//#define RCC_FLAG_IWDGRST                 ((uint8_t)0x7D)
//#define RCC_FLAG_WWDGRST                 ((uint8_t)0x7E)
//#define RCC_FLAG_LPWRRST                 ((uint8_t)0x7F)

/*****************************************
*自定义数据类型
****************************************/
typedef struct  //2017-09-21 14:40:35
{		//公历日月年周
	vu8  w_year[4] ;  //"2017"
	vu8  dash1 ;      //"-"
	vu8  w_month[2] ; //"09"
	vu8  dash2 ;      //"-"
	vu8  w_date[2] ;  //“21”
	vu8  spacing ;    //" "
	vu8  hour[2] ;    //"14"
	vu8  colon1 ;     //":"
	vu8  min[2] ;     //"40"
	vu8  colon2 ;     //":"
	vu8  sec[2] ;	    //"35"
vu8  week ;	
}Calendar_s ;                             //Calendar_s结构体，纪录实时时间

#define CALENDAR_LEN sizeof(Calendar_s)   //Calendar_s结构体字节长度
	
typedef union uBytes19
{
	Calendar_s 	sCalendar ;
	uint8_t 	  bytes[CALENDAR_LEN] ;
}Calendar_u ;                             //Calendar_u共用体，纪录实时时间

typedef struct 
{
	char 	lon[20] ;
	char 	lat[20];
}LBStype ;                             //共用体，纪录基站定位和时间
typedef void (*RTCFP)(void) ;  //定义函数指针类型变量	
/****************************************************************************/

/*****************************************
*供外部使用的常变量
****************************************/
extern Calendar_u  uCalendar ;
extern LBStype LBS;
/********************************************************************************
*供内部使用的函数声明
*********************************************************************************/
//void RTC_IRQHandler(void) ;                                          //RTC中断处理函数
uint8_t CheckLeepYear(uint16_t year) ; 															 //平年,闰年判断
uint8_t RTC_Get_Week(uint16_t year, uint8_t month, uint8_t day) ;    //根据日期获取星期
void Rtc_Hook(uint16_t rtcIt) ;                                      //RTC中断回调函数钩子函数
void Rtc_Sec_Callback(void) ;                                        //RTC 秒中断回调函数                              
RunResult N32_RTC_Initia(void);
/********************************************************************************
*对外接口函数声明
*********************************************************************************/
//extern RTCFP     Rtc_RegHookCallback(uint16_t rtcIt, RTCFP pCallback)  ;  //RTC中断回调函数注册函数
//extern RunResult RTC_Init(IntPriority_e ePriority);                     //初始化RTC
//extern RunResult RTC_Get(Calendar_u *getCalendar) ;                       //更新时间  
//extern RunResult RTC_Set(Calendar_u *setCalendar);                        //设置时间	
//extern RunResult RTC_Alarm_Set(Calendar_u *setCalendar) ;                 //设置RTC闹钟时间点


#endif







