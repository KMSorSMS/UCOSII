#include "main.h"

typedef uint32_t  u32;

extern void Clock_Init(void);
extern void LED_Init(void);
extern void LED_ON(void);
extern void LED_OFF(void);
//extern void SYS_DELAY_MS(unsigned int milliseconds);
extern void SYSTICK_INIT(void);
//extern uint32_t TIM2_CH1_Counter;
//extern uint32_t TIM4_CH3_Counter;

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}


void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	*/
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 84;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);


	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
		| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

}

void SetSystick(void){
	SysTick->CTRL &= ~(1 << 2);		//设置AHB外设时钟源为HCLK/8
	SysTick->CTRL |= (1 << 1);		//开启中断
	SysTick->LOAD = 0xA;			//设置重装载值为0xA,缩小10倍，这样HAL库的DELAY函数就是1微秒
	SysTick->VAL = 0;				//清空计数器
	SysTick->CTRL |= (1 << 0);		//开启计数器
}

void initAll(void) {
	HAL_Init();  			 		//初始化HAL库
	SystemClock_Config();			//初始化系统时钟
	MX_GPIO_Init();					//初始化GPIOA,用于控制LED
	// LED_Init();						//初始化LED
	// SYSTICK_INIT();					//初始化滴答定时器，需要微秒级延时时使用
	//设置SYSCLK,设置AHB外部时钟源，并且设置分频/8,开启中断，设置重装载值为0xA,使得每次中断是1微秒
	SetSystick();
	USART1_Init(115200);			//初始化串口1,波特率为115200
	iic_init();						//初始化IIC总线
	MPU6050_Init();					//初始化MPU6050
	PWM_TIM_Init(5000 - 1, 84 - 1); //初始化PWM
	MotorPWMInit();					//初始化电机PWM
	MS5611_Init();					//初始化MS5611
	// HMC5883L_INIT();				//初始化HMC5883L
	HMC5883L_Init();				//初始化HMC5883L
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

int main(void) {
	initAll();
	while (1) {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		HAL_Delay(1000 * 500);
		startMPU6050();
		startPWM();
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		HAL_Delay(1000 * 500);
	}
}
