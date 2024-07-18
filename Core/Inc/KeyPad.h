/*
 * KeyPad.h
 *
 *  Created on: Jun 27, 2024
 *      Author: Admin
 */
#ifndef __KEYPAD_H
#define __KEYPAD_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "main.h"
#include <stdio.h>
extern uint8_t keyPressed;

void KeyPad_Init(void);
uint8_t KeyPad_Scan(void);
void KeyLogic();
void KeyLogic_Action();
#endif /* __KEYPAD_H */
