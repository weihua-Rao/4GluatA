#ifndef _PHONECODE_H_
#define _PHONECODE_H_
 #include "oled.h"
#include "main.h"
//#include "n32g45x.h"
//#include "syslib.h"
#ifdef 	Main_def
#define Extern_def  
#else
#define Extern_def extern
#endif

#define countof(a) (sizeof(a) / sizeof(*(a)))
	
//#define only_oneCall			/*条件编译 客户个性化:当有接听电话时 后面号码不再拨打电话 开启下一个警情事件*/
	
/** 位带操作 */
// 根据公式将位带区寄存器转换成对应的位带别名区的地址
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
//#define BITBAND(addr, bitnum) (0x42000000+0x10C14*32+(12<<2))
// 将地址转换成指针的形式，方便赋值
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr))

// 封装好要操作的寄存器地址和比特位
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))

/** 列出各个GPIO口的输入输出寄存器地址映射 */ 
// 输出
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x14 0x40010814 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C14
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x40011014 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x40011414 
//#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
//#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
//#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

// 输入
#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x10 0x40010810 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C10 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011010 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011410 
//#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
//#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
//#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 

/** 
  * IO口操作，只对单一的IO口
  * 注意：确保n的值在0~16区间，分别对应PIN脚
  */
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //输出
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //输入

//#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //输出
//#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //输入

//#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //输出
//#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //输入

//#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //输出
//#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //输入

/***************************END********************************/
#define True	1
#define False	0

#define message_model   //短信的编码格式 1 TXT 0 PDU
#define W433_Enabel  1 //条件编译 1:对应4个按键钥匙的遥控，控制主机5678四个端口。 2 ：最多可对应八个发射机，每个发射机对应一个端口动作  
#define Region_Code   0   //0 内地简体 1港澳台繁体  2英文
//#define SIM_only_mode      //条件编译 仅执行SIM卡功能 目前香港必须编译 仅SIM模式

#define LED1  PBout(3) 		//1回路LED 
#define LED2  PBout(4) 		//2回路LED 
#define LED3  PBout(5) 		//3回路LED 
#define LED4  PBout(6) 		//4回路LED 
#define LED5  PBout(7) 		//5回路LED 
#define LED6  PCout(0) 	 //6回路LED 
#define LED7  PCout(12) 	//7回路LED 
#define LED8  PDout(2) 	//8回路LED 

#define Trigger8  PAin(4) 	//触发端口8 
#define Trigger7  PAin(5) 	//触发端口7 
#define Trigger6  PAin(6) 	//触发端口6 
#define Trigger5  PAin(7)		//触发端口5 
#define Trigger4  PCin(4)	  //触发端口4 
#define Trigger3  PCin(5) 	//触发端口3 
#define Trigger2  PBin(0) 	//触发端口2 
#define Trigger1  PBin(1) 	//触发端口1 

/**以下是3*3 矩阵按键代码**/
//Extern_def volatile unsigned long SimRest;
//Extern_def volatile unsigned long Trigger8;
//Extern_def volatile unsigned long Trigger7;
//Extern_def volatile unsigned long Trigger6;
//Extern_def volatile unsigned long Trigger5;
//Extern_def volatile unsigned long Trigger4;
//Extern_def volatile unsigned long Trigger3;
//Extern_def volatile unsigned long Trigger2;
//Extern_def volatile unsigned long Trigger1;

//#define OUT_A5  PAout(5)		//触发端口5
//#define OUT_A6  PAout(6) 	//触发端口3 
//#define OUT_A7  PAout(7) 	//触发端口2 
//#define IN_B0  PBin(0) 	//触发端口1 
//#define IN_B1  PBin(1) 	//触发端口7
//#define IN_B2  PBin(2) 	//触发端口8 
/**END**/

#define I2C_PW	 		PCout(15)		// //I2C 总线电源控制 标准输出
#define SimRest	 		PBin(15)		// 设置模式检测位	上拉输入
#define AlrmRelay  	PBout(14)			//继电器输出 标准输出
#define SimRTX  		PAout(12) //控制SIM模块发送数据 1停止数据流 开漏输出
#define PowerOn  		PAout(8)			//开机输出电平启动SIM模块; 开漏输出
#define rs485_rw  	PAout(1)			//rs485 R/W en 标准输出
/*低速时钟关闭外部晶振 复用通用IO口*/
#define Tled  			PCout(14)			// Tled指示灯 开漏输出
#define AlrmBuzzer  PCout(7)			//蜂鸣器 标准输出
#define RestKey  		PAout(11)		  //SIM模块重启 开漏输出
#define PowerOff 		PBin(12)				// 	停电检测	上拉输入
#define UserQuery_KEY PCin(13)				//OLED面板按键检测 上拉输入
#define Rx_433_IO 	PCin(6)				//433M input 上拉输入
#define PWR_LVoff  	PAout(15)		  // 低压关断电池 标准输出
/*END*/
Extern_def u16 Battery_Voltage;//采集电池电压
Extern_def	__IO uint16_t ADC_convertedValue[3];	//PB13

#if (Region_Code == 0)	//0 内地
#define W433_Inching    "70B952A86A215F0F" //003A:点动模式 0x00
#define W433_SelfHold   "81EA95016A215F0F" //:自锁模式 0x11 
#define W433_InterLock  "4E9295016A215F0F" //:互锁模式 0x22
#define W433_OFF 			  "906563A7517395ED" //:遥控关闭 0x33

#define P_OFF   				"8BBE59075DF2505C75350021" 		//设备已停电!
#define P_ON    				"8BBE59076062590D4F9B7535" 		//设备恢复供电
#define MobileCheck   	"67E58BE20023" 		   //查询#	移动联通 TXT代码	   
//#define CDMACheck   		"2C5F10" 		   //电信 查询	PDU代码
//#define MobileCheck   	"00510055004500520059" //QUERY	TXT代码	   
#define CDMACheck   		"2C5F10" 		   //查询	PDU代码

//#define RlyON    				"54385408" 			//			  吸合
//#define RlyOFF   				"91CA653E" 		  //		      释放
#define Sccuces     		"8BBE7F6E6210529F"   //"设置成功"
#define Mess_Sccuces   "77ED4FE153F77801FF0C8BBE7F6E6210529F" //"短信号码，设置成功"
#define Phone_Sccuces  "75358BDD53F77801FF0C8BBE7F6E6210529F" //"电话号码，设置成功"	
#define MesegSccuces   "4FE1606FFF0C8BBE7F6E6210529F"   //"信息,设置成功"
#define DatClear    	 "4FE1606F5DF2516890E86E059664FF0C5DF26062590D51FA53828BBE7F6E3002" //信息已全部清除，已恢复出厂设置。
//#define Reset_Com   		"0043004C00450041004E" 			//CLEAN
#define Reset_Com   		"6E05966400230023" 			//清除##
#define ClearSccu     	"6E0596646210529F"   //"清除成功"
#define DataOver     	 	"4FE1606F6EA251FA"   //"信息溢出"
#define RealyON    			"7EE7753556685DF254385408" 			//	继电器已吸合
#define RealyOFF   			"7EE7753556685DF291CA653E" 			//	继电器已释放
#define SetFail   			"8BBE7F6E59318D25" 			//	设置失败

#define RMTON					"0052004D0054004F004E"//RMTON 布/撤防控制  1 开启
#define RMTOFF					"0052004D0054004F00460046"//RMTOFF 布/撤防控制 0 关闭 
#define AlarmMode_SIM			"00530049004D536162A58B66"//SIM卡报警 报警方式 0，全功能 1，SIM卡短信电话 2，物联卡上传报警信号 3禁止报警，仅传送状态数据至公众号
#define AlarmMode_IOT			"72698054536162A58B66"//物联卡报警
#define AlarmMode_FULL		"5168529F80FD62A58B66"//全功能报警
#define AlarmMode_OFF			"517395ED62A58B66 "//关闭报警
//#define EraseStr					"004500520041005300450023"//ERASE#
#define EraseStr					"64E696640023"//擦除#
//#define Erase1					"64E69664002300310023"//擦除#1#
//#define Erase2					"64E69664002300320023"//擦除#2#
//#define Erase3					"64E69664002300330023"//擦除#3#
//#define Erase4					"64E69664002300340023"//擦除#4#
//#define Erase5					"64E69664002300350023"//擦除#5#
//#define Erase6					"64E69664002300360023"//擦除#6#
//#define Erase7					"64E69664002300370023"//擦除#7#
//#define Erase8					"64E69664002300380023"//擦除#8#
#define EraseSccuc     	"64E696646210529F"   //擦除成功

//信息#8#  查询第8回路的全部短信信息 
#define QueryComStr					"4FE1606F0023"//信息#
//char DatSet[]   = "8BF78BBE7F6E4FE1606F";  //请设置信息
#define str_check_extpwr_off "591690E875356E90FF1A505C7535000A" //外部电源：停电
#define str_check_extpwr_on  "591690E875356E90FF1A6B635E38000A"//外部电源：正常
#define str_check_signal "4FE153F7FF1A"//信号：99,99		   10(强度5F3A5EA6)
#define str_check_diff   "FF0C8F835DEE000A"// ,较差换行
#define str_check_temp   "6E295EA6FF1A"//温度：
#define str_check_hum    "6E7F5EA6FF1A"//湿度：
#define str_set_TemHum   "6E296E7F5EA6"//温湿度
#define str_set_Allprot  "516890E87AEF53E3"//全部端口
#define str_set_PowOffOn "505C67657535"//停来电
#define str_set_msg_phone  "77ED4FE153F77801002C8BBE7F6E6210529F"//短信号码,设置成功
#define str_set_call_phone "75358BDD53F77801002C8BBE7F6E6210529F"//电话号码,设置成功
#define str_QueryPortInfo_NCmsg "95ED540877ED4FE1FF1A"//闭合短信：
#define str_QueryPortInfo_NOmsg "000A65AD5F0077ED4FE1FF1A"//换行  断开短信：
#define str_set_NC_msg   "95ED5408544A8B6677ED4FE1002C8BBE7F6E6210529F"//闭合告警短信,设置成功
#define str_set_NO_msg   "65AD5F0089E3966477ED4FE1002C8BBE7F6E6210529F"//断开解除短信,设置成功

#define RS485_hostMod  "005200530034003800354E3B673A0023"//RS485主机#
#define RS485_SlaveMod "005200530034003800354ECE673A0023"//RS485从机# 1~9#
#define Stand_Mod 		 "72EC7ACB6A215F0F0023"//独立模式#
#define RS485_SlaveFault "005200530034003800350020003%1d53F74ECE673A6545969C"//RS485 003%1d号从机故障        “003%1d”打印一位数字符串
#define wifi_setNet "0077006900660069914D7F510023"//wifi配网#  配網

#elif (Region_Code == 1)	// 1港澳台
#define W433_Inching     "9EDE52D56A215F0F"//點動模式 0x00
#define W433_SelfHold    "81EA93966A215F0F"//自鎖模式 0x11 
#define W433_InterLock   "4E9293966A215F0F"//互鎖模式 0x22
#define W433_OFF 			   "905963A795DC9589"//遙控關閉 0x33

#define  PowerON_COM   "002A4F8696FB0023"		//*來電#
#define PowerOFF_COM   "002A505C96FB0023"		//*停電#
#define P_OFF   "8A2D50995DF2505C96FB0021"		//設備已停電!
#define P_ON    "8A2D509960625FA94F9B96FB"		//設備恢復供電
#define MobileCheck   "67E58A62"		   //移动联通 查詢	TXT代码	   
#define CDMACheck   "2C5F10"		   //电信 查询	PDU代码
#define RlyON    "54385408"			//			  吸合
#define RlyOFF   "91CB653E "		  //		     釋放
#define Sccuces  "8A2D7F6E6210529F"  //設置成功
#define Mess_Sccuces   "7C218A0A865F78BCFF0C8A2D7F6E6210529F"//"簡訊號碼，設置成功"
#define Phone_Sccuces  "96FB8A71865F78BCFF0C8A2D7F6E6210529F"//"電話號碼，設置成功"	
#define MesegSccuces   "4FE1606FFF0C8A2D7F6E6210529F"  //信息,設置成功
//#define DatClear  "4FE1606F5DF26E059664"  //信息已清除
#define DatClear    	  "4FE1606F5DF2516890E86E059664FF0C5DF260625FA951FA5EE08A2D7F6E3002" //信息已全部清除，已恢復出廠設置。
#define SetFail   			"8A2D7F6E59316557" 			//設置失敗
#define EraseStr					"64E696640023"//擦除#
#define QueryComStr					"4FE1606F0023"//信息#
#define Reset_Com   "6E059664"			//"清除"
#define ClearSccu  "6E0596646210529F"  //"清除成功"
#define DataOver   "4FE1606F6EA251FA"  //"信息溢出"
#define RealyON  = "7E7C96FB56685DF254385408"			//繼電器已吸合
#define RealyOFF = "7E7C96FB56685DF291CB653E"			//繼電器已釋放

#define str_check_extpwr_off "591690E896FB6E90FF1A505C96FB000A" //外部電源：停電
#define str_check_extpwr_on  "591690E896FB6E90FF1A6B635E38000A" //外部電源：正常
#define str_check_signal "4FE1865FFF1A"//信號：99,99		   10(强度5F3A5EA6)
#define str_check_diff   "FF0C8F035DEE000A"// ,較差换行
#define str_check_temp   "6EAB5EA6FF1A"//溫度：
#define str_check_hum    "6FD55EA6FF1A"//濕度：
#define str_set_TemHum   "6EAB6FD55EA6"//溫濕度
#define str_set_PowOffOn "505C4F8696FB"//停來電
#define str_set_msg_phone  "77ED4FE1865F78BCFF0C8A2D7F6E6210529F"//短信號碼，設置成功
#define str_set_call_phone "96FB8A7153F77801002C8BBE7F6E6210529F"//電話號碼，設置成功
#define str_QueryPortInfo_NCmsg "9589540877ED4FE1FF1A"//閉合短信：
#define str_QueryPortInfo_NOmsg "000A65B7958B77ED4FE1FF1A"//换行  斷開短信：
#define str_set_NC_msg   "95895408544A8B6677ED4FE1FF0C8A2D7F6E6210529F"//閉合告警短信，設置成功
#define str_set_NO_msg   "65B7958B89E3966477ED4FE1FF0C8A2D7F6E6210529F"//斷開解除短信，設置成功
/*以下還沒有轉換*/
#define RMTON					"0052004D0054004F004E"//RMTON 布/撤防控制  1 开启
#define RMTOFF					"0052004D0054004F00460046"//RMTOFF 布/撤防控制 0 关闭 
#define AlarmMode_SIM			"00530049004D536162A58B66"//SIM卡报警 报警方式 0，全功能 1，SIM卡短信电话 2，物联卡上传报警信号 3禁止报警，仅传送状态数据至公众号
#define AlarmMode_IOT			"72698054536162A58B66"//物联卡报警
#define AlarmMode_FULL		"5168529F80FD62A58B66"//全功能报警
#define AlarmMode_OFF			"517395ED62A58B66 "//关闭报警

#elif (Region_Code == 2)	// 2海外
#endif

#define SetTemerHumUnic "005400480023"			//温湿度识别符"TH#" 的Unicode码

#define  CSCA_W433_Address  0x0801F000 		//+偏移0  存储短信中心号码地址			2024.5.10	
#define  CSCA_N  "CSCA#" 			 // "CSCA#" GB232短信中心号码
#define  CSCA_N_uni   "00430053004300410023" 			 //"CSCA#"UNICODE码 短信中心号码

#define W433_Ide_offset  0x40  //0x0801F000 +偏移0x40  存储 3个识别码 2个地址码  4个键值码 ABCD //18 d3 --17 1b 1d 1e
#define Alarm_Mode_offAddr	0x4a	//报警方式 存储地址 0x0801F000 +偏移0x40+9
#define Alarm_RemoteCtr_offAddr	0x4b	//报警方式 存储地址 0x0801F000 +偏移0x40+10
#define W433_Flag_offset 0x80 //0x0801F000 +偏移0x80  存储

#define  TemperHum_AddrOffset   0x380  //温湿度地址偏移量
#define  TemperHum_AddrSection   0  //温湿度存入的扇区 下票
#define  TemperHum_Alarm_Num   9  //温湿度 报警序号

#define  PowerOFF_AddrSection   1  //停电告警存入的扇区 下票
#define  PowerOFF_Alarm_Num   10  //停电告警 报警序号

#define  M433_AddrOffset   0x400  //433M地址偏移量 数据长度56
//#define  M433_Interval_Area   0x10  //间隔区域大小
#define  RS485_AddrOffset   0x500  //24-11-8 RS485保存参数的地址 偏移量 数据长度 16 RegisterSlaveType
#define  Alarm_maxGroup 10		//报警最大数量
/*******************移动编码************************/
//号码格式：@#-----------#-----------#-----------#
// unicode  * -- 002a  @ -- 0040   # -- 0023
#define StrNeed "0023" 				 //#Unicode码 用于号码分割识别符
#define NumberCode "0040" 			 //0023 @# Unicode码 用于号码开始符
//#define NuExCode "004000310023" 			 // @1# Unicode码 用于号码开始符 扩展号码

#define PoneCode "0026" 			 //0023 &# Unicode码 拔打电话的号码前三个
//#define PoneExCode "002600310023" 			 // &1# Unicode码 拔打电话的号码 扩展号码

//uint8_t HeadMeg[]="+CMT: \"0";	 //+CMT: "+86  ---中国的代码+86	

#define needle1  "0025" 	 //% 的Unicode码
//#define needle1  "002500310023" 	 //%1# 的Unicode码
#define needle2  "002500320023" 	 //%2#的Unicode码
#define needle3  "002500330023" 	 //%3#的Unicode码
#define needle4  "002500340023" 
#define needle5  "002500350023" 
#define needle6  "002500360023" 
#define needle7  "002500370023" 
#define needle8  "002500380023" 
/*****************/
#define Open_1  "002A" 	 //* 的Unicode码
//#define Open_1  "002A00310023" 	 //*1# 的Unicode码
#define Open_2  "002A00320023" 	 //*2#的Unicode码
#define Open_3  "002A00330023" 	 //*3#的Unicode码
#define Open_4  "002A00340023" 
#define Open_5  "002A00350023" 
#define Open_6  "002A00360023" 
#define Open_7  "002A00370023" 
#define Open_8  "002A00380023" 	 //*8# 的Unicode码
//char *PasswordCode ="002A002A0023";			//修改密码关键字		 **#
//char *Passdword = "003600360036003600360036";//		   初始密码	  666666
/***********************电信编码***************************/
#define SetTemerHum "TH#"			//
#define CDMA_StrNeed "#" 				 //# Ascii码 用于号码分割识别符
#define CDMA_NumberCode "@" 			 // @# Ascii码 用于号码开始符
//#define CDMA_NuExCode "@1#" 			 // @1# Ascii码 用于号码开始符 扩展号码

#define CDMA_PoneCode "&" 			 // &# Ascii码 拔打电话的号码前三个
//#define CDMA_PoneExCode "&1#" 			 // &1# Ascii码 拔打电话的号码 扩展号码

#define CDMA_needle1 "%1#"  	 //*1# 的Ascii码
#define CDMA_needle2 "%2#" 	 //*2#的Ascii码
#define CDMA_needle3 "%3#"	 //*3#的Ascii码
#define CDMA_needle4 "%4#"
#define CDMA_needle5 "%5#"
#define CDMA_needle6 "%6#"
#define CDMA_needle7 "%7#"
#define CDMA_needle8 "%8#"	 //*8# 的Ascii码
/*****************/
#define CDMA_Open1 "*1#"	 //%1# 的Ascii码
#define CDMA_Open2 "*2#"	 //%2#的Ascii码
#define CDMA_Open3 "*3#"	 //%3#的Ascii码
#define CDMA_Open4 "*4#"
#define CDMA_Open5 "*5#"
#define CDMA_Open6 "*6#"
#define CDMA_Open7 "*7#"
#define CDMA_Open8 "*8#"
//char *CDMA_PasswordCode ="**#";			//修改密码关键字		 **#
//char *CDMA_Passdword = "666666";//		   初始密码	  666666

/***********************END***************************/
#define Content_Lenth 250  					//数据长度
#define Mseg_Lenth 500  					//350数据长度

extern const u32 History_Alarm_Addres[10];	
extern char MsegTemp[];		//读取临时短信
extern char *TTSVoice;	
#define	TimeOutSet1		5//5  //
//#define	TimeOutSet2		5

//extern DeviceAlarmModeTYPE DeviceAlarmMode;
extern 	const	char* portstate[];
typedef enum {SetNull = 0, SetERR = 2, SteOK=3}SetResultType;

//#define CloseAlarm "闭合"
//#define OpenAlarm "断开"
#define CloseAlarm "C"
#define OpenAlarm "O"								
#define PhoneLenght 60

#define  StrCall "\""
#define  One_unicode   "0031"
#define  One_str   "1"
#define  NumberCall  "+CMT: \""				  //+CMT:   ---识别代码+86
#define  loose_code  "+CMT: \"00"				  //+CMT: 0031
//char *compact_code = "+CMT: \"1";				  //+CMT: 1 
//extern char  *StrCall;
//extern uint8_t ATD_Messag_Flag;		  //message_model,短信的编码格式 1 TXT 0 PDU
//extern char *one_head;

#define Num_Interval 64 //短信 电话接收号码地址间隔 
#define SMS_Size0 0 //短信号码0 存储空间大小
#define SMS_Size1 256 //短信号码1 存储空间大小
#define Phone_Size0 512 //电话号码0 存储空间大小
#define Phone_Size1 768 //电话号码1 存储空间大小
#define Close_Message_offset 0x0400 //(1024)闭合短信内容存储地址偏移1K
#define Open_Message_offset 0x0600 //(1024+512)闭合短信内容存储地址偏移1K
Extern_def u8 PhoneNumLen;	//被叫手机号码

Extern_def uint32_t  AlarmBitFlag;//报警位标识

extern const u16 SMS_Number_Offset[6];
extern const u16 Call_Number_Offset[6];
Extern_def u8 PhoneAddrIndex;//当前获取号码地址段序号
Extern_def char PhoneNumber[PhoneLenght];	//被叫手机号码
Extern_def uint32_t SMS_CurrentIndex,CallCurrentIndex;
Extern_def char MassageEepromAddress;
//#define	BRT_Timer1	1
//#define	BRT_Timer2	2
Extern_def uint8_t txt_smsphone_lenght;		//txt模式下 接收到短信的主叫号码的长度
Extern_def uint8_t UartInt;
Extern_def uint8_t ModelFlag;					//制式
Extern_def uint8_t MQTT_Infor;					//制式
Extern_def u8 wifi_linkFlag;//配网成功标识
#define TemHumSaveLen 20
Extern_def union  
	{
	u8 ValueArry[TemHumSaveLen];
	struct    
	{
		int           Temper_H ;  //温度上限
		int           Temper_L ;  //温度下限	
		int           Humidi_H ;	//湿度上限
		int           Humidi_L ;	//湿度下限
	}ValueUnit;
}AlarmValue ;
	
typedef struct{
							u8 Num;
							u16 Adrr_Offset;
								}Alarmtype;	
extern Alarmtype Send_Alarm,Receive_Alarm;

#define 	History_Alarm_size	0x40		//64byte	
#define 	History_Alarm_Total_size	0x280		//640 byte	
								
//typedef struct{
//							char year20[2] ;//
//							char year[2];//
//							char hen1[1] ;//
//							char moth[2];//
//							char hen2[1] ;//
//							char day[2];//
//	
//							char hours[2];//
//							char mao1[1] ;//
//							char mini[2];//
//							char mao2[1] ;//	
//							char set[2];//
//								}Alarm_Time_type;

typedef struct{
							u8 AlarmFlag;//报警标识 0x89						
							u8 PortNum;//
							u8 Tem_Hum;//用来区分是温度报警 还是湿度报警
							u8 Close_or_Open;//0闭合或1断开 报警
							u8 Month;
							u8 Date; 
							u8 Year;
							u8 Hours;
							u8 Minutes; 
							u8 Seconds;	
								}HistoryAlarmtype;
extern HistoryAlarmtype Save_history,Read_history;
#define KeyReturnDelay 		30000						
typedef struct {
								u8 CloseState;//闭合状态标识
								u8 LastValuae ;//按键上次状态值
								u16 QueryCnt_C ;//按键延时次数
								u16 QueryCnt_O ;//按键延时次数	
								u8 CurrentPage ;//当前页面
								u8 KeyExeFlag;
								u16 QueryDelay;
								}UserKeyType;	
typedef struct {
								u16 Power_Flag : 1;	//停 断电标识位
								u16 RestFlag : 1; //复位标识
								u16 StratUPFlag : 1;//开机标识位 检测模式
								u16 GSMRet_Flag : 1;//工作模式
//								u8 Alarm_ON : 1;	//当前运行状态 0撤防  1布防
								u16 RunState    : 1;//运行状态 RunState   0 运行 1设置
//								u8 Card_Type  : 1;//卡类型 0 SIM卡 1 IOT卡
								u16 SIMorMQTT : 1;//接收的数据是sim 或mqtt  0 sim  1 mqtt
								u16 Refresh_Sever : 1;//刷新服务器标识
								}DeivceFlag_type;

Extern_def struct 	{
					u8 TemH_flag : 1;	//
					u8 TemL_flag : 1; //
					u8 HumH_flag : 1;//
					u8 HumL_flag : 1;//
					u8 Talarm_flag : 1;
					u8 Halarm_flag : 1;
					u8 Type_TorH : 1;//报警类型 0 温度报警 1 湿度报警
//					u8 Hnormal_flag : 1;
					}AlarmTemHum;

union  Tr_IO
	{
		u16 PortFlag;
struct {
		u16 Port1:1;
		u16 Port2:1;
		u16 Port3:1;
		u16 Port4:1;
		u16 Port5:1;
		u16 Port6:1;
		u16 Port7:1;
		u16 Port8:1;
		u16 Port9:1;
		u16 Port10:1;
		u16 Port11:1;
		u16 Port12:1;
		u16 Port13:1;
	  }Portbit;
	 };
	
typedef union  
{
	u8 SurDat;
	struct{
		u8 Bety_L:4;
		u8 Bety_H:4;
	}DouBty;
}U16_shfit_u8Type;

typedef struct
{
	u8 Trcnt;
	u8 Interval;
}Port;
extern UserKeyType UserKey;		
extern DeivceFlag_type DeivceFlag;

extern  Port Input1,Input2,Input3,Input4,Input5,Input6,Input7,Input8,Power;
extern U16_shfit_u8Type yyt;
//extern union  U8Data cdmaPdu;
//Extern_def union Tr_IO ProtState ;//触发口上次状态值
Extern_def uint32_t RS485_slaveAlarmFlag;//从机发生故障时的地址标识位 作用：只发送一次短信息
typedef enum
{
    FAILED = 0,
    PASSED = !FAILED
} TestStatus;

extern const uint32_t MessgeAddr[];
//extern char *NumberCall;
//extern uint8_t ArrayStor;
//extern uint32_t  CSCA_Num ;
//extern char *CSCA_N ,*CSCA_N_uni;			 // 短信中心号码
 
Extern_def char *cut_str,*start_meg,*start_ext_meg,*start_phone,*start_ext_phone,\
			*close1,*close2,*close3,*close4,*close5,*close6,*close7,*close8,\
			*open1,*open2,*open3,*open4,*open5,*open6,*open7,*open8,*AlarmTemperStr;
void GPIO_Configuration(void);
void NVIC_Configuration(void);
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);

void CheckPhone(void);
u8 PduPendPhone(void);
void HistorySave(void);
void PwoerCheck(void);
void Event_Line(void);
RunResult CheckReturnInfor(char *soucer);
void CurState(char *Source);
SetResultType CleanAll(char*soucer);
SetResultType MessageSet(char *soucer);
void main_call_code(char*soucer);
SetResultType NumberSet(char *source);
void Set_433Model(char*soucer,u8 Dat);

u8 Hex2Ascii(u8 dat);
u8 Ascii2Hex(u8 dat);
u8 hand(const char *soucer,const char *compare);
u8 AsciiToHexDat(char *dat,char len);

void  PhoneFunc(u8 Phone);
extern void  EventFunc(char *Message);
extern void InputState(void);
extern void AlarmTask(void *pvParameters);
extern QueueHandle_t AlarmPort_Queue;	//报警队列句柄
extern TaskHandle_t SetModesTaskHandler;
extern void response(char*soucer,u16 lenght,char *message );
ErrorStatus CSCA_CHCK(u32 CSCA_Adrrs,u32 offset_Addr);
void CSCA_CMD(u32 CSCA_Adrrs,u32 offset_Addr);
void User_SetCSCA(u32 CSCA_Adrrs,u32 OffsetAdd);
ErrorStatus HistoryRead(OLEDLINE_e DisLine,u8 HistoryNum);
SetResultType EraseSector(char *source);
SetResultType QueryPortInfo(char *soucer);
void ScanQuery(void);

ErrorStatus GetPhoneNumber(u32 AlarmPort_Addr,u16 *SMS_or_Call);
ErrorStatus GetAlarmMassage(u32 AlarmPort_Addr,u16 Close_or_Open);
SetResultType RemoteCtr_Enable(char *soucer);
SetResultType SET_Alarm_Mode(char *source);
SetResultType SET_AlarmValue_TemperHum(char *source);
ErrorStatus EC20_CEREG(void);
ErrorStatus Alarm_T(void);
void Arrearage(char*soucer);
ErrorStatus AllGetPhoneNumber(u8 cursor);
SetResultType WorkingMod(char *source);
SetResultType WIFI_linkNet(char *source);
#ifdef __cplusplus
}
#endif

#endif

//#define UratLenth 12 
//数据结构			   字头 1Byt 命令 1BYT 数据包 8BYT 校验 1BYT 结束符1
//union  UratDat
//{
//	u8 DatArr[UratLenth];	  //数据总长度
//	struct{
//		u8 UHead;		  // 0x88
//		u8 UCom;			  // 0x0a ---> 测试模式接收数据不报警  0x09 ---> 正常工作接收数据 0x0b-->数据接收错误返回重新发数据 0x0c-->调节总体灵敏度
//		u8 UDat[8];		  // '','',......
//		u8 UCheck;		  // 0x77
//		u8 UEnd;			  // 0
//		}UFromat;
//};	
//extern char PhoneNumber[];
//extern uint32_t NumberAddr[6];

//const u8 OSMapTbl[]   = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
//const u8 OSUnMapTbl[] = {
//    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
//    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
//};


