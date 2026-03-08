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
 * @file n32g43x_it.h
 * @author Nations
 * @version V1.2.2
 *
 * @copyright Copyright (c) 2022, Nations Technologies Inc. All rights reserved.
 */
#ifndef __N32G43X_IT_H__
#define __N32G43X_IT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g45x.h"

typedef enum
{ 
	INT_RANK_0 =  0 ,
  INT_RANK_1 =  1 ,
  INT_RANK_2 =  2 ,
  INT_RANK_3 =  3 ,
  INT_RANK_4 =  4 ,
  INT_RANK_5 =  5 ,
  INT_RANK_6 =  6 ,
	INT_RANK_7 =  7 ,
  INT_RANK_8 =  8 ,
  INT_RANK_9 =  9 ,
  INT_RANK_10 = 10 ,
  INT_RANK_11 = 11 ,
  INT_RANK_12 = 12,
	INT_RANK_13 = 13,
  INT_RANK_14 = 14,
  INT_RANK_15 = 15
}IntPriority_e ;  //FreeRTOS 中断分组选用组4所以，没有子优先级，只有16级抢占优先级

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
//void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
//void SysTick_Handler(void);
//void USART1_IRQHandler(void);
//void USART2_IRQHandler(void);
//void USART3_IRQHandler(void);
//void UART4_IRQHandler(void);
//void UART5_IRQHandler(void);
#ifdef __cplusplus
}
#endif

#endif /* __N32G43X_IT_H__ */
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
