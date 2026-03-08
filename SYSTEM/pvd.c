#include "pvd.h"		 
#include "sysport.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"

/********************************************************************************
  * @file    pvd.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   pvd相关驱动
  ******************************************************************************
*******************************************************************************/

/*****************************************
*供内部使用的常变量
****************************************/
							    
  
/**************************************************************************************************
* 名    称：  void Delay_Us(u32 nus)
* 功    能：  延时nus
* 入口参数：
*           @param nus    要延时的us数，没有系统调度
* 说    明：  nus:0~59652323(最大值即2^32/fac_us@fac_us=72)	    59652323
Delay_Ms_StopScheduler
*************************************************************************************************/ 								   
void Pvd_Init( EXTITrigger_TypeDef exitTrigger, IntPriority_e ePriority)
{
		/*开启EXIT功能需要 开启AFIO时钟且初始化 EXIT和NVIC*/
		EXTI_InitTypeDef EXTI_InitStructure ;                //EXIT参数 结构体定义	
		NVIC_InitTypeDef NVIC_InitStructure ;	               //NVIC参数 结构体定义
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); //GPIO EXIT功能需要开启AFIO时钟

		EXTI_InitStructure.EXTI_Line    = EXTI_Line16 ;	           //中断引脚
		EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt ;	   //中断模式
		EXTI_InitStructure.EXTI_Trigger = exitTrigger ;            //IO外部中断触发方式
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init( &EXTI_InitStructure );					
		/*清中断标志位后再开中断*/		
		EXTI_ClearITPendingBit( EXTI_Line16); 

		NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn ;			         
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ePriority ;	//抢占优先级配置 
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0 ;					        //freertos无子优先级
		NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE ;						        //失能外部中断通道
		NVIC_Init(&NVIC_InitStructure);                                     //中断NVIC配置
		NVIC_EnableIRQ(PVD_IRQn) ;

		PWR_PVDLevelConfig(PWR_PVDLevel_2V8) ;
		PWR_PVDCmd(ENABLE) ;
}

//void PVD_IRQHandler(void)
//{
//if( EXTI_GetFlagStatus(EXTI_Line16) != RESET )
//	{
//	EXTI_ClearITPendingBit(EXTI_Line16);  
//	WriteLogToFlash("Pvd Irq!") ;
//  SysErr("Pvd Irq!") ;
//	}
//}

void PWR_PVD_Init(void)
{ 
		NVIC_InitTypeDef NVIC_InitStructure; 
		EXTI_InitTypeDef EXTI_InitStructure; 
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//使能PWR时钟 
		NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn; //使能PVD所在的外部中断通道 
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //子优先级0
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道 
		NVIC_Init(&NVIC_InitStructure); EXTI_StructInit(&EXTI_InitStructure);
	
		EXTI_InitStructure.EXTI_Line = EXTI_Line16; //PVD连接到中断线16上 
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; //使用中断模式 
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //电压低于阀值时产生中断 
		EXTI_InitStructure.EXTI_LineCmd = ENABLE; //使能中断线 
		EXTI_Init(&EXTI_InitStructure); //初始 
		PWR_PVDLevelConfig(PWR_PVDLevel_2V8);//设定监控阀值 
		PWR_PVDCmd(ENABLE);//使能PVD 
} 
u16  num = 0 ;
void PVD_IRQHandler(void) 
{ 
		EXTI_ClearITPendingBit(EXTI_Line16);//清中断 
//		num = BKP_ReadBackupRegister(BKP_DR10); 
//		num++; //用户添加紧急处理代码处 
//		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);//使能PWR和BKP外设时钟 
//		PWR_BackupAccessCmd(ENABLE);//使能后备寄存器访问 
//		BKP_WriteBackupRegister(BKP_DR1, num);//启动界面 
}
			
//uint16_t power_time = 0 ;
//void PVD_IRQHandler(void) 
//{	
//	EXTI_ClearITPendingBit(EXTI_Line16);//清中断 
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR , ENABLE);//使能PWR和BKP外设时钟 
//	PWR_BackupAccessCmd(ENABLE);                        //使能后备寄存器访问 
//	while(1) 
//		{ 
//			power_time++; 
//		  BKP_WriteBackupRegister(BKP_DR14, power_time) ;			
//		} 
//}











