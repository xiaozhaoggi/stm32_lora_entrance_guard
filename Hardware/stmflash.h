#ifndef __STMFLASH_H__
#define __STMFLASH_H__
#include "stm32f1xx_hal.h"
#include <stdio.h>

#define  	NET_FLASH_ADDR			0x0807F834
#define 	INT_FLAG_ADDR				0x0807F830

#define		CARDID_NUM					500*4 //卡号数量*4字节
#define		PASSWORD_NUM				10*4	//密码数量*4字节
#define 	CARDID_BASE_ADDR		0x0807F000//000-7D0 0x7D0/4=500个卡号
#define  	PASSWORD_BASE_ADDR	0x0807F800//800-828 0x28/4=10个密码

#define   NULL_FLASH					0xffffffff	

//用户根据自己的需要设置
#define STM32_FLASH_SIZE 	512 	 		//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN 	1              	//使能FLASH写入(0，不是能;1，使能)
#define FLASH_WAITETIME  	50000          	//FLASH等待超时时间

//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 		//STM32 FLASH的起始地址

uint8_t STMFLASH_GetStatus(void);				  //获得状态
uint8_t STMFLASH_WaitDone(uint16_t time);				  //等待操作结束
uint8_t STMFLASH_ErasePage(uint32_t paddr);			  //擦除页
uint8_t STMFLASH_WriteHalfWord(uint32_t faddr, uint16_t dat);//写入半字
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr);		  //读出半字  
void STMFLASH_WriteLenByte(uint32_t WriteAddr,uint32_t DataToWrite,uint16_t Len);	//指定地址开始写入指定长度的数据
uint32_t STMFLASH_ReadLenByte(uint32_t ReadAddr,uint16_t Len);						//指定地址开始读取指定长度数据
void STMFLASH_Write(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite);		//从指定地址开始写入指定长度的数据
void STMFLASH_Read(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead);   		//从指定地址开始读出指定长度的数据

//测试写入
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

















