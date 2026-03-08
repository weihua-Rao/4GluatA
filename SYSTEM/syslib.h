#ifndef __SYSLIB_H
#define __SYSLIB_H

//#include <stdint.h>
//#include <string.h>
#include "n32g45x.h" 

/********************************************************
*常变量定义
*********************************************************/
/*二级stringfication,将宏定义转为字符串*/
#define STR1(R)  #R                                   //#：会把参数转换为字符串
#define STR2(R)  STR1(R)                              //二级stringfication,将宏定义转为字符串

#define ENUM_CHIP_TYPE_CASE(x)   case x: return(#x) ; //#：会把参数转换为字符串
typedef enum {InParamErr = 0, OutParamErr = 1, RUNERR = 2, RUNOK=3, TIMEOUT=4, NETERR=5} RunResult ; //RunResult枚举类型，用于枚举函数执行结果类型

/**************************************************************************************************
* 名    称：  static inline const char *RunResultToString(RunResult result)
* 功能说明：  输出枚举成员名的字符串指针。
* 入口参数：  RunResult类型的枚举
* 出口参数：  为枚举的成员名字符串指针 
**************************************************************************************************/
//static inline const char *RunResultToString(RunResult result)
static  char *RunResultToString(RunResult result)
{
    switch (result)
    {
		    ENUM_CHIP_TYPE_CASE(InParamErr)
        ENUM_CHIP_TYPE_CASE(OutParamErr)
        ENUM_CHIP_TYPE_CASE(RUNERR)
        ENUM_CHIP_TYPE_CASE(RUNOK)
			  ENUM_CHIP_TYPE_CASE(TIMEOUT)
				ENUM_CHIP_TYPE_CASE(NETERR)
    }
    return "未知错误";
}

/*环形缓冲区代码*/
#define MAX_QUEUE_LEN  (2048)   //帧环形缓冲区长度bytes 
#define RW_OK   	      0       //帧环形缓冲区读数据成功
#define FULL_ERROR      1       //帧环形缓冲区写数据溢出
#define EMPTY_ERROR     2       //帧环形缓冲区读数据空
#define HEAD1           0x12    //帧数据帧头第一个字节
#define HEAD2           0x34    //帧数据帧头第二个字节
#define TAIL1           0x56    //帧数据帧尾第一个字节
#define TAIL2           0x78    //帧数据帧尾第二个字节

/********************************************************
*自定义数据类型
*********************************************************/
typedef struct
{
 volatile uint16_t  getIndex ;             //帧环形缓冲区读指针
 volatile uint16_t  insertIndex ;          //帧环形缓冲区写指针
 volatile uint16_t  hasDataLen ;           //帧环形缓冲区现有数据长度
          char      dataBuf[MAX_QUEUE_LEN];//帧环形缓冲区数据区
} FrameQueue_s ;                           //帧环形缓冲区结构体

/*****************************************
*内部函数声明
****************************************/
void kmp_next2(const char* _ptn, char* _next) ;

/*****************************************
*对外接口函数声明
****************************************/
extern void    InitQueueMem(FrameQueue_s *sFrame) ;                                        //环形缓冲区初始化
extern uint8_t InsertQueueMemData(FrameQueue_s *sFrame, char *data, uint16_t len) ;        //插入帧数据到环形缓冲区中
extern uint8_t GetQueueMemData(FrameQueue_s* sFrame, char* data, uint16_t* pLen);          //从环形缓冲区中取出一帧数据

extern void SysStrcat(char *dest, uint16_t destSize, char *src) ;                          //将destSize长度的数据src拷贝到dest中
extern void CopyStr(char *dest, char *src, char startLot, char endLot, uint16_t maxLen)  ; //从src的startLot字符开始，endLot字符结束的数据拷贝到dest中
extern uint16_t CopyValues(uint8_t *dest, uint8_t *src, uint8_t lot, uint16_t maxLen) ;    //将src中的数据拷贝到dest中，遇到lot或者拷贝长度大约maxLen则停止拷贝
extern void HexConvertToString(uint8_t * dest, uint8_t * src, uint16_t length);            //hex数组转字符串
extern void ByteToHexStr( uint8_t* dest, const uint8_t* src, uint16_t sourceLen)       ;   //byte转字符串
extern int  kmp(const char* _str, const char* _ptn) ;                                //查找子串
//extern uint16_t atoi(const char *nptr)  ;
extern char * itoa(int value, char *string, int radix) ; 
extern char * uitoa(unsigned int value, char *string) ;

#endif

