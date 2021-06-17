/*****************************************************************
 *
 * File name:       LED_HC15C.C
 * Description:     Functions used to support LED operation on the HC 15C
 * Author:          Hab S. Collector
 * Date:            7/4/11
 * LAST EDIT:       8/03/2012 
 * Hardware:        NXP LPC1769
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 

// INCLUDES
#include "LED_HC15C.H"
#include "lpc17xx_gpio.h"

// GLOBAL VARS


// EXTERNS



/*************************************************************************
 * Function Name: initLED
 * Parameters:    void
 * Return:        void
 *
 * Description: Sets the LED pin value as outputs and leaves all LEDs OFF
 * see comment // LED INTERFACE in HC15C_DEFINES
 * STEP 1: Define LEDs as outputs (1=OUT 0=IN)
 * STEP 2: LEDs to off value 
 *************************************************************************/
 void init_LED(void)
   {
   // STEP 1
   GPIO_SetDir(PORT1,(LED_USB|LED_BAR0|LED_BAR1|LED_BAR2|LED_BAR3|LED_BAR4|LED_BAR5),1);
   
   // STEP 2
   GPIO_SetValue(PORT1,(LED_USB|LED_BAR0|LED_BAR1|LED_BAR2|LED_BAR3|LED_BAR4|LED_BAR5));
   }
   


/*************************************************************************
 * Function Name: switchLED
 * Parameters:    uint8_t LED_Status, uint32_t LEDs_ToChange
 * Return:        void
 *
 * Description: Sets the LED value to OFF, or ON.  LEDs_ToChange is an ORed value
 * see comment // LED INTERFACE in HC15C_DEFINES.  
 * STEP 1: Perform Status operation as a case.  Note setting the GPIO port bit turn off the LED.
 *************************************************************************/
 void switchLED(uint8_t LED_Status, uint32_t LEDs_ToChange)
 {
 
 // STEP 1
 switch(LED_Status)
  {
  case LED_ON:
      GPIO_ClearValue(PORT1, LEDs_ToChange);
  break;
  
  case LED_OFF:
       GPIO_SetValue(PORT1, LEDs_ToChange);
  break;
  
  default:
  break;
  }

 } // END OF switchLED

