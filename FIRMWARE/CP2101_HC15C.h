/*****************************************************************
 *
 * File name:       CP2101.H
 * Description:     Project definitions and function prototypes for use with CP2101_HC15C.c
 * Author:          Hab S. Collector
 * Date:            7/21/2011
 * Hardware:               
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent 
 *                  on other includes - everything these functions need should be passed to them
 *
 *****************************************************************/ 

#ifndef _CP2101_DEFINES
#define _CP2101_DEFINES


// INCLUDES
#include "HC15C_DEFINES.h"
#include "CMSIS\lpc17xx_pinsel.h"
#include "CMSIS\lpc17xx_uart.h"


// ENUMERATED TYPES AND STRUCTURES


// STRUCTURES


// DEFINES
#define CP2101_TEST_STRING   "Hello Hab from HC 15C\r\n"


// PROTOTYPE FUNCITONS
void init_CP2101_polling(void);
void CP2101_testprint(void);


#endif