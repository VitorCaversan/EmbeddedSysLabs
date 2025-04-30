#include "./sysTick.h"

#define CLK_FREQ  120000000 // 120MHz
#define SYS_TICKS 1000 // 1ms

static void sysTickIntHandler(void);

//--------------------------------------------------

static unsigned long          SysClock = 0;
static volatile unsigned long sysTicks1ms = 0;

//--------------------------------------------------

extern void sysTick_setClkFreq()
{
    SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), CLK_FREQ);
}

extern void sysTick_setupSysTick(void)
{
    SysTickPeriodSet(SysClock / SYS_TICKS);
    SysTickIntRegister(sysTickIntHandler);
    SysTickIntEnable();
    SysTickEnable();

    return;
}

extern unsigned long sysTick_getTimeInMs(void)
{
    return sysTicks1ms;
}

//--------------------------------------------------

static void sysTickIntHandler(void)
{
    sysTicks1ms++;

    return;
}