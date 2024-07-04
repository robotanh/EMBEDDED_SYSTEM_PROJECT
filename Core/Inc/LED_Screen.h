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
    0b11111001, // 1
    0b10100100, // 2
    0b10110000, // 3
    0b10011001, // 4
    0b10010010, // 5
    0b10000010, // 6
    0b11111000, // 7
    0b10000000, // 8
      0b10010000// 9
};

uint8_t digitMapWithDP[10] = {
    0b11000000, // 0 with DP
	0b11111001, // 1 with DP (0b01111001)
    0b10100100, // 2 with DP
    0x30, // 3 with DP (0b00110000)
    0x19, // 4 with DP (0b00011001)
    0x12, // 5 with DP (0b00010010)
    0x02, // 6 with DP (0b00000010)
    0x78, // 7 with DP (0b01111000)
    0x00, // 8 with DP (0b00000000)
    0x10  // 9 with DP (0b00010000)
};



#endif /* INC_LED_SCREEN_H_ */
