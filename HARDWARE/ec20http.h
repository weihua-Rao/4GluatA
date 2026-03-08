#ifndef _EC20HTTP_H
#define _EC20HTTP_H
//#include "main.h"
#include "ec20net.h"
/*****************************************
*自定义变量类型
****************************************/
typedef struct    
{ 
	char       *postBuf   ;     //HTTP请求 Header+Body格式，其中Body用一个%s限定格式，下面body字段给出具体格式
	char       *host      ;     //HTTP请求的主机
	char       *httpPort  ;     //HTTP请求的接口名
	char       *body      ;     //HTTP请求body格式
}POSTP_s ;                    //存放POST请求相关参数的结构体变量类型

//typedef struct    
//{
//	u8 Topic_len;
//	u8 Mesg_Len;	
//	char       *Topic   ;     // 
//	char       *Message      ;     // 
//}MQTT_Type ; 

/********************************************************
ec20模块TCP/IP相关AT指令处理
*********************************************************/
enum eHttpCmdNum
{
   AT_SAPBR =0, AT_SAPBR_APN =1, AT_SAPBR_CID1 =2,AT_CNTPCID =3, AT_CNTP =4, AT_CCLK =5, 
   MCONFIG =6, MIPSTART =7, MCONNECT =8,MSUB =9, MPUB =10 ,MQTTMSGSET = 11,MDISCONNECT=12,
	MIPCLOSE = 13,MQTTSTART =14 ,CGSN=15,CIPGSMLOC=16,MQTTSTATU=17
}  ;     //枚举ec20模块http相关指令

enum MQTT_TOPIC
{
	alarm1=0,alarm2,alarm3,alarm4,alarm5,alarm6,alarm7,alarm8,alarmpower,alarmTepHum,
	lbs=10,lbsReq,csq,uphome,up_sms,up_call,up_close_massage,upopen_massage,
	SetTemperHum=18,
/*******************/
	down_sms=19,down_call,down_close_massage,down_open_massage,down_alarm,
	down_erase=24,down_get,down_query,down_TemperHum=27
}  ;     //枚举
extern char Clientid_IMEI[16];
extern char*MQTT_TOPIC[];
/*****************************************
*内部函数声明
****************************************/
RunResult EC20_SendHttpCmd( uint8_t cmdNum, char *format,... ) ;  // EC20通过串口发送HTTP相关命令
RunResult Http_Config(void) ;                                     //HTTP相关基本配置
//RunResult Http_PDP_Init( void ) ;                                 //EC20 HTTP context的初始化，只需要调用一次即可                     
RunResult Set_HttpURL(char *host) ;                               //设置HTTP请求的URL
void      HttpErrorCode( int errCode ) ;                          //解析HTTP返回的错误码

/*****************************************
*对外接口函数声明
*注意：所有共外部调用的接口入口参数全部统一为ChannalP_s *channal
****************************************/ 
//extern RunResult Http_Init( void ) ;                              //HTTP初始化
//extern RunResult Send_Post( POSTP_s *psHttpP, char* postBody ) ;  //http发送post请求
//extern RunResult Http_Read( void ) ;                              //读取HTTP返回的数据报文
extern RunResult SYNC_NetTime( void );
RunResult mqttiniti(void);
RunResult mqttsub_Push(void);
//RunResult Mqtt_Mpub(char *Topic, u8 pos, char *messeage);
RunResult SAPBR_Init( void );
RunResult SYNC_LBS_Time( void );
RunResult Mqtt_MpubAlarm(void);
void Mqtt_check(char *mqtt_data);
u8 MQTT_GetPurtNum (char *mqtt_data,char* compare);
RunResult Mqtt_sms_call_phone(char PortNum, u8 sms_call);
RunResult Mqtt_PortMessage(char PortNum, u8 close_open);
RunResult  MQTT_DeviceState(u8 cmd);
void MQTT_AlarmTemHum(void);
RunResult AlarmTemHum_Trigger(void);
RunResult MQTT_AlarmT_Hm(void);
RunResult Get_SN(void);
void MQTT_cqs_temper(void);
void Mqtt_lbs(void);
#endif
