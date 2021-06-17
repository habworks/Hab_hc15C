/*****************************************************************
 *
 * File name:       TOUCH_TASKS.H
 * Description:     Project definitions and function prototypes for use with POWER_TASKS.c
 * Author:          Hab S. Collector
 * Date:            9/25/2011
 * LAST EDIT:       7/03/2012
 * Hardware:               
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes
 *                   - everything these functions need should be passed to them. This include 
 *                  supports CTL RTOS functions
 *
 *****************************************************************/ 

#ifndef _TOUCH_TASKS_DEFINES
#define _TOUCH_TASKS_DEFINES

#include "HC15C_DEFINES.H"

// ENUMERATED TYPES 


// STRUCTURES


// DEFINES
// TOUCH KEYS:
#define MASK_B_TOUCH_DATA   0x80000000
#define ALL_KEYS_SELECTED   0xFFFFFFFF
#define NO_KEYS_SELECTED    0x00
#define MASK_ATN_KEY        ((uint32_t)(1<<0))
#define MASK_KEY_0          ((uint32_t)(1<<3))
#define MASK_KEY_1          ((uint32_t)(1<<4))
#define MASK_KEY_2          ((uint32_t)(1<<15))
#define MASK_KEY_3          ((uint32_t)(1<<0))
#define MASK_KEY_4          ((uint32_t)(1<<7))
#define MASK_KEY_5          ((uint32_t)(1<<22))
#define MASK_KEY_6          ((uint32_t)(1<<23))
#define MASK_KEY_7          ((uint32_t)(1<<19))
#define MASK_KEY_8          ((uint32_t)(1<<11))
#define MASK_KEY_9          ((uint32_t)(1<<12))
#define MASK_KEY_ENTER      ((uint32_t)(1<<5))
#define MASK_KEY_CAL        ((uint32_t)(1<<20)) // FUNCTION KEY FOR CAL
#define MASK_KEY_METER      ((uint32_t)(1<<21)) // FUNCTION KEY FOR METER FUNCTIONS VOLTS & OHMS
#define MASK_KEY_ALINK      ((uint32_t)(1<<8))  // FUNCTION KEY FOR VCOMM LINK & UNLINK
#define MASK_KEY_FLASH      ((uint32_t)(1<<9))  // FUNCTION KEY FOR FLASH DRIVE CONTENTS
#define MASK_KEY_MUSIC      ((uint32_t)(1<<10)) // FUNCTION KWY FOR AUDIO PLAY & HC15C SETUP
#define MASK_KEY_LSHIFT     ((uint32_t)(1<<15))
#define MASK_0TO9_ONLY      (MASK_KEY_0|MASK_KEY_1|MASK_KEY_2|MASK_KEY_3|MASK_KEY_4|MASK_KEY_5|MASK_KEY_6|MASK_KEY_7|MASK_KEY_8|MASK_KEY_9)

// ALIAS TOUCH KEY NAMES
#define MASK_KEY_UP     MASK_KEY_8
#define MASK_KEY_DOWN   MASK_KEY_2
#define MASK_KEY_STOP   MASK_KEY_5
#define MASK_KEY_START  MASK_KEY_6
#define MASK_RESET_SWAT MASK_KEY_4
#define MASK_STOP_SWAT  MASK_KEY_5
#define MASK_START_SWAT MASK_KEY_6
#define MASK_VMAX_RST   MASK_KEY_4

// PROTOTYPES
void click_taskFn(void *);
BOOLEAN filter_KeyPressEvent(uint32_t *);
void select_key_functionA(uint32_t);
void select_key_functionB(uint32_t);
void select_All_Normal_Keys(void);

#endif