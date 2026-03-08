#ifndef _EEPROM_read_wirte_H_
#define _EEPROM_read_wirte_H_

//#include "flash.h"
#include "PhoneCode.h"
/******************************************************************************
*N32G435CB内部FLASH(128K)分配
ROOT
APP0
APP1
信息存储空间
未尾1K留备用
*******************************************************************************/
/*编译生成应用程序APP编号  配置APP号选择*/
#if defined APP0_CODE                                       //代码编译存放到APP0代码区
       #define FLASH_OFFSET (APP0_AREA_ADDR-FLASH_BASE)		//0x08000000 - 0x08000000
#elif defined APP1_CODE                                     //代码编译存放到APP1代码区
       #define FLASH_OFFSET (APP1_AREA_ADDR-FLASH_BASE)		//0x0801C000 - 0x08000000
#elif defined APP2_CODE                                     //代码编译存放到APP2代码区
       #define FLASH_OFFSET (APP2_AREA_ADDR-FLASH_BASE)
#elif defined APP3_CODE                                     //代码编译存放到APP3代码区
       #define FLASH_OFFSET (APP3_AREA_ADDR-FLASH_BASE)
#endif

/***********内部FLASH资源分配***********///详细Flash分配请参阅《IAP版本内部FLASH分配》
#define BOOT_AREA_ADDR      0x08000000   //存放BootLoader代码的flash起始地址
#define BOOT_AREA_SIZE      0x18000      //BootLoader代码区96K，范围（0x08000000~0x08018800）
//Reserved1（2K）                        //预留页空间
#define IAPCONFIG_AREA_ADDR 0x08018800   //存放IAP配置参数信息 起始地址
#define IAPCONFIG_AREA_SIZE 0x0800       //IAP配置信息 2K

#define APPCONFIG_AREA_ADDR 0x08019000   //存放APP配置参数信息 起始地址
#define APPCONFIG_AREA_SIZE 0x0800       //APP配置信息 2K
//Reserved2(2K）                         //预留页空间
//Reserved3(2K）                         //预留页空间
//Reserved4(2K）                         //预留页空间
//Reserved5(2K）                         //预留页空间
//Reserved6(2K）                         //预留页空间
#define APP0_AREA_ADDR      0x08000000   //APP0代码区域，即默认无bootloader时APP存储的区域。APP0_CODE
#define APP0_AREA_SIZE      0x18000      //APP0代码区96K，范围（0x08000000~0x08018800）

#define APP1_AREA_ADDR      0x0801C000   //APP1代码区域：存放APP1代码（应急程序）
#define APP1_AREA_SIZE      0x20000      //APP1代码区 128K

#define APP2_AREA_ADDR      0x0803C000   //APP2代码区域：存放APP2代码
#define APP2_AREA_SIZE      0x20000      //APP2代码区 128K

#define APP3_AREA_ADDR      0x0805C000   //APP3代码区域：存放APP3代码
#define APP3_AREA_SIZE      0x20000      //APP3代码区 128K
//Reserved7(2K）                         //预留页空间
//Reserved8(2K）                         //预留页空间
//Reserved9(2K）                         //预留页空间
//Reserved10(2K）                        //预留页空间
//Reserved11(2K）                        //预留页空间
#define LOG_AREA_ADDR       0x0807F000                          //本地LOG存储的区域，该区域分为LogoFlashInfo区域(2K)和LogoFlash(LOG_AREA_SIZE-2K)两个区域
#define LOG_AREA_SIZE      (FLASH_BASE+FLASH_SIZE-LOG_AREA_ADDR)//最后余下的所有FLASH空间都用作存储LOG

Extern_def u8 UartDelay;

uint8_t EEPROM_read_Byte(uint32_t Read_address);
ErrorStatus EEPROM_read_n(u32 EE_address,char *DataAddress,u16 number);
ErrorStatus EEPROM_SectorErase(u32 FLASH_WRITE_START_ADDR);
//u8 Byte_Read(uint);
//ErrorStatus EEPROM_write_n(u32 FLASH_WRITE_START_ADDR,u8 *DataAddress,u16 number);
ErrorStatus EEPROM_write_Byte(u32 FLASH_WRITE_START_ADDR,u8 Dat);
ErrorStatus EEPROM_write_n(u32 FLASH_START_ADDR,u8 *DataAddress,u16 number);
//u8 IapReadByte(uint addr);
ErrorStatus EEPROM_SectorErase(u32 FLASH_WRITE_START_ADDR);
ErrorStatus SectorErase_protect(u32 SectionAddr, u8 *DataAddress, u32 offsetAddr, u32 DataLen,u16 shfit_size);
//extern ErrorStatus EEPROM_write_4x8to32_n(u32 FLASH_START_ADDR,u8 *DataAddress,u16 number);
//extern ErrorStatus EEPROM_read_4x8to32_n(u32 FLASH_START_ADDR,char *DataAddress,u16 number);
#endif
