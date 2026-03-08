#ifndef  _BEEPx_H
#define  _BEEPx_H
#include "sys.h"

/*****************************************
*beep驱动可供外部使用的常变量
****************************************/
#define BEEP_PORT             PE             //蜂鸣器控制GPIO端口
#define BEEP_PIN              PIN0          //蜂鸣器控制GPIO引脚
#define BEEP_ON()             PEout(0) = 1  //蜂鸣器打开
#define BEEP_OFF()            PEout(0) = 0  //蜂鸣器关闭 

/*****************************************
*内部函数声明
****************************************/

/*****************************************
*对外接口函数声明
****************************************/
extern void Beep_Init(void) ;                //BEEP初始化   
extern void Beep_Blink(void) ;               //BEEP 关闭或者打开一段时间
extern void Beep_Reverse(void) ;             //BEEP开关状态切换

#endif

