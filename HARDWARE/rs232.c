//#define 	MAIN_URAT
//#include "main.h"
//#include "UserConf.h"
#include "rs232.h"
#include "sysport.h"

//FrameQueue_s      sRS232Queue ;    //RS232 接收数据帧数据环形缓冲区，上层只需要在初始化RS232后，处理该缓冲区数据即可
//USART_InitType USART_InitStructure;
//void USART_Config(void)
//{
//     GPIO_InitType GPIO_InitStructure;
//    /* Initialize GPIO_InitStructure */
//    GPIO_InitStruct(&GPIO_InitStructure);
//    /* Configure USARTy Tx as alternate function push-pull */
//    GPIO_InitStructure.Pin            = USARTy_TxPin;
//    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_AF_PP;
//    GPIO_InitStructure.GPIO_Alternate = USARTy_Tx_GPIO_AF;
//    GPIO_InitPeripheral(USARTy_GPIO, &GPIO_InitStructure);

//    /* Configure USARTy Rx as alternate function push-pull and pull-up */
//    GPIO_InitStructure.Pin            = USARTy_RxPin;
//    GPIO_InitStructure.GPIO_Pull      = GPIO_Pull_Up;
//    GPIO_InitStructure.GPIO_Alternate = USARTy_Rx_GPIO_AF;
//    GPIO_InitPeripheral(USARTy_GPIO, &GPIO_InitStructure);
//		
//	/* USARTy and USARTz configuration ------------------------------------------------------*/
//    USART_StructInit(&USART_InitStructure);
//    USART_InitStructure.BaudRate            = 115200;
//    USART_InitStructure.WordLength          = USART_WL_8B;
//    USART_InitStructure.StopBits            = USART_STPB_1;
//    USART_InitStructure.Parity              = USART_PE_NO;
//    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;//USART_HFCTRL_RTS_CTS;
//    USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;
//    /* Configure USARTy and USARTz */
//    USART_Init(USARTy, &USART_InitStructure);
//    /* Enable USARTy Receive and Transmit interrupts */
//    USART_ConfigInt(USARTy, USART_INT_RXDNE, ENABLE);
//    USART_ConfigInt(USARTy, USART_INT_TXDE, ENABLE);

//    /* Enable the USARTy and USARTz */
//    USART_Enable(USARTy, ENABLE);
// }

void RCC_Configuration(void)
{
//	/* PCLK1 = HCLK/2 */
	RCC_ConfigPclk2(RCC_HCLK_DIV2);	
//		/* TIM1 clock enable */
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_TIM1, ENABLE);
	
		/* PCLK1 = HCLK/4 */
	RCC_ConfigPclk1(RCC_HCLK_DIV4);
	/* TIM2&3 clock enable */
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM2 | RCC_APB1_PERIPH_TIM3 | RCC_APB1_PERIPH_TIM5, ENABLE);

	/* Enable DMA1 and DMA2 clocks */
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA1 | RCC_AHB_PERIPH_DMA2, ENABLE);//RCC_AHB_PERIPH_DMA1 | 

    /* Enable GPIO clock */
//	RCC_EnableAPB2PeriphClk(USARTy_GPIO_CLK , ENABLE);
    /* Enable USARTy and USARTz Clock */
//  USARTy_APBxClkCmd(USARTy_CLK, ENABLE);
/* GPIOC clock enable */
  RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_GPIOC | RCC_APB2_PERIPH_GPIOD, ENABLE);

	//ADC
//	/* Enable  ADC3, clocks */
	RCC_EnableAHBPeriphClk( RCC_AHB_PERIPH_ADC3 ,	 ENABLE);
	
	/* RCC_ADCHCLK_DIV16*/
	ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB,RCC_ADCHCLK_DIV16);
	RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSE, RCC_ADC1MCLK_DIV8);  //selsect HSE as RCC ADC1M CLK Source		

}

//void Uart1Sends(char *puts)
//{	
//	TX1_Data = puts;
//	USART_ConfigInt(USARTy, USART_INT_TXDE, ENABLE);//DISABLE
//	while(TX1_Data != NULL);//等待发送完成
//}

//void Uart1BYTE(char temp)
//{
//	TX1_Data = &temp;
//	USART_ConfigInt(USARTy, USART_INT_TXDE, ENABLE);//DISABLE
//	while(USART_GetIntStatus(USARTy, USART_INT_TXDE) != RESET);//等待发送完成
//}


/**************************************************************************************************
* 名    称：  void RS232Init(uint32_t baudrate)      
* 功能说明：  1、初始化RS232接收帧缓冲区sRS232Queue
*             5、注册RS232串口接收帧回调函数
***************************************************************************************************/
//void RS232Init(uint32_t baudrate) 
//{
//	  InitQueueMem(&sRS232Queue) ;                                                //初始化RS485接收帧缓冲区sRS232Queue
//    UARTx_Init(RS232_COM, baudrate, USART_MODE_RX | USART_MODE_TX, INT_RANK_5) ;//初始化RS485映射的串口RS232_COM，设置为收发模式，中断优先级为INT_RANK_5				
//		Uart_RegHookCallback(RS232_COM, Rs232RecFrameCallback) ;                    //注册RS485接收帧中断回调函数
//}

///**************************************************************************************************
//* 名    称：  void Rs232RecFrameCallback(char *recvBuf, uint16_t recvLen)
//* 功能说明：  当RS485串口接收到数据帧时会回调此函数，此函数将数据帧插入到“帧环形缓冲区”sRS232Queue中
//*             应用层无需关注如何接收，只需要在初始化RS232后，处理sRS232Queue帧数据即可。
//**************************************************************************************************/
//void Rs232RecFrameCallback(char *recvBuf, uint16_t recvLen)
//{
//	if( RW_OK != InsertQueueMemData(&sRS232Queue, recvBuf, recvLen) )
//	  {
//		  ErrorLogPrintf("sRS232Queue溢出！") ;
//		}	
//}

///****************************************************************************
//* 名    称：void RS232SendData(char* sendData, uint16_t dataLen)
//* 功    能：通过RS232_COM端口输出dataLen长度的数据 sendData 
//* 入口参数：
//*           @param *sendData    输出的数据指针
//*           @param dataLen      输出数据的长度
//****************************************************************************/
//void RS232SendData(char* sendData, uint16_t dataLen)
//{
//  UARTx_SendData(RS232_COM, sendData, dataLen) ;
//}

