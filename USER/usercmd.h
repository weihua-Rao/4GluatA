#ifndef __USERCMD_H
#define __USERCMD_H
#include <string.h>
#include <stdint.h>
#include "n32g45x.h"                   
#include "mac.h"
//#include "PhoneCode.h"
#include "syslib.h"
#include "ec20module.h"
//#include "main.h"
/******************************************************************************
*内部使用的常变量定义
*******************************************************************************/
/*配置：缓冲区大小*/
#define IAPFLASHCONFIGLEN     sizeof(IAPConfig_s)            //宏定义IAPConfig_s结构体长度
#define APPFLASHCONFIGLEN     sizeof(APPConfig_s)            //宏定义APPConfig_s结构体长度
	
#define FOLDER_MAXLEN         64                             //文件夹路径的最大字符串长度
#define USERNAME_MAXLEN       64                             //用户名的最大字符串长度
#define PASSWORD_MAXLEN       64                             //用户名密码的最大字符串长度

/*配置：参数控制*/
#define IS_IapFlag_ALL_PERIPH(PERIPH) (((PERIPH) == 0x30) || \
                                       ((PERIPH) == 0x31)  )          //IAPConfig_s结构体中IapFlag参数校验

#define IS_RunAppNum_ALL_PERIPH(PERIPH) (((PERIPH) == 0x30) || \
                                         ((PERIPH) == 0x31) || \
                                         ((PERIPH) == 0x32) || \
                                         ((PERIPH) == 0x33))          //IAPConfig_s结构体中RunAppNum参数校验
																				 
#define IS_JumpResult_ALL_PERIPH(JumpResult)(((PERIPH) == 0x30) || \
                                             ((PERIPH) == 0x31)  )    //IAPConfig_s结构体中JumpResult参数校验

#define IS_VolumeRank_ALL_PERIPH(PERIPH)   (((PERIPH) >= 0x31) && \
                                            ((PERIPH) <= 0x39)  )


/******************************************************************************
*用户自定义数据类型
*******************************************************************************/
/*IAP本地FLASH存储格式*/
typedef struct
{
char     Head ;                          //固定字符：'['
char     IapFlag ;    //'0'\'1'          //BootLoader需要读，然后判断是否进行APP升级。
uint8_t  RunAppNum  ; //'1'\'2'\'3'      //BootLoader需要读，然后跳转。  APP启动开始需要改此位。
uint8_t  JumpResult ; //'0'\'1'          //APP需要读此位，然后设置app.limitRank
char     BootVers[VERSION_LEN+1] ;       //记录BootLoader版本号
char     FtpFolder[FOLDER_MAXLEN] ;      //app的bin文件存放再ftp服务器的文件夹，boot将从此文件夹下载新程序
char     FtpUsername[USERNAME_MAXLEN] ;  //FTP服务器登陆的用户名
char     FtpPassword[PASSWORD_MAXLEN] ;  //FTP服务器登陆密码
char     FtpIP[MAX_IP_LEN] ;             //FTP服务器的IP 
char     TcpIP[MAX_IP_LEN] ;             //TCP服务器的IP
char     TcpPort[PORT_MAXLEN] ;          //TCP服务器的PORT  
char     Reserve ;                       //预留,采用一页FLASH存储IAP配置信息，所以用户可根据需要增加IAPConfig_s结构体参数。 
char     Tail ;                          //固定字符：']' 
}IAPConfig_s ;                           //IAP配置信息结构体类型

/*APP本地FLASH存储格式*///5B37333131303131
typedef struct
{
char     Head ;                          //固定字符：'['
uint8_t  VolumeRank ;                    //'1'~'9'   音量大小等级
char     Tail ;                          //固定字符：']' 
}APPConfig_s ;                           //APP配置信息结构体类型

/*IAP本地FLASH存储格式共用体*/
typedef union 
{
	IAPConfig_s 	    sIapFlash ;                         //IAPConfig_s结构体类型变量
	uint8_t 	        iapFlashBuffer[IAPFLASHCONFIGLEN] ;
}IAPFlash_u ;                                           //IAP配置信息共用体类型

/*APPP本地FLASH存储格式共用体*/
typedef union 
{
	APPConfig_s 	    sAppFlash ;                         //APPConfig_s结构体类型变量
	uint8_t 	        appFlashBuffer[APPFLASHCONFIGLEN] ;
}APPFlash_u ;                                           //APP配置信息共用体类型

/******************************************************************************
*供外部使用的常变量声明
*******************************************************************************/
//extern FrameQueue_s      sUsercmdQueue ;                //定义帧环形缓冲区，用于存放用户配置命令
/******************************************************************************
*供外部使用的常变量定义
*******************************************************************************/

/*用来保存FLASH内的配置信息*/
extern IAPFlash_u uIapFlash ;             //定义IAPFlash_u共用体变量
extern APPFlash_u uAppFlash ;             //定义APPFlash_u共用体变量

/*****************************************
*供内部使用的函数声明
****************************************/
void InitIapFlashConfig( IAPFlash_u *config ) ;         //先从FLASH读出IAPFlash信息，然后对IAPFlash_u类型的全局变量初始化，最终再写入FLASH中
void InitAppFlashConfig( APPFlash_u *config ) ;         //先从FLASH读出APPFlash信息，然后对APPFlash_u类型的全局变量初始化，最终再写入FLASH中
char RestartCmdProcess( char *cmdBuf, int bufLen ) ;    //解析“软重启”字段操作命令
char GetConfigCmdProcess(char *cmdBuf, int bufLen) ;    //解析“GET_IAP”字段操作命令:获取存储在内部flash中IAPCONFIG_AREA_ADDR地址的IAP配置信息
char IapFlagCmdProcess(char *cmdBuf, int bufLen) ;      //解析“IapFlag”字段操作命令
char RunAppNumCmdProcess( char *cmdBuf, int bufLen ) ;  //解析“RunAppNum”字段操作命令
char FtpFolderCmdProcess( char *cmdBuf, int bufLen ) ;  //解析“FtpFolder”字段操作命令
char FtpUsernameCmdProcess( char *cmdBuf, int bufLen ) ;//解析“FtpUsername”字段操作命令
char FtpPasswordCmdProcess( char *cmdBuf, int bufLen ) ;//解析“FtpPassword”字段命令
char FtpIPCmdProcess( char *cmdBuf, int bufLen )  ;     //解析“FtpIP”字段命令
char TcpIPCmdProcess( char *cmdBuf, int bufLen ) ;      //解析“TcpIP”字段命令
char TcpPortCmdProcess( char *cmdBuf, int bufLen ) ;    //解析“TcpPort”字段命令
char VolumeRankCmdProcess(void) ;

/*****************************************
*对外接口函数声明
****************************************/
extern void Get_uIapFlash(IAPFlash_u *config) ;  				//从内部flash中IAPCONFIG_AREA_ADDR地址读取一页，获取FLASH中的IAP信息
extern void Set_uIapFlash(IAPFlash_u *config);   				//将新的IAP信息写入FLASH
extern void Get_uAppFlash(APPFlash_u *config) ;  				//从内部flash中APPFLASHCONFIGLEN地址读取一页，获取FLASH中的APP信息
extern void Set_uAppFlash(APPFlash_u *config) ;  				//将新的APP信息写入FLASH
extern void Cmd_Process( char *cmdBuf, int bufLen ) ;   //对用户控制命令进行处理
extern RunResult CmdResponse(char *cmdBuf, char *backBuf) ;    //硬件回复收到的命令帧


#endif
