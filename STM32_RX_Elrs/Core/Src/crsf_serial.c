#include "crsf_serial.h"
#include <string.h>
#include "gpio.h"

extern uint8_t uartRxBuf[];
extern uint16_t oldPos;

void CrsfSerial_Init(CrsfSerial_HandleTypeDef *hcrsf, UART_HandleTypeDef *huart, uint32_t baud) {
    hcrsf->huart = huart;
    hcrsf->baud = baud;
    hcrsf->rxBufPos = 0;
    hcrsf->lastReceive = 0;
    hcrsf->lastChannelsPacket = 0;
    hcrsf->linkIsUp = false;
    hcrsf->passthroughBaud = 0;

    memset(hcrsf->rxBuf, 0, sizeof(hcrsf->rxBuf));
    memset(hcrsf->channels, 0, sizeof(hcrsf->channels));

    hcrsf->onLinkDown = NULL;
    hcrsf->onLinkUp = NULL;
    hcrsf->onOobData = NULL;
    hcrsf->onPacketChannels = NULL;
    hcrsf->onPacketBattery = NULL;
    hcrsf->onPacketLinkStatistics = NULL;
    hcrsf->onPacketGps = NULL;
}

void CrsfSerial_Begin(CrsfSerial_HandleTypeDef *hcrsf, uint32_t baud) {
    hcrsf->baud = baud ? baud : hcrsf->baud;
    // HAL_UART_Init must be called elsewhere with correct parameters
}

static void ShiftBuffer(CrsfSerial_HandleTypeDef *hcrsf, uint8_t cnt) {
    if (cnt >= hcrsf->rxBufPos) {
        hcrsf->rxBufPos = 0;
        return;
    }
    memmove(hcrsf->rxBuf, hcrsf->rxBuf + cnt, hcrsf->rxBufPos - cnt);
    hcrsf->rxBufPos -= cnt;
}

static void CrsfSerial_UnpackChannels(CrsfSerial_HandleTypeDef *hcrsf, const uint8_t *payload) {
    uint16_t temp[CRSF_NUM_CHANNELS] = {0};
    temp[0]  = ((payload[0]  >> 0) | (payload[1] << 8)) & 0x07FF;
    temp[1]  = ((payload[1]  >> 3) | (payload[2] << 5)) & 0x07FF;
    temp[2]  = ((payload[2]  >> 6) | (payload[3] << 2) | (payload[4] << 10)) & 0x07FF;
    temp[3]  = ((payload[4]  >> 1) | (payload[5] << 7)) & 0x07FF;
    temp[4]  = ((payload[5]  >> 4) | (payload[6] << 4)) & 0x07FF;
    // Corrected line for temp[4]
    //temp[4]  = ((payload[5] >> 4) | ((payload[6] & 0x7F) << 4)) & 0x07FF;
    temp[5]  = ((payload[6]  >> 7) | (payload[7] << 1) | (payload[8] << 9)) & 0x07FF;
    temp[6]  = ((payload[8]  >> 2) | (payload[9] << 6)) & 0x07FF;
    temp[7]  = ((payload[9]  >> 5) | (payload[10] << 3)) & 0x07FF;
    temp[8]  = ((payload[11] >> 0) | (payload[12] << 8)) & 0x07FF;
    temp[9]  = ((payload[12] >> 3) | (payload[13] << 5)) & 0x07FF;
    temp[10] = ((payload[13] >> 6) | (payload[14] << 2) | (payload[15] << 10)) & 0x07FF;
    temp[11] = ((payload[15] >> 1) | (payload[16] << 7)) & 0x07FF;
    temp[12] = ((payload[16] >> 4) | (payload[17] << 4)) & 0x07FF;
    temp[13] = ((payload[17] >> 7) | (payload[18] << 1) | (payload[19] << 9)) & 0x07FF;
    temp[14] = ((payload[19] >> 2) | (payload[20] << 6)) & 0x07FF;
    temp[15] = ((payload[20] >> 5) | (payload[21] << 3)) & 0x07FF;

    for (int i = 0; i < CRSF_NUM_CHANNELS; i++) {
        hcrsf->channels[i] = temp[i];
    }
}



static void HandlePacket(CrsfSerial_HandleTypeDef *hcrsf, uint8_t len) {
    crsf_header_t *hdr = (crsf_header_t *)hcrsf->rxBuf;
    raw_type = hcrsf->rxBuf[2]; // Index 2 is always the type
    struct_type = hdr->type;

//    switch (packet_type) {
    switch (hdr->type) {
        case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
            CrsfSerial_UnpackChannels(hcrsf, hdr->data); // <--- UNPACK channels into hcrsf->channels
            if (hcrsf->onPacketChannels) hcrsf->onPacketChannels();
            hcrsf->lastChannelsPacket = HAL_GetTick();
            hcrsf->linkIsUp = true;
<<<<<<< HEAD
            /* 🔑 RC frame edge for FC */
            telemetry_window_open = true;
            //telemetry_window_deadline = micros() + TELEMETRY_WINDOW_US;
=======
            hcrsf->telemetry_window_open = true;
            hcrsf->rc_packet_count++;
            /* 🔑 RC frame edge for FC */
>>>>>>> baea64b (Update: CRSF parsing and telemetry improvments)
            break;
        case CRSF_FRAMETYPE_LINK_STATISTICS:
            memcpy(&hcrsf->linkStatistics, hdr->data, sizeof(crsfLinkStatistics_t));
            if (hcrsf->onPacketLinkStatistics) hcrsf->onPacketLinkStatistics(&hcrsf->linkStatistics);
            break;
<<<<<<< HEAD
        case CRSF_FRAMETYPE_GPS:
            memcpy(&hcrsf->gpsSensor, hdr->data, sizeof(crsf_sensor_gps_t));
            if (hcrsf->onPacketGps) hcrsf->onPacketGps(&hcrsf->gpsSensor);
            break;
        case CRSF_FRAMETYPE_BATTERY_SENSOR:
            memcpy(&hcrsf->batterySensor, hdr->data, sizeof(crsf_sensor_battery_t));
            if (hcrsf->onPacketBattery) hcrsf->onPacketBattery(&hcrsf->batterySensor);
<<<<<<< HEAD
            hcrsf->linkIsUp = true;
=======
>>>>>>> 363fa39 (telemetry send upload)
            break;
        case CRSF_FRAMETYPE_COMMAND:
            // The transmitter has sent a command. For ELRS, this is often a poll.
            // The first byte of the data is the extended destination/source.
            // The second byte (hdr->data[1]) is the actual command ID.
            // For a simple poll, the packet type 0x28 is all we need.
            // This is typically a telemetry poll from the TX
<<<<<<< HEAD
//            if (hcrsf->telemetry_tx_len > 0) {
//                // Signal main loop to send the queued telemetry
//            	hcrsf->telemetry_poll_received = true;
//          	    HAL_GPIO_TogglePin(DPIN_LED_GPIO_Port, DPIN_LED_Pin);
//            }
            /* 🔑 RC frame edge for FC */
            telemetry_window_open = true;
            //telemetry_window_deadline = micros() + TELEMETRY_WINDOW_US;
=======
            if (hcrsf->telemetry_tx_len > 0) {
                // Signal main loop to send the queued telemetry
            	hcrsf->telemetry_poll_received = true;
          	    HAL_GPIO_TogglePin(DPIN_LED_GPIO_Port, DPIN_LED_Pin);
            }
>>>>>>> 363fa39 (telemetry send upload)
=======
        case CRSF_FRAMETYPE_COMMAND:
            /* 🔑 RC frame edge for FC */
>>>>>>> baea64b (Update: CRSF parsing and telemetry improvments)
            break;
    }
}

//static void ProcessByte(CrsfSerial_HandleTypeDef *hcrsf, uint8_t b) {
void ProcessByte(CrsfSerial_HandleTypeDef *hcrsf, uint8_t b) {
    hcrsf->rxBuf[hcrsf->rxBufPos++] = b;
    if (hcrsf->rxBufPos >= 2) {
        uint8_t len = hcrsf->rxBuf[1];
        if (len >= 3 && hcrsf->rxBufPos >= len + 2) {
            uint8_t crc = crc8_dvb_s2(hcrsf->rxBuf + 2, len - 1);
            if (crc == hcrsf->rxBuf[len + 1]) {
                HandlePacket(hcrsf, len);
                ShiftBuffer(hcrsf, len + 2);
            } else {
                ShiftBuffer(hcrsf, 1);
            }
        } /*else if (len > CRSF_MAX_PACKET_SIZE) {
            // Packet too long, discard
            ShiftBuffer(hcrsf, 1); // Discard first byte and try again
        }*/
    }
}

void CrsfSerial_Loop(CrsfSerial_HandleTypeDef *hcrsf) {
//    uint8_t b;
//    while (HAL_UART_Receive(hcrsf->huart, &b, 1, 0) == HAL_OK) {
//        hcrsf->lastReceive = HAL_GetTick();
//        if (CrsfSerial_GetPassthroughMode(hcrsf)) {
//            if (hcrsf->onOobData) hcrsf->onOobData(b);
//        } else {
//            ProcessByte(hcrsf, b);
//        }
//    }
//    while (HAL_UART_Receive_DMA(hcrsf->huart, &b, 1) == HAL_OK) {
//        hcrsf->lastReceive = HAL_GetTick();
//        if (CrsfSerial_GetPassthroughMode(hcrsf)) {
//            if (hcrsf->onOobData) hcrsf->onOobData(b);
//        } else {
//            ProcessByte(hcrsf, b);
//        }
//    }
    if (hcrsf->linkIsUp && HAL_GetTick() - hcrsf->lastChannelsPacket > CRSF_FAILSAFE_STAGE1_MS) {
        if (hcrsf->onLinkDown){
        	hcrsf->onLinkDown();
        }
        hcrsf->linkIsUp = false;
    }
}

void CrsfSerial_UART_IdleCallback(CrsfSerial_HandleTypeDef *hcrsf)
{
    uint16_t dmaPos = UART_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(hcrsf->huart->hdmarx);
    uint16_t len;

    if (dmaPos != oldPos) {
        if (dmaPos > oldPos) {
            len = dmaPos - oldPos;
            for (uint16_t i = 0; i < len; i++) {
                ProcessByte(hcrsf, uartRxBuf[oldPos + i]);
            }
        } else {
            len = UART_RX_BUFFER_SIZE - oldPos;
            for (uint16_t i = 0; i < len; i++) {
                ProcessByte(hcrsf, uartRxBuf[oldPos + i]);
            }
            for (uint16_t i = 0; i < dmaPos; i++) {
                ProcessByte(hcrsf, uartRxBuf[i]);
            }
        }

        oldPos = dmaPos;
        if (oldPos >= UART_RX_BUFFER_SIZE) oldPos = 0;
    }
}

void CrsfSerial_WriteByte(CrsfSerial_HandleTypeDef *hcrsf, uint8_t b) {
	HAL_UART_Transmit_DMA(hcrsf->huart, &b, 1);
}

//void CrsfSerial_WriteBuffer(CrsfSerial_HandleTypeDef *hcrsf, const uint8_t *buf, uint16_t len) {
uint8_t CrsfSerial_WriteBuffer(CrsfSerial_HandleTypeDef *hcrsf, const uint8_t *buf, uint16_t len) {
	//return HAL_UART_Transmit_DMA(hcrsf->huart, (uint8_t *)buf, len);

    // Enable the Transmission Complete interrupt
    __HAL_UART_ENABLE_IT(hcrsf->huart, UART_IT_TC);
    return HAL_UART_Transmit_DMA(hcrsf->huart, (uint8_t*)buf, len);

}

//void CrsfSerial_QueuePacket(CrsfSerial_HandleTypeDef *hcrsf, uint8_t type, const void *payload, uint8_t len) {
uint8_t CrsfSerial_QueuePacket(CrsfSerial_HandleTypeDef *hcrsf, uint8_t type, const void *payload, uint8_t len) {
    if (len > CRSF_MAX_PAYLOAD_LEN) return 4;
    // If another packet is already waiting, return an error.
    // (A more advanced queue would handle this better).

<<<<<<< HEAD
    uint8_t buf[CRSF_MAX_PACKET_SIZE];
=======
    uint8_t *buf = hcrsf->telemetry_tx_buffer;
>>>>>>> baea64b (Update: CRSF parsing and telemetry improvments)
    buf[0] = RADIO_ADDRESS;	//CRSF_SYNC_BYTE;
    buf[1] = len + 2;
    buf[2] = type;
    memcpy(&buf[3], payload, len);
    buf[len + 3] = crc8_dvb_s2(&buf[2], len + 1);
    return CrsfSerial_WriteBuffer(hcrsf, buf, len + 4);
}

void CrsfSerial_QueuePacketChannels(CrsfSerial_HandleTypeDef *hcrsf) {
    uint8_t buf[22]; // 11 channels * 11 bits = 242 bits = 30.25 bytes ~ 22 packed bytes
    // TODO: Pack the channel values here properly
    memset(buf, 0, sizeof(buf));
    CrsfSerial_QueuePacket(hcrsf, CRSF_FRAMETYPE_RC_CHANNELS_PACKED, buf, sizeof(buf));
}

HAL_StatusTypeDef Crsf_QueueTelemetryPacket(CrsfSerial_HandleTypeDef *hcrsf, uint8_t type, const void *payload, uint8_t payload_len) {
    if (payload_len > CRSF_MAX_PAYLOAD_LEN) {
        return HAL_ERROR; // Payload too large
    }
    if (hcrsf->telemetry_tx_len > 0) {
        return HAL_BUSY; // Queue is full (only one packet at a time for now)
    }

    uint8_t total_len = payload_len + 2; // Type + Payload + CRC

    hcrsf->telemetry_tx_buffer[0] = hcrsf->device_address; // Source address
    hcrsf->telemetry_tx_buffer[1] = total_len;
    hcrsf->telemetry_tx_buffer[2] = type;
    memcpy(&hcrsf->telemetry_tx_buffer[3], payload, payload_len);
    hcrsf->telemetry_tx_buffer[total_len + 1] = crc8_dvb_s2(&hcrsf->telemetry_tx_buffer[2], payload_len + 1);

    hcrsf->telemetry_tx_len = total_len + 2; // Store total frame length

    return HAL_OK; // Packet successfully queued
}

HAL_StatusTypeDef Crsf_TransmitPacket(CrsfSerial_HandleTypeDef *hcrsf, const uint8_t *packet, uint8_t packet_len) {
    if (hcrsf->tx_busy) {
        return HAL_BUSY;
    }
    hcrsf->tx_busy = true;

    // Clear TC flag before starting transmission to prevent immediate interrupt
    __HAL_UART_CLEAR_FLAG(hcrsf->huart, UART_FLAG_TC);

    // Enable TC interrupt to know when transmission is truly complete
    __HAL_UART_ENABLE_IT(hcrsf->huart, UART_IT_TC);

    return HAL_UART_Transmit_DMA(hcrsf->huart, (uint8_t*)packet, packet_len);
}

int CrsfSerial_GetChannel(CrsfSerial_HandleTypeDef *hcrsf, unsigned int ch) {
    return ch < CRSF_NUM_CHANNELS ? hcrsf->channels[ch] : 0;
}

void CrsfSerial_SetChannel(CrsfSerial_HandleTypeDef *hcrsf, unsigned int ch, unsigned int value_us) {
    if (ch < CRSF_NUM_CHANNELS)
        hcrsf->channels[ch] = value_us;
}

const crsfLinkStatistics_t *CrsfSerial_GetLinkStatistics(CrsfSerial_HandleTypeDef *hcrsf) {
    return &hcrsf->linkStatistics;
}

const crsf_sensor_gps_t *CrsfSerial_GetGpsSensor(CrsfSerial_HandleTypeDef *hcrsf) {
    return &hcrsf->gpsSensor;
}

bool CrsfSerial_IsLinkUp(CrsfSerial_HandleTypeDef *hcrsf) {
    return hcrsf->linkIsUp;
}

bool CrsfSerial_GetPassthroughMode(CrsfSerial_HandleTypeDef *hcrsf) {
    return hcrsf->passthroughBaud != 0;
}

void CrsfSerial_SetPassthroughMode(CrsfSerial_HandleTypeDef *hcrsf, bool val, uint32_t passthroughBaud) {
    if (val) hcrsf->passthroughBaud = passthroughBaud ? passthroughBaud : hcrsf->baud;
    else hcrsf->passthroughBaud = 0;
}

