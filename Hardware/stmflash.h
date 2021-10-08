#ifndef __STMFLASH_H__
#define __STMFLASH_H__
#include "stm32f1xx_hal.h"
#include <stdio.h>

#define  	NET_FLASH_ADDR			0x0807F834
#define 	INT_FLAG_ADDR				0x0807F830

#define		CARDID_NUM					500*4 //��������*4�ֽ�
#define		PASSWORD_NUM				10*4	//��������*4�ֽ�
#define 	CARDID_BASE_ADDR		0x0807F000//000-7D0 0x7D0/4=500������
#define  	PASSWORD_BASE_ADDR	0x0807F800//800-828 0x28/4=10������

#define   NULL_FLASH					0xffffffff	

//�û������Լ�����Ҫ����
#define STM32_FLASH_SIZE 	512 	 		//��ѡSTM32��FLASH������С(��λΪK)
#define STM32_FLASH_WREN 	1              	//ʹ��FLASHд��(0��������;1��ʹ��)
#define FLASH_WAITETIME  	50000          	//FLASH�ȴ���ʱʱ��

//FLASH��ʼ��ַ
#define STM32_FLASH_BASE 0x08000000 		//STM32 FLASH����ʼ��ַ

uint8_t STMFLASH_GetStatus(void);				  //���״̬
uint8_t STMFLASH_WaitDone(uint16_t time);				  //�ȴ���������
uint8_t STMFLASH_ErasePage(uint32_t paddr);			  //����ҳ
uint8_t STMFLASH_WriteHalfWord(uint32_t faddr, uint16_t dat);//д�����
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr);		  //��������  
void STMFLASH_WriteLenByte(uint32_t WriteAddr,uint32_t DataToWrite,uint16_t Len);	//ָ����ַ��ʼд��ָ�����ȵ�����
uint32_t STMFLASH_ReadLenByte(uint32_t ReadAddr,uint16_t Len);						//ָ����ַ��ʼ��ȡָ����������
void STMFLASH_Write(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite);		//��ָ����ַ��ʼд��ָ�����ȵ�����
void STMFLASH_Read(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����

//����д��
void Test_Write(uint32_t WriteAddr,uint16_t WriteData);								   

void writeFlash(uint32_t writeFlashData, uint32_t addr);
void read_flash();
void clear_all_cardid();

uint8_t flash_init();
uint32_t read_netid();

uint8_t search_cardid(uint32_t card_id);
uint32_t search_cardid_addr(uint32_t card_id);
uint32_t insert_cardid_addr();

uint8_t search_password(uint32_t password);
uint32_t search_password_addr(uint32_t password);
uint32_t insert_password_addr();

void printFlash(uint32_t addr);
	   
#endif

















