#include "lora.h"

extern UART_HandleTypeDef huart1;

void lora_send(unsigned char *pstr, int len)
{	

		HAL_UART_Transmit(&huart1, pstr, 4,10);
	 
}

void lora_get()
{
	
}