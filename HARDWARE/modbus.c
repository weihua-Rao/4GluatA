
/***************************************************************************************
  Copyright (C), 2013-20xx, Wen Qian Tech. Co., Ltd.
  File name:   stc_modbus_master.c
  Author:      Liang-wen-jun   
  Version:     v1.0       
  Date:        2013.01.20
  Description: modbus 主机测试程序
******************************************************************************************/

/******************************************************************************************
                                    外部头文件声明
******************************************************************************************/

#include "modbus.h"

/*****************************************************************************************
                                    外部变量声明
******************************************************************************************/

u8 COIL_STATEUS[10];				 //线圈状态
u8 DISCRETE_STATEUS[10];			 //离散量状态
u8 INPUT_REG_STATEUS[10];			 //输入寄存器状态
u8 KEEP_REG_STATEUS[10];			 //保持寄存器状态

//extern PF_SEND  mb_send;				 //串口发送函数

/*****************************************************************************************
                                    内部类型定义
******************************************************************************************/

typedef struct MB_RD_CMD_TAG        //读寄存器或线圈帧信息结构类型
{   
    u8     slave_addr;
    u8     func_code;
    u16    rd_addr;                 //读寄存器基地址，注意大小端
	u16    target_num;
	u16    crc16;    
}MB_READ_CMD;

typedef struct MB_RD_RESPOND_TAG    //读寄存器或线圈的响应帧信息结构类型
{   
    u8     slave_addr;
    u8     func_code;
    u8     byte_num;
	u8     *p_data;
	u16    crc16;    
}MB_RD_RESPOND;

typedef struct MB_WR_SINGER_TAG     //写单个线圈或寄存器结构类型
{    
    u8     slave_addr;
    u8     func_code;
    u16    wr_addr;                 //写寄存器基地址，注意大小端
	u16    wr_data;
	u16    crc16;

}MB_WR_SINGLE_CMD;


typedef struct MB_WR_RESPOND_TAG    //写单个线圈或寄存器的响应结构类型
{   
    u8     slave_addr;
    u8     func_code;
    u16    wr_addr;                 //写寄存器基地址，注意大小端
	u16    wr_value;                //输出的值
	u16    crc16;    
}MB_WR_SINGER_RESPOND;


/*****************************************************************************************
                                     内部函数声明
******************************************************************************************/

static u8 mb_maseter_cmd01_prepare(MB_READ_CMD *p_mb_cmd);
static u8 mb_maseter_cmd03_prepare(MB_READ_CMD *p_mb_cmd);
static u8 mb_maseter_cmd05_prepare(MB_WR_SINGLE_CMD *p_mb_cmd);

/*****************************************************************************************
                                     全局函数声明
******************************************************************************************/

u8 mb_master_rec01_process(u8 *p_rec_data,u8 len);
u8 mb_master_rec03_process(u8 *p_rec_data,u8 len);
u8 mb_master_rec05_process(u8 *p_rec_data,u8 len);


/*****************************************************************************************
                                 以下为内部函数定义区
******************************************************************************************/

/*****************************************************************************************
* 函数名  : mb_master_fun01_test
* 功  能  : modbus功能码01测试  读取线圈连续状态BIT
* 参  数  : 无
* 返回值  : 无
* 说  明  : 根据实际modbus通信需求，填写通信数据
******************************************************************************************/

void  mb_master_fun01_test(void)
{
    u8 len;
    MB_READ_CMD mb_cmd;


    mb_cmd.slave_addr = 0x01;         //从机地址
    mb_cmd.rd_addr    = 0x0000;       //读取的起始地址
    mb_cmd.target_num = 0x0008;       //线圈数量
    
    len = mb_maseter_cmd01_prepare(&mb_cmd);                  //准备功能码01的其它协议数据

    if(NULL != mb_send)
    {
        mb_send((u8*)&mb_cmd,len);                            //发送一帧modbus命令
    }

    delay_ms(200);                                             //延时等待从机执行完成 
    
        
    return;
}

/*****************************************************************************************
* 函数名  : mb_master_fun03_test
* 功  能  : modbus功能码03测试  读取寄存器连续块的内容
* 参  数  : 无
* 返回值  : 无
* 说  明  : 根据实际modbus通信需求，填写通信数据
******************************************************************************************/
void  mb_master_fun03_test(void)
{
    u8 len;
    MB_READ_CMD xdata mb_cmd;


    mb_cmd.slave_addr = 0x01;         //从机地址
    mb_cmd.rd_addr    = 0x0000;       //读取的起始地址
    mb_cmd.target_num = 0x0001;       //寄存器数量
    
    len = mb_maseter_cmd03_prepare(&mb_cmd);                //准备功能码03的其它协议数据

    if(NULL != mb_send)
    {
        mb_send((u8*)&mb_cmd,len);                          //发送一帧modbus命令
    }

    delay_ms(200);                                           //延时等待从机执行完成 
    
    
    return;
}

/*****************************************************************************************
* 函数名  : mb_master_fun05_test
* 功  能  : modbus功能码05测试  写单个线圈 BIT
* 参  数  : 无
* 返回值  : 无
* 说  明  : 根据实际modbus通信需求，填写通信数据
******************************************************************************************/
void  mb_master_fun05_test(void)
{
    u8 len;
    MB_WR_SINGLE_CMD xdata mb_cmd;


    mb_cmd.slave_addr = 0x01;         //从机地址
    mb_cmd.wr_addr    = 0x0000;       //写线圈的起始地址
    mb_cmd.wr_data    = 0xFF00;       //写入的值
    
    len = mb_maseter_cmd05_prepare(&mb_cmd);        //准备功能码05的其它协议数据

    if(NULL != mb_send)
    {
        mb_send((u8*)&mb_cmd,len);                  //发送一帧modbus命令
    }

    delay_ms(200);                                   //延时等待从机执行完成  
    
    
    return;
}

/*****************************************************************************************
* 函数名  : mb_maseter_cmd01_prepare
* 功  能  : modbus功能码01测试,准备01功能码和crc数据    
* 参  数  : p_mb_cmd
* 返回值  : u8
* 说  明  : 根据实际modbus通信需求，填写通信数据
******************************************************************************************/

STATIC u8 mb_maseter_cmd01_prepare(MB_READ_CMD *p_mb_cmd)
{
    u8 len;

    p_mb_cmd->func_code  = 0x01;                          // 功能码 
    //switch_byte(&p_mb_cmd->rd_addr);					  //大端模式下不需要进行高低字节交换，keil为大端模式
    //switch_byte(&p_mb_cmd->target_num);  				  //大端模式下不需要进行高低字节交换，keil为大端模式
    len = sizeof(MB_READ_CMD); 
    p_mb_cmd->crc16 = Cal_CRC16((u8*)p_mb_cmd,len-2);     //CRC16计算
    //switch_byte(&p_mb_cmd->crc16);					  //大端模式下不需要进行高低字节交换，keil为大端模式

    return len;
}

/*****************************************************************************************
* 函数名  : mb_maseter_cmd03_prepare
* 功  能  : modbus功能码03测试,准备03功能码和crc数据    
* 参  数  : 无
* 返回值  : 无
* 说  明  : 根据实际modbus通信需求，填写通信数据
******************************************************************************************/

STATIC u8 mb_maseter_cmd03_prepare(MB_READ_CMD *p_mb_cmd)
{
    u8 len;

    p_mb_cmd->func_code  = 0x03;                          //功能码 
    //switch_byte(&p_mb_cmd->rd_addr);					  //大端模式下不需要进行高低字节交换，keil为大端模式
    //switch_byte(&p_mb_cmd->target_num);  
    len = sizeof(MB_READ_CMD); 
    p_mb_cmd->crc16 = Cal_CRC16((u8*)p_mb_cmd,len-2);     //CRC16计算
    //switch_byte(&p_mb_cmd->crc16);

    return len;
}

/*****************************************************************************************
* 函数名  : mb_maseter_cmd05_prepare
* 功  能  : modbus功能码05测试，准备05功能码和crc数据  
* 参  数  : 无
* 返回值  : 无
* 说  明  : 根据实际modbus通信需求，填写通信数据
******************************************************************************************/

STATIC u8 mb_maseter_cmd05_prepare(MB_WR_SINGLE_CMD *p_mb_cmd)
{
    u8 len;

    p_mb_cmd->func_code  = 0x05;                          //功能码 
    //switch_byte(&p_mb_cmd->wr_addr);					  //大端模式下不需要进行高低字节交换，keil为大端模式
    //switch_byte(&p_mb_cmd->wr_data);  
    len = sizeof(MB_WR_SINGLE_CMD); 
    p_mb_cmd->crc16 = Cal_CRC16((u8*)p_mb_cmd,len-2);     //CRC16计算
    //switch_byte(&p_mb_cmd->crc16);

    return len;
}

/*****************************************************************************************
* 函数名  : mb_master_rec01_process
* 功  能  : modbus功能码01接收从机数据处理  
* 参  数  : p_rec_data，len
* 返回值  : u8
* 说  明  : 根据实际modbus通信需求，填写通信数据
******************************************************************************************/

u8 mb_master_rec01_process(u8 *p_rec_data,u8 len)
{
    u16 xdata tmp_crc;
    u16 xdata rec_crc;  
    u8  *p_mb_data;
    u8  xdata byte_num;

    if((NULL == p_rec_data) || (len < 6))
    {
        return FALSE;
    }
    
    rec_crc = (p_rec_data[len-2]<<8) + p_rec_data[len-1];
    tmp_crc = Cal_CRC16(p_rec_data ,len-2);
    
    if(rec_crc != tmp_crc)
    {
        return FALSE;
    }

    byte_num  = p_rec_data[2];		//读取的字节数
    p_mb_data = p_rec_data + 3;

    memcpy(COIL_STATEUS,p_mb_data,byte_num);

    return TRUE;
}

/*****************************************************************************************
* 函数名  : mb_master_rec03_process
* 功  能  : modbus功能码03接收从机数据处理  
* 参  数  : p_rec_data，len
* 返回值  : u8
* 说  明  : 根据实际modbus通信需求，填写通信数据
******************************************************************************************/

u8 mb_master_rec03_process(u8 *p_rec_data,u8 len)
{
    u16 xdata tmp_crc;
    u16 xdata rec_crc;  
    u8  *p_mb_data;
    u8  xdata byte_num;

    if((NULL == p_rec_data) || (len < 6))
    {
        return FALSE;
    }
    
    rec_crc = (p_rec_data[len-2]<<8) + p_rec_data[len-1];
    tmp_crc = Cal_CRC16(p_rec_data ,len-2);
    
    if(rec_crc != tmp_crc)
    {
        return FALSE;
    }

    byte_num  = p_rec_data[2];    //读取的字节数，是寄存器数的两倍
    p_mb_data = p_rec_data + 3;   //寄存器数据起始地址

    memcpy(KEEP_REG_STATEUS,p_mb_data,byte_num);	  //将读取到的数据写到KEEP_REG_STATEUS数组中

    return TRUE;
}

/*****************************************************************************************
* 函数名  : mb_master_rec05_process
* 功  能  : modbus功能码05接收从机数据处理  
* 参  数  : p_rec_data，len
* 返回值  : u8
* 说  明  : 根据实际modbus通信需求，填写通信数据
******************************************************************************************/

u8 mb_master_rec05_process(u8 *p_rec_data,u8 len)
{
    u16 xdata tmp_crc;
    u16 xdata rec_crc;  

    if((NULL == p_rec_data) || (len < 8))
    {
        return FALSE;
    }
    
    rec_crc = (p_rec_data[len-2]<<8) + p_rec_data[len-1];
    tmp_crc = Cal_CRC16(p_rec_data ,len-2);
    
    if(rec_crc != tmp_crc)
    {
        return FALSE;
    }

    return TRUE;		     //返回真，表明本次写从机操作正常
}

/*****************************************************************************************
                                         END
******************************************************************************************/
