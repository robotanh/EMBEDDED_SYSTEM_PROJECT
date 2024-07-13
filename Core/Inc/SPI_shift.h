/*
 * SPI_shift.h
 *
 *  Created on: Jul 13, 2024
 *      Author: Admin
 */

#ifndef INC_SPI_SHIFT_H_
#define INC_SPI_SHIFT_H_

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include "main.h"

void ShiftOut_SPI(uint8_t *data, size_t size);

#endif /* INC_SPI_SHIFT_H_ */
