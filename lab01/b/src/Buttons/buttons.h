/**
 * Course: Sistemas Embarcados - DAELN-UTFPR
 * Authors: Joao Vitor Caversan, Pedro Henrique, Rafal Merling
 * 
 * Module: Buttons
 */

#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"

/**
 * @brief Configures the push-button on the board.
 * 
 * The button are located on pins PJ0 and PJ1 of the board.
 * 
 */
extern void btns_configBtns(void);

/**
 * @brief Configures all the external interrupts used in the system.
 * 
 * The ones used in this system are for the push-buttons on the board.
 * It DOES NOT configure the port or the pin as input, only the interrupt.
 */
extern void btns_configInterrupts(void);

#endif // _BUTTONS_H_