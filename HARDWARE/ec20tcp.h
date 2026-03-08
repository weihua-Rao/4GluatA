#ifndef _EC20TCP_H
#define _EC20TCP_H
#include "ec20net.h"
#include "usercmd.h"

#define  TCP_CONTEXTID          1          //1~16 本驱动强制规定，TCP/IP协议只用一个链路1D（即contextID=1）用于TCP链路
/*****************************************
*自定义变量类型
****************************************/
typedef enum           //The URC of TCP/IP AT commands will be reported to the host in the format of beginning with “+QIURC:”
{
  CLOSED = 0 ,         //Socket service connection is closed
  RECV = 1 ,           //receiving data, the module will report a URC to the host
  INCOMING_FULL = 2 ,  //incoming connection reaches the limit
  INCOMING_CONT = 3 ,  //he <service_type> is “TCP LISTENER”, when a remote client connects to this server
  PDPDEACT = 4,        //PDP context may be deactivated by the network	
	UNKNOWM = 5          //未知的URC
} TcpUrcType ;         //《Quectel_EC20_R2.1_AT_Commands_Manual_V1.0》 P31

//typedef struct                              //contextID:链路ID  connetcID：通道ID
//{ 
//	uint8_t 	    connectId       ;           //TCP链路通道ID
//	char       serverIP[MAX_IP_LEN] ;      //TCP链路通道	的Server IP
//	char      MqttUser[MAX_IP_LEN] ;                //TCP链路通道	的Server PORT
//	char      MqttPass[MAX_IP_LEN] ;                 //TCP链路通道	的Local  PORT	
//}ChannalP_s ;                               //存放TCP通道参数结构体变量类型
typedef struct                              //contextID:链路ID  connetcID：通道ID
{ 
	uint8_t 	    connectId       ;           //TCP链路通道ID
	uint8_t       serverIP[MAX_IP_LEN] ;      //TCP链路通道	的Server IP
	char*      serverPort ;                //TCP链路通道	的Server PORT
	char*      localPort ;                 //TCP链路通道	的Local  PORT	
	char*					Mqtt_User;
	char*					Mqtt_Password;
}ChannalP_s ; 
/*****************************************
*内部函数声明
****************************************/
RunResult   EC20_SendTcpCmd( uint8_t cmdNum, char *format,... ) ;             // EC20通过串口发送TCP相关命令
RunResult   Open_Socket(uint8_t connectId, uint8_t *serverIp, uint16_t serverPortNum,  uint16_t localPortNum )  ; // 打开一个tcp通道
RunResult   Close_Socket(uint8_t connectId) ;                                 // 关闭一个tcp通道
RunResult   Query_Socket(uint8_t connectId) ;                                 // 查询一个tcp通道状态
	
/*****************************************
*对外接口函数声明
*注意：所有共外部调用的接口入口参数全部统一为ChannalP_s *channal
****************************************/
extern RunResult Tcp_PDP_Init( void ) ;                                                   //EC20 TCP context的初始化，只需要调用一次即可
extern RunResult Tcp_Channal_Init( ChannalP_s *channal ) ;                                //TCP通道初始化
extern RunResult Tcp_SendData(ChannalP_s *channal, uint8_t *sendBuf, uint16_t sendLen) ;  //通过TCP通道发送TCP上行数据
extern TcpUrcType TcpUrcHandle( char *recvBuf, uint16_t recvLen ) ;                       //TCP/IP协议中模块返回的一写URC分类处理

#endif
