//#include "user_tcp.h" 
//#include <stdlib.h>
//#include "FreeRTOS.h"
//#include "event_groups.h"
#include "main.h"
/********************************************************************************
  * @file    gate_tcp.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   提供Quectel模块EC20关于TCP/IP硬件驱动程序
  ******************************************************************************
  * @attention
  * 约定基本名词如下：
	*          contextID:链路ID  connetcID：通道ID   channal：连接通道
  * EC20模块链路ID范围1~16，每一个链路ID都会对应一个本地IP； 通道ID范围0~11。
  * 每一个链路ID可以有12个通道ID。
  * 本驱动强制规定，TCP/IP协议只用一个链路1D（即contextID=1）用于TCP链路。
  * 通过不同的通道ID我们可以多台不同IP和端口的服务器。
  * 结构体ChannalP_s封装了每一路连接通道的参数。
*******************************************************************************/
//FrameQueue_s      sTcp0Queue ;   //需要TCP通道0 发送的数据在此环形缓冲区，在OS中有个TCP发送的任务，一直在发送TCP数据，需要发送数据只需要将数据丢到此缓冲区即可

/*****************************************
*usertcp内部常变量
****************************************/

/*****************************************
*usertcp全局变量定义
****************************************/                         
ChannalP_s sChannal0= { TCP_CONNECTID0, TCP_SERVERIP, TCP_SERVERPORT, TCP_LOCALPORT ,MQTT_Name, MQTT_PWD};	
//ChannalP_s s_Channal1    =  { TCP_CONNECTID1, TCP_CONNECTID1_SERVERIP, TCP_CONNECTID1_SERVERPORT, TCP_CONNECTID1_LOCALPORT };  //TCP通道1
//ChannalP_s s_Channal2    =  { TCP_CONNECTID2, TCP_CONNECTID2_SERVERIP, TCP_CONNECTID2_SERVERPORT, TCP_CONNECTID2_LOCALPORT };  //TCP通道2

/**************************************************************************************************
* 名    称：  void SetAppTcpIP(ChannalP_s *psChannal, char* ip)
* 功能说明：  设置登TCP服务器的IP
**************************************************************************************************/
//void SetAppTcpIP(ChannalP_s *psChannal, char* ip)
//{	
//  memset(psChannal->serverIP , 0, MAX_IP_LEN ) ;
//	strcpy((char*)psChannal->serverIP, ip) ;
//}

/**************************************************************************************************
* 名    称：  void SetAppTcpPort(ChannalP_s *psChannal, char* port)
* 功能说明：  设置登TCP服务器的PORT
**************************************************************************************************/
//void SetAppTcpPort(ChannalP_s *psChannal, char* port)
//{	
//  //memset(psChannal->serverPort , 0, MAX_IP_LEN ) ;
//	//strcpy((char*)psChannal->serverIP, ip) ;
//	psChannal->serverPort = atoi(port) ;
//}

/*********************************************************************************************************************
* 名    称：RunResult AppTcpInit(void)
* 功    能：断开Socket 去激活TCP链路的PDP
*********************************************************************************************************************/
//void TcpDisconnetc(void)
//{
//	 Close_Socket(TCP_CONNECTID0) ;  //断开Socket
//   Deact_Context(TCP_CONTEXTID) ;  //去激活TCP链路的PDP
//}

/*********************************************************************************************************************
* 名    称：RunResult AppTcpInit(void)
* 功    能：初始化应用TCP链路的PDP和打开TCP0的通道SOCKET服务
*********************************************************************************************************************/
RunResult AppTcpInit(void)
{
	RunResult runResult = TIMEOUT ;
  runResult = Tcp_PDP_Init() ;                                       //TCP链路PDP初始化
if( RUNERR == runResult)
	{
	runResult = Tcp_PDP_Init() ;
	vTaskDelay(2*configTICK_RATE_HZ);    //延时2S钟 再执行下次TCP连接		
	}
	
//	if( RUNOK == runResult)
//	 {
//		  runResult = TIMEOUT ;
//		  runResult = Tcp_Channal_Init(&sChannal0) ;
//			if( RUNOK == runResult)
//				{
//					AppLogPrintf( "TCP Server连接成功:%s:%d ！", sChannal0.serverIP, sChannal0.serverPort) ;
//					StatusBarPrintf("TCP Server连接成功");
//					return(runResult) ;
//				}
//			else
//				{
//					ErrorLogPrintf("TCP Server连接失败:%s:%d ！", sChannal0.serverIP, sChannal0.serverPort) ;
//					StatusBarPrintf("TCP Server连接失败");
//				}
//			vTaskDelay(2*configTICK_RATE_HZ);    //延时2S钟 再执行下次TCP连接				
//	 }
//  else
//	 {
//	   return(runResult) ;
//	 }
	return(runResult) ;
}

/**************************************************************************************************
* 名    称： RunResult TcpWritedata( UPDATATYPE_e updataType, char *format, ...) 
* 说    明： 通过TCP将可变参数数据上传到TCP SERVER
* 入口参数：
*				 @param1  updataType: UPDATATYPE_e枚举类型数据，表明数据帧类型
*				     @arg  ERRORRAME
*            @arg  WARNINGFRAME	
*            @arg  HEARTFRAME	
*            @arg  TCPLOGFRAME	
*            @arg  TCPCMDBACKFRAME
*        @param2 char *format,...  可变参变量
*				 @param3 ... ：可变参数
* 出口参数：
*       @param1 status: RunResult枚举类型数据
* 调用方法：  TcpWritedata( TCPLOGFRAME, "I am heart %d .",  5)  ; 
* 数据帧格式： (L0142MAC:31FFD405524E353728902251;00&I am heart 5 .$X)
* 注    意：  最大负载长度为：TCP_LOADBUF_MAXLEN
  ************************************************************************************************************/
//RunResult TcpWritedata( UPDATATYPE_e updataType, char *format, ...) 
//{
//	  static uint8_t counter = 0x30 ;
//		RunResult status = RUNOK ;
//		char *upDataBuf = (char*)portMalloc(TCP_LOADBUF_MAXLEN+sizeof(TcpFrame_S)+32) ;             //分配内存长度为负载长度+TcpFrame_S结构体长度 
//	  TcpFrame_S *sTcpFrameData = (TcpFrame_S *)upDataBuf ;                                                       //tcp数据帧变量	
//		va_list ap;  
//    va_start(ap, format);     
//    int outLen = vsnprintf(&(sTcpFrameData->loadHead)+1 , TCP_LOADBUF_MAXLEN, (const char*)format, ap ) ; 	//到此为止，所有的参数情况已经汇总到upDataBuf了 
//		if((outLen<=0)||( outLen > TCP_LOADBUF_MAXLEN)) 	 
//		  {
//				ErrorLogPrintf("%s,%d:upDataBuf spillover！",__FILE__, __LINE__) ;
//				va_end (ap);
//				portFree(upDataBuf) ;
//				return RUNERR ;
//			}
//		sTcpFrameData->head = '(' ;
//		sTcpFrameData->frameType = updataType ;
//		sTcpFrameData->loadLen[0] = outLen/100 + 0x30;
//		sTcpFrameData->loadLen[1] = outLen%100/10 + 0x30 ;
//		sTcpFrameData->loadLen[2] = outLen%10 + 0x30;
//		sTcpFrameData->frameNum = counter ;
//		counter ++ ;
//		if( counter > 0x39 )   //防止counter字符数字溢出
//		 {
//			 counter = 0x30 ;
//		 }
//		strncpy(sTcpFrameData->macHead, "MAC:", 4) ;
//    GetDeviceMacAddress(sTcpFrameData->macid, STRMACID) ; 
//		sTcpFrameData->macTail = ';' ;
//		memset(sTcpFrameData->cmdCode, 0x30, 2) ;
//		sTcpFrameData->loadHead = '&' ;
////		for(int n =0; n<outLen; n++)     //将loadBuf中的0x0D 0x0A替换为*，防止转发软件生成LOG混乱。
////		{
////		 if( (*(sTcpFrameData->loadBuf+n) == 0x0D ) || (*(sTcpFrameData->loadBuf+n) == 0x0A) )
////		 {
////			 *(sTcpFrameData->loadBuf+n) = '*' ;
////		 }
////		} 		
//    strncat(upDataBuf, "$X)", 3) ;	 //TcpFrame_S结构体的最后三个字节拼接到	upDataBuf尾部 
//		if( RW_OK != InsertQueueMemData(&sTcp0Queue, upDataBuf, strlen(upDataBuf))) 
//			{
//				ErrorLogPrintf("%s,%d:sTcp0Queue spillover！", __FILE__, __LINE__) ;
//			}
//		va_end (ap);
//		portFree(upDataBuf) ;		
//		return (status) ;
//}





