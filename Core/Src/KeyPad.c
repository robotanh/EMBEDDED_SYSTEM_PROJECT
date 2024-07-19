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

uint32_t accumulatedNumber = 0;
uint8_t numberOfDigits = 0;

uint32_t row1 =0; 	//Display total liters
uint32_t row2 =0;	//Display total liters
typedef enum {
    KEY_IDLE,
    KEY_DEBOUNCING,
    KEY_PRESSED,
    KEY_HOLDING
} KeyState;


typedef enum {
    SEQ_IDLE,
/////////////E KEY//////////////
	SEQ_DISP_PRICE,
	SEQ_ENTER_OLD_PASSWORD,
	SEQ_ENTER_NEW_PASSWORD,
/////////////P KEY//////////////
	SEQ_PRESSED_P,
	SEQ_PRESSED_P_NUM,
	SEQ_PRESSED_P_PSWRD_SETPRICE,
/////////////T KEY//////////////
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

// T + L pressed show total from beginning
void formatTotalLiters(long unsigned int total, uint32_t* buffer1, uint32_t* buffer2)
{
	if (total < 100000000) {

		* buffer1 = total / 1000000;
		* buffer2 = total % 1000000;
		LEDPointFlag = 3;

	} else {
		* buffer1 = total / 100000000;
		* buffer2 = (total % 100000000 ) /100;
		LEDPointFlag = 1;
	}
}
// T + $ pressed show total per shift
void formatTotalLitersShift(long unsigned int total, uint32_t* buffer1, uint32_t* buffer2)
{
	if (total < 1000000000) {

		* buffer1 = total / 1000000;
		* buffer2 = total % 1000000;
		LEDPointFlag = 3;

	} else {
		* buffer1 =0;
		* buffer2 = 0;
		LEDPointFlag = -11;
	}
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
/////////////////////////////////////////////////////KEY A/////////////////////////////////////////////////////////
			case 'A':
				snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", 0);
				snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 10000);
				snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 0);
				break;
/////////////////////////////////////////////////////KEY B/////////////////////////////////////////////////////////
			case 'B':
				snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", 0);
				snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 100000);
				snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 0);
				break;
/////////////////////////////////////////////////////KEY C/////////////////////////////////////////////////////////
			case 'C':
				snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", 0);
				snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
				snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 1);
				break;
/////////////////////////////////////////////////////KEY E/////////////////////////////////////////////////////////
			case 'E':
				if(seqState == SEQ_IDLE){
					seqState = SEQ_DISP_PRICE;
				}
				else if (seqState == SEQ_PRESSED_P_NUM&&
					accumulatedNumber==password) {
					seqState = SEQ_PRESSED_P_PSWRD_SETPRICE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				else if(seqState == SEQ_PRESSED_P_PSWRD_SETPRICE){
					currPrice = accumulatedNumber;
					seqState = SEQ_IDLE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				else if(seqState == SEQ_ENTER_OLD_PASSWORD&&accumulatedNumber == password){
					seqState = SEQ_ENTER_NEW_PASSWORD;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				else if(seqState == SEQ_ENTER_NEW_PASSWORD){
					password = accumulatedNumber;
					seqState = SEQ_IDLE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				else {
					seqState = SEQ_IDLE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				break;
/////////////////////////////////////////////////////KEY P/////////////////////////////////////////////////////////
			case 'P':
				if (seqState == SEQ_IDLE) {
					seqState = SEQ_PRESSED_P;
				} else {
					seqState = SEQ_IDLE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				break;
/////////////////////////////////////////////////////KEY T/////////////////////////////////////////////////////////
			case 'T':
				if (seqState == SEQ_IDLE) {
					seqState = SEQ_PRESSED_T;
				} else {
					seqState = SEQ_IDLE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				break;
/////////////////////////////////////////////////////KEY $/////////////////////////////////////////////////////////
			case '$':
				if (seqState == SEQ_PRESSED_T) {
					seqState = SEQ_PRESSED_T_$;
				} else {
					seqState = SEQ_IDLE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				break;
/////////////////////////////////////////////////////KEY L/////////////////////////////////////////////////////////
			case 'L':
				if (seqState == SEQ_PRESSED_T) {
					seqState = SEQ_PRESSED_T_L;
				} else {
					seqState = SEQ_IDLE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				break;
/////////////////////////////////////////////////////KEY D/////////////////////////////////////////////////////////
			case 'D':
				if (seqState == SEQ_PRESSED_T) {
					seqState = SEQ_ENTER_OLD_PASSWORD;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				} else {
					seqState = SEQ_IDLE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				break;
/////////////////////////////////////////////////////KEY F/////////////////////////////////////////////////////////
			case 'F':
				if (seqState == SEQ_PRESSED_T) {
					seqState = SEQ_PRESSED_T_F4;
				} else {
					seqState = SEQ_IDLE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				break;
			default:
/////////////////////////////////////////////////////KEY 0-9/////////////////////////////////////////////////////////
				if(keyPressed >= '0' && keyPressed <= '9') {
					if(seqState==SEQ_PRESSED_P){
						seqState=SEQ_PRESSED_P_NUM;
						accumulatedNumber = keyPressed - '0';
						numberOfDigits = 1;
					}else if (seqState == SEQ_PRESSED_P_NUM ||
							seqState == SEQ_PRESSED_P_PSWRD_SETPRICE||
							seqState == SEQ_ENTER_OLD_PASSWORD ||
							seqState == SEQ_ENTER_NEW_PASSWORD) {
						if (numberOfDigits < 6) {
							accumulatedNumber = accumulatedNumber * 10 + (keyPressed - '0');
							numberOfDigits++;
						}
					}
				}else{
					seqState = SEQ_IDLE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				break;
		}
	}
	keyPressed = 0xFF;
}

void KeyLogic_Action() {
    char buffer[7];
    switch (seqState) {
        case SEQ_IDLE:
            snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", 0);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 0);
            break;
        case SEQ_DISP_PRICE:
        	snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "GIA   ");
			snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", currPrice);
			snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 0);
        case SEQ_ENTER_OLD_PASSWORD:
            snprintf(buffer, sizeof(buffer), "%06ld", accumulatedNumber);
            snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%s", buffer);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "0L0 ");
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 0);
            LEDPointFlag = -1;
            break;
        case SEQ_ENTER_NEW_PASSWORD:
            snprintf(buffer, sizeof(buffer), "%06ld", accumulatedNumber);
            snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%s", buffer);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), " NEUU ");
            LEDPointFlag = -1;
            break;
        case SEQ_PRESSED_P:
        	snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", accumulatedNumber);
			snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
			snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "P     ");
        case SEQ_PRESSED_P_NUM:
            snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", accumulatedNumber);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "P88888");
        case SEQ_PRESSED_P_PSWRD_SETPRICE:
            snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", accumulatedNumber);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "GIA   ");
        case SEQ_PRESSED_T:
            snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", 0);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 999999);
            break;
        case SEQ_PRESSED_T_$:
        	 // Format the total liters into two parts
			formatTotalLitersShift(totalLitersShift, &row1, &row2);

			// Ensure the combined string fits into the buffer
			char row1StrShift[7]; // Buffer to hold formatted row1 string
			snprintf(row1StrShift, sizeof(row1StrShift), "%06ld", row1);

			// Combine "L.. " with the last two digits of row1
			char combinedStrShift[8]; // Buffer to hold combined string "L.. " and last two digits of row1
			snprintf(combinedStrShift, sizeof(combinedStrShift), "SH.%04ld", row1 % 10000); // Extract last two digits of row1

			// Fill SevenSegBuffer[0] with combinedStr and pad with spaces if necessary
			for (int i = 0; i < 6; ++i) {
				if (i < strlen(combinedStrShift)) {
					SevenSegBuffer[0][i] = combinedStrShift[i];
				} else {
					SevenSegBuffer[0][i] = ' '; // Pad with spaces
				}
			}
			snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06ld", row2);
			snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "SHIFT ");

            break;
        case SEQ_PRESSED_T_L:
            // Format the total liters into two parts
            formatTotalLiters(totalLiters, &row1, &row2);

            // Ensure the combined string fits into the buffer
            char row1Str[7]; // Buffer to hold formatted row1 string
            snprintf(row1Str, sizeof(row1Str), "%06ld", row1);

            // Combine "L.. " with the last two digits of row1
            char combinedStr[8]; // Buffer to hold combined string "L.. " and last two digits of row1
            snprintf(combinedStr, sizeof(combinedStr), "L.%04ld", row1 % 10000); // Extract last two digits of row1

            // Fill SevenSegBuffer[0] with combinedStr and pad with spaces if necessary
            for (int i = 0; i < 6; ++i) {
                if (i < strlen(combinedStr)) {
                    SevenSegBuffer[0][i] = combinedStr[i];
                } else {
                    SevenSegBuffer[0][i] = ' '; // Pad with spaces
                }
            }


            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06ld", row2);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "TOTAL ");



            break;



        case SEQ_PRESSED_T_F3:
            snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", 333333);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 0);
            break;
        case SEQ_PRESSED_T_F4:
            snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", 444444);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 0);
            break;
        default:
            snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", 0);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 0);
            break;
    }
}
