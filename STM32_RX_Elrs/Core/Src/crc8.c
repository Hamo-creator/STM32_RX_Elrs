/*
 * crc8.c
 *
 *  Created on: Apr 14, 2025
 *      Author: Khalil
 */

#include "crc8.h"

void Crc8_Init(Crc8_HandleTypeDef *hcrc8, uint8_t poly)
{
    for (int idx = 0; idx < 256; ++idx)
    {
        uint8_t crc = idx;
        for (int shift = 0; shift < 8; ++shift)
        {
            crc = (crc << 1) ^ ((crc & 0x80) ? poly : 0);
        }
        hcrc8->lut[idx] = crc & 0xFF;
    }
}

uint8_t Crc8_Calculate(Crc8_HandleTypeDef *hcrc8, uint8_t *data, uint8_t len)
{
    uint8_t crc = 0;
    while (len--)
    {
        crc = hcrc8->lut[crc ^ *data++];
    }
    return crc;
}

uint8_t crc8_dvb_s2(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x80) ? (crc << 1) ^ 0xD5 : (crc << 1);
    }
    return crc;
}

