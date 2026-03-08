#include "systick.h"	
#include "sys.h"  
#include "sysport.h"
#include "watchdog.h"
/*********FreeRTOS层外部调用******** */
#include "FreeRTOS.h"					//FreeRTOS使用	
#include "task.h"
#include "usart.h" 
/*****************************************/

/********************************************************************************
  * @file    systick.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   STM32系统滴答定时器初始化、并为OS提供时钟基准
  ******************************************************************************
	* 说明：系统滴答定时器10ms中断一次，所有在延时小于10ms时不需要调用FreeRTOS延时函数。
*******************************************************************************/

/*****************************************
*从外部引用的函数或常变量
****************************************/
extern void xPortSysTickHandler(void);  //FreeRTOS中用于系统节拍的处理函数  	

/*****************************************
*供内部使用的常变量
****************************************/
static u8  facUs=0;							  //us延时倍乘数			   
static u16 facMs=0;							  //ms延时倍乘数,在FreeRTOS下,代表每个节拍的ms数

/*****************************************
*供外部使用的常变量
****************************************/
uint32_t tickCounter = 0 ;                          //定义一个全局变量，在systick中断中当做累加器使用。

/**************************************************************************************************
* 名    称：  void SysTick_Handler(void)
* 外部引用：  xPortSysTickHandler
* 功    能：  系统滴答定时器中断处理函数
* 说    明：  SysTick_Init中设置每1/configTICK_RATE_HZ秒中断一次  	FreeRTOS中configTICK_RATE_HZ = 100，
*             所以SysTick 10ms中断一次
  *************************************************************************************************/
void SysTick_Handler(void)
{	
	if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)//系统已经运行
		{
			xPortSysTickHandler();
      if( tickCounter >= 0xFFFFFFFE)	//累加器溢出清零
			  {
          tickCounter = 0 ;
				}				
			tickCounter++ ;                 //累加器累加
		}
	else  //系统如果没有开始任务调度，那么在Systick中断中喂狗，否则在任务中喂狗
  	{
		  Watchdog_Feed() ;  //看门狗	喂狗
//			IWDG_ReloadKey(); 		   
		}
}

/**************************************************************************************************
* 名    称：  void SysTick_Init(void)
* 功    能：  初始化系统滴答定时器
* 说    明：  SYSTICK的时钟固定为AHB时钟，基础例程里面SYSTICK时钟频率为AHB/8
*             这里为了兼容FreeRTOS，所以将SYSTICK的时钟频率改为AHB的频率！
*             SYSCLK:系统时钟频率
*             每1/configTICK_RATE_HZ秒中断一次  	FreeRTOS中configTICK_RATE_HZ = 100，
*             所以SysTick 10ms中断一次
  *************************************************************************************************/
void SysTick_Init(void)
{
	u32 reload;
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);    //选择外部时钟  HCLK
	facUs  =  SystemCoreClock/1000000;				          //不论是否使用OS,fac_us都需要使用,1us计数器累加的次数
	reload =  SystemCoreClock/1000000;			          	//每秒钟的计数次数 单位为M  
	reload *= 1000000/configTICK_RATE_HZ;			          //根据configTICK_RATE_HZ设定溢出时间
												                              //reload为24位寄存器,最大值:16777216,在72M下,约合0.233s左右	
	facMs = 1000/configTICK_RATE_HZ;				            //代表OS可以延时的最少单位	   
  //SCB->SHP[11]=2;//设置SYSTICK的优先级为1，注意SYSTICK属于系统异常，所以他的优先级在SCB里设置。
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;       	  //开启SYSTICK中断
	SysTick->LOAD  = reload; 						                //每1/configTICK_RATE_HZ秒中断一次	
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;   	      //开启SYSTICK    
}								    
  
/**************************************************************************************************
* 名    称：  void Delay_Us(u32 nus)
* 功    能：  延时nus
* 入口参数：
*           @param nus    要延时的us数，没有系统调度
* 说    明：  nus:0~59652323(最大值即2^32/fac_us@fac_us=72)	    59652323
Delay_Ms_StopScheduler
  *************************************************************************************************/ 								   
void Delay_Us(u32 nus)
{	
  if( nus > 59652323 )	
	  {
		  nus = 59652323 ;
			SysErr("") ;
		}
	u32 ticks;
	u32 told, tnow, tcnt = 0 ;
	u32 reload = SysTick->LOAD;				  //LOAD的值	    	 
	ticks = nus*facUs; 						      //需要的节拍数 
	told  = SysTick->VAL;        				//刚进入时的计数器值
	while(1)
	{
		tnow = SysTick->VAL;	
		if( tnow!=told )
		{	    
			if( tnow<told )
			   tcnt+=told-tnow ;	         //这里注意一下SYSTICK是一个递减的计数器就可以了.
			else 
			   tcnt+=reload-tnow+told ;	    
			told = tnow;
			if( tcnt>=ticks )
			   break;			                 //时间超过/等于要延迟的时间,则退出.
		}  
	}	
// Watchdog_Feed() ;                                 //看门狗喂狗	
}  

/**************************************************************************************************
* 名    称：  void Delay_Ms(u32 nms)
* 外部引用：  void vTaskDelay( const TickType_t xTicksToDelay )
* 功    能：  延时nms  在任务调度器没有启用时nms范围为0~59652 
* 入口参数：
*           @param nms    要延时的ms数
* 说    明： 在任务调度器没有启用时nms范围为0~59652 
  *************************************************************************************************/ 
void Delay_Ms(u32 nms)
{	
	Watchdog_Feed() ;
	if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)//FreeRTOS系统已经运行
		{		
			if( nms>=facMs )						    //延时的时间大于OS的最少时间周期 
				{ 
					vTaskDelay(nms/facMs) ;   	//FreeRTOS延时
				}
			nms %= facMs ;						      //OS已经无法提供这么小的延时了,采用普通方式延时    
		}
	Delay_Us((u32)(nms*1000)) ;				  //系统没有运行直接调用us延时，系统开始运行后延时余数采用普通方式延时
}

/**************************************************************************************************
* 名    称：  void Delay_Ms_StopScheduler(u32 nms)
* 功    能：  延时nms nms范围为0~59652 
* 入口参数：
*             @param nms    要延时的ms数
* 说    明： 延时nms,不会引起任务调度 
  *************************************************************************************************/ 
void Delay_Ms_StopScheduler(u32 nms)  //中断系统任务调度
{
	u32 i ;
	Watchdog_Feed() ;
	for(i=0; i<nms; i++) 
	    Delay_Us(1000) ;
}







































