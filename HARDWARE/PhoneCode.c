#include "PhoneCode.h"
#include "n32g45x.h"
#include <string.h>
#include "ec20module.h"
//#include "main.h"
//uint8_t message_model = 1;		  //短信的编码格式 1 TXT 0 PDU

const	char* portstate[2] ={"0","1"};
char *TTSVoice;							//tts文本指针
//DeviceAlarmModeTYPE DeviceAlarmMode = simANDmqtt;//0：SIM卡和上传到MQTT服务器报警

u8 loose_model = 1;	// +CMT: 0031 松散编码1	   +CMT: 1 紧凑编码0	目前针对电信主叫号码的识别
//const u16 SMS_Number_Offset[6]  ={0,0x40,0x80,0x100,0x140,0x180};//短信接收每个号码空0x40
const u16 SMS_Number_Offset[6]  ={0,0x40,0x80,0xc0,0x100,0x140};//短信接收每个号码空0x40
//const u16 Call_Number_Offset[6] ={0x200,0x240,0x280,0x300,0x340,0x380};//拔打电话 每个号码空0x40
const u16 Call_Number_Offset[6] ={0x200,0x240,0x280,0x2c0,0x300,0x340};//拔打电话 每个号码空0x40
//const u16 TemperHum_AddrOffset  = 0x380;//温湿度地址偏移量
//36K存储空间 从104K开始存储信息 共有128K
/*扇区存储格式:
短信被叫号码6个 512BYTE 电话被叫号码6个 512BYTE  闭合报警短信信息 512BYTE 断开报警短信信息 512BYTE
*/
//CSCA_W433_Address  0x0801F000 		//+偏移0  存储短信中心号码地址			2024.5.10	
const uint32_t MessgeAddr[Alarm_maxGroup] = {0x0801A000,0x0801A800,//温湿度报警 ,停电/来电报警消息;104k strat
																 0x0801B000,0x0801B800,0x0801C000,0x0801C800,0x0801D000,0x0801D800,0x0801E000,0x0801E800//8组触发短信闭合断开
																 };				  
																 
/*历史报警记录空间(0x40)64byte*10*/
const u32 History_Alarm_Addres[10] ={0x0801F800,0x0801F840,0x0801F880,0x0801F8C0,0x0801F900,0x0801F940,0x0801F980,0x0801F9C0,0x0801FA00,0x0801FA40};//历史报警存储地址	2024.5.20																	 

Alarmtype Send_Alarm,Receive_Alarm;
HistoryAlarmtype Save_history,Read_history;	
UserKeyType UserKey;
char NONicode[] = "5BFC901A0020";		  //导通空格
char NCNicode[] = "65AD5F000020";		  //断开空格
//const char NumUnicode[10][5] = {{"0030"},{"0031"},{"0032"},{"0033"},{"0034"},{"0035"},{"0036"},{"0037"},{"0038"},{"0039"}};
const char *NumUnicode[10] = {"0030","0031","0032","0033","0034","0035","0036","0037","0038","0039"};
char *Check_code;//查询代码  PDU  TXT
//char *NumberCall = "+CMT: \"" ;				  //+CMT:   ---识别代码+86
////char *compact_code = "+CMT: \"1";				  //+CMT: 1  
//char *loose_code = "+CMT: \"00";				  //+CMT: 0031  

//const char *IFC_ON = "AT+IFC=2,2\r\n";
//const char *IPR = "AT+IPR=115200\r\n";
//const char *ATI = "ATI\r\n";
//const char *CSQ = "AT+CSQ\r\n";
//const char *CPIN = "AT+CPIN?\r\n";
//const char *COPS = "AT+COPS?\r\n";
//char OpenMod[]  = "5F008DEF89E653D1";  //"开路触发"
//char CloseMod[] = "77ED8DEF89E653D1";  //"短路触发"
//char *Read_ID   = "00257CFB521753F70025";//"%系列号%"
//char *Rao_wh = "0049004453F700237CBE521B8FEA";	  //ID号#精创迪
//char JCD[] = "7CBE521B8FEA";		//精创迪
//SetResultType SetResult ; //RunResult枚举类型，用于枚举函数执行结果类型

//char *StrCall="\"";
//char *One_unicode = "0031";
//char *One_str = "1";
//char *one_head;

//static char N_Flag=0;				//检验注册码标识
uint8_t InterTimer = 200;		//同一端口报警与解除报警之间的触发间隔时间。 8s
//uint8_t CheckLen;				//查询命令执行时，被叫号码长度
//uint8_t half_width1 = 0,half_width2 = 0; //全角 半角转换标识	,half_width3 = 0

//OS_Q Alarm;						//队列
Port Input1,Input2,Input3,Input4,Input5,Input6,Input7,Input8,Power;
U16_shfit_u8Type yyt;
U16_shfit_u8Type cdmaPdu;

void TIM9_ConfigForLSI(void);
//void test11(void);
__IO uint32_t LsiFreq = 40000;

//#define LSI_TIM_MEASURE
//调试用 查看时钟用
RCC_ClocksType RCC_Clocks;

extern TaskHandle_t AlarmTaskHandler;
extern TaskHandle_t CallResultTaskHandler;
extern TaskHandle_t LedTaskHandler;
extern  TaskHandle_t WIFI_onlinDetTaskHandler;
extern union Tr_IO W433SimuPort;

UserKeyType UserKey =
									{
								 .CloseState = 0,
								 .LastValuae = 0,//按键上次状态值
								 .QueryCnt_C = 0,
								 .QueryCnt_O = 0,//按键延时次数	
								 .CurrentPage = 0,//当前页面
								 .KeyExeFlag = 0,
								 .QueryDelay = 0,										
									};
DeivceFlag_type DeivceFlag=
												{
												.Power_Flag = 0,				 //停 断电标识位
												.RestFlag = 1, //复位标识
												.StratUPFlag = 1,//开机标识位 检测模式
												.GSMRet_Flag = 1,
//												.Alarm_ON = 0,	//当前运行状态 0撤防  1布防
												.RunState = 0,//运行状态 RunState   0 运行 1设置
//												.Card_Type = 1,//卡类型 0 SIM卡 1 IOT卡
												.SIMorMQTT = 0,//接收的数据是sim 或mqtt  0 sim 1 mqtt	
												.Refresh_Sever = 0,
												};
									
u8 Hex2Ascii(u8 dat)
{
	dat &= 0x0f;
	if(dat <= 9)	return (dat+'0');
	return (dat-10+'A');
}
u8 Ascii2Hex(u8 dat)
{
	if(dat < 'A')	return ( dat&0x0f );
	return (dat-'A'+10);
}

//u8 AsciiToHexDat(char *dat,char len)
//{
//	u8 Temp;

//	Temp = Ascii2Hex(*dat);len-- ;
//		if(len > 0)
//		{
//			Temp <<= 2;dat++;
//		 Temp += Ascii2Hex(*dat);
//			return Temp;
//		}
//return Temp;
//}

/**
 * @brief  Configures the different GPIO ports.
 */
void GPIO_Configuration(void)
{
    GPIO_InitType GPIO_InitStructure;
    /* Initialize GPIO_InitStructure */
    GPIO_InitStruct(&GPIO_InitStructure);
		RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_AFIO, ENABLE);
		GPIO_ConfigPinRemap(GPIO_RMP_SW_JTAG_SW_ENABLE, ENABLE);	//禁用JTAG 使能SW
//GPIOA
		GPIO_InitStructure.Pin        = GPIO_PIN_8 | GPIO_PIN_11 | GPIO_PIN_12; // | GPIO_PIN_15  rs485 urat --> pin2 pin3
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;		//开漏输出
//		GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
//		GPIO_InitStructure.GPIO_Pull    = GPIO_No_Pull;
//		GPIO_InitStructure.GPIO_Speed =	GPIO_Speed_2MHz;
		GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
//		GPIO_SetBits(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);

		GPIO_InitStructure.Pin        = GPIO_PIN_1 | GPIO_PIN_15;
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
//		GPIO_InitStructure.GPIO_Speed =	GPIO_Speed_2MHz;
		GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);	
	
//		GPIO_InitStruct(&GPIO_InitStructure);		//带上拉输入
		GPIO_InitStructure.Pin        =  GPIO_PIN_4 | GPIO_PIN_5 
																		| GPIO_PIN_6 | GPIO_PIN_7;//
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;//GPIO_Mode_Input;
//		GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
//		GPIO_InitStructure.GPIO_Pull    = GPIO_No_Pull;
		GPIO_InitStructure.GPIO_Speed =	GPIO_Speed_2MHz;
		GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

//GPIOB	
		GPIO_InitStruct(&GPIO_InitStructure);
		GPIO_InitStructure.Pin        = GPIO_PIN_14 ;//| GPIO_PIN_15;//GPIO_PIN_1 | GPIO_PIN_12 |
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
//		GPIO_InitStructure.GPIO_Pull    = GPIO_No_Pull;
		GPIO_InitStructure.GPIO_Speed =	GPIO_Speed_2MHz;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
//    GPIO_ResetBits(GPIOB, GPIO_InitStructure.Pin);
		
//		GPIO_InitStruct(&GPIO_InitStructure);
		GPIO_InitStructure.Pin        =  GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
																		//GPIO_PIN_9 | GPIO_PIN_8 | 
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
//		GPIO_InitStructure.GPIO_Pull    = GPIO_No_Pull;
//		GPIO_InitStructure.GPIO_Speed =	GPIO_Speed_2MHz;
    GPIO_SetBits(GPIOB, GPIO_InitStructure.Pin);//GPIO_InitStructure.Pin  GPIO_ResetBits
		GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
 
		GPIO_InitStruct(&GPIO_InitStructure);//带上拉输入
		GPIO_InitStructure.Pin        = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_12 | GPIO_PIN_15;  
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;//GPIO_Mode_Input;
		GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);	
	
    /* Configure PB13 as analog input -------------------------*/
    GPIO_InitStructure.Pin       = GPIO_PIN_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
		
//GPIOC		
		GPIO_InitStruct(&GPIO_InitStructure);
	  GPIO_InitStructure.Pin        = GPIO_PIN_7 | GPIO_PIN_15;//
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;//GPIO_Mode_IPU;
//		GPIO_InitStructure.GPIO_Current = GPIO_DC_2mA;
//		GPIO_InitStructure.GPIO_Pull    = GPIO_Pull_Up;
		GPIO_InitStructure.GPIO_Speed =	GPIO_Speed_2MHz;
    GPIO_InitPeripheral(GPIOC, &GPIO_InitStructure);
//		GPIO_SetBits(GPIOC, GPIO_PIN_13);
		
		GPIO_InitStruct(&GPIO_InitStructure);
	  GPIO_InitStructure.Pin        = GPIO_PIN_0 | GPIO_PIN_12 | GPIO_PIN_14;	//开漏输出 urat2 GPIO_PIN_10 PIN_11
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;//GPIO_Mode_IPU;
		GPIO_InitStructure.GPIO_Speed =	GPIO_Speed_2MHz;
		GPIO_SetBits(GPIOC, GPIO_PIN_12);
		GPIO_InitPeripheral(GPIOC, &GPIO_InitStructure);
		 
		GPIO_InitStruct(&GPIO_InitStructure);
	  GPIO_InitStructure.Pin        = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_13;//带上拉输入
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitPeripheral(GPIOC, &GPIO_InitStructure);
		
//GPIOD		
		GPIO_InitStruct(&GPIO_InitStructure);
	  GPIO_InitStructure.Pin        = GPIO_PIN_2;	//开漏输出
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;//GPIO_Mode_IPU;
		GPIO_InitStructure.GPIO_Speed =	GPIO_Speed_2MHz;
 		GPIO_SetBits(GPIOD, GPIO_PIN_2);
		GPIO_InitPeripheral(GPIOD, &GPIO_InitStructure);
}
/**
 * @brief  Configures the nested vectored interrupt controller.
 */
void NVIC_Configuration(void)
{
    NVIC_InitType NVIC_InitStructure;
    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    /* Enable the USARTy Interrupt */
//    NVIC_InitStructure.NVIC_IRQChannel            = USARTy_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
    /* Enable the USARTz Interrupt */
//    NVIC_InitStructure.NVIC_IRQChannel            = USARTz_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//    NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);

    /* Enable the TIM1 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
		
    /* Enable the TIM2 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
		
    /* Enable the TIM3 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;

    NVIC_Init(&NVIC_InitStructure);	
		
    /* Enable the TIM5 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;

    NVIC_Init(&NVIC_InitStructure);		
    /* Enable CRC clock */
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_CRC, ENABLE);
}

//查询当前设备状态
void InputState(void)
{	
//char *buf,*haystack;	
int signalval;//StrLen,
char *Dis_unicode = (char*)portMalloc(15*sizeof(u8) ) ;

//	EC20_SendModuleCmd(AT_CSQ, NULL );
	memset(MsegTemp,0,Mseg_Lenth);				  //清除内容  必须要清除
	if(PowerOff)						//停电
	strcpy(MsegTemp,str_check_extpwr_off);	//"591690E875356E90FF1A505C7535000A"外部电源：停电  8
	else
	strcpy(MsegTemp,str_check_extpwr_on);	//"591690E875356E90FF1A6B635E38000A"外部电源：正常
 	strcat(MsegTemp,str_check_signal);	//"4FE153F7FF1A"信号：99,99		   10(强度5F3A5EA6)
	
	strncpy(Dis_unicode,(const char*)sEc20Param.csq,2);
	cdma_mobile_txt_shift(Dis_unicode,2);	//字符串转换成UNICODE
	snprintf(MsegTemp+44, 15, "%s", Dis_unicode) ;  //strlen(MsegTemp) 
  signalval = atoi((const char*)sEc20Param.csq);	//字符串转换成hex
/***************************获取网络信号数值 功能正常用上面一句替代掉 ************************************/
//	haystack= ec20AtBuf;	
//	buf= strstr( haystack, ": ");					//
//	haystack = buf + strlen(": ");
//	buf= strstr( haystack, ",");
//	buf[0]='\0';
///*判断数值长度*/
//	StrLen = strlen(haystack);
//	if(StrLen == 1)
//		{
//		signalval = Ascii2Hex(*haystack);
//		strcat(MsegTemp,"0030");			//1位数值时补0 拼成2位数
//		strcat(MsegTemp,NumUnicode[Ascii2Hex(*haystack)]); 
//		}
//	else
//		{
//		signalval = Ascii2Hex(*haystack);
//		strcat(MsegTemp,NumUnicode[Ascii2Hex(*haystack)]);haystack++;
//		signalval *= 10;	   //扩大10倍
//		signalval += Ascii2Hex(*haystack);
//		strcat(MsegTemp,NumUnicode[Ascii2Hex(*haystack)]);
//		}
/***************************END***********************************/
/***************************网络信号质量 ************************************/
/*如果数值小于10，很差，20> 一般 >=10  良好>20*/	
	if(signalval > 20)		 strcat(MsegTemp,"FF0C826F597D000A");		  // ,良好换行
	else if(signalval > 10)strcat(MsegTemp,"FF0C4E00822C000A");		  // ,一般换行
	else 									 strcat(MsegTemp,str_check_diff);		  //"FF0C8F835DEE000A" ,较差换行
/***************************插入一行 温度 湿度 ************************************/		
//	char *Dis_unicode = (char*)portMalloc(30*sizeof(u8) ) ;
//		gcvt(SENx.T,2,Dis_unicode);
//		cdma_mobile_txt_shift(Dis_unicode,5);	//字符串转换成UNICODE
//		strcat(MsegTemp,"6E295EA6FF1A");	//温度：	
//		strcat(MsegTemp,Dis_unicode);	//显示湿度数据
//		strcat(MsegTemp,"00200020");	//空格 空格
//		
//		gcvt(SENx.RH,2,Dis_unicode);
//		cdma_mobile_txt_shift(Dis_unicode,5);	//字符串转换成UNICODE
//		strcat(MsegTemp,"6E7F5EA6FF1A");	//湿度：	
//		strcat(MsegTemp,Dis_unicode);	//
//		strcat(MsegTemp,"000A7AEF53E3FF1A0031");	//换行/n端口：1	
//	portFree(Dis_unicode) ;			
strncpy(Dis_unicode,(const char*)sEc20Param.Temper,5);
		cdma_mobile_txt_shift(Dis_unicode,5);	//字符串转换成UNICODE
		strcat(MsegTemp,str_check_temp);	//"6E295EA6FF1A"温度：	
		strcat(MsegTemp,Dis_unicode);	//显示湿度数据
		strcat(MsegTemp,"00200020");	//空格 空格
strncpy(Dis_unicode,(const char*)sEc20Param.Humidi,5);		
		cdma_mobile_txt_shift(Dis_unicode,5);	//字符串转换成UNICODE
		strcat(MsegTemp,str_check_hum);	//"6E7F5EA6FF1A"湿度：	
		strcat(MsegTemp,Dis_unicode);	//
		strcat(MsegTemp,"000A7AEF53E3FF1A0031");	//换行/n端口：1	
/***************************END***********************************/	

	if(LED8)strcat(MsegTemp,NCNicode);						  //1回路 Trigger8
	else strcat(MsegTemp,NONicode);
	strcat(MsegTemp,NumUnicode[2]);
	if(LED7)strcat(MsegTemp,NCNicode);
	else strcat(MsegTemp,NONicode);
	strcat(MsegTemp,NumUnicode[3]);
	if(LED6)strcat(MsegTemp,NCNicode);
	else strcat(MsegTemp,NONicode);
	strcat(MsegTemp,NumUnicode[4]);
	if(LED5)strcat(MsegTemp,NCNicode);
	else strcat(MsegTemp,NONicode);
	strcat(MsegTemp,NumUnicode[5]);
	if(LED4)strcat(MsegTemp,NCNicode);
	else strcat(MsegTemp,NONicode);
	strcat(MsegTemp,NumUnicode[6]);
	if(LED3)strcat(MsegTemp,NCNicode);
	else strcat(MsegTemp,NONicode);
	strcat(MsegTemp,NumUnicode[7]);
	if(LED2)strcat(MsegTemp,NCNicode);					 //7回路
	else strcat(MsegTemp,NONicode);
	strcat(MsegTemp,NumUnicode[8]);		   
	if(LED1)strcat(MsegTemp,NCNicode);					//8回路Trigger1
	else strcat(MsegTemp,NONicode);
/********433M 功能*********/	
	strcat(MsegTemp,"000A003400330033004DFF1A");//换行433M:
	if(W433Hard.Head.W433Model == 0x11)strcat(MsegTemp,W433_SelfHold);//自锁模式 0x11
	else if(W433Hard.Head.W433Model == 0x22)strcat(MsegTemp,W433_InterLock);//互锁模式 0x22
	else if(W433Hard.Head.W433Model == 0x33)strcat(MsegTemp,W433_OFF);//遥控关闭 0x33
	else strcat(MsegTemp,W433_Inching);//点动模式 0x00
/*********END***********/
portFree(Dis_unicode) ;	
}
////SEVER 是否服务器指令 1 是
////查询当前设备状态
//void InputState(void)
//{	
//char *buf,*haystack;	
//u8 StrLen,signalval;
////char tt[]={0x2B,0x43,0x53,0x51,0x0D,0x0D,0x0A,0x2B,0x43,0x53,0x51,0x3A,0x20,0x30,0x34,0x2C,0x39,0x39,0x0D,0x0A};
////sim900a_send_cmd((char*)CSQ,"OK",100);
//	EC20_SendModuleCmd(AT_CSQ, NULL );
//	memset(MsegTemp,0,Mseg_Lenth);				  //清除内容  必须要清除
//	if(PowerOff)						//停电
//	strcpy(MsegTemp,"591690E875356E90FF1A505C7535000A");	//外部电源：停电
//	else
//	strcpy(MsegTemp,"591690E875356E90FF1A6B635E38000A");	//外部电源：正常

//	strcat(MsegTemp,"4FE153F7FF1A");	//信号(强度5F3A5EA6)：99,99		   10
////if(ModelFlag)				   //运营商 1 电信 0 移动
////strcpy(MsegTemp,"4FE153F7FF1A75354FE1");	//信号:电信(强度5F3A5EA6)99,99		   10
////else strcpy(MsegTemp,"4FE153F7FF1A79FB52A8002F8054901A");	//信号:移动/联通

////	 haystack= tt;
//	haystack= ec20AtBuf;//RX1_Buffer;	
//	buf= strstr( haystack, ": ");					//
//	haystack = buf + strlen(": ");
//	buf= strstr( haystack, ",");
//	buf[0]='\0';
///*判断数值长度*/
//	StrLen = strlen(haystack);
//	if(StrLen == 1)
//		{
//		signalval = Ascii2Hex(*haystack);
//		strcat(MsegTemp,"0030");			//1位数值时补0 拼成2位数
//		strcat(MsegTemp,NumUnicode[Ascii2Hex(*haystack)]); 
//		}
//	else
//		{
//		signalval = Ascii2Hex(*haystack);
//		strcat(MsegTemp,NumUnicode[Ascii2Hex(*haystack)]);haystack++;
//		signalval *= 10;	   //扩大10倍
//		signalval += Ascii2Hex(*haystack);
//		strcat(MsegTemp,NumUnicode[Ascii2Hex(*haystack)]);
//		}
/////*如果数值小于10，很差，20> 一般 >=10  良好>20*/	
////	if(signalval > 20)strcat(MsegTemp,"FF0C826F597D000A7AEF53E3FF1A0031");		  // ,良好换行/n端口：1
////	else if(signalval > 10)strcat(MsegTemp,"FF0C4E00822C000A7AEF53E3FF1A0031");		  // ,一般换行/n端口：1
////	else strcat(MsegTemp,"FF0C8F835DEE000A7AEF53E3FF1A0031");		  // ,较差换行/n端口：1
///***************************网络信号质量 ************************************/
///*如果数值小于10，很差，20> 一般 >=10  良好>20*/	
//	if(signalval > 20)strcat(MsegTemp,"FF0C826F597D000A");		  // ,良好换行
//	else if(signalval > 10)strcat(MsegTemp,"FF0C4E00822C000A");		  // ,一般换行
//	else strcat(MsegTemp,"FF0C8F835DEE000A");		  // ,较差换行
///***************************插入一行 温度 湿度 ************************************/		
//	char *Dis_unicode = (char*)portMalloc(30*sizeof(u8) ) ;
//		gcvt(SENx.T,2,Dis_unicode);
//		cdma_mobile_txt_shift(Dis_unicode,5);	//字符串转换成UNICODE
//		strcat(MsegTemp,"6E295EA6FF1A");	//温度：	
//		strcat(MsegTemp,Dis_unicode);	//显示湿度数据
//		strcat(MsegTemp,"00200020");	//空格 空格
//		
//		gcvt(SENx.RH,2,Dis_unicode);
//		cdma_mobile_txt_shift(Dis_unicode,5);	//字符串转换成UNICODE
//		strcat(MsegTemp,"6E7F5EA6FF1A");	//湿度：	
//		strcat(MsegTemp,Dis_unicode);	//
////		strcat(MsegTemp,"000A7AEF53E3FF1A0031");	//换行/n端口：1	
//	portFree(Dis_unicode) ;			
///****************************服务器下行消息 上传经纬度***********************************/
//		if(MQTT_Infor)
//		{
//		strcat(MsegTemp,"000A");	//换行		
//		strcat(MsegTemp,"LBS:");		
//		strcat(MsegTemp,LBS.lon);	//经度
//		strcat(MsegTemp,LBS.lat);	//度
//		}
//		strcat(MsegTemp,"000A7AEF53E3FF1A0031");	//换行/n端口：1			
///***************************************************************/
//	if(LED8)strcat(MsegTemp,NCNicode);						  //1回路 Trigger8
//	else strcat(MsegTemp,NONicode);
//	strcat(MsegTemp,NumUnicode[2]);
//	if(LED7)strcat(MsegTemp,NCNicode);
//	else strcat(MsegTemp,NONicode);
//	strcat(MsegTemp,NumUnicode[3]);
//	if(LED6)strcat(MsegTemp,NCNicode);
//	else strcat(MsegTemp,NONicode);
//	strcat(MsegTemp,NumUnicode[4]);
//	if(LED5)strcat(MsegTemp,NCNicode);
//	else strcat(MsegTemp,NONicode);
//	strcat(MsegTemp,NumUnicode[5]);
//	if(LED4)strcat(MsegTemp,NCNicode);
//	else strcat(MsegTemp,NONicode);
//	strcat(MsegTemp,NumUnicode[6]);
//	if(LED3)strcat(MsegTemp,NCNicode);
//	else strcat(MsegTemp,NONicode);
//	strcat(MsegTemp,NumUnicode[7]);
//	if(LED2)strcat(MsegTemp,NCNicode);					 //7回路
//	else strcat(MsegTemp,NONicode);
//	strcat(MsegTemp,NumUnicode[8]);		   
//	if(LED1)strcat(MsegTemp,NCNicode);					//8回路Trigger1
//	else strcat(MsegTemp,NONicode);
///********433M 功能正常 暂时信用 备后续启用*********/	
//	strcat(MsegTemp,"000A003400330033004DFF1A");//换行433M:
//	if(W433Hard.Head.W433Model == 0x11)strcat(MsegTemp,W433_SelfHold);//自锁模式 0x11
//	else if(W433Hard.Head.W433Model == 0x22)strcat(MsegTemp,W433_InterLock);//互锁模式 0x22
//	else if(W433Hard.Head.W433Model == 0x33)strcat(MsegTemp,W433_OFF);//遥控关闭 0x33
//	else strcat(MsegTemp,W433_Inching);//点动模式 0x00
///*********END***********/
//}
//查询当前状态，获取主叫号码
void CurState(char *Source)
{

#ifdef  message_model	//TXT
		/*************电信****************/
	if(ModelFlag)
		{
	//TXT
		if(hand(Source,loose_code))		//+CMT: 0031 松散编码  
			{mobile_txt_post_phone(Source);}
		else {cdma_txt_post_phone(Source);} //+CMT: 1 紧凑编码	
		}
	/*************移动联通****************/
	else 
		{
	//TXT
		if(hand(Source,loose_code))		//+CMT: 0031 松散编码		  
			{mobile_txt_post_phone(Source);}
		else {cdma_txt_post_phone(Source);} //+CMT: 1 紧凑编码
		}			
#else		 //PDU
		/*************电信****************/
	if(ModelFlag)
		{
			//PDU 
			CDMA_pdu_query(Source);	//ObjLen = 	
		}
	/*************移动联通****************/
	else 
		{
			 //PDU
			mobile_pdu_query(Source);	//ObjLen = 
		}			
#endif			
}   

u8 hand(const char *soucer,const char *compare)
{
	if(strstr(soucer,compare)!=NULL)
//	if(kmp(soucer, compare)>=0)	
		return 1;
	else
		return 0;
}

////******************************短信号码存储***********************************************************
////Target  组号; 
////*Sour 	 号码字符串
//void  EEpro_PDU(char *soucer, u32 *TrageAdd,u8 Target,char *Sour)
//{
//u8 StrLen = 12, i = 0,y=0;
////char Temp;
//u8 PhoneAscii[14]={0};
//	
//	StrLen = strlen(Sour); StrLen = StrLen/4;
////char *tt="13692190284";

//	if(hand(soucer,"@"))		// 紧凑编码
//	   {	StrLen = strlen(Sour);
//	   		for(i=0; i<StrLen; i++)
//			{
//			PhoneAscii[i] =Sour[i];
//			}
////			PhoneAscii[11] = '\0';
//	   }
//	else						  		//松散编码
//	   {	StrLen = strlen(Sour); StrLen = StrLen/4;
//	   		for(i=0,y=1; i<StrLen; i++,y++)
//			{
//			PhoneAscii[i] =Sour[y*4-1];
//			}
////			PhoneAscii[11] = '\0';
//		}

//		 EEPROM_write_n(TrageAdd[Target]+1,PhoneAscii,StrLen);
//		 EEPROM_write_Byte(TrageAdd[Target],StrLen);					//地址首位，存储号码长度．
//}
////******************************拔打电话的号码存储***********************************************************
////Target  组号; 
////*Sour 	 号码字符串
//void  EEpro_Phone(char *soucer,u32 *TrageAdd,u8 Target,char *Sour)
//{
//	u8 StrLen = 12, i = 0,y=0;
//	u8 PhoneAscii[12]={0};
//	
//	if(hand(soucer,"&"))		// 紧凑编码
//	   {	StrLen = strlen(Sour);
//	   		for(i=0; i<StrLen; i++)
//			{
//			PhoneAscii[i] =Sour[i];
//			}
//	   }
//	else				//松散编码
//	   {	StrLen = strlen(Sour); StrLen = StrLen/4;
//	   		for(i=0,y=1; i<StrLen; i++,y++)
//			{
//			PhoneAscii[i] =Sour[y*4-1];
//			}
//		}
//	EEPROM_write_n(TrageAdd[Target]+1,PhoneAscii,StrLen);
////	EEPROM_write_Byte(TrageAdd[Target],11);					//地址首位，存储号码标识．
//	EEPROM_write_Byte(TrageAdd[Target],StrLen);				//号码长度
//}

/*******************END************************/
//功能：自动识别主叫号码 1 松散编码	0 紧凑编码 
void main_call_code(char*soucer)
{

 if((hand(soucer,"0040"))||(hand(soucer,"0026"))||(hand(soucer,"002A"))||(hand(soucer,"0023"))||(hand(soucer,"0025")))//松散编码@,&,*,#,%unicode
//	else	
		{loose_model = 1;
		cut_str =  StrNeed; start_meg = NumberCode; //start_ext_meg	= NuExCode;
		start_phone = PoneCode; AlarmTemperStr = SetTemerHumUnic;//start_ext_phone = PoneExCode;

		close1 = needle1; close2 = needle2; close3 = needle3; close4 = needle4; 
		close5 = needle5; close6 = needle6; close7 = needle7; close8 = needle8;
		
		open1 = Open_1; open2 = Open_2; open3 = Open_3; open4 = Open_4;
		open5 = Open_5; open6 = Open_6; open7 = Open_7; open8 = Open_8;
		}
//	if((hand(soucer,"@"))||(hand(soucer,"&"))||(hand(soucer,"*"))||(hand(soucer,"#"))||(hand(soucer,"%")))  //紧凑编码
else
		{loose_model = 0;// 松散编码	0   紧凑编码 1
		cut_str = CDMA_StrNeed; start_meg = CDMA_NumberCode; //start_ext_meg = CDMA_NuExCode;//
		start_phone = CDMA_PoneCode; AlarmTemperStr = SetTemerHum;//start_ext_phone = CDMA_PoneExCode;

		close1 = CDMA_needle1; close2 = CDMA_needle2; close3 = CDMA_needle3; close4 = CDMA_needle4; 
		close5 = CDMA_needle5; close6 = CDMA_needle6; close7 = CDMA_needle7; close8 = CDMA_needle8;
		
		open1 = CDMA_Open1; open2 = CDMA_Open2; open3 = CDMA_Open3; open4 = CDMA_Open4;
		open5 = CDMA_Open5; open6 = CDMA_Open6; open7 = CDMA_Open7; open8 = CDMA_Open8;
		}		
		
//Uart1BYTE(Hex2Ascii(loose_model));	   //测试
}

#ifdef 	message_model  //TXT
void response(char*soucer,u16 lenght,char *message )
{
		OCCUPY_EC20(20*configTICK_RATE_HZ);  //等待占用EC20资源 等20S	
//u8 len;
//	if(ModelFlag)		   //运营商 1 电信 
//	{
			//TXT
	if(loose_model) 		//+CMT: 0031   松散编码
		{
		mobile_txt_post_phone(soucer);
		cdma_mobile_txt_shift(PhoneNumber,txt_smsphone_lenght);
		mobile_txt_message(PhoneNumber,message);
		}
	else				   //+CMT: 1	   紧凑编码
		{
		cdma_txt_post_phone(soucer);
		cdma_mobile_txt_shift(PhoneNumber,txt_smsphone_lenght);
		cdma_txt_message(PhoneNumber,message);
		}
//	}
//	else				 //运营商  0 移动
//	{
//	 //TXT
//	if(loose_model) 		//+CMT: 0031   松散编码
//		{
//		mobile_txt_post_phone(soucer);
//		cdma_mobile_txt_shift(txt_smsphone_lenght);
//		mobile_txt_message(PhoneNumber,message);
//		}
//	else				   //+CMT: 1	   紧凑编码
//		{
//		cdma_txt_post_phone(soucer);
//		cdma_mobile_txt_shift(txt_smsphone_lenght);
//		cdma_txt_message(PhoneNumber,message);
//		}
//	}
	RELESE_EC20();		                      		//释放EC20资源			
}
//检测是否是运营商发送的欠费信息 转发给用户
void Arrearage(char*soucer)
{
	char *haystack;//=Content;
	char *buf;
	
	if(loose_model) 		//+CMT: 0031   松散编码
		{
		haystack = mobile_txt_post_phone(soucer);
		if(hand(PhoneNumber,"10001")||hand(PhoneNumber,"10086")||hand(PhoneNumber,"10011"))	//10001 10086 10011
			{
				u8 PhoneNumLen = EEPROM_read_Byte(MessgeAddr[1]);		//向停电的第一个短信接收号码 转发信息
				if((PhoneNumLen > 2)&&(PhoneNumLen < 15))
				{
				memset(PhoneNumber,0,PhoneLenght);
				EEPROM_read_n((MessgeAddr[1]+1), PhoneNumber, PhoneNumLen);   //号码长度	
				cdma_mobile_txt_shift(PhoneNumber,PhoneNumLen);//被叫号码转换成UNICODE码
					
				buf= strstr( haystack, "\n");
				haystack = buf+1;
				buf= strstr( haystack, "\n");
				buf[0]='\0';
				strncpy (MsegTemp,haystack,Mseg_Lenth-4);
		OCCUPY_EC20(20*configTICK_RATE_HZ);  //等待占用EC20资源 等20S			
				mobile_txt_message(PhoneNumber,MsegTemp);
		RELESE_EC20();		                      		//释放EC20资源
				memset(MsegTemp, 0, Mseg_Lenth) ;	
				}

			}			
		}

//	else				   //+CMT: 1	   紧凑编码
//		{
//		cdma_txt_post_phone(soucer);
//		cdma_mobile_txt_shift(PhoneNumber,txt_smsphone_lenght);
//		cdma_txt_message(PhoneNumber,message);
//		}

}
#else							 //PDU
void response(char*soucer,u8 lenght,char *message )
{
u8 len;
	if(ModelFlag)		   //运营商 1 电信 
	{
			//PDU 
		if(loose_model) 		//+CMT: 0031   松散编码
		{
		len = mobile_txt_post_phone(soucer);
		}
		else				   //+CMT: 1	   紧凑编码
		{
		len = cdma_pdu_post_Phone(soucer);
		}
		reponse_cdma_pdu(len,lenght,message);
	}
	else				 //运营商  0 移动
	{
				 //PDU 
		if(loose_model) 		//+CMT: 0031   松散编码
		{
		len = mobile_txt_post_phone(soucer);
		}
		else				   //+CMT: 1	   紧凑编码
		{
		len = cdma_txt_post_phone(soucer);
		}
//	len = mobile_txt_post_phone();	//获取主叫号码 存在PhoneNumber[]中	 向PhoneNumber发送 设置成功
	response_mobile_pdu(len,lenght,message);
	}
}
#endif


//初始化自动查询短信中心号码
ErrorStatus CSCA_CHCK(u32 CSCA_Adrrs,u32 offset_Addr)
{
if(RUNERR == EC20_SendModuleCmd(AT_CSCA, NULL ))return ERROR ;
	
//	如果查询到中心号码 对其保存以便下次 找不到中心号码时 重新幅值	
  char *haystack,*buf;
	 haystack= ec20AtBuf;	
	 buf= strstr( haystack,"\"");
	 Tled=0;

	 haystack = buf + strlen("\"");
   buf = strstr( haystack, "\"");
   buf[0]='\0';
u8	StrLen = strlen(haystack);  
	if(( StrLen > 4 )&&(StrLen < 0xff))
		{
		if(EEPROM_read_Byte(CSCA_Adrrs + offset_Addr) != 0xff)return SUCCESS;		//已经有保存值
		EEPROM_write_Byte(CSCA_Adrrs + offset_Addr,StrLen);					//地址首位，存储号码长度．
//		EEPROM_write_n(CSCA_Adrrs+offset_Addr+4,(uint8_t*)haystack,StrLen);
		SectorErase_protect(CSCA_Adrrs, (u8*)haystack,4,StrLen,0);			
		return SUCCESS;		
		}
return ERROR ;
}

//设置短信中心号码
//读取上次存储的中心号码 对现在的SIM卡进行设置
void CSCA_CMD(u32 CSCA_Adrrs,u32 offset_Addr)
{

u8 len = EEPROM_read_Byte(CSCA_Adrrs+offset_Addr);//0x08
	if((2 < len) && (len < 0xff))
	{
	memset(MsegTemp,0,Mseg_Lenth);
	EEPROM_read_n((CSCA_Adrrs+offset_Addr+4),MsegTemp,len);
//	snprintf(send_dat,49,"AT+CSCA=\"%s\",145\r\n",MsegTemp);
	EC20_SendModuleCmd(AT_CSCA, NULL,MsegTemp);
	}
}

//用户发短信进行设置短信中心号码
//u32 CSCA_Adrrs扇区地址 u32 OffsetAdd  偏移量
void User_SetCSCA(u32 CSCA_Adrrs,u32 OffsetAdd)
{
  char *haystack;//=Content;
	char* buf;
	char *Model_StrNeed;
	
	 Model_StrNeed = cut_str;
	 haystack= ec20AtBuf;	
	 buf= strstr( haystack, Model_StrNeed);
	 Tled=0;

	 haystack = buf + strlen(Model_StrNeed);
    //* Get next token: 
   buf = strstr( haystack, Model_StrNeed);
   buf[0]='\0';
	EEPROM_SectorErase(CSCA_Adrrs);				   //擦除对应的扇区 
 
	if( buf != NULL )
		{
u8 StrLen=0, i = 0,y=0;
//u8 PhoneAscii[13]={0};
u8 *PhoneAscii = portMalloc(15*sizeof(uint8_t));
	StrLen = strlen(haystack); 
	if(hand(haystack,"@"))	// 紧凑编码
	   {	
	   	for(i=0; i<StrLen; i++)
			{
			PhoneAscii[i] =haystack[i];
			}
	   }
	else						  		//松散编码
	   {
		 StrLen = StrLen/4;
	   	for(i=0,y=1; i<StrLen; i++,y++)
			{
			PhoneAscii[i] =haystack[y*4-1];
			}
		 }
		 PhoneAscii[i+1] = '\0';
//		 EEPROM_write_Byte(CSCA_Adrrs,StrLen);					//地址首位，存储号码长度．
//		 EEPROM_write_n(CSCA_Adrrs +4,(uint8_t*)PhoneAscii,StrLen);
		SectorErase_protect(CSCA_Adrrs, PhoneAscii,OffsetAdd,StrLen+1,0);
		portFree(PhoneAscii);		 
		}
	response(ec20AtBuf,18,Sccuces);
	Ec20AtBufReset() ;  
}
//unicode码转换为 HEX数值 最大2位
//strlen 1 2 4 8 
ErrorStatus Strtohex(u8 *target,char *sour,u8 strlen)
{
	u8 i,y;
		if(strlen <= 2)i = 1;
	else {i = 4;}
	
//		for(y=1; y<3; y++)//0 1 3 7
	for(y=1; (y*i)<=strlen; y++)//0 1 3 7
			{
				if(y == 2){*target <<= 4;*target |= (sour[y*i-1] & 0x0f);}
				else *target = (sour[y*i-1] & 0x0f);
			}
			
	return SUCCESS;
}
//@8#1369219***8#  第8回路6组存储的号码
//@1#1369219***8#  第1回路
//Topic: /download/sms_phone  "call_phone" 
//Message: port=8&phone1=电话号码& phone2=电话号码& phone3=电话号码& phone4=电话号码& phone5=电话号码& phone6=电话号码 
SetResultType NumberSet(char *source)
{
	char *haystack, *buf,*Model_StrNeed,*find_char;//
u32 SectionAddr,OffsetAdd;//地址
u16 StrLen, i,	y;		
u8 Numbre;
u8 Len_StrNeed;	
uint8_t all_set = 0;	
		 haystack= source;
	if(DeivceFlag.SIMorMQTT)//mqtt数据   1 mqtt  0 sim
	{
		if( hand(source, "sms_phone")){OffsetAdd = SMS_Size0; }			//find_char = "&"; 短信号码设置前3组 保护另一段数据	 0 unico码   1字符串
		else if(hand(source,"call_phone")){OffsetAdd =  Phone_Size0;}	//find_char = "&"; 电话号码设置前3组 保护另一段数据
		else return SetNull;
	 Model_StrNeed = "&phone";//分隔字符串 &phone1=		
	 Len_StrNeed = strlen(Model_StrNeed)+2;	// 后移补偿2字节 1=
	 buf= strstr( haystack, "port=");//头字符串
	 haystack = buf + 5;	//后移长度 port=	
	}
	else			//sim数据
	{
/************************短信接收号码*****************************/
		if( hand(source, start_meg)){OffsetAdd = SMS_Size0; find_char = start_meg;}			// 短信号码设置前3组 保护另一段数据	 0 unico码   1字符串
//		else if(hand(source,start_ext_meg)){OffsetAdd = SMS_Size1; find_char = "@";}
/************************拔打电话号码*****************************/		
		else if(hand(source,start_phone)){OffsetAdd =  Phone_Size0;find_char = start_phone;}			// 电话号码设置前3组 保护另一段数据
//		else if(hand(source,start_ext_phone)){OffsetAdd =  Phone_Size1;find_char = "&";}
		else return SetNull;	
	 Model_StrNeed = cut_str;//"0023"  "#"	
	 Len_StrNeed = strlen(Model_StrNeed);	
	 buf= strstr( haystack, find_char);//
	 haystack = buf + strlen(find_char);		
	}

	 Tled=0;			
/************************获取回路序号 字符/ ****************************/
	 buf = strstr( haystack, Model_StrNeed);		
if( buf != NULL )
	{
    buf[0]='\0';
		StrLen = strlen(haystack);	
	}
	if((StrLen > 8)&&(DeivceFlag.SIMorMQTT == 0))//大于2位数字 且 SIM模式回复短信   1 mqtt  0 sim
	{response(source,18,SetFail);return SetERR;}//设置失败
	
//else return SetERR;	//没有数据
//else StrLen = '1';	// 为兼容之前版本 @# 默认为设置1回路
	
//	if(*haystack == 'a'){Numbre = 'a';}//24-11-22增加指令@a#同时设置所有的端口号码 包括停电来电接收号码
//	else
	if(StrLen != 0)
		{
		Strtohex(&Numbre,haystack,StrLen);if(Numbre == 0x10)Numbre = 10;
		if((Numbre < 1)||(Numbre > 10)) {response(source,18,SetFail);return SetERR;}//设置失败
		if(Numbre > 8)SectionAddr = MessgeAddr[Numbre-9];// 温度湿度，停电
		else SectionAddr = MessgeAddr[Numbre+1];//MessgeAddr[Numbre-1];
		}
		else all_set = 1;//24-11-27增加指令@a#同时设置所有的端口号码 包括停电来电接收号码

haystack = buf + Len_StrNeed;	   
buf = strstr( haystack, Model_StrNeed);
/*********************存储被叫号码******************************/			
char *PortBuf = portMalloc(512*sizeof(uint8_t));//257
char *EEprBuf = PortBuf;
u16 Total_len=0;	
//		memset(PortBuf,0,512);
		while( buf != NULL )
		{
    	buf[0]='\0';
/**************************/
		StrLen = strlen(haystack);	
				
//	if(hand(haystack,find_char))		
	if((!loose_model)|(DeivceFlag.SIMorMQTT))//紧凑编码0   or   mqtt数据
	   {
			*EEprBuf++ = StrLen;	 
	   	for(i=0; i<StrLen; i++)
			{
			*EEprBuf++ =haystack[i];
			}
//			*EEprBuf++ = '\0';
	   }
	else						  		//松散编码1
	   {	StrLen /= 4;
			*EEprBuf++ = StrLen ;
	   	for(i=0,y=1; i < StrLen ; i++,y++)
			{
			*EEprBuf++ =haystack[y*4-1];
			}
//			*EEprBuf++ = '\0';
		}
//		 Total_len += i;
		 Total_len += Num_Interval;		//短信,电话接收号码 地址间隔
//if(Total_len < 256)EEprBuf += (Num_Interval - StrLen-1);		 //0x40
if(Total_len < 512)EEprBuf += (Num_Interval - StrLen-1);		 //0x40		
/**************************************************************/			
			haystack = buf + Len_StrNeed;
		    //* Get next token: 
		  buf = strstr( haystack, Model_StrNeed);
		}
//		if(Numbre == 'a')//24-11-22增加指令@a#同时设置所有的端口号码 包括停电来电接收号码
		if(all_set)//24-11-27增加指令@a#同时设置所有的端口号码 包括停电来电接收号码
		{
			for(uint8_t i=0; i<10; i++)
//			SectorErase_protect(MessgeAddr[i], (u8*)PortBuf,OffsetAdd,Total_len,0);//只存储已接收到的的号码，下一组清0，剩下的号码保持不变
			SectorErase_protect(MessgeAddr[i], (u8*)PortBuf,OffsetAdd,384,0);//存储接收到的的号码，并将剩下的号码清0
			sprintf(EEprBuf,str_set_Allprot);//"516890E87AEF53E3"全部端口
		}
		else
//		{SectorErase_protect(SectionAddr, (u8*)PortBuf,OffsetAdd,Total_len,0);//只存储已接收到的的号码，下一组清0，剩下的号码保持不变
		{SectorErase_protect(SectionAddr, (u8*)PortBuf,OffsetAdd,384,0);////存储接收到的的号码，并将剩下的号码清0
			
		if(Numbre == 9)strcpy(EEprBuf,str_set_TemHum);//"6E296E7F5EA6"温湿度
		else if(Numbre == 10)strcpy(EEprBuf,str_set_PowOffOn);//"505C67657535"停来电
		else sprintf(EEprBuf,"7B2C003%1d7AEF53E3",Numbre);//第n端口
		}
		if((OffsetAdd == SMS_Size0)||(OffsetAdd == SMS_Size1))
		//			snprintf(EEprBuf,100,"7B2C003%1d7AEF53E377ED4FE153F77801002C8BBE7F6E6210529F",Numbre);//第n端口短信号码,设置成功
			strcat(EEprBuf,str_set_msg_phone);//"77ED4FE153F77801002C8BBE7F6E6210529F"短信号码,设置成功
		else 
		//			snprintf(EEprBuf,100,"7B2C003%1d7AEF53E375358BDD53F77801002C8BBE7F6E6210529F",Numbre);//第n端口电话号码,设置成功
			strcat(EEprBuf,str_set_call_phone);//"75358BDD53F77801002C8BBE7F6E6210529F"电话号码,设置成功
		if(DeivceFlag.SIMorMQTT == 0)//SIM模式回复短信   1 mqtt  0 sim
		response(source,52+2,EEprBuf);		
	
//		response(source,strlen(EEprBuf),EEprBuf);
		portFree(PortBuf);	 
//		memset(SIMQueue.dataBuf,0,MAX_QUEUE_LEN);				  //清除内容				  //清除内容
return SteOK;		
}
//	if((StrLen == 1)||(StrLen == 2))		// 紧凑编码//@#8#1369219***8# 
//	   {
//			strncpy(&Numbre,haystack,StrLen);
//			Numbre = atoi(&Numbre);
//			SectionAddr = MessgeAddr[Numbre-1];//atoi(&Numbre)
//			     //* Get next token: 
//			haystack = buf + Len_StrNeed;	   
//			buf = strstr( haystack, Model_StrNeed);
//	   }
//	else if((StrLen == 4)||(StrLen == 8))			//松散编码//@#0038#1369219***8# 
//	   {
//			strncpy(&Numbre,&haystack[3],StrLen);
//			Numbre = atoi(&Numbre);
//			SectionAddr = MessgeAddr[Numbre-1];//atoi(&Numbre)
//			     //* Get next token: 
//	haystack = buf + Len_StrNeed;	   
//	buf = strstr( haystack, Model_StrNeed);
//		 }
		 
//	else// if(StrLen == 0)		//@##1369219***8# //@#1369219***8# 
//		{
//		Numbre = '1';
//		Numbre = atoi(&Numbre);
//		SectionAddr = MessgeAddr[Numbre-1];
//			    //* Get next token: 
//	haystack = buf + Len_StrNeed;	   
//	buf = strstr( haystack, Model_StrNeed);
//		}
//	else								//@#1369219***8# 
//		{
//		SectionAddr = 	MessgeAddr[0]; 
//		}	
//MQTT 远程 撤布防控制 开启 或 关闭
//开启后 可以在公众号上 远程选择报警模块报警途径 
//信息格式：RMTON  RMTOFF
SetResultType RemoteCtr_Enable(char *soucer)
{
	u8 Buffer[2]={0};//0     SIM卡和上传到MQTT服务器报警  //MQTT远程撤、布防控制 1开启 0关闭
		if	(hand(soucer,RMTON))Buffer[1]=ENABLE;//W433Hard.Head.Alarm_Remoter_Enable = ENABLE;1开启    远程撤、布防控制 
		else if(hand(soucer,RMTOFF))Buffer[1]=DISABLE;//W433Hard.Head.Alarm_Remoter_Enable = DISABLE;//0关闭
		else  return SetNull;//没有数据
		SectorErase_protect(CSCA_W433_Address, Buffer,Alarm_Mode_offAddr,2,0);	//报警方式 存储地址 0x0801F000 +偏移0x40+9
		EEPROM_read_n(CSCA_W433_Address+Alarm_Mode_offAddr, (char *)W433Hard.Head.DeviceAlarmMode, 2);	
		response(soucer,18,Sccuces);
//		MQTT_DeviceState();		  //发布设备数据 Topic: /upload/home	
return SteOK;			
}

//短信&公众号 设置温度湿度报警阀值  
//温湿度TH#23.5#2#HH#85.2#10.0#  
//Topic: /download/SetTemperHum 
//Message:TH#23.5#2#85.2#10.0#   if ## 清除当前段数据
SetResultType SET_AlarmValue_TemperHum(char *source)
{

char *haystack, *buf,*Model_StrNeed;
//u32 SectionAddr,OffsetAdd;//地址
u16 StrLen, i,	y;		
//u8 modelfromat=0;// 1 松散编码	  0 紧凑编码	
u8 Len_StrNeed;	

	 haystack= source;
//char tt[]={"/SetTemperHumMessage:TH#23.5#2#85.2#10.0#"};	
//DeivceFlag.SIMorMQTT=1;	 source= tt;haystack= source;
	if(DeivceFlag.SIMorMQTT)//mqtt数据   0 sim 1 mqtt
	{
		if( hand(source, "/SetTemperHum"));			//{modelfromat=0;} 
		else return SetNull;
	 Model_StrNeed = "#";//分隔字符串 		
	
	}
	else			//sim数据
	{
/************************接收*****************************/
		if( hand(source,AlarmTemperStr));			//温湿度识别符"TH#" 
		else return SetNull;	
	 Model_StrNeed = cut_str;//"0023"  "#"	

	}
	 Len_StrNeed = strlen(Model_StrNeed);	// 后移补偿2字节 1=
	 buf= strstr( haystack, AlarmTemperStr);//头字符串
	 haystack = buf + strlen(AlarmTemperStr);	//后移长度 TH#
	 Tled=0;			
/***********************  ****************************/
	
/*********************存储报警阀值******************************/			
char *PortBuf = portMalloc(20*sizeof(uint8_t));//257
int *EEprBuf = &AlarmValue.ValueUnit.Temper_H;
buf= strstr( haystack, Model_StrNeed);//头字符串	
		while( buf != NULL )
		{
    	buf[0]='\0';
/**************************/
		StrLen = strlen(haystack);	
			if(StrLen>12)StrLen=12;	
	if((!loose_model)|(DeivceFlag.SIMorMQTT))//紧凑编码0   or   mqtt数据
	   {
//			*PortBuf++ = StrLen;	 
	   	for(i=0; i<StrLen; i++)
			{
			*(PortBuf+i) =haystack[i];
			}
	   }
	else						  		//松散编码
	   {	StrLen /= 4;
//			*PortBuf++ = StrLen ;
	   	for(i=0,y=1; i < StrLen ; i++,y++)
			{
			*(PortBuf+i) =haystack[y*4-1];
			}
		}
//		 if(*PortBuf == 'D')*PortBuf = '-';

		if(StrLen == 0){*PortBuf = 0xff;i++;}//## 清除当前段数据
			*(PortBuf+i) = '\0';
if(*PortBuf == 'D')
	{*PortBuf = '0';	
	*EEprBuf = ~atoi(PortBuf)+1;// 负数求补码
	}
else		
*EEprBuf = atoi(PortBuf);
EEprBuf++;
/**************************************************************/			
			haystack = buf + Len_StrNeed;
		  buf = strstr( haystack, Model_StrNeed);
		}
portFree(PortBuf);	
		SectorErase_protect(MessgeAddr[TemperHum_AddrSection], AlarmValue.ValueArry,TemperHum_AddrOffset,TemHumSaveLen,0);

	if(!DeivceFlag.SIMorMQTT)response(source,18,Sccuces);//mqtt数据   0 sim 1 mqtt		
	MQTT_AlarmTemHum();		  //发布温度和湿度 上下限报警阀值
		
//		memset(SIMQueue.dataBuf,0,MAX_QUEUE_LEN);				  //清除内容				  //清除内容
return SteOK;	
} 

//短信设置主机工作模式 RS485主机 RS485从机 独立模式 三种
SetResultType WorkingMod(char *source)
{
	char *haystack, *buf,*Model_StrNeed;//
	u16 StrLen;		
	u8 Numbre;
	
	 haystack = source;		//sim数据

	if(hand(source, RS485_hostMod)){RegisterEEpro_Slave.Host_or_Slave = Host_Flag;}			//RS485主机#
	else if(hand(source,RS485_SlaveMod)){RegisterEEpro_Slave.Host_or_Slave = Slave_Flag;}			//RS485从机#
	else if(hand(source,Stand_Mod)){RegisterEEpro_Slave.Host_or_Slave = Eight_chnnal_Flag;}		//独立模式#
	else return SetNull;
	
	if(RegisterEEpro_Slave.Host_or_Slave == Slave_Flag) //RS485从机#从机地址号#
	{
		buf= strstr( haystack, RS485_SlaveMod);//
		haystack = buf + strlen(RS485_SlaveMod);
/************************#从机地址号# ****************************/			
		Model_StrNeed = cut_str;//"0023"  "#"	
		buf = strstr( haystack, Model_StrNeed);		
		if( buf != NULL )
			{
				buf[0]='\0';
				StrLen = strlen(haystack);	
			}
		else {response(source,18,SetFail);return SetERR;}//设置失败	没有数据
		
	Strtohex(&Numbre,haystack,StrLen);if(Numbre == 0x10)Numbre = 10;
	if((Numbre < 1)||(Numbre > 10)) {response(source,18,SetFail);return SetERR;}//设置失败
	RegisterEEpro_Slave.LocalAddr =  Numbre;//从机的本地地址
	}
	else RegisterEEpro_Slave.LocalAddr =  HostAddr;
	Tled=0;			

	SectorErase_protect(CSCA_W433_Address, &RegisterEEpro_Slave.Host_or_Slave, RS485_AddrOffset, sizeof(RegisterEEpro_Slave),0);
	response(source,18,Sccuces);// 设置成功

return SteOK;		
}
/***wifi 配置帐号&密码***/
SetResultType WIFI_linkNet(char *source)
{
	char *haystack, *buf,*Model_StrNeed;//
	u16 StrLen;		
	SetResultType result = SteOK;
	
	 haystack = source;		//sim数据
	if(strstr(haystack,wifi_setNet)==NULL)return SetNull;			//wifi配网#

		buf= strstr( haystack, wifi_setNet);//
		haystack = buf + strlen(wifi_setNet);
/*********************wifi_ssid**************************/			
		Model_StrNeed = cut_str;//"0023"  "#"	
		buf = strstr( haystack, Model_StrNeed);	
char *wifi_ssid = portMalloc(20*sizeof(uint8_t));
char *wifi_ssid_sP = wifi_ssid;	
	
u8 i;	// long value = strtol(str, NULL, 16);
		if( buf != NULL )
			{
				buf[0]='\0';
				StrLen = strlen(haystack);
				for(i=2;i<StrLen;i+=4,wifi_ssid_sP++)	
				{
					*(haystack + i+2) = 0;//隔4个字节清0一次
					*wifi_ssid_sP = strtol((haystack + i), NULL, 16);//将字符串解析为十六进制数值（字符串转HEX整数）如"1A3F"解析为0x1A3F
				}		
			}
//			else {response(source,18,SetFail);result = SetERR;}//设置失败	没有数据
/*********************wifi_password*************************/					
			haystack = buf + strlen(Model_StrNeed);
			buf= strstr( haystack, Model_StrNeed);//
char *wifi_password = portMalloc(20*sizeof(uint8_t));	
char *wifi_password_sp = wifi_password;	
		if( buf != NULL )
			{
				buf[0]='\0';
				StrLen = strlen(haystack);
				for(i=2;i<StrLen;i+=4,wifi_password_sp++)	
				{
					*(haystack + i+2) = 0;//隔4个字节清0一次
					*wifi_password_sp = strtol((haystack + i), NULL, 16);
				}		
			}		  
		else {result = SetERR;}//设置失败	没有数据
if(result == SteOK)
{	
WIFI_SendModuleCmd(AT_WJAP,NULL,wifi_ssid,wifi_password);
Tled=0;			
//response(source,18,Sccuces);// 设置成功
}
//else response(source,18,SetFail);//设置失败
portFree(wifi_ssid);
portFree(wifi_password);	

return result;		
}
//短信&公众号 设置 报警方式  
//mode: 0，全功能。  1，SIM卡短信电话。2，物联卡上传报警信号 。3禁止报警，仅传送状态数据至公众号 
SetResultType SET_Alarm_Mode(char *source)
{
u8 Numbre;
	
	if(DeivceFlag.SIMorMQTT)//mqtt数据   0 sim 1 mqtt
		{
		if(hand(source, "/download/ALARM"))
			{ 
			Numbre = MQTT_GetPurtNum(source,"/ALARM");
			W433Hard.Head.DeviceAlarmMode = (DeviceAlarmModeTYPE)Numbre;
			SectorErase_protect(CSCA_W433_Address, &Numbre,Alarm_Mode_offAddr,1,0);	//报警方式 存储地址 0x0801F000 +偏移0x40+9	
			}
			else return SetNull;
		}	
	else			//sim数据
	{
		if(hand(source, AlarmMode_SIM)){Numbre = 1;}//SIM卡报警
		else if(hand(source, AlarmMode_IOT)){Numbre = 2;}//物联卡报警
		else if(hand(source, AlarmMode_FULL)){Numbre = 0;}//全功能报警
		else if(hand(source, AlarmMode_OFF)){Numbre = 3;}//关闭报警
		else return SetNull;//没有数据 
		response(source,18,Sccuces);	
	}
	SectorErase_protect(CSCA_W433_Address, &Numbre, Alarm_Mode_offAddr, 1,0);	//报警方式 存储地址 0x0801F000 +偏移0xc0
	Numbre = EEPROM_read_Byte(CSCA_W433_Address+Alarm_Mode_offAddr);	////报警方式 存储地址 0x0801F000 +偏移0xc0
	W433Hard.Head.DeviceAlarmMode = (DeviceAlarmModeTYPE)Numbre;
//	MQTT_DeviceState();		  //发布设备数据 Topic: /upload/home

return SteOK;		
} 
//擦除一个扇区 回路消息 电话号码
//			"64E69664002300380023" 擦除#8#
//擦除#8#  擦除第8回路扇区的全部信息 
SetResultType EraseSector(char *source)
{
char *haystack, *buf,*Model_StrNeed;
u32 SectionAddr;//地址
u16 StrLen;
u8 	Numbre;
	
if(DeivceFlag.SIMorMQTT)//mqtt数据   0 sim 1 mqtt
	{	
		if(hand(source,"/download/ERASE"))		  //端口->擦除信息
		{
			Numbre = MQTT_GetPurtNum(source,"/ERASE");
			if(((Numbre > 0))&&(Numbre < 0xff))EEPROM_SectorErase(MessgeAddr[Numbre+1]);//(MessgeAddr[Numbre-1]);
			else return SetERR;
		}
		else	return SetNull;//没有数据	
	}	
	else			//sim数据
	{
	if(!hand(source, EraseStr))return SetNull;//没有数据 "64E696640023" 擦除#
	 Model_StrNeed = cut_str;//"0023"  "#"
	 haystack= source;
	 StrLen = strlen(Model_StrNeed);
/************************获取回路序号 字符/ ****************************/
	 buf= strstr( haystack, Model_StrNeed);
	 haystack = buf + StrLen;
	 buf = strstr( haystack, Model_StrNeed);	
		if( buf != NULL )
			{
				buf[0]='\0';
			StrLen = strlen(haystack);

			Strtohex(&Numbre,haystack,StrLen);if(Numbre == 0x10)Numbre = 10;
			if((Numbre < 1)||(Numbre > 10)) {response(source,18,SetFail);return SetERR;}
				if(Numbre > 8)SectionAddr = MessgeAddr[Numbre-9];// 温度湿度，停电
	else SectionAddr = MessgeAddr[Numbre+1];//MessgeAddr[Numbre-1];
//			SectionAddr = MessgeAddr[Numbre+1];//MessgeAddr[Numbre-1];
			char *PortBuf = portMalloc(50*sizeof(uint8_t));			

//		if((StrLen == 1)||(StrLen == 2))		// 紧凑编码
//			 {
//				strncpy(&Numbre,haystack,StrLen);Numbre = atoi(&Numbre);
//				if(((Numbre > 0))&&(Numbre < 11))SectionAddr = MessgeAddr[Numbre-1];//atoi(&Numbre)
//				else return SetERR;
//			 }
//		else if((StrLen == 4)||(StrLen == 8))			//松散编码 "64E69664002300380023" 擦除#8#
//			 {
//				strncpy(&Numbre,&haystack[3],StrLen);Numbre = atoi(&Numbre);
//				if(((Numbre > 0))&&(Numbre < 11))SectionAddr = MessgeAddr[Numbre-1];//atoi(&Numbre)
//				else return SetERR;
//			 }		
//		else								// 
//			{
//			 return SetERR; 
//			}
			EEPROM_SectorErase(SectionAddr);	
			if(Numbre == 10)response(source,16+2,"64E696646210529F");// 擦除成功
			else
			{
//			snprintf(PortBuf,50,"7B2C003%1d7AEF53E3002C64E696646210529F",Numbre);//第n端口,擦除成功
//			response(source,36+2,PortBuf);

			if(Numbre == 9)strcpy(PortBuf,str_set_TemHum);//"6E296E7F5EA6"温湿度
			else if(Numbre == 10)strcpy(PortBuf,str_set_PowOffOn);//"505C67657535"停来电
			else sprintf(PortBuf,"7B2C003%1d7AEF53E3",Numbre);//第n端口
			strcat(PortBuf,"002C64E696646210529F");//,擦除成功
			response(source,52+2,PortBuf);

			}
			portFree(PortBuf);			
		}		
	}	
	Mqtt_sms_call_phone(Numbre,0);// PortNum 端口号 1~9  sms_call 0 短信号码 1 电话号码
	Mqtt_sms_call_phone(Numbre,1);
	Mqtt_PortMessage(Numbre, 0);// PortNum 端口号 1~9  close_open 0 闭合短信 1 断开短信
	Mqtt_PortMessage(Numbre, 1);	
return SteOK;		
} 
//查询其中一个回路 闭合短信+断开短信
//
//信息#8#  查询第8回路的全部短信信息 
SetResultType QueryPortInfo(char *source)
{
	char *haystack, *buf,*Model_StrNeed;
u32 SectionAddr;//地址
u16 StrLen,MessLen;	

	if(!hand(source, QueryComStr))return SetNull;//没有数据 
	 Model_StrNeed = cut_str;//"0023"  "#"
	 haystack= source;
	 StrLen = strlen(Model_StrNeed);
/************************获取回路序号 字符/ ****************************/
	 buf= strstr( haystack, Model_StrNeed);
	 haystack = buf + StrLen;
	 buf = strstr( haystack, Model_StrNeed);	
	if( buf != NULL )
		{
				buf[0]='\0';
		u8 Numbre;
		StrLen = strlen(haystack);

		Strtohex(&Numbre,haystack,StrLen);if(Numbre == 0x10)Numbre = 10;
		if((Numbre < 1)||(Numbre > 10)) {response(source,18,SetFail);return SetERR;}
		if(Numbre > 8)SectionAddr = MessgeAddr[Numbre-9];// 温度湿度，停电
	else SectionAddr = MessgeAddr[Numbre+1];//MessgeAddr[Numbre-1];
//		SectionAddr = MessgeAddr[Numbre+1];// MessgeAddr[Numbre-1];				
//		if((StrLen == 1)||(StrLen == 2))		// 紧凑编码
//			 {
//				strncpy(&Numbre,haystack,StrLen);Numbre = atoi(&Numbre);
//				if(((Numbre > 0))&&(Numbre < 10))SectionAddr = MessgeAddr[Numbre-1];
//				 else return SetERR;
//			 }
//		else if((StrLen == 4)||(StrLen == 8))			//松散编码 "64E69664002300380023" 擦除#8#
//			 {
//				strncpy(&Numbre,&haystack[3],StrLen);Numbre = atoi(&Numbre);
//				if(((Numbre > 0))&&(Numbre < 10))SectionAddr = MessgeAddr[Numbre-1];
//				 else return SetERR;
//			 }		
//		else								// 
//			{
//			 return SetERR; 
//			}
/************************END****************************/			
		/************************获取闭合短信****************************/
	memset(MsegTemp,0,Mseg_Lenth);
	strcpy(MsegTemp,str_QueryPortInfo_NCmsg);	//"95ED540877ED4FE1FF1A"闭合短信：
			// 
//strcat(MsegTemp,"4FE153F7FF1A");	//信号(强度5F3A5EA6)：99,99		   10			
	StrLen = EEPROM_read_Byte(SectionAddr+Close_Message_offset);	
		if((2 < StrLen) && (StrLen < 0xff))//消息地址首位，长度判断是否有信息
		{
			EEPROM_read_n((SectionAddr+Close_Message_offset+1),MsegTemp+20,StrLen);	//读取信息 偏移10个字节（闭合短信：）
		}	
		/************************获取断开短信****************************/	
	StrLen = EEPROM_read_Byte(SectionAddr+Open_Message_offset);	
		if((2 < StrLen) && (StrLen < 0xff))//消息地址首位，长度判断是否有信息
		{
			strcat(MsegTemp,str_QueryPortInfo_NOmsg);//"000A65AD5F0077ED4FE1FF1A"换行  断开短信：		
			MessLen = strlen(MsegTemp);
			EEPROM_read_n((SectionAddr+Open_Message_offset+1),MsegTemp+MessLen,StrLen);	//读取信息 偏移MessLen个字节
		}
	MessLen = strlen(MsegTemp);	
	response(source,MessLen+2,MsegTemp);//
	return SteOK;		
	}
return SetERR ;	
}

//参数：needle--> 比较的信息
//参数：Group --> 组序号
//信息格式：*1#******#	 备用功能 测试通过
//信息格式：%1#******#
//Topic: /download/close_massage 
//Message: port=8&mesag=Unicode
SetResultType MessageSet(char *soucer)
{
//    u8 NumLen = 0,StrLeng = 0,len = 0;

	char *haystack;//=Content;
	char* buf;
	char *Model_StrNeed,*close_open;
//SetResultType result;
u32 SectionAddr,OffsetAdd;//地址	
u8 StrLen;
u8 	Numbre;

 	 haystack= soucer;//RX1_Buffer;
//char ttt[] = "/open_massage/864708068765984D,127 byte,port=1&mesag=\\u6df1\\u5733\\u5e02\\uff0c\\u9999\\u533a&";	
//soucer=haystack=ttt;DeivceFlag.SIMorMQTT = 1;
	if(DeivceFlag.SIMorMQTT)//mqtt数据   0 sim 1 mqtt
	{
	if(hand(soucer, "close_massage")){ OffsetAdd = Close_Message_offset;}
	else if(hand(soucer, "open_massage")){ OffsetAdd = Open_Message_offset;}
//	else if(hand(soucer, PowerOFF_COM)){ OffsetAdd = Open_Message_offset;}
//	else if(hand(soucer, PowerON_COM)){ OffsetAdd = Close_Message_offset;}
	else return SetNull;//没有数据 

	buf= strstr( haystack, "port=");//分割字符串
	haystack = buf + 5;
	buf = strstr( haystack, "&");
	Model_StrNeed = "\0";	   //;"&"查找最未尾结束字符
	}
	else			//sim数据
	{
	if(hand(soucer, close1)){close_open = close1; OffsetAdd = Close_Message_offset;}
	else if(hand(soucer, open1)){close_open =open1; OffsetAdd = Open_Message_offset;}
//	else if(hand(soucer, PowerOFF_COM)){close_open =PowerOFF_COM; OffsetAdd = Open_Message_offset;}
//	else if(hand(soucer, PowerON_COM)){close_open =PowerON_COM; OffsetAdd = Close_Message_offset;}
	else return SetNull;//没有数据 

	StrLen = strlen(close_open);
	Model_StrNeed = cut_str;	   //分割字符串 "0023"  "#"	
	buf= strstr( haystack, close_open);//分割字符串 *  %
	haystack = buf + StrLen;
	buf = strstr( haystack, Model_StrNeed);
	}
	
/************************获取回路序号 字符/ ****************************/
if( buf != NULL )
	{
    	buf[0]='\0';
	StrLen = strlen(haystack);	
		
	Strtohex(&Numbre,haystack,StrLen);if(Numbre == 0x10)Numbre = 10;
	if((Numbre < 1)||(Numbre > 10)) {response(soucer,18,SetFail);return SetERR;}
		if(Numbre > 8)SectionAddr = MessgeAddr[Numbre-9];// 温度湿度，停电
	else SectionAddr = MessgeAddr[Numbre+1];//MessgeAddr[Numbre-1];
//	SectionAddr = MessgeAddr[Numbre+1];//MessgeAddr[Numbre-1];
	
//	if((StrLen == 1)||(StrLen == 2))		// 紧凑编码//@#8#1369219***8# 
//	   {
//			strncpy(&Numbre,haystack,StrLen);
//			SectionAddr = MessgeAddr[Numbre-1];
//	   }
//	else if((StrLen == 4)||(StrLen == 8))			//松散编码//@#0038#1369219***8# 
//	   {
//			strncpy(&Numbre,&haystack[3],StrLen);
//			SectionAddr = MessgeAddr[Numbre-1];
//		 }		
//	else	return SetERR;
	}
//	Numbre = atoi(&Numbre);
//	if(((Numbre < 1))||(Numbre > 10)) return SetERR;
/************************END****************************/	
/**********************查找到正确命令 开始存储短信**************************/				
	
	haystack = buf + strlen(cut_str);
	buf = strstr( haystack, Model_StrNeed);//查找最未尾结束字符 #
	if(DeivceFlag.SIMorMQTT == 0)buf[0]='\0'; //临时测试用 正式需要加结尾符&
	
 u8 MesLen = strlen(haystack);
char *EEprBuf = portMalloc(1024*sizeof(uint8_t));	
char *EEprt = EEprBuf + 1;	
	 if((MesLen > 4) && (MesLen < 130))
	 {
//		*EEprt++ = MesLen;
//		wprintf();	
		if(DeivceFlag.SIMorMQTT)//mqtt数据   0 sim 1 mqtt	
		{
			if((NULL == strstr( haystack, "\\u"))||(MesLen < 6))goto MessageSet_out;//没有数据;
			u8 y = 0;
			while(MesLen)
			{
				if(MesLen > 6)MesLen -= 6; else MesLen = 0;
				buf= strstr( haystack, "\\u");//分割字符串
				if(buf != NULL)
				{
				haystack = buf + 2;	
					for(u8 i = 0; i <4; y++,i++,EEprt++,haystack++)
					*EEprt = toupper(*haystack);
				}
				else MesLen = 0;//
			}
			MesLen = y; *EEprt = '\0';
//			*EEprBuf = MesLen;
		}
		else strncpy(EEprt,haystack,MesLen);
		*EEprBuf = MesLen;
		
		SectorErase_protect(SectionAddr, (u8*)EEprBuf, OffsetAdd, MesLen+1,0);
if(DeivceFlag.SIMorMQTT)goto MessageSet_out;//if mqtt数据break		 
//		if(Numbre == 10)response(soucer,16+2,Sccuces);// 停电来电
//		else
//		{
//		 if(OffsetAdd == Close_Message_offset)snprintf(EEprBuf,100,"7B2C003%d7AEF53E395ED540877ED4FE1002C8BBE7F6E6210529F",Numbre);//第n端口闭合短信,设置成功
//		else snprintf(EEprBuf,100,"7B2C003%d7AEF53E365AD5F0077ED4FE1002C8BBE7F6E6210529F",Numbre);//第n端口断开短信,设置成功
//		response(soucer,52+2,EEprBuf);	
//		}
		
		if(Numbre == 9)strcpy(EEprBuf,str_set_TemHum);//"6E296E7F5EA6"温湿度
		else if(Numbre == 10)strcpy(EEprBuf,str_set_PowOffOn);//"505C67657535"停来电
		else sprintf(EEprBuf,"7B2C003%1d7AEF53E3",Numbre);//第n端口
		
		if(OffsetAdd == Close_Message_offset)
			strcat(EEprBuf,str_set_NC_msg);//"95ED5408544A8B6677ED4FE1002C8BBE7F6E6210529F"闭合告警短信,设置成功
		else 
			strcat(EEprBuf,str_set_NO_msg);//"65AD5F0089E3966477ED4FE1002C8BBE7F6E6210529F"断开解除短信,设置成功
		response(soucer,52+2,EEprBuf);
		
	 }
	 else {response(soucer,18,SetFail);}
	 
MessageSet_out:	
	 
//memset(SIMQueue.dataBuf,0,MAX_QUEUE_LEN);					  //清除内容				  //清除内容
portFree(EEprBuf);
return SteOK;	 
}

void Set_433Model(char*soucer,u8 Dat)
{
//	EEPROM_SectorErase(W433ModelAddr);				   //擦除对应的扇区
//	EEPROM_write_Byte(W433ModelAddr,Dat);					//数据长度 存入
	SectorErase_protect(CSCA_W433_Address, &Dat,W433_Flag_offset,1,0);//0x0801F000 +偏移0x80		
	response(soucer,18,Sccuces);
//#ifdef 	message_model  //TXT
////sim900a_send_cmd("AT+CMGF=1\r\n","OK",100);
//EC20_SendModuleCmd(AT_TXT, NULL );	
//#endif
	Wait_For_Nms(1000) ;
	Ec20AtBufReset();		  	  //清除内容
}

RunResult sim900a_send_cmd(char *cmd,char *ack,u16 waittime)
{
	RunResult Result = TIMEOUT;
	u8 CmdLen = strlen(cmd);
	//	delay_ms(2600);//延时
	Ec20AtBufReset() ;				  //清除内容
	UARTx_SendData(EC20_UART, cmd,  CmdLen);	 //DMA发送
	while(waittime--)
		{
		Delay_Ms_StopScheduler(10);//Delay_Ms((u32)(100)) ;//
		if(hand(ec20AtBuf,ack))
			{Result = RUNOK;	goto CMDOUT;}//
		IWDG_ReloadKey();
		}//if(T0_1MS == 0)Resul=0;
	Result = RUNERR;	
CMDOUT:		
	return Result;
}

//复位_设置模块里的信息		 
//void Reset(void)
//{	
////	if(!message_model)sim900a_send_cmd("AT+CMGF=1\r\n","OK",100);  //Uart1Sends("AT+CMGF=1\r\n");在PDU模式下 需要转换成 TEXT模式
//#ifdef 	message_model  //TXT
//EC20_SendModuleCmd(AT_TXT, NULL );	
//#endif	
//	Wait_For_Nms(1000) ;//DelaySec(2);
//	Ec20AtBufReset() ;	//memset(RX1_Buffer,0,COM_RX1_Lenth);				  //清除内容
//}

//清除模块里的全部信息
SetResultType CleanAll(char*soucer)
{
	Tled=0;	
	if(!hand(soucer, Reset_Com))return SetNull;//没有数据
	
	response(soucer,22,DatClear);
//Uart1Sends("111\r\n");
//	if(!message_model)sim900a_send_cmd("AT+CMGF=1\r\n","OK",100);  //Uart1Sends("AT+CMGF=1\r\n");在PDU模式下 需要转换成 TEXT模式
	Wait_For_Nms(1000) ;//DelaySec(3);
//	MissSendOK = 1;
	
	EEPROM_SectorErase(MessgeAddr[0]);	  //擦除1扇区	---　温湿度报警
	EEPROM_SectorErase(MessgeAddr[1]);	  //擦除2扇区	---　停电报警消息;来电通知
	EEPROM_SectorErase(MessgeAddr[2]);	  //擦除3扇区	---　短信内容1
	EEPROM_SectorErase(MessgeAddr[3]);	  //擦除4扇区	---　短信内容2
	EEPROM_SectorErase(MessgeAddr[4]);	  //擦除5扇区	---　短信内容3
	EEPROM_SectorErase(MessgeAddr[5]);	  //擦除6扇区	---　短信内容4
	EEPROM_SectorErase(MessgeAddr[6]);	  //擦除7扇区	---　短信内容5
	EEPROM_SectorErase(MessgeAddr[7]);	  //擦除8扇区	---　短信内容6			   
	
	EEPROM_SectorErase(MessgeAddr[8]);	  //擦除9扇区	---　短信内容7
	EEPROM_SectorErase(MessgeAddr[9]);	  //擦除10扇区---　短信内容8
	EEPROM_SectorErase(CSCA_W433_Address);						//擦除短信中心号码
	
	EEPROM_SectorErase(History_Alarm_Addres[0]);//0x0801F800历史报警记录空间(0x40)64byte*10*  
 	Ec20AtBufReset() ;
return SteOK;	
}
//
ErrorStatus GetPhoneNumber(u32 AlarmPort_Addr,u16 *SMS_or_Call)
{
//	u8 NumLen;
		for(;PhoneAddrIndex < 6;PhoneAddrIndex++)
		{
		PhoneNumLen = EEPROM_read_Byte(AlarmPort_Addr + SMS_or_Call[PhoneAddrIndex]);
			
		if((PhoneNumLen > 2)&&(PhoneNumLen < 15))goto GetPhoneOK;//地址首位，长度标识位	
		}
GetPhoneOK:		
		if(PhoneAddrIndex >= 6)
		{
			PhoneAddrIndex = 0;

			return ERROR ;
		}	
			memset(PhoneNumber,0,PhoneLenght);
			EEPROM_read_n((AlarmPort_Addr + SMS_or_Call[PhoneAddrIndex]+1), PhoneNumber, PhoneNumLen);   //号码长度
		
return SUCCESS;				
}
//24-11-7更新 每次从数组里获取号码
ErrorStatus AllGetPhoneNumber(u8 Mes_or_Phone)
{
	if(Array_cousor >= Rs485ArrayLen)Array_cousor = 0;
	PhoneNumLen = Msg_and_call_Phone[Array_cousor] &0x0f;	//tmelen.DouBty.Bety_L;
	
	if(((PhoneNumLen < 3)||(PhoneNumLen > 13))||
		(((Msg_and_call_Phone[Array_cousor] & 0xf0) != MsgNumberFlag)&&((Msg_and_call_Phone[Array_cousor] & 0xf0) != PhoneNumberFlag)))
	{
		Array_cousor = 0;
		return ERROR ;
	}
	if((Msg_and_call_Phone[Array_cousor] & 0xf0) != Mes_or_Phone)return ERROR ;	
	
			memset(PhoneNumber,0,PhoneLenght);
			memcpy(PhoneNumber,&Msg_and_call_Phone[Array_cousor + 1],PhoneNumLen);
			Array_cousor += PhoneNumLen+1;//包括前缀标识位 +1
return SUCCESS;				
}
//获取报警信息内容后进行发短信 打电话
//Close_Message_offset Open_Message_offset
ErrorStatus GetAlarmMassage(u32 AlarmPort_Addr,u16 Close_or_Open)
{
	u8 MessLen1;
//	MessLen = EEPROM_read_Byte(AlarmPort_Addr + Close_or_Open);	
	MessLen1 = *(vu8*)(AlarmPort_Addr + Close_or_Open);
	if((2 < MessLen1) && (MessLen1 < 0xff))//消息地址首位，长度判断是否有信息
	{
		memset(MsegTemp,0,Mseg_Lenth);
		EEPROM_read_n((AlarmPort_Addr + Close_or_Open+1),MsegTemp,MessLen1);	//读取信息
		if(Receive_Alarm.Num == TemperHum_AddrSection)//如果是温湿度报警 添加温度湿度数值
		{
			char *temp,*hump;
			char *Dis_unicode = (char*)portMalloc(10*sizeof(u8) ) ;
//			if(AlarmTemHum.TemH_flag)			temp="6E295EA68FC79AD8FF1A";//温度过高：
//			else if(AlarmTemHum.TemL_flag)temp="6E295EA68FC74F4EFF1A";//温度过低：
//			else 													temp="6E295EA66B635E38FF1A";//温度正常：
//			
//			if(AlarmTemHum.HumH_flag)			hump="6E7F5EA68FC79AD8FF1A";//湿度过高：
//			else if(AlarmTemHum.HumL_flag)hump="6E7F5EA68FC74F4EFF1A";//湿度过低：
//			else 													hump="6E7F5EA66B635E38FF1A";//湿度正常：
			if(SENx.T > AlarmValue.ValueUnit.Temper_H)			temp="6E295EA68FC79AD8FF1A";//温度过高：SENx.T > AlarmValue.ValueUnit.Temper_H
			else if(SENx.T < AlarmValue.ValueUnit.Temper_L)temp="6E295EA68FC74F4EFF1A";//温度过低：
			else 													temp="6E295EA66B635E38FF1A";//温度正常：
			
			if(SENx.RH > AlarmValue.ValueUnit.Humidi_H)			hump="6E7F5EA68FC79AD8FF1A";//湿度过高：
			else if(SENx.RH < AlarmValue.ValueUnit.Humidi_L)hump="6E7F5EA68FC74F4EFF1A";//湿度过低：
			else 													hump="6E7F5EA66B635E38FF1A";//湿度正常：
			
		strncpy(Dis_unicode,(const char*)sEc20Param.Temper,5);
		cdma_mobile_txt_shift(Dis_unicode,5);	//字符串转换成UNICODE
		strcat(MsegTemp,temp);	//温度：	
		strcat(MsegTemp,Dis_unicode);	//显示湿度数据
		strcat(MsegTemp,"00200020");	//空格 空格
		strncpy(Dis_unicode,(const char*)sEc20Param.Humidi,5);		
		cdma_mobile_txt_shift(Dis_unicode,5);	//字符串转换成UNICODE
		strcat(MsegTemp,hump);	//湿度：	
		strcat(MsegTemp,Dis_unicode);	//
//		strcat(MsegTemp,"000A7AEF53E3FF1A0031");	//换行/n端口：1	
		portFree (Dis_unicode);
		}
//		MsegTemp[MessLen] = 0;
//		xEventGroupSetBits(AlarmStateHandler, SMS_ReadyBIT_1);  //使能短信就绪位，发送短信
		PhoneAddrIndex = 0;	return SUCCESS;		
	}
//	else if(Receive_Alarm.Num == 8)//停电  恢复供电
//	{
//	if(Send_Alarm.Adrr_Offset == Close_Message_offset)strncpy (MsegTemp,P_ON,35);
//	else strncpy (MsegTemp,P_OFF,35);	 
//	}
	else return ERROR ;
//return SUCCESS;				
}

#ifdef message_model			//TXT
void  EventFunc(char *Mess_Addr) //u8
{
Tled=0;//
//u8 MessageLen =  EEPROM_read_Byte(MessgeAddr[Alarm_PortNum] + Message_offset);
ErrorStatus  result;
	
	//运营商 1 电信  //0 移动
		if(loose_model) 		//+CMT: 0031   松散编码
		{
			result = mobile_txt_message(PhoneNumber,Mess_Addr);
		}
		else				   //+CMT: 1	   紧凑编码
		{
			result = cdma_txt_message(PhoneNumber,Mess_Addr);
		}
		PhoneAddrIndex++;																						//开始向下一个号码发送短信
		xEventGroupClearBits(AlarmStateHandler,SMS_ReSendBIT_2|SMS_FailBIT_3);//清除标识		
result = result;
/**2024/9/2 关闭短信重发机制 只发一次！**/		
//判断短信是否成功发送		
//	if(SUCCESS == result)
//		{
//		PhoneAddrIndex++;																						//开始向下一个号码发送短信
//		xEventGroupClearBits(AlarmStateHandler,SMS_ReSendBIT_2|SMS_FailBIT_3);//清除标识
//		}
//	else 
//		{
//		if((SMS_ReSendBIT_2 & xEventGroupGetBits(AlarmStateHandler)) == SMS_ReSendBIT_2)
//			{
//			xEventGroupClearBits(AlarmStateHandler,SMS_ReSendBIT_2|SMS_FailBIT_3);//清除标识
//			PhoneAddrIndex++;//开始向下一个号码发送短信
//			}
//		else xEventGroupSetBits(AlarmStateHandler, SMS_FailBIT_3);//置位3短信发送失败
/**END**/			
//		}	

		
//////判断短信是否成功发送		
////	if(SUCCESS == result)PhoneAddrIndex++;//开始向下一个号码发送短信SMS_FailBIT_3 SMS_ReSendBIT_2
////	else 
////		{
////		if((SMS_ReSendBIT_2 & xEventGroupGetBits(AlarmStateHandler)) == SMS_ReSendBIT_2)
////			{
////			xEventGroupClearBits(AlarmStateHandler,SMS_ReSendBIT_2|SMS_FailBIT_3);//清除标识
////			PhoneAddrIndex++;//开始向下一个号码发送短信
////			}
////		else xEventGroupSetBits(AlarmStateHandler, SMS_FailBIT_3);//置位3短信发送失败
////		}	
}
#else											//PDU
void  EventFunc(char *Mess_Addr) //u8
{
	Tled=0;//
u8 MessageLen =  EEPROM_read_Byte(MessgeAddr[Alarm_PortNum] + Message_offset);
	if(ModelFlag)			//运营商 1 电信
	{
	//PDU
		cdma_pdu_message( MessageLen, Mess_Addr);
	}
	else			   	 //0 移动
	{
	//PDU
		{
		mobile_pdu_Message( MessageLen, Mess_Addr);
		}
	}
}
#endif
//void StratFunSelect(void)
//{
//if(SimRest)//进入工作模式
//	{
//		vTaskSuspend(SetModesTaskHandler);//挂起设置参数任务
//	}
//	else
//	{
//		vTaskSuspend(AlarmTaskHandler);//挂起报警任务SetModesTask
//		vTaskSuspend(LedTaskHandler);//	挂起Led任务
//		vTaskResume(SetModesTaskHandler);//	恢复设置参数任务
//	}
//}
/*
查询历史报警记录 用户设置的信息 3秒后返回工作主页
*/
//void ScanQuery(void)
//{
//	static u8 QueryDelay;
//	u8 pageTemp;
//		if(!UserQuery_KEY)	   //(!UserKey.CloseState)&&用户按健按下闭合有效 下降沿
//		{ 
//			if(++UserKey.QueryCnt > QueryDelay)
//			{
//				QueryDelay = 5;		//长按延时5有效
//				if(UserKey.CloseState)QueryDelay = 1;//后执行翻页速度
//			UserKey.CloseState = 1;UserKey.LastValuae=1;
//				UserKey.QueryCnt = 0;
//				
//			if(UserKey.CurrentPage++ > 54)UserKey.CurrentPage = 1;//最多54 5*10+3=53+1页循环;插入了一页显示温度湿度报警阀值
//				else vTaskSuspend(LedTaskHandler);//挂起Led任务
//			switch(UserKey.CurrentPage)
//				{
//					case 1:case 2:case 3:case 4:	//显示历史报警记录页面
//								if(ERROR == HistoryAlarmPage1())UserKey.CurrentPage = 4;//
//								break;
//					default:	//显示端口信息页面
//								
//								if(UserKey.CurrentPage>50)pageTemp = UserKey.CurrentPage-1;
//								else pageTemp = UserKey.CurrentPage;
//								if(UserKey.CurrentPage==50)TemperHum_Disp();
//								else UserInforPage1((pageTemp-4) % 5,(pageTemp - 5) / 5);
//								break;	
//				}	
//			}
//		}	
//		if((UserKey.LastValuae)&&(UserQuery_KEY))	//else 按键松开有效 上升沿 超过延时返回首页
//		{ UserKey.CloseState = 0;QueryDelay = 0;
//			if(++UserKey.QueryCnt > SingeDelay*10)					//查询 30次
//				{
//				UserKey.CurrentPage = UserKey.LastValuae = UserKey.QueryCnt = 0;	
//				if(!DeivceFlag.RestFlag)SetModePage(); //设置模式
//				else	HomeMenu();//工作模式 显示首页
//				}
//		}
////		else {UserKey.QueryCnt = 0;QueryDelay = 0;}//UserKey.LastValuae=
//}	

void ScanQuery(void)
{
//	static u8 QueryDelay;
	u8 pageTemp;
		if(UserKey.KeyExeFlag == 1)	   //(!UserKey.CloseState)&&用户按健按下闭合有效 下降沿
		{ 
				UserKey.KeyExeFlag = 0;
			if(UserKey.CurrentPage++ > 54)UserKey.CurrentPage = 1;//最多54 5*10+3=53+1页循环;插入了一页显示温度湿度报警阀值
				else vTaskSuspend(LedTaskHandler);//挂起Led任务
			switch(UserKey.CurrentPage)
				{
					case 1:case 2:case 3:case 4:	//显示历史报警记录页面
								if(ERROR == HistoryAlarmPage1())UserKey.CurrentPage = 4;//
								break;
					default:	//显示端口信息页面
						
								if(UserKey.CurrentPage > 5)pageTemp = UserKey.CurrentPage-6;
								else pageTemp = UserKey.CurrentPage;
					
								if(UserKey.CurrentPage == 5)TemperHum_Disp();//50温度 湿度显示报警数值
								else UserInforPage1((pageTemp+1) % 5,pageTemp / 5);	
					
								break;	
				}	
		
		}	
		else if(UserKey.KeyExeFlag == 2)	//按键松开有效 上升沿 超过延时返回首页
		{
			UserKey.KeyExeFlag = 0;

				UserKey.CurrentPage = 0;	//UserKey.LastValuae = UserKey.QueryCnt =
				if(!DeivceFlag.RestFlag)SetModePage(); //设置模式
				else	HomeMenu();//工作模式 显示首页

		}
//		else {UserKey.QueryCnt = 0;QueryDelay = 0;}//UserKey.LastValuae=
}				
//输入信号扫描
void PwoerCheck(void)
{
	static u8 Flag_433M_write = 0;
	Flag_433M_write = Flag_433M_write;
//停电 发送一次停电短信
	 if(PowerOff)
	 	{
	 	Wait_For_Nms(100) ;//Delay10m(20);//
		 if(PowerOff)	 
			{
				if(!DeivceFlag.Power_Flag)
					{
					Tled=0;DeivceFlag.Power_Flag = 1;
					Send_Alarm.Num = PowerOFF_AddrSection;Send_Alarm.Adrr_Offset = Close_Message_offset;
					BaseType_t err = xQueueSend(AlarmPort_Queue,&Send_Alarm,0);
//					OLED_DrawBMP(120, 0, 8, 16, false, power_off);
					}
			}
		}
//来电 发送一次来电短信
	else if(!PowerOff)
	 	{
	 	Wait_For_Nms(100) ;//Delay10m(20);
		 if(!PowerOff)	 
			{
				if(DeivceFlag.Power_Flag)
					{
					Tled=0;DeivceFlag.Power_Flag = 0;
					Send_Alarm.Num = PowerOFF_AddrSection;Send_Alarm.Adrr_Offset = Open_Message_offset;
					BaseType_t err = xQueueSend(AlarmPort_Queue,&Send_Alarm,0);
					OLED_DrawBMP(120, 0, 8, 16, false, (const unsigned char*)ASCII_HIGH16);	//清除停电图票
					}
			}
		}
/**************/
//#if W433_Enabel == 1
//if((DeivceFlag.GSMRet_Flag)&&(Check_Code_Time)&&(W433Hard.Head.WirteFlag==0x1a))		//写入W433识别码 地址码
//	{
//		W433Hard.Head.WirteFlag = 0;
//Beep = 4;
////		W433Hard.Head.Temp_Data1 = 0xAA;
////		W433Hard.Head.Temp_Data2 = 0x55;
////		W433Hard.Head.Temp_Data3 = 0xCC;
//		W433Hard.Head.Eeprom_Addr1 = RF_Addr1;
//		W433Hard.Head.Eeprom_Addr2 = RF_Addr2;
////		SectorErase_protect(CSCA_W433_Address, W433Hard.W433Dat,W433_Ide_offset,5+1,0);//0x0801A800 +偏移0x40  存储 3个识别码 2个地址码  4个键值码 ABCD //18 d3 --17 1b 1d 1e		
//		SectorErase_protect(CSCA_W433_Address, W433Hard.W433Dat,W433_Ide_offset,5,0);//0x0801A800 +偏移0x40  存储 3个识别码 2个地址码  4个键值码 ABCD //18 d3 --17 1b 1d 1e		
//	}	
/****************/		
#if W433_Enabel == 1
if((DeivceFlag.GSMRet_Flag)&&(Check_Code_Time)&&(W433Hard.Head.WirteFlag==0x1a))		//写入W433识别码 地址码
	{
		W433Hard.Head.WirteFlag = 0;
//Beep = 4;
Flag_433M_write = 1;
Save_433M_Maxlen = Save_433M_Value[0].Device_Index = 4;
		if((Save_433M_Value[0].ID_Addr1 == 0)||((Save_433M_Value[0].ID_Addr1 == 0xff)))Save_433M_Value[0].ID_Addr1 = RF_Addr1;
		if((Save_433M_Value[0].ID_Addr2 == 0)||((Save_433M_Value[0].ID_Addr2 == 0xff)))Save_433M_Value[0].ID_Addr2 = RF_Addr2;
		
		if((Save_433M_Value[0].Key_A == 0)||((Save_433M_Value[0].Key_A == 0xff))){Save_433M_Value[0].Key_A = RF_Addr3;W433SimuPort.Portbit.Port1 = 1;Beep = 2;}//
		else if((Save_433M_Value[0].Key_B == 0)||((Save_433M_Value[0].Key_B == 0xff))){Save_433M_Value[0].Key_B = RF_Addr3;W433SimuPort.Portbit.Port2 = 1;Beep = 4;}//
		else if((Save_433M_Value[0].Key_C == 0)||((Save_433M_Value[0].Key_C == 0xff))){Save_433M_Value[0].Key_C = RF_Addr3;W433SimuPort.Portbit.Port3 = 1;Beep = 6;}//
		else if((Save_433M_Value[0].Key_D == 0)||((Save_433M_Value[0].Key_D == 0xff))){Save_433M_Value[0].Key_D = RF_Addr3;W433SimuPort.Portbit.Port4 = 1;Beep = 8;}//
//		else Flag_433M_write = 0;

OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,   "A:%x ",RF_Addr1) ;
OledPrintf(LINE_MID, HIGH_16, LINE3, false,   "A:%x ",RF_Addr2) ;
OledPrintf(LINE_RIGHT, HIGH_16, LINE3, false,   "D:%x ",RF_Addr3) ;		
	}	
/***************************************************/	
#elif W433_Enabel == 2	
if((DeivceFlag.GSMRet_Flag)&&(Check_Code_Time)&&(W433Hard.Head.WirteFlag==0x1a))		//写入W433识别码 地址码
	{
u8 i;
W433Hard.Head.WirteFlag = 0;
		for(i=0; i < Size_433M && (Save_433M_Value[i].Device_Index > 0 && Save_433M_Value[i].Device_Index < 0xff); i++)
		{
		if((Save_433M_Value[i].ID_Addr1 == RF_Addr1)&&(Save_433M_Value[i].ID_Addr2 == RF_Addr2))goto M433_Addr;
			
		}
		if(i >= Size_433M)goto M433_EEpro;
		Save_433M_Maxlen = i + 1;
		Save_433M_Value[i].Device_Index = 0x68;//		
		Save_433M_Value[0].Device_Index = Save_433M_Maxlen;
		Save_433M_Value[i].ID_Addr1 = RF_Addr1;
		Save_433M_Value[i].ID_Addr2 = RF_Addr2;
		Save_433M_Value[i].Key_A = RF_Addr3;
		Beep = 4;//(i + 1)*2;
	Flag_433M_write = 1;
				goto M433_EEpro;
M433_Addr:
		if(Save_433M_Value[i].Key_A != RF_Addr3)
		{Save_433M_Value[i].Key_B = RF_Addr3;Beep = 4;Flag_433M_write = 1;}
M433_EEpro:	
		Beep = Beep;
		
//*TraggetTrcnt[i] = SingeDelay;
if(RF_Addr3 == Save_433M_Value[i].Key_A)W433SimuPort.PortFlag |= (1<<i);
else if(RF_Addr3 == Save_433M_Value[i].Key_B)W433SimuPort.PortFlag &= ~(1<<i);

OledPrintf(LINE_LEFT, HIGH_16, LINE3, false,   "A:%x ",RF_Addr1) ;
OledPrintf(LINE_MID, HIGH_16, LINE3, false,   "A:%x ",RF_Addr2) ;
OledPrintf(LINE_RIGHT, HIGH_16, LINE3, false,   "D:%x ",RF_Addr3) ;
	}
/**********************************************************/	
#endif	
//复位 下降沿复位
	if((DeivceFlag.RestFlag)&&(!SimRest))//进入设置模式
	{
	Wait_For_Nms(100) ;//DelaySec(1);
		if(!SimRest)
		{
			DeivceFlag.Refresh_Sever = 1;//
		DeivceFlag.GSMRet_Flag = 1; DeivceFlag.RunState = 1;//运行状态 RunState   0 运行 1设置
		MQTT_DeviceState(1);
			vTaskResume(SetModesTaskHandler);//恢复设置参数任务
			if(RegisterEEpro_Slave.Host_or_Slave != Slave_Flag)	//六通道RS485从机模式不执行下二句
			{				
			vTaskSuspend(AlarmTaskHandler);//挂起报警任务SetModesTask
			vTaskSuspend(CallResultTaskHandler);//挂起模块处理回复消息任务
			vTaskSuspend(WIFI_onlinDetTaskHandler);	//挂起WIFI
			}
			vTaskSuspend(LedTaskHandler);//	挂起Led任务
			
			xEventGroupClearBits(AlarmStateHandler,(EventBits_t)0);	//清零标志组			
//			xEventGroupClearBits(AlarmStateHandler,	AlarmWaitBIT_0);		//关闭获取报警事件
//			xEventGroupClearBits(AlarmStateHandler, AlarmWaitBIT_0 | SMS_ReadyBIT_1 | Call_ReadyBIT_5);		//关闭获取报警事件 ，短信 电话 接收任务
////			prvLockQueue(AlarmPort_Queue);	//AlarmPort_Queue队列上锁
//			EC20_SendModuleCmd(AT_TXT, NULL);
			Ec20AtBufReset() ;
			SetModePage();				
	/************************433配对控制信号*****************************/			
			Check_Code_Time = 300;//30复位3秒内配对有效
	/************************END*****************************/				
		DeivceFlag.RestFlag = 0;	
	Wifi_init();			
		}
	}
	else if((DeivceFlag.GSMRet_Flag)&&(SimRest))//进入工作模式
	{
	Wait_For_Nms(100) ;//DelaySec(1);
		if(SimRest)
	{
//		WIFI_SendModuleCmd(AT_WCONFIG,NULL,0);
#ifdef W433_Enabel	
if(Flag_433M_write)			
		{
		Flag_433M_write = 0;
		SectorErase_protect(CSCA_W433_Address,&Save_433M_Value[0].Device_Index,M433_AddrOffset,56,0);//存储数据长度56
		}		
		EEPROM_read_n((CSCA_W433_Address+M433_AddrOffset),(char*)&Save_433M_Value[0].Device_Index, 56);//读取数据长度56
		Save_433M_Maxlen = Save_433M_Value[0].Device_Index;
#endif			
			DeivceFlag.GSMRet_Flag = 0;	DeivceFlag.RestFlag = 1;DeivceFlag.RunState = 0;//运行状态 RunState   0 运行 1设置
		MQTT_DeviceState(1);
//工作模式 执行下
		xEventGroupClearBits(AlarmStateHandler,(EventBits_t)0);	//清零标志组
		xEventGroupSetBits(AlarmStateHandler,	AlarmWaitBIT_0);		//开启获取报警事件
		xQueueReset(AlarmPort_Queue);//xQueueGenericReset(AlarmPort_Queue,pdFALSE );	//复位报警队列 非新创建的	
//		InitQueueMem(&MQTT_PublishQueue);
		vTaskSuspend(SetModesTaskHandler);//挂起设置参数任务
		
		if(RegisterEEpro_Slave.Host_or_Slave != Slave_Flag)	//六通道RS485从机模式不执行下二句
		{	
				vTaskResume(AlarmTaskHandler);	//恢复报警任务 
				vTaskResume(CallResultTaskHandler);//恢复模块处理回复消息任务
				vTaskResume(WIFI_onlinDetTaskHandler);//恢复WIFI
		}
////		prvUnlockQueue(AlarmPort_Queue);	//AlarmPort_Queue队列解锁		
//		xEventGroupSetBits(AlarmStateHandler, AlarmWaitBIT_0 | SMS_ReadyBIT_1 | Call_ReadyBIT_5);		//开启获取报警事件 ，恢复开启 短信 电话 接收任务

//#ifdef W433_Enabel

//#endif
		W433Init();//读取W433参数Read_Eeprom_Data();			
		HomeMenu();
/********每退出设置模式 发布所有的设置页面参数********/
		if(DeivceFlag.Refresh_Sever)
		{DeivceFlag.Refresh_Sever = 0;
		for(u8 num = 1;num < 11;num++)
			{
			Mqtt_sms_call_phone(num,0);// PortNum 端口号 1~9  sms_call 0 短信号码 1 电话号码
			Mqtt_sms_call_phone(num,1);
			Mqtt_PortMessage(num, 0);// PortNum 端口号 1~9  close_open 0 闭合短信 1 断开短信
			Mqtt_PortMessage(num, 1);	
			}MQTT_AlarmTemHum();//温度和湿度 上下限报警阀值
/**END**/			
		}
//		WIFI_SendModuleCmd(AT_WCONFIG,NULL,0);//关闭手机配网
	}
	}
		
//if(ScanProtFlag)
//	{
//	if(Beep){Beep--;AlrmBuzzer = ~AlrmBuzzer;}
//	else {AlrmBuzzer = 0;}
//	ScanProtFlag = 0;PortSignal();}//定时器控制 输入信号扫描
}

/*
保存历史报警记录，将上一数据向后移一个单位0x80 128byte
*/
void HistorySave(void)
{
	u8 dat_len ; 
	dat_len = sizeof(Save_history);
	Save_history.PortNum = Receive_Alarm.Num ;		//报警的端口号
	
	if(Save_history.PortNum == TemperHum_AddrSection)     //0温湿度 报警存入的扇区 下票
	{
		Save_history.PortNum = TemperHum_Alarm_Num;//9 报警序号
	if(!AlarmTemHum.Type_TorH)Save_history.Tem_Hum = 'T';
	else Save_history.Tem_Hum = 'H';
	}
	else if(Save_history.PortNum == PowerOFF_AddrSection)     //1停电告警存入的扇区 下票
	{
		Save_history.PortNum = PowerOFF_Alarm_Num;//10停电告警 报警序号
	  Save_history.Tem_Hum = '-';
	}
	else {Save_history.PortNum --;Save_history.Tem_Hum = '-';}
	
	if(Receive_Alarm.Adrr_Offset == Close_Message_offset)Save_history.Close_or_Open = 'C';//闭合报警0
	else Save_history.Close_or_Open = 'O';//断开报警1
	
	Save_history.AlarmFlag = dat_len;////报警标识 0x89
	Save_history.Month = RTC_DateStructure.Month;
	Save_history.Date = RTC_DateStructure.Date;
	Save_history.Year = RTC_DateStructure.Year;
	Save_history.Hours = RTC_TimeStructure.Hours;
	Save_history.Minutes = RTC_TimeStructure.Minutes;
	Save_history.Seconds = RTC_TimeStructure.Seconds;
	
	SectorErase_protect(History_Alarm_Addres[0], &Save_history.AlarmFlag, 0, dat_len,History_Alarm_size);//向后移一个单位History_Alarm_size	
} 
//功能：读取一条信息并显示 出错返回
//DisLine 显示行号
//HistoryNum  读取历史报警存储位置  报警记录空间(0x40) 64byte*10
ErrorStatus HistoryRead(OLEDLINE_e DisLine,u8 HistoryNum)
{
	u8 dat_len ;
	dat_len = sizeof(Read_history);
	if(dat_len != EEPROM_read_Byte(History_Alarm_Addres[HistoryNum]))return ERROR;	
	EEPROM_read_n(History_Alarm_Addres[HistoryNum],(char*)&Read_history,dat_len);

//if(NULL == strncmp((const char*)&Read_history.PortNum,(const char*)&Save_history.PortNum,3))/****记录没有更新 不刷新显示***/
//	{
////	char *AlarmClose_or_Open;
////	if(0 == Read_history.Close_or_Open)AlarmClose_or_Open = CloseAlarm;	//闭合报警
////	else AlarmClose_or_Open = OpenAlarm;//
	/*显示格式 NO：编号 类型 闭合--年-月-日--时-分-秒*/
	OledPrintf(LINE_LEFT, HIGH_12, DisLine, false, "%1x%1c%1c %02d-%02d-%02d %02d:%02d:%02d",
							Read_history.PortNum,Read_history.Tem_Hum,Read_history.Close_or_Open,Read_history.Year,Read_history.Month,Read_history.Date,
							Read_history.Hours,Read_history.Minutes,Read_history.Seconds);
//	}
return SUCCESS;		
}
//报警的短信信息后缀加上时间
ErrorStatus Alarm_T(void)
{
char *Alarm_time = portMalloc(30*sizeof(char)) ;
	/*显示格式 NO：编号 类型 闭合--年-月-日--时-分-秒*/
	snprintf(Alarm_time, 50,  "20%02d-%02d-%02d %02d:%02d:%02d",
							Read_history.Year,Read_history.Month,Read_history.Date,
							Read_history.Hours,Read_history.Minutes,Read_history.Seconds);
	cdma_mobile_txt_shift(Alarm_time,19);//strlen(Alarm_time)被叫号码转换成UNICODE码
	strcat(MsegTemp,Alarm_time);//后缀拼接时间
	portFree(Alarm_time);
	return SUCCESS;
}
	
//显示行号
//读取已存储的报警信息位置 (短信接收号码 电话接听号码 闭合报警 断开报警)
ErrorStatus UserInfor_Read(OLEDLINE_e DisLine,u8 Addr_Num)
{
	/*显示短信接收号码*/	
	if(SUCCESS == GetPhoneNumber(MessgeAddr[Addr_Num],(u16*)SMS_Number_Offset))
	OledPrintf(LINE_LEFT, HIGH_12, DisLine, false, "%d; ", PhoneNumber);
	/*显示电话被叫号码*/	
	if(SUCCESS == GetPhoneNumber(MessgeAddr[Addr_Num],(u16*)Call_Number_Offset))
	OledPrintf(LINE_LEFT, HIGH_12, DisLine, false, "%d; ", PhoneNumber);

	/*显示闭合报警信息*/	
	if(SUCCESS == GetAlarmMassage(MessgeAddr[Addr_Num],Close_Message_offset))
	OledPrintf(LINE_LEFT, HIGH_12, DisLine, false, "%s; ", MsegTemp);
	/*显示断开报警信息*/	
	if(SUCCESS == GetAlarmMassage(MessgeAddr[Addr_Num],Open_Message_offset))
	OledPrintf(LINE_LEFT, HIGH_12, DisLine, false, "%s; ", MsegTemp);	
	
return SUCCESS;		
}

					
 #ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param file pointer to the source file name
 * @param line assert_param error line source number
 */
void assert_failed(const uint8_t *expr, const uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* Infinite loop */
    while (1)
    {
    }
}

#endif

#ifdef LSI_TIM_MEASURE
/**
 * @brief  Configures TIM9 to measure the LSI oscillator frequency.
 */
//void TIM9_ConfigForLSI(void)
//{
//    NVIC_InitType NVIC_InitStructure;
//    TIM_ICInitType TIM_ICInitStructure;
//    /* Enable TIM9 clocks */
//    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM9, ENABLE);
//    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);
//    /* Enable the TIM9 Interrupt */
//    NVIC_InitStructure.NVIC_IRQChannel                   = TIM9_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
//    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
//    /* Configure TIM9 prescaler */
//    TIM_ConfigPrescaler(TIM9, 0, TIM_PSC_RELOAD_MODE_IMMEDIATE);
//    /* Connect internally the TM9_CH3 PB14 Input Capture to the LSI clock output */
//      GPIO_ConfigPinRemap(GPIOB_PORT_SOURCE, GPIO_PIN_SOURCE14, GPIO_AF1_TIM9);
//    /* TIM9 configuration: Input Capture mode 
//       The LSI oscillator is connected to TIM9 CH3
//       The Rising edge is used as active edge,
//       The TIM9 CCDAT3 is used to compute the frequency value */
//    TIM_ICInitStructure.Channel     = TIM_CH_3;
//    TIM_ICInitStructure.IcPolarity  = TIM_IC_POLARITY_RISING;
//    TIM_ICInitStructure.IcSelection = TIM_IC_SELECTION_DIRECTTI;
//    TIM_ICInitStructure.IcPrescaler = TIM_IC_PSC_DIV8;
//    TIM_ICInitStructure.IcFilter    = 0;
//    TIM_ICInit(TIM9, &TIM_ICInitStructure);
//    TIM9->CTRL1 |= TIM_CTRL1_C3SEL;  ////////////
//    /* TIM9 Counter Enable */
//    TIM_Enable(TIM9, ENABLE);
//    /* Reset the flags */
//    TIM9->STS = 0;
//    /* Enable the CC3 Interrupt Request */
//    TIM_ConfigInt(TIM9, TIM_INT_CC3, ENABLE);
//}
#endif

/**
 * @}
 */
