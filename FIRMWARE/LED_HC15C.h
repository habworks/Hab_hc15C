/*****************************************************************
 *
 * File name:       LED_HC15C.H
 * Description:     Project definitions and function prototypes for use with LED_HC15C.c
 * Author:          Hab S. Collector
 * Date:            7/4/2011
 * Hardware:               
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent 
 *                  on other includes - everything these functions need should be passed to them
 *
 *****************************************************************/ 

#ifndef _LED_HC15C_DEFINES
#define _LED_HC15C_DEFINES


// INCLUDES
#include "HC15C_DEFINES.h"
#include "CMSIS\lpc17xx_gpio.h"

// ENUMERATED TYPES
enum LED_STATES
     {
     LED_OFF,
     LED_ON,
     LED_BLINK
     };

// DEFINES

// PROTOTYPE FUNCITONS
void initLED(void);
void switchLED (uint8_t, uint32_t);

#endif