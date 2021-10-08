#ifndef __LCD_H
#define __LCD_H

#include "stm32f1xx_hal.h"
//LCD��Ҫ������
typedef struct  
{										    
	uint16_t width;			//LCD ����
	uint16_t height;			//LCD �߶�
	uint16_t id;				  //LCD ID
	uint8_t  dir;			  //���������������ƣ�0��������1��������	
	uint16_t	 wramcmd;		//��ʼдgramָ��
	uint16_t  setxcmd;		//����x����ָ��
	uint16_t  setycmd;		//����y����ָ��	 
}_lcd_dev; 	

//LCD����
extern _lcd_dev lcddev;	//����LCD��Ҫ����
/////////////////////////////////////�û�������///////////////////////////////////	 
#define USE_HORIZONTAL  	 1//����Һ����˳ʱ����ת���� 	0-0����ת��1-90����ת��2-180����ת��3-270����ת

//////////////////////////////////////////////////////////////////////////////////	  
//����LCD�ĳߴ�
#define LCD_W 240
#define LCD_H 320

//TFTLCD������Ҫ���õĺ���		   
extern uint16_t  POINT_COLOR;//Ĭ�Ϻ�ɫ    
extern uint16_t  BACK_COLOR; //������ɫ.Ĭ��Ϊ��ɫ

////////////////////////////////////////////////////////////////////
//-----------------LCD�˿ڶ���---------------- 
#define LCD_CS   6      	 //Ƭѡ����            PC6
#define LCD_RS   7      	 //�Ĵ���/����ѡ������ PC7 
#define LCD_RST  5        //��λ����            PC5
	
//���ʹ�ùٷ��⺯���������еײ㣬�ٶȽ����½���14֡ÿ�룬���������˾�Ƽ�����
//����IO����ֱ�Ӳ����Ĵ���������IO������ˢ�����ʿ��Դﵽ28֡ÿ�룡 

//GPIO��λ�����ߣ�
#define	LCD_CS_SET  GPIOC->BSRR=1<<LCD_CS    //Ƭѡ�˿�  	 PC6
#define	LCD_RS_SET	GPIOC->BSRR=1<<LCD_RS    //����/����  	 PC7	  
#define	LCD_RST_SET	GPIOC->BSRR=1<<LCD_RST   //��λ			   PC5

//GPIO��λ�����ͣ�							    	
#define	LCD_CS_CLR  GPIOC->BRR=1<<LCD_CS     //Ƭѡ�˿�  		PC6
#define	LCD_RS_CLR	GPIOC->BRR=1<<LCD_RS     //����/���� 	  PC7 
#define	LCD_RST_CLR	GPIOC->BRR=1<<LCD_RST    //��λ			 	  PC5

//������ɫ
#define WHITE       0xFFFF
#define BLACK      	0x0000	  
#define BLUE       	0x001F  //��ɫ
#define BRED        0XF81F  //����ɫ
#define GRED 			 	0XFFE0  //��ɫ
#define GBLUE			 	0X07FF  //��ɫ
#define RED         0xF800 //��ɫ
#define MAGENTA     0xF81F //����ɫ
#define GREEN       0x07E0 //��ɫ
#define CYAN        0x7FFF //��ɫ
#define YELLOW      0xFFE0 //��ɫ
#define BROWN 			0XBC40 //��ɫ
#define BRRED 			0XFC07 //����ɫ
#define GRAY  			0X8430 //��ɫ  ����ɫ
//GUI��ɫ

#define DARKBLUE      	 0X01CF	//����ɫ
#define LIGHTBLUE      	 0X7D7C	//ǳ��ɫ  
#define GRAYBLUE       	 0X5458 //����ɫ
//������ɫΪPANEL����ɫ 
 
#define LIGHTGREEN     	0X841F //ǳ��ɫ
#define LIGHTGRAY     0XEF5B //ǳ��ɫ(PANNEL)
#define LGRAY 			 		0XC618 //ǳ��ɫ(PANNEL),���屳��ɫ

#define LGRAYBLUE      	0XA651 //ǳ����ɫ(�м����ɫ)
#define LBBLUE          0X2B12 //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)
	    															  
void LCD_Init(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Clear(uint16_t Color);	 
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void LCD_DrawPoint(uint16_t x,uint16_t y);//����
uint16_t  LCD_ReadPoint(uint16_t x,uint16_t y); //����
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);		   
void LCD_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd);

uint16_t LCD_RD_DATA(void);//��ȡLCD����									    
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue);
void LCD_WR_DATA(uint8_t data);
uint16_t LCD_ReadReg(uint8_t LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(uint16_t RGB_Code);
uint16_t LCD_ReadRAM(void);		   
uint16_t LCD_BGR2RGB(uint16_t c);
void LCD_SetParam(void);
void Lcd_WriteData_16Bit(uint16_t Data);
void LCD_direction(uint8_t direction );


#endif