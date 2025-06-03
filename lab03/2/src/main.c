#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
// #include "system_TM4C1294.h"   // CMSIS-Core
#include "cmsis_os2.h"         // CMSIS-RTOS
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

#include "Uart/uart.h"
#include "Leds/leds.h"
#include "TempSens/tempSens.h"
#include "SysTick/sysTick.h"

#define PERIOD_TO_SEND_INFO 10000 // 1 second
#define PERIOD_TO_GET_INFO 500 // 0.5 second

osThreadId_t thread1_id, thread2_id, thread3_id;

unsigned long tempSensValues[10] = { 0 };
float mean = 0;
float stdDev = 0;

static osMutexId_t tempMutex_id;


void thread1 (void *arg)
{
    uint32_t i = 0;
    while (1) {
        osStatus_t stat = osMutexAcquire(tempMutex_id, osWaitForever);
        if (stat == osOK)
        {
            tempSensValues[i] = tempSens_getTempSensRead();

            osMutexRelease(tempMutex_id);
        }
        
        i = ((i + 1) % 10);

        osDelay(PERIOD_TO_GET_INFO);
    } // while
} // thread1

void thread2 (void *arg)
{
    while (1) {
        osStatus_t stat = osMutexAcquire(tempMutex_id, osWaitForever);
        if (stat == osOK)
        {
            mean = 0;
            stdDev = 0;
            for(int i = 0; i < 10; i++) mean += (tempSensValues[i] / 10.0f);
            for(int i = 0; i < 10; i++) stdDev += ((pow(((float)tempSensValues[i] - mean), 2)) / 10.0f);
            stdDev = sqrtf(stdDev);

            osMutexRelease(tempMutex_id);
        }

        osDelay(PERIOD_TO_GET_INFO);
    } // while
} // thread2

void thread3 (void *arg)
{
    char str[50] = { 0 };
    while (1) {
        snprintf(str, sizeof(str), "%.2f", mean);
        str[49] = '\0';
        uart_sendString(str);
        uart_sendString(",");
        snprintf(str, sizeof(str), "%.2f", stdDev);
        str[49] = '\0';
        uart_sendString(str);
        uart_sendString("\r\n");

        osDelay(PERIOD_TO_SEND_INFO);
    } // while
} // thread3

int main (void)
{
    sysTick_setClkFreq();
    osKernelInitialize();
    
    tempMutex_id = osMutexNew(NULL);
    if (tempMutex_id == NULL)
    {
        return -1;
    }

    thread1_id = osThreadNew(thread1, NULL, NULL);
    thread2_id = osThreadNew(thread2, NULL, NULL);
    thread3_id = osThreadNew(thread3, NULL, NULL);

    leds_configLeds();
    uart_setupUart();
    tempSens_configTempSens();
    
    osKernelStart();

    while (1);
} // main
