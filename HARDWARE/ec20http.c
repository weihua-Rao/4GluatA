//#include "main.h"  //#define STR2(R)  STR1(R)
#include "ec20http.h" 
#include <stdlib.h>
#include "syslib.h"  //#define STR2(R)  STR1(R)
#include "PhoneCode.h"
/********************************************************************************
  * @file    ec20http.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   提供Quectel模块EC20关于HTTP驱动程序
  ******************************************************************************
  * @attention
  *
  * 约定基本名词如下：
	*          contextID:链路ID  connetcID：通道ID   channal：连接通道
  * EC20模块链路ID范围1~16，每一个链路ID都会对应一个本地IP； 通道ID范围0~11。
  * 每一个链路ID可以有12个通道ID。
  * 本驱动强制规定，HTTP协议只用一个链路1D（即contextID=2）用于HTTP链路。
	* @use：
	*  先调用Http_Init()初始化HTTP链路等参数，接着调用Send_Post发送POST请求，最后通过Http_Read读取接收到的POST数据
*******************************************************************************/

/*****************************************
*内部常量
****************************************/
#define  HTTP_CMDPACK_LEN        300      //EC20 HTTP相关命令字符串的最大长度
#define  HTTP_CONTEXTID          2        //1~16 本驱动强制规定，HTTP协议只用一个链路1D（即contextID=2）用于HTTP链路

char Clientid_IMEI[16] = {0} ;           //存放模块SN号码 Clientid_IMEI
//static 
extern volatile EC20_CMD_DATA_s sModuleCmd[30];
//extern volatile EC20_CMD_DATA_s sNetCmd[7];
//eHttpCmdNumtype eHttpCmdNum;
char *phone_num[8]={"phone1=%s&","phone2=%s&","phone3=%s&","phone4=%s&","phone5=%s&","phone6=%s&","phone7=%s&","phone8=%s"};
char *MQTT_TOPIC[28]=
	{
	"/alarmSing",
	"/alarm2Sing",
	"/alarm3Sing",
	"/alarm4Sing",
	"/alarm5Sing",
	"/alarm6Sing",
	"/alarm7Sing",
	"/alarm8Sing",
	"/alarmpower",
	"/upload/alarmTepHum",	
	"/lbsLocation",
	"/lbsLocationReq",
	"/csq",
	"/upload/home",
	"/upload/sms_phone",
	"/upload/call_phone",
	"/upload/close_massage",
	"/upload/open_massage",
	"/upload/SetTemperHum",	

/*******************/
"/download/sms_phone",
"/download/call_phone",
"/download/close_massage",
"/download/open_massage",
"/download/ALARM",
"/download/ERASE",
"/download/GET",
"/download/QUERY",
"/download/SetTemperHum",
};

/********************************************************
ec20模块TCP/IP相关AT指令处理
*********************************************************/
//enum eHttpCmdNum
//{
//   AT_SAPBR =0, AT_SAPBR_APN =1, AT_SAPBR_CID1 =2,AT_CNTPCID =3, AT_CNTP =4, AT_CCLK =5, 
//   MCONFIG =6, MIPSTART =7, MCONNECT =8,MSUB =9, MPUB =10 ,MDISCONNECT=11,
//	MIPCLOSE = 12, MQTTSTART =13 ,CGSN=14,
//}  ;     //枚举ec20模块http相关指令
//同步网络时间
volatile EC20_CMD_DATA_s sHttpCmd[18]=
{
//  cmdNum            cmdStr,                        timeout(100ms), trueStr, trueOffset, falseStr revResult rtyNum
 {AT_SAPBR,       "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n", 	5,          "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //激活PDP是使用CNTP命令同步时间的前提
 {AT_SAPBR_APN,   "AT+SAPBR=3,1,\"APN\",\"\"\r\n", 					5,          "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //设置PDP承载之APN参数
 {AT_SAPBR_CID1,  "AT+SAPBR=1,1\r\n", 											5,          "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //激活<cid>=1的PDP
 {AT_CNTPCID,     "AT+CNTPCID=1\r\n", 											5,          "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //设置使用的PDP的<cid>=1
 {AT_CNTP,   			"AT+CNTP\r\n", 														10,        "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //同步网络时间
 {AT_CCLK,    		"AT+CCLK?\r\n", 													5,         "+CCLK:",   -1,      "ERROR",  TIMEOUT,   2       }, //查询时间
//初始化MQTT 激活PDP上下文
 {MCONFIG,      	"AT+MCONFIG=\"%s\",\"%s\",\"%s\"\r\n",		10,          "OK",      -1,      "ERROR",  TIMEOUT,   1       }, //<clientid>、 用户名和密码
 {MIPSTART,     	"AT+MIPSTART=\"%s\",\"%s\"\r\n",					5,          "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //IP地址或域名地址， 以及端口号
 {MCONNECT,   		"AT+MCONNECT=1,60\r\n", 									5,          "CONNACK OK",-1,    "ERROR",  TIMEOUT,   2       }, //建立mqtt会话
 {MSUB,   				"AT+MSUB=\"%s\",0\r\n", 									5,          "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //订阅
 {MPUB,   				"AT+MPUB=\"%s\",%d,0,\"%s\"\r\n", 				5,          "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //发布， 消息格式缺省为ASCII格式
 {MQTTMSGSET, 		"AT+MQTTMSGSET=0\r\n", 										5,          "OK",      -1,      "ERROR",  TIMEOUT,   2       },//0设置为直接上报消息 1 cache方式 需要用+MQTTMSGGET来读
 {MDISCONNECT,   	"AT+MDISCONNECT\r\n", 										5,          "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //关闭MQTT连接
 {MIPCLOSE,   		"AT+MIPCLOSE\r\n", 												5,          "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //关闭TCP链接
 {MQTTSTART,   		"AT+MQTTSTART\r\n", 											5,          "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //启动MQTT服务 激活PDP上下文
 {CGSN,      			"AT+CGSN\r\n", 														5,          "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //查询 IMEI 号

 {CIPGSMLOC, 			"AT+CIPGSMLOC=1,1\r\n", 									100,          "OK",     -1,      "ERROR",  TIMEOUT,   2       }, //读取基站定位LBS信息和时间
 {MQTTSTATU,      "AT+MQTTSTATU\r\n", 											5,    "+MQTTSTATU :1",  -1, "+MQTTSTATU :0", TIMEOUT,   2       }, //查询MQTT连接
 
// {CMQTTCONNECT,  "AT+CMQTTCONNECT=0,\"%s\",60,1,\"%s\",\"%s\"\r\n",5, "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //连接到MQTT服务器，激活PDP上下文
// {CMQTTACCQ,  	 "AT+CMQTTACCQ=0,\"%s\"\r\n",							5, 					"OK",     -1,      "ERROR",  TIMEOUT,   2       }, //获取一个将连接到MQTT服务器而不是SSL/TLS的客户机
// {CMQTTSUB,  	 	 "AT+CMQTTSUB=0\r\n",							5, 					"OK",     -1,      "ERROR",  TIMEOUT,   2       }, //

 //sim900a_send_cmd("AT+CMQTTCONNECT=0,\"tcp://www.szjcd.top:9001\",60,1,\"szjcd\",\"123456\"\r\n","OK",100);	* 
  /*AT+CMQTTSUB=0\r\n"
 +CCLK: "18/05/16,15:49:28+32"
 */
} ;  //EC20模块HTTP相关指令的EC20_CMD_DATA_s结构体类型参数

/**************************************************************************************************
* 名    称：  static const char *HttpCmdNumToString(enum eHttpCmdNum result)
* 功能说明：  输出枚举成员名的字符串指针。
* 入口参数：  eHttpCmdNum类型的枚举
* 出口参数：  为枚举的成员名字符串指针 
**************************************************************************************************/
static inline const char *HttpCmdNumToString(enum eHttpCmdNum result)
{   
    switch (result)
			{
				 ENUM_CHIP_TYPE_CASE(AT_SAPBR)
				 ENUM_CHIP_TYPE_CASE(AT_SAPBR_APN)
				 ENUM_CHIP_TYPE_CASE(AT_SAPBR_CID1)
				 ENUM_CHIP_TYPE_CASE(AT_CNTPCID)
				 ENUM_CHIP_TYPE_CASE(AT_CNTP)
				 ENUM_CHIP_TYPE_CASE(AT_CCLK)
				
				 ENUM_CHIP_TYPE_CASE(MCONFIG)
				 ENUM_CHIP_TYPE_CASE(MIPSTART)
				 ENUM_CHIP_TYPE_CASE(MCONNECT)
				 ENUM_CHIP_TYPE_CASE(MSUB)
				 ENUM_CHIP_TYPE_CASE(MPUB)
				 ENUM_CHIP_TYPE_CASE(MQTTMSGSET)
				 ENUM_CHIP_TYPE_CASE(MDISCONNECT)
				 ENUM_CHIP_TYPE_CASE(MIPCLOSE)
				 ENUM_CHIP_TYPE_CASE(MQTTSTART)
				 ENUM_CHIP_TYPE_CASE(CGSN)
				 ENUM_CHIP_TYPE_CASE(CIPGSMLOC)
				 ENUM_CHIP_TYPE_CASE(MQTTSTATU)
			}
    return "无此命令";
}

/**************************************************************************************************
* 名    称：  RunResult EC20_SendHttpCmd( uint8_t cmdNum, char *format,... ) 
* 功能说明：  MCU串口向EC20发送http相关命令
* 入口参数：   
*            @param1 cmdNum  EC20_CMD_DATA_s中cmdNum成员命令编号
*            @param2 char *format,...  可变参变量
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
RunResult EC20_SendHttpCmd( uint8_t cmdNum, char *format,... ) 
{
    uint8_t revTimes = 0 ;
	  RunResult  status = TIMEOUT ;
		uint8_t retryTimes = sHttpCmd[cmdNum].rtyNum ; 
    char *cmdPack = NULL ;
		format = sHttpCmd[cmdNum].cmdStr ;	
	  cmdPack = portMalloc(HTTP_CMDPACK_LEN*sizeof(uint8_t)) ;		
    va_list ap;  
    va_start (ap, format); 
    int outLen = vsnprintf(cmdPack, HTTP_CMDPACK_LEN, (const char*)format, ap) ;			
    if((outLen<=0)||( outLen > HTTP_CMDPACK_LEN)) 	//vsprintf (temp, cmd, ap);  			//到此为止，所有的参数情况已经汇总到temp了 
		  {
			  ErrorLogPrintf("Http cmdPack 溢出！--增加HTTP_CMDPACK_LEN数值。") ;
				goto httpCmdOut ;
			}
		while(retryTimes--)
			{
			  Ec20AtBufReset() ;
				revTimes = 0 ;
				//UARTx_Printf(EC20_UART, (uint8_t *)"%s", (uint8_t *)cmdPack);		
				UARTx_SendData(EC20_UART, cmdPack, outLen)	 ;	 //DMA发送					
				while( revTimes++ < (sHttpCmd[cmdNum].timeout*2) )
					{ 
						Delay_Ms_StopScheduler(50);//	Wait_For_Nms(50);
            sHttpCmd[cmdNum].trueOffset = kmp(ec20AtBuf, sHttpCmd[cmdNum].trueStr) ;					
						if( sHttpCmd[cmdNum].trueOffset >= 0)
							{   
									status = RUNOK ; 
								  goto httpCmdOut ;
							}
						else if( kmp(ec20AtBuf, sHttpCmd[cmdNum].falseStr) >= 0)
							{
									status = RUNERR ;	
								  goto httpCmdOut ;
							}
					}
				Delay_Ms_StopScheduler(1000);//	Wait_For_Nms( 1000 ) ;		
			}
httpCmdOut:portFree(cmdPack) ;			
		va_end (ap);
		DebugLogPrintf("%s %s",  HttpCmdNumToString((enum eHttpCmdNum)cmdNum), RunResultToString(status) ) ;
    return (status) ;		
}
/**************************************************************************************************
* 名    称：  SAPBR_Init(void) 
* 功能说明：  SAPBR 初始化
**************************************************************************************************/
RunResult SAPBR_Init( void )
{
 RunResult runResult = TIMEOUT ;
	
	runResult = EC20_SendHttpCmd(AT_SAPBR, NULL ) ; //设置承载类型为GPRS
	runResult = EC20_SendHttpCmd(AT_SAPBR_APN, NULL ) ; //设置PDP承载之APN参数
	runResult = EC20_SendHttpCmd(AT_SAPBR_CID1, NULL ) ; //激活<cid>=1的PDP
	runResult = EC20_SendHttpCmd(AT_CNTPCID, NULL ) ; 
	runResult = EC20_SendHttpCmd(AT_CNTP, NULL ) ; //同步网络时间
	
 return(runResult) ;
}
///**************************************************************************************************
//* 名    称：  RunResult SYNC_NetTime(void) 
//* 功能说明：  同步基站定位和时间
//**************************************************************************************************/
////+CIPGSMLOC: 0,22.731085,114.035042,2024/09/26,10:32:16  内地
////+NITZ: 24/09/26,10:21:29+32,0    香港

//RunResult SYNC_LBS_Time( void )
//{
// RunResult runResult = TIMEOUT ;
// char *buf,*haystack;		
//	
//	runResult = EC20_SendHttpCmd(CIPGSMLOC, NULL ) ; //获取基站定位和时间
//	
//	haystack= ec20AtBuf;	
//	buf= strstr( haystack, "+CIPGSMLOC:");					//
//	haystack = buf + 12;//地址后移12位 strlen("+CIPGSMLOC:") 0X0D
//	if('0' != *haystack)return RUNERR;//判断是否读取成功 0
//	haystack += 2;		//后移2位 0,
//	buf= strstr( haystack, ",");
//	buf[0]='\0';
//	strncpy(LBS.lon,haystack,20);	//存储经度
//	haystack = buf + 1;//后移一位 ,

//	buf= strstr( haystack, ",");
//	buf[0]='\0';
//	strncpy(LBS.lat,haystack,20);	//存储纬度
//	haystack = buf+3;		//后移3位,24

//	CopyValues((uint8_t*)&uCalendar.bytes[2], (uint8_t*)haystack, 0x0d, 18) ;// 存储 年 月 日 时 分 秒
//	RTC_DateRegulate();
//	RTC_TimeRegulate();

// return(runResult) ;
//}
/**************************************************************************************************
* 名    称：  RunResult SYNC_NetTime(void) 
* 功能说明：  同步基站定位和时间
**************************************************************************************************/
//+CIPGSMLOC: 0,22.731085,114.035042,2024/09/26,10:32:16  内地
//+NITZ: 24/09/26,10:21:29+32,0    香港

//RunResult SYNC_LBS_Time( void )
//{
// RunResult runResult = TIMEOUT ;
// char *buf,*haystack;		
//	
//	runResult = EC20_SendHttpCmd(CIPGSMLOC, NULL ) ; //获取基站定位和时间
//	
//	haystack= ec20AtBuf;	
//	if(hand(ec20AtBuf,"+CIPGSMLOC:"))
//	{
//	buf= strstr( haystack, "+CIPGSMLOC:");					//
//	haystack = buf + 12;//地址后移12位 strlen("+CIPGSMLOC:") 0X0D
//	if('0' != *haystack)return RUNERR;//判断是否读取成功 0
//	haystack += 2;		//后移2位 0,
//	buf= strstr( haystack, ",");
//	buf[0]='\0';
//	strncpy(LBS.lon,haystack,20);	//存储经度
//	haystack = buf + 1;//后移一位 ,

//	buf= strstr( haystack, ",");
//	buf[0]='\0';
//	strncpy(LBS.lat,haystack,20);	//存储纬度	
//	}
//	haystack = buf + 1;//后移一位 ,
//	buf= strstr( haystack, "/");
//	haystack = buf-2;		//左移2位,24
////	else
////	haystack = buf+3;		//后移3位,24

//	CopyValues((uint8_t*)&uCalendar.bytes[2], (uint8_t*)haystack, 0x0d, 18) ;// 存储 年 月 日 时 分 秒
//	RTC_DateRegulate();
//	RTC_TimeRegulate();

// return(runResult) ;
//}
RunResult SYNC_LBS_Time( void )
{
	//港澳台 只能同步网络时间时间 不能用基站定位 没数据且有时没有数据下发
#if (Region_Code == 0)	//0 内地	
 RunResult runResult = TIMEOUT ;
 char *buf,*haystack;		
//	char tt[]={"+NITZ: 24/09/26,07:39:54+32,0"};	
	runResult = EC20_SendHttpCmd(CIPGSMLOC, NULL ) ; //获取基站定位和时间
	
  char Resul = 1;u8 delayWait = 50;
//	while(Resul){	Resul &= !hand(ec20AtBuf,"+CIPGSMLOC:");	Resul &= !hand(ec20AtBuf,"+NITZ:");
		while(Resul){	Resul &= !hand(ec20AtBuf,"+CIPGSMLOC:");
								Delay_Ms_StopScheduler(1);
								if(--delayWait == 0)Resul=0;
							}
	if(Resul)return TIMEOUT;			 //0失败
	
	haystack= ec20AtBuf;//ec20AtBuf;	
	if(hand(ec20AtBuf,"+CIPGSMLOC:"))
	{
	buf= strstr( haystack, "+CIPGSMLOC:");					//
	haystack = buf + 12;//地址后移12位 strlen("+CIPGSMLOC:") 0X0D
	if('0' != *haystack)return RUNERR;//判断是否读取成功 0
	haystack += 2;		//后移2位 0,
	buf= strstr( haystack, ",");
	buf[0]='\0';
	strncpy(LBS.lon,haystack,20);	//存储经度
	haystack = buf + 1;//后移一位 ,

	buf= strstr( haystack, ",");
	buf[0]='\0';
	strncpy(LBS.lat,haystack,20);	//存储纬度	

	haystack = buf+3;		//后移3位,24
	}
	
	else if(hand(ec20AtBuf,"+NITZ:"))			//仅获取时间
	{
	buf= strstr( haystack, "+NITZ:");	
			
	haystack = buf + 7;//地址后移7位
	}
	else return (runResult) ;
//CopyValues((uint8_t*)&uCalendar.bytes[2], (uint8_t*)haystack, 0x0d, 18) ;// 存储 年 月 日 时 分 秒	
	CopyValues(&uCalendar.bytes[2], (uint8_t*)haystack, 0x0d, 18) ;// 存储 年 月 日 时 分 秒
	RTC_DateRegulate();
	RTC_TimeRegulate();
#elif (Region_Code == 1)	// 1港澳台	//港澳台 只能同步网络时间时间 不能用基站定位 没数据且有时没有数据下发
	SYNC_NetTime();
#endif	
 return RUNOK ;
}
/**************************************************************************************************
* 名    称：  RunResult SYNC_NetTime(void) 
* 功能说明：  同步网络时间
**************************************************************************************************/
RunResult SYNC_NetTime( void )
{
	RunResult runResult = TIMEOUT ;
	
//		runResult = EC20_SendHttpCmd(AT_SAPBR, NULL ) ; 
//		runResult = EC20_SendHttpCmd(AT_SAPBR_APN, NULL ) ; 
//		runResult = EC20_SendHttpCmd(AT_SAPBR_CID1, NULL ) ; //激活<cid>=1的PDP
//		runResult = EC20_SendHttpCmd(AT_CNTPCID, NULL ) ; 
//		runResult = EC20_SendHttpCmd(AT_CNTP, NULL ) ; //同步网络时间
		runResult = EC20_SendHttpCmd(AT_CCLK, NULL ) ; //获取更新后的时间	

		if( RUNOK == runResult )
			{
//				CopyValues((uint8_t*)&uCalendar.bytes[2], (uint8_t*)&ec20AtBuf[sHttpCmd[AT_CCLK].trueOffset+8], '+', 20) ;// 5+5
					CopyValues(&uCalendar.bytes[2], (uint8_t*)&ec20AtBuf[sHttpCmd[AT_CCLK].trueOffset+8], '+', 20) ;// 5+5
				   RTC_DateRegulate();
           RTC_TimeRegulate();
			}
 return(runResult) ;
}
/**************************************************************************************************
* 名    称：  RunResult Http_Config(void) 
* 功能说明：  http相关的基本配置，包括请求头、超时时间、请求的数据类型等
**************************************************************************************************/
//RunResult Http_Config(void)
//{
//    RunResult runStatus = TIMEOUT ;
//		runStatus = EC20_SendHttpCmd(HTTPCONTEST, NULL ) ; 
//		if( RUNOK != runStatus )
//		  {
//			  return (runStatus) ;
//			}
//			
//	  runStatus = EC20_SendHttpCmd(ENREQHEADER, NULL ) ;
//		if( RUNOK != runStatus )
//		  {
//			  return (runStatus) ;
//			}	
////		if( (kmp(sEc20Param.ec20SoftVer , "EC20CEFDKGR06A03M2G") < 0)	) //EC20该版本需要增加下面指令
////			{
////				runStatus = EC20_SendHttpCmd(CLOSETIME, NULL ) ; //ec20SoftVer
////				if( RUNOK != runStatus )
////					{
////						return (runStatus) ;
////					}
////			}	
////	  runStatus = EC20_SendHttpCmd(CLOSETIME, NULL ) ; //ec20SoftVer
////		if( RUNOK != runStatus )
////		  {
////			  return (runStatus) ;
////			}	
//			
//		runStatus = EC20_SendHttpCmd(BODYTYPE, NULL ) ;		
//		return (runStatus) ;
//}

///**************************************************************************************************
//* 名    称：  RunResult Http_PDP_Init(void) 
//* 功能说明：  http协议的链路ID contextid配置，强制设置为HTTP_CONTEXTID
//**************************************************************************************************/
//RunResult Http_PDP_Init( void )
//{
//	  RunResult runResult = TIMEOUT ; 
//	  uint8_t *httpLocalIp = portMalloc(MAX_IP_LEN) ;
//		runResult = Query_Context( HTTP_CONTEXTID, httpLocalIp ) ;  //查询HTTP_CONTEXTID是否激活
//	  if( RUNOK == runResult ) /*HTTP_CONTEXTID已激活*/  //去激活->再次激活
//		  {
////			  runResult = Deact_Context(HTTP_CONTEXTID) ;
////				if( RUNOK != runResult ) /*HTTP_CONTEXTID去激活失败*/ //直接返回错误
////				  {
////					  return RUNERR ; 
////					}
//				return RUNOK ;
//			}
//	  runResult = ActivePDP(HTTP_CONTEXTID, httpLocalIp) ;	
//    if( RUNOK == runResult )	
//		  {			
//		    AppLogPrintf("HTTP本地IP：%s", httpLocalIp) ;
//			}
//	  portFree(httpLocalIp) ;
//	  return(runResult) ;
//}

///**************************************************************************************************
//* 名    称：  RunResult Set_HttpURL(char *host)
//* 功能说明：  设置http请求的主机地址
//* 入口参数：   
//*            @param1 *host  主机地址
//**************************************************************************************************/
//RunResult Set_HttpURL(char *host)
//{
//    RunResult runStatus = TIMEOUT ;
//    int hostLen = strlen(host) ;
//	  char *url = portMalloc(10+hostLen) ;
//	  snprintf(url, 10+hostLen, "%s%s", "http://", host) ;
//		runStatus = EC20_SendHttpCmd(SETURLCMD, NULL, strlen((const char*)url) ) ;	  
//		if( RUNOK == runStatus )
//		  {
//			  runStatus = EC20_SendHttpCmd(HTTPURL, NULL, url ) ;
//				if( RUNOK == runStatus )
//					{
//						AppLogPrintf("POST URL:%s.", url) ;
//					}
//			}
//    portFree(url) ;			
//		return (runStatus) ;
//}

///**************************************************************************************************
//* 名    称：  RunResult Http_PDP_Init(void) 
//* 功能说明：  http初始化，包括基本参数、和链路初始化
//**************************************************************************************************/
//RunResult Http_Init( void )
//{
//	 RunResult runStatus = TIMEOUT ;
//   if( RUNOK != Http_Config() )
//		   return (runStatus) ; 
//	 runStatus = Http_PDP_Init() ;
//	 return (runStatus) ;
//}

///**************************************************************************************************
//* 名    称：  RunResult Send_Post( POSTP_s *psHttpP, char* postBody )
//* 功能说明：  http发送post请求
//* 入口参数：   
//*            @param1 *psHttpP  POSTP_s类型数据指针，包含http请求的参数
//*            @param2 *postBody post请求的负载数据
//* 出口参数：   
//*            @param1 status  RunResult枚举类型变量，返回函数运行结果
//*                            如果返回RUNOK说明已经请求成功，接下来就可以调用Http_Read（）函数读取存储在缓冲区内的数据了
//**************************************************************************************************/
//RunResult Send_Post( POSTP_s *psHttpP, char* postBody )
//{
//	  int outLen = 0 ;
//		RunResult runStatus = TIMEOUT ;
//		Set_HttpURL(psHttpP->host) ;        //设置HTTP请求URL
//	  char *headerBodyBuf = portMalloc(HTTP_CMDPACK_LEN) ;
//		outLen = snprintf(headerBodyBuf, HTTP_CMDPACK_LEN, psHttpP->postBuf, psHttpP->host, psHttpP->httpPort, psHttpP->host,strlen(postBody), postBody ) ;
//		if((outLen<=0)||(outLen>HTTP_CMDPACK_LEN))
//		{
//				runStatus = RUNERR ;
//				ErrorLogPrintf("headerBodyBuf 溢出！--增加HTTP_CMDPACK_LEN数值。") ;
//				goto PostOut ;
//		}
//			
//		runStatus = EC20_SendHttpCmd(POSTREQCMD, NULL, outLen ) ; //发送POST请求命令
//		if( RUNOK != runStatus )
//			{ 
//				goto PostOut ;
//			}
//				
//		runStatus = EC20_SendHttpCmd(POSTREQBUF, NULL, headerBodyBuf ) ;//发送POST请求数据 //DebugLogPrintf("%s",headerBodyBuf) ;
//PostOut:
//		portFree(headerBodyBuf) ;
//		return (runStatus) ;
//}

///**************************************************************************************************
//* 名    称：  RunResult Http_Read( void )
//* 功能说明：  读取http返回的数据, 将读取到的http数据存于ec20HttpBuf中
//**************************************************************************************************/
//RunResult Http_Read( void )
//{
//	  uint8_t revTimes = 0 ;
//	  int errCode  = 0 ; 
//		RunResult runStatus = TIMEOUT ;
//	  Ec20HttpBufReset() ;                            //ec20HttpBuf清空 准备接下来接收数据
//	  httpDataMode = true ;                           //将EC20_UART串口接收模式改为：接收http数据模式
//		UARTx_SendData(EC20_UART, sHttpCmd[HTTPREAD].cmdStr, strlen((const char*)sHttpCmd[HTTPREAD].cmdStr) ) ;	 //DMA发送					
//		while( revTimes++ < (sHttpCmd[HTTPREAD].timeout) )
//			{ 
//				Wait_For_Nms(100) ;                         //等待100ms
//				sHttpCmd[HTTPREAD].trueOffset = kmp(ec20HttpBuf, sHttpCmd[HTTPREAD].trueStr) ;  //在ec20HttpBuf缓冲区中查找	sHttpCmd[HTTPREAD].trueStr字串				
//				if( sHttpCmd[HTTPREAD].trueOffset >= 0)
//					{   
//						  if( ec20HttpBuf[sHttpCmd[HTTPREAD].trueOffset+12] == 0x30 )  /*"AT+QHTTPREAD"指令返回“+QHTTPREAD: 0” 表示请求成功*/
//							  {
//									errCode = 0 ;
//									runStatus = RUNOK ;
//								}
//							else                                                        /*"AT+QHTTPREAD"指令返回“+QHTTPREAD: <err>” 表示请求出错*/
//							  {
//								  errCode = (ec20HttpBuf[sHttpCmd[HTTPREAD].trueOffset+1]-0x30)*100+(ec20HttpBuf[sHttpCmd[HTTPREAD].trueOffset+2]-0x30)*10+
//									          (ec20HttpBuf[sHttpCmd[HTTPREAD].trueOffset+3]) ;
//									runStatus = RUNERR ;									
//								}
//              HttpErrorCode(errCode) ;
//							break ;
//					}
//				else if( kmp(ec20AtBuf, sHttpCmd[HTTPREAD].falseStr) >= 0)
//					{
//							runStatus = RUNERR ;	
//							break ;
//					}
//			}
//	  httpDataMode = false ;                          //关闭 接收http数据模式
//		DebugLogPrintf("%s %s",  HttpCmdNumToString(HTTPREAD), RunResultToString(runStatus) ) ;		 
//		return (runStatus) ;
//}

/**************************************************************************************************
* 名    称：  void HttpErrorCode( int errCode )
* 功能说明：  解析HTTP返回的错误码,请参照《Quectel_EC2x&EG9x&EM05_HTTP(S)_AT_Commands_Manual_V1.0.pdf》手册P35
* 入口参数：   
*            @param1 errCode  错误码
**************************************************************************************************/
//void HttpErrorCode( int errCode )
//{
//  switch( errCode)
//	 {
//		case 0:
//			AppLogPrintf("Http Operation successful.") ;
//		  break ;
//		case 701:
//			AppLogPrintf("HTTP(S) unknown error.") ;
//		  break ;
//		case 702:
//			AppLogPrintf("HTTP(S) timeout.") ;
//		  break ;
//		default:
//			AppLogPrintf("HTTP(S) other error.") ;
//		  break ;	   
//	 }
//}

//获取SN系列号函数
//void SNread(void)
//{	char SnNum;
//	char *buf,*haystack;
///*
//	char xdata ttt[]={0x41,0x54,0x2B,0x47,0x53,0x4E,0x0D,0x0D,0x0A,0x38,0x36,0x39,\
//	0x34,0x36,0x37,0x30,0x34,0x30,0x39,0x34,0x37,0x39,0x39,0x37,0x0D,0x0A,0x0D,0x0A,0x4F,0x4B,0x0D,0x0A};//+CSQ: 24,0
//*/
//	haystack= uart1_rx_buf;
////	haystack= ttt;	
//	buf= strstr( haystack, "\r\n");					//
//	haystack = buf + strlen("\r\n");
//	buf[0]='\0';
////	strncpy(Clientid_IMEI,uart1_rx_buf,14);
//	for(SnNum=0; SnNum<15;SnNum++)
//	{Clientid_IMEI[SnNum] = *haystack++;}
//	Clientid_IMEI[15]=0;
//}
RunResult Get_SN(void)
{
	char *buf,*haystack;	

/***************获取模块唯一IEME序列号*****************/
	EC20_SendHttpCmd(CGSN, NULL );

	haystack = ec20AtBuf;
	buf= strstr( haystack, "\r\n");					//查找头部字符串 "\r\n"
	haystack = buf + strlen("\r\n");
	u8 len = 0;
	for(;(len<10)&&((*haystack <'0')||(*haystack > '9'));len++,haystack++);//过滤掉非 0~9数字 字符，自动查找SN起始位置
	buf= strstr( haystack, "\r\n");					//查找结尾字符串 "\r\n"
	buf[0]='\0';															//结尾清零 
	len = strlen(haystack);								//获取SN长度
/***************END*****************/
//	if((Clientid_IMEI[14]<'0')||(Clientid_IMEI[14]>'9'))
	if(len == 15)	
	{
		memcpy(Clientid_IMEI,haystack,15);
		Clientid_IMEI[15] = 'D';			//产品类别暂定为后缀“D”
		StatusBarPrintf("SN:");
//		OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,"%s", Clientid_IMEI) ;
		OLED_ShowString(0, 2, false, Clientid_IMEI, HIGH_16);	
		AppLogPrintf("序列号获取成功:%s",Clientid_IMEI);
		return RUNOK;
	}
else
	{
		StatusBarPrintf("序列号获取失败");
		ErrorLogPrintf("序列号获取失败");
	}
return RUNERR;	
}
//mqtt初始化
RunResult mqttiniti(void)
{
RunResult Result = TIMEOUT ;

//char xdata mqttclient[35] = {0};
//	char       *mqttclient = NULL ;
//	mqttclient = portMalloc(35*sizeof(uint8_t)) ;
//	memset (mqttclient,0,35);	
//	memset (Clientid_IMEI,0,15);	
//	EC20_SendModuleCmd(AT_CFUN, 0 );
//	EC20_SendModuleCmd(AT_CGREG, NULL );Delay_Ms_StopScheduler(50);
//	EC20_SendModuleCmd(AT_CGATT, NULL );Delay_Ms_StopScheduler(50);
//	EC20_SendModuleCmd(AT_CSQ, NULL );Delay_Ms_StopScheduler(500);
	OledClearLine(LINE1);	
if(RUNOK != Get_SN())return RUNERR ;
/***************获取模块唯一IEME序列号*****************/	
	
//	EC20_SendHttpCmd(CGSN, NULL );
//char *buf,*haystack;	
	
//	haystack = ec20AtBuf;
//	buf= strstr( haystack, "\r\n");					//查找头部字符串 "\r\n"
//	haystack = buf + strlen("\r\n");
//	u8 len = 0;
//	for(;(len<10)&&((*haystack <'0')||(*haystack > '9'));len++,haystack++);//过滤掉非 0~9数字 字符，自动查找SN起始位置
//	buf= strstr( haystack, "\r\n");					//查找结尾字符串 "\r\n"
//	buf[0]='\0';															//结尾清零 
//	len = strlen(haystack);								//获取SN长度

////	if((Clientid_IMEI[14]<'0')||(Clientid_IMEI[14]>'9'))
//	if(len == 15)	
//	{
//		memcpy(Clientid_IMEI,haystack,15);
//		Clientid_IMEI[15] = 'D';			//产品类别暂定为后缀“D”
//		StatusBarPrintf("SN:");
////		OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,"%s", Clientid_IMEI) ;
//		OLED_ShowString(0, 2, false, Clientid_IMEI, HIGH_16);	
//		AppLogPrintf("序列号获取成功:%s",Clientid_IMEI);
//	}
//else
//	{
//		StatusBarPrintf("序列号获取失败");
//		ErrorLogPrintf("序列号获取失败");
//		return RUNERR;
//	}
/***************END*****************/
	
//	EC20_SendHttpCmd(MDISCONNECT, NULL );//模块先关闭MQTT连接
//	EC20_SendHttpCmd(MIPCLOSE, NULL );//关闭TCP链接	
//	UARTx_SendData(EC20_UART, "AT+MDISCONNECT\r\n", strlen("AT+MDISCONNECT\r\n"))	 ;	 //DMA发送 模块先关闭MQTT连接
//	Delay_Ms_StopScheduler(200);//
//	UARTx_SendData(EC20_UART, "AT+MIPCLOSE\r\n", strlen("AT+MIPCLOSE\r\n"))	 ;	 //DMA发送 关闭TCP链接
//	Delay_Ms_StopScheduler(200);//	Wait_For_Nms(1000);	
/*连接到MQTT服务器 <clientid>  用户名  密码 */
	if(RUNERR == EC20_SendHttpCmd(MCONFIG, NULL, Clientid_IMEI, sChannal0.Mqtt_User, sChannal0.Mqtt_Password ))return RUNERR ;
	Delay_Ms_StopScheduler(50);
/*mqtt服务器的IP地址或域名地址， 以及端口号 "www.szjcd.top",9001,"tcp" "szjcd","123456" */
	if(RUNERR == EC20_SendHttpCmd(MIPSTART,NULL, (char*)sChannal0.serverIP,sChannal0.serverPort ))return RUNERR ;
	Delay_Ms_StopScheduler(100);
//链接MQTT服务器
	/*启动MQTT服务，激活PDP上下文*/
	if(RUNERR == EC20_SendHttpCmd(MCONNECT, NULL ))return RUNERR ;
	Delay_Ms_StopScheduler(100);
	Result = EC20_SendHttpCmd(MQTTMSGSET, NULL );//设置为直接上报消息
	Delay_Ms_StopScheduler(100);

	return Result ;
}
//激活PDP上下文
//	EC20_SendModuleCmd(CGSOCKCONT, NULL );Delay_Ms_StopScheduler(500);
//	EC20_SendModuleCmd(CGPADDR, NULL );Delay_Ms_StopScheduler(500);
//	EC20_SendModuleCmd(AT_CSQ, NULL );Delay_Ms_StopScheduler(500);
//	sim900a_send_cmd("AT+CGSOCKCONT=1,\"IP\",\"CMNET\"\r\n","OK",100);	
//	Delay_Ms_StopScheduler(1000);//	Wait_For_Nms(1000);
//	sim900a_send_cmd("AT+CGPADDR\r\n","OK",100);	
//	Delay_Ms_StopScheduler(1000);//	Wait_For_Nms(1000);
//	sim900a_send_cmd("AT+CSQ\r\n","OK",100);	
//	Delay_Ms_StopScheduler(1000);//	Wait_For_Nms(1000);
//	sim900a_send_cmd("AT+CSQ\r\n","OK",100);	
//	Delay_Ms_StopScheduler(1000);//	Wait_For_Nms(1000);
////链接MQTT服务器
//	/*启动MQTT服务，激活PDP上下文*/
//		EC20_SendModuleCmd(MCONNECT, NULL );Delay_Ms_StopScheduler(500);
//	sim900a_send_cmd("AT+CMQTTSTART\r\n","OK",100);	
//Delay_Ms_StopScheduler(1000);//	Wait_For_Nms(1000);

//	Uart1SendsStr("ATQ1\r\n");				   //抑制回执OK
//	Wait_For_Nms(6000);
//uart1_rx_complete = 0;
//	Uart1SendsStr("AT+GSN\r\n");			  //取芯片序列号
	
//	Wait_For_Nms(1);
//	while(!uart1_rx_complete);	   //  临时测试屏蔽	
//	Wait_For_Nms(3000);

	/*获取一个将连接到MQTT服务器而不是SSL/TLS的客户机*/	
//	EC20_SendModuleCmd(CMQTTACCQ, NULL );Delay_Ms_StopScheduler(200);
//	strcpy(mqttclient,"AT+CMQTTACCQ=0,\"");
//	strcat(mqttclient,Clientid_IMEI);
//	strcat(mqttclient,"\"\r\n");
//	sim900a_send_cmd(mqttclient,"OK",100);	
//	Delay_Ms_StopScheduler(1000);//Wait_For_Nms(1000);
/***************END*************/	
	
//	/*设置遗嘱主题*/
//	sim900a_send_cmd("AT+CMQTTWILLTOPIC=0,10\r\n","OK",100);	
//	Wait_For_Nms(1000);
//	/*设置遗嘱消息*/
//	sim900a_send_cmd("AT+CMQTTWILLMSG=0,6,1\r\n","OK",100);	
//	Wait_For_Nms(1000);
	
///*连接到MQTT服务器 "www.szjcd.top",9001,"tcp" "szjcd","123456" */
//EC20_SendModuleCmd(CMQTTCONNECT, sChannal0.serverIP,sChannal0.MQTT_Name,sChannal0.MQTT_Password );
//	Delay_Ms_StopScheduler(500);
//RunResult runResult = sim900a_send_cmd("AT+CMQTTCONNECT=0,\"tcp://www.szjcd.top:9001\",60,1,\"szjcd\",\"123456\"\r\n","OK",100);	
//	Wait_For_Nms(1000);
/***************END*************/	
//portFree(mqttclient) ;
//portFree(Clientid_IMEI) ;

//	MqttSendCom("AT+CMQTTSUB=0,6,1\r\n","OK",100,"simcom");
//	Wait_For_Nms(1000);
//	MqttSendCom("AT+CMQTTTOPIC=0,5\r\n","OK",100,"test1");
//	Wait_For_Nms(1000);
//	MqttSendCom("AT+CMQTTSUBTOPIC=0,9,1\r\n","OK",100,"/rep");
//	Wait_For_Nms(1000);
//	sim900a_send_cmd("AT+CMQTTSUB=0\r\n","OK",100);
//	Wait_For_Nms(1000);
/*********************规格书实例*********************************/		
//	/*从服务器订阅一个消息*/
//	sim900a_send_cmd("AT+CMQTTSUB=0,9,1\r\n","OK",100);	
//	Wait_For_Nms(1000);
//	/*为发布消息设置主题*/
//	sim900a_send_cmd("AT+CMQTTTOPIC=0,9\r\n","OK",100);	
//	Wait_For_Nms(1000);
//	/*设置发布消息的有效负载*/
//	sim900a_send_cmd("AT+CMQTTPAYLOAD=0,60\r\n","OK",100);	
//	Wait_For_Nms(1000);
//	/*发布一条消息*/
//	sim900a_send_cmd("AT+CMQTTPUB=0,1,60\r\n","OK",100);	
//	Wait_For_Nms(1000);
//	/*为订阅消息设置一个主题*/
//	sim900a_send_cmd("AT+CMQTTSUBTOPIC=0,9,1\r\n","OK",100);	
//	Wait_For_Nms(1000);
//	/*订阅消息*/
//	sim900a_send_cmd("AT+CMQTTSUB=0\r\n","OK",100);	
//	Wait_For_Nms(1000);
//	/*从服务器取消订阅一个主题*/
//	sim900a_send_cmd("AT+CMQTTUNSUB=0,120\r\n","OK",100);	
//	Wait_For_Nms(1000);
//	/*和服务器断开连接*/	
//	sim900a_send_cmd("AT+CMQTTDISC=0,120\r\n","OK",100);	
//	Wait_For_Nms(1000);
//	/*释放客户端*/
//	sim900a_send_cmd("AT+CMQTTREL=0\r\n","OK",100);	
//	Wait_For_Nms(1000);
//	/*停止MQTT*/
//	sim900a_send_cmd("AT+CMQTTSTOP\r\n","OK",100);	
//	Wait_For_Nms(1000);
//portFree(mqttclient) ;
////portFree(Clientid_IMEI) ;
//return runResult;
//}

//上传服务器请求的 短信接收号码 或 电话被叫号码
// PortNum 端口号 1~10  sms_call 0 短信号码 1 电话号码
/*
注意服务器上端口顺序：1~8依次对应端口 9 湿度温度 10 停电来电  
*/
//+MPUB="/864708068765984D/upload/call_phone",0,0,
//"port=2&phone1=0&phone2=0&phone3=0&phone4=0&phone5=0&phone6=0&"
RunResult Mqtt_sms_call_phone(char PortNum, u8 sms_call)
{
	u16 *Address;
	RunResult  rusult;
	if((mqtt_only != W433Hard.Head.DeviceAlarmMode)&&(simANDmqtt != W433Hard.Head.DeviceAlarmMode))return OutParamErr;
	
	if(((PortNum < 1))||(PortNum > 10))return RUNERR;
	
	char *mqttBuf = (char*)portMalloc(130*sizeof (char)) ;	
	char *mqttps = mqttBuf+1;
	
	snprintf (mqttps,10,"port=%d&",PortNum);
	mqttps += 7;	//port=8&	
	
	if(sms_call)//电话号码
	{
	Address =  (u16*)Call_Number_Offset;
	}	
	else 						//短信号码
	{
	Address =	(u16*)SMS_Number_Offset;
	}
u8 eerpom_inx;	
//u8 eerpom_inx = (PortNum + 1)%10;//转换EEPROM 存取序号 0~9
	if(PortNum > Alarm_maxGroup)return RUNERR;//溢出报错	
	if(PortNum > 8)eerpom_inx = PortNum % 9;// 0温度湿度，1停电
	else eerpom_inx = PortNum + 1;
	for(u8 i=0; i<6; i++)
	 {
		memset(PhoneNumber,0,Mseg_Lenth);
//		u8 PhoneNumLen = EEPROM_read_Byte(MessgeAddr[PortNum-1]+Address[i]);
		u8 PhoneNumLen = EEPROM_read_Byte(MessgeAddr[eerpom_inx]+Address[i]); 
		if((PhoneNumLen > 2)&&(PhoneNumLen < 15))//地址首位，长度标识位	
			{	
//			EEPROM_read_n((MessgeAddr[PortNum-1]+Address[i]+1), PhoneNumber, PhoneNumLen);   //号码长度		
			EEPROM_read_n((MessgeAddr[eerpom_inx]+Address[i]+1), PhoneNumber, PhoneNumLen);   //号码长度		
				snprintf (mqttps,20,phone_num[i],(const char*)&PhoneNumber);//&phone1=电话号码
			mqttps += PhoneNumLen + 8;//8 phone1=1&
			}
		else
			{
			snprintf (mqttps,15,phone_num[i],"0");//如果没有号码 打印0
			mqttps += 1 + 8;	//8		phone1=0&
			}			
	 }
	 
//	rusult = EC20_SendHttpCmd(MPUB, NULL,MQTT_TOPIC[up_sms+sms_call], 0, mqttBuf) ;//	sms_call 0 短信号码 1 电话号码 
	*mqttBuf = (char)up_sms+sms_call;
	if( RW_OK != InsertQueueMemData(&MQTT_PublishQueue, mqttBuf, strlen(mqttBuf)) )//up_sms+sms_call
	{
		ErrorLogPrintf("%s,%d:Mqtt_sms_call_phone！",__FILE__, __LINE__) ;
	}	
//OUT_Mqtt_sms_call_phone:	
	 portFree(mqttBuf) ;
	 return rusult;
}
//上传服务器请求的 闭合短信 或 断开短信
// PortNum 端口号 1~10  close_open 0 闭合短信 1 断开短信
/*
注意服务器上端口顺序为：1~8依次对应端口 9 停电来电 10 湿度温度 
---和SIM短信有差异 3~10依次对应端口 1 停电来电 2 湿度温度
*/
RunResult Mqtt_PortMessage(char PortNum, u8 close_open)
{
	u16 Address,strlen;
	RunResult  rusult;
	if((mqtt_only != W433Hard.Head.DeviceAlarmMode)&&(simANDmqtt != W433Hard.Head.DeviceAlarmMode))return OutParamErr;
	if(((PortNum < 1))||(PortNum > 15))return RUNERR;
	
	char *mqttBuf = (char*)portMalloc(300) ;	
	char *mqttps = mqttBuf + 1;
	
		if(close_open)//断开短信
			{
			Address =  Open_Message_offset;
			}	
		else 						//闭合短信
			{
			Address =	Close_Message_offset;
			}	
	u8 eerpom_inx;
//u8 eerpom_inx = (PortNum + 1)%10;//转换EEPROM 存取序号 0~9
	if(PortNum > Alarm_maxGroup)return RUNERR;//溢出报错		
	if(PortNum > 8)eerpom_inx = PortNum % 9;// 0温度湿度，1停电
	else eerpom_inx = PortNum + 1;
			
	u8 StrLen = EEPROM_read_Byte(MessgeAddr[eerpom_inx]+Address);
		if((StrLen > 2)&&(StrLen < 200))//地址首位，长度标识位	
			{	
			EEPROM_read_n((MessgeAddr[eerpom_inx]+Address+1), mqttps, StrLen);   //mqttBuf短信长度	
			*(mqttps+StrLen) = 0;//mqttBuf
/*以下将UNICODE中插入 \u 分析符，并转小写*/				
				char * temp = MsegTemp;				
				while(StrLen)
				{
				if(StrLen >= 4)StrLen -= 4;else StrLen = 0;
				strncat (MsegTemp,"\\5cu",4);temp += 4;// 转义符 “\”0X5C u
				for(u8 i = 0; i< 4; i++,temp++,mqttps++)		
				*temp = tolower (*mqttps);	//转小写
				}	
				*temp = '\0';
/*END*/				
			strlen = snprintf (mqttBuf + 1,200,"port=%d&mesag=%s",PortNum,(const char*)&MsegTemp);//直接打印出内容
			}
		else
			{
			strlen = snprintf (mqttBuf + 1,50,"port=%d&mesag=%d",PortNum,0);
			}

//	rusult = EC20_SendHttpCmd(MPUB, NULL,MQTT_TOPIC[up_close_massage+close_open],0, mqttBuf) ;//close_open 0 闭合短信 1 断开短信 q=0 
			
	*mqttBuf = (char)up_close_massage+close_open;
	if( RW_OK != InsertQueueMemData(&MQTT_PublishQueue, mqttBuf, strlen + 1) )//up_sms+sms_call
	{
		ErrorLogPrintf("%s,%d:Mqtt_PortMessage！",__FILE__, __LINE__) ;
	}	
	
	portFree(mqttBuf) ;
			
	 return rusult;
}

//报警时上传alarm消息
//Receive_Alarm.Num 范围 2~9 对应服务器端口1~8  0 温度  1 停电
RunResult Mqtt_MpubAlarm(void)
{
	RunResult rusult;
	char c_or_o[2] = {0};
 if(Receive_Alarm.Num > 10)return RUNERR;
	
	if(Receive_Alarm.Adrr_Offset == Close_Message_offset)c_or_o[1] = '0';	//0 闭合报警 1 断开报警
	else c_or_o[1] = '1';//"1"
 
//rusult = EC20_SendHttpCmd(MPUB, NULL,MQTT_TOPIC[Receive_Alarm.Num], 0, c_or_o) ;//
	if(Receive_Alarm.Num == TemperHum_AddrSection) MQTT_AlarmT_Hm();// 0 温度 向服务器发布 温度和湿度报警
	else		// 1~9 
	{	
	if(Receive_Alarm.Num >= 2)c_or_o[0] = Receive_Alarm.Num - 2;	//范围 2~9 对应服务器端口1~8 ； 1 对应停电
	else c_or_o[0] = 1;	// 1 对应停电
			 
		if( RW_OK != InsertQueueMemData(&MQTT_PublishQueue, c_or_o, 2) )//up_sms+sms_call
		{
			ErrorLogPrintf("%s,%d:Mqtt_MpubAlarm！",__FILE__, __LINE__) ;
		}	
	}
	
return rusult;
}
//解析  message ；输出数值
//获取字符串中的 message 
//*mqtt_data 源数据 *compare  比较字串
//+MSUB: "mqtt/topic",9 byte,SSSSddddd
u8 MQTT_GetPurtNum (char *mqtt_data,char* compare)
{
char *haystack, *buf;//
	
	  haystack= mqtt_data;
//	 StrLen = strlen(Model_StrNeed);
		haystack = strstr( haystack, compare);//获取回路
//		haystack += (strlen(compare) + 2);
		haystack = strstr( haystack, "byte,");
		haystack += strlen("byte,");;
		buf= strstr( haystack, "\r");
		buf[0] = 0;
		if( *haystack < 'F' )
			{
				buf[0]='\0';
			return	atoi(haystack);
			}
		 else return 0xff;	//没有数据 返回ERROR	
}
//发布位置
void Mqtt_lbs(void)
{
char *MQTT_Infor = (char*)portMalloc(30*sizeof(u8) ) ;
u16 strlen;	
	strlen = snprintf (MQTT_Infor + 1,50,"lat=%s&lng=%s",LBS.lat,LBS.lon);
//	EC20_SendHttpCmd(MPUB, NULL,MQTT_TOPIC[lbs],0, MQTT_Infor) ;//
	
		*MQTT_Infor = lbs;
	if( RW_OK != InsertQueueMemData(&MQTT_PublishQueue, MQTT_Infor, strlen + 1) )//
	{
		ErrorLogPrintf("%s,%d:Mqtt_lbs！",__FILE__, __LINE__) ;
	}	
portFree(MQTT_Infor); 
}
//MQTT 发布 信号_温湿度
//TOPIC csq : csq=0&TEM=23.45&HUM=89.56
void MQTT_cqs_temper(void)
{	
char *MQTT_Infor = (char*)portMalloc(200*sizeof(u8) ) ;
int strlen;

	strlen = snprintf(MQTT_Infor+1,100,"CSQ=%s&TEM=%.2f&HUM=%.2f",(const char*)sEc20Param.csq,SENx.T,SENx.RH) ; //
	
//	EC20_SendHttpCmd(MPUB, NULL,MQTT_TOPIC[uphome],0, MQTT_Infor+1) ;//	
	*MQTT_Infor = (char)csq;
	if( RW_OK != InsertQueueMemData(&MQTT_PublishQueue, MQTT_Infor, strlen + 1) )//uphome
	{
		ErrorLogPrintf("%s,%d:MQTT_cqs_temper！",__FILE__, __LINE__) ;
	}	
portFree(MQTT_Infor); 
}
//MQTT 向服务器发布当前设备状态
//cmd  1:指令查询  0：常规检测
RunResult MQTT_DeviceState(u8 cmd)
{	
int strlen;
static u16 stateFirst = 0;
		ProtState.Portbit.Port9 = PowerOff; ProtState.Portbit.Port10 = DeivceFlag.RunState;
		ProtState.Portbit.Port11 = W433Hard.Head.Alarm_Remoter_Enable; ProtState.Portbit.Port12 = W433Hard.Head.DeviceAlarmMode;
	
	if((stateFirst == ProtState.PortFlag) && (!cmd))return RUNERR;
		stateFirst = ProtState.PortFlag;
char *MQTT_Infor = (char*)portMalloc(200*sizeof(u8) ) ;	
		
strlen = snprintf(MQTT_Infor+1,100,"PWR=%s&S1=%s&S2=%s&S3=%s&S4=%s&S5=%s&S6=%s&S7=%s&S8=%s&RUS=%d&EENA=%d&DEF=%d",  //&csq=%s
				portstate[PowerOff],\
				portstate[LED1],portstate[LED2],portstate[LED3],portstate[LED4],portstate[LED5],portstate[LED6],portstate[LED7],portstate[LED8],\
				DeivceFlag.RunState,W433Hard.Head.Alarm_Remoter_Enable,W433Hard.Head.DeviceAlarmMode) ;  //,(const char*)sEc20Param.csq,W433Hard.Head.Alarm_Remoter_Enable DeivceFlag.Alarm_ON=1; // 0 布防  1 撤防 	DeviceAlarmMode = (DeviceAlarmModeTYPE)
	
//	EC20_SendHttpCmd(MPUB, NULL,MQTT_TOPIC[uphome],0, MQTT_Infor+1) ;//	
	*MQTT_Infor = (char)uphome;
	if( RW_OK != InsertQueueMemData(&MQTT_PublishQueue, MQTT_Infor, strlen + 1) )//uphome
	{
		ErrorLogPrintf("%s,%d:MQTT_DeviceState！",__FILE__, __LINE__) ;
	}	
		if(cmd)Mqtt_lbs();
portFree(MQTT_Infor); 
	return RUNOK;
}
//MQTT 向服务器发布 温度和湿度 上下限报警阀值
void MQTT_AlarmTemHum(void)
{	
char *MQTT_Infor = (char*)portMalloc(50*sizeof(u8) ) ;
	u16 strlen;
//Message: TH#23#2#85#10#
strlen = snprintf(MQTT_Infor + 1,100,"TH#%d#%d#%d#%d#", 
					AlarmValue.ValueUnit.Temper_H,AlarmValue.ValueUnit.Temper_L,AlarmValue.ValueUnit.Humidi_H,AlarmValue.ValueUnit.Humidi_L	) ;//
	
//	EC20_SendHttpCmd(MPUB, NULL,MQTT_TOPIC[SetTemperHum],0, MQTT_Infor) ;//
	*MQTT_Infor = (char)SetTemperHum;
	if( RW_OK != InsertQueueMemData(&MQTT_PublishQueue, MQTT_Infor, strlen + 1) )//uphome
	{
		ErrorLogPrintf("%s,%d:MQTT_AlarmTemHum！",__FILE__, __LINE__) ;
	}		
portFree(MQTT_Infor); 
}
//MQTT 向服务器发布 温度和湿度报警
//temH&85&temL&-10&humH&98&humL&15&
RunResult MQTT_AlarmT_Hm(void)
{	
	char *MQTT_Infor = (char*)portMalloc(40*sizeof(u8) ) ;
	char *MQTT_Infor_sp = MQTT_Infor + 1;
	//Message: temH&85&temL&-10&humH&98&humL&15&
	if(!AlarmTemHum.Type_TorH) //温度触发报警	0
	{
		if(AlarmTemHum.Talarm_flag)//温度报警
			{snprintf(MQTT_Infor_sp,100,"temH&%.2f&", SENx.T);}
	//解除温度报警
		else
			{snprintf(MQTT_Infor_sp,100,"temH&ff&");}
	}
	else
	{
	//湿度
		if(AlarmTemHum.Halarm_flag)
			{	snprintf(MQTT_Infor_sp,100,"humH&%.2f&", SENx.T);}
	//解除湿度报警
		else 
			{	snprintf(MQTT_Infor_sp,100,"humH&ff&");	}
	}

//EC20_SendHttpCmd(MPUB, NULL,MQTT_TOPIC[alarmTepHum],0, MQTT_Infor) ;//
	*MQTT_Infor = (char)alarmTepHum;
	if( RW_OK != InsertQueueMemData(&MQTT_PublishQueue, MQTT_Infor, strlen(MQTT_Infor)) )//uphome
	{
		ErrorLogPrintf("%s,%d:MQTT_AlarmT_Hm！",__FILE__, __LINE__) ;
	}	
portFree(MQTT_Infor); 
		return RUNOK;
}


//char compar(int Dat1,int dat2)
//{
//	
//	else if((SENx.T > AlarmValue.ValueUnit.Temper_L)&&(AlarmTemHum.Talarm_flag))//温度高于设定低温阀值补偿+1 且是在发生报警标志1
//		{
//		AlarmTemHum.Talarm_flag = AlarmTemHum.TemL_flag = 0;//解除温度过低标志置0
//		Send_Alarm.Adrr_Offset = Open_Message_offset;//断开报警
//		AlarmTemHum.Type_TorH = 0;//解除温度报警
//		}
//}
//温度低于或高于阀值2度 复位告警  湿度低于或高于阀值10%复位告警 下次再次触发阀值时产生报警
//触发温度和湿度 上下限报警
//temH&85&temL&-10&humH&98&humL&15&
//RunResult AlarmTemHum_Trigger(void)
//{	
////Message: temH&85&temL&-10&humH&98&humL&15&
////	AlarmValue.ValueUnit.Temper_L=-2;
////	int ty=-3;
////AlarmValue.ValueUnit.Temper_H = 33;
//	if((AlarmValue.ValueUnit.Temper_H != 0xffffffff)&&(SENx.T > AlarmValue.ValueUnit.Temper_H)&&(!AlarmTemHum.TemH_flag))//温度高于设定阀值 且没有发生过报警
//		{
//		AlarmTemHum.Talarm_flag = AlarmTemHum.TemH_flag = 1;	//报警标志置1 温度过高标志置1
//		AlarmTemHum.TemL_flag = 0;Send_Alarm.Adrr_Offset = Close_Message_offset;//温度过低标志清0 闭合报警
//		AlarmTemHum.Type_TorH = 0;//高温度触发报警
//		}
//	else if((AlarmValue.ValueUnit.Temper_L != 0xffffffff)&&(SENx.T < (AlarmValue.ValueUnit.Temper_L))&&(!AlarmTemHum.TemL_flag))//温度低于设定阀值 且没有发生过报警
////		else if((AlarmValue.ValueUnit.Temper_L != 0xffffffff)&&(ty < (AlarmValue.ValueUnit.Temper_L))&&(!AlarmTemHum.TemL_flag))//温度低于设定阀值 且没有发生过报警
//  	{
//		AlarmTemHum.Talarm_flag = AlarmTemHum.TemL_flag = 1;//报警标志置1 温度过低标志置1
//		AlarmTemHum.TemH_flag = 0;Send_Alarm.Adrr_Offset = Close_Message_offset;//清0温度过高标志 断开报警
//		AlarmTemHum.Type_TorH = 0;//低温度触发报警
//		}
////解除温度报警
//	else if((AlarmValue.ValueUnit.Temper_H != 0xffffffff)&&(SENx.T < AlarmValue.ValueUnit.Temper_H - 2)&&(AlarmTemHum.TemH_flag))//温度低于设定高温阀值补偿-1 且是在发生报警标志1
//		{
//		AlarmTemHum.Talarm_flag = AlarmTemHum.TemH_flag = 0;	//解除温度过高标志置0
//		Send_Alarm.Adrr_Offset = Open_Message_offset;//断开报警
//		AlarmTemHum.Type_TorH = 0;//解除高温度报警
//		}
//	else if((AlarmValue.ValueUnit.Temper_L != 0xffffffff)&&(SENx.T > AlarmValue.ValueUnit.Temper_L + 2)&&(AlarmTemHum.TemL_flag))//温度高于设定低温阀值补偿+1 且是在发生报警标志1
//		{
//		AlarmTemHum.Talarm_flag = AlarmTemHum.TemL_flag = 0;//解除温度过低标志置0
//		Send_Alarm.Adrr_Offset = Open_Message_offset;//断开报警
//		AlarmTemHum.Type_TorH = 0;//解除低温度报警
//		}
////湿度
//	else if((AlarmValue.ValueUnit.Humidi_H != 0xffffffff)&&(SENx.RH > AlarmValue.ValueUnit.Humidi_H)&&(!AlarmTemHum.HumH_flag))//AlarmTemHum.Halarm_flag
//		{
//		AlarmTemHum.Halarm_flag = AlarmTemHum.HumH_flag = 1;	//
//		AlarmTemHum.HumL_flag = 0;Send_Alarm.Adrr_Offset = Close_Message_offset;
//		AlarmTemHum.Type_TorH = 1;//湿度触发报警
//		}
//	else if((AlarmValue.ValueUnit.Humidi_L != 0xffffffff)&&(SENx.RH < AlarmValue.ValueUnit.Humidi_L)&&(!AlarmTemHum.HumL_flag))
//		{
//		AlarmTemHum.Halarm_flag = AlarmTemHum.HumL_flag = 1;//
//		AlarmTemHum.HumH_flag = 0;Send_Alarm.Adrr_Offset = Close_Message_offset;
//		AlarmTemHum.Type_TorH = 1;
//		}
////解除湿度报警
//	else if((AlarmValue.ValueUnit.Humidi_H != 0xffffffff)&&(SENx.RH < AlarmValue.ValueUnit.Humidi_H - 10)&&(AlarmTemHum.HumH_flag))
//		{
//		AlarmTemHum.Halarm_flag = AlarmTemHum.HumH_flag = 0;	//
//		Send_Alarm.Adrr_Offset = Open_Message_offset;
//		AlarmTemHum.Type_TorH = 1;
//		}
//	else if((AlarmValue.ValueUnit.Humidi_L != 0xffffffff)&&(SENx.RH > AlarmValue.ValueUnit.Humidi_L + 10)&&(AlarmTemHum.HumL_flag))
//		{
//		AlarmTemHum.Halarm_flag = AlarmTemHum.HumL_flag = 0;//
//		Send_Alarm.Adrr_Offset = Open_Message_offset;
//		AlarmTemHum.Type_TorH = 1;
//		}	
//	else return OutParamErr;//

//	Send_Alarm.Num = TemperHum_AddrSection;     //0温湿度 报警序号
//	xQueueSend(AlarmPort_Queue,&Send_Alarm,0);//
//	return RUNOK;
//}
//高低温报警 产生警后温度低于L或高于H阀值 复位告警 下次再次触发有效
//触发温度和湿度 上下限报警
//temH&85&temL&-10&humH&98&humL&15&
//AlarmValue.ValueUnit.Temper_H > AlarmValue.ValueUnit.Temper_L 高低温报警 采暖
//AlarmValue.ValueUnit.Temper_H < AlarmValue.ValueUnit.Temper_L 高低温报警 制冷 后续开发
RunResult AlarmTemHum_Trigger(void)
{	
//Message: temH&85&temL&-10&humH&98&humL&15&
//	AlarmValue.ValueUnit.Temper_L=-2;
//	int ty=-3;
//AlarmValue.ValueUnit.Temper_H = 33;
	if((AlarmValue.ValueUnit.Temper_H != 0xffffffff)&&(SENx.T >= AlarmValue.ValueUnit.Temper_H)&&(!AlarmTemHum.TemH_flag))//温度高于设定阀值 且没有发生过报警
		{
		AlarmTemHum.Talarm_flag = AlarmTemHum.TemH_flag = 1;	//报警标志置1 温度过高标志置1
		AlarmTemHum.TemL_flag = 0;Send_Alarm.Adrr_Offset = Close_Message_offset;//温度过低标志清0 闭合报警
		AlarmTemHum.Type_TorH = 0;//高温度触发报警
		}
	else if((AlarmValue.ValueUnit.Temper_L != 0xffffffff)&&(SENx.T <= (AlarmValue.ValueUnit.Temper_L))&&(!AlarmTemHum.TemL_flag))//温度低于设定阀值 且没有发生过报警
//		else if((AlarmValue.ValueUnit.Temper_L != 0xffffffff)&&(ty < (AlarmValue.ValueUnit.Temper_L))&&(!AlarmTemHum.TemL_flag))//温度低于设定阀值 且没有发生过报警
  	{
		AlarmTemHum.Talarm_flag = AlarmTemHum.TemL_flag = 1;//报警标志置1 温度过低标志置1
		AlarmTemHum.TemH_flag = 0;Send_Alarm.Adrr_Offset = Close_Message_offset;//清0温度过高标志 断开报警
		AlarmTemHum.Type_TorH = 0;//低温度触发报警
		}
//解除温度报警
	else if((AlarmValue.ValueUnit.Temper_H != 0xffffffff)&&(SENx.T <= AlarmValue.ValueUnit.Temper_L)&&(AlarmTemHum.TemH_flag))//温度低于设定高温阀值补偿-1 且是在发生报警标志1
		{
		AlarmTemHum.Talarm_flag = AlarmTemHum.TemH_flag = 0;	//解除温度过高标志置0
		Send_Alarm.Adrr_Offset = Open_Message_offset;//断开报警
		AlarmTemHum.Type_TorH = 0;//解除高温度报警
		}
	else if((AlarmValue.ValueUnit.Temper_L != 0xffffffff)&&(SENx.T >= AlarmValue.ValueUnit.Temper_H)&&(AlarmTemHum.TemL_flag))//温度高于设定低温阀值补偿+1 且是在发生报警标志1
		{
		AlarmTemHum.Talarm_flag = AlarmTemHum.TemL_flag = 0;//解除温度过低标志置0
		Send_Alarm.Adrr_Offset = Open_Message_offset;//断开报警
		AlarmTemHum.Type_TorH = 0;//解除低温度报警
		}
//湿度
	else if((AlarmValue.ValueUnit.Humidi_H != 0xffffffff)&&(SENx.RH >= AlarmValue.ValueUnit.Humidi_H)&&(!AlarmTemHum.HumH_flag))//AlarmTemHum.Halarm_flag
		{
		AlarmTemHum.Halarm_flag = AlarmTemHum.HumH_flag = 1;	//
		AlarmTemHum.HumL_flag = 0;Send_Alarm.Adrr_Offset = Close_Message_offset;
		AlarmTemHum.Type_TorH = 1;//湿度触发报警
		}
	else if((AlarmValue.ValueUnit.Humidi_L != 0xffffffff)&&(SENx.RH <= AlarmValue.ValueUnit.Humidi_L)&&(!AlarmTemHum.HumL_flag))
		{
		AlarmTemHum.Halarm_flag = AlarmTemHum.HumL_flag = 1;//
		AlarmTemHum.HumH_flag = 0;Send_Alarm.Adrr_Offset = Close_Message_offset;
		AlarmTemHum.Type_TorH = 1;
		}
//解除湿度报警
	else if((AlarmValue.ValueUnit.Humidi_H != 0xffffffff)&&(SENx.RH <= AlarmValue.ValueUnit.Humidi_L)&&(AlarmTemHum.HumH_flag))
		{
		AlarmTemHum.Halarm_flag = AlarmTemHum.HumH_flag = 0;	//
		Send_Alarm.Adrr_Offset = Open_Message_offset;
		AlarmTemHum.Type_TorH = 1;
		}
	else if((AlarmValue.ValueUnit.Humidi_L != 0xffffffff)&&(SENx.RH >= AlarmValue.ValueUnit.Humidi_H)&&(AlarmTemHum.HumL_flag))
		{
		AlarmTemHum.Halarm_flag = AlarmTemHum.HumL_flag = 0;//
		Send_Alarm.Adrr_Offset = Open_Message_offset;
		AlarmTemHum.Type_TorH = 1;
		}	
	else return OutParamErr;//

	Send_Alarm.Num = TemperHum_AddrSection;     //0温湿度 报警序号
	xQueueSend(AlarmPort_Queue,&Send_Alarm,0);//
	return RUNOK;
}

//检查服务器发布的数据指令 做相应的动作
//*mqtt_data 源数据
void Mqtt_check(char *mqtt_data)
{
u8 Numbre;	
	
	if	(hand(mqtt_data,"/lbsLocationReq"))		  //上传最新位置
	{
		Mqtt_lbs();
	}
//	else if(hand(mqtt_data,"/download/ALARM"))		 
//	{
//		Numbre = MQTT_GetPurtNum(mqtt_data,"/ERASE");
//		//		if(Numbre == 0)DeivceFlag.Alarm_ON=0;
//		W433Hard.Head.DeviceAlarmMode = (DeviceAlarmModeTYPE)Numbre;
//		SectorErase_protect(CSCA_W433_Address, &Numbre,Alarm_Mode_offAddr,1,0);	//报警方式 存储地址 0x0801F000 +偏移0x40+9	
//		//		else if(Numbre == 1)DeivceFlag.Alarm_ON=1; // 0 布防  1 撤防 
//		MQTT_DeviceState();		  //发布设备数据 Topic: /upload/home
//	}
//	else if(hand(mqtt_data,"/download/ERASE"))		  //端口->擦除信息
//	{
//		Numbre = MQTT_GetPurtNum(mqtt_data,"/ERASE");
//		if(Numbre < 10)
//		EEPROM_SectorErase(MessgeAddr[Numbre]);
//	}	
	else if(hand(mqtt_data,"/download/GET"))		  //端口->读取信息
	{
		Numbre = MQTT_GetPurtNum(mqtt_data,"/GET");
		
		if((Numbre>0)&&(Numbre < 0xff))
			{
			Mqtt_sms_call_phone(Numbre,0);// PortNum 端口号 1~9  sms_call 0 短信号码 1 电话号码
			Mqtt_sms_call_phone(Numbre,1);
			Mqtt_PortMessage(Numbre, 0);// PortNum 端口号 1~9  close_open 0 闭合短信 1 断开短信
			Mqtt_PortMessage(Numbre, 1);	
			if(Numbre==10)MQTT_AlarmTemHum();
			}
			
	}
//	else if(hand(mqtt_data,"/download/QUERY"))MQTT_DeviceState();		  //服务器请求获取设备状态数据 设备发布Topic: /upload/home

}
//发布消息
//*Topic		（指令）
//*pos（指令跟随的内容）
//*messeage	 （内容）
//RunResult Mqtt_Mpub(char *Topic, u8 pos, char *messeage)
//{

////	UARTx_SendString(EC20_UART,(u8*)cmd,NULL);
////	Delay_Ms_StopScheduler(200);//Wait_For_Nms(200);
//if(RUNERR == EC20_SendHttpCmd(MPUB, NULL, Topic, pos, messeage))return RUNERR ;//	

//	return RUNOK;
//}
//订阅 发布主题 通配符“#”
RunResult mqttsub_Push(void)
{
/*
char *MQTT_Infor = (char*)portMalloc(30*sizeof(u8) ) ;	
//	strncpy (MQTT_Infor,Clientid_IMEI,16);
	snprintf(MQTT_Infor,30,"/%s/download/#",Clientid_IMEI);
//if(RUNERR == EC20_SendHttpCmd(MSUB, NULL, "DownInformation"))return RUNERR ;//订阅主题	
if(RUNERR == EC20_SendHttpCmd(MSUB, NULL, MQTT_Infor))return RUNERR ;//订阅主题
Delay_Ms_StopScheduler(100);	
	snprintf(MQTT_Infor,30,"/%s/lbsLocationReq",Clientid_IMEI);	
if(RUNERR == EC20_SendHttpCmd(MSUB, NULL, MQTT_Infor))return RUNERR ;//订阅主题		
portFree(MQTT_Infor); 	
*/


if(RUNERR == EC20_SendHttpCmd(MSUB, NULL, "/download/#"))return RUNERR ;//订阅主题
Delay_Ms_StopScheduler(100);	
if(RUNERR == EC20_SendHttpCmd(MSUB, NULL, "/lbsLocationReq"))return RUNERR ;//订阅主题		

return RUNOK;
}


	/**参数设置**/
//	MqttSendCom("AT+CMQTTSUBTOPIC=0,15,1\r\n","OK",100,"/set/parameters");
//	Wait_For_Nms(1000);
//	/**打开/关闭总电源**/
//	MqttSendCom("AT+CMQTTSUBTOPIC=0,10,1\r\n","OK",100,"/set/power");
//	Wait_For_Nms(1000);
//	/**打开/关闭远程摄像头电源**/
//	MqttSendCom("AT+CMQTTSUBTOPIC=0,17,1\r\n","OK",100,"/set/camera_power");
//	Wait_For_Nms(1000);
//	/**立即执行/停止融冰操作**/
//	MqttSendCom("AT+CMQTTSUBTOPIC=0,10,1\r\n","OK",100,"/set/deice");
//	Wait_For_Nms(1000);
//	/**立即进行/停止角度倾斜测量**/
//	MqttSendCom("AT+CMQTTSUBTOPIC=0,10,1\r\n","OK",100,"/set/angle");
//	Wait_For_Nms(1000);
//	/**单独打开/关闭备用远程控制端口1-4**/
//	MqttSendCom("AT+CMQTTSUBTOPIC=0,9,1\r\n","OK",100,"/set/port");
//	Wait_For_Nms(1000);
//	/**单独打开/关闭备用干节点接口1-2**/
//	MqttSendCom("AT+CMQTTSUBTOPIC=0,9,1\r\n","OK",100,"/set/node");
//	Wait_For_Nms(1000);

	/**测量数据上报**/
//	MqttSendCom("AT+CMQTTTOPIC=0,20\r\n","OK",100,"/report/data/measure");
//	Wait_For_Nms(1000);
//	/**发电状态及环境温度上报**/
//	MqttSendCom("AT+CMQTTTOPIC=0,19\r\n","OK",100,"/report/data/common");
//	Wait_For_Nms(1000);
//	/**设备状态上报**/
//	MqttSendCom("AT+CMQTTTOPIC=0,19\r\n","OK",100,"/report/data/device");
//	Wait_For_Nms(1000);
//	/**命令执行结果上报**/
//	MqttSendCom("AT+CMQTTTOPIC=0,14\r\n","OK",100,"/report/result");
//	Wait_For_Nms(1000);




