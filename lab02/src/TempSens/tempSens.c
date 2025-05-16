#include "./tempSens.h"

//--------------------------------------------------

#define ADC_SEQUENCER 3 //Using sequencer 3 of the ADC for single sampling

//--------------------------------------------------

extern void SetupADC(void) {

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));
    ADCSequenceConfigure(ADC0_BASE, ADC_SEQUENCER, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, ADC_SEQUENCER, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, ADC_SEQUENCER);
    ADCIntClear(ADC0_BASE, ADC_SEQUENCER);

    return;
}

extern uint32_t getTempSensRead(void){

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