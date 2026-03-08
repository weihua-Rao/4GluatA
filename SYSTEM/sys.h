#ifndef __SYS_H
#define __SYS_H	
#include "n32g45x.h"	 
																	   	
/********************************************************************************
*对外接口函数声明
*********************************************************************************/
extern void WFI_SET(void);		  //执行WFI指令
extern void INTX_DISABLE(void); //关闭所有中断
extern void INTX_ENABLE(void);	//开启所有中断
extern void MSR_MSP(u32 addr);	//设置堆栈地址

#endif
