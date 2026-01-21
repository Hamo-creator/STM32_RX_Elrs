/*
 * neo6m.h
 *
 *  Created on: Oct 26, 2025
 *      Author: Khalil
 */

#ifndef SRC_NEO6M_H_
#define SRC_NEO6M_H_


#include "main.h"
#include <stdint.h>

typedef struct {
    float latitude;     // degrees
    float longitude;    // degrees
    float altitude;     // meters
    float speed;        // km/h
    uint8_t num_sats;   // number of satellites
    uint8_t fix;        // 1 = valid fix
} NEO6M_Data_t;

void NEO6M_Init(UART_HandleTypeDef *huart);
void NEO6M_Process(uint8_t *data, uint16_t len);
NEO6M_Data_t *NEO6M_GetData(void);


#endif /* SRC_NEO6M_H_ */
