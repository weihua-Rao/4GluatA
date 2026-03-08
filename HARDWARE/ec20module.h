#ifndef _EC20MODULE_H
#define _EC20MODULE_H

//#include "main.h"
#include <stdbool.h>  
#include "usart.h"  
#include "syslib.h"
/*****************************************
*ec20module内部常变量
****************************************/
//#define RS485_COM           COM2             //RS485映射的串口
#define  EC20_UART                COM3                 //STM32与EC20通信串口的映射
#define  WIFI_UART                COM4                 //STM32与WIFI通信串口的映射
#define  MODULE_CMDPACK_LEN       1288                 //EC20 module相关命令字符串的最大长度
#define  SIM_ICCID_LEN            25                   //存放 SIM卡ICCID号 数组的长度
#define  EC20_VER_LEN             20                   //粗放EC20模块 软件版本号 数组的长度
#define  EC20_VOL_LEN             4                    //存放EC20模块电压 数组的长度
#define  CSQ_LEN                  3                    //存放EC20模块信号质量 数组的长度
/*******************************************************************************/
//#define PA_PORT				PA
////#define CO2power          	PIN7           //PA7 CO2电源

///*EC20 3.6V供电稳压片使能引脚：高电平使能*/
//#define  EC20_POW_PORT            PB				//PE
//#define  EC20_POW_PIN             PIN15			//PIN3		//
//#define  EC20_POW                 PBout(EC20_POW_PIN)

//#define PC_PORT				PC
//#define motor_down          PIN6           //PC6	防冻罩电机反向	下降
////PC0 电池电压检测
////PC1 升降电机负载电压检测
////PD13 风力发电电压检测 需要改到PC2 PC3
////PD14 太阳能发电电压检测

//#define PD_PORT				PD
//#define RS485RD_1          PIN0           //PD0 RS485 第1个芯片 读写控制 温湿度
//#define RS485RD_2          PIN1           //PD1	RS485 第2个芯片 读写控制 重量风向。。。。。
//#define Temp18B20         PIN6           //PD6 18B20数据通信脚
//#define SparRelay1         PIN8           //PD9	备用1
//#define CCDRelay           PIN9			//摄像头融冰 
//#define PowerMainON        PIN10         //PD10电池供电开关
////#define delete2            PIN11         //PD11 桥接电源输出 已禁用
//#define PowerOut           PIN12           //PD12	上电启动输出
//#define motor_up           PIN15           //PD15	防冻罩电机正向	上升

//#define PE_PORT				PE
////													PIN0				蜂鸣器控制GPIO端口
//#define McuLed        		 PIN1			//MCU工作指示灯
//#define SimPowerKey        PIN2			//SIM7600重启控制口　高有效
//#define AngleS             PIN3			//角度仪信号信号继电器
//#define winddirection      PIN4			//风向控制继电器
//#define CO2power           PIN5     //CO2控制继电器
//#define windspeed          PIN6 			//风速控制继电器
//#define WeightSignal       PIN7 			//PIN7  	称重 温湿度信号切换继电器
//#define Power_weightON     PIN8			//称重电源输出开关 
//#define Power_waveON       PIN9			//温湿度模块电源开关 
//#define camera_power       PIN10			//摄像头电源开关
//#define delete2            PIN11       //桥接电源输出
//#define LineRelay          PIN12			//模拟线融冰
//#define Reset_weight       PIN13	//称重复位开关输出
//#define SparRelay2         PIN14           //PD8 备用2
//#define RestSim            PIN15	//复位sim7600
///*******************************************************************************/

///*EC20 硬件复位引脚：常态低电平，高电平脉冲复位EC20*/
//#define  EC20_RST_PORT            PE           //PE15  
//#define  STM32_LED                 PEout(PIN1) 

//#define  EC20_PWRKEY_PIN             PIN2		//PIN11
//#define  EC20_PWRKEY                 PEout(EC20_PWRKEY_PIN)

//#define  EC20_RST_PIN             PIN15		//PIN11
//#define  EC20_RST                 PEout(EC20_RST_PIN) 
/*****************************************
*ec20module用户自定义数据类型
****************************************/
typedef enum 
{
  AT_MODE      = 0 ,       									 //AT命令模式
	USER_MODE    = 1 ,	     									 //接收sever用户数据模式
} EC20RECMODE_e ;           								 //EC20串口工作模式

typedef struct
{
	uint8_t      cmdNum ;                      //命令序号
  char         *cmdStr ;                     //发送命令字符串
  uint16_t     timeout ;                     //接收回码超时时间t=revTimeout*100ms
  char         *trueStr ;                    //正确的回码子串
	volatile int trueOffset ;                  //正确回码字符子串在ec20AtBuf中的偏移地址
	char         *falseStr ;                   //指令处理失败返回的字符串指针
  RunResult    revResult ;                   //处理结果
  uint8_t      rtyNum ;                      //失败后重发次数
}EC20_CMD_DATA_s;                            //ec20通用命令收发参数结构体

typedef struct    
{
	char             Temper[6] ;  //温度
	char             Humidi[6] ;	//湿度
  u8             csq[CSQ_LEN] ;               //3+1bytes  存放信号质量CSQ
	char             simICCID[SIM_ICCID_LEN+1] ;    //25+1bytes 存放SIM卡的ICCID号 
}ModuleParam_s ;//EC20存放模块参数的结构体

enum eModuleCmdNum 
{
  AT_AT = 0, AT_ATF = 1, AT_IPR =2, AT_IFC =3, AT_ATI =4, AT_CSQ = 5, AT_COPS =6,
  AT_CPIN =7,AT_CIMI=8,AT_CNMI=9,AT_CPMS = 10, AT_CSCA=11,AT_TXT =12, AT_CSCS = 13, 
	AT_CSMP =14,AT_CHUP = 15, AT_VOLTE =16,AT_AUDCH =17,AT_CLVL=18,AT_VTD=19,AT_CLCC = 20, 
	AT_CTTSPRM=21,AT_PDU =22, AT_ICCID = 23,AT_CREG2 =24,AT_CREG =25, AT_CGREG2 =26,
	AT_CGREG =27,AT_CPOWD=28,AT_RETSET=29, AT_CFUN=30,AT_ATD=31,AT_ATH=32,AT_CTTS=33,AT_CMGD=34,AT_CSCASET=36
} ;      //枚举ec20模块Module相关指令

/*****************************************
*ec20可供外部使用的常变量
****************************************/
#define  MAX_IP_LEN           20             													//IPV4 IP地址的最大长度
#define  PORT_MAXLEN          6		//6                                       //IPV4 端口的最大长度（0-65535）
#define  EC20_ATBUF_LEN       350//320            													//ec20AtBuf缓冲区长度
#define  EC20_HTTPBUF_LEN     250//320            													//ec20HTTPBUF缓冲区长度

extern ModuleParam_s        sEc20Param ;     													//声明ModuleParam_s结构体变量sEc20Param，存放EC20一些信息，供上层调用
extern volatile bool        httpDataMode ;              							//串口模式：接收http数据模式
extern char                 ec20AtBuf[EC20_ATBUF_LEN] ; 							//EC20 AT命令回码存放缓冲区
extern char                 ec20HttpBuf[EC20_HTTPBUF_LEN] ; 					//EC20 AT命令回码存放缓冲区
extern FrameQueue_s         SIMQueue ;                								//环形帧缓冲区，存储模块返回的tcp相关数据
//extern FrameQueue_s         sUrcQueue ;                 							//环形帧缓冲区，存储模块返回的URC数据
extern FrameQueue_s         MQTT_PublishQueue ;

/*****************************************
*内部函数声明
****************************************/
RunResult EC20_SendModuleCmd( uint8_t cmdNum, char *format,... ) ; 		// EC20通过串口发送module相关命令
void      EC20_Uart_Init(void) ;                                   		//初始化EC20_UART
void      EC20_GPIO_Init( void ) ;                                 		//初始化EC20相关控制IO
void      EC20_POWON(void) ;                                       		//EC20 3.6V供电稳压片使能上电
void      EC20_POWOFF(void) ;                                      		//EC20 3.6V供电稳压片失能断电
RunResult EC20_START(void) ;                                       		//EC20开机上电、等待串口“RDY”回码
//RunResult Fun_N32_AT( void ) ;                                 		//mcu与EC20通过EC20_UART握手
//RunResult EC20_CloseEcho(void) ;                                   		//关闭EC20 AT命令的回显
void      Ec20ReceiveFrameCallback(char *recvBuf, uint16_t recvLen) ; //EC20_UART串口帧中断回调函数

/*****************************************
*对外接口函数声明
****************************************/
extern void      Ec20AtBufReset(void) ;                        				//ec20AtBuf缓冲区 初始化
extern void 		 Ec20HttpBufReset(void) ;                             //ec20HttpBuf缓冲区 初始化
extern RunResult EC20_CLOSE(void) ;                                   //EC20关机流程 软关机+断电
//extern RunResult EC20_Query_SoftRelese(char *version) ;               //获取EC20的软件版本号
//extern RunResult EC20_Query_Voltage(char *voltage) ;                  //获取EC20的供电电压
extern RunResult EC20_Module_Init( void ) ;                           //EC20模块相关初始化
extern RunResult SimComInti(char *errInfo, uint8_t errLen);

#endif
