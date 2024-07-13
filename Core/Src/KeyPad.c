/*
 * KeyPad.c
 *
 *  Created on: Jun 27, 2024
 *      Author: Admin
 */

#include "KeyPad.h"


uint8_t keyPressed = 0xFF;
uint8_t lcd_num = 0;
//const uint8_t keyMap[4][5] = {
//    {'D', 'C', 'B', 'A', '*'},
//    {'#', '9', '6', '3', '7'},
//    {'0', '8', '5', '2', '4'},
//    {'*', '7', '4', '1', '1'}
//};

const uint8_t keyMap[4][5] = {
    {13, 12, 11, 10,100},
    {200, 9, 6, 3, 7},
    {0, 8, 5, 2, 4},
    {100, 7, 4, 1, 1}
};

#define DEBOUNCE_DELAY pdMS_TO_TICKS(500)

GPIO_InitTypeDef GPIO_InitStructPrivate = {0};
TickType_t lastDebounceTime = 0;


void KeyPad_Init(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);
}


uint8_t KeyPad_Scan(void) {
    TickType_t currentMillis = xTaskGetTickCount();

    if (currentMillis - lastDebounceTime < DEBOUNCE_DELAY) {
        return 0xFF;
    }

    for (uint8_t row = 0; row < 4; row++) {
        /* Set all rows low */
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);

        /* Set the current row high */
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 << row, GPIO_PIN_SET);

        /* Read each column */
        for (uint8_t col = 0; col < 5; col++) {
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0 << col) == GPIO_PIN_SET) {
                lastDebounceTime = currentMillis;
                return keyMap[row][col];
            }
        }
    }

    return 0xFF;
}

void KeyLogic(){
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
}

