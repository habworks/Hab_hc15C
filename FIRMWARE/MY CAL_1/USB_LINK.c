/*****************************************************************
 *
 * File name:       USB_LINK.C
 * Description:     Functions used to support USB LINK features.  Works in conjunction with
 *                  USB VCOM files of the VCOM USB dir GV WORKS REV VCOM_LIB REV 1.3.    
 * Author:          Hab S. Collector
 * Date:            12/31/11
 * LAST EDIT:       7/04/2012  
 * Hardware:        NXP LPC1769
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 
 
 #include "lpc17xx_pinsel.h"
 #include "USB_LINK.H"
 #include "usbserial.h"
 #include "HC15C_PROTOCOL.H"
 #include "CORE_FUNCTIONS.H"
 #include "DIP204.H"
 #include "LED_HC15C.H"
 
 // GLOBALS
 BOOLEAN VCOM_Link = FALSE;
 
 // EXTERNS
 extern Type_CalSettings CalSettings;
 
 
 
/*************************************************************************
 * Function Name: call_USB_VCOM_Link
 * Parameters:    void
 * Return:        void
 *
 * Description: Connects the HC15C to the USB via a Virtual COM.  Uses GV Works
 * USB Virtual COM library 
 * NOTE: There is no need to conserve power when in sleep mode because the HC15C will be 
 * powered by the computer's USB port and not its on board batteries.  
 * NOTE: Going to sleep while transmitting data via the USB port may cause a crash.
 * STEP 1: Set the Cal Setting for USB and disable sleep mode.  Display left and right arrow blinking = USB Connect
 * connect ICON
 *************************************************************************/
 void call_USB_VCOM_Link(void)
 {
 
 // STEP 1
 VCOM_Link = TRUE;
 CalSettings.SleepDisable = TRUE;
 usbSerialInit();
 DIP204_ICON_set(ICON_UP_ARROW, ICON_BLINK);
 DIP204_ICON_set(ICON_DOWN_ARROW, ICON_BLINK);
 } // END call_USB_VCOM_Link




/*************************************************************************
 * Function Name: call_USB_VCOM_UnLink
 * Parameters:    void
 * Return:        void
 *
 * Description: Disconnects the HC15C from the USB Virtual COM.  Uses GV Works
 * USB Virtual COM library 
 * NOTE: The function call_USB_Link changes the function of the USB LED pin from GPIO to
 * USB connect.  Change back to GPIO when you disconnect 
 * STEP 1: Do nothing if USB Link not connected
 * STEP 2: Disconnect USB VCOMM, remove ICONS, turn off USB LED by making it GPIO and
 * re-enable sleep mode
 *************************************************************************/
 void call_USB_VCOM_UnLink(void)
 {
 
 PINSEL_CFG_Type PINSEL_PinCfgStruct;
 
 // STEP 1
 if (!VCOM_Link)
   {
   return;
   }
 
  // STEP 2
 USBHwConnect(FALSE); 
 DIP204_ICON_set(ICON_UP_ARROW, ICON_OFF);
 DIP204_ICON_set(ICON_DOWN_ARROW, ICON_OFF);
 // FROM TABLE 8.5 USER MANUAL LPC17XX - NOTE: P1.18 IS GPIO
 // MUST BE MADE GPIO BEFORE CALL TO TURN OF USB LED
 PINSEL_PinCfgStruct.Funcnum = 0;
 PINSEL_PinCfgStruct.OpenDrain = 0;
 PINSEL_PinCfgStruct.Pinmode = 0;
 PINSEL_PinCfgStruct.Pinnum = 18;
 PINSEL_PinCfgStruct.Portnum = PORT1;
 PINSEL_ConfigPin(&PINSEL_PinCfgStruct); 
 switchLED(LED_OFF, LED_USB);
 CalSettings.SleepDisable = FALSE;
 VCOM_Link = FALSE;
 
 } // END OF call_USB_VCOM_UnLink




/*************************************************************************
 * Function Name: call_WriteToUSB_Meter
 * Parameters:    double uint8_t
 * Return:        void
 *
 * Description: Accepts the meter type (Volt or Ohms) and creates a packet using the 
 * passed double value (the voltage or ohms read).  This packet is then transmitted via 
 * the USB COM port via a Virtual Com Port link.  
 * NOTE: The function usbSerialInit must be previously called.
 * STEP 1: Disassemble the double value into bytes for transmission
 * STEP 2: Create a Transmission Packet
 * STEP 3: Transmit the packet via the VCOM port
 *************************************************************************/
 void call_WriteToUSB_Meter(double Value, uint8_t MeterType)
 {
   
 Union_DoubleInBytes VoltValue;
 uint8_t TX_PayLoad[sizeof(double)],
         TX_Packet[20];
 
 // STEP 1
 VoltValue.DoubleValue = Value;
 for (uint8_t ByteCount = 0; ByteCount < sizeof(double); ByteCount++)
   {
   TX_PayLoad[ByteCount] = VoltValue.ByteValue[ByteCount];
   }
 
 // STEP 2
 createTX_Buffer(TX_PayLoad, sizeof(double), TX_Packet, MeterType);
 
 // STEP 3
 for (uint8_t ByteCount = 0; ByteCount < (sizeof(double) + 4); ByteCount++)
   {
    VCOM_putc(TX_Packet[ByteCount]);
   }

 } // END OF  call_WriteToUSB_Meter

 

  
  

  
