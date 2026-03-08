#ifndef __SYSPORT_H
#define __SYSPORT_H
//#include <stdint.h>
#include <string.h>
//#include "usart_n32.h"   
#include "sysTick.h" 


/*****************************************
*自定义
****************************************/
#define Wait_For_Nus(us) Delay_Us(us)          //systick实现的us延时函数 使用 Wait_For_Nus函数作为对上层的接口    
#define Wait_For_Nms(ms) Delay_Ms(ms)          //systick实现的ms延时函数 使用 Wait_For_Nms函数作为对上层的接口
//#define vAssertCalled(char,int) printf("Error:%s,%d\r\n",char,int)
//#define configASSERT(x) if((x)==0) vAssertCalled(__FILE__, __LINE__)
#define LOG_BUF_LEN                  512       //AppLogPrintf、DebugLogPrintf、ErrorLogPrintf函数输出的日志最大长度
#define configERRORLOGPRINTF_ENABLE  1         //ErrorLogPrintf()函数输出使能
#define configDEBUGLOGPRINTF_ENABLE  1         //1//DebugLogPrintf()函数输出使能
#define configAPPLOGPRINTF_ENABLE    0        //1 //AppLogPrintf()函数输出使能
														 
/*******************************************************************************
*内部函数声明
*******************************************************************************/


/*******************************************************************************
*对外接口函数声明
*******************************************************************************/
extern void *portMalloc(size_t xWantedSize ) ;     //提供SYSTEM上层动态分配内存空间的接口
extern void  portFree(void *addr ) ;               //提供SYSTEM上层动态释放内存空间的接口
extern void  AppLogPrintf( char *format, ...) ;    //APP相关LOG输出
extern void  DebugLogPrintf( char *format, ...) ;  //Debut相关LOG输出
extern void  ErrorLogPrintf( char *format, ...) ;  //Error相关LOG输出
extern char *gcvt(double value, int ndigit, char *buf);
#endif

