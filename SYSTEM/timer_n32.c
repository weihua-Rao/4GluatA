#define timer_io_c
#include "n32g45x.h"
//#include "PhoneCode.h"
//u8 relay_delay = 0;			 //继电器联动防抖时长
TIM_TimeBaseInitType TIM_TimeBaseStructure;
OCInitType TIM_OCInitStructure;
//uint16_t PrescalerValue = 0;
u8 SingeDelay = 1;	   //信号防抖采集的次数

/**
 * @brief  Configures tim1 clocks.
 *频率Fpwm = TUMER_CLK/[(arr+1)*(psc+1)/]
 *TIMER_CLK是挂上系统时钟APBX总线上的时钟
 *arr是设置在下一个更新事件装入活动的自动重装寄存器周期的值
 *psc是预分频值
 频率单位 :hz 周期单位:s
 */
 
void TIM_Configuration(void)
{
    /* Compute the prescaler value */
//    PrescalerValue = (uint16_t) (SystemCoreClock / 12000000) - 1;
//PrescalerValue = (uint16_t)(SystemCoreClock / 12000) - 1;
    /* Time base configuration */
	
    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);    
    TIM_TimeBaseStructure.Period    = 2600-1;// 2600 2000~~2800-1
    TIM_TimeBaseStructure.Prescaler = 9-1;
    TIM_TimeBaseStructure.ClkDiv    = 0;
    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;

    TIM_InitTimeBase(TIM1, &TIM_TimeBaseStructure);
    /* TIM1 enable update irq */
    TIM_ConfigInt(TIM1, TIM_INT_UPDATE, ENABLE);
    /* TIM1 enable counter */
    TIM_Enable(TIM1, ENABLE);

/* TIM2 configuration ------------------------------------------------------*/
    /* Time Base configuration */
    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.Period    = 0x8FF; // 0x8FF;
    TIM_TimeBaseStructure.Prescaler = 10000;//0x4
    TIM_TimeBaseStructure.ClkDiv    = 0x0;
    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;
    TIM_InitTimeBase(TIM2, &TIM_TimeBaseStructure);	
//    /* TIM2 enable update irq */
//    TIM_ConfigInt(TIM2, TIM_INT_UPDATE, ENABLE);
/* TIM1 channel1 configuration in PWM mode */
    TIM_OCInitStructure.OcMode      = TIM_OCMODE_PWM2;
    TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
    TIM_OCInitStructure.Pulse       = 0x7F;//0x7F
    TIM_OCInitStructure.OcPolarity  = TIM_OC_POLARITY_LOW;
    TIM_InitOc1(TIM2, &TIM_OCInitStructure);
    TIM_InitOc2(TIM2, &TIM_OCInitStructure);
    TIM_InitOc4(TIM2, &TIM_OCInitStructure);
/* TIM2 counter enable */
    TIM_Enable(TIM2, ENABLE);
/* TIM1 main Output Enable */
    TIM_EnableCtrlPwmOutputs(TIM2, ENABLE);		
	/**
 * @brief  Configures tim2 clocks. 54000000hz
 */
    /* Time base configuration */
//    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);    
//    TIM_TimeBaseStructure.Period    = 50000-1;
//    TIM_TimeBaseStructure.Prescaler = 108-1;
//    TIM_TimeBaseStructure.ClkDiv    = 0;
//    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;

//    TIM_InitTimeBase(TIM2, &TIM_TimeBaseStructure);

//    /* Prescaler configuration */
////    TIM_ConfigPrescaler(TIM2, PrescalerValue, TIM_PSC_RELOAD_MODE_IMMEDIATE);

//    /* TIM2 enable update irq */
//    TIM_ConfigInt(TIM2, TIM_INT_UPDATE, ENABLE);

//    /* TIM2 enable counter */
//    TIM_Enable(TIM2, ENABLE);
//		
///**
// * @brief  Configures tim3 clocks.
// */
//    /* Time base configuration */
//    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);    
//    TIM_TimeBaseStructure.Period    = 40000-1;
//    TIM_TimeBaseStructure.Prescaler = 27-1;
//    TIM_TimeBaseStructure.ClkDiv    = 0;
//    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;

//    TIM_InitTimeBase(TIM3, &TIM_TimeBaseStructure);

//    /* Prescaler configuration */
////    TIM_ConfigPrescaler(TIM3, PrescalerValue, TIM_PSC_RELOAD_MODE_IMMEDIATE);

//    /* TIM2 enable update irq */
//    TIM_ConfigInt(TIM3, TIM_INT_UPDATE, ENABLE);

//    /* TIM2 enable counter */
//    TIM_Enable(TIM3, ENABLE);
//		
///**
// * @brief  Configures tim5 clocks.
// */
//    /* Compute the prescaler value */
////    PrescalerValue = 0; //(uint16_t) (SystemCoreClock / 12000000) - 1;

//    /* Time base configuration */
//    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);
//    TIM_TimeBaseStructure.Period    = 60000-1;
//    TIM_TimeBaseStructure.Prescaler = 30-1;
//    TIM_TimeBaseStructure.ClkDiv    = 0;
//    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;

//    TIM_InitTimeBase(TIM5, &TIM_TimeBaseStructure);

//    /* Prescaler configuration */
////    TIM_ConfigPrescaler(TIM5, PrescalerValue, TIM_PSC_RELOAD_MODE_IMMEDIATE);

//    /* TIM5 enable update irq */
//    TIM_ConfigInt(TIM5, TIM_INT_UPDATE, ENABLE);

//    /* TIM5 enable counter */
//    TIM_Enable(TIM5, ENABLE);
}

ADC_InitType ADC_InitStructure;
DMA_InitType DMA_InitStructure;
//__IO uint16_t ADC1ConvertedValue[5],ADC2ConvertedValue[5],ADC3ConvertedValue[5],ADC4ConvertedValue[5];

//#define ADC_MDA2chl5_EN  //ADC3 和 UART4_TX 映射同一个DMA2_5通道的冲突，故ADC3放弃使用DMA模式
void ADC_Initial(ADC_Module* ADCx)
{

/* ADC configuration ------------------------------------------------------*/
    ADC_InitStructure.WorkMode       = ADC_WORKMODE_INDEPENDENT;
    ADC_InitStructure.MultiChEn      = DISABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;//ADC_EXT_TRIGCONV_T2_CC3;
    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber      = 1;
    ADC_Init(ADCx, &ADC_InitStructure);

/* DMA2 Channel5 Configuration ----------------------------------------------*/
#ifdef 	ADC_MDA2chl5_EN	
    DMA_DeInit(DMA2_CH5);
    DMA_InitStructure.PeriphAddr     = (uint32_t)&ADC3->DAT;
    DMA_InitStructure.MemAddr        = (uint32_t)ADC_convertedValue;
    DMA_InitStructure.Direction      = DMA_DIR_PERIPH_SRC;
    DMA_InitStructure.BufSize        = 3;
    DMA_InitStructure.PeriphInc      = DMA_PERIPH_INC_DISABLE;
    DMA_InitStructure.DMA_MemoryInc  = DMA_MEM_INC_ENABLE;
    DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_HALFWORD;
    DMA_InitStructure.MemDataSize    = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.CircularMode   = DMA_MODE_CIRCULAR;
    DMA_InitStructure.Priority       = DMA_PRIORITY_HIGH;
    DMA_InitStructure.Mem2Mem        = DMA_M2M_DISABLE;
    DMA_Init(DMA2_CH5, &DMA_InitStructure);
    /* Enable DMA2 channel5 */
    DMA_EnableChannel(DMA2_CH5, ENABLE);	

    /* ADC2 regular channel14 configuration */
    ADC_ConfigRegularChannel(ADCx, ADC3_Channel_05_PB13, 1, ADC_SAMP_TIME_13CYCLES5);
    /* Enable ADCx DMA */
    ADC_EnableDMA(ADCx, ENABLE);
    /* Enable ADCx external trigger */
    ADC_EnableExternalTrigConv(ADCx, ENABLE);
#endif		

/* Enable ADC */
    ADC_Enable(ADCx, ENABLE);
    /*Check ADC Ready*/
    while(ADC_GetFlagStatusNew(ADCx,ADC_FLAG_RDY) == RESET)
        ;
    /* Start ADC calibration */
    ADC_StartCalibration(ADCx);
    /* Check the end of ADC calibration */
    while (ADC_GetCalibrationStatus(ADCx))
        ;		
}

uint16_t ADC_GetData(ADC_Module* ADCx, uint8_t ADC_Channel)
{
    uint16_t dat;
    
    ADC_ConfigRegularChannel(ADCx, ADC_Channel, 1, ADC_SAMP_TIME_239CYCLES5);
    /* Start ADC Software Conversion */
    ADC_EnableSoftwareStartConv(ADCx, ENABLE);
    while(ADC_GetFlagStatus(ADCx, ADC_FLAG_ENDC)==0){
    }
    ADC_ClearFlag(ADCx, ADC_FLAG_ENDC);
    ADC_ClearFlag(ADCx, ADC_FLAG_STR);
    dat=ADC_GetDat(ADCx);
    return dat;
}

 

