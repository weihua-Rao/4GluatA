#include "sysport.h"
#include <string.h>
#include "usart.h" 
#include "rtc.h"   
#include "malloc.h"
#include "portable.h"	
/*********************************************************************************************************************
* 名    称：void *portMalloc(size_t xWantedSize )
* 功    能：对pvPortMalloc进行封装，在分配完内存后，将空间初始化为0
* 说    明：实验中发现存在分配的内存空间不是空的情况，所以对pvPortMalloc做一次封装。
*********************************************************************************************************************/
void *portMalloc(size_t xWantedSize )
{
#ifdef  FREERTOS_MALLOC
	void *p = pvPortMalloc(xWantedSize);
			
	if( p == NULL)
		{
			SysErr("") ; //内存再次分配失败！					
			NVIC_SystemReset();
		}
	return p ;
#elseif
	
  void *p = NULL ;
//	uint8_t fBytes = xWantedSize%4  ;
//	if(  fBytes != 0 )  //4字节对齐处理
//	  {
//		  xWantedSize = xWantedSize+(4-fBytes) ;
//		}
	p = MyMalloc(SRAMIN, xWantedSize) ;
  uint8_t usedRate = MyMenPerused(SRAMIN);
	
	if( usedRate > 75 )
	  {
			SysErr("\r\n动态内存占用已超过：%d%%！\r\n", usedRate) ;
		}
	if( p != NULL )
		{
			memset(p, 0, xWantedSize) ;  //实验中发现存在分配的内存空间不是空的情况，所以对pvPortMalloc做一次封装。
		}
	else
	  {			
			SysErr("\r\n内存占用：%d%%--动态内存溢出！初始化动态内存空间！", usedRate) ;
		  MyMenInit(SRAMIN) ;                         //动态内存分配初始化
			p = MyMalloc(SRAMIN, xWantedSize) ;
			if( p == NULL)
			  {
				  SysErr("") ; //内存再次分配失败！					
//					SystemSoftReset() ;
					NVIC_SystemReset();
				}
		}
	return p ;
#endif
}

/*********************************************************************************************************************
* 名    称：void *portFree(void *addr )
* 功    能：释放由portMalloc动态分配的内存空间
*********************************************************************************************************************/
void portFree(void *addr )
{
#ifdef  FREERTOS_MALLOC
     vPortFree(addr) ;
#else
		 MyFree(SRAMIN, addr);	
#endif
	addr = NULL ;
}

/*********************************************************************************************************************
* 名    称：void AppLogPrintf( char *format, ...)
* 功    能：USB口输出应用信息
* 说    明：输出格式为：
*								 APP Log:
*											Time:14:40:58  同步包月车成功,挂起vip_task！
*********************************************************************************************************************/
void AppLogPrintf( char *format, ...)
{
#if(configAPPLOGPRINTF_ENABLE == 1)
    {
			va_list ap;                          //ap指向参数的地址 
			va_start (ap, format);               //让ap指针指向可变参数表里的第一个参数
			char *log = portMalloc(LOG_BUF_LEN) ;
			//printf("\r\nAppLog: %s ", &uCalendar.bytes[11])	;	
			//int outLen1 = snprintf(log, LOG_BUF_LEN, "\r\nAppLog: %s ", &uCalendar.bytes[11]);//包含时间戳
      int outLen1 = snprintf(log, LOG_BUF_LEN, "LogApp:"); 			                          //不包含时间戳
			int outLen2 = vsnprintf((log+outLen1), LOG_BUF_LEN-outLen1, (const char*)format, ap);	
      //strcat(log, "\r\n") ;			
			if(outLen2 > 0 ) 	
				  UARTx_SendData(UART_DEBUG, log, outLen2+outLen1);
      else                   //vsnprintf执行失败
          SysErr("") ;			
			va_end(ap) ;				
			portFree(log) ;
    }
#endif
}

/*********************************************************************************************************************
* 名    称：void DebugLogPrintf( char *format, ...)
* 功    能：USB口输出调试信息
* 说    明：输出格式为： 
*								 DEBUG Log:
*											Time:14:40:58  同步包月车成功,挂起vip_task！
*********************************************************************************************************************/
void DebugLogPrintf( char *format, ...)
{
#if(configDEBUGLOGPRINTF_ENABLE == 1)
    {	
			va_list ap;            //ap指向参数的地址 
			va_start (ap, format); //让ap指针指向可变参数表里的第一个参数
			char *log = portMalloc(LOG_BUF_LEN) ;
			//printf("\r\nDebugLog: %s ", &uCalendar.bytes[11])	;
			//int outLen = vsnprintf(log, LOG_BUF_LEN, (const char*)format, ap); 
			//int outLen1 = snprintf(log, LOG_BUF_LEN, "\r\nDebLog: %s ", &uCalendar.bytes[11]); 
			int outLen1 = snprintf(log, LOG_BUF_LEN, "LogDeb:"); 
			int outLen2 = vsnprintf((log+outLen1), LOG_BUF_LEN-outLen1, (const char*)format, ap); 
			//strcat(log, "\r\n") ;
			if(outLen2 > 0) 
				  UARTx_SendData(UART_DEBUG, log, outLen2+outLen1);
      else                                                         	//vsnprintf执行失败
			    SysErr("") ;				
			va_end(ap) ;				
			portFree(log) ;
    }
#endif
}

/*********************************************************************************************************************
* 名    称：void ErrorLog( char *format, ...)
* 功    能：UART_DEBUG口输出错误信息
* 说    明：输出格式为： DEBUG Log: 14:40:20 + log内容
*********************************************************************************************************************/
void ErrorLogPrintf( char *format, ...)
{
#if(configERRORLOGPRINTF_ENABLE == 1)
    {
      va_list ap;            //ap指向参数的地址 
			va_start (ap, format); //让ap指针指向可变参数表里的第一个参数
			char *log = portMalloc(LOG_BUF_LEN) ;
			//printf("\r\nErrLog: %s ", &uCalendar.bytes[11])	;
			//int outLen = vsnprintf(log, LOG_BUF_LEN, (const char*)format, ap); 
			//int outLen1 = snprintf(log, LOG_BUF_LEN, "\r\nErrLog: %s ", &uCalendar.bytes[11]);
      int outLen1 = snprintf(log, LOG_BUF_LEN, "LogErr:");			
			int outLen2 = vsnprintf((log+outLen1), LOG_BUF_LEN-outLen1, (const char*)format, ap);
		//	strcat(log, "\r\n") ;
			if(outLen2 > 0) 	
				 UARTx_SendData(UART_DEBUG, log, outLen2+outLen1);
//			    printf((const char*)log) ;
      else                                                      //vsnprintf执行失败
			    SysErr("") ;	
			va_end(ap) ;				
			portFree(log) ;
    }
#endif
}





