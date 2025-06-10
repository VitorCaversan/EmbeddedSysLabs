/**
 * Course: Sistemas Embarcados - DAELN-UTFPR
 * Authors: Joao Vitor Caversan, Pedro Henrique, Rafal Merling
 * 
 * Module: Uart
 */

#ifndef _UART_H_
#define _UART_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"
#include "../SysTick/sysTick.h"

/**
 * @brief Sets up the UART0 peripheral and its interrupts.
 * 
 * MUST BE CALLED AFTER sysTick_setClkFreq
 */
extern void uart_setupUart(void);

/**
 * @brief Sends a string through the UART0 peripheral.
 * 
 * @param str The string to be sent. Must have a \0 terminator
 */
extern void uart_sendString(const char *str);

/**
 * @brief Sends an array of characters through the UART0 peripheral.
 * 
 * @param str Pointer to the array of characters to be sent
 * @param size The amount of characters to be sent
 */
extern void uart_sendArray(const char *str, unsigned long size);

/**
 * @brief Sends a value in decimal format through the UART0 peripheral.
 * 
 * @param val The value to be sent
 */
extern void uart_sendValInDecimal(unsigned long val);

#endif // _UART_H_