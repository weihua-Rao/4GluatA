#ifndef _RS232_H_
#define _RS232_H_
//#include "n32g43x.h"
#include "usart.h"
//#include "PhoneCode.h"
#include "syslib.h"
/****************************************************************************
* 系统层面错误LOG输出
****************************************************************************/
//#define SysErr(...)   {printf("LogErr: %s,%d",__FILE__,  __LINE__); printf(__VA_ARGS__) ;}  //输出系统错误
//#ifdef  OPENSYSLOG
//#define SysLog(...)   {printf("LogSys: ");  printf(__VA_ARGS__) ;}                         //输出系统日志 
//#else
//#define SysLog(...)
//#endif


//#define _USART1_USART2_

//#ifdef _USART1_USART2_
//#define USARTy            USART1
//#define USARTy_GPIO       GPIOA
#define USARTy_CLK        RCC_APB2_PERIPH_USART1
#define USARTy_GPIO_CLK   RCC_APB2_PERIPH_GPIOA
//#define USARTy_RxPin      GPIO_PIN_10
//#define USARTy_TxPin      GPIO_PIN_9
//#define USARTy_RTSPin      GPIO_PIN_12
//#define USARTy_CTSPin      GPIO_PIN_11
//#define USARTy_Rx_GPIO_AF  GPIO_AF4_USART1
//#define USARTy_Tx_GPIO_AF  GPIO_AF4_USART1
//#define USARTy_RTS_GPIO_AF GPIO_AF4_USART1
//#define USARTy_CTS_GPIO_AF GPIO_AF4_USART1
#define USARTy_APBxClkCmd RCC_EnableAPB2PeriphClk
//#define USARTy_IRQn       USART1_IRQn
//#define USARTy_IRQHandler USART1_IRQHandler

//#define USARTz             USART2
//#define USARTz_GPIO        GPIOA
//#define USARTz_CLK         RCC_APB1_PERIPH_USART2
//#define USARTz_GPIO_CLK    RCC_APB2_PERIPH_GPIOA
//#define USARTz_RxPin       GPIO_PIN_3
//#define USARTz_TxPin       GPIO_PIN_2
//#define USARTz_RTSPin      GPIO_PIN_1
//#define USARTz_CTSPin      GPIO_PIN_0
//#define USARTz_Rx_GPIO_AF  GPIO_AF4_USART2
//#define USARTz_Tx_GPIO_AF  GPIO_AF4_USART2
//#define USARTz_RTS_GPIO_AF GPIO_AF4_USART2
//#define USARTz_CTS_GPIO_AF GPIO_AF4_USART2
//#define USARTz_APBxClkCmd RCC_EnableAPB1PeriphClk
//#define USARTz_IRQn       USART2_IRQn
//#define USARTz_IRQHandler USART2_IRQHandler
//#endif
//*************************UART*****************************//

//#define	COM_TX2_Lenth	20
//#define	COM_RX2_Lenth	200

//#define	USART1	1
//#define	USART2	2

//#define	UART_ShiftRight	0		//同步移位输出
//#define	UART_8bit_BRTx	(1<<6)	//8位数据,可变波特率
//#define	UART_9bit		(2<<6)	//9位数据,固定波特率
//#define	UART_9bit_BRTx	(3<<6)	//9位数据,可变波特率

//#define	UART1_SW_P30_P31	0
//#define	UART1_SW_P36_P37	(1<<6)
//#define	UART1_SW_P16_P17	(2<<6)	//必须使用内部时钟
//#define	UART2_SW_P10_P11	0
//#define	UART2_SW_P46_P47	1

//串口数据结构
//#define	COM_TX1_Lenth	250
//#define	COM_RX1_Lenth	250

//typedef struct
//{ 
//	u8	id;				//串口号

//	u8	TX_read;		//发送读指针
//	u8	TX_write;		//发送写指针
//	u8	B_TX_busy;		//忙标志

//	u8 	RX_Cnt;			//接收字节计数
//	u8	RX_TimeOut;		//接收超时
//	u8	B_RX_OK;		//接收块完成
//} COMx_Define; 

//typedef struct
//{ 
//	u8	UART_Mode;			//模式,         UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
//	u8	UART_BRT_Use;		//使用波特率,   BRT_Timer1,BRT_Timer2
//	u32	UART_BaudRate;		//波特率,       ENABLE,DISABLE
//	u8	Morecommunicate;	//多机通讯允许, ENABLE,DISABLE
//	u8	UART_RxEnable;		//允许接收,   ENABLE,DISABLE
//	u8	BaudRateDouble;		//波特率加倍, ENABLE,DISABLE
//	u8	UART_Interrupt;		//中断控制,   ENABLE,DISABLE
//	u8	UART_Polity;		//优先级,     PolityLow,PolityHigh
//	u8	UART_P_SW;			//切换端口,   UART1_SW_P30_P31,UART1_SW_P36_P37,UART1_SW_P16_P17(必须使用内部时钟)
//	u8	UART_RXD_TXD_Short;	//内部短路RXD与TXD, 做中继, ENABLE,DISABLE

//} COMx_InitDefine;

#define TxBufferSize2 (countof(TxBuffer2) - 1)
#define RxBufferSize1 TxBufferSize2
#define RxBufferSize2 TxBufferSize1


//Extern_def uint8_t RC1_Received_Flag; //,RC2_Received_Flag
//Extern_def COMx_Define COM1,COM2;
//Extern_def uint8_t Usart_RX_bety;
//extern USART_InitType USART_InitStructure;

void RCC_Configuration(void);
//void Uart1Sends(char *str);
//void Uart1BYTE(char temp);
//void TX1_WriteBuff(u8 dat);
//void USART_Config(void);
//void RUTX_Check(void);
//void PrintString1(u8 *puts);
//void PrintString2(u8 *puts);

/*****************************************
*驱动可供外部使用的常变量
****************************************/
//#define RS232_COM           COM5               //RS232映射的串口
//extern FrameQueue_s         sRS232Queue ;      //RS232 接收数据帧数据环形缓冲区，上层只需要在初始化RS232后，处理该缓冲区数据即可
/*****************************************
*驱动可供外部使用的常变量
****************************************/
                                      

/*****************************************
*内部函数声明
****************************************/
//void Rs232RecFrameCallback(char *recvBuf, uint16_t recvLen) ;  //RS232接收帧数据回调函数

/*****************************************
*对外接口函数声明
****************************************/
//void RS232Init(uint32_t baudrate) ;                     //RS232串口初始化
extern void RS232SendData(char* sendData, uint16_t dataLen) ;  //通过RS232_COM端口输出dataLen长度的数据 sendData 

#endif
