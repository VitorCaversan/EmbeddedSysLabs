#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "system_TM4C1294.h"   // CMSIS-Core
#include "cmsis_os2.h"         // CMSIS-RTOS
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

#include "src/SysTick/sysTick.h"
#include "src/Uart/uart.h"
#include "src/Leds/leds.h"
#include "src/TempSens/tempSens.h"

#define PERIOD_TO_SEND_INFO 500 // 0.5 second

osThreadId_t thread1_id, thread2_id, thread3_id;
static unsigned long g_TimeSinceLastUpdate = 0;
unsigned long tempSensValues[10];
float mean = 0;
float stdDev = 0;



void thread1 (void *arg)
{
    uint32_t i = 0;
    while (1) {
        if(i > 9) i = 0;
        tempSensValues[i] = tempSens_getTempSensRead(); //(Usar Mutex/Semaforo?)
        i++;
        osDelay(500)
    } // while
} // thread1

void thread2 (void *arg)
{
    while (1) {
        //calculo da média/desvio padrão das ultimas 10 leituras (Usar Mutex/Semaforo?)
    } // while
} // thread2

void thread3 (void *arg)
{
    while (1) {
        if (sysTick_getTimeInMs() - g_TimeSinceLastUpdate >= PERIOD_TO_SEND_INFO)
        {
            g_TimeSinceLastUpdate = sysTick_getTimeInMs();
            uart_sendValInDecimal(mean);   //(Usar Mutex/Semaforo?)
            uart_sendString(",");
            uart_sendValInDecimal(stdDev); //(Usar Mutex/Semaforo?)
            uart_sendString("\r\n");
        }
    } // while
} // thread3

void main (void)
{
    osKernelInitialize();

    thread1_id = osThreadNew(thread1, NULL, NULL);
    thread2_id = osThreadNew(thread2, NULL, NULL);
    thread3_id = osThreadNew(thread3, NULL, NULL);

    sysTick_setClkFreq();
    sysTick_setupSysTick();
    leds_configLeds();
    uart_setupUart();
    tempSens_configTempSens();

    osKernelStart();

    while (1);
} // main
