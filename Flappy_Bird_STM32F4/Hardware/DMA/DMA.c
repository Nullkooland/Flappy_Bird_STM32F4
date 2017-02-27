#include "DMA.h"

DMA_HandleTypeDef hdma2_fsmc;

void DMA2_FSMC_Init(void)
{
	__HAL_RCC_DMA2_CLK_ENABLE();
	HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

	hdma2_fsmc.Instance = DMA2_Stream1;
	hdma2_fsmc.Init.Channel = DMA_CHANNEL_4;
}

