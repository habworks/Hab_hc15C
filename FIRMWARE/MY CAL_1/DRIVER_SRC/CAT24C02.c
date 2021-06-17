/*****************************************************************
 *
 * File name:       CAT24C02.C
 * Description:     Functions used to support 24C02 256x8 OR 128x8 EEPROM memory
 * Author:          Hab S. Collector
 * Date:            7/4/11
 * Hardware:        NXP LPC1769
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 

#include "CAT24C02.H"
#include "TIMERS_HC15C.H"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2c.h"
 
 static void I2C_Stop (LPC_I2C_TypeDef *I2Cx);

 /*************************************************************************
 * Function Name: init_I2C1
 * Parameters:    uint32_t
 * Return:        void
 *
 * Description: Sets up I2C1 for interface at passed clk frequency to a CAT24C0x type EEPROM
 * ON I2C1.  Works with function CAT24C0X_write and _read
 * 
 * STEP 1: Setup the P0.0 and P0.1 to act as I2C1
 * STEP 2: Init I2C1 at the desired I2C frequency and enable
 *************************************************************************/
 void init_I2C1(uint32_t I2C_Frequency)
 {
  // PIN TYPE DEFINITION
  // FROM TABLE 8.5 USER MANUAL LPC17XX
  PINSEL_CFG_Type PinCfgStruct;
  
  // STEP 1
  PinCfgStruct.OpenDrain = 0;
  PinCfgStruct.Pinmode = 0;
  PinCfgStruct.Funcnum = 3;
  PinCfgStruct.Pinnum = 0;
  PinCfgStruct.Portnum = PORT0;
  PINSEL_ConfigPin(&PinCfgStruct);
  PinCfgStruct.Pinnum = 1;
  PINSEL_ConfigPin(&PinCfgStruct);
  
  // STEP 2
  I2C_Init(LPC_I2C1, I2C_Frequency);
  I2C_Cmd(LPC_I2C1, ENABLE);
  
 } // END OF FUNCTION init_I2C1




 /*************************************************************************
 * Function Name: CAT24C0X_write
 * Parameters:  uint8_t *, uint8_t, uint8_t, uint8_t
 * Return:      uint8_t
 *
 * Description: The power down store function of the calculator works by building
 * an array with the bytes to store, then storing all the values in a single call
 * to this function.  As the EEPROM can only store 16 bytes MAX at a time, the memory
 * must be parsed and stored in blocks (pages) of 16 bytes (or less - the last store - as
 * needed).  
 * NOTE: Uses CMSIS I2C transfer functions.  Requires the I2C be previously initialize
 * STEP 1: Check make sure you are not writing more data than the EEPROM can fit and assign pointer to data
 * STEP 2: Until all data is written check to see if a page worth of data is left. If so, set 
 * the writing of data to page size (16+1), if not, set as the difference as what is left to be 
 * written.
 * STEP 3: If this is the very first byte written it is 0 (address 0) if not it is the index from
 * the loop.  The first byte is the address location
 * STEP 4: Write the data (16 bytes if full page)
 * STEP 5: Store the page to EEPROM
 * NOTE: STEPS 1-4 BUILD A PAGE ARRAY TO STORE. THE ARRAY IS SUCH THAT BYTE 0 = THE LOCATION
 * IN MEMORY WHERE THE DATA (BYTE 1-16) WILL BE STORE IN CONSECUTIVE ORDER.  STEP 5 IS THE ACTUAL
 * STORAGE OPERATION TO THE EEPROM
 *************************************************************************/
 uint8_t CAT24C0X_write(uint8_t *WriteArray, uint8_t MemoryWriteSize, uint8_t MemoryAddress, uint8_t MemorySlaveAddress)
 {
  
  BOOLEAN AllDataWritten = FALSE;
  static uint8_t TXWriteArray[(16 + 1)];

  uint8_t *ptr_WriteArray;
  uint8_t MemoryIndex = 0,
          BytesToWrite = 0,
          MemoryWriteArrayIndex = 0,
          BytesRemaining = MemoryWriteSize,
          MemorySize = MemoryWriteSize;

  // STEP 1
  init_I2C1(I2C_24C0X_FREQUENCY);
  ptr_WriteArray = WriteArray;
  if ((MemorySize + MemoryAddress) > MAX_EEPROM_SIZE)
    return(FALSE);
  
  do
    {
    // STEP 2
    if ((MemorySize - MemoryIndex) > 16)
      {
      // A FULL WIRTE IS 17 BYTES - LOCATION + 16 DATA BYTES
      BytesToWrite = 17;
      }
    else
      {
      BytesToWrite = (MemorySize - MemoryIndex);
      AllDataWritten = TRUE;
      }
    
    // STEP 3:
    // LOAD THE FIRST BYTE AS THE ADDRESS OF MEMORY TO WRITE FROM
    MemoryWriteArrayIndex = 0;
    if (MemoryIndex == 0)
      {
      TXWriteArray[MemoryWriteArrayIndex] = *ptr_WriteArray;
      ptr_WriteArray++;
      BytesRemaining--;
      }
    else
      {
      TXWriteArray[MemoryWriteArrayIndex] = MemoryIndex;
      }
    MemoryWriteArrayIndex++;
    
    // STEP 4
    // LOAD THE PAGE OR UNTIL NO MORE BYTES
    while ((MemoryWriteArrayIndex < 17) && (BytesRemaining > 0))
      {
      TXWriteArray[MemoryWriteArrayIndex] = *ptr_WriteArray;
      ptr_WriteArray++;
      MemoryWriteArrayIndex++;
      MemoryIndex++;
      BytesRemaining--;
      }
    
    // STEP 5
    delayXms(10);
    I2C_Stop(LPC_I2C1);
    delayXms(10);
    CAT24C0X_write16(TXWriteArray, BytesToWrite, TXWriteArray[0], SLAVE_ADDRESS_24C0X);
    delayXms(10);
    I2C_Stop(LPC_I2C1);
    
    } while (AllDataWritten == FALSE);
  
  return(TRUE);
  
 } // END OF FUNCTION CAT24C0X_write




 /*************************************************************************
 * Function Name: CAT24C0X_read
 * Parameters:      uint8_t *, uint8_t, uint8_t, uint8_t
 * Return:          uint8_t
 *
 * Description: Reads the contents of the EEPROM memory.  The first parameter is the
 * Array in which the bytes read will be stored.  The size of the array MUST equal the 
 * the desired number of bytes to be read.  Next parameter is the size of the array, best passed
 * using sizeof(Array) operator.  Next parameter is the Memory Address of the first byte to
 * be read.  Last parameter is the slave address of the EEPROM in 7 bit form right shifted. 
 * The main function works by writing to the I2C device address the memory to read location, then 
 * it reads x bytes - x determined by the size of the read array
 * NOTE: Uses CMSIS I2C transfer functions.  Requires the I2C be previously initialize
 * NOTE: See also function CAT24C0X_write
 * A MAX OF 16 BYTES CAN BE READ AT ONE TIME.  BEYOND 16 WILL RESULT IN WEIRNESS
 * STEP 1: Creates a simple write array for use with the CMSIS function.  Sets the Memory read address as write array[0]
 * STEP 2: Check if the operation is valid based on array size and address
 * STEP 3: Load the I2C_M_SETUP_Type with the corresponding values
 * STEP 4: Read the data from the EEPROM return a true or false condition based on read result
 *************************************************************************/
 uint8_t CAT24C0X_read(uint8_t *MemoryToReadArray, uint8_t MemoryBytesToRead, uint8_t MemoryAddress, uint8_t MemorySlaveAddress)
 {
  // I2CM TYPDEF
  I2C_M_SETUP_Type RX_Setup;
  
  // STEP 1
  uint8_t WriteData[2];
  WriteData[0] = MemoryAddress;  
  
  // STEP 2
  if ((MemoryBytesToRead + MemoryAddress) > MAX_EEPROM_SIZE)
    return(FALSE);
  
  // STEP 3
  RX_Setup.sl_addr7bit = MemorySlaveAddress;
	RX_Setup.tx_data = WriteData;	// Get address to read at writing address
	RX_Setup.tx_length = 1;
	RX_Setup.rx_data = MemoryToReadArray;
	RX_Setup.rx_length = MemoryBytesToRead;
	RX_Setup.retransmissions_max = I2C_RETRY_ATTEMPS;
  
  // STEP 4
 	if (I2C_MasterTransferData(LPC_I2C1, &RX_Setup, I2C_TRANSFER_POLLING) == SUCCESS)
		return (TRUE);
	else
		return (FALSE);
  
  } // END OF FUNCTION CAT24C0X_read



 /*************************************************************************
 * Function Name: CAT24C0X_write
 * Parameters:  uint8_t *, uint8_t, uint8_t, uint8_t
 * Return:      uint8_t
 *
 * Description: Writes the contents of the buffer to memory.  First parameter an array is passed.  
 * The first location of the array contains the address in memory where the first byte of 
 * data will be stored.  Valid address (0-127 or 255 24C01 or 24C02). All subsequent 
 * bytes of the array will be stored, incrementally thereafter.  The function is independent of the size
 * of the array being passed.  As a pointer is passed, there is no need to malloc memory.
 * All bytes of the array will be stored.  To store 1 byte of data will require a 
 * 2 byte array (1 address + 1 data); 2 bytes, a 3 byte array (1 address + 2 bytes).  The
 * second parameter is the sizeof the array best to pass as sizeof(Array).  Next parameter is the
 * memory write location address Array[0]. Used to qualify if operation is valid.  Last parameter is the 
 * slave address sent as a 7 bit value (right shifted).
 * The main function works by transmitting the write array.  The write array has the first
 * address to be written to located in the first position of the array.
 * NOTE: Uses CMSIS I2C transfer functions.  Requires the I2C be previously initialize
 * A MAX OF 16 BYTES CAN BE WRITTEN AT ONE TIME.  BEYOND 16 WILL RESULT IN WEIRNESS
 * STEP 1: Check make sure you are not writing more data than the EEPROM can fit
 * STEP 2: Load the I2C_M_SETUP_Type with the corresponding values
 * STEP 3: Write the data to the 24C02 return a true or false condition based on write result
 *************************************************************************/
 uint8_t CAT24C0X_write16(uint8_t *MemoryWriteArray, uint8_t MemorySize, uint8_t MemoryAddress, uint8_t MemorySlaveAddress)
 {
 
  // I2CM TYPDEF
  I2C_M_SETUP_Type TX_Setup;
  
  // STEP 1
  if ((MemorySize + MemoryAddress) > MAX_EEPROM_SIZE)
    return(FALSE);

  // STEP 2
  TX_Setup.sl_addr7bit = MemorySlaveAddress;
  TX_Setup.tx_data = MemoryWriteArray;
  TX_Setup.tx_length = MemorySize;
  TX_Setup.rx_data = NULL;
  TX_Setup.rx_length = 0;
  TX_Setup.retransmissions_max = I2C_RETRY_ATTEMPS;
  
  // STEP 3
  if (I2C_MasterTransferData(LPC_I2C1, &TX_Setup, I2C_TRANSFER_POLLING) == SUCCESS)
    return (TRUE);
  else
    return (FALSE);
  
 } // END OF FUNCTION CAT24C0X_write



/*************************************************************************
 * Function Name: I2C_Stop
 * Parameters:  LPC_I2C_TypeDef *
 * Return:      void
 *
 * Description: Performs an I2C Stop
 *************************************************************************/  
static void I2C_Stop (LPC_I2C_TypeDef *I2Cx)
{

	/* Make sure start bit is not active */
	if (I2Cx->I2CONSET & I2C_I2CONSET_STA)
	{
		I2Cx->I2CONCLR = I2C_I2CONCLR_STAC;
	}
	I2Cx->I2CONSET = I2C_I2CONSET_STO;
	I2Cx->I2CONCLR = I2C_I2CONCLR_SIC;
}

