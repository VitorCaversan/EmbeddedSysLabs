#include "./leds.h"

//--------------------------------------------------

#define LED_PORTN GPIO_PORTN_BASE
#define LED_PORTF GPIO_PORTF_BASE
#define LED_0_PIN GPIO_PIN_0
#define LED_1_PIN GPIO_PIN_1
#define LED_2_PIN GPIO_PIN_0
#define LED_3_PIN GPIO_PIN_4

//--------------------------------------------------

extern void leds_configLeds(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    
    GPIOPinTypeGPIOOutput(LED_PORTN, LED_1_PIN | LED_0_PIN);
    GPIOPinTypeGPIOOutput(LED_PORTF, LED_2_PIN | LED_3_PIN);

    return;
}

extern void leds_turnOnLed(LedId ledId)
{
    unsigned long ledPort = 0;
    unsigned long ledPin  = 0;
    
    switch (ledId)
    {
        case LED_0:
            ledPort = LED_PORTN;
            ledPin  = LED_0_PIN;
            break;
        case LED_1:
            ledPort = LED_PORTN;
            ledPin  = LED_1_PIN;
            break;
        case LED_2:
            ledPort = LED_PORTF;
            ledPin  = LED_2_PIN;
            break;
        case LED_3:
            ledPort = LED_PORTF;
            ledPin  = LED_3_PIN;
            break;
        default:
            break;
    }

    GPIOPinWrite(ledPort, ledPin, ledPin);

    return;
}

extern void leds_turnOffLed(LedId ledId)
{
    unsigned long ledPort = 0;
    unsigned long ledPin  = 0;
    
    switch (ledId)
    {
        case LED_0:
            ledPort = LED_PORTN;
            ledPin  = LED_0_PIN;
            break;
        case LED_1:
            ledPort = LED_PORTN;
            ledPin  = LED_1_PIN;
            break;
        case LED_2:
            ledPort = LED_PORTF;
            ledPin  = LED_2_PIN;
            break;
        case LED_3:
            ledPort = LED_PORTF;
            ledPin  = LED_3_PIN;
            break;
        default:
            break;
    }

    GPIOPinWrite(ledPort, ledPin, 0);

    return;
}


//--------------------------------------------------
