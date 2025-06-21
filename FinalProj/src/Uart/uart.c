#include "./uart.h"
#include "../SysTick/sysTick.h"

//--------------------------------------------------

/**
 * @brief Handles a UART interrupt.
 */
static void uartIntHandler(void);

//--------------------------------------------------

extern void uart_setupUart(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0))
        ;
    UARTConfigSetExpClk(UART0_BASE,
                        sysTick_getClkFreq(),
                        115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTFIFOEnable(UART0_BASE);
    UARTIntEnable(UART0_BASE, UART_INT_RX);
    UARTIntRegister(UART0_BASE, uartIntHandler);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
        ;
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
}

extern void uart_sendString(const char *str)
{
    while (*str != '\0')
    {
        UARTCharPut(UART0_BASE, *str++);
    }
}

extern void uart_sendArray(const char *str, unsigned long size)
{
    for (unsigned long i = 0; i < size; i++)
    {
        UARTCharPut(UART0_BASE, str[i]);
    }
}

extern void uart_sendValInDecimal(unsigned long val)
{
    // Max unsigned long on a 32-bit Tiva is 10 digits, +1 for '\0'
    char buf[11] = {0};
    unsigned char len = 0;

    len = sprintf(buf, "%lu", val);

    for (unsigned char itr = 0; itr < len; itr++)
    {
        UARTCharPut(UART0_BASE, buf[itr]);
    }

    return;
}

extern unsigned long
uart_GetBytes(unsigned long uartBase, unsigned char *buff, unsigned long buffSize)
{
    unsigned long byteCount = 0;
    char gottenChar = 0;

    while (UARTCharsAvail(uartBase) && (byteCount < buffSize))
    {
        gottenChar = UARTCharGet(uartBase);
        
        if (gottenChar != 0xFF)
        {
            buff[byteCount++] = (unsigned char)gottenChar;
        }
    }

    return byteCount;
}

//--------------------------------------------------

static void uartIntHandler(void)
{
    uint32_t status = UARTIntStatus(UART0_BASE, true);
    UARTIntClear(UART0_BASE, status);
    uint8_t last = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
}