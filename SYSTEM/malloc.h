#ifndef __MALLOC_H
#define __MALLOC_H
#include "n32g45x.h"

/****************************************************************************
* 供外部是用的常变量
****************************************************************************/  
//#include <stddef.h>
#ifndef NULL
#define NULL 0
#endif

/*定义两个内存池)*/
#define SRAMIN	  0		//内部内存池
#define SRAMEX    1		//外部内存池 
#define SRAMBANK 	2	  //定义支持的SRAM块数.	

/********************************************************************************
*内部函数声明
*********************************************************************************/
void MyMemset(void *s, u8 c, u32 count);	 //设置内存
void MyMemcpy(void *des, void *src, u32 n);//复制内存     
u32  MyMenMalloc(u8 memx, u32 size);	     //内存分配(内部调用)
u8   MyMemFree(u8 memx, u32 offset);		   //内存释放(内部调用)

/********************************************************************************
*对外接口函数声明
*********************************************************************************/
extern void  MyMenInit(u8 memx);				              //内存管理初始化函数(外/内部调用)
extern void *MyMalloc(u8 memx, u32 size);			        //内存分配(外部调用)
extern void  MyFree(u8 memx, void *ptr);  			      //内存释放(外部调用)
extern void *MyRealloc(u8 memx, void *ptr, u32 size); //重新分配内存(外部调用)
extern u8    MyMenPerused(u8 memx);				            //获得内存使用率(外/内部调用) 



#endif














