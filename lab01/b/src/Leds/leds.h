/**
 * Course: Sistemas Embarcados - DAELN-UTFPR
 * Authors: Joao Vitor Caversan, Pedro Henrique, Rafal Merling
 * 
 * Module: Leds
 */

#ifndef _LEDS_H_
#define _LEDS_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

typedef enum LedId
{
    LED_0 = 0,
    LED_1,
    LED_2,
    LED_3
} LedId;

/**
 * @brief Configures the LEDs on the board.
 * 
 * The 4 LEDs are located on pins PN0, PN1 and two on PF4 of the board.
 */
extern void leds_configLeds(void);

/**
 * @brief Turns on the LED specified by the ledId parameter.
 * 
 * @param ledId The ID of the LED to be turned on.
 */
extern void leds_turnOnLed(LedId ledId);

/**
 * @brief Turns off the LED specified by the ledId parameter.
 * 
 * @param ledId The ID of the LED to be turned off.
 */
extern void leds_turnOffLed(LedId ledId);


#endif // _LEDS_H_