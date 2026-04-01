/*
 * neo6m.c
 *
 *  Created on: Oct 26, 2025
 *      Author: Khalil
 */

#include "neo6m.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define NMEA_MAX_LEN 128

static UART_HandleTypeDef *gps_uart;
static NEO6M_Data_t gps_data;
static char nmea_buf[NMEA_MAX_LEN];
static uint16_t nmea_index = 0;

static float nmea_to_decimal(float nmea, char dir);

void NEO6M_Init(UART_HandleTypeDef *huart)
{
    gps_uart = huart;
    memset(&gps_data, 0, sizeof(gps_data));
}

void NEO6M_Process(uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        char c = data[i];

        if (c == '\n') {
            nmea_buf[nmea_index] = '\0';
            nmea_index = 0;

            if (strstr(nmea_buf, "$GPGGA") != NULL)
            {
                // Example: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
                char *token;
                char *ptr = nmea_buf;
                uint8_t field = 0;

                while ((token = strsep(&ptr, ",")) != NULL)
                {
                    field++;
                    switch (field)
                    {
                    case 3: // latitude
                        gps_data.latitude = nmea_to_decimal(atof(token), 'N'); // placeholder dir
                        break;
                    case 4: // N/S
                        gps_data.latitude = nmea_to_decimal(atof(((char *)strchr(nmea_buf, ',') + 1)), token[0]);
                        break;
                    case 5: // longitude
                        gps_data.longitude = nmea_to_decimal(atof(token), 'E');
                        break;
                    case 6: // E/W
                        gps_data.longitude = nmea_to_decimal(atof(((char *)strchr(nmea_buf, ',') + 1)), token[0]);
                        break;
                    case 7: // fix quality (0 = invalid, 1 = GPS fix)
                        gps_data.fix = atoi(token);
                        break;
                    case 8: // number of satellites
                        gps_data.num_sats = atoi(token);
                        break;
                    case 10: // altitude
                        gps_data.altitude = atof(token);
                        break;
                    default:
                        break;
                    }
                }
            }
            else if (strstr(nmea_buf, "$GPRMC") != NULL)
            {
                // Example: $GPRMC,hhmmss.sss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a*hh
                char *token;
                char *ptr = nmea_buf;
                uint8_t field = 0;
                char lat_dir = 'N', lon_dir = 'E';

                float lat = 0, lon = 0, spd = 0;

                while ((token = strsep(&ptr, ",")) != NULL)
                {
                    field++;
                    switch (field)
                    {
                    case 3:
                        lat = atof(token);
                        break;
                    case 4:
                        lat_dir = token[0];
                        break;
                    case 5:
                        lon = atof(token);
                        break;
                    case 6:
                        lon_dir = token[0];
                        break;
                    case 8: // speed in knots
                        spd = atof(token);
                        break;
                    default:
                        break;
                    }
                }

                gps_data.latitude = nmea_to_decimal(lat, lat_dir);
                gps_data.longitude = nmea_to_decimal(lon, lon_dir);
                gps_data.speed = spd * 1.852f; // convert knots to km/h
            }
        }
        else if (c == '$') {
            nmea_index = 0;
            nmea_buf[nmea_index++] = c;
        }
        else if (nmea_index < NMEA_MAX_LEN - 1) {
            nmea_buf[nmea_index++] = c;
        }
    }
}

NEO6M_Data_t *NEO6M_GetData(void)
{
    return &gps_data;
}

/* Convert NMEA degrees-minutes to decimal degrees */
static float nmea_to_decimal(float nmea, char dir)
{
    int degrees = (int)(nmea / 100);
    float minutes = nmea - (degrees * 100);
    float decimal = degrees + minutes / 60.0f;

    if (dir == 'S' || dir == 'W')
        decimal = -decimal;

    return decimal;
}


