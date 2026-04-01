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
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);  // Elevator
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);  // Flabs
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);  // FWheel
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);  // Rudder
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);  // Throttle
}

void ServoControl_Update(const int *channels) {

    int throttle_us = map(channels[CHANNEL_THROTTLE], 172, 1811, 1000, 2000); // 450, 1100
    int rudder_us   = map(channels[CHANNEL_RUDDER], 172, 1811, 1000, 2000);   // 600, 900
    int elevator_us = map(channels[CHANNEL_ELEVATOR], 172, 1811, 1000, 2000);
    int flabs_us  = map(channels[CHANNEL_FLABS], 172, 1811, 1000, 2000);
    int fwheel_us    = map(channels[CHANNEL_FWHEEL], 172, 1811, 1000, 2000);

    // Clamp values to safe servo range
    if (throttle_us < SERVO_MIN_US) throttle_us = SERVO_MIN_US;
    if (throttle_us > SERVO_MAX_US) throttle_us = SERVO_MAX_US;

    if (rudder_us < SERVO_MIN_US) rudder_us = SERVO_MIN_US;
    if (rudder_us > SERVO_MAX_US) rudder_us = SERVO_MAX_US;

    if (elevator_us < SERVO_MIN_US) elevator_us = SERVO_MIN_US;
    if (elevator_us > SERVO_MAX_US) elevator_us = SERVO_MAX_US;

    if (flabs_us < SERVO_MIN_US) flabs_us = SERVO_MIN_US;
    if (flabs_us > SERVO_MAX_US) flabs_us = SERVO_MAX_US;

    if (fwheel_us < SERVO_MIN_US) fwheel_us = SERVO_MIN_US;
    if (fwheel_us > SERVO_MAX_US) fwheel_us = SERVO_MAX_US;

    // Reverse flabs_us for the rudder
    int flabs_us_reversed = SERVO_MIN_US + SERVO_MAX_US - flabs_us;

    // Rverse fwheel_us for the front wheel
    int fwheel_us_reversed = SERVO_MIN_US + SERVO_MAX_US - fwheel_us;

    // Update PWM pulse widths
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, elevator_us);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, flabs_us);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, fwheel_us_reversed);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, flabs_us_reversed);	//rudder_us
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, throttle_us);
}


