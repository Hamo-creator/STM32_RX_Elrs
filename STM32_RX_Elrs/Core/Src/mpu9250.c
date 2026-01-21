/*
 * mpu9250.c
 *
 *  Created on: Oct 13, 2025
 *      Author: Khalil
 */
#include "mpu9250.h"
//#include "stm32f4xx_hal.h"   // or your MCU’s HAL include

// Structures
struct RawData rawData;

struct SensorData sensorData;

struct GyroCal gyroCal;

struct Attitude attitude;

// Variables
uint8_t dev_addr;
float _dt, _tau;
float aScaleFactor, gScaleFactor;

// Initialize the MPU9250 sensor
// Make sure these are defined correctly at the top of your file

uint8_t mpu_init(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    uint8_t check;
    uint8_t data;

    aScaleFactor = 8192.0f;
    gScaleFactor = 65.5f;
    _tau = 0.98f;
    _dt  = 0.004f;

    // ----------------------------------------------------------------------
    // Step 0: Define the device address in the 8-bit format HAL expects.
    // This is the 7-bit address (0x68) shifted left by one.
    // ----------------------------------------------------------------------
    dev_addr = MPU9250_ADDR_AD0_LOW << 1;

    // ----------------------------------------------------------------------
    // Step 1: Check if the device is ready on the bus.
    // This is the same call that worked in your scanner.
    // ----------------------------------------------------------------------
    status = HAL_I2C_IsDeviceReady(hi2c, dev_addr, 2, 100);
    if (status != HAL_OK) {
        return 200; // Return a unique code for "Device Not Ready"
    }

    // ----------------------------------------------------------------------
    // Step 2: Check the WHO_AM_I register.
    // Use the exact same 'dev_addr' for this call.
    // ----------------------------------------------------------------------
    status = HAL_I2C_Mem_Read(hi2c, dev_addr, MPU9250_WHO_AM_I,
                              I2C_MEMADD_SIZE_8BIT, &check, 1, 100);

    if (status != HAL_OK) {
        return 201; // Return a unique code for "WHO_AM_I Read Failed"
    }

    if (check != MPU9255_WHO_AM_I_ANS) {
        // If the ID is wrong, return the wrong ID you received.
        // This is where you were getting 112 before.
        return check;
    }

/*    if (check != MPU9250_WHO_AM_I_ANS) {
        // If the ID is wrong, return the wrong ID you received.
        // This is where you were getting 112 before.
        return check;
    }*/

    // If you get past this point, communication is working!

    // ----------------------------------------------------------------------
    // Step 3: Wake up the sensor by writing 0 to the power management register.
    // ----------------------------------------------------------------------
    data = 0x00; // Clear sleep bit
    status = HAL_I2C_Mem_Write(hi2c, dev_addr, MPU9250_PWR_MGMT_1,
                               I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    if (status != HAL_OK) {
        return 202; // Return code for "Wake Up Failed"
    }

    // A small delay is good practice after power state changes.
    HAL_Delay(10);

    // --- All subsequent configuration writes ---
    // Use the same 'dev_addr' for all of them.

    // Configure accelerometer (e.g., ±4g)
    data = 0x08;
    HAL_I2C_Mem_Write(hi2c, dev_addr, MPU9250_ACCEL_CONFIG, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);

    // Configure gyroscope (e.g., ±500 °/s)
    data = 0x08;
    HAL_I2C_Mem_Write(hi2c, dev_addr, MPU9250_GYRO_CONFIG, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);

    // ... add other configurations as needed ...

    // ----------------------------------------------------------------------
    // If all steps complete, return 1 for success.
    // ----------------------------------------------------------------------
    return 1;
}
/// @brief Set the accelerometer full scale range.
/// @param I2Cx Pointer to I2C structure config.
/// @param aScale Set 0 for ±2g, 1 for ±4g, 2 for ±8g, and 3 for ±16g.
void MPU_writeAccFullScaleRange(I2C_HandleTypeDef *I2Cx, uint8_t aScale)
{
    // Variable init
    uint8_t select;

    // Set the value
    switch (aScale)
    {
    case AFSR_2G:
        aScaleFactor = 16384.0;
        select = 0x00;
        HAL_I2C_Mem_Write(I2Cx, dev_addr, MPU9250_ACCEL_CONFIG, 1, &select, 1, I2C_TIMOUT_MS);
        break;
    case AFSR_4G:
        aScaleFactor = 8192.0;
        select = 0x08;
        HAL_I2C_Mem_Write(I2Cx, dev_addr, MPU9250_ACCEL_CONFIG, 1, &select, 1, I2C_TIMOUT_MS);
        break;
    case AFSR_8G:
        aScaleFactor = 4096.0;
        select = 0x10;
        HAL_I2C_Mem_Write(I2Cx, dev_addr, MPU9250_ACCEL_CONFIG, 1, &select, 1, I2C_TIMOUT_MS);
        break;
    case AFSR_16G:
        aScaleFactor = 2048.0;
        select = 0x18;
        HAL_I2C_Mem_Write(I2Cx, dev_addr, MPU9250_ACCEL_CONFIG, 1, &select, 1, I2C_TIMOUT_MS);
        break;
    default:
        aScaleFactor = 8192.0;
        select = 0x08;
        HAL_I2C_Mem_Write(I2Cx, dev_addr, MPU9250_ACCEL_CONFIG, 1, &select, 1, I2C_TIMOUT_MS);
        break;
    }
}

/// @brief Set the gyroscope full scale range.
/// @param I2Cx Pointer to I2C structure config.
/// @param gScale Set 0 for ±250°/s, 1 for ±500°/s, 2 for ±1000°/s, and 3 for ±2000°/s.
void MPU_writeGyroFullScaleRange(I2C_HandleTypeDef *I2Cx, uint8_t gScale)
{
    // Variable init
    uint8_t select;

    // Set the value
    switch (gScale)
    {
    case GFSR_250DPS:
        gScaleFactor = 131.0;
        select = 0x00;
        HAL_I2C_Mem_Write(I2Cx, dev_addr, MPU9250_GYRO_CONFIG, 1, &select, 1, I2C_TIMOUT_MS);
        break;
    case GFSR_500DPS:
        gScaleFactor = 65.5;
        select = 0x08;
        HAL_I2C_Mem_Write(I2Cx, dev_addr, MPU9250_GYRO_CONFIG, 1, &select, 1, I2C_TIMOUT_MS);
        break;
    case GFSR_1000DPS:
        gScaleFactor = 32.8;
        select = 0x10;
        HAL_I2C_Mem_Write(I2Cx, dev_addr, MPU9250_GYRO_CONFIG, 1, &select, 1, I2C_TIMOUT_MS);
        break;
    case GFSR_2000DPS:
        gScaleFactor = 16.4;
        select = 0x18;
        HAL_I2C_Mem_Write(I2Cx, dev_addr, MPU9250_GYRO_CONFIG, 1, &select, 1, I2C_TIMOUT_MS);
        break;
    default:
        gScaleFactor = 65.5;
        select = 0x08;
        HAL_I2C_Mem_Write(I2Cx, dev_addr, MPU9250_GYRO_CONFIG, 1, &select, 1, I2C_TIMOUT_MS);
        break;
    }
}

/// @brief Read raw data from IMU.
/// @param I2Cx Pointer to I2C structure config.
void MPU_readRawData(I2C_HandleTypeDef *I2Cx)
{
    // Init buffer
    uint8_t buf[14];

    // Subroutine for reading the raw data
    HAL_I2C_Mem_Read(I2Cx, dev_addr, MPU9250_ACCEL_XOUT_H, 1, buf, 14, I2C_TIMOUT_MS);

    // Bit shift the data
    rawData.ax = buf[0] << 8 | buf[1];
    rawData.ay = buf[2] << 8 | buf[3];
    rawData.az = buf[4] << 8 | buf[5];
    // temperature = buf[6] << 8 | buf[7];
    rawData.gx = buf[8] << 8 | buf[9];
    rawData.gy = buf[10] << 8 | buf[11];
    rawData.gz = buf[12] << 8 | buf[13];
}

/// @brief Find offsets for each axis of gyroscope.
/// @param I2Cx Pointer to I2C structure config.
/// @param numCalPoints Number of data points to average.
void MPU_calibrateGyro(I2C_HandleTypeDef *I2Cx, uint16_t numCalPoints)
{
    // Init
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;

    // Zero guard
    if (numCalPoints == 0)
    {
        numCalPoints = 1;
    }

    // Save specified number of points
    for (uint16_t ii = 0; ii < numCalPoints; ii++)
    {
        MPU_readRawData(I2Cx);
        x += rawData.gx;
        y += rawData.gy;
        z += rawData.gz;
        HAL_Delay(3);
    }

    // Average the saved data points to find the gyroscope offset
    gyroCal.x = (float)x / (float)numCalPoints;
    gyroCal.y = (float)y / (float)numCalPoints;
    gyroCal.z = (float)z / (float)numCalPoints;
}

/// @brief Calculate the real world sensor values.
/// @param I2Cx Pointer to I2C structure config.
void MPU_readProcessedData(I2C_HandleTypeDef *I2Cx)
{
    // Get raw values from the IMU
    MPU_readRawData(I2Cx);

    // Convert accelerometer values to g's
    sensorData.ax = rawData.ax / aScaleFactor;
    sensorData.ay = rawData.ay / aScaleFactor;
    sensorData.az = rawData.az / aScaleFactor;

    // Compensate for gyro offset
    sensorData.gx = rawData.gx - gyroCal.x;
    sensorData.gy = rawData.gy - gyroCal.y;
    sensorData.gz = rawData.gz - gyroCal.z;

    // Convert gyro values to deg/s
    sensorData.gx /= gScaleFactor;
    sensorData.gy /= gScaleFactor;
    sensorData.gz /= gScaleFactor;
}

/// @brief Calculate the attitude of the sensor in degrees using a complementary filter.
/// @param I2Cx Pointer to I2C structure config.
void MPU_calcAttitude(I2C_HandleTypeDef *I2Cx)
{
    // Read processed data
    MPU_readProcessedData(I2Cx);

    // Complementary filter
    float accelPitch = atan2(sensorData.ay, sensorData.az) * RAD2DEG;
    float accelRoll = atan2(sensorData.ax, sensorData.az) * RAD2DEG;

    attitude.r = _tau * (attitude.r - sensorData.gy * _dt) + (1 - _tau) * accelRoll;
    attitude.p = _tau * (attitude.p + sensorData.gx * _dt) + (1 - _tau) * accelPitch;
    attitude.y += sensorData.gz * _dt;
}

