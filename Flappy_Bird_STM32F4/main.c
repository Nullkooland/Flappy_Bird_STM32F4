#include <stm32f4xx_hal.h>
#include "System\DELAY\delay.h"
#include "Hardware\LCD\lcd.h"
#include "game.h"

void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

int main(void)
{
	HAL_Init();
	System_Clock_Init(336, 8, 2, 7);
	LCD_Init();
	Game_Init();
	Draw_Toad();

	__GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.Pin = GPIO_PIN_6;

	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	for (;;)
	{
		Toad_Go();
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		HAL_Delay(50);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		HAL_Delay(50);
	}
}
