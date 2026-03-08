
#include "main.h"
/********************************************************************************
  * @file    ec20module.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   提供Quectel模块EC20模块级驱动程序（控制IO、电源开关、回码、握手、CSQ、ICCID等）
  ******************************************************************************
  * @attention
	* 外部调用EC20_Module_Init()对模块进行初始化，此函数如果返回失败则说明MCU无法与EC20通过串口通信
	  可能是EC20供电、或者模块没有安装就位导致的。
  * 网络通信前模块需要先查询SIM是否就绪，接着注册到CS Service和PS service，
	* 最后才能配置相关通信协议实现通讯
*******************************************************************************/
extern void fullTohalf(uint16_t destSize, char *src);
/*****************************************
*ec20 module相关的常变量
****************************************/
ModuleParam_s     sEc20Param ;                     //定义存放EC20模块参数的结构体
volatile bool     httpDataMode = false ;           //串口模式：接收http数据模式
char              ec20AtBuf[EC20_ATBUF_LEN] ;      //EC20 AT命令回码存放缓冲区

FrameQueue_s      SIMQueue ;                      //环形帧缓冲区，存储模块返回的tcp相关数据
FrameQueue_s      MQTT_PublishQueue ;                      //环形帧缓冲区

extern char *Check_code;
/********************************************************
ec20模块Module指令相关处理
*********************************************************/
volatile EC20_CMD_DATA_s sModuleCmd[36]=
{
//  cmdNum        cmdStr,        timeout     trueStr       trueOffset  falseStr  revResult  rtyNum
  {AT_AT,   		"AT\r\n",         	10,       "OK" ,          -1,       "ERROR",  TIMEOUT,    3       },  //握手指令        
  {AT_ATF, 			"AT&F\r\n",       	3,        "OK" ,        	-1,       "ERROR",   TIMEOUT,    1       },  //所有参数恢复默认值
  {AT_IPR,    	"AT+IPR=115200\r\n",3,        "OK" ,          -1,       "ERROR",  TIMEOUT,    2       },  //初始化波特率
	{AT_IFC,  		"AT+IFC=2,2\r\n",   3,        "OK" ,          -1,       "ERROR",  TIMEOUT,    2       },  // 开启回件流控 SimRTX = 0;启用
	{AT_ATI, 			"ATI\r\n",     			3,        "OK" ,          -1,       "ERROR",  TIMEOUT,    2       },  //检查握手信号
	{AT_CSQ,     	"AT+CSQ\r\n",   		3,        "OK" ,        	-1,   	 	"ERROR", 	TIMEOUT,    2 			},	//检查信号质量
	{AT_COPS,     "AT+COPS?\r\n",   	8,        "OK" ,        	-1,   	 	"ERROR", 	TIMEOUT,    5  			},	//是否注册到网络
	{AT_CPIN,     "AT+CPIN?\r\n",   	50,        "OK" ,        	-1,   	 	"ERROR", 	TIMEOUT,    5  			},	//检查SIM卡
  {AT_CIMI,			"AT+CIMI\r\n",     10,       "OK" ,         -1,       "ERROR",  TIMEOUT,    2       },     //运营商检测
	{AT_CNMI,			"AT+CNMI=2,2,0,0,0\r\n",10,    "OK" ,         -1,       "ERROR",  TIMEOUT,    2       },	//新消息直接发给TE
	{AT_CPMS,			"AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n",10,"OK" ,  -1,       "ERROR",  TIMEOUT,    2       },	//
	{AT_CSCA,			"AT+CSCA?\r\n", 		10,       "+CSCA",          -1,       "ERROR",  TIMEOUT,    2       },	//查询短信中心号码
	{AT_TXT,			"AT+CMGF=1\r\n", 		10,       "OK" ,          -1,       "ERROR",  TIMEOUT,    2       },	//TXT
	{AT_CSCS,			"AT+CSCS=\"UCS2\"\r\n",10,    "OK" ,          -1,       "ERROR",  TIMEOUT,    2       },	//UCS2
	{AT_CSMP,			"AT+CSMP=17,167,0,8\r\n",10,  "OK" ,          -1,       "ERROR",  TIMEOUT,    2       },	//UCS2 24 直接显示在终端，25短消息存储到SIM中，仅在TEXT格式下用到
	{AT_CHUP,			"AT+CHUP\r\n", 			10,       "OK" ,          -1,       "ERROR",  TIMEOUT,    2       },	//拒绝呼入
	{AT_VOLTE,		"AT+SETVOLTE=1\r\n",10,       "OK" ,          -1,       "ERROR",  TIMEOUT,    2       },	// //  1 开启VOLTE
	{AT_AUDCH,		"AT+AUDCH=0,0\r\n", 10,       "OK" ,          -1,       "ERROR",  TIMEOUT,    2       },	//输出通道0 听筒 ,输入 MAIN MIC
	{AT_CLVL,			"AT+CLVL=80\r\n", 	10,       "OK" ,          -1,       "ERROR",  TIMEOUT,    2       },	//  1 开启VOLTE
	{AT_VTD,			"AT+VTD=6\r\n", 		10,       "OK" ,          -1,       "ERROR",  TIMEOUT,    2       },	//TONE 0~10
	{AT_CLCC,			"AT+CLCC\r\n", 			10,       "OK" ,          -1,       "ERROR",  TIMEOUT,    2       },	// 通话状态查询
	{AT_CTTSPRM,	"AT+CTTSPARAM=90,0,50,50,0\r\n",10,"OK" ,     -1,       "ERROR",  TIMEOUT,    2       },	//volume,mode,pitch,speed,channel
	{AT_PDU,			"AT+CMGF=0\r\n", 		10,       "OK" ,          -1,       "ERROR",  TIMEOUT,    2       },	//PDU模式

	{AT_ICCID,    "AT+ICCID\r\n",     5,        "+ICCID" ,    	-1,    		"ERROR",  TIMEOUT,    3       },//查询 SIM 卡 ICCID 号码
	{AT_CREG2,    "AT+CREG=2\r\n",    3,        "OK" ,        	-1,    		"ERROR",  TIMEOUT,    2       },//4G网络附着
	{AT_CREG,     "AT+CREG?\r\n",     8,        "OK" ,    			-1,    		"ERROR",  TIMEOUT,    3       },
	{AT_CGREG2,   "AT+CGREG=2\r\n",   3,        "OK" ,        	-1,    		"ERROR",  TIMEOUT,    2       },
	{AT_CGREG,    "AT+CEREG?\r\n",    8,        "OK" ,   				-1,   		"ERROR",  TIMEOUT,    3       },
	{AT_CPOWD,    "AT+CPOWD=1\r\n",  	5,   "POWERED DOWN", -1,    		"ERROR",  TIMEOUT,    2       },  //关机命令 
	{AT_RETSET,   "AT+RETSET\r\n",  	8,        "OK" ,        	-1,   	 	"ERROR", 	TIMEOUT,    5 			},	//重启并恢复出厂设置
  {AT_CFUN,   	"AT+CFUN=%d\r\n", 	5,        "OK",      			-1,      "ERROR",  TIMEOUT,   2       }, //0 进入飞行模式  1退出
  {AT_ATD,   		"ATD%s;\r\n", 			5,        "OK",      			-1,      "ERROR",  TIMEOUT,   1       }, //打电话指令
  {AT_ATH,   		"ATH\r\n", 					5,        "OK",      			-1,      "ERROR",  TIMEOUT,   2       }, //挂机
	{AT_CTTS,   	"AT+CTTS=1,\"%s\"\r\n",2000,   "+CTTS:0",      -1,      "ERROR",  TIMEOUT,   1       }, //
	{AT_CMGD,   	"AT+CMGD=1,4\r\n",	5,     		"OK",      			-1,      "ERROR",  TIMEOUT,   2       }, //清除SIM存储内容
	{AT_CSCASET,	"AT+CSCA=\"%s\",145\r\n",	10, "OK" ,          -1,      "ERROR",  TIMEOUT,    2       },	//设置短信中心号码
} ;   //EC20模块module相关指令的EC20_CMD_DATA_s结构体类型参数

/**************************************************************************************************
* 名    称：  static const char *ModuleCmdNumToString(enum eModuleCmdNum result)
* 功能说明：  输出枚举成员名的字符串指针。
* 入口参数：  eModuleCmdNum类型的枚举
* 出口参数：  为枚举的成员名字符串指针 
**************************************************************************************************/
static const char *ModuleCmdNumToString(enum eModuleCmdNum result)
{
    switch (result)
    {
			ENUM_CHIP_TYPE_CASE(AT_AT)
			ENUM_CHIP_TYPE_CASE(AT_ATF)
			ENUM_CHIP_TYPE_CASE(AT_IPR)
			ENUM_CHIP_TYPE_CASE(AT_IFC)
			ENUM_CHIP_TYPE_CASE(AT_ATI)
			ENUM_CHIP_TYPE_CASE(AT_CSQ)
			ENUM_CHIP_TYPE_CASE(AT_COPS)
			
			ENUM_CHIP_TYPE_CASE(AT_CPIN)
			ENUM_CHIP_TYPE_CASE(AT_CIMI)
			ENUM_CHIP_TYPE_CASE(AT_CNMI)
			ENUM_CHIP_TYPE_CASE(AT_CPMS)
			ENUM_CHIP_TYPE_CASE(AT_CSCA)			
			ENUM_CHIP_TYPE_CASE(AT_TXT)
			ENUM_CHIP_TYPE_CASE(AT_CSCS)
			
			ENUM_CHIP_TYPE_CASE(AT_CSMP)
			ENUM_CHIP_TYPE_CASE(AT_CHUP)
			ENUM_CHIP_TYPE_CASE(AT_VOLTE)
			ENUM_CHIP_TYPE_CASE(AT_AUDCH)
			ENUM_CHIP_TYPE_CASE(AT_CLVL)
			ENUM_CHIP_TYPE_CASE(AT_VTD)
			ENUM_CHIP_TYPE_CASE(AT_CLCC)
			
			ENUM_CHIP_TYPE_CASE(AT_CTTSPRM)
			ENUM_CHIP_TYPE_CASE(AT_PDU)
			ENUM_CHIP_TYPE_CASE(AT_ICCID)
			ENUM_CHIP_TYPE_CASE(AT_CREG2)
			ENUM_CHIP_TYPE_CASE(AT_CREG)			
			ENUM_CHIP_TYPE_CASE(AT_CGREG2)
			
			ENUM_CHIP_TYPE_CASE(AT_CGREG)			
			ENUM_CHIP_TYPE_CASE(AT_CPOWD)
			ENUM_CHIP_TYPE_CASE(AT_RETSET)
			ENUM_CHIP_TYPE_CASE(AT_CFUN)
			ENUM_CHIP_TYPE_CASE(AT_ATD)
			ENUM_CHIP_TYPE_CASE(AT_ATH)
			ENUM_CHIP_TYPE_CASE(AT_CTTS)
			ENUM_CHIP_TYPE_CASE(AT_CMGD)
			ENUM_CHIP_TYPE_CASE(AT_CSCASET)
    }
    return "无此命令";
}

/**************************************************************************************************
* 名    称：  void Ec20AtBufReset(void)
* 功能说明：   ec20AtBuf缓冲区初始化  
**************************************************************************************************/
void Ec20AtBufReset(void)    
{
  memset(ec20AtBuf, 0, EC20_ATBUF_LEN) ;
}  

/**************************************************************************************************
* 名    称：  void Ec20HttpBufReset(void)
* 功能说明：  ec20HttpBuf缓冲区初始化  
**************************************************************************************************/
//void Ec20HttpBufReset(void)    
//{
//  memset(ec20HttpBuf, 0, EC20_HTTPBUF_LEN) ;
//}

/**************************************************************************************************
* 名    称：  void Ec20ReceiveFrameCallback(void)
* 功能说明：  EC20模块——>MCU串口3接收空闲中断回调函数 
*             将EC20模块返回的数据分为三大类：1、http相关的数据；2、tcp相关的数据；3、模块URC相关的数据。
**************************************************************************************************/
void Ec20ReceiveFrameCallback(char *recvBuf, uint16_t recvLen)
{
//	 int pos = 0;
		//printf("\r\nEC20:%s", buf) ;
//char *buf = portMalloc(1024) ;
//uint16_t len = 0 ;
		
//	if( httpDataMode == true )                           /*串口接收模式为：接收HTTP数据*/
//			{		
//				strncat(ec20HttpBuf, recvBuf, recvLen) ;
////						SysLog("COM3 RxLen:%d; RxBuf:%s", len, uart3_Dma_Rx_Buf);  
//			}
//else 
	if((kmp(recvBuf, "+CMT:")>=0 )||((kmp(recvBuf, "+MSUB:")>=0 ) ))		  /*"+QIURC:"判断是否为TCP下行数据或者URC "+MSUB:"判断是否为MQTT下行数据或者URC*/
			{
//				pos = kmp(recvBuf, "+CMT:") ; 
//				if( pos>=0)                                           /*"+QIURC: \"recv\""判断是否为接收到TCP下行数据*/
//					{ 
						fullTohalf(recvLen, recvBuf);//全角转半角
						if( RW_OK != InsertQueueMemData(&SIMQueue, recvBuf, recvLen) )
						 {MQTT_Infor = 0;
								ErrorLogPrintf("%s,%d:SIMQueue 溢出！",__FILE__, __LINE__) ;
						 }						 
//					}
//				else 																									/*判断是模块返回的TCP URC*/
//					{
//						if( RW_OK != InsertQueueMemData(&sUrcQueue, recvBuf, recvLen) )
//						 {
//								ErrorLogPrintf("%s,%d:sUrcQueue spillover！",__FILE__, __LINE__) ;
//						 }								
//					}	
			}
//	else if(kmp(recvBuf, "+MSUB:")>=0 ) 		                    /*"+MSUB:"判断是否为MQTT下行数据或者URC*/
//			{
//				if( RW_OK != InsertQueueMemData(&SIMQueue, recvBuf, recvLen) )//MQTT下行数据也插入到此
//				 {
//					 MQTT_Infor = 1;
//						ErrorLogPrintf("%s,%d:MQTT SIMQueue spillover！",__FILE__, __LINE__) ;
//				 }						 
//			}		
		else                                                        //EC20模块AT命令回码
			{
				if( recvLen < (EC20_ATBUF_LEN-strlen((const char*)ec20AtBuf))) /*ec20AtBuf剩余长度足以存贮recvBuf*/
				 {
//						strcat((char*)ec20AtBuf, recvBuf) ;                 //将接收到的AT命名回码数据存储到ec20AtBuf中去						 
				 SysStrcat((char*)ec20AtBuf,recvLen, recvBuf);//带有全角转半角功能 自动检测数据
				 }
				else                                                    /*ec20AtBuf剩余长度不足以存贮recvBuf*/
				 {
						ErrorLogPrintf("%s,%d:ec20AtBuf spillover. ",__FILE__, __LINE__) ; //报错
						Ec20AtBufReset() ;                                            //先清空	ec20AtBuf
						strncpy((char*)ec20AtBuf, recvBuf, EC20_ATBUF_LEN) ;			   //再将recvBuf中的数据拷贝到ec20AtBuf中去				 
				 }
			}
//if(RW_OK == GetQueueMemData(&sTcpQueue, buf, &len))		
//AppLogPrintf("AtBuf %s", buf ) ;
//portFree(buf) ;			
}

/**************************************************************************************************
* 名    称： RunResult EC20_Module_Init( void )
* 功能说明： EC20模块初始化（IO初始化、供电、关闭回显、握手、查询SIM卡号、注册CS\PS网络、查询CSQ）
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
RunResult EC20_Module_Init( void )
{
	  RunResult runResult = TIMEOUT ; 
		PowerOn = 0;//EC20_POWON();
		AlrmRelay = 0;	 SimRTX = 0;
		RestKey = 0;Delay_Ms_StopScheduler(1000);
	RestKey = 1;//Wait_For_Nms(5000) ;DelaySec(10);
		Tled=0;			//
		runResult = EC20_START() ;    //                                                      //EC20开机
		PowerOn = 1;                       
// USART_DMA_ENABLE(EC20_UART, USART_MODE_RX | USART_MODE_TX,INT_RANK_1);
//	Fun_N32_AT() ;                                                      //EC20串口握手
		
//	runResult = EC20_CloseEcho() ;                                          //EC20关闭命令回显
//	  EC20_Query_SoftRelese(sEc20Param.ec20SoftVer) ;                              //查询EC20软件版本号并通过debug串口输出
//	AppLogPrintf("EC20 软件版本：%.*s", EC20_VER_LEN, sEc20Param.ec20SoftVer) ;
//	  EC20_Query_Voltage(sEc20Param.ec20Voltage) ;                                 //查询EC20供电电压并输出（实测：刚刚启动EC20无法获取电压）
//		AppLogPrintf("EC20 供电电压：%s mV", sEc20Param.ec20Voltage) ;               // EC20_CLOSE() ;
		return(runResult) ;
}
 
/**************************************************************************************************
* 名    称： RunResult EC20_SendModuleCmd( uint8_t cmdNum, char *format,... )  
* 功能说明：  MCU串口向EC20发送Module相关命令
* 入口参数：   
*            @param1 cmdNum  EC20_CMD_DATA_s中cmdNum成员命令编号
*            @param2 char *format,...  可变参变量
* 出口参数：   
*            @param  status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
RunResult EC20_SendModuleCmd( uint8_t cmdNum, char *format,... ) 
{
    uint8_t    revTimes = 0 ;
	  RunResult  status = TIMEOUT ;
		uint8_t    retryTimes = sModuleCmd[cmdNum].rtyNum ; 
    char       *cmdPack = NULL ;
		format  = sModuleCmd[cmdNum].cmdStr ;	
	  cmdPack = portMalloc(MODULE_CMDPACK_LEN*sizeof(uint8_t)) ;		
    va_list ap;  
    va_start (ap, format);     
    int outLen = vsnprintf(cmdPack, MODULE_CMDPACK_LEN, (const char*)format, ap); 	//vsprintf (temp, cmd, ap);  			//到此为止，所有的参数情况已经汇总到temp了 
		if((outLen<=0)||( outLen > MODULE_CMDPACK_LEN)) 	 
		  {
				ErrorLogPrintf("%s,%d:EC20_SendModuleCmd！",__FILE__, __LINE__) ;
				va_end (ap);
				portFree(cmdPack) ;
				return RUNERR ;
			}
		while(retryTimes--)                                             //命令失败重试
			{
			  Ec20AtBufReset() ;                                          //ec20AtBuf缓冲区清空 
				revTimes = 0 ;                                              //轮询计数器清零
        UARTx_SendData(EC20_UART, cmdPack, outLen)	 ;	            //EC20_UART发送AT命令		
				while( revTimes++ < sModuleCmd[cmdNum].timeout )            /*在命令超时时间内一直在ec20AtBuf缓冲区内循环查找trueStr*/
					{ 
							Delay_Ms_StopScheduler(100);//Wait_For_Nms(100) ;                                     //轮询查找时间间隔
//            Delay_Ms(100) ;
						sModuleCmd[cmdNum].trueOffset = kmp(ec20AtBuf, sModuleCmd[cmdNum].trueStr) ;  //获取sModuleCmd[cmdNum].trueStr在ec20AtBuf中的偏移值    						
						if( sModuleCmd[cmdNum].trueOffset >= 0)                 /*在ec20AtBuf中获取到sModuleCmd[cmdNum].trueStr字串*/
							{   
						      status = RUNOK ; 
								  goto OUT ;
							}
						else if( kmp(ec20AtBuf, sModuleCmd[cmdNum].falseStr) >= 0)/*在ec20AtBuf中没有获取到sModuleCmd[cmdNum].trueStr字串*/
							{
									status = RUNERR ;
									goto OUT ;
							}
					}
					Delay_Ms_StopScheduler(500+( sModuleCmd[cmdNum].rtyNum-retryTimes)*100);
//				 Wait_For_Nms( 500+( sModuleCmd[cmdNum].rtyNum-retryTimes)*100 ) ; //失败后延时一段时间再次发起请求命令		
			}
OUT:
		portFree(cmdPack) ;	                                                   //释放内存空间		
		va_end (ap) ;                                                          //释放内存空间
		DebugLogPrintf("%s %s", ModuleCmdNumToString((enum eModuleCmdNum)cmdNum), RunResultToString(status) ) ; //输出命令执行结果			
    return (status) ;	                                                     //返回函数执行结果
}

/**************************************************************************************************
* 名    称：  void EC20_Uart_Init(void)
* 功能说明：  初始化EC20_UART，初始化环形缓冲区、注册EC20_UART串口帧中断回调函数
**************************************************************************************************/
void EC20_Uart_Init(void)
{
	UARTx_Init(EC20_UART, 115200, USART_MODE_RX | USART_MODE_TX, INT_RANK_1) ;  // MCU与EC20通讯的串口初始化，设置为收发模式，中断优先级INT_RANK_1
 
	Ec20AtBufReset() ;                                                          //ec20AtBuf缓冲区初始化 
//  Ec20HttpBufReset() ;	                                                      //ec20HttpBuf缓冲区初始化 
	InitQueueMem(&SIMQueue) ;                                                  //初始化环形帧缓冲区，存储模块返回的tcp相关数据
//	InitQueueMem(&sUrcQueue) ;                                                  //初始化环形帧缓冲区，存储模块返回的URC数据
  InitQueueMem(&MQTT_PublishQueue) ;
	Uart_RegHookCallback(EC20_UART, Ec20ReceiveFrameCallback) ;	                //注册EC20接收帧中断回调函数 
}

/**************************************************************************************************
* 入口参数：   
*            @param1 *errInfo  存放错误信息
*            @param2 errLen    错误信息的长度
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//模块初始化
RunResult SimComInti(char *errInfo, uint8_t errLen)
{	
	RunResult runStatus = TIMEOUT ;
	
	EC20_SendModuleCmd(AT_ATF, NULL );//所有参数恢复默认值
	Delay_Ms(500);//Delay_Ms_StopScheduler(500) ;
	EC20_SendModuleCmd(AT_AT, NULL );
	EC20_SendModuleCmd(AT_IPR, NULL );
	EC20_SendModuleCmd(AT_IFC, NULL ); //开启回件流控 SimRTX = 0;启用
	EC20_SendModuleCmd(AT_ATI, NULL );
	EC20_SendModuleCmd(AT_COPS, NULL );
	
	runStatus = EC20_SendModuleCmd(AT_CPIN, NULL );
		if( RUNOK != runStatus)
			{		
				snprintf(errInfo, errLen, "请插入SIM卡") ;
			  ErrorLogPrintf("未检测到SIM卡") ;StatusBarPrintf("未检测到SIM卡");
				return runStatus;
		  }
	  AppLogPrintf("Sim卡ICCID：%.*s", SIM_ICCID_LEN, sEc20Param.simICCID) ;
	
	  runStatus = EC20_SendModuleCmd(AT_CREG, NULL );
	  if( RUNOK !=  runStatus ) 
		  {
				snprintf(errInfo, errLen, "SIM卡欠费") ;
			  ErrorLogPrintf("SIM欠费无法注册上网络") ;StatusBarPrintf("SIM欠费无法注册上网络");
				return runStatus ;
			}
/****运营商检测*****/
	EC20_SendModuleCmd(AT_CIMI, NULL );
//			DeivceFlag.Card_Type = 0;
//#ifdef SIM_only_mode
//	StatusBarPrintf("移动卡") ;ModelFlag=0;
//#else
	if(hand(ec20AtBuf,"46003")||hand(ec20AtBuf,"46005")||hand(ec20AtBuf,"46011"))
		{StatusBarPrintf("电信卡") ;ModelFlag=1;}					//电信版本
//		else
//{StatusBarPrintf("移动卡") ;ModelFlag=0;}	//上下二句仅用于香港 强制显示 移动卡
	else if(hand(ec20AtBuf,"46000")||hand(ec20AtBuf,"46001")||hand(ec20AtBuf,"46002")||hand(ec20AtBuf,"46007"))//00, 02移动 01联通
		{StatusBarPrintf("移动卡") ;ModelFlag=0;}	
	else if(hand(ec20AtBuf,"45400"))//香港CSL	
		{StatusBarPrintf("CSL") ;ModelFlag=0;}		
	else 
		{StatusBarPrintf("IOT 卡") ;
		ModelFlag=0;
//		DeivceFlag.Card_Type=1;					//IOT 卡类型 0 SIM卡 1 IOT卡
//			//IOT卡 如果全功能 或 仅SIM卡报警 转换到仅IOT报警模式
//		if((simANDmqtt == W433Hard.Head.DeviceAlarmMode)||(Sim_only == W433Hard.Head.DeviceAlarmMode)){W433Hard.Head.DeviceAlarmMode = mqtt_only;}
		}
//#endif	
//	if((ModelFlag)&&(!message_model)){Check = CDMACheck;}//电信 PDU模式
//	else Check = MobileCheck; //移动 PDU TXT; 电信 TXT						
#ifdef message_model	//TXT
Check_code = MobileCheck;	
#else									//PDU
Check_code = CDMACheck;	
#endif	
/***END****/
	EC20_SendModuleCmd(AT_CNMI, NULL );//新消息直接发给TE
	EC20_SendModuleCmd(AT_CPMS, NULL );
	
	if(ERROR == CSCA_CHCK(CSCA_W433_Address,0))
	CSCA_CMD(CSCA_W433_Address,0);								//设置短信中心号码
//sim900a_send_cmd("AT+CIPSTART=\"TCP\",\"www.szjcd.top\",9001\r\n","OK",100);  //39.108.91.128 CIP OPEN
//Delay_Ms_StopScheduler(2000) ;//DelaySec(2);	
//	EC20_SendModuleCmd(AT_CSCA, NULL ); 
//	EC20_SendModuleCmd(AT_TXT, NULL );
	if(RUNOK != EC20_SendModuleCmd(AT_TXT, NULL )){xEventGroupClearBits(netEventHandler,	MODULEBIT_0);DeivceFlag.GSMRet_Flag = 1;} //
	EC20_SendModuleCmd(AT_CSCS, NULL );//UCS2
//	sim900a_send_cmd("AT+CSMP=17,167,0,24\r\n","OK",100);  //UCS2 24 直接显示在终端，25短消息存储到SIM中，仅在TEXT格式下用到
	EC20_SendModuleCmd(AT_CSMP, NULL );//AT+CSMP=17,167,0,8\r\n

/********END*********/
	EC20_SendModuleCmd(AT_CHUP, NULL );//拒绝呼入
	EC20_SendModuleCmd(AT_VOLTE, NULL );  //  1 开启VOLTE
	//	Uart1Sends("AT+CSDVC=1\r\n");		  //切换到耳机  3 喇叭
	//	DelaySec(1);		   
	//	Uart1Sends("AT+CSDVC=3\r\n");		  //切换到喇叭 1 耳机  3 喇叭
	EC20_SendModuleCmd(AT_AUDCH, NULL );	   //输出通道0 听筒 ,输入 MAIN MIC
	EC20_SendModuleCmd(AT_CLVL, NULL );//0~100	  
	EC20_SendModuleCmd(AT_VTD, NULL ); //TONE 0~10	  
	EC20_SendModuleCmd(AT_CLCC, NULL ); //通话状态查询

	EC20_SendModuleCmd(AT_CTTSPRM, NULL );//volume,mode,pitch,speed,channel
	Delay_Ms_StopScheduler(20) ;//200
	EC20_Query_CSQ(sEc20Param.csq) ;                             //获取CSQ的值
 
//#ifdef message_model		//TXT
//	EC20_SendModuleCmd(AT_TXT, NULL );
//#else //PDU
//	EC20_SendModuleCmd(AT_PDU, NULL );	//向主叫号码PhoneNumber发送 端口状态						
//#endif		
//	Delay_Ms_StopScheduler(200) ;

//	sim900a_send_cmd("AT+CIPCLOSE=0\r\n","OK",50);  //CIP CLOSE	
	return runStatus ;
}

/****************************************************************************
* 名    称：void EC20_POWON(void) 
* 功    能：打开EC20模块供电电源
****************************************************************************/
void EC20_POWON(void)
{
//	 Wait_For_Nms(3000) ;                                       //开机前保持3S断电状态
   PowerOn = 1;//EC20_POW = 1 ;
}

/****************************************************************************
* 名    称：void EC20_POWOFF(void) 
* 功    能：关闭EC20模块供电电源
****************************************************************************/
void EC20_POWOFF(void)
{
   PowerOn = 0;//EC20_POW = 0 ;
	 Delay_Ms_StopScheduler(3000) ;  //Wait_For_Nms断电后保持5S断电状态，因为模块内核可能由于电容放电没有完全断电清除网络参数
//		EC20_POW = 1 ;
}
/****************************************************************************
 
* 功    能：检测是否在线
****************************************************************************/
ErrorStatus EC20_CEREG(void)
{
	EC20_SendModuleCmd(AT_CGREG, NULL );

//	 ErrorStatus runResult = sim900a_send_cmd("AT+CEREG?\r\n","OK",200) ;

	if(hand(ec20AtBuf,"+CEREG: 0,1")||hand(ec20AtBuf,"+CEREG: 0,5"))return 	SUCCESS;

	return 	ERROR;
}
/****************************************************************************
 
* 功    能：进入飞行模式 等待5S后 退出飞行模式
****************************************************************************/
void AirMode(void)
{
   EC20_SendModuleCmd(AT_CFUN, NULL,"0" ) ;//进入飞行模式
	 Delay_Ms(3000) ;  //保持3S断电状态
	 EC20_SendModuleCmd(AT_CFUN, NULL,"1" ) ;//退出飞行模式
}
/**************************************************************************************************
* 名    称： RunResult EC20_Query_CSQ(char *csq)
* 功能说明： 查询网络CSQ（信号质量）
* 入口参数：   
*            @param1 *csq  存放csq的内存地址
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
RunResult EC20_Query_CSQ(uint8_t *csq)
{

//	 memset(csq, 0, CSQ_LEN);
	 RunResult runResult = EC20_SendModuleCmd(AT_CSQ, NULL ) ;
	
	 if( RUNOK == runResult )
		 {
char *haystack = ec20AtBuf,*buf;
	 buf= strstr( haystack, "+CSQ:");//头字符串
	 haystack = buf + 6;	//后移长度	
//			 CopyValues((uint8_t*)csq, (uint8_t*)haystack,  ',', CSQ_LEN) ;//+CSQ:			 
			 CopyValues(csq, (uint8_t*)haystack,  ',', CSQ_LEN) ;//+CSQ:
			*(csq + CSQ_LEN -1) = 0;
		 } 		     
	 return (runResult) ;		 
}
/****************************************************************************
* 名    称：RunResult EC20_START(void) 
* 功    能：EC20开机启动
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
* 说    明：EC20上电大概15S内串口会输出“RDY”字符串
****************************************************************************/
RunResult EC20_START(void)
{
	u16 findCount = 0 ;
	RunResult runStatus = TIMEOUT ;
DebugLogPrintf("EC20 Start " ) ;	
//  EC20_POWON() ;                                                     //开启EC20电源
	Ec20AtBufReset() ;                                                 //ec20AtBuf缓冲区初始化
 	while( findCount < 300)                                             //循环等待EC20开机标志
	{
		 findCount++ ;
	   Delay_Ms_StopScheduler(200) ;
		 if( kmp(ec20AtBuf, "SMS READY") >= 0)                                 /*收到EC20返回的”RDY“字符串，EC20开机完成*/
			 {
				 DebugLogPrintf("EC20 Start Waited:%d S", findCount ) ;
				 findCount = 300;
				 runStatus = RUNOK ;
				 break ;
			 }
	}
	return(runStatus) ;
}

/****************************************************************************
* 名    称：RunResult EC20_CLOSE(void)
* 功    能：EC20关机流程  软关机-->等待1S-->断电
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
****************************************************************************/
RunResult EC20_CLOSE(void)    
{	
	  RunResult runStatus = TIMEOUT ;
		runStatus = EC20_SendModuleCmd(AT_CPOWD, NULL ) ;
		if( RUNOK != runStatus )
			{		
				ErrorLogPrintf("EC20 软关机失败，即将断电关机！") ;
		  }
		Delay_Ms_StopScheduler(1000) ;                                       //《AT Command》手册中建议延时1s后再断电
		EC20_POWOFF() ;
		return (runStatus) ;
}

/**************************************************************************************************
* 名    称： RunResult Fun_N32_AT( void )
* 功能说明： MCU串口发送AT指令，验证串口通信是否就绪
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
RunResult Fun_N32_AT( void )
{
	  RunResult runStatus = TIMEOUT ;
		runStatus = EC20_SendModuleCmd(AT_AT, NULL ) ;
		return (runStatus) ;
}

/**************************************************************************************************
* 名    称： RunResult EC20_CloseEcho(void)
* 功能说明： 关闭EC20指令回码
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//RunResult EC20_CloseEcho(void)
//{
//	RunResult runStatus = TIMEOUT ;
//	runStatus = EC20_SendModuleCmd( AT_IPR, NULL ) ;
////runStatus = EC20_SendModuleCmd( AT_CNMI, NULL ) ;
//	//  runStatus = EC20_SendModuleCmd( AT_ATF, NULL ) ;
////	if( runStatus != RUNOK ) 
////	  {
////		  EC20_SendModuleCmd( AT_IPR, NULL ) ;                                //关闭串口AT命令ECHO 
////		  runStatus = EC20_SendModuleCmd( AT_ATF, NULL ) ;
////			// EC20_SendModuleCmd( AT_IFC, sModuleCmd[AT_IFC].cmdStr ) ;  //为了兼容前面版本，不保存				  
////    } 
//	return (runStatus) ;
//}

/**************************************************************************************************
* 名    称： RunResult EC20_Query_SoftRelese(char *version)
* 功能说明： 查询EC20的固件版本
* 入口参数：   
*            @param1 *version  存放version的内存地址
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//RunResult EC20_Query_SoftRelese(char *version)
//{
//	 RunResult runStatus = TIMEOUT ;
//	 char *start ;
//	 memset(version, 0, EC20_VER_LEN+1);
//	 runStatus = EC20_SendModuleCmd(AT_ATI, NULL ) ;
//	 if( RUNOK == runStatus )
//		 {
//			 start = strchr( (const char*)ec20AtBuf, 0x0D )  ;
//			 CopyValues((uint8_t*)version, (uint8_t*)(start+2),  0x0D, EC20_VER_LEN) ;				
//		 }		   
//	 return (runStatus) ;		 
//}

/**************************************************************************************************
* 名    称： RunResult EC20_Query_Voltage(char *voltage)
* 功能说明： 查询EC20的供电电压
* 入口参数：   
*            @param1 *voltage  存放voltage的内存地址
* 出口参数：   
*            @param1 status  RunResult枚举类型变量，返回函数运行结果
**************************************************************************************************/
//RunResult EC20_Query_Voltage(char *voltage)
//{
//	 RunResult runStatus = TIMEOUT ;
//	 char *start ;
//	 memset(voltage, 0, EC20_VOL_LEN+1);
//	 runStatus = EC20_SendModuleCmd(AT_CIMI, NULL ) ;
//	 if( RUNOK == runStatus )
//		 {
//			 start = strrchr( (const char*)ec20AtBuf, ',' ) ;
//			 CopyValues((uint8_t*)voltage, (uint8_t*)start+1, 0x0D, EC20_VOL_LEN) ;			
//		 }
//   else
//	   {
//		   strcpy(voltage, "3500") ;
//		 }		 
//	 return (runStatus) ;		 
//}














