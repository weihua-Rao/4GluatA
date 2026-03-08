#include "io.h"
//#include <math.h>
#include "n32g45x_exti.h"
//#include "usart.h"
#include "main.h"
/********************************************************************************
  * @file    io.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   提供IO 外部中断相关驱动
  ******************************************************************************
	* @驱动功能：
	* 1、GPIO初始化
	* 2、IOEXTI初始化
	* 3、外部中断回调函数注册函数
	* 4、GPIO输出电平翻转
	* 5、本驱动没有实现输出IO电平，在上册应用中直接使用位带操作更改输出高低电平
  * @使用方法：
	* 1、先调用IoExti_Init()对IO初始化、NVIC、EXTI初始化
	* 2、在上层注册回调函数，编写回调函数
*******************************************************************************/

/*****************************************
*中断事件回调函数相关配置
****************************************/
IOFP  ioFp[PINSUM] = {NULL} ; //EXTI_LINE0~EXTI_LINE15 定义16个IO中断线对应的回调函数指针

/****************************************************************************
* IO EXTI相关配置RCC_APB2_PERIPH_GPIOA
****************************************************************************/
const uint32_t  portRcc[PSUM]      = {RCC_APB2_PERIPH_GPIOA, RCC_APB2_PERIPH_GPIOB, RCC_APB2_PERIPH_GPIOC, RCC_APB2_PERIPH_GPIOD,
                                      RCC_APB2_PERIPH_GPIOE, RCC_APB2_PERIPH_GPIOF, RCC_APB2_PERIPH_GPIOG} ;        //所有IO端口的时钟
GPIO_Module*   portx[PSUM]        = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG} ;                   			 //所有IO端口
const uint16_t  pinx[PINSUM]       = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, 
																			GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, 
																			GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11, 
																			GPIO_PIN_12,GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15} ;                 			 //IO端口下的所有IO引脚
const uint8_t   extiPort[PSUM]     = {GPIOA_PORT_SOURCE, GPIOB_PORT_SOURCE, GPIOC_PORT_SOURCE,
																		  GPIOD_PORT_SOURCE, GPIOE_PORT_SOURCE, GPIOF_PORT_SOURCE,
																		  GPIOG_PORT_SOURCE} ;                                                    //所有中断端口
const uint8_t   extiPin[PINSUM]    = {GPIO_PIN_SOURCE0, GPIO_PIN_SOURCE1, GPIO_PIN_SOURCE2, GPIO_PIN_SOURCE3,
																			GPIO_PIN_SOURCE4, GPIO_PIN_SOURCE5, GPIO_PIN_SOURCE6, GPIO_PIN_SOURCE7,
																			GPIO_PIN_SOURCE8, GPIO_PIN_SOURCE9, GPIO_PIN_SOURCE10, GPIO_PIN_SOURCE11,
																			GPIO_PIN_SOURCE12, GPIO_PIN_SOURCE13, GPIO_PIN_SOURCE14, GPIO_PIN_SOURCE15,} ; //所有IO外部中断引脚
const uint32_t extiLine[PINSUM]    = {EXTI_LINE0, EXTI_LINE1, EXTI_LINE2, EXTI_LINE3,
																			EXTI_LINE4, EXTI_LINE5, EXTI_LINE6, EXTI_LINE7,
																			EXTI_LINE8, EXTI_LINE9, EXTI_LINE10, EXTI_LINE11,
																			EXTI_LINE12, EXTI_LINE13, EXTI_LINE14, EXTI_LINE15} ;                      //所有外部中断线
IRQn_Type  extiChannel[PINSUM] = {EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn,   
																		  EXTI4_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, 
																		  EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, 
																		  EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, } ;        //所有中断通道

/**************************************************************************************************
* 名    称：void Gpio_Init(PORT_e ePortx, PIN_e ePinx, GPIO_ModeType gpioMode)
* 功能说明：GPIO初始化
* 入口参数：
*           @param1 ePortx        PORT_e枚举类型，表示IO端口
*           @param2 ePinx         PIN_e枚举类型数据，表示IO引脚
*           @param3 gpioMode      GPIO_ModeType枚举类型，表示IO引脚模式																		
* 出口参数：无
* 调用方法：外部调用
*************************************************************************************************/ 
void Gpio_Init(PORT_e ePortx, PIN_e ePinx, GPIO_ModeType gpioMode)
{
    /*通用IO引脚初始化*/	
		GPIO_InitType GPIO_InitStructure ;              //GPIO参数 结构体定义		
    RCC_EnableAPB2PeriphClk(portRcc[ePortx] , ENABLE);  //GPIO 端口时钟开启
		GPIO_InitStructure.Pin  = pinx[ePinx] ;       //需要初始化引脚
		GPIO_InitStructure.GPIO_Mode = gpioMode ;          //IO引脚模式
//		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //IO速度配置
		GPIO_InitPeripheral(portx[ePortx], &GPIO_InitStructure);     //GPIO 引脚初始化   
}

/**************************************************************************************************
* 名    称：void IoExti_Init(PORT_e ePortx, PIN_e ePinx, GPIO_ModeType gpioMode, EXTI_TriggerType exitTrigger, 
*           	               IntPriority_e ePriority, IOFP pCallback)
* 功能说明：io初始化。GPIO初始化，判断如果gpioMode开启了外部中断则初始化EXTI和NVIC，接着注册中断回调函数
* 入口参数：
*           @param1 ePortx        PORT_e枚举类型，表示IO端口
*           @param2 ePinx         PIN_e枚举类型数据，表示IO引脚
*           @param3 gpioMode      GPIO_ModeType枚举类型，表示IO引脚模式
*           @param4 exitTrigger   EXTI_TriggerType枚举类型数据，表示IO中断触发模式
*           @param5 ePriority     IntPriority_e枚举类型，表示IO端口中断抢占优先级
*           @param6 pCallback     IOFP类型函数指针																		
* 出口参数：无
* 调用方法：外部调用
* 注    意：《STM32F10XXX参考手册》中P137页中每一个EXTI中断线只能连接个IO引脚，
*             例如：PA9与PC9只能由一个引脚连接到EXTI9中短线上。
*************************************************************************************************/ 
void IoExti_Init(PORT_e ePortx, PIN_e ePinx, GPIO_ModeType gpioMode, EXTI_TriggerType exitTrigger, 
	               IntPriority_e ePriority)
{	
		Gpio_Init(ePortx, ePinx, gpioMode) ;                                    //通用IO引脚初始化       
		if((GPIO_Mode_IPD==gpioMode) || (GPIO_Mode_IPU==gpioMode))              //判断是否开启了IO外部中断
		{
		    /*开启EXIT功能需要 开启AFIO时钟且初始化 EXIT和NVIC*/
				EXTI_InitType EXTI_InitStructure ;      													//EXIT参数 结构体定义	
				NVIC_InitType NVIC_InitStructure ;	  													  //NVIC参数 结构体定义
				RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE); 								//GPIO EXIT功能需要开启AFIO时钟

				NVIC_InitStructure.NVIC_IRQChannel = extiChannel[ePinx] ;	          //外部中断线		         
				NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ePriority ;	//抢占优先级配置 
				NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0 ;					        //freertos无子优先级
				NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE ;						        //失能外部中断通道
				NVIC_Init(&NVIC_InitStructure);                                     //中断NVIC配置
				
				GPIO_ConfigEXTILine( extiPort[ePortx], extiPin[ePinx] ) ;  					//中断Line配置
				EXTI_InitStructure.EXTI_Line    = extiLine[ePinx] ;	       					//中断引脚
				EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt ;	   					//中断模式
				EXTI_InitStructure.EXTI_Trigger = exitTrigger ;            					//IO外部中断触发方式
				EXTI_InitStructure.EXTI_LineCmd = ENABLE;                           //使能外部中断
				EXTI_InitPeripheral( &EXTI_InitStructure );	                                  //外部中断初始化
				
		    /*清中断标志位后再开中断*/		
				EXTI_ClrITPendBit( extiLine[ePinx] ); 
				NVIC_EnableIRQ(extiChannel[ePinx]) ;//IRQn_Type
		}
}

/**************************************************************************************************
* 名    称：void Io_Reverse(PORT_e ePortx, PIN_e ePinx)
* 功能说明：GPIO输出电平翻转
* 入口参数：
*           @param1 ePortx        PORT_e枚举类型，表示IO端口
*           @param2 ePinx         PIN_e枚举类型数据，表示IO引脚																
* 调用方法：外部调用
*************************************************************************************************/ 
void Io_Reverse(PORT_e ePortx, PIN_e ePinx)
{
	portx[ePortx]->POD ^= pinx[ePinx] ;
}

/**************************************************************************************************
* 名    称：IOFP  Io_RegHookCallback(PIN_e ePinx, IOFP pCallback) 
* 功能说明：io外部中断回调函数-注册函数，EXTI驱动中定义了IOFP函数指针类型，并定义了ioFp16个中断指针变量
*           Io_RegHookCallback函数就是将回调函数地址传递给ioFp指针变量
* 入口参数：
*           @param1 ePinx        PIN_e枚举类型数据
*           @param2 pCallback    IOFP类型函数指针
* 出口参数：
*           @param1 pCallback    IOFP类型函数指针
* 调用方法：外部调用
*************************************************************************************************/ 
IOFP  Io_RegHookCallback(PIN_e ePinx, IOFP pCallback) 
{
	if( ioFp[ePinx] == NULL )
		 ioFp[ePinx] = pCallback ;
	else
		 SysErr("IO Callback repeat reg!") ;
  return 	pCallback ;
} 

/**************************************************************************************************
* 名    称：void Io_Hook(uint32_t extiLine)
* 功能说明：io外部中断内调用的钩子函数（执行到中断会把相应的回调函数勾出来运行，嘿嘿嘿。。。
*           当然不同的中断线（extiLine）会勾出不同的回调函数ioFp[]）
* 入口参数：
*           @param1 extiLine     中断线EXTI_Line0~EXTI_Line15
*************************************************************************************************/ 
void Io_Hook(uint32_t extiLine)
 {
	 int n = log(extiLine)/log(2) ;  //对数运算将extiLine的值转化为中断线需要0~15
	 if( ioFp[n] != NULL) 
		   ioFp[n]()  ;                //通过ipFP函数指针跳转到已经注册的回调函数运行
 }
 
/**************************************************************************************************
* 名    称：  void EXTI0_IRQHandler(void)   
* 功能说明：   EXTI0_IRQn中断处理函数  
 *************************************************************************************************/
void EXTI0_IRQHandler(void)
{
	if( EXTI_GetITStatus(EXTI_LINE0) != RESET )
	{
 		EXTI_ClrStatusFlag( EXTI_LINE0 );       //清除LINE0上的中断标志位	
    SysLog("EXTI0 IRQ!") ;		
		Io_Hook(EXTI_LINE0) ;               //跳转到EXTI0中断回调函数运行	
	}
	EXTI_ClrITPendBit( EXTI_LINE0 ); //清除LINE0上的中断标志位 
}

/**************************************************************************************************
* 名    称：  void EXTI1_IRQHandler(void)   
* 功能说明：  EXTI1_IRQn中断处理函数  
  *************************************************************************************************/
void EXTI1_IRQHandler(void)   
{
	if( EXTI_GetITStatus( EXTI_LINE1 ) == SET )
	{
    EXTI_ClrStatusFlag( EXTI_LINE1 );
    SysLog("EXTI1 IRQ!") ;
		Io_Hook(EXTI_LINE1) ;               //EXTI1中断回调函数			
	}
	EXTI_ClrITPendBit( EXTI_LINE1 ); //清除LINE1上的中断标志位 
}

/**************************************************************************************************
* 名    称：  void EXTI2_IRQHandler(void)   
* 功能说明：  EXTI2_IRQn中断处理函数  
 *************************************************************************************************/
void EXTI2_IRQHandler(void)
{
	if( EXTI_GetITStatus(EXTI_LINE2) != RESET )
	{
 		EXTI_ClrStatusFlag( EXTI_LINE2 );       //清除LINE2上的中断标志位 
    SysLog("EXTI2 IRQ!") ;		
		Io_Hook(EXTI_LINE2) ;               //EXTI2中断回调函数			
	}
	EXTI_ClrITPendBit( EXTI_LINE2 ); //清除LINE2上的中断标志位 
}

/**************************************************************************************************
* 名    称：  void EXTI3_IRQHandler(void)   
* 功能说明：  EXTI3_IRQn中断处理函数  
*************************************************************************************************/
void EXTI3_IRQHandler(void)   
{
	if( EXTI_GetITStatus( EXTI_LINE3 ) == SET )
	{
    EXTI_ClrStatusFlag( EXTI_LINE3 );
    SysLog("EXTI3 IRQ!") ;
		Io_Hook(EXTI_LINE3) ;               //EXTI3中断回调函数			
	}
	EXTI_ClrITPendBit( EXTI_LINE3 ); //清除LINE3上的中断标志位 
}

/**************************************************************************************************
* 名    称：  void EXTI4_IRQHandler(void)   
* 功能说明：  EXTI4_IRQn中断处理函数  
 *************************************************************************************************/
void EXTI4_IRQHandler(void)
{
	if( EXTI_GetITStatus(EXTI_LINE4) != RESET )
	{
 		EXTI_ClrStatusFlag( EXTI_LINE4 );       //清除LINE4上的中断标志位 
    SysLog("EXTI4 IRQ!") ;		
		Io_Hook(EXTI_LINE4) ;               //EXTI4中断回调函数
	}
	EXTI_ClrITPendBit( EXTI_LINE4 ); //清除LINE4上的中断标志位 
}

/**************************************************************************************************
* 名    称：  void EXTI9_5_IRQHandler(void)   
* 功能说明：  EXTI_LINE5~EXTI_LINE9中断处理函数
  *************************************************************************************************/
void EXTI9_5_IRQHandler(void)
{
	for( int n=5; n<=9; n++ )
	{
	  if( EXTI_GetITStatus(extiLine[n]) != RESET )
			{
				EXTI_ClrStatusFlag( extiLine[n] ); //清除EXTI9_5上的中断标志位 
				char log[64] = {0} ;
				snprintf(log, 64, "EXTI%d IRQ!", n) ;				
        SysLog(log) ;				
				Io_Hook(extiLine[n]) ;         //EXTI9_5中断回调函数
			}
	}		
}

/**************************************************************************************************
* 名    称：  void EXTI15_10_IRQHandler(void)   
* 功能说明：  EXTI_LINE15~EXTI_LINE10中断处理函数 
  *************************************************************************************************/
void EXTI15_10_IRQHandler(void)
{
for( int n=10; n<=15; n++ )
	{
	  if( EXTI_GetITStatus(extiLine[n]) != RESET )
			{
				EXTI_ClrStatusFlag( extiLine[n] ); //清除EXTI15_10上的中断标志位  
				char log[64] = {0} ;
				snprintf(log, 64, "EXTI%d IRQ!", n) ;				
        SysLog(log) ;			
				Io_Hook(extiLine[n]) ;         //EXTI15_10中断回调函数
			}
	}
}







