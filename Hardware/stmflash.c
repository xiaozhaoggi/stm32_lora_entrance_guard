/***************************************************************
Copyright  SIEE OF HUE All rights reserved.
文件名		: stmflash.c
作者	  	: 张博钊
版本	   	: V1.0
描述	   	: stm32flash存储操作
其他	   	: 无
电话 	   	: 13229321127 
日志	   	: 初版V1.0 2021/4/23 张博钊创建
***************************************************************/
#include "stmflash.h"
#include <stdlib.h>

extern void    FLASH_PageErase(uint32_t PageAddress);
//flash: 0x0800 0000 - 0x0807ffff
uint8_t net_id;
uint32_t default_password = 123456;
//读取指定地址的半字(16位数据) 
//faddr:读地址 
//返回值:对应数据.
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr)
{
	return *(__IO uint16_t*)faddr; 
}
#if STM32_FLASH_WREN	//如果使能了写   
//不检查的写入
//WriteAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数   
void STMFLASH_Write_NoCheck(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite)   
{ 			 		 
	uint16_t i;
	for(i=0;i<NumToWrite;i++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//地址增加2.
	}  
} 
//从指定地址开始写入指定长度的数据
//WriteAddr:起始地址(此地址必须为2的倍数!!)
//pBuffer:数据指针
//NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //字节
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
uint16_t STMFLASH_BUF[STM_SECTOR_SIZE/2];//最多是2K字节
void STMFLASH_Write(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite)	
{
	uint32_t secpos;	   //扇区地址
	uint16_t secoff;	   //扇区内偏移地址(16位字计算)
	uint16_t secremain; //扇区内剩余地址(16位字计算)	   
 	uint16_t i;    
	uint32_t offaddr;   //去掉0X08000000后的地址
	
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//非法地址
	
	HAL_FLASH_Unlock();					//解锁
	offaddr=WriteAddr-STM32_FLASH_BASE;		//实际偏移地址.
	secpos=offaddr/STM_SECTOR_SIZE;			//扇区地址  0~255 for STM32F103RCT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//在扇区内的偏移(2个字节为基本单位.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//扇区剩余空间大小   
	if(NumToWrite<=secremain)secremain=NumToWrite;//不大于该扇区范围
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//读出整个扇区的内容
		for(i=0;i<secremain;i++)	//校验数据
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//需要擦除  	  
		}
		if(i<secremain)				//需要擦除
		{
			FLASH_PageErase(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);	//擦除这个扇区
			FLASH_WaitForLastOperation(FLASH_WAITETIME);            	//等待上次操作完成
			CLEAR_BIT(FLASH->CR, FLASH_CR_PER);							//清除CR寄存器的PER位，此操作应该在FLASH_PageErase()中完成！
																		//但是HAL库里面并没有做，应该是HAL库bug！
			for(i=0;i<secremain;i++)//复制
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//写入整个扇区  
		}else 
		{
			FLASH_WaitForLastOperation(FLASH_WAITETIME);       	//等待上次操作完成
			STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//写已经擦除了的,直接写入扇区剩余区间. 
		}
		if(NumToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;				//扇区地址增1
			secoff=0;				//偏移位置为0 	 
		   	pBuffer+=secremain;  	//指针偏移
			WriteAddr+=secremain*2;	//写地址偏移(16位数据地址,需要*2)	   
		   	NumToWrite-=secremain;	//字节(16位)数递减
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//下一个扇区还是写不完
			else secremain=NumToWrite;//下一个扇区可以写完了
		}	 
	};	
	HAL_FLASH_Lock();		//上锁
}
#endif

//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void STMFLASH_Read(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead)   	
{
	uint16_t i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
		ReadAddr+=2;//偏移2个字节.	
	}
}

//////////////////////////////////////////测试用///////////////////////////////////////////
//WriteAddr:起始地址
//WriteData:要写入的数据
void Test_Write(uint32_t WriteAddr,uint16_t WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//写入一个字 
}

//FLASH写入数据
void writeFlash(uint32_t writeFlashData, uint32_t addr)
{
	//1、解锁FLASH
  HAL_FLASH_Unlock();
	
	//2、擦除FLASH
	//初始化FLASH_EraseInitTypeDef
	FLASH_EraseInitTypeDef f;
	f.TypeErase = FLASH_TYPEERASE_PAGES;
	f.PageAddress = addr;
	f.NbPages = 1;
	//设置PageError
	uint32_t PageError = 0;
	//调用擦除函数
	HAL_FLASHEx_Erase(&f, &PageError);

	//3、对FLASH烧写
	HAL_FLASH_Program(TYPEPROGRAM_WORD, addr, writeFlashData);
	
	//4、锁住FLASH
  HAL_FLASH_Lock();
}
uint8_t flash_init()
{
	uint16_t temp = 0x66;
	if(*(__IO uint16_t*)INT_FLAG_ADDR != 0x66) 
	{

		STMFLASH_Write(PASSWORD_BASE_ADDR,(uint16_t *)&default_password,2);
		HAL_Delay(300);
		STMFLASH_Write(INT_FLAG_ADDR,(uint16_t *)&temp,1);
		
	}
}
/*

			上电后读flash

														*/
void read_flash()
{
	net_id = *(__IO uint32_t*)NET_FLASH_ADDR;
}

/*

				读netid

												*/
uint32_t read_netid()
{
	return *(__IO uint32_t*)NET_FLASH_ADDR;
}

/*

			在flash中寻卡号

												*/
uint8_t search_cardid(uint32_t card_id)
{
	uint16_t i;
	uint32_t temp;
	for(i = 0; i < CARDID_NUM; i+=4)
	{
		temp = *(__IO uint32_t*)(CARDID_BASE_ADDR+i);
		if(card_id == temp)
		return 1;
	}
	return 0;
}

/*

			在flash中寻卡号地址

												*/
uint32_t search_cardid_addr(uint32_t card_id)
{
	uint16_t i;
	uint32_t temp;
	for(i = 0; i < CARDID_NUM; i+=4)
	{
		temp = *(__IO uint32_t*)(CARDID_BASE_ADDR+i);
		if(card_id == temp)
		return CARDID_BASE_ADDR+i;
	}
	return 0;
}

/*

			在flash中寻空余的卡号存储地址

																*/
uint32_t insert_cardid_addr()
{
	uint16_t i;
	uint32_t temp;
	for(i = 0; i < CARDID_NUM; i+=4)
	{
		temp = *(__IO uint32_t*)(CARDID_BASE_ADDR+i);
		if(temp == NULL_FLASH)
		return CARDID_BASE_ADDR+i;
	}
	return 0;
}

/*

			删除所有卡号

												*/
void clear_all_cardid()
{
	FLASH_PageErase(254*STM_SECTOR_SIZE+STM32_FLASH_BASE);	//擦除这个扇区
}
/*

			在flash中寻密码

												*/
uint8_t search_password(uint32_t password)
{
	uint16_t i;
	uint32_t temp;
	for(i = 0; i < PASSWORD_NUM; i+=4)
	{
		temp = *(__IO uint32_t*)(PASSWORD_BASE_ADDR+i);
		if(password == temp)
		return 1;
	}
	return 0;
}

/*

			在flash中寻密码地址

														*/
uint32_t search_password_addr(uint32_t password)
{
	uint16_t i;
	uint32_t temp;
	for(i = 0; i < PASSWORD_NUM; i+=4)
	{
		temp = *(__IO uint32_t*)(PASSWORD_BASE_ADDR+i);
		if(password == temp)
		return PASSWORD_BASE_ADDR+i;
	}
	return 0;
}

/*

			在flash中寻空余的密码存储地址

																*/
uint32_t insert_password_addr()
{
	uint16_t i;
	uint32_t temp;
	for(i = 0; i < PASSWORD_NUM; i+=4)
	{
		temp = *(__IO uint32_t*)(PASSWORD_BASE_ADDR+i);
		if(temp == NULL_FLASH)
		return PASSWORD_BASE_ADDR+i;
	}
	return 0;
}

//FLASH读取数据并打印（测试用）
void printFlash(uint32_t addr)
{
  uint32_t temp = *(__IO uint32_t*)(addr);

	printf("addr:0x%x, data:0x%x\r\n", addr, temp);
}






