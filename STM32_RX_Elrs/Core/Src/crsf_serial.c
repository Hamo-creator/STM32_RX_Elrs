#include "crsf_serial.h"
#include <string.h>

uint8_t uartRxBuf[UART_RX_BUFFER_SIZE];
uint16_t oldPos = 0;

// crc implementation from CRSF protocol document rev7

static uint8_t crsf_crc8tab[256] = {

    0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54, 0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,

    0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06, 0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,

    0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0, 0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,

    0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2, 0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,

    0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9, 0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,

    0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B, 0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,

    0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D, 0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,

    0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F, 0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,

    0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB, 0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,

    0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9, 0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,

    0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F, 0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,

    0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D, 0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,

    0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26, 0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,

    0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74, 0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,

    0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82, 0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,

    0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0, 0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9
};

static uint8_t crsf_crc8(const uint8_t *ptr, uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        crc = crsf_crc8tab[crc ^ *ptr++];
    }
    return crc;
}

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

//static void CrsfSerial_UnpackChannels(CrsfSerial_HandleTypeDef *hcrsf, const uint8_t *payload) {
//    uint32_t bits = 0;
//    uint8_t bitsAvailable = 0;
//    uint8_t ch = 0;
//
//    for (int i = 0; i < 22; i++) { // 22 bytes for 16 channels
//        bits |= ((uint32_t)payload[i]) << bitsAvailable;
//        bitsAvailable += 8;
//
//        while (bitsAvailable >= 11 && ch < CRSF_NUM_CHANNELS) {
//            hcrsf->channels[ch++] = bits & 0x7FF; // extract 11 bits
//            bits >>= 11;
//            bitsAvailable -= 11;
//        }
//    }
//}

static void CrsfSerial_UnpackChannels(CrsfSerial_HandleTypeDef *hcrsf, const uint8_t *payload) {
    uint16_t temp[CRSF_NUM_CHANNELS] = {0};
    temp[0]  = ((payload[0]  >> 0) | (payload[1] << 8)) & 0x07FF;
    temp[1]  = ((payload[1]  >> 3) | (payload[2] << 5)) & 0x07FF;
    temp[2]  = ((payload[2]  >> 6) | (payload[3] << 2) | (payload[4] << 10)) & 0x07FF;
    temp[3]  = ((payload[4]  >> 1) | (payload[5] << 7)) & 0x07FF;
    temp[4]  = ((payload[5]  >> 4) | (payload[6] << 4)) & 0x07FF;
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
    switch (hdr->type) {
        case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
            CrsfSerial_UnpackChannels(hcrsf, hdr->data); // <--- UNPACK channels into hcrsf->channels
            if (hcrsf->onPacketChannels) hcrsf->onPacketChannels();
            hcrsf->lastChannelsPacket = HAL_GetTick();
            hcrsf->linkIsUp = true;
            break;
        case CRSF_FRAMETYPE_LINK_STATISTICS:
            memcpy(&hcrsf->linkStatistics, hdr->data, sizeof(crsfLinkStatistics_t));
            if (hcrsf->onPacketLinkStatistics) hcrsf->onPacketLinkStatistics(&hcrsf->linkStatistics);
            break;
        case CRSF_FRAMETYPE_GPS:
            memcpy(&hcrsf->gpsSensor, hdr->data, sizeof(crsf_sensor_gps_t));
            if (hcrsf->onPacketGps) hcrsf->onPacketGps(&hcrsf->gpsSensor);
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
        }
    }
}

void CrsfSerial_Loop(CrsfSerial_HandleTypeDef *hcrsf) {
    if (hcrsf->linkIsUp && HAL_GetTick() - hcrsf->lastChannelsPacket > CRSF_FAILSAFE_STAGE1_MS) {
        if (hcrsf->onLinkDown) hcrsf->onLinkDown();
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

void CRSF_PrepareTelemetryPacket(uint8_t *packet, uint8_t type, const uint8_t *payload, uint8_t payload_len) {
    packet[0] = ELRS_ADDRESS;               // Receiver address (0xC8)
    packet[1] = payload_len + 2;            // Length = type + payload + CRC
    packet[2] = type;                       // Frame type (GPS, link stats, custom telemetry, etc.)
    memcpy(&packet[3], payload, payload_len);
    packet[3 + payload_len] = crsf_crc8(&packet[2], payload_len + 1); // CRC over type + payload
}

void CrsfSerial_WriteByte(CrsfSerial_HandleTypeDef *hcrsf, uint8_t b) {
	HAL_UART_Transmit_DMA(hcrsf->huart, &b, 1);
}

void CrsfSerial_WriteBuffer(CrsfSerial_HandleTypeDef *hcrsf, const uint8_t *buf, uint16_t len) {
	HAL_UART_Transmit_DMA(hcrsf->huart, (uint8_t *)buf, len);
}

void CrsfSerial_QueuePacket(CrsfSerial_HandleTypeDef *hcrsf, uint8_t type, const void *payload, uint8_t len) {
    if (len > CRSF_MAX_PAYLOAD_LEN) return;
    uint8_t buf[CRSF_MAX_PACKET_SIZE];
    buf[0] = CRSF_SYNC_BYTE;
    buf[1] = len + 2;
    buf[2] = type;
    memcpy(&buf[3], payload, len);
    buf[len + 3] = crc8_dvb_s2(&buf[2], len + 1);
    CrsfSerial_WriteBuffer(hcrsf, buf, len + 4);
}

void CrsfSerial_QueuePacketChannels(CrsfSerial_HandleTypeDef *hcrsf) {
    uint8_t buf[22]; // 11 channels * 11 bits = 242 bits = 30.25 bytes ~ 22 packed bytes
    // TODO: Pack the channel values here properly
    memset(buf, 0, sizeof(buf));
    CrsfSerial_QueuePacket(hcrsf, CRSF_FRAMETYPE_RC_CHANNELS_PACKED, buf, sizeof(buf));
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

