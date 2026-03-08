#include "user_oled.h"
#include <stdlib.h>
//#include <string.h>
//#include "sysport.h"
#include "rtc.h"
#include "ec20module.h"
//#include "mac.h"
#include "usercmd.h"
#include "PhoneCode.h"
//#include "main.h" 
/********************************************************************************
  * @file    user_oled.c
  * @author  晏诚科技  Mr.Wang
  * @version V1.0.0
  * @date    11-Dec-2018
  * @brief   提供用户OLED显示接口
  ******************************************************************************
  * @attention
*******************************************************************************/

/**菜单界面如下***********************************************************************************************************/
/**************************************************************************************************
* 名    称：  void HomeMenu(void)
* 功能说明：  HOME页面，即主页页面，显示时间日期等信息
* 菜单索引：  0
  *************************************************************************************************/
extern RTC_DateType  RTC_DateStructure;
extern RTC_TimeType  RTC_TimeStructure;
extern TaskHandle_t LedTaskHandler;        //任务句柄
void HomeMenu(void)  
{	
	OledClear();
	vTaskResume(LedTaskHandler);//	恢复Led任务

	OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,   "T:       H:    ") ;
	TemperHumidity();				//第二行显示 温度 湿度
	HistoryRead(LINE3,0);		//第三行 上次报警时间
	DisplayTime();					//第四行 年月时间
//  DisplayMenuBack() ;

	if(RegisterEEpro_Slave.Host_or_Slave == Slave_Flag)	//六通道RS485从机
	{
		if(Rs485Sending.link_state == Slave_linkOK)StatusBarPrintf("RS485连接成功");
		else if(Rs485Sending.link_state == Reging_Succse)StatusBarPrintf("RS485注册成功") ;
		else if(Rs485Sending.link_state == Reging_Fail)StatusBarPrintf("RS485注册失败") ;
		else if(Rs485Sending.link_state == Slave_link_regNG)StatusBarPrintf("RS485没有注册") ;
		else if(Rs485Sending.link_state == Slave_linkNG)StatusBarPrintf("RS485连接失败") ;
		else StatusBarPrintf("RS485连接中..") ;
		
	OledShowChar(106, 0, HIGH_12,false,Hex2Ascii(RegisterEEpro_Slave.LocalAddr));//显示从机地址
	}
	else
	{
		if(RegisterEEpro_Slave.Host_or_Slave == Host_Flag)	//六通道RS485主机
		{OLED_ShowString(35, 0, false, "RS HOST", HIGH_16);Rs485Sending.link_state = Host_working;}
		else OledPrintf(LINE_MID, HIGH_16, LINE1, false, "运行中") ;//八通道独立模式
		
	DisplayCsq();		//第一行右侧显示CQS值
		
		//IOT卡类型 如果全功能 或 仅SIM卡报警 转换到仅IOT报警模式
//	if((DeivceFlag.Card_Type)&&((simANDmqtt == W433Hard.Head.DeviceAlarmMode)||(Sim_only == W433Hard.Head.DeviceAlarmMode)))
//			{W433Hard.Head.DeviceAlarmMode = mqtt_only;}
	
	if(simANDmqtt == W433Hard.Head.DeviceAlarmMode)//全功能报警
		OLED_ShowString(92, 0, false, "ALL", HIGH_12);   // 
	else if(Sim_only == W433Hard.Head.DeviceAlarmMode)//仅SIM卡
		OLED_ShowString(95, 0, false, "SIM", HIGH_12); 
		else if (mqtt_only == W433Hard.Head.DeviceAlarmMode)//仅IOT
		OLED_ShowString(95, 0, false, "IOT" ,HIGH_12); 
	else 
		OLED_ShowString(95, 0, false, "OFF", HIGH_12); //关闭报警
	
	OLED_DrawBMP(120, 0, 8, 16, false, WIFI_online[wifi_onlineState]);//0 WIFI连接成功  1 WIFI连接失败 2 ondisplay
	}
}

void SetModePage(void)  
{	
	OledClear();

	OledPrintf(LINE_MID, HIGH_16, LINE1, false, "信息设置中") ;
}
//
void TemperHumidity(void)
{
	char *oledBuf = (char*)portMalloc(10) ;

//	gcvt(SENx.T,2,oledBuf);
	gcvt(SENx.T,2,sEc20Param.Temper);
//	snprintf(oledBuf, 16, "%s",oledBuf) ; //
	OLED_ShowString(18, 2, false, sEc20Param.Temper, HIGH_16);//温度 oledBuf
	gcvt(SENx.RH,2,oledBuf);
	snprintf(sEc20Param.Humidi, 6, "%s",oledBuf) ; //	oledBuf
	OLED_ShowString(88, 2, false, sEc20Param.Humidi, HIGH_16);	//88湿度		
	portFree(oledBuf) ;
}
void HistoryAlarmNull(void)
{
	OledClear();
	OledPrintf(LINE_MID, HIGH_16, LINE1, true,   "报警记录") ;
//	OledPrintf(LINE_MID, HIGH_16, LINE2, true,   "空") ;
}
//功能：显示当前页面报警记录 UserKey.CurrentPage
//显示历史报警记录页面
//每页最多显示3条
//1~4页
ErrorStatus HistoryAlarmPage1(void)
{
u8 HistoryInforAddr;	
	OledClear();
	OledPrintf(LINE_MID, HIGH_16, LINE1, true,   "报警记录  %d",UserKey.CurrentPage ) ;
	for(u8 i=0; i<3; i++)		//,UserKey.CurrentPage++
	{
		HistoryInforAddr = i+(UserKey.CurrentPage-1)*3;//读取当前页面存储的信息 地址数组的下标号 每页的起始分别是 0 3 6 9
		if(HistoryInforAddr > 9)return ERROR;//大于9 溢出
	if(ERROR == HistoryRead((OLEDLINE_e)(i+1),HistoryInforAddr))	//	
		{
		UserKey.CurrentPage = 4;	return ERROR;//HistoryAlarmNull();
		}	
	}
return SUCCESS ;	
}
//温度 湿度显示报警数值
ErrorStatus TemperHum_Disp(void )
{
//u8 PortNum = 	(UserKey.CurrentPage - 5) / 5;
//u8 StrLen;	
	OledClear();
//char *strBuf = portMalloc(10*sizeof(char));
	
OledPrintf(LINE_MID, HIGH_16, LINE1, true,   "温湿度报警值") ;
//memset(AlarmValue.ValueArry,0,TemHumSaveLen);//
//EEPROM_read_n((MessgeAddr[PortNum]+TemperHum_AddrOffset),AlarmValue.ValueArry, TemHumSaveLen);   //长度
		
if(AlarmValue.ValueUnit.Temper_H == 0xffffffff)OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,   "Th: 无") ;
	else OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,   "Th:%d ",AlarmValue.ValueUnit.Temper_H) ;
if(AlarmValue.ValueUnit.Temper_L == 0xffffffff)OledPrintf(LINE_RIGHT, HIGH_16, LINE2, false,   "Tl: 无") ;
	else OledPrintf(LINE_RIGHT, HIGH_16, LINE2, false,   "Tl:%d ",AlarmValue.ValueUnit.Temper_L) ;	
	
if(AlarmValue.ValueUnit.Humidi_H == 0xffffffff)OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,   "Hh: 无") ;
	else OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,   "Hh:%d ",AlarmValue.ValueUnit.Humidi_H) ;
if(AlarmValue.ValueUnit.Humidi_L == 0xffffffff)OledPrintf(LINE_RIGHT, HIGH_16, LINE3, false,   "Hl: 无") ;
	else OledPrintf(LINE_RIGHT, HIGH_16, LINE3, false,   "Hl:%d ",AlarmValue.ValueUnit.Humidi_L) ;			 	
//OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,   "Th:%d   Tl:%d  ",AlarmValue.ValueUnit.Temper_H,AlarmValue.ValueUnit.Temper_L) ;
//OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,   "Hh:%d   Hl:%d  ",AlarmValue.ValueUnit.Humidi_H,AlarmValue.ValueUnit.Humidi_L) ;

//portFree (strBuf);		
return SUCCESS ;	 
}
//显示用户设置的信息
//每行显示1组手机号 每页最多3组号码
//每回路分5页显示 2页短信被叫号码 2页电话被叫号码 1页报警短信 闭合短信+断开短信
//DisPage 当前的1/2还是2/2页面， PortNum 1~8回路编号
ErrorStatus UserInforPage1(char DisPage,u8 PortNum )
{
//ErrorStatus state;
//u8 first = 0;	
u16 *Address;
//char DisPage = (UserKey.CurrentPage-4) % 5;	
//u8 PortNum = 	(UserKey.CurrentPage - 5) / 5;
u8 StrLen;	
		if(PortNum > Alarm_maxGroup)return ERROR ;//溢出报错
	OledClear();
	
	if((DisPage == 1)||(DisPage == 2))
	{
	if(UserKey.CurrentPage > 15)OledPrintf(LINE_LEFT, HIGH_16, LINE1, true,   "短信号码 %d/2 %d路",DisPage, PortNum-1) ;	
	else if(UserKey.CurrentPage > 8)OledPrintf(LINE_LEFT, HIGH_16, LINE1, true,   "供电短信号码 %d/2",DisPage) ;		
	else OledPrintf(LINE_LEFT, HIGH_16, LINE1, true,   "温湿度短信 %d/2",DisPage) ;	
	Address = (u16*)SMS_Number_Offset;
	}	
	else if((DisPage == 3)||(DisPage == 4))
	{
	if(UserKey.CurrentPage>15)OledPrintf(LINE_LEFT, HIGH_16, LINE1, true,   "电话号码 %d/2 %d路",DisPage-2,PortNum-1) ;	
	else if(UserKey.CurrentPage>9)OledPrintf(LINE_LEFT, HIGH_16, LINE1, true,   "供电电话号码 %d/2",DisPage-2) ;	
	else 	OledPrintf(LINE_LEFT, HIGH_16, LINE1, true,   "温湿度电话 %d/2",DisPage-2) ;	
	Address = (u16*)Call_Number_Offset;	
	}
/*************显示当前回路是否有信息 闭合+断开*******************/	
	else// if(DisPage == 5)				
	{
char *page_stype_exist,*page_stype_NUll	;	
	if(UserKey.CurrentPage>15){OledPrintf(LINE_MID, HIGH_16, LINE1, true,   "报警短信    %d:路",PortNum-1);
															page_stype_exist = "闭合短信：";page_stype_NUll = "断开短信：";}
	
	else if(UserKey.CurrentPage>10){OledPrintf(LINE_MID, HIGH_16, LINE1, true,   "供电报警短信") ;
															page_stype_exist = "断电短信：";page_stype_NUll = "供电短信：";}
	
	else {OledPrintf(LINE_MID, HIGH_16, LINE1, true,   "温湿度报警短信") ;	
															page_stype_exist = "报警短信";page_stype_NUll = "无警短信";}
	StrLen = EEPROM_read_Byte(MessgeAddr[PortNum]+Close_Message_offset);	
		
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, false, page_stype_exist);
		if((2 < StrLen) && (StrLen < 0xff))//消息地址首位，长度判断是否有信息
		{
			OledPrintf(LINE_RIGHT, HIGH_16, LINE2, false, "有");
		}		
	 else {OledPrintf(LINE_RIGHT, HIGH_16, LINE2, false, "无");}

	StrLen = EEPROM_read_Byte(MessgeAddr[PortNum]+Open_Message_offset);
	OledPrintf(LINE_LEFT, HIGH_16, LINE3, false, page_stype_NUll); 
	if((2 < StrLen) && (StrLen < 0xff))//消息地址首位，长度判断是否有信息
		{
			OledPrintf(LINE_RIGHT, HIGH_16, LINE3, false, "有");
		}		
	 else {OledPrintf(LINE_RIGHT, HIGH_16, LINE3, false, "无");}
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, " 详细发短信查询");
return SUCCESS ;	 
	}
/*****************************************************/
//	else if(DisPage == 5)				/*显示闭合报警信息*/
//	{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE1, true,   "闭合信息   %d:路",PortNum + 1) ;
////	if(SUCCESS == GetAlarmMassage(MessgeAddr[PortNum],Close_Message_offset))
//	StrLen = EEPROM_read_Byte(MessgeAddr[PortNum]+Close_Message_offset);	
//		if((2 < StrLen) && (StrLen < 0xff))//消息地址首位，长度判断是否有信息
//		{
//	//		memset(MsegTemp,0,Mseg_Lenth);
//			EEPROM_read_n((MessgeAddr[PortNum]+Close_Message_offset+1),MsegTemp,StrLen);	//读取信息
//			OledPrintf(LINE_LEFT, HIGH_16, LINE2, false, "%s; ", MsegTemp);return SUCCESS ;
//		}		
//	return ERROR ;
//	}
//	else					/*显示断开报警信息*/
//	{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE1, true,   "断开信息   %d:路",PortNum + 1) ;
//	StrLen = EEPROM_read_Byte(MessgeAddr[PortNum]+Open_Message_offset);	
//		if((2 < StrLen) && (StrLen < 0xff))//消息地址首位，长度判断是否有信息
//		{
//	//		memset(MsegTemp,0,Mseg_Lenth);
//			EEPROM_read_n((MessgeAddr[PortNum]+Open_Message_offset+1),MsegTemp,StrLen);	//读取信息
//			OledPrintf(LINE_LEFT, HIGH_16, LINE2, false, "%s; ", MsegTemp);return SUCCESS ;
//		}		
//	return ERROR ;
//	}

	/*显示短信电话接收号码*/
	char *oledBuf = (char*)portMalloc(50) ;	
	u8 LineNo;
	if((DisPage == 2)||(DisPage == 4))LineNo = 3;//短信 电话号码 换页显示行号
	else LineNo = 0;
	for(u8 i=0; i<3; i++)
	 {
		memset(PhoneNumber,0,PhoneLenght);
		u8 PhoneNumLen = EEPROM_read_Byte(MessgeAddr[PortNum]+Address[i+LineNo]);
		if((PhoneNumLen > 2)&&(PhoneNumLen < 15))//地址首位，长度标识位	
			EEPROM_read_n((MessgeAddr[PortNum]+Address[i+LineNo]+1), PhoneNumber, PhoneNumLen);   //号码长度		
		
		OledPrintf(LINE_LEFT, HIGH_16, (OLEDLINE_e)(i+1), true, "NO%d:%s",i+LineNo+1,(const char*)&PhoneNumber) ;
	 }
	portFree(oledBuf) ;
return SUCCESS ;	
}
////显示用户设置的信息
////每行显示2组手机号 每页最多6组号码
////第5页短信被叫号码 第6页电话被叫号码
//ErrorStatus UserInforPage1(void)
//{
////ErrorStatus state;
//u8 first = 0;	
//u32 *Address;
//char DisPage = UserKey.CurrentPage % 4;	
//u8 PortNum = 	(UserKey.CurrentPage - 5) / 4;
//	OledClear();
//	
//	if(DisPage == 1)
//	{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE1, true,   "短信号码   %d:路",PortNum + 1) ;
//	Address = (u32*)SMS_Number_Offset;
//	}	
//	else if(DisPage == 2)
//	{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE1, true,   "电话号码   %d:路",PortNum + 1) ;
//	Address = (u32*)Call_Number_Offset;	
//	}
//	else if(DisPage == 3)				/*显示闭合报警信息*/
//	{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE1, true,   "闭合信息   %d:路",PortNum + 1) ;
//		/*显示闭合报警信息*/	
//	if(SUCCESS == GetAlarmMassage(MessgeAddr[PortNum],Close_Message_offset))
//	OledPrintf(LINE_LEFT, HIGH_16, LINE2, false, "%s; ", MsegTemp);
//	return SUCCESS ;
//	}
//	else					/*显示断开报警信息*/
//	{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE1, true,   "断开信息   %d:路",PortNum + 1) ;
//	/*显示断开报警信息*/	
//	if(SUCCESS == GetAlarmMassage(MessgeAddr[PortNum],Open_Message_offset))
//	OledPrintf(LINE_LEFT, HIGH_12, LINE2, false, "%s; ", MsegTemp);		
//	return SUCCESS ;
//	}

//	/*显示短信电话接收号码*/
//	char *oledBuf = (char*)portMalloc(50) ;	
//	for(u8 i=0; i<6; i++)
//	{
//		memset(PhoneNumber,0,Mseg_Lenth);
//		u8 PhoneNumLen = EEPROM_read_Byte(MessgeAddr[PortNum]+Address[i]);
//		if((PhoneNumLen > 2)&&(PhoneNumLen < 15))//地址首位，长度标识位	
//			EEPROM_read_n((MessgeAddr[PortNum]+Address[i]+1), PhoneNumber, PhoneNumLen);   //号码长度		
//		
//		if(first++ == 0)//每行显示2组手机号 第1行
//			OledPrintf(LINE_LEFT, HIGH_12, (OLEDLINE_e)((i/2)+1), true, "%s",(const char*)&PhoneNumber) ;
//		else//每行显示2组手机号 第2行
//		{
//			first = 0;
//			OledPrintf(LINE_RIGHT, HIGH_12, (OLEDLINE_e)((i/2)+1), true, "%s",(const char*)&PhoneNumber) ;			
//		}
//	}
//	portFree(oledBuf) ;
//return SUCCESS ;	
//}
/**************************************************************************************************
* 名    称：  void Sub1Select1(uint8_t select)
* 功能说明：  一级子菜单界面，选中第一项
* 菜单索引：  1
  *************************************************************************************************/
void Sub1Select1(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,   "FTP 信息") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,  "TCP 信息") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false,  "设备升级") ;
}
/**************************************************************************************************
* 名    称：  void Sub1Select2(uint8_t select)
* 功能说明：  一级子菜单界面，选中第2项
* 菜单索引：  2
  *************************************************************************************************/
void Sub1Select2(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,   "FTP 信息") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, true,    "TCP 信息") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false,   "设备升级") ;
}
/**************************************************************************************************
* 名    称：  void Sub1Select3(uint8_t select)
* 功能说明：  一级子菜单界面，选中第3项
* 菜单索引：  3
  *************************************************************************************************/
void Sub1Select3(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,  "FTP 信息") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,  "TCP 信息") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, true,   "设备升级") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Select4(uint8_t select)
* 功能说明：  一级子菜单界面，选中第4项
* 菜单索引：  4
  *************************************************************************************************/
void Sub1Select4(void)
{
  OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,  "TCP 信息") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,  "设备升级") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, true,   "设备信息") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Select5(uint8_t select)
* 功能说明：  一级子菜单界面，选中第5项
* 菜单索引：  27
  *************************************************************************************************/
void Sub1Select5(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,  "设备升级") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,  "设备信息") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, true,   "GPS 信息") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Click1Select1(uint8_t select)
* 功能说明：  一级子菜单界面，点击第一项后显示的“二级子菜单”选择第一项
* 菜单索引：  5
  *************************************************************************************************/
void Sub1Click1Select1(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,   "查询FTP账号") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,  "查询FTP密码") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false,  "查询FTP  IP") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Click1Select2(uint8_t select)
* 功能说明：  一级子菜单界面，点击第一项后显示的“二级子菜单”选择第二项
* 菜单索引：  6
  *************************************************************************************************/
void Sub1Click1Select2(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,   "查询FTP账号") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, true,    "查询FTP密码") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false,   "查询FTP  IP") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Click1Select3(uint8_t select)
* 功能说明：  一级子菜单界面，点击第一项后显示的“二级子菜单”选择第三项
* 菜单索引：  7
  *************************************************************************************************/
void Sub1Click1Select3(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,   "查询FTP账号") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,   "查询FTP密码") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, true,    "查询FTP  IP") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Click1Select4(uint8_t select)
* 功能说明：  一级子菜单界面，点击第一项后显示的“二级子菜单”选择第四项
* 菜单索引：  8
  *************************************************************************************************/
void Sub1Click1Select4(void)
{

  OledPrintf(LINE_LEFT, HIGH_16,   LINE2,   false,   "查询FTP密码") ;
	OledPrintf(LINE_LEFT, HIGH_16,   LINE3,   false,   "查询FTP  IP") ;
		OledPrintf(LINE_LEFT, HIGH_16, LINE4,   true,    "查询FTP路径") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Click1Select1(uint8_t select)
* 功能说明：  一级子菜单界面，点击第二项后显示的“二级子菜单”选择第一项
* 菜单索引：  9
  *************************************************************************************************/
void Sub1Click2Select1(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,   "查询TCP  IP") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,  "查询TCP端口") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false,  " ") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Click1Select2(uint8_t select)
* 功能说明：  一级子菜单界面，点击第二项后显示的“二级子菜单”选择第二项
* 菜单索引：  10
  *************************************************************************************************/
void Sub1Click2Select2(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,   "查询TCP  IP") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, true,    "查询TCP端口") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false,   " ") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Click1Select1(uint8_t select)
* 功能说明：  一级子菜单界面，点击第三项后显示的“二级子菜单”选择第一项
* 菜单索引：  11
  *************************************************************************************************/
void Sub1Click3Select1(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,   "查询当前APP") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,  "立即升级APP") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false,  " ") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Click1Select2(uint8_t select)
* 功能说明：  一级子菜单界面，点击第三项后显示的“二级子菜单”选择第二项
* 菜单索引：  12
  *************************************************************************************************/
void Sub1Click3Select2(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,   "查询当前APP") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, true,    "立即升级APP") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false,   " ") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Click1Select1(uint8_t select)
* 功能说明：  一级子菜单界面，点击第四项后显示的“二级子菜单”选择第一项
* 菜单索引：  13
  *************************************************************************************************/
void Sub1Click4Select1(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,   "查询MACID") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,  "查询APP 版本") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false,  "查询BOOT版本") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Click1Select2(uint8_t select)
* 功能说明：  一级子菜单界面，点击第四项后显示的“二级子菜单”选择第二项
* 菜单索引：  14
  *************************************************************************************************/
void Sub1Click4Select2(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,   "查询MACID") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, true,    "查询APP 版本") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false,   "查询BOOT版本") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Click1Select2(uint8_t select)
* 功能说明：  一级子菜单界面，点击第四项后显示的“二级子菜单”选择第三项
* 菜单索引：  15
  *************************************************************************************************/
void Sub1Click4Select3(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,   "查询MACID") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,   "查询APP 版本") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, true,    "查询BOOT版本") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Click5Select1(uint8_t select)
* 功能说明：  一级子菜单界面，点击第五项后显示的“二级子菜单”选择第一项
* 菜单索引：  17
  *************************************************************************************************/
void Sub1Click5Select1(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,   "经纬度") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,  "角方向") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false,  "速度值") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Click5Select2(uint8_t select)
* 功能说明：  一级子菜单界面，点击第五项后显示的“二级子菜单”选择第二项
* 菜单索引：  18
  *************************************************************************************************/
void Sub1Click5Select2(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,   "经纬度") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, true,    "角方向") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false,   "速度值") ;
}


void Sub1Click5Select3(void)
{
	OledPrintf(LINE_LEFT, HIGH_16, LINE2, false,   "经纬度") ;
  OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,   "角方向") ;
	OledPrintf(LINE_LEFT, HIGH_16, LINE4, true,    "速度值") ;
}

/**************************************************************************************************
* 名    称：  void Sub1Click1Click1(uint8_t select)
* 功能说明：  一级子菜单界面，点击第一项后显示的“二级子菜单”点击第一项后的动作
* 菜单索引：  16  
  *************************************************************************************************/
//void Sub1Click1Click1(void)
//{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,  "FTP账号:") ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE3, false, uIapFlash.sIapFlash.FtpUsername ) ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, " " ) ;	
//}

/**************************************************************************************************
* 名    称：  void Sub1Click1Click2(uint8_t select)
* 功能说明：  一级子菜单界面，点击第一项后显示的“二级子菜单”点击第二项后的动作
* 菜单索引：  17  
  *************************************************************************************************/
//void Sub1Click1Click2(void)
//{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,  "FTP密码:") ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE3, false, uIapFlash.sIapFlash.FtpPassword ) ;
//	if( strlen( uIapFlash.sIapFlash.FtpFolder) >  16 )  //单行不够显示
//	  {
//	    	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, &uIapFlash.sIapFlash.FtpFolder[16] ) ;	
//		}
//	else
//	  {
//	      OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, " " ) ;		
//		}			
//}

/**************************************************************************************************
* 名    称：  void Sub1Click1Click3(uint8_t select)
* 功能说明：  一级子菜单界面，点击第一项后显示的“二级子菜单”点击第三项后的动作
* 菜单索引：  18  
  *************************************************************************************************/
//void Sub1Click1Click3(void)
//{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,  "FTP IP:") ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE3, false, uIapFlash.sIapFlash.FtpIP ) ;
//	if( strlen( uIapFlash.sIapFlash.FtpFolder) >  16 )  //单行不够显示
//	  {
//	    	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, &uIapFlash.sIapFlash.FtpFolder[16] ) ;	
//		}
//	else
//	  {
//	      OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, " " ) ;		
//		}			
//}

/**************************************************************************************************
* 名    称：  void Sub1Click1Click3(uint8_t select)
* 功能说明：  一级子菜单界面，点击第一项后显示的“二级子菜单”点击第四项后的动作
* 菜单索引：  19  
  *************************************************************************************************/
//void Sub1Click1Click4(void)
//{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,  "FTP路径:") ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE3, false, uIapFlash.sIapFlash.FtpFolder ) ;
//	if( strlen( uIapFlash.sIapFlash.FtpFolder) >  16 )  //单行不够显示
//	  {
//	    	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, &uIapFlash.sIapFlash.FtpFolder[16] ) ;	
//		}
//	else
//	  {
//	      OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, " " ) ;		
//		}			
//}

/**************************************************************************************************
* 名    称：  void Sub1Click2Click1(uint8_t select)
* 功能说明：  一级子菜单界面，点击第二项后显示的“二级子菜单”点击第一项后的动作
* 菜单索引：  20  
  *************************************************************************************************/
//void Sub1Click2Click1(void)
//{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,  "TCP IP:") ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE3, false, uIapFlash.sIapFlash.TcpIP ) ;
//	if( strlen( uIapFlash.sIapFlash.FtpFolder) >  16 )  //单行不够显示
//	  {
//	    	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, &uIapFlash.sIapFlash.FtpFolder[16] ) ;	
//		}
//	else
//	  {
//	      OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, " " ) ;		
//		}			
//}

/**************************************************************************************************
* 名    称：  void Sub1Click2Click2(uint8_t select)
* 功能说明：  一级子菜单界面，点击第二项后显示的“二级子菜单”点击第二项后的动作
* 菜单索引：  21  
  *************************************************************************************************/
//void Sub1Click2Click2(void)
//{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,  "TCP端口:") ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE3, false, uIapFlash.sIapFlash.TcpPort ) ;
//	if( strlen( uIapFlash.sIapFlash.FtpFolder) >  16 )  //单行不够显示
//	  {
//	    	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, &uIapFlash.sIapFlash.FtpFolder[16] ) ;	
//		}
//	else
//	  {
//	      OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, " " ) ;		
//		}			
//}

/**************************************************************************************************
* 名    称：  void Sub1Click3Click1(uint8_t select)
* 功能说明：  一级子菜单界面，点击第三项后显示的“二级子菜单”点击第一项后的动作
* 菜单索引：  22  
  *************************************************************************************************/
//void Sub1Click3Click1(void)
//{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,  "当前运行APP号:") ;
//	char *currentAPP = (char*)portMalloc(16) ;
//	snprintf(currentAPP, 16, "RunAppNum:[%c]。", uIapFlash.sIapFlash.RunAppNum ) ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE3, false, currentAPP ) ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, " " ) ;
//	portFree(currentAPP) ;		
//}

/**************************************************************************************************
* 名    称：  void Sub1Click3Click2(uint8_t select)
* 功能说明：  一级子菜单界面，点击第三项后显示的“二级子菜单”点击第二项后的动作
* 菜单索引：  23  
  *************************************************************************************************/
//void Sub1Click3Click2(void)
//{
//	uIapFlash.sIapFlash.IapFlag = 0x31 ; //改配置：将负载值赋值给uIapFlash.sIapFlash.IapFlag      
//	Write_Flash_OnePage(IAPCONFIG_AREA_ADDR, uIapFlash.iapFlashBuffer, IAPFLASHCONFIGLEN ) ;      //保存配置
//	AppLogPrintf("升级设备，即将重启") ;
//	Wait_For_Nms(2) ;
//	NVIC_SystemReset();//SystemSoftReset() ; 			
//}

/**************************************************************************************************
* 名    称：  void Sub1Click3Click1(uint8_t select)
* 功能说明：  一级子菜单界面，点击第四项后显示的“二级子菜单”点击第一项后的动作
* 菜单索引：  24  
  *************************************************************************************************/
//void Sub1Click4Click1(void)
//{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,   "MACID:") ;
//	char *macid = (char*)portMalloc( MAC_BYTES_LEN+1 ) ;
//	GetDeviceMacAddress((uint8_t*)macid, STRMACID) ; 
//  OledPrintf(LINE_LEFT, HIGH_12, LINE3, false,  macid ) ; //显示12字高，最多可以显示21个字符
//	OledPrintf(LINE_LEFT, HIGH_12, LINE4, false,  &macid[21] ) ;
//	portFree(macid) ;		
//}

/**************************************************************************************************
* 名    称：  void Sub1Click3Click2(uint8_t select)
* 功能说明：  一级子菜单界面，点击第四项后显示的“二级子菜单”点击第二项后的动作
* 菜单索引：  25  
  *************************************************************************************************/
//void Sub1Click4Click2(void)
//{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,  "APP 版本号:") ;
//	char *version = (char*)portMalloc(VERSION_LEN+1) ;
//	Query_AppVersion(version) ;
//  OledPrintf(LINE_LEFT, HIGH_16, LINE3, false, version) ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, " ") ;
//	portFree(version) ; 			
//}
/**************************************************************************************************
* 名    称：  void Sub1Click3Click1(uint8_t select)
* 功能说明：  一级子菜单界面，点击第四项后显示的“二级子菜单”点击第三项后的动作
* 菜单索引：  26  
  *************************************************************************************************/
//void Sub1Click4Click3(void)
//{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,  "BOOT版本号:") ;
//  OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,   uIapFlash.sIapFlash.BootVers) ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, " ") ;	
//}

/**************************************************************************************************
* 名    称：  void Sub1Click5Click1(uint8_t select)
* 功能说明：  一级子菜单界面，点击第五项后显示的“二级子菜单”点击第一项后的动作
* 菜单索引：  31  
  *************************************************************************************************/
//void Sub1Click5Click1(void)
//{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,  "经纬度:") ;
//	if( (sRMCData.longitude[0] == 0x00) && (sRMCData.latitude[0] == 0x00 ) )
//	{
//			OledPrintf(LINE_LEFT, HIGH_16, LINE3, false, "GPS信号弱...") ;
//			OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, " ") ;			
//	}
//	else
//	{
//			OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, "%s%c", sRMCData.longitude, sRMCData.eLongitudeDirect ) ;	
//	    OledPrintf(LINE_LEFT, HIGH_16, LINE3, false, "%s%c", sRMCData.latitude, sRMCData.eLatitudeDirect ) ;	
//	}
//}

/**************************************************************************************************
* 名    称：  void Sub1Click5Click2(uint8_t select)
* 功能说明：  一级子菜单界面，点击第五项后显示的“二级子菜单”点击第二项后的动作
* 菜单索引：  32  
  *************************************************************************************************/
//void Sub1Click5Click2(void)
//{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,  "角方向:") ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE3, false, "%s 度", sRMCData.angDirect ) ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, " " ) ;			
//}

/**************************************************************************************************
* 名    称：  void Sub1Click5Click3(uint8_t select)
* 功能说明：  一级子菜单界面，点击第五项后显示的“二级子菜单”点击第三项后的动作
* 菜单索引：  33  
  *************************************************************************************************/
//void Sub1Click5Click3(void)
//{
//	OledPrintf(LINE_LEFT, HIGH_16, LINE2, true,  "速度值:") ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE3, false, "%s 节", sRMCData.speed ) ;
//	OledPrintf(LINE_LEFT, HIGH_16, LINE4, false, " " ) ;
//}

/**菜单界面如上***********************************************************************************************************/

/**************************************************************************************************
* 名    称：  void StatusBarPrintf(char *pData) 
* 功能说明：  OLED第一行显示字符串
* 入口参数：   
*            @param1 *pData  显示内容的字符串指针
**************************************************************************************************/
void StatusBarPrintf(char *pData)
{
//	OledClearLine(LINE1);
	OledPrintf(LINE_LEFT, HIGH_16, LINE1, false, pData) ;                        //OLED第二行显示标题/进度
}

/**************************************************************************************************
* 名    称：  void DisplayTime(void)
* 功能说明：  OLED第二行显示时、分；第三行显示年、月、日。
**************************************************************************************************/
void DisplayTime(void)
{
		char *rtcTime = portMalloc(30) ;
	
//	RTC_DateShow();RTC_TimeShow();
//	if(RTC_TimeStructure.Seconds > 59)RTC_TimeStructure.Seconds = 59;	//临时调试用
//		strncpy (rtcTime, (const char*)&uCalendar.bytes[11] , 5  ) ;
	snprintf(rtcTime,30,"20%02d-%02d-%02d   %02d:%02d:%02d",RTC_DateStructure.Year,RTC_DateStructure.Month,
	RTC_DateStructure.Date,RTC_TimeStructure.Hours,RTC_TimeStructure.Minutes,RTC_TimeStructure.Seconds);
//		OledPrintf(LINE_MID, HIGH_12, LINE3, false, "20%0.2d-%0.2d-%0.2d",RTC_DateStructure.Year,RTC_DateStructure.Month,RTC_DateStructure.Date) ;	
//		OledPrintf(LINE_MID, HIGH_16, LINE2, false, rtcTime) ;
//		memset(rtcTime, 0, 30) ;
//		strncpy (rtcTime, (const char*)&uCalendar.bytes, 10  ) ;
//		OledPrintf(LINE_MID, HIGH_16, LINE2, false, "%0.2d-%0.2d-%0.2d",RTC_TimeStructure.Hours,RTC_TimeStructure.Minutes,RTC_TimeStructure.Seconds) ;	
//		OledPrintf(LINE_MID, HIGH_12, LINE3, false, rtcTime) ;		 

	OLED_ShowString(0, 6, false, rtcTime, HIGH_12);	//X Y 128*64
	portFree(rtcTime) ;
}

/**************************************************************************************************
* 名    称：  void DisplayPointBlink(void)
* 功能说明：  OLED第二行显示时、分中间':'符号闪烁
**************************************************************************************************/
void DisplayPointBlink(void)
{
	  static unsigned int times = 0 ;
		if( times%2 == 0)
		 OLED_ShowString(74, 2, false, " ", HIGH_16) ;
		else
		 OLED_ShowString(74, 2, false, ":", HIGH_16) ;
		times ++ ;
}

/**************************************************************************************************
* 名    称：  void DisplayMenuBack&Back(void)
* 功能说明：  OLED第四行显示"MENU             BACK"
**************************************************************************************************/
void DisplayMenuBack(void)
{
   OledPrintf(LINE_LEFT, HIGH_12, LINE4, false, "MENU             BACK") ;
}

/**************************************************************************************************
* 名    称：  void DisplayStatusBar(void)
* 功能说明：  OLED第一行右侧区域显示信号图片并显示CSQ值,OLED第一行右侧区域显示电池图片并显示电压值。
**************************************************************************************************/
//void DisplayStatusBar(void)
//{
//		char *oledBuf = (char*)portMalloc(16) ;
//		snprintf(oledBuf, 2, "%s", sEc20Param.csq) ;        //CSQ%s显示屏显示csq
//		DisplayCsq(oledBuf) ;
//		portFree(oledBuf) ;
//}

/**************************************************************************************************
* 名    称：  void DisplayCsq(char *pData) 
* 功能说明：  OLED第一行左侧区域显示信号图片并显示CSQ值。
* 入口参数：   
*            @param1 *pData  显示内容的字符串指针
**************************************************************************************************/
void DisplayCsq(void)//char *pData
{
//	   OLED_ShowString(0, 0, false, (char *)"     ", HIGH_16);
	   int csq = atoi((const char*)sEc20Param.csq) ;
	   if( csq >= 28)
	     OLED_DrawBMP(0, 0, 32, 16, false, csqHighBmp) ;
		 else if(csq < 23)
			 OLED_DrawBMP(0, 0, 32, 16, false, csqLowBmp) ;
		 else
			 OLED_DrawBMP(0, 0, 32, 16, false, csqMidBmp) ;
		 OLED_ShowString(24, 0, false, (char*)sEc20Param.csq, HIGH_12);  //23pData显示“CSQ:26”
}


/**************************************************************************************************
* 名    称：  void DisplayBattery(char *pData)
* 功能说明：  OLED第一行中间区域显示GPS图标 表示已经定位就绪。
* 入口参数：   
*            @param1 *pData  显示内容的字符串指针
**************************************************************************************************/
void DisplayGnss(void)
{
	//   OLED_DrawBMP(32, 0, 32, 16, false, batBmp) ;
}

/**************************************************************************************************
* 名    称：  void DisplayBattery(char *pData)
* 功能说明：  OLED第一行右侧区域显示电池图片并显示电压值。
* 入口参数：   
*            @param1 *pData  显示内容的字符串指针
**************************************************************************************************/
//void DisplayBattery(char *pData)
//{
//	   OLED_ShowString(64, 0, false, (char *)"        ", HIGH_16);
//	   OLED_DrawBMP(64, 0, 32, 16, false, batBmp) ;
//		 OLED_ShowString(64+26, 0, false, pData, HIGH_12);
//}

/**************************************************************************************************
* 名    称：  void DisplayTitle(char *pData) 
* 功能说明：  OLED第二行显示标题/进度,OLED第三行清空
* 入口参数：   
*            @param1 *pData  显示内容的字符串指针
**************************************************************************************************/
void DisplayTitle(char *pData)
{
	   OledPrintf(LINE_LEFT, HIGH_16, LINE2, false, pData) ;                        //OLED第二行显示标题/进度
	   OledClearLine(LINE3) ;                                                       //OLED第三行清空
}

/**************************************************************************************************
* 名    称：  void DisplayInfo(char *pData) 
* 功能说明：  OLED第三行显示标题/进度的内容信息。
* 入口参数：   
*            @param1 *pData  显示内容的字符串指针
**************************************************************************************************/
void DisplayInfo(char *pData)
{
		 OledPrintf(LINE_MID, HIGH_16, LINE3, false, pData) ;                        //OLED第三行显示标题/进度的内容信息
}

/**************************************************************************************************
* 名    称：  void DisplayWarning(char *pSecond, char *pThird)
* 功能说明：  显示警告信息。
* 入口参数：  
*            @param1  *pSecond  第二行显示的内容指针   
*            @param2  *pThird   第三行显示的内容指针
**************************************************************************************************/
void DisplayWarning(char *pSecond, char *pThird)
{
		 OledPrintf(LINE_LEFT, HIGH_16, LINE2, false, pSecond) ;                        //OLED第二行显示标题/进度
		 OledPrintf(LINE_LEFT, HIGH_16, LINE3, false, pThird) ;                         //OLED第三行显示标题/进度的内容信息
}

