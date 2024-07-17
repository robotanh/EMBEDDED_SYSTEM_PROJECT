#include "KeyPad.h"

uint8_t keyPressed = 0xFF;
uint8_t lcd_num = 0;

const uint8_t keyMap[4][5] = {
    {'C', '7', '4', '1', 'A'},
    {'0', '8', '5', '2', 'B'},
    {'E', '9', '6', '3', 'D'},
	{'T', 'P', '$', 'L', 'F'}
};

#define DEBOUNCE_DELAY pdMS_TO_TICKS(50)
#define HOLD_DELAY pdMS_TO_TICKS(300)

GPIO_InitTypeDef GPIO_InitStructPrivate = {0};
TickType_t lastDebounceTime = 0;
TickType_t lastKeyPressTime = 0;
uint8_t lastKeyPressed = 0xFF;

typedef enum {
    KEY_IDLE,
    KEY_DEBOUNCING,
    KEY_PRESSED,
    KEY_HOLDING
} KeyState;


typedef enum {
    SEQ_IDLE,
    SEQ_PRESSED_T,
    SEQ_PRESSED_T_L,
	SEQ_PRESSED_T_$,
	SEQ_PRESSED_T_F3,
	SEQ_PRESSED_T_F4,
} SequenceState;


KeyState keyState = KEY_IDLE;
SequenceState seqState = SEQ_IDLE;

void KeyPad_Init(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);
}
uint8_t ScanColumns(uint8_t row) {
    switch (row) {
        case 0:
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET) return keyMap[0][0];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET) return keyMap[0][1];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_RESET) return keyMap[0][2];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_RESET) return keyMap[0][3];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET) return keyMap[0][4];
            break;
        case 1:
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET) return keyMap[1][0];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET) return keyMap[1][1];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_RESET) return keyMap[1][2];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_RESET) return keyMap[1][3];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET) return keyMap[1][4];
            break;
        case 2:
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET) return keyMap[2][0];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET) return keyMap[2][1];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_RESET) return keyMap[2][2];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_RESET) return keyMap[2][3];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET) return keyMap[2][4];
            break;
        case 3:
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET) return keyMap[3][0];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET) return keyMap[3][1];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_RESET) return keyMap[3][2];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_RESET) return keyMap[3][3];
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET) return keyMap[3][4];
            break;
        default:
            return 0xFF;
    }
    return 0xFF;  // No key pressed
}



uint8_t KeyPad_Scan(void) {
    uint8_t key;

    // Scan row 1
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);
    key = ScanColumns(0);
    if (key != 0xFF) return key;

    // Scan row 2
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);
    key = ScanColumns(1);
    if (key != 0xFF) return key;

    // Scan row 3
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9, GPIO_PIN_SET);
    key = ScanColumns(2);
    if (key != 0xFF) return key;

    // Scan row 4
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8, GPIO_PIN_SET);
    key = ScanColumns(3);
    if (key != 0xFF) return key;

    return 0xFF;  // No key pressed
}




void KeyLogic() {
    TickType_t currentMillis = xTaskGetTickCount();
    uint8_t currentKey = KeyPad_Scan();

    switch (keyState) {
        case KEY_IDLE:
            if (currentKey != 0xFF) {
                lastKeyPressed = currentKey;
                lastDebounceTime = currentMillis;
                keyState = KEY_DEBOUNCING;
            }
            break;

        case KEY_DEBOUNCING:
            if (currentMillis - lastDebounceTime >= DEBOUNCE_DELAY) {
                if (currentKey == lastKeyPressed) {
                    keyState = KEY_PRESSED;
                    lastKeyPressTime = currentMillis;
                } else {
                    keyState = KEY_IDLE;
                }
            }
            break;

        case KEY_PRESSED:
            if (currentKey == lastKeyPressed) {
                if (currentMillis - lastKeyPressTime >= HOLD_DELAY) {
                    keyState = KEY_HOLDING;
                }
            } else {
                keyPressed = lastKeyPressed;
                keyState = KEY_IDLE;
            }
            break;

        case KEY_HOLDING:
            if (currentKey != lastKeyPressed) {
                keyPressed = lastKeyPressed;
                keyState = KEY_IDLE;
            }
            break;
    }

    if (keyPressed != 0xFF) {
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
            case 'T':
                SevenSegBuffer[0] = 0;
                SevenSegBuffer[1] = 0;
                SevenSegBuffer[2] = 10000;
                if (seqState == SEQ_IDLE) {
					seqState = SEQ_PRESSED_T;
				} else {
					seqState = SEQ_IDLE;
				}
                break;
            case 'P':
                SevenSegBuffer[0] = 0;
                SevenSegBuffer[1] = 0;
                SevenSegBuffer[2] = 100000;
                break;
            case '$':
				SevenSegBuffer[0] = 0;
				SevenSegBuffer[1] = 0;
				SevenSegBuffer[2] = 111111;
				break;
            case 'L':
				SevenSegBuffer[0] = 0;
				SevenSegBuffer[1] = 111111;
				SevenSegBuffer[2] = 0;
				if (seqState == SEQ_PRESSED_T) {
					//Show total
					seqState = SEQ_IDLE;
				} else {
					seqState = SEQ_IDLE;
				}
				break;
            default:
            	seqState = SEQ_IDLE;
                break;
        }
        keyPressed = 0xFF;
    }
}

