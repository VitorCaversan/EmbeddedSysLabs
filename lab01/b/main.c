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

#define CLK_FREQ  120000000 // 120MHz
#define SYS_TICKS 1 // 1s

#define LED_PORTN GPIO_PORTN_BASE
#define LED_PORTF GPIO_PORTF_BASE
#define BTNS_PORTJ GPIO_PORTJ_BASE
#define LED_PIN_1 GPIO_PIN_1
#define LED_PIN_0 GPIO_PIN_0
#define LED_PIN_4 GPIO_PIN_4
#define BTN_PIN_0 GPIO_PIN_0
#define BTN_PIN_1 GPIO_PIN_1

static uint32_t SysClock;
static volatile unsigned int SysTicks1ms = 0;
static unsigned char rxbuffer[3] = {0};
static volatile int blinkTime = 0;

void SysTickIntHandler(void);
void SetupSysTick(void);
void ConfigLEDs(void);

/**
 * @brief Configures the push-button on the board.
 * 
 * The button are located on pins PJ0 and PJ1 of the board.
 * 
 */
static void configBtns(void);

// Handler SysTick
void SysTickIntHandler(void) {
    SysTicks1ms++;
    if (SysTicks1ms >= blinkTime && blinkTime != 0) {
        GPIOPinWrite(LED_PORTN, LED_PIN_1 | LED_PIN_0, 0);
        GPIOPinWrite(LED_PORTF, LED_PIN_4, 0);
        blinkTime = 0;
    }
}

void SetupSysTick(void) {
    SysTickPeriodSet(SysClock / SYS_TICKS);
    SysTickIntRegister(SysTickIntHandler);
    SysTickIntEnable();
    SysTickEnable();
}

void ConfigLEDs(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(LED_PORTN, LED_PIN_1 | LED_PIN_0);
    GPIOPinTypeGPIOOutput(LED_PORTF, LED_PIN_4);
}

static void configBtns(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));
    GPIOPinTypeGPIOInput(BTNS_PORTJ, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPadConfigSet(BTNS_PORTJ, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

int main(void) {
    SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), CLK_FREQ);
    
    ConfigLEDs();
    configBtns();
    SetupSysTick();

    while (1) {
        __asm(" WFI"); // espera por interrupcao, modo low power
    }
}
