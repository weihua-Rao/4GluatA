 #include "logflash.h"	
 #include <string.h>
 #include "sysport.h"  
 #include "rtc.h"
  #include "usart.h"
 #include	"n32g45x_flash.h"
 #include "flash.h"
 
/********************************************************************************
  * @file    logflash.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   MCU未使用的FLASH空间用于硬件日志的存储
  ******************************************************************************
	* @attention:LogoFlashInfo区域（占用1页空间，区域起始地址：LOGFLASH_INFO_ADDRESS）存放uLogFlash共用体信息，用来纪录logflash运行的信息。
	* LogoFlash区域（内部FLASH余下的所有空间，区域其实地址为：LOGFLASH_START_ADDRESS）紧跟LogoFlashInfo区域后面，用于存放LOG数据。
  * @use：
	* 先调用void LogFlash_Init(void)初始化，上层需要定义LOG_AREA_ADDR地址
*******************************************************************************/

/*****************************************
*供内部使用常变量
****************************************/
LogFlash_u uLogFlash ;                    //LogFlash信息共用体变量uLogFlash，存放logflash相关信息

/****************************************************************************
* 名    称：void LogFlash_Init(void) 
* 功    能：初始化LogFlash相关配置信息结构体uLogFlash，串口输出历史LOG
****************************************************************************/
void LogFlash_Init(void)
{
   Read_Flash_Byte(LOGFLASH_INFO_ADDRESS, uLogFlash.bytes, sizeof(LogFlash_s)) ;             					 //读取存在flash中LogoFlashInfo区域存储的LogFlash信息共用体数据
	 if( (uLogFlash.sLogFlash.head == 0x50505050) && (uLogFlash.sLogFlash.tail == 0x05050505))           /*条件成立说明LogFlash已经被配置过*/
	   {
//			  AppLogPrintf("本地日志存储溢出：%d次 .\r\n本地日志: ", uLogFlash.sLogFlash.overflowCounter) ;  //输出LogoFlash区域存储溢出的次数
		    DebugLogPrintf("本地日志存储溢出：%d次 .\r\n本地日志: ", uLogFlash.sLogFlash.overflowCounter) ;  //输出LogoFlash区域存储溢出的次数
 			  ReadLogFromFlash(LOGFLASH_START_ADDRESS, uLogFlash.sLogFlash.writeOffset) ;                    //将现存的所有LOG读出，通过debug串口输出
			  uLogFlash.sLogFlash.overflowCounter = 0 ;                                                      //串口输出LOG后，将LogoFlash区域存储溢出的次数清零。
			  Refresh_LogFlash_Info() ;                                                                      //更新存在flash中LogoFlashInfo区域存储的LogFlash信息共用体数据
		    return ;
		 }
	else   																																															 /*条件不成立说明LogFlash第一次被初始化*/
	   {
		    uLogFlash.sLogFlash.head = 0x50505050 ;                                                        //uLogFlash信息 数据头
				uLogFlash.sLogFlash.tail = 0x05050505 ;                                                        //uLogFlash信息 数据尾
				uLogFlash.sLogFlash.overflowCounter = 0 ;                                                      //uLogFlash信息 存储区域溢出次数
				Erase_LogFlash() ;                                                                             //将LogoFlash存储区域擦除
		 }
}

/**************************************************************************************************
* 名    称：void Erase_LogFlash(void)
* 功能说明：擦除LogFlash存储区的所有数据
* 调用方法：外部调用
*************************************************************************************************/
void Erase_LogFlash(void)
{
    FLASH_Unlock();						                                      //解锁FLASH
    for( uint8_t n = 0; n < LOGFLASH_SIZE/FLASH_PAGE_SIZE; n++)		  //循环页，擦除LOGFLASH区域 
       {
	       FLASH_EraseOnePage(LOGFLASH_START_ADDRESS+n*FLASH_PAGE_SIZE) ;//擦除一页数据			
			 }
		uLogFlash.sLogFlash.writeOffset = 0 ;                           //uLogFlash信息 写偏移量清零                
		uLogFlash.sLogFlash.readOffset  = 0 ;                           //uLogFlash信息 读偏移量清零
		Refresh_LogFlash_Info() ;	                                      //将uLogFlash共用体数据存储到LogoFlashInfo区域中
    FLASH_Lock();                                                   //FLASH上锁			 
}

/****************************************************************************
* 名    称：void Refresh_LogFlash_Info(void) 
* 功    能：读取存在flash中LogoFlashInfo区域存储的LogFlash信息共用体数据，即将uLogFlash
*           共用体数据存储到LogoFlashInfo区域中。
****************************************************************************/
void Refresh_LogFlash_Info(void)
{
   FLASH_EraseOnePage(LOGFLASH_INFO_ADDRESS) ;                                          //擦除LogoFlashInfo区域
	 Write_Flash_OnePage(LOGFLASH_INFO_ADDRESS, uLogFlash.bytes, sizeof(LogFlash_s)) ; //将uLogFlash共用体数据存储到LogoFlashInfo区域中
}

///****************************************************************************
//* 名    称：void WriteLogToFlash(char *buffer)
//* 功    能：向FLASH写入buffer
//* 入口参数：
//*           @param1 *buffer  需要写入FLASH中数据缓冲区的指针
//****************************************************************************/
//void WriteLogToFlash(char *buffer)
//{
//	if( strlen(buffer) == 0 )	    
//	{
//	  return ;
//	}
//	uint16_t bufferLen = strlen(buffer) + 20 ;                                                                     //计算需要分配的内存空间长度，多分配20bytes用于存储uCalendar.bytes等数据
//	char* logWriteBuffer = portMalloc(bufferLen) ;                                                                 //分配内存
//	uint16_t  outLen = 0, index = 0, timeOut = 0 ;
//	outLen = snprintf((char*)(logWriteBuffer), bufferLen, "*%s %s* ",(const char*)&uCalendar.bytes[11], buffer) ;  //拼接字符串，防止logFlashBuffer越界
//	outLen = outLen/2+(outLen%2) ;				              																													 //将logLen强制改变为2的倍数 将logLen字节长度转为半字长度
//	FLASH_Unlock();			                  																																				 //FLASH解锁	
//	FLASH_ClearFlag(FLASH_FLAG_BUSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR) ;										 //清FLASH标志位
//	for( timeOut=0; (SET == FLASH_GetFlagSTS(FLASH_FLAG_BUSY)); timeOut++)
//		{
//			 Wait_For_Nms(10) ;
//			 if( timeOut>10 )
//				 {
//					 ErrorLogPrintf("写Flash忙等待超时！") ;
//					 FLASH_Unlock();			//FLASH解锁
//					 portFree(logWriteBuffer) ;									 
//					 return ;
//				 }
//		}					
//	uint16_t halfWord ;
//	while(outLen--)
//		{
//			FLASH_ClearFlag( FLASH_FLAG_EOP ) ;
//			halfWord  = *(logWriteBuffer+(index++)) ;                                                                  //取uint16_t类型数据halfWord LSB
//			if( (halfWord & 0x00FF) == 0x0000)                                                                         //将halfWord中的空字符（0x00）用“——”符号（0x2D）替换
//				 halfWord = halfWord | 0x002D ;
//			halfWord |= *(logWriteBuffer+(index++)) << 8 ;                                                             //取uint16_t类型数据halfWord HSB
//			if( (halfWord & 0xFF00) == 0x0000)
//				 halfWord = halfWord | 0x2D00 ;
//			
//			if( (uLogFlash.sLogFlash.writeOffset) >= (LOGFLASH_SIZE - bufferLen) )                                     //地址超出FLASH范围后重新开始写
//				{
//					 uLogFlash.sLogFlash.writeOffset = 0 ;
//					 uLogFlash.sLogFlash.overflowCounter++ ;                                                               //溢出计数器加一
//				}		
//			if( ( (LOGFLASH_START_ADDRESS + uLogFlash.sLogFlash.writeOffset) % FLASH_PAGE_SIZE ) == 0 )                //写入地址为页首地址则擦除此页
//				{
//					FLASH_EraseOnePage(LOGFLASH_START_ADDRESS+uLogFlash.sLogFlash.writeOffset) ;
//				}
//			FLASH_ProgramOBData(LOGFLASH_START_ADDRESS+uLogFlash.sLogFlash.writeOffset, halfWord) ;                  //地址偏移量先偏移后写，防止FLASH_ERROR_PG错误
//			uLogFlash.sLogFlash.writeOffset += 2;
//		}
//	Refresh_LogFlash_Info() ;                                                                                      //更新存在flash中LogoFlashInfo区域存储的LogFlash信息共用体数据
//	FLASH_Lock();                                                                                                  //FLASH上锁
//	AppLogPrintf( "Flash偏移长度:%#x", uLogFlash.sLogFlash.writeOffset );		 
//	portFree(logWriteBuffer) ;                                                                                     //释放动态内存空间
//}
/****************************************************************************
* 名    称：void WriteLogToFlash(char *buffer)
* 功    能：向FLASH写入buffer
* 入口参数：
*           @param1 *buffer  需要写入FLASH中数据缓冲区的指针
****************************************************************************/
void WriteLogToFlash(char *buffer)
{
	if( strlen(buffer) == 0 )	    
	{
	  return ;
	}
	uint16_t bufferLen = strlen(buffer) + 20 ;                                                                     //计算需要分配的内存空间长度，多分配20bytes用于存储uCalendar.bytes等数据
	char* logWriteBuffer = portMalloc(bufferLen) ;                                                                 //分配内存
	uint16_t  outLen = 0, index = 0, timeOut = 0 ;
	outLen = snprintf((char*)(logWriteBuffer), bufferLen, "*%s %s* ",(const char*)&uCalendar.bytes[11], buffer) ;  //拼接字符串，防止logFlashBuffer越界
	outLen = outLen/2+(outLen%2) ;				              																													 //将logLen强制改变为2的倍数 将logLen字节长度转为半字长度
	FLASH_Unlock();			                  																																				 //FLASH解锁	
	FLASH_ClearFlag(FLASH_FLAG_BUSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR) ;										 //清FLASH标志位
	for( timeOut=0; (SET == FLASH_GetFlagSTS(FLASH_FLAG_BUSY)); timeOut++)
		{
			 Wait_For_Nms(10) ;
			 if( timeOut>10 )
				 {
					 ErrorLogPrintf("写Flash忙等待超时！") ;
					 FLASH_Unlock();			//FLASH解锁
					 portFree(logWriteBuffer) ;									 
					 return ;
				 }
		}					
	uint32_t halfWord ;
		outLen /= 2;
	while(outLen--)
		{
			FLASH_ClearFlag( FLASH_FLAG_EOP ) ;
			halfWord  = *(logWriteBuffer+(index++)) ;                                                                  //取uint16_t类型数据halfWord LSB
			if( (halfWord & 0x000000FF) == 0x00000000)                                                                         //将halfWord中的空字符（0x00）用“——”符号（0x2D）替换
				 halfWord = halfWord | 0x0000002D ;
			halfWord |= *(logWriteBuffer+(index++)) << 8 ;                                                             //取uint16_t类型数据halfWord HSB
			if( (halfWord & 0x0000FF00) == 0x00000000)
				 halfWord = halfWord | 0x00002D00 ;
			
			if( (uLogFlash.sLogFlash.writeOffset) >= (LOGFLASH_SIZE - bufferLen) )                                     //地址超出FLASH范围后重新开始写
				{
					 uLogFlash.sLogFlash.writeOffset = 0 ;
					 uLogFlash.sLogFlash.overflowCounter++ ;                                                               //溢出计数器加一
				}		
			if( ( (LOGFLASH_START_ADDRESS + uLogFlash.sLogFlash.writeOffset) % FLASH_PAGE_SIZE ) == 0 )                //写入地址为页首地址则擦除此页
				{
					FLASH_EraseOnePage(LOGFLASH_START_ADDRESS+uLogFlash.sLogFlash.writeOffset) ;
				}
//			FLASH_ProgramOBData(LOGFLASH_START_ADDRESS+uLogFlash.sLogFlash.writeOffset, halfWord) ;    操作OB区的      //地址偏移量先偏移后写，防止FLASH_ERROR_PG错误

		/****替换上一行代码****/

			halfWord  |= *(logWriteBuffer+(index++)) << 16 ;                                                                  //取uint16_t类型数据halfWord LSB
			if( (halfWord & 0x00FF0000) == 0x00000000)                                                                         //将halfWord中的空字符（0x00）用“——”符号（0x2D）替换
				 halfWord = halfWord | 0x002D0000 ;
			halfWord |= *(logWriteBuffer+(index++)) << 24 ;                                                             //取uint16_t类型数据halfWord HSB
			if( (halfWord & 0xFF000000) == 0x00000000)
				 halfWord = halfWord | 0x2D000000 ;
			FLASH_ProgramWord(LOGFLASH_START_ADDRESS+uLogFlash.sLogFlash.writeOffset, halfWord);

		/***END***/
		uLogFlash.sLogFlash.writeOffset += 4;
		}
	Refresh_LogFlash_Info() ;                                                                                      //更新存在flash中LogoFlashInfo区域存储的LogFlash信息共用体数据
	FLASH_Lock();                                                                                                  //FLASH上锁
	AppLogPrintf( "Flash偏移长度:%#x", uLogFlash.sLogFlash.writeOffset );		 
	portFree(logWriteBuffer) ;                                                                                     //释放动态内存空间
}
/****************************************************************************
* 名    称：void ReadLogFromFlash(uint32_t readAddr, uint32_t length)
* 功    能：向readAddr地址读出log数据，并通过串口输出
* 入口参数：
*           @param1 readAddr       读出的地址
*           @param3 length         需要写入数据的长度
****************************************************************************/
void ReadLogFromFlash(uint32_t readAddr, uint32_t length)
{
  uint8_t* logReadBuffer = portMalloc(FLASH_PAGE_SIZE) ;
	uint16_t n = length/FLASH_PAGE_SIZE ;
	while(n--)
		{
			Read_Flash_Byte(readAddr, logReadBuffer, FLASH_PAGE_SIZE) ;
			UARTx_SendData(UART_DEBUG, (char*)logReadBuffer, FLASH_PAGE_SIZE ) ;
			readAddr = readAddr+FLASH_PAGE_SIZE ;
		}
	memset(logReadBuffer, 0, FLASH_PAGE_SIZE) ; 
	Read_Flash_Byte(readAddr, logReadBuffer, length%FLASH_PAGE_SIZE ) ;
	UARTx_SendData(UART_DEBUG, (char*)logReadBuffer, length%FLASH_PAGE_SIZE ) ;	
	portFree(logReadBuffer) ;
}



