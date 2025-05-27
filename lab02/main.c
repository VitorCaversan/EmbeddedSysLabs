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

//--------------------------------------------------

#define PERIOD_TO_SEND_INFO 1000 // 1 second

typedef enum smState
{
    STATE_OFF = 0,
    STATE_LOW,
    STATE_MID,
    STATE_HIGH
} smState;

//--------------------------------------------------

typedef struct smContext
{
    smState state;
    unsigned long tempSensValue;
    unsigned long dutyCycle;
} smContext;

//--------------------------------------------------

/**
 * @brief Changes the state of the state machine.
 * 
 * @param newState The new state to be set.
 */
static void changeToState(smState newState);

//--------------------------------------------------

static smContext g_smContext = {STATE_OFF, 0, 0};
static unsigned long g_TimeSinceLastUpdate = 0;

//--------------------------------------------------

int main(void)
{
    sysTick_setClkFreq();
    
    sysTick_setupSysTick();
    leds_configLeds();
    uart_setupUart();
    tempSens_configTempSens();
    dcMotor_configDCMotors();
    
    leds_turnOnLed(LED_0);
    leds_turnOffLed(LED_1);
    leds_turnOffLed(LED_2);
    leds_turnOffLed(LED_3);

    while (1)
    {
        g_smContext.tempSensValue = tempSens_getTempSensRead();

        if (sysTick_getTimeInMs() - g_TimeSinceLastUpdate >= PERIOD_TO_SEND_INFO)
        {
            g_TimeSinceLastUpdate = sysTick_getTimeInMs();
            uart_sendValInDecimal(g_smContext.tempSensValue);
            uart_sendString(",");
            uart_sendValInDecimal(g_smContext.dutyCycle);
            uart_sendString("\r\n");
        }

        switch (g_smContext.state)
        {
            case STATE_OFF:
                if (g_smContext.tempSensValue >= 25)
                {
                    changeToState(STATE_LOW);
                }
                break;

            case STATE_LOW:
                if (g_smContext.tempSensValue >= 30)
                {
                    changeToState(STATE_MID);
                }
                else if (g_smContext.tempSensValue < 25)
                {
                    changeToState(STATE_OFF);
                }
                break;

            case STATE_MID:
                if (g_smContext.tempSensValue > 35)
                {
                    changeToState(STATE_HIGH);
                }
                else if (g_smContext.tempSensValue < 30)
                {
                    changeToState(STATE_LOW);
                }
                break;

            case STATE_HIGH:
                if (g_smContext.tempSensValue <= 35)
                {
                    changeToState(STATE_MID);
                }
                break;

            default:
                break;
        }

        // __asm(" WFI"); // espera por interrupcao, modo low power
    }
}

//--------------------------------------------------

static void changeToState(smState newState)
{
    g_smContext.state = newState;

    switch (newState)
    {
        case STATE_OFF:
        {
            leds_turnOffLed(LED_1);
            leds_turnOffLed(LED_2);
            leds_turnOffLed(LED_3);
            g_smContext.dutyCycle = 0;
            dcMotor_setDutyCycle(g_smContext.dutyCycle);
            dcMotor_TurnOffMotor();
        }
        break;

        case STATE_LOW:
        {
            g_smContext.state = STATE_LOW;
            leds_turnOnLed(LED_1);
            leds_turnOffLed(LED_2);
            g_smContext.dutyCycle = 50;
            dcMotor_TurnOnMotor(CLOCKWISE);
            dcMotor_setDutyCycle(g_smContext.dutyCycle);
        }
        break;

        case STATE_MID:
        {
            g_smContext.state = STATE_MID;
            leds_turnOnLed(LED_2);
            leds_turnOffLed(LED_3);
            g_smContext.dutyCycle = 75;
            dcMotor_setDutyCycle(g_smContext.dutyCycle);
        }
        break;

        case STATE_HIGH:
        {
            g_smContext.state = STATE_HIGH;
            leds_turnOnLed(LED_3);
            g_smContext.dutyCycle = 95;
            dcMotor_setDutyCycle(g_smContext.dutyCycle);
        }
        break;

        default:
            break;
    }
}