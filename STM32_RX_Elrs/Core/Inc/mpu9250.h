/*
 * mpu9250.h
 *
 *  Created on: Oct 13, 2025
 *      Author: Khalil
 */

#ifndef MPU9250_H_
#define MPU9250_H_

#include "stm32f4xx_hal.h"   // or your specific STM32 HAL header
#include <math.h>

#define MPU9250_ADDR_AD0_LOW   0x68
#define MPU9250_WHO_AM_I       0x75
#define MPU9250_WHO_AM_I_ANS   0x71
#define MPU9255_WHO_AM_I_ANS   0x70
#define MPU9250_PWR_MGMT_1     0x6B
// Add other register defines here...
#define MPU9250_ACCEL_CONFIG   0x1C
#define MPU9250_GYRO_CONFIG    0x1B
#define MPU9250_SMPLRT_DIV     0x19
#define MPU9250_CONFIG         0x1A
#define I2C_TIMOUT_MS		   1000

// Register map
#define MPU9250_SELF_TEST_X_GYRO   0x00
#define MPU9250_SELF_TEST_Y_GYRO   0x01
#define MPU9250_SELF_TEST_Z_GYRO   0x02
#define MPU9250_SELF_TEST_X_ACCEL  0x0D
#define MPU9250_SELF_TEST_Y_ACCEL  0x0E
#define MPU9250_SELF_TEST_Z_ACCEL  0x0F
#define MPU9250_XG_OFFSET_H        0x13
#define MPU9250_XG_OFFSET_L        0x14
#define MPU9250_YG_OFFSET_H        0x15
#define MPU9250_YG_OFFSET_L        0x16
#define MPU9250_ZG_OFFSET_H        0x17
#define MPU9250_ZG_OFFSET_L        0x18

#define MPU9250_ACCEL_CONFIG2      0x1D

#define MPU9250_INT_PIN_CFG        0x37
#define MPU9250_INT_ENABLE         0x38
#define MPU9250_INT_STATUS         0x3A

#define MPU9250_ACCEL_XOUT_H       0x3B
#define MPU9250_ACCEL_XOUT_L       0x3C
#define MPU9250_ACCEL_YOUT_H       0x3D
#define MPU9250_ACCEL_YOUT_L       0x3E
#define MPU9250_ACCEL_ZOUT_H       0x3F
#define MPU9250_ACCEL_ZOUT_L       0x40
#define MPU9250_TEMP_OUT_H         0x41
#define MPU9250_TEMP_OUT_L         0x42
#define MPU9250_GYRO_XOUT_H        0x43
#define MPU9250_GYRO_XOUT_L        0x44
#define MPU9250_GYRO_YOUT_H        0x45
#define MPU9250_GYRO_YOUT_L        0x46
#define MPU9250_GYRO_ZOUT_H        0x47
#define MPU9250_GYRO_ZOUT_L        0x48

#define MPU9250_PWR_MGMT_2         0x6C


#define MPU9250_USER_CTRL          0x6A
#define MPU9250_FIFO_EN            0x23
#define MPU9250_SIGNAL_PATH_RESET  0x68

// Constants
#define RAD2DEG 57.2957795131

// Full scale ranges
enum gyroscopeFullScaleRange
{
    GFSR_250DPS,
    GFSR_500DPS,
    GFSR_1000DPS,
    GFSR_2000DPS
};

enum accelerometerFullScaleRange
{
    AFSR_2G,
    AFSR_4G,
    AFSR_8G,
    AFSR_16G
};

// --- Struct definitions ---
struct RawData
{
    int16_t ax, ay, az, gx, gy, gz;
};

struct SensorData
{
    float ax, ay, az, gx, gy, gz;
};

struct GyroCal
{
    float x, y, z;
};

struct Attitude
{
    float r, p, y;
};

// ---- Extern variable declarations ----
extern struct RawData rawData;
extern struct SensorData sensorData;
extern struct GyroCal gyroCal;
extern struct Attitude attitude;

// Function prototype
uint8_t mpu_init(I2C_HandleTypeDef *hi2c);
void MPU_calibrateGyro(I2C_HandleTypeDef *I2Cx, uint16_t numCalPoints);
void MPU_calcAttitude(I2C_HandleTypeDef *I2Cx);
void MPU_readRawData(I2C_HandleTypeDef *I2Cx);
void MPU_readProcessedData(I2C_HandleTypeDef *I2Cx);
void MPU_writeGyroFullScaleRange(I2C_HandleTypeDef *I2Cx, uint8_t gScale);
void MPU_writeAccFullScaleRange(I2C_HandleTypeDef *I2Cx, uint8_t aScale);

#endif /* MPU9250_H_ */
