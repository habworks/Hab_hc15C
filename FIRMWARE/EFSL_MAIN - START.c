 /**************************************************************************************************
 **************************PROJECT HEADER NOTES: YOU MUST READ THIS*********************************
 ***************************************************************************************************
 * PROJECT NAME: HC 15C TEST HARDWARE
 * PROJECT DESCRIPTION: A collection of routines to test hardware of HC 15C REV 1
 *
 * FIRMWARE ENGINEER:      HAB S. COLLECTOR
 * HARDWARE ENGINEER:      HAB S. COLLECTOR
 * PROJECT ENGINEER:       HAB S. COLLECTOR
 * ORGINAL RELEASE DATE: 7/3/11
 * RELEASE: 1, 
 *
 * THIS FIRMWARE RESIDES ON THE TARGET HARDWARE
 * EUT SPECIFICS: 
 * EUT HARDWARE PCB: PCB-HC15C
 * EUT HARDWARE SCH: SCH-HC15C
 * 
 *
 * FIRMWARE DEVELOPMENT ENVIRONMENT: Rowley CrossStudio for ARM REV 2.0.3...
 *
 * REVIEW MATERIAL: (TO BE ARCHEIVED WITH PROJECT)
 * REFERENCE DOCUMENTS: As this is firmware, Engineers reviewing this code may find it necessary to review both the hardware and software
 * documents associated with this file.  To complete your understanding consult reference documents for additional clarity:
 *               
 **************************************************************************************************
 **************************************************************************************************/


// INCLUDE FILES
#include <stdio.h>
#include <ctl_api.h>


//#include "CMSIS\lpc_types.h"
#include <cross_studio_io.h>
#include "HC15C_DEFINES.H"
#include "TIMER_HC15C.H"
#include "DIP204.H"


extern void initLED(void);
extern void switchLED (uint8_t LED_Status, uint32_t LEDs_ToChange);
extern void delayXms(uint32_t);



#ifdef DEBUG
  void check_failed(uint8_t *file, uint32_t line)
  {
  debug_printf("Wrong parameters value: file %s on line %d\r\n", file, line);
  while(1);
  }
#endif


// VARIABLES DEFINED HERE USED IN THIS MODULE AND OTHERS


// VARIABLES USED HERE DEFINED ELSEWHERE


// PROTOTYPE EXTERNAL FUNCTIONS





/*************************************************************************
 * Function Name: main
 * Parameters: void
 * Return: int
 *
 * Description: MAIN ROUTINE: THIS FIRMWARE USES A TASKING LIBRARY FUNCTIONS
 * FOR IRQ SETUPS ONLY.  THE USE OF THE TASKING FUNCTIONS DOES NOT MAKE THIS
 * AN RTOS BASED SOFTWARE.  THE SOFTWARE IS VERY PROCEDURALY IN NATURE.  IT
 * BASICALY RUNS ONE TEST AFTER ANOTHER
 * STEP 1 Define GPIO, set default states, init hardware resources
 * STEP 2 Show intro test screen 
 * STEP 3 Enter endless loop to run various tests on the EUT.  In the event of a failed
 * test - report as a fail and turn off.  If all tests pass then report as passed.
 * NOTES: 
 *      1. Fail and pass conditions are both handled by test functions
 *************************************************************************/
int main(void)
{
  // DEFINE VARS
  uint8_t LineText[20];
  uint32_t PCLK_Value;

  
  
    // DEFINE LOCAL VARIABLES

    // PLAY WITH TASK TIMER 0
    ctl_HC15C_OnTimerCounter0(ISR_TimerCounter0);
    

    // PLAY WITH LEDs
    // DEFINE LEDS
    initLED();
    switchLED(LED_ON, (LED_USB|LED_DRIVE|LED_METER|LED_HEX|LED_RAD));
    delayXms(500);
    switchLED(LED_OFF, (LED_USB|LED_DRIVE|LED_METER|LED_HEX|LED_RAD));
    delayXms(200);
    switchLED(LED_ON, LED_USB);
    delayXms(200);
    switchLED(LED_ON, (LED_USB|LED_DRIVE));
    delayXms(200);
    switchLED(LED_ON, (LED_USB|LED_DRIVE|LED_METER));
    delayXms(200);
    switchLED(LED_ON, (LED_USB|LED_DRIVE|LED_METER|LED_HEX));
    delayXms(200);
    switchLED(LED_ON, (LED_USB|LED_DRIVE|LED_METER|LED_HEX|LED_RAD));
    delayXms(200);
    switchLED(LED_OFF, (LED_USB|LED_DRIVE|LED_METER|LED_HEX|LED_RAD));
    delayXms(200);
    switchLED(LED_ON, LED_DRIVE);

  // PLAY WITH DISPLAY
    init_DIP204();
  
  while(1);


}
 // END OF MAIN



