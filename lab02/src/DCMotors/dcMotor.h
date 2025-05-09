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

/**
 * @brief Configures the DC motors control on the board.
 * 
 * It sets up the timer for PWM generation and the GPIO pins for controlling the DC motors with an
 * LM1939 IC.
 * The function initializes the necessary peripherals and configures the pins for output.
 */
extern void dcMotor_configDCMotors(void);


#endif // _DC_MOTOR_H_