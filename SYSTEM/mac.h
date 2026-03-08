#ifndef __MACx_H
#define __MACx_H
#include "n32g45x.h"

#define VERSION_LEN           11  //版本字符串长度 “Nov 24 2014”
#define MAC_BYTES_LEN         24  //设备MACID的字节长度“31FFD405524E353728902251”

typedef enum
{
  HEXMACID = 0,         //16进制形式
	STRMACID = 1          //字符串形式
}MACIDFORMAT_e ;        //MACID形式

typedef union 
{
    uint32_t 	macValue[3] ;                //存放U32
		uint8_t 	macBytes[MAC_BYTES_LEN/2] ;  //存放U8
}MACID_u ;                                 //定义共用体可以快速将uint32_t数组拆分为uint8_t数组，描述数据转换


extern void Query_AppVersion( char *version) ;                                           //获取软件编译版本号
extern void GetDeviceMacAddress(uint8_t *macAddressNBytes, MACIDFORMAT_e eMacIdFormat) ; //获取STM32唯一识别号作为设备MACID
#endif
