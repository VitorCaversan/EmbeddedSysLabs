/**
 * Course: Sistemas Embarcados - DAELN-UTFPR
 * Authors: Joao Vitor Caversan, Pedro Henrique, Rafal Merling
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"

#include "src/Buttons/buttons.h"
#include "src/SysTick/sysTick.h"
#include "src/Uart/uart.h"
#include "src/Leds/leds.h"

static unsigned char rxbuffer[3] = {0};
static volatile int blinkTime = 0;

int main(void)
{
    sysTick_setClkFreq();
    
    sysTick_setupSysTick();
    leds_configLeds();
    btns_configBtns();
    uart_setupUart();
    
    btns_configInterrupts();

    leds_turnOnLed(LED_0);
    leds_turnOnLed(LED_1);
    leds_turnOnLed(LED_2);
    leds_turnOnLed(LED_3);

    while (1)
    {
        __asm(" WFI"); // espera por interrupcao, modo low power
    }
}
