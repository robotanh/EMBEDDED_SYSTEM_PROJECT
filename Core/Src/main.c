/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "KeyPad.h"
#include "LED_Screen.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

uint32_t lcd_num=0;
volatile uint8_t SevenSegScanState = 0;
uint32_t SevenSegBuffer[3] = {123456, 654321, 987654};
uint8_t keyPressed = 0xFF;

/* Definitions for Led3x6Task */
osThreadId_t Led3x6TaskHandle;
const osThreadAttr_t Led3x6Task_attributes = {
  .name = "Led3x6Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for KeyPad4x5Task */
osThreadId_t KeyPad4x5TaskHandle;
const osThreadAttr_t KeyPad4x5Task_attributes = {
  .name = "KeyPad4x5Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
void Led3x6Run(void *argument);
void KeyPad4x5Run(void *argument);

/* USER CODE BEGIN PFP */
void ShiftOut_SPI(uint8_t *data, size_t size);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


uint8_t displayBuffer[2][5];  // Double buffer
volatile uint8_t currentBufferIndex = 0;

void ShiftOut_SPI(uint8_t *data, size_t size)
{
	HAL_GPIO_WritePin(Latch_SPI1_GPIO_Port, Latch_SPI1_Pin, GPIO_PIN_RESET); // Pull STCP (Latch) low
	HAL_GPIO_WritePin(OE_GPIO_Port, OE_Pin, GPIO_PIN_SET);
	 for (volatile int i = 0; i < 1000; i++) __NOP();
	while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
    if (HAL_SPI_Transmit(&hspi1, data, size, HAL_MAX_DELAY) != HAL_OK)
    {
    	Error_Handler();
    }
//    osDelay(10);
    for (volatile int i = 0; i < 1000; i++) __NOP();
    while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
    HAL_GPIO_WritePin(Latch_SPI1_GPIO_Port, Latch_SPI1_Pin, GPIO_PIN_SET); // Pull STCP (Latch) high
    HAL_GPIO_WritePin(OE_GPIO_Port, OE_Pin, GPIO_PIN_RESET);
}
uint8_t* SevenSegLEDsHandler(uint32_t* buffer, uint8_t scan_state) {
    static uint8_t output[3];
    switch (scan_state) {
        case 0:
            output[0] = buffer[0] % 10;
            output[1] = buffer[1] % 10;
            output[2] = buffer[2] % 10;
            break;
        case 1:
            output[0] = (buffer[0] / 10) % 10;
            output[1] = (buffer[1] / 10) % 10;
            output[2] = (buffer[2] / 10) % 10;
            break;
        case 2:
            output[0] = (buffer[0] / 100) % 10;
            output[1] = (buffer[1] / 100) % 10;
            output[2] = (buffer[2] / 100) % 10;
            break;
        case 3:
            output[0] = (buffer[0] / 1000) % 10;
            output[1] = (buffer[1] / 1000) % 10;
            output[2] = (buffer[2] / 1000) % 10;
            break;
        case 4:
            output[0] = (buffer[0] / 10000) % 10;
            output[1] = (buffer[1] / 10000) % 10;
            output[2] = (buffer[2] / 10000) % 10;
            break;
        case 5:
            output[0] = (buffer[0] / 100000) % 10;
            output[1] = (buffer[1] / 100000) % 10;
            output[2] = (buffer[2] / 100000) % 10;
            break;
    }
    return output;
}

void UpdateDisplayBuffer(uint32_t* buffer, uint8_t scan_state, uint8_t bufferIndex) {
    uint8_t* curr_digit = SevenSegLEDsHandler(buffer, scan_state);
    uint8_t curr_scan;
    switch (scan_state) {
        case 0:
            curr_scan = 0b11111110;
            break;
        case 1:
            curr_scan = 0b11111101;
            break;
        case 2:
            curr_scan = 0b11111011;
            break;
        case 3:
            curr_scan = 0b11110111;
            break;
        case 4:
            curr_scan = 0b11101111;
            break;
        case 5:
            curr_scan = 0b11011111;
            break;
        default:
            curr_scan = 0b11111111;
            break;
    }
    displayBuffer[bufferIndex][0] = 0b11111111; //skip bít
    displayBuffer[bufferIndex][1] = digitMapWithDP[curr_digit[2]];
    displayBuffer[bufferIndex][2] = curr_scan;
    displayBuffer[bufferIndex][3] = digitMapWithDP[curr_digit[1]];
    displayBuffer[bufferIndex][4] = digitMapWithDP[curr_digit[0]];
}

void SevenSegLEDsScan() {
    uint8_t bufferIndex = (currentBufferIndex + 1) % 2;
    UpdateDisplayBuffer(SevenSegBuffer, SevenSegScanState, bufferIndex);

    __disable_irq();  // Disable interrupts
    ShiftOut_SPI(displayBuffer[currentBufferIndex], 5);
    currentBufferIndex = bufferIndex;  // Swap buffers
    __enable_irq();   // Enable interrupts

    SevenSegScanState = (SevenSegScanState + 1) % 6;
}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Led3x6Task */
  Led3x6TaskHandle = osThreadNew(Led3x6Run, NULL, &Led3x6Task_attributes);

  /* creation of KeyPad4x5Task */
  KeyPad4x5TaskHandle = osThreadNew(KeyPad4x5Run, NULL, &KeyPad4x5Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Latch_SPI1_Pin|OUT0_Pin|OUT1_Pin|OUT2_Pin
                          |OUT3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OE_GPIO_Port, OE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : D0_Pin D1_Pin D2_Pin D3_Pin
                           D4_Pin */
  GPIO_InitStruct.Pin = D0_Pin|D1_Pin|D2_Pin|D3_Pin
                          |D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Latch_SPI1_Pin OUT0_Pin OUT1_Pin OUT2_Pin
                           OUT3_Pin */
  GPIO_InitStruct.Pin = Latch_SPI1_Pin|OUT0_Pin|OUT1_Pin|OUT2_Pin
                          |OUT3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : OE_Pin */
  GPIO_InitStruct.Pin = OE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OE_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_Led3x6Run */
/**
  * @brief  Function implementing the Led3x6Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Led3x6Run */
void Led3x6Run(void *argument)
{
  /* USER CODE BEGIN 5 */

  /* Infinite loop */

	uint8_t clear_buffer[5] = {0b11111111,0b11111111,0b11111111,0b11111111,0b11111111};
  for(;;)
  {
	  SevenSegLEDsScan();
	  osDelay(1);
	  ShiftOut_SPI(clear_buffer, 5);

//	  SevenSegLEDsScan();
	  osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_KeyPad4x5Run */
/**
* @brief Function implementing the KeyPad4x5Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_KeyPad4x5Run */
void KeyPad4x5Run(void *argument)
{
  /* USER CODE BEGIN KeyPad4x5Run */
  /* Infinite loop */
  for(;;)
  {
	  //////////////////////////////////////////////////TODO (1) IF USING LCD/////////////////////////////////////////////////////////
	  //////////////////////////////////////////////TODO (2) IF TESTING 3X6 LEDS//////////////////////////////////////////////////////
	  keyPressed = KeyPad_Scan();
	  if(keyPressed<10){
//			  uint32_t temp=lcd_num*10+keyPressed; //  			TODO (1) UNCOMMENT IF USING LCD
		  uint32_t temp=SevenSegBuffer[0]*10+keyPressed; //	TODO (2) UNCOMMENT IF TESTING 3X6 LEDS
		  if(temp<=99999999){
			  lcd_num=temp;
//				  Update_LCD(lcd_num); // 						TODO (1) UNCOMMENT IF USING LCD
			  SevenSegBuffer[0]=temp; //					TODO (2) UNCOMMENT IF TESTING 3X6 LEDS
		  }
	  }
	  else if(keyPressed>=10 &&keyPressed<100){
//			  lcd_num=0; //										TODO (1) UNCOMMENT IF USING LCD
//			  Update_LCD(lcd_num); // 							TODO (1) UNCOMMENT IF USING LCD
		  SevenSegBuffer[0]=0; //							TODO (2) UNCOMMENT IF TESTING 3X6 LEDS

	  }
    osDelay(1);
  }
  /* USER CODE END KeyPad4x5Run */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
