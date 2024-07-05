/*
 * LED_Screen.h
 *
 *  Created on: Jun 24, 2024
 *      Author: Admin
 */

#ifndef INC_LED_SCREEN_H_
#define INC_LED_SCREEN_H_
uint8_t digitMap[10] = {
    0b11000000, // 0
    0b01111110, // 1
    0b10100100, // 2
    0b10110000, // 3
    0b10011001, // 4
    0b10010010, // 5
    0b10000010, // 6
    0b01111000, // 7
    0b10000000, // 8
    0b10010000  // 9
};

uint8_t digitMapWithDP[10] = {
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

#endif /* INC_LED_SCREEN_H_ */
