
//#include "main.h"
#include "EEPROM_read_wirte.h"
//#include "n32g43x_flash.h"

#define EEPROM_32Bety_Model		//小端 32字节存储模式

union 
	{
		uint32_t Dat32;
		u8 Dat8[4];
	}EEPROM_4x8to32;
//========================================================================

// 描述: 从指定EEPROM首地址读出字节放指定的缓冲.
// 参数: EE_address:  读出EEPROM的首地址.
//       DataAddress: 读出数据放缓冲的首地址.
//========================================================================

uint8_t EEPROM_read_Byte(uint32_t Read_address)
{
//	uint8_t dat;
////	Wait_Delayms(100);   //如果有串口中断等待完成
//	dat = (*(__IO uint8_t*)Read_address);
//	return dat;
	return *(vu8*)Read_address;
}

//========================================================================
// 函数: void EEPROM_read_n(u16 EE_address,u8 *DataAddress,u16 number)
// 描述: 从指定EEPROM首地址读出n个字节放指定的缓冲.
// 参数: EE_address:  读出EEPROM的首地址.
//       DataAddress: 读出数据放缓冲的首地址.
//       number:      读出的字节长度.
// 返回: non.
// 版本: V1.0, 2012-10-22
//========================================================================

ErrorStatus EEPROM_read_n(u32 FLASH_START_ADDR,char *DataAddress,u16 number)
{	
u32 EE_address;	
//	Wait_Delayms(100);
	SimRTX = 1;
//	EE_address = FLASH_START_ADDR + 3;
	EE_address = FLASH_START_ADDR;
    /* Locks the FLASH Program Erase Controller */
//    FLASH_Lock();
	
#if defined  EEPROM_32Bety_Model
	do
	{
		EEPROM_4x8to32.Dat32 = (*(__IO uint32_t*)EE_address);			//读出的数据送往
//		EEPROM_4x8to32.Dat32 = *(vu32*)EE_address;			//读出的数据送往
		for(u8 i = 0;(i<4)&&(number>0);i++,number--)
		{
		*DataAddress = EEPROM_4x8to32.Dat8[i];
		 DataAddress++;
		}
		EE_address += 4;

	}while(number);
#else	
	do
	{
		*DataAddress = (*(__IO uint8_t*)EE_address);			//读出的数据送往
		EE_address += 4;
		DataAddress++;
	}while(--number);
#endif
	
	SimRTX = 0;
	return SUCCESS;
}

/******************** 扇区擦除函数 *****************/
//========================================================================
// 函数: void EEPROM_SectorErase(u16 EE_address)
// 描述: 把指定地址的EEPROM扇区擦除.
// 参数: EE_address:  要擦除的扇区EEPROM的地址.
// 返回: non.
// 版本: V1.0, 2013-5-10
//========================================================================
ErrorStatus EEPROM_SectorErase(u32 FLASH_WRITE_START_ADDR)
{
	
//	Wait_Delayms(100);
	SimRTX = 1;
	
	 /* Unlocks the FLASH Program Erase Controller */
    FLASH_Unlock();

    /* Erase */
    if (FLASH_COMPL != FLASH_EraseOnePage(FLASH_WRITE_START_ADDR))
    {
		return ERROR;
    }
/* Locks the FLASH Program Erase Controller */
    FLASH_Lock();
		
		SimRTX = 0;
	return SUCCESS;	
}

//========================================================================
// 函数: void EEPROM_write_n(u16 EE_address,u8 *DataAddress,u16 number)
// 描述: 把缓冲的n个字节写入指定首地址的EEPROM.
// 参数: EE_address:  写入EEPROM的首地址.
//       DataAddress: 写入源数据的缓冲的首地址.
//       number:      写入的字节长度.
// 返回: non.
// 版本: V1.0, 2012-10-22
//========================================================================

ErrorStatus EEPROM_write_n(u32 FLASH_START_ADDR,u8 *DataAddress,u16 number)
{
u32 FLASH_WRITE_START_ADDR;	
//	UartDelay = 100;
//	while(UartInt){IWDG_ReloadKey();if(!UartDelay)UartInt=0;}	   //如果有串口中断等待完成
	SimRTX = 1;
//	FLASH_WRITE_START_ADDR = FLASH_START_ADDR + 3;
	FLASH_WRITE_START_ADDR = FLASH_START_ADDR;	
/* Unlocks the FLASH Program Erase Controller */
  FLASH_Unlock();
	  /* Program */
#if defined  EEPROM_32Bety_Model
	do
	{EEPROM_4x8to32.Dat32 = 0;
		for(u8 i=0; i<4; i++,DataAddress++)
		{
		EEPROM_4x8to32.Dat8[i] = *DataAddress;
		}
		if(number < 4)number = 0;//return ERROR;
		else number -= 4;
		if (FLASH_COMPL != FLASH_ProgramWord(FLASH_WRITE_START_ADDR, EEPROM_4x8to32.Dat32))
		{
			SysErr("") ;  //写Flash出错！
			FLASH_Lock();         //上锁
			return ERROR;
		}
		FLASH_WRITE_START_ADDR += 4;
		
	}while(number);
#else
	do
	{
		if (FLASH_COMPL != FLASH_ProgramWord(FLASH_WRITE_START_ADDR, *DataAddress))
		{
//			while(1);
			return ERROR;
		}
		FLASH_WRITE_START_ADDR += 4;
		DataAddress ++;
	}while(--number);
/* Locks the FLASH Program Erase Controller */
    
#endif
FLASH_Lock();
	SimRTX = 0;
	return SUCCESS;	
}

//========================================================================
// 描述: 1个字节写入指定首地址的EEPROM.
// 参数: EE_address:  写入EEPROM的首地址.
//       Dat: 写入源数据.

//========================================================================
ErrorStatus EEPROM_write_Byte(u32 FLASH_WRITE_START_ADDR,u8 Dat)
{
//	UartDelay = 100;
//	while(UartInt){IWDG_ReloadKey();if(!UartDelay)UartInt=0;}	   //如果有串口中断等待完成
	SimRTX = 1;
		 
/* Unlocks the FLASH Program Erase Controller */
  FLASH_Unlock();
	  /* Program */

		if (FLASH_COMPL != FLASH_ProgramWord(FLASH_WRITE_START_ADDR, Dat))
		{
			return ERROR;
		}

/* Locks the FLASH Program Erase Controller */
    FLASH_Lock();

	SimRTX = 0;
	return SUCCESS;	
}
/* 对扇区进行擦除 保留同一扇区中不需修改的数据    */
//*SectionAddr,操作扇区地址
//*DataAddress 数据
//offsetAddr 数据写入地址偏移量
// DataLen写入数据长度
//u8 shfit_size 后移数据长度
ErrorStatus  SectorErase_protect(u32 SectionAddr, u8 *DataAddress, u32 offsetAddr, u32 DataLen,u16 shfit_size)
{
	u32 EE_address,DatLen;
	if(DataLen > 512)
	{
		AppLogPrintf("data lenght over:%d ",DataLen) ;
		return ERROR;
	}
	u32 *Port_Buf = portMalloc(513*sizeof(u32)) ;
u32 *DataBuf = Port_Buf;
//	memset(DataBuf, 0xffffffff, 2048) ;

		SimRTX = 1; 
		EE_address = SectionAddr;DatLen = 512;// 2048/4;
			/* Locks the FLASH Program Erase Controller */
//		FLASH_Lock();
/****************将扇区数据全部读出到缓存区*********************/
	shfit_size /= 4;			//相当于4个字节
		if(shfit_size > 0)		// 数据后移长度
		{
			DatLen -= shfit_size;
			do
			{
				*(DataBuf + shfit_size) = (*(__IO uint32_t*)EE_address);			//读出的数据送往
				EE_address +=4;DataBuf ++;
			}while(DatLen--);	
		}
		else
		{
			do
			{
				*DataBuf = (*(__IO uint32_t*)EE_address);			//读出的数据送往
				EE_address +=4;DataBuf ++;
			}while(DatLen--);		
		}

/****************将扇区数据全部擦除*********************/		
	EEPROM_SectorErase(SectionAddr);
/****************将要写入的数据存入缓存区*********************/
		DataBuf = Port_Buf + offsetAddr/4;		//因是4字节 加1 相当于4个字节
		do
		{
			for(u8 i=0; i<4; i++,DataAddress++)//
			{
			EEPROM_4x8to32.Dat8[i] = *DataAddress;
			}
			if(DataLen < 4)DataLen = 0;//return ERROR;
			else DataLen -= 4;
			*DataBuf = EEPROM_4x8to32.Dat32;
			DataBuf ++;//因是4字节自加1 相当于4个字节
		}while(DataLen);
		*(u8*)DataBuf = '\0';
/****************将更新后的缓存区数据全部写入扇区*********************/
	EE_address = SectionAddr;DatLen = 512;	DataBuf = Port_Buf;
	FLASH_Unlock();
		do
		{
			if (FLASH_COMPL != FLASH_ProgramWord(EE_address, *DataBuf))
			{
				portFree(Port_Buf);return ERROR;
			}
			EE_address +=4;DataBuf++;
			DatLen--;
		}while(DatLen);
	FLASH_Lock();
		portFree(Port_Buf);
	SimRTX = 0;
	return SUCCESS;	
}	 
///* 对扇区进行擦除 保留同一扇区中不需修改的数据    */
/////Save_addr,扇区区保护的起始地址 数组下标
////*EprAdd 地址数组名
//// ,counter,连续写多少个字节；*/
//void SectorErase_protect(u16 *EprAdd, u8 Save_addr)
//{
//u8 i,ProAdd;
//u8 xdata protect_buffer[39]={0} ;
//    /* 将该扇区数据 0 - (USED_BYTE_QTY_IN_ONE_SECTOR-1) 字节数据读入缓冲区保护 */
//	////每个号码空0x40	64

//	UartDelay = 100;
//	while(UartInt){IWDG_ReloadKey();if(!UartDelay)UartInt=0;}	   //如果有串口中断等待完成
//	SimRTX = 0;
//	EA=0;

//	for(i=0,ProAdd=0;i<3;i++)
//	{
//	EEPROM_read_n(EprAdd[Save_addr+i],&protect_buffer[ProAdd],13);
////	ProAdd += SectorProtect;
//ProAdd += 13;
//	}
//	/* 擦除 要修改/写入 的扇区 */
////    EEPROM_SectorErase(0x0200);
//	EEPROM_SectorErase(EprAdd[0]);
////	Uart1Sends(protect_buffer);
////	Uart1Sends("~~~\r\n");
//    /* 将保护缓冲区的数据写入 Data Flash, EEPROM */
//	for(i=0,ProAdd=0;i<3;i++)
//	{
//	EEPROM_write_n(EprAdd[Save_addr+i],&protect_buffer[ProAdd],13);
////	ProAdd += SectorProtect;
//ProAdd += 13;
//	}
//	EA = 1;		//重新允许中断
//	SimRTX = 1;
//}
