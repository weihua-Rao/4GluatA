#ifndef __LOGFLASH_H
#define __LOGFLASH_H
#include "n32g45x.h"
#include "flash.h"
#include "EEPROM_read_wirte.h"
/*****************************************
*自定义数据类型
****************************************/
typedef struct
{
  uint32_t head      ;          //固定：0x50505050
	uint32_t writeOffset ;        //写入数据的偏移地址
	uint16_t overflowCounter ;    //FLASH溢出次数
	uint32_t readOffset ;         //读数据的偏移地址
	uint32_t tail ;               //固定：0x05050505
}LogFlash_s ;                   //LogFlash信息结构体

typedef union
{
  LogFlash_s    sLogFlash ;     //LogFlash信息结构体
	uint8_t 	    bytes[sizeof(LogFlash_s)] ;
}LogFlash_u ;                   //LogFlash信息共用体

/*****************************************
*供外部使用常变量
****************************************/
extern LogFlash_u uLogFlash ;   //LogFlash信息共用体变量uLogFlash，存放logflash相关信息

#define LOGFLASH_INFO_ADDRESS     LOG_AREA_ADDR //0x0807F000                //第254页
#define LOGFLASH_START_ADDRESS   (LOGFLASH_INFO_ADDRESS+FLASH_PAGE_SIZE) //0x0807F000 + 2048
#define LOGFLASH_SIZE            (FLASH_BASE+FLASH_SIZE-LOGFLASH_START_ADDRESS)

/*****************************************
*内部函数声明
****************************************/
void ReadLogFromFlash(uint32_t readAddr, uint32_t length) ;
void Refresh_LogFlash_Info(void) ;

/*****************************************
*对外接口函数声明
****************************************/
extern void LogFlash_Init(void) ;
extern void Erase_LogFlash(void) ;
extern void WriteLogToFlash(char *buffer) ;

#endif
