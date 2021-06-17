
#include <StdAfx.h>
#include <stdio.h>
#include <Windows.h>
#include "SUPPORT.H"


static unsigned char crc8_table[256];     /* 8BIT TABLE */
static bool MadeTable = false;



/************************************************************************************************************************/
/**********************************************RX SUPPORT ROUTINES*******************************************************/
/************************************************************************************************************************/

/*************************************************************************
 * Function Name: check_packet_crc
 * Parameters: unsigned char *, int
 * Return: bool
 *
 * Description: Checks the passed packet (array), and passed size to see if the
 * check sum is valid.  If so, return true.  The checksum is calucated over the 
 * entire packet but does not include the SYN (first byte) or the check sum itself
 * (the last byte).
 * STEP 1: Assign pointer
 * STEP 2: Scan the packet and calculate the CS
 * STEP 3: Compare the calcualted CS against the packet CS and return accordingly
 *************************************************************************/
bool check_packet_crc(unsigned char *PacketToCheck, int SizeOfPacket)
{
	unsigned char CheckSum = 0;
	unsigned char *ptr_PacketToCheck;

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
 * Function Name: createTX_Buffer
 * Parameters: constant unsigned char *, int, unsigned char *, int, bool
 * Return: By Reference
 *
 * Description: Creates the packet message for a scenario buffer.  Adds the SYN,
 * Message Type, Length and CheckSum.  Takes the scene buffer, scene buffer size,
 * the packet buffer to fill and the message type.
 * STEP 1: Assign pointers to arrays and set checksum to 0
 * STEP 2: Load the first three bytes of the array: SYN, TYPE, LEN
 * STEP 3: Calculate the checksum and complete loading of the array
 * STEP 4: Assign the checksum as the last byte in the arry to load
 *************************************************************************/
void createTX_Buffer(const unsigned char LoadBuffer[], int LoadBufferSize, unsigned char ScenePacket[], int SceneMsgType)
{

	const unsigned char *ptrSceneLoadBuffer;
    unsigned char *ptrScenePacket;
	unsigned char CRC_CheckSum;

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
 * Function Name: init_crc8
 * Parameters: void
 * Return: Loads a static array and sets a static variable flag
 *
 * Description: Loads a CRC8 polynomial checksum array (256 bytes).  Sets the static
 * value to indicate the array is loaded. 
 * STEP 1: Load the array  x^8 + x^2 + x + 1
 * STEP 2: Set the static varible that the array has been loaded
 *************************************************************************/
void init_crc8()
{

	int i,j;
	unsigned char crc;
   
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
	MadeTable = true;
   }

} // END OF init_crc8





/*************************************************************************
 * Function Name: crc8
 * Parameters: unsigned char *, unsigned char
 * Return: Loads a static array and sets a static variable flag
 *
 * Description: For a byte array whose accumulated crc value is stored in *crc, computes
 * resultant crc obtained by appending m to the byte array.  Note, befor calling the crc must
 * be first set to 0.
 * STEP 1: Check if table loaded - if not load
 * STEP 2: Calculate the present checksum via use of crc table
 *************************************************************************/
void crc8(unsigned char *crc, unsigned char m)
{
   // STEP 1
	if (!MadeTable)
     init_crc8();
   
   // STEP 2
   *crc = crc8_table[(*crc) ^ m];
   *crc &= 0xFF;

} // END OF crc8
