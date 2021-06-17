/****************************************************************************************************
 * File name:    	SUPPORT.h
 * File type:		Visual C++ Windows Form CLI
 * Description:  	This is an include file for the .cpp file of the same name.  It also supports the 
 * main form written to the CLI (Form1.h).   This file define parameters are taken from the HC15C Protocol.
 * It would be unwise to try and understand this software without first reviewing the HC15C firmware as the two work hand in hand.
 * This software intended application is to be the Windows interface for the HC15C (Hab's Super Calculator device)
 *
 * Author:			Hab S. Collector
 * Date of Orgin:	12/13/11
 * Last Edited By:  Hab S. Collector
 * Last Edit Date:  7/07/12
 * Hardware:		Windows PC running Win7 OS
 * Development:		Visual Studion VC++ 2010 Express
 * Notes:           This is a Win32 Application.
 *					It will be necessary to consult the reference documents, associated HC15C firmware and perhaps the schematic
 *                  to understand the operations of this firmware.  
****************************************************************************************************/ 



#ifndef _SUPPORT_DEFINE
#define _SUPPORT_DEFINE

/**************************************USED COMMON ROUTINES*****************************************/
/* DEFINES FOR HAB EDIT NEXT LINE
# define HAB_EDIT */

// CRC-8 
#define	GP	0x0107
#define DI	0x07

// SERIAL PORT
#define DEFAULT_SERIAL_BPS "19200"


/**************************************USED IN TX ROUTINES******************************************/
// FORM NAME AND REV
#define FORM_NAME				"hc 15C Win App"
#define REV_MAJOR				1
#define REV_MINOR				0
#define SOFTWARE_DD_FILE_NAME   "HC15C SDD.pdf"
#define USER_MANUAL_FILE_NAME   "HC15C UMTS.pdf"
#define DOCUMENT_SHELL_NAME	    "PDF VIEWER"
#define VIDEO_HELP_FILE_NAME	"How To Video.MP4"
#define VIDEO_HELP_SHELL_NAME	"AVI PLAYER"

// PROTOCOL DEFINES USED WITH TX
#define ADDR1_INDEX			3
#define ADDR2_INDEX			4
#define TX_PAYLOAD_INDEX	5
// TARGET GROUP ADDRESS RANGE
#define LOWEST_ADDRESS	1
#define HIGHEST_ADDRESS	4095


/**************************************USED IN RX ROUTINES******************************************/
// PROTOCOL
// START PACKET
#define START_OF_PACKET	0xF0

// HC15C PROTOCOL MSG TYPES
// TYPE VALUES
#define TYPE_STOP		0xF0 // STOPS THE METER
#define TYPE_VOLT		0xF1 // FOR THE TRANSFER OF VOLT READING
#define TYPE_OHMS		0xF2 // FOR THE TRANSFER OF OHM VALUE READING
#define TYPE_TIME		0xF3 // TELLS THE APP TO ENABLE SET TIME
#define TYPE_TBD1		0xF4 
#define TYPE_TBD2		0xF5
#define TYPE_TBD3		0xF0

// PROTOCOL INDEX AND SIZE
#define START_INDEX			0
#define MSG_TYPE_INDEX		1
#define LENGTH_INDEX		2
#define RX_PAYLOAD_INDEX	3
#define MAX_PACKET_SIZE		0xFE

// VOLT METER RANGES
#define LOW_RNG_MIN 0
#define LOW_RNG_MED 5
#define LOW_RNG_MAX 10
#define MED_RNG_MIN 10
#define MED_RNG_MED 15
#define MED_RNG_MAX 20
#define MAX_RNG_MIN 20
#define MAX_RNG_MED 25
#define MAX_RNG_MAX 30
// ACTIVE * LABEL UPDATE INTERVAL
#define ACTIVE_LABEL_UPDATE 3

// USED WITH TEXTBOX SCROLLING
#define LINEAR_TEXTBOX_BUFFER_SIZE	20				// TOTAL NUMBER OF LINES TEXT BOX WILL BUFFER - TOTAL DISPALY IS 2X THIS NUMBER
#define TEXTBOX_EOL_CHAR			'\n'
#define OPEN_CIRCUIT_READING		-999

// STRUCTS AND ENUMBS
typedef enum 
{
	OHM,
	VOLT
}SELECTED_SET_TEXT;

// ALL PROTOTYES
// COMMON PROTOTYPES
void crc8(unsigned char *, unsigned char);
bool checkUDC_Address(void);
// TX PROTOTYPES
void init_crc8(void);
void createTX_Buffer(const unsigned char *, int, unsigned char *, int);
int getPayID(int);
// RX PROTOTYPES
bool check_packet_crc(unsigned char *, int);
void NGLF_ParsingEngine(unsigned char *, int, int);



#endif