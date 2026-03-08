#ifndef _EC20NET_H
#define _EC20NET_H

//#include "main.h"   
#include "syslib.h"   
#include "sysport.h"  

//enum eNetCmdNum 
//{
//AT_CSTT =0, AT_CIICR =1, AT_CIFSR =2, 
//AT_CGATT =3,AT_CIPMUX =4,AT_CGPADDR=5,AT_CIPQSEND=6, AT_CIPSHUT = 7
//} ; //枚举ec20模块NET相关指令 

#define  NET_CMDPACK_LEN        128       //EC20 NET相关命令字符串的最大长度
#define  APN                    "CMNET"   //APN，使用的SIM运营商不同对应的APN可能不通

/*****************************************
*内部函数声明
****************************************/
RunResult EC20_SendPDPCmd( uint8_t cmdNum, char *format,... ) ;  // EC20通过串口发送net相关命令
RunResult Config_CsttAPN(void) ;                  //配置context（会话）
//RunResult Deact_Context( uint8_t contextId ) ;                   //去激活context
//RunResult Act_Context( uint8_t contextId ) ;                     //激活context
RunResult Query_Context( uint8_t contextId, uint8_t *localIp ) ; //查询context状态
//RunResult EC20_QueryCsServiceStatus(void) ;                      //配置查询 连接CS SERVER状态
//RunResult EC20_QueryPsServiceStatus(void) ;                      //配置查询 连接PS SERVER状态

/*****************************************
*对外接口函数声明
****************************************/
//extern RunResult EC20_Query_SimIccid(char* simICCID) ;            //查询sim卡的ICCID号
//extern char*     EC20_Query_NetInfo(void) ;                       //查询网络信息
extern RunResult EC20_Query_CSQ(uint8_t *csq) ;                      //查询信号指令
extern RunResult EC20_Net_Reg(char *errInfo, uint8_t errLen) ;    //注册网络，包括查询SIMiccid、网络信息、信号质量
//extern RunResult Act_CIICR(void) ;//PDP激活并输出PDP链路IP
//extern RunResult Query_CIFSR(uint8_t *localIp );                //PDP去激活
//extern RunResult EC20_SendNetCmd( uint8_t cmdNum, char *format,... ) ;
//extern RunResult SIM7600init(char *errInfo, uint8_t errLen);
//extern RunResult SIM7600init(void);
#endif
