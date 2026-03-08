#ifndef __PVD_H
#define __PVD_H 			   
#include "sys.h"  
#include "stm32f10x_it.h"

/*****************************************
*对外接口函数声明
****************************************/
extern void Pvd_Init( EXTITrigger_TypeDef exitTrigger, IntPriority_e ePriority) ;//PVD初始化
extern void PWR_PVD_Init(void) ;       //PVD初始化
extern void PVD_IRQHandler(void) ;     //PVD中断处理函数

#endif





























