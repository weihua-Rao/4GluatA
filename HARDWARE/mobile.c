#include "Main.H"

//大陆地区
const u8 PDU_Type[6] = {0x30,0x30,0x31,0x31,0x30,0x30};//0x30,0x44,0x39,0x31,0x36,0x38};  //协议数据单元类型 "1100"//0D 91 68 13位手机号码
//台湾地区
//code u8 PDU_Type[6] = {0x30,0x30,0x31,0x31,0x30,0x30};//,0x30,0x45,0x39,0x31,0x38,0x38,0x36};  //协议数据单元类型 "1100"//0e 91 68 13位手机号码
const u8 PDU_PD[6] = {0x30,0x30,0x30,0x38,0x30,0x30};   // 数据编码  "000800"	
const char *enter = "0";
/**************************************PDU部分********************************************/

//GSM移动联通
//PDU模式下获取主叫号码
//格式：0891683108705505F0040D91683196120982F4000388813052224023230454385408
u8 mobile_pdu_query(char*soucer)
{
char *haystack;//=Content;
char *buf;
u8 i = 0;//,y=0
char Temp;
//	 Uart1Sends("~~~\r\n");		2B 43 4D 54 3A 20 22 					//test
//	 Uart1Sends(RX1_Buffer);							//test
//	 Uart1Sends("***\r\n");		 					//test
	 haystack= soucer;//RX1_Buffer;	
//	haystack= Contdat;					//test
	buf= strstr( haystack, enter);					// -1 是为了解决手机号码前 向后移一位问题
	haystack = buf + strlen(enter);
	buf[0]='\0';
//	 haystack++;							//从第二位开始，数小于16
	 Temp = Ascii2Hex(*haystack);	
	 Temp = (Temp+2)*2+i;	
	 for(i=0;i<Temp;i++)haystack++;			//移动到主叫号码长度值
//	 haystack++;							//只取第二位 最大小于16
	 Temp = Ascii2Hex(*haystack);
	 for(i=0;i<5;i++)haystack++;  //向后移4位 91 86 限于大陆区域号码 +86
	 Temp -= 2; //向后移2位 86 限于大陆区域11位号码 +86

	for(i=0;i<Temp;i+=2)						   //对电话号码进行排序
	{
	 PhoneNumber[i+1] = *haystack;haystack++;
	 PhoneNumber[i] = *haystack;haystack++;
	}
PhoneNumber[Temp] = 0;		 	   //11去掉“F”
	return Temp;		  //返回号码长度
}

//Phone 电话号码 		数组下标	 PDU
//MessLenght 数据包长度, 
//Message内容EEPROM地址
//void SendMessage(u8 Phone, u8 MessLenght, u16 MessageAddr)
void mobile_pdu_Message(u8 MessLenght, char *MessageAddr)
{
	u8 Phoneleng,LL;				 //PhLen,没有用到的变量
	char MsmLen[2];

//	PhLen = strlen(PhoneNumber);	
	memset(Content,0,Content_Lenth);				  //清除内容  必须要清除
	LL = MessLenght/2;				 //*4G模块由由于接收尾清除了0x0a,0x0d,因此少减2个。
	sprintf(MsmLen,"%02X",(int)LL);

	PhoneoRece(PhoneNumLen);
	Phoneleng = mobile_pdu_Phone(PhoneNumLen);
	strcat(&Content[Phoneleng],MsmLen);						 //短信长度
	Content[Phoneleng+2] = 0;							  //加字符结束符
	strcat(&Content[Phoneleng+2],MessageAddr);
	mobile_PDU_Mes(LL);
}
/*************************** 移动联通PDU 发送短信部分*****************************************/
/// 发送短信	 PDU
// Leght---> 发送PDU总的字符串长度
void mobile_PDU_Mes(u8 Leght)
{	
char *SendStr = portMalloc(100 * sizeof(char));
//	char SendLen[4] = {0};			   
	u8 StrLen;
	Leght += 15;						//PDU串前缀长度+后缀
	memset(SendStr,0,100);
	StrLen = sprintf(SendStr,"AT+CMGS=%03d\r\n",Leght);
	UARTx_SendData(EC20_UART, SendStr, StrLen) ;
	Delay_Ms_StopScheduler(500);//500ms 不会引起任务调度
	StrLen = strlen(Content);
	UARTx_SendData(EC20_UART, Content, StrLen)	 ;
	Delay_Ms_StopScheduler(200);//2000ms 不会引起任务调度
	USART_SendData(UARTx_COM[EC20_UART], 0x1a);
	UARTx_SendString(EC20_UART,(u8*)"%s","\r\n");
	portFree(SendStr) ;
//	Delay_Ms_StopScheduler(500);//500ms 不会引起任务调度
u8 times = 0;
		while( times++ < 70 )   
			{
				Wait_For_Nms(20) ;//Delay_Ms_StopScheduler(20);//
			  if( NULL != strstr((const char*)ec20AtBuf, "OK") ) { break ; }				 
			}
}
// 发送短信	 构建PDU 头串
//Phone _ 号码数组下标
//u8 PDU_Phone(u8 Phone)
u8 mobile_pdu_Phone(u8 PhoneLen)
{
	u8  i,templen;
//	Phoneleng = strlen(PhoneNumber);
////	 	Uart1BYTE(Hex2Ascii(Phoneleng>>4)); Uart1BYTE(Hex2Ascii(Phoneleng));	   //测试
		  for(i = 0;i < 6; i++)
			{
			Content[i] = PDU_Type[i];					   //协议数据单元类型	 "003100"
			}
if(PhoneLen == 11){strcat(&Content[6],"0D9168");templen = 12;}			 //	 大陆
else if(PhoneLen == 12){strcat(&Content[6],"0C91");templen = 10;}			//	台湾
			if(PhoneLen%2)PhoneLen+=1;					//"F"
			for(i = 0;i < PhoneLen; templen++,i++)						//14		??????????????????????
			{
			Content[templen] = PhoneNumber[i];					  // 目标号码	 
			}
//			  Uart1Sends(PhoneNumber);
//			  Uart1Sends(Content);
//			Phoneleng = Phoneleng+12; 											  
		  	for(i = 0;i < 6; templen++,i++)
			{
			Content[templen] = PDU_PD[i];					   //24 数据编码  "000800"
			}
			return (templen);		   //30
}

//PDU模式下回复短信，
//NumLen 被叫号码长度 短信长度  短信内容
void response_mobile_pdu(u8 NumLen, u8 MessLenght, char *MessageAddr)
{
//	char *SendLen;
	u8 Phoneleng,LL;				 //没有用到的变量
	char  MsmLen[3];
Tled=0;	
	Ec20AtBufReset() ;// //清除内容  必须要清除
//	Wait_For_Nms(100) ;//Delay10m(10);
//	if(!message_model)sim900a_send_cmd("AT+CMGF=0\r\n","OK",50);//Uart1Sends("AT+CMGF=0\r\n");  //在PDU模式下 需要转换成 PDU模式	
//	Wait_For_Nms(1000) ;//DelaySec(2);
	LL = (MessLenght - 2)/2;
	sprintf(MsmLen,"%02X",(int)LL);

	PhoneoRece(NumLen);			   //11位号码按PDU12位模式排序+F
//	EEPROM_read_n((NumberAddr[Phone]+1), PhoneNumber, 12); //号码长度12 
	Phoneleng = mobile_pdu_Phone(NumLen);
	strcat(&Content[Phoneleng],MsmLen);						 //短信长度
//	EEPROM_read_n((MessageAddr+1),&Content[Phoneleng+2],MessLenght);
	Content[Phoneleng+2] = 0;							  //加字符结束符
	strcat(&Content[Phoneleng+2],MessageAddr); 
	mobile_PDU_Mes(LL);	

u8 times = 0;
		while( times++ < 70 )   
			{
				Wait_For_Nms(20) ;//Delay_Ms_StopScheduler(20);//
			  if( NULL != strstr((const char*)ec20AtBuf, "OK") ) { break ; }				 
			}
//	T0_1MS = 800;LL = 1;
//	while(LL){LL=!hand(ec20AtBuf,"OK"); IWDG_ReloadKey();if(!T0_1MS)LL=0;}
//	Wait_For_Nms(500) ;//DelaySec(2);
}
/**************************************END********************************************/
/**************************************TXT部分********************************************/
//TXT模式下gsm回复短信，
//被叫号码  短信内容
ErrorStatus  mobile_txt_message(char *number, char *Message)
{
//	u8 LL;				 
Tled=0;	
	
	char *SendStr = portMalloc(100 * sizeof(char));
	u16 StrLen;
	memset(SendStr,0,100);
	Ec20AtBufReset() ;
	StrLen = sprintf(SendStr,"AT+CMGS=%s\r\n",number);
	UARTx_SendData(EC20_UART, SendStr, StrLen) ;
  Delay_Ms_StopScheduler(500);//500ms 不会引起任务调度
	StrLen = strlen(Message);
	UARTx_SendData(EC20_UART, Message, StrLen)	 ;
	Delay_Ms_StopScheduler(200);//2000ms 不会引起任务调度
	USART_SendData(UARTx_COM[EC20_UART], 0x1a);
//	UARTx_SendString(EC20_UART,(u8*)"%s","\r\n");
	portFree(SendStr) ;
//	Delay_Ms_StopScheduler(500);//500ms 不会引起任务调度
	
u8 times = 0;
	while( times++ < 25 )   
		{
			Wait_For_Nms(20) ;//Delay_Ms_StopScheduler(20);//
			if( NULL != strstr((const char*)ec20AtBuf, "+CMGS:")) { return SUCCESS; }	//"OK"			 
		}
return ERROR;			
}
//GSM 移动联通text模式下获取主叫号码
//格式：@#-----------#	  适用于 头位为1的手机号码	  1******
char* mobile_txt_post_phone(char*soucer)
{
	char *haystack;//=Content;
	char *buf;
	u8 StrLen = 0, i = 0,y;
//	char *Content1= "+CMT: \"002B0038003600310033003600390032003100390030003200380034\",\"\",\"21/01/05,13:10:04+32";
	 haystack= soucer;	
	 buf= strstr( haystack, NumberCall);					//+CMT: "
	if(strstr(haystack,"002B")!=NULL)haystack = buf + 7 + 4;
	else haystack = buf + 7;//strlen(NumberCall);//strlen(StrCall);
	
		buf = strstr( haystack, StrCall);
		buf[0]='\0';

	StrLen = strlen(haystack); 
	StrLen = StrLen/4; //StrLen += 1;
/**************直接按顺序存储***************/
		for(i=0,y = 1; i<StrLen; i++,y++)
			{
			PhoneNumber[i] =haystack[y*4-1];
			}
			PhoneNumber[StrLen] = 0;
/**************END***************/		 

buf[0]= *StrCall; txt_smsphone_lenght = StrLen;

return buf++;	//	返回截取号码后，第一个地址
}
//u8 mobile_txt_post_phone(char*soucer)
//{
//    char *haystack;//=Content;
//	char *buf;
///*+CMT: "002B0038003600310033003600390032003100390030003200380034","","21/01/05,13:10:04+32"*/
//u8 StrLen = 0, i = 0;
////char Temp;
////	 Uart1Sends("~~~\r\n");						//test
//	 haystack= soucer;//RX1_Buffer;	
////	 haystack= 	PhoneNumber8;					//test
//	 buf= strstr( haystack, one_head)-1;					// -1 是为了解决手机号码前 向后移一位问题
//	 haystack = buf + strlen(one_head);

//		buf = strstr( haystack, StrCall);
//		buf[0]='\0';
////Uart1Sends("~~~\r\n");							//test
////Uart1Sends(haystack);							//test
////Uart1Sends("***\r\n");

//	StrLen = strlen(haystack); 
//	StrLen = StrLen/4; StrLen += 1;
///**************直接按顺序存储***************/
//		for(i=0; i<StrLen; i++)
//			{
//			PhoneNumber[i] =haystack[i*4];
//			}
//			PhoneNumber[StrLen] = 0;
///**************END***************/		 

//buf[0]= *StrCall; txt_smsphone_lenght = StrLen;

//return StrLen;
//}
/**************************************END********************************************/

