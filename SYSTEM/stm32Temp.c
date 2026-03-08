#include "stm32Temp.h"
#include "stm32f10x_adc.h"
#include "sysport.h"

/********************概*********************述*******************************
*                  STM32内部自带温度传感器 通过ADC1可以获取到MCU温度
*                  通过STM32内部自带的温度传感器获取MCU温度
****************************************************************************/

/********************************************************************************
  * @file    stm32Temp.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   STM32内部自带温度传感器 通过ADC1可以获取到MCU温度
  ******************************************************************************
*******************************************************************************/

/**************************************************************************************************
* 名    称：  void T_Adc_Init(void)  //ADC通道初始化
* 功能说明：  ADC1初始化
  *************************************************************************************************/
void T_Adc_Init(void)  //ADC通道初始化
{
		ADC_InitTypeDef ADC_InitStructure; 
//		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1	, ENABLE );	  //使能GPIOA,ADC1通道时钟
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC |RCC_APB2Periph_ADC1	, ENABLE );	  //使能GPIOA,ADC1通道时钟
		RCC_ADCCLKConfig(RCC_PCLK2_Div6);                                     //分频因子6时钟为72M/6=12MHz

		ADC_DeInit(ADC1);                                                     //将外设 ADC1 的全部寄存器重设为缺省值	 
		ADC_InitStructure.ADC_Mode = ADC_Mode_Independent ;	                  //ADC工作模式:ADC1和ADC2工作在独立模式
		ADC_InitStructure.ADC_ScanConvMode = DISABLE ;	                      //模数转换工作在单通道模式
		ADC_InitStructure.ADC_ContinuousConvMode = ENABLE ;	                //DISABLE 模数转换工作在单次转换模式
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None ;	//转换由软件而不是外部触发启动
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right ;	              //ADC数据右对齐
		ADC_InitStructure.ADC_NbrOfChannel = 1 ;	                            //顺序进行规则转换的ADC通道的数目
		ADC_Init(ADC1, &ADC_InitStructure) ;        	                        //根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器
//ADC_RegularChannelConfig (ADC1,ADC_Channel_10,1,ADC_SampleTime_7Cycles5);
//		ADC_TempSensorVrefintCmd(ENABLE) ;                                    //开启内部温度传感器 
		ADC_Cmd(ADC1, ENABLE) ;	                                              //使能指定的ADC1
		ADC_ResetCalibration(ADC1) ;	                                        //重置指定的ADC1的复位寄存器
		while(ADC_GetResetCalibrationStatus(ADC1)) ;	                        //获取ADC1重置校准寄存器的状态,设置状态则等待
		ADC_StartCalibration(ADC1) ;	                
		while(ADC_GetCalibrationStatus(ADC1)) ;		                            //获取指定ADC1的校准程序,设置状态则等待
}

/**************************************************************************************************
* 名    称：  uint16_t T_Get_Adc(uint8_t ch)
* 功能说明：  获取相应通道的ADC转化值
* 入口参数：  
*				      @param1  ch: 通道
* 出口参数：  
*				      @param1  uint16_t: ch通道ADC转化的值
  *************************************************************************************************/
uint16_t T_Get_Adc(uint8_t ch)   
	{ 
		ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	 //ADC1,ADC通道3,第一个转换,采样时间为239.5周期	  			     
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);		                           //使能指定的ADC1的软件转换启动功能
		while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));                      //等待转换结束
		return ADC_GetConversionValue(ADC1);	                               //返回最近一次ADC1规则组的转换结果
	}

/**************************************************************************************************
* 名    称：  uint16_t T_Get_Temp(void)
* 功能说明：  对ADC1通道16，ADC转化5次取平均值
* 入口参数：  无
* 出口参数：  
*				      @param1  uint16_t: ADC1通道16，ADC转化5次返回的平均值
  *************************************************************************************************/
uint16_t T_Get_Temp(void)
	{
		uint16_t temp_val = 0 ;
		uint8_t t;
		for( t=0; t<5; t++ )
			{
					temp_val += T_Get_Adc(ADC_Channel_16);	  //TampSensor
					Wait_For_Nms(10);
			}
		return temp_val/5;
	}

	/**************************************************************************************************
* 名    称：  uint16_t T_Get_Adc_Average( uint8_t ch, uint8_t times)
* 功能说明：  对ADC1通道ch，ADC转化times次取平均值
* 入口参数：  
*				      @param1  ch: 通道
*				      @param2  times: 采样次数         
* 出口参数：  
*				      @param1  uint16_t: 对ADC1通道ch，ADC转化times次返回的平均值
  *************************************************************************************************/
uint16_t T_Get_Adc_Average( uint8_t ch, uint8_t times)
{
	uint32_t temp_val=0;
	uint8_t t;
	for( t=0; t<times; t++ )
	{
		temp_val += T_Get_Adc(ch) ;
		Wait_For_Nms(10) ;
	}
	return temp_val/times ;
} 	   

	/**************************************************************************************************
* 名    称：  short Get_Temprate(void)	//获取内部温度传感器温度值
* 功能说明：  获取内部温度传感器温度值的100倍     
* 出口参数：  short：温度值(扩大了100倍,单位:℃.)
  *************************************************************************************************/
short Get_Temprate(void)	//获取内部温度传感器温度值
{
	uint32_t adcx ;
	short result ;
 	double temperate ;
	adcx=T_Get_Adc_Average(ADC_Channel_16,10) ;	//读取通道16,20次取平均
	temperate = (float)adcx*(3.3/4096) ;		    //电压值 
	temperate = (1.43-temperate)/0.0043+25 ;	  //转换为温度值 	 
	result = temperate*=100 ;				          	//扩大100倍.
	return  result ;
}

