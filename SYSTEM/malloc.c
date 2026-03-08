#include "malloc.h"	 

/********************************************************************************
  * @file    malloc.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   STM32内部RAM 动态内存分配驱动
  ******************************************************************************
*******************************************************************************/
#define FREERTOS_MALLOC		//使用库函数heap_4 自带的内存管理
/****************************************************************************
* 内部使用的相关常变量定义
****************************************************************************/
/*mem1内存参数设定.mem1完全处于 内部SRAM 里面*/
#define MEM1_BLOCK_SIZE		 32  	  					                  //内存块大小为32字节
#define MEM1_MAX_SIZE			(11*1024)  						              //最大管理内存 20K
#define MEM1_ALLOC_TABLE_SIZE	(MEM1_MAX_SIZE/MEM1_BLOCK_SIZE) //内存表大小

/*mem2内存参数设定.mem2的内存池处于 外部SRAM 里面*/
#define MEM2_BLOCK_SIZE			32  	  						              //内存块大小为32字节
#define MEM2_MAX_SIZE			 (32)  						                  //最大管理内存960K  没有外部RAM则不分配空间，否则会导致编译出来的ZI-data很大，不好判断内存占用
#define MEM2_ALLOC_TABLE_SIZE	(MEM2_MAX_SIZE/MEM2_BLOCK_SIZE) //内存表大小 

/*内存池(32字节对齐)*/
__align(32) u8 mem1base[MEM1_MAX_SIZE];												                       //内部SRAM内存池,__align(32)需要内存32bits对齐
__align(32) u8 mem2base[MEM2_MAX_SIZE] __attribute__((at(0X68000000)));					     //外部SRAM内存池，__attribute__限定内存起始地址

/*内存管理表*/
u16 mem1mapbase[MEM1_ALLOC_TABLE_SIZE];												                        //内部SRAM内存池MAP
u16 mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((at(0X68000000+MEM2_MAX_SIZE)));	//外部SRAM内存池MAP

/*内存管理参数*/
const u32 memtblsize[SRAMBANK] = {MEM1_ALLOC_TABLE_SIZE,MEM2_ALLOC_TABLE_SIZE};			  //内存表大小
const u32 memblksize[SRAMBANK] = {MEM1_BLOCK_SIZE,MEM2_BLOCK_SIZE};						        //内存分块大小
const u32 memsize[SRAMBANK] = {MEM1_MAX_SIZE,MEM2_MAX_SIZE};								          //内存总大小

/*内存管理控制器类型定义*/
struct MallocDev_s 
{
	void (*init)(u8);					    //初始化函数指针
	u8 (*perused)(u8);		  	    //内存使用率函数指针
	u8 	*membase[SRAMBANK];				//内存池基地址 管理SRAMBANK个区域的内存
	u16 *memmap[SRAMBANK]; 				//内存管理状态表
	u8   memrdy[SRAMBANK]; 				//内存管理是否就绪
} ;

/*定义内存管理控制器变量*/
struct MallocDev_s sMallco_dev=
{
	MyMenInit,				            //内存初始化
	MyMenPerused,							    //内存使用率
	mem1base,mem2base,					  //内存池
	mem1mapbase,mem2mapbase,		  //内存管理状态表
	0,0,  		 									  //内存管理未就绪
};
/****************************************************************************/

/****************************************************************************
* 名    称：void MyMemcpy(void *des, void *src, u32 n）
* 功    能：复制内存
* 入口参数：
*           @param1 *des:目的地址
*           @param2 *src:源地址
*           @param3  n:  需要复制的内存长度(字节为单位)	
****************************************************************************/
void MyMemcpy(void *des, void *src, u32 n)  
{  
    u8 *xdes = des ;
	  u8 *xsrc = src ; 
    while(n--)
		{
     *xdes++ = *xsrc++ ; 		
		}			 
}  

/****************************************************************************
* 名    称：void MyMemset(void *s,u8 c,u32 count)
* 功    能：设置内存
* 入口参数：
*           @param1 *s:内存首地址
*           @param2 c :要设置的值
*           @param3 count:需要设置的内存大小(字节为单位)	
****************************************************************************/
void MyMemset(void *s, u8 c, u32 count)  
{  
    u8 *xs = s ;  
    while(count--)
		{
		 *xs++ = c; 
		} 
}	   

/****************************************************************************
* 名    称：void MyMenInit(u8 memx)
* 功    能：内存管理初始化 
* 入口参数：
*           @param1 memx:所属内存块
****************************************************************************/
void MyMenInit(u8 memx)  
{  
   MyMemset(sMallco_dev.memmap[memx], 0,memtblsize[memx]*2);//内存状态表数据清零  
	 MyMemset(sMallco_dev.membase[memx], 0,memsize[memx]);	  //内存池所有数据清零  
	 sMallco_dev.memrdy[memx]=1;								              //内存管理初始化OK  
}  

/****************************************************************************
* 名    称：u8 MyMenPerused(u8 memx) 
* 功    能：获取内存使用率 
* 入口参数：
*           @param1 memx:所属内存块
* 出口参数：
*           @param1 返回值:使用率(0~100)
****************************************************************************/
u8 MyMenPerused(u8 memx)  
{  
    u32 used = 0 ;  
    u32 i ;  
    for(i=0; i<memtblsize[memx]; i++)  
    {  
        if(sMallco_dev.memmap[memx][i])used++; 
    } 
    return (used*100)/(memtblsize[memx]) ;  
}  

/****************************************************************************
* 名    称：u8 MyMenMalloc(u8 memx) 
* 功    能：内存分配(内部调用)
* 入口参数：
*           @param1 memx:所属内存块
*           @param2 size:要分配的内存大小(字节)
* 出口参数：
*           @param1 返回值:0XFFFFFFFF,代表错误;其他,内存偏移地址
****************************************************************************/
u32 MyMenMalloc(u8 memx,u32 size)  
{  
    signed long offset = 0 ;  
    u32 nmemb;	                                          //需要的内存块数  
	  u32 cmemb = 0 ;																				//连续空内存块数
    u32 i ;  
    if(!sMallco_dev.memrdy[memx]) sMallco_dev.init(memx); //未初始化,先执行初始化 
    if(size==0) return 0XFFFFFFFF ;												//不需要分配
    nmemb = size/memblksize[memx] ;  											//获取需要分配的连续内存块数
    if(size%memblksize[memx])nmemb++ ;  
    for(offset=memtblsize[memx]-1; offset>=0; offset--)		//搜索整个内存控制区  
    {     
			if(!sMallco_dev.memmap[memx][offset]) cmemb++;			//连续空内存块数增加
			else cmemb=0;																				//连续内存块清零
			if(cmemb == nmemb)																	//找到了连续nmemb个空内存块
			{
							for(i=0; i<nmemb; i++)  										//标注内存块非空 
							{  
									sMallco_dev.memmap[memx][offset+i] = nmemb ;  
							}  
							return (offset*memblksize[memx]) ;					//返回偏移地址  
			}
    }  
    return 0XFFFFFFFF ;																		//未找到符合分配条件的内存块  
}  
  
/****************************************************************************
* 名    称：u8 MyMemFree(u8 memx,u32 offset)  
* 功    能：释放内存(内部调用)
* 入口参数：
*           @param1 memx:所属内存块
*           @param2 offset:内存地址偏移
* 出口参数：
*           @param1 返回值:0,释放成功;1,释放失败;
****************************************************************************/
u8 MyMemFree(u8 memx, u32 offset)  
{  
    int i;  
    if(!sMallco_dev.memrdy[memx])										//未初始化,先执行初始化
	  {
		    sMallco_dev.init(memx) ;    
        return 1 ;																	//未初始化  
    }  
    if(offset<memsize[memx])												//偏移在内存池内. 
    {  
        int index=offset/memblksize[memx] ;					//偏移所在内存块号码  
        int nmemb=sMallco_dev.memmap[memx][index] ;	//内存块数量
        for(i=0;i<nmemb;i++)  											//内存块清零
        {  
            sMallco_dev.memmap[memx][index+i]=0 ;  
        }  
        return 0 ;  
    }else return 2 ;																//偏移超区了.  
}  

/****************************************************************************
* 名    称：void MyFree(u8 memx,void *ptr) 
* 功    能：释放内存(外部调用) 
* 入口参数：
*           @param1 memx:所属内存块
*           @param2 ptr:内存首地址 
****************************************************************************/
void MyFree(u8 memx, void *ptr)  
{  
	  u32 offset ;   
	  if(ptr==NULL) return ;                         //地址为0.  
 	  offset=(u32)ptr-(u32)sMallco_dev.membase[memx] ;     
    MyMemFree(memx,offset) ;	                     //释放内存      
}  

/****************************************************************************
* 名    称：void *MyMalloc(u8 memx,u32 size) 
* 功    能：分配内存(外部调用) 
* 入口参数：
*           @param1 memx:所属内存块
*           @param2 size:内存大小(字节)
* 出口参数：
*           @param1 返回值:分配到的内存首地址.
****************************************************************************/
void *MyMalloc(u8 memx, u32 size)  
{  
    u32 offset ;   
	  offset = MyMenMalloc(memx,size) ;  	   	 	   
    if(offset==0XFFFFFFFF) return NULL ;  
    else return (void*)((u32)sMallco_dev.membase[memx]+offset) ;  
}  

/****************************************************************************
* 名    称：void *MyRealloc(u8 memx,void *ptr,u32 size)  
* 功    能：重新分配内存(外部调用)
* 入口参数：
*           @param1 memx:所属内存块
*           @param2 *ptr:旧内存首地址
*           @param3 size:要分配的内存大小(字节)
* 出口参数：
*           @param1 返回值:新分配到的内存首地址
****************************************************************************/
void *MyRealloc(u8 memx, void *ptr, u32 size)  
{  
    u32 offset ;    
    offset = MyMenMalloc(memx,size) ;   	
    if(offset==0XFFFFFFFF) return NULL ;     
    else  
    {  									   
	    MyMemcpy((void*)((u32)sMallco_dev.membase[memx]+offset),ptr,size) ;	//拷贝旧内存内容到新内存   
      MyFree(memx,ptr) ;  											  		                    //释放旧内存
      return (void*)((u32)sMallco_dev.membase[memx]+offset) ;  			    	//返回新内存首地址
    }  
}












