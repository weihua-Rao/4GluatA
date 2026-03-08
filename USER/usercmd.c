//#include "usercmd.h"
//#include "flash.h"
//#include "usart.h"
//#include "sysport.h"
//#include "user_tcp.h"
//#include "userapp.h"
//#include "osPort.h"
//#include "logflash.h"	
//#include "key.h"
//#include "ec20tcp.h"
//#include "user_tcp.h" 
#include "main.h"

/******************************************************************************
*供外部使用的常变量定义
*******************************************************************************/
FrameQueue_s      sUsercmdQueue ;  //定义帧环形缓冲区，用于存放用户配置命令

/*用来保存FLASH内的配置信息*/
IAPFlash_u uIapFlash ;             //定义IAPFlash_u共用体变量
APPFlash_u uAppFlash ;             //定义APPFlash_u共用体变量
/*命令处理异常*/
char  *noCmdErr  = "命令不支持" ;
char  *paramErr  = "参数不合法!" ;
char  *formErr   = "命令格式错误!" ;

//extern union
//{
//	uint16_t U16Data;
//	struct{
//		uint8_t Bety_H;
//		uint8_t Bety_L;
//		}Bety;
//}U16_U8;


//十六进制小字程序
//输入地址2位 返回一个字节
//uint8_t Ascii2Hex(uint8_t dat)
//{
////	u8 datbyte;
////
////	if(*dat < 'A')datbyte = *dat&0x0f;
////	else datbyte = *dat-'A'+10;
////
////	datbyte <<= 4;dat++;
//	if(dat < 'A')return (dat&0x0f);
//	else if(dat < 'a')return (dat-'A'+10);
//	return 	(dat-'a'+10);
//}
/*********************************************************************************************************************
* 名    称：void InitIapFlashConfig( IAPFlash_u *config )
* 功    能：先从FLASH读出IAPFlash信息，然后对IAPFlash_u类型的全局变量初始化，最终再写入FLASH中
* 说    明：读取RunAppNum，更新FLASH中的RunAppNum标志位
*********************************************************************************************************************/
void InitIapFlashConfig( IAPFlash_u *config )
{ 
//	   InitQueueMem(&sUsercmdQueue) ;               //初始化用于存放用户配置命令的环形缓冲区sUsercmdQueue
//	   Get_uIapFlash(config) ;                      //从FLASH中读取IAP信息到共用体变量uIapFlash中
//	   config->sIapFlash.Head = '[' ;               //帧头初始化
//	   config->sIapFlash.IapFlag = 0x30 ;           //升级标志位初始化
//		 #if defined APP0_CODE
//           config->sIapFlash.RunAppNum =0x30 ;    //当前APP为APP0
//	         strcpy((char*)config->sIapFlash.BootVers, "No Boot") ;
//		 #elif defined APP1_CODE
//					 config->sIapFlash.RunAppNum =0x31 ;    //当前APP为APP1
//		 #elif defined APP2_CODE
//					 config->sIapFlash.RunAppNum =0x32 ;    //当前APP为APP2
//		 #elif defined APP3_CODE
//					 config->sIapFlash.RunAppNum =0x33 ;    //当前APP为APP3
//		 #endif	 
//		 config->sIapFlash.JumpResult = 0x31 ;        //系统从boot跳转到APP成功，将JumpResult位设置为true并写入FLASH。//如果BOOT启动判断该为为false，则判定APP有问题，直接运行应急程序APP1
//	   if((uint8_t)config->sIapFlash.FtpFolder[0] == 0xFF )          /*未配置过FtpFolder*/
//		   {
//				 memset( config->sIapFlash.FtpFolder, 0, FOLDER_MAXLEN) ;
//			   strcpy( config->sIapFlash.FtpFolder, "/bujiz") ;            //如果未配置FtpFolder，则将默认路径设置为‘/ABM‘
//			 }
//		 if( (uint8_t)config->sIapFlash.FtpUsername[0] == 0xFF )       /*FLASH中存储ftp用户名的字段有效*/
//		   {
//				 memset( config->sIapFlash.FtpUsername, 0, USERNAME_MAXLEN) ;
//         strcpy( config->sIapFlash.FtpUsername, "bujiz") ;           //如果未配置FtpUsername，则将FTP默认用户名设置为‘ABM‘				 
//		   }
//		 if( (uint8_t)config->sIapFlash.FtpPassword[0] == 0xFF )       /*FLASH中存储ftp用户名密码的字段无效*/
//		   {
//				 memset( config->sIapFlash.FtpPassword, 0, PASSWORD_MAXLEN) ;
//	       strcpy( config->sIapFlash.FtpPassword, "Abm123456") ; //如果未配置FtpUsername，则将FTP默认用户名密码设置为‘Shop344794749			
//		   }
//		 if( (uint8_t)config->sIapFlash.FtpIP[0] == 0xFF )             /*FLASH中存储FtpIP字段无效*/
//		   {
//				 memset( config->sIapFlash.FtpIP, 0, MAX_IP_LEN) ;
//	       strcpy( config->sIapFlash.FtpIP, "121.5.217.252") ;        //"121.41.79.87"如果未配置FtpIP，则将FTP服务器IP设置为"121.41.79.87"	
//		   }
//		 if( ((uint8_t)config->sIapFlash.TcpIP[0] == 0xFF) || 
//			   ((uint8_t)config->sIapFlash.TcpIP[0] == 0x00))             /*FLASH中存储FtpIP字段无效*/
//		   {
//				 memset( config->sIapFlash.TcpIP, 0, MAX_IP_LEN) ;
//	       strcpy( config->sIapFlash.TcpIP, TCP_CONNECTID0_SERVERIP0) ;//如果未配置FtpIP，则将FTP服务器IP设置为TCP_CONNECTID0_SERVERIP0
//		   }
//		 else                                                         /*FLASH中存储FtpIP字段有效，将s_Channal0缺省IP修改为配置IP*/   
//			 {
//			   SetAppTcpIP(&sChannal0, uIapFlash.sIapFlash.TcpIP ) ;             //更新TCP服务器的IP
//			 }
//		 if( ((uint8_t)config->sIapFlash.TcpPort[0] == 0xFF) ||
//         ((uint8_t)config->sIapFlash.TcpPort[0] == 0x00)			 )            /*FLASH中存储FtpIP字段无效*/
//		   {
//				 memset( config->sIapFlash.TcpPort, 0, PORT_MAXLEN) ;  
//				 char port[PORT_MAXLEN] = {0} ;
//				 itoa(TCP_CONNECTID0_SERVERPORT0, port, 10);
//	       strcpy( config->sIapFlash.TcpPort, "7019") ;        //port        //如果未配置TcpPort，则将FTP服务器IP设置为TCP_CONNECTID0_SERVERPORT0	
//		   }
//		 else
//		   {
//			   SetAppTcpPort(&sChannal0, uIapFlash.sIapFlash.TcpPort ) ; //更新TCP服务器的port
//			 }
//		
//	   config->sIapFlash.Tail = ']' ;               								 //帧尾初始化
//	   Set_uIapFlash(config) ;                     								   //将初始化信息重新写入FLASH中
}

/**************************************************************************************************
* 名    称：  void Get_uIapFlash(IAPFlash_u *config)
* 功能说明：  从内部flash中IAPCONFIG_AREA_ADDR地址读取一页，获取FLASH中的IAP信息
* 入口参数：  
*            @param1 *uIapInfo  IAPFlash_u共用体变量指针
**************************************************************************************************/
void Get_uIapFlash(IAPFlash_u *config)
{
	  memset( config->iapFlashBuffer , 0, IAPFLASHCONFIGLEN) ;
	  Read_Flash_Byte(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN) ;
}

/**************************************************************************************************
* 名    称：  void Set_uIapFlash(IAPFlash_u *config)
* 功能说明：  将新的IAP信息写入FLASH
* 入口参数：  
*            @param1 *uIapInfo  IAPFlash_u共用体变量指针
**************************************************************************************************/
void Set_uIapFlash(IAPFlash_u *config)
{
	Write_Flash_OnePage(IAPCONFIG_AREA_ADDR, config->iapFlashBuffer, IAPFLASHCONFIGLEN) ;
}

/*********************************************************************************************************************
* 名    称：void InitAppFlashConfig( APPFlash_u *config )
* 功    能：先从FLASH读出APPFlash信息，然后对APPFlash_u类型的全局变量初始化，最终再写入FLASH中
* 说    明：先读取FLASH中存储的配置信息，然后做校验，不合法的标志位全部初始化为默认值，最后再写入FLASH
*********************************************************************************************************************/
void InitAppFlashConfig( APPFlash_u *config )
{ 
		Get_uAppFlash(config) ;                                                    //从FLASH中读取IAP信息到共用体变量uAppFlash中
	
		Set_uAppFlash(config) ;
}

/**************************************************************************************************
* 名    称：  void Get_uIapFlash(APPFlash_u *config)
* 功能说明：  从内部flash中APPFLASHCONFIGLEN地址读取一页，获取FLASH中的APP信息
* 入口参数：  
*            @param1 *uIapInfo  IAPFlash_u共用体变量指针
**************************************************************************************************/
void Get_uAppFlash(APPFlash_u *config)
{
	  memset( config->appFlashBuffer, 0, APPFLASHCONFIGLEN) ;
	 	Read_Flash_Byte(APPCONFIG_AREA_ADDR, config->appFlashBuffer, APPFLASHCONFIGLEN ) ;
}

/**************************************************************************************************
* 名    称：  Set_uIapFlash(APPFlash_u *config)
* 功能说明：  将新的IAP信息写入FLASH
* 入口参数：  
*            @param1 *uIapInfo  IAPFlash_u共用体变量指针
**************************************************************************************************/
void Set_uAppFlash(APPFlash_u *config)
{
	Write_Flash_OnePage(APPCONFIG_AREA_ADDR, config->appFlashBuffer, APPFLASHCONFIGLEN) ;
}

/****************************************************************************
* 名    称：void Cmd_Process( char *cmdBuf, int bufLen )
* 功    能：对用户控制命令进行处理
* 入口参数：
*           @param1 cmdBuf    指向控制命令的指针
*           @param2 bufLen    控制命令数据的长度
* 说    明：对用户控制命令进行处理，实际就是响应命令动作并且将新的参数保存到对应的FLASH区域
* 格    式：(L0072MAC:31FFD405524E353728902251;10&CMD_RST$X)
****************************************************************************/
void Cmd_Process( char *cmdBuf, int bufLen )
{
	  TcpFrame_S *psBuf = (TcpFrame_S*)cmdBuf ;
	  char state ;
	  switch( psBuf->cmdCode[1] )    /*解析操作码第2字节*/
			 {
					case 0x30:  //重启指令
						{
								state = RestartCmdProcess( cmdBuf, bufLen ) ;
								break ;
						}
					case 0x31:  //Config指令
						{
								state = GetConfigCmdProcess( cmdBuf, bufLen ) ;
								break ;
						}
					case 0x32:  //IapFlag指令 升级标识
						{
								state = IapFlagCmdProcess( cmdBuf, bufLen ) ;
								break ;
						}												
					case 0x33:  //RunAppNum指令
						{
								state = RunAppNumCmdProcess( cmdBuf, bufLen) ;
								break ;
						}
					case 0x34:  //FtpFolder指令
						{
								state = FtpFolderCmdProcess( cmdBuf, bufLen) ;
								break ;
						}
					case 0x35:  //FtpUsername指令
						{
								state = FtpUsernameCmdProcess( cmdBuf, bufLen) ;
								break ;
						}
					case 0x36:  //FtpPassword指令
						{
								state = FtpPasswordCmdProcess( cmdBuf, bufLen) ;
								break ;
						}	
					case 0x37:  //FtpIP指令
						{
								state = FtpIPCmdProcess( cmdBuf, bufLen) ;
								break ;
						}	
					case 0x38:  //TcpIP指令
						{
								state = TcpIPCmdProcess( cmdBuf, bufLen) ;
								break ;
						}	
					case 0x39:  //TcpIP指令
						{
								state = TcpPortCmdProcess( cmdBuf, bufLen) ;
								break ;
						}						
					case 0x40:  //音量等级指令
						{
								state = VolumeRankCmdProcess() ;
								break ;
						}									
					default :  //指令号不合法，不存在该指令
						{
								state = 'N' ;
								break ;
						}						 
			 }
		 if( state == 'P' ) //指令负载中参数不合法
			 {
				 CmdResponse( cmdBuf, paramErr) ;						 
			 }
		 else if(state == 'I') //指令格式错误
			 {
				 CmdResponse( cmdBuf, formErr) ;							 
			 }
		 else if(state == 'N')
			 {
				 CmdResponse( cmdBuf, noCmdErr) ;					 
			 }
}
/**************************************************************************************************
* 名    称： char RestartCmdProcess( char *cmdBuf, int bufLen ) 
* 说    明： 解析“软重启”命令
* 入口参数：
*				 @param1  *cmdBuf: 命令数据帧指针
*        @param2 bufLen  命令数据帧长度
* 出口参数：
*       @param1 state: 指令处理结果‘S’：成功；‘P’：参数错误；‘I’：负载格式错误 
* 数据帧格式： (c0072MAC:31FFD405524E353728902251;10&CMD_RST$X)
  ************************************************************************************************/
char RestartCmdProcess( char *cmdBuf, int bufLen ) 
{
	char state ;
	TcpFrame_S *psBuf = (TcpFrame_S*)cmdBuf ;
	if( kmp((&(psBuf->loadHead)+1), "CMD_RST") == 0 )  //LOAD:CMD_RST
		{	
      state = 'S' ;                                   //标志指令处理成功			
      CmdResponse( cmdBuf, "*SET* User control device restart!") ;	     //通过TCP上传命令处理回码	
			vTaskDelay(2*configTICK_RATE_HZ) ;              //等待2S
			if( pdTRUE == xSemaphoreTake(ec20MutexSemaphore, (TickType_t)5*1000) ) //可能此时EC20资源处于被占用状态。 5S内没有获取到EC20资源，则强制重启
				{
//					 TcpDisconnetc() ;
					 //Close_Socket(TCP_CONNECTID0) ;   //断开Socket连接
					 //Deact_Context(TCP_CONTEXTID) ;
					 EC20_CLOSE() ;
				}
      RELESE_EC20() ;                               //释放EC20资源	
//      WriteLogToFlash("重启命令软重启!") ;				
			NVIC_SystemReset();//SystemSoftReset() ;
		}
	else
		{
		  state = 'P' ;                                 //指令参数不合法
		}
  return state ;
}

/**************************************************************************************************
* 名    称： char GetConfigCmdProcess( char *cmdBuf, int bufLen ) 
* 说    明： 解析“GET_IAP”命令:获取存储在内部flash中IAPCONFIG_AREA_ADDR地址的IAP配置信息
* 入口参数：
*				 @param1  *cmdBuf: 命令数据帧指针
*        @param2 bufLen  命令数据帧长度
* 出口参数：
*       @param1 state: 指令处理结果‘S’：成功；‘P’：参数错误；‘I’：负载格式错误 
* 数据帧格式： (c0072MAC:31FFD405524E353728902251;01&GET_IAP$X)
  ************************************************************************************************/
char GetConfigCmdProcess(char *cmdBuf, int bufLen) 
{
	char state ;
//	TcpFrame_S *psBuf = (TcpFrame_S*)cmdBuf ;	
//	Read_Flash_Byte(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;         //读uIapFlash配置
//	if( kmp((&(psBuf->loadHead)+1), "GET_IAP") == 0 ) //LOAD:GET_IAP
//	{
//	  char *backBuf = portMalloc(512) ;
//		Query_AppVersion((char*)gateApp.appVers) ;
//		snprintf(backBuf, 512, "*GET* IapFlag:[%c]!  RunAppNum:[APP%c]!  JumpResult:[%c]!  BootVers:[%s]!  AppVers:[%s]!  Csq:[%s]!  Ec20Vol:[%s]!  \
//SimIccid:[%s]!  FtpUsername:[%s]!  FtpPassword:[%s]!  FtpFolder:[%s]!  FtpIP:[%s]!  TcpIP:[%s]!  TcpPort:[%s]!",	
//		uIapFlash.sIapFlash.IapFlag, uIapFlash.sIapFlash.RunAppNum, uIapFlash.sIapFlash.JumpResult, uIapFlash.sIapFlash.BootVers, gateApp.appVers,\
//		sEc20Param.csq, sEc20Param.ec20Voltage, sEc20Param.simICCID, uIapFlash.sIapFlash.FtpUsername,uIapFlash.sIapFlash.FtpPassword,\
//		uIapFlash.sIapFlash.FtpFolder, uIapFlash.sIapFlash.FtpIP, uIapFlash.sIapFlash.TcpIP, uIapFlash.sIapFlash.TcpPort) ;
//		CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
//	  state = 'S' ; //指令执行成功
//	  portFree(backBuf) ;
//	}
//	else //负载（负载长度）不合法
//	{
//	  state = 'I' ; //指令负载中参数不合法
//	}
  return state ;	
}

/**************************************************************************************************
* 名    称： char IapFlagCmdProcess( char *cmdBuf, int bufLen ) 
* 说    明： 解析“IapFlag”字段命令
* 入口参数：
*				 @param1  *cmdBuf: 命令数据帧指针
*        @param2   bufLen：命令数据帧长度
* 出口参数：
*       @param1 state: 指令处理结果‘S’：成功；‘P’：参数错误；‘I’：负载格式错误 
* 数据帧格式： (c0012MAC:31FFD405524E353728902251;12&1$X)
  ************************************************************************************************/
char IapFlagCmdProcess(char *cmdBuf, int bufLen) 
{
	char state ;
	TcpFrame_S *psBuf = (TcpFrame_S*)cmdBuf ;
	Read_Flash_Byte(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;         //读配置
	char *backBuf = (char*)portMalloc(256) ;
	int len = snprintf(backBuf, 256, "*GET* IapFlag:[%c]! ", uIapFlash.sIapFlash.IapFlag) ;
	if((psBuf->cmdCode[0] == 0x31) && (kmp(psBuf->loadLen, "001") == 0) )  //SET命令&&此命令负载长度为1字节
	{   
			if( IS_IapFlag_ALL_PERIPH( *(&(psBuf->loadHead)+1)) )  //负载参数合法校验
			{
				uIapFlash.sIapFlash.IapFlag = *(&(psBuf->loadHead)+1) ; //改配置：将负载值赋值给uIapFlash.sIapFlash.IapFlag      
				Write_Flash_OnePage(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;      //保存配置
				snprintf((backBuf+len), 256-len, "*SET* IapFlag:[%c]!", uIapFlash.sIapFlash.IapFlag ) ;
				state = 'S' ; //指令执行成功
				CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
			}
			else
			{
				state = 'P' ; //指令负载中参数不合法
			}
	}
	else if((psBuf->cmdCode[0] == 0x30) && (kmp(psBuf->loadLen, "001") == 0) ) //GET命令&&此命令负载长度为1字节
	{
	    state = 'S' ;                            //读指令执行成功
		  CmdResponse( cmdBuf, backBuf) ;	   //通过TCP上传命令处理回码
	}
	else //负载（负载长度）不合法
	{
	    state = 'I' ; //指令负载中参数不合法
	}	
	portFree(backBuf) ;
  return state ;		
}

/**************************************************************************************************
* 名    称： char RunAppNumCmdProcess( char *cmdBuf, int bufLen ) 
* 说    明： 解析“RunAppNum”字段命令
* 入口参数：
*				 @param1  *cmdBuf: 命令数据帧指针
*        @param2   bufLen：命令数据帧长度
* 出口参数：
*       @param1 state: 指令处理结果‘S’：成功；‘P’：参数错误；‘I’：负载格式错误 
* 数据帧格式： (c0012MAC:31FFD405524E353728902251;13&1$X)
  ************************************************************************************************/
char RunAppNumCmdProcess( char *cmdBuf, int bufLen )  
{
	char state ;
	TcpFrame_S *psBuf = (TcpFrame_S*)cmdBuf ;
	Read_Flash_Byte(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;         //读配置
	char *backBuf = (char*)portMalloc(256) ;
	Query_AppVersion((char*)gateApp.appVers) ;
	int len = snprintf(backBuf, 256, "*GET* RunAppNum:[APP%c] !   *GET* AppVersion:[%s] !", uIapFlash.sIapFlash.RunAppNum, gateApp.appVers) ;
	if((psBuf->cmdCode[0] == 0x31) && (kmp(psBuf->loadLen, "001") == 0) )  //SET命令&&此命令负载长度为1字节
	{
    if( IS_RunAppNum_ALL_PERIPH( *(&(psBuf->loadHead)+1) ) )  //负载参数合法
				{
					uIapFlash.sIapFlash.RunAppNum = *(&(psBuf->loadHead)+1) ;                                         //改配置   
					Write_Flash_OnePage(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;      //保存配置
					snprintf((backBuf+len), 256-len, "*SET* RunAppNum:[APP%c]重启后生效!", uIapFlash.sIapFlash.RunAppNum ) ;
					state = 'S' ; //指令执行成功
					CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
				}
				else //负载参数不合法
				{
				  state = 'P' ; //指令负载中参数不合法
				}		
	}
	else if((psBuf->cmdCode[0] == 0x30) && (kmp(psBuf->loadLen, "001") == 0) ) //GET命令&&此命令负载长度为1字节
	{
	    state = 'S' ;      //读指令执行成功
		  CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
	}
	else //负载（负载长度）不合法
	{
	    state = 'I' ; //指令负载中参数不合法
	}	
	portFree(backBuf) ;
  return state ;		
}

/**************************************************************************************************
* 名    称： char FtpFolderCmdProcess( char *cmdBuf, int bufLen ) 
* 说    明： 解析“FtpFolder”字段命令
* 入口参数：
*				 @param1  *cmdBuf: 命令数据帧指针
*        @param2   bufLen：命令数据帧长度
* 出口参数：
*       @param1 state: 指令处理结果‘S’：成功；‘P’：参数错误；‘I’：负载格式错误 
* 数据帧格式： (c0042MAC:31FFD405524E353728902251;14&/ftp$X)
  ************************************************************************************************/
char FtpFolderCmdProcess( char *cmdBuf, int bufLen )  
{
	char state ;
	TcpFrame_S *psBuf = (TcpFrame_S*)cmdBuf ;
	unsigned int loadLen = (psBuf->loadLen[0]-0x30)*100 + (psBuf->loadLen[1]-0x30)*10 + (psBuf->loadLen[2]-0x30) ;
	Read_Flash_Byte(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;         //读配置
	char *backBuf = (char*)portMalloc(256) ;

	int len = snprintf(backBuf, 256, "*GET* FtpFolder:[%s]!", uIapFlash.sIapFlash.FtpFolder) ;
	if((psBuf->cmdCode[0] == 0x31) && ( *(&(psBuf->loadHead)+loadLen+1) == '$' ) )  //SET命令&&此命令负载长度校验成功
	{
			memset(uIapFlash.sIapFlash.FtpFolder, 0, FOLDER_MAXLEN ) ;
			strncpy(uIapFlash.sIapFlash.FtpFolder, (&(psBuf->loadHead)+1), loadLen) ;      //将新的FtpFolder拷贝到uIapFlash.sIapFlash.FtpFolder中   
			Write_Flash_OnePage(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;      //保存配置
			snprintf((backBuf+len), 256-len, "*SET* FtpFolder:[%s]!", uIapFlash.sIapFlash.FtpFolder ) ;
			state = 'S' ; //指令执行成功
			CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
	}
	else if((psBuf->cmdCode[0] == 0x30) && ( *(&(psBuf->loadHead)+loadLen+1) == '$' ) ) //GET命令&&此命令负载长度校验成功
	{
	    state = 'S' ;                              //读指令执行成功
		  CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
	}
	else //负载（负载长度）不合法
	{
	    state = 'I' ; //指令负载中参数不合法
	}	
	portFree(backBuf) ;
  return state ;		
}

/**************************************************************************************************
* 名    称： char FtpUsernameCmdProcess( char *cmdBuf, int bufLen ) 
* 说    明： 解析“FtpUsername”字段命令
* 入口参数：
*				 @param1  *cmdBuf: 命令数据帧指针
*        @param2   bufLen：命令数据帧长度
* 出口参数：
*       @param1 state: 指令处理结果‘S’：成功；‘P’：参数错误；‘I’：负载格式错误 
* 数据帧格式： (c0032MAC:31FFD405524E353728902251;15&ABM$X)
  ************************************************************************************************/
char FtpUsernameCmdProcess( char *cmdBuf, int bufLen )  
{
	char state ;
	TcpFrame_S *psBuf = (TcpFrame_S*)cmdBuf ;
	unsigned int loadLen = (psBuf->loadLen[0]-0x30)*100 + (psBuf->loadLen[1]-0x30)*10 + (psBuf->loadLen[2]-0x30) ;
	Read_Flash_Byte(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;         //读配置
	char *backBuf = (char*)portMalloc(256) ;

	int len = snprintf(backBuf, 256, "*GET* FtpUsername:[%s]!", uIapFlash.sIapFlash.FtpUsername) ;
	if((psBuf->cmdCode[0] == 0x31) && ( *(&(psBuf->loadHead)+loadLen+1) == '$' ) )  //SET命令&&此命令负载长度校验成功
	{
			memset(uIapFlash.sIapFlash.FtpUsername, 0, USERNAME_MAXLEN ) ;
			strncpy(uIapFlash.sIapFlash.FtpUsername, (&(psBuf->loadHead)+1), loadLen) ;      //将新的FtpUsername拷贝到uIapFlash.sIapFlash.FtpUsername中  
			Write_Flash_OnePage(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;      //保存配置
			snprintf((backBuf+len), 256-len, "*SET* FtpUsername:[%s]!", uIapFlash.sIapFlash.FtpUsername ) ;
			state = 'S' ; //指令执行成功
			CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
	}
	else if((psBuf->cmdCode[0] == 0x30) && ( *(&(psBuf->loadHead)+loadLen+1) == '$' ) ) //GET命令&&此命令负载长度校验成功
	{
	    state = 'S' ;                              //读指令执行成功
		  CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
	}
	else //负载（负载长度）不合法
	{
	    state = 'I' ; //指令负载中参数不合法
	}	
	portFree(backBuf) ;
  return state ;		
}

/**************************************************************************************************
* 名    称： char FtpPasswordCmdProcess( char *cmdBuf, int bufLen ) 
* 说    明： 解析“FtpPassword”字段命令
* 入口参数：
*				 @param1  *cmdBuf: 命令数据帧指针
*        @param2   bufLen：命令数据帧长度
* 出口参数：
*       @param1 state: 指令处理结果‘S’：成功；‘P’：参数错误；‘I’：负载格式错误 
* 数据帧格式： (c0130MAC:35FFDC054D52323238780843;16&Shop344794749$X)
  ************************************************************************************************/
char FtpPasswordCmdProcess( char *cmdBuf, int bufLen )  
{
	char state ;
	TcpFrame_S *psBuf = (TcpFrame_S*)cmdBuf ;
	unsigned int loadLen = (psBuf->loadLen[0]-0x30)*100 + (psBuf->loadLen[1]-0x30)*10 + (psBuf->loadLen[2]-0x30) ;
	Read_Flash_Byte(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;         //读配置
	char *backBuf = (char*)portMalloc(256) ;

	int len = snprintf(backBuf, 256, "*GET* FtpPassword:[%s]!", uIapFlash.sIapFlash.FtpPassword) ;
	if((psBuf->cmdCode[0] == 0x31) && ( *(&(psBuf->loadHead)+loadLen+1) == '$' ) )  //SET命令&&此命令负载长度校验成功
	{
			memset(uIapFlash.sIapFlash.FtpPassword, 0, PASSWORD_MAXLEN ) ;
			strncpy(uIapFlash.sIapFlash.FtpPassword, (&(psBuf->loadHead)+1), loadLen) ;      //将新的FtpPassword拷贝到uIapFlash.sIapFlash.FtpPassword中  
			Write_Flash_OnePage(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;      //保存配置
			snprintf((backBuf+len), 256-len, "*SET* FtpPassword:[%s]!", uIapFlash.sIapFlash.FtpPassword ) ;
			state = 'S' ; //指令执行成功
			CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
	}
	else if((psBuf->cmdCode[0] == 0x30) && ( *(&(psBuf->loadHead)+loadLen+1) == '$' ) ) //GET命令&&此命令负载长度校验成功
	{
	    state = 'S' ;                              //读指令执行成功
		  CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
	}
	else //负载（负载长度）不合法
	{
	    state = 'I' ; //指令负载中参数不合法
	}	
	portFree(backBuf) ;
  return state ;		
}

/**************************************************************************************************
* 名    称： char FtpIPCmdProcess( char *cmdBuf, int bufLen ) 
* 说    明： 解析“FtpIP”字段命令
* 入口参数：
*				 @param1  *cmdBuf: 命令数据帧指针
*        @param2   bufLen：命令数据帧长度
* 出口参数：
*       @param1 state: 指令处理结果‘S’：成功；‘P’：参数错误；‘I’：负载格式错误 
* 数据帧格式： (c0120MAC:35FFDC054D52323238780843;17&121.41.79.87$X)
  ************************************************************************************************/
char FtpIPCmdProcess( char *cmdBuf, int bufLen )  
{
	char state ;
	TcpFrame_S *psBuf = (TcpFrame_S*)cmdBuf ;
	unsigned int loadLen = (psBuf->loadLen[0]-0x30)*100 + (psBuf->loadLen[1]-0x30)*10 + (psBuf->loadLen[2]-0x30) ;
	Read_Flash_Byte(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;         //读配置
	char *backBuf = (char*)portMalloc(256) ;

	int len = snprintf(backBuf, 256, "*GET* FtpIP:[%s]!", uIapFlash.sIapFlash.FtpIP) ;
	if((psBuf->cmdCode[0] == 0x31) && ( *(&(psBuf->loadHead)+loadLen+1) == '$' ) )               //SET命令&&此命令负载长度校验成功
	{
			memset(uIapFlash.sIapFlash.FtpIP, 0, MAX_IP_LEN ) ;
			strncpy(uIapFlash.sIapFlash.FtpIP, (&(psBuf->loadHead)+1), loadLen) ;                    //将新的FtpIP拷贝到uIapFlash.sIapFlash.FtpIP中  
			Write_Flash_OnePage(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ; //保存配置
			snprintf((backBuf+len), 256-len, "*SET* FtpIP:[%s]!", uIapFlash.sIapFlash.FtpIP ) ;
			state = 'S' ; //指令执行成功
			CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
	}
	else if((psBuf->cmdCode[0] == 0x30) && ( *(&(psBuf->loadHead)+loadLen+1) == '$' ) ) //GET命令&&此命令负载长度校验成功
	{
	    state = 'S' ;                              //读指令执行成功
		  CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
	}
	else //负载（负载长度）不合法
	{
	    state = 'I' ; //指令负载中参数不合法
	}	
	portFree(backBuf) ;
  return state ;		
}

/**************************************************************************************************
* 名    称： char TcpIPCmdProcess( char *cmdBuf, int bufLen ) 
* 说    明： 解析“TcpIP”字段命令
* 入口参数：
*				 @param1  *cmdBuf: 命令数据帧指针
*        @param2   bufLen：命令数据帧长度
* 出口参数：
*       @param1 state: 指令处理结果‘S’：成功；‘P’：参数错误；‘I’：负载格式错误 
* 数据帧格式： (c0120MAC:35FFDC054D52323238780843;18&121.41.79.87$X)
  ************************************************************************************************/
char TcpIPCmdProcess( char *cmdBuf, int bufLen )  
{
	char state ;
	TcpFrame_S *psBuf = (TcpFrame_S*)cmdBuf ;
	unsigned int loadLen = (psBuf->loadLen[0]-0x30)*100 + (psBuf->loadLen[1]-0x30)*10 + (psBuf->loadLen[2]-0x30) ;
	Read_Flash_Byte(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;         //读配置
	char *backBuf = (char*)portMalloc(256) ;

	int len = snprintf(backBuf, 256, "*GET* TcpIP:[%s]!", uIapFlash.sIapFlash.TcpIP) ;
	if((psBuf->cmdCode[0] == 0x31) && ( *(&(psBuf->loadHead)+loadLen+1) == '$' ) )               //SET命令&&此命令负载长度校验成功
	{
			memset(uIapFlash.sIapFlash.TcpIP, 0, MAX_IP_LEN ) ;
			strncpy(uIapFlash.sIapFlash.TcpIP, (&(psBuf->loadHead)+1), loadLen) ;                    //将新的TcpIP拷贝到uIapFlash.sIapFlash.TcpIP中  
			Write_Flash_OnePage(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ; //保存配置
			snprintf((backBuf+len), 256-len, "*SET* TcpIP:[%s]!", uIapFlash.sIapFlash.TcpIP ) ;
			state = 'S' ; //指令执行成功
			CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
	}
	else if((psBuf->cmdCode[0] == 0x30) && ( *(&(psBuf->loadHead)+loadLen+1) == '$' ) ) //GET命令&&此命令负载长度校验成功
	{
	    state = 'S' ;                              //读指令执行成功
		  CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
	}
	else //负载（负载长度）不合法
	{
	    state = 'I' ; //指令负载中参数不合法
	}	
	portFree(backBuf) ;
  return state ;		
}

/**************************************************************************************************
* 名    称： char TcpPortCmdProcess( char *cmdBuf, int bufLen ) 
* 说    明： 解析“TcpPort”字段命令
* 入口参数：
*				 @param1  *cmdBuf: 命令数据帧指针
*        @param2   bufLen：命令数据帧长度
* 出口参数：
*       @param1 state: 指令处理结果‘S’：成功；‘P’：参数错误；‘I’：负载格式错误 
* 数据帧格式： (c0040MAC:35FFDC054D52323238780843;19&7000$X)
  ************************************************************************************************/
char TcpPortCmdProcess( char *cmdBuf, int bufLen )  
{
	char state ;
	TcpFrame_S *psBuf = (TcpFrame_S*)cmdBuf ;
	unsigned int loadLen = (psBuf->loadLen[0]-0x30)*100 + (psBuf->loadLen[1]-0x30)*10 + (psBuf->loadLen[2]-0x30) ;
	Read_Flash_Byte(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;         //读配置
	char *backBuf = (char*)portMalloc(256) ;

	int len = snprintf(backBuf, 256, "*GET* TcpPort:[%s]!", uIapFlash.sIapFlash.TcpPort) ;
	if((psBuf->cmdCode[0] == 0x31) && ( *(&(psBuf->loadHead)+loadLen+1) == '$' ) )                 //SET命令&&此命令负载长度校验成功
	{
			memset(uIapFlash.sIapFlash.TcpPort, 0, MAX_IP_LEN ) ;
			strncpy(uIapFlash.sIapFlash.TcpPort, (&(psBuf->loadHead)+1), loadLen) ;                    //将新的TcpPort拷贝到uIapFlash.sIapFlash.TcpPort中  
			Write_Flash_OnePage(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;   //保存配置
			snprintf((backBuf+len), 256-len, "*SET* TcpPort:[%s]!", uIapFlash.sIapFlash.TcpPort ) ;
			state = 'S' ; //指令执行成功
			CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
	}
	else if((psBuf->cmdCode[0] == 0x30) && ( *(&(psBuf->loadHead)+loadLen+1) == '$' ) ) //GET命令&&此命令负载长度校验成功
	{
	    state = 'S' ;                              //读指令执行成功
		  CmdResponse( cmdBuf, backBuf) ;	     //通过TCP上传命令处理回码
	}
	else //负载（负载长度）不合法
	{
	    state = 'I' ; //指令负载中参数不合法
	}	
	portFree(backBuf) ;
  return state ;		
}

char VolumeRankCmdProcess(void) 
{	
  return 'S' ;	
}

/**************************************************************************************************
* 名    称： RunResult CmdResponse(char *cmdBuf, char *format, ...)
* 说    明： 硬件回复收到的命令帧
* 入口参数：
*				 @param1  pCmdcode: 接收的命令类型
*        @param2 char *format,...  可变参变量
*				 @param3 ... ：可变参数
* 出口参数：
*       @param1 status: RunResult枚举类型数据 
* 数据帧格式：(R0193MAC:35FFDC054D52323238780843;00&*GET* IapFlag:[0]! $X)
* 注    意：  最大负载长度为：TCP_LOADBUF_MAXLEN
  ************************************************************************************************************/
RunResult CmdResponse(char *cmdBuf, char *backBuf) 
{
	  RunResult status = RUNOK ;
//	  TcpFrame_S *psBuf = (TcpFrame_S*)cmdBuf ;
//				static uint8_t counter = 0x30 ;
//				char *upDataBuf = (char*)portMalloc(TCP_LOADBUF_MAXLEN+sizeof(TcpFrame_S)+32) ;             //分配内存长度为负载长度+TcpFrame_S结构体长度 
//				TcpFrame_S *sTcpFrameData = (TcpFrame_S *)upDataBuf ;                                                       //tcp数据帧变量	   
//				int outLen = snprintf(&(sTcpFrameData->loadHead)+1 , TCP_LOADBUF_MAXLEN, "%s", backBuf ) ; 	//到此为止，所有的参数情况已经汇总到upDataBuf了 
//				if((outLen<=0)||( outLen > TCP_LOADBUF_MAXLEN)) 	 
//					{
//						ErrorLogPrintf("%s,%d:upDataBuf spillover！",__FILE__, __LINE__) ;
//						portFree(upDataBuf) ;
//						return RUNERR ;
//					}
//				sTcpFrameData->head = '(' ;
//				sTcpFrameData->loadLen[0] = outLen/100 + 0x30;
//				sTcpFrameData->loadLen[1] = outLen%100/10 + 0x30 ;
//				sTcpFrameData->loadLen[2] = outLen%10 + 0x30;
//				sTcpFrameData->frameNum = counter ;
//				counter ++ ;
//				if( counter > 0x39 )   //防止counter字符数字溢出
//				 {
//					 counter = 0x30 ;
//				 }
//				strncpy(sTcpFrameData->macHead, "MAC:", 4) ;
//				GetDeviceMacAddress(sTcpFrameData->macid, STRMACID) ; 
//				sTcpFrameData->macTail = ';' ;
//				memcpy(sTcpFrameData->cmdCode, psBuf->cmdCode, 2) ;
//				sTcpFrameData->loadHead = '&' ;
//				
//				if( psBuf->frameType == TCPCMDFRAME )   //TCP server下发的命令
//					{
//						sTcpFrameData->frameType = TCPCMDBACKFRAME ;  
//						strncat(upDataBuf, "$X)", 3) ;	 //TcpFrame_S结构体的最后三个字节拼接到	upDataBuf尾部 
//						if( RW_OK != InsertQueueMemData(&sTcp0Queue, upDataBuf, strlen(upDataBuf))) 
//							{
//								ErrorLogPrintf("%s,%d:sTcp0Queue spillover！", __FILE__, __LINE__) ;
//							}
//					}
//					else                                     //COM上位机发来的命令
//					{
//							sTcpFrameData->frameType = COMCMDBACKFRAME ;
//						  strncat(upDataBuf, "$X)", 3) ;	 //TcpFrame_S结构体的最后三个字节拼接到	upDataBuf尾部 
//						  //UARTx_SendData(UART_DEBUG, upDataBuf, strlen(upDataBuf)+1) ;
//						  AppLogPrintf(upDataBuf) ;
//					}			
//				portFree(upDataBuf) ;	
//		
		return (status) ;
}



