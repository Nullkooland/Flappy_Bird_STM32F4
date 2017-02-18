#pragma once
#include <stm32f4xx_hal.h>
#include "Hardware\LCD\lcd.h"

#define SCREEN_TOUCHED !HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_5)
#define DEFAULT_TOAD_POS_X 32
#define DEFAULT_TOAD_POS_Y 180
#define TUBE_WIDTH 32

void Game_Init(void);
void Toad_Go(void);

void Tube_Move(void);
void Game_Over(void);