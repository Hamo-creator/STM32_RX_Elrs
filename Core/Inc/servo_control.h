/*
 * servo_control.h
 *
 *  Created on: May 6, 2025
 *      Author: Khalil
 */

#ifndef SERVO_CONTROL_H_
#define SERVO_CONTROL_H_

#include "tim.h"  // Include HAL timer handles

// Map your RC channel indices to servo roles
/*
#define CHANNEL_THROTTLE  0
#define CHANNEL_RUDDER    1
#define CHANNEL_ELEVATOR  2
#define CHANNEL_AILERON   3
#define CHANNEL_FLABS	  4
*/

typedef enum {
	CHANNEL_FLABS = 0,   // Explicitly setting the first value is good practice
	CHANNEL_ELEVATOR,    // Will automatically be 1
	CHANNEL_THROTTLE,    // Will automatically be 2
	CHANNEL_FWHEEL, 	 // Will automatically be 3
	CHANNEL_EMPTY,
	CHANNEL_RUDDER, 	 // Will automatically be 5 // Add other channels here if needed
	CRSF_CHANNEL_COUNT   // A useful trick: this member's value is the total number of channels (5)
} CrsfChannel_t;


// Servo signal limits in microseconds
#define SERVO_MIN_US 1000
#define SERVO_MAX_US 2000

// Initializes all PWM timers (TIM2, TIM3)
void ServoControl_Init(void);

// Updates all servos with current RC values
void ServoControl_Update(const int *channels);


#endif /* INC_SERVO_CONTROL_H_ */
