#ifndef __TIMERx_H
#define __TIMERx_H
#include "stm32f10x_it.h" 
#include "stm32f10x_tim.h"

/*****************************************
*自定义枚举类型
****************************************/
typedef enum
{
	TIMER2 = 0 ,
	TIMER3 = 1 ,
	TIMER4 = 2 ,
	TIMER5 = 3 ,
	TIMER6 = 4,
	TIMER7 = 5 ,
	TIMERSUM = 6 
}TIMER_e ;                        //硬件定时器枚举

/*****************************************
*中断事件回调函数相关配置
****************************************/
typedef void (*TIMERFP)(void) ;   //定义函数指针类型变量int (*p_callback)()


/********************************************************************************
*内部函数声明
*********************************************************************************/
void Timer_Hook(TIMER_e eTIMERn) ;//定时器中断处理函数 钩子函数
	
/********************************************************************************
*对外接口函数声明
*********************************************************************************/
extern void Timerx_Init(TIMER_e eTIMERn, u16 timeMs, IntPriority_e ePriority, FunctionalState NewState); //定时器初始化函数
extern TIMERFP  Timer_RegHookCallback(TIMER_e eTIMERn, TIMERFP pCallback) ;  //定时器中断回调函数注册函数
extern void Timerx_Reset(TIMER_e eTIMERn) ;                                  //定时器复位函数
extern void Timerx_Open(TIMER_e eTIMERn) ;                                   //定时器开启定时函数
extern void Timerx_Close(TIMER_e eTIMERn) ;                                  //定时器关闭定时函数  
extern void TIM2_IRQHandler(void)  ;																				 //timer2中断处理函数
extern void TIM3_IRQHandler(void)  ;																				 //timer3中断处理函数
extern void TIM4_IRQHandler(void)  ;																				 //timer4中断处理函数
extern void TIM5_IRQHandler(void)  ;																				 //timer5中断处理函数
extern void TIM6_IRQHandler(void)  ;																				 //timer6中断处理函数
extern void TIM7_IRQHandler(void)  ;																				 //timer7中断处理函数

#endif
