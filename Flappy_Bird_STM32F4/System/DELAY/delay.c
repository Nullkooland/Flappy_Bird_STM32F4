#include"delay.h"

void System_Clock_Init(uint32_t PLL_N, uint32_t PLL_M, uint32_t PLL_P, uint32_t PLL_Q)
{
	HAL_StatusTypeDef ret = HAL_OK;
	RCC_OscInitTypeDef RCC_OscInitStructure;
	RCC_ClkInitTypeDef RCC_ClkInitStructure;

	__HAL_RCC_PWR_CLK_ENABLE(); //使能PWR时钟

								//下面这个设置用来设置调压器输出电压级别，以便在器件未以最大频率工作
								//时使性能与功耗实现平衡，此功能只有STM32F42xx和STM32F43xx器件有，
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);//设置调压器输出电压级别1

	RCC_OscInitStructure.OscillatorType = RCC_OSCILLATORTYPE_HSE;    //时钟源为HSE
	RCC_OscInitStructure.HSEState = RCC_HSE_ON;                      //打开HSE
	RCC_OscInitStructure.PLL.PLLState = RCC_PLL_ON;//打开PLL
	RCC_OscInitStructure.PLL.PLLSource = RCC_PLLSOURCE_HSE;//PLL时钟源选择HSE
	RCC_OscInitStructure.PLL.PLLN = PLL_N; //主PLL倍频系数(PLL倍频),取值范围:64~432.  
	RCC_OscInitStructure.PLL.PLLM = PLL_M; //主PLL和音频PLL分频系数(PLL之前的分频),取值范围:2~63.
	RCC_OscInitStructure.PLL.PLLP = PLL_P; //系统时钟的主PLL分频系数(PLL之后的分频),取值范围:2,4,6,8.(仅限这4个值!)
	RCC_OscInitStructure.PLL.PLLQ = PLL_Q; //USB/SDIO/随机数产生器等的主PLL分频系数(PLL之后的分频),取值范围:2~15.
	ret = HAL_RCC_OscConfig(&RCC_OscInitStructure);//初始化

	if (ret != HAL_OK) while (1);

	//选中PLL作为系统时钟源并且配置HCLK,PCLK1和PCLK2
	RCC_ClkInitStructure.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStructure.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;//设置系统时钟时钟源为PLL
	RCC_ClkInitStructure.AHBCLKDivider = RCC_SYSCLK_DIV1;//AHB分频系数为1
	RCC_ClkInitStructure.APB1CLKDivider = RCC_HCLK_DIV4; //APB1分频系数为4
	RCC_ClkInitStructure.APB2CLKDivider = RCC_HCLK_DIV2; //APB2分频系数为2
	ret = HAL_RCC_ClockConfig(&RCC_ClkInitStructure, FLASH_LATENCY_4);//同时设置FLASH延时周期为5WS，也就是6个CPU周期。
//	printf("%u\r\n", HAL_RCC_GetHCLKFreq());
	
	if (ret != HAL_OK) while (1);
}