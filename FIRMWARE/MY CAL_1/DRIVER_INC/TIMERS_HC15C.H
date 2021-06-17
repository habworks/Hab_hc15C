/*****************************************************************
 *
 * File name:       TIMERS_HC15C.H
 * Description:     Project definitions and function prototypes for use with TIMERS_HC15C.c
 * Author:          Hab S. Collector
 * Date:            9/22/2011
 * Hardware:               
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent 
 *                  on other includes - everything these functions need should be passed to them
 *
 *****************************************************************/ 
 
#ifndef _TIMER_HC15C_DEFINES
#define _TIMER_HC15C_DEFINES
  
#include "HC15C_DEFINES.H"
  
// ENUMERATED TYPES

// DEFINES

// PROTOTYPE FUNCITONS
void delayXms(uint32_t);
void SysTimerCallFromISR(void);
void init_HC15C_OnTimerCounter0(uint32_t);
void TIMER0_IRQHandler(void);
void init_HC15C_OnTimerCounter1(uint32_t);
void TIMER1_IRQHandler(void); 
void init_HC15C_OnTimerCounter2(uint32_t);
void TIMER2_IRQHandler(void); 

#endif