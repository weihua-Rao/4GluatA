
#include "main.h"
#include "rtc.h"
/********************************************************************************
  * @file    gateapp.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   针对SmartGate应用程序，对所有驱动进行封装
  ******************************************************************************
  * @attention
  * 所有对除USER层的封装都放在gateapp中
*******************************************************************************/


/*********应用层外部调用文件*******************************************************/

/**********************************************************************************
*内部使用的常变量
**********************************************************************************/

/**********************************************************************************
*供外部使用的常变量
***********************************************************************************/

Application gateApp ;                         //gateApp存放应用参数

/*应用层缓冲队列*/
EventGroupHandle_t netEventHandler ;          //事件标志组（标志位反馈网络状态）
//EventGroupHandle_t osSafeEventHandler ;       //事件标志组（标志位OS每个任务的运行情况）
SemaphoreHandle_t  ec20MutexSemaphore;	      //互斥信号量句柄（EC20资源抢占及释放）
//SemaphoreHandle_t  WeightTempSemaphore;
char               lastOccupyEc20[50] = {0} ; //全局数组 记录上一次占用EC20资源的__FILE__,  __LINE__
char               lastReleseEc20[50] = {0} ; //全局数组 记录上一次释放EC20资源的__FILE__,  __LINE__
volatile unsigned long *LED_array[8]	= {&LED8,&LED7,&LED6,&LED5,&LED4,&LED3,&LED2,&LED1};
/**************************************************************************************************
* 名    称：  void Board_Init(void) 
* 功能说明：  驱动及功能块初始化
**************************************************************************************************/
void Board_Init(void) 
{
   	SysTick_Init() ;	    				                            //系统滴答定时器初始化	1ms中断一次  中断内部进行了喂狗操作	
	  LogFlash_Init() ;                                         //初始化本地FLASH总存储的LOG    						
//	  MyMenInit(SRAMIN) ;                                       //动态内存分配初始化
//	RCC_ConfigLse(TSC_CLK_SRC_LSI);							//关闭外部低速时钟 使用内部RC
	N32_RTC_Initia();														//实时时钟初始化

//	  PrintfDeviceInfo() ;                                      //UART_DEBUG输出硬件信息
//	  Rtc_RegHookCallback(RTC_IT_ALR, Rtc_Alr_Callback) ;       //注册RTC闹钟中断回调函数
//	  UserKeyInit() ;                                           //初始化KEY按键	
//		Led_Init() ;		  					                              //初始化LED
//		Beep_Init() ;                                             //初始化蜂鸣器
ADC_Initial(ADC3);//ADC3_5
//		T_Adc_Init() ;                                            //初始化ADC用于MCU内部温度检测
	  OLED_Init() ;                                             //OLED初始化
MYI2C_Init(&SENx,1000,0x38);    //2000:读取数据周期2S;   0x38:AHT20地址
//MYI2C_Handle(&SENx);   //不使用死延时则定时10ms调用一次该函数
//	 printf("RH = %0.2f",SENx.RH);
//	 printf("T = %0.2f\n",SENx.T);
//char *oledBuf = (char*)portMalloc(16) ;
//snprintf(oledBuf, 16, "%s",(char*)&SENx.RH) ; //显示屏显示EC20电压
//OLED_ShowString(64, 0, false, oledBuf, HIGH_16);
//	GetEEpro_rs485_user(CSCA_W433_Address + RS485_AddrOffset);//初始化	COM2 RS485映射的串口 
	  //Pvd_Init( EXTI_Trigger_Rising_Falling, INT_RANK_14) ;
//	  PWR_PVD_Init() ;
/*433M遥控功能*/
//#ifdef W433_Enabel
	if(ERROR == Factory_Eeprom_Init())	
	{
	AppLogPrintf("SN错误重启系统!") ;
//	StatusBarPrintf("SN ERROR!") ;		
  NVIC_SystemReset();
	}
//#endif

	W433Init();
	
/***自动识别有几个433m终端已经配对成功的端口，LED亮灯对应**/	
	EEPROM_read_n((CSCA_W433_Address+M433_AddrOffset),(char*)&Save_433M_Value[0].Device_Index, 56);//读取数据长度56
//	Save_433M_Maxlen = Save_433M_Value[0].Device_Index;
	if(W433Hard.Head.W433Model != 0x33)
		{
			for(u8 i=0;i<8;i++)
			{
			if((Save_433M_Value[i].Device_Index > 0) && (Save_433M_Value[i].Device_Index < 0xff)) 
				*LED_array[i] = 0;
			else  *LED_array[i] = 1;		
			}
		}	
/****END****/		
}
/**************************************************************************************************
* 名    称：  void PrintfDeviceInfo(void)
* 功能说明：  DEBUG口输出信息
**************************************************************************************************/
//void PrintfDeviceInfo(void)
//{
//	 	printf("\r\n**************************************************************" ) ;
//		printf("\r\n版权归属 : 精创迪科技 2022/3-2027/3 .\
//						\r\n技术支持 : Mr. Rao  ") ;
//	  SetBootVersion(&gateApp, (char*)uIapFlash.sIapFlash.BootVers) ;
//	  printf("\r\nBoot版本 : %*.*s", 0, VERSION_LEN, gateApp.bootVers) ;
//		Query_AppVersion((char*)gateApp.appVers) ;
//		SetAppVersion(&gateApp, (char*)gateApp.appVers) ;
//		printf("\r\nAPP 版本 : %s", gateApp.appVers) ;	
//    printf("\r\nRunAppNum: APP%d", uIapFlash.sIapFlash.RunAppNum-0x30 ) ;	
//		GetDeviceMacAddress((uint8_t*)gateApp.macId, STRMACID) ; 			
//		printf("\r\n设备编号 : %.*s", MAC_BYTES_LEN, gateApp.macId) ;	
//	  DeviceRstReason((uint8_t*)gateApp.rstReason, VERSION_LEN) ;
//	  printf("\r\n重启原因 : %.*s", VERSION_LEN, gateApp.rstReason) ;
//		printf("\r\n**************************************************************" ) ;
//	  char buf[128] = {0} ;
//		snprintf(buf, 128, "重启原因:%s.", gateApp.rstReason) ;
//    WriteLogToFlash(buf) ;	
//}

/**************************************************************************************************
* 名    称：  void DeviceRstReason(uint8_t *reason, uint8_t maxLen)
* 功能说明：  判断硬件重启原因
**************************************************************************************************/
void DeviceRstReason(uint8_t *reason, uint8_t maxLen)
{
	if( SET == RCC_GetFlagStatus( RCC_FLAG_PORRST) )//RCC_FLAG_PORRST
	  {
			strncpy((char*)reason, "重上电启动", maxLen) ;
		}
	if( SET == RCC_GetFlagStatus(RCC_FLAG_SFTRST) )//RCC_FLAG_SFTRST
	  {
			strncpy((char*)reason, "软复位启动", maxLen) ;
		}
	if( SET == RCC_GetFlagStatus(RCC_FLAG_IWDGRST) )//RCC_FLAG_IWDGRST
	  {
			strncpy((char*)reason, "独立看门狗启动", maxLen) ;
		}
	if( SET == RCC_GetFlagStatus(RCC_FLAG_WWDGRST) )//RCC_FLAG_WWDGRST
	  {
			strncpy((char*)reason, "窗口看门狗启动", maxLen) ;
		}
	if( (RESET == RCC_GetFlagStatus(RCC_FLAG_SFTRST))  &&
		  (RESET == RCC_GetFlagStatus(RCC_FLAG_IWDGRST)) &&
	    (RESET == RCC_GetFlagStatus(RCC_FLAG_WWDGRST)) &&
		  (SET   == RCC_GetFlagStatus(RCC_FLAG_PINRST))
   	)
	  {
			strncpy((char*)reason, "硬复位启动", maxLen) ;
		}
	RCC_ClrFlag() ;//RCC_ClearFlag
}

/****************************************************************************
* 名    称：void OccpyEc20(TickType_t timeout, char* file, int line)
* 功    能：获取EC20资源， 如果超过timeout没有获取到资源则写入LOG然后重启系统
* 注    意：一直挂起延时为：portMAX_DELAY
****************************************************************************/
void OccpyEc20(TickType_t timeout, char* file, int line)
{
		BaseType_t result = pdFAIL ;
		result = xSemaphoreTake(ec20MutexSemaphore, timeout) ;
		if( result == pdPASS )
			{
				memset(lastOccupyEc20, 0, 50); 
				snprintf( lastOccupyEc20, 49, "%s %d 抢占4G!", file, line) ; 
				AppLogPrintf(lastOccupyEc20) ;
//				UARTx_SendData(UART_DEBUG, lastOccupyEc20, strlen(lastOccupyEc20)) ;
			}
		else
			{
				char *buf = portMalloc(192);
				snprintf( buf, 192, "上次:%s, 上次:%s, 当前%s %d 抢占4G失败，系统软重启！20%02d-%02d-%02d %02d:%02d:%02d", 
						lastOccupyEc20, lastReleseEc20, file, line ,
						Read_history.Year,Read_history.Month,Read_history.Date,
						Read_history.Hours,Read_history.Minutes,Read_history.Seconds
				) ;
				WriteLogToFlash(buf) ;
				AppLogPrintf(buf) ;
				portFree(buf) ;				
//				SystemSoftReset() ;          //系统重启
				NVIC_SystemReset();
			}	
}

/****************************************************************************
* 名    称：void TcpUpFlashLog(void)
* 功    能：通过TCP上传本地FLASH存储的LOG
****************************************************************************/
//void TcpUpFlashLog(void)
//{
//	  uint32_t readAddr = LOGFLASH_START_ADDRESS ;
//		uint8_t* logReadBuffer = portMalloc(FLASH_PAGE_SIZE/8) ;
//		uint16_t n = uLogFlash.sLogFlash.writeOffset/(FLASH_PAGE_SIZE/8) ;
//		while(n--)
//			{
//				Read_Flash_Byte(readAddr, logReadBuffer, (FLASH_PAGE_SIZE/8)) ;
//				TcpWritedata( TCPLOGFRAME, "本地log:%s", (char*)logReadBuffer);
//				readAddr = readAddr+(FLASH_PAGE_SIZE/8) ;
//			}
//		memset(logReadBuffer, 0, (FLASH_PAGE_SIZE/8)) ; 
//		Read_Flash_Byte(readAddr, logReadBuffer, uLogFlash.sLogFlash.writeOffset%(FLASH_PAGE_SIZE/8) ) ;
//		TcpWritedata( TCPLOGFRAME, "本地log:%s", (char*)logReadBuffer) ;
//		portFree(logReadBuffer) ;
//		Erase_LogFlash() ;
//}

/**************************************************************************************************
* 名    称：  void RefreshOledTime(void)
* 功能说明：  刷新显示屏第3行、4行显示时间“2018-03-20 17:51”
**************************************************************************************************/
RunResult RefreshOledTime(void)
{
	RunResult result = RUNOK;
static 	RTC_DateType uLastDate;
	RTC_DateShow();RTC_TimeShow();
			if( memcmp((const void*)&uLastDate, (const void*)&RTC_DateStructure ,4) == 0  )  /*秒钟之前的数据完全相同，则只改变时间点*/
		  {
//			  DisplayPointBlink() ;
				OledPrintf(LINE_RIGHT, HIGH_12, LINE4, false, "%02d:%02d:%02d",RTC_TimeStructure.Hours,RTC_TimeStructure.Minutes,RTC_TimeStructure.Seconds);
//	snprintf(rtcTime,80,"%0.2d:%0.2d:%0.2d",RTC_TimeStructure.Hours,RTC_TimeStructure.Minutes,RTC_TimeStructure.Seconds);
//	OLED_ShowString(0, 6, false, rtcTime, HIGH_12);	//X Y 128*64				
			}
		else                                                      /*年月日时分有发生改变*/
			{
//				result = SYNC_NetTime();
				result = SYNC_LBS_Time();//基站定位LBS信息和时间	
				if(RUNOK != result)
				{	
					SAPBR_Init();
//					result = SYNC_NetTime();
					result = SYNC_LBS_Time();
//				memcpy((void*)&uLastDate.WeekDay, (const void*)&RTC_DateStructure.WeekDay, sizeof(RTC_DateStructure));
				}
//				if(RUNOK == result)
//				{
					memcpy((void*)&uLastDate.WeekDay, (const void*)&RTC_DateStructure.WeekDay, sizeof(RTC_DateStructure));	
					DisplayTime() ;//OLED更新时间						
//				}
			}
return result;	
//	static Calendar_u uLastTime ;			
//	if( currentMenuIndex == 0 )   //HTTP网络状态标志位为STATE_OK，表示已经获取到正确时间 (GETHTTPSTATE == STATE_OK) && 
//	{
//		if( memcmp(uLastTime.bytes, uCalendar.bytes, 16) == 0  )  /*秒钟之前的数据完全相同，则只改变时间点*/
//		  {
//			  DisplayPointBlink() ;
//			}
//		else                                                      /*年月日时分有发生改变*/
//			{
//				DisplayTime() ;	
//		    strncpy((char*)uLastTime.bytes, (const char*)uCalendar.bytes, 20) ;				
//			}
//	}
}
/**************************************************************************************************
* 名    称：  void RefreshWeightTime(void)
* 功能说明：  继电器供电 时间长度 定时器
**************************************************************************************************/
//void SendRs485Call(void)
//{
//发送任务通知
////	 xTaskNotify( menuTaskHandler,
////													 0,
////													eIncrement );	//eAction
////	vTaskNotifyGiveFromISR(DataProcess_Handler,&xHigherPriorityTaskWoken);//发送任务通知

//	xTaskNotifyFromISR((TaskHandle_t	)SendRs485TaskHandler,		//接收任务通知的任务句柄
//							(uint32_t		)1,						//Task.Num任务通知值
//							(eNotifyAction	)eSetValueWithOverwrite,	//覆写的方式发送任务通知
//							(BaseType_t*)	pdTRUE);//退出此函数以后是否进行任务切换pdFALSE
	
//		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//如果需要的话进行一次任务切换
//}
/**************************************************************************************************
* 名    称：  void RunTaskTime(void)
* 功能说明：  通用定时器 单位10秒 
**************************************************************************************************/
void RunTaskTime(void)
{
//char buf = 0;
BaseType_t xHigherPriorityTaskWoken;	

portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//如果需要的话进行一次任务切换		
//	if(TimerTask[Co2Time].TimeSw)//CO2采集数据开关打开    
//		{
//		if(TimerTask[Co2Time].TimeCount < TimerTask[Co2Time].TimeSetValue)TimerTask[Co2Time].TimeCount++;
//		else
//			{	
//				if((CO2_5 & xEventGroupGetBits(Rs485EventHandler)) != CO2_5)
//				{
//					TimerTask[Co2Time].TimeCount = 0;
//					xEventGroupSetBits(Rs485EventHandler, CO2_5);
//					buf = 5;InsertQueueMemData(&DataCollectQueue, &buf, 1);
//				}
//			}
//		}		

}

/**************************************************************************************************
* 名    称：  void Rtc_Alr_Callback(void)
* 功能说明：  RTC闹钟中断回调函数，中断内先重设闹钟时间为第二天，同时重启设备
**************************************************************************************************/
//void Rtc_Alr_Callback(void)
//{
//	Calendar_u  uAlaCal ;
//	memcpy(uAlaCal.bytes, uCalendar.bytes, CALENDAR_LEN) ;
//	uAlaCal.sCalendar.w_date[1] += 1 ;               //天数+1天
//	memcpy((u8*)uAlaCal.sCalendar.hour, "01", 2) ;   //凌晨1点闹钟时、分取决于此刻的时、分值//
//	//memcpy((u8*)uAlaCal.sCalendar.min,  "42", 2) ; //凌晨30分//uAlaCal.sCalendar.sec[0] += 2 ; 
//	RTC_Alarm_Set(&uAlaCal) ;
//	WriteLogToFlash((char*)uCalendar.bytes) ;        //将新日期写入FLASH中         
//	WriteLogToFlash("Rtc_Alr_Callback!") ;
//	SysLog("Rtc_Alr_Callback!") ;
//	SystemSoftReset() ;
//}

/**************************************************************************************************
* 名    称：  void InitApplictationState(Application *appPointer)
* 功能说明：  对Application类型数据进行初始化
* 入口参数：  @param *appPointer    Application结构指针
  *************************************************************************************************/
//void InitApplictationState(Application *appPointer)
//{
//  memset(appPointer->appVers,   0, VERSION_LEN+1) ;
//	memset(appPointer->bootVers,  0, VERSION_LEN+1) ;
//	memset(appPointer->macId,     0, MAC_BYTES_LEN+1) ;
//}


//void SetAppVersion(Application *appPointer, char *version) 
//{
//  strncpy((char*)appPointer->appVers, version, VERSION_LEN) ;
//	*(appPointer->appVers+VERSION_LEN+1) = 0 ;
//}

//void SetBootVersion(Application *appPointer, char *version) 
//{
//  strncpy((char*)appPointer->bootVers, version, VERSION_LEN) ;
//	*(appPointer->bootVers+VERSION_LEN+1) = 0 ;
//}
