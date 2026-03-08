#ifndef __FLASH_H
#define __FLASH_H

#include "n32g45x_it.h"
#include "n32g45x_flash.h"	
#include "syslib.h"

//#if defined STM32F10X_HD                                      //stm32f10中HD规格的mcu，FLASH共计256页，每一页为2Kbutes
//    #define FLASH_PAGE_SUM   256                              //内部flash总页数 N32G43X
//		#define FLASH_PAGE_SIZE  2048                             //每一页的大小（bytes）
//    #define FLASH_SIZE      (FLASH_PAGE_SUM*FLASH_PAGE_SIZE)  //内部flash的总大小（bytes）
//#elif defined STM32F10X_MD
//    #define FLASH_PAGE_SUM   256
//		#define FLASH_PAGE_SIZE  1024
//    #define FLASH_SIZE      (FLASH_PAGE_SUM*FLASH_PAGE_SIZE)
//#elif defined STM32F10X_LD
//    #define FLASH_PAGE_SUM   128
//		#define FLASH_PAGE_SIZE  1024
//    #define FLASH_SIZE      (FLASH_PAGE_SUM*FLASH_PAGE_SIZE)
//#endif
#if defined N32G45X                                      //N32G451X FLASH共计128页，每一页为2Kbutes
    #define FLASH_PAGE_SUM   256//128                              //内部flash总页数 N32G451X N32G453X
		#define FLASH_PAGE_SIZE  2048                             //每一页的大小（bytes）
    #define FLASH_SIZE      (FLASH_PAGE_SUM*FLASH_PAGE_SIZE)  //内部flash的总大小（bytes）
#elif defined STM32F10X_MD
    #define FLASH_PAGE_SUM   256
		#define FLASH_PAGE_SIZE  1024
    #define FLASH_SIZE      (FLASH_PAGE_SUM*FLASH_PAGE_SIZE)
#elif defined STM32F10X_LD
    #define FLASH_PAGE_SUM   128
		#define FLASH_PAGE_SIZE  1024
    #define FLASH_SIZE      (FLASH_PAGE_SUM*FLASH_PAGE_SIZE)
#endif
/*****************************************
*内部函数声明
****************************************/

/*****************************************
*对外接口函数声明
****************************************/
extern void Read_Flash_Byte(uint32_t readAddr, uint8_t *readBuf, uint16_t readLen) ;                  //从FLASH中读取readLen字节长度的数据
extern void Read_Flash_HalfWord(u32 readAddr, u16 *readBuf, u16 readLen)  ;                           //从FLASH中读取readLen板子长度的数据
//extern FLASH_STS Write_Flash_NoCheck(u32 WriteAddr, u16 *writeBuf, uint16_t writeLen) ;            //不检查FLASH有没有被擦除，写writeLen半字长度的数据到Flash中
extern RunResult Write_Flash_OnePage(uint32_t writePageAddr, uint8_t *writeBuf, uint16_t writeLen) ;  //写一页长度的数据到FLASH中              
extern RunResult Write_Flash(uint32_t writeAddr, uint8_t *writeBuf, uint16_t writeLen) ;              //检查擦除要写入的页，写writeLen字节长度数据到flash中去    

#endif
