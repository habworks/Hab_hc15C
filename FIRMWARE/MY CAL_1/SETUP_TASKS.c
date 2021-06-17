/*****************************************************************
 *
 * File name:       SETUP_TASKS.C
 * Description:     CTL RTOS Functions used to support the user setup functions 
                    related functions of the HC15C
 * Author:          Hab S. Collector`
 * Date:            3/5/12
 * LAST EDIT:       7/03/2012 
 * Hardware:        NXP LPC1769
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 
 
#include "SETUP_TASKS.H"
#include "DIP204.H"
#include "TOUCH_TASKS.H"
#include "CORE_FUNCTIONS.H"
#include <ctl_api.h>
#include <stdio.h>
#include <string.h>


// GLOBALS

// EXTERNS
extern CTL_EVENT_SET_t CalEvents;
extern CTL_TASK_t clock_task;
extern Type_CalSettings CalSettings;
extern volatile uint8_t BackLightTimer;




/*************************************************************************
 * Function Name: setup_taskFn
 * Parameters:    void *
 * Return:        void
 *
 * Description: RTOS CTL task to display the UI Setup parameters. This task must 
 * not be called unless the call_SetupMode is first called.  The functions: call_BL_TimeOutSetup,
 * call_CalVerboseSetup, and call_TimeToSleepSetup set this event
 * NOTE: The events set up the UI functions of Backlight timeout, Cal Verboseness,
 * and sleep timeout.
 * NOTE: Back light and sleep time out are in 10's of seconds
 * STEP 1: Clear the previous results
 * STEP 2: Display the updated settings
 * STEP 3: Clear the event
 *************************************************************************/
 void setup_taskFn(void *p)
 {
 
 uint8_t LineText[DISPLAY_COLUMN_TOTAL];
 
 while (1)
   {
   ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS, &CalEvents, (EVENT_SETUP), CTL_TIMEOUT_NONE, 0);
   
   // STEP 1
   // CLEAR PREVIOUS
   sprintf(LineText, "   ");
   DIP204_txt_engine(LineText, SETUP_BL_ROW, SETUP_POSITION, strlen(LineText));
   DIP204_txt_engine(LineText, SETUP_CAL_TALK_ROW, SETUP_POSITION, strlen(LineText));
   DIP204_txt_engine(LineText, SETUP_TIMEOUT_ROW, SETUP_POSITION, strlen(LineText));
   
   // STEP 2
   // DISPLAY SETTINGS
   sprintf(LineText, "%d", (CalSettings.Setup.BackLightTimeOut * 10));
   DIP204_txt_engine(LineText, SETUP_BL_ROW, SETUP_POSITION, strlen(LineText));
   sprintf(LineText, "%d", CalSettings.Setup.CalVerbose);
   DIP204_txt_engine(LineText, SETUP_CAL_TALK_ROW, SETUP_POSITION, strlen(LineText));
   sprintf(LineText, "%d", (CalSettings.Setup.TimeToSleep * 10));
   DIP204_txt_engine(LineText, SETUP_TIMEOUT_ROW, SETUP_POSITION, strlen(LineText));

   // STEP 3
   ctl_events_set_clear(&CalEvents, 0, (EVENT_SETUP));    
   }
   
 } // END OF setup_taskFn





/*************************************************************************
 * Function Name: call_SetupMode
 * Parameters:    void
 * Return:        void
 *
 * Description: Places the calculator in User Setup Mode.  Setup Mode displays 
 * three user options to change: BackLight Time Out time - the time it takes the 
 * the back light to time out.  Cal Verbose - the level of voice help the calcualtor
 * responds with.
 * STEP 1: End the present mode, set up keys to use
 * STEP 2: Ready the display
 * STEP 3: Set for Setup Mode
 *************************************************************************/
 void call_SetupMode(void)
 {
 
 uint8_t LineText[DISPLAY_COLUMN_TOTAL];
 
 // STEP 1
 call_EndPresentMode();
 CalSettings.Mask_KeyTouchA = CalSettings.Mask_KeyTouchB = NO_KEYS_SELECTED;
 CalSettings.Mask_KeyTouchA = (MASK_ATN_KEY | MASK_KEY_LSHIFT);
 CalSettings.Mask_KeyTouchB = (MASK_KEY_CAL | MASK_KEY_1 |
                                              MASK_KEY_2 |
                                              MASK_KEY_3 |
                                              MASK_KEY_METER |
                                              MASK_KEY_FLASH |
                                              MASK_KEY_MUSIC );

 // STEP 2
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 strcpy(LineText, "USER SETUP:");
 DIP204_txt_engine(LineText, 1, 0, strlen(LineText));
 strcpy(LineText, "1.BL TIME OUT:");
 DIP204_txt_engine(LineText, SETUP_BL_ROW, 2, strlen(LineText));
 strcpy(LineText, "2.CAL VERBOSE:");
 DIP204_txt_engine(LineText, SETUP_CAL_TALK_ROW, 2, strlen(LineText));
 strcpy(LineText, "3.TIME 2DREAM:");
 DIP204_txt_engine(LineText, SETUP_TIMEOUT_ROW, 2, strlen(LineText));
 // UPDATE THE VALUES
 sprintf(LineText, "%d", (CalSettings.Setup.BackLightTimeOut * 10));
 DIP204_txt_engine(LineText, SETUP_BL_ROW, SETUP_POSITION, strlen(LineText));
 sprintf(LineText, "%d", CalSettings.Setup.CalVerbose);
 DIP204_txt_engine(LineText, SETUP_CAL_TALK_ROW, SETUP_POSITION, strlen(LineText));
 sprintf(LineText, "%d", (CalSettings.Setup.TimeToSleep * 10));
 DIP204_txt_engine(LineText, SETUP_TIMEOUT_ROW, SETUP_POSITION, strlen(LineText));
 
 // STEP 3
 if (CalSettings.CalMode != SETUP_MODE)
   {
    #if defined(REMOVE_RESTORE)
      ctl_task_restore(&setup_task);
    #elif defined(SUSPEND_RUN)
      ctl_HabTaskRun(&setup_task);
    #endif 
   // DISPLAY ICON
   DIP204_ICON_set(ICON_MAIL, ICON_ON);
   CalSettings.CalMode = SETUP_MODE;
   }  
   
 } // END OF call_SetupMode
 
 
 
 
 /*************************************************************************
 * Function Name: call_SetupModeEnd
 * Parameters: void
 * Return: void
 *
 * Description: Performs the actions necessary to end Setup Mode.  End such that the next
 * mode can start itself.  This function sets CalMode to calculator mode as a default.  
 * It is up to the next mode function to set its mode of operation.
 * STEP 1: Select all Keys for the next function
 * STEP 2: Prep the display for use by the next Mode
 **************************************************************************/
 void call_SetupModeEnd(void)
 {
 
 // STEP 1
 // WAIT FOR METER EVENT TO CLEAR - AS THIS WILL BE A STABLE TIME TO SWITCH
 #if defined(REMOVE_RESTORE)
    ctl_task_remove(&setup_task);
  #elif defined(SUSPEND_RUN)
    ctl_HabTaskSuspend(&setup_task);
  #endif

 // STEP 2
 select_All_Normal_Keys();

 // STEP 3
 DIP204_ICON_set(ICON_MAIL, ICON_OFF);
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 CalSettings.CalMode = CAL_MODE;
  
 } // END OF call_SetupModeEnd




/*************************************************************************
 * Function Name: call_BL_TimeOutSetup
 * Parameters: void
 * Return: void
 *
 * Description: Changes the Cal Back Light Timeout settings to the next selection and sets
 * an event to run the setup tasks.  The time value is in 10's of seconds
 * STEP 1: Change Settings
 * STEP 2: Clear the current backlight time and Setup for event
 **************************************************************************/
void call_BL_TimeOutSetup(void)
{
 
 // STEP 1
 CalSettings.Setup.BackLightTimeOut++;
 if (CalSettings.Setup.BackLightTimeOut > MAX_TIME_TO_BACKLIGHT)
   CalSettings.Setup.BackLightTimeOut = MIN_TIME_TO_BACKLIGHT;
 
 // STEP 2
 BackLightTimer = 0;
 ctl_events_set_clear(&CalEvents, EVENT_SETUP, 0);
 
} // END OF call_BL_TimeOutSetup




/*************************************************************************
 * Function Name: call_CalVerboseSetup
 * Parameters: void
 * Return: void
 *
 * Description: Changes the Cal Verbose settings to the next selection and sets
 * an event to run the setup tasks
 * STEP 1: Setup for event
 **************************************************************************/
void call_CalVerboseSetup(void)
{
 
 // STEP 1
 CalSettings.Setup.CalVerbose++;
 if (CalSettings.Setup.CalVerbose > FULL_INTERACTIVE)
   CalSettings.Setup.CalVerbose = NO_SOUND;
 ctl_events_set_clear(&CalEvents, EVENT_SETUP, 0);
 
} // END OF call_CalVerboseSetup




/*************************************************************************
 * Function Name: call_TimeToSleepSetup
 * Parameters: void
 * Return: void
 *
 * Description: Changes the Cal Time to sleep settings to the next selection and sets
 * an event to run the setup tasks.  The time value is in 10's of seconds
 * STEP 1: Setup for event
 **************************************************************************/
void call_TimeToSleepSetup(void)
{
 
 // STEP 1
 CalSettings.Setup.TimeToSleep++;
 if (CalSettings.Setup.TimeToSleep > MAX_TIME_TO_SLEEP)
   CalSettings.Setup.TimeToSleep = MIN_TIME_TO_SLEEP;
 ctl_events_set_clear(&CalEvents, EVENT_SETUP, 0);
 
} // END OF call_TimeToSleepSetup




