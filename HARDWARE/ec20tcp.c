//#include "ec20tcp.h" 
//#include <stdlib.h>
//#include "syslib.h"  //#define STR2(R)  STR1(R)
//#include "includes.h"
#include "n32g45x_dma.h"
#include "main.h" 
/********************************************************************************
  * @file    ec20tcp.c
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
  * 结构体ChannalP_s封装了每一路连接通道的参数
	* @use：
	*  先调用Tcp_PDP_Init()初始化TCP链路，接着调用Tcp_Channal_Init打开Socket，最后通过Tcp_SendData发送TCP数据
*******************************************************************************/

/*****************************************
*内部使用的常变量定义
****************************************/
#define  TCP_CMDPACK_LEN        128        //EC20 TCP相关命令字符串的最大长度
//extern volatile EC20_CMD_DATA_s sNetCmd[14];
/********************************************************
ec20模块TCP/IP相关AT指令处理
*********************************************************/
enum eTcpCmdNum 
{ 
	AT_CSTT =0, AT_CIICR =1, AT_CIFSR =2, AT_CGATT =3,AT_CIPMUX =4,AT_CGPADDR=5,
	AT_CIPQSEND=6, AT_CIPSHUT = 7,TCPSENDBUF =8, AT_CIPSTART =9, AT_CIPSTATUS =10, 
	AT_CIPSEND =11, AT_CIPCLOSE =12
} ;   //枚举ec20模块TCP相关指令

volatile EC20_CMD_DATA_s sTcpCmd[13]=
{
//  cmdNum            cmdStr,                                   timeout(100ms),    trueStr,         trueOffset falseStr revResult rtyNum
	{AT_CSTT,      "AT+CSTT=\"%s\"\r\n",            					8,        "OK" ,        -1,    	"ERROR",  TIMEOUT,    5  			},//配置接入点APN
	{AT_CIICR,   	 "AT+CIICR\r\n",                          	15,       "OK" , 				-1,    	"ERROR",  TIMEOUT,    3       },//激活移动场景
	{AT_CIFSR,   	 "AT+CIFSR\r\n",                          	15,       "." , 				12,    	"ERROR",  TIMEOUT,    3       },//查询本地IP
/*扩展指令*/
	{AT_CGATT,     "AT+CGATT?\r\n",                           8,        "+CGATT: 1" , -1,    "ERROR",  TIMEOUT,    5       },//将MT附着GPRS 0 分离 1 附着
	{AT_CIPMUX,    "AT+CIPMUX=%d\r\n",            				8,        "OK" ,        -1,    	"ERROR",  TIMEOUT,    5  			},//多IP链接 0 单路 1 多路
  {AT_CGPADDR,       "AT+CGPADDR\r\n", 												5,          "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //显示 PDP 地址 
	
  {AT_CIPQSEND,   "AT+CIPQSEND=%d\r\n", 												5, 								"OK" ,						 -1,   		 "ERROR",   TIMEOUT,   3  },//设置为快发模式（ 推荐使用这种模式）
	{AT_CIPSHUT,    "AT+CIPSHUT\r\n",                          	5,       						"SHUT OK" , 					-1,    "ERROR",  	TIMEOUT,    3  },//关闭移动场景 强制等待2秒
  {TCPSENDBUF,        "%s" ,                                    (2*10),              "SEND OK" ,      -1,   		 "ERROR",  TIMEOUT,   1       }, //SOCKET发送负载数据
	{AT_CIPSTART,   "AT+CIPSTART="STR2(TCP_CONNECTID0)",\"TCP\",\"%s\",%s\r\n", (20*10), "CONNECT OK" , -1,   		 "ERROR",   TIMEOUT,   3  },//打开socket 手册回码等待150S
	{AT_CIPSTATUS,  "AT+CIPSTATUS\r\n",                          15,        				 "OK" , 						-1,  			 "ERROR",  TIMEOUT,   3  },//查询socket状态
  
	{AT_CIPSEND,    "AT+CIPSEND=%d\r\n" ,                   		(2*10),              ">" ,              -1,  			 "ERROR",  TIMEOUT,   1  }, //通过socket发送数据
  {AT_CIPCLOSE,   "AT+CIPCLOSE=%d,0\r\n" ,                  	(10*10),             "CLOSE OK" ,       -1,  			 "ERROR",  TIMEOUT,   2  }, //关闭socket  0慢关 1快关

//	{OPENSOCKET,        "AT+QIOPEN="STR2(TCP_CONTEXTID)",%d,\"TCP\",\"%s\",%d,%d,1\r\n", (20*10), "+QIOPEN:", -1,"ERROR",  TIMEOUT,   1       }, //打开socket 手册回码等待150S
//  {CLOSESOCKET,       "AT+QICLOSE=%d,10\r\n" ,                  (10*10),             "OK" ,              -1,   "ERROR",  TIMEOUT,   2       }, //关闭socket  
//  {QUERYSOCKET,       "AT+QISTATE=1,%d\r\n" ,                    5,                  "+QISTATE:" ,       -1,   "ERROR",  TIMEOUT,   2       }, //查询socket状态
//  {TCPSENDCMD,        "AT+QISEND=%d,%d\r\n" ,                   (2*10),              ">" ,               -1,   "ERROR",  TIMEOUT,   1       }, //通过socket发送数据
} ;   //EC20模块TCP相关指令的EC20_CMD_DATA_s结构体类型参数


/**************************************************************************************************
* 名    称：  static const char *TcpCmdNumToString(enum eTcpCmdNum result)
* 功能说明：  输出枚举成员名的字符串指针。
* 入口参数：  eTcpCmdNum类型的枚举
* 出口参数：  为枚举的成员名字符串指针 
**************************************************************************************************/
static inline const char *TcpCmdNumToString(enum eTcpCmdNum result)
{
    switch (result)
    {
			ENUM_CHIP_TYPE_CASE(AT_CSTT)
			ENUM_CHIP_TYPE_CASE(AT_CIICR)
			ENUM_CHIP_TYPE_CASE(AT_CIFSR)
			ENUM_CHIP_TYPE_CASE(AT_CGATT)
			ENUM_CHIP_TYPE_CASE(AT_CIPMUX)
			ENUM_CHIP_TYPE_CASE(AT_CGPADDR)

			ENUM_CHIP_TYPE_CASE(AT_CIPQSEND)
			ENUM_CHIP_TYPE_CASE(AT_CIPSHUT)  
			ENUM_CHIP_TYPE_CASE(TCPSENDBUF)
			ENUM_CHIP_TYPE_CASE(AT_CIPSTART)
			ENUM_CHIP_TYPE_CASE(AT_CIPSTATUS)

			ENUM_CHIP_TYPE_CASE(AT_CIPSEND)
			ENUM_CHIP_TYPE_CASE(AT_CIPCLOSE)
    }
    return "无此命令";
}

/**************************************************************************************************
* 名    称：  RunResult Tcp_PDP_Init( void )
* 功能说明：  初始化TCP链路
* 出口参数：   
*            @param1 runResult  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
RunResult Tcp_PDP_Init( void )
{
	  RunResult runResult = TIMEOUT ; 
	  uint8_t *tcpLocalIp = portMalloc(MAX_IP_LEN*sizeof(uint8_t)) ;
	
	UARTx_SendData(EC20_UART, "AT+MDISCONNECT\r\n", strlen("AT+MDISCONNECT\r\n"))	 ;	 //DMA发送 模块先关闭MQTT连接
	Delay_Ms_StopScheduler(200);//
	UARTx_SendData(EC20_UART, "AT+MIPCLOSE\r\n", strlen("AT+MIPCLOSE\r\n"))	 ;	 //DMA发送 关闭TCP链接
	Delay_Ms_StopScheduler(200);//	Wait_For_Nms(1000);	
		
	 EC20_SendTcpCmd(AT_CIPMUX, NULL,TCP_CONTEXTID);//多IP链接 0 单路 1 多路
	 EC20_SendTcpCmd(AT_CIPQSEND,NULL,TCP_CONTEXTID);//设置为快发模式 1（ 推荐使用这种模式）
	
	  EC20_SendTcpCmd(AT_CGATT, NULL);  //MT附着GPRS 0 分离 1 附着
		EC20_SendTcpCmd(AT_CIPSHUT, NULL);  //关闭移动场景 
		Delay_Ms_StopScheduler(2000);//强制等待2秒
		EC20_SendTcpCmd(AT_CSTT, NULL, APN ) ;	//配置接入点APN
		EC20_SendTcpCmd(AT_CIICR, NULL) ;                //激活移动场景

		runResult = EC20_SendTcpCmd(AT_CIFSR, NULL) ;//查询本地IPtrueOffset
			if( RUNOK == runResult )
			{	
			CopyValues(tcpLocalIp, (uint8_t*)&ec20AtBuf[sTcpCmd[AT_CIFSR].trueOffset-2],'\r', MAX_IP_LEN) ;//绝对偏移地址 12
			AppLogPrintf("TCP本地IP：%s", tcpLocalIp) ;
			}
//if(W433Hard.Head.DeviceAlarmMode != Sim_only)//2024/9/2 增加在仅SIM模式下 不连接www.szjcdz.top网址
//{			
		runResult = EC20_SendTcpCmd(AT_CIPSTART, NULL, sChannal0.serverIP, sChannal0.serverPort) ;
	  if( RUNOK == runResult )
			{
				Query_Socket(sChannal0.connectId) ;
			  AppLogPrintf( "TCP Server连接成功:%s:%d ！", sChannal0.serverIP, sChannal0.serverPort) ;
				StatusBarPrintf("Server连接成功");
			}
		else 	
			{ 
				runResult = RUNERR;
				AppLogPrintf("TCP初始化失败") ;StatusBarPrintf("TCP初始化失败");
			}			
//}			
//	  runResult = EC20_SendTcpCmd(AT_CIPSTATUS, NULL) ; //查询下链接状态
	
//		if( RUNOK == runResult )	
//			{
//				AppLogPrintf("TCP本地IP：%s", tcpLocalIp) ;
//			}
//		else
//			{
//				AppLogPrintf("TCP初始化失败") ;StatusBarPrintf("TCP初始化失败");
//			}			

	  portFree(tcpLocalIp) ;
		return(runResult) ;
}

/**************************************************************************************************
* 名    称：  RunResult Tcp_Channal_Init( ChannalP_s *channal )
* 功能说明：  TCP连接通道初始化
* 入口参数：   
*            @param1 *channal：ChannalP_s结构体变量，存放TCP连接通道相关参数
* 出口参数：   
*            @param1 runResult  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
RunResult Tcp_Channal_Init( ChannalP_s *channal )
{
	  RunResult runResult = TIMEOUT ; 

//	runResult = Open_Socket(channal->connectId, channal->serverIP, channal->serverPort, channal->localPort) ;
	runResult = EC20_SendTcpCmd(AT_CIPSTART, NULL, channal->connectId, channal->serverIP, channal->serverPort, channal->localPort) ;
	  if( RUNOK == runResult )
			Query_Socket(channal->connectId) ;
		return(runResult) ;

//    runResult = Close_Socket(channal->connectId) ;
//	  if( runResult != RUNOK )                          //关闭SOCKET失败直接返回失败状态
//		return(runResult) ;	
//	  runResult = Open_Socket(channal->connectId, channal->serverIP, channal->serverPort, channal->localPort) ;
//	  if( RUNOK == runResult )
//			Query_Socket(channal->connectId) ;
//		return(runResult) ;
}

/**************************************************************************************************
* 名    称：  RunResult Tcp_SendData(ChannalP_s *channal, uint8_t *sendBuf, uint16_t sendLen)
* 功能说明：  通过连接通道channal将长度为sendLen的sendBuf缓冲区数据发送到服务器
* 入口参数：   
*            @param1 *channal：ChannalP_s结构体变量，存放TCP连接通道相关参数
*            @param2 *sendBuf：发送数据缓冲区地址
*            @param3  sendLen：发送数据的长度
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
RunResult Tcp_SendData(ChannalP_s *channal, uint8_t *sendBuf, uint16_t sendLen) 
{
	  if( sendLen <=0 )
		  {
			  return (RUNOK) ;
			}
	  uint8_t times = 0, enableSendData = 0 ;
	  uint16_t revTimes = 0 ;
  	RunResult runStatus = TIMEOUT ;
		Ec20AtBufReset() ;
    UARTx_SendString(EC20_UART, (uint8_t*)sTcpCmd[AT_CIPSEND].cmdStr, channal->connectId, sendLen );	
		while( times++ < 70 )   
			{
				Delay_Ms_StopScheduler(20);//Wait_For_Nms(20) ;
			  if( NULL != strchr((const char*)ec20AtBuf, '>') )
				  {
						enableSendData = 1 ;
						break ;
				  }				 
			}
			
			if( enableSendData == 1 ) //已经成功收到‘>’，串口可以发送TCP需要上传的数据
		  { 
				UARTx_SendData(EC20_UART, (char*)sendBuf, sendLen) ;
				while( revTimes++ < sTcpCmd[TCPSENDBUF].timeout )
					{ 
						Delay_Ms_StopScheduler(50);//Wait_For_Nms(100);
            sTcpCmd[TCPSENDBUF].trueOffset = kmp(ec20AtBuf, sTcpCmd[TCPSENDBUF].trueStr) ;					
						if( sTcpCmd[TCPSENDBUF].trueOffset >= 0)
							{   
									runStatus = RUNOK ; 
                  DebugLogPrintf("%s %s", TcpCmdNumToString(TCPSENDBUF), RunResultToString(runStatus)) ;
                  break ;								
							}
						else if( kmp(ec20AtBuf, sTcpCmd[TCPSENDBUF].falseStr) >= 0)
							{
									runStatus = RUNERR ;	
                  ErrorLogPrintf("%s %s %s", TcpCmdNumToString(TCPSENDBUF), RunResultToString(runStatus), ec20AtBuf ) ;	
                  break ;								
							}
					}
				//EC20_SendTcpCmd( TCPSENDBUF, NULL )  ;
			}
		else
		  {
				ErrorLogPrintf("%s %s %s %s", TcpCmdNumToString(AT_CIPSEND), RunResultToString(runStatus), "未收到\">\"", ec20AtBuf ) ;
			}
		return (runStatus) ;
}
/**************************************************************************************************
* 名    称：  RunResult EC20_SendTcpCmd( uint8_t cmdNum, char *format,... ) 
* 功能说明：  MCU串口向EC20发送Tcp相关命令
* 入口参数：   
*            @param1 cmdNum  EC20_CMD_DATA_s中cmdNum成员命令编号
*            @param2 char *format,...  可变参变量
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
RunResult EC20_SendTcpCmd( uint8_t cmdNum, char *format,... ) 
{
   uint8_t revTimes = 0 ;
	  RunResult  status = TIMEOUT ;
		uint8_t retryTimes = sTcpCmd[cmdNum].rtyNum ; 
    char *cmdPack = NULL ;
		format = sTcpCmd[cmdNum].cmdStr ;	
	  cmdPack = portMalloc(TCP_CMDPACK_LEN*sizeof(uint8_t)) ;		
    va_list ap;  
    va_start (ap, format);     
    int outLen = vsnprintf(cmdPack, TCP_CMDPACK_LEN, (const char*)format, ap); 	//vsprintf (temp, cmd, ap);  			//到此为止，所有的参数情况已经汇总到temp了 
		if((outLen<=0)||( outLen > TCP_CMDPACK_LEN)) 	 
		  {
			  ErrorLogPrintf("Tcp cmdPack 溢出！--增加TCP_CMDPACK_LEN数值。") ;
				status = RUNERR ;
				goto tcpCmdOut ;
			}
		while(retryTimes--)
			{
			  Ec20AtBufReset() ;
				revTimes = 0 ;
        UARTx_SendData(EC20_UART, cmdPack, outLen )	 ;	 //DMA发送					
				while( revTimes++ < sTcpCmd[cmdNum].timeout )
					{ 
						Delay_Ms_StopScheduler(200);//Wait_For_Nms(100) ;
            sTcpCmd[cmdNum].trueOffset = kmp(ec20AtBuf, sTcpCmd[cmdNum].trueStr) ;					
						if( sTcpCmd[cmdNum].trueOffset >= 0)
							{   
									status = RUNOK ; 
								  goto tcpCmdOut ;
							}
						else if( kmp(ec20AtBuf, sTcpCmd[cmdNum].falseStr) >= 0)
							{
									status = RUNERR ;	
								  goto tcpCmdOut ;
							}
					}
				Delay_Ms_StopScheduler(1000);//Wait_For_Nms( 1000 ) ;		
			}	
tcpCmdOut:
		portFree(cmdPack) ;
		va_end(ap) ;
	  DebugLogPrintf("%s %s", TcpCmdNumToString((enum eTcpCmdNum)cmdNum), RunResultToString(status) ) ;
    return (status) ;		
}

/**************************************************************************************************
* 名    称：  RunResult Open_Socket(uint8_t connectId, uint8_t *serverIp, uint16_t serverPortNum,  uint16_t localPortNum ) 
* 功能说明：  打开TCP Socket
* 入口参数：   
*            @param1 connectId：TCP连接通道ID
*            @param2 *serverIp：存放服务器IP的指针
*            @param3 serverPortNum：服务器端口号
*            @param4 localPortNum：模块本地端口号
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//RunResult Open_Socket(uint8_t connectId, uint8_t *serverIp, uint16_t serverPortNum,  uint16_t localPortNum ) 
//{
//	  RunResult runStatus = TIMEOUT ;
//		runStatus = EC20_SendTcpCmd(AT_CIPSTART, NULL, connectId, serverIp, serverPortNum, localPortNum ) ;
//		if( RUNOK == runStatus )
//			{			
//			 if(  (ec20AtBuf[sTcpCmd[AT_CIPSTART].trueOffset+9] == (0X30+connectId)) && 
//					  (ec20AtBuf[sTcpCmd[AT_CIPSTART].trueOffset+11] == '0')
//				  )
//					{
//						 runStatus = RUNOK ;
//					}	
//       else
//			    {
//					  runStatus = RUNERR ; 
//					}				 
//			} 
//		return (runStatus) ;
//}

/**************************************************************************************************
* 名    称：  RunResult Close_Socket(uint8_t connectId) 
* 功能说明：  关闭TCP Socket
* 入口参数：   
*            @param1 connectId：TCP连接通道ID
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
RunResult Close_Socket(uint8_t connectId)
{
    RunResult runStatus = TIMEOUT ;
		runStatus = EC20_SendTcpCmd(AT_CIPCLOSE, NULL, connectId ) ;  
		return (runStatus) ;
}

/**************************************************************************************************
* 名    称：  RunResult Query_Socket(uint8_t connectId)
* 功能说明：  查询TCP Socket状态
* 入口参数：   
*            @param1 connectId：TCP连接通道ID
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
RunResult Query_Socket(uint8_t connectId)
{
    RunResult runStatus = TIMEOUT ;
		runStatus = EC20_SendTcpCmd(AT_CIPSTATUS, NULL, connectId ) ;
		if( RUNOK == runStatus )
			{
				uint8_t serverPort[5] = {0}  ;
				ChannalP_s *sReturnP ;
				sReturnP = portMalloc(sizeof(ChannalP_s)) ;
				sReturnP->connectId = ec20AtBuf[sTcpCmd[AT_CIPSTATUS].trueOffset+33] ;
				u8 offset = CopyValues((uint8_t*)&sReturnP->serverIP, (uint8_t*)&ec20AtBuf[sTcpCmd[AT_CIPSTATUS].trueOffset+44], '"', MAX_IP_LEN) ; 
				CopyValues(serverPort, (uint8_t*)&ec20AtBuf[sTcpCmd[AT_CIPSTATUS].trueOffset+47+offset], '"', 5) ; //43+strlen((const char*)sReturnP->serverIP)
//				DebugLogPrintf("TCP Channal：%c 。 服务器IP：%s。 服务器端口：%s", sReturnP->connectId, sReturnP->serverIP, serverPort) ;
	DebugLogPrintf("TCP Channal：%c 。 服务器IP：%s。 服务器端口：%s", sReturnP->connectId, sReturnP->serverIP, serverPort) ;
		    portFree(sReturnP) ; 						
			} 
		return (runStatus) ;
}

/**************************************************************************************************
* 名    称：  TcpUrcType TcpUrcHandle( char *recvBuf, uint16_t recvLen )
* 功能说明：  TCP/IP协议中模块返回的一写URC分类处理
* 入口参数：   
*            @param1 recvBuf  接收的数据
*            @param2 recvLen  接收的数据长度
* 出口参数：   
*            @param  TcpUrcType  TcpUrcType枚举类型变量，返回URC类型
**************************************************************************************************/
TcpUrcType TcpUrcHandle( char *recvBuf, uint16_t recvLen )
{
	TcpUrcType urcType = UNKNOWM ;
  if( kmp(recvBuf, "CLOSED") > 0 )//"+QIURC: \"closed\""
	  {
		  urcType = CLOSED ;
		}
	else if( kmp(recvBuf, "+PDP DEACT") > 0 )//"+QIURC: \"pdpdeact\""
	  {
		  urcType = PDPDEACT ;
		}
	else if( kmp(recvBuf, "+QIURC: \"incoming\"") > 0 )
	  {
		  urcType = INCOMING_FULL ;
		}
	else if( kmp(recvBuf, "+QIURC: \"incoming full\"") > 0 )
	  {
		  urcType = INCOMING_CONT ;
		}
	else 
	  {
		  urcType = UNKNOWM ;
		}
//if(RUNOK != SYNC_LBS_Time()){SAPBR_Init();SYNC_LBS_Time();}        		//基站定位LBS信息和时间			
	return urcType ;
}












