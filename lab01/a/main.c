/**
 * Course: Sistemas Embarcados - DAELN-UTFPR
 * Authors: João Vitor Caversan, Pedro Henrique, Rafal Merling
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

/**
 * @brief Takes action depending on the received number.
 * 
 * 1 to 4 - Blink LED1 and LED2 on board N for the specified time in seconds.
 * 5 and 6 - Checks the state of the buttons on board F and sends the result to the terminal.
 */
void UARTIntHandler(void);

void SysTickIntHandler(void);
void SetupSysTick(void);
void SetupUart(void);
void ConfigLEDs(void);

/**
 * @brief Configures the push-button on the board.
 * 
 * The button are located on pins PJ0 and PJ1 of the board.
 * 
 */
static void configBtns(void);
void UARTSendString(const char *str);

// Handler UART
void UARTIntHandler(void) { 
    uint32_t status = UARTIntStatus(UART0_BASE, true);
    UARTIntClear(UART0_BASE, status);
    uint8_t last = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);

    rxbuffer[0] = rxbuffer[1];
    rxbuffer[1] = rxbuffer[2];
    rxbuffer[2] = last;

    if ((rxbuffer[2] >= '1') && (rxbuffer[2] <= '4'))
    {
        blinkTime = (rxbuffer[2] - '0') * 1000;
        SysTicks1ms = 0;
        GPIOPinWrite(LED_PORTN, LED_PIN_1 | LED_PIN_0, LED_PIN_1 | LED_PIN_0);
        GPIOPinWrite(LED_PORTF, LED_PIN_4, LED_PIN_4);
    }
    else if (rxbuffer[2] == '5')
    {
        UARTSendString("Botao 5: ");
        UARTSendString(GPIOPinRead(BTNS_PORTJ, BTN_PIN_0) ? "NAO PRESSIONADO\r\n" : "PRESSIONADO\r\n");
    }
    else if (rxbuffer[2] == '6')
    {
        UARTSendString("Botao 6: ");
        UARTSendString(GPIOPinRead(BTNS_PORTJ, BTN_PIN_1) ? "NAO PRESSIONADO\r\n" : "PRESSIONADO\r\n");
    }

    return;
}
// Handler SysTick
void SysTickIntHandler(void) {
    SysTicks1ms++;
    if (SysTicks1ms >= blinkTime && blinkTime != 0) {
        GPIOPinWrite(LED_PORTN, LED_PIN_1 | LED_PIN_0, 0);
        GPIOPinWrite(LED_PORTF, LED_PIN_4, 0);
        blinkTime = 0;
        
        UARTCharPut(UART0_BASE, 'O');
        UARTCharPut(UART0_BASE, 'K');
        UARTCharPut(UART0_BASE, '\r');
        UARTCharPut(UART0_BASE, '\n');
        UARTSendString("Teste de led completo\r\n");
    }
}

void SetupSysTick(void) {
    SysTickPeriodSet(SysClock / 1000);
    SysTickIntRegister(SysTickIntHandler);
    SysTickIntEnable();
    SysTickEnable();
}

void SetupUart(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
    UARTConfigSetExpClk(UART0_BASE, SysClock, 115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTFIFODisable(UART0_BASE);
    UARTIntEnable(UART0_BASE, UART_INT_RX);
    UARTIntRegister(UART0_BASE, UARTIntHandler);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
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

// String
void UARTSendString(const char *str) {
    while (*str != '\0') { 
        UARTCharPut(UART0_BASE, *str++);
    }
}

int main(void) {
    SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 120000000);
    
    ConfigLEDs();
    configBtns();
    SetupUart();
    SetupSysTick();

    while (1) {
        __asm(" WFI"); // espera por interrupçao, modo low power
    }
}
