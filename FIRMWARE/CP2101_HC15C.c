/*****************************************************************
 *
 * File name:       CP2101_HC15C.H
 * Description:     Project definitions and function prototypes for use with UART_HC15C.c
 * Author:          Hab S. Collector
 * Date:            7/21/2011
 * Hardware:               
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent 
 *                  on other includes - everything these functions need should be passed to them
 *
 *****************************************************************/
 
 // INCLUDES
#include "CP2101_HC15C.H"
#include <stdio.h>

// GLOBAL VARS

// EXTERNS
extern void delayXms(uint8_t);




/*************************************************************************
 * Function Name: init_CP2101_polling
 * Parameters: void
 * Return: void
 *
 * Description: Configures UART0 FOR POLLING INTERFACE TO USB TO UART CP2101
 * can be called.
 * STEP 1: Configure P0.2 and P0.3 as UART 0 pins
 * STEP 2: Configure UART TX/RX and FIFO structures 
 * STEP 3: Enable UART
 *************************************************************************/
 void init_CP2101_polling(void)
 {
   // UART Configuration structure variable
	UART_CFG_Type UARTConfigStruct;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	// Pin configuration for UART0
	PINSEL_CFG_Type PinCfgStruct;
  
  // STEP 1
  PinCfgStruct.Funcnum = 1;
	PinCfgStruct.OpenDrain = 0;
	PinCfgStruct.Pinmode = 0;
	PinCfgStruct.Pinnum = 2;
	PinCfgStruct.Portnum = PORT0;
	PINSEL_ConfigPin(&PinCfgStruct);
	PinCfgStruct.Pinnum = 3;
	PINSEL_ConfigPin(&PinCfgStruct);
  
  // STEP 2
  /* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 9600bps
	 * 8 data bit
	 * 1 Stop bit
	 * None parity
	 */
	//UART_ConfigStructInit(&UARTConfigStruct);
   // Initialize UART0 peripheral with given to corresponding parameter
  UARTConfigStruct.Baud_rate = 600; // PROBLEM 600 WORKS FOR 9600
	UARTConfigStruct.Databits = UART_DATABIT_8;
	UARTConfigStruct.Parity = UART_PARITY_NONE;
	UARTConfigStruct.Stopbits = UART_STOPBIT_1;
   
	UART_Init(LPC_UART0, &UARTConfigStruct);
 
	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig(LPC_UART0, &UARTFIFOConfigStruct);

  // STEP 3
  // Enable UART Transmit
	UART_TxCmd(LPC_UART0, ENABLE);

 } // END OF FUNCTION init_CP2101_polling




/*************************************************************************
 * Function Name: init_CP2101_polling
 * Parameters: void
 * Return: void
 *
 * Description: Prints a test string and test character
 * NOTE: UART must first be initialized
 * STEP 1: Print the test string
 * STEP 2: Print a test character
 *************************************************************************/
 void CP2101_testprint(void)
 {
 
 uint8_t PrintString[] = CP2101_TEST_STRING;
 
 UART_Send(LPC_UART0, PrintString, sizeof(PrintString), BLOCKING);
 UART_SendByte(LPC_UART0, '0');
 
 } // END OF CP2101_testprint
  
 