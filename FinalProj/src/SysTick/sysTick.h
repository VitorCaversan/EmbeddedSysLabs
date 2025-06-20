/**
 * Course: Sistemas Embarcados - DAELN-UTFPR
 * Authors: Joao Vitor Caversan, Pedro Henrique, Rafal Merling
 * 
 * Module: Systick
 */

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

/**
 * @brief Sets and gets the clock frequency
 */
extern void sysTick_setClkFreq();
extern unsigned long sysTick_getClkFreq(void);

/**
 * @brief Configures sysTick interrupts
 */
extern void sysTick_setupSysTick(void);

/**
 * @brief Returns the time in ms since the configuration of the systick
 * 
 * @return unsigned long Time in ms
 */
extern unsigned long sysTick_getTimeInMs(void);

#endif // _SYSTICK_H_