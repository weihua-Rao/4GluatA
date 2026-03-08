#include "main.h"

const char CDMA_PDU_Type[17] = {0x30,0x30,0x30,0x30,0x30,0x32,0x31,0x30,0x30,0x32,0x30,0x34,0x30,0x37,\
							0x30,0x32,0x43};  //协议数据单元类型 "00 00 02 1002 04 07 00 02 0C"//0D 91 68 13位手机号码
const char CDMA_PDU_PD[12] = {'0','0','0','3','2','0','0','0','3','0','0','1'};//
//						'0','6','2','0','1','2'};   // 数据编码  "000800"
//code char CDMA_DatPdu[3] = {'2','0','1'};	
const char CDMA_PDU_END[24] = {'0','8','0','1','0','0','0','9','0','1','0','0',\
						 '0','A','0','1','0','0','0','D','0','1','0','1'};   // 数据编码  "000800"	

char Content[Content_Lenth];		//组织短信内容	
char MsegTemp[Mseg_Lenth];		//读取临时短信
char *TX1_Data;//,RX1_Buffer[COM_RX1_Lenth];		
						 
/**************************************PDU部分********************************************/
//Phone 电话号码 		数组下标	 PDU
//MessLenght 数据包长度, 
//Message内容EEPROM地址
void cdma_pdu_message( u8 MessLenght, char *MessageAddr)
{
	u8 Phoneleng,LL;				 //PhLen,没有用到的变量
	char MsmLen[2];

//PhLen = EEPROM_read_Byte(NumberAddr[Phone]);
//EEPROM_read_n((NumberAddr[Phone]+1), PhoneNumber, PhLen);   //号码长度12 		
//	LL = (MessLenght - 2)/2;
//	LL = MessLenght/2;				 //*4G模块由由于接收尾清除了0x0a,0x0d,因此少减2个。
//	sprintf(MsmLen,"%02X",(int)LL);
	memset(Content,0,Content_Lenth);				  //清除内容  必须要清除
//	Uart1Sends("AT+CMGF=0\r\n");  //PDU模式
//	DelaySec(3);
	MessLenght+=2;
	LL = MessLenght/2;				 //*4G模块由由于接收尾清除了0x0a,0x0d,因此少减2个。
	sprintf(MsmLen,"%02X",(int)(LL+21));

	Phoneleng = CDMA_PDU_Phone();								 //30
	strcat(&Content[Phoneleng],MsmLen);					  //短信长度	  18
	Content[Phoneleng+2] = 0;
	CDMA_PDU_END_Send(32,MessageAddr,MessLenght);							//32
	CDMA_PDU_Mes(LL);				   //41*2
}

// 发送短信	 PDU
// Leght---> 发送PDU总的字符串长度
void CDMA_PDU_Mes(u8 Leght)
{
	char *SendStr = portMalloc(100 * sizeof(char));
 	u8 StrLen;
	
	Leght += 37;						//PDU串前缀长度+后缀
	memset(SendStr,0,100);
//	StrLen = sprintf(SendStr,"AT+CMGS=%-03d\r\n",Leght);
	StrLen = sprintf(SendStr,"AT+CMGS=%03d\r\n",Leght);	
	UARTx_SendData(EC20_UART, SendStr, StrLen) ;

//	Wait_For_Nms(100) ;
	Delay_Ms_StopScheduler(500);//500ms 不会引起任务调度
	StrLen = strlen(Content);
	UARTx_SendData(EC20_UART, Content, StrLen)	 ;
//	Uart1Sends(Content);  //("77ED4FE1003100234E2D56FD60A8597D");//
	Delay_Ms_StopScheduler(200);//2000ms 不会引起任务调度
	USART_SendData(UARTx_COM[EC20_UART], 0x1a);
	UARTx_SendString(EC20_UART,(u8*)"%s","\r\n");
	
	portFree(SendStr) ;
u8 times = 0;
	while( times++ < 70 )   
		{
			Wait_For_Nms(20) ;//Delay_Ms_StopScheduler(20);//
			if( NULL != strstr((const char*)ec20AtBuf, "OK") ) { break ; }				 
		}
}
/// 发送短信	 构建PDU 头串
//Phone _ 号码数组下标
//u8 PDU_Phone(u8 Phone)
u8 CDMA_PDU_Phone(void)
{
	u8 i,Phoneleng = 11;
	u8 yy = 0;
////	Phoneleng = EEPROM_read_Byte(NumberAddr[Phone]);
//		EEPROM_read_n((NumberAddr[Phone]+1), PhoneNumber, Phoneleng);
////	sprintf(SendLen,"%02d",(int)Phoneleng);
////	Uart1BYTE(Hex2Ascii(Phoneleng>>4)); Uart1BYTE(Hex2Ascii(Phoneleng));	   //测试
	for(i = 0;i < 17; i++)
	{
		Content[i] = CDMA_PDU_Type[i];					   //协议数据单元类型	 "00 00 02 1002 04 07 00 02 0C"
	}
	/**************************目标号码编码***************************/
	for(i=0;i<Phoneleng;i++)
	{
		PhoneNumber[i]=PhoneNumber[i]&0x0f;
	}
	
	for(i = Phoneleng;i<0xff;i--)
	{
		yyt.SurDat = PhoneNumber[i]<<2;
		PhoneNumber[i]	= yyt.DouBty.Bety_L + yy;
		yy = yyt.DouBty.Bety_H;
	}
	for(i=0;i<Phoneleng;i++)
	{
		Content[i+17] = Hex2Ascii(PhoneNumber[i]);
	}
	/**********************END**********************/
	Content[28] = '0'; Content[29] = '8'; //20 +	Phoneleng
	return (30);
}
//编码结束串 含短信内容
//	DatAdd  起始下标	   固定值32
//  SemLen  数组大小长度	(5个汉字	20)
//  Source[] 源地址
void CDMA_PDU_END_Send(u8 DatAdd, char *Source, u8 SemLen)
{
	u8 chr,uu,yy,LenStr;	   //短信内容计数
	u8 Rem;			//判别短信内容计数字节是不是奇数 	奇数为1 时 与后一字节或0x08
	char SendLen[2];
char *MessegTemp = portMalloc(200*sizeof(uint8_t));	 //汉字内容编码	临时变量
	
	strcat(&Content[DatAdd],CDMA_PDU_PD);					//35
	Content[DatAdd+12] = 0;
	
	chr=SemLen/2+2;
	sprintf(SendLen,"%02X",(int)chr);	//发送16进制长度串
//	SemLen-=2;			  //数据长度
	strcat(&Content[DatAdd+12],SendLen);					//长度
//	Content[DatAdd+14] = 0;
	/*********************************短信内容处理*******************************************/	
	Content[DatAdd+14] = '2';
	Content[DatAdd+15] = 0;
	LenStr = SemLen/4; Rem = LenStr%2;
	LenStr >>= 1;
	sprintf(SendLen,"%02X",(int)LenStr);
	strcat(&Content[DatAdd+15],SendLen);					//
	Content[DatAdd+17] = 0;
//	for(uu = SemLen; uu >= 0; uu--)
//	{	
//	MessegTemp[uu+1]=Ascii2Hex(Source[uu]);if(uu==0)break;	  
//	}
	uu = SemLen;
	do
	{
	MessegTemp[uu+1]=Ascii2Hex(Source[uu]);	
	}
	while(uu--);
	
	 MessegTemp[0] = 0;					  //防数据干扰
//	for(uu=SemLen;uu>=0;uu--)
//	{
//		yyt.SurDat = MessegTemp[uu]<<3;
//		MessegTemp[uu] = yyt.DouBty.Bety_L + yy;
//		yy = yyt.DouBty.Bety_H;				if(uu==0)break;
//	}
	uu=SemLen;
	do
	{
		yyt.SurDat = MessegTemp[uu]<<3;
		MessegTemp[uu] = yyt.DouBty.Bety_L + yy;
		yy = yyt.DouBty.Bety_H;	
	}
	while(uu--);
	
	if(Rem){MessegTemp[0] = MessegTemp[0]|0x08;}
	
	for(uu=0; uu < SemLen+1; uu++)
	{
		Content[49+uu] = Hex2Ascii(MessegTemp[uu]);
	}
	Content[50+SemLen] = 0;
	strcat(&Content[50+SemLen],CDMA_PDU_END);					//51 	47
	Content[50+SemLen+24] = 0;
	portFree(MessegTemp);
}
//PDU模式下回复短信，
//NumLen 被叫号码长度 短信长度  短信内容
void reponse_cdma_pdu(u8 NumLen, u8 MessLenght, char *MessageAddr)
{
//	char *SendLen;
	u8 Phoneleng,LL;				 //没有用到的变量
	char MsmLen[3];
	NumLen = NumLen;
Tled=0;	
	memset(Content,0,Content_Lenth);				  //清除内容  必须要清除
//Wait_For_Nms(100) ;//	Delay10m(10);
//if(!message_model)sim900a_send_cmd("AT+CMGF=0\r\n","OK",50); //在PDU模式下 需要转换成 PDU模式	
//Wait_For_Nms(1000) ;//	DelaySec(2);
	LL = MessLenght/2;				 //*4G模块由由于接收尾清除了0x0a,0x0d,因此少减2个。
	sprintf(MsmLen,"%02X",(int)(LL+21));
//	Uart1Sends(PhoneNumber); Uart1Sends("***");
//	EEPROM_read_n((NumberAddr[0]+1), PhoneNumber, 11);   //号码长度11 
//	Uart1Sends(PhoneNumber);Uart1Sends("!!!");
	Phoneleng = CDMA_PDU_Phone();								 //30
	strcat(&Content[Phoneleng],MsmLen);					  //短信长度	  18
//Uart1BYTE(Hex2Ascii(MessLenght>>4)); Uart1BYTE(Hex2Ascii(MessLenght));	   //测试
//Uart1Sends("!!!\r\n");	LL=MessLenght;
	Content[Phoneleng+2] = 0;
	CDMA_PDU_END_Send(32,MessageAddr,MessLenght);							//32 MessLenght
	CDMA_PDU_Mes(LL);				   //41*2	

u8 times = 0;
	while( times++ < 70 )   
		{
			Wait_For_Nms(20) ;//Delay_Ms_StopScheduler(20);//
			if( NULL != strstr((const char*)ec20AtBuf, "OK") ) { break ; }				 
		}
}

//CDMA电信PDU模式下获取主叫号码
//格式：@#-----------#	  适用于 头位为1的手机号码	  1******
u8 cdma_txt_post_phone(char*soucer)
{
	char *haystack;//=Content; "6E059664";
	char *buf;
	u8 StrLen = 0, i = 0;//,y=0
//	char *Content1= "+CMT: \"13692190284\",,\"18";
	haystack= soucer;	
	buf= strstr( haystack, NumberCall);					//
	haystack = buf + 6;		//正常是7 预留一位置 用来查找字符
	if(strstr(haystack,"\"+") != NULL)
		haystack+=2;
	else haystack++;
	
		buf = strstr( haystack, StrCall);
		buf[0]='\0';
		
	StrLen = strlen(haystack); //StrLen = StrLen/4;
/**************直接按顺序存储***************/
		for(i=0; i<StrLen; i++)
			{
			PhoneNumber[i] =haystack[i];
			}
			PhoneNumber[StrLen] = '\0';

	buf[0]= *StrCall; txt_smsphone_lenght = StrLen;
  return StrLen;
/**************END***************/		 
} 
u8 cdma_pdu_post_Phone(char*soucer)
{
    char *haystack;//=Content; "6E059664";
	char *buf;
//	char *Content1= "+CMT: \"13692190284\",,\"18";
u8 StrLen = 0, i = 0;//,y=0
//char Temp;
//	 Uart1Sends("~~~\r\n");						//test
//	 Uart1Sends(RX1_Buffer);							//test
//	 Uart1Sends("***\r\n");		 					//test
	 haystack= soucer;//RX1_Buffer;	
//	 haystack= 	Content1;					//test
	 buf= strstr( haystack, StrCall);					// -1 是为了解决手机号码前 向后移一位问题
	 haystack = buf + strlen(StrCall);

		buf[0]='\0';
		haystack = buf+ strlen(StrCall);
		buf = strstr( haystack, StrCall);
		buf[0]='\0';
//Uart1Sends("~~~\r\n");							//test
//Uart1Sends(haystack);							//test
//Uart1Sends("***\r\n");

	StrLen = strlen(haystack); //StrLen = StrLen/4;
//Uart1BYTE(Hex2Ascii(StrLen>>4)); Uart1BYTE(Hex2Ascii(StrLen));	   //测试

/**************直接按顺序存储***************/
//	StrLen = strlen(Sour); StrLen = StrLen/4;
	
		for(i=0; i<StrLen; i++)
			{
			PhoneNumber[i] =haystack[i];
			}
			PhoneNumber[StrLen] = '\0';
  return StrLen;
} 
//CDMA电信	仅限于查询功能使用
//PDU模式下获取主叫号码
//格式：0000021002020702C4DA48640A100601FC0818000310ED70010620133F2C5F1003061912092306360A0140
u8 CDMA_pdu_query(char*soucer)
{
char *haystack;//=Content;
char *buf;
u8 i = 0;//,y=0
u8 Temp,dat;
/*
u8 xdata Condat[]={0x34,0x30,0x30,0x30,0x32,0x31,0x30,0x30,0x32,0x30,0x32,0x30,0x37,0x30,0x32,0x43,0x34,0x44,\
				0x41,0x34,0x38,0x36,0x34,0x30,0x41,0x31,0x30,0x30,0x36};
*/
//	 Uart1Sends("~~~\r\n");	 					//test
//	 Uart1Sends(RX1_Buffer);							//test
//	 Uart1Sends("***\r\n");		 					//test
	 haystack= soucer;//RX1_Buffer;	
//	haystack= Condat;					//test
	buf= strstr( haystack, "0002");					// -1 是为了解决手机号码前 向后移一位问题
	haystack = buf + strlen("0002");
	haystack = haystack + 7;		  //偏移7
	buf[0]='\0';
	haystack = haystack + 2;		  //偏移2
	Temp = Ascii2Hex(*haystack++); Temp <<= 2;
	Temp += (Ascii2Hex(*haystack)>>2);

	for(i=0;i<Temp;i++)						   //对电话号码进行排序
	{
	dat = Ascii2Hex(*haystack++); dat <<= 2;
	dat += (Ascii2Hex(*haystack) >> 2);
	PhoneNumber[i] =Hex2Ascii(dat);
	if(PhoneNumber[i] == 'A')PhoneNumber[i] = '0';
	}
	PhoneNumber[i++] = 0;
	return Temp;		  //返回号码长度
} 
/**************************************END********************************************/
/**************************************TXT部分********************************************/
//CDMA电信text模式下获取主叫号码
//格式：@#-----------#	  适用于 头位为1的手机号码	  1******
//u8 cdma_txt_post_phone(char*soucer)
//{
//    char *haystack;//=Content; "6E059664";
//	char *buf;
////	char *Content1= "+CMT: \"13692190284\",,\"18";
//u8 StrLen = 0, i = 0;//,y=0
////char Temp;
////	 Uart1Sends("~~~\r\n");						//test
////	 Uart1Sends(RX1_Buffer);							//test
////	 Uart1Sends("***\r\n");		 					//test
//	 haystack= soucer;//RX1_Buffer;	
////	 haystack= 	Content1;					//test
//	 buf= strstr( haystack, one_head);	//StrCall	// -1 是为了解决手机号码前 向后移一位问题
////	 haystack = buf + strlen(StrCall);
//	 haystack = buf;
//		buf = strstr( haystack, StrCall);
//		buf[0]='\0';

//	StrLen = strlen(haystack); //StrLen = StrLen/4;
///**************直接按顺序存储***************/
////	StrLen = strlen(Sour); StrLen = StrLen/4;
//	
//		for(i=0; i<StrLen; i++)
//			{
//			PhoneNumber[i] =haystack[i];
//			}
//			PhoneNumber[StrLen] = '\0';

//buf[0]= *StrCall; txt_smsphone_lenght = StrLen;
//  return StrLen;
///**************END***************/		 
//} 
//TXT模式下cdma回复短信，
//被叫号码  短信内容
ErrorStatus   cdma_txt_message(char *number, char *Message)
{
//	u8 LL;				 //没有用到的变量
Tled=0;	

char *SendStr = portMalloc(100 * sizeof(char));
	u8 StrLen;
	memset(SendStr,0,100);Ec20AtBufReset() ;
	StrLen = sprintf(SendStr,"AT+CMGS=%s\r\n",number);
	UARTx_SendData(EC20_UART, SendStr, StrLen) ;
  Delay_Ms_StopScheduler(500);//500ms 不会引起任务调度
	StrLen = strlen(Message);
	UARTx_SendData(EC20_UART, Message, StrLen)	 ;
	Delay_Ms_StopScheduler(200);//2000ms 不会引起任务调度
	USART_SendData(UARTx_COM[EC20_UART], 0x1a);
//	UARTx_SendString(EC20_UART,(u8*)"%s","\r\n");
	portFree(SendStr) ;
	
u8 times = 0;
	while( times++ < 25 )   
		{
			Wait_For_Nms(20) ;//Delay_Ms_StopScheduler(20);//
			if( NULL != strstr((const char*)ec20AtBuf, "+CMGS:")) { return SUCCESS; }	//"OK"			 
		}
return ERROR;			
}

////TXT模式下 电信&移动 对被叫号码进行转换编码 
//void cdma_mobile_txt_shift(u8 lenght)
//{
//u8 data_temp[12]={0};
//u8 i;
//	for(i=0;i<lenght;i++)
//	   {data_temp[i] = PhoneNumber[i];}
//	for(i=0;i<lenght;i++)
//	{	
//	 PhoneNumber[i*4] = '0';
//	 PhoneNumber[i*4+1] = '0';
//	 PhoneNumber[i*4+2] = '3';
//	 PhoneNumber[i*4+3] = data_temp[i];
//	}
//	PhoneNumber[i*4] = 0;
//}

//字符串转换成UNICODE 
void cdma_mobile_txt_shift(char *Source,u8 lenght)
{
	char *data_temp = (char*)portMalloc(13*sizeof(u8) ) ;
	strncpy (data_temp,Source,lenght);
u8 i;

	for(i=0;i<lenght;i++)
	{
		if((data_temp[i] >= '0')&&(data_temp[i] <= '9'))
		{Source[i*4] = '0';Source[i*4+1] = '0';Source[i*4+2] = '3';Source[i*4+3] = data_temp[i];}
		else
		{ Source[i*4] = '0';Source[i*4+1] = '0'; Source[i*4+2] = Hex2Ascii(data_temp[i]>>4); Source[i*4+3] = Hex2Ascii(data_temp[i]);}	
	}
	Source[i*4] = 0;
	
	portFree(data_temp) ;	
}
//字符串转换成UNICODE 
//void cdma_mobile_txt_shift(char *Source,u8 lenght)
//{
//	char *data_temp = (char*)portMalloc(13*sizeof(u8) ) ;
//	strncpy (data_temp,Source,lenght);
//u8 i;

//	for(i=0;i<lenght;i++)
//	{	
//		if(data_temp[i] == '.')// 小数点 002e 
//		{ Source[i*4] = '0';Source[i*4+1] = '0'; Source[i*4+2] = '2'; Source[i*4+3] = 'E';}
//		else
//		{Source[i*4] = '0';Source[i*4+1] = '0';Source[i*4+2] = '3';Source[i*4+3] = data_temp[i];}			
//	}
//	Source[i*4] = 0;
//	
//	portFree(data_temp) ;	
//}
//******************************打电话号码改变顺序 为发短信方式PDU ***********************************************************
void PhoneoRece(u8 lenght)
{
char i,Temp;
	
	for(i=0; i<lenght; i+=2)
		{ 
		 Temp = PhoneNumber[i];
		 PhoneNumber[i] = PhoneNumber[i+1];
		 PhoneNumber[i+1] = Temp;
		}
		if(lenght%2)
		{
//		PhoneNumber[lenght] = PhoneNumber[lenght-1];
		PhoneNumber[lenght-1] = 'F';
		}
		PhoneNumber[lenght+1] = 0;	 //12
}
/**************************************END********************************************/

