#include "flash.h"	
#include "n32g45x_flash.h"	
#include "n32g45x_rcc.h"  
#include "usart.h"
#include "sysport.h"		
#include "EEPROM_read_wirte.h"
/****************************************************************************
* 名    称：u16 FLASH_ReadHalfWord(u32 faddr)
* 功    能：读取指定地址的半字(16位数据)
* 入口参数：
*           @param faddr    FLASH地址
*                     @arg 0x8000000---0x8080000范围 faddr:读地址(此地址必须为2的倍数!!)
* 出口参数：返回读取的半字
* 注    意：faddr:读地址(此地址必须为2的倍数!!)
****************************************************************************/
u16 FLASH_ReadHalfWord(u32 faddr) 
{
	return *(vu16*)faddr; 
}
/****************************************************************************
* 名    称：void Read_Flash_Byte(uint32_t readAddr, uint8_t *p, uint16_t length)
* 功    能：从FLASH的readAddr地址处读取readLen字节长度的数据到p地址的缓冲区中
* 入口参数：
*           @param1 readAddr   读FLASH起始地址
*           @param2 *readBuf   读取数据缓冲区的地址
*           @param3 readLen    需要读出字节数据的长度
* 出口参数：无
****************************************************************************/
void Read_Flash_Byte(uint32_t readAddr, uint8_t *readBuf, uint16_t readLen)
{
	while(readLen--)
	{
		*(readBuf++) = *((uint8_t*)readAddr++) ;
	}
}

/****************************************************************************
* 名    称：void Read_Flash_Byte(uint32_t readAddr, uint8_t *p, uint16_t length)
* 功    能：从FLASH的readAddr地质处读取readLen半字超度的数据到p地址的缓冲区中
* 入口参数：
*           @param1 readAddr   读FLASH起始地址
*           @param2 *readBuf    读取数据缓冲区的地址
*           @param3 readLen     需要读出半字数据的长度
* 出口参数：无
****************************************************************************/
void Read_Flash_HalfWord(u32 readAddr, u16 *readBuf, u16 readLen)   	
{
	u16 i ;
	for(i=0;i<readLen;i++)
	{
		readBuf[i] = FLASH_ReadHalfWord(readAddr) ;//读取2个字节.
		readAddr += 2 ;//偏移2个字节.	
	}
}

/****************************************************************************
* 名    称：FLASH_Status Write_Flash_NoCheck(u32 WriteAddr, u16 *writeBuf, uint16_t writeLen)  
* 功    能：不检查FLASH有没有被擦除，直接向writeAddr地址写入长度writeLen数据,
* 入口参数：
*           @param1 writeAddr      写入起始地址
*           @param2 *writeBuf      需要写入FLASH中数据缓冲区的地址
*           @param3 writeLen       半字(16位)数
* 出口参数：
*           @param1 FLASH_Status   反馈FLASH写入结果
*                     @arg : FLASH_BUSY = 1, //忙：闪存正在被操作
														 FLASH_ERROR_PG,  //编程错误： 试图对内容不是0xFFFF的地址编程
														 FLASH_ERROR_WRP, 编程错误
														 FLASH_ERROR_TIMEOUT, 编程超时
*                     @arg FLASH_COMPLETE:  FLASH编程成功
* 说    明：对FLASH进行写入或者擦除需要使能HSI。
****************************************************************************/
//FLASH_STS Write_Flash_NoCheck(u32 WriteAddr, u16 *writeBuf, uint16_t writeLen)   
//{ 			 		 
//	u16 i;
//	FLASH_STS writeResult ;
//	for(i=0;i<writeLen;i++)
//	{
//		  writeResult = FLASH_ProgramOBData(WriteAddr,writeBuf[i]);
//		  if( writeResult != FLASH_COMPL )
//				break ;
//	    WriteAddr+=2;//地址增加2.
//	} 
//  return 	writeResult ;
//} 

/****************************************************************************
* 名    称：void Write_Flash_MultiPage(u32 WriteAddr,u16 *writeBuf, uint16_t writeLen) 
* 功    能：向writeAddr地址写入长度writeLen数据
* 入口参数：
*           @param1 writeAddr      写入起始地址
*           @param2 *writeBuf      需要写入FLASH中数据缓冲区的地址
*           @param3 writeLen       半字(16位)数
* 出口参数：
*           @param1 RunResult   反馈FLASH写入结果
*                     @arg RUNERR: FLASH编程错误
*                     @arg RUNOK:  FLASH编程成功
* 说    明：对FLASH进行写入或者擦除需要使能HSI。
****************************************************************************/
//RunResult Write_Flash_MultiPage(u32 WriteAddr, u16 *writeBuf, uint16_t writeLen)	
//{
//	u32 secpos;	   //扇区地址
//	u16 secoff;	   //扇区内偏移地址(16位字计算)
//	u16 secremain; //扇区内剩余地址(16位字计算)	   
// 	u16 i;    
//	u32 offaddr;   //去掉0X08000000后的地址
//  u16 STMFLASH_BUF[FLASH_PAGE_SIZE/2];//最多是2K字节
//	if(WriteAddr<FLASH_BASE||(WriteAddr>=(FLASH_BASE+1024*FLASH_SIZE))) return (InParamErr) ;//非法地址
//	if( SET!= RCC_GetFlagStatus(RCC_FLAG_HSIRD))
//	  {
//		  	SysErr("") ;    //HSI被禁用，无法写或擦除FLASH！
//				return (RUNERR );
//		}
//	FLASH_Unlock();						//解锁
//	offaddr=WriteAddr-FLASH_BASE;		//实际偏移地址.
//	secpos=offaddr/FLASH_PAGE_SIZE;			//扇区地址  0~127 for STM32F103RBT6
//	secoff=(offaddr%FLASH_PAGE_SIZE)/2;		//在扇区内的偏移(2个字节为基本单位.)
//	secremain=FLASH_PAGE_SIZE/2-secoff;		//扇区剩余空间大小   
//	if(writeLen<=secremain)secremain = writeLen;//不大于该扇区范围
//	while(1) 
//	{	
//		Read_Flash_HalfWord(secpos*FLASH_PAGE_SIZE+FLASH_BASE,STMFLASH_BUF,FLASH_PAGE_SIZE/2);//读出整个扇区的内容
//		for(i=0;i<secremain;i++)//校验数据
//		{
//			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//需要擦除  	  
//		}
//		if(i<secremain)//需要擦除
//		{
//			FLASH_EraseOnePage(secpos*FLASH_PAGE_SIZE+FLASH_BASE);//擦除这个扇区
//			for(i=0;i<secremain;i++)//复制
//			{
//				STMFLASH_BUF[i+secoff]=writeBuf[i];	  
//			}
//			Write_Flash_NoCheck(secpos*FLASH_PAGE_SIZE+FLASH_BASE,STMFLASH_BUF,FLASH_PAGE_SIZE/2);//写入整个扇区  
//		}else Write_Flash_NoCheck(WriteAddr,writeBuf,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
//		if(writeLen==secremain)break;//写入结束了
//		else//写入未结束
//		{
//			secpos++;				//扇区地址增1
//			secoff=0;				//偏移位置为0 	 
//		   	writeBuf+=secremain;  	//指针偏移
//			WriteAddr+=secremain;	//写地址偏移	   
//		   	writeLen-=secremain;	//字节(16位)数递减
//			if(writeLen>(FLASH_PAGE_SIZE/2))secremain=FLASH_PAGE_SIZE/2;//下一个扇区还是写不完
//			else secremain=writeLen;//下一个扇区可以写完了
//		}	 
//	};	
//	FLASH_Lock();//上锁
//	return (RUNOK );
//}

/****************************************************************************
* 名    称：RunResult Write_Flash_OnePage(uint32_t writePageAddr, uint8_t *writeBuf, uint16_t writeLen) 
* 外部引用：ErrorLogPrintf()
* 功    能：向页起始地址写入数据，数据长度最多一页长度
* 入口参数：
*           @param1 writePageAddr  写入的页起始地址
*           @param2 *writeBuf      需要写入FLASH中数据缓冲区的地址
*           @param3 writeLen       需要写入数据的长度（最大值：FLASH_PAGE_SIZE）
* 出口参数：
*           @param1 RunResult   反馈FLASH写入结果
*                     @arg RUNERR: FLASH编程错误
*                     @arg RUNOK:  FLASH编程成功
* 说    明：对FLASH进行写入或者擦除需要使能HSI。
****************************************************************************/
//RunResult Write_Flash_OnePage(uint32_t writePageAddr, uint8_t *writeBuf, uint16_t writeLen)
//{
//	uint16_t halfWord, timeOut = 0 ;
//	writeLen = writeLen/2+(writeLen%2) ;				//将length强制改变为2的倍数
//	if( SET!= RCC_GetFlagStatus(RCC_FLAG_HSIRD))
//	  {
//		  	SysErr("") ;    //HSI被禁用，无法写或擦除FLASH！
//				return (RUNERR );
//		}
//	FLASH_Unlock();			        //FLASH解锁
//	if( (writePageAddr<FLASH_BASE) || (writePageAddr>=(FLASH_BASE+FLASH_SIZE)) || (writePageAddr%FLASH_PAGE_SIZE != 0) )  //地址必须为页首地址且在FLASH地址范围内 非法地址
//	  {
//		    SysErr("") ;    //写入FLASH地址非法！
//			  FLASH_Lock();         //上锁
//				return (RUNERR );
//		}
//	if( writeLen > FLASH_PAGE_SIZE)//写入的数据长度不能超过一页
//	  {
//		    SysErr("") ;    //写入FLASH长度超限！
//				FLASH_Lock();         //上锁
//				return (RUNERR );
//		}
//	FLASH_ClearFlag(FLASH_FLAG_BUSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);//清标志位
//	if( FLASH_COMPL != FLASH_EraseOnePage(writePageAddr))     //执行页擦除，并监测擦除状态
//	  {
//		    SysErr("") ;    //FLASH 页擦除失败！
//		}
//	for( timeOut=0; (SET == FLASH_GetFlagSTS(FLASH_FLAG_BUSY)); timeOut++)
//		{
//			 Wait_For_Nms(10) ;
//			 if( timeOut>10 )
//				 {
//					 SysErr("") ;   //写Flash忙等待超时！
//					 FLASH_Unlock();			//FLASH解锁	
//					 return (RUNERR);
//				 }
//		}
//		
//	while(writeLen--)
//	{
//	  FLASH_ClearFlag( FLASH_FLAG_EOP ) ;
//		halfWord  = *(writeBuf++);
//		halfWord |= *(writeBuf++) << 8;
//		FLASH_ProgramOBData(writePageAddr, halfWord);
//    for( timeOut=0; ( SET != FLASH_GetFlagSTS(FLASH_FLAG_EOP) ); timeOut++)
//		   {
//			   Wait_For_Nms(10) ;
//			   if(timeOut>30)
//			     {
//				     SysErr("") ;  //写Flash出错！
//						 return (RUNERR);
//				   }
//		   }
//		writePageAddr += 2;
//	}
//	FLASH_Lock();                  //上锁
//	return (RUNOK) ;
//}
/****************************************************************************
* 名    称：RunResult Write_Flash_OnePage(uint32_t writePageAddr, uint8_t *writeBuf, uint16_t writeLen) 
* 外部引用：ErrorLogPrintf()
* 功    能：向页起始地址写入数据，数据长度最多一页长度
* 入口参数：
*           @param1 writePageAddr  写入的页起始地址
*           @param2 *writeBuf      需要写入FLASH中数据缓冲区的地址
*           @param3 writeLen       需要写入数据的长度（最大值：FLASH_PAGE_SIZE）
* 出口参数：
*           @param1 RunResult   反馈FLASH写入结果
*                     @arg RUNERR: FLASH编程错误
*                     @arg RUNOK:  FLASH编程成功
* 说    明：对FLASH进行写入或者擦除需要使能HSI。
****************************************************************************/
RunResult Write_Flash_OnePage(uint32_t writePageAddr, uint8_t *writeBuf, uint16_t writeLen)
{
 
	uint16_t timeOut = 0 ;
	writeLen = writeLen/2+(writeLen%2) ;				//将length强制改变为2的倍数
	if( SET!= RCC_GetFlagStatus(RCC_FLAG_HSIRD))
	  {
		  	SysErr("") ;    //HSI被禁用，无法写或擦除FLASH！
				return (RUNERR );
		}
	FLASH_Unlock();			        //FLASH解锁
	if( (writePageAddr<FLASH_BASE) || (writePageAddr>=(FLASH_BASE+FLASH_SIZE)) || (writePageAddr%FLASH_PAGE_SIZE != 0) )  //地址必须为页首地址且在FLASH地址范围内 非法地址
	  {
		    SysErr("") ;    //写入FLASH地址非法！
			  FLASH_Lock();         //上锁
				return (RUNERR );
		}
	if( writeLen > FLASH_PAGE_SIZE)//写入的数据长度不能超过一页
	  {
		    SysErr("") ;    //写入FLASH长度超限！
				FLASH_Lock();         //上锁
				return (RUNERR );
		}
	FLASH_ClearFlag(FLASH_FLAG_BUSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);//清标志位
	if( FLASH_COMPL != FLASH_EraseOnePage(writePageAddr))     //执行页擦除，并监测擦除状态
	  {
		    SysErr("") ;    //FLASH 页擦除失败！
		}
	for( timeOut=0; (SET == FLASH_GetFlagSTS(FLASH_FLAG_BUSY)); timeOut++)
		{
			 Wait_For_Nms(10) ;
			 if( timeOut>10 )
				 {
					 SysErr("") ;   //写Flash忙等待超时！
					 FLASH_Unlock();			//FLASH解锁	
					 return (RUNERR);
				 }
		}
uint32_t halfWord;
	writeLen /= 2;	
	while(writeLen--)
	{
	  FLASH_ClearFlag( FLASH_FLAG_EOP ) ;
		halfWord  = *(writeBuf++);
		halfWord |= *(writeBuf++) << 8;
//		FLASH_ProgramOBData(writePageAddr, halfWord);//操作OB区的
		/****替换上一行代码****/
//		writeLen--;
		halfWord |= *(writeBuf++) << 16;halfWord |= *(writeBuf++) << 24;
//		FLASH_ProgramWord(writePageAddr, halfWord);
			if (FLASH_COMPL != FLASH_ProgramWord(writePageAddr, halfWord))
			{
			 SysErr("") ;  //写Flash出错！
			 FLASH_Lock();         //上锁
			 return (RUNERR);
			}
		/***END***/

		writePageAddr += 4;
	}
	FLASH_Lock();                  //上锁
	return (RUNOK) ;
}

/****************************************************************************
* 名    称：RunResult Write_Flash(uint32_t writeAddr, uint8_t *writeBuf, uint16_t writeLen) 
* 功    能：向writeAddr地址写入长度writeLen数据
* 入口参数：
*           @param1 writeAddr      写入起始地址
*           @param2 *writeBuf      需要写入FLASH中数据缓冲区的地址
*           @param3 writeLen       需要写入数据的长度
* 出口参数：
*           @param1 RunResult   反馈FLASH写入结果
*                     @arg RUNERR: FLASH编程错误
*                     @arg RUNOK:  FLASH编程成功
* 说    明：对FLASH进行写入或者擦除需要使能HSI。
****************************************************************************/
//RunResult Write_Flash(uint32_t writeAddr, uint8_t *writeBuf, uint16_t writeLen)
//{
//	uint16_t  index = 0, timeOut = 0, halfWord = 0 ;
//	u32 writeOffset = 0 ;
//	writeLen = writeLen/2+(writeLen%2) ;				//将logLen强制改变为2的倍数 将logLen字节长度转为半字长度
//	if( SET!= RCC_GetFlagStatus(RCC_FLAG_HSIRD))//RCC_CTRL_FLAG_HSIRDF
//	  {
//		  	SysErr("") ;    //HSI被禁用，无法写或擦除FLASH！
//				return (RUNERR );
//		}
//	FLASH_Unlock();			                        //FLASH解锁	
//	FLASH_ClearFlag(FLASH_FLAG_BUSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);//清标志位
//	for( timeOut=0; (SET == FLASH_GetFlagSTS(FLASH_FLAG_BUSY)); timeOut++)
//		{
//			 Wait_For_Nms(10) ;
//			 if( timeOut>10 )
//				 {
//					 SysErr("") ;  //写Flash出错！
//					 FLASH_Unlock();			//FLASH解锁									 
//					 return(RUNERR) ;
//				 }
//		}					
//	while(writeLen--)
//		{
//			FLASH_ClearFlag( FLASH_FLAG_EOP ) ;
//			halfWord  = *(writeBuf+(index++)) ;                       //取uint16_t类型数据halfWord LSB
//			halfWord |= *(writeBuf+(index++)) << 8 ;                  //取uint16_t类型数据halfWord HSB	
//			if( ( (writeAddr + writeOffset) % FLASH_PAGE_SIZE ) == 0 ) //写入地址为页首地址则擦除此页
//				{
//					FLASH_EraseOnePage(writeAddr+writeOffset) ;
//				}
//			FLASH_ProgramOBData(writeAddr+writeOffset, halfWord) ;  //地址偏移量先偏移后写，防止FLASH_ERROR_PG错误
//			writeOffset += 2;
//		}
//	FLASH_Lock();                                                 //上锁
//	return(RUNOK) ;
//}

RunResult Write_Flash(uint32_t writeAddr, uint8_t *writeBuf, uint16_t writeLen)
{
	uint16_t  index = 0, timeOut = 0;
	u32 writeOffset = 0 ;
	writeLen = writeLen/2+(writeLen%2) ;				//将logLen强制改变为2的倍数 将logLen字节长度转为半字长度
	if( SET!= RCC_GetFlagStatus(RCC_FLAG_HSIRD))//RCC_CTRL_FLAG_HSIRDF
	  {
		  	SysErr("") ;    //HSI被禁用，无法写或擦除FLASH！
				return (RUNERR );
		}
	FLASH_Unlock();			                        //FLASH解锁	
	FLASH_ClearFlag(FLASH_FLAG_BUSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);//清标志位
	for( timeOut=0; (SET == FLASH_GetFlagSTS(FLASH_FLAG_BUSY)); timeOut++)
		{
			 Wait_For_Nms(10) ;
			 if( timeOut>10 )
				 {
					 SysErr("") ;  //写Flash出错！
					 FLASH_Unlock();			//FLASH解锁									 
					 return(RUNERR) ;
				 }
		}	
u32 halfWord = 0 ;
	writeLen /= 2;		
	while(writeLen--)
		{
			FLASH_ClearFlag( FLASH_FLAG_EOP ) ;
			halfWord  = *(writeBuf+(index++)) ;                       //取uint16_t类型数据halfWord LSB
			halfWord |= *(writeBuf+(index++)) << 8 ;                  //取uint16_t类型数据halfWord HSB	
			if( ( (writeAddr + writeOffset) % FLASH_PAGE_SIZE ) == 0 ) //写入地址为页首地址则擦除此页
				{
					FLASH_EraseOnePage(writeAddr+writeOffset) ;
				}
//			FLASH_ProgramOBData(writeAddr+writeOffset, halfWord) ; //操作OB区的 //地址偏移量先偏移后写，防止FLASH_ERROR_PG错误
		/****替换上一行代码****/
//		writeLen--;
		halfWord |= *(writeBuf++) << 16;halfWord |= *(writeBuf++) << 24;
//		FLASH_ProgramWord(writePageAddr, halfWord);
			if (FLASH_COMPL != FLASH_ProgramWord(writeAddr+writeOffset, halfWord))
			{
			 SysErr("") ;  //写Flash出错！
			 FLASH_Lock();         //上锁
			 return (RUNERR);
			}
		/***END***/
	
			writeOffset += 4;
		}
	FLASH_Lock();                                                 //上锁
	return(RUNOK) ;
}





