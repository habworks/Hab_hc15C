/*****************************************************************
 *
 * File name:       STMPE24M31_HC15C.H
 * Description:     Project definitions and function prototypes for use with STMPE24M31_HC15C.c/
 * Author:          Hab S. Collector
 * Date:            7/27/2011
 * LAST EDIT:       8/19/2012 
 * Hardware:        HC15C       
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent 
 *                  on other includes - everything these functions need should be passed to them
 *
 *****************************************************************/
 
 // INCLUDES
#include <ctl_api.h>
#include "STMPE24M31_HC15C.H"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_exti.h"
#include "START_N_SLEEP_TASKS.H"


// GLOBAL VARS
// STMPE24M31 SETUP REGISTER VALUES: IN THE FORM OF [INSTRUCTION], [REGISTER, VALUE]
const uint8_t STMPE24M31_SETUP_REGISTER_ARRAY[][2] = 
  {
  // BASIC TOUCH SETUP
  {0x03, 0x00}, // SYSCON-1: NO SLEEP ENABLE MODE
  {0x04, 0x60}, // SYSCON-2: GPIO AND PWM CLOCK DISABLE
  {0x72, 0xFF}, // CH0-7 AS CAP TOUCH
  {0x73, 0xFF}, // CH8-15 AS CAP TOUCH
  {0x74, 0xFF}, // CH16-23 AS CAP TOUCH
  // STEP UP TVR (TOUCH VARIANCE REGISTER VALUES)
  {0x7C, 0x01},
  {0xC0, 0x30},
  {0xC2, 0x30},
  {0xC4, 0x30},
  {0xC6, 0x30},
  {0xC8, 0x30},
  {0xCA, 0x30},
  {0xCC, 0x30},
  {0xCE, 0x30},
  {0xD0, 0x30},
  {0xD2, 0x30},
  {0xD4, 0x30},
  {0xD6, 0x30},
  {0xD8, 0x30},
  {0xDA, 0x30},
  {0xDC, 0x30},
  {0xDE, 0x30},
  {0xE0, 0x30},
  {0xE2, 0x30},
  {0xE4, 0x30},
  {0xE6, 0x30},
  {0xE8, 0x30},
  {0xEA, 0x30},
  {0xEC, 0x30},
  {0xEE, 0x30},
  // SET UP EVR (ENVIRONMENTAL VARINACE REGISTER)
  {0x7C, 0x02},
  {0xC0, 0x0A},
  {0xC2, 0x0A},
  {0xC4, 0x0A},
  {0xC6, 0x0A},
  {0xC8, 0x0A},
  {0xCA, 0x0A},
  {0xCC, 0x0A},
  {0xCE, 0x0A},
  {0xD0, 0x0A},
  {0xD2, 0x0A},
  {0xD4, 0x0A},
  {0xD6, 0x0A},
  {0xD8, 0x0A},
  {0xDA, 0x0A},
  {0xDC, 0x0A},
  {0xDE, 0x0A},
  {0xE0, 0x0A},
  {0xE2, 0x0A},
  {0xE4, 0x0A},
  {0xE6, 0x0A},
  {0xE8, 0x0A},
  {0xEA, 0x0A},
  {0xEC, 0x0A},
  {0xEE, 0x0A},
  // SET UP CONTROL MODES
  {0x78, 0x05}, // MAF_SET: MEDIUM AVG FILTER
  {0x77, 0x05}, // CAL_MODE: CALIBRATION
  {0x76, 0x01}, // CAL_INT: CALIBRATION INTERVAL
  {0x08, 0xFF}, // INT_STA: IRQ STATUS REGISTER (CLEAR ALL IRQs)
  {0x09, 0x04}, // INT_EN: IRQ ENABLE REGISTER: IRQ ON TOUCH KEY EVENT
  {0x06, 0x01}, // INT_CTL: IRQ CONTROL REGISTER: ENABLE THE IRQ, LEVEL TRIGGERED, ACTIVE LOW            
  {0x70, 0x81}  // CAP_SEN_CTL: CAP SENSE CTL REGISTER: 
  };

// STMPE24M31 AUTO TUNE COMMAND (I THINK): IN THE FORM OF [INSTRUCTION], [REGISTER, VALUE]
const uint8_t STMPE24M31_AUTO_TUNE_COMMAND_ARRAY[][2] = 
  {
  {0x77, 0x55},
  {0x70, 0x81}
  };

// TYPEDEFS
//static CTL_ISR_FN_t ISR_EINT1_Var;

// EXTERNS
// VARS
extern CTL_EVENT_SET_t CalEvents;
extern CTL_MESSAGE_QUEUE_t MsgQueue;
// FUNCTIONS
extern void delayXms(uint32_t);




/*************************************************************************
 * Function Name: init_I2C0
 * Parameters:    uint32_t
 * Return:        void
 *
 * Description: Configure I2C0 for interface at passed clock frequency to a STMPE24M31
 * 24 CHANNEL CAP TOUCH CONTROLLER ON I2C0.  
 * 
 * STEP 1: Setup the P0.28 and P0.27 to act as I2C0
 * STEP 2: Init I2C1 at the desired I2C frequency and enable
 *************************************************************************/
 void init_I2C0(uint32_t I2C_Frequency)
 {
  // PIN TYPE DEFINITION
  PINSEL_CFG_Type PinCfgStruct;
  
  // STEP 1
  // FROM TABLE 8.5 USER MANUAL LPC17XX
  PinCfgStruct.OpenDrain = 0;
  PinCfgStruct.Pinmode = 0;
  PinCfgStruct.Funcnum = 1;
  PinCfgStruct.Pinnum = 27;
  PinCfgStruct.Portnum = PORT0;
  PINSEL_ConfigPin(&PinCfgStruct);
  PinCfgStruct.Pinnum = 28;
  PINSEL_ConfigPin(&PinCfgStruct);
  
  // STEP 2
  I2C_Init(LPC_I2C0, I2C_Frequency);
  I2C_Cmd(LPC_I2C0, ENABLE);
  
 } // END OF FUNCTION init_I2C0




/*************************************************************************
 * Function Name: STMPE24M31_write
 * Parameters:  uint8_t *, uint8_t, uint8_t
 * Return:      uint8_t
 *
 * Description: Writes the contents of the array to STMPE24M31.  First parameter an array is passed.  
 * The first location of the array contains the address in memory where the first byte of 
 * data will be written to in the STMPE24M31.  Valid address (0x00 or 0xEE). All subsequent 
 * bytes of the array will be stored, incrementally thereafter.  The function is independent of the size
 * of the array being passed.  As a pointer is passed, there is no need to malloc memory.
 * All bytes of the array will be written.  To store 1 byte of data will require a 
 * 2 byte array (1 address + 1 data); 2 bytes, a 3 byte array (1 address + 2 bytes).  The
 * second parameter is the sizeof the array best to pass as sizeof(Array).  Last parameter is the 
 * slave address sent as a 7 bit value (right shifted).
 * NOTE: Uses CMSIS I2C transfer functions.  Requires the I2C be previously initialize
 * 
 * STEP 1: Load structure value for writing the array
 * STEP 2: Write the array/register(s)
 *************************************************************************/
 BOOLEAN STMPE24M31_write(uint8_t *RegisterWriteArray, uint8_t RegisterSize, uint8_t STMPE24M31_SlaveAddress)
 {
 
  // I2CM TYPDEF
  I2C_M_SETUP_Type TX_Setup;

  // STEP 1
  TX_Setup.sl_addr7bit = STMPE24M31_SlaveAddress;
  TX_Setup.tx_data = RegisterWriteArray;
  TX_Setup.tx_length = RegisterSize;
  TX_Setup.rx_data = NULL;
  TX_Setup.rx_length = 0;
  TX_Setup.retransmissions_max = I2C_STMPE24M31_RETRY_ATTEMPS;
  
  // STEP 2
  if (I2C_MasterTransferData(LPC_I2C0, &TX_Setup, I2C_TRANSFER_POLLING) == SUCCESS)
    return (TRUE);
  else
    return (FALSE);
  
 } // END OF FUNCTION STMPE24M31_write




/*************************************************************************
 * Function Name: STMPE24M31_read
 * Parameters:    uint8_t *, uint8_t, uint8_t
 * Return:        uint8_t
 *
 * Description: Reads register value(s) of the STMPE24M31.  The first parameter is the
 * Array in which the bytes read will be stored.  NOTE, the size of the array MUST equal the 
 * the desired number of bytes to be read.  Next parameter is the size of the array, best passed
 * using sizeof(Array) operator.  Next is register to read.  Last parameter is the slave address of 
 * the EEPROM in 7 bit form right shifted. 
 * NOTE: Uses CMSIS I2C transfer functions.  Requires the I2C be previously initialize
 * NOTE: See also function CAT24C0X_write
 * STEP 1: Declare an Array for use with the I2C function call
 * STEP 2: Load structure value for reading the array
 * STEP 3: Read the array/register(s)
 *************************************************************************/
 BOOLEAN STMPE24M31_read(uint8_t *RegisterToReadArray, uint8_t RegisterBytesToRead, uint8_t RegisterAddress, uint8_t STMPE24M31_SlaveAddress)
 {
  // I2CM TYPDEF
  I2C_M_SETUP_Type RX_Setup;
  
  // STEP 1
  uint8_t WriteData[2];
  WriteData[0] = RegisterAddress;  
  
  // STEP 2
  RX_Setup.sl_addr7bit = STMPE24M31_SlaveAddress;
	RX_Setup.tx_data = WriteData;	// Get address to read at writing address
	RX_Setup.tx_length = 1;
	RX_Setup.rx_data = RegisterToReadArray;
	RX_Setup.rx_length = RegisterBytesToRead;
	RX_Setup.retransmissions_max = I2C_STMPE24M31_RETRY_ATTEMPS;
  
  // STEP 3
 	if (I2C_MasterTransferData(LPC_I2C0, &RX_Setup, I2C_TRANSFER_POLLING) == SUCCESS)
		return (TRUE);
	else
		return (FALSE);
  
  } // END OF FUNCTION STMPE24M31_read




/*************************************************************************
 * Function Name: STMPE24M31_init
 * Parameters:    uint8_t
 * Return:        uint8_t
 *
 * Description: Configures the STMPE24M31 as a 24 Channel Cap Touch Controller with
 * and IRQ active response on cap touch - Based on the constant 2D array STMPE24M31_SETUP_REGISTER_ARRAY
 * NOTE: Uses the function STMPE24M31_write.  Requires the I2C0 be previously initialize
 * NOTE: See also function CAT24C0X_write
 * NOTE: They are 2x STMPE24M31 - so this function will have to be called twice
 * STEP 1: Declare Values and arrays for use with the write functions
 * STEP 2: Set values depending on which STMPE24M31 device
 * STEP 3: Calculate the size of STMPE24M31_SETUP_REGISTER_ARRAY (of values to pass)
 * STEP 4: Extract the register and value information from STMPE24M31_SETUP_REGISTER_ARRAY
 * STEP 5: Write the setup information
 * STEP 6: If you get here return TRUE
 *************************************************************************/
 BOOLEAN STMPE24M31_init(uint8_t STMPE24M31_Device)
 {
 
 // STEP 1
 uint8_t DeviceAddress, ArraySize;
 uint8_t RegisterAndValue[2];
 
 // STEP 2
 switch (STMPE24M31_Device)
   {
   case STMPE24M31_ACAP:
   DeviceAddress = SLAVE_ADDRESS_STMPE24M31_ACAP;
   break;
   
   case STMPE24M31_BCAP:
   DeviceAddress = SLAVE_ADDRESS_STMPE24M31_BCAP;
   break;
   
   default:
   return(FALSE);
   break;
   }
 
 // STEP 3
 // GUARANTEED TO BE AN EVEN NUMBER AS THIS IS A TWO-DIM ARRAY
 ArraySize = sizeof(STMPE24M31_SETUP_REGISTER_ARRAY)/2;
 
 // STEP 4
 for(uint8_t SetupInstruction = 0; SetupInstruction < ArraySize; SetupInstruction++)
   {
   RegisterAndValue[REGISTER_BYTE] = STMPE24M31_SETUP_REGISTER_ARRAY[SetupInstruction][REGISTER_BYTE];
   RegisterAndValue[VALUE_BYTE] = STMPE24M31_SETUP_REGISTER_ARRAY[SetupInstruction][VALUE_BYTE];
   // STEP 5
   if (STMPE24M31_write(RegisterAndValue, sizeof(RegisterAndValue), DeviceAddress) != TRUE)
     return(FALSE);
   }
 
 // STEP 6
 return(TRUE);   
 
 } // END OF FUNCTION STMPE24M31_init
 
 
 
 
/*************************************************************************
 * Function Name: init_STMPE24M31_EINT
 * Parameters:    void
 * Return:        void
 *
 * Description: Sets up the External IRQs(2 and 1) for STMPE24M31 device A and B for level
 * sensitive active low
 * STEP 1: Define pin assignments for External IRQs - NOT GPIO IRQs (GPIO IRQs cannot be level set)
 * STEP 2: Set all External IRQs to a default state - OK because the design only uses these 2
 * STEP 3: Set external IRQs as: level, active low
 * STEP 4: Set priority and enable
 *************************************************************************/
 void init_STMPE24M31_EINT(void)
 {
  
  // TYPEDEF
  PINSEL_CFG_Type PinCfgStruct;
  EXTI_InitTypeDef EXTICfgstruct;
  
  // STEP 1
  // FROM TABLE 8.5 USER MANUAL LPC17XX
  PinCfgStruct.OpenDrain = 0;
  PinCfgStruct.Pinmode = 0;  // can also be 2
  PinCfgStruct.Funcnum = 1;
  PinCfgStruct.Pinnum = 11;
  PinCfgStruct.Portnum = PORT2;
  PINSEL_ConfigPin(&PinCfgStruct);
  PinCfgStruct.Pinnum = 12;
  PINSEL_ConfigPin(&PinCfgStruct);
  
  // STEP 2
  /* SETS TO A DEFAULT STATE:
   * ALL EINTs IRQs ARE CLEARED
   * ALL EINTs LEVEL SENSATIVE
   * ALL EINTs ACTIVE LOW */
  EXTI_Init();
  
  // STEP 3
  // EXTI 1 - B DEVICE
  EXTICfgstruct.EXTI_Line = EXTI_EINT1;
  EXTICfgstruct.EXTI_Mode = EXTI_MODE_LEVEL_SENSITIVE;
  EXTICfgstruct.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
  EXTI_Config(&EXTICfgstruct);
  // EXTI 2 - A DEVICE
  EXTICfgstruct.EXTI_Line = EXTI_EINT2;
  EXTICfgstruct.EXTI_Mode = EXTI_MODE_LEVEL_SENSITIVE;
  EXTICfgstruct.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
  EXTI_Config(&EXTICfgstruct);
  
  // STEP 4
  // EXTI 1 - B DEVICE
  ctl_set_priority(EINT1_IRQn, EINT1_IRQ_PRIORITY);
  ctl_unmask_isr(EINT1_IRQn);
  // EXTI 2 - A DEVICE
  ctl_set_priority(EINT2_IRQn, EINT2_IRQ_PRIORITY);
  ctl_unmask_isr(EINT2_IRQn);
   
 } // END OF FUNCTION init_STMPE24M31_EINT
 
 
 
 
/*************************************************************************
 * Function Name: EINT1_IRQHandler
 * Parameters:    void
 * Return:        void
 *
 * Description: IRQ Handler for External IRQ (STMPE24M31 Device B).  This ISR will
 * call the function ISR_STMPE24M31_AandB with the pass parameter that this is touch device B
 * STEP 1: It is possible this interrupt can interrupt itself.  Do not let that happen, so disable both touch IRQs.
 * It is also possible that the device is sleeping and this is a touch that will
 * wake the device from sleeping.
 * STEP 2: Call ISR routine to deal with IRQ event, pass Device B address for processing
 * and clear the IRQ.  Note the ISR must first removes the IRQ condition.  After condition is removed
 * the IRQ can be cleared. 
 * STEP 3: Re-enable both touch IRQs
 *************************************************************************/
 void EINT1_IRQHandler(void)
 {
 
  // STEP 1
  ctl_mask_isr(EINT2_IRQn);
  ctl_mask_isr(EINT1_IRQn);
  // CHECK IF WAKE FROM DEEP SLEEP
  if (LPC_SC->PCON & (1<<9))  
    {
    LPC_SC->PCON |= ((1<<3));
    SCB->SCR = 0x00;
    wakeFromSleep();
    }  
  
  // STEP 2
  ISR_STMPE24M31_AandB(SLAVE_ADDRESS_STMPE24M31_BCAP);
  EXTI_ClearEXTIFlag(EXTI_EINT1);
  
  // STEP 3
  ctl_unmask_isr(EINT2_IRQn);
  ctl_unmask_isr(EINT1_IRQn);
  
 } // END OF FUNCTION EINT1_IRQHandler




/*************************************************************************
 * Function Name: EINT2_IRQHandler
 * Parameters:    void
 * Return:        void
 *
 * Description: IRQ Handler for External IRQ (STMPE24M31 Device A).  This ISR will
 * call the function ISR_STMPE24M31_AandB with the pass parameter that this is touch device A
 * STEP 1: It is possible this interrupt can interrupt itself.  Do not let that happen, so disable both 
 * touch IRQs.  It is also possible that the device is sleeping and this is a touch that will
 * wake the device from sleeping.
 * STEP 2: Call ISR routine to deal with IRQ event, pass Device B address for processing
 * and clear the IRQ.  Note the ISR must first removes the IRQ condition.  After condition is removed
 * the IRQ can be cleared. 
 * STEP 3: Re-enable both touch IRQs
 *************************************************************************/
 void EINT2_IRQHandler(void)
 {

  // STEP 1
  ctl_mask_isr(EINT2_IRQn); 
  ctl_mask_isr(EINT1_IRQn); 
  // CHECK IF WAKE FROM DEEP SLEEP
  if (LPC_SC->PCON & (1<<9))
    {
    LPC_SC->PCON |= ((1<<3));
    SCB->SCR = 0x00;
    wakeFromSleep();
    }

  // STEP 2
  ISR_STMPE24M31_AandB(SLAVE_ADDRESS_STMPE24M31_ACAP);
  EXTI_ClearEXTIFlag(EXTI_EINT2);
  
  // STEP 3
  ctl_unmask_isr(EINT2_IRQn); 
  ctl_unmask_isr(EINT1_IRQn); 

 } // END OF FUNCTION EINT2_IRQHandler
 
 
 
 
/*************************************************************************
 * Function Name: ISR_STMPE24M31_AandB
 * Parameters:    uint8_t
 * Return:        void
 *
 * Description: Interrupt Service Function - called by either Interrupt Handler for STMPE24M31 device A or B.  
 * Read the IRQ Status register (INT_STA) and performs the actions based on the IRQ event.  The normal IRQ
 * would be that there has been a touch event.  In the typical case there are 4 IRQs - In this order:
 * 0x24 - Valid Touch Data
 * 0x00 - NA
 * 0x24 - Data appears to be always 0x00
 * 0x00 - NA
 * On KeyTouch events (INT_STA_TOUC {0x04} read three bytes from the KeyFilterRegister, you can qualify if this
 * is the correct read by its value != 0.
 * Load the value in byte order to a variable and set a flag that the device key was pressed.
 * The most important aspect of this handler is to capture the channel data and set the corresponding key pressed 
 * device flag to be acted upon and cleared by main routine
 * STEP 1: Define Arrays for use
 * STEP 2: Read the IRQ Status register
 * STEP 3: Take action: if good condition read 3 bytes of the key filter data and store to an uint32_t and
 * set a flag that key data is ready - If not take necessary action based on IRQ
 * STEP 4: Clear the IRQ by writing what cause the IRQ back to the INT STA register
 *************************************************************************/
 void ISR_STMPE24M31_AandB(uint8_t DeviceAddress)
 {
 // STEP 1 
 uint8_t KeyFilterRegister[3];
 uint8_t INT_STA_ReadRegister[1], INT_STA_WriteRegister[2];
 uint8_t RegisterAndValue[2];
 uint32_t KeyFilterRead;
 
 // STEP 2
 STMPE24M31_read(INT_STA_ReadRegister, sizeof(INT_STA_ReadRegister), STMPE24M31_INT_STA_REG, DeviceAddress);   

 // STEP 3
 // EXPECTED GOOD CONDITIONS: TREAT ALL THE SAME - TOUCH KEY EVENT VALID DATA TO BE READ
 switch(INT_STA_ReadRegister[0])
   {
   case INT_STA_TOUCH:
   case (INT_STA_TOUCH|INT_STA_WAKEUP):
   case (INT_STA_TOUCH|INT_STA_WAKEUP|INT_STA_END_OF_CALIBRATION):
   case (INT_STA_TOUCH|INT_STA_WAKEUP|INT_STA_ABNORMAL_CONDITION):
   STMPE24M31_read(KeyFilterRegister, sizeof(KeyFilterRegister), STMPE24M31_KEY_FILT_DATA_REG, DeviceAddress);
   // LOAD THE KEY FILTER REGISTER
   KeyFilterRead = 0x00;
   KeyFilterRead = KeyFilterRegister[2];
   KeyFilterRead <<= 8;
   KeyFilterRead |= KeyFilterRegister[1];
   KeyFilterRead <<= 8;
   KeyFilterRead |= KeyFilterRegister[0];
   // LOAD THE MSG QUEUE FOR DATA
   if (KeyFilterRead != 0)
     {
     // SET MSb TO INDICATE BCAP DATA
     if (DeviceAddress == SLAVE_ADDRESS_STMPE24M31_BCAP)
       KeyFilterRead |= 0x80000000;
     ctl_message_queue_post_nb(&MsgQueue, (void *)KeyFilterRead);
     }   
   break;
   
   case INT_STA_WAKEUP:   
   case INT_STA_END_OF_CALIBRATION:
   break;
   
   default:
   break;
   }

 // CHECK IF AN ABNORMAL CONDITION HAS OCCURED - ISSUE AN AUTO TUNE
 if ((INT_STA_ReadRegister[0] & INT_STA_ABNORMAL_CONDITION) != 0x00)
   {
   // LOAD ARRAY WITH THE AUTO TUNE COMMAND AND WRITE 
   for(uint8_t SetupInstruction = 0; SetupInstruction < (sizeof(STMPE24M31_AUTO_TUNE_COMMAND_ARRAY)/2); SetupInstruction++)
     {
     RegisterAndValue[REGISTER_BYTE] = STMPE24M31_AUTO_TUNE_COMMAND_ARRAY[SetupInstruction][REGISTER_BYTE];
     RegisterAndValue[VALUE_BYTE] = STMPE24M31_AUTO_TUNE_COMMAND_ARRAY[SetupInstruction][VALUE_BYTE];
     // WRITE THE AUTO TUNE COMMAND: CHECK IF IT FAILED
     if (STMPE24M31_write(RegisterAndValue, sizeof(RegisterAndValue), DeviceAddress) != TRUE)
       while(1); // THIS IS REALLY BAD: WRITE OF AUTO TUNE COMMAND FAILED
     }
   }
 
 // STEP 4
 INT_STA_WriteRegister[VALUE_BYTE] = INT_STA_ReadRegister[0];
 INT_STA_WriteRegister[REGISTER_BYTE] = STMPE24M31_INT_STA_REG;
 STMPE24M31_write(INT_STA_WriteRegister, sizeof(INT_STA_WriteRegister), DeviceAddress);
   
 } // END OF FUNCTION ISR_STMPE24M31_AandB




/*************************************************************************
 * Function Name: void STMPE24M31_reset
 * Parameters:    uint8_t
 * Return:        void
 *
 * Description: Reset the STMPE24M31 by writing a reset value to SYSCON-3
 * STEP 1: Declare an array for use with I2C STMPE24M31_write
 * STEP 2: Select the device address based
 * STEP 3: Write the reset value and wait
 * STEP 4: return
 *************************************************************************/
 BOOLEAN STMPE24M31_reset(uint8_t STMPE24M31_Device)
 {
 
 // STEP 1
 uint8_t SYSCON1_WriteRegister[2];
 uint8_t DeviceAddress;
 
 // STEP 2
 switch (STMPE24M31_Device)
   {
   case STMPE24M31_ACAP:
   DeviceAddress = SLAVE_ADDRESS_STMPE24M31_ACAP;
   break;
   
   case STMPE24M31_BCAP:
   DeviceAddress = SLAVE_ADDRESS_STMPE24M31_BCAP;
   break;
   
   default:
   return(FALSE);
   break;
   }
 
 // STEP 3
 SYSCON1_WriteRegister[VALUE_BYTE] = SYSCON1_SOFT_RESET;
 SYSCON1_WriteRegister[REGISTER_BYTE] = STMPE24M31_SYSCON1_REG;
 STMPE24M31_write(SYSCON1_WriteRegister, sizeof(SYSCON1_WriteRegister), DeviceAddress);
 // DATA SHEET DOES NOT SPECIFIY HOW LONG TO WAIT FOR A SOFT - HARD RESET IS 2us
 delayXms(10); // SHOULD BE LONG ENOUGH
 
 // STEP 4
 return(TRUE);
 
 } // END OF FUNCTION STMPE24M31_reset




/*************************************************************************
 * Function Name: void STMPE24M31_readIntStatus
 * Parameters:    uint8_t
 * Return:        void
 *
 * Description: Reads the System Interrupt Status Register and returns that value
 * STEP 1: Declare an array for use with I2C STMPE24M31_write
 * STEP 2: Select the device address based
 * STEP 3: Write the reset value and wait and return
 *************************************************************************/
 uint8_t STMPE24M31_readIntStatus(uint8_t DeviceAddress)
 {
 
 // STEP 1 
 uint8_t INT_STA_ReadRegister[1];
 
 // STEP 2
 STMPE24M31_read(INT_STA_ReadRegister, sizeof(INT_STA_ReadRegister), STMPE24M31_INT_STA_REG, DeviceAddress);   
 
 // STEP 3
 return(INT_STA_ReadRegister[0]);

 } // END OF FUNCTION STMPE24M31_readIntStatus
   
   

 
 
 
   
   
     
     
   