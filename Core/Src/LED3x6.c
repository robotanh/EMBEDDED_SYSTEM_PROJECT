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
volatile uint8_t SevenSegScanState = 0;
uint32_t SevenSegBuffer[3] = {123456, 654321, 987654};
uint8_t displayBuffer[2][5];  // Double buffer
volatile uint8_t currentBufferIndex = 0;

uint8_t* SevenSegLEDsHandler(uint32_t* buffer, uint8_t scan_state) {
    static uint8_t output[3];
    switch (scan_state) {
        case 0:
            output[0] = buffer[0] % 10;
            output[1] = buffer[1] % 10;
            output[2] = buffer[2] % 10;
            break;
        case 1:
            output[0] = (buffer[0] / 10) % 10;
            output[1] = (buffer[1] / 10) % 10;
            output[2] = (buffer[2] / 10) % 10;
            break;
        case 2:
            output[0] = (buffer[0] / 100) % 10;
            output[1] = (buffer[1] / 100) % 10;
            output[2] = (buffer[2] / 100) % 10;
            break;
        case 3:
            output[0] = (buffer[0] / 1000) % 10;
            output[1] = (buffer[1] / 1000) % 10;
            output[2] = (buffer[2] / 1000) % 10;
            break;
        case 4:
            output[0] = (buffer[0] / 10000) % 10;
            output[1] = (buffer[1] / 10000) % 10;
            output[2] = (buffer[2] / 10000) % 10;
            break;
        case 5:
            output[0] = (buffer[0] / 100000) % 10;
            output[1] = (buffer[1] / 100000) % 10;
            output[2] = (buffer[2] / 100000) % 10;
            break;
    }
    return output;
}

void UpdateDisplayBuffer(uint32_t* buffer, uint8_t scan_state, uint8_t bufferIndex) {
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
    displayBuffer[bufferIndex][0] = 0b11111111; //skip bít
    displayBuffer[bufferIndex][1] = digitMapWithOutDP[curr_digit[2]];
    displayBuffer[bufferIndex][2] = curr_scan;
    displayBuffer[bufferIndex][3] = digitMapWithOutDP[curr_digit[1]];
    displayBuffer[bufferIndex][4] = digitMapWithOutDP[curr_digit[0]];
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
