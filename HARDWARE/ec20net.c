#include "ec20net.h" 
//#include "syslib.h"
#include "main.h"
/********************************************************************************
  * @file    ec20.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   提供Quectel模块EC20公共硬件驱动程序（激活去激活ContextID链路、EC20串口回调函数）
  ******************************************************************************
  * @attention
  * 约定基本名词如下：
	*      contextID:链路ID  
  * EC20模块链路ID范围1~16，每一个链路ID都会对应一个本地IP； 通道ID范围0~11。
  * 每一个链路ID可以单独相同/不同通信协议（TCP/IP、UDP、HTTP、FTP）。
	* @use：
	*  先调用EC20_Uart_Init()初始化通讯串口，接着调用ActivePDP()激活相关链路，最后通过链路实现
	*  TCP\HTTTP\FTP通信
*******************************************************************************/

/*****************************************
*ec20 NET 驱动内部使用常变量
****************************************/
//#define  NET_CMDPACK_LEN        128       //EC20 NET相关命令字符串的最大长度
//#define  APN                    "CMNET"   //APN，使用的SIM运营商不同对应的APN可能不通

/********************************************************
ec20模块PDP连接相关AT指令处理
*********************************************************/
//volatile EC20_CMD_DATA_s sNetCmd[8]=
//{
//  cmdNum            cmdStr,                          timeout(100ms), trueStr,   trueOffset falseStr 	revResult 	rtyNum
//	{AT_CSTT,      "AT+CSTT=\"%s\"\r\n",            					8,        "OK" ,        -1,    	"ERROR",  TIMEOUT,    5  			},//配置接入点APN
//	{AT_CIICR,   	 "AT+CIICR\r\n",                          	15,       "OK" , 				-1,    	"ERROR",  TIMEOUT,    3       },//激活移动场景
//	{AT_CIFSR,   	 "AT+CIFSR\r\n",                          	15,       "." , 				12,    	"ERROR",  TIMEOUT,    3       },//查询本地IP
///*扩展指令*/
//	{AT_CGATT,     "AT+CGATT?\r\n",                           8,        "+CGATT: 1" , -1,    "ERROR",  TIMEOUT,    5       },//将MT附着GPRS 0 分离 1 附着
//	{AT_CIPMUX,    "AT+CIPMUX=%d\r\n",            				8,        "OK" ,        -1,    	"ERROR",  TIMEOUT,    5  			},//多IP链接 0 单路 1 多路
//  {AT_CGPADDR,       "AT+CGPADDR\r\n", 												5,          "OK",      -1,      "ERROR",  TIMEOUT,   2       }, //显示 PDP 地址 
//	{AT_CIPQSEND,   "AT+CIPQSEND=%d\r\n", 												5, 								"OK" ,						 -1,   		 "ERROR",   TIMEOUT,   3  },//设置为快发模式（ 推荐使用这种模式）
//	{AT_CIPSHUT,    "AT+CIPSHUT\r\n",                          	5,       						"SHUT OK" , 					-1,    "ERROR",  	TIMEOUT,    3  },//关闭移动场景 强制等待2秒

//	sim900a_send_cmd("AT+CIPSTART=\"TCP\",\"www.szjcd.top\",9001\r\n","OK",100);  //39.108.91.128 CIP OPEN
//	{ATSOCK,        "AT+CGSOCKCONT=1,\"IP\",\"CMNET\"\r\n",    8,        "OK" ,   -1,    "ERROR",  TIMEOUT,    3       },
//	{ATMQTTSTAR,       "AT+CMQTTSTART\r\n",               15,        "OK" ,     -1,    "ERROR",  TIMEOUT,    3       },
//	{ATGSN,     "AT+GSN\r\n",                            15,        "OK" ,    -1,    "ERROR",  TIMEOUT,    3       },
//	{ATACCQ,     "AT+CMQTTACCQ=0,\"%s\"\r\n",            15,  "OK" ,        -1,    "ERROR",  TIMEOUT,    2       },
//	{ATCONNECT,   "AT+CMQTTCONNECT=0,\"tcp://%s\",60,1,\"%s\",\"%s\"\r\n",   15,  "OK" ,        -1,    "ERROR",  TIMEOUT,    2       },
//	{QUERYNETINFO,   "AT+QNWINFO\r\n",                          3,        "+QNWINFO:" , -1,    "ERROR",  TIMEOUT,    1       },
//} ;  //EC20模块NET相关指令的EC20_CMD_DATA_s结构体类型参数("AT+CMQTTCONNECT=0,\"tcp://59.110.116.62:9100\",60,1,\"icemonitor\",\"123456\"\r\n","OK",100)

/**************************************************************************************************
* 名    称：  static const char *NetCmdNumToString(enum eNetCmdNum result)
* 功能说明：  输出枚举成员名的字符串指针。
* 入口参数：  eNetCmdNum类型的枚举
* 出口参数：  为枚举的成员名字符串指针 
**************************************************************************************************/
//static inline const char *NetCmdNumToString(enum eNetCmdNum result)
//{
//    switch (result)
//    {
//			 ENUM_CHIP_TYPE_CASE(AT_CSTT)
//			 ENUM_CHIP_TYPE_CASE(AT_CIICR)
//			 ENUM_CHIP_TYPE_CASE(AT_CIFSR)
//			 ENUM_CHIP_TYPE_CASE(AT_CGATT)
//			 ENUM_CHIP_TYPE_CASE(AT_CIPMUX)
//			 ENUM_CHIP_TYPE_CASE(AT_CGPADDR)
//			 ENUM_CHIP_TYPE_CASE(AT_CIPQSEND)
//			ENUM_CHIP_TYPE_CASE(AT_CIPSHUT)
//    }
//		ErrorLogPrintf("EC20 无效eNetCmdNum!") ;
//    return "无此命令";
//}

/**************************************************************************************************
* 名    称：  RunResult EC20_SendNetCmd( uint8_t cmdNum, char *format,... ) 
* 功能说明：  MCU串口向EC20发送PDP相关命令
* 入口参数：   
*            @param1 cmdNum  EC20_CMD_DATA_s中cmdNum成员命令编号
*            @param2 char *format,...  可变参变量
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//RunResult EC20_SendNetCmd( uint8_t cmdNum, char *format,... ) 
//{
//    uint8_t revTimes = 0 ;
//	  RunResult  status = TIMEOUT ;
//		uint8_t retryTimes = sNetCmd[cmdNum].rtyNum ; 
//    char *cmdPack = NULL ;
//		format = sNetCmd[cmdNum].cmdStr ;	
//	  cmdPack = portMalloc(NET_CMDPACK_LEN*sizeof(uint8_t)) ;			
//    va_list ap;  
//    va_start (ap, format);     
//    int outLen = vsnprintf(cmdPack, NET_CMDPACK_LEN, (const char*)format, ap); 	//vsprintf (temp, cmd, ap);  			//到此为止，所有的参数情况已经汇总到temp了 
//		if((outLen<=0)||( outLen > NET_CMDPACK_LEN)) 	//vsprintf (temp, cmd, ap);  			//到此为止，所有的参数情况已经汇总到temp了 
//		  {
//				ErrorLogPrintf("%s,%d:cmdPack spillover！",__FILE__, __LINE__) ; //增加NET_CMDPACK_LEN数值
//				status = RUNERR ; 
//        goto netCmdOut ;
//			}
////AppLogPrintf("%s", cmdPack) ;
////	AppLogPrintf("%s", ec20AtBuf) ;		
//		
//			while(retryTimes--)
//			{
//			  Ec20AtBufReset() ;
//				revTimes = 0 ;
//				//UARTx_Printf(EC20_UART, (uint8_t *)"%s", (uint8_t *)cmdPack);	
//        UARTx_SendData(EC20_UART, cmdPack,  outLen);	 //DMA发送
////				UARTx_SendString(EC20_UART, (uint8_t*)cmdPack,  NULL)	 ;
//				while( revTimes++ < sNetCmd[cmdNum].timeout )
//					{ 
//						Delay_Ms_StopScheduler(200);//Wait_For_Nms(200);
////            sNetCmd[cmdNum].trueOffset = kmp(ec20AtBuf, sNetCmd[cmdNum].trueStr) ;					
//						if( kmp(ec20AtBuf, sNetCmd[cmdNum].trueStr) >= 0)
//							{   
//									status = RUNOK ;
//                  goto netCmdOut ;
//							}
//						else if( kmp(ec20AtBuf, sNetCmd[cmdNum].falseStr) >= 0)
//							{
//									status = RUNERR ;
//                  goto netCmdOut ;
//							}
//					}
//				Delay_Ms_StopScheduler(200);//Wait_For_Nms( 200 ) ;		
//			}	
//netCmdOut:portFree(cmdPack) ;
//		va_end (ap);
//		DebugLogPrintf("%s %s %d %d",  NetCmdNumToString((enum eNetCmdNum)cmdNum), RunResultToString(status) ,retryTimes,revTimes) ;
//    return (status) ;		
//}

/**************************************************************************************************
* 名    称：  RunResult Config_Context( uint8_t contextId )
* 功能说明：  对contextID进行配置( <APN>, <username>, <password>等信息)
* 入口参数：   
*            @param1 contextId:  取值范围1~16
* 出口参数：   
*            @param1 runStatus  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//RunResult Config_CsttAPN(void)
//{
//		RunResult runStatus = EC20_SendNetCmd(AT_CSTT, NULL, APN ) ;    
//		return (runStatus) ;
//}

/**************************************************************************************************
* 名    称：  RunResult Act_Context( uint8_t contextId )
* 功能说明：  激活contextID
* 入口参数：   
*            @param1 contextId:  取值范围1~16
* 出口参数：   
*            @param1 runStatus  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//RunResult Act_CIICR(void)
//{
//		RunResult runStatus = EC20_SendNetCmd(AT_CIICR, NULL) ;
//		return (runStatus) ;
//}

/**************************************************************************************************
* 名    称：  RunResult Deact_Context( uint8_t contextId )
* 功能说明：  去激活contextID
* 入口参数：   
*            @param1 contextId:  取值范围1~16
* 出口参数：   
*            @param1 runStatus  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//RunResult Deact_Context( uint8_t contextId )
//{
//		RunResult runStatus = EC20_SendNetCmd(DEACTCONTEXT, NULL, contextId) ;    
//		return (runStatus) ;
//}

/**************************************************************************************************
* 名    称：  RunResult Query_Context( uint8_t contextId, uint8_t *localIp )
* 功能说明：  查询contextID激活状态，并获取对应本地IP地址
* 入口参数：   
*            @param1 contextId:   取值范围1~16
*            @param2 *localIp:    保存本地IP数据的地址
* 出口参数：   
*            @param1 runStatus  RunResult枚举类型变量，返回函数运行结果
* 注    意：目前只支持contextID小于等于3链路的本地IP获取
**************************************************************************************************/
//RunResult Query_CIFSR(uint8_t *localIp )
//{
//		RunResult runStatus = EC20_SendNetCmd(AT_CIFSR, NULL) ;//QUERYCONTEXT
//			if( RUNOK == runStatus )
//			{	
//			CopyValues(localIp, (uint8_t*)&ec20AtBuf[sNetCmd[AT_CIFSR].trueOffset], '"', MAX_IP_LEN) ;
//				return RUNOK;
//			}
//return RUNERR	;
				
//		if( RUNOK == runStatus )
//			{
//				switch(contextId)
//					{
//					  case 1:
//							 	sNetCmd[ATCGPADD].trueOffset = kmp(ec20AtBuf, "+QIACT: 1" );//QUERYCONTEXT
//                break ;
//						case 2:
//							 	sNetCmd[ATCGPADD].trueOffset = kmp(ec20AtBuf, "+QIACT: 2" );
//                break ;
//						case 3:
//							 	sNetCmd[ATCGPADD].trueOffset = kmp(ec20AtBuf, "+QIACT: 3" );
//                break ;	
//            default:
//							  sNetCmd[ATCGPADD].trueOffset = kmp(ec20AtBuf, "+QIACT: 1" );
//                break ;      						
//					}
//				if( (sNetCmd[ATCGPADD].trueOffset>=0) && 
//				  	(ec20AtBuf[sNetCmd[ATCGPADD].trueOffset+8] == (0x30+contextId))&&
//					  (ec20AtBuf[sNetCmd[ATCGPADD].trueOffset+10] == '1')   //偏移地址10指示context状态，Activated：1；Deactivated：0
//				  )
//				  {
//					  runStatus = RUNOK ;
//						CopyValues(localIp, (uint8_t*)&ec20AtBuf[sNetCmd[ATCGPADD].trueOffset+15], '"', MAX_IP_LEN) ;
//						//AppLogPrintf("ContextId %c 本地IP：%s",contextId, localIp) ;
//					}
//        else
//				  {
//					  runStatus = RUNERR ;
//					}					
//			}   
//		return (runStatus) ;
//}
/**************************************************************************************************
* 名    称： RunResult EC20_QueryCsServiceStatus(void)
* 功能说明： 注册CS网络
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//RunResult EC20_QueryCsServiceStatus(void)
//{
//	 RunResult runStatus = TIMEOUT ;
//	 int checkCsTimes = 45 ;  //45*2S内没有连接到CS Server则表示连接失败《TCP(IP)_AT_Commands》手册P8 要求90S等待 <stat>equals to 1 or 5
//	 EC20_SendNetCmd(AT_CSTT, NULL ) ;
//	 while( checkCsTimes-- )
//	   {
//			  Delay_Ms_StopScheduler(200);//Wait_For_Nms(2000) ;
//				runStatus = EC20_SendNetCmd(AT_CREG, NULL ) ;
////				if( RUNOK == runStatus )
////					{
//						 if( (sNetCmd[AT_CREG].trueOffset > 0)&&
//							   ((ec20AtBuf[sNetCmd[AT_CREG].trueOffset+9] == 0x31)||(ec20AtBuf[sNetCmd[AT_CREG].trueOffset+9] == 0x35) ))
//							 { 
//									runStatus =RUNOK ;
//								  break;	
//							 }
//						 else
//						   {
//							    runStatus =RUNERR ;
//							 }							 
//					}
//		 }
//	 return (runStatus) ;			 
//}

/**************************************************************************************************
* 名    称： RunResult EC20_QueryPsServiceStatus(void)
* 功能说明： 注册PS网络
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//RunResult EC20_QueryPsServiceStatus(void)
//{
//	 RunResult runResult = TIMEOUT ;
//	 int checkPsTimes = 30 ;         //30*2S内没有连接到CS Server则表示连接失败 《TCP(IP)_AT_Commands》手册P8 要求60S等待 <stat>equals to 1 or 5
//	 EC20_SendNetCmd(SETPS, NULL ) ; //Enable network registration unsolicited result code with location information+CREG: <stat>[,<lac>,<ci>[,<Act>]]
//	 while( checkPsTimes-- )
//	   {
//			  Delay_Ms_StopScheduler(200);//Wait_For_Nms(2000) ;
//				runResult = EC20_SendNetCmd(AT_CEREG, NULL ) ; //查询模块连接PS server的状态
//				if( RUNOK == runResult )
//					 {
//						 if(
//							  (sNetCmd[AT_CEREG].trueOffset > 0)&&
//							  ((ec20AtBuf[sNetCmd[AT_CEREG].trueOffset+10] == 0x31)||(ec20AtBuf[sNetCmd[AT_CEREG].trueOffset+10] == 0x35)) 
//						   )
//							 {
//								 runResult =RUNOK ;
//                 break ;								 
//							 }
//						 else
//						   {
//							   runResult =RUNERR ;
//							 }           							 
//					 }
//		 }
//	 return (runResult) ;
//}
/**************************************************************************************************
* 名    称： RunResult EC20_Query_SimIccid(char *simICCID)  
* 功能说明： 查询SIM卡的ICCID编号
* 入口参数：   
*            @param1 *simICCID：  存放ICCID的内存地址
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//RunResult EC20_Query_GSN(char *simICCID)
//{
////	 char *start ;
//	 memset(simICCID, 0, SIM_ICCID_LEN);
//	 RunResult  runStatus = EC20_SendNetCmd(ATGSN, NULL ) ;
////AppLogPrintf("%s", ec20AtBuf) ;CopyValues((uint8_t*)simICCID, (uint8_t*)ec20AtBuf,  'O', SIM_ICCID_LEN) ;
////	AppLogPrintf("%s", simICCID) ;
//	 if( RUNOK == runStatus )
//		 {
//			 CopyValues((uint8_t*)simICCID, (uint8_t*)&ec20AtBuf[2],  0x0D, SIM_ICCID_LEN) ;			
//		 }   
//	 return (runStatus) ;		 
//}
///**************************************************************************************************
//* 名    称： RunResult EC20_Query_SimIccid(char *simICCID)  
//* 功能说明： 查询SIM卡的ICCID编号
//* 入口参数：   
//*            @param1 *simICCID：  存放ICCID的内存地址
//* 出口参数：   
//*            @param1 status  RunResult枚举类型变量，返回函数运行结果
//**************************************************************************************************/
//RunResult EC20_Query_SimIccid(char *simICCID)
//{
//	 char *start ;
//	 memset(simICCID, 0, SIM_ICCID_LEN);
//	 RunResult  runStatus = EC20_SendNetCmd(AT_ICCID, NULL ) ;
//	 if( RUNOK == runStatus )
//		 {
//			 CopyValues((uint8_t*)simICCID, (uint8_t*)&ec20AtBuf[(sNetCmd[AT_ICCID].trueOffset+8)],  0x0D, SIM_ICCID_LEN) ;			
//		 }   
//	 return (runStatus) ;		 
//}

/**************************************************************************************************
* 名    称： RunResult EC20_Query_CSQ(char *csq)
* 功能说明： 查询网络CSQ（信号质量）
* 入口参数：   
*            @param1 *csq  存放csq的内存地址
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//RunResult EC20_Query_CSQ(uint8_t *csq)
//{
////	uint8_t i;
////	i=sNetCmd[ATCSQ].trueOffset;
//	 memset(csq, 0, CSQ_LEN+1);
//	 RunResult runResult = EC20_SendNetCmd(AT_CSQ, NULL ) ;
//	
//	 if( RUNOK == runResult )
//		 {
//			 CopyValues((uint8_t*)csq, (uint8_t*)&ec20AtBuf[sNetCmd[AT_CSQ].trueOffset+6],  ',', CSQ_LEN) ;			
//		 } 		     
//	 return (runResult) ;		 
//}

/**************************************************************************************************
* 名    称： RunResult EC20_Net_Reg(char *errInfo, uint8_t errLen)
* 功能说明： 模块注册到CS\PS服务器
* 入口参数：   
*            @param1 *errInfo  存放错误信息
*            @param2 errLen    错误信息的长度
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//RunResult EC20_Net_Reg(char *errInfo, uint8_t errLen)
//{
//	  RunResult runResult = TIMEOUT ;
//    runResult = EC20_Query_SimIccid(sEc20Param.simICCID) ;
//	  if( RUNOK !=  runResult ) 
//		  {
//				snprintf(errInfo, errLen, "请插入SIM卡") ;
//			  ErrorLogPrintf("未检测到SIM卡") ;
//				return (RUNERR) ;
//			}
//	  AppLogPrintf("Sim卡ICCID：%.*s", SIM_ICCID_LEN, sEc20Param.simICCID) ;
//	
//	  runResult = EC20_QueryCsServiceStatus() ;
//	  if( RUNOK !=  runResult ) 
//		  {
//				snprintf(errInfo, errLen, "SIM卡欠费") ;
//			  ErrorLogPrintf("SIM欠费无法注册上网络") ;
//				return (RUNERR) ;
//			}
//	  EC20_QueryPsServiceStatus() ;
//		AppLogPrintf("网络信息：%s", EC20_Query_NetInfo()) ;	//AppLogPrintf
//		EC20_Query_CSQ(sEc20Param.csq) ;
//		snprintf(errInfo, errLen, "信号质量:%s", sEc20Param.csq) ;
//		AppLogPrintf("LTE CSQ：%.*s", CSQ_LEN, sEc20Param.csq) ;
//		return (runResult) ;
//}
/**************************************************************************************************
* 名    称： RunResult EC20_Net_Reg(char *errInfo, uint8_t errLen)
* 功能说明： 模块注册到CS\PS服务器
* 入口参数：   
*            @param1 *errInfo  存放错误信息
*            @param2 errLen    错误信息的长度
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/

//RunResult SIM7600init(void)	
//{
//	  RunResult runResult = TIMEOUT ;

//		runResult = EC20_SendNetCmd(ATCPIN, NULL ) ;
////	  runResult = EC20_SendNetCmd(ATCSQ, NULL ) ;sNetCmd[ATCSQ].trueOffset
//		runResult = EC20_Query_CSQ(sEc20Param.csq );
////AppLogPrintf("%s", sEc20Param.csq) ;		
//		runResult = EC20_SendNetCmd(ATCREG, NULL ) ;
//		runResult = EC20_SendNetCmd(ATCGREG, NULL ) ;
//	  runResult = EC20_SendNetCmd(ATCNMI, NULL ) ;
//		runResult = EC20_SendNetCmd(ATSOCK, NULL ) ;
//		runResult = EC20_SendNetCmd(ATCGPADD, NULL ) ;
// 
//		runResult = EC20_SendNetCmd(ATMQTTSTAR, NULL ) ;
//		runResult = EC20_Query_GSN(sEc20Param.simICCID) ;
//		runResult = EC20_SendNetCmd(ATACCQ, NULL,sEc20Param.simICCID) ;
////AppLogPrintf("%s", sEc20Param.simICCID) ;	
////		runResult = EC20_SendNetCmd(ATCONNECT, NULL,sChannal0.serverIP,sChannal0.MqttUser, sChannal0.MqttPass) ;
//	runResult = EC20_SendNetCmd(ATCONNECT, NULL,"59.110.116.62:9100","icemonitor", "123456") ;
//	
//		return (runResult) ;
//}
/**************************************************************************************************
* 名    称：  RunResult ActivePDP( uint8_t contextId, uint8_t *localIp)
* 功能说明：  配置PDP链路，激活PDP链路 并输出链路IP
* 入口参数：   
*            @param1 contextId:   取值范围1~16
*            @param2 *localIp:    保存本地IP数据的地址
* 出口参数：   
*            @param1 runStatus  RunResult枚举类型变量，返回函数运行结果
* 异常处理：
**************************************************************************************************/
//RunResult ActivePDP( uint8_t contextId, uint8_t *localIp)
//{ 	
//	  RunResult runResult = TIMEOUT ; 
//	  Config_CsttAPN() ;                //配置PDP VPN等信息
//	  runResult = Act_Context(contextId) ;       //激活PDP
//	  if( runResult == RUNOK ) 
//		  {
//			  Query_Context(contextId, localIp) ;    //激活成功->获取PDP的IP
//		  }
//		return (runResult) ;
//}

/**************************************************************************************************
* 名    称：  RunResult DeactivePDP( uint8_t contextId, uint8_t *localIp)
* 功能说明：  去激活PDP链路
* 入口参数：   
*            @param1 contextId:   取值范围1~16
* 出口参数：   
*            @param1 runStatus  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//RunResult DeactivePDP( uint8_t contextId)
//{ 	
//	  RunResult runResult = TIMEOUT ; 
//		runResult = Deact_Context(contextId) ;
//}













