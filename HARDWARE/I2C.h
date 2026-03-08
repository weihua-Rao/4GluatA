#ifndef _I2C_H_
#define _I2C_H_ 

#include "n32g45x.h"
#include "n32g45x_i2c.h"
#include "syslib.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define NON_REENTRANT

typedef struct
{
const u8 SlavAdd;
	u8 Cmd_Dat_RegAdd;
	u8* SourDat;
	u32 DatLen;
}I2CBufferType;

typedef enum
{
    C_READY = 0,
    C_START_BIT,
    C_STOP_BIT
}CommCtrl_t;

typedef enum
{
    MASTER_OK = 0,
    MASTER_BUSY,
    MASTER_MODE,
    MASTER_TXMODE,
    MASTER_RXMODE,
    MASTER_SENDING,
    MASTER_SENDED,
    MASTER_RECVD,
    MASTER_BYTEF,
    MASTER_BUSERR,
    MASTER_UNKNOW,
    SLAVE_OK = 20,
    SLAVE_BUSY,
    SLAVE_MODE,
    SLAVE_BUSERR,
    SLAVE_UNKNOW,
	RE_MASTER_BUSY,
	RE_MASTER_MODE,

}ErrCode_t;

#define MODULE_SELF_RESET       1		//模块自复位  
#define MODULE_RCC_RESET        2		//RCC复位
#define SYSTEM_NVIC_RESET       3		//系统复位
#define COMM_RECOVER_MODE       0		

//#define TEST_BUFFER_SIZE  27
#define I2CT_FLAG_TIMEOUT ((uint32_t)0x1000)
#define I2CT_LONG_TIMEOUT ((uint32_t)(10 * I2CT_FLAG_TIMEOUT))//10

#define I2C_MASTER_ADDR   0x30
//#define I2C_SLAVE_ADDR    0x78

#define I2C1_TEST
#define I2C1_REMAP
#define I2Cx I2C1
#define I2Cx_SCL_PIN GPIO_PIN_8
#define I2Cx_SDA_PIN GPIO_PIN_9
#define GPIOx        GPIOB

//#define OLED_RESET_SET() GPIO_SetBits(GPIOA,GPIO_PIN_0)
//#define OLED_RESET_CLEAR() GPIO_ResetBits(GPIOA,GPIO_PIN_0)

extern __IO uint32_t I2CTimeout;
extern CommCtrl_t Comm_Flag;

void CommTimeOut_CallBack(char* hardwave, ErrCode_t errcode);
//extern int i2c_master_send(uint8_t* data, u8 TargetAddr ,int len);
extern int i2c_master_init(void);
extern int i2c_master_recv(uint8_t* data, u8 TargetAddr ,int len);

extern RunResult I2C1_Send_Fun(I2CBufferType I2CBuffer );
#ifdef __cplusplus
}
#endif

#endif
