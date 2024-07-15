#include "KeyPad.h"

uint8_t keyPressed = 0xFF;
uint8_t lcd_num = 0;

const uint8_t keyMap[4][5] = {
    {'A', 'B', 'C', 'D', 'E'},
    {'F', '9', '6', '3', 'G'},
    {'0', '8', '5', '2', 'H'},
    {'I', '7', '4', '1', 'J'}
};

#define DEBOUNCE_DELAY pdMS_TO_TICKS(300)

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

void KeyLogic() {
    keyPressed = KeyPad_Scan();

    switch (keyPressed) {
        case '0':
            SevenSegBuffer[0] = 1;
            SevenSegBuffer[1] = 0;
            SevenSegBuffer[2] = 0;
            break;
        case '1':
            SevenSegBuffer[0] = 10;
            SevenSegBuffer[1] = 0;
            SevenSegBuffer[2] = 0;
            break;
        case '2':
            SevenSegBuffer[0] = 100;
            SevenSegBuffer[1] = 0;
            SevenSegBuffer[2] = 0;
            break;
        case '3':
            SevenSegBuffer[0] = 1000;
            SevenSegBuffer[1] = 0;
            SevenSegBuffer[2] = 0;
            break;
        case '4':
            SevenSegBuffer[0] = 10000;
            SevenSegBuffer[1] = 0;
            SevenSegBuffer[2] = 0;
            break;
        case '5':
            SevenSegBuffer[0] = 100000;
            SevenSegBuffer[1] = 0;
            SevenSegBuffer[2] = 0;
            break;
        case '6':
            SevenSegBuffer[0] = 0;
            SevenSegBuffer[1] = 1;
            SevenSegBuffer[2] = 0;
            break;
        case '7':
            SevenSegBuffer[0] = 0;
            SevenSegBuffer[1] = 10;
            SevenSegBuffer[2] = 0;
            break;
        case '8':
            SevenSegBuffer[0] = 0;
            SevenSegBuffer[1] = 100;
            SevenSegBuffer[2] = 0;
            break;
        case '9':
            SevenSegBuffer[0] = 0;
            SevenSegBuffer[1] = 1000;
            SevenSegBuffer[2] = 0;
            break;
        case 'A':
            SevenSegBuffer[0] = 0;
            SevenSegBuffer[1] = 10000;
            SevenSegBuffer[2] = 0;
            break;
        case 'B':
            SevenSegBuffer[0] = 0;
            SevenSegBuffer[1] = 100000;
            SevenSegBuffer[2] = 0;
            break;
        case 'C':
            SevenSegBuffer[0] = 0;
            SevenSegBuffer[1] = 0;
            SevenSegBuffer[2] = 1;
            break;
        case 'D':
            SevenSegBuffer[0] = 0;
            SevenSegBuffer[1] = 0;
            SevenSegBuffer[2] = 10;
            break;
        case 'E':
            SevenSegBuffer[0] = 0;
            SevenSegBuffer[1] = 0;
            SevenSegBuffer[2] = 100;
            break;
        case 'F':
            SevenSegBuffer[0] = 0;
            SevenSegBuffer[1] = 0;
            SevenSegBuffer[2] = 1000;
            break;
        case 'G':
            SevenSegBuffer[0] = 0;
            SevenSegBuffer[1] = 0;
            SevenSegBuffer[2] = 10000;
            break;
        case 'H':
            SevenSegBuffer[0] = 0;
            SevenSegBuffer[1] = 0;
            SevenSegBuffer[2] = 100000;
            break;
        case 'I':
            SevenSegBuffer[0] = 0;
            SevenSegBuffer[1] = 0;
            SevenSegBuffer[2] = 11;
            break;
        case 'J':
            SevenSegBuffer[0] = 0;
            SevenSegBuffer[1] = 0;
            SevenSegBuffer[2] = 111;
            break;
        default:
            // No valid key pressed
            break;
    }
}
