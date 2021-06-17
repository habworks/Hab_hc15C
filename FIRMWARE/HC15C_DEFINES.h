/*****************************************************************
 *
 * File name:     HC15C_DEFINES.h
 * Description:   Project definitions and hardware pin assignments
 * Author:        Hab S. Collector
 * Date:          07/03/11
 * Hardware:      PCB-HC15C REV1
 * Firmware Tool: Rowley CrossStudio 
 *
*****************************************************************/ 
#ifndef _HC15C_DEFINES
#define _HC15C_DEFINES


// GENERIC TYPE DECLARATIONS
typedef unsigned char BOOLEAN;        // 1 byte  0 or 1 FALSE or TRUE
typedef unsigned char uint8_t;        // 1 byte  0 to 255   
typedef signed char int8_t;           // 1 byte -127 to 127 
typedef unsigned short int uint16_t;  // 2 bytes 0 to 65535 
typedef signed short int int16_t;     // 2 bytes -32767 to 32767 
typedef unsigned long int uint32_t;   // 4 bytes 0 to 4294967295 
typedef signed long int int32_t;      // 4 bytes -2147483647 to 2147483647 

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


// GENERIC DEFINES
// #define FALSE 0
// #define TRUE  !FALSE

// POR INTRODUCTION SCREEN
// LCD DISPLAY           01234567890123456789
#define LCD_INTRO_LINE1 "      HC 15C"
#define LCD_INTRO_LINE2 " RPN CALCULATOR"
#define LCD_INTRO_LINE3 " HabWorks DESIGNS"
#define LCD_INTRO_LINE4 " VER: 1.0"

// USED IN SOME SERIAL COMM
#define DO_NOTHING '|'    // DO NOT EXPECT USER TO PRESS THIS CHAR
#define NO_KEY_PRESSED 999

// USED IN INT ROUNDING
#define ROUNDING 0.5

//DEFINE THE HARDWARE BY PORT ASSIGMENTS
// LED INTERFACE - CAL STATUS DISPLAY
#define LED_USB    ((uint32_t)(1<<18))      // P1.18 USB CONNECT
#define LED_DRIVE  ((uint32_t)(1<<19))      // P1.19    
#define LED_METER  ((uint32_t)(1<<20))      // P1.20
#define LED_HEX    ((uint32_t)(1<<21))      // P1.21
#define LED_RAD    ((uint32_t)(1<<22))      // P1.22
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
// ANALOG OUT
#define AOUT        ((uint32_t)(1<<26))     // P0.26 DAC OUT


//WAIT TIMEs IN ms
#define DEBOUNCE_TIME        100        
#define INTRO_SPLASH_WAIT    3000
#define SHOW_USER_QUICKLY    500
#define SHOW_USER_SLOWLY     2000

//INTERRUPT VIC MASK
#define I2C0_IRQ    9
#define I2C1_IRQ    19
#define TIMER0_IRQ  4
#define TIMER1_IRQ  5

//TASKING PRIORITY
enum TASKING_PRIORITY
    {
    TIMER0_IRQ_PRIORITY,
    TIMER1_IRQ_PRIORITY,
    I2C0_IRQ_PRIORITY,
    EINT0_IRQ_PRIORITY
    };


#endif
