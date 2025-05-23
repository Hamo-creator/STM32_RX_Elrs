/*
 * servo_control.c
 *
 *  Created on: May 6, 2025
 *      Author: Khalil
 */

#include "servo_control.h"

int map(int value, int in_min, int in_max, int out_min, int out_max) {
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void ServoControl_Init(void) {
    // Start all configured PWM channels
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);  // Throttle
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);  // Rudder
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);  // Elevator
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);  // Aileron
}

void ServoControl_Update(const int *channels) {
//    int throttle_us = channels[CHANNEL_THROTTLE];
//    int rudder_us   = channels[CHANNEL_RUDDER];
//    int elevator_us = channels[CHANNEL_ELEVATOR];
//    int aileron_us  = channels[CHANNEL_AILERON];

    int throttle_us = map(channels[CHANNEL_THROTTLE], 172, 1811, 1000, 2000); // 450, 1100
    int rudder_us   = map(channels[CHANNEL_RUDDER], 172, 1811, 1000, 2000);   // 600, 900
    int elevator_us = map(channels[CHANNEL_ELEVATOR], 172, 1811, 1000, 2000);
    int aileron_us  = map(channels[CHANNEL_AILERON], 172, 1811, 1000, 2000);

    // Clamp values to safe servo range
    if (throttle_us < SERVO_MIN_US) throttle_us = SERVO_MIN_US;
    if (throttle_us > SERVO_MAX_US) throttle_us = SERVO_MAX_US;

    if (rudder_us < SERVO_MIN_US) rudder_us = SERVO_MIN_US;
    if (rudder_us > SERVO_MAX_US) rudder_us = SERVO_MAX_US;

    if (elevator_us < SERVO_MIN_US) elevator_us = SERVO_MIN_US;
    if (elevator_us > SERVO_MAX_US) elevator_us = SERVO_MAX_US;

    if (aileron_us < SERVO_MIN_US) aileron_us = SERVO_MIN_US;
    if (aileron_us > SERVO_MAX_US) aileron_us = SERVO_MAX_US;

    // Update PWM pulse widths
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, throttle_us);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, rudder_us);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, elevator_us);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, aileron_us);
}


