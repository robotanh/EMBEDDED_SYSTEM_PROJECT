#include "KeyPad.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																													//
//												VARIABLES AND CONSTANTS												//
//																													//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t keyPressed = 0xFF;
uint8_t lcd_num = 0;

const uint8_t keyMap[4][5] = {
    {'C', '7', '4', '1', 'A'}, //F1
    {'0', '8', '5', '2', 'B'}, //F2
    {'E', '9', '6', '3', 'D'}, //F3
	{'T', 'P', '$', 'L', 'F'}  //F4
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
	SEQ_NUMBER,
/////////////P KEY//////////////
	SEQ_PRESSED_P,
	SEQ_PRESSED_P_F2_PSWRD,
	SEQ_PRESSED_P_F2_PSWRD_ROUND,
	SEQ_PRESSED_P_NUM,
	SEQ_PRESSED_P_NUM_SHOWHIST,
	SEQ_PRESSED_P_NUM_SETIDVOI,
	SEQ_PRESSED_P_SET_F1_PRICE,
	SEQ_PRESSED_P_SET_F2_PRICE,
	SEQ_PRESSED_P_SET_F3_PRICE,
	SEQ_PRESSED_P_SET_F4_PRICE,
	SEQ_PRESSED_P_PSWRD_SETPRICE,
/////////////T KEY//////////////
    SEQ_PRESSED_T,
    SEQ_PRESSED_T_L,
	SEQ_PRESSED_T_$,
	SEQ_PRESSED_T_F3,
	SEQ_PRESSED_T_F4,
	SEQ_PRESSED_T_F4_PASSWORD,
/////////////L KEY//////////////
	SEQ_PRESSED_L,
/////////////$ KEY//////////////
	SEQ_PRESSED_$
} SequenceState;


typedef enum {
	PRICE_ROUND_50,
	PRICE_ROUND_100,
	PRICE_DEFAULT
} PriceState;

PriceState currentPriceState = PRICE_ROUND_50;
KeyState keyState = KEY_IDLE;
SequenceState seqState = SEQ_IDLE;


// Define the timer handle
TimerHandle_t xBlinkTimer;
char blinkText[7] = "";
char blinkText1 [7] = "";
int numBlinkRow =1;		//number of row will blink

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																													//
//												HELPER FUNCTIONS													//
//																													//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Timer callback function
void vBlinkTimerCallback(TimerHandle_t xTimer) {
    static int toggle = 0;
    toggle = !toggle;
    if (toggle) {
    	if(numBlinkRow == 1) snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), blinkText);
    	else if (numBlinkRow == 2){
    		snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), blinkText);
    		snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), blinkText1);
    	}
    } else {
    	if(numBlinkRow == 1) snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), " ");
    	else if (numBlinkRow == 2){
    		snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), " ");
    		snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), " ");
    	}
    }
}

void StopBlinking() {
    if (xBlinkTimer != NULL) {
        xTimerStop(xBlinkTimer, 0);
    }
}
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
		LEDPointFlag = -1;
	}
}

void formatFloat(float value, char* buffer)
{
    int integerPart = (int)value;
    int decimalPart = (int)((value - integerPart) * 100);

    snprintf(buffer, 7, "%03d.%02d", integerPart, decimalPart);
}

void setOrderPrice (uint32_t inputPrice)
{
	orderPrice=inputPrice;
	orderLiter=(double)orderPrice/(double)roundedPrice;
}
void setOrderLiter(uint32_t inputLiter){
	orderLiter=inputLiter;
	orderPrice=orderLiter*roundedPrice;
}
void IdleEnv(){
	numberOfDigits = 0;
	accumulatedNumber = 0;

}
void setIdle(){
	seqState=SEQ_IDLE;
	IdleEnv();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																													//
//													KEY LOGIC FSM													//
//																													//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
/////////////////////////////////////////////////////KEY F1/////////////////////////////////////////////////////////
			case 'A':
				if(seqState==SEQ_IDLE){					// F1
					setOrderPrice(10000);
				}else if(seqState==SEQ_PRESSED_L){		// L -> F1
					setOrderLiter(1);
					setIdle();
				}else{
					setIdle();
				}
				break;
/////////////////////////////////////////////////////KEY F2/////////////////////////////////////////////////////////
			case 'B':
				if(seqState==SEQ_IDLE){					// F2
					setOrderPrice(15000);
				}else if(seqState==SEQ_PRESSED_L){		// L -> F2
					setOrderLiter(2);
					setIdle();
				}else if(seqState==SEQ_PRESSED_P){		// P -> F2
					seqState=SEQ_PRESSED_P_F2_PSWRD;
				}else{
					setIdle();
				}
				break;
/////////////////////////////////////////////////////KEY F3/////////////////////////////////////////////////////////
			case 'D':
				if(seqState==SEQ_IDLE){					// F3
					setOrderPrice(20000);
				}else if(seqState==SEQ_PRESSED_L){		// L -> F3
					setOrderLiter(5);
					setIdle();
				}else if (seqState == SEQ_PRESSED_T) {	// T -> F3
					seqState = SEQ_ENTER_OLD_PASSWORD;
					IdleEnv();
				} else {
					setIdle();
				}
				break;
/////////////////////////////////////////////////////KEY F4/////////////////////////////////////////////////////////
			case 'F':
				if(seqState==SEQ_IDLE){					// F4
					setOrderPrice(50000);
				}else if(seqState==SEQ_PRESSED_L){		// L -> F4
					setOrderLiter(10);
					setIdle();
				}else if (seqState == SEQ_PRESSED_T) {	// T -> F4
					seqState = SEQ_PRESSED_T_F4;
				} else {
					setIdle();
				}
				break;
/////////////////////////////////////////////////////KEY C/////////////////////////////////////////////////////////
			case 'C':
				snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", 0);
				snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
				snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 1);
				break;
/////////////////////////////////////////////////////KEY E/////////////////////////////////////////////////////////
			case 'E':
				if(seqState == SEQ_IDLE){											// {SEQ_IDLE}: 						E to display roundPrice
					seqState = SEQ_DISP_PRICE;
				}else if (seqState == SEQ_PRESSED_$){								// {SEQ_PRESSED_$}: 				$ -> [OrderPrice] -> E to set Order Price
					setOrderPrice(accumulatedNumber);
					setIdle();
				}else if (seqState == SEQ_PRESSED_L){								// {SEQ_PRESSED_L}: 				L -> [OrderLiter] -> E to set Order Liter
					setOrderLiter(accumulatedNumber);
					setIdle();
				}else if (seqState == SEQ_PRESSED_P_NUM&&							// {SEQ_PRESSED_P_NUM}:				P -> [997979] -> E to go to show history
					accumulatedNumber==997979) {
					seqState = SEQ_PRESSED_P_NUM_SHOWHIST;
					IdleEnv();

				}else if (seqState == SEQ_PRESSED_P_NUM&&							// {SEQ_PRESSED_P_NUM}:				P -> [999032] -> E to set ID gaspump
						accumulatedNumber==999032){
					seqState = SEQ_PRESSED_P_NUM_SETIDVOI;
					IdleEnv();
				}else if (seqState == SEQ_PRESSED_P_NUM_SETIDVOI){
					if ( 11 <= accumulatedNumber  && accumulatedNumber <= 47){
						IDvoi = accumulatedNumber;

					}
					seqState = SEQ_IDLE;
					IdleEnv();
				}
				else if (seqState == SEQ_PRESSED_P_NUM&&
					accumulatedNumber==995591) {
					seqState = SEQ_PRESSED_P_SET_F1_PRICE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				else if (seqState == SEQ_PRESSED_P_NUM&&
					accumulatedNumber==995592) {
					seqState = SEQ_PRESSED_P_SET_F2_PRICE;
					numberOfDigits = 0;
					accumulatedNumber = 0;

				}
				else if (seqState == SEQ_PRESSED_P_NUM&&
					accumulatedNumber==995593) {
					seqState = SEQ_PRESSED_P_SET_F3_PRICE;
					numberOfDigits = 0;
					accumulatedNumber = 0;

				}
				else if (seqState == SEQ_PRESSED_P_NUM&&
					accumulatedNumber==995594) {
					seqState = SEQ_PRESSED_P_SET_F4_PRICE;
					numberOfDigits = 0;
					accumulatedNumber = 0;

				}
				else if (seqState == SEQ_PRESSED_P_SET_F1_PRICE) {
					F1Price=accumulatedNumber;
					seqState=SEQ_IDLE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				else if (seqState == SEQ_PRESSED_P_SET_F2_PRICE) {
					F2Price=accumulatedNumber;
					seqState=SEQ_IDLE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				else if (seqState == SEQ_PRESSED_P_SET_F3_PRICE) {
					F3Price=accumulatedNumber;
					seqState=SEQ_IDLE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				else if (seqState == SEQ_PRESSED_P_SET_F4_PRICE) {
					F4Price=accumulatedNumber;
					seqState=SEQ_IDLE;
					numberOfDigits = 0;
					accumulatedNumber = 0;
				}
				else if (seqState == SEQ_PRESSED_P_NUM&&							// {SEQ_PRESSED_P_NUM}:				P -> [PSSWRD] -> E to go to {SEQ_PRESSED_P_PSWRD_SETPRICE}
					accumulatedNumber==password) {
					seqState = SEQ_PRESSED_P_PSWRD_SETPRICE;
					IdleEnv();
				}
				else if(seqState == SEQ_PRESSED_P_PSWRD_SETPRICE){					//  {SEQ_PRESSED_P_PSWRD_SETPRICE}: P -> [PSSWRD] -> E -> [PRICE] -> E to set currPrice and apply roundPrice settings
					currPrice = accumulatedNumber;
					switch (currentPriceState){
						case PRICE_ROUND_50:
							if(currPrice%50<25){
								roundedPrice=currPrice-(currPrice%50);
							}else{
								roundedPrice=currPrice-(currPrice%50)+50;
							}
							break;
						case PRICE_ROUND_100:
							if(currPrice%100<50){
								roundedPrice=currPrice-(currPrice%100);
							}else{
								roundedPrice=currPrice-(currPrice%100)+100;
							}
							break;
						default:
							roundedPrice=currPrice;
							break;
					}
					setIdle();
				}else if (seqState == SEQ_PRESSED_P_F2_PSWRD&&						// {SEQ_PRESSED_P_F2_PSWRD}:		P -> F2 -> [PSSWRD] -> E to go to {SEQ_PRESSED_P_F2_PSWRD_ROUND}
						accumulatedNumber==password){
					seqState = SEQ_PRESSED_P_F2_PSWRD_ROUND;
					IdleEnv();
				}else if (seqState == SEQ_PRESSED_P_F2_PSWRD_ROUND){				// {SEQ_PRESSED_P_F2_PSWRD_ROUND}:	P -> F2 -> [PSSWRD] -> E -> [0|1|2] -> E to confirm roundPrice
					switch(accumulatedNumber){
						case 0:															// [0]: round 50
							if(currPrice%50<25){
								roundedPrice=currPrice-(currPrice%50);
							}else{
								roundedPrice=currPrice-(currPrice%50)+50;
							}
							currentPriceState=PRICE_ROUND_50;
							break;
						case 1:															// [1]: round 100
							if(currPrice%100<50){
								roundedPrice=currPrice-(currPrice%100);
							}else{
								roundedPrice=currPrice-(currPrice%100)+100;
							}
							currentPriceState=PRICE_ROUND_100;
							break;
						case 2:															// [2]: no rounding
							roundedPrice=currPrice;
							currentPriceState=PRICE_DEFAULT;
							break;
						default:
							break;
					}
					setIdle();
				}else if(seqState == SEQ_PRESSED_T_F4&&    							// {SEQ_PRESSED_T_F4}: T -> F4 -> [PSSWRD] -> E to delete totalLitersShift
						accumulatedNumber == password){
					seqState = SEQ_PRESSED_T_F4_PASSWORD;
					IdleEnv();
					totalLitersShift = 0;
				}else if(seqState == SEQ_ENTER_OLD_PASSWORD&&
						accumulatedNumber == password){
					seqState = SEQ_ENTER_NEW_PASSWORD;
					IdleEnv();
				}else if(seqState == SEQ_ENTER_NEW_PASSWORD){
					password = accumulatedNumber;
					setIdle();
				}else {
					setIdle();
				}
				break;
/////////////////////////////////////////////////////KEY P/////////////////////////////////////////////////////////
			case 'P':
				if (seqState == SEQ_IDLE) {
					seqState = SEQ_PRESSED_P;
				} else {
					setIdle();
				}
				break;
/////////////////////////////////////////////////////KEY T/////////////////////////////////////////////////////////
			case 'T':
				if (seqState == SEQ_IDLE) {
					seqState = SEQ_PRESSED_T;
				} else {
					setIdle();
				}
				break;
/////////////////////////////////////////////////////KEY $/////////////////////////////////////////////////////////
			case '$':
				if (seqState == SEQ_IDLE) {
					seqState = SEQ_PRESSED_$;
				}else if (seqState == SEQ_PRESSED_T) {
					seqState = SEQ_PRESSED_T_$;
				} else {
					setIdle();
				}
				break;
/////////////////////////////////////////////////////KEY L/////////////////////////////////////////////////////////
			case 'L':
				if (seqState == SEQ_IDLE) {
					seqState = SEQ_PRESSED_L;
				} else if (seqState == SEQ_PRESSED_T) {
					seqState = SEQ_PRESSED_T_L;
				} else {
					setIdle();
				}
				break;
			default:
/////////////////////////////////////////////////////KEY 0-9/////////////////////////////////////////////////////////
				if(keyPressed >= '0' && keyPressed <= '9')
				{
					if(seqState==SEQ_PRESSED_P)
					{
						seqState=SEQ_PRESSED_P_NUM;
						accumulatedNumber = keyPressed - '0';
						numberOfDigits = 1;
					}
					else if (seqState == SEQ_PRESSED_P_NUM ||
							seqState == SEQ_PRESSED_P_F2_PSWRD ||
							seqState == SEQ_PRESSED_P_SET_F1_PRICE||
							seqState == SEQ_PRESSED_P_SET_F2_PRICE||
							seqState == SEQ_PRESSED_P_SET_F3_PRICE||
							seqState == SEQ_PRESSED_P_SET_F4_PRICE||
							seqState == SEQ_PRESSED_P_PSWRD_SETPRICE||
							seqState == SEQ_PRESSED_P_NUM_SETIDVOI||
							seqState == SEQ_PRESSED_T_F4||
							seqState == SEQ_ENTER_OLD_PASSWORD ||
							seqState == SEQ_ENTER_NEW_PASSWORD ||
							seqState == SEQ_NUMBER ||
							seqState == SEQ_PRESSED_$||
							seqState == SEQ_PRESSED_L
							)
					{
						if (numberOfDigits < 6) {
							accumulatedNumber = accumulatedNumber * 10 + (keyPressed - '0');
							numberOfDigits++;
						}
					}
					else if(seqState == SEQ_PRESSED_P_F2_PSWRD_ROUND)
					{
						if (numberOfDigits < 1) {
							accumulatedNumber = accumulatedNumber * 10 + (keyPressed - '0');
							numberOfDigits++;
						}
					}
					else if(seqState == SEQ_PRESSED_P_NUM_SHOWHIST) 	//// {SEQ_PRESSED_P_NUM}:				P -> [997979] -> E -> number
					{
						accumulatedNumber = keyPressed - '0';
						numberOfDigits = 1;
					}
					else
					{
						seqState = SEQ_NUMBER;
						accumulatedNumber = keyPressed - '0';
						numberOfDigits = 1;
					}
				}else{
					setIdle();
				}
				break;
		}
	}
	keyPressed = 0xFF;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																													//
//												KEY LOGIC ACTION FSM												//
//																													//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void KeyLogic_Action() {
    char buffer[7];
    switch (seqState) {
        case SEQ_IDLE:
        	LEDPointFlag = -1;
        	snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", 0);
        	snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06ld", orderPrice);
        	formatFloat(orderLiter, SevenSegBuffer[2]);
            break;
        case SEQ_DISP_PRICE:
        	snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "GIA   ");
			snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", IDvoi);
			snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 0);
			break;
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
        	snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06ld", accumulatedNumber);
			snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
			snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "P     ");
			break;
        case SEQ_PRESSED_P_F2_PSWRD:
        	snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06ld", accumulatedNumber);
			snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
			snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "SL ARO");
			break;
        case SEQ_PRESSED_P_F2_PSWRD_ROUND:
        	snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "0.  50");
			snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "1. 100");
			snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "2. 1..");
			break;
        case SEQ_PRESSED_P_NUM:
            snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06ld", accumulatedNumber);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "P88888");
            break;
        case SEQ_PRESSED_P_SET_F1_PRICE:
			snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06ld", accumulatedNumber);
			snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06ld", F1Price);
			snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "SET F1");
			break;
		case SEQ_PRESSED_P_SET_F2_PRICE:
			snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06ld", accumulatedNumber);
			snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06ld", F2Price);
			snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "SET F2");
			break;
		case SEQ_PRESSED_P_SET_F3_PRICE:
			snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06ld", accumulatedNumber);
			snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06ld", F3Price);
			snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "SET F3");
			break;
		case SEQ_PRESSED_P_SET_F4_PRICE:
			snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06ld", accumulatedNumber);
			snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06ld", F4Price);
			snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "SET F4");
			break;
        case SEQ_PRESSED_P_NUM_SETIDVOI:
        	snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06ld", accumulatedNumber);
        	snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "......");
        	numBlinkRow =1;
			snprintf(blinkText, sizeof(blinkText), "SET ID"); // Set blink text
			if (xBlinkTimer == NULL) {
				xBlinkTimer = xTimerCreate("BlinkTimer", pdMS_TO_TICKS(300), pdTRUE, (void *)0, vBlinkTimerCallback);
				if (xBlinkTimer != NULL) {
					xTimerStart(xBlinkTimer, 0);
				}
			}
        	break;
        case SEQ_PRESSED_P_NUM_SHOWHIST:
        	LEDPointFlag = -1;
        	if(1<=accumulatedNumber && accumulatedNumber <=5){
				// Format the total liters into two parts
				formatTotalLiters(histTotalLiters[accumulatedNumber-1], &row1, &row2);

				// Ensure the combined string fits into the buffer
				char row1StrHist[7]; // Buffer to hold formatted row1 string
				snprintf(row1StrHist, sizeof(row1StrHist), "%06ld", row1);

				// Combine "L.. " with the last two digits of row1
				char combinedStrHist[8]; // Buffer to hold combined string "L.. " and last two digits of row1
				snprintf(combinedStrHist, sizeof(combinedStrHist), "1.%04ld", row1 % 10000); // Extract last two digits of row1

				// Fill SevenSegBuffer[0] with combinedStr and pad with spaces if necessary
				for (int i = 0; i < 6; ++i) {
					if (i < strlen(combinedStrHist)) {
						SevenSegBuffer[0][i] = combinedStrHist[i];
					} else {
						SevenSegBuffer[0][i] = ' '; // Pad with spaces
					}
				}


				snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06ld", row2);

				numBlinkRow =1;
				snprintf(blinkText, sizeof(blinkText), "HIST "); // Set blink text
				if (xBlinkTimer == NULL) {
					xBlinkTimer = xTimerCreate("BlinkTimer", pdMS_TO_TICKS(300), pdTRUE, (void *)0, vBlinkTimerCallback);
					if (xBlinkTimer != NULL) {
						xTimerStart(xBlinkTimer, 0);
					}
				}
        	}
        	else{
        		accumulatedNumber = 1;
        	}

        	break;
        case SEQ_PRESSED_P_PSWRD_SETPRICE:
            snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06ld", accumulatedNumber);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "GIA   ");
            break;
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

			numBlinkRow =1;
			snprintf(blinkText, sizeof(blinkText), "SHIFT "); // Set blink text
			if (xBlinkTimer == NULL) {
				xBlinkTimer = xTimerCreate("BlinkTimer", pdMS_TO_TICKS(300), pdTRUE, (void *)0, vBlinkTimerCallback);
				if (xBlinkTimer != NULL) {
					xTimerStart(xBlinkTimer, 0);
				}
			}

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

            numBlinkRow =1;
			snprintf(blinkText, sizeof(blinkText), "TOTAL "); // Set blink text
			if (xBlinkTimer == NULL) {
				xBlinkTimer = xTimerCreate("BlinkTimer", pdMS_TO_TICKS(300), pdTRUE, (void *)0, vBlinkTimerCallback);
				if (xBlinkTimer != NULL) {
					xTimerStart(xBlinkTimer, 0);
				}
			}



            break;

        case SEQ_PRESSED_T_F3:
            snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", 333333);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 0);
            break;
        case SEQ_PRESSED_T_F4:
        	snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06ld", accumulatedNumber);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "DELETE");
            break;
        case SEQ_PRESSED_T_F4_PASSWORD:
        	snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), " ");
			snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), " ");
			snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "DONE ");
			break;
        case SEQ_NUMBER:
			snprintf(buffer, sizeof(buffer), "%06ld", accumulatedNumber);
			snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%s", buffer);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 0);
			LEDPointFlag = -1;
			break;
        case SEQ_PRESSED_$:
			snprintf(buffer, sizeof(buffer), "%06ld", accumulatedNumber);
			snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%s", buffer);

			LEDPointFlag = -1;
			numBlinkRow =2;
			snprintf(blinkText1, sizeof(blinkText1), "SET   "); // Set blink text
			snprintf(blinkText, sizeof(blinkText), "GIA   ");
			if (xBlinkTimer == NULL) {
				xBlinkTimer = xTimerCreate("BlinkTimer", pdMS_TO_TICKS(300), pdTRUE, (void *)0, vBlinkTimerCallback);
				if (xBlinkTimer != NULL) {
					xTimerStart(xBlinkTimer, 0);
				}
			}

//			snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "SET   ");
//			snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "GIA   ");
			break;
        case SEQ_PRESSED_L:
			snprintf(buffer, sizeof(buffer), "%06ld", accumulatedNumber);
			snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%s", buffer);

			LEDPointFlag = -1;
			numBlinkRow =2;
			snprintf(blinkText1, sizeof(blinkText1), "SET   "); // Set blink text
			snprintf(blinkText, sizeof(blinkText), "LIT   ");
			if (xBlinkTimer == NULL) {
				xBlinkTimer = xTimerCreate("BlinkTimer", pdMS_TO_TICKS(300), pdTRUE, (void *)0, vBlinkTimerCallback);
				if (xBlinkTimer != NULL) {
					xTimerStart(xBlinkTimer, 0);
				}
			}

//			snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "SET   ");
//			snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "LIT   ");
			break;
        default:
        	LEDPointFlag = -1;
            snprintf(SevenSegBuffer[0], sizeof(SevenSegBuffer[0]), "%06d", 0);
            snprintf(SevenSegBuffer[1], sizeof(SevenSegBuffer[1]), "%06d", 0);
            snprintf(SevenSegBuffer[2], sizeof(SevenSegBuffer[2]), "%06d", 0);
            break;
    }
}
