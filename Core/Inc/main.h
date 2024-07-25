/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//extern uint32_t SevenSegBuffer[3];
extern char SevenSegBuffer[3][7];
extern uint32_t password;
extern long unsigned int totalLiters;		//Total liters from beginning
extern long unsigned int totalLitersShift;	//Total liter per shift, ( > 1000000 return 0)

extern uint32_t currPrice;
extern uint32_t roundedPrice;

extern uint32_t orderPrice;
extern double orderLiter;

 extern int LEDPointFlag; // 0 -> 5 to choose which LED has point
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define D0_Pin GPIO_PIN_0
#define D0_GPIO_Port GPIOB
#define D1_Pin GPIO_PIN_1
#define D1_GPIO_Port GPIOB
#define D2_Pin GPIO_PIN_2
#define D2_GPIO_Port GPIOB
#define Latch_SPI1_Pin GPIO_PIN_14
#define Latch_SPI1_GPIO_Port GPIOB
#define OE_Pin GPIO_PIN_7
#define OE_GPIO_Port GPIOC
#define D3_Pin GPIO_PIN_3
#define D3_GPIO_Port GPIOB
#define D4_Pin GPIO_PIN_4
#define D4_GPIO_Port GPIOB
#define OUT0_Pin GPIO_PIN_6
#define OUT0_GPIO_Port GPIOB
#define OUT1_Pin GPIO_PIN_7
#define OUT1_GPIO_Port GPIOB
#define OUT2_Pin GPIO_PIN_8
#define OUT2_GPIO_Port GPIOB
#define OUT3_Pin GPIO_PIN_9
#define OUT3_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
