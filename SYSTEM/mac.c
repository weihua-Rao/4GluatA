#include "mac.h"
#include <string.h>
#include "syslib.h"

/********************************************************************************
  * @file    mac.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   提供stm32唯一识别码的接口、提供根据——DATA——获取编译版本号的接口
  ******************************************************************************
	* @驱动功能：
	* 1、GetDeviceMacAddress()提供stm32唯一识别码（我们称之为设备MAC地址）的接口
	* 2、Query_AppVersionIOEXTI()提供根据——DATA——获取编译版本号的接口
*******************************************************************************/

/**************************************************************************************************
* 名    称：  void Query_AppVersion( char *version)
* 功能说明：  获取软件版本号
* 出口参数：   @param1  char *version: 存储版本号的字符指针	  “Nov 24 2014”
* 原理说明：软件版本号是KEIL工程编译时的系统日期，其中__DATE__为宏定义的日期 ，__TIME__为宏定义的时间
  *************************************************************************************************/
	void Query_AppVersion( char *version)
{
	 memset(version, 0, VERSION_LEN);
   strncpy( (char*)version, __DATE__, VERSION_LEN ) ; //长度11bytes
	 #ifdef Beta
	    *(version+3) = 'T' ;   //如果版本为未发布的版本 则在将版本号第4为字符更换为‘T’
	 #endif
		
	 #ifdef CUSTOM
	    *(version+6) = 'C' ;   //如果版本为定制版本 则在将版本号第7为字符更换为‘C’
	 #endif
}

/**************************************************************************************************
* 名    称：  void GetDeviceMacAddress(uint8_t *macAddressNBytes, MACIDFORMAT_e eMacIdFormat)
* 功能说明：  获取STM32唯一识别号作为设备MACID
* 入口参数：
*				      @param1 MACIDFORMAT_e eMacIdFormat: MACI以什么形式输出	
*                @arg1 STRMACID：获取的MACID进行了16进制转字符串，长度为24字节
*                @arg1 HEXMACID：获取的MACID为16进制数，长度为12字节
* 出口参数：无	
* 原理说明：STM32在如下FLASH地址存放了只读的唯一识别码，将对应地址的数据读出即可作为设备MACID
*						0x1FFFF7E8: *(u16 *)(0x1FFFF7E8));	
*						0x1FFFF7EA: *(u16 *)(0x1FFFF7E8+2));	
*						0x1FFFF7EE: *(u32 *)(0x1FFFF7E8+4));
*						0x1FFFF7F6: *(u32 *)(0x1FFFF7E8+8));	
  *************************************************************************************************/
void GetDeviceMacAddress(uint8_t *macAddressNBytes, MACIDFORMAT_e eMacIdFormat)
{
	MACID_u macAddressBytes ;
	memset( macAddressBytes.macBytes, 0, MAC_BYTES_LEN/2) ;
	macAddressBytes.macValue[0] = *(u32 *)(0x1FFFF7E8) ;
	macAddressBytes.macValue[1] = *(u32 *)(0x1FFFF7E8+4) ;
	macAddressBytes.macValue[2] = *(u32 *)(0x1FFFF7E8+8) ;	
	switch( eMacIdFormat )
	{
	  case STRMACID:
		         			ByteToHexStr(macAddressNBytes, macAddressBytes.macBytes, MAC_BYTES_LEN/2)  ;	
									break ;
	  case HEXMACID:
		              strncpy((char*)macAddressNBytes, (const char*)macAddressBytes.macBytes, MAC_BYTES_LEN/2) ;\
									break ;
	  default :
		              ByteToHexStr(macAddressNBytes, macAddressBytes.macBytes, MAC_BYTES_LEN/2)  ;	
									break ;
	}    
	//*(macAddressNBytes+MAC_BYTES_LEN-1) = 0x00 ; //添加字符串尾0x00
}


