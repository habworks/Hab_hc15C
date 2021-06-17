/*****************************************************************
 *
 * File name:       POWER_TASKS.H
 * Description:     Project definitions and function prototypes for use with POWER_TASKS.c
 * Author:          Hab S. Collector
 * Date:            9/25/2011
 * LAST EDIT:       8/01/2012
 * Hardware:               
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes
 *                   - everything these functions need should be passed to them. This include 
 *                  supports CTL RTOS functions
 *
 *****************************************************************/ 

#ifndef _POWER_TASKS_DEFINES
#define _POWER_TASKS_DEFINES


// INCLUDES
#include "HC15C_DEFINES.h"

// ENUMERATED TYPES


// STRUCTURES

// USB POWER
#define MIN_USB_VOLTAGE 4.5
// DEFINES CELL CHARGE FOR 2 CELLS (CELL = 3.6 Li 1/2AA)
#define Q_10_PERCENT   (3.15 * 2)
#define Q_25_PERCENT   (3.20 * 2)
#define Q_50_PERCENT   (3.25 * 2)
#define Q_75_PERCENT   (3.30 * 2)
#define Q_100_PERCENT  (3.35 * 2)


// PROTOTYPE FUNCITONS
void batQ_taskFn(void *);

#endif