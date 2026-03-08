#ifndef __USART_H
#define __USART_H
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
//#include "misc.h"
//#include "sys.h" 
#include "n32g45x.h"
#include "n32g45x_usart.h"
#include "n32g45x_it.h" 
//#include "dma.h"
//#include "syslib.h"

/****************************************************************************
* 供外部使用的常变量
****************************************************************************/

/****************************************************************************
*串口相关配置
****************************************************************************/
#define UART_DEBUG                COM1  //log输出重定向         
#define UARTx_DMATX_EN            1     //uart1`uart4可以选择是否通过串口DMA发送数据
#define UART1_DMA_RX_MAXLEN       256   //串口1 DMA接收一帧数据的最大长度
#define UART2_DMA_RX_MAXLEN       256   //串口2 DMA接收一帧数据的最大长度 
#define UART3_DMA_RX_MAXLEN       512   //320 串口3 DMA接收一帧数据的最大长度
#define UART4_DMA_RX_MAXLEN       256   //串口4 DMA接收一帧数据的最大长度 
#define UART5_RX_MAXLEN           256   //串口5 接收缓冲区的最大长度

/****************************************************************************
* 系统层面错误LOG输出
****************************************************************************/
#define SysErr(...)   {printf("LogErr: %s,%d",__FILE__,  __LINE__); printf(__VA_ARGS__) ;}  //输出系统错误
#ifdef  OPENSYSLOG
#define SysLog(...)   {printf("LogSys: ");  printf(__VA_ARGS__) ;}                         //输出系统日志 
#else
#define SysLog(...)
#endif

/*****************************************
*中断事件回调函数相关配置
****************************************/
typedef void (*UARTFP)(char *recvBuf, uint16_t recvLen) ;  //定义函数指针类型变量

/****************************************************************************
* 自定义数据类型
****************************************************************************/
typedef enum
{
  COM1 = 0 ,
	COM2 = 1 ,
	COM3 = 2 ,
	COM4 = 3 ,
	COM5 = 4 ,
	COMSUM = 5
}COM_e ;    /*端口号*/

/****************************************************************************/
extern USART_Module *UARTx_COM[];
/********************************************************************************
*供内部使用的函数声明
*********************************************************************************/
void Uart_Hook(COM_e eCOMn, char *recvBuf, uint16_t recvLen) ; 	      //串口中断回调函数钩子函数
void UARTx_DmaTx_Init(COM_e eCOMn) ;                                  //串口DMA发送初始化
void UARTx_DmaRx_Init(COM_e eCOMn ) ;                                 //串口DMA接收初始化
void UART1_DMA_TX_TC_Callback_Functions(void) ;                   		//串口1DMA发送完成中断回调函数
void UART2_DMA_TX_TC_Callback_Functions(void) ;                   		//串口2DMA发送完成中断回调函数
void UART3_DMA_TX_TC_Callback_Functions(void) ;                   		//串口3DMA发送完成中断回调函数
void UART4_DMA_TX_TC_Callback_Functions(void) ;                   		//串口4DMA发送完成中断回调函数
void UART1_DMA_RX_TC_Callback_Functions(void) ;                   		//串口1DMA接收完成中断回调函数
void UART2_DMA_RX_TC_Callback_Functions(void) ;                   		//串口2DMA接收完成中断回调函数
void UART3_DMA_RX_TC_Callback_Functions(void) ;                   		//串口3DMA接收完成中断回调函数
void UART4_DMA_RX_TC_Callback_Functions(void) ;                   		//串口4DMA接收完成中断回调函数

/********************************************************************************
*对外接口函数声明
*********************************************************************************/
extern void   UARTx_Init(COM_e eCOMn, uint32_t baudrate, uint16_t uartMode, IntPriority_e ePriority)  ; //串口参数初始化
extern void   UARTx_SendData(COM_e eCOMn, const char *Data, uint16_t len); 															//串口发送len长度的数据
extern void   UARTx_SendString(COM_e eCOMn, uint8_t *sendString,...) ;     															//串口发送可变长数据
extern UARTFP Uart_RegHookCallback(COM_e eCOMn, UARTFP pCallback)  ; 	     															//串口中断回调函数注册函数
extern void   USART1_IRQHandler(void)  ;																	 															//串口1中断处理函数
//extern void   USART2_IRQHandler(void)  ;																	 															//串口2中断处理函数
extern void   USART3_IRQHandler(void)  ;																	 															//串口3中断处理函数
extern void   UART4_IRQHandler(void)  ;																	   															//串口4中断处理函数
extern void   UART5_IRQHandler(void)  ;																	   															//串口5中断处理函数

#endif
