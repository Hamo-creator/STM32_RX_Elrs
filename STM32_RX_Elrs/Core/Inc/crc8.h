/*
 * crc8.h
 *
 *  Created on: Apr 14, 2025
 *      Author: khali
 */

#ifndef INC_CRC8_H_
#define INC_CRC8_H_

#include <stdint.h>

#define CRC8_LUT_SIZE 256

typedef struct {
    uint8_t lut[CRC8_LUT_SIZE];
} Crc8_HandleTypeDef;

// Initializes the CRC8 LUT with the given polynomial
void Crc8_Init(Crc8_HandleTypeDef *hcrc8, uint8_t poly);

// Calculates CRC8 over a buffer of data
uint8_t Crc8_Calculate(Crc8_HandleTypeDef *hcrc8, uint8_t *data, uint8_t len);

uint8_t crc8_dvb_s2(const uint8_t *data, uint8_t len);


#endif /* INC_CRC8_H_ */
