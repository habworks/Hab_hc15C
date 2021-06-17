/*****************************************************************
 *
 * File name:         DIP204.C
 * Description:       Functions used to support PN: EA DIP204 LCD
 * Author:            Hab S. Collector
 * Date:              6/9/11
 * Last Edit:         8/03/12
 * Hardware:          EA DIP204 LCD, 4X20 LCD WITH SPI INTERFACE Controller hardware is: SamSung KS0073
 * Firmware Tool:     CrossStudio for ARM
 * Notes:             This file should be written as to not be dependent on other includes.
 *                    Everything these functions need should be passed to them.
 *                    It will be necessary to consult the reference documents and associated schematics to understand
 *                    the operations of this firmware.
 *****************************************************************/ 

// INCLUDES
#include "DIP204.H"
#include "lpc17xx_ssp.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include <string.h> 
#include <stdio.h>
#include <ctl.h>

// GLOBAL VARS
uint8_t sLineNum = 0, sColNum = 0;

// EXTERNS
extern void delayXms(uint32_t);
extern CTL_MUTEX_t DIP204MutexPrint, DIP204MutexClear, DIP204Mutex, DIP204MutexGoTo, DIP204MutexICON;

// PROTOTYPE FUNCITONS

// DEFINES
// ENUMERATED TYPES


/*************************************************************************
 * Function Name: init_SSPI1
 * Parameters: uint32_t
 * Return: void
 *
 * Description: Init SSPI1 for use with DIP204 LCD display
 * STEP 1: Declare CMSIS structures.  Configure Pin Select Register as SSPI with SLE (LCD_CS) as a GPIO
 * STEP 2: RESET THE DIP204
 * STEP 3: Configure the SSPI by setting its CLK and configure reg for use with the DIP204
 * STEP 4: Enable the SSPI1 port
 *************************************************************************/
 void init_SSPI1(uint32_t ClkFrequency)
 {
 
 // STEP 1
 // TYPE DEF VAR STRUCTURES FOR CMSIS
 PINSEL_CFG_Type PINSEL_PinCfgStruct;
 SSP_CFG_Type SSP_ConfigStruct;
 
  /* INIT SPI1 PIN CONNECT
   * P0.22 -LCD_RST - used as GPIO
   * P0.6 - CS_LED - used as GPIO
   * P0.7 - SCK1; - used as SSP
   * P0.8 - MISO1 - used as SSP
   * P0.9 - MOSI1 - used as SSP
   */
  PINSEL_PinCfgStruct.Funcnum = 2;
  PINSEL_PinCfgStruct.OpenDrain = 0;
  PINSEL_PinCfgStruct.Pinmode = 0;
  PINSEL_PinCfgStruct.Portnum = PORT0;
  PINSEL_PinCfgStruct.Pinnum = 7;
  PINSEL_ConfigPin(&PINSEL_PinCfgStruct);
  PINSEL_PinCfgStruct.Pinnum = 8;
  PINSEL_ConfigPin(&PINSEL_PinCfgStruct);
  PINSEL_PinCfgStruct.Pinnum = 9;
  PINSEL_ConfigPin(&PINSEL_PinCfgStruct);
  // MAKE THE SEL PIN A GPIO
  // NECESSARY BECUASE CS NEEDS TO BE ATICVE FOR THE ENTIRE 3 BTYE TRANSFER
  GPIO_SetDir(PORT0, (LCD_CS|LCD_RST), 1);
  GPIO_SetValue(PORT0, (LCD_CS|LCD_RST));
  
  // STEP 2
  reset_DIP204();
  
  // STEP 3
  SSP_ConfigStruct.Databit = SSP_DATABIT_8;
  SSP_ConfigStruct.CPHA = SSP_CPHA_FIRST;
  SSP_ConfigStruct.CPOL = SSP_CPOL_HI;
  SSP_ConfigStruct.Mode = SSP_MASTER_MODE;
  SSP_ConfigStruct.FrameFormat = SSP_FRAME_SPI;
  SSP_ConfigStruct.ClockRate = ClkFrequency;
  // INIT SSPI1
  SSP_Init(LPC_SSP1, &SSP_ConfigStruct);
  
  // STEP 4
  SSP_Cmd(LPC_SSP1, ENABLE);
 
 } // END OF FUNCTION initSSPI1



/*************************************************************************
 * Function Name: reset_DIP204
 * Parameters: void
 * Return: void
 *
 * Description: Reset the DIP204 Display - The display must be init before it can be reset
 * STEP 1: Reset Display
 *************************************************************************/
 void reset_DIP204(void)
 {
  // STEP 1
  GPIO_ClearValue(PORT0, LCD_RST);
  delayXms(DIP204_RST_DELAY_TIME);
  GPIO_SetValue(PORT0, LCD_RST);
 } // END OF FUNCTION reset_DIP204



/*************************************************************************
 * Function Name: DIP204_engine
 * Parameters: uint8_t, uint8_t
 * Return: void
 *
 * Description: Sends Display Instructions (Command Write, Command Read, Data Write,
 * Data Read) to the DIP204 LCD Display.  Display Instructions are sent with the corresponding
 * Instruction Data.  To do this first the prefix command is sent (Command Write or Read, Data
 * Write or Read).  After the command prefix, the data itself is sent.  The one byte data is sent
 * as two bytes with the order reversed.
 * Notes: Dummy reads are necessary to force the SPI action if not using CMSIS functions       
 * STEP 1: Declare CMSIS structures for use.  Load the TX Buffer with DIP204 2 Byte Command
 * STEP 2: Chip Select the LCD
 * STEP 3: Send the command Prefix
 * STEP 4: Transmit the Instruction Data as two bytes:
 *         Transmit the Instruction Data LSN first in reverse order
 *         Transmit the Instruction Data MSN first in reverse order
 * STEP 5: Disable LCD
 *************************************************************************/
void DIP204_engine(uint8_t DisplayInstruction, uint8_t InstructionData)
{
  
  ctl_mutex_lock(&DIP204Mutex, CTL_TIMEOUT_NONE, 0);
  
  // STEP 1
  uint8_t TX_Buffer[5], RX_Buffer[5];
  SSP_DATA_SETUP_Type SSP_xferConfigStruct;
  
  // LOAD TX BUFFER
  // LOAD BUFFER STYLE FOR FUTURE REFERENCE WITH 3 BYTE COMMAND. SEND WILL WORK - ALSO NOTED HERE FOR CLARITY
  // BUT IN THIS CASE WILL SEND JUST ONE AT A TIME.  IGNORE TX[1] AND TX[2]  YOU ARE ONLY SENDING 1 COMMAND
  TX_Buffer[0] = DisplayInstruction;
  // SEND THE LEAST SIGNIFICANT NIBBLE OF THE CMD BYTE IN THE FORMAT (FORMAT: LSN:D0_D1_D2_D3)
  TX_Buffer[1] = reverseBitOrder(InstructionData & 0x0F);
  // SEND THE MOST SIGNIFICANT NIBBLE OF THE CMD BYTE IN THE FORMAT (FORMAT: MSN:D4_D5_D6_D7)
  TX_Buffer[2] = reverseBitOrder((InstructionData >> 4) & 0x0F);
  /* ASSIGN BUFFER TO STRUCTURE: RX BUFFER NOT IMPLEMENTED
   * NOTE RX BUFFER FIRST BYTE COULD BE USED TO CHECK TRANSMIT - FIRST BYTE IS ECHOED BACK VIA MISO */
  SSP_xferConfigStruct.tx_data = TX_Buffer;
  SSP_xferConfigStruct.rx_data = RX_Buffer;
  SSP_xferConfigStruct.length = 3; // YOU WOULD MAKE THIS VALUE 1

  // STEP 2
  // LCD CS - TO STAY ACTIVE FOR THE FULL 3 BYTE TRANSFER
  GPIO_ClearValue(PORT0, LCD_CS);
  
  // STEP 3
  SSP_ReadWrite(LPC_SSP1, &SSP_xferConfigStruct, SSP_TRANSFER_POLLING);

  /* STEP 4 - IF USING 1 BYTE COMMAND DO NOT COMMENT OUT THIS STEP
  // SEND THE LEAST SIGNIFICANT NIBBLE OF THE CMD BYTE IN THE FORMAT (FORMAT: LSN:D0_D1_D2_D3)
  TX_Buffer[0] = reverseBitOrder(InstructionData & 0x0F);
  SSP_ReadWrite(LPC_SSP1, &SSP_xferConfigStruct, SSP_TRANSFER_POLLING);
  // SEND THE MSOT SIGNIFICANT NIBBLE OF THE CMD BYTE IN THE FORMAT (FORMAT: MSN:D4_D5_D6_D7)
  TX_Buffer[0] = reverseBitOrder((InstructionData >> 4) & 0x0F);
  SSP_ReadWrite(LPC_SSP1, &SSP_xferConfigStruct, SSP_TRANSFER_POLLING);
  * END OF COMMENT OUT STEP P4*/
  
  // STEP 5
  GPIO_SetValue(PORT0, LCD_CS);
  
  ctl_mutex_unlock(&DIP204Mutex); 

} // END OF FUNCTION DPI204_engine




/*************************************************************************
 * Function Name: init_DIP204
 * Parameters: void
 * Return: void
 *
 * Description: Init the DISPLAY to be used.  Will clear the display of all 
 * ICONs and text.  Set the display on, with no cursor displaying the introduction.
 * NOTE: Requires SSPI1 to be init at DIP204 frequency.
 * NOTE: CODE DOES NOT LOOK AT BUSY FLAG.  CMD INSTRUCTION TIME IS GIVEN IN DATA
 * SHEET.  USE 2ms DELAY AS A DEFAULT TO COVER EVERY CONDITION.
 * STEP 1: Start the SPI bus interface for operation with the LCD.  Define and set and
 * clear the LCD reset.  Allow time for diplay to settle after POR with reset active 
 * STEP 2: FOLLOW SEQUENCE OF COMMAND GIVEN IN MANUAL FOR STARTUP OPERATION
 * STEP 3: Set cursor to home position and display opening screen and firmware rev
 *************************************************************************/
 void init_DIP204(void)
 {
         
  uint8_t LineText[20];
  
  // STEP 1
  init_SSPI1(SSPI_DIP204_CLK);
  GPIO_SetDir(PORT0, LCD_RESET, 1);
  GPIO_ClearValue(PORT0, LCD_RESET);
  delayXms(100);
  GPIO_SetValue(PORT0, LCD_RESET);
  delayXms(10);
  
  // STEP 2
  // FUNCTION SET: 8BIT, RE=0
  DIP204_engine(START_BYTE_CMD_WRITE, 0x30);               
  delayXms(WAIT_FOR_DISPLAY);
  // ENTRY MODE: CURSOR AUTO INC        
  DIP204_engine(START_BYTE_CMD_WRITE, 0x06);               
  delayXms(WAIT_FOR_DISPLAY);
  // FUNCTION SET: 8BIT, RE=1, BLINK ENABLE
  DIP204_engine(START_BYTE_CMD_WRITE, 0x36);                
  delayXms(WAIT_FOR_DISPLAY);
  // EX FUNCTION SET: 4 LINE MODE
  DIP204_engine(START_BYTE_CMD_WRITE, 0x09);               
  delayXms(WAIT_FOR_DISPLAY);
  
  // CLEAR THE ICONS TO OFF
  // SET SERRAM ADDRESS TO BASE 00
  DIP204_engine(START_BYTE_CMD_WRITE, 0x40);               
  delayXms(WAIT_FOR_DISPLAY);
  // WRITE 00 X16 TO CLEAR ALL ICONS
  for (int I=0; I<16; I++)                                               
   {
   DIP204_engine(START_BYTE_DAT_WRITE, 0x00);
   delayXms(WAIT_FOR_DISPLAY);
   }
  
  // STEP 3
  // FUNCTION SET: 8BIT, RE=0
  DIP204_engine(START_BYTE_CMD_WRITE, 0x30);
  delayXms(WAIT_FOR_DISPLAY);
  // DISPLAY ON, CURSOR ON
  DIP204_engine(START_BYTE_CMD_WRITE, 0x0E);
  delayXms(WAIT_FOR_DISPLAY);
  // CLEAR DISPLAY
  DIP204_engine(START_BYTE_CMD_WRITE, 0x01);
  delayXms(WAIT_FOR_DISPLAY);
  // DISPLAY ON, CURSOR ON, CURSOR BLINK
  DIP204_engine(START_BYTE_CMD_WRITE, 0x0F); 
  delayXms(WAIT_FOR_DISPLAY);
  DIP204_txt_engine(LCD_INTRO_LINE1,1,0,strlen(LCD_INTRO_LINE1));
  DIP204_txt_engine(LCD_INTRO_LINE2,2,0,strlen(LCD_INTRO_LINE2));
  DIP204_txt_engine(LCD_INTRO_LINE3,3,0,strlen(LCD_INTRO_LINE3));
  strcpy(LineText,LCD_INTRO_LINE4);
  strcat(LineText,MAJOR_REV);
  strcat(LineText,".");
  strcat(LineText,MINOR_REV);
  DIP204_txt_engine(LineText,4,0,strlen(LineText));
  DIP204_set_cursor(CURSOR_OFF);
         
 } // END OF FUNCTION init_DIP204
         
         


/*************************************************************************
 * Function Name: DIP204_ICON_set
 * Parameters: uint8_t, uint8_t
 * Return: void
 *
 * Description: Set 1 of 16 ICONS to a status of off, on, or blink
 * NOTE: CODE DOES NOT LOOK AT BUSY FLAG.  CMD INSTRUCTION TIME IS GIVEN IN DATA
 * SHEET.  
 * STEP 1: Sequence of instructions as defined in the data sheet for DIP204
 * STEP 2: ENTER THE SPECIFIC ICON AND SPECIFIC STATUS
 * STEP 3: Continue sequence of instructions as defined in data sheet for DIP204
 * STEP 4: Reset the cursor location using the global values for cursor location as this function will reset cursor position
 *************************************************************************/
 void DIP204_ICON_set(uint8_t ICON_ToSet, uint8_t ICON_Status)
 {
 
 ctl_mutex_lock(&DIP204Mutex, CTL_TIMEOUT_NONE, 0); 
 // STEP 1
 // FUNCTION SET 8BIT, RE=1, BLINK ENABLE
 DIP204_engine(START_BYTE_CMD_WRITE, 0x36);
 
 // STEP 2
 // SET SEGRAM ADRESS OF ICON TO SET STATUS
 DIP204_engine(START_BYTE_CMD_WRITE, (CMD_SET_SEG_ADDR | ICON_ToSet));
 // WRITE DATA OF ICON STATUS
 DIP204_engine(START_BYTE_DAT_WRITE, ICON_Status);
 
 // STEP 3
 // FUNCTION SET TO 8BIIT, RE=0
 DIP204_engine(START_BYTE_CMD_WRITE, 0x30);
 // SET DDRAM ADR - RESTORE DDRAM ADDRESS
 DIP204_engine(START_BYTE_CMD_WRITE, 0x80);
  
 // STEP 4
 // FIX ME... THIS WOULD BE BEST HANDLED AS A COMMAND TO THE DISPLAY READ MEM CONTNENTS AS STEP 1
 // THEN RESTORE AS A FINAL STEP.  THIS IS A PATCH FOR NOW.
 // CHANGING THE ICON RESETS THE CURSOR POSITON TO 0,0 - THIS USES THE GLOBAL VARS OF CURSOR POSTION TO REPLACE
 DIP204_cursorToXY(sLineNum, sColNum);
 
 ctl_mutex_unlock(&DIP204Mutex); 
         
 } // END OF FUNCTION DIP204_ICON_set
 
 
 
 
 /*************************************************************************
 * Function Name: reverseBitOrder
 * Parameters: uint8_t
 * Return: uint8_t
 *
 * Description: Reverse the bit order of a byte.  MSb becomes LSb
 * STEP 1: Mask the 0bit of the value and OR it to the value to be returned
 * STEP 2: If not the last bit position shift the value to be returned to left
 * and shift the value to be reverse right
 * STEP 3: Return the reversed value
 *************************************************************************/
 uint8_t reverseBitOrder(uint8_t ByteValue)
 {
 uint8_t MaskByte = 0x01, ReverseByte = 0x00, MaskedValue;
 
 for (int BitCount = 0; BitCount < 8; BitCount++)
   {
   // STEP 1
   MaskedValue = ByteValue & MaskByte;
   ReverseByte |= MaskedValue;
   // STEP 2
   if (BitCount != 7)
     {
     ReverseByte = ReverseByte << 1;
     ByteValue = ByteValue >> 1;
     }
   }
 
 // STEP 3
 return (ReverseByte);
         
 } // END OF FUNCTION reverseBitOrder 




/*************************************************************************
 * Function Name: DIP204_cursorToXY
 * Parameters: uint8_t, uint8_t
 * Return: void
 *
 * Description: Sets the cursor position to the corresponding line number and coloumn number.
 * NOTE: Line number is from 1-4
 * NOTE: Column number is from 0-19
 * Then ouptuts this combine value to set the DD RAM Address
 * STEP 1: Set the Y / Vertical location based on Line number - Default to line 4
 * STEP 2: Set the X / Horizontal locatioin based on Column number - Default to Column 0
 * STEP 3: Send command to set location and wait until complete
 *************************************************************************/
 void DIP204_cursorToXY(uint8_t LineNumber, uint8_t ColumnNumber)
 {
  
 ctl_mutex_lock(&DIP204Mutex, CTL_TIMEOUT_NONE, 0); 
 // STEP 1
 sLineNum = LineNumber;
 sColNum = ColumnNumber;
 // SET THE CURSOR POSITION
 switch(LineNumber)
   {
   case 1: 
      LineNumber = LINE1_START_ADDRESS;
          break;
   case 2:
          LineNumber = LINE2_START_ADDRESS;
          break;
   case 3:
          LineNumber = LINE3_START_ADDRESS;
          break;
   case 4:
   default:
          LineNumber = LINE4_START_ADDRESS;
   break;
   }
 
 // STEP 2
 if (ColumnNumber > CMD_SET_DDR_ADDR - 1)
   ColumnNumber = 0;
 LineNumber = LineNumber + ColumnNumber;
 
 // STEP 3
 DIP204_engine(START_BYTE_CMD_WRITE, (CMD_SET_DDR_ADDR|LineNumber));
 delayXms(WAIT_FOR_DISPLAY);
 
 ctl_mutex_unlock(&DIP204Mutex); 
         
 } // END OF FUNCTION DIP204_cursorToXY
                 
                 

 
/*************************************************************************
 * Function Name: DIP204_txt_engine
 * Parameters: uint8_t *, uint8_t, uint8_t, uint8_t
 * Return: unsigned void
 *
 * Description: sends the string value to line x column y of DIP204 LCD Display of specified string length
 * Notes: Dummy reads are necessary to force the SPI action.       Line Number 1-4, Column Number (0-19)
 * STEP 1: Assign the array to a pointer and GOTO XY location of the display where data is to be written
 * STEP 2: Limit the max string that can be written to the lenght of the character display
 * STEP 3: Write the contents of the array memory by incrementing the pointer
 *************************************************************************/
 void DIP204_txt_engine(uint8_t StringArray[], uint8_t LineNumber, uint8_t ColumnNumber, uint8_t StringLength)
 {
  ctl_mutex_lock(&DIP204Mutex, CTL_TIMEOUT_NONE, 0); 
 
  uint8_t * DisplayText;
  
  // STEP 1
  DisplayText = StringArray;
  DIP204_cursorToXY(LineNumber, ColumnNumber);
  
  // STEP 2
  // LIMIT LINE LENGHT TO 20 CHARS - display width of LCD
  if (StringLength > LCD_CHAR_WIDTH)
    StringLength = LCD_CHAR_WIDTH;
  
  // STEP 3
  for (int CharCount = 0; CharCount < StringLength; CharCount++)
    {
    DIP204_engine(START_BYTE_DAT_WRITE, *DisplayText);
    DisplayText++;
    }
  
 ctl_mutex_unlock(&DIP204Mutex); 
 } // END OF FUNCTION DIP204_txt_engine




/*************************************************************************
 * Function Name: DIP204_clearLine
 * Parameters: uint8_t
 * Return: void
 *
 * Description: Clears the line number by writing spaces to all column locations.  Also
 * sets the cursor to column 0 position
 * NOTE: Line number is from 1-4
 * NOTE: Column number is from 0-19
 * 
 * STEP 1: Set the Y / Vertical address based on Line number - default to line 4
 * STEP 2: Write spaces in all locations
 * STEP 3: Set position to start of line
 *************************************************************************/
 void DIP204_clearLine(uint8_t LineNumber)
 {
         
 uint8_t LineText[20];
 
 // STEP 1
 if (LineNumber > DISPLAY_LINE_TOTAL)
   LineNumber = DISPLAY_LINE_TOTAL;
 
 // STEP 2
 //                01234567890123456789
 sprintf(LineText,"                    ");
 DIP204_txt_engine(LineText, LineNumber, 0, strlen(LineText));
 
 // STEP 3
 DIP204_cursorToXY(LineNumber, 0);
 
 } // END OF FUNCTION DIP204_clearLine
 
 
 
 
 /*************************************************************************
 * Function Name: DIP204_loadText
 * Parameters: uint8_t *, uint8_t 
 * Return: uint8_t
 *
 * Description: Right Justifies the text to the line number specified
 * NOTE: Line number is from 1-4
 * NOTE: Column number is from 0-19
 * NOTE: Cursor position is incremented to first position of next line
 * STEP 1: Calcualte string lenght and Column Number for right justify start of text position
 * STEP 2: Test to see if string is valid for right justify display - return False if not
 * STEP 3: print the string return true
 *************************************************************************/
 uint8_t DIP204_loadText(uint8_t StringArray[], uint8_t LineNumber)
 {
     
 uint8_t TextLength, ColumnNumber;
 
 // STEP 1
 TextLength = strlen(StringArray);
 ColumnNumber = LCD_CHAR_WIDTH - TextLength;
 
 // STEP 2 TEST
 if (ColumnNumber < 0)
   return(FALSE);
 
 // STEP 3
 DIP204_txt_engine(StringArray, LineNumber, ColumnNumber, strlen(StringArray));
 return(TRUE); 
 
 } // END OF FUNCTION DIP204_loadText




/*************************************************************************
 * Function Name: DIP204_cursor
 * Parameters: enum DIP204_CURSOR
 * Return: void
 *
 * Description: Sets the cursor to on, off or blink
 * STEP 1: Switch and perform action based on calling parameter
 * STEP 2: Wait for action to complete
 *************************************************************************/
void DIP204_set_cursor(enum DIP204_CURSOR CursorStatus)
 {
 
 // STEP 1
 switch(CursorStatus)
   {
   case CURSOR_ON:
     DIP204_engine(START_BYTE_CMD_WRITE, (CMD_DISPALY|MASK_DISPLAY_ON|MASK_CURSOR_ON));
   break;
   
   case CURSOR_OFF:
     DIP204_engine(START_BYTE_CMD_WRITE, (CMD_DISPALY|MASK_DISPLAY_ON));
   break;
   
   default:
   case CURSOR_BLINK:
     DIP204_engine(START_BYTE_CMD_WRITE, (CMD_DISPALY|MASK_DISPLAY_ON|MASK_CURSOR_ON|MASK_CURSOR_BLINK));
   break;
   }
 
 // STEP 2
 delayXms(WAIT_FOR_DISPLAY);
 
 } // END OF FUNCTION DIP204_cursor




/*************************************************************************
 * Function Name: DIP204_clearDisplay
 * Parameters: void
 * Return: void
 *
 * Description: Clears the display - the state of the cursor is left as is.
 * If you want to change the state of the cursor call DIP204_cursor.  Cannot be
 * called without init_DIP204
 * STEP 1: Clear screen and give time for command to process
 *************************************************************************/
 void DIP204_clearDisplay(void)
 {
  ctl_mutex_lock(&DIP204Mutex, CTL_TIMEOUT_NONE, 0); 
  
  DIP204_engine(START_BYTE_CMD_WRITE, 0x01);
  delayXms(WAIT_FOR_DISPLAY);
  
  ctl_mutex_unlock(&DIP204Mutex); 
  
 } // END OF FUNCTION DIP204_clear




/*************************************************************************
 * Function Name: DIP204_DisplayOff
 * Parameters: void
 * Return: void
 *
 * Description: Turns off the display.  Specifically Display off, Cursor off,
 * Blink off.
 * STEP 1: Turn off display
 *************************************************************************/
 void DIP204_DisplayOff(void)
 {
  
  // STEP 1
  DIP204_engine(START_BYTE_CMD_WRITE, CMD_DISPALY);
  delayXms(WAIT_FOR_DISPLAY);
  
 } // END OF FUNCTION DIP204_DisplayOff
 
         
         