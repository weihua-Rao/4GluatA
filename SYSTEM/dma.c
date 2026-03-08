//#include "dma.h" 
#include "n32g45x_dma.h"
#include "main.h" 
/********************************************************************************
  * @file    dma.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.1.0
  * @date    26-Dec-2018
  * @brief   提供DMA相关驱动
  ******************************************************************************
	*	DMA1串口通道一览表
				USART1_TX  通道4
				USART1_RX  通道5
				USART2_TX  通道7
				USART2_RX  通道6
				USART3_TX  通道2
				USART3_RX  通道3
	*	DMA2串口通道一览表
				UART4_TX   通道5
				UART4_RX   通道3
		 1、串口1DMA发送TX，用DMA1的通道4 。
					 1、开启DMA1时钟，RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE) 。
					 2、DMA_Init函数配置DMA，可以先不开辟缓冲区和内部RAM地址，当有数据要TX的时候再确定RAM地址和发送缓冲区大小。
					 3、DMA_ITConfig函数可以配置DMA相关中断
					 4、如果DMA开启的相关发送状态的中断，需要用NVIC_Init函数配置优先级 。
					 5、USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //使能串口1的DMA发送   
					 6、调用DMA_TxEnable函数，开辟发送缓冲区，并实现串口1数据TX，同时DMA发送完成会产生DMA TC中断。
		 2、串口1DMA接收RX，用DMA1的通道5 。
					 1、开启DMA1时钟，RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE) 。
					 2、DMA_Init函数配置DMA，需要先开辟串口1DMA接收缓冲区，和设定串口1DMA接收缓冲区大小。
					 3、DMA_ITConfig函数可以配置DMA相关中断
					 4、如果DMA开启的相关接收状态的中断，需要用NVIC_Init函数配置优先级 。
					 5、USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE); //使能串口1的DMA接收
					 6、DMA_RX_Enable(DMA1_Channel5, (u32)&ReceBuff, RECE_BUF_SIZE ) ;	
*******************************************************************************/


//DMA_Channel_TypeDef* dmaChx[DMA_CHSUM] = {DMA1_Channel1, DMA1_Channel2, DMA1_Channel3, DMA1_Channel4, DMA1_Channel5,
//                                          DMA1_Channel6, DMA1_Channel7, DMA2_Channel1, DMA2_Channel2, DMA2_Channel3, 
//                                          DMA2_Channel4, DMA2_Channel5} ; //DMA1和DMA2共计12个通道
DMA_ChannelType* dmaChx[DMA_CHSUM] = {DMA1_CH1, DMA1_CH2, DMA1_CH3, DMA1_CH4, DMA1_CH5, DMA1_CH6, DMA1_CH7,DMA1_CH8,
                                      DMA2_CH1, DMA2_CH2, DMA2_CH3, DMA2_CH4, DMA2_CH5, DMA2_CH6, DMA2_CH7,DMA2_CH8} ; //DMA1和DMA2共计16个通道
//const uint32_t  dmaIRQChx[DMA_CHSUM]   = {DMA1_Channel1_IRQn, DMA1_Channel2_IRQn, DMA1_Channel3_IRQn, DMA1_Channel4_IRQn, 
//	                                        DMA1_Channel5_IRQn, DMA1_Channel6_IRQn, DMA1_Channel7_IRQn, DMA2_Channel1_IRQn,
//                                          DMA2_Channel2_IRQn, DMA2_Channel3_IRQn, DMA2_Channel4_5_IRQn,  DMA2_Channel4_5_IRQn} ; //DMA1和DMA2共计12个通道的global Interrupt 
const uint32_t  dmaIRQChx[DMA_CHSUM]   = {DMA1_Channel1_IRQn, DMA1_Channel2_IRQn, DMA1_Channel3_IRQn, DMA1_Channel4_IRQn, 
	                                        DMA1_Channel5_IRQn, DMA1_Channel6_IRQn, DMA1_Channel7_IRQn, DMA1_Channel8_IRQn,
																					DMA2_Channel1_IRQn, DMA2_Channel2_IRQn, DMA2_Channel3_IRQn, DMA2_Channel4_IRQn, 
	                                        DMA2_Channel5_IRQn, DMA2_Channel6_IRQn, DMA2_Channel7_IRQn, DMA2_Channel8_IRQn,
																					} ; //  DMA1和DMA2共计16个通道的global Interrupt 

/*DMA中断回调相关参数*/
DMAFP  dmaTcFp[DMA_CHSUM] = {NULL} ; //DMA1_CH1~DMA1_CH7\ DMA2_CH1~DMA2_CH5 定义12个DMA通道“TC中断（传输完成）”的回调函数指针
DMAFP  dmaTeFp[DMA_CHSUM] = {NULL} ; //DMA1_CH1~DMA1_CH7\ DMA2_CH1~DMA2_CH5 定义12个DMA通道“TE中断（传输错误）”的回调函数指针	

/**************************************************************************************************
* 名    称：DMAFP  Dma_RegHookCallback(DMACH_e DMA_CHx, DMAIRQTYPE_e eIrqType, DMAFP pCallback)
* 功能说明：DMA中断(TC\TE\HT)回调函数-注册函数。 根据参数eIrqType，区分注册的回调函数是TC\TE中断回调。
* 入口参数：
*           @param1 eIrqType     DMAIRQTYPE_e枚举类型数据 
*           @param2 pCallback    DMAFP类型函数指针
*           @param3 DMA_CHx      DMACH_e枚举类型数据
* 出口参数：
*           @param1 pCallback    DMAFP类型函数指针
* 调用方法：外部调用
*************************************************************************************************/ 
DMAFP  Dma_RegHookCallback(DMACH_e DMA_CHx, DMAIRQTYPE_e eIrqType, DMAFP pCallback) 
{
	if( DMATC == eIrqType)
		{ 
			if( dmaTcFp[DMA_CHx] == NULL )
					dmaTcFp[DMA_CHx] = pCallback ;
			else
					SysErr("DmaTc Callback repeat reg!") ;
		}
	else if( DMATE == eIrqType)
		{ 
			if( dmaTeFp[DMA_CHx] == NULL )
					dmaTeFp[DMA_CHx] = pCallback ;
			else
					SysErr("DmaTe Callback repeat reg!") ;
		}
	else
	  {
		  SysErr("DmaTe Callback \"eIrqType\" error!") ;
		}

  return 	pCallback ;
} 

/**************************************************************************************************
* 名    称：void Io_Hook(uint32_t DMAy_FLAG)
* 功能说明：DMA中断内调用的钩子函数（执行到中断会把相应的回调函数勾出来运行，嘿嘿嘿。。。
*           当然不同的中断DMAy_FLAG会勾出不同的回调函数）
* 入口参数：
*           @param1 DMAy_FLAG     中断标志，用以区分是哪个DMA的哪一种中断类型
*************************************************************************************************/
/**DMA1**/
void Dma1_Hook(uint32_t DMAy_FLAG)
{
		switch(DMAy_FLAG)	 
		{
				case DMA1_FLAG_TC1: if( dmaTcFp[0] != NULL) dmaTcFp[0](); break ;  //                                 
				case DMA1_FLAG_TE1: if( dmaTeFp[0] != NULL) dmaTeFp[0](); break ;   //                

				case DMA1_FLAG_TC2: if( dmaTcFp[1] != NULL) dmaTcFp[1](); break ;                            
				case DMA1_FLAG_TE2: if( dmaTeFp[1] != NULL) dmaTeFp[1](); break ;     
					 
				case DMA1_FLAG_TC3: if( dmaTcFp[2] != NULL) dmaTcFp[2](); break ;                                    
				case DMA1_FLAG_TE3: if( dmaTeFp[2] != NULL) dmaTeFp[2](); break ;            

				case DMA1_FLAG_TC4: if( dmaTcFp[3] != NULL) dmaTcFp[3](); break ;                                    
				case DMA1_FLAG_TE4: if( dmaTeFp[3] != NULL) dmaTeFp[3](); break ;                      

				case DMA1_FLAG_TC5: if( dmaTcFp[4] != NULL) dmaTcFp[4](); break ;                          
				case DMA1_FLAG_TE5: if( dmaTeFp[4] != NULL) dmaTeFp[4](); break ;                 

				case DMA1_FLAG_TC6: if( dmaTcFp[5] != NULL) dmaTcFp[5](); break ;                                
				case DMA1_FLAG_TE6: if( dmaTeFp[5] != NULL) dmaTeFp[5](); break ;                  

				case DMA1_FLAG_TC7: if( dmaTcFp[6] != NULL) dmaTcFp[6](); break ;                                
				case DMA1_FLAG_TE7: if( dmaTeFp[6] != NULL) dmaTeFp[6](); break ;                     

				case DMA1_FLAG_TC8: if( dmaTcFp[7] != NULL) dmaTcFp[7](); break ;                                      
				case DMA1_FLAG_TE8: if( dmaTeFp[7] != NULL) dmaTeFp[7](); break ;                     

			  default: break ;
		}
}
/**DMA2**/
void Dma2_Hook(uint32_t DMAy_FLAG)
{
		switch(DMAy_FLAG)	 
		{

				case DMA2_FLAG_TC1: if( dmaTcFp[8] != NULL) dmaTcFp[8](); break ;  //                                 
				case DMA2_FLAG_TE1: if( dmaTeFp[8] != NULL) dmaTeFp[8](); break ;   //                

				case DMA2_FLAG_TC2: if( dmaTcFp[9] != NULL) dmaTcFp[9](); break ;                            
				case DMA2_FLAG_TE2: if( dmaTeFp[9] != NULL) dmaTeFp[9](); break ;     
					 
				case DMA2_FLAG_TC3: if( dmaTcFp[10] != NULL) dmaTcFp[10](); break ;                                    
				case DMA2_FLAG_TE3: if( dmaTeFp[10] != NULL) dmaTeFp[10](); break ;            

				case DMA2_FLAG_TC4: if( dmaTcFp[11] != NULL) dmaTcFp[11](); break ;                                    
				case DMA2_FLAG_TE4: if( dmaTeFp[11] != NULL) dmaTeFp[11](); break ;                      

				case DMA2_FLAG_TC5: if( dmaTcFp[12] != NULL) dmaTcFp[12](); break ;                          
				case DMA2_FLAG_TE5: if( dmaTeFp[12] != NULL) dmaTeFp[12](); break ;                 

				case DMA2_FLAG_TC6: if( dmaTcFp[13] != NULL) dmaTcFp[13](); break ;                                
				case DMA2_FLAG_TE6: if( dmaTeFp[13] != NULL) dmaTeFp[13](); break ;                  

				case DMA2_FLAG_TC7: if( dmaTcFp[14] != NULL) dmaTcFp[14](); break ;                                
				case DMA2_FLAG_TE7: if( dmaTeFp[14] != NULL) dmaTeFp[14](); break ;                     

				case DMA2_FLAG_TC8: if( dmaTcFp[15] != NULL) dmaTcFp[15](); break ;                                      
				case DMA1_FLAG_TE8: if( dmaTeFp[15] != NULL) dmaTeFp[15](); break ;                     

			  default: break ;
		}
}									
/**************************************************************************************************
  * 名    称：  void DMA_Config(DMACH_e DMA_CHx, u32 periAddr, u32 memAddr, DMADIR_e tranDire, DMAMODE_e CircMode, IntPriority_e ePriority) 
  * 说    明：  初始化DMA相关参数
	* 入口参数：
	*				 @param1  DMA_CHx: 选择DMA通道
	*				   This parameter can be one of the following values:
	*				   DMA1_Channel1, DMA1_Channel2...DMA1_Channel7 or , DMA2_Channel1...DMA2_Channel5.
	*				
	*				 @param2  periAddr: 外设基地址
	*				   This parameter can be any combination of the following values:
	*				     @arg (u32)&USART1->DR
	*				
	*				 @param3  memAddr: RAM基地址BUFF
	*				   This parameter can be: (u32)SendBuff or (uint32_t)&TEXT_TO_SEND.
	*				
	*				 @param4  tranDire: 传输方向
	*				   This parameter can be any combination of the following values:
	*				     @arg DMA_DIR_PeripheralDST  //   TX
	*				     @arg DMA_DIR_PeripheralSRC  //   RX
	*				
	*				 @param5  mode: 循环模式BUFF是否循环
	*				   This parameter can be any combination of the following values:
	*				     @arg DMA_Mode_Normal        //不循环
	*				     @arg DMA_Mode_Circular      //循环模式
  *                     @arg DISABLE 
	*				 @param6  ePriority: IntPriority_e枚举类型
* 出口参数：无
* 说    明： 	DMA_Config(DMA1_Channel4,(u32)&USART1->DR,(u32)SendBuff, DMA_DIR_PeripheralDST,  DMA_Mode_Normal，INT_RANK_12);
* 调用方法：无 
  ************************************************************************************************************/
void DMA_Config(DMACH_e DMA_CHx, u32 periAddr, u32 memAddr, DMADIR_e tranDire, DMAMODE_e CircMode, IntPriority_e ePriority) //先不开辟DMA缓冲区，节省内存
{	
		DMA_InitType DMA_InitStructure;
//	  if( DMA_CHx < 8 )                                                    //DMA1下通道1~7， DMA2下通道1~5 共计12个DMA通道
//				RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA1, ENABLE);	             //使能DMA1传输、
//		else
//			  RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA2, ENABLE);	             //使能DMA2传输、
		DMA_DeInit(dmaChx[DMA_CHx]);                                         //将DMA的通道1寄存器重设为缺省值		
		DMA_InitStructure.PeriphAddr = periAddr;                 //DMA外设基地址
		DMA_InitStructure.MemAddr = memAddr;                      //DMA内存基地址  
		DMA_InitStructure.Direction = tranDire;                                //数据传输方向，从内存读取发送到外设 DMA_DIR_PeripheralDST
		DMA_InitStructure.BufSize = 0;                                //DMA通道的DMA缓存的大小
		DMA_InitStructure.PeriphInc = DMA_PERIPH_INC_DISABLE;     //外设地址寄存器不变
		DMA_InitStructure.DMA_MemoryInc = DMA_MEM_INC_ENABLE;              //内存地址寄存器递增
		DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_BYTE;  //数据宽度为8位
		DMA_InitStructure.MemDataSize = DMA_MemoryDataSize_Byte;      //数据宽度为8位
		DMA_InitStructure.CircularMode = CircMode ;                              //工作在正常模式 DMA_Mode_Normal
		DMA_InitStructure.Priority = DMA_PRIORITY_MEDIUM;                //DMA通道 x拥有中优先级 
		DMA_InitStructure.Mem2Mem = DMA_M2M_DISABLE;                         //DMA通道x没有设置为内存到内存传输
		DMA_Init(dmaChx[DMA_CHx], &DMA_InitStructure);                       //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器	  
    
		NVIC_InitType NVIC_InitStructure;
		NVIC_InitStructure.NVIC_IRQChannel = dmaIRQChx[DMA_CHx] ;			
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ePriority ;	 //抢占优先级， 
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0 ;					         //子优先级
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								                                
		NVIC_Init(&NVIC_InitStructure);
			
	  DMA_ConfigInt( dmaChx[DMA_CHx], DMA_INT_TXC, ENABLE ) ;	               //使能DMA中断通道	
} 

/****************************************************************************
* 名    称：DMA1_Channel4_IRQHandler
* 功    能: DMA1_Channel4中断处理函数
* 注    意：当DMA_CNDTRx = 0 时则产生相应中断。
*           所以TX为变长DMA长度，发送完成就会产生TC中断。而RX为定长DMA，所以当DMA_CNDTRx计数循环到0（接收缓冲区溢出）时产生中断
****************************************************************************/
 void DMA1_Channel1_IRQHandler(void)  
{
	if( DMA_GetFlagStatus( DMA1_FLAG_GL1,DMA1 ) == SET ) //判断是否为DMA TC(传输完成)中断
	{
			DMA_ClrIntPendingBit( DMA1_FLAG_GL1,DMA1  );     //清中断标志位
      SysLog("DMA1_TC1!") ;		
		  Dma1_Hook(DMA1_FLAG_GL1) ;                   //运行钩子函数，跳转到回调函数运行
	}
	if( DMA_GetFlagStatus( DMA1_FLAG_TE1 ,DMA1 ) == SET ) //判断是否为DMA TE(传输错误)中断
	{
			DMA_ClrIntPendingBit( DMA1_FLAG_TE1,DMA1  );     //清中断标志位
		  SysErr("DMA1_TE1!") ;
		  Dma1_Hook(DMA1_FLAG_TE1) ;		                //运行钩子函数，跳转到回调函数运行
	}
}

///****************************************************************************
//* 名    称：DMA1_Channel4_IRQHandler
//* 功    能: DMA1_Channel4中断处理函数
//* 注    意：当DMA_CNDTRx = 0 时则产生相应中断。
//*           所以TX为变长DMA长度，发送完成就会产生TC中断。而RX为定长DMA，所以当DMA_CNDTRx计数循环到0（接收缓冲区溢出）时产生中断
//****************************************************************************/
 void DMA1_Channel2_IRQHandler(void)  
{
	if( DMA_GetFlagStatus( DMA1_FLAG_TC2,DMA1 ) == SET )
	{ 
			DMA_ClrIntPendingBit( DMA1_FLAG_TC2,DMA1 );
      SysLog("DMA1_TC2!") ;			
		  Dma1_Hook(DMA1_FLAG_TC2) ;		
	}
	if( DMA_GetFlagStatus( DMA1_FLAG_TE2,DMA1 ) == SET )
	{
			DMA_ClrIntPendingBit( DMA1_FLAG_TE2,DMA1 );
      SysErr("DMA1_TE2!") ;			
		  Dma1_Hook(DMA1_FLAG_TE2 ) ;		
	}
}

///****************************************************************************
//* 名    称：DMA1_Channel4_IRQHandler
//* 功    能: DMA1_Channel4中断处理函数
//* 注    意：当DMA_CNDTRx = 0 时则产生相应中断。
//*           所以TX为变长DMA长度，发送完成就会产生TC中断。而RX为定长DMA，所以当DMA_CNDTRx计数循环到0（接收缓冲区溢出）时产生中断
//****************************************************************************/
 void DMA1_Channel3_IRQHandler(void)  
{	
	if( DMA_GetFlagStatus( DMA1_FLAG_TC3,DMA1 ) == SET )
	{ 
			DMA_ClrIntPendingBit( DMA1_FLAG_TC3,DMA1 );
      SysLog("DMA1_TC3!") ;			
		  Dma1_Hook(DMA1_FLAG_TC3) ;			
	}
	if( DMA_GetFlagStatus( DMA1_FLAG_TE3,DMA1 ) == SET )
	{
			DMA_ClrIntPendingBit( DMA1_FLAG_TE3,DMA1 ); 
      SysErr("DMA1_TE3!") ;	
		  Dma1_Hook(DMA1_FLAG_TE3) ;		
	}
}

///****************************************************************************
//* 名    称：DMA1_Channel4_IRQHandler
//* 功    能: DMA1_Channel4中断处理函数
//* 注    意：当DMA_CNDTRx = 0 时则产生相应中断。
//*           所以TX为变长DMA长度，发送完成就会产生TC中断。而RX为定长DMA，所以当DMA_CNDTRx计数循环到0（接收缓冲区溢出）时产生中断
//****************************************************************************/
 void DMA1_Channel4_IRQHandler(void)  
{
	if( DMA_GetFlagStatus( DMA1_FLAG_TC4,DMA1 ) == SET )
	{ 
			DMA_ClrIntPendingBit( DMA1_FLAG_TC4,DMA1 );
      SysLog("DMA1_TC4!") ;			
		  Dma1_Hook(DMA1_FLAG_TC4) ;			
	}
	if( DMA_GetFlagStatus( DMA1_FLAG_TE4,DMA1 ) == SET )
	{
			DMA_ClrIntPendingBit( DMA1_FLAG_TE4,DMA1 ); 
      SysErr("DMA1_TE4!") ;	
		  Dma1_Hook(DMA1_FLAG_TE4) ;		
	}
}

///****************************************************************************
//* 名    称：DMA1_Channel4_IRQHandler
//* 功    能: DMA1_Channel4中断处理函数
//* 注    意：当DMA_CNDTRx = 0 时则产生相应中断。
//*           所以TX为变长DMA长度，发送完成就会产生TC中断。而RX为定长DMA，所以当DMA_CNDTRx计数循环到0（接收缓冲区溢出）时产生中断
//****************************************************************************/
 void DMA1_Channel5_IRQHandler(void)  
{
	if( DMA_GetFlagStatus( DMA1_FLAG_TC5,DMA1 ) == SET )
	{ 
			DMA_ClrIntPendingBit( DMA1_FLAG_TC5,DMA1 ); 
      SysLog("DMA1_TC5!") ;	
		  Dma1_Hook(DMA1_FLAG_TC5) ;		
	}
	if( DMA_GetFlagStatus( DMA1_FLAG_TE5,DMA1 ) == SET )
	{
			DMA_ClrIntPendingBit( DMA1_FLAG_TE5,DMA1 );
      SysErr("DMA1_TE5!") ;			
		  Dma1_Hook(DMA1_FLAG_TE5) ;		
	}
}

///****************************************************************************
//* 名    称：DMA1_Channel4_IRQHandler
//* 功    能: DMA1_Channel4中断处理函数
//* 注    意：当DMA_CNDTRx = 0 时则产生相应中断。
//*           所以TX为变长DMA长度，发送完成就会产生TC中断。而RX为定长DMA，所以当DMA_CNDTRx计数循环到0（接收缓冲区溢出）时产生中断
//****************************************************************************/
 void DMA1_Channel6_IRQHandler(void)  
{
	if( DMA_GetFlagStatus( DMA1_FLAG_TC6,DMA1 ) == SET )
	{ 
			DMA_ClrIntPendingBit( DMA1_FLAG_TC6,DMA1 ); 
      SysLog("DMA1_TC6!") ;	
		  Dma1_Hook(DMA1_FLAG_TC6) ;		
	}
	if( DMA_GetFlagStatus( DMA1_FLAG_TE6,DMA1 ) == SET )
	{
			DMA_ClrIntPendingBit( DMA1_FLAG_TE6,DMA1 ) ;
      SysErr("DMA1_TE6!") ;			
		  Dma1_Hook(DMA1_FLAG_TE6) ;		
	}
}

///****************************************************************************
//* 名    称：DMA1_Channel7_IRQHandler
//* 功    能: DMA1_Channel7中断处理函数
//* 注    意：当DMA_CNDTRx = 0 时则产生相应中断。
//*           所以TX为变长DMA长度，发送完成就会产生TC中断。而RX为定长DMA，所以当DMA_CNDTRx计数循环到0（接收缓冲区溢出）时产生中断
//****************************************************************************/
 void DMA1_Channel7_IRQHandler(void)  
{
	if( DMA_GetFlagStatus( DMA1_FLAG_TC7,DMA1 ) == SET )
	{ 
			DMA_ClrIntPendingBit( DMA1_FLAG_TC7,DMA1 );
      SysLog("DMA1_TC7!") ;	//Rs485Sending.link_state = SUCCESS;		
		  Dma1_Hook(DMA1_FLAG_TC7) ;			
	}
	if( DMA_GetFlagStatus( DMA1_FLAG_TE7,DMA1 ) == SET )
	{
			DMA_ClrIntPendingBit( DMA1_FLAG_TE7,DMA1 ); 
      SysErr("DMA1_TE7!") ;	
		  Dma1_Hook(DMA1_FLAG_TE7) ;		
	}
}
///****************************************************************************
//* 名    称：DMA1_Channel8_IRQHandler
//* 功    能: DMA1_Channel8中断处理函数
//* 注    意：当DMA_CNDTRx = 0 时则产生相应中断。
//*           所以TX为变长DMA长度，发送完成就会产生TC中断。而RX为定长DMA，所以当DMA_CNDTRx计数循环到0（接收缓冲区溢出）时产生中断
//****************************************************************************/
 void DMA1_Channel8_IRQHandler(void)  
{
	if( DMA_GetFlagStatus( DMA1_FLAG_TC8,DMA1 ) == SET )
	{ 
			DMA_ClrIntPendingBit( DMA1_FLAG_TC8,DMA1 );
      SysLog("DMA1_TC8!") ;	//Rs485Sending.link_state = SUCCESS;		
		  Dma1_Hook(DMA1_FLAG_TC8) ;			
	}
	if( DMA_GetFlagStatus( DMA1_FLAG_TE8,DMA1 ) == SET )
	{
			DMA_ClrIntPendingBit( DMA1_FLAG_TE8,DMA1 ); 
      SysErr("DMA1_TE8!") ;	
		  Dma1_Hook(DMA1_FLAG_TE8) ;		
	}
}
//************************************DMA2_1****************************************/
 void DMA2_Channel1_IRQHandler(void)  
{
	if( DMA_GetFlagStatus( DMA2_FLAG_GL1,DMA2 ) == SET ) //判断是否为DMA TC(传输完成)中断
	{
			DMA_ClrIntPendingBit( DMA2_FLAG_GL1,DMA2  );     //清中断标志位
      SysLog("DMA2_TC1!") ;		
		  Dma2_Hook(DMA2_FLAG_GL1) ;                   //运行钩子函数，跳转到回调函数运行
	}
	if( DMA_GetFlagStatus( DMA2_FLAG_TE1 ,DMA2 ) == SET ) //判断是否为DMA TE(传输错误)中断
	{
			DMA_ClrIntPendingBit( DMA2_FLAG_TE1,DMA2  );     //清中断标志位
		  SysErr("DMA2_TE1!") ;
		  Dma2_Hook(DMA2_FLAG_TE1) ;		                //运行钩子函数，跳转到回调函数运行
	}
}
//************************************DMA2_2****************************************/
 void DMA2_Channel2_IRQHandler(void)  
{
	if( DMA_GetFlagStatus( DMA2_FLAG_TC2,DMA2 ) == SET )
	{ 
			DMA_ClrIntPendingBit( DMA2_FLAG_TC2,DMA2 );
      SysLog("DMA2_TC2!") ;			
		  Dma2_Hook(DMA2_FLAG_TC2) ;		
	}
	if( DMA_GetFlagStatus( DMA2_FLAG_TE2,DMA2 ) == SET )
	{
			DMA_ClrIntPendingBit( DMA2_FLAG_TE2,DMA2 );
      SysErr("DMA2_TE2!") ;			
		  Dma2_Hook(DMA2_FLAG_TE2 ) ;		
	}
}
//************************************DMA2_3****************************************/
 void DMA2_Channel3_IRQHandler(void)  
{	
	if( DMA_GetFlagStatus( DMA2_FLAG_TC3,DMA2 ) == SET )
	{ 
			DMA_ClrIntPendingBit( DMA2_FLAG_TC3,DMA2 );
      SysLog("DMA2_TC3!") ;			
		  Dma2_Hook(DMA2_FLAG_TC3) ;			
	}
	if( DMA_GetFlagStatus( DMA2_FLAG_TE3,DMA2 ) == SET )
	{
			DMA_ClrIntPendingBit( DMA2_FLAG_TE3,DMA2 ); 
      SysErr("DMA2_TE3!") ;	
		  Dma2_Hook(DMA2_FLAG_TE3) ;		
	}
}
//************************************DMA2_4****************************************/
 void DMA2_Channel4_IRQHandler(void)  
{
	if( DMA_GetFlagStatus( DMA2_FLAG_TC4,DMA2 ) == SET )
	{ 
			DMA_ClrIntPendingBit( DMA2_FLAG_TC4,DMA2 );
      SysLog("DMA2_TC4!") ;			
		  Dma2_Hook(DMA2_FLAG_TC4) ;			
	}
	if( DMA_GetFlagStatus( DMA2_FLAG_TE4,DMA2 ) == SET )
	{
			DMA_ClrIntPendingBit( DMA2_FLAG_TE4,DMA2 ); 
      SysErr("DMA2_TE4!") ;	
		  Dma2_Hook(DMA2_FLAG_TE4) ;		
	}
}
//************************************DMA2_5****************************************/
 void DMA2_Channel5_IRQHandler(void)  
{
	if( DMA_GetFlagStatus( DMA2_FLAG_TC5,DMA2 ) == SET )
	{ 
			DMA_ClrIntPendingBit( DMA2_FLAG_TC5,DMA2 ); 
      SysLog("DMA2_TC5!") ;	
		  Dma2_Hook(DMA2_FLAG_TC5) ;		
	}
	if( DMA_GetFlagStatus( DMA2_FLAG_TE5,DMA2 ) == SET )
	{
			DMA_ClrIntPendingBit( DMA2_FLAG_TE5,DMA2 );
      SysErr("DMA2_TE5!") ;			
		  Dma2_Hook(DMA2_FLAG_TE5) ;		
	}
}
//************************************DMA2_6****************************************/
 void DMA2_Channel6_IRQHandler(void)  
{
	if( DMA_GetFlagStatus( DMA2_FLAG_TC6,DMA2 ) == SET )
	{ 
			DMA_ClrIntPendingBit( DMA2_FLAG_TC6,DMA2 ); 
      SysLog("DMA2_TC6!") ;	
		  Dma2_Hook(DMA2_FLAG_TC6) ;		
	}
	if( DMA_GetFlagStatus( DMA2_FLAG_TE6,DMA2 ) == SET )
	{
			DMA_ClrIntPendingBit( DMA2_FLAG_TE6,DMA2 ) ;
      SysErr("DMA2_TE6!") ;			
		  Dma2_Hook(DMA2_FLAG_TE6) ;		
	}
}
//************************************DMA2_7****************************************/
 void DMA2_Channel7_IRQHandler(void)  
{
	if( DMA_GetFlagStatus( DMA2_FLAG_TC7,DMA2 ) == SET )
	{ 
			DMA_ClrIntPendingBit( DMA2_FLAG_TC7,DMA2 );
      SysLog("DMA2_TC7!") ;	//Rs485Sending.link_state = SUCCESS;		
		  Dma2_Hook(DMA2_FLAG_TC7) ;			
	}
	if( DMA_GetFlagStatus( DMA2_FLAG_TE7,DMA2 ) == SET )
	{
			DMA_ClrIntPendingBit( DMA2_FLAG_TE7,DMA2 ); 
      SysErr("DMA2_TE7!") ;	
		  Dma2_Hook(DMA2_FLAG_TE7) ;		
	}
}
//************************************DMA2_8****************************************/
 void DMA2_Channel8_IRQHandler(void)  
{
	if( DMA_GetFlagStatus( DMA2_FLAG_TC8,DMA2 ) == SET )
	{ 
			DMA_ClrIntPendingBit( DMA2_FLAG_TC8,DMA2 );
      SysLog("DMA2_TC8!") ;	//Rs485Sending.link_state = SUCCESS;		
		  Dma2_Hook(DMA2_FLAG_TC8) ;			
	}
	if( DMA_GetFlagStatus( DMA2_FLAG_TE8,DMA2 ) == SET )
	{
			DMA_ClrIntPendingBit( DMA2_FLAG_TE8,DMA2 ); 
      SysErr("DMA2_TE8!") ;	
		  Dma2_Hook(DMA2_FLAG_TE8) ;		
	}
}












