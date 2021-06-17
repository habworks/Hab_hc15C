/*****************************************************************
 *
 * File name:         DIP204.H
 * Description:       Project definitions and function prototypes for use with DIP204.c
 * Author:            Hab S. Collector
 * Date:              6/9/2011
 * Hardware:               
 * Firmware Tool:     CrossStudio for ARM
 * Notes:             This file should be written as to not be dependent 
 *                    on other includes - everything these functions need should be passed to them
*****************************************************************/ 

#ifndef _DIP204_DEFINES
#define _DIP204_DEFINES


// INCLUDES
#include "HC15C_DEFINES.h"


// GLOBAL VARS

// EXTERNS DECLARED

// ENUMERATED TYPES AND STRUCTURES
enum DIP204_CURSOR
     {
     CURSOR_ON,
     CURSOR_OFF,
     CURSOR_BLINK
     };

// DEFINES
// SPI DEFINES
#define SSPI_DIP204_CLK       50000   // VALUE IN Hz
#define DIP204_RST_DELAY_TIME 5       // TIME IN ms
#define DISPLAY_LINE_TOTAL    4
#define DISPLAY_COLUMN_TOTAL  20
#define WAIT_FOR_DISPLAY      2       // TIME IN ms A NECESSARY WAIT BETWEEN SUCESSIVE DISPLAY FUNCTION CALLS

// ROW COLUMN VALUES
#define LINE1_START_ADDRESS 0x00
#define LINE2_START_ADDRESS 0x20
#define LINE3_START_ADDRESS 0x40
#define LINE4_START_ADDRESS 0x60

// LCD DISPLAY INSTRUCTION COMMANDS
// LCD START BYTE COMMAND VALUES
#define START_BYTE_CMD_WRITE 0xF8
#define START_BYTE_CMD_READ  0xFC
// LCD START BYTE DATA VALUES
#define START_BYTE_DAT_WRITE 0xFA
#define START_BYTE_DAT_READ  0xFE

/* LCD COMMAND VALUES
 * COMMAND VALUES AND VALUES THAT CAN BE MASKED WITH THE BASE
 * COMMAND.  NOTE SOME COMMANDS REQUIRE YOU TO HAVE THE RE BIT
 * SET.  THE RE BIT IS SET FROM THE FUNCTION SET COMMAND */
// CLEAR DISPLAY ADDRESS TO 00 
#define CMD_CLEAR_DISPLAY   0x01
// CURSOR HOME
#define CMD_CURSOR_HOME     0x02
  #define MASK_SLEEP        0X01      // RE = 1
// FUNCTION SET
#define CMD_FUNCTION        0x20
  #define MASK_8BIT         0x10
  #define MASK_SET_RE       0x04
  #define MASK_SHIFTBLINK   0x02
// EX FUNCTION - REQUIRES RE BIT PREVIOUSLY SET
#define CMD_EX_FUNCTION     0x08
  #define MASK_4LINE        0x01      // RE = 1
// SET SEGRAM ADR - REQIRES RE BIT PREVIOUSLY SET
#define CMD_SET_SEG_ADDR    0x40   // MASK WITH LOWER NIBBLE FOR ADDRESS VALUE
// DISPLAY COMMANDS
#define CMD_DISPALY         0x08
  #define MASK_DISPLAY_ON   0x04
  #define MASK_CURSOR_ON    0x02
  #define MASK_CURSOR_BLINK 0x01
// DDRAM ADDRESS
#define CMD_SET_DDR_ADDR    0x80

// LCD MISC
#define LCD_CHAR_WIDTH 20
#define BUSY_FLAG_MASK 0x80

// LCD ICON STATUS VALUES ARE BROKEN INTO TWO CATEGORIES.  NON-BATTERY AND BATTERY (ICON_BATTERY).
// ICON SEGRAM ADDRESS VALUES
#define ICON_PHONE               0x00
#define ICON_RING                0x01
#define ICON_MAIL                0x02       // SIGNIFIES STAT VALUES ARE STORED
#define ICON_INFO                0x03       
#define ICON_WAIT                0x04
#define ICON_CALCULATOR          0x05
#define ICON_DRIVE               0x06
#define ICON_TEMP                0x07       // RADIANT ICON FOR RAD ANGLE MODE
#define ICON_TIME                0x08       // USE IN CLOCK MODE
#define ICON_SPEAKER             0x09
#define ICON_LEFT_ARROW          0x0A
#define ICON_RIGHT_ARROW         0x0B
#define ICON_UP_ARROW            0x0C
#define ICON_DOWN_ARROW          0x0D
#define ICON_ALERT               0x0E
#define ICON_BATTERY             0x0F        // USE ONLY WITH BAT ICON STATUS
// NON BATTERY ICON STATUS
#define ICON_OFF                 0x00        // WORKS FOR NON-BAT AND BAT ICONS
#define ICON_ON                  0x10
#define ICON_BLINK               0x50
// BATTERY ICON STATUS
#define ICON_BAT_FULL            0x1F
#define ICON_BAT_3QTR            0x1E
#define ICON_BAT_2QTR            0x1C
#define ICON_BAT_1QTR            0x18
#define ICON_BAT_EMPTY           0x10
#define ICON_BAT_EWARN           0x50        // THIS IS THE EMPTY ICON BLINKING

// PROTOTYPE FUNCTIONS
void reset_DIP204(void);
void init_SSPI1(uint32_t);
void init_DIP204(void);
void DIP204_engine(uint8_t, uint8_t);
void DIP204_ICON_set(uint8_t, uint8_t);
uint8_t reverseBitOrder(uint8_t);
void DIP204_txt_engine(uint8_t *, uint8_t, uint8_t, uint8_t);
void DIP204_cursorToXY(uint8_t, uint8_t);
void DIP204_clearLine(uint8_t);
uint8_t DIP204_loadText(uint8_t *, uint8_t);
void init_PWM_BackLight(void);
void DIP204_set_cursor(enum DIP204_CURSOR);
void DIP204_clearDisplay(void);
void DIP204_DisplayOff(void);

#endif