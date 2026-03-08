#include "main.h"
#include "cmsis_armcc.h"
//#include "I2C.h"
//#include "log.h"
/** @addtogroup I2C_Master
 * @{
 */

#ifdef NON_REENTRANT
static uint32_t Mutex_Flag = 0;
#endif

volatile TestStatus test_status   = FAILED;

 __IO uint32_t I2CTimeout;
CommCtrl_t Comm_Flag = C_READY;

static uint8_t RCC_RESET_Flag = 0;

int i2c_master_init(void)
{
    I2C_InitType i2c1_master;
    GPIO_InitType i2c1_gpio;
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_I2C1, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
#ifdef I2C1_REMAP
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);
    GPIO_ConfigPinRemap(GPIO_RMP_I2C1, ENABLE);
#endif
    /*PB6/PB8 -- SCL; PB7/PB9 -- SDA*/
    i2c1_gpio.Pin        = I2Cx_SCL_PIN | I2Cx_SDA_PIN;
    i2c1_gpio.GPIO_Speed = GPIO_Speed_2MHz;
    i2c1_gpio.GPIO_Mode  = GPIO_Mode_AF_OD;
    GPIO_InitPeripheral(GPIOx, &i2c1_gpio);

    I2C_DeInit(I2C1);
    i2c1_master.BusMode     = I2C_BUSMODE_I2C;
    i2c1_master.FmDutyCycle = I2C_FMDUTYCYCLE_2;
    i2c1_master.OwnAddr1    = I2C_MASTER_ADDR;
    i2c1_master.AckEnable   = I2C_ACKEN;
    i2c1_master.AddrMode    = I2C_ADDR_MODE_7BIT;
    i2c1_master.ClkSpeed    = 300000; //100000 100K

    I2C_Init(I2C1, &i2c1_master);
    I2C_Enable(I2C1, ENABLE);
  
	  return 0;	
}

//int i2c_master_send(uint8_t* data, u8 TargetAddr, int len)
//{
//    uint8_t* sendBufferPtr = data;
//    
//#ifdef NON_REENTRANT
//    if (Mutex_Flag)
//        return -1;
//    else
//        Mutex_Flag = 1;
//#endif
//    
//    I2CTimeout             = I2CT_LONG_TIMEOUT;
//    while (I2C_GetFlag(I2C1, I2C_FLAG_BUSY))
//    {
//        if ((I2CTimeout--) == 0)
//        {
//            CommTimeOut_CallBack(MASTER_BUSY);
//        }
//    }
//    
//    if (Comm_Flag == C_READY)
//    {
//        Comm_Flag = C_START_BIT;
//        I2C_GenerateStart(I2C1, ENABLE);
//    }
//    
//    I2CTimeout = I2CT_LONG_TIMEOUT;
//    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_MODE_FLAG)) // EV5
//    {
//        if ((I2CTimeout--) == 0)
//        {
//            CommTimeOut_CallBack(MASTER_MODE);
//        }
//    }
//    
//    I2C_SendAddr7bit(I2C1, TargetAddr , I2C_DIRECTION_SEND);   
//    I2CTimeout = I2CT_LONG_TIMEOUT;
//    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_TXMODE_FLAG)) // EV6
//    {
//        if ((I2CTimeout--) == 0)
//        {
//            CommTimeOut_CallBack(MASTER_TXMODE);
//        }
//    }
//    Comm_Flag = C_READY;
//    
//    // send data
//    while (len-- > 0)
//    {
//        I2C_SendData(I2C1, *sendBufferPtr++);
//        I2CTimeout = I2CT_LONG_TIMEOUT;
//        while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDING)) // EV8
//        {
//            if ((I2CTimeout--) == 0)
//            {
//                CommTimeOut_CallBack(MASTER_SENDING);
//            }
//        }
//    }

//    I2CTimeout = I2CT_LONG_TIMEOUT;
//    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDED)) // EV8-2
//    {
//        if ((I2CTimeout--) == 0)
//        {
//            CommTimeOut_CallBack(MASTER_SENDED);
//        }
//    }
//    
//    if (Comm_Flag == C_READY)
//    {
//        Comm_Flag = C_STOP_BIT;
//        I2C_GenerateStop(I2C1, ENABLE);
//    }
//    
//    while (I2C_GetFlag(I2C1, I2C_FLAG_BUSY))
//    {
//        if ((I2CTimeout--) == 0)
//        {
//            CommTimeOut_CallBack(MASTER_BUSY);
//        }
//    }
//    Comm_Flag = C_READY;

//#ifdef NON_REENTRANT
//    if (Mutex_Flag)
//        Mutex_Flag = 0;
//    else
//        return -2;
//#endif
//    
//    return 0;
//}

int i2c_master_recv(uint8_t* data, u8 TargetAddr, int len)
{
    uint8_t* recvBufferPtr = data;
	
#ifdef NON_REENTRANT
    if (Mutex_Flag)
        return -1;
    else
        Mutex_Flag = 1;
#endif
    
    I2CTimeout             = I2CT_LONG_TIMEOUT;
    while (I2C_GetFlag(I2C1, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack("AHT20 EER!", RE_MASTER_BUSY);return RUNERR;
        }
    }
    I2C_ConfigAck(I2C1, ENABLE);

    // send start
    if (Comm_Flag == C_READY)
    {
        Comm_Flag = C_START_BIT;
        I2C_GenerateStart(I2C1, ENABLE);
    }
    
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_MODE_FLAG)) // EV5
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack("AHT20 EER!", RE_MASTER_MODE);return RUNERR;
        }
    }
    // send addr
    I2C_SendAddr7bit(I2C1, TargetAddr, I2C_DIRECTION_RECV);
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_RXMODE_FLAG)) // EV6
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack("AHT20 EER!", MASTER_RXMODE);return RUNERR;
        }
    }
    Comm_Flag = C_READY;
    
    if (len == 1)
    {
        I2C_ConfigAck(I2C1, DISABLE);
        (void)(I2C1->STS1); /// clear ADDR
        (void)(I2C1->STS2);
        if (Comm_Flag == C_READY)
        {
            Comm_Flag = C_STOP_BIT;
            I2C_GenerateStop(I2C1, ENABLE);
        }
        
        I2CTimeout = I2CT_LONG_TIMEOUT;
        while (!I2C_GetFlag(I2C1, I2C_FLAG_RXDATNE))
        {
            if ((I2CTimeout--) == 0)
            {
                CommTimeOut_CallBack("AHT20 EER!", MASTER_RECVD);return RUNERR;
            }
        }
        *recvBufferPtr++ = I2C_RecvData(I2C1);
        len--;
    }
    else if (len == 2)
    {
        I2C1->CTRL1 |= 0x0800; /// set ACKPOS
        (void)(I2C1->STS1);
        (void)(I2C1->STS2);
        I2C_ConfigAck(I2C1, DISABLE);
        
        I2CTimeout = I2CT_LONG_TIMEOUT;
        while (!I2C_GetFlag(I2C1, I2C_FLAG_BYTEF))
        {
            if ((I2CTimeout--) == 0)
            {
                CommTimeOut_CallBack("AHT20 EER!", MASTER_BYTEF);return RUNERR;
            }
        }
        
        if (Comm_Flag == C_READY)
        {
            Comm_Flag = C_STOP_BIT;
            I2C_GenerateStop(I2C1, ENABLE);
        }
        
        *recvBufferPtr++ = I2C_RecvData(I2C1);
        len--;
        *recvBufferPtr++ = I2C_RecvData(I2C1);
        len--;
    }
    else
    {
        I2C_ConfigAck(I2C1, ENABLE);
        (void)(I2C1->STS1);
        (void)(I2C1->STS2);
        
        while (len)
        {
            if (len == 3)
            {
                I2CTimeout = I2CT_LONG_TIMEOUT;
                while (!I2C_GetFlag(I2C1, I2C_FLAG_BYTEF))
                {
                    if ((I2CTimeout--) == 0)
                    {
                        CommTimeOut_CallBack("AHT20 EER!", MASTER_BYTEF);return RUNERR;
                    }
                }
                I2C_ConfigAck(I2C1, DISABLE);
                *recvBufferPtr++ = I2C_RecvData(I2C1);
                len--;
                
                I2CTimeout = I2CT_LONG_TIMEOUT;
                while (!I2C_GetFlag(I2C1, I2C_FLAG_BYTEF))
                {
                    if ((I2CTimeout--) == 0)
                    {
                        CommTimeOut_CallBack("AHT20 EER!", MASTER_BYTEF);return RUNERR;
                    }
                }
                
                if (Comm_Flag == C_READY)
                {
                    Comm_Flag = C_STOP_BIT;
                    I2C_GenerateStop(I2C1, ENABLE);
                }
        
                *recvBufferPtr++ = I2C_RecvData(I2C1);
                len--;
                *recvBufferPtr++ = I2C_RecvData(I2C1);
                len--;
                
                break;
            }
            
            I2CTimeout = I2CT_LONG_TIMEOUT;
            while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_RECVD_FLAG)) // EV7
            {
                if ((I2CTimeout--) == 0)
                {
                    CommTimeOut_CallBack("AHT20 EER!", MASTER_RECVD);return RUNERR;
                }
            }
            *recvBufferPtr++ = I2C_RecvData(I2C1);
            len--;
        }
    }
    
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (I2C_GetFlag(I2C1, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack("AHT20 EER!", RE_MASTER_BUSY);return RUNERR;
        }
    }
    Comm_Flag = C_READY;
    
#ifdef NON_REENTRANT
    if (Mutex_Flag)
        Mutex_Flag = 0;
    else
        return -2;
#endif
    
    return 0;
}

//void Delay(uint32_t nCount)
//{
//    uint32_t tcnt;
//    while (nCount--)
//    {
//        tcnt = 144000 / 5;
//        while (tcnt--){;}
//    }
//}

//void Delay_us(uint32_t nCount)
//{
//    uint32_t tcnt;
//    while (nCount--)
//    {
//        tcnt = 144 / 5;
//        while (tcnt--){;}
//    }
//}
void IIC_RestoreSlaveByClock(void)
{
    uint8_t i;
    GPIO_InitType i2cx_gpio;
    
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
    GPIO_AFIOInitDefault();
    GPIO_DeInit(GPIOx);
    
    i2cx_gpio.Pin        = I2Cx_SCL_PIN;
    i2cx_gpio.GPIO_Speed = GPIO_Speed_50MHz;
    i2cx_gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(GPIOx, &i2cx_gpio);
    
    for (i = 0; i < 9; i++)
    {
        GPIO_SetBits(GPIOx, I2Cx_SCL_PIN);
        Delay_Us(5);
        GPIO_ResetBits(GPIOx, I2Cx_SCL_PIN);
        Delay_Us(5);
    }
}
    
void SystemNVICReset(void)
{
//    __set_FAULTMASK((uint32_t)1);
//    log_info("***** NVIC system reset! *****\r\n");
	 AppLogPrintf("***** NVIC system reset! *****\r\n");
    __NVIC_SystemReset();
}

void IIC_RCCReset(void)
{
    if (RCC_RESET_Flag >= 3)
    {
        SystemNVICReset();
    }
    else
    {
        RCC_RESET_Flag++;
        
        RCC_EnableAPB1PeriphReset(RCC_APB1_PERIPH_I2C1, ENABLE);
        RCC_EnableAPB1PeriphReset(RCC_APB1_PERIPH_I2C1, DISABLE);
        
        RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_I2C1, DISABLE );
        GPIO_ConfigPinRemap(GPIO_RMP_I2C1, DISABLE);
        GPIOB->PH_CFG &= 0xFFFFFF00;
        RCC_EnableAPB2PeriphClk( RCC_APB2_PERIPH_AFIO, DISABLE);
        RCC_EnableAPB2PeriphClk (RCC_APB2_PERIPH_GPIOB, DISABLE );
        
        RCC_EnableAPB1PeriphReset(RCC_APB1_PERIPH_I2C1, ENABLE);
        RCC_EnableAPB1PeriphReset(RCC_APB1_PERIPH_I2C1, DISABLE);
        
        IIC_RestoreSlaveByClock();
//			i2c_master_init();
//        log_info("***** IIC module by RCC reset! *****\r\n");
		 AppLogPrintf("***** IIC module by RCC reset! *****\r\n");
        i2c_master_init();
    }
}

void IIC_SWReset(void)
{
    GPIO_InitType i2cx_gpio;
    
    i2cx_gpio.Pin        = I2Cx_SCL_PIN | I2Cx_SDA_PIN;
    i2cx_gpio.GPIO_Speed = GPIO_Speed_50MHz;
    i2cx_gpio.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitPeripheral(GPIOx, &i2cx_gpio);
    
    I2CTimeout = I2CT_LONG_TIMEOUT;
    for (;;)
    {
        if ((I2Cx_SCL_PIN | I2Cx_SDA_PIN) == (GPIOx->PID & (I2Cx_SCL_PIN | I2Cx_SDA_PIN)))
        {
            I2Cx->CTRL1 |= 0x8000;
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            I2Cx->CTRL1 &= ~0x8000;
//            log_info("***** IIC module self reset! *****\r\n");
					 AppLogPrintf("***** IIC module self reset! *****\r\n"); 
            break;
        }
        else
        {
            if ((I2CTimeout--) == 0)
            {
                IIC_RCCReset();
            }
        }
    }
}
static const char *I2C_ERR_NumToString(ErrCode_t result)
{   
    switch (result)
    {
			ENUM_CHIP_TYPE_CASE(MASTER_OK)
			ENUM_CHIP_TYPE_CASE(MASTER_BUSY)
			ENUM_CHIP_TYPE_CASE(MASTER_MODE)
			ENUM_CHIP_TYPE_CASE(MASTER_TXMODE)
			ENUM_CHIP_TYPE_CASE(MASTER_RXMODE)
			ENUM_CHIP_TYPE_CASE(MASTER_SENDING)
			ENUM_CHIP_TYPE_CASE(MASTER_SENDED)
			
			ENUM_CHIP_TYPE_CASE(MASTER_RECVD)
			ENUM_CHIP_TYPE_CASE(MASTER_BYTEF)
			ENUM_CHIP_TYPE_CASE(MASTER_BUSERR)
			ENUM_CHIP_TYPE_CASE(MASTER_UNKNOW)
			ENUM_CHIP_TYPE_CASE(SLAVE_OK)			
			ENUM_CHIP_TYPE_CASE(SLAVE_BUSY)
			ENUM_CHIP_TYPE_CASE(SLAVE_MODE)
			
			ENUM_CHIP_TYPE_CASE(SLAVE_BUSERR)
			ENUM_CHIP_TYPE_CASE(SLAVE_UNKNOW)	
			ENUM_CHIP_TYPE_CASE(RE_MASTER_BUSY)
			ENUM_CHIP_TYPE_CASE(RE_MASTER_MODE)
    }
    return "ÎÞ´ËÃüÁî";
}

void CommTimeOut_CallBack(char* hardwave, ErrCode_t errcode)
{
//    log_info("...ErrCode:%d\r\n", errcode);
 AppLogPrintf("...I2C ErrCode:%s, %s ",hardwave, I2C_ERR_NumToString(errcode)); //RunResultToString(status)
  
	OLED_Init();// Delay_Ms(100);
#if (COMM_RECOVER_MODE == MODULE_SELF_RESET)
    IIC_SWReset();
#elif (COMM_RECOVER_MODE == MODULE_RCC_RESET)
    IIC_RCCReset();
#elif (COMM_RECOVER_MODE == SYSTEM_NVIC_RESET)
    SystemNVICReset();
#endif
}

//
RunResult I2C1_Send_Fun(I2CBufferType I2CBuffer )
{
//	return 0;
#ifdef NON_REENTRANT
    if (Mutex_Flag)
        return -1;
    else
        Mutex_Flag = 1;
#endif
char* EER_hardwave;
	if(I2CBuffer.SlavAdd == OledSlaveAddr)EER_hardwave = "OLED EER!";
	else if(I2CBuffer.SlavAdd == ATH20SlaveAddr)EER_hardwave = "AHT20 EER!";
	else EER_hardwave = "OTHE EER!";
/*************************************/			
   I2CTimeout = I2CT_LONG_TIMEOUT;
    while (I2C_GetFlag(I2C1, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack(EER_hardwave,MASTER_BUSY); goto OUT_I2C1_Send_Fun;
        }
    }
    
    if (Comm_Flag == C_READY)
    {
        Comm_Flag = C_START_BIT;
        I2C_GenerateStart(I2C1, ENABLE);
    }
    
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_MODE_FLAG)) // EV5
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack(EER_hardwave, MASTER_MODE);goto OUT_I2C1_Send_Fun;
        }
    }
    
    I2C_SendAddr7bit(I2C1, I2CBuffer.SlavAdd, I2C_DIRECTION_SEND);   
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_TXMODE_FLAG)) // EV6
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack(EER_hardwave, MASTER_TXMODE);goto OUT_I2C1_Send_Fun;
        }
    }
    Comm_Flag = C_READY;
//send data & CMD
		I2C_SendData(I2C1, I2CBuffer.Cmd_Dat_RegAdd);
		I2CTimeout = I2CT_LONG_TIMEOUT;
		while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDING)) // EV8
		{
				if ((I2CTimeout--) == 0)
				{
						CommTimeOut_CallBack(EER_hardwave, MASTER_SENDING);goto OUT_I2C1_Send_Fun;
				}
		}

    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDED)) // EV8-2
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack(EER_hardwave, MASTER_SENDED);goto OUT_I2C1_Send_Fun;
        }
    }
//send data
//		if(!DtaCMD)outLen=0;		//0·¢ÃüÁî
while (I2CBuffer.DatLen -- > 0)
		{
			I2C_SendData(I2C1, *I2CBuffer.SourDat++);

			        I2CTimeout = I2CT_LONG_TIMEOUT;
        while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDING)) // EV8
        {
            if ((I2CTimeout--) == 0)
            {
                CommTimeOut_CallBack(EER_hardwave, MASTER_SENDING);goto OUT_I2C1_Send_Fun;
            }
				}		
						I2CTimeout = I2CT_LONG_TIMEOUT;
				while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDED)) // EV8-2
				{
						if ((I2CTimeout--) == 0)
						{
								CommTimeOut_CallBack(EER_hardwave, MASTER_SENDED);goto OUT_I2C1_Send_Fun;
						}
				}
       
		}
//   if (Comm_Flag == C_READY)
//    {
        Comm_Flag = C_STOP_BIT;
        I2C_GenerateStop(I2C1, ENABLE);
//    }
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (I2C_GetFlag(I2C1, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
        {
					CommTimeOut_CallBack(EER_hardwave, MASTER_BUSY);goto OUT_I2C1_Send_Fun;
        }
    }
OUT_I2C1_Send_Fun:		
    Comm_Flag = C_READY;
/*************************************/	
#ifdef NON_REENTRANT
    if (Mutex_Flag)
        Mutex_Flag = 0;
    else
        return -2;
#endif
    
    return RUNOK;
}

/**
 * @}
 */
