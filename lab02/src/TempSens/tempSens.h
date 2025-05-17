/**
 * Course: Sistemas Embarcados - DAELN-UTFPR
 * Authors: Joao Vitor Caversan, Pedro Henrique, Rafal Merling
 * 
 * Module: Temperature Sensor
 */

#ifndef _TEMP_SENS_H_
#define _TEMP_SENS_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"


/**
 * @brief Configures the temperature sensor on the board.
 * 
 * The temperature sensor is read from the ADC0 channel 0. The function sets up the
 * necessary GPIO pins and ADC configurations to enable reading the temperature sensor.
 */
extern void tempSens_configTempSens(void);

/**
 * @brief Returns the ADC value read on the Temperature Sensor.
 * 
 * The temperature sensor is read from the ADC0 channel 0. The function returns the 
 * reading from the temperature sensor.
 */
extern uint32_t tempSens_getTempSensRead(void);


#endif // _TEMP_SENS_H_