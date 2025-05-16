#include "./dcMotor.h"

//--------------------------------------------------

#define PWM_FREQUENCY 12000 // PWM Frequency

uint32_t SysClock;
volatile uint32_t g_ui32PWMDutyCycle = 0; // Variable to store the DutyCycle status
bool g_bFadeUp = true; // Controls the fade direction.

//--------------------------------------------------

extern void dcMotor_configDCMotors(void)
{
    setupPWM();

    return;
}

extern void setupPWM(void) {

    // Enables PWM and its port
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_1);
    GPIOPinConfigure(GPIO_PG1_M0PWM5);

    // Configures PWM clock 
    SysCtlPWMClockSet(SYSCTL_PWMDIV_2); 

    // Configures PWM generator
    uint32_t pwmPeriod = (SysClock/1) / PWM_FREQUENCY;  // Adjust clock division here
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, pwmPeriod);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, g_ui32PWMDutyCycle);
    PWMOutputState(PWM0_BASE, PWM_OUT_5_BIT, true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);

    return;
}

extern void setDutyCycle(uint32_t pwmPercent) {

    uint32_t maxPeriod = PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2);
    uint32_t increment = maxPeriod / 100; // Increment based on PWM period - 1000 = 10 s  100 = 1s

    if (pwmPercent < 0) pwmPercent = 0;
    else if (pwmPercent > 100) pwmPercent = 100;

    g_ui32PWMDutyCycle = increment * pwmPercent;

    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, g_ui32PWMDutyCycle);

    return;
}

extern void incrementDutyCycle(uint32_t increments, bool fadeUp){
    
    uint32_t maxPeriod = PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2);	  
	uint32_t increment = maxPeriod / 100;  // Increment based on PWM period - 1000 = 10 s  100 = 1s
    g_bFadeUp = fadeUp

    if (g_bFadeUp) {

        for (int i = 0; i <= increments; i++){

            if (g_ui32PWMDutyCycle + increment <= maxPeriod) {
                g_ui32PWMDutyCycle += increment;
            } 
            else g_ui32PWMDutyCycle = maxPeriod;
        }
        
    } 
    else {

        for (int i = 0; i <= increments; i++){

            if (g_ui32PWMDutyCycle > increment) {
                g_ui32PWMDutyCycle -= increment;
            } 
            else g_ui32PWMDutyCycle = 0;
        }
    }

    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, g_ui32PWMDutyCycle);

    return;
}

//--------------------------------------------------
