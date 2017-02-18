#pragma once
#include <stm32f4xx_hal.h>

#define Touched !HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_5)

extern int8_t toad_SpeedY;
void Touch_Init(void);