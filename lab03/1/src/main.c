#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
// #include "system_TM4C1294.h"   // CMSIS-Core
#include "cmsis_os2.h"         // CMSIS-RTOS
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

#include "SysTick/sysTick.h"
#include "Leds/leds.h"

osThreadId_t thread1_id, thread2_id, thread3_id;

void thread1 (void *arg)
{
    while (1) {
        leds_turnOnLed(LED_0);
        osDelay(1000);
        leds_turnOffLed(LED_0);
        osDelay(1000);
    } // while
} // thread1

void thread2 (void *arg)
{
    while (1) {
        leds_turnOnLed(LED_1);
        osDelay(250);
        leds_turnOffLed(LED_1);
        osDelay(250);
    } // while
} // thread2

void thread3 (void *arg)
{
    while (1) {
        leds_turnOnLed(LED_2);
        osDelay(250);
        leds_turnOffLed(LED_2);
        osDelay(250);
    } // while
} // thread3

int main (void)
{
    osKernelInitialize();

    thread1_id = osThreadNew(thread1, NULL, NULL);
    thread2_id = osThreadNew(thread2, NULL, NULL);
    thread3_id = osThreadNew(thread3, NULL, NULL);

    //sysTick_setClkFreq();
    //sysTick_setupSysTick();
    leds_configLeds();

    osKernelStart();

    while (1);
} // main
