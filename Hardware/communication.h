#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H

#include "rc522.h"
#include <stdio.h>
#include "stm32f1xx_hal.h"

#define  MULOR  0xA001       //CRC异或多项式


//order
#define READING_CARD		0x00
#define OPEN_DOOR				0x01
#define	MOD_ADDRESS			0x02
#define	GET_ADDRESS			0x03
#define	ADD_CARDID			0x04	
#define	DEL_CARDID			0x05
#define	CLR_CARDID			0x06
#define	ADD_PASSWORD		0x07
#define	ADJUST_DATA			0x08
#define CHANGE_PASSWORD	0x09
#define SEND_PASSWORD		0x0a
//status
#define SUCCEED 		0x00
#define CRCERROR 		0x01
#define OTHERERROR  0x02

//cmd return len
#define READING_CARD_LEN		0x0b
#define OPEN_DOOR_LEN				0x07
#define MOD_ADDRESS_LEN			0x07
#define GET_ADDRESS_LEN			0x08
#define	ADD_CARDID_LEN 			0x07
#define	DEL_CARDID_LEN			0x07
#define	CLR_CARDID_LEN			0x07
#define	ADD_PASSWORD_LEN		0x07
#define	ADJUST_DATA_LEN			0x07
#define CHANGE_PASSWORD_LEN	0x07
#define SEND_PASSWORD_LEN		0x0a

typedef	struct LORA_COM
{
	#define SOF				0xAA
	unsigned char des;
	unsigned char sou;
	unsigned char len;
	unsigned char cmd;
	unsigned char status;
	unsigned char payload[100];
	unsigned char crc_m[2];
	unsigned char crc_s[2];
	#define EOFF				0x55
}LORA_COM;



uint8_t rc522_work(unsigned int *car_num);
void send_cardid(unsigned char status);
void send_password(unsigned int password);
void lora_cmd_judge(LORA_COM *lora_com);
void lora_irq(unsigned char res, LORA_COM *lora_com);
unsigned char lock_control(unsigned char control);
void data_led();
void display_main_gui(void);
void display_week(uint8_t week);
void beep(uint8_t mode);
void lcd_switch(uint8_t mode);
unsigned int CRC_Check(const unsigned char *buff,unsigned int length);

#endif