
//#include "syslib.h"
//#include "n32g43x.h"

//#include "stdio.h"
//#include "syslib.h"
//#include "misc.h"
#include "rtc.h"
//#include	"n32g43x_rcc.h"
//#include	"n32g43x_rtc.h"
//#include	"n32g43x_exti.h"
//#include	"usart.h"
//#include	"rs232.h"
//#include "stm32f10x_pwr.h"
//#include "stm32f10x_bkp.h"
#include "n32g45x_rtc.h"
#include "main.h"
#include "User_RTC_Config.h"
//#include "sysport.h"

/********************************************************************************
  * @file    rtc.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   提供STM32内部rtc相关驱动
  ******************************************************************************
*******************************************************************************/

/*****************************************
*供内部使用的常变量
****************************************/
RTCFP  rtcSecFp = NULL ;                                           //RTC秒中断回调函数指针
RTCFP  rtcAlrFp = NULL ;																					 //RTC闹钟中断回调函数指针
const uint8_t weekTable[12]={0,3,3,6,1,4,6,2,5,0,3,5};             //月修正数据表	  
const uint8_t monTable[12] ={31,28,31,30,31,30,31,31,30,31,30,31}; //平年的月份日期表

/*****************************************
*供外部使用的常变量
****************************************/
Calendar_u  uCalendar ;                                             //Calendar_u共用体变量uCalendar，用于纪录实时时间
LBStype LBS;

/**************************************************************************************************
* 名    称：RTCFP  Rtc_RegHookCallback(uint16_t rtcIt, RTCFP pCallback) 
* 功能说明：rtc中断回调函数-注册函数，驱动中定义了rtcSecFp秒中断回调函数指针类型，并定义了rtcAlrFp闹钟中断回调函数指针变量
*           Rtc_RegHookCallback函数就是将回调函数地址传递给rtcSecFp\rtcAlrFp指针变量
* 入口参数：
*           @param1 rtcIt        RTC_IT_SEC 或者RTC_IT_ALR
*           @param2 pCallback     RTCFP类型函数指针
* 出口参数：
*           @param1 pCallback     RTCFP类型函数指针
*************************************************************************************************/ 
RTCFP  Rtc_RegHookCallback(uint16_t rtcIt, RTCFP pCallback) 
{ 
	if( RTC_A_ALARM == rtcIt )  //秒中断回调函数注册
	 {
	   	if( rtcSecFp == NULL )
		    rtcSecFp = pCallback ;			
			else
				 SysErr("Rtc SecTi Callback repeat reg!") ;
	 }
//	else if( RTC_IT_ALR == rtcIt)
//	 {
//	   	if( rtcAlrFp == NULL )
//		    rtcAlrFp = pCallback ;			
//			else
//				 SysErr("Rtc AlrTi Callback repeat reg!") ;	 
//	 }
//	else
//	 {
//	     SysErr("Rtc_RegHookCallback Failed!") ;	 
//	 }
  return 	pCallback ;
} 

/**************************************************************************************************
* 名    称：void Rtc_Hook(uint16_t rtcIt)
* 功能说明：RTC中断内调用的钩子函数（执行到中断会把相应的回调函数勾出来运行，嘿嘿嘿。。。
*           当然不同的RTC_IT会勾出不同的回调函数）
* 入口参数：
*           @param1 rtcIt        RTC_IT_SEC 或者RTC_IT_ALR
*************************************************************************************************/ 
void Rtc_Hook(uint16_t rtcIt)
 {
	 switch(rtcIt)
	 {
	   case RTC_A_ALARM:
			  	if( rtcSecFp != NULL)
 		      rtcSecFp() ;
					break ;
//	   case RTC_IT_ALR:  if( rtcAlrFp != NULL) rtcAlrFp()  ; break ;
     default: break ;																														
	 }
 }
 
/**************************************************************************************************
* 名    称：  void RTC_IRQHandler(void)
* 功能说明：  RTC时钟中断
* 说    明：  每秒触发一次中断
*************************************************************************************************/
//void RTC_WKUP_IRQHandler(void)
//{	
//	if ( RTC_GetITStatus(RTC_A_ALARM) != RESET )//秒钟中断
//		{
//      RTC_ClrIntPendingBit(RTC_A_ALARM);		 //RTC_ClearITPendingBit清秒中断
////			RTC_WaitForLastTask() ;
//      SysLog("RTC_IT_SEC!") ;
//			Rtc_Hook(RTC_A_ALARM) ;						
//		}
//	if( RTC_GetITStatus(RTC_IT_ALR)!= RESET )  //闹钟中断
//		{
//			RTC_ClrIntPendingBit(RTC_IT_ALR);		 //RTC_ClearITPendingBit清闹钟中断
//			RTC_WaitForLastTask() ;		
//			SysLog("RTC_IT_ALR!") ; 
//      Rtc_Hook(RTC_IT_ALR) ;	
//	  } 		
//}
void RTCAlarm_IRQHandler(void)
{	
	if ( RTC_GetITStatus(RTC_INT_ALRA) != RESET )//秒钟中断
		{
			RTC_ClrIntPendingBit(RTC_INT_ALRA);

       EXTI_ClrITPendBit(EXTI_LINE17);
//			RTC_WaitForLastTask() ;
      SysLog("RTC_IT_SEC!") ;
			Rtc_Hook(RTC_A_ALARM) ;						
		}

}
/**************************************************************************************************
* 名    称：  void RTCAlarm_IRQHandler(void)
* 功能说明：  RTC闹钟中断
* 说    明：  RTC计数器到达RTC->ALR值时发生中断。 注意：RTC闹钟中断挂载在EXTI_Line17中断线上的，注意需要清标志位
*************************************************************************************************/
//void RTCAlarm_IRQHandler(void)
//{	
//	if( RTC_GetITStatus(RTC_IT_ALR)!= RESET )  //闹钟中断
//		{
//		  EXTI_ClrITPendBit(EXTI_LINE17);//EXTI_ClearITPendingBit(EXTI_Line17)
//			RTC_ClrIntPendingBit(RTC_IT_ALR);		 //RTC_ClearITPendingBit清闹钟中断
//			RTC_WaitForLastTask() ;		
//			//SysLog("RTC_IT_ALR!") ; 
//      Rtc_Hook(RTC_IT_ALR) ;	
//	  } 
//}

/**************************************************************************************************
* 名    称：  void Rtc_Sec_Callback(void)
* 功能说明：  RTC秒中断回调函数
*************************************************************************************************/
void Rtc_Sec_Callback(void)
{
//   RTC_Get(&uCalendar);                   //更新时间
//	RTC_DateShow();RTC_TimeShow();	
//	RefreshOledTime() ;                 //OLED更新时间
}

/**************************************************************************************************
* 名    称：  void RTC_NVIC_Config(IntPriority_e ePriority)
* 功能说明：  RTC时钟中断和RTC闹钟中断优先级管理
* 入口参数：
*             @param1 ePriority     IntPriority_e枚举类型，表示RTC中断的中断抢占优先级
*************************************************************************************************/
void RTC_NVIC_Config(IntPriority_e ePriority)
{	
  NVIC_InitType NVIC_InitStructure ;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn ;		                    //RTC中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ePriority ;	  //设置抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0 ;	                  //设置子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE ;		                  //使能该通道中断
	NVIC_Init(&NVIC_InitStructure);		                                    //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
	
	NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn ;		              //RTCAlarm中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ePriority ;	  //设置抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0 ;	                  //设置子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                       //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);		                                    //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器		
}

/**************************************************************************************************
* 名    称：  RunResult RTC_Init(IntPriority_e ePriority)
* 功能说明：  初始化RTC功能块
* 入口参数：
*             @param1 ePriority     IntPriority_e枚举类型，表示RTC中断的中断抢占优先级
* 出口参数：  RunResult: 反映处理结果
* 说    明：  当LSE失效会启用LSI作为RTC时钟，LSI频率漂移大可能会导致走时不准的问题
*************************************************************************************************/
RunResult N32_RTC_Initia(void)
{
    /* RTC date and time default value*/
    RTC_DateAndTimeDefaultVale();
    /* Enable the PWR clock */
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_PWR | RCC_APB1_PERIPH_BKP, ENABLE);
    /* Allow access to RTC */
    PWR_BackupAccessEnable(ENABLE);
    if (USER_WRITE_BKP_DAT1_DATA != BKP_ReadBkpData(BKP_DAT1) )//从指定的后备寄存器中读出数据:读出了与写入的指定数据不相乎
    {
        /* RTC clock source select */
        if(SUCCESS==RTC_CLKSourceConfig(RTC_CLK_SRC_TYPE_HSE128, true, false))//RTC_CLK_SRC_TYPE_LSE
        {
           RTC_PrescalerConfig();
				/* ALRMASS Value(128) = 500ms / (1 / (32768/128)) */
//           RTC_ConfigAlarmSubSecond(RTC_A_ALARM, 128, RTC_SUBS_MASK_SS14_7);//128
           /* Adjust time by values entered by the user on the hyperterminal */
//					RTC_AlarmRegulate(RTC_A_ALARM);
					RTC_DateRegulate();
          RTC_TimeRegulate();
           
           BKP_WriteBkpData(BKP_DAT1, USER_WRITE_BKP_DAT1_DATA);
//           log_info("\r\n RTC Init Success\r\n");
        }
        else
        {
//           log_info("\r\n RTC Init Faile\r\n");
        }
    }
//		EXTI17_RTCAlarm_Configuration(ENABLE);	
		PWR_BackupAccessEnable(DISABLE);//禁止后备寄存器访问
//		Rtc_RegHookCallback(RTC_A_ALARM, Rtc_Sec_Callback) ;     //注册RTC秒中断回调函数
return RUNOK;		
}		

/**************************************************************************************************
* 名    称：  RunResult RTC_Init(IntPriority_e ePriority)
* 功能说明：  初始化RTC功能块
* 入口参数：
*             @param1 ePriority     IntPriority_e枚举类型，表示RTC中断的中断抢占优先级
* 出口参数：  RunResult: 反映处理结果
* 说    明：  当LSE失效会启用LSI作为RTC时钟，LSI频率漂移大可能会导致走时不准的问题
*************************************************************************************************/
//RunResult RTC_Init(IntPriority_e ePriority)
//{
//	//检查是不是第一次配置时钟
//	uint32_t lsTimeOut = 0 ;
//	uCalendar.sCalendar.spacing = 0x20 ; uCalendar.sCalendar.dash1  = uCalendar.sCalendar.dash2  = '-' ; uCalendar.sCalendar.colon1 = uCalendar.sCalendar.colon2 = ':' ;
//	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_PWR | RCC_APB1_PERIPH_BKP, ENABLE ) ;	//RCC_APB1PeriphClockCmd使能PWR和BKP外设时钟 
////  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE) ; //使能闹钟必须开启AFIO时钟
//	PWR_BackupAccessEnable(ENABLE);                	        //使能后备寄存器访问 
//	if (BKP_ReadBkpData(BKP_DAT1) != 0x5050)		    //从指定的后备寄存器中读出数据:读出了与写入的指定数据不相乎
//			{	 			
//						BKP_DeInit();	                              //复位备份区域 
//            SysLog("BKP_DeInit!") ;				
//						RCC_ConfigLse(RCC_LSE_ENABLE);	                //设置外部低速晶振(LSE),使用外设低速晶振
////						RTC_EnterConfigMode() ;                     //RTC允许配置	CNT\ALR\PRL		
////		        RTC_WaitForLastTask();	                    //等待最近一次对RTC寄存器的写操作完成		
//						while ( (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (lsTimeOut < 0x00FFFFFF))	//检查指定的RCC标志位设置与否,等待低速晶振就绪 72M主频下大概等待2.8S
//									{
//											lsTimeOut++; 
//											//Wait_For_Nms(10) ;									  
//									}									
//						if( lsTimeOut < 0x00FFFFF0 )                             //LSE就绪成功
//							{
//									RCC_ConfigRtcClk( RCC_RTCCLK_SRC_LSE ) ;		     //设置RTC时钟(RTCCLK),选择LSE作为RTC时钟  
//									lsTimeOut = 1 ; 										
//							}
//						else
//							{
//									lsTimeOut = 0 ;
//									RCC_EnableLsi(ENABLE) ;                    //开启内部LSI时钟
//									while ( (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) && (lsTimeOut < 0x00FFFFFF))	//检查指定的RCC标志位设置与否,等待低速晶振就绪
//												{
//														lsTimeOut++;
//														//Wait_For_Nms(10) ;
//												}
//									if( lsTimeOut < 0x00FFFFF0 )            //内部LSI时钟准备就绪
//										{
//											RCC_ConfigRtcClk( RCC_RTCCLK_SRC_LSI ) ; //设置RTC时钟(RTCCLK),选择LSI作为RTC时钟 
//											SysErr("") ;                        //LSE失效,LEI作为RTC时钟！
//											lsTimeOut = 0 ; 														
//										}
//									else
//	                 	{
//											SysErr("") ;                        //LSE+LSI均失效！
//											return (RUNERR) ;                   //初始化时钟失败,晶振有问题	 
//										}									
//							}
//					
//						RCC_EnableRtcClk(ENABLE) ;	                      //使能RTC时钟  					
////						RTC_WaitForLastTask();	                      //等待最近一次对RTC寄存器的写操作完成
////						if( lsTimeOut == 0 )
////						  {
////							  RTC_SetPrescaler(40000);                  //设置RTC预分频的值  LSI
////							}
////						else
////						  {
////                RTC_SetPrescaler(32767);                  //设置RTC预分频的值 LSE
////							}					
////						RTC_WaitForLastTask();	                      //等待上次对RTC寄存器写操作完成
////						//RTC_Set((Calendar_u *)"2018-01-21 09:00:00") ;
////            RTC_WaitForLastTask();	                      //等待上次对RTC寄存器写操作完成
////		        RTC_ExitConfigMode();                         //退出配置模式  
////						RTC_WaitForLastTask();	                      //等待上次对RTC寄存器写操作完成						
//						BKP_WriteBkpData( BKP_DAT1, 0x5050 ) ;	//向指定的后备寄存器中写入用户程序数据
//			}
//	else//已经初始化过RTC，期间后备份区域没有断电系统继续计时
//			{
////				//RTC_WaitForSynchro() ;	                        //RTC读操作前等待 
////				RTC_WaitForLastTask() ;	                          //RTC写操作前等待 
//				RTC_ConfigInt( RTC_IT_SEC, ENABLE ) ;	            //使能RTC秒中断
//			}
////	RTC_WaitForLastTask();	                                //等待上次对RTC寄存器写操作完成
//	RTC_ConfigInt(RTC_IT_SEC, ENABLE);		                    //使能RTC秒中断
////	RTC_WaitForLastTask();	                                //等待上次对RTC寄存器写操作完成
//	RTC_ConfigInt(RTC_IT_ALR, ENABLE);		                    //使能闹钟中断
////  RTC_WaitForLastTask();	                                //等待上次对RTC寄存器写操作完成
////	EXTI_InitTypeDef EXTI_InitStructure;                  //如果这只了EXTI线17中断测产生RTC闹钟中断RTCAlarm_IRQHandler，否则如果开启闹钟中断，在RTC全局中断内判断闹钟中断标志位
////	EXTI_ClearITPendingBit( EXTI_Line17 ); 
////	EXTI_InitStructure.EXTI_Line = EXTI_Line17 ;	
////	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt ;  //如果是中断事件，则在这条线上产生一个脉冲，不产生RTC闹钟中断RTCAlarm_IRQHandler
////	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling ;   
////	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
////	EXTI_Init( &EXTI_InitStructure );                     //EXTI中断线17为RTC闹钟中断线
//	PWR_BackupAccessEnable(DISABLE);	                          //禁止后备寄存器访问 
//	RTC_NVIC_Config(ePriority) ;                            //RCT中断分组设置
//	RTC_Get(&uCalendar);                                    //更新uCalendar时间			
//  Rtc_RegHookCallback(RTC_IT_SEC, Rtc_Sec_Callback) ;     //注册RTC秒中断回调函数		
//	return (RUNOK) ;                                        //ok
//}	

/**************************************************************************************************
* 名    称：  RunResult RTC_Get(Calendar_u *getCalendar)
* 功能说明：  设置RTC闹钟时间点
* 入口参数：  
*             @param1 *getCalendar: 指向Calendar_u类型数据的指针           
* 出口参数：  
*             @param RunResult：返回值,返回函数运行结果.
* 说    明：  RTC_Get(&uCalendar) ;
*************************************************************************************************/
//RunResult RTC_Get(Calendar_u *getCalendar)
//{
//	static uint16_t daycnt = 0 ;          //static修饰 保证只有改变天数时才更新年月日
//	u32 timecount  = 0, temp = 0  ; 
//	uint16_t temp1 = 0 ;	
//  vu8 hour, min, sec, w_month, w_date, week;		
//	vu16 w_year;	
//	RTC_WaitForSynchro() ;		       //RTC读操作前等待 
//  timecount = RTC_GetCounter() ;	 
// 	temp = timecount/86400 ;         //得到天数(秒钟数对应的)
//	if( daycnt != temp )             //超过一天了才会影响年月日
//	{	  
//		daycnt = temp;
//		temp1 = 1970;	                 //从1970年开始
//		while( temp >= 365 )
//		{				 
//			if( CheckLeepYear(temp1) )    //是闰年
//			{
//				if( temp >= 366 )
//					  temp -= 366 ;          //闰年的秒钟数
//				else 
//          {
//					  temp1++ ;
//						break ;
//					}  
//			}
//			else  
//				temp -= 365 ;	             //平年 
//			temp1++ ;  
//		}   
//		w_year = temp1 ;               //得到年份
//		getCalendar->sCalendar.w_year[0] = w_year/1000+'0' ;
//		getCalendar->sCalendar.w_year[1] = (w_year%1000)/100+'0' ;
//		getCalendar->sCalendar.w_year[2] = ((w_year%1000)%100)/10 + '0' ;
//		getCalendar->sCalendar.w_year[3] =   w_year%10 + '0' ;
//		temp1 = 0 ;
//		while( temp >= 28 )            //超过了一个月
//		{
//			if( CheckLeepYear(w_year)&&temp1==1 )//当年是不是闰年/2月份
//			{
//				if( temp >= 29 )
//					 temp -= 29 ;            //闰年的秒钟数
//				else
//  				 break ; 
//			}
//			else 
//			{
//				if( temp >= monTable[temp1] )
//					  temp -= monTable[temp1] ;//平年
//				else 
//					  break ;
//			}
//			temp1++ ;  
//		}
//		w_month = temp1+1 ;	             //得到月份
//		w_date = temp+1 ;  	             //得到日期 
//		getCalendar->sCalendar.w_month[0] = w_month/10+'0' ;
//		getCalendar->sCalendar.w_month[1] = w_month%10+'0' ;
//		getCalendar->sCalendar.w_date[0]  = w_date/10+'0'  ;
//		getCalendar->sCalendar.w_date[1]  = w_date%10+'0'  ;
//	}
//	temp = timecount%86400 ;     		   //得到秒钟数   	
//	hour = temp/3600 ;               	 //小时
//	min = (temp%3600)/60 ; 	           //分钟
//	sec = (temp%3600)%60 ; 	           //秒钟	
//	getCalendar->sCalendar.hour[0] =  hour/10+'0' ;
//	getCalendar->sCalendar.hour[1] =  hour%10+'0' ;         	
//	getCalendar->sCalendar.min[0] = min/10+'0' ;
//	getCalendar->sCalendar.min[1] = min%10+'0' ;
//	getCalendar->sCalendar.sec[0] = sec/10+'0' ;
//	getCalendar->sCalendar.sec[1] = sec%10+'0' ;
//	week = RTC_Get_Week( w_year, w_month, w_date ) ;  //获取星期   
//	return (RUNOK) ;
//}	

/**************************************************************************************************
* 名    称：  uint8_t RTC_Set(Calendar_u *setCalendar)
* 功能说明：  设置RTC当前时间
* 入口参数：  
*             @param *setCalendar：Calendar_u类型共用体指针
* 出口参数：  
*             @param uint8_t：返回值:0,成功;其他:错误代码.
* 说    明：  
*************************************************************************************************/
//RunResult RTC_Set(Calendar_u *setCalendar) //
//{
//  u16 t = 0 ;
//	u32 seccount = 0 ; //存储setCalendar日期计算出来的总秒钟数 初始化RTC计数器
//	uint16_t syear = (setCalendar->bytes[0]-'0')*1000+(setCalendar->bytes[1]-'0')*100+(setCalendar->bytes[2]-'0')*10+(setCalendar->bytes[3]-'0') ;
//	uint8_t  smon  = (setCalendar->bytes[5]-'0')*10  +(setCalendar->bytes[6]-'0') ;
//  uint8_t  sday  = (setCalendar->bytes[8]-'0')*10  +(setCalendar->bytes[9]-'0') ;
//  uint8_t	 hour  = (setCalendar->bytes[11]-'0')*10 +(setCalendar->bytes[12]-'0') ;
//  uint8_t  min   = (setCalendar->bytes[14]-'0')*10 +(setCalendar->bytes[15]-'0') ;
//  uint8_t  sec   = (setCalendar->bytes[17]-'0')*10 +(setCalendar->bytes[18]-'0');
//		
//	if( syear<1970 || syear>2099 )
//		  return (RUNERR);	
//	
//	for( t = 1970; t < syear; t++ )    	 //把所有年份的秒钟相加
//	{
//		if(CheckLeepYear(t))
//			 seccount += 31622400;           //闰年的秒钟数
//		else 
//			 seccount += 31536000;			     //平年的秒钟数
//	}
//	smon -= 1;
//	for( t=0; t<smon; t++ )	             //把前面月份的秒钟数相加
//	{
//		seccount += (u32)monTable[t]*86400; //月份秒钟数相加
//		if(CheckLeepYear(syear)&&t==1)
//			 seccount += 86400;              //闰年2月份增加一天的秒钟数	   
//	}
//	seccount += (u32)(sday-1)*86400 ;    //把前面日的秒钟数相加 
//	seccount += (u32)hour*3600 ;         //小时秒钟数
//  seccount += (u32)min*60 ;	           //分钟秒钟数
//	seccount += sec ;                    //最后的秒钟加上去
//	
//	PWR_BackupAccessCmd( ENABLE ) ;	     //使能RTC和后备寄存器访问 
//	RTC_EnterConfigMode() ;              //进入RTC配置模式 	CNT\ALR\PRL	
//	RTC_WaitForLastTask();	             //等待最近一次对RTC寄存器的写操作完成
//	RTC_SetCounter( seccount ) ;	       //设置RTC计数器的值
//	RTC_WaitForLastTask() ;	             //等待最近一次对RTC寄存器的写操作完成  
//	RTC_ExitConfigMode() ;               //退出RTC配置模式    
//	RTC_WaitForLastTask() ;	             //等待最近一次对RTC寄存器的写操作完成  
//	return (RUNOK) ;	    
//}

/**************************************************************************************************
* 名    称：  uint8_t RTC_Alarm_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
* 功能说明：  设置RTC闹钟时间
* 入口参数：  
*             @param *setCalendar：Calendar_u类型共用体指针
* 出口参数：  
*             @param uint8_t：返回值:0,成功;其他:错误代码.
* 说    明：  1970~2099年为合法年份
*************************************************************************************************/
//RunResult RTC_Alarm_Set(Calendar_u *setCalendar)
//{
//  u16 t = 0 ;
//	u32 seccount = 0 ;
//	uint16_t syear = (setCalendar->bytes[0]-'0')*1000+(setCalendar->bytes[1]-'0')*100+(setCalendar->bytes[2]-'0')*10+(setCalendar->bytes[3]-'0') ;
//	uint8_t  smon  = (setCalendar->bytes[5]-'0')*10  +(setCalendar->bytes[6]-'0') ;
//  uint8_t  sday  = (setCalendar->bytes[8]-'0')*10  +(setCalendar->bytes[9]-'0') ;
//  uint8_t	 hour  = (setCalendar->bytes[11]-'0')*10 +(setCalendar->bytes[12]-'0') ;
//  uint8_t  min   = (setCalendar->bytes[14]-'0')*10 +(setCalendar->bytes[15]-'0') ;
//  uint8_t  sec   = (setCalendar->bytes[17]-'0')*10 +(setCalendar->bytes[18]-'0');
//	if( syear<1970 || syear>2099 )
//		  return (RUNERR) ;	   
//	for( t=1970; t<syear; t++ )	//把所有年份的秒钟相加
//		{
//			if(CheckLeepYear(t))
//				 seccount += 31622400 ; //闰年的秒钟数
//			else 
//				 seccount += 31536000 ;	//平年的秒钟数
//		}
//	smon -= 1;
//	for( t=0; t<smon; t++ )	    //把前面月份的秒钟数相加
//		{
//			seccount += (u32)monTable[t]*86400 ;        //月份秒钟数相加
//			if(CheckLeepYear(syear)&& (t==1) )
//				 seccount += 86400 ;                       //闰年2月份增加一天的秒钟数	   
//		}
//	seccount += (u32)(sday-1)*86400 ;              //把前面日期的秒钟数相加 
//	seccount += (u32)hour*3600 ;                   //小时秒钟数
//  seccount += (u32)min*60;	                     //分钟秒钟数
//	seccount +=  sec;                              //最后的秒钟加上去 			    
//		
//	PWR_BackupAccessCmd( ENABLE ) ;	               //使能后备寄存器访问 
//	RTC_EnterConfigMode() ;                        //RTC允许配置	CNT\ALR\PRL	
//	RTC_WaitForLastTask();	                       //等待最近一次对RTC寄存器的写操作完成
//	RTC_SetAlarm(seccount); 
//	RTC_WaitForLastTask();	                       //等待最近一次对RTC寄存器的写操作完成  	
//	RTC_ExitConfigMode();	                         //RTC退出配置	CNT\ALR\PRL	
//  RTC_WaitForLastTask();	                       //等待最近一次对RTC寄存器的写操作完成  
//	PWR_BackupAccessCmd(DISABLE);	                 //禁止后备寄存器访问 

//	return (RUNOK);	    
//}

/**************************************************************************************************
* 名    称：  uint8_t CheckLeepYear(uint16_t year)
* 功能说明：  检查year是否为闰年
* 入口参数：  
*             @param uint16_t year：年份数值
* 出口参数：  
*             @param uint8_t：检查结果
                      @arg 1: year为闰年
*                     @arg 0: year为平年
* 说    明： 
*             月份   1  2  3  4  5  6  7  8  9  10 11 12
*             闰年   31 29 31 30 31 30 31 31 30 31 30 31
*             非闰年 31 28 31 30 31 30 31 31 30 31 30 31
*************************************************************************************************/
uint8_t CheckLeepYear(uint16_t year)
{			  
	if( year%4 == 0 ) //必须能被4整除
	{ 
		if( year%100 == 0 ) 
		{ 
			if( year%400 == 0 )
				 return 1 ; //如果以00结尾,还要能被400整除 	   
			else 
				 return 0 ;   
		}
		else 
			return 1 ;   
	}
	else 
		return 0 ;	
}

/**************************************************************************************************
* 名    称：  Ruint8_t RTC_Get_Week(uint16_t year, uint8_t month, uint8_t day)
* 功能说明：  获取现在是星期几
* 入口参数：  
*             @param1 year: 年分
*             @param2 mon:  月份
*             @param3 day:  日          
* 出口参数：  
*             @param1 uint8_t: 星期几？
*             @param RunResult：返回值,返回函数运行结果.
* 说    明：  输入公历日期得到星期(只允许1901-2099年)
*************************************************************************************************/
uint8_t RTC_Get_Week(uint16_t year, uint8_t month, uint8_t day)
{	
	uint16_t temp2 ;
	uint8_t  yearH, yearL ;
	
	yearH = year/100 ;	
	yearL = year%100 ; 
	if ( yearH>19 )   	// 如果为21世纪,年份数加100  
		   yearL+=100;
	// 所过闰年数只算1900年之后的  
	temp2 = yearL+yearL/4 ;
	temp2 = temp2%7 ; 
	temp2 = temp2+day+weekTable[month-1] ;
	if ( yearL%4==0&&month<3 )
		   temp2--;
	return(temp2%7) ;
}			  
















