#ifndef _KEY_H
#define _KEY_H

#include "stm32f1xx_hal.h"

#define FRONT_SIZE  16
#define XINGHAO_X		100
#define XINGHAO_Y		60
 
#define KEY_COL1   HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0)
#define KEY_COL2   HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1)
#define KEY_COL3   HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_2)
#define KEY_COL4   HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_3)

unsigned char key_scan();
void scan_row(char row);
void manage_key_value(unsigned char key_value);
void set_password(unsigned char key_value);
void password_to_opendoor(uint8_t key_value);
void change_password(uint8_t key_value);
#endif
