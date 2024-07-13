/*
 * SPI_shift.c
 *
 *  Created on: Jul 13, 2024
 *      Author: Admin
 */

#include "SPI_shift.h"
extern SPI_HandleTypeDef hspi1;
void ShiftOut_SPI(uint8_t *data, size_t size)
{
	HAL_GPIO_WritePin(Latch_SPI1_GPIO_Port, Latch_SPI1_Pin, GPIO_PIN_RESET); // Pull STCP (Latch) low
	HAL_GPIO_WritePin(OE_GPIO_Port, OE_Pin, GPIO_PIN_SET);
	 for (volatile int i = 0; i < 1000; i++) __NOP();
//	osDelay(1);
	while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
    if (HAL_SPI_Transmit(&hspi1, data, size, HAL_MAX_DELAY) != HAL_OK)
    {
    	Error_Handler();
    }
//    osDelay(1);
    for (volatile int i = 0; i < 1000; i++) __NOP();
    while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
    HAL_GPIO_WritePin(Latch_SPI1_GPIO_Port, Latch_SPI1_Pin, GPIO_PIN_SET); // Pull STCP (Latch) high
    HAL_GPIO_WritePin(OE_GPIO_Port, OE_Pin, GPIO_PIN_RESET);

}
