/*
 * crsf_serial.h
 *
 *  Created on: Apr 14, 2025
 *      Author: Khalil
 */

#ifndef CRSF_SERIAL_H
#define CRSF_SERIAL_H

#include <stdint.h>
#include <stdbool.h>
#include "crc8.h"
#include "crsf_protocol.h"
#include "stm32f4xx_hal.h"  // Adjust to your STM32 series if needed

typedef enum {
    FSA_NO_PULSES,
    FSA_HOLD
} eFailsafeAction;

#define CRSF_PACKET_TIMEOUT_MS     100
<<<<<<< HEAD
#define CRSF_FAILSAFE_STAGE1_MS    900	//300

#define TELEMETRY_WINDOW_US   1000   // ~1ms

extern volatile bool telemetry_window_open;
extern volatile uint32_t telemetry_window_deadline;
=======
#define CRSF_FAILSAFE_STAGE1_MS    300	//900

#define TELEMETRY_WINDOW_US   1000   // ~1ms
>>>>>>> baea64b (Update: CRSF parsing and telemetry improvments)

extern uint8_t rx_dma_byte;  // One byte buffer
extern volatile uint8_t crsf_tx_busy;

extern volatile uint8_t g_last_crsf_packet_type;
extern volatile uint8_t raw_type;
extern volatile uint8_t struct_type;

extern volatile uint8_t g_crsf_telemetry_to_send;
extern uint8_t telemetry_tx_buffer[CRSF_MAX_PACKET_SIZE];
extern uint8_t telemetry_tx_len;

#define UART_RX_BUFFER_SIZE 	128
extern uint8_t uartRxBuf[UART_RX_BUFFER_SIZE];

// Callback function types
//typedef void (*CrsfChannelsReceivedCallback_t)(const uint16_t *channels);
//typedef void (*CrsfLinkStatisticsReceivedCallback_t)(const crsfLinkStatistics_t *stats);
typedef void (*CrsfTelemetryPollReceivedCallback_t)(void);

typedef struct {
    UART_HandleTypeDef *huart;
    uint8_t  device_address;            // CRSF address of this device (FC or TX)

    uint8_t rxBuf[CRSF_MAX_PACKET_SIZE];
    uint8_t rxBufPos;
    Crc8_HandleTypeDef crc;
    crsf_sensor_battery_t batterySensor;
    crsfLinkStatistics_t linkStatistics;
    crsf_sensor_gps_t gpsSensor;
    uint32_t baud;
    uint32_t lastReceive;
    uint32_t lastChannelsPacket;
    bool linkIsUp;
    bool telemetry_window_open;
    uint8_t rc_packet_count;

    // Telemetry Queue (for sending telemetry from FC to TX)
    uint8_t telemetry_tx_buffer[CRSF_MAX_PACKET_SIZE]; // Buffer for queued telemetry packet
    uint8_t telemetry_tx_len;           // Length of queued telemetry packet

    // TX State
    volatile bool tx_busy;              // Flag indicating if UART TX is busy

    uint32_t passthroughBaud;
    int channels[CRSF_NUM_CHANNELS];

    // Event Handlers
    void (*onLinkUp)(void);
    void (*onLinkDown)(void);
    void (*onOobData)(uint8_t b);
    void (*onPacketChannels)(void);
    void (*onPacketBattery)(crsf_sensor_battery_t *batterySensor);
    void (*onPacketLinkStatistics)(crsfLinkStatistics_t *ls);
    void (*onPacketGps)(crsf_sensor_gps_t *gpsSensor);
} CrsfSerial_HandleTypeDef;

// Initialization
void CrsfSerial_Init(CrsfSerial_HandleTypeDef *hcrsf, UART_HandleTypeDef *huart, uint32_t baud);

// Usage
void CrsfSerial_Begin(CrsfSerial_HandleTypeDef *hcrsf, uint32_t baud);
void CrsfSerial_Loop(CrsfSerial_HandleTypeDef *hcrsf);
void CrsfSerial_WriteByte(CrsfSerial_HandleTypeDef *hcrsf, uint8_t b);
//void CrsfSerial_WriteBuffer(CrsfSerial_HandleTypeDef *hcrsf, const uint8_t *buf, uint16_t len);
//void CrsfSerial_QueuePacket(CrsfSerial_HandleTypeDef *hcrsf, uint8_t type, const void *payload, uint8_t len);
uint8_t CrsfSerial_WriteBuffer(CrsfSerial_HandleTypeDef *hcrsf, const uint8_t *buf, uint16_t len);
uint8_t CrsfSerial_QueuePacket(CrsfSerial_HandleTypeDef *hcrsf, uint8_t type, const void *payload, uint8_t len);
void CrsfSerial_QueuePacketChannels(CrsfSerial_HandleTypeDef *hcrsf);

/**
 * @brief Prepares and queues a telemetry sensor packet for transmission.
 *        Typically used on the Flight Controller side in response to a poll.
 * @param handle Pointer to the CRSF handle structure.
 * @param type The CRSF frame type (e.g., CRSF_FRAMETYPE_BATTERY_SENSOR).
 * @param payload Pointer to the payload data.
 * @param payload_len Length of the payload data.
 * @return HAL_StatusTypeDef HAL_OK if packet is queued, HAL_BUSY if TX is busy.
 */
HAL_StatusTypeDef Crsf_QueueTelemetryPacket(CrsfSerial_HandleTypeDef *hcrsf, uint8_t type, const void *payload, uint8_t payload_len);
/**
 * @brief Low-level function to start a CRSF packet transmission.
 *        Handles busy state and half-duplex switching.
 * @param handle Pointer to the CRSF handle structure.
 * @param packet Pointer to the complete CRSF packet (including address, length, type, payload, CRC).
 * @param packet_len Total length of the packet.
 * @return HAL_StatusTypeDef HAL_OK if transmission started, HAL_BUSY if UART TX is busy.
 */
HAL_StatusTypeDef Crsf_TransmitPacket(CrsfSerial_HandleTypeDef *hcrsf, const uint8_t *packet, uint8_t packet_len);

// Accessors
int CrsfSerial_GetChannel(CrsfSerial_HandleTypeDef *hcrsf, unsigned int ch);
void CrsfSerial_SetChannel(CrsfSerial_HandleTypeDef *hcrsf, unsigned int ch, unsigned int value_us);
const crsfLinkStatistics_t *CrsfSerial_GetLinkStatistics(CrsfSerial_HandleTypeDef *hcrsf);
const crsf_sensor_gps_t *CrsfSerial_GetGpsSensor(CrsfSerial_HandleTypeDef *hcrsf);
bool CrsfSerial_IsLinkUp(CrsfSerial_HandleTypeDef *hcrsf);
bool CrsfSerial_GetPassthroughMode(CrsfSerial_HandleTypeDef *hcrsf);
void CrsfSerial_SetPassthroughMode(CrsfSerial_HandleTypeDef *hcrsf, bool val, uint32_t passthroughBaud);

#endif // CRSF_SERIAL_H

