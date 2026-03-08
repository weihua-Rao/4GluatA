#ifndef __iox_H
#define __iox_H
#include "n32g45x_gpio.h"
#include "n32g45x_it.h"
#include "n32g45x_exti.h" 
#include <stdio.h>

/*****************************************
*自定义枚举类型
****************************************/
typedef enum
{
  PA = 0, PB = 1, PC = 2, PD = 3,	PE = 4,	PF = 5,	PG = 6, PSUM = 7
}PORT_e ;                                 //STM32F103VET6所有的IO端口枚举

typedef enum
{
  PIN0 = 0, PIN1 = 1, PIN2 = 2, PIN3 = 3, PIN4 = 4, PIN5 = 5, PIN6 = 6, PIN7 = 7,
	PIN8 = 8, PIN9 = 9, PIN10 = 10, PIN11 = 11, PIN12 = 12, PIN13 = 13, PIN14 = 14, PIN15 = 15, PINSUM = 16
}PIN_e ;                                  //STM32F103VET6所有的IO端口下的IO引脚枚举

/*****************************************
*中断事件回调函数相关配置
****************************************/
typedef void (*IOFP)(void) ;              //定义函数指针类型变量
												
/*****************************************
*内部函数声明
****************************************/
void  Io_Hook(uint32_t extiLine) ;        //IO外部中断钩子函数

/*****************************************
*对外接口函数声明
****************************************/
extern void Gpio_Init(PORT_e ePortx, PIN_e ePinx, GPIO_ModeType gpioMode) ;   //GPIO初始化 GPIOMode_TypeDef  EXTITrigger_TypeDef
extern void IoExti_Init(PORT_e ePortx, PIN_e ePinx, GPIO_ModeType gpioMode, EXTI_TriggerType exitTrigger, IntPriority_e ePriority) ; //GPIOMode_TypeDef外部中断初始化
extern void Io_Reverse(PORT_e ePortx, PIN_e ePinx) ;                             //IO电平翻转函数接口
extern IOFP Io_RegHookCallback(PIN_e ePinx, IOFP pCallback)  ;                   //IO外部中断回调函数注册函数
extern void EXTI0_IRQHandler(void)   ;                                           //外部中断线0中断处理函数
extern void EXTI1_IRQHandler(void)   ;                                           //外部中断线1中断处理函数
extern void EXTI2_IRQHandler(void)   ;                                           //外部中断线2中断处理函数
extern void EXTI3_IRQHandler(void)   ;                                           //外部中断线3中断处理函数
extern void EXTI4_IRQHandler(void)   ;                                           //外部中断线4中断处理函数
extern void EXTI15_10_IRQHandler(void) ;                                         //外部中断线10~15中断处理函数
extern void EXTI9_5_IRQHandler(void) ;                                           //外部中断线5~9中断处理函数

#endif

