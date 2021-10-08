/***************************************************************
Copyright  SIEE OF HUE All rights reserved.
�ļ���		: stmflash.c
����	  	: �Ų���
�汾	   	: V1.0
����	   	: stm32flash�洢����
����	   	: ��
�绰 	   	: 13229321127 
��־	   	: ����V1.0 2021/4/23 �Ų��ȴ���
***************************************************************/
#include "stmflash.h"
#include <stdlib.h>

extern void    FLASH_PageErase(uint32_t PageAddress);
//flash: 0x0800 0000 - 0x0807ffff
uint8_t net_id;
uint32_t default_password = 123456;
//��ȡָ����ַ�İ���(16λ����) 
//faddr:����ַ 
//����ֵ:��Ӧ����.
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr)
{
	return *(__IO uint16_t*)faddr; 
}
#if STM32_FLASH_WREN	//���ʹ����д   
//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��   
void STMFLASH_Write_NoCheck(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite)   
{ 			 		 
	uint16_t i;
	for(i=0;i<NumToWrite;i++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//��ַ����2.
	}  
} 
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
//pBuffer:����ָ��
//NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
uint16_t STMFLASH_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽ�
void STMFLASH_Write(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite)	
{
	uint32_t secpos;	   //������ַ
	uint16_t secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	uint16_t secremain; //������ʣ���ַ(16λ�ּ���)	   
 	uint16_t i;    
	uint32_t offaddr;   //ȥ��0X08000000��ĵ�ַ
	
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//�Ƿ���ַ
	
	HAL_FLASH_Unlock();					//����
	offaddr=WriteAddr-STM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	secpos=offaddr/STM_SECTOR_SIZE;			//������ַ  0~255 for STM32F103RCT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//����ʣ��ռ��С   
	if(NumToWrite<=secremain)secremain=NumToWrite;//�����ڸ�������Χ
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//������������������
		for(i=0;i<secremain;i++)	//У������
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//��Ҫ����  	  
		}
		if(i<secremain)				//��Ҫ����
		{
			FLASH_PageErase(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);	//�����������
			FLASH_WaitForLastOperation(FLASH_WAITETIME);            	//�ȴ��ϴβ������
			CLEAR_BIT(FLASH->CR, FLASH_CR_PER);							//���CR�Ĵ�����PERλ���˲���Ӧ����FLASH_PageErase()����ɣ�
																		//����HAL�����沢û������Ӧ����HAL��bug��
			for(i=0;i<secremain;i++)//����
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//д����������  
		}else 
		{
			FLASH_WaitForLastOperation(FLASH_WAITETIME);       	//�ȴ��ϴβ������
			STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 
		}
		if(NumToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;				//������ַ��1
			secoff=0;				//ƫ��λ��Ϊ0 	 
		   	pBuffer+=secremain;  	//ָ��ƫ��
			WriteAddr+=secremain*2;	//д��ַƫ��(16λ���ݵ�ַ,��Ҫ*2)	   
		   	NumToWrite-=secremain;	//�ֽ�(16λ)���ݼ�
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//��һ����������д����
			else secremain=NumToWrite;//��һ����������д����
		}	 
	};	
	HAL_FLASH_Lock();		//����
}
#endif

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void STMFLASH_Read(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead)   	
{
	uint16_t i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
}

//////////////////////////////////////////������///////////////////////////////////////////
//WriteAddr:��ʼ��ַ
//WriteData:Ҫд�������
void Test_Write(uint32_t WriteAddr,uint16_t WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//д��һ���� 
}

//FLASHд������
void writeFlash(uint32_t writeFlashData, uint32_t addr)
{
	//1������FLASH
  HAL_FLASH_Unlock();
	
	//2������FLASH
	//��ʼ��FLASH_EraseInitTypeDef
	FLASH_EraseInitTypeDef f;
	f.TypeErase = FLASH_TYPEERASE_PAGES;
	f.PageAddress = addr;
	f.NbPages = 1;
	//����PageError
	uint32_t PageError = 0;
	//���ò�������
	HAL_FLASHEx_Erase(&f, &PageError);

	//3����FLASH��д
	HAL_FLASH_Program(TYPEPROGRAM_WORD, addr, writeFlashData);
	
	//4����סFLASH
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

			�ϵ���flash

														*/
void read_flash()
{
	net_id = *(__IO uint32_t*)NET_FLASH_ADDR;
}

/*

				��netid

												*/
uint32_t read_netid()
{
	return *(__IO uint32_t*)NET_FLASH_ADDR;
}

/*

			��flash��Ѱ����

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

			��flash��Ѱ���ŵ�ַ

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

			��flash��Ѱ����Ŀ��Ŵ洢��ַ

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

			ɾ�����п���

												*/
void clear_all_cardid()
{
	FLASH_PageErase(254*STM_SECTOR_SIZE+STM32_FLASH_BASE);	//�����������
}
/*

			��flash��Ѱ����

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

			��flash��Ѱ�����ַ

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

			��flash��Ѱ���������洢��ַ

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

//FLASH��ȡ���ݲ���ӡ�������ã�
void printFlash(uint32_t addr)
{
  uint32_t temp = *(__IO uint32_t*)(addr);

	printf("addr:0x%x, data:0x%x\r\n", addr, temp);
}






