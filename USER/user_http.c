//#include "user_http.h" 
//#include <stdlib.h>
//#include "cjson.h"
//#include "FreeRTOS.h"
//#include "event_groups.h"
//#include "userapp.h"
//#include "key.h"
//#include "user_flash.h"
//#include "user_tcp.h"
//#include "rtc.h"
//#include "logflash.h"
#include "main.h"
/********************************************************************************
  * @file    gate_http.c
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
/*硬件注册接口 post请求相关参数宏定义*/
#define  REG_BUF    "POST http://%s%s HTTP/1.1\r\nContent-Type:application/json;charset=UTF-8\r\nHost:%s\r\ncontent-length:%d\r\n\r\n%s"  //注册接口的请求数据	
#define  REG_HOST   "quan.suning.com"     //注册接口域名，使用苏宁提供的对时接口
#define  REG_PORT   "/getSysTime.do"      //注册接口名
#define  REC_BODY   "{\"mac\":\"%s\"}"    //body:{\"mac\":\"31FFD305524E353723892251\"}


/***************postBuf**********host***********httpPort**********body***************/ 
POSTP_s sReg = {REG_BUF,       REG_HOST,        REG_PORT,        REC_BODY} ;  //注册接口的POSTP_s实例化     

/*********************************************************************************************************************
* 名    称：RunResult Reg_Post(void)
* 功    能：reg接口提交POST请求，完成硬件注册，同步RTC时间、设置RTC闹钟
* 说    明：
*               1、注册接口POST请求body:
*                  REC_BODY   "{"mac":"31FFD305524E353723892251"}" 
*               2、注册接口POST返回JSON:
*                  "{"sysTime2":"2020-10-10 10:58:44","sysTime1":"20201010105844"}"
* 注    意： 其实苏宁提供的对时接口不需要任何body数据都可以请求成功，本例程将包含硬件MAC的json数据作为请求的body发起请求有两个目的：
*            1、http中josn很常用，所以给出使用cjson打包json数据包的方法。2、在实际应用中可以通过硬件地址完成设备的授权。
*********************************************************************************************************************/
RunResult Reg_Post(void)
{
		RunResult runResult = TIMEOUT ;
		GetDeviceMacAddress((uint8_t*)gateApp.macId, STRMACID) ;
	
		cJSON * regBody = 0 ;                                                   //定义cjson
		char *  regBodyStr = 0 ;                                        		//定义指向cjson的字符串指针
		regBody = cJSON_CreateObject();	                                		    //创建根节点对象
		cJSON_AddStringToObject(regBody, "mac", (const char*)gateApp.macId);	                //向根节点加入字符串对象
		regBodyStr=cJSON_Print(regBody);    
		
				DebugLogPrintf("硬件注册JSON：%s", regBodyStr);
				runResult = Send_Post( &sReg, regBodyStr) ;
				if(runResult == RUNOK )			                                        //post请求成功		 
				 {
					 runResult =  Http_Read() ;                                       //读取post返回的数据
					 if( RUNOK == runResult )                                        //读取POST返回数据成功
						 {									
								cJSON *json=NULL, *json_time=NULL;                          //json是json对象指针,json_time是 sysTime2对象的指针
								json = cJSON_Parse((const char*)strrchr(ec20HttpBuf, '{'));	//解析数据包
								if (NULL == json) 											                    //如果解析失败
									{
										 AppLogPrintf("cjson err:%s", cJSON_GetErrorPtr()) ;
										 runResult = RUNERR ;
									}
								else                                                        //解析json成功
									{
										json_time = cJSON_GetObjectItem(json, "sysTime2") ;      //查找“sysTime2”字段
										if( (json_time != NULL) && (json_time->type == cJSON_String)) /*存在“sysTime2”字段*/
											{
                         strncpy( (char*)uCalendar.bytes, json_time->valuestring, 19) ;
												 RTC_Set( &uCalendar ) ;                                //设置时间	
												 Calendar_u  uAlaCal ;                                  //设置闹钟
												 memcpy(uAlaCal.bytes, uCalendar.bytes, CALENDAR_LEN) ;
												 uAlaCal.sCalendar.w_date[1] += 1 ; //天数+1天
												 memcpy((u8*)uAlaCal.sCalendar.hour, "01", 2) ; //凌晨1点 闹钟时、分取决于此刻的时、分值
												 //memcpy((u8*)uAlaCal.sCalendar.min,  "48", 2) ; //凌晨30分//uAlaCal.sCalendar.sec[0] += 2 ;
												 RTC_Alarm_Set(&uAlaCal) ;
												 DebugLogPrintf("%s为获取到的服务器时间.", json_time->valuestring);
											}
											else																													/*不存在“sysTime2”字段*/
											{
												runResult = RUNERR ;
												ErrorLogPrintf("No \"json_time\" .") ;
											}
										
									}
								cJSON_Delete(json);												       //释放内存
						 }
					 else //读取POST返回数据失败
						 {
						 }
					}
				else  //POST请求失败
					{
					  SETHTTPERR ;
						WriteLogToFlash("Http post err.") ;
					}				 			  
	cJSON_Delete(regBody) ;
		return runResult  ;
}





