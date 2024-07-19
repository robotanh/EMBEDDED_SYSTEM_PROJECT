/*
 * LED3x6.c
 *
 *  Created on: Jul 13, 2024
 *      Author: Admin
 */

#include "LED3x6.h"
#include "SPI_shift.h"
#include "stm32f4xx_hal.h"

uint8_t digitMapWithOutDP[10] = {
    0b11000000, // 0 without DP
    0b11111001, // 1 without DP
    0b10100100, // 2 without DP
    0b10110000, // 3 without DP
    0b10011001, // 4 without DP
    0b10010010, // 5 without DP
    0b10000010, // 6 without DP
    0b11111000, // 7 without DP
    0b10000000, // 8 without DP
    0b10010000  // 9 without DP
};

uint8_t digitMapWithDP[10] = {
    0b01000000, // 0 with DP
    0b01111001, // 1 with DP
    0b00100100, // 2 with DP
    0b00110000, // 3 with DP
    0b00011001, // 4 with DP
    0b00010010, // 5 with DP
    0b00000010, // 6 with DP
    0b01111000, // 7 with DP
    0b00000000, // 8 with DP
    0b00010000  // 9 with DP
};

uint8_t specialCharMap[15] = {
    0b11000111, // 'L'
    0b01111111, // '.'
    0b11111000, // 'T'
    0b11000000, // 'O'
    0b10001000, // 'A'
    0b10010010, // 'S'
    0b10001001, // 'H'
    0b11001111, // 'I'
    0b10001110, // 'F'
	0b10111100,	// 'G'
	0b10001100,	// 'P'
    0b10110001, // 'C'
    0b10000110, // 'E'
    0b11000001, // 'U'
    0b11001000, // 'N'
};


volatile uint8_t SevenSegScanState = 0;
//uint32_t SevenSegBuffer[3] = {123456, 654321, 987654};
char SevenSegBuffer[3][7] = {"123456", "654321", "987654"};
uint8_t displayBuffer[2][5];  // Double buffer
volatile uint8_t currentBufferIndex = 0;


uint8_t CharToSegment(char c) {
    if (c >= '0' && c <= '9') {
        return digitMapWithOutDP[c - '0'];
    } else if (c == 'L') {
        return specialCharMap[0];
    } else if (c == '.') {
        return specialCharMap[1];
    } else if (c == 'T') {
        return specialCharMap[2];
    } else if (c == 'O') {
        return specialCharMap[3];
    } else if (c == 'A') {
        return specialCharMap[4];
    } else if (c == 'S') {
        return specialCharMap[5];
    } else if (c == 'H') {
        return specialCharMap[6];
    } else if (c == 'I') {
        return specialCharMap[7];
    } else if (c == 'F') {
        return specialCharMap[8];
    } else if (c == 'G') {
        return specialCharMap[9];
    } else if (c == 'P') {
        return specialCharMap[10];
    } else if (c == 'C') {
        return specialCharMap[11];
    } else if (c == 'E') {
        return specialCharMap[12];
    } else if (c == 'U') {
        return specialCharMap[13];
    } else if (c == 'N') {
        return specialCharMap[14];
    } else {
        return 0b11111111; // Blank
    }
}


uint8_t* SevenSegLEDsHandler(char buffer[3][7], uint8_t scan_state) {
    static uint8_t output[3];
    for (int i = 0; i < 3; i++) {
        int len = strlen(buffer[i]);
        if (scan_state < 6) {
            if (scan_state < len) {
                output[i] = CharToSegment(buffer[i][len - 1 - scan_state]);
            } else {
                output[i] = 0b11111111; // Blank
            }
        } else {
            output[i] = 0b11111111; // Blank
        }
    }
    return output;
}

void UpdateDisplayBuffer(char buffer[3][7], uint8_t scan_state, uint8_t bufferIndex) {
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
    if (LEDPointFlag >= 0 && LEDPointFlag <= 5) {
        if (scan_state == LEDPointFlag) {
            displayBuffer[bufferIndex][0] = 0b11111111; // Skip bit
            displayBuffer[bufferIndex][1] = curr_digit[2];
            displayBuffer[bufferIndex][2] = curr_scan;
            displayBuffer[bufferIndex][3] = curr_digit[1] & 0b01111111; // Add DP
            displayBuffer[bufferIndex][4] = curr_digit[0];
        } else {
            displayBuffer[bufferIndex][0] = 0b11111111; // Skip bit
            displayBuffer[bufferIndex][1] = curr_digit[2];
            displayBuffer[bufferIndex][2] = curr_scan;
            displayBuffer[bufferIndex][3] = curr_digit[1];
            displayBuffer[bufferIndex][4] = curr_digit[0];
        }
    } else {
        displayBuffer[bufferIndex][0] = 0b11111111; // Skip bit
        displayBuffer[bufferIndex][1] = curr_digit[2];
        displayBuffer[bufferIndex][2] = curr_scan;
        displayBuffer[bufferIndex][3] = curr_digit[1];
        displayBuffer[bufferIndex][4] = curr_digit[0];
    }
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
