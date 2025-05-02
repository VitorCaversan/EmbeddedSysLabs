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

#define LED_PORTN GPIO_PORTN_BASE
#define LED_PORTF GPIO_PORTF_BASE
#define LED_PIN_1 GPIO_PIN_1
#define LED_PIN_0 GPIO_PIN_0
#define LED_PIN_4 GPIO_PIN_4

static unsigned char rxbuffer[3] = {0};
static volatile int blinkTime = 0;

void ConfigLEDs(void);


void ConfigLEDs(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(LED_PORTN, LED_PIN_1 | LED_PIN_0);
    GPIOPinTypeGPIOOutput(LED_PORTF, LED_PIN_4);
}

int main(void)
{
    sysTick_setClkFreq();
    
    sysTick_setupSysTick();
    ConfigLEDs();
    btns_configBtns();
    uart_setupUart();
    
    btns_configInterrupts();

    // liga todos os LEDs
    GPIOPinWrite(LED_PORTN, LED_PIN_1 | LED_PIN_0, LED_PIN_1 | LED_PIN_0);
    GPIOPinWrite(LED_PORTF, LED_PIN_4, LED_PIN_4);

    while (1)
    {
        __asm(" WFI"); // espera por interrupcao, modo low power
    }
}
