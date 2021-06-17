/*****************************************************************
 *
 * File name:         SETUP_TASKS.H
 * Description:       Project definitions and function prototypes for use with SETUP_TASKS.c
 * Author:            Hab S. Collector
 * Date:              3/5/2011
 * Hardware:               
 * Firmware Tool:     CrossStudio for ARM
 * Notes:             This file should be written as to not be dependent 
 *                    on other includes - everything these functions need should be passed to them
*****************************************************************/ 

#ifndef _SETUP_TASKS_DEFINES
#define _SETUP_TASKS_DEFINES

// INCLUDES
#include "HC15C_DEFINES.H"

// SCREEN LOCATIONS
#define SETUP_BL_ROW       2
#define SETUP_CAL_TALK_ROW 3
#define SETUP_TIMEOUT_ROW  4
#define SETUP_POSITION     17

// SETUP LIMITS (VALUE IN 10'S OF SECONDS)
#define MAX_TIME_TO_SLEEP         20
#define MIN_TIME_TO_SLEEP         6
#define DEFAULT_TIME_TO_SLEEP     6
#define MAX_TIME_TO_BACKLIGHT     6  
#define MIN_TIME_TO_BACKLIGHT     0
#define DEFAULT_BACKLIGHT_TIMEOUT 3


// PROTOTYPES
void setup_taskFn(void *);
void call_SetupMode(void);
void call_SetupModeEnd(void);
void call_BL_TimeOutSetup(void);
void call_CalVerboseSetup(void);
void call_TimeToSleepSetup(void);

#endif