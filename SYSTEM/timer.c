#include "timer.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include "sysport.h" 

/********************************************************************************
  * @file    timer.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   stm32硬件定时器相关驱动
  ******************************************************************************
	* @注意：	定时器的定时范围为0.1S~65S,如果需要精度更高的定时或者需要更大的延时
	*         时间需要自己修改Timerx_Init（）函数中的分频系数
*******************************************************************************/

/****************************************************************************
* TIMER相关配置
****************************************************************************/
TIM_TypeDef *  TIMx_TIMER[TIMERSUM]     = {TIM2, TIM3, TIM4, TIM5, TIM6, TIM7} ;		 //(TIM_TypeDef *)类型的数据，映射timer2~timer7																			 
const uint32_t TIMx_RCC[TIMERSUM]       = {RCC_APB1Periph_TIM2, 
                                           RCC_APB1Periph_TIM3, 
																					 RCC_APB1Periph_TIM4, 
																					 RCC_APB1Periph_TIM5, 
																					 RCC_APB1Periph_TIM6, 
																					 RCC_APB1Periph_TIM7}  ;                   //timer2~timer7的时钟线
																					 
const uint8_t  TIMx_IRQ[TIMERSUM]       = {TIM2_IRQn, TIM3_IRQn, TIM4_IRQn, TIM5_IRQn, TIM6_IRQn, TIM7_IRQn} ;//timer2~timer7的中断线

/*****************************************
*中断事件回调函数相关配置
****************************************/
TIMERFP  timerFp[TIMERSUM] = {NULL} ;                                                //TIM2~TIM7 定义6个TIMER中断线对应的回调函数指针

/**************************************************************************************************
* 名    称：TIMERFP  Timer_RegHookCallback(TIMER_e eTIMERn, TIMERFP pCallback) 
* 功能说明：Timer中断回调函数-注册函数，Timer驱动中定义了TIMERFP函数指针类型，并定义了timerFp6个中断指针变量
*           Timer_RegHookCallback函数就是将回调函数地址传递给ioFp指针变量
* 入口参数：
*           @param1 pCallback：   TIMERFP类型函数指针
*           @param2 eTIMERn       TIMER_e枚举类型数据
* 出口参数：
*           @param1 pCallback      TIMER_e类型函数指针
* 调用方法：外部调用
*************************************************************************************************/ 
TIMERFP  Timer_RegHookCallback(TIMER_e eTIMERn, TIMERFP pCallback) 
{
	if( timerFp[eTIMERn] == NULL )
		  timerFp[eTIMERn] = pCallback ;
	else
		 SysErr("") ; //Timer Callback repeat reg!
  return 	pCallback ;
} 

/**************************************************************************************************
* 名    称：void Timer_Hook(TIMER_e eTIMERn)
* 功能说明：Timer中断内调用的钩子函数（执行到中断会把相应的回调函数勾出来运行，嘿嘿嘿。。。
*           当然不同的中断eTIMERn会勾出不同的回调函数timerFp[TIMERSUM]）
* 入口参数：
*           @param2 eTIMERn       TIMER_e枚举类型数据
*************************************************************************************************/ 
void Timer_Hook(TIMER_e eTIMERn)
 {
   if( timerFp[eTIMERn] != NULL )
	 {
	     timerFp[eTIMERn]() ;
	 }																											
 }
 
/**************************************************************************************************
* 名    称：  void Timerx_Init(TIMER_e eTIMERn, u16 timeMs, IntPriority_e ePriority, FunctionalState NewState)
* 外部引用：  ErrorLogPrintf
* 功    能：  基础定时器初始化
* 入口参数：
*           @param1  eTIMERn    端口
*                      @arg TIMER_e 枚举类型数据
*           @param2  timeMs     定时时长，单位ms
*           @param3  ePriority     IntPriority_e枚举类型，表示IO端口中断抢占优先级
*           @param4  NewState:  是否使能定时器
*                      @arg ENABLE     :打开定时器
*                      @arg DISABLE    :关闭定时器
* 说    明：定时时间计算:  定时器时钟为72M，分频系数为64799，
*                          所以定时器的频率为72M/(64799+1)，自动重装载为TimerPeriod，
*                          那么定时器定时时间就是:(TimerPeriod+1)/(72M/((7200-1)+1))S
*                           T = (TimerPeriod+1)*((7200-1)+1)/72000  MS = (TimerPeriod+1)*0.1 MS
*														Tmax = (65536+1)*0.1 = 6553 MS 						 
************************************************************************************************************/
void Timerx_Init(TIMER_e eTIMERn, u16 timeMs, IntPriority_e ePriority, FunctionalState NewState)
{
	  if( timeMs > 6553 )
		  {
				timeMs = 6553 ;
				SysErr("") ;
			}
    RCC_APB1PeriphClockCmd(TIMx_RCC[eTIMERn], ENABLE) ;
	
	  NVIC_InitTypeDef NVIC_InitStructure ;
		NVIC_InitStructure.NVIC_IRQChannel = TIMx_IRQ[eTIMERn];
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ePriority;	     //抢占优先级preemptionPriority
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			      //子优先级0  
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				        //使能
		NVIC_Init(&NVIC_InitStructure);
	
	  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure ; 	
		TIM_TimeBaseStructure.TIM_Prescaler = (7200-1) ;				      //预分频系数为7200-1，这样计数器时钟为72MHz/7200 = 10kHz
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;			//设置时钟分割 TIM_CKD_DIV1=0x0000,不分割
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up ; 	//设置计数器模式为向上计数模式
		TIM_TimeBaseStructure.TIM_Period = timeMs/0.1-1 ;		          //设置计数溢出大小（注意最大值65536,）
		TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;	
		TIM_TimeBaseInit(TIMx_TIMER[eTIMERn],&TIM_TimeBaseStructure);	//将配置应用到TIM7中	
		//TIM_UpdateRequestConfig( TIMx_TIMER[eTIMERn], TIM_UpdateSource_Regular);
		TIM_ITConfig(TIMx_TIMER[eTIMERn], TIM_IT_Update,ENABLE);			//使能中断			
	  TIM_Cmd(TIMx_TIMER[eTIMERn], NewState);
		TIM_ClearITPendingBit(TIMx_TIMER[eTIMERn] , TIM_FLAG_Update) ;//使能定时器中断前先清空中断标志位
		//NVIC_EnableIRQ(TIMx_IRQ[eTIMERn]) ;
}

/**************************************************************************************************
* 名    称：  void Timerx_Reset(TIMER_e eTIMERn)
* 功    能：  定时器eTIMERn计数值清零
* 入口参数：
*           @param  eTIMERn    端口
*                      @arg  TIMER_e枚举类型数据
***************************************************************************************************/
void Timerx_Reset(TIMER_e eTIMERn)
{
	TIM_SetCounter( TIMx_TIMER[eTIMERn], 0x0000 ) ;  //TIM7重新计数
}

/**************************************************************************************************
* 名    称：  void Timerx_Open(TIMER_e eTIMERn)
* 功    能：  打开定时器eTIMERn,且清零计数器
* 入口参数：
*           @param  eTIMERn    端口
*                      @arg  TIMER_e枚举类型数据
***************************************************************************************************/
void Timerx_Open(TIMER_e eTIMERn)
{
	TIM_SetCounter( TIMx_TIMER[eTIMERn], 0x0000 ) ;  //TIM7重新计数
	TIM_Cmd(TIMx_TIMER[eTIMERn], ENABLE);
}

/**************************************************************************************************
* 名    称：  void Timerx_Close(TIMER_e eTIMERn)
* 功    能：  关闭定时器eTIMERn
* 入口参数：
*           @param  eTIMERn    端口
*                      @arg  TIMER_e枚举类型数据
************************************************************************************************************/
void Timerx_Close(TIMER_e eTIMERn)
{
	TIM_Cmd(TIMx_TIMER[eTIMERn], DISABLE) ;
	//TIM_SetCounter( TIM6,0x0000 ) ;
}

void TIM2_IRQHandler(void) 
{ 
	if(TIM_GetITStatus(TIM2, TIM_IT_Update)== SET)
	{	
	  TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update) ;
		SysLog("TIM2_IRQ!") ;
		Timer_Hook(TIMER2) ; //Timer2中断回调函数
	}
}

void TIM3_IRQHandler(void) 
{ 
	if(TIM_GetITStatus(TIM3, TIM_IT_Update)== SET)
	{	
	  TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update) ;
		SysLog("TIM3_IRQ!") ;
		Timer_Hook(TIMER3) ; //Timer3中断回调函数
	}
}

void TIM4_IRQHandler(void) 
{ 
	if(TIM_GetITStatus(TIM4, TIM_IT_Update)== SET)
	{	
	  TIM_ClearITPendingBit(TIM4 , TIM_FLAG_Update) ;
		SysLog("TIM4_IRQ!") ;
		Timer_Hook(TIMER4) ; //Timer4中断回调函数
	}
}

void TIM5_IRQHandler(void) 
{ 
	if(TIM_GetITStatus(TIM5, TIM_IT_Update)== SET)
	{	
	  TIM_ClearITPendingBit(TIM5 , TIM_FLAG_Update) ;
		SysLog("TIM5_IRQ!") ;
		Timer_Hook(TIMER5) ; //Timer5中断回调函数
	}
} 

void TIM6_IRQHandler(void) 
{ 
	if(TIM_GetITStatus(TIM6, TIM_IT_Update)== SET)
	{	
	  TIM_ClearITPendingBit(TIM6 , TIM_FLAG_Update) ;
		SysLog("TIM6_IRQ!") ;
		Timer_Hook(TIMER6) ; //Timer6中断回调函数
	}
}

void TIM7_IRQHandler(void) 
{ 
	if(TIM_GetITStatus(TIM7, TIM_IT_Update)== SET)
	{	
	  TIM_ClearITPendingBit(TIM7 , TIM_FLAG_Update) ;
		Timer_Hook(TIMER7) ;
	}
}

