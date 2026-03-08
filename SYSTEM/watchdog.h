#ifndef __WATCHDOGx_H
#define __WATCHDOGx_H
#include "n32g45x.h"


/*****************************************
*对外接口函数声明
****************************************/
extern void Watchdog_Init( void ) ;  //STM32硬件看门狗初始化函数
void Watchdog_Feed( void ) ;  //STM32看门狗喂狗接口


#endif
