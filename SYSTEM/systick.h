#ifndef __SYSTICK_H
#define __SYSTICK_H 

//#include "sys.h"  
#include "n32g45x_it.h" 

/*****************************************
*供外部使用的常变量
****************************************/
extern uint32_t tickCounter ;                          //定义一个全局变量，在systick中断中当做累加器使用。
/*****************************************
*对外接口函数声明
****************************************/
extern void SysTick_Init(void) ;                       //STM32滴答定时器初始化
extern void Delay_Us(u32 nus) ;                        //us延时函数
extern void Delay_Ms(u32 nms) ;                        //ms延时函数
extern void Delay_Ms_StopScheduler(u32 nms) ;          //ms延时函数，关闭freertos任务轮询。
#endif





























