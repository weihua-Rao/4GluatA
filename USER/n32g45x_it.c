/*****************************************************************************
 * Copyright (c) 2022, Nations Technologies Inc.
 *
 * All rights reserved.
 * ****************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Nations' name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY NATIONS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL NATIONS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ****************************************************************************/

/**
 * @file n32g43x_it.c
 * @author Nations
 * @version V1.2.2
 *
 * @copyright Copyright (c) 2022, Nations Technologies Inc. All rights reserved.
 */
#include "n32g45x_it.h"
#include "main.h"

/** @addtogroup N32G43X_StdPeriph_Template
 * @{
 */

//extern uint8_t TxBuffer1[];
//extern uint8_t TxBuffer2[];
//extern uint8_t RxBuffer1[];
//extern uint8_t RxBuffer2[];
extern __IO uint8_t TxCounter1;
extern __IO uint8_t TxCounter2;
extern __IO uint8_t RxCounter1;
extern __IO uint8_t RxCounter2;
extern uint8_t NbrOfDataToTransfer1;
extern uint8_t NbrOfDataToTransfer2;
extern uint8_t NbrOfDataToRead1;
extern uint8_t NbrOfDataToRead2;
extern __IO uint32_t LsiFreq;
/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief  This function handles NMI exception.
 */
void NMI_Handler(void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Memory Manage exception.
 */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Bus Fault exception.
 */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Usage Fault exception.
 */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles SVCall exception.
 */
//void SVC_Handler(void)
//{
//}

/**
 * @brief  This function handles Debug Monitor exception.
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  This function handles SysTick Handler.
 */
//void SysTick_Handler(void)
//{
//	TimingDelay--;
//}
/******************************************************************************/
/*                 N32G43x Peripherals Interrupt Handlers                     */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_n32g43x.s).                                                 */
/******************************************************************************/

/**
 * @brief  This function handles TIM1 update interrupt request.
 */
void TIM1_UP_IRQHandler(void)
{
  if (TIM_GetIntStatus(TIM1, TIM_INT_UPDATE) != RESET)
  {
    TIM_ClrIntPendingBit(TIM1, TIM_INT_UPDATE);
		Wireless_Receive();
//			if(!UserQuery_KEY)UserKey.CloseState = 0;
//			else UserKey.CloseState = 0;
//			Tled = !Tled;
//			Rx_433_IO = ~Rx_433_IO;
			
//	static u16 QueryDelay;
		if(!UserQuery_KEY)	   //用户按健按下闭合有效 下降沿
		{ 
			if(++UserKey.QueryCnt_C > UserKey.QueryDelay)
			{
				UserKey.QueryDelay = 5000;		//长按延时5有效
				if(UserKey.CloseState)UserKey.QueryDelay = 200;//后执行翻页速度
				UserKey.CloseState = UserKey.LastValuae=1;
				UserKey.QueryCnt_C = 0;
				UserKey.KeyExeFlag = 1;
			}
			UserKey.QueryCnt_O = 0;
		}	
		else if((UserKey.LastValuae)&&(UserQuery_KEY))	//按键松开有效 上升沿 超过延时返回首页
		{ 
			UserKey.CloseState = 0;UserKey.QueryDelay = 50;
				if(++UserKey.QueryCnt_O > KeyReturnDelay)					//延时30000返回首页
				{
					UserKey.KeyExeFlag = 2;UserKey.LastValuae = UserKey.QueryCnt_O = 0;
				}
				UserKey.QueryCnt_C = 0;
		}			
  }
}

/**
 * @brief  This function handles USARTy global interrupt request.
 */
//void USARTy_IRQHandler(void)
//{
//    if (USART_GetIntStatus(USARTy, USART_INT_RXDNE) != RESET)
//    {

//		}


//    if(USART_GetIntStatus(USARTy, USART_INT_OREF) != RESET)
//    {
//        /*Read the STS register first,and the read the DAT 
//        register to clear the overflow interrupt*/
//        (void)USARTy->STS;
//        (void)USARTy->DAT;
//    }
//}

/**
 * @brief  This function handles USARTz global interrupt request.
 */
void USARTz_IRQHandler(void)
{
//    if (USART_GetIntStatus(USARTz, USART_INT_RXDNE) != RESET)
//    {
//        /* Read one byte from the receive data register */
//        RxBuffer2[RxCounter2++] = USART_ReceiveData(USARTz);

//        if (RxCounter2 == NbrOfDataToRead1)
//        {
//            /* Disable the USARTz Receive interrupt */
//            USART_ConfigInt(USARTz, USART_INT_RXDNE, DISABLE);
//        }
//    }
//    if (USART_GetIntStatus(USARTz, USART_INT_TXDE) != RESET)
//    {
//        /* Write one byte to the transmit data register */
//        USART_SendData(USARTz, TxBuffer2[TxCounter2++]);

//        if (TxCounter2 == NbrOfDataToTransfer2)
//        {
//            /* Disable the USARTz Transmit interrupt */
//            USART_ConfigInt(USARTz, USART_INT_TXDE, DISABLE);
//        }
//    }
//    if(USART_GetIntStatus(USARTz, USART_INT_OREF) != RESET)
//    {
//        /*Read the STS register first,and the read the DAT 
//        register to clear the overflow interrupt*/
//        (void)USARTz->STS;
//        (void)USARTz->DAT;
//    }
}

/**
 * @brief  This function handles TIM2 global interrupt request.
 */
void TIM2_IRQHandler(void)		//100ms 10hz
{
if (TIM_GetIntStatus(TIM2, TIM_INT_UPDATE) != RESET)
	{
		TIM_ClrIntPendingBit(TIM2, TIM_INT_UPDATE);
	}
}

/**
 * @brief  This function handles TIM2 global interrupt request.
 */
void TIM3_IRQHandler(void)		//20ms 50hz  (17.2ms)
{
	if (TIM_GetIntStatus(TIM3, TIM_INT_UPDATE) != RESET)
	{
		TIM_ClrIntPendingBit(TIM3, TIM_INT_UPDATE);

	}
}

/**
 * @brief  This function handles TIM2 global interrupt request.
 */
void TIM5_IRQHandler(void)		//20ms 50hz  (17.2ms)
{
	if (TIM_GetIntStatus(TIM5, TIM_INT_UPDATE) != RESET)
	{
		TIM_ClrIntPendingBit(TIM5, TIM_INT_UPDATE);
//Tled = !Tled;
	}
}
/**
 * @brief  This function handles TIM9 global interrupt request.
 */
//void TIM9_IRQHandler(void)
//{
//    if (TIM_GetIntStatus(TIM9, TIM_INT_CC3) != RESET)
//    {
//        /* Clear TIM9 Capture compare interrupt pending bit */
//        TIM_ClrIntPendingBit(TIM9, TIM_INT_CC3);
//    }
//}

/* 如果使用堆栈溢出钩子，则提供实现 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    /* 在这里实现你的处理逻辑 */
    (void)xTask;   /* 避免编译器警告 */
    (void)pcTaskName;
 
    /* 比如记录日志，或者重启系统等 */
}
/******************************************************************************/
/*                 N32G45X Peripherals Interrupt Handlers                     */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_n32g45x.s).                                                 */
/******************************************************************************/

/**
 * @brief  This function handles External interrupt Line 0 request.
 */
//void EXTI0_IRQHandler(void)
//{
//    Delay(0x5FFFFF);
//    if (ControlFlag == 2)
//    {
//        ControlFlag = 0;
//    }

//    if (ControlFlag == 0)
//    {
//        /* Flush DAT register and clear the USARTz RXNE flag */
//        USART_ReceiveData(USARTz);
//        /* Enable the USARTz mute mode*/
//        USART_EnableRcvWakeUp(USARTz, ENABLE);

//        ControlFlag = 1;
//    }
//    else if (ControlFlag == 1)
//    {
//        /* Send the address mark (0x102) to wakeup USARTz */
//        USART_SendData(USARTy, 0x102);
//        /* Wait while USARTy TXE = 0 */
//        while (USART_GetFlagStatus(USARTy, USART_FLAG_TXDE) == RESET)
//        {
//        }

//        ControlFlag = 2;
//    }

//    /* Clear Key Button EXTI Line Pending Bit */
//    EXTI_ClrITPendBit(EXTI_LINE0);
//}

