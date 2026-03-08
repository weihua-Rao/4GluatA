#ifndef mobile_pdu_H
#define mobile_pdu_H

//#include "n32g43x.h"

extern char Content[];		//组织短信内容	
extern char MsegTemp[];		//读取临时短信
//extern char *TX1_Data,RX1_Buffer[];

u8 mobile_pdu_query(char*soucer);
u8 mobile_pdu_Phone(u8 PhoneLen);
void mobile_PDU_Mes(u8 Leght);
void mobile_pdu_Message(u8 MessLenght, char *MessageAddr);
void response_mobile_pdu(u8 NumLen, u8 MessLenght, char *MessageAddr);

ErrorStatus  mobile_txt_message(char *number, char *Message);
char*  mobile_txt_post_phone(char*soucer);
#endif
