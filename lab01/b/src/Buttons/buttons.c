/**
 * Course: Sistemas Embarcados - DAELN-UTFPR
 * Authors: Joao Vitor Caversan, Pedro Henrique, Rafal Merling
 * 
 * Module: Buttons
 */

#include "./buttons.h"
#include "../Global/global.h"
#include "../SysTick/sysTick.h"

#define BTNS_PORTJ GPIO_PORTJ_BASE
#define BTN_PIN_0 GPIO_PIN_0
#define BTN_PIN_1 GPIO_PIN_1

#define DEBOUNCE_IN_MS  200

//--------------------------------------------------

static void portJIntHandler(void);

//--------------------------------------------------

extern void btns_configBtns(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));
    GPIOPinTypeGPIOInput(BTNS_PORTJ, BTN_PIN_0 | BTN_PIN_1);
    GPIOPadConfigSet(BTNS_PORTJ, BTN_PIN_0 | BTN_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

extern void btns_configInterrupts(void)
{
    // Trigger type: rising, falling, both edges or level
    GPIOIntTypeSet(BTNS_PORTJ, BTN_PIN_0 | BTN_PIN_1, GPIO_FALLING_EDGE);

    // Handlers
    GPIOIntRegister(BTNS_PORTJ, portJIntHandler);

    // (Alternate: static) Link against INT_GPIOA
    // IntRegister(INT_GPIOA, PortAIntHandler);

    // Priority and enable the IRQ in the NVIC
    IntPrioritySet(INT_GPIOJ, 0x00);
    IntEnable(INT_GPIOJ);

    // Enable the pin-level interrupt and globally enable interrupts
    GPIOIntEnable(BTNS_PORTJ, GPIO_PIN_0 | GPIO_PIN_1);
    IntMasterEnable();
}

//--------------------------------------------------

static void portJIntHandler(void)
{
    static unsigned long btn1DebounceInMs = 0;
    static unsigned long btn2DebounceInMs = 0;

    unsigned long intStatus = GPIOIntStatus(BTNS_PORTJ, true);
    // Acknowledge them
    GPIOIntClear(BTNS_PORTJ, intStatus);

    if (((intStatus & BTN_PIN_0) != 0) &&
        ((sysTick_getTimeInMs() - btn1DebounceInMs) > DEBOUNCE_IN_MS))
    {
        btn1DebounceInMs = sysTick_getTimeInMs();
        g_setBtnPressed(true);
        g_setPrevBtnSysTick(sysTick_getTimeInMs());
    }
    else if (((intStatus & BTN_PIN_1) != 0) &&
             ((sysTick_getTimeInMs() - btn2DebounceInMs) > DEBOUNCE_IN_MS) &&
             (g_wasBtnPressed() == true))
    {
        btn2DebounceInMs = sysTick_getTimeInMs();
        g_setBtnPressed(false);

        g_setTimeBtwnBtnPresses(sysTick_getTimeInMs() - g_getPrevBtnSysTick());
    }
    else
    {
        // Do nothing, just return
    }

    return;
}