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
#include "driverlib/systick.h"

#include "src/SysTick/sysTick.h"
#include "src/Uart/uart.h"
#include "src/Leds/leds.h"
#include "src/DCMotors/dcMotor.h"
#include "src/TempSens/tempSens.h"

int main(void)
{
    sysTick_setClkFreq();
    
    sysTick_setupSysTick();
    leds_configLeds();
    uart_setupUart();
    
    leds_turnOffLed(LED_0);
    leds_turnOffLed(LED_1);
    leds_turnOffLed(LED_2);
    leds_turnOffLed(LED_3);

    while (1)
    {
        __asm(" WFI"); // espera por interrupcao, modo low power
    }
}
