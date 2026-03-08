//#include "syslib.h"
//#include <string.h> 
//#include "usart_n32.h"
//#include "sysport.h"
#include "main.h"
/********************************************************************************
  * @file    syslib.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   提供数据处理相关的函数、提供帧环形缓冲区
  ******************************************************************************
*******************************************************************************/

/**************************************************************************************************
* 名    称：void InitQueueMem(FrameQueue_s* sFrame)
* 功能说明：帧环形缓冲区初始化
* 入口参数：
*           @param1 *sFrame       FrameQueue_s结构体类型指针																	
* 调用方法：外部调用
*************************************************************************************************/ 
void InitQueueMem(FrameQueue_s* sFrame)
{
	sFrame->getIndex    = 0;
	sFrame->insertIndex = 0;
	sFrame->hasDataLen  = 0;
	memset(sFrame->dataBuf, 0 , MAX_QUEUE_LEN) ;
}

/**************************************************************************************************
* 名    称：uint8_t InsertQueueMemData(FrameQueue_s *sFrame, char *data, uint16_t len)
* 功能说明：将数据插入帧环形缓冲区中，并以HEAD1、HEAD2为帧头，以TAIL1、TAIL2为帧尾
* 入口参数：
*           @param1 *sFrame       FrameQueue_s结构体类型指针
*           @param2 *data         需要插入帧缓冲区的数据指针
*           @param3 len           需要插入帧缓冲区的数据长度																		
* 出口参数：FULL_ERROR/RW_OK
* 调用方法：外部调用
*************************************************************************************************/ 
uint8_t InsertQueueMemData(FrameQueue_s *sFrame, char *data, uint16_t len) //插入一帧数据，函数内部会自动添加0x1234,0x5678 4个字节分别作为帧头和帧尾	写入缓冲区
{
    if((sFrame->hasDataLen+len+4) >= MAX_QUEUE_LEN )                        /*缓冲区剩余空间不够插入 新的数据帧和4字节帧头帧尾*/
    {
		    return FULL_ERROR;
    }
    else                                                                    /*缓冲区剩余空间够插入新的数据帧*/
    {   
				sFrame->dataBuf[sFrame->insertIndex] = HEAD1 ;                      //插入帧头第一个字节HEAD1
				sFrame->insertIndex = (sFrame->insertIndex+1) % MAX_QUEUE_LEN;      //核心语句，实现数据环形写入，一旦指针到MAX_QUEUE_LEN位置，则又从0开始写入缓冲区
		    sFrame->dataBuf[sFrame->insertIndex] = HEAD2 ;                      //插入帧头第二个字节HEAD2
				sFrame->insertIndex = (sFrame->insertIndex+1) % MAX_QUEUE_LEN;      //核心语句，实现数据环形写入，一旦指针到MAX_QUEUE_LEN位置，则又从0开始写入缓冲区
		    
			  for(int n =0; n<len; n++ )                                           //循环插入数据
		      {
						 sFrame->dataBuf[sFrame->insertIndex] = *(data++) ;
					   sFrame->insertIndex = (sFrame->insertIndex+1) % MAX_QUEUE_LEN; //核心语句，实现数据环形写入，一旦指针到MAX_QUEUE_LEN位置，则又从0开始写入缓冲区
					}
			  sFrame->dataBuf[sFrame->insertIndex] = TAIL1 ;                       //插入帧尾TAIL1
				sFrame->insertIndex = (sFrame->insertIndex+1) % MAX_QUEUE_LEN;      //核心语句，实现数据环形写入，一旦指针到MAX_QUEUE_LEN位置，则又从0开始写入缓冲区
			  sFrame->dataBuf[sFrame->insertIndex] = TAIL2 ;                       //插入帧尾TAIL2
				sFrame->insertIndex = (sFrame->insertIndex+1) % MAX_QUEUE_LEN;      //核心语句，实现数据环形写入，一旦指针到MAX_QUEUE_LEN位置，则又从0开始写入缓冲区
		   
        sFrame->hasDataLen+= (len+4) ;                                      //更新sFrame->hasDataLen 已经存储数据长度
				if( sFrame->hasDataLen >= MAX_QUEUE_LEN)                            //未知异常
				  {
					 InitQueueMem(sFrame) ;
					 SysErr("") ;
					}
        return RW_OK;
    }
}

/**************************************************************************************************
* 名    称：uint8_t GetQueueMemData(FrameQueue_s* sFrame, char* data, uint16_t* pLen)
* 功能说明：从帧缓冲区中取出一帧数据
* 入口参数：
*           @param1 *sFrame       FrameQueue_s结构体类型指针
*           @param2 *data         取出的帧数据存放的内存地址
*           @param3 len           去除的帧数据长度																		
* 出口参数：EMPTY_ERROR/RW_OK
* 调用方法：外部调用
*************************************************************************************************/ 
uint8_t GetQueueMemData(FrameQueue_s* sFrame, char* data, uint16_t* pLen)
{
    if(0 == sFrame->hasDataLen)                                                       	/*帧缓冲区数据为空，即没有数据*/
    {
		    return EMPTY_ERROR;
    }
    else                                                                              	/*帧缓冲区数据不为空，即存在帧数据*/
    {		  
			  sFrame->getIndex = (sFrame->getIndex + 2) % MAX_QUEUE_LEN;                    	//跳过帧头HEAD1、HEAD2 2个字节
			  *pLen = 0 ;                                                                   	//pLen置0
			  while( (TAIL1 != sFrame->dataBuf[sFrame->getIndex])                           
					  ||((TAIL2 != sFrame->dataBuf[(sFrame->getIndex + 1) % MAX_QUEUE_LEN])) )    /*查找帧尾*/
						{
							 (*pLen)++ ;                                                              //pLen计数器累加
							 *(data++) = sFrame->dataBuf[sFrame->getIndex] ;                          //先将sFrame->dataBuf[sFrame->getIndex]赋值给 *(data++)，在将data指针指向下一个字节
							 sFrame->getIndex = (sFrame->getIndex + 1) % MAX_QUEUE_LEN;
							 if( (*pLen >= sFrame->hasDataLen) || (sFrame->getIndex >= MAX_QUEUE_LEN))//异常处理
							   {
									 InitQueueMem(sFrame) ;
									 SysErr("") ;
								   return EMPTY_ERROR;
								 }
						}
        sFrame->getIndex = (sFrame->getIndex + 2) % MAX_QUEUE_LEN;                      //跳过帧尾TAIL1、TAIL2 2个字节						
		    sFrame->hasDataLen -= (*pLen+4) ;                                               //计算正确的帧数据长度
        return RW_OK;
    }
}

/**************************************************************************************************
* 名    称： void SysStrcat(char *dest, uint16_t destSize, char *src)
* 说    明： 将str字符串拼接到dest字符串结尾处
* 入口参数：
*				 @param1  *dest:     目的地址
*				 @param2  destSize:  目的空间的最大size
*				 @param3  *src:      源地址
  ****************************************************************************************************/
void SysStrcat(char *dest, uint16_t destSize, char *src)
{
	int dLen  = strlen(dest) ;
	int sLen  = strlen(src) ;

  if( (dLen+sLen) >= destSize)         //字符串拼接后将发生溢出
	  {
		  *(dest+destSize-sLen-1) = 0x00 ; //将目的字符串的最后添加字符串结尾
		}

/*************************全角符号转半角符号 *****************************/
//for (int i = 0; i < destSize; i+=4) {		
for (int i = 0; i < destSize; i++) {
  // 全角空格转换为半角空格	  全角和半角的空格的Unicode值相差12256
	if ((src[i] == '3')&&(src[i+1] == '0')&&(src[i+2] == '0')&&(src[i+3] == '0'))
	{src[i] = '0';src[i+1] = '0';src[i+2] = '2';src[i+3] = '0';}
	// 除空格外的其他字符转换
	else	if ((src[i] == 'F')&&(src[i+1] == 'F')&&(src[i+2] >= '0')&&(src[i+2] <= '5'))
  //65248是全角和半角的Unicode值相差   以下程序转换为+0x20	
	{src[i] = '0';src[i+1] = '0';src[i+2] += 2;}
																	}
/*************************END*****************************/		
	strcat(dest, src) ;                  //此时可以安全拼接字符串了
}

/*************************全角符号转半角符号 *****************************/
void fullTohalf(uint16_t destSize, char *src)
{
/*************************全角符号转半角符号 *****************************/
	int i = 0;
	while((src[i] != '\n')&&(i < destSize))i++;	//从换行符开始查找
	for ( i++; i < destSize; i+=4) 
		{
//		// 全角空格转换为半角空格	  全角和半角的空格的Unicode值相差12256
//		if ((src[i] == '3')&&(src[i+1] == '0')&&(src[i+2] == '0')&&(src[i+3] == '0'))
//		{src[i] = '0';src[i+1] = '0';src[i+2] = '2';src[i+3] = '0';i+=3;}
//		// 除空格外的其他字符转换
//		else	
			if ((src[i] == 'F')&&(src[i+1] == 'F')&&(src[i+2] >= '0')&&(src[i+2] <= '5'))
		//65248是全角和半角的Unicode值相差   以下程序转换为+0x20	
		{src[i] = '0';src[i+1] = '0';src[i+2] += 2;i+=3;}
		}		
}
/**************************************************************************************************
* 名    称： uint16_t CopyValues(uint8_t *dest, const uint8_t *src, uint8_t lot, uint8_t maxLen)
* 说    明： 从源地址scr向目的地址dst拷贝字符，遇到lot字符即结束拷贝,最多查询src后面maxLen长度的数据
* 入口参数：
*				 @param1  *dest:  目的地址
*				 @param2  *src:   源地址
*				 @param3  lot:    拷贝停止标志字符
*				 @param4  maxLen: 拷贝数据的最大长度
* 出口参数：
*				 @param1  uint16_t: 返回lot在源数组中的偏移量
  ************************************************************************************************************/
uint16_t CopyValues(uint8_t *dest, uint8_t *src, uint8_t lot, uint16_t maxLen)
{
	uint16_t i = 0;
	for(i = 0; *(src+i)!=lot; i++)
	{
		*(dest+i) = *(src+i);
		if(i >= (maxLen-1))
			break;                  //防止过多拷贝，造成死循环
	}
	return i ;
	//*(dest+i) = 0 ;           //添加字符串结尾
}

/**************************************************************************************************
* 名    称： void CopyStr(char *dest, char *src, char startLot, char endLot, uint16_t maxLen)
* 说    明： 从源地址scr向目的地址dst拷贝字符，从字符startLot(不包含startLot)开始，遇到endLot前一个字符即结束拷贝,最多查询src后面maxLen长度的数据
* 入口参数：
*				 @param1  *dest:    目的地址
*				 @param2  *src:     源地址
*				 @param3  startLot: 拷贝开始标志字符
*				 @param4  endLot:   拷贝停止标志字符
*				 @param5  maxLen:   拷贝数据的最大长度
  ************************************************************************************************************/
void CopyStr(char *dest, char *src, char startLot, char endLot, uint16_t maxLen)
{
	uint16_t j = 0;
	char *startAddr ;
	startAddr = strchr((const char *)src, startLot);
	for(j = 1; *(startAddr+j)!=endLot; j++)
	{
		*(dest+j-1) = *(startAddr+j);
		if(j > maxLen)
			break;//防止过多拷贝，在成死循环
	}
//	*(dest+j) = *(startAddr+j);
}

/**************************************************************************************************
* 名    称： void HexConvertToString(uint8_t * dest, uint8_t * src, uint16_t length)
* 功能说明： 将16进制数据转化为字符串数据
* 入口参数：
*				 @param1  *dest: 目的地址
*				 @param2  *src:  源地址
*				     @param3  length: 源地址数据长度
* 说    明： 例如将例如源数组{0x33,0x31}调用函数后得到目的字符串为“31”
  *************************************************************************************************/	
void HexConvertToString(uint8_t * dest, uint8_t *src, uint16_t length)
{
	uint8_t *xad;
	uint16_t i = 0;
	uint8_t ch;
	xad = src + length - 1;
	for(i = 0; i < length; i++,xad--)
		{
			ch = (*xad>>4)&0x0F;
			dest[i<<1] = ch+((ch<10)?'0':'7');
			ch = *xad & 0x0F;
			dest[(i<<1)+1] = ch + ((ch<10)?'0':'7');
			
		}
}

/**************************************************************************************************
* 名    称： void ByteToHexStr( uint8_t* dest, const uint8_t* src, uint16_t sourceLen)   
* 功能说明： 将16进制数据转化为16进制字符串数据
* 入口参数：
*				     @param1  *dest: 目的地址
*				     @param2  *src:  源地址
*				     @param3  sourceLen: 源地址数据长度
* 出口参数：  无
* 说    明：  例如源数组{0x43,0x21}调用函数后得到目的数组为{0x34,0x33,0x32,0x31}
* 调用实例：  ByteToHexStr(macAddressBytes.bytes, macAddressNBytes, 12)  ;
  *************************************************************************************************/	
void ByteToHexStr( uint8_t* dest, const uint8_t* src, uint16_t sourceLen)  
{  
    uint16_t i;  
    uint8_t highByte, lowByte;  
  
    for (i = 0; i < sourceLen; i++)  
    {  
        highByte = src[i] >> 4;  
        lowByte = src[i] & 0x0f;  
  
        highByte += 0x30;  
  
        if (highByte > 0x39)  
            dest[i * 2] = highByte + 0x07;  
        else  
            dest[i * 2] = highByte;  
  
        lowByte += 0x30;  
        if (lowByte > 0x39)  
            dest[i * 2 + 1] = lowByte + 0x07;  
        else  
            dest[i * 2 + 1] = lowByte;  
    }   
} 

/**************************************************************************************************
 ● itoa()：将整型值转换为字符串。

  ● ltoa()：将长整型值转换为字符串。

  ● ultoa()：将无符号长整型值转换为字符串。

  ● gcvt()：将浮点型数转换为字符串，取四舍五入。

  ● ecvt()：将双精度浮点型值转换为字符串，转换结果中不包含十进制小数点。

  ● fcvt()：指定位数为转换精度，其余同ecvt()。
* 名    称：  char *itoa(int value, char *string, int radix)
* 功能说明：  整形数据转字符串
* 入口参数：  int value, char *string, int radix
* 出口参数：  char *：返回的字符串指针
* 说    明：   
*		          radix=10 标示是10进制	非十进制，转换结果为0 ;  
* 					  例：d=-379 ;执行	itoa(d, buf, 10) ; 后buf="-379"
  *************************************************************************************************/
char *itoa(int value, char *string, int radix)
{
    int     i, d ;
    int     flag = 0 ;
    char    *ptr = string ;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0 ;
        return string ;
    }

    if (!value)
    {
        *ptr++ = 0x30 ;
        *ptr = 0 ;
        return string ;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-' ;

        /* Make the value positive. */
        value *= -1 ;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i ;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30) ;
            value -= (d * i) ;
            flag = 1 ;
        }
    }
    /* Null terminate the string. */
    *ptr = 0 ;

    return string ;

} /* NCL_Itoa */

/**************************************************************************************************
 gcvt()：将浮点型数转换为字符串，取四舍五入。
* 名    称：  *gcvt(double value, int ndigit, char *buf)
* 功能说明：  整形数据转字符串
* 入口参数：  double value 源浮点数值, int ndigit 精确小数点位数（四舍五入）, char *buf 目标缓存
* 出口参数：  char *：返回的字符串指针
* 说    明：   
*************************************************************************************************/
char *gcvt(double value, int ndigit, char *buf)
{
	int sign;			   // 符号标志
	int integer, linteger; // 整数部分
	double fraction;	   // 小数部分
	int count, k;		   // 计数器
	int i;				   // 循环变量
 
	// 判断符号
	if (value < 0)
	{
		sign = 1;		// 负数
		value = -value; // 取绝对值
		buf[0] = '-';	// 添加负号
	}
	else
	{
		sign = 0; // 正数或零
	}
 
	// 判断零
	if (value == 0)
	{
		buf[sign] = '0';	  // 添加零字符
		buf[sign + 1] = '\0'; // 添加结束符
		return buf;			  // 返回字符串指针
	}
 
	// 判断整数
	integer = (int)value;		// 取整数部分
	fraction = value - integer; // 取小数部分
	if (fraction == 0)
	{
		sprintf(buf + sign, "%d", integer); // 直接转换整数部分为字符串
		return buf;							// 返回字符串指针
	}
	// 处理小数
	if (integer == 0)
	{
		buf[sign] = '0';
		sign++;
		count = 0, k = -1; // 初始化计数器
		int j;
		while (fraction != 0 && count < ndigit)
		{								// 循环乘以10直到小数部分为零或者达到最大位数
			value *= 10;				// 乘以10
			integer = (int)value;		// 取整数部分
			fraction = value - integer; // 取小数部分
			count++;					// 计数器加一
		}
 
		// 处理四舍五入
		if (fraction >= 0.5)
		{			   // 如果小数部分大于等于0.5，就进位
			integer++; // 整数部分加一
			if (integer == pow(10, count))
			{			 // 如果整数部分等于10的count次方，说明进位后多了一位
				count++; // 计数器加一
			}
		}
		linteger = integer;
		while (integer > 0)
		{
			integer /= 10; // 将num除以10
 
			k++; // 将len加一
		}
 
		j = k + 1;
		for (; k < count; k++)
		{
			buf[sign + k - 1] = '0';
		}
 
		// 转换为字符串
		sprintf(buf + sign + count - j, "%d", linteger); // 转换整数部分为字符串
 
		// 插入小数点
 
		for (i = sign + count; i > sign; i--)
		{ // 从后往前移动count位字符，空出一位插入小数点
			buf[i] = buf[i - 1];
		}
		buf[sign] = '.'; // 插入小数点
 
		// 删除末尾的小数点或空格（如果有的话）
		i = sign + count + 1; // 定位到末尾字符的位置
 
		while (buf[i] == '.' || buf[i] == ' ')
		{ // 如果是小数点或空格，就删除它
			buf[i] = '\0';
			i--;
		}
		buf[i + 1] = '\0';
		//	printf("%d\n",k);
		return buf; // 返回字符串指针
	}
 
	count = 0, k = 0; // 初始化计数器
	while (fraction != 0 && count < ndigit)
	{								// 循环乘以10直到小数部分为零或者达到最大位数
		value *= 10;				// 乘以10
		integer = (int)value;		// 取整数部分
		fraction = value - integer; // 取小数部分
		count++;					// 计数器加一
	}
 
	// 处理四舍五入
	if (fraction >= 0.5)
	{			   // 如果小数部分大于等于0.5，就进位
		integer++; // 整数部分加一
		if (integer == pow(10, count))
		{			 // 如果整数部分等于10的count次方，说明进位后多了一位
			count++; // 计数器加一
		}
	}
 
	// 转换为字符串
	sprintf(buf + sign, "%d", integer); // 转换整数部分为字符串
 
	while (integer > 0)
	{
		integer /= 10; // 将num除以10
		k++;		   // 将len加一
	}
 
	// 插入小数点
 
	for (i = sign + k; i > (sign + k) - count; i--)
	{ // 从后往前移动count位字符，空出一位插入小数点
		buf[i] = buf[i - 1];
	}
	buf[i] = '.'; // 插入小数点
 
	// 删除末尾的小数点或空格（如果有的话）
	i = sign + k + 1; // 定位到末尾字符的位置
 
	while (buf[i] == '.' || buf[i] == ' ')
	{ // 如果是小数点或空格，就删除它
		buf[i] = '\0';
		i--;
	}
	buf[i + 1] = '\0';
	return buf; // 返回字符串指针
}
 
/**************************************************************************************************
* 名    称：  char * uitoa(unsigned int num, char *str) 
* 功能说明：  无符号整形数据转10进制字符串
* 入口参数：  int value, char *string
* 出口参数：  char *：返回的字符串指针
  *************************************************************************************************/
char *uitoa(unsigned int value, char *string)   
{    
		unsigned char i = 0, index = 0 ;
		unsigned char temp = 0 ;
		unsigned int bottom = 10000 ;
		for(i = 0; i < 5; i++)
			 {
				temp = value/bottom ;
				if(temp>0 || index>0) string[index++]=0x30+temp ;
				value %= bottom ;
				bottom /= 10 ;
			 } 
		if(index==0) string[index++]=0x30;
		 
		//string[index]='\0';
		return string;   
} 

/**************************************************************************************************
* 名    称：  int kmp(const char* _str, const char* _ptn)
* 功能说明：  从源地址—str中寻找字符串—ptn, 并返回偏移量
* 入口参数：
*				     @param1  *src: 源地址
*				     @param2  *_ptn: 指向子串的指针
* 出口参数：
*				     @param1  int: 
*                     @arg -1：源地址找不到子串
*                     @arg 非-1：子串相对源地址的偏移量
  *************************************************************************************************/
int kmp(const char *_str, const char *_ptn)
{
    uint16_t m = strlen((const char *)_str) ;
    uint16_t n = strlen((const char *)_ptn) ;
    uint16_t i = 0, j = 0 ;
    uint16_t loop = 0 ;
    
    char* next = (char*)portMalloc((size_t)(n*sizeof(char))) ;
    kmp_next2(_ptn, next) ;
    i = 0 ; j = 0 ;
    while(i < m && j < n)
    {
        loop++ ;
        if(_str[i] == _ptn[j])
				 { 
				   ++i ; 
				   ++j ; 
				 }
        else if(0 == j)
            ++i ;
        else
            j = next[j] ;
    }

    portFree(next) ; //next = NULL;

    if(j >= n)
        return i-n ;
    else
        return -1 ;
}

void kmp_next2(const char* _ptn, char* _next)
{
    uint16_t n = strlen((const char *)_ptn) ;
    uint16_t i, j ;
    
    if(n >= 1)
        _next[0] = 0 ;
    
    i = 1 ; 
		j = 0 ;
    while(i < n)
    {
        if(_ptn[i] == _ptn[j])
          {
            ++i ; 
						++j ;
            if(_ptn[i] != _ptn[j])
                _next[i] = j ;
            else
                _next[i] = _next[j] ;
          }
        else if(0 == j)
            _next[++i] = j ;
        else
            j = _next[j] ;
    }
}






