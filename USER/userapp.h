#ifndef __USERAPPx_H
#define __USERAPPx_H

//#include "PhoneCode.h"

//#include <stdbool.h>  //bool布尔类型变量定义
//#include <stdint.h>
//#include <stdlib.h>
#include "mac.h" 
//#include "rs232.h"
//#include "rs485.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
//#include "osPort.h"

/*********应用层外部调用文件*******************************************************/
//#include "stm32Temp.h"
//#include "gate_timer.h"
/**********************************************************************************/

/**********************************************************************************
**GateApp配置选项
*********************************************************************************/

/**********************************************************************************
**自定义变量类型
*********************************************************************************/
typedef struct
{
	  char                  macId[MAC_BYTES_LEN+1] ;    //存放 硬件mac地址字符串 的数组
  	char                  appVers[VERSION_LEN+1] ;    //存放 软件版本号字符串 的数组
	  char                  bootVers[VERSION_LEN+1] ;   //存放 boot软件版本号 的数组
	  char                  rstReason[VERSION_LEN+1] ;  //存放 上次设备重启原因 的数组
}Application ;                                        //存放 应用程序一写参数的结构体变量类型

///**********************************************************************************
//*常变量声明
//*********************************************************************************/
extern Application gateApp ;                          //gateApp存放应用参数

#define STATE_OK             1                      	//表示标志位反映的状态正常
#define STATE_ERR            0                     		//表示标志位反映的状态异常
/*OS任务状态事件标志位相关常变量*/
//extern EventGroupHandle_t   osSafeEventHandler ;      //OS任务状态事件标志组（标志位OS每个任务的运行情况）
//#define CHECKTASKINTERVAL   180                 			//180秒(3mins)检测各个任务是否正常被运行
//#define NETBIT_0         	 (EventBits_t)(1 << 0)      //标志位0：反映"网络维护任务"状态 
//#define LTEBIT_1         	 (EventBits_t)(1 << 1)      //标志位1：反映"LTE串口接收任务"状态
//#define CAMERABIT_2      	 (EventBits_t)(1 << 2)      //标志位2：反映"摄像头数据处理任务"状态
//#define TCPUPBIT_3       	 (EventBits_t)(1 << 3)      //标志位3：反映"TCP上传任务"状态
//#define TCPHEARTBIT_4    	 (EventBits_t)(1 << 4)      //标志位4：反映"TCP心跳任务"状态
//#define UCMDBIT_5        	 (EventBits_t)(1 << 5)      //标志位5：反映"用户命令任务"状态

//#define READOSSAFEEVENT     xEventGroupGetBits(osSafeEventHandler)                //获取标志组osSafeEventHandler的值
//#define GETTASKOSSAFE       (((NETBIT_0|LTEBIT_1|CAMERABIT_2|TCPUPBIT_3|TCPHEARTBIT_4|UCMDBIT_5) &  xEventGroupGetBits(osSafeEventHandler))\
//                            == (NETBIT_0|LTEBIT_1|CAMERABIT_2|TCPUPBIT_3|TCPHEARTBIT_4|UCMDBIT_5)) //获取”标志位0&标志位1&标志位2&标志位3&标志位4&标志位5的状态“

/*网络状态事件标志位相关常变量*/
extern EventGroupHandle_t   netEventHandler ;         //网络状态事件标志组（标志位反馈网络状态）
#define MODULEBIT_0   		 (EventBits_t)(1 << 0)      //标志位0：反映EC20 moudle初始化状态 
#define NETREGBIT_1   		 (EventBits_t)(1 << 1)      //标志位1：反映EC20 EC20_Net_Reg初始化状态
#define TCPBIT_2     		   (EventBits_t)(1 << 2)      //标志位2：反映EC20 TCP状态
#define MQTTBIT_3     		 (EventBits_t)(1 << 3)      //标志位3：反映mqtt状态

#define GETMODULESTATE   	 ((MODULEBIT_0 & xEventGroupGetBits(netEventHandler)) == MODULEBIT_0)  //获取“标志位0：EC20 moudle初始化状态"
#define GETNETREGSTATE   	 ((NETREGBIT_1 & xEventGroupGetBits(netEventHandler)) == NETREGBIT_1)  //获取”标志位1：EC20 EC20_Net_Reg初始化状态“
#define GETTCPSTATE      	 ((TCPBIT_2    & xEventGroupGetBits(netEventHandler)) == TCPBIT_2)     //获取”标志位2：EC20 TCP状态“
//#define GETMQTTSTATE     	 ((MQTTBIT_3   & xEventGroupGetBits(netEventHandler)) == MQTTBIT_3)    //获取”标志位3：EC20 MQTT请求状态“
//#define GETUSERNETSTATE  	 (((MODULEBIT_0|NETREGBIT_1|TCPBIT_2|MQTTBIT_3) &  xEventGroupGetBits(netEventHandler)) == (MODULEBIT_0|NETREGBIT_1|TCPBIT_2|MQTTBIT_3)) //获取”标志位0&标志位1&标志位2&标志位3的状态“
//#define GETUSERNETSTATE  	 (((MODULEBIT_0|NETREGBIT_1|TCPBIT_2) &  xEventGroupGetBits(netEventHandler)) == (MODULEBIT_0|NETREGBIT_1|TCPBIT_2)) //获取”标志位0&标志位1&标志位2&标志位3的状态“
#define GETUSERNETSTATE  	 (((MODULEBIT_0|NETREGBIT_1) &  xEventGroupGetBits(netEventHandler)) == (MODULEBIT_0|NETREGBIT_1)) //获取”标志位0&标志位1状态“

#define SETMODULEOK        xEventGroupSetBits(netEventHandler, MODULEBIT_0)  //设置“标志位0：EC20 moudle初始化状态 STATE_OK"
#define SETNETREGOK      	 xEventGroupSetBits(netEventHandler, NETREGBIT_1)  //设置”标志位1：EC20 EC20_Net_Reg初始化状态 STATE_OK“
#define SETTCPOK         	 xEventGroupSetBits(netEventHandler, TCPBIT_2)     //设置”标志位2：EC20 TCP状态 STATE_OK““
#define SETMQTTOK        	 xEventGroupSetBits(netEventHandler, MQTTBIT_3)    //设置”标志位3：EC20 MQTT请求状态 STATE_OK“““

#define CLEATCP        	 xEventGroupClearBits(netEventHandler,	TCPBIT_2)  //清零“标志位2：EC20 TCP状态 STATE_ERR"
#define CLEAMQTT       	 xEventGroupClearBits(netEventHandler,	MQTTBIT_3) //清零”标志位3：EC20 MQTT请求状态 STATE_ERR“
#define CLEANETREG    	 xEventGroupClearBits(netEventHandler,	NETREGBIT_1|TCPBIT_2) //|MQTTBIT_3清零”标志位1、标志位2、标志位3的状态 STATE_ERR“ 
#define CLEAMODULE     	 xEventGroupClearBits(netEventHandler,	MODULEBIT_0|NETREGBIT_1|TCPBIT_2) //|MQTTBIT_3清零”标志位0、标志位1、标志位2、标志位3的状态 STATE_ERR“ 

/*报警状态标志组*/
extern EventGroupHandle_t   AlarmStateHandler ;         //报警状态事件标志组（标志位反馈报警状态）
#define AlarmWaitBIT_0   		 (EventBits_t)(1 << 0)      //等待报警就绪 
#define SMS_ReadyBIT_1   		 (EventBits_t)(1 << 1)      //短信发送准备就绪
#define SMS_ReSendBIT_2      (EventBits_t)(1 << 2)      //短信重发
#define SMS_FailBIT_3      (EventBits_t)(1 << 3)      //短信发送失败
#define SMS_UnderWayBIT_4      (EventBits_t)(1 << 4)      //短信发送中
#define Call_ReadyBIT_5   		 (EventBits_t)(1 << 5)      //拨打电话准备就绪
#define Call_RepBIT_6   		 (EventBits_t)(1 << 6)      //电话重拨
#define Call_FailBIT_7      (EventBits_t)(1 << 7)      //电话拨打失败
#define Call_UnderWayBIT_8      (EventBits_t)(1 << 8)      //电话拨打中
#define CTTS_ingBIT_9      (EventBits_t)(1 << 9)      //TTS进行中
#define Rs485SlaveAlarmBIT_10   (EventBits_t)(1 << 10)      //等待RS485 从机就绪
#define Rs485HostBIT_11   (EventBits_t)(1 << 11)      //等待RS485主机 就绪
#define Rs485ReceiveBIT_12   (EventBits_t)(1 << 12)      //等待RS485接收数据就绪
//extern QueueHandle_t Message_Queue;	//信息队列句柄
/*EC20资源抢占互斥信号量相关*/
extern SemaphoreHandle_t  ec20MutexSemaphore ;         //互斥信号量句柄（EC20资源抢占及释放）
//extern SemaphoreHandle_t  WeightTempSemaphore;
extern char                lastOccupyEc20[50] ;         //全局数组 记录上一次占用EC20资源的__FILE__,  __LINE__
extern char                lastReleseEc20[50] ;         //全局数组 记录上一次释放EC20资源的__FILE__,  __LINE__
#define OCCUPY_EC20(t)	 xSemaphoreTake(ec20MutexSemaphore, t);\
													memset(lastOccupyEc20, 0, 50);\
                         snprintf( lastOccupyEc20, 49, "%s %d 占用4G!",  __FILE__,  __LINE__) ;\
												 AppLogPrintf(lastOccupyEc20);
//												 \
//												 UARTx_SendData(UART_DEBUG, lastOccupyEc20, strlen(lastOccupyEc20))//  portMAX_DELAY  //等待占用EC20资源  一直等待：t = portMAX_DELAY
#define RELESE_EC20(void)  xSemaphoreGive(ec20MutexSemaphore); \
													 memset(lastReleseEc20, 0, 50);\
                           snprintf( lastReleseEc20, 49, "%s %d 释放4G!",  __FILE__,  __LINE__) ;\
													 AppLogPrintf(lastReleseEc20) ;
//													 \
//													 UARTx_SendData(UART_DEBUG, lastReleseEc20, strlen(lastReleseEc20)) //释放EC20资源


/**********************************************************************************
*内部函数声明
***********************************************************************************/

/********************************************************************************
*对外接口函数声明
*********************************************************************************/
extern void       OccpyEc20(TickType_t timeout, char* file, int line) ; 	//获取EC20资源， 如果超过timeout没有获取到资源则写入LOG然后重启系统
extern void 			Board_Init(void) ;                                    	//驱动及功能块初始化
//extern void 			PrintfDeviceInfo(void) ;                              	//DEBUG口输出设备信息
extern void 			DeviceRstReason(uint8_t *reason, uint8_t maxLen) ;    	//获取硬件上次重启原因
//extern void       TcpUpFlashLog(void) ;                                 	//通过TCP上传本地FLASH存储的LOG
extern void 			Rtc_Alr_Callback(void) ;                              	//RTC闹钟中断回调函数，中断内先重设闹钟时间为第二天，同时重启设备
//extern void 			InitApplictationState(Application *appPointer);         //初始化Application结构体变量
//extern void 			SetAppVersion(Application *appPointer, char *version) ; //设置Application结构体变量中的APP软件版本
//extern void 			SetBootVersion(Application *appPointer, char *version) ;//设置Application结构体变量中的boot软件版本
//extern void 			SetMacId(Application *appPointer, char *macId)  ;       //设置Application结构体变量中的macid
RunResult        RefreshOledTime(void) ;                                 //刷新显示屏第3行、4行显示时间
//extern void 			RefreshWeightTime(void);															//RS485刷新采集重量，温湿度等数据
extern void 			RunTaskTime(void);
//extern void SendRs485Call(void);

#endif
