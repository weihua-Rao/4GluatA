#ifndef _CDMA_H_
#define _CDMA_H_

#include "n32g45x.h"

uint8_t CDMA_pdu_query(char*soucer);
u8 CDMA_PDU_Phone(void);
void CDMA_PDU_END_Send(u8 DatAdd, char *Source, u8 SemLen);
void CDMA_PDU_Mes(u8 Leght);
void cdma_pdu_message(u8 MessLenght, char *MessageAddr);
void reponse_cdma_pdu(u8 NumLen, u8 MessLenght, char *MessageAddr);
u8 cdma_txt_post_phone(char*soucer);

ErrorStatus   cdma_txt_message(char *number, char *Message);
void cdma_mobile_txt_shift(char *Source,u8 lenght);
void  PhoneoRece(u8 lenght);
u8 cdma_pdu_post_Phone(char*soucer);

#endif
