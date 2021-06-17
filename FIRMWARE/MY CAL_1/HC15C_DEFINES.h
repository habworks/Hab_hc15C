/*****************************************************************
 *
 * File name:     HC15C_DEFINES.h
 * Description:   Project definitions and hardware pin assignments
 * Author:        Hab S. Collector
 * Date:          07/03/11
 * LAST EDIT:     7/19/2012 
 * Hardware:      PCB-HC15C REV2
 * Firmware Tool: Rowley CrossStudio 
 ******************************************************************/ 
#ifndef _HC15C_DEFINES
#define _HC15C_DEFINES

// DEFINE FIRMWARE REVISION
#define MAJOR_REV "1"
#define MINOR_REV "1"

// GENERIC TYPE DECLARATIONS
typedef unsigned char BOOLEAN;        // 1 byte  0 or 1 FALSE or TRUE
typedef unsigned char uint8_t;        // 1 byte  0 to 255   
typedef signed char int8_t;           // 1 byte -127 to 127 
typedef unsigned short int uint16_t;  // 2 bytes 0 to 65535 
typedef signed short int int16_t;     // 2 bytes -32767 to 32767 
typedef unsigned long int uint32_t;   // 4 bytes 0 to 4294967295 
typedef signed long int int32_t;      // 4 bytes -2147483647 to 2147483647 
/*
#ifndef BOOLEAN
  #ifdef FALSE 
    #undef FALSE
  #endif
  #ifdef TRUE
    #undef TRUE
  #endif  
  typedef enum { FALSE = 0, TRUE } BOOLEAN;
  #define BOOLEAN
#endif
*/    


// DEFINE ENUMERATED TYPES
// DEFINE PORTS
enum PORT_NUMBERS
   {
   PORT0,
   PORT1,
   PORT2,
   PORT3,
   PORT4
   };

// POR INTRODUCTION SCREEN
// LCD DISPLAY           01234567890123456789
#define LCD_INTRO_LINE1 "       hc 15C"
#define LCD_INTRO_LINE2 "   RPN CALCULATOR"
#define LCD_INTRO_LINE3 "  HabWorks Designs"
#define LCD_INTRO_LINE4 "      VER: "

// USED IN INT ROUNDING
#define ROUNDING 0.5

// DEFINE THE HARDWARE BY PORT ASSIGMENTS
// LED INTERFACE - CAL STATUS DISPLAY
#define LED_USB    ((uint32_t)(1<<18))      // P1.18 USB CONNECT
#define LED_BAR0   ((uint32_t)(1<<24))      // P1.24
#define LED_BAR1   ((uint32_t)(1<<23))      // P1.23
#define LED_BAR2   ((uint32_t)(1<<22))      // P1.22
#define LED_BAR3   ((uint32_t)(1<<21))      // P1.21
#define LED_BAR4   ((uint32_t)(1<<20))      // P1.20
#define LED_BAR5   ((uint32_t)(1<<19))      // P1.19

// SSP1 DIP204 GPIO INTERFACE
#define LCD_CS     ((uint32_t)(1<<6))       // P0.6
#define LCD_RST    ((uint32_t)(1<<22))      // PO.22
// ADC METER PINS
#define RANGE_10V   ((uint32_t)(1<<4))      // P1.1  ADC 10V SCALE
#define RANGE_20V   ((uint32_t)(1<<8))      // P1.8  ADC 20V SCALE
#define RANGE_30V   ((uint32_t)(1<<9))      // P1.9  ADC 30V SCALE
#define SEL_V_C     ((uint32_t)(1<<1))      // P1.1 HI FOR VOLTAGE / LO FOR CONTINUNITY
// STMPE24M31 IRQ PINS
#define INT_ACAP    ((uint32_t)(1<<12))     // P2.12 EINT2 CH A CAP TOUCH IRQ
#define INT_BCAP    ((uint32_t)(1<<11))     // P2.11 EINT1 CH B CAP TOUCH IRQ
// AUDIO OUT
#define AOUT        ((uint32_t)(1<<26))     // P0.26 DAC OUT
#define PWR_AUDIO   ((uint32_t)(1<<19))     // P0.19  
// LCD RESET
#define LCD_RESET   ((uint32_t)(1<<22))     // P0.22 LCD RESET ACTIVE LOW

// PCONP POWER
#define PCUART0   ((uint32_t)(1<<3))
#define PCUART1   ((uint32_t)(1<<4))
#define PCTIM1    ((uint32_t)(1<<2))

// TASKING 
// TASKING VALUES
#define SYSTEM_TICK_VALUE 1   // VALUE IN ms
// TASK EVENTS
#define EVENT_INIT          ((uint16_t)(1<<0))
#define EVENT_D_SLEEP       ((uint16_t)(1<<1))
#define EVENT_BAT_Q         ((uint16_t)(1<<2))
#define EVENT_ALARM         ((uint16_t)(1<<3))
#define EVENT_BACK_L        ((uint16_t)(1<<4))
#define EVENT_CLOCK         ((uint16_t)(1<<5))
#define EVENT_SWAT          ((uint16_t)(1<<6))
#define EVENT_METER         ((uint16_t)(1<<7))
#define EVENT_OHMS          ((uint16_t)(1<<8))
#define EVENT_OHMS_UI       ((uint16_t)(1<<9))
#define EVENT_SD_LIST       ((uint16_t)(1<<10))
#define EVENT_SD_UI         ((uint16_t)(1<<11))
#define EVENT_MAINT         ((uint16_t)(1<<12))
#define EVENT_MUSIC_LIST    ((uint16_t)(1<<13))
#define EVENT_SETUP         ((uint16_t)(1<<14))
// MESSAGE QUEUES
#define MAX_TOUCH_MSG       20
#define MAX_AUDIO_MSG       20

// INTERRUPTS
// PRIORITY
enum TASKING_IRQ_PRIORITY
  {
  EINT1_IRQ_PRIORITY = 1,  // TOUCH CH B IRQ - HIGHEST PRIORITY
  EINT2_IRQ_PRIORITY,      // TOUCH CH A IRQ
  TIMER2_IRQ_PRIORITY,     // AUDIO PLAYBACK TIMER
  TIMER0_IRQ_PRIORITY,     // GENERIC TIMER HAS MULTIPLE USES  
  EINT0_IRQ_PRIORITY,      // EXTERNAL WAKE FROM SLEEP IRQ
  RTC_IRQ_PRIORITY,        // RTC IRQ FOR CLOCK
  TIMER1_IRQ_PRIORITY      // USED WITH OHMS & VOLT METER TO UPDATE READOUT // CLOCK TO UPDATE TIME
  };

#endif
