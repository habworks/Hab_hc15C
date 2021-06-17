/*****************************************************************
 *
 * File name:         HC15C_PROTOCOL.H
 * Description:       Project definitions and function prototypes for use with HC15C_PROTOCOL.c
 * The HC15C protocol allows communication between the HC15C and the HC15C Win App
 * Author:            Hab S. Collector
 * Date:              12/31/2011
 * LAST EDIT:         7/07/2012
 * Hardware:               
 * Firmware Tool:     CrossStudio for ARM
 * Notes:             This file should be written as to not be dependent 
 *                    on other includes - everything these functions need should be passed to them
*****************************************************************/

#ifndef _HC15C_PROTOCOL_DEFINES
#define _HC15C_PROTOCOL_DEFINES

// INCLUDES
#include "HC15C_DEFINES.h"

/**************************************USED COMMON ROUTINES*****************************************/
/* DEFINES FOR HAB EDIT NEXT LINE
# define HAB_EDIT */

// CRC-8 
#define	GP	0x0107
#define DI	0x07

// SERIAL PORT
#define DEFAULT_SERIAL_BPS 19200


/**************************************USED IN TX ROUTINES******************************************/
// PROTOCOL DEFINES USED WITH TX
#define ADDR1_INDEX			  3
#define ADDR2_INDEX			  4
#define TX_PAYLOAD_INDEX	5
// TARGET GROUP ADDRESS RANGE
#define LOWEST_ADDRESS	  1
#define HIGHEST_ADDRESS	  4095


/**************************************USED IN RX ROUTINES******************************************/
// PROTOCOL
// START PACKET
#define START_OF_PACKET	0xF0

// HC15C PROTOCOL MSG TYPES
// TYPE VALUES
#define TYPE_STOP		0xF0 // STOPS THE METER
#define TYPE_VOLT		0xF1 // FOR THE TRANSFER OF VOLT READING
#define TYPE_OHMS		0xF2 // FOR THE TRANSFER OF OHM VALUE READING
#define TYPE_TIME 	0xF3 // ENABLES THE WIN APP TO SET THE HC15C TIME AND DATE
#define TYPE_TBD1		0xF4 // TBD - FUTURE USE
#define TYPE_TBD3		0xF5 // TBD - FUTURE USE
#define TYPE_TBD4		0xF0 // TBD - FUTURE USE

// PROTOCOL INDEX AND SIZE
#define START_INDEX             0
#define MSG_TYPE_INDEX          1
#define LENGTH_INDEX            2
#define RX_PAYLOAD_INDEX        3
#define MAX_PACKET_SIZE_HC15C		0xFE

// MISC
#define TEST_STRING "0007:0004:2012:0022:0010:0015" // 7/4/2012 10:10:15PM - USED FOR TESTING

// STRUCTS & ENUMS
typedef struct
  {
  uint8_t   Month;
  uint8_t   Date;
  uint16_t  Year;
  uint8_t   Hour;
  uint8_t   Minute;
  uint8_t   Second;
  } Type_DateTimeVCOM;

enum TIME_PARAMETER
  {
  MONTH,
  DATE,
  YEAR,
  HOUR,
  MINUTE,
  SECOND
  };




// ALL PROTOTYES
// COMMON PROTOTYPES
void crc8(uint8_t *, uint8_t);
void init_crc8(void);
BOOLEAN check_packet_crc(uint8_t *, uint8_t);
// TX PROTOTYPES
void createTX_Buffer(const uint8_t *, uint8_t, uint8_t *, uint8_t);
// RX PROTOTYPES
BOOLEAN parseDateTimeFromVCOM(uint8_t *, Type_DateTimeVCOM *);



#endif