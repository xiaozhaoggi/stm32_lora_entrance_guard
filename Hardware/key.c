/***************************************************************
Copyright  SIEE OF HUE All rights reserved.
文件名		: key.c
作者	  	: 张博钊
版本	   	: V1.0
描述	   	: 按键相关函数  包括显示屏操作
其他	   	: 无
电话 	   	: 13229321127
日志	   	: 初版V1.0 2021/4/23 张博钊创建
***************************************************************/

#include "key.h"
#include "lcd.h"
#include "gui.h"
#include <stdlib.h>
#include "stmflash.h"
#include "string.h"
#include "communication.h"
/*	
	1 	2 	3
	4 	5 	6	   
	7 	8   9		
	*	  0	  #		*/
unsigned char key_value[3][4] = {	0x3,0x6,0x9,0xb,
																	0x2,0x5,0x8,0x0,
																	0x1,0x4,0x7,0xa	};
unsigned char key_status;
unsigned char password[6];		
unsigned char password_num;																	
unsigned char set_password_flag;
static uint32_t password_addr;	
static uint8_t menu_sta = 0;//1：进入输入密码开门模式.  2：修改密码模式															
unsigned char key_scan()
{

	scan_row(0);
		
	if(key_status && KEY_COL1 == GPIO_PIN_SET && KEY_COL2 == GPIO_PIN_SET &&  KEY_COL3 == GPIO_PIN_SET && KEY_COL4 == GPIO_PIN_SET)
		key_status = 0;
	HAL_Delay(100);
	//if(key_status == 0)  //支持连按
	{	
		if(KEY_COL1 == GPIO_PIN_RESET)
		{	
			key_status = 1;
			scan_row(1);
			if(KEY_COL1 == GPIO_PIN_SET)
				return key_value[0][0];
			else
			{
				scan_row(2);
				if(KEY_COL1 == GPIO_PIN_SET)
					return key_value[1][0];
				else
				{
					scan_row(3);
					if(KEY_COL1 == GPIO_PIN_SET)
						return key_value[2][0];
				}
			}
					
		}
		
		if(KEY_COL2 == GPIO_PIN_RESET)
		{
			key_status = 1;
			scan_row(1);
			if(KEY_COL2 == GPIO_PIN_SET)
				return key_value[0][1];
			else
			{
				scan_row(2);
				if(KEY_COL2 == GPIO_PIN_SET)
					return key_value[1][1];
				else
				{
					scan_row(3);
					if(KEY_COL2 == GPIO_PIN_SET)
						return key_value[2][1];
				}
			}
				
		}
		
		if(KEY_COL3 == GPIO_PIN_RESET)
		{
			key_status = 1;
			scan_row(1);
			if(KEY_COL3 == GPIO_PIN_SET)
				return key_value[0][2];
			else
			{
				scan_row(2);
				if(KEY_COL3 == GPIO_PIN_SET)
					return key_value[1][2];
				else
				{
					scan_row(3);
					if(KEY_COL3 == GPIO_PIN_SET)
						return key_value[2][2];
				}
			}	
			
		}
		
		if(KEY_COL4 == GPIO_PIN_RESET)
		{
			key_status = 1;
			scan_row(1);
			if(KEY_COL4 == GPIO_PIN_SET)
				return key_value[0][3];
			else
			{
				scan_row(2);
				if(KEY_COL4 == GPIO_PIN_SET)
					return key_value[1][3];
				else
				{
					scan_row(3);
					if(KEY_COL4 == GPIO_PIN_SET)
						return key_value[2][3];
				}
			}			
		}
	}
	
	return 0x0d;
}

void scan_row(char row)
{
	switch(row)
	{
		
		case 0:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
			break;
		
		case 1:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
			break;
		
		case 2:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
			break;
		
		case 3:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
			break;
		
		default:
			break;
		
	}
}

void manage_key_value(unsigned char key_value)
{
	char i;
	uint32_t temp;
	switch(key_value)
	{
		case 0x0a:
			
			break;
		
		case 0x0b:
			HAL_Delay(1000);
			HAL_Delay(1000);
			if(key_scan() == 0x0b)
			{
				set_password_flag = 1;
				password_num = 0;
				LCD_Clear(BLACK);
				Gui_StrCenter(20,70,WHITE,BLACK,"1.Input password to open door",FRONT_SIZE,0);
				Gui_StrCenter(20,140,WHITE,BLACK,"2.Change password",FRONT_SIZE,0);
				break;
			}

			break;
		
		case 0x0d: 
			break;
		
		default:

			break;
	}
	
}

void set_password(unsigned char key_value)
{
	uint8_t i;
	switch(key_value)
	{
		case 0x0a:
			LCD_Fill(100,60,220,72,BLACK);
			memset(password, 0, sizeof(password));
			password_num = 0;
			break;
		case 0x0b:

			break;
		
		case 0x0d: 
			
			break;
			
		
		default:
			
			if(menu_sta == 0)//如果在主菜单
			{
				if(key_value == 0x01)//按下1 进入输入密码开门界面
				{
					menu_sta = 1;
					HAL_Delay(1000);
					LCD_Clear(BLACK);
					Gui_StrCenter(20,30,WHITE,BLACK,"Enter password",FRONT_SIZE,0);
					for(uint8_t i  = 0; i < 6; i++)
					Show_Str(110+20*i,70,WHITE,BLACK,"-",FRONT_SIZE,0);
					break;
				}
				else if(key_value == 0x02)//按下2  进入修改密码界面
				{
					menu_sta = 2;
					HAL_Delay(1000);
					LCD_Clear(BLACK);
					Gui_StrCenter(20,30,WHITE,BLACK,"Enter old password",FRONT_SIZE,0);
					for(uint8_t i  = 0; i < 6; i++)
					 Show_Str(110+20*i,70,WHITE,BLACK,"-",FRONT_SIZE,0);
					break;
				}
			}
			
			if(menu_sta == 1)//如果进入输入密码开门界面
			{
				password_to_opendoor(key_value);
			}
			if(menu_sta == 2)//如果进入修改密码界面
			{
				change_password(key_value);
			}
			
			break;
		
	}
	
}

void password_to_opendoor(uint8_t key_value)
{
	uint32_t temp;
	password[password_num] = key_value;
	Show_Str(110+password_num*20,60,WHITE,BLACK,"*",FRONT_SIZE,0);
	if(password_num < 5)
		password_num++;
	
	if(password_num == 5 && password[5]!= NULL)
	{
		temp = password[0]*100000+password[1]*10000+password[2]*1000+password[3]*100+password[4]*10+password[5]*1;	
		if(search_password(temp))
		{
			send_password(temp);//发送此时开门的密码
			lock_control(1);
			//printf("door open\r\n");
		}	
		else
		{
			LCD_Clear(BLACK);
			//printf("unknown password");
			Gui_StrCenter(0,60,WHITE,BLACK,"Unknown password",FRONT_SIZE,0);
			HAL_Delay(1000);
			HAL_Delay(1000);
			display_main_gui();
		}
//		printf("password: ");
//		printf("%u", temp);
//		printf("\r\n");
		memset(password, NULL, sizeof(password));
		password_num = 0;
		
		set_password_flag = 0;//开门功能完成
		memset(password, 0, sizeof(password));
		password_num = 0;
		menu_sta = 0;
		
		
	}
}

void change_password(uint8_t key_value)
{
	static uint32_t temp1;
         uint32_t temp2;
	static uint8_t change_sta = 0; //0：未输满6位密码。  1：有此密码 输6位新密码。  
																	//2：再次确认新密码。 ①两次输入密码一样，修改密码，返回主界面。②输入密码不一样，返回主界面

	switch(change_sta)
	{
		case 0://还未输满6位密码
		
				password[password_num] = key_value;
				Show_Str(110+password_num*20,60,WHITE,BLACK,"*",FRONT_SIZE,0);
				if(password_num < 5)
					password_num++;
			
			if(password_num == 5 && password[5]!= NULL)//如果输完6位密码
			{
				temp1 = password[0]*100000+password[1]*10000+password[2]*1000+password[3]*100+password[4]*10+password[5]*1;
				password_addr = search_password_addr(temp1);
				if(password_addr)//如果有这个密码
				{
					LCD_Clear(BLACK);
					change_sta = 1;//进入修改状态
					memset(password, 0, sizeof(password));
					password_num = 0;
					LCD_Clear(BLACK);
					Gui_StrCenter(20,30,WHITE,BLACK,"Enter new password",FRONT_SIZE,0);
					for(uint8_t i  = 0; i < 6; i++)
					 Show_Str(110+20*i,70,WHITE,BLACK,"-",FRONT_SIZE,0);
				}
				else
				{
					
					set_password_flag = 0;//修改功能完成
					memset(password, 0, sizeof(password));
					password_num = 0;
					change_sta = 0;
					menu_sta = 0;
					
					LCD_Clear(BLACK);
					Gui_StrCenter(20,60,WHITE,BLACK,"Unknown password",FRONT_SIZE,0);
					HAL_Delay(1000);
					HAL_Delay(1000);
					
					display_main_gui();
				}
			}
		 break;
			
		case 1://如果进入修改状态
		
			
			password[password_num] = key_value;
			Show_Str(110+password_num*20,60,WHITE,BLACK,"*",FRONT_SIZE,0);
			if(password_num < 5)
				password_num++;
			
			if(password_num == 5 && change_sta && password[5]!= NULL)//如果输入完新密码
			{
				LCD_Clear(BLACK);
				change_sta = 0;
				temp1 = password[0]*100000+password[1]*10000+password[2]*1000+password[3]*100+password[4]*10+password[5]*1;//计算新密码
				change_sta = 2;

				memset(password, 0, sizeof(password));
				password_num = 0;
				
				LCD_Clear(BLACK);
				Gui_StrCenter(20,30,WHITE,BLACK,"Enter new password again",FRONT_SIZE,0);
				for(uint8_t i  = 0; i < 6; i++)
				 Show_Str(110+20*i,70,WHITE,BLACK,"-",FRONT_SIZE,0);
			}
			 break;
			
		case 2: 
			password[password_num] = key_value;
			Show_Str(110+password_num*20,60,WHITE,BLACK,"*",FRONT_SIZE,0);
			if(password_num < 5)
				password_num++;
			
			if(password_num == 5 && change_sta && password[5]!= NULL)//如果输入完新密码
			{
				temp2 = password[0]*100000+password[1]*10000+password[2]*1000+password[3]*100+password[4]*10+password[5]*1;//计算新密码
				
				if(temp1 == temp2)
				{
					STMFLASH_Write(password_addr,(uint16_t *)&temp1,2);

					LCD_Clear(BLACK);
					Gui_StrCenter(20,60,WHITE,BLACK,"Change password succeed!",FRONT_SIZE,0);
					HAL_Delay(1000);
					HAL_Delay(1000);
				}
				else
				{
					LCD_Clear(BLACK);
					Gui_StrCenter(20,60,WHITE,BLACK,"The two passwords are inconsistent",FRONT_SIZE,0);
					HAL_Delay(1000);
					HAL_Delay(1000);
				}
				
				
				set_password_flag = 0;//修改功能完成
				memset(password, 0, sizeof(password));
				password_num = 0;
				change_sta = 0;
				menu_sta = 0;
				
				display_main_gui();
			}
			break;
		}
	
}
