/*****************************************************************
 *
 * File name:       START_N_SLEEP.C
 * Description:     Project definitions and function prototypes for use with START_N_SLEEP.C
 * Author:          Hab S. Collector
 * Date:            1/23/12
 * Hardware:        NXP LPC1769
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 
 
#ifndef _START_N_SLEEP_TASKS_DEFINES
#define _START_N_SLEEP_TASKS_DEFINES


// INCLUDES
#include "HC15C_DEFINES.h"

// FUNCTION PROTOTYPES
void init_taskFn(void *);
void init_WakeFromSleep_EINT(void);
void EINT0_IRQHandler(void);
void wakeFromSleep(void);
void goToSleep(void);

#endif