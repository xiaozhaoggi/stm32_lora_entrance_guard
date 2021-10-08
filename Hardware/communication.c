/***************************************************************
Copyright  SIEE OF HUE All rights reserved.
文件名		: communication.c
作者	  	: 张博钊
版本	   	: V1.0
描述	   	: 串口通讯、操作外围电路
其他	   	: 无
电话 	   	: 13229321127
日志	   	: 初版V1.0 2021/4/23 张博钊创建
***************************************************************/


#include "communication.h"
#include "stmflash.h"
#include "string.h"
#include "lcd.h"
#include "gui.h"
#include "key.h"
extern UART_HandleTypeDef huart1;
extern RTC_HandleTypeDef hrtc;
extern uint32_t writeFlashData;
extern uint32_t addr;
extern uint8_t net_id;
extern RTC_TimeTypeDef Time;
extern RTC_DateTypeDef Date;
extern const unsigned char gImage_siee[3200];		
unsigned char rc522_cardid[4];
unsigned char rec_done_flag;
unsigned char adjust_time_flag;
unsigned char crc_buf[20];
LORA_COM lora_com;

int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1);
  return ch;
}

uint8_t rc522_work(unsigned int *car_num)
{
	char status;
	unsigned char tag_type[2];

	status = PcdRequest(0x52,tag_type);
	if(status == 0)
	{
		PcdAnticoll(rc522_cardid);
		*car_num =(rc522_cardid[0]<<24 | rc522_cardid[1]<<16 | rc522_cardid[2]<<8 | rc522_cardid[3]);
		return 1;
	}
	return 0;
}

/*

	下位机在读到卡号的时候主动向上位机发送
	status：0 未开门  1：开门

																					*/
void send_cardid(unsigned char status)
{
	uint8_t buf[13];
	unsigned int crc16;
	
	buf[0] = SOF;
	buf[1] = 0x00;
	buf[2] = net_id;
	buf[3] = READING_CARD_LEN;
	buf[4] = READING_CARD;
	buf[5] = status;
	buf[6] = rc522_cardid[0];
	buf[7] = rc522_cardid[1];
	buf[8] = rc522_cardid[2];
	buf[9] = rc522_cardid[3];
	
	for(uint8_t i = 1; i < READING_CARD_LEN-1; i++)
		crc_buf[i-1] = buf[i];
	
	crc16 = CRC_Check(crc_buf, READING_CARD_LEN-2);	
	
	buf[10] = crc16 >> 8;
	buf[11] = crc16 & 0xff;
	buf[12] = EOFF;
	
	for(uint8_t i = 0; i< 13; i++)
		printf("%c",buf[i]);
}

/*

	下位机在输入密码成功开门的时候主动向上位机发送

																									*/
void send_password(unsigned int password)
{
	uint8_t buf[12];
	unsigned int crc16;
	
	buf[0] = SOF;
	buf[1] = 0x00;
	buf[2] = net_id;
	buf[3] = SEND_PASSWORD_LEN;
	buf[4] = SEND_PASSWORD;
	buf[5] = password>>24;
	buf[6] = (password>>16)&0xff;
	buf[7] = (password>>8)&0xff;
	buf[8] = password&0xff;
	
	for(uint8_t i = 1; i < SEND_PASSWORD_LEN-1; i++)
		crc_buf[i-1] = buf[i];
	
	crc16 = CRC_Check(crc_buf, SEND_PASSWORD_LEN-2);	
	
	buf[9] = crc16 >> 8;
	buf[10] = crc16 & 0xff;
	buf[11] = EOFF;
	
	for(uint8_t i = 0; i< 12; i++)
		printf("%c",buf[i]);
		
	data_led();//发送数据指示灯
}

/*

			lora命令处理函数

															*/
void lora_cmd_judge(LORA_COM *lora_com)
{
	LORA_COM *pcom = lora_com;
	uint8_t ret,i;
	uint32_t writeFlashData,addr;
	uint16_t crc16;
	uint32_t carid;
	
	crc16 = CRC_Check(crc_buf, pcom->len-2);	//先进行crc校验
	pcom->crc_s[0] = crc16 >> 8;
	pcom->crc_s[1] = crc16 & 0xff;
	if((pcom->crc_s[0] == pcom->crc_m[0]) && (pcom->crc_s[1] == pcom->crc_m[1]))
		pcom->status = SUCCEED;
	else 
		pcom->status = CRCERROR;	
	
	memset(crc_buf, 0, 20);	
	
	switch(pcom->cmd)
	{
		case OPEN_DOOR:
			
			crc_buf[0] = pcom->sou;
			crc_buf[1] = net_id;
			crc_buf[2] = OPEN_DOOR_LEN;
			crc_buf[3] = pcom->cmd;
			crc_buf[4] = pcom->status;
			crc16 = CRC_Check(crc_buf,5);
			pcom->crc_s[0] = crc16 >> 8;
			pcom->crc_s[1] = crc16 & 0xff;
		
			printf("%c",SOF);
			printf("%c",pcom->sou);//sou为主机地址即目标地址
			printf("%c",net_id);//des为下位机地址即源地址
			printf("%c",OPEN_DOOR_LEN);
			printf("%c",pcom->cmd);
			printf("%c",pcom->status);
			printf("%c",pcom->crc_s[0]);
			printf("%c",pcom->crc_s[1]);
			printf("%c",EOFF);
		
			if(pcom->status == SUCCEED) //如果奇偶校验正确
			lock_control(1);
		
			break;
		
		case MOD_ADDRESS:
			
			if(pcom->status == SUCCEED)
			{				
				net_id = pcom->payload[0];
				writeFlashData = net_id;
				addr = NET_FLASH_ADDR;
				STMFLASH_Write(addr,(uint16_t *)&writeFlashData,1);
			}
			
			crc_buf[0] = pcom->sou;
			crc_buf[1] = net_id;
			crc_buf[2] = MOD_ADDRESS_LEN;
			crc_buf[3] = pcom->cmd;
			crc_buf[4] = pcom->status;
			crc16 = CRC_Check(crc_buf,5);
			pcom->crc_s[0] = crc16 >> 8;
			pcom->crc_s[1] = crc16 & 0xff;
		
			printf("%c",SOF);
			printf("%c",pcom->sou);//sou为主机地址即目标地址
			printf("%c",pcom->payload[0]);//发送修改后新的地址
			printf("%c",MOD_ADDRESS_LEN);
			printf("%c",pcom->cmd);
			printf("%c",pcom->status);
			printf("%c",pcom->crc_s[0]);
			printf("%c",pcom->crc_s[1]);
			printf("%c",EOFF);
		
			break;
		
		case GET_ADDRESS:
			
			if(pcom->status == SUCCEED)
			{
				net_id = read_netid()&0xff;
				
				crc_buf[0] = pcom->sou;
				crc_buf[1] = net_id;
				crc_buf[2] = GET_ADDRESS_LEN;
				crc_buf[3] = pcom->cmd;
				crc_buf[4] = pcom->status;
				crc_buf[5] = net_id;
				crc16 = CRC_Check(crc_buf,6);
				pcom->crc_s[0] = crc16 >> 8;
				pcom->crc_s[1] = crc16 & 0xff;
			
				printf("%c",SOF);
				printf("%c",pcom->sou);//sou为主机地址即目标地址
				printf("%c",net_id);//发送修改后新的地址
				printf("%c",GET_ADDRESS_LEN);
				printf("%c",pcom->cmd);
				printf("%c",pcom->status);
				printf("%c",net_id);
				printf("%c",pcom->crc_s[0]);
				printf("%c",pcom->crc_s[1]);
				printf("%c",EOFF);
		}
			break;
		
		case ADD_CARDID:
			
			if(pcom->status == SUCCEED)
			{
				for(i = pcom->len-6; i > 0; i-=4)//组成32位cardid 并进行写入处理
				{
					writeFlashData = (pcom->payload[i-4]<<24) | (pcom->payload[i-3]<<16) | (pcom->payload[i-2]<<8) | (pcom->payload[i-1]);
					if(search_cardid(writeFlashData))//如果已经有这个卡号 不再写
						continue;
					addr = insert_cardid_addr();
					if(addr)//如果有空余地址
						STMFLASH_Write(addr,(uint16_t *)&writeFlashData,2);
					else
						pcom->status = OTHERERROR;
				}
				
			}
				
			crc_buf[0] = pcom->sou;
			crc_buf[1] = net_id;
			crc_buf[2] = ADD_CARDID_LEN;
			crc_buf[3] = pcom->cmd;
			crc_buf[4] = pcom->status;
			crc16 = CRC_Check(crc_buf,5);
			pcom->crc_s[0] = crc16 >> 8;
			pcom->crc_s[1] = crc16 & 0xff;
			
			printf("%c",SOF);
			printf("%c",pcom->sou);//sou为主机地址即目标地址
			printf("%c",net_id);//des为下位机地址即源地址
			printf("%c",ADD_CARDID_LEN);
			printf("%c",pcom->cmd);
			printf("%c",pcom->status);
			printf("%c",pcom->crc_s[0]);
			printf("%c",pcom->crc_s[1]);
			printf("%c",EOFF);
			
			break;
		
		case DEL_CARDID:
			
			if(pcom->status == SUCCEED)
			{
					for(i = pcom->len-6; i > 0; i-=4)//组成32位cardid 并进行擦除处理
				{
					carid = (pcom->payload[i-4]<<24) | (pcom->payload[i-3]<<16) | (pcom->payload[i-2]<<8) | (pcom->payload[i-1]);
					if(search_cardid(carid))//如果有这个卡号 擦除
						addr = search_cardid_addr(carid);
					else
						continue;
					writeFlashData = NULL_FLASH;
					STMFLASH_Write(addr,(uint16_t *)&writeFlashData,2);
				}	
			}
			
			
			
			crc_buf[0] = pcom->sou;
			crc_buf[1] = net_id;
			crc_buf[2] = DEL_CARDID_LEN;
			crc_buf[3] = pcom->cmd;
			crc_buf[4] = pcom->status;
			crc16 = CRC_Check(crc_buf,5);
			pcom->crc_s[0] = crc16 >> 8;
			pcom->crc_s[1] = crc16 & 0xff;
			
			printf("%c",SOF);
			printf("%c",pcom->sou);//sou为主机地址即目标地址
			printf("%c",net_id);//des为下位机地址即源地址
			printf("%c",DEL_CARDID_LEN);
			printf("%c",pcom->cmd);
			printf("%c",pcom->status);
			printf("%c",pcom->crc_s[0]);
			printf("%c",pcom->crc_s[1]);
			printf("%c",EOFF);
			
			break;
		
		case CLR_CARDID:
			
			if(pcom->status == SUCCEED)
			clear_all_cardid();
			
			crc_buf[0] = pcom->sou;
			crc_buf[1] = net_id;
			crc_buf[2] = CLR_CARDID_LEN;
			crc_buf[3] = pcom->cmd;
			crc_buf[4] = pcom->status;
			crc16 = CRC_Check(crc_buf,5);
			pcom->crc_s[0] = crc16 >> 8;
			pcom->crc_s[1] = crc16 & 0xff;
			
			printf("%c",SOF);
			printf("%c",pcom->sou);//sou为主机地址即目标地址
			printf("%c",net_id);//des为下位机地址即源地址
			printf("%c",CLR_CARDID_LEN);
			printf("%c",pcom->cmd);
			printf("%c",pcom->status);
			printf("%c",pcom->crc_s[0]);
			printf("%c",pcom->crc_s[1]);
			printf("%c",EOFF);
		
			break;
			
		case ADD_PASSWORD:
			
			if(pcom->status == SUCCEED)
			{
				for(i = pcom->len-6; i > 0; i-=4)//组成32位密码 并进行写入处理
				{
					writeFlashData = (pcom->payload[i-4]<<24) | (pcom->payload[i-3]<<16) | (pcom->payload[i-2]<<8) | (pcom->payload[i-1]);
					if(search_password(writeFlashData))//如果已经有这个密码 不再写
						continue;
					addr = insert_password_addr();
					if(addr)//如果有空余地址
						STMFLASH_Write(addr,(uint16_t *)&writeFlashData,2);
					else
						pcom->status = OTHERERROR;
				}
				
			}
				
			crc_buf[0] = pcom->sou;
			crc_buf[1] = net_id;
			crc_buf[2] = ADD_PASSWORD_LEN;
			crc_buf[3] = pcom->cmd;
			crc_buf[4] = pcom->status;
			crc16 = CRC_Check(crc_buf,5);
			pcom->crc_s[0] = crc16 >> 8;
			pcom->crc_s[1] = crc16 & 0xff;
			
			printf("%c",SOF);
			printf("%c",pcom->sou);//sou为主机地址即目标地址
			printf("%c",net_id);//des为下位机地址即源地址
			printf("%c",ADD_PASSWORD_LEN);
			printf("%c",pcom->cmd);
			printf("%c",pcom->status);
			printf("%c",pcom->crc_s[0]);
			printf("%c",pcom->crc_s[1]);
			printf("%c",EOFF);
			
			break;
			
			case ADJUST_DATA:
				
				if(pcom->status == SUCCEED)
				{
					Time.Hours = pcom->payload[0];
					Time.Minutes = pcom->payload[1];
					Time.Seconds = pcom->payload[2];
					Date.Year = pcom->payload[3];
					Date.Month = pcom->payload[4];
					Date.Date = pcom->payload[5];
					Date.WeekDay = pcom->payload[6];
					HAL_RTC_SetTime(&hrtc, &Time, RTC_FORMAT_BCD);
					HAL_RTC_SetDate(&hrtc, &Date, RTC_FORMAT_BCD);
					adjust_time_flag = 1;
				}
				
				crc_buf[0] = pcom->sou;
				crc_buf[1] = net_id;
				crc_buf[2] = ADJUST_DATA_LEN;
				crc_buf[3] = pcom->cmd;
				crc_buf[4] = pcom->status;
				crc16 = CRC_Check(crc_buf,5);
				pcom->crc_s[0] = crc16 >> 8;
				pcom->crc_s[1] = crc16 & 0xff;
				
				printf("%c",SOF);
				printf("%c",pcom->sou);//sou为主机地址即目标地址
				printf("%c",net_id);//des为下位机地址即源地址
				printf("%c",ADJUST_DATA_LEN);
				printf("%c",pcom->cmd);
				printf("%c",pcom->status);
				printf("%c",pcom->crc_s[0]);
				printf("%c",pcom->crc_s[1]);
				printf("%c",EOFF);
				
		case CHANGE_PASSWORD:
			
				if(pcom->status == SUCCEED)
				{
				
					writeFlashData = (pcom->payload[0]<<24) | (pcom->payload[1]<<16) | (pcom->payload[2]<<8) | (pcom->payload[3]);
					addr = search_password_addr(writeFlashData);
					if(addr)//如果有这个密码
					{
						writeFlashData = (pcom->payload[4]<<24) | (pcom->payload[5]<<16) | (pcom->payload[6]<<8) | (pcom->payload[7]);
						STMFLASH_Write(addr,(uint16_t *)&writeFlashData,2);
					}
					else
						pcom->status = OTHERERROR;
				
				}
				
			crc_buf[0] = pcom->sou;
			crc_buf[1] = net_id;
			crc_buf[2] = CHANGE_PASSWORD_LEN;
			crc_buf[3] = pcom->cmd;
			crc_buf[4] = pcom->status;
			crc16 = CRC_Check(crc_buf,5);
			pcom->crc_s[0] = crc16 >> 8;
			pcom->crc_s[1] = crc16 & 0xff;
			
			printf("%c",SOF);
			printf("%c",pcom->sou);//sou为主机地址即目标地址
			printf("%c",net_id);//des为下位机地址即源地址
			printf("%c",CHANGE_PASSWORD_LEN);
			printf("%c",pcom->cmd);
			printf("%c",pcom->status);
			printf("%c",pcom->crc_s[0]);
			printf("%c",pcom->crc_s[1]);
			printf("%c",EOFF);
			
			break;
		
			
		default:
			break;
	}
	memset(pcom, 0, sizeof(LORA_COM));
	memset(crc_buf, 0, 20);
	data_led();
	
}
/*

			门锁控制函数

															*/
unsigned char lock_control(unsigned char control)//0关 1开
{
	if(control == 0)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
	}
	if(control == 1)
	{
		beep(1);//1：响一声
		lcd_switch(1);//亮屏
		LCD_Clear(BLACK);
		Gui_StrCenter(0,60,WHITE,BLACK,"Door open",FRONT_SIZE,0);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
		HAL_Delay(1000);
		HAL_Delay(1000);
		HAL_Delay(1000);
		display_main_gui();
		lock_control(0);
	}
	
	return 1;	
	
}

/*

			lora接收中断处理函数

																*/
void lora_irq(unsigned char res, LORA_COM *lora_com)
{
	static unsigned char index = 0, payload_index = 0;
	LORA_COM *pcom = lora_com;
	switch(index)
	{
		case 0:
			if(res == SOF)
				index++;
			else
				index = 0;
			break;
			
		case 1://DES
			pcom->des = res;
			crc_buf[0] = res;
			index++;
		break;
		
		case 2://SOU
			pcom->sou = res;
			crc_buf[1] = res;
			index++;
		break;
		
		case 3://LEN
			pcom->len = res;
			crc_buf[2] = res;
			index++;
			if(pcom->len < 6)
			index = 0;	
		break;
		
		case 4://CMD
			pcom->cmd = res;
			crc_buf[3] = res;
			index++;
			payload_index = 0;
			if(pcom->len == 6)
			index++;	
		break;
			
		case 5://PAYLOAD
			pcom->payload[payload_index] = res;
			crc_buf[4+payload_index] = res;
			payload_index++;
			if(payload_index > (pcom->len - 7))
			{
				index++;
				payload_index = 0;
			}
		break;
		
		case 6://CRC16 1
			pcom->crc_m[0] = res;
			index++;
		break;

		case 7://CRC16 2
			pcom->crc_m[1] = res;
			index++;
		break;
		
		case 8:		
			
		if(res == EOFF)
			rec_done_flag = 1;
		
			index = 0;
				
	}
}

void data_led()
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
	HAL_Delay(200);			
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);//数据指示灯
}

void display_main_gui(void)
{
		LCD_Clear(BLACK);
	//Show_Str(64,140,MAGENTA,BLACK, "信号与系统实验室",24,0);// 0  2
	//Show_Str(28,140,MAGENTA,BLACK, "通信与信息技术实验中心",24,0);// 1  2
	//	Show_Str(76,140,MAGENTA,BLACK, "通信原理实验室",24,0);// 2  2
//	my_show_24str(16, 140,MAGENTA,BLACK, "RFID",0);
//	Show_Str(112,140,MAGENTA,BLACK, "原理与应用实验室",24,0);//4  19 2
	//Show_Str(52,140,MAGENTA,BLACK, "高频电子线路实验室",24,0);//5  2
	//Show_Str(76,140,MAGENTA,BLACK, "移动通信实验室",24,0);// 6  2
	//Show_Str(52,140,MAGENTA,BLACK, "现代通信技术实验室",24,0);//7  2

	//Show_Str(64,140,MAGENTA,BLACK, "多媒体技术实验室",24,0);// 9  2
	//Show_Str(76,140,MAGENTA,BLACK, "综合布线实验室",24,0);// 11  2
	//Show_Str(28,140,MAGENTA,BLACK, "智能控制综合应用实验室",24,0);//12  2
	
	//Show_Str(76,140,MAGENTA,BLACK, "光纤通信实验室",24,0);// 13  2
	//Show_Str(40,140,MAGENTA,BLACK, "物联网与传感网实验室",24,0);//14  2

	//Show_Str(52,140,MAGENTA,BLACK, "物联网应用体验中心",24,0);//15  2
	//Show_Str(52,140,MAGENTA,BLACK, "数据通信网络实验室",24,0);//16  2
	//Show_Str(16,140,MAGENTA,BLACK, "程控交换与宽带接入实验室",24,0);//17  2
	//Show_Str(88,140,MAGENTA,BLACK, "数据中心机房",24,0);//18  2
	//Show_Str(100,140,MAGENTA,BLACK, "专家工作室",24,0);//255 2
	//Show_Str(112,140,MAGENTA,BLACK, "教工之家",24,0);//254 2
	//Show_Str(40,140,MAGENTA,BLACK, "教师第三党支部活动室",24,0);//253 2
	//Show_Str(40,140,MAGENTA,BLACK, "教师第五党支部活动室",24,0);//252 2

	//Show_Str(40,140,MAGENTA,BLACK, "教师第一党支部活动室",24,0);//251 2
	//Show_Str(40,140,MAGENTA,BLACK, "教师第二党支部活动室",24,0);//250 2
	//Show_Str(40,140,MAGENTA,BLACK, "教师第四党支部活动室",24,0);//249 2
	//Show_Str(40,140,MAGENTA,BLACK, "教师第六党支部活动室",24,0);//248 1
	//Show_Str(100,140,MAGENTA,BLACK, "党员档案室",24,0);//247 2
	//Show_Str(112,140,MAGENTA,BLACK, "智慧教室",24,0);//246 2

	
	Gui_Drawbmp16(0,0,gImage_siee);
	Show_Str(88,185,WHITE,BLACK, "信息与电气工程学院",16,0);
}

void display_week(uint8_t week)
{
	switch(week)
	{
		case 0:
			Show_Str(262,220,WHITE,BLACK,"星期日",16,0);
			break;
		
		case 1:
			Show_Str(262,220,WHITE,BLACK,"星期一",16,0);
			break;
		
		case 2:
			Show_Str(262,220,WHITE,BLACK,"星期二",16,0);
			break;
		
		case 3:
			Show_Str(262,220,WHITE,BLACK,"星期三",16,0);
			break;
		
		case 4:
			Show_Str(262,220,WHITE,BLACK,"星期四",16,0);
			break;
		
		case 5:
			Show_Str(262,220,WHITE,BLACK,"星期五",16,0);
			break;
		
		case 6:
			Show_Str(262,220,WHITE,BLACK,"星期六",16,0);
			break;
	}
		
}
/*
mode: 1 响一声
			0	连续响三声
										*/
void beep(uint8_t mode)
{
	if(mode)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_Delay(200);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
	}
}

/*
		LCD开关
		mode : 1： 开  0：关
										*/
void lcd_switch(uint8_t mode)
{
	if(mode)
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
}
//CRC校验计算 MSB First
unsigned int CRC_Check(const unsigned char *buff,unsigned int length)
{
	unsigned int n;
	unsigned int tmp=0xffff;
	unsigned int ret;
	unsigned char i;
	for(n = 0; n < length; n++)
	{
        tmp = buff[n] ^ tmp;
        for(i = 0;i < 8;i++)
				{
            if(tmp & 0x01){
                tmp = tmp >> 1;
                tmp = tmp ^ MULOR;
            }   
            else{
                tmp = tmp >> 1;
            }   
        }   
    }   
    ret = tmp >> 8;
    ret = ret | (tmp << 8);
		return ret;
}

