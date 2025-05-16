/**
 * Course: Sistemas Embarcados - DAELN-UTFPR
 * Authors: Joao Vitor Caversan, Pedro Henrique, Rafal Merling
 * 
 * Module: Temperature Sensor
 */

#ifndef _DC_MOTOR_H_
#define _DC_MOTOR_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"

/**
 * @brief Configures the DC motors control on the board.
 * 
 * It sets up the timer for PWM generation and the GPIO pins for controlling the DC motors with an
 * LM1939 IC.
 * The function initializes the necessary peripherals and configures the pins for output.
 */
extern void dcMotor_configDCMotors(void);

/**
 * @brief Configures the PWM on the board.
 * 
 * It sets up the PWM clock, generator and port. 
 * The function initializes and configures the necessary components for using the PWM
 */
extern void setupPWM(void);

/**
 * @brief Sets the Duty Cycle for the PWM by percentage.
 * 
 * It sets up the Duty Cycle for the PWM by percentage for controlling the DC motors.
 * The function receives the percentage value to set the Duty Cycle to.
 */
extern void setDutyCycle(uint32_t pwmPercent);

/**
 * @brief Increments the Duty Cycle for the PWM by X increments.
 * 
 * It increments the Duty Cycle for the PWM by X increments.
 * The function receives the number of increments and the fadeUp flag to increment the Duty Cycle.
 */
extern void incrementDutyCycle(uint32_t increments, bool fadeUp);

#endif // _DC_MOTOR_H_