#ifndef _USER_TCP_H
#define _USER_TCP_H
#include "ec20tcp.h"
#include "mac.h"

/*****************************************
*user_tcp.h常变量
****************************************/
//#define  TCP_CONNECTID0              0                //TCP0通道 的ID
//#define  TCP_CONNECTID0_SERVERIP0   "121.5.217.252"    //tcp://www.szjcd.top:9001 TCP0通道 主TCP服务器ip地址tcp://www.szjcd.top:9001
//#define  TCP_CONNECTID0_SERVERPORT0  7019            //"icemonitor"TCP0通道 主TCP服务器端口号
//#define  TCP_CONNECTID0_LOCALPORT0   6000             //"123456"TCP0通道 的本地端口

#define  TCP_CONNECTID0              0                //TCP0通道 的ID
#define  TCP_SERVERIP   "www.szjcd.top"    // TCP0通道 主TCP服务器ip地址tcp://www.szjcd.top:9001
#define  TCP_SERVERPORT  "9001"            //"icemonitor"TCP0通道 主TCP服务器端口号
#define  TCP_LOCALPORT   "6000"             //"123456"TCP0通道 的本地端口
#define  MQTT_Name  			"szjcd"            //"icemonitor"TCP0通道 主TCP服务器端口号
#define  MQTT_PWD    "123456"            //TCP0通道 的本地端口

/*****************************************
*用户自定义变量类型
****************************************/
typedef enum 
{
	  BOOTTOTCPFRAME   = 'B' ,       //(硬件TX->Tcp server/串口上位机) BOOT中所有tcp上行的数据包
	
		TCPHEARTFRAME    = 'H' ,       //(硬件TX->Tcp server)TCP心跳数据包
		TCPLOGFRAME      = 'L' ,       //(硬件TX->Tcp server)Log数据包	
	
		TCPSERVERACK     = 'A' ,       //(Tcp server->硬件RX)硬件上行的TCP心跳数据包、Log数据包，Tcp server下发的ACK确认包
	
	  TCPCMDFRAME    	 = 'C' ,       //(Tcp server->硬件RX)用户控制命令帧
		COMCMDFRAME      = 'c' ,       //(串口上位机->硬件RX)用户控制命令帧
	
		TCPCMDBACKFRAME  = 'R' ,       //(硬件TX->Tcp server)用户数据帧帧，硬件回复帧
	  COMCMDBACKFRAME  = 'r'         //(硬件TX->串口上位机)用户数据帧帧，硬件回复帧
}UPDATATYPE_e ;                    //用户数据帧类型枚举

typedef struct
{
		char  				head ;                 //TCP数据帧头
		UPDATATYPE_e  frameType ;            //数据帧类型  UPDATATYPE_e枚举类型 
		char  				loadLen[3] ;           //负载数据的长度，从loadHead~loadTail（不包含loadHead和loadTail）字节长度      
		uint8_t 			frameNum ;             //数据帧序号（字符串‘0’~‘9’循环）
	  char          macHead[4] ;           //mac地址标志头字符串 固定字符串“MAC:”
	  uint8_t       macid[MAC_BYTES_LEN] ; //设备的mac地址
    char          macTail ;              //MAC地址结尾标识符       	
		char  				cmdCode[2] ;           //命令操作码
		char 				  loadHead ;	           //负载数据头 '&'
//	char          *loadBuf ;             //负载数据缓冲区地址
		char 				  loadTail ;	           //负载数据尾 ‘$’
		char  				reserve ; 	           //预留1byte 固定为‘X’
    char  				tail ;     	           //TCP数据帧尾   
}TcpFrame_S ;                            //tcp数据帧结构体类型

/*****************************************
*ec20tcp外部常变量及申明
****************************************/
#define TCP_LOADBUF_MAXLEN    512                 //TCP帧数据负载的最大长度
//#define CHANNAL0_MAX_NUM      2                 //TCP通道0所连接的TCP服务器共计提供CHANNAL0_MAX_NUM个连接点，硬件扫描连接上任意一个即可
extern ChannalP_s sChannal0 ;                    //TCP通道0的参数实例化

/*****************************************
*驱动可供外部使用的常变量
****************************************/
//extern FrameQueue_s      sTcp0Queue ;             //需要TCP connect0发送的数据在此环形缓冲区，在OS中有个TCP发送的任务，一直在发送TCP数据，需要发送数据只需要将数据丢到此缓冲区即可
	
/*****************************************
*对外接口函数声明
****************************************/
//extern void 		 SetAppTcpIP(ChannalP_s *psChannal, char* ip) ;              //设置登TCP服务器的IP
//extern void 		 SetAppTcpPort(ChannalP_s *psChannal, char* port) ;          //设置登TCP服务器的PORT
extern RunResult AppTcpInit(void) ;               												   //初始化应用TCP链路的PDP和打开TCP0的通道SOCKET服务  
//extern void      TcpDisconnetc(void) ;                                       //断开Socket 去激活TCP链路的PDP
extern RunResult TcpWritedata( UPDATATYPE_e updataType, char *format, ...) ; //通过TCP将可变参数数据上传到TCP SERVER


#endif
