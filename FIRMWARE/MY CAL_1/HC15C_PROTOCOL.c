/*****************************************************************
 *
 * File name:       HC15C_PROTOCOL.C
 * Description:     Functions used to support USB LINK features in the TX and RX of HC15C protocol
 * messages between the calculator and the HC15C Win App.  
 * Author:          Hab S. Collector
 * Date:            12/31/11
 * LAST EDIT:       7/06/2012
 * Hardware:        NXP LPC1769
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/

#include "HC15C_PROTOCOL.H"
#include "lpc_types.h"
#include <stdlib.h>
#include <string.h>

// GLOBALS
BOOLEAN MadeTable = FALSE;
uint8_t crc8_table[256];


/*************************************************************************
 * Function Name: createTX_Buffer
 * Parameters: constant unsigned char *, int, unsigned char *, int, bool
 * Return: By Reference
 *
 * Description: Creates the packet message for a scenario buffer.  Adds the SYN,
 * Message Type, Length and CheckSum.  Takes the payload buffer, and payload buffer size.
 * The payload is a union byte of a double value that is the value (volt / ohm) you are
 * transmitting.
 * NOTE: A completed packet would look like: 
 * F0 F1 08 FF E3 80 1E EC 6A 02 40 AE  THIS IS A VOLT PACKET FOR 2.302V
 * F0 F2 08 84 72 5E 40 C6 9C 84 40 26  THIS IS A OHM PACKET FOR 650 OHM
 * the packet buffer to fill and the message type.
 * STEP 1: Assign pointers to arrays and set checksum to 0
 * STEP 2: Load the first three bytes of the array: SYN, TYPE, LEN
 * STEP 3: Calculates the checksum as it complete loading of the array with data
 * STEP 4: Assign the checksum as the last byte in the array to load
 *************************************************************************/
void createTX_Buffer(const uint8_t LoadBuffer[], uint8_t LoadBufferSize, uint8_t ScenePacket[], uint8_t SceneMsgType)
{

	const uint8_t *ptrSceneLoadBuffer;
  uint8_t *ptrScenePacket;
	uint8_t CRC_CheckSum;

	// STEP 1
	ptrSceneLoadBuffer = LoadBuffer;
	ptrScenePacket = ScenePacket;
	CRC_CheckSum = 0;

	// STEP 2
	// LOAD THE FIRST 3 BYTES OF BUFFER WITH KNOW SETUP INFO
	ptrScenePacket[START_INDEX] = START_OF_PACKET;  // NO CHECKSUM FOR SYN
	// CHECKSUM FROM HERE UNTIL END OF DATA CALCULATED
	ptrScenePacket[MSG_TYPE_INDEX] = SceneMsgType;
	crc8(&CRC_CheckSum, ptrScenePacket[MSG_TYPE_INDEX]);
	ptrScenePacket[LENGTH_INDEX] = LoadBufferSize; 
	crc8(&CRC_CheckSum, ptrScenePacket[LENGTH_INDEX]);

	// STEP3
	// LOAD THE PAYLOAD BYTES AND CALCULATE THE CHECKSUM
	for (int ByteCount = 0; ByteCount < LoadBufferSize; ByteCount++)
	{
		ptrScenePacket[(ADDR1_INDEX + ByteCount)] = *ptrSceneLoadBuffer;
		crc8(&CRC_CheckSum, *ptrSceneLoadBuffer);
		ptrSceneLoadBuffer++;
	}

	// STEP 4
	// LOAD THE CHECKSUM: IT IS LAST BYTE OF PACKET
	ptrScenePacket[(LoadBufferSize + 3)] = CRC_CheckSum; // ADDED: SYN + TYPE + LEN AND START FROM 0

} // END OF createTX_Buffer




/************************************************************************************************************************/
/******************************************COMMON SUPPORT ROUTINES*******************************************************/
/************************************************************************************************************************/


/*************************************************************************
 * Function Name: check_packet_crc
 * Parameters: unsigned char *, int
 * Return: bool
 *
 * Description: Checks the passed packet (array), and passed size to see if the
 * check sum is valid.  If so, return true.  The checksum is calculated over the 
 * entire packet but does not include the SYN (first byte) or the check sum itself
 * (the last byte).
 * STEP 1: Assign pointer
 * STEP 2: Scan the packet and calculate the CS
 * STEP 3: Compare the calculated CS against the packet CS and return accordingly
 *************************************************************************/
BOOLEAN check_packet_crc(uint8_t *PacketToCheck, uint8_t SizeOfPacket)
{
	uint8_t CheckSum = 0;
	uint8_t *ptr_PacketToCheck;

	// STEP 1
	ptr_PacketToCheck = PacketToCheck;

	// STEP 2
	for (int ByteCount = 0; ByteCount < SizeOfPacket; ByteCount++)
	{
		if (ByteCount == 0)	// First Byte is SYN
		{
			ptr_PacketToCheck++;
			continue; // Compiler sees this as possible to generate WARNING C4715
		}
		if (ByteCount != (SizeOfPacket - 1)) // If not CS Byte
			crc8(&CheckSum, *ptr_PacketToCheck);
		else
		{
		// STEP 3
		if (CheckSum == *ptr_PacketToCheck)
			return (TRUE);
		else
			return (FALSE);
		}
		ptr_PacketToCheck++;
	}
	/* CODE SHOULD NEVER GET HERE - IF IT DID SOMETHNG WENT VERY WRONG
	 * THIS CODE IS ADDED TO REMOVE WARNIG C4715 'not all control paths return a value" */
    return(FALSE);	 

} // END OF check_packet_crc




/*************************************************************************
 * Function Name: init_crc8
 * Parameters: void
 * Return: Loads a static array and sets a static variable flag
 *
 * Description: Loads a CRC8 polynomial checksum array (256 bytes).  Sets the static
 * value to indicate the array is loaded. 
 * STEP 1: Load the array  x^8 + x^2 + x + 1
 * STEP 2: Set the static varible that the array has been loaded
 *************************************************************************/
void init_crc8(void)
{

	uint16_t i,j;
	uint8_t crc;
   
	// STEP 1
	if (!MadeTable) 
	{
		for (i=0; i<256; i++) 
		{
			crc = i;
			for (j=0; j<8; j++)
				crc = (crc << 1) ^ ((crc & 0x80) ? DI : 0);
			crc8_table[i] = crc & 0xFF;
		}

	// STEP 2
	MadeTable = TRUE;
  }

} // END OF init_crc8





/*************************************************************************
 * Function Name: crc8
 * Parameters: uint8_t *, uint8_t
 * Return: Loads a static array and sets a static variable flag
 *
 * Description: For a byte array whose accumulated crc value is stored in *crc, computes
 * resultant crc obtained by appending m to the byte array.  Note, befor calling the crc must
 * be first set to 0.
 * STEP 1: Check if table loaded - if not load
 * STEP 2: Calculate the present checksum via use of crc table
 *************************************************************************/
void crc8(uint8_t *crc, uint8_t m)
{
   // STEP 1
	if (!MadeTable)
     init_crc8();
   
   // STEP 2
   *crc = crc8_table[(*crc) ^ m];
   *crc &= 0xFF;

} // END OF crc8




/*************************************************************************
 * Function Name: parseDateTimeFromVCOM
 * Parameters: uint8_t *, Type_DateTimeVCOM *
 * Return: BOOLEAN
 *
 * Description: The function accepts the Set Time and Date packet of the HC15C
 * Win App.  The packet looks like:
 * XXXX:XXXX:XXXX:XXXX:XXXX:XXXX\R = MONTH:DATE:YEAR:HOUR:MINUTE:SECOND (30 BYTES). FOR EXAMPLE THE STRING
 * 0007:0004:2012:0022:0010:0015 IS A DATE OF 7/4/2012, 10:10:15PM - THE MSG IS TRANSMITTED WITH
 * A CR AT THE END OF THE SEQUENCE. THE MSG MUST BE DONE THIS WAY TO WORK WITH THE USB VCOM LIB 
 * VCOM_gets FUNCTION.
 * STEP 1: Copy 4 Chars from the packet string and convert to integer.
 * STEP 2: Since the string packet has a definite sequence to it: month, date, year, hour, minute, second
 * Take the converted integer and load it to the corresponding DateTimeVCOM strcut member
 *************************************************************************/
BOOLEAN parseDateTimeFromVCOM(uint8_t *TimeDateString, Type_DateTimeVCOM *DateTimeVCOM)
{
  uint8_t *ptr_TimeDateString;
  uint8_t TimeValueString[5];
  uint16_t TimeValue;
  
  ptr_TimeDateString = TimeDateString;
  
  // STEP 1
  // COPY 4 CHARS FROM THE PACKET STRING
  for (uint8_t TimeParameter = 0; TimeParameter < 6; TimeParameter++)
    {
    strncpy(TimeValueString, ptr_TimeDateString, 4);
    TimeValueString[4] = '\0';
    TimeValue = atoi(TimeValueString);
    // ADVANCE THE POINTER FOR THE NEXT READ - NEXT TIME PARAMETER (4+1)
    ptr_TimeDateString += 5;
    
    // STEP 2
    switch(TimeParameter)
      {
      case MONTH:
      DateTimeVCOM->Month = TimeValue;
      break;
      
      case DATE:
      DateTimeVCOM->Date = TimeValue;
      break;
      
      case YEAR:
      DateTimeVCOM->Year = TimeValue;
      break;
      
      case HOUR:
      DateTimeVCOM->Hour = TimeValue;
      break;;
      
      case MINUTE:
      DateTimeVCOM->Minute = TimeValue;
      break;
      
      case SECOND:
      DateTimeVCOM->Second = TimeValue;
      break;
      
      default:
      // YOU SHOULD NEVER SEE THIS - IF YOU DID SOMTHING WENT WRONG
      return(FALSE);
      break;
      }
    
    }

return(TRUE);

} // END OF parseDateTimeFromVCOM