#include "./global.h"

static SysData gSysData;

//------------------
extern bool g_wasBtnPressed(void)
{
    return gSysData.btnWasPressed;
}
extern void g_setBtnPressed(bool isBtnPressed)
{
    gSysData.btnWasPressed = isBtnPressed;

    return;
}
//------------------
extern unsigned long g_getPrevBtnSysTick(void)
{
    return gSysData.prevBtnSysTick;
}
extern void g_setPrevBtnSysTick(unsigned long newSysTick)
{
    gSysData.prevBtnSysTick = newSysTick;

    return;
}
//------------------
extern unsigned long g_getTimeBtwnBtnPresses(void)
{
    return gSysData.timeBtwnBtnPresses;
}
extern void g_setTimeBtwnBtnPresses(unsigned long newTime)
{
    gSysData.timeBtwnBtnPresses = newTime;

    return;
}