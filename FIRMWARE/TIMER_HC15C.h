/*****************************************************************
 *
 * File name:       TIMER_HC15C.H
 * Description:     Project definitions and function prototypes for use with TIMER_HC15C.c
 * Author:          Hab S. Collector
 * Date:            7/4/2011
 * Hardware:               
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent 
 *                  on other includes - everything these functions need should be passed to them
 *
 *****************************************************************/ 

#ifndef _TIMER_HC15C_DEFINES
#define _TIMER_HC15C_DEFINES


// INCLUDES
#include <ctl_api.h>
#include "HC15C_DEFINES.h"
#include "LED_HC15C.H"
#include "CMSIS\lpc17xx_timer.h"


// PROTOTYPE FUNCITONS
void ctl_HC15C_OnTimerCounter0(CTL_ISR_FN_t);
void ISR_TimerCounter0(void);

#endif