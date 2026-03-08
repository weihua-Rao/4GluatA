//#include "usart.h"
//#include "n32g43x.h"
//#include "sysport.h" 
//#include "io.h"
#include "main.h"
/********************************************************************************
  * @file    usart.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.1.0
  * @date    26-Dec-2018
  * @brief   提供串口外设相关驱动
  ******************************************************************************
	* @驱动功能：
	* 1、GPIO初始化
	* 2、提供串口发送函数
	* 3、采用串口DMA进行数据收发
	* 4、采用串口接收帧中断配合DMA为上层提供帧数据
  * @使用方法：
	* 1、先调用UARTx_Init()对UART进行初始化、NVIC、EXTI初始化
	* 2、在应用层调用Uart_RegHookCallback()函数注册串口相关回调函数
	* @总结
	*    采用串口空闲中断配合串口回调函数可以实现，应用层只需要注册回调函数，底层只要
	     接收到一帧数据就会跳转到回调，应用层只需要在回调中处理帧数据即可。
*******************************************************************************/

/****************************************************************************
* 供外部使用的常变量
****************************************************************************/

/****************************************************************************
* 内部使用的常变量
****************************************************************************/
char uart1_Dma_Rx_Buf[UART1_DMA_RX_MAXLEN] ;      //UART1串口DMA接收缓冲区
char uart2_Dma_Rx_Buf[UART2_DMA_RX_MAXLEN] ;      //UART2串口DMA接收缓冲区
char uart3_Dma_Rx_Buf[UART3_DMA_RX_MAXLEN] ;      //UART3串口DMA接收缓冲区
char uart4_Dma_Rx_Buf[UART4_DMA_RX_MAXLEN] ;      //UART4串口DMA接收缓冲区
//char uart5_Rx_Buf[UART5_RX_MAXLEN] ;              //UART5中断接收数据缓冲区
volatile int uart5_Rx_Index = 0 ;                 //UART5中断接收数据缓冲区数据指针

/*串口相关参数*/
USART_Module *UARTx_COM[COMSUM]     = {USART1, USART2, USART3, UART4} ;                           //, UART4, UART5 5个串口的寄存器映射地址
const u8       UARTx_IRQ[COMSUM]     = {USART1_IRQn, USART2_IRQn, USART3_IRQn, UART4_IRQn} ;  //, UART4_IRQn, UART5_IRQn 5个串口TX引脚对应串口中断线
PORT_e         UARTx_TX_PORT[COMSUM] = {PA, PA, PB, PC} ;                                             //, PC, PC 5个串口TX引脚对应IO端口
PIN_e          UARTx_TX_PIN [COMSUM] = {PIN9, PIN2, PIN10, PIN10} ;                                //, PIN10, PIN12 5个串口TX引脚对应IO引脚
PORT_e         UARTx_RX_PORT[COMSUM] = {PA, PA, PB, PC} ;                                             //, PC, PD 5个串口RX引脚对应IO端口
PIN_e          UARTx_RX_PIN [COMSUM] = {PIN10, PIN3, PIN11, PIN11} ;                                //, PIN11, PIN2 5个串口RX引脚对应IO引脚
const u32      UARTx_UART_RCC[COMSUM]= {RCC_APB2_PERIPH_USART1, RCC_APB1_PERIPH_USART2, RCC_APB1_PERIPH_USART3, RCC_APB1_PERIPH_UART4};
//                                        , RCC_APB1_PERIPH_UART5} ;
                                       
/*串口DMA相关参数*/
DMACH_e  UARTxDma_Tx_CHx[COMSUM-1]      = { DMA1CH4, DMA1CH7, DMA1CH2, DMA2CH5} ;                   //, DMA1_CH2 , DMA2_CH5 UARTx DMA发送通道，UART5没有DMA功能
DMACH_e  UARTxDma_Rx_CHx[COMSUM-1]      = { DMA1CH5, DMA1CH6, DMA1CH3, DMA2CH3} ;                   //, DMA1_CH3 , DMA2_CH3 UARTx DMA接收通道，UART5没有DMA功能
const u16 UARTxDMA_RX_BUFLEN[COMSUM-1]   = {UART1_DMA_RX_MAXLEN, UART2_DMA_RX_MAXLEN, UART3_DMA_RX_MAXLEN, UART4_DMA_RX_MAXLEN} ;         //, UART4_DMA_RX_MAXLEN 串口DMA接收缓冲区字节长度

char *UARTxDMA_RX_BUFADDR[COMSUM-1]  = {uart1_Dma_Rx_Buf, uart2_Dma_Rx_Buf,uart3_Dma_Rx_Buf,uart4_Dma_Rx_Buf} ; // , (u32)uart4_Dma_Rx_Buf 串口DMA接收缓冲区地址
USART_Module *UARTx_Dma_periAddr[COMSUM-1]   = {USART1, USART2, USART3, UART4} ;   //, (u32)&UART4->DAT  串口数据寄存器地址

//u32 UARTxDMA_RX_BUFADDR[COMSUM-1]  = {(u32)uart1_Dma_Rx_Buf, (u32)uart2_Dma_Rx_Buf,(u32)uart3_Dma_Rx_Buf} ; // , (u32)uart4_Dma_Rx_Buf 串口DMA接收缓冲区地址
//u32 UARTx_Dma_periAddr[COMSUM-1]   = {(u32)&USART1->DAT, (u32)&USART2->DAT, (u32)&USART3->DAT} ;   //, (u32)&UART4->DAT  串口数据寄存器地址


DMAFP     uartTX_dmaTC_Callback[COMSUM-1]= {UART1_DMA_TX_TC_Callback_Functions, UART2_DMA_TX_TC_Callback_Functions,
																						UART3_DMA_TX_TC_Callback_Functions,UART4_DMA_TX_TC_Callback_Functions} ; // DMA发送完成中断回调函数
DMAFP     uartTX_dmaTE_Callback[COMSUM-1]= {NULL} ;                                     //, NULL DMA串口发送失败中断回调函数
DMAFP     uartRX_dmaTC_Callback[COMSUM-1]= {NULL} ;                                     //, NULL DMA串口接收完成中断回调函数
DMAFP     uartRX_dmaTE_Callback[COMSUM-1]= {NULL} ;                                     //, NULL DMA串口接收失败中断回调函数

volatile bool UARTxDma_Tx_TcFlag[4] = {false, false, false} ;                                   // , false串口DMA发送完成全局标志
/**************************************************************************************************************/ 

/*****************************************
*中断事件回调函数相关配置
****************************************/
UARTFP  uartFp[COMSUM] = {NULL} ;       //定义UART1~UART5共计5个串口回调函数指针

 /**************************************************************************************************
* 名    称：UARTFP  Uart_RegHookCallback(UARTFP pCallback, COM_e eCOMn) 
* 功能说明：串口接收超时中断回调函数-注册函数。串口在接收到一帧数据时会触发中断执行回调。数据时通过DMA接收到相应的缓冲区中的
* 入口参数：
*           @param1 pCallback    UARTFP类型函数指针
*           @param2 eCOMn        COM_e枚举类型数据
* 出口参数：
*           @param1 pCallback    UARTFP类型函数指针
*************************************************************************************************/ 
UARTFP  Uart_RegHookCallback(COM_e eCOMn, UARTFP pCallback) 
{
	if( uartFp[eCOMn] == NULL )
		  uartFp[eCOMn] = pCallback ;
	else
		 SysErr("Uart Callback repeat reg!") ; //Uart Callback repeat reg!
  return 	pCallback ;
} 

/**************************************************************************************************
* 名    称：void Uart_Hook(COM_e eCOMn, char *recvBuf)
* 功能说明：串口接收超时中断内调用的钩子函数（执行到中断会把相应的回调函数勾出来运行，嘿嘿嘿。。。
*           当然不同的串口会勾出不同的回调函数uartFp[]）
* 入口参数：
*           @param1 eCOMn       COM_e枚举变量，区分UART1~UART5 五个串口
*           @param2 *recvBuf    串口在此中断中接收到的数据存放地址 
*           @param3 recvLen     串口在此中断中接收到的数据长度
*************************************************************************************************/ 
void Uart_Hook(COM_e eCOMn, char *recvBuf, uint16_t recvLen)
 {
	 if( uartFp[eCOMn] != NULL)
	   {
		   uartFp[eCOMn](recvBuf, recvLen) ;
		 }
 }
 
/*********加入以下代码,支持printf函数,而不需要选择use MicroLIB**************/  
#if 1
#pragma import(__use_no_semihosting)             
                
struct __FILE  //标准库需要的支持函数 
{ 
	int handle; 
}; 

FILE __stdout;       
  
void _sys_exit(int x) //定义_sys_exit()以避免使用半主机模式  
{ 
	x = x; 
} 

void _ttywrch(int ch)
{
    ch=ch;
}

int fputc(int ch, FILE *f)//重定义fputc函数 
{      
  UARTx_COM[UART_DEBUG]->DAT = (u8) ch; 
	while(( UARTx_COM[UART_DEBUG]->STS & 0X40 ) == 0) ;//循环发送,直到发送完毕   TC	
	return ch;
}
#endif 

/****************************************************************************/ 
void UARTx_Init(COM_e eCOMn, uint32_t baudrate, uint16_t uartMode, IntPriority_e ePriority) 
{
//		GPIO_InitType GPIO_InitStructure;	
		NVIC_InitType NVIC_InitStructure;	
		USART_InitType USART_InitStructure;
		
/*对串口IO初始化*/ 	
		RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO , ENABLE) ;                        //IO复用时钟开启	
		Gpio_Init(UARTx_TX_PORT[eCOMn], UARTx_TX_PIN[eCOMn], GPIO_Mode_AF_PP) ;
		Gpio_Init(UARTx_RX_PORT[eCOMn], UARTx_RX_PIN[eCOMn], GPIO_Mode_IN_FLOATING) ; //GPIO_Mode_IN_FLOATING GPIO_Mode_IPU
		 
/*对串口相关参数初始化*/ 
    if( eCOMn == COM1 )                                                      //COM1和COM2、3、4、5所在的APB不同，所以要分开开启APB时钟
			  RCC_EnableAPB2PeriphClk(UARTx_UART_RCC[eCOMn] , ENABLE) ;
		else
			  RCC_EnableAPB1PeriphClk(UARTx_UART_RCC[eCOMn] , ENABLE) ;
		USART_InitStructure.BaudRate 						= baudrate;						    //速率baudratebps
		USART_InitStructure.WordLength 					= USART_WL_8B;		//数据位8位
		USART_InitStructure.StopBits 						= USART_STPB_1;			  //停止位1位
		USART_InitStructure.Parity 							= USART_PE_NO;				//无校验位
		USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;   //无硬件流控
		USART_InitStructure.Mode 								= uartMode ; 							//USART_Mode_Rx | USART_Mode_Tx;		//收发模式
		USART_Init(UARTx_COM[eCOMn], &USART_InitStructure);							        //配置串口参数函数 
		USART_ClrFlag(UARTx_COM[eCOMn], USART_FLAG_CTSF | USART_FLAG_LINBD | USART_FLAG_TXC | USART_FLAG_RXDNE) ;	 //清标志位
    if(  ((USART_InitStructure.Mode & USART_MODE_TX) == USART_MODE_TX)
			 &&(eCOMn != COM5))	 																									//当串口UART1~UART4发送功能启用，则初始化串口发送DMA
			{
				//UARTx_DmaTx_Init(eCOMn) ;                                         //串口发送DMA初始化 
				USART_EnableDMA(UARTx_COM[eCOMn], USART_DMAREQ_TX, ENABLE);            //使能串口的DMA发送
//				DMA_Config((DMACH_e)UARTxDma_Tx_CHx[eCOMn], UARTx_Dma_periAddr[eCOMn], 0, DMA_DIR_MTP, NORMAL, INT_RANK_0) ;	//TX优先级如果设置为OS可屏蔽的话，在系统OS启动前无法使用串口输出
				DMA_Config(UARTxDma_Tx_CHx[eCOMn], (u32)&UARTx_Dma_periAddr[eCOMn]->DAT, 0, DMA_DIR_MTP, NORMAL, INT_RANK_0) ;	//INT_RANK_0 TX优先级如果设置为OS可屏蔽的话，在系统OS启动前无法使用串口输出
        Dma_RegHookCallback(UARTxDma_Tx_CHx[eCOMn], DMATC, uartTX_dmaTC_Callback[eCOMn]) ;	//注册DMA传输完成回调函数 DMACH_e  DMAIRQTYPE_e  DMAFP 
        Dma_RegHookCallback(UARTxDma_Tx_CHx[eCOMn], DMATE, uartTX_dmaTE_Callback[eCOMn]) ;	//注册DMA传输失败回调函数			
			}				
		
    if((USART_InitStructure.Mode & USART_MODE_RX) == USART_MODE_RX)	 //当串口接收功能启用，则开启接收中断和接收空闲中断
			{
        /*对串口NVIC初始化*/ 				
				NVIC_InitStructure.NVIC_IRQChannel = UARTx_IRQ[eCOMn];			        //设置串口中断
				NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ePriority;	  //抢占优先级
				NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0 ;				          //子优先级为0
				NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					            //使能
				NVIC_Init(&NVIC_InitStructure);
				USART_ConfigInt(UARTx_COM[eCOMn], USART_INT_IDLEF, ENABLE);            //使能串口接收空闲类型			    //USART_ITConfig(UARTx_COM[eCOMn], USART_IT_RXNE, ENABLE);          //使能串口接收中断			
				if( eCOMn != COM5 )  //UART1~UART4开启串口DMA接收
					{
						//UARTx_DmaRx_Init(eCOMn ) ;                                    //串口接收DMA初始化
						USART_EnableDMA(UARTx_COM[eCOMn], USART_DMAREQ_RX, ENABLE);        //使能串口的DMA接收
//						DMA_Config((DMACH_e)UARTxDma_Rx_CHx[eCOMn], UARTx_Dma_periAddr[eCOMn], UARTxDMA_RX_BUFADDR[eCOMn], DMA_DIR_PTM, NORMAL, ePriority) ;

//						DMA_Config((DMACH_e)UARTxDma_Rx_CHx[eCOMn], UARTx_Dma_periAddr[eCOMn], UARTxDMA_RX_BUFADDR[eCOMn], DMA_DIR_PTM, NORMAL, ePriority) ;
						DMA_Config(UARTxDma_Rx_CHx[eCOMn], (u32)&UARTx_Dma_periAddr[eCOMn]->DAT, (u32)UARTxDMA_RX_BUFADDR[eCOMn], DMA_DIR_PTM, NORMAL, ePriority) ;
						DMA_SetCurrDataCounter(dmaChx[UARTxDma_Rx_CHx[eCOMn]], UARTxDMA_RX_BUFLEN[eCOMn]);  //DMA通道的DMA缓存的大小
						Dma_RegHookCallback(UARTxDma_Rx_CHx[eCOMn], DMATC, uartRX_dmaTC_Callback[eCOMn]) ;	//注册DMA传输完成回调函数
						Dma_RegHookCallback(UARTxDma_Rx_CHx[eCOMn], DMATE, uartRX_dmaTE_Callback[eCOMn]) ;	//注册DMA传输失败回调函数
						DMA_EnableChannel(dmaChx[UARTxDma_Rx_CHx[eCOMn]], ENABLE);                    								//使能dmaChx[UARTxDma_Tx_CHx[eCOMn]]所指示的通道 					
					}
				else if( eCOMn == COM5)                                             //UART5不具有DMA通道，所以开启串口接收中断、串口接收空闲中断来接收数据
				  {
					  USART_ConfigInt(UARTx_COM[eCOMn], USART_INT_RXDNE, ENABLE);  
					}
			}	
		USART_Enable(UARTx_COM[eCOMn], ENABLE);	                                  //Enable the USART1 
}

/****************************************************************************
* 名    称：void UARTx_DmaTx_Init(COM_e eCOMn) 
* 功    能：串口DMA发送初始化
* 入口参数：
*           @param1 COMn    端口
*                     @arg COM_e枚举类型数据
* 说    明：默认开启TC中断，中断回调函数需要在stm32f10x_it.h文件中注册
* 注    意：所有中断优先级需要在stm32f10x_it.h文件中配置
****************************************************************************/
void UARTx_DmaTx_Init(COM_e eCOMn)
{
	USART_EnableDMA(UARTx_COM[eCOMn], USART_DMAREQ_TX, ENABLE);           //使能串口的DMA发送 
//	  DMA_Config((DMACH_e)UARTxDma_Tx_CHx[eCOMn], UARTx_Dma_periAddr[eCOMn], 0, DMA_DIR_MTP, NORMAL, INT_RANK_6) ;
  
//	  DMA_Config((DMACH_e)UARTxDma_Tx_CHx[eCOMn], UARTx_Dma_periAddr[eCOMn], 0, DMA_DIR_MTP, NORMAL, INT_RANK_6) ;
	DMA_Config(UARTxDma_Tx_CHx[eCOMn], (u32)&UARTx_Dma_periAddr[eCOMn]->DAT, 0, DMA_DIR_MTP, NORMAL, INT_RANK_6) ;
}

/****************************************************************************
* 名    称：void UARTx_DmaRx_Init(COM_e eCOMn )
* 功    能：串口DMA接收初始化
* 入口参数：
*           @param1 COMn    端口
*                     @arg COM_e枚举类型数据
* 说    明：默认开启TC中断，中断回调函数需要在stm32f10x_it.h文件中注册
* 注    意：所有中断优先级需要在stm32f10x_it.h文件中配置
****************************************************************************/
void UARTx_DmaRx_Init(COM_e eCOMn )
{
	USART_EnableDMA(UARTx_COM[eCOMn], USART_DMAREQ_RX, ENABLE);             //使能串口的DMA接收
//DMA_Config((DMACH_e)UARTxDma_Rx_CHx[eCOMn], UARTx_Dma_periAddr[eCOMn], UARTxDMA_RX_BUFADDR[eCOMn], DMA_DIR_PTM, NORMAL, INT_RANK_6) ;

//  DMA_Config((DMACH_e)UARTxDma_Rx_CHx[eCOMn], UARTx_Dma_periAddr[eCOMn], UARTxDMA_RX_BUFADDR[eCOMn], DMA_DIR_PTM, NORMAL, INT_RANK_6) ;
  DMA_Config(UARTxDma_Rx_CHx[eCOMn], (u32)&UARTx_Dma_periAddr[eCOMn]->DAT, (u32)UARTxDMA_RX_BUFADDR[eCOMn], DMA_DIR_PTM, NORMAL, INT_RANK_6) ;
	DMA_SetCurrDataCounter(dmaChx[UARTxDma_Rx_CHx[eCOMn]], UARTxDMA_RX_BUFLEN[eCOMn]);          //DMA通道的DMA缓存的大小
	DMA_EnableChannel(dmaChx[UARTxDma_Rx_CHx[eCOMn]], ENABLE);                     //使能dmaChx[UARTxDma_Tx_CHx[eCOMn]]所指示的通道 
}

/****************************************************************************
* 名    称：void UARTx_DmaRx_Restart(COM_e eCOMn) 
* 功    能：串口DMA接收
* 入口参数：
*           @param1 COMn    端口
*                     @arg COM_e枚举类型数据
* 说    明：清除接收缓冲区
****************************************************************************/
void UARTx_DmaRx_Restart(COM_e eCOMn)
{
	DMA_EnableChannel(dmaChx[UARTxDma_Rx_CHx[eCOMn]], DISABLE);                                  //失能dmaChx[UARTxDma_Tx_CHx[eCOMn]]所指示的通道	
	//	DMA_Config((DMACH_e)UARTxDma_Rx_CHx[eCOMn], UARTx_Dma_periAddr[eCOMn], UARTxDMA_RX_BUFADDR[eCOMn], DMA_DIR_PTM, NORMAL) ;
	memset(UARTxDMA_RX_BUFADDR[eCOMn], 0, UARTxDMA_RX_BUFLEN[eCOMn]) ;
	DMA_SetCurrDataCounter(dmaChx[UARTxDma_Rx_CHx[eCOMn]], UARTxDMA_RX_BUFLEN[eCOMn]); //DMA通道的DMA缓存的大小
	DMA_EnableChannel(dmaChx[UARTxDma_Rx_CHx[eCOMn]], ENABLE);                                   //使能dmaChx[UARTxDma_Tx_CHx[eCOMn]]所指示的通道 
}

/****************************************************************************
* 名    称：void UARTx_SendData(COM_e eCOMn, const char *sendData, uint16_t dataLen)
* 功    能：通过eCOMn端口输出dataLen长度的数据  
* 入口参数：
*           @param COMn     端口
*                     @arg COM_e枚举类型数据
*           @param *sendData    输出的数据指针
*           @param dataLen      输出数据的长度
* 注    意：通过宏定义UARTx_DMATX_EN决定是否启用串口DMA发送数据
*           发送等待DMA TC中断判断串口数据是否发送完成  UARTx_DMA_Tx(UART_DEBUG, (u32)log, LOG_BUF_LEN ) ;
****************************************************************************/
void UARTx_SendData(COM_e eCOMn, const char *sendData, uint16_t dataLen)
{	
	if( (sendData == NULL) || (dataLen <= 0) )
	  {
			SysErr("") ;
		  return ;
		}
	if(UARTx_DMATX_EN && (eCOMn != COM5))                                     //使能串口DMA发送,UART5不支持串口DMA发送
	  {
			 DMA_EnableChannel(dmaChx[UARTxDma_Tx_CHx[eCOMn]], DISABLE );                   //关闭通道才能对 CMAR值做改写
			 UARTxDma_Tx_TcFlag[eCOMn] = false ;                                  //串口eCOMn发送完成标志位清除
			 dmaChx[UARTxDma_Tx_CHx[eCOMn]]->MADDR = (uint32_t)sendData ;          //->CMAR DMA 内存地址
			 DMA_SetCurrDataCounter(dmaChx[UARTxDma_Tx_CHx[eCOMn]],dataLen);      //DMA通道的DMA缓存的大小
			 //DMA_InitStructure.DMA_MemoryBaseAddr = uartTxBufferAdder;          //DMA内存基地址
			 //DMA_InitStructure.DMA_BufferSize = uartTxDataLen;                  //DMA通道的DMA缓存的大小
			 //DMA_Init(DMA_CHx, &DMA_InitStructure);
			 DMA_EnableChannel(dmaChx[UARTxDma_Tx_CHx[eCOMn]], ENABLE);                     //使能dmaChx[UARTxDma_Tx_CHx[eCOMn]]所指示的通道 

//			while( UARTxDma_Tx_TcFlag[eCOMn] == false );		//等待串口eCOMn DMA发送完成标志位	
			u32 delaycn = 0xF00000;			
			while(( UARTxDma_Tx_TcFlag[eCOMn] == false )&&(delaycn--));

	  }
		else                                                                    //采用查询串口发送完成标志的方法来发送串口数据
	  {
			 uint16_t index = 0 ;
			 const char *pos = sendData ;
			 USART_GetFlagStatus(UARTx_COM[eCOMn], USART_FLAG_TXC);                //解决第一个字符发送失败的问题
			 for(index = 0; index < dataLen; index++)
			  {
					 USART_SendData(UARTx_COM[eCOMn], *(pos+index)) ;
					 while(USART_GetFlagStatus(UARTx_COM[eCOMn], USART_FLAG_TXC)==RESET) ;
			  }
		}
}

/****************************************************************************
*单字节发送不支持串口MDA发送，尽量不要使用
* 名    称：void UARTx_Printf(COM_e eCOMn, uint8_t *Data,...)
* 功    能：格式化串口输出函数
* 入口参数：USARTx:  指定串口
			Data：   发送数组
			...:     不定参数
* 出口参数：无
* 说    明：格式化串口输出函数
*        	"\r"	回车符	   UARTx_Printf(USART1, "abcdefg\r")   
*					"\n"	换行符	   UARTx_Printf(USART1, "abcdefg\r\n")
*					"%s"	字符串	   UARTx_Printf(USART1, "字符串是：%s","abcdefg")
*					"%d"	十进制	   UARTx_Printf(USART1, "a=%d",10)
****************************************************************************/
void UARTx_SendString(COM_e eCOMn, uint8_t *Data,...)
{ 
	  const char *s;
    int d;
    char buf[16];
    va_list ap;
    va_start(ap, Data);

	  while(*Data!=0)
		{				                          //判断是否到达字符串结束符
			if(*Data == 0x5c)
				{									  //'\'
					switch (*++Data)
							{
								case 'r':							          //回车符
									USART_SendData(UARTx_COM[eCOMn], 0x0D);	   

									Data++;
									break;
								case 'n':							          //换行符
									USART_SendData(UARTx_COM[eCOMn], 0x0A);	
									Data++;
									break;
								
								default:
									Data++;
										break;
						 }		 
				}
			else if(*Data=='%')
				{									  //
					switch (*++Data)
						{				
								case 's':										  //字符串
													s = va_arg(ap, const char *);
													for ( ; *s; s++) 
													{
															USART_SendData(UARTx_COM[eCOMn],*s);
															while(USART_GetFlagStatus(UARTx_COM[eCOMn], USART_FLAG_TXC)==RESET);
													}
									        Data++;
													break;
								case 'd':										  //十进制
										d = va_arg(ap, int);
										itoa(d, buf, 10);
										for (s = buf; *s; s++) 
										{
												USART_SendData(UARTx_COM[eCOMn],*s);
												while(USART_GetFlagStatus(UARTx_COM[eCOMn], USART_FLAG_TXC)==RESET);
										}
										Data++;
										break;
								default:
									  Data++;
										break;
					 }		 
				}
			else 
				USART_SendData(UARTx_COM[eCOMn], *Data++);
			while(USART_GetFlagStatus(UARTx_COM[eCOMn], USART_FLAG_TXC)==RESET) ;
		}
}

/****************************************************************************
* 名    称：void USART1_IRQHandler(void)
* 功    能：USART1中断服务程序
****************************************************************************/
void USART1_IRQHandler(void)  
{		
		/* 串口1接收空闲中断 */
	if( USART_GetIntStatus(USART1, USART_INT_IDLEF) != RESET )  
		{
			DMA_EnableChannel(dmaChx[UARTxDma_Rx_CHx[COM1]], DISABLE);                   //失能dmaChx[UARTxDma_Tx_CHx[eCOMn]]所指示的通道	
			uint8_t clearFlag = USART1->STS ;                                   //清除空闲中断标志USART_ClearITPendingBit(USART1, USART_INT_IDLEF) ; 
			clearFlag = USART1->DAT ;
			uint16_t len = UARTxDMA_RX_BUFLEN[COM1] - DMA_GetCurrDataCounter(dmaChx[UARTxDma_Rx_CHx[COM1]]) ;
      //SysLog("COM1 RxLen:%d; RxBuf:%s,", len, uart1_Dma_Rx_Buf);
			Uart_Hook(COM1, uart1_Dma_Rx_Buf, len) ;                           //跳转到回调函数
      UARTx_DmaRx_Restart(COM1) ;			
		}
	if(USART_GetIntStatus(USART1, USART_INT_RXDNE) != RESET)	   							 //判断读寄存器是否非空
		{	
				USART_ClrIntPendingBit(USART1, USART_INT_RXDNE) ;
		}
		
	if(USART_GetIntStatus(USART1, USART_INT_TXDE) != RESET)                   //这段是为了避免STM32 USART 第一个字节发不出去的BUG 
		{ 
				USART_ConfigInt(USART1, USART_INT_TXDE, DISABLE);					         //禁止发缓冲器空中断， 
		}
}

/****************************************************************************
* 名    称：void USART2_IRQHandler(void)
* 功    能：USART2中断服务程序
****************************************************************************/
void USART2_IRQHandler(void)  
{		
	if( USART_GetIntStatus(USART2, USART_INT_IDLEF) != RESET )  
		{
			DMA_EnableChannel(dmaChx[UARTxDma_Rx_CHx[COM2]], DISABLE);                    //失能dmaChx[UARTxDma_Tx_CHx[eCOMn]]所指示的通道	
			uint8_t clearFlag = USART2->STS ;                                    //清除空闲中断标志位 无法通过USART_ClearITPendingBit(UART4, USART_IT_IDLE)清楚标志位
			clearFlag = USART2->DAT ;
			uint16_t len = UARTxDMA_RX_BUFLEN[COM2] - DMA_GetCurrDataCounter(dmaChx[UARTxDma_Rx_CHx[COM2]]) ;
      //SysLog("COM2 RxLen:%d; RxBuf:%s", len, uart2_Dma_Rx_Buf);
			Uart_Hook(COM2, uart2_Dma_Rx_Buf, len) ;                                //跳转到回调函数
      UARTx_DmaRx_Restart(COM2) ;	
		
		}
  if(USART_GetIntStatus(USART2, USART_INT_RXDNE) != RESET)	   							 //判断读寄存器是否非空
		{	
       USART_ClrIntPendingBit(USART2, USART_INT_RXDNE) ;                  //清除接收中断标志位		
		}
	if(USART_GetIntStatus(USART2, USART_INT_TXDE) != RESET)                   //这段是为了避免STM32 USART 第一个字节发不出去的BUG 
		{ 
				USART_ConfigInt(USART2, USART_INT_TXDE, DISABLE);					         //禁止发缓冲器空中断， 
		}
}

/****************************************************************************
* 名    称：void USART3_IRQHandler(void)
* 功    能：USART3中断服务程序Ec20ReceiveFrameCallback
****************************************************************************/
void USART3_IRQHandler(void)  
{
	if( USART_GetIntStatus(USART3, USART_INT_IDLEF) != RESET )  /* 串口3接收空闲中断 */
		{
			DMA_EnableChannel(dmaChx[UARTxDma_Rx_CHx[COM3]], DISABLE);     //失能dmaChx[UARTxDma_Tx_CHx[eCOMn]]所指示的通道	
			uint8_t clearFlag = USART3->STS ;                     //需要测试中断标志是否真的清零、、、、、、、、、、、、、、、、、、、、、
			clearFlag = USART3->DAT ;
			uint16_t len =UARTxDMA_RX_BUFLEN[COM3] - DMA_GetCurrDataCounter(dmaChx[UARTxDma_Rx_CHx[COM3]]) ; //DMAy_Channelx->CNDTR
 /*****下行代码需要屏蔽，在中段中耗时太长，影响帧接收，会导致丢失帧数据******/
      //SysLog("COM3 RxLen:%d; RxBuf:%s", len, uart3_Dma_Rx_Buf);   
			Uart_Hook(COM3, uart3_Dma_Rx_Buf, len) ;                                //跳转到回调函数
      UARTx_DmaRx_Restart(COM3) ;				
		}
  if(USART_GetIntStatus(USART3, USART_INT_RXDNE) != RESET)	 /* 串口3接收中断 */	
		{	
       USART_ClrIntPendingBit(USART3, USART_INT_RXDNE) ;  //清除接收中断标志位					
		}
	if(USART_GetIntStatus(USART3, USART_INT_TXDE) != RESET)                   //这段是为了避免STM32 USART 第一个字节发不出去的BUG 
		{ 
				USART_ConfigInt(USART3, USART_INT_TXDE, DISABLE);					         //禁止发缓冲器空中断， 
		}
}

/****************************************************************************
* 名    称：void UART4_IRQHandler(void)
* 功    能：UART4中断服务程序
****************************************************************************/
void UART4_IRQHandler(void)  
{		
		/* 串口4接收空闲中断 */
	if( USART_GetIntStatus(UART4, USART_INT_IDLEF) != RESET )  
		{
			DMA_EnableChannel(dmaChx[UARTxDma_Rx_CHx[COM4]], DISABLE);                   //失能dmaChx[UARTxDma_Tx_CHx[eCOMn]]所指示的通道	
			uint8_t clearFlag = UART4->STS ;                                    //清楚空闲中断标志位 无法通过USART_ClearITPendingBit(UART4, USART_IT_IDLE)清楚标志位
			clearFlag = UART4->DAT ;
			uint16_t len = UARTxDMA_RX_BUFLEN[COM4] - DMA_GetCurrDataCounter(dmaChx[UARTxDma_Rx_CHx[COM4]]) ;
      //SysLog("COM4 RxLen:%d; RxBuf:%s", len, uart4_Dma_Rx_Buf);
			Uart_Hook(COM4, uart4_Dma_Rx_Buf, len) ;                                //跳转到回调函数
      UARTx_DmaRx_Restart(COM4) ;				
		}
	if(USART_GetIntStatus(UART4, USART_INT_RXDNE) != RESET)	   							 //判断读寄存器是否非空
		{	
        USART_ClrIntPendingBit(UART4, USART_INT_RXDNE) ;                  //清除中断标志			
		}
	if(USART_GetIntStatus(UART4, USART_INT_TXDE) != RESET)                    //这段是为了避免STM32 USART 第一个字节发不出去的BUG 
		{ 
				USART_ConfigInt(UART4, USART_INT_TXDE, DISABLE);					           //禁止发缓冲器空中断， 
		}
	USART_ClrFlag(UART4, USART_FLAG_TXC );                                //清除TC中断标志
}

/****************************************************************************
* 名    称：void UART5_IRQHandler(void)
* 功    能：UART5中断服务程序
****************************************************************************/
//void UART5_IRQHandler(void)  
//{		
//  if(USART_GetIntStatus(UART5, USART_INT_RXDNE) != RESET)	   					 //判断读寄存器是否非空
//		{	
//     uart5_Rx_Buf[uart5_Rx_Index++] = UART5->DAT	;	
//		 uart5_Rx_Buf[uart5_Rx_Index] = 0x00 ;                           //保证uart5_Rx_Buf中有正确的字符串小尾巴，在数据处理时不会越界处理
//     if( uart5_Rx_Index >= (UART5_RX_MAXLEN-1) )                     //接收溢出则从uart5_Rx_Buf[0]开始再接收
//		   {
//			   uart5_Rx_Index = 0 ;
//			 }			 
//		}
//	if( USART_GetIntStatus(UART5, USART_INT_IDLEF) != RESET )  
//		{
//			uint8_t clearFlag = UART5->STS ;                                //需要测试中断标志是否真的清零、、、、、、、、、、、、、、、、、、、、、
//			clearFlag = UART5->DAT ;
//      //SysLog("COM5 RxLen:%d; RxBuf:%s", uart5_Rx_Index, uart5_Rx_Buf);
//			Uart_Hook(COM5, (char*)uart5_Rx_Buf, uart5_Rx_Index) ;                         //跳转到回调函数运行
//      memset((char*)uart5_Rx_Buf, 0, UART5_RX_MAXLEN) ;	             //清空uart5_Rx_Buf	
//      uart5_Rx_Index = 0 ;                                           //uart5_Rx_Buf	指针uart5_Rx_Index清零			
//      USART_ClrIntPendingBit(UART5, USART_INT_IDLEF) ;                //清除接收中断标志位			
//		}
//	if(USART_GetIntStatus(UART5, USART_INT_TXDE) != RESET)                //这段是为了避免STM32 USART 第一个字节发不出去的BUG 
//		{ 
//			USART_ConfigInt(UART5, USART_INT_TXDE, DISABLE);					         //禁止发缓冲器空中断， 
//		}
//}

/****************************************************************************
* 名    称：void UART1_DMA_TX_TC_Callback_Functions(void))
* 功    能：USART1 DMA发送完成中断回调函数
****************************************************************************/
void UART1_DMA_TX_TC_Callback_Functions(void)
{
   UARTxDma_Tx_TcFlag[COM1] = true ;
}

/****************************************************************************
* 名    称：void UART2_DMA_TX_TC_Callback_Functions(void))
* 功    能：USART2 DMA发送完成中断回调函数
****************************************************************************/
void UART2_DMA_TX_TC_Callback_Functions(void)
{
   UARTxDma_Tx_TcFlag[COM2] = true ;
//		rs485_rw = 0;
}

/****************************************************************************
* 名    称：void UART3_DMA_TX_TC_Callback_Functions(void))
* 功    能：USART3 DMA发送完成中断回调函数
****************************************************************************/
void UART3_DMA_TX_TC_Callback_Functions(void)
{
   UARTxDma_Tx_TcFlag[COM3] = true ;
}

///****************************************************************************
//* 名    称：void UART4_DMA_TX_TC_Callback_Functions(void))
//* 功    能：USART4 DMA发送完成中断回调函数
//****************************************************************************/
void UART4_DMA_TX_TC_Callback_Functions(void)
{
   UARTxDma_Tx_TcFlag[COM4] = true ;
}

/****************************************************************************
* 名    称：void UART1_DMA_RX_TC_Callback_Functions(void))
* 功    能：USART1 DMA接收完成中断回调函数
****************************************************************************/
void UART1_DMA_RX_TC_Callback_Functions(void)
{
  	 //SysLog("\r\nUart1Dma Rx TC!") ;
}

/****************************************************************************
* 名    称：void UART2_DMA_RX_TC_Callback_Functions(void))
* 功    能：USART2 DMA接收完成中断回调函数
****************************************************************************/
void UART2_DMA_RX_TC_Callback_Functions(void)
{
  
}

/****************************************************************************
* 名    称：void UART3_DMA_RX_TC_Callback_Functions(void))
* 功    能：USART3 DMA接收完成中断回调函数
****************************************************************************/
void UART3_DMA_RX_TC_Callback_Functions(void)
{}

/****************************************************************************
* 名    称：void UART4_DMA_RX_TC_Callback_Functions(void))
* 功    能：USART4 DMA接收完成中断回调函数
****************************************************************************/
void UART4_DMA_RX_TC_Callback_Functions(void)
{}



	
	
