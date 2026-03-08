#ifndef __DMA_H
#define	__DMA_H	   
//#include "n32g43x_dma.h"
//#include "stm32f10x.h"
//#include "usart.h"
//#include "misc.h"
//#include "sysTick.h"
//#include "Led.h"
#include "n32g45x.h"
#include "n32g45x_it.h"
/****************************************************************************
* 自定义数据类型
****************************************************************************/
typedef enum
{
  DMA1CH1 = 0 ,
	DMA1CH2 = 1 ,
	DMA1CH3 = 2 ,
	DMA1CH4 = 3 ,
	DMA1CH5 = 4 ,
	DMA1CH6 = 5 ,
	DMA1CH7 = 6 ,  //以上为DMA1的7个通道
	DMA1CH8 = 7 ,
	
	DMA2CH1 = 8 ,
	DMA2CH2 = 9 ,
	DMA2CH3 = 10 ,
	DMA2CH4 = 11 ,
	DMA2CH5 = 12 , 
	DMA2CH6 = 13 ,
	DMA2CH7 = 14 ,
	DMA2CH8 = 15 , //以上为DMA2的8个通道
	DMA_CHSUM = 16,//16
}DMACH_e ;

/*DMA中断回调相关配置*/
typedef void (*DMAFP)(void) ;      //定义函数指针类型变量

typedef enum
{
  DMATC =  0 ,    //DMA传输完成中断
	DMATE =  1      //DMA传输错误中断
//  DMAHT =  2 ,  //DMA传输一半中断
//  DMAGL =  3 	  //DMA1全局中断
}DMAIRQTYPE_e ;

typedef enum
{
  DMA_DIR_MTP =  ((uint32_t)0x00000010) ,  //内存到外设
	DMA_DIR_PTM =  ((uint32_t)0x00000000)    //外设到内存 
}DMADIR_e ;                                //DMA数据传输方向

typedef enum
{
  CIRCULAR =  ((uint32_t)0x00000020) ,     //循环模式
	NORMAL   =  ((uint32_t)0x00000000)       //不循环 
}DMAMODE_e ;                               //DMA数据传输是否循环传输

/********************************************************************************
*供外部使用的常变量
*********************************************************************************/
extern DMA_ChannelType* dmaChx[DMA_CHSUM] ;

/********************************************************************************
*对内函数声明
*********************************************************************************/
void Dma1_Hook(uint32_t DMAy_FLAG) ;
void Dma2_Hook(uint32_t DMAy_FLAG) ;
/********************************************************************************
*对外接口函数声明
*********************************************************************************/
extern DMAFP  Dma_RegHookCallback(DMACH_e DMA_CHx, DMAIRQTYPE_e eIrqType, DMAFP pCallback)  ;
extern void   DMA_Config(DMACH_e DMA_CHx, u32 periAddr, u32 memAddr, DMADIR_e tranDire, DMAMODE_e CircMode, IntPriority_e ePriority);//配置DMA1_CHx
extern void   DMA1_Channel4_IRQHandler(void) ;

#endif




