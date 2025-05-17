#include "./tempSens.h"

//--------------------------------------------------

#define ADC_SEQUENCER 3 //Using sequencer 3 of the ADC for single sampling

//--------------------------------------------------

/**
 * @brief Configures the ADC on the board.
 * 
 * The temperature sensor is read from the ADC0 channel 0. The function sets up the
 * necessary GPIO pins and ADC configurations to enable reading the temperature sensor.
 */
static void SetupADC(void);

//--------------------------------------------------

extern uint32_t tempSens_getTempSensRead(void)
{

    uint32_t adcValue;

    ADCProcessorTrigger(ADC0_BASE, ADC_SEQUENCER); // Starts the conversion process
    while(!ADCIntStatus(ADC0_BASE, ADC_SEQUENCER, false)); // Wait for the conversion end
    ADCIntClear(ADC0_BASE, ADC_SEQUENCER); // clears ADC interrupt
    ADCSequenceDataGet(ADC0_BASE, ADC_SEQUENCER, &adcValue); // Get read value

    return adcValue;
}

extern void tempSens_configTempSens(void)
{
    SetupADC();  
    
    return;
}

//--------------------------------------------------

static void SetupADC(void)
{
    // GPIO Port E and ADC0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE) ||
          !SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {}

    // PE4 as an ADC input (AIN9)
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_4);

    // ADC sequence 3 for a single, processor-triggered sample
    ADCSequenceDisable(ADC0_BASE, ADC_SEQUENCER);
    ADCSequenceConfigure(ADC0_BASE, ADC_SEQUENCER, ADC_TRIGGER_PROCESSOR, 0);
    // sample channel 9, set interrupt flag & end of sequence
    ADCSequenceStepConfigure(ADC0_BASE, ADC_SEQUENCER, 0,
                             ADC_CTL_CH9 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, ADC_SEQUENCER);
    ADCIntClear(ADC0_BASE, ADC_SEQUENCER);

    return;
}