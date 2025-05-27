/**
 * Course: Sistemas Embarcados - DAELN-UTFPR
 * Authors: Joao Vitor Caversan, Pedro Henrique, Rafal Merling
 * 
 * Module: Global variables that control the system data and states.
 * It only contains getters and setters.
 */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct SysData
{
    bool          btnWasPressed;
    unsigned long prevBtnSysTick; // Last SysTick value
    unsigned long timeBtwnBtnPresses; // Time between button presses in ms
} SysData;

//------------------
extern bool g_wasBtnPressed(void);
extern void g_setBtnPressed(bool isBtnPressed);
//------------------
extern unsigned long g_getPrevBtnSysTick(void);
extern void g_setPrevBtnSysTick(unsigned long newSysTick);
//------------------
extern unsigned long g_getTimeBtwnBtnPresses(void);
extern void g_setTimeBtwnBtnPresses(unsigned long newTime);
//------------------

#endif // _GLOBAL_H_