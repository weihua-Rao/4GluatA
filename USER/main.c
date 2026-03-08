#define Main_def
#include "main.h"

/**********************************************************************************
*内部函数声明
**********************************************************************************/
/*FreeRTOS任务函数声明*/
void StartTask(void *pvParameters);     //开始任务函数    ：初始化、创建其他任务
void LedTask(void *pvParameters);       //LED任务函数     ：DATA作为500ms呼吸灯
void ScanPortTask(void *pvParameters);
void NetTask(void *pvParameters);       //网络任务函数    ：初始化网络、维护网络
      //用户命令任务函数：处理用户命令（用户命令可能来自USB口或者EC20的TCP下行数据）
void SetModesTask(void *pvParameters);
void AlarmTask(void *pvParameters);     //查询AlarmQueue是否有报警任务，向短信发送任务发送任务通知
//void TcpDownTask(void *pvParameters);
void MessageSendTask(void *pvParameters);//发送短信，任务等待时间1秒，所有号码发送完后，向拨打电话任务发送任务通知
void PhoneCallTask(void *pvParameters);//拨打电话 ,向处理电话回执消息任务发送任务通知
void CallResultTask (void *pvParameters);//处理拨打电话后回执消息 忙线 挂机 接听中播放TTS 等待任务通知 来自发送短信任务 打电话任务
void Host_Rs485GetAlarm(void *pvParameters);       //USB任务函数   ：处理USB串口数据
void Execute_SlaveAlarm(void *pvParameters);
void MQTT_Publish(void *pvParameters);
void WIFI_onlinDet(void *pvParameters);       //wifi 网络检测
/**********************************************************************************
*内部常变量
**********************************************************************************/
/*FreeRTOS任务优先级*/
#define START_TASK_PRIO		 	14    //开始任务 H
#define CallResult_TASK_PRIO 13        //
#define Phone_TASK_PRIO		 	 12       //
#define Message_TASK_PRIO		 11 
#define NET_TASK_PRIO		   	10        //2网络任务
#define ScanPort_TASK_PRIO	 9
#define MQTT_Publish_TASK_PRIO		 8
#define Host_Rs485_TASK_PRIO		 7        //rs485优先级
#define Execute_Slave_TASK_PRIO		 6
#define Alarm_TASK_PRIO		 	 5       //ucmd优先级
#define SetModes_TASK_PRIO	 4 
#define KeyQuery_TASK_PRIO	 3
#define WIFI_TASK_PRIO			2
#define LED_TASK_PRIO		    1      //30任务优先级
//#define DEBUG_TASK_PRIO		 1        //任务优先级   L

/*FreeRTOS任务堆栈大小*/
#define START_STK_SIZE 		256       //任务堆栈大小	//256个入栈空间（256*4 bytes）
#define LED_STK_SIZE 		  256       //128任务堆栈大小	
#define ScanPort_STK_SIZE    256
#define NET_STK_SIZE 		  256       //任务堆栈大小	//256个入栈空间（256*4 bytes）
#define SetModes_STK_SIZE 		256
#define Alarm_STK_SIZE 		256       //任务堆栈大小
//#define TcpDown_STK_SIZE 		256       //任务堆栈大小	//256个入栈空间（256*4 bytes）
#define Message_STK_SIZE 		256 
#define Phone_STK_SIZE 		128       //任务堆栈大小
#define CallResult_STK_SIZE 		128       //任务堆栈大小	//256个入栈空间（256*4 bytes）
#define KeyQuery_STK_SIZE 		256 
#define Host_Rs485_STK_SIZE 		128       //任务堆栈大小
#define Execute_Slave_STK_SIZE 		128 
#define MQTT_Publish_STK_SIZE 		128 
#define WIFI_onlinDet_STK_SIZE 		128
/*FreeRTOS任务句柄*/
TaskHandle_t StartTaskHandler;      //任务句柄
TaskHandle_t LedTaskHandler;        //任务句柄
TaskHandle_t ScanPortTaskHandler;
TaskHandle_t NetTaskHandler;        //任务句柄
//TaskHandle_t TcpUrcTaskHandler;
TaskHandle_t SetModesTaskHandler;
TaskHandle_t AlarmTaskHandler;       //任务句柄
//TaskHandle_t TcpDownTaskHandler;      //任务句柄 
TaskHandle_t MessageTaskHandler;
TaskHandle_t PhoneTaskHandler;       //任务句柄
TaskHandle_t CallResultTaskHandler;      //任务句柄 
TaskHandle_t KeyQueryTaskHandler;
TaskHandle_t Host_Rs485TaskHandler;
TaskHandle_t Execute_SlaveTaskHandler;      //任务句柄
TaskHandle_t MQTT_PublishTaskHandler;      //任务句柄
TaskHandle_t WIFI_onlinDetTaskHandler; 
//队列
#define AlarmPort_Q_NUM   25   	//报警队列的数量 
QueueHandle_t AlarmPort_Queue;	//报警队列句柄

/*FreeRTOS软件定时器句柄*/
//TimerHandle_t timeRefresh_h ;       //显示屏时间刷新
//TimerHandle_t RS485WeightRefresh_h ;       //RS485刷新
//TimerHandle_t RunTaskRefresh_h ;       //通用定时器
/*RS485采集任务标志组*/
//EventGroupHandle_t Rs485EventHandler ;
/*数据完成标志组*/
EventGroupHandle_t AlarmStateHandler;
//EventGroupHandle_t  DataFinishHandler ;
u8 Call_Delay;//电话拨打延时
	
int main(void)
{
 /* System Clocks Configuration */
    RCC_Configuration();
    /* NVIC configuration */
    NVIC_Configuration();
	  NVIC_SetVectorTable(FLASH_BASE, FLASH_OFFSET) ;   //设置中断向量表的位置和偏移量  SCB->VTOR = FLASH_BASE | FLASH_OFFSET ;  
//	  Watchdog_Feed() ;                                 //独立看门狗喂狗
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4) ;  //设置系统中断优先级分组4	  无子优先级、0~15共计16个抢占优先级	  
//		InitIapFlashConfig(&uIapFlash) ;                  //读出FLASH中的Iap配置，初始化共用体变量uIapFlash，最后重新写入FLASH中	
//		InitAppFlashConfig(&uAppFlash) ;                  //读出FLASH中的App配置，初始化共用体变量uAppFlash，最后重新写入FLASH中	
// 	  InitApplictationState(&gateApp);                  //初始化结构体变量gateApp  
	GPIO_Configuration();
PWR_LVoff = 0;//打开电池供电	
	UsbInit(222500) ;                               //222500 调试串口、串口转USB口初始化
	EC20_Uart_Init() ;                             //EC20_UART串口初始化,初始化接收缓冲区、注册串口回调函数
	Wifi_Uart_Init();
//	UARTx_SendData(WIFI_UART, "AT+WRSSI\r\n", strlen("AT+WRSSI\r\n"))	 ;
//InitQueueMem(&sUsercmdQueue) ;
//SET_AlarmValue_TemperHum(MsegTemp);
//	Watchdog_Init();
	
/**创建开始任务**/
    xTaskCreate((TaskFunction_t )StartTask,           //任务函数
                (const char*    )"start_task",        //任务名称
                (uint16_t       )START_STK_SIZE,      //任务堆栈大小
                (void*          )NULL,                //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,     //任务优先级
                (TaskHandle_t*  )&StartTaskHandler    //任务句柄 
               ) ;								
    vTaskStartScheduler() ;                           //开启任务调度  以上代码段系统还未启动，所有可屏蔽中断都出去屏蔽状态。
}

//开始任务任务函数
void StartTask(void *pvParameters)
{
//	BaseType_t  xNewQueue = pdTRUE;//pdFALSE;  
//	  Watchdog_Feed() ;                                 //看门狗喂狗
  taskENTER_CRITICAL();                             //进入临界区		    
  Board_Init() ;                                    //硬件驱动初始化和功能块初始化
//	HomeMenu() ;                                      //OLED显示主界面
//OLED_DrawBMP(120, 0, 8, 16, false, WIFI_online[0]);
//	OLED_DrawBMP(120, 0, 8, 16, false, WIFI_online[1]);
//WriteLogToFlash("OLED CHINA!") ;
	TIM_Configuration();	
	Trigger1=Trigger2=Trigger3=Trigger4=Trigger5=Trigger6=Trigger7=Trigger8=1;rs485_rw = 1;
	MYI2C_Handle(&SENx);
	//		T_Adc_Init();//ADC 
/*创建网络状态标志组*/
	  netEventHandler = xEventGroupCreate() ;           //网络状态事件标志位，相当于一个状态机，标志网络状态的各个环节         
/*报警标志组*/
	  AlarmStateHandler = xEventGroupCreate() ;           //网络状态事件标志位，相当于一个状态机，标志网络状态的各个环节         
/*创建端口报警队列*/
	AlarmPort_Queue = xQueueCreate(AlarmPort_Q_NUM,sizeof(Alarmtype));//sizeof(u8)*4*
/*创建EC20资源互斥信号量*/
	ec20MutexSemaphore = xSemaphoreCreateMutex(); 	  //创建EC20资源互斥信号量，EC20模块需要有严格的命令时序，所以无论哪个任务需要操作EC20都需要等待EC20资源空闲。
	
	Watchdog_Feed() ;
/*创建任务*/
//		xTaskCreate((TaskFunction_t)KeyQueryTask,    (const char*)"KeyQueryTask",    (uint16_t)KeyQuery_STK_SIZE,    (void*)NULL, (UBaseType_t)KeyQuery_TASK_PRIO,     (TaskHandle_t*)&KeyQueryTaskHandler);		
//		xTaskCreate((TaskFunction_t)TcpUrcTask,   (const char*)"TcpUrcTask",   (uint16_t)TcpUrc_STK_SIZE,   (void*)NULL,	(UBaseType_t)TcpUrc_TASK_PRIO,	   (TaskHandle_t*)&TcpUrcTaskHandler);
if(RegisterEEpro_Slave.Host_or_Slave == Host_Flag)//六通道RS485主机
		{
		xTaskCreate((TaskFunction_t)Host_Rs485GetAlarm,(const char*)"Host_Rs485_task", (uint16_t)Host_Rs485_STK_SIZE,   (void*)NULL,  (UBaseType_t)Host_Rs485_TASK_PRIO,   (TaskHandle_t*)&Host_Rs485TaskHandler);   
//    xTaskCreate((TaskFunction_t)NetTask,     (const char*)"NetTask",     (uint16_t)NET_STK_SIZE,     (void*)NULL,	(UBaseType_t)NET_TASK_PRIO,  	   (TaskHandle_t*)&NetTaskHandler);  
		xTaskCreate((TaskFunction_t)AlarmTask,    (const char*)"AlarmTask",    (uint16_t)Alarm_STK_SIZE,    (void*)NULL, (UBaseType_t)Alarm_TASK_PRIO,     (TaskHandle_t*)&AlarmTaskHandler);		
		xTaskCreate((TaskFunction_t)MessageSendTask,   (const char*)"MessageSendTask", (uint16_t)Message_STK_SIZE,   (void*)NULL,	(UBaseType_t)Message_TASK_PRIO,	   (TaskHandle_t*)&MessageTaskHandler);
		xTaskCreate((TaskFunction_t)PhoneCallTask,    (const char*)"PhoneCallTask",    (uint16_t)Phone_STK_SIZE,    (void*)NULL, (UBaseType_t)Phone_TASK_PRIO,     (TaskHandle_t*)&PhoneTaskHandler);		
		xTaskCreate((TaskFunction_t)CallResultTask,    (const char*)"CallResultTask",    (uint16_t)CallResult_STK_SIZE,    (void*)NULL, (UBaseType_t)CallResult_TASK_PRIO,     (TaskHandle_t*)&CallResultTaskHandler);		
		xTaskCreate((TaskFunction_t)MQTT_Publish,    (const char*)"MQTT_PublishTask",    (uint16_t)MQTT_Publish_STK_SIZE,    (void*)NULL, (UBaseType_t)MQTT_Publish_TASK_PRIO,     (TaskHandle_t*)&MQTT_PublishTaskHandler);		
		xTaskCreate((TaskFunction_t)WIFI_onlinDet,    (const char*)"WIFI_onlinDetTask",    (uint16_t)WIFI_onlinDet_STK_SIZE, (void*)NULL, (UBaseType_t)WIFI_TASK_PRIO,     (TaskHandle_t*)&WIFI_onlinDetTaskHandler);		
		}
	else if(RegisterEEpro_Slave.Host_or_Slave == Slave_Flag)	//六通道RS485从机
	{
		xTaskCreate((TaskFunction_t)Execute_SlaveAlarm,(const char*)"Execute_SlaveTask",(uint16_t)Execute_Slave_STK_SIZE,(void*)NULL, (UBaseType_t)Execute_Slave_TASK_PRIO,(TaskHandle_t*)&Execute_SlaveTaskHandler);		
// 		xTaskCreate((TaskFunction_t)RS485_CMD,    (const char*)"RS485_CMDTask",    (uint16_t)RS485_CMD_STK_SIZE,    (void*)NULL, (UBaseType_t)RS485_CMD_TASK_PRIO,     (TaskHandle_t*)&RS485_CMDTaskHandler);		
	}
	else//八通道主机
	{
		xTaskCreate((TaskFunction_t)AlarmTask,    (const char*)"AlarmTask",    (uint16_t)Alarm_STK_SIZE,    (void*)NULL, (UBaseType_t)Alarm_TASK_PRIO,     (TaskHandle_t*)&AlarmTaskHandler);		
		xTaskCreate((TaskFunction_t)MessageSendTask,   (const char*)"MessageSendTask", (uint16_t)Message_STK_SIZE,   (void*)NULL,	(UBaseType_t)Message_TASK_PRIO,	   (TaskHandle_t*)&MessageTaskHandler);
		xTaskCreate((TaskFunction_t)PhoneCallTask,    (const char*)"PhoneCallTask",    (uint16_t)Phone_STK_SIZE,    (void*)NULL, (UBaseType_t)Phone_TASK_PRIO,     (TaskHandle_t*)&PhoneTaskHandler);		
		xTaskCreate((TaskFunction_t)CallResultTask,    (const char*)"CallResultTask",    (uint16_t)CallResult_STK_SIZE,    (void*)NULL, (UBaseType_t)CallResult_TASK_PRIO,     (TaskHandle_t*)&CallResultTaskHandler);		
    xTaskCreate((TaskFunction_t)NetTask,     (const char*)"NetTask",     (uint16_t)NET_STK_SIZE,     (void*)NULL,	(UBaseType_t)NET_TASK_PRIO,  	   (TaskHandle_t*)&NetTaskHandler);  
		xTaskCreate((TaskFunction_t)MQTT_Publish,    (const char*)"MQTT_PublishTask",    (uint16_t)MQTT_Publish_STK_SIZE,    (void*)NULL, (UBaseType_t)MQTT_Publish_TASK_PRIO,     (TaskHandle_t*)&MQTT_PublishTaskHandler);		
		xTaskCreate((TaskFunction_t)WIFI_onlinDet,    (const char*)"WIFI_onlinDetTask",    (uint16_t)WIFI_onlinDet_STK_SIZE, (void*)NULL, (UBaseType_t)WIFI_TASK_PRIO,     (TaskHandle_t*)&WIFI_onlinDetTaskHandler);		
	}
	  xTaskCreate((TaskFunction_t)LedTask,     (const char*)"LedTask",     (uint16_t)LED_STK_SIZE,     (void*)NULL,	(UBaseType_t)LED_TASK_PRIO,	     (TaskHandle_t*)&LedTaskHandler);   
	  xTaskCreate((TaskFunction_t)ScanPortTask,(const char*)"ScanPortTask",(uint16_t)ScanPort_STK_SIZE,(void*)NULL,	(UBaseType_t)ScanPort_TASK_PRIO, (TaskHandle_t*)&ScanPortTaskHandler);   
		xTaskCreate((TaskFunction_t)SetModesTask,   (const char*)"SetModesTask",   (uint16_t)SetModes_STK_SIZE,   (void*)NULL,	(UBaseType_t)SetModes_TASK_PRIO,	   (TaskHandle_t*)&SetModesTaskHandler);
	
		vTaskDelete(StartTaskHandler) ;                  //删除开始任务
    taskEXIT_CRITICAL() ;                            //退出临界区
}

/*********************************************************************************************************************
* SIM7600初始化—-网络状态检测
*********************************************************************************************************************/
void NetTask(void *pvParameters)
{
//	RunResult NetResult;
	while(1)                                     //EC20初始化过程对时序要求很高，所以在此期间需要完全占用EC20资源
    {	
#if ( configCHECK_FOR_STACK_OVERFLOW > 0 )			
UBaseType_t stack_high_watermark = uxTaskGetStackHighWaterMark(NetTaskHandler);//获取任务堆栈最高水平线
if (stack_high_watermark < NET_STK_SIZE) {
    // 堆栈溢出
} else {
    // 堆栈未溢出
}
#endif	
//EventBits_t uu = xEventGroupGetBits(netEventHandler);	
	
			static uint8_t failedTimes = 0 ;
//OccpyEc20(60*configTICK_RATE_HZ, __FILE__,  __LINE__) ;  //抢占EC20资源，超时时间位60S，如果超时则重启系统			
//			if( !DeivceFlag.StratUPFlag )                    //GETUSERNETSTATE MODULEBIT_0\NETREGBIT_1\TCPBIT_2\MQTTBIT_3 标志位全部置1，说明网络全部正常
			if(GETUSERNETSTATE )  
				{
//				EC20_Query_CSQ(sEc20Param.csq);					
////				if(RUNOK != EC20_Query_CSQ(sEc20Param.csq)){CLEAMODULE;DeivceFlag.StratUPFlag=1;} //获取CSQ的值
////				if(RUNOK != EC20_SendModuleCmd(AT_COPS, NULL)){CLEAMODULE;DeivceFlag.StratUPFlag=1;}	//发送模块指令 检测是否正常在线
//				if(W433Hard.Head.DeviceAlarmMode == Sim_only){if(ERROR == EC20_CEREG()){CLEAMODULE;DeivceFlag.StratUPFlag=1;}}//    SIM卡报警				
//				else {
//						if(RUNOK != EC20_SendHttpCmd(MQTTSTATU, NULL)){CLEATCP;DeivceFlag.StratUPFlag=1;}
//						}	//DeivceFlag.GSMRet_Flag = 1;发送模块指令 检测MQTT是否正常在线

				failedTimes = 0 ;                    //网络维护失败计次清零
				}
			else                                     //MODULEBIT_0\NETREGBIT_1\TCPBIT_2\MQTTBIT_3 标志位全部置1，说明网络全部正常
			  {
/**测试用的 检测是否会重启**/					
//			Send_Alarm.Num = 8;Send_Alarm.Adrr_Offset = Close_Message_offset;
//			xQueueSend(AlarmPort_Queue,&Send_Alarm,0);					
					
					Tled = 0;
					if(DeivceFlag.StratUPFlag){vTaskSuspend(ScanPortTaskHandler);DeivceFlag.GSMRet_Flag = 1;}//如果重新开机 执行一次挂起端口扫描任务
					vTaskSuspend(LedTaskHandler);//挂起Led任务
					
//					xEventGroupClearBits(AlarmStateHandler, AlarmWaitBIT_0 | SMS_ReadyBIT_1 | Call_ReadyBIT_5);		//关闭获取报警事件 ，短信 电话 接收任务
//          xEventGroupClearBits(AlarmStateHandler,(EventBits_t)0);	//清零标志组
					failedTimes++ ;		                   //网络维护失败计次累加	
          if( failedTimes>2 )	               //网络维护失败计次达到2次  直接重启系统
					  {
							WriteLogToFlash("系统网络崩溃，重启系统!") ;
              NVIC_SystemReset();//SystemSoftReset() ;            
						}
//						OCCUPY_EC20(portMAX_DELAY);  //等待占用EC20资源  一直等待
					OccpyEc20(60*configTICK_RATE_HZ, __FILE__,  __LINE__) ;  //抢占EC20资源，超时时间位60S，如果超时则重启系统
					RunResult runResult = TIMEOUT ;					
					if( GETMODULESTATE == STATE_ERR )    /*EC20没有module初始化成功则执行*/
						{
						StatusBarPrintf("握手中:") ;	   //显示屏状态栏显示“握手中：”
						runResult = EC20_Module_Init() ; //需要保证串口通信正常且关闭模块串口ECHO成功
						if( RUNOK == runResult) 				 /*模块UART \gpio\POW\ECHO\RELESE皆无异常*/
							{
								SETMODULEOK ;                //设置“标志位0：EC20 moudle初始化状态 STATE_OK"
								AppLogPrintf("握手成功!") ;   
							StatusBarPrintf("握手成功!") ;
							}
						else                             /*EC20_Module_Init初始化失败不做异常处理，该任务会一直运行EC20_Module_Init()*/ 
							{
								StatusBarPrintf("握手失败!") ;
								AppLogPrintf("握手失败") ;
//								NVIC_SystemReset();
//								SETMODULEOK;
							}
						}
					 Delay_Ms_StopScheduler(600) ;
					if((GETMODULESTATE == STATE_OK) &&
							GETNETREGSTATE == STATE_ERR )           //  EC20module初始化完成，且EC20_Net_Reg初始化未完成，则进行CS、PS初始化
						{
							OledClearLine(LINE1) ;
						StatusBarPrintf("入网中:") ;            //显示屏状态栏显示“入网中” 
				
						char *errInfo = portMalloc(30) ;
						runResult = SimComInti(errInfo, 30) ; //连接运营商核心网，即CS SERVER、PS SERVER
						
							if(runResult == RUNOK) 				        // EC20_Net_Reg（SIM/CS/PS）皆无异常则往下执行
							{
								StatusBarPrintf("入网成功") ;
								SETNETREGOK ;                       //设置”标志位1：EC20 EC20_Net_Reg初始化状态 STATE_OK“
							
								SAPBR_Init();
//								if(RUNOK == SYNC_NetTime())        		//同步网络时间 香港澳台 只能获取时间 不能用基站定位 没数制
								if(RUNOK != SYNC_LBS_Time())        		//基站定位LBS信息和时间	更新时间
									SYNC_NetTime();
//								RefreshOledTime() ;                 //OLED更新时间							
							}
						else                                    //1、EC20_Net_Reg异常，需要重新进行EC20_Module_Init; 2、模块关机
							{
//									WriteLogToFlash(errInfo) ;
								StatusBarPrintf("入网失败") ;
								CLEAMODULE ;                      //设置“标志位0：EC20 moudle初始化状态 清零"
								EC20_CLOSE() ;                      //EC20关机    
							}
						AppLogPrintf(errInfo) ;                 //提示错误：请插入SIM卡、SIM欠费、CSQ
						portFree(errInfo) ;
						}
					if(  (GETMODULESTATE == STATE_OK) &&      
							 (GETNETREGSTATE == STATE_OK) && 
							 (GETTCPSTATE    == STATE_ERR) &&  
							 (W433Hard.Head.DeviceAlarmMode != Sim_only))       //EC20module、EC20_Net_Reg初始化都完成，则进行AppTcpInit初始化
						{
							StatusBarPrintf("连接TCP服务器:");     //显示屏状态栏显示“连接TCP服务器:”
							runResult=RUNOK; 
							runResult = AppTcpInit() ;           		//TCP PDP初始化 SOCKET连接
							if( RUNOK == runResult) 				    		//TCP连接成功 
								{ 
									SETTCPOK ;                          //设置”标志位2：EC20 TCP状态 STATE_OK““
//									TcpWritedata( TCPLOGFRAME, "Device tcp connetced!") ;  //上传TCP数据，更新服务器硬件客户端列表
									AppLogPrintf("TCP连接成功") ;
/*获取时间移到第240行*/									
//									SAPBR_Init();
////								if(RUNOK == SYNC_NetTime())        		//同步网络时间
//								if(RUNOK == SYNC_LBS_Time())        		//基站定位LBS信息和时间	
//									RefreshOledTime() ;                 //OLED更新时间
/************END*************/								
//								if(StratUPFlag){StratUPFlag = 0;vTaskResume(ScanPortTaskHandler);}//如果重新开机 执行一次恢复端口扫描任务
//								vTaskResume(LedTaskHandler);//	恢复Led任务
//								Tled = 1;
//								xEventGroupSetBits(AlarmStateHandler, AlarmWaitBIT_0);		//开启获取报警事件
								}
							else                                		//TCP连接失败不影响主业务逻辑，所以TCP异常只做重新初始化，不做异常处理
								{
									CLEATCP ;                       //设置”标志位2：EC20 TCP状态清零
									AppLogPrintf("TCP连接失败") ;
//										WriteLogToFlash("TCP连接失败!") ;                
								}
//						}						
//					if( (GETMODULESTATE == STATE_OK) &&      
//						  (GETNETREGSTATE == STATE_OK) &&
//							(GETMQTTSTATE == STATE_ERR) )        		//EC20module、EC20_Net_Reg初始化都 完成，则进行 网络时间同步 及 MQTT连接 
//						{
							StatusBarPrintf("连接MQTT服务器:") ;     		//显示屏显示“连接MQTT服务器” 
							runResult = mqttiniti();           		//MQTT 初始化
							runResult = mqttsub_Push();							//MQTT注册主题
							if( RUNOK == runResult) 	           		// MQTT初始化正常
								{
									SETMQTTOK ;			                 		//设置”标志位3：EC20 MQTT请求状态 STATE_OK“““					
									StatusBarPrintf("MQTT连接成功") ; 
									AppLogPrintf("MQTT连接成功") ;

//									runResult = Reg_Post() ;         		//通过苏宁提供的对时接口对时注册
//									runResult = SYNC_NetTime() ;         		//同步网络时间

//									TcpWritedata( TCPLOGFRAME, "Device Register!") ;                          //TCP上报注册结果								
//									if( RUNOK != runResult)          		//Http 请求数据失败，严重错误，直接重启系统
//										{
////											WriteLogToFlash("Http初始化OK，注册请求失败，重启系统!") ;
//                      NVIC_SystemReset();//SystemSoftReset() ;          		//系统重启
//										}
//                  RefreshOledTime() ;                 //OLED更新时间																		
								}
							else                                	  //Http 初始化异常 直接重启EC20重新初始化
								{
									CLEAMQTT ;
									AppLogPrintf("MQTT初始化失败") ;
//									WriteLogToFlash("MQTT初始化失败!") ;					  
								}
						  }
															
								if(DeivceFlag.StratUPFlag)
									{
									memset(Save_433M_Value,0,Size_433M * sizeof(Receive_433M));
									LED1 = LED2 = LED3 = LED4 = LED5 = LED6 = LED7 = LED8 = 1;
									DeivceFlag.StratUPFlag = 0;vTaskResume(ScanPortTaskHandler);
									}//如果重新开机 执行一次恢复端口扫描任务
								vTaskResume(LedTaskHandler);//	恢复Led任务
								Tled = 1;
								xEventGroupSetBits(AlarmStateHandler, AlarmWaitBIT_0);		//开启获取报警事件
//						if( GETUSERNETSTATE )                    		//MODULEBIT_0\NETREGBIT_1\TCPBIT_2\MQTTBIT_3 标志位全部置1，说明网络全部正常
//						{
//							EC20_Query_CSQ(sEc20Param.csq) ;                             //获取CSQ的值
////							EC20_Query_Voltage(sEc20Param.ec20Voltage) ;                 //获取EC20电压值
//							DisplayStatusBar() ;                 		//状态栏显示信号质量和电压	
//							failedTimes = 0 ;                    		//网络维护失败计次清零
//						}
					RELESE_EC20();		                      		//释放EC20资源
          AppLogPrintf("释放EC20资源!") ;						
				}	
//				uu = xEventGroupGetBits(netEventHandler);
//			RELESE_EC20();		                      		//释放EC20资源	
			vTaskDelay(15*configTICK_RATE_HZ) ;              //最高网络维护频率：5S/次
    }
}

/*********************************************************************************************************************
* DATA灯闪烁任务函数
*********************************************************************************************************************/
void LedTask(void *pvParameters)
{
	u16 Datlen;
 static Rs485LinkStateType rs485_link_state ;
	static u8 ADC_converNum = 0;
	static u8 fail_count[10] = {0};
	static u8 timer3S = 0;
    while(1)
    {
			timer3S++;
/****RS485 处理函数****/			
//		if(Rs485Sending.Sending)//RS485发出指令后 容忍的等待时间开始计时标志 5S
//			{
			if(Rs485Sending.DelayTime >0)
				{
					Rs485Sending.DelayTime--;
//					OledShowChar(112, 0, HIGH_12,false,' ');//清除显示出错的从机地址
				}
			else 		//等待回复 时间溢出
				{
					Rs485Sending.Sending = 0;fail_count[RegisterEEpro_Slave.SlaveAddr_read_cousor-1]++;
					if(RegisterEEpro_Slave.Host_or_Slave == Host_Flag)//如果当下是主机模式
					{
						if(fail_count[RegisterEEpro_Slave.SlaveAddr_read_cousor-1] > 2)
						{	Rs485Sending.link_state = Host_linkNG; fail_count[RegisterEEpro_Slave.SlaveAddr_read_cousor-1] = 0;
							OledShowChar(112, 0, HIGH_12,false,Hex2Ascii(RegisterEEpro_Slave.RegisterArray[RegisterEEpro_Slave.SlaveAddr_read_cousor-1]));//显示出错的从机地址

							if(!(RS485_slaveAlarmFlag & (0x01 << RegisterEEpro_Slave.RegisterArray[RegisterEEpro_Slave.SlaveAddr_read_cousor-1]))//读取当前从地址 与 报警标识 相与
							&&(Rs485Sending.link_state == Host_working))//且在工作状态
							{						
								RS485_slaveAlarmFlag |= (0x01 << RegisterEEpro_Slave.RegisterArray[RegisterEEpro_Slave.SlaveAddr_read_cousor-1]);
								uint8_t rs485PhoneDatLen = GetRs485_Phone(MessgeAddr[2]);//获取第一回路的号码 如果主机向从机发出的指令得到回复错误数据 发短信Receive_Alarm.Num
								if(rs485PhoneDatLen > 3)//如果有获取到被叫号码 执行打电话短信
								{		
									memset(MsegTemp,0,Mseg_Lenth);
									sprintf(MsegTemp,RS485_SlaveFault,RegisterEEpro_Slave.RegisterArray[RegisterEEpro_Slave.SlaveAddr_read_cousor-1]);//RS485 003%1d号从机故障 “003%1d”打印一位数字符串
									Alarm_T();	//报警的短信信息后缀加上时间	
									xEventGroupSetBits(AlarmStateHandler,	SMS_ReadyBIT_1);											//SIM卡报警
								}
							}						
						}
//					if(Rs485Sending.link_state == Host_working)
						xEventGroupSetBits(AlarmStateHandler,	Rs485HostBIT_11);//开启发送RS485查询从机报警任务						
					}
					else if(RegisterEEpro_Slave.Host_or_Slave == Slave_Flag)												/*从机 的处理函数*/
					{
						Rs485Sending.link_state = Slave_linkNG;
					}
				}
//			}
//恢复链接 处理函数			
		if(rs485_link_state != Rs485Sending.link_state)
		{
			rs485_link_state = Rs485Sending.link_state;
			if(RegisterEEpro_Slave.Host_or_Slave == Host_Flag)//如果当下是主机模式
				{
					
					if((Rs485Sending.link_state == Slave_linkOK)&&(RS485_slaveAlarmFlag == 0))
					{OledShowChar(112, 0, HIGH_12,false,' ');//清除显示出错的从机地址
						OLED_ShowString(35, 0, false, "RS HOST", HIGH_16);
						fail_count[RegisterEEpro_Slave.SlaveAddr_read_cousor-1] = 0;
						RS485_slaveAlarmFlag &= ~(0x001 << RegisterEEpro_Slave.RegisterArray[RegisterEEpro_Slave.SlaveAddr_read_cousor-1]);}//清标识位
					else if(Rs485Sending.link_state == Host_alarm)	
					{OLED_ShowString(35, 0, false, "ALARM..", HIGH_16);//
						RS485_slaveAlarmFlag &= ~(0x001 << RegisterEEpro_Slave.RegisterArray[RegisterEEpro_Slave.SlaveAddr_read_cousor-1]);}//清标识位
					else if(Rs485Sending.link_state == Host_working)	OLED_ShowString(35, 0, false, "RS HOST", HIGH_16);				
				}
			else if(RegisterEEpro_Slave.Host_or_Slave == Slave_Flag)//如果当下是从机模式
			{
					if(Rs485Sending.link_state == Slave_linkOK)StatusBarPrintf("RS485连接成功");
					else if(Rs485Sending.link_state == Slave_linkNG)StatusBarPrintf("RS485连接失败") ;			
			}	
		}
			
/****END****/
		MYI2C_Handle(&SENx);
		RefreshOledTime() ;                 //OLED更新时间			
		if(RegisterEEpro_Slave.Host_or_Slave != Slave_Flag)DisplayCsq();		//六通道RS485从机		
		TemperHumidity();
			
#if 0
    /* Test on channel1 transfer complete flag */
    while(!DMA_GetFlagStatus(DMA1_FLAG_TC8,DMA1));
    /* Clear channel1 transfer complete flag */
    DMA_ClearFlag(DMA1_FLAG_TC8,DMA1);
    /* TIM1 counter disable */
//    TIM_Enable(TIM2, DISABLE);
#endif
static char wififlag = 2;
	if(DeivceFlag.Power_Flag)//停电开启电池电量监测
	{	
	ADC_convertedValue[ADC_converNum]=ADC_GetData(ADC3,ADC3_Channel_05_PB13);
	if(++ADC_converNum > 2)//获取中间值 
		{
			ADC_converNum=0;
			if(ADC_convertedValue[0] > ADC_convertedValue[1])Battery_Voltage = ADC_convertedValue[0];
			else Battery_Voltage = ADC_convertedValue[1];
			if(Battery_Voltage > ADC_convertedValue[2])Battery_Voltage = ADC_convertedValue[2];
			else Battery_Voltage = ADC_convertedValue[1];
//float aa = (float)(Battery_Voltage * 3.3f/4095)*2;

			u8 num;
				if(Battery_Voltage > 0x7a0)	
					num	= (Battery_Voltage - 0x7a0)/0x60;	
				else 
					{
					num = 0;
					EC20_CLOSE();
//					PWR_EnterStandbyState();//进入 STANDBY 模式
					PWR_LVoff = 1;//电池电压过低判断电池	
					}
//			if(num<4)PWR_LVoff = 1;//电源测试用	
			if(num > 5)	num = 5;		
			OLED_DrawBMP(120, 0, 8, 16, false, batBmp[num]);			
		}	
		wififlag = 2;
	}
	else  //因为WIFI 和 电池电量显示是在OLED同一个区域，故电池供电时不显示WIFI信号；
	{
		if(wififlag != wifi_onlineState)
		{
		wififlag = wifi_onlineState;OLED_DrawBMP(120, 0, 8, 16, false, WIFI_online[wifi_onlineState]);//0 WIFI连接成功  1 WIFI连接失败 2 Nodisplay
		}
	}
//	else PWR_LVoff = 0;//电源测试用
//			Data_Led_Reverse() ;	                                  //DATA LED灯翻转	
//Io_Reverse(PD_PORT, motor_up) ;				
//			Beep_Reverse() ;					//蜂鸣器翻转	
/*************************用户查询 SIM卡存储溢出*****************************/
			char *Sim_Data = portMalloc(500*sizeof(char)) ;	//100
			if(RW_OK == GetQueueMemData(&SIMQueue, Sim_Data, &Datlen))	
			{
				if(hand(Sim_Data, "+CMT:"))DeivceFlag.SIMorMQTT = 0;//接收的数据是sim    0 sim 1 mqtt
				else DeivceFlag.SIMorMQTT = 1;
//+MSUB: "/864708068765984D/download/QUERY",1 byte,1				
				if((DeivceFlag.SIMorMQTT)&&(hand(Sim_Data,"/download/QUERY")))
					{MQTT_DeviceState(1);}		  //服务器请求获取设备状态数据 设备发布Topic: /upload/home
				else if((!DeivceFlag.SIMorMQTT)&&((hand(Sim_Data,CDMACheck))||(hand(Sim_Data,MobileCheck))))//用户查询指令
				{																						//指令比较正确 触发查询当前输入口状态
					InputState();
					u16 MessLen = strlen(MsegTemp);	
					response(Sim_Data,MessLen+2,MsegTemp);//
				}
//转发 10001 10086 10011 充值余额提醒信息
//				else Arrearage(Sim_Data);	
			}
			else if((timer3S%60) == 0)MQTT_cqs_temper();//60S 发布一次温度湿度消息
			MQTT_DeviceState(0);
			AlarmTemHum_Trigger();//温度湿度报警检测				
			portFree(Sim_Data) ;	                                                   //释放内存空间		
			vTaskDelay(1*configTICK_RATE_HZ) ;       //1S钟运行一次
    }
}

/*********************************************************************************************************************
* 输入端口扫描任务函数
*********************************************************************************************************************/
//uint16_t CopsDelay10S;			  //60秒计数  查询当前网络模式 防止掉线 触发自动重启
void ScanPortTask(void *pvParameters)//100ms 10hz
{
//static u8	UserCheck_Delay;
//	u16 Datlen;
    while(1)
    {
			Watchdog_Feed() ;
			Wireless_Handle();			//无线处理 10ms调用			
			if(Beep>0)	
				{Beep--;AlrmBuzzer = ~AlrmBuzzer;}
			else {AlrmBuzzer = 0;}
			PortSignal();					// 输入信号扫描
			PwoerCheck();//设置参数 停电报警
			ScanQuery();//查询历史报警记录 按健

/*RS485处理收到注册信息的函数*/			
			if(Rs485Sending.reg_infro == SET )
			{
			Rs485Sending.reg_infro = RESET ;
			if(RegisterEEpro_Slave.Host_or_Slave == Host_Flag)//如果当下是主机模式
				{
					u8 Temp = (u8)Rs485Sending.link_state;
//					if(Rs485Sending.link_state == Reging_Succse)
//					{
//					SectorErase_protect(CSCA_W433_Address, &RegisterEEpro_Slave.Host_or_Slave, RS485_AddrOffset, sizeof(RegisterEEpro_Slave),0);
//					}
					Rs485_SendBuffer.Fromat.Func_num  = Host_ReturnReg;
					RS485SendData(Rs485_ReceiveBuffer.Fromat.Source_Addr,HostAddr,Host_ReturnReg,1,&Temp);//向从机回复注册结果
				}
			else																					/*如果当下是从机 的处理函数*/
				{
					if(Rs485Sending.link_state == Reging_Succse)
						StatusBarPrintf("RS485注册成功");
					else if(Rs485Sending.link_state == Reging_Fail)
						StatusBarPrintf("RS485注册失败") ;
					else if(Rs485Sending.link_state == Slave_linkNG)
						StatusBarPrintf("RS485连接失败") ; 
				}
			}
/**END**/
			
			vTaskDelay(0.2*configTICK_RATE_HZ) ;       //0.2钟运行一次
    }
}

/*********************************************************************************************************************
 SetModesTask 设置模式 设置信息 被叫号码
*********************************************************************************************************************/
void SetModesTask(void *pvParameters)
{
	SetResultType Result; 
	u16 Datlen;
	for(;;)
	{
//		Check_Code();//433对码 100ms调用
		char *SimData = portMalloc(100*sizeof(char)) ;	
//	if(strlen((const char*)ec20AtBuf) > 1)//接收数据长度大于1
	if(RW_OK == GetQueueMemData(&SIMQueue, SimData, &Datlen))	
		{
		if(hand(SimData, "+CMT:"))DeivceFlag.SIMorMQTT = 0;//接收的数据是sim    0 sim 1 mqtt
			else DeivceFlag.SIMorMQTT = 1;
		main_call_code(SimData); 
		Result = NumberSet(SimData);//设置短信接收号码
		if(SetNull == Result){Result = MessageSet(SimData);}//设置短信内容
		if(SetNull == Result){Result = EraseSector(SimData);}//擦除一个扇区
		if(SetNull == Result){Result = SET_Alarm_Mode(SimData);}//短信&公众号 设置 报警方式  
		if(SetNull == Result){Result = SET_AlarmValue_TemperHum(SimData);}//短信&公众号 设置温度湿度报警阀值
		if(SetNull == Result){Result = WorkingMod(SimData);}//设置当前主机是 独立式 还是RS485主机 或 RS485从机
/*wifi*/
		if(SetNull == Result){Result = WIFI_linkNet(SimData);}//设置wifi ssid password
		if(DeivceFlag.SIMorMQTT)//数据是mqtt  1 
			{
			if(SetNull == Result)Mqtt_check(SimData);//检查服务器发布的数据指令 做相应的动作
			}
		else//数据是 sim  0
			{
			if(SetNull == Result){Result = CleanAll(SimData);}//清除模块里的全部信息
			if(SetNull == Result){Result = QueryPortInfo(SimData);}//查询端口短信内容			
			if(SetNull == Result){Result = RemoteCtr_Enable(SimData);}//MQTT 远程 撤布防控制 开启 或 关闭
			if(SetNull == Result)
				{
#ifdef W433_Enabel
				if(hand(SimData,W433_Inching))Set_433Model(SimData,0x00);//:点动模式 0x00
				else if(hand(SimData,W433_SelfHold))Set_433Model(SimData,0x11);//:自锁模式 0x11
				else if(hand(SimData,W433_InterLock))Set_433Model(SimData,0x22);//:互锁模式 0x22
				else if(hand(SimData,W433_OFF))Set_433Model(SimData,0x33);//:遥控关闭 0x33
//#elif (W433_Enabel == 2)
//				Set_433Model(SimData,0x00);//:点动模式 0x00
//				if(hand(SimData,W433_OFF))Set_433Model(SimData,0x33);//:遥控关闭 0x33					
#endif			
				if((hand(SimData,CSCA_N))||(hand(SimData,CSCA_N_uni)))User_SetCSCA(CSCA_W433_Address,0);//设置短信中心号码 			
				}			
			}
		Tled = 1;	
		}
		
		if(wifi_linkFlag)//配网成功标识
		{wifi_linkFlag = 0;
		OLED_DrawBMP(120, 0, 8, 16, false, WIFI_online[wifi_onlineState]);//0 WIFI连接成功
		WIFI_SendModuleCmd(AT_WAUTOCONN,NULL,1);//AT+WAUTOCONN=1放在AT+WJAP后面
		WIFI_SendModuleCmd(AT_WCONFIG,NULL,0);//关闭配网		
		}

	portFree(SimData) ;	                                                   //释放内存空间		
	vTaskDelay(1*configTICK_RATE_HZ);
	}
}	

/*********************************************************************************************************************
AlarmTask 处理短信电话报警任务事件标志组类型
从报警队列缓存获取一组事件
再从该事件读取预置好的短信接收序号 电话接听序号（手机号码存放的下标）
读取的结果置1相应的事件标志数组
使能9BIT短信发送
*********************************************************************************************************************/
void AlarmTask(void *pvParameters)
{
//  u16 len = 0 ;
//	EventBits_t EventValue;
//	EventBits_t AlarmPhoneIndex;//报警对象号码存储的地址下标
//u8 AlarmTask_straup = 0;//再次轮询任务	
//	EventValue = EventValue;
	while(1)
	{
		xEventGroupWaitBits( AlarmStateHandler,
                                 (AlarmWaitBIT_0), //等待事件标志组 空闲标志位 AlarmBIT_16
                                 pdTRUE,//退出函数清除bit pdFALSE
                                 pdTRUE,//等待的位全部置1进入 pdFALSE 
                                 (TickType_t	)portMAX_DELAY );

//	if((Rs485Sending.remote_alarm)&&(RegisterEEpro_Slave.Host_or_Slave == Host_Flag))//六通道RS485主机
//	{
//	xEventGroupSetBits(AlarmStateHandler,	Rs485HostBIT_11);	//处理从机发出的无报警任务指令 从机没报警任务 开启主机RS485轮询下一个从机
//	goto AlarmTask_OUT;
//	}	
	Rs485Sending.remote_alarm = 0;
	xEventGroupClearBits(AlarmStateHandler,(EventBits_t)0);		//复位标识组
//	if( RW_OK == GetQueueMemData(&SignalQueue, buf, &len))	 //从sRS232Queue帧缓冲区中获取一帧数据准备处理
	if( pdTRUE == xQueueReceive(AlarmPort_Queue,&Receive_Alarm,0))	
		{	AlarmBitFlag &= ~(0x01 << Send_Alarm.Num);
//量产程序以下二句需要 判断是否有设置报警信息 否则不执行
	HistorySave();
	HistoryRead(LINE3,0);		//第三行 上次报警时间
			
W433Hard.Head.DeviceAlarmMode=mqtt_only;//测试测试
			
			if(Alarm_Disabled == W433Hard.Head.DeviceAlarmMode)goto OUT_AlarmTask;//撤防 禁止报警 仅上传基础的状态数据
//				AlarmTask_straup = 1;//{xEventGroupSetBits(AlarmStateHandler,	AlarmWaitBIT_0);}	//置1报警空闲标识 继续获取下一个队列事件
//			else if(mqtt_only == W433Hard.Head.DeviceAlarmMode)//仅上传到MQTT服务器进行触发报警
//				{
//					Mqtt_MpubAlarm();//上传到MQTT服务器
//					xEventGroupSetBits(AlarmStateHandler,	AlarmWaitBIT_0);Tled = 1;//置1报警空闲标识 继续获取下一个队列事件
//				}
			else			//全功能报警 SIM卡和上传到MQTT服务器报警
				{
//				if(simANDmqtt == W433Hard.Head.DeviceAlarmMode)Mqtt_MpubAlarm();//如果是全功能 上传到MQTT服务器报警信号
				u8 rs485PhoneDatLen = 0;	
				if(SUCCESS == GetAlarmMassage(MessgeAddr[Receive_Alarm.Num],Receive_Alarm.Adrr_Offset))//获取报警信息内容
					{
					rs485PhoneDatLen = GetRs485_Phone(MessgeAddr[Receive_Alarm.Num]);
//					xEventGroupSetBits(AlarmStateHandler,	AlarmWaitBIT_0);										//没有信息  置1报警空闲标识 继续获取下一个队列事件
					}
				if(rs485PhoneDatLen > 3)//如果有获取到被叫号码 执行打电话短信
					{
						Alarm_T();	//报警的短信信息后缀加上时间	
//						xEventGroupSetBits(AlarmStateHandler,	SMS_ReadyBIT_1);											//SIM卡报警
//					 if(simANDmqtt == W433Hard.Head.DeviceAlarmMode)Mqtt_MpubAlarm();							//全功能 上传到MQTT服务器报警信号
						Rs485_SendBuffer.Fromat.Func_num  = Host_IdleLink;Rs485Sending.link_state = Host_alarm;
					if(simANDmqtt == W433Hard.Head.DeviceAlarmMode){Mqtt_MpubAlarm();xEventGroupSetBits(AlarmStateHandler,	SMS_ReadyBIT_1);}											//全功能模式 SIM卡报警+上传到MQTT服务器报警信号
					else if(Sim_only == W433Hard.Head.DeviceAlarmMode){xEventGroupSetBits(AlarmStateHandler,	SMS_ReadyBIT_1);}	//SIM卡报警
					else if(mqtt_only == W433Hard.Head.DeviceAlarmMode){Mqtt_MpubAlarm();}	//MQTT模式 上传到MQTT服务器报警信号xEventGroupSetBits(AlarmStateHandler,	AlarmWaitBIT_0);
					else {Rs485Sending.link_state = Host_working; goto OUT_AlarmTask;}//
					}
					else {Rs485Sending.link_state = Host_working; goto OUT_AlarmTask;}
//					{
//					AlarmTask_straup = 1;//xEventGroupSetBits(AlarmStateHandler,	AlarmWaitBIT_0);	//没有信息  置1报警空闲标识 继续获取下一个队列事件					
//					}
//				if(Receive_Alarm.Num == 16){EventFunc(P_OFF);TTSVoice = P_OFF;}//设备已停电!
//				else if(Receive_Alarm.Num == 17){EventFunc(P_ON); TTSVoice = P_ON;}//设备恢复供电				
				}				
		}
		else 
		{
			AlarmBitFlag = 0;Rs485_SendBuffer.Fromat.Func_num  = Host_GetSlaveAlarmCmd;
OUT_AlarmTask:

//	if(RegisterEEpro_Slave.Host_or_Slave == Eight_chnnal_Flag)
	if(RegisterEEpro_Slave.Host_or_Slave != Host_Flag)	
 xEventGroupSetBits(AlarmStateHandler,	AlarmWaitBIT_0);//置1报警空闲标识 继续获取下一个队列事件	
		}
		Tled = 1;
		if(RegisterEEpro_Slave.Host_or_Slave == Host_Flag)//开启发送RS485查询从机报警任务
			{xEventGroupSetBits(AlarmStateHandler, Rs485HostBIT_11);Tled = 1;}
				
vTaskDelay(1*configTICK_RATE_HZ) ;                       //：1S/次
	}
}
/*
等待标志组指定的事件位9BIT 开启轮询有效位
来自于报警任务
发送完短信，清零当前位，阻塞1秒，重启任务，所有号码发送完后，置19BIT拨打电话
*/
void MessageSendTask(void *pvParameters)
{
//  u16 len = 0 ;
	EventBits_t EventValue;
//	EventBits_t AlarmPhoneIndex;//报警对象号码存储的地址下标
	for(;;)
	{
		//等待事件标志组 AlarmBIT_7
		EventValue = xEventGroupWaitBits( AlarmStateHandler,
                                 SMS_ReadyBIT_1,
                                 pdFALSE,//退出函数的时候不清除bit 
                                 pdTRUE,//设置的都置1才进入
                                 (TickType_t	)portMAX_DELAY );
		
		OCCUPY_EC20(30*configTICK_RATE_HZ);  //等待占用EC20资源  30s	
/*
获取位2短信重发，位3短信发送失败；都置0 开始读取新的被叫号，否则重发一次短信
如获取号码失败，置1位4短信发送完成；置1位5拨打电话就绪；清零位1 短信发送就绪位
*/		
	if((SMS_FailBIT_3 & EventValue) == SMS_FailBIT_3)	//位3短信发送失败xEventGroupGetBits(AlarmStateHandler)
		{
		xEventGroupSetBits(AlarmStateHandler,	SMS_ReSendBIT_2);	//位2短信重发 置1
		EventFunc(MsegTemp);						
		}
	else
		{
//		if(SUCCESS == GetPhoneNumber(MessgeAddr[Receive_Alarm.Num],(u16*)SMS_Number_Offset))
		if(SUCCESS == AllGetPhoneNumber(MsgNumberFlag))	
			{
				cdma_mobile_txt_shift(PhoneNumber,PhoneNumLen);//被叫号码转换成UNICODE码
				EventFunc(MsegTemp);
			}
		else
			{
				xEventGroupClearBits(AlarmStateHandler,SMS_ReadyBIT_1);	//	关闭发短信任务
				xEventGroupSetBits(AlarmStateHandler,	Call_ReadyBIT_5);		//开启打电话任务
				TTSVoice = MsegTemp;
			}	
		}
RELESE_EC20();//释放EC20资源
vTaskDelay(3*configTICK_RATE_HZ) ;                       //3S
	}
}
/*
等待指定的事件位19BIT置1，
检查18BIT 置1重拨 0读取新的号码。
开始轮询有效标志位，ADT 电话号码 清零事件位19BIT
恢复任务CallResultTask()接听结果 
*/
void PhoneCallTask(void *pvParameters)
{
//  u16 len = 0 ;
	EventBits_t EventValue;
	while(1)
	{
				//等待事件标志组 AlarmBIT_15
		EventValue = xEventGroupWaitBits( AlarmStateHandler,
                                 Call_ReadyBIT_5,
                                 pdTRUE,//退出函数的时候清除bit pdFALSE
                                 pdTRUE,//设置的都置1才进入
                                 (TickType_t	)portMAX_DELAY );
		
		OCCUPY_EC20(30*configTICK_RATE_HZ);  //等待占用EC20资源  30s	
		if((Call_FailBIT_7 & EventValue) == Call_FailBIT_7)  //1 上次拨打电话失败  0 成功 xEventGroupGetBits(AlarmStateHandler))
		{
			xEventGroupSetBits(AlarmStateHandler,	Call_RepBIT_6 | Call_UnderWayBIT_8);	//电话重拨,置1 打电话进行中
			EC20_SendModuleCmd(AT_ATD, NULL,PhoneNumber);	
			Call_Delay = 50;// 电话接通最大时间 26S"
		}
	else							//获取新号码
		{
//		if(SUCCESS == GetPhoneNumber(MessgeAddr[Receive_Alarm.Num],(u16*)Call_Number_Offset))
		if(SUCCESS == AllGetPhoneNumber(PhoneNumberFlag))		
			{
			EC20_SendModuleCmd(AT_ATD, NULL,PhoneNumber);
			Call_Delay = 50;//	电话接通最大时间
			xEventGroupSetBits(AlarmStateHandler,	Call_UnderWayBIT_8);//	置1 打电话进行中			
			}
		else				//电话号码获取失败
			{
//			xEventGroupClearBits(AlarmStateHandler,Call_ReadyBIT_5);//清除 电话空闲位
			xEventGroupSetBits(AlarmStateHandler,	AlarmWaitBIT_0);		//开启下一个报警事件
			Rs485Sending.link_state = Host_working;
			}	
		}
	
RELESE_EC20();//释放EC20资源			
		vTaskDelay(15*configTICK_RATE_HZ) ;                       //拔号时间	  15秒左右/次
	}
}
/*
等待模组回复电话连接回执消息 阻塞0.5S 自行挂起
恢复任务 来自：发送短信任务PhoneCallTask
处理拨打电话后回执消息 忙线 挂机 接听中播放TTS 等待任务通知 来自发送短信任务 打电话任务
*/
void CallResultTask (void *pvParameters)
{

	EventBits_t EventValue,EvenStatus;//
//	static u32 CTTS_Delay;
	EventValue = EventValue;
	while(1)
	{
		//等待事件标志组 
		EventValue = xEventGroupWaitBits( AlarmStateHandler,
																		Call_UnderWayBIT_8,// SMS_UnderWayBIT_4 | 
																		pdFALSE,//退出函数的时候不清除bit 
																		pdFALSE,//设置的只要有一个为1进入
																	 (TickType_t	)portMAX_DELAY );
		
		OCCUPY_EC20(30*configTICK_RATE_HZ);  //等待占用EC20资源  30s	

		EvenStatus = xEventGroupGetBits(AlarmStateHandler);		
		if(EvenStatus & Call_UnderWayBIT_8)//1正在打电话
		{
			if(Call_Delay > 0)Call_Delay--;
//			else goto CallPeak;

//			if(strlen((const char*)ec20AtBuf) > 1)//接收数据长度大于1
//			{

			if((hand(ec20AtBuf,"NO CARRIER"))||(hand(ec20AtBuf,"NO DIALTION"))||(hand(ec20AtBuf,"NO ANSWER"))||(hand(ec20AtBuf,"ERROR"))\
				||(hand(ec20AtBuf,"BUSY"))||(!Call_Delay))									//or 电话接听超时 
				{
//CallPeak:	
					EC20_SendModuleCmd(AT_ATH, NULL);
					if((EvenStatus & Call_RepBIT_6) == Call_RepBIT_6)  //是否 电话重拨
					{
//						xEventGroupClearBits(AlarmStateHandler,Call_ReadyBIT_5);	//电话开拨 置1
						PhoneAddrIndex++;//开始向下一个号码电话开拨
						xEventGroupClearBits(AlarmStateHandler, Call_RepBIT_6 | Call_FailBIT_7);//清0 电话重拨 电话拨打失败
					}
					else xEventGroupSetBits(AlarmStateHandler, Call_FailBIT_7);//拨打失败置1
					
					xEventGroupClearBits(AlarmStateHandler, Call_UnderWayBIT_8);//清0 正在打电话
					xEventGroupSetBits(AlarmStateHandler, Call_ReadyBIT_5);	//电话开拨 置1	
				}	

//			else if(hand(ec20AtBuf,"END"))//对方已挂断电话 
//				{
//					EC20_SendModuleCmd(AT_ATH, NULL);
//					xEventGroupClearBits(AlarmStateHandler, Call_RepBIT_6 | Call_FailBIT_7|Call_UnderWayBIT_8);//清0 电话重拨 电话拨打失败 //清0 正在打电话
//					PhoneAddrIndex++;//拔打下一个电话
//					xEventGroupSetBits(AlarmStateHandler, Call_ReadyBIT_5);	//电话开拨 置1
//				}

			else if(hand(ec20AtBuf,"CONNECT"))//对方已接听电话状态
				{
					Delay_Ms_StopScheduler(1500);
					EC20_SendModuleCmd(AT_CTTS, NULL,TTSVoice);
//					CTTS_Delay = 100;
//					xEventGroupSetBits(AlarmStateHandler, CTTS_ingBIT_9);	//TTS进行中 置1
					EC20_SendModuleCmd(AT_ATH, NULL);
					xEventGroupClearBits(AlarmStateHandler, Call_RepBIT_6 |Call_FailBIT_7|Call_UnderWayBIT_8|CTTS_ingBIT_9);//清0 电话重拨 电话拨打失败 //清0 正在打电话
					PhoneAddrIndex++;//拔打下一个电话
					xEventGroupSetBits(AlarmStateHandler, Call_ReadyBIT_5);//电话开拨 置1
					
/*客户个性化:当有接听电话时 后面号码不再拨打电话 开启下一个警情事件*/
#ifdef only_oneCall 
					xEventGroupClearBits(AlarmStateHandler, Call_ReadyBIT_5);//电话清 0
					xEventGroupSetBits(AlarmStateHandler,	AlarmWaitBIT_0);		//开启下一个报警事件
#endif
				}
//			else if(hand(ec20AtBuf,"+CTTS:0"))//播放TTS语音结束
//				{
//					EC20_SendModuleCmd(AT_ATH, NULL);
//					xEventGroupClearBits(AlarmStateHandler, Call_RepBIT_6 |Call_FailBIT_7|Call_UnderWayBIT_8|CTTS_ingBIT_9);
//					xEventGroupSetBits(AlarmStateHandler, Call_ReadyBIT_5);//电话开拨 置1
//				}
//			if(EvenStatus & Call_UnderWayBIT_8)//TTS进行中 倒计时
//				{
//				if(CTTS_Delay > 0)CTTS_Delay--;
//					else
//					{
//					xEventGroupClearBits(AlarmStateHandler, Call_RepBIT_6 |Call_FailBIT_7|Call_UnderWayBIT_8|CTTS_ingBIT_9);
//					xEventGroupSetBits(AlarmStateHandler, Call_ReadyBIT_5);//电话开拨 置1
//					}
//				}	
//			}
		}
		RELESE_EC20();//释放EC20资源
		vTaskDelay(0.5*configTICK_RATE_HZ) ;                       //处理RS232数据：0.5S/次
	}
}
/*
主机获取从机 是否有报警任务
轮询一遍后 如果本机有报警任务将挂起此任务 
如果本机没有任务 再获取从机报警任务 依次循环...
*/
void Host_Rs485GetAlarm(void *pvParameters)
{
	EventBits_t EventValue;
	EventValue = EventValue;
 u8 datTemp = 0;
	while(1)
	{
		EventValue = xEventGroupWaitBits( AlarmStateHandler,
                                 (Rs485HostBIT_11), //等待事件标志组 空闲标志位 AlarmBIT_16
                                 pdTRUE,//退出函数清除bit pdFALSE
                                 pdTRUE,//等待的位全部置1进入 pdFALSE 
                                 (TickType_t	)portMAX_DELAY );
		
//		vTaskSuspend(AlarmTaskHandler);//挂起报警任务SetModesTask
//		Rs485Sending.link_state = Host_working;
		if(RegisterEEpro_Slave.SlaveAddr_read_cousor < RegisterEEpro_Slave.SlaveAddr_write_cousor)//向已经注册成功的从机地址 发送获取报警指令
		{
		RS485SendData(RegisterEEpro_Slave.RegisterArray[RegisterEEpro_Slave.SlaveAddr_read_cousor],HostAddr,Host_GetSlaveAlarmCmd,0,&datTemp);//获取从机的报警任务 01 02 00 crc	
		RegisterEEpro_Slave.SlaveAddr_read_cousor++;
		Rs485Sending.remote_alarm = 1;			
			if(Rs485Sending.link_state == Host_alarm)
			xEventGroupSetBits(AlarmStateHandler, Rs485HostBIT_11);	//开启主机获取从机报警任务
		}	
		else 
		{
		RegisterEEpro_Slave.SlaveAddr_read_cousor = Rs485Sending.remote_alarm = 0;
//		xEventGroupClearBits(AlarmStateHandler, Rs485HostBIT_11);//关闭本任务 Host_Rs485GetAlarm	
			if(Rs485Sending.link_state == Host_alarm)
			xEventGroupSetBits(AlarmStateHandler, Rs485HostBIT_11);	//开启主机获取从机报警任务
			else xEventGroupSetBits(AlarmStateHandler, AlarmWaitBIT_0);	//开启主报警任务SetModesTask 置1	
//		vTaskSuspend(Host_Rs485TaskHandler);//挂起Host_Rs485GetAlarm	
//		vTaskResume(AlarmTaskHandler);//	恢复报警任务SetModesTask	
		}	

		vTaskDelay(0.5*configTICK_RATE_HZ) ;                       //  1秒左右/次
	}
}

/*
从机获得主机指令后 查询本机是否有报警任务，并发给主机
*/
void Execute_SlaveAlarm(void *pvParameters)
{
//	EventBits_t EventValue;//,EvenStatus
//	EventValue = EventValue;
// static Rs485LinkStateType rs485_link_state ;
	while(1)
	{
		xEventGroupWaitBits( AlarmStateHandler,
                                 (Rs485SlaveAlarmBIT_10), //等待事件标志组 空闲标志位 AlarmBIT_16
                                 pdTRUE,//退出函数清除bit pdFALSE
                                 pdTRUE,//等待的位全部置1进入 pdFALSE 
                                 (TickType_t	)portMAX_DELAY );
		
//		xEventGroupClearBits(AlarmStateHandler,(EventBits_t)0);		//复位标识组
//LED7=1;

//		if(rs485_link_state != Rs485Sending.link_state)//如果当下是从机模式  /*此处会导致I2C死机*/
//		{
//			rs485_link_state = Rs485Sending.link_state;
//				if(Rs485Sending.link_state == Slave_linkOK)
//StatusBarPrintf("RS485连接成功");
//				else if(Rs485Sending.link_state == Slave_linkNG)StatusBarPrintf("RS485连接失败") ;	
//		}		
			
		if( pdTRUE == xQueueReceive(AlarmPort_Queue,&Receive_Alarm,0))	 //从sRS232Queue帧缓冲区中获取一帧数据准备处理
		{	AlarmBitFlag &= ~(0x01 << Send_Alarm.Num);
//量产程序以下二句需要 判断是否有设置报警信息 否则不执行
		HistorySave();
		HistoryRead(LINE3,0);		//第三行 上次报警时间
		
		u8 RS485PhoneLen = 0;		
		if(SUCCESS == GetAlarmMassage(MessgeAddr[Receive_Alarm.Num],Receive_Alarm.Adrr_Offset))//获取报警信息内容
			{
				RS485PhoneLen = GetRs485_Phone(MessgeAddr[Receive_Alarm.Num]);
			}
		
		if(RS485PhoneLen > 3)//如果有获取到被叫号码 执行打电话短信
			{Rs485_SendBuffer.Fromat.Func_num  = slave_phoneDat;
				RS485SendData(HostAddr,RegisterEEpro_Slave.LocalAddr, slave_phoneDat,RS485PhoneLen,(u8*)Msg_and_call_Phone);//从机向主机发送电话号码
				Alarm_T();	//报警的短信信息后缀加上时间
				u8 tempLen = strlen(MsegTemp);
				
			u32 delaycn = 0XAFFFF;			
//			while(( Rs485Sending.link_state == Send_ing )&&(delaycn--))__NOP();//DMA发送完成
	while((DMA_GetFlagStatus( DMA1_FLAG_TC7,DMA1 ) == RESET )&&(delaycn--))__NOP();//DMA发送完成
			if(!delaycn)Rs485Sending.link_state = Slave_linkNG;//StatusBarPrintf("RS485连接失败") ;
				
				Delay_Ms_StopScheduler(10) ;	Rs485_SendBuffer.Fromat.Func_num  = slave_SMSDat;
				RS485SendData(HostAddr,RegisterEEpro_Slave.LocalAddr, slave_SMSDat,tempLen,(u8*)MsegTemp);//通过RS485向主机发送报警内容
				Rs485Sending.Sending = 0;
			}
			else 
			{
				xEventGroupSetBits(AlarmStateHandler,	Rs485SlaveAlarmBIT_10);//没有信息  置1报警空闲标识 继续获取下一个队列事件
			}
		}
		else //没有报警队列 向主机发送结束标志
		{Rs485_SendBuffer.Fromat.Func_num  = Salve_stateEND;
			RS485SendData(HostAddr,RegisterEEpro_Slave.LocalAddr, Salve_stateEND,0,0);//通过RS485向主机发送报警内容	
			AlarmBitFlag = 0;Tled = 1;
		}			
		vTaskDelay(0.5*configTICK_RATE_HZ) ;                       //：0.5S/次
	}	
}
/*********************************************************************************************************************
MQTT 发布消息
*********************************************************************************************************************/
void MQTT_Publish(void *pvParameters)
{
   for(;;)
	 {
//		SETTCPUPTASKOK ;		                                             //TCPUP任务已正常运行 
		if( GETTCPSTATE == STATE_OK )                                    //TCP网络正常
	   {
				char *buf = portMalloc(512) ;
			  uint16_t len = 0 ;
				if( RW_OK == GetQueueMemData(&MQTT_PublishQueue, buf, &len))        //从sTcp0Queue帧缓冲区中获取一帧数据准备处理
					{
						OCCUPY_EC20(5*configTICK_RATE_HZ);
						RunResult status = EC20_SendHttpCmd(MPUB, NULL,MQTT_TOPIC[*buf],0,(buf+1));//发布消息	
//						RunResult status = Tcp_SendData(&sChannal0, (uint8_t*)buf, len) ;
//						RunResult status = Mqtt_SendData(&sChannal0, (uint8_t*)buf, len) ;
						if( RUNOK != status)                                     //TCP第一次发送数据失败
							{
							    
										CLEATCP ;                                      //SETTCPERR TCP网络状态异常
//										ErrorLogPrintf("MQTT_Publish Err!");
								    WriteLogToFlash("MQTT_Publish Err!");           //本地FLASH记录	
								  				
							} 
						RELESE_EC20() ;                                          //释放EC20资源
					}							
				portFree(buf) ;   					
			}
    vTaskDelay(0.5*configTICK_RATE_HZ);	                                                 //50/configTICK_RATE_HZ=0.5S运行一次			 
	 }
}
/*********************************************************************************************************************
WIFI 在线检测 掉线报警触发第8回路
#ifdef wifi_alarmNumber_8	 wifi第8回路  
*********************************************************************************************************************/
void WIFI_onlinDet(void *pvParameters)
{
	static u8 Error_count;
   for(;;)
	 {
		 RunResult result = WIFI_SendModuleCmd(AT_PINT,NULL);
		 if(result == RUNOK ){Error_count = 0; wifi_onlineState = 0; wifi_Alarm = 1;}
		 else 
		 {
		 if(Error_count < 10)Error_count++;
		 if(Error_count > 5){wifi_onlineState = 2; }              //错误大于5次关闭WIFI vTaskDelete(WIFI_onlinDetTaskHandler) ;并删除任务
		 else if(Error_count > 1){wifi_onlineState = 1;if(wifi_Alarm == 1)wifi_Alarm = 2;}//大于1次触发掉线报警第8回路
		 if(result == RUNERR ){WIFI_SendModuleCmd(AT_RST,NULL);Error_count = 0; }	
		 } 
//			char *buf = portMalloc(256) ;
//			uint16_t len = 0 ;
//			if( RW_OK == GetQueueMemData(&WIFI_Queue, buf, &len))        //从sTcp0Queue帧缓冲区中获取一帧数据准备处理
//				{
//				if((kmp(buf, "AT+PING:")>=0 )&&((kmp(buf, "+ERROR")>=0 )))
//					{
//						if(++Error_count > 2){Error_count = 0; ErrorLogPrintf("wifi disline"); }
//						else UARTx_SendData(WIFI_UART, "AT+RST\r\n", strlen("AT+RST\r\n"))	 ;
//					}
//					
//				else if((kmp(buf, "+PING:")>=0 )&&((kmp(buf, "OK")>=0 )))Error_count = 0;
////					else if(kmp(WIFI_Queue, "+WRSSI:")>=0 );
//				}
//				else
//				{
//				UARTx_SendData(WIFI_UART, "AT+WRSSI\r\n", strlen("AT+WRSSI\r\n"))	 ;
//				UARTx_SendData(WIFI_UART, "AT+PING=\"8.8.8.8\"\r\n", strlen("AT+PING=\"8.8.8.8\"\r\n"))	 ;	 //					
//				}
//				
//			portFree(buf) ;   					
DebugLogPrintf("WIFI_onlinDet %s",  RunResultToString(result) ) ; //命令执行结果
    vTaskDelay(10*configTICK_RATE_HZ);	                                                 //50/configTICK_RATE_HZ=0.5S运行一次			 
	 }
}
/*
		RS485数据处理任务函数
*/
//void RS485_CMD(void *pvParameters)
//{
//	EventBits_t EventValue;//,EvenStatus
//	EventValue = EventValue;   
//	unsigned int  tmp_crc;
//   unsigned int  rec_crc;
//u8 temp;
//int result;	
//	while(1)
//	{
//		EventValue = xEventGroupWaitBits( AlarmStateHandler,
//														 (Rs485ReceiveBIT_12), //等待RS485接收数据就绪
//														 pdTRUE,//退出函数清除bit pdFALSE
//														 pdTRUE,//等待的位全部置1进入 pdFALSE 
//														 (TickType_t	)portMAX_DELAY );
//		
//  if(Rs485_ReceiveBuffer.Fromat.Target_Addr == 0)goto reEEROR;// || (recvLen < 4)
//	if((Rs485_TranSmit.Fromat.Func_num != slave_RegDat)&&(Rs485_TranSmit.Fromat.Target_Addr != RegisterEEpro_Slave.LocalAddr))goto reEEROR;
//	
//	rec_crc = (Rs485_TranSmit.Fromat.DAT[Rs485_TranSmit.Fromat.Dat_len]<<8) + Rs485_TranSmit.Fromat.DAT[Rs485_TranSmit.Fromat.Dat_len + 1];
//	tmp_crc = CRC16_CalcBufCrc((u8*)Rs485_TranSmit.RS485_Dat ,Rs485_TranSmit.Fromat.Dat_len + 4);
//	
//	if(rec_crc != tmp_crc)
//		{
//		goto reEEROR;
//		}
//	//主机  从机屏蔽指令	
//	if(RegisterEEpro_Slave.Host_or_Slave == Host_Flag) Rs485_TranSmit.Fromat.Func_num &= 0x0f;//如果当下是主机模式 命令屏蔽高4位
//	else if(RegisterEEpro_Slave.Host_or_Slave == Slave_Flag)Rs485_TranSmit.Fromat.Func_num &= 0xf0;//如果当下是从机模式 命令屏蔽低4位
//	else Rs485_TranSmit.Fromat.Func_num = 0x00;	
//		
//	switch(Rs485_TranSmit.Fromat.Func_num)
//	{
///*主机处理从机的数据*/		
//		case slave_phoneDat:	//memset (Msg_and_call_Phone,0,Rs485ArrayLen);											//处理从机发出的电话号码数据
//												memcpy (Msg_and_call_Phone,Rs485_TranSmit.Fromat.DAT, Rs485_TranSmit.Fromat.Dat_len);
////												Rs485Sending.Sending = 0;
////												RS485_memcpy(0, Rs485_TranSmit.Fromat.DAT, Rs485_TranSmit.Fromat.Dat_len);break ;
//												break ;
//		case slave_SMSDat:	memcpy(MsegTemp, Rs485_TranSmit.Fromat.DAT, Rs485_TranSmit.Fromat.Dat_len);//处理从机发出的短信数据
//												MsegTemp[Rs485_TranSmit.Fromat.Dat_len] = 0;
//												Rs485Sending.Sending = 0;xEventGroupSetBits(AlarmStateHandler,	SMS_ReadyBIT_1);	//SIM卡报警 开始执行报警发短信打电话
//												break ;
//		
//		case Salve_stateEND:	xEventGroupSetBits(AlarmStateHandler,	Rs485HostBIT_11);	//处理从机发出的无报警任务指令 从机没报警任务 开启主机RS485轮询下一个从机	
//												Rs485Sending.Sending = 0;break ;
//		//sizeof(array)/sizeof(array[0])
//		
//		case slave_RegDat:	result = search(RegisterEEpro_Slave.RegisterArray, RegTotal,Rs485_TranSmit.Fromat.Source_Addr);
////												result = (int*)bsearch(&sRS485_dat->Fromat.Source_Addr, RegisterEEpro_Slave.RegisterArray, RegTotal, sizeof(uint8_t ), compare);//查找已注册的数组中是否存在
//												if(((RegisterEEpro_Slave.SlaveAddr_write_cousor < RegTotal)&&(result < 0))&&(!SimRest))			//处理从机发起的请求注册地址
//												{
//													RegisterEEpro_Slave.RegisterArray[RegisterEEpro_Slave.SlaveAddr_write_cousor] = Rs485_TranSmit.Fromat.Source_Addr;//主机将从机地址写入注册数组内
//													temp = SUCCESS ;																																		//成功
//													RegisterEEpro_Slave.SlaveAddr_write_cousor++;
//												}
//												else temp = FAILED ;
//												if(RegisterEEpro_Slave.SlaveAddr_write_cousor > RegTotal)RegisterEEpro_Slave.SlaveAddr_write_cousor = RegTotal;
//												RS485SendData(Rs485_TranSmit.Fromat.Source_Addr,HostAddr,Host_ReturnReg,1,&temp);//向从机回复注册结果
//												break ;
///*从机处理函数*/
//		case Host_ReturnReg: if(SUCCESS == Rs485_TranSmit.Fromat.DAT[0]){Rs485Sending.link_state = SUCCESS;StatusBarPrintf("RS485连接成功");}//从机接收主机返回的地址注册 是否成功	
//												else 
//													{
//														Rs485Sending.link_state = ERROR;StatusBarPrintf("RS485连接失败") ;
//														USART_EnableDMA(UARTx_COM[RS485_COM], USART_DMAREQ_TX | USART_DMAREQ_RX, DISABLE);            //关闭串口的DMA发送 接收
////												USART_Enable(UARTx_COM[RS485_COM], DISABLE);//关闭串口
//													}
//												break ;		
//		case Host_GetSlaveAlarmCmd: xEventGroupSetBits(AlarmStateHandler,	Rs485SlaveAlarmBIT_10);	//主机发起 获取从机报警任务	
//												break ;													
//		default:break;
//	}

//reEEROR:

//__NOP();

//		vTaskDelay(0.5*configTICK_RATE_HZ) ;                       //：1S/次
//	}	
//}
/*
查询历史报警记录 用户设置的信息 3秒后返回工作主页
*/
//void KeyQueryTask (void *pvParameters)
//{
//	
//	while(1)
//	{
//		if((!UserKey.CloseState)&&((!UserQuery_KEY)))	   //用户按健按下闭合有效 下降沿
//		{ 
////			if(++UserKey.QueryCnt > 1)
////			{
//			UserKey.LastValuae=1;UserKey.QueryCnt = 0;
//				
//			if(UserKey.CurrentPage++ > 43)UserKey.CurrentPage = 1;//最多51 5*8+3=43页循环
//				else vTaskSuspend(LedTaskHandler);//挂起Led任务
//			switch(UserKey.CurrentPage)
//				{
//					case 1:case 2:case 3:case 4:	//显示历史报警记录页面
//								if(ERROR == HistoryAlarmPage1())UserKey.CurrentPage = 4;//
//								break;
//					default:	//显示端口信息页面
//								UserInforPage1();
//								break;	
//				}	
////			}
//		}	
//		else if((UserKey.LastValuae)&&(UserQuery_KEY))	//按键松开有效 上升沿 超过延时返回首页
//			{ UserKey.CloseState = 0;
//			if(++UserKey.QueryCnt > SingeDelay*10)					//查询 30次
//				{
//				UserKey.CurrentPage = UserKey.LastValuae = UserKey.QueryCnt = 0;		
//				HomeMenu();//显示首页
//				}
//			}
//		else {UserKey.QueryCnt = 0;}//UserKey.LastValuae=
//	
//		vTaskDelay(0.5*configTICK_RATE_HZ) ;                       //处理RS232数据：0.5S/次		
// }
//}
/*以下任务暂缓 后续开发*/

///*********************************************************************************************************************
//*TcpUrcTask任务函数
//*********************************************************************************************************************/
//void TcpUrcTask(void *pvParameters)
//{
//	uint16_t len = 0 ;
//    while(1)
//    {
////			if(strlen((const char*)ec20AtBuf) > 1)//接收数据长度大于1
////			{
////				if(hand(ec20AtBuf,"SMS FULL"))//SIM卡是否存储满
////					{EC20_SendModuleCmd(AT_CMGD, NULL);}
////				else if(hand(ec20AtBuf,"NOT AVAILABLE"))//掉网
////				{
////				xEventGroupClearBits(netEventHandler,	NETREGBIT_1);  //设置“标志位0：EC20 moudle初始化状态 重启
////				} 
////			}	
//			
////			  SETCAMERATASKOK ;                                      		 //任务已正常运行标志位
//			  char *buf = portMalloc(256*sizeof (char)) ;
//				
//				if(RW_OK == GetQueueMemData(&sUrcQueue, buf, &len))        //从sUrcQueue帧缓冲区中获取一帧数据准备处理
//					{  
//						 TcpUrcType urcType = TcpUrcHandle(buf, len) ;         //URC分类解析
//						 switch( urcType )
//						   {
//							 case CLOSED:
//								   CLEAMQTT ;                                     //出现 TCP 断链 设置TCP状态为 STATE_ERR
//								   ErrorLogPrintf("Tcp server closed .") ;
////							     WriteLogToFlash("Tcp server closed .") ;
//							     break ;
//							 case PDPDEACT:
//								  CLEATCP ; CLEANETREG ;                                  //设置MODULE状态为 STATE_ERR                       
//								   ErrorLogPrintf("Tcp PDP deact .") ;
////							     WriteLogToFlash("Tcp PDP deact .") ;
//							     break ;
//							 case INCOMING_FULL:
//								   ErrorLogPrintf("Tcp incoming full .") ;
////							     WriteLogToFlash("Tcp incoming full .") ;
//							     break ;								   
//							 case INCOMING_CONT:
//								   ErrorLogPrintf("Tcp incoming .") ;
////							     WriteLogToFlash("Tcp incoming .") ;
//							     break ;
//               default:
//								 ErrorLogPrintf("Tcp unknown urc :%s ",buf) ;
////							     WriteLogToFlash("Tcp unknown urc .") ;
//							     break ;             								 
//							}
//					}
//				portFree(buf) ;			  
//        vTaskDelay(0.5*configTICK_RATE_HZ);                       // (0.5*configTICK_RATE_HZ)/configTICK_RATE_HZ = 0.5S 运行一次
//    }
//}
/*********************************************************************************************************************
 TcpDownTask任务函数 处理查询命令 设置电话号码、短信 w433等参数
*********************************************************************************************************************/
//void TcpDownTask(void *pvParameters)
//{
//  while(1)
//	{
////		SETLTETASKOK ;                                                //该任务已正常运行标志位
//		char *buf = portMalloc(1024) ;
//		uint16_t len = 0 ;
//		if(RW_OK == GetQueueMemData(&sTcpQueue, buf, &len))           //从sTcpQueue帧缓冲区中获取一帧数据准备处理
//			{
//				int pos = 0;
//				pos = kmp(buf, "+QIURC: \"recv\"") ; 
//				if( pos>=0)                                               /*判断是否位接收到TCP下行数据*/
//					{
///*(L0142MAC:31FFD405524E353728902251;00&I am heart 5 .$X)*/
//						char *tcpData = portMalloc(512) ;
//						CopyStr(tcpData, buf+pos, 0x0A, 0x00, 256) ;							
//						TcpFrame_S *psRcmd = (TcpFrame_S*)tcpData ;
//						int dataLen = strlen(tcpData) ;                       //接收数据的长度
//						if( (psRcmd->head         == '(')&&
//								(psRcmd->loadHead     == '&')&&
//								(*(tcpData+dataLen-5) == '$')&& 
//								(*(tcpData+dataLen-3) == ')')                     //用户数据帧合法性校验						
//							)
//							{
//								switch( psRcmd->frameType )                       //按照帧类型分类处理
//								  {
//									  case TCPCMDFRAME:
//										   {
//											   if( RW_OK != InsertQueueMemData(&sUsercmdQueue, tcpData, dataLen) )
//														{
//															ErrorLogPrintf("sUsercmdQueue溢出！") ;
//														}
//													break ;
//											 }
//									 	case TCPSERVERACK:                            //Tcp server对硬件上行的数据ACK确认回复包
//											 {
//												 AppLogPrintf("Tcp server确认收到数据帧:%s ", tcpData) ;
//												 break ;
//											 }
//									 default:
//									     {
//										     AppLogPrintf("TCP收到下行未知类型数据帧:%s ", tcpData) ;
//												 TcpWritedata( TCPCMDBACKFRAME, "TCP收到下行未知类型数据帧") ;
//												 break ;
//									     }								
//									}
//							}
//						else                                                  /*tcp下行数据，但是非下行命令帧*/
//							{
//								AppLogPrintf("TCP收到下行'非'法数据帧：%s ", tcpData) ;
//							}
//						portFree(tcpData) ;
//					}
//				else                                       	              /*TCP下行数据解析失败*/	
//					{
//						ErrorLogPrintf("TCP 下行数据解析失败：%s ", buf) ;							
//					}								 
//			}							
//	  portFree(buf) ;		
//		vTaskDelay(3);                                                //3/configTICK_RATE_HZ=0.03s运行一次 30ms
//	}}
// //USB任务函数   ：处理USB串口数据
//void UsbTask(void *pvParameters)
//{
//  while(1)
//	{                                                              //任务已经正常运行标志位
//		char *buf = portMalloc(512) ;
//		uint16_t len = 0 ;
//	  if( RW_OK == GetQueueMemData(&sUsbQueue, buf, &len))	       //从sUsbQueue帧缓冲区中获取一帧数据准备处理
//			{							
//				TcpFrame_S *psRcmd = (TcpFrame_S*)buf ;
//				int bufLen = strlen(buf) ;
//				if( (psRcmd->head     == '(')&&
//						(psRcmd->loadHead == '&')&&
//						(*(buf+bufLen-5)  == '$')&& 
//						(*(buf+bufLen-3)  == ')')                            //用户数据帧合法性校验 合法命令帧最后5个字节为“$X)\r\n”					
//					)
//					{
//						switch( psRcmd->frameType )                       	 //按照帧类型分类处理
//							{
//								case COMCMDFRAME:                                  //下行的控制命令帧
//									 {
//										 if( RW_OK != InsertQueueMemData(&sUsercmdQueue, buf, bufLen) )
//												{
//													ErrorLogPrintf("sUsercmdQueue溢出！") ;
//												}
//											break ;
//									 }
//								case TCPSERVERACK:                               //USB 上位机软件对硬件上行的数据ACK确认回复包
//								   {
//									   AppLogPrintf("USB上位机确认收到数据帧：%s .", buf) ;
//										 break ;
//									 }									
//							 default:
//									 {
//										 AppLogPrintf("USB收到下行未知类型数据帧：%s .", buf) ;
//										 TcpWritedata( TCPCMDBACKFRAME, "USB收到下行未知类型数据帧") ;
//										 break ;
//									 }								
//							}
//					}
//				else                                                     /*收到USB数据，但是非下行命令帧*/
//					{
//						AppLogPrintf("USB收到下行'非'法数据帧：%s .", buf) ;
//					}
//			}							
//		portFree(buf) ;
//		vTaskDelay(0.5*configTICK_RATE_HZ);
//	}
//}
///*********************************************************************************************************************
//* 用户命令处理任务函数
//*********************************************************************************************************************/
//void UcmdTask(void *pvParameters)
//{
//    while(1)
//    {	
////      SETUCMDTASKOK ;                                                        	     
//			char *buf = portMalloc(512) ;
//			uint16_t len = 0 ;
//			if(RW_OK == GetQueueMemData(&sUsercmdQueue, buf, &len)) //从sUsercmdQueue帧缓冲区中获取一帧数据准备处理
//				{
//					Cmd_Process(buf, len) ;	                            //处理用户帧命令				
//				}	
//      portFree(buf) ;				
//      vTaskDelay(0.5*configTICK_RATE_HZ);
//    }
//}
//--- 发布一条消息
//-- @string topic UTF8编码的字符串
//-- @string payload 用户自己控制payload的编码，mqtt.lua不会对payload做任何编码转换
//-- @number[opt=0] qos 0/1/2, default 0
//-- @number[opt=0] retain 0或者1
//-- @return bool 发布成功返回true，失败返回false
//-- @usage
//-- mqttc = mqtt.client("clientid-123", nil, nil, false)
//-- mqttc:connect("mqttserver.com", 1883, "tcp")
//-- mqttc:publish("/topic", "publish from luat mqtt client", 0)
//function mqttc:publish(topic, payload, qos, retain)
//    if not self.connected then
//        log.info("mqtt.client:publish", "not connected")
//        return false
//    end
//    
//    qos = qos or 0
//    retain = retain or 0
//    
//    if not self:write(packPUBLISH(0, qos, retain, qos > 0 and self.getNextPacketId() or 0, topic, payload)) then
//        log.info("mqtt.client:publish", "socket send failed")
//        return false
//    end
//    
//    if qos == 0 then return true end
//    
//    if not self:waitfor(qos == 1 and PUBACK or PUBCOMP, self.commandTimeout, nil, true) then
//        log.warn("mqtt.client:publish", "wait ack timeout")
//        return false
//    end
//    
//    return true
//end

//--- 接收消息
//-- @number timeout 接收超时时间，单位毫秒
//-- @string[opt=nil] msg 可选参数，控制socket所在的线程退出recv阻塞状态
//-- @return result 数据接收结果，true表示成功，false表示失败
//-- @return data 
//--                如果result为true，表示服务器发过来的mqtt包
//--
//--                如果result为false，超时失败,data为"timeout"
//--                如果result为false，msg控制退出，data为msg的字符串
//--                如果result为false，socket连接被动断开控制退出，data为"CLOSED"
//--                如果result为false，PDP断开连接控制退出，data为"IP_ERROR_IND"
//--
//--                如果result为false，mqtt不处于连接状态，data为nil
//--                如果result为false，收到了PUBLISH报文，发送PUBACK或者PUBREC报文失败，data为nil
//--                如果result为false，收到了PUBREC报文，发送PUBREL报文失败，data为nil
//--                如果result为false，收到了PUBREL报文，发送PUBCOMP报文失败，data为nil
//--                如果result为false，发送PINGREQ报文失败，data为nil
//-- @return param 如果是msg控制退出，param的值是msg的参数；其余情况无意义，为nil
//-- @usage
//-- true, packet = mqttc:receive(2000)
//-- false, error_message = mqttc:receive(2000)
//-- false, msg, para = mqttc:receive(2000,"APP_SEND_DATA")
//function mqttc:receive(timeout, msg)
//    if not self.connected then
//        log.info("mqtt.client:receive", "not connected")
//        return false
//    end
//    
//    return self:waitfor(PUBLISH, timeout, msg)
//end

//--- 断开与服务器的连接
//-- @return nil
//-- @usage
//-- mqttc = mqtt.client("clientid-123", nil, nil, false)
//-- mqttc:connect("mqttserver.com", 1883, "tcp")
//-- process data
//-- mqttc:disconnect()
//function mqttc:disconnect()
//    if self.io then
//        if self.connected then self:write(packZeroData(DISCONNECT)) end
//        self.io:close()
//        self.io = nil
//    end
//    self.cache = {}
//    self.inbuf = ""
//    self.connected = false
//end

