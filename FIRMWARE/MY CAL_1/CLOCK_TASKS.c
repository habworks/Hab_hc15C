/*****************************************************************
 *
 * File name:       CLOCK_TASK.C
 * Description:     CTL RTOS Functions used to support time clock related functions of the HC15C
 * Author:          Hab S. Collector
 * Date:            11/24/11
 * LAST EDIT:       7/15/2012
 * Hardware:        NXP LPC1769
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 

#include "CLOCK_TASKS.H"
#include "AUDIO_TASKS.H"
#include "TOUCH_TASKS.H"
#include "START_N_SLEEP_TASKS.H"
#include "DIP204.H"
#include "CORE_FUNCTIONS.H"
#include "HC15C_PROTOCOL.H"
#include "USB_LINK.H"
#include "lpc17xx_rtc.h"
#include "usbserial.h"
#include <ctl_api.h>
#include <stdio.h>
#include <string.h>

// GLOBALS
Type_SWAT SWAT;
BOOLEAN EnableClockSet = FALSE;
uint8_t CursorSet = TIME_HOUR;
Type_AlarmSetTime AlarmSetTime;


// EXTERNS
extern CTL_EVENT_SET_t CalEvents;
extern CTL_TASK_t clock_task;
extern Type_CalSettings CalSettings;
extern BOOLEAN VCOM_Link;
extern CTL_MESSAGE_QUEUE_t AudioQueue;



/*************************************************************************
 * Function Name: clock_taskFn
 * Parameters:    void *
 * Return:        void
 *
 * Description: RTOS CTL task to manage the time clock function.  The calculator 
 * Time Mode displays the RTC clock and stop watch.  The job of this task is to
 * update the clock and stop watch time of the calculator
 * STEP 1: Update the Clock time - The RTC ISR sets an event every second.  Update the display
 * only in the position that changes (sec, min, hours - in that order) as this will look
 * smooth to the user.  As the hour updates recall the clock Mode function as this will 
 * also update the date (11:59pm going to 12:00am is date change)
 * STEP 2: Check to see if user wants to set time.  The time is set from the HC15C Win App.
 * For the user to set the time, the USB Link function key would have been pressed and the 
 * unit would have to be connected to a USB port.  If so, wait for the Set Date Time command
 * to be sent from the Win App.  The app sends a string to the USB - redisplay the date time and
 * close the link.
 * STEP 3: Update the StopWatch function.  Timer 1 ISR sets an event every 10msec.  So the 
 * stop watch is to updated in only the position that changes every 10msec.  10msec changes
 * to the next increment after 10 iterations.  Seconds change to minutes after 60.  Minutes to
 * hours after 60.  At 24 hours stop the stop watch and require a reset.
 * STEP 4: An alarm event can be set from anywhere.  On event clear the display, display alarm
 * is set with alarm icon blinking.  Turn off all keys, but stop (5) and ATN keys and play alarm 
 * sound.  Only a pressing of 5 or ATN key will turn off alarm and return to present mode
 *************************************************************************/
 void clock_taskFn(void *p)
 {
   
 static uint32_t SecondVal, MinuteVal, HourVal, TimeColPosition;
 uint8_t TimeUpdate[4];
 uint8_t SetTimeFromVCOM[30],
         LineText[DISPLAY_COLUMN_TOTAL],
         TimeString[DISPLAY_COLUMN_TOTAL],
         DateString[DISPLAY_COLUMN_TOTAL];
 Type_DateTimeVCOM DateTimeVCOM;

 while (1)
   {
   ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS, &CalEvents, (EVENT_CLOCK|EVENT_SWAT|EVENT_ALARM), CTL_TIMEOUT_NONE, 0);
  
   // STEP 1
   // TIME INCREMENT THE CLOCK
   if (CalEvents & EVENT_CLOCK)
     {
     if (HourVal != RTC_GetTime(LPC_RTC, RTC_TIMETYPE_HOUR))
       {
       HourVal = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_HOUR);
       // NOTE IF THE HOUR HAS CHANGED IT IS POSSIBLE THE DATE HAS CHANGED - CLEAR DATE TIME AND RE-DISPLAY
       DIP204_clearLine(TIME_ROW);
       DIP204_clearLine(DATE_ROW);
       call_BuildDateTimeStrings(DateString, TimeString);
       DIP204_txt_engine(TimeString,TIME_ROW,0,strlen(TimeString));
       DIP204_txt_engine(DateString,DATE_ROW,0,strlen(DateString)); 
       }
     if (MinuteVal != RTC_GetTime(LPC_RTC, RTC_TIMETYPE_MINUTE))
       {
       MinuteVal = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_MINUTE);
       TimeColPosition = MINUTE_POSITION;
       sprintf(TimeUpdate,"%02d", MinuteVal);
       DIP204_txt_engine(TimeUpdate,TIME_ROW,TimeColPosition,strlen(TimeUpdate)); 
       }
     if (SecondVal != RTC_GetTime(LPC_RTC, RTC_TIMETYPE_SECOND))
       {
       SecondVal = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_SECOND);
       TimeColPosition = SECOND_POSITION;
       sprintf(TimeUpdate,"%02d", SecondVal);
       DIP204_txt_engine(TimeUpdate,TIME_ROW,TimeColPosition,strlen(TimeUpdate)); 
       } 
     ctl_events_set_clear(&CalEvents, 0, EVENT_CLOCK);
     } // END OF CLOCK EVENT
   
   // STEP 2
   // IF USER PRESSED USBLINK - GET THE LOCAL TIME FROM HC15C WIN APP
   if (VCOM_Link & CalSettings.USB_Link & EnableClockSet)
     {
     // DISPLAY CLOCK UPDATE MESSAGE WAITING
     sprintf(LineText,PROMPT_WAITING_ON_UI);
     DIP204_txt_engine(LineText,CLOCK_SET_ROW,0,strlen(LineText));
     // SEND COMMAND TO ENABLE CLOCK SET IN WIN APP.  BLOCK HERE UNTIL TIME MSG IS RECEIVED FROM HC15C WIN APP
     call_WriteToUSB_Meter(0, TYPE_TIME);  // ENABLE TIME IN WIN APP HAS NO VALUE - ONLY CARES ABOUT COMMAND TYPE
     VCOM_gets(SetTimeFromVCOM);
     // MSG RECEIVED - CLOSE THE LINK
     call_USB_VCOM_UnLink();
     // PARASE THE DATA AND LOAD TIME
     if (parseDateTimeFromVCOM(SetTimeFromVCOM, &DateTimeVCOM))
       {
       RTC_SetTime (LPC_RTC, RTC_TIMETYPE_SECOND, DateTimeVCOM.Second);
       RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MINUTE, DateTimeVCOM.Minute);
       RTC_SetTime (LPC_RTC, RTC_TIMETYPE_HOUR, DateTimeVCOM.Hour);
       RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MONTH, DateTimeVCOM.Month);
       RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH, DateTimeVCOM.Date);
       RTC_SetTime (LPC_RTC, RTC_TIMETYPE_YEAR, DateTimeVCOM.Year);
       }
     // RE-DISPLAY TIME DATE AND CLEAR UI WAIT PROMT
     DIP204_clearLine(TIME_ROW);
     DIP204_clearLine(DATE_ROW);
     DIP204_clearLine(CLOCK_SET_ROW);
     call_BuildDateTimeStrings(DateString, TimeString);
     DIP204_txt_engine(TimeString,TIME_ROW,0,strlen(TimeString));
     DIP204_txt_engine(DateString,DATE_ROW,0,strlen(DateString));
     }
      
   // STEP 3
   // CHECK FOR AND ACT ON STOP WATCH EVENTS
   if (CalEvents & EVENT_SWAT)
     {
     // UPDATE MILISECOND COUNT
     SWAT.TenMiliSecTime += 10;
     if (SWAT.TenMiliSecTime < 100)
       {
       sprintf(TimeUpdate,"%02d", SWAT.TenMiliSecTime);
       DIP204_txt_engine(TimeUpdate,SWAT_ROW,SWAT_MIL_POSITION,strlen(TimeUpdate));
       }
     else
       {
       // UPDATE SECOND COUNT
       SWAT.TenMiliSecTime = 0;
       sprintf(TimeUpdate,"%02d", SWAT.TenMiliSecTime);
       DIP204_txt_engine(TimeUpdate,SWAT_ROW,SWAT_MIL_POSITION,strlen(TimeUpdate));
       SWAT.SecTime++;
       if (SWAT.SecTime < 60)
         {
         sprintf(TimeUpdate,"%02d", SWAT.SecTime);
         DIP204_txt_engine(TimeUpdate,SWAT_ROW,SWAT_SEC_POSITION,strlen(TimeUpdate));
         }
       else
         {
         // UPDATE MINUTE COUNT
         SWAT.SecTime = 0;
         sprintf(TimeUpdate,"%02d", SWAT.TenMiliSecTime);
         DIP204_txt_engine(TimeUpdate,SWAT_ROW,SWAT_MIL_POSITION,strlen(TimeUpdate));
         sprintf(TimeUpdate,"%02d", SWAT.SecTime);
         DIP204_txt_engine(TimeUpdate,SWAT_ROW,SWAT_SEC_POSITION,strlen(TimeUpdate));
         SWAT.MinuteTime++;
         if (SWAT.MinuteTime < 60)
           {
           sprintf(TimeUpdate,"%02d", SWAT.MinuteTime);
           DIP204_txt_engine(TimeUpdate,SWAT_ROW,SWAT_MIN_POSITION,strlen(TimeUpdate));
           }
         else
           {
           // UPDATE HOUR COUNT
           SWAT.MinuteTime = 0;
           sprintf(TimeUpdate,"%02d", SWAT.MinuteTime);
           DIP204_txt_engine(TimeUpdate,SWAT_ROW,SWAT_MIN_POSITION,strlen(TimeUpdate));
           SWAT.HourTime++;
           if (SWAT.HourTime < 24)
             {
             sprintf(TimeUpdate,"%02d", SWAT.HourTime);
             DIP204_txt_engine(TimeUpdate,SWAT_ROW,SWAT_HR_POSITION,strlen(TimeUpdate));
             }
           else
             call_SWAT_Stop();
           }
         }
       }    
     ctl_events_set_clear(&CalEvents, 0, EVENT_SWAT);
     } // END OF SWAT EVENT
     
     // STEP 4
     // CHECK FOR AND ACT ON STOP TIME ALARM EVENT
     if (CalEvents & EVENT_ALARM)
       {
       // AS YOU CAN BE IN ANY MODE: CALL ALARM MODE TO END THE PRESENT MODE OF WHAT EVER TASK
       // YOU WERE IN WHEN THE ALARM WENT OFF.  
       //call_EndPresentMode();
       // SHOW ALARM TO USER
       call_ShowAlarm();
       DIP204_ICON_set(ICON_RING, ICON_BLINK);
       Type_AudioQueueStruct AudioQueueStruct;
       strcpy(AudioQueueStruct.FileName, TIME_ALARM_WAV);
       AudioQueueStruct.PlayLevel = CORE_SOUND;
       // PLAY ALRM UNTIL USER PRESS STOP
       do
         {
         ctl_PostMessageByMemAllocate(&AudioQueueStruct);
         ctl_timeout_wait(ctl_get_current_time()+1000);
         } while (CalSettings.TimeAlarm.AlarmEnable);
       CalSettings.TimeAlarm.AlarmEvent = FALSE;
       ctl_events_set_clear(&CalEvents, 0, 0xFFF);
       // RETUN USER TO CLOCK MODE - USER WOULD LIKE TO SEE THE TIME
       call_ClockMode();
       } // END OF EVENT_ALARM
   } // END OF WHILE
   
 
} // END OF clock_taskFn




/*************************************************************************
 * Function Name: call_ClockMode
 * Parameters:    void 
 * Return:        void
 *
 * Description: Places the calculator in Clock mode.  Clock Mode displays the
 * RTC clock at the present time and date and stop watch reset to zero.  
 * Clock Mode is one of the fundamental modes of operation.  Clock mode is run from a task
 * which is restored by this function call.  Note neither the time clock nor
 * stop watch are updated here.  They are updated in the clock task (event set by irq).
 * STEP 1: End the present mode.  Clear display, set clock icon and set key mask as 
 * appropriate for clock mode.
 * STEP 2: Build the Time and Date Strings
 * STEP 3: Display Time and Date
 * STEP 4: Display stop watch at 0
 * STEP 5: Set to Cal Mode and restore the clock task (clock task also needs RTC IRQ).
 *************************************************************************/
 void call_ClockMode(void)
 {
 
 uint8_t TimeString[DISPLAY_COLUMN_TOTAL],
         DateString[DISPLAY_COLUMN_TOTAL],
         LineText[DISPLAY_COLUMN_TOTAL];
  
 // STEP 1
 call_EndPresentMode();
 EnableClockSet = FALSE;
 CalSettings.Mask_KeyTouchA = CalSettings.Mask_KeyTouchB = NO_KEYS_SELECTED;
 CalSettings.Mask_KeyTouchA = (MASK_ATN_KEY | MASK_KEY_LSHIFT);
 CalSettings.Mask_KeyTouchB = (MASK_KEY_2| MASK_KEY_4 | MASK_KEY_5 | MASK_KEY_6 | MASK_KEY_CAL   | 
                                                                      MASK_KEY_METER |
                                                                      MASK_KEY_ALINK |
                                                                      MASK_KEY_FLASH |
                                                                      MASK_KEY_MUSIC );
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 
 // STEP 2
 // BUILD TIME AND DATE STRINGSX
 call_BuildDateTimeStrings(DateString, TimeString);
 
 // STEP 3
 // DISPLAY TIME AND DATE                                    
 DIP204_txt_engine(TimeString,TIME_ROW,0,strlen(TimeString));
 DIP204_txt_engine(DateString,DATE_ROW,0,strlen(DateString));   
 
 // STEP 4
 /* SET CAL MODE LAST SO THAT YOU CAN RESTORE CLOCK TASK ON FIRST TIME IN THIS FUNCTION AND WILL
  * NOT RESTORE ON SUBSEQUENT CALLS TO THIS FUNCTION IF ALREADY IN CLOCK MODE
  * NOTE: THE STOP WATCH MUST BE DISPLAYED FROM HERE AS WELL FOR THE SIMILAR REASONS
  * STOP WATCH CAN BE UPDATED FROM THE CLOCK TASKS AS WELL. */
 if (CalSettings.CalMode != CLOCK_MODE)
   {
    #if defined(REMOVE_RESTORE)
      ctl_task_restore(&clock_task);
    #elif defined(SUSPEND_RUN)
      ctl_HabTaskRun(&clock_task);
    #endif 
   DIP204_ICON_set(ICON_TIME, ICON_ON);
   CalSettings.CalMode = CLOCK_MODE;
   SWAT.Status = STOP;
   SWAT.TenMiliSecTime = SWAT.SecTime = SWAT.MinuteTime = SWAT.HourTime = 0;
   // SET CLOCK TASK TO RUN. USES RTC IRQ ENABLED TO IRQ EVERY SECOND
   RTC_CntIncrIntConfig(LPC_RTC, RTC_TIMETYPE_SECOND, ENABLE);
   }
 
 sprintf(LineText,"SWAT: %02d:%02d:%02d:%02d", SWAT.HourTime,SWAT.MinuteTime,SWAT.SecTime,SWAT.TenMiliSecTime); 
 DIP204_txt_engine(LineText,SWAT_ROW,0,strlen(LineText)); 
  
 } // END OF call_ClockMode




/*************************************************************************
 * Function Name: RTC_IRQHandler
 * Parameters: void
 * Return: void
 *
 * Description: IRQ Handler for the RTC.  When the RTC IRQ is enabled this
 * IRQ services: a stoppage of the RTC osc, a time IRQ (by setting the clock event),
 * and the alarm IRQ by setting the alarm IRQ.
 * NOTE: This IRQ is used to: increment the second, minute and hour times within
 * the clock task.  The time is incremented this way so as the time change to the user 
 * appears smooth.
 * STEP 1: Check for RTC stop error and try to correct
 * STEP 2: Check for time increment.  If so set event for clock task if Clock Mode or display
 * count down time for audio file if in Music Mode
 * STEP 3: Check for Alarm IRQ.  Convert AlarmSetTime to Military time and check if it
 * equals the preset time.  If so set an alarm event.  Note the alarm event could have taken
 * place while in deep sleep, if so wake from deep sleep as you should.
 **************************************************************************/
 void RTC_IRQHandler(void)
 {
 
 uint8_t LineText[DISPLAY_COLUMN_TOTAL];

 // STEP 1 
 // CHECK IF RTC ERROR STOP CONDITION AND ATTEMPT RESTART
 if (LPC_RTC->RTC_AUX & RTC_OSC_FAIL)
   {
   LPC_RTC->RTC_AUX |= RTC_OSC_FAIL;
   }

 // STEP 2
 // INCREMENT THE CLOCK AT CLOCK IRQ INTERVAL (1s)
 if (RTC_GetIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE))
   {
    // HANDLE CLOCK RELATED TASKS
		if (CalSettings.CalMode == CLOCK_MODE)
      ctl_events_set_clear(&CalEvents, EVENT_CLOCK, 0);
   // HANDLE MUSIC AND AUDIO PLAY BACK TASKS
   if ((CalSettings.CalMode == MUSIC_LIST_MODE) && (CalSettings.MusicPlayBack.Playing) && (!CalSettings.MusicPlayBack.Pause))
     {
     if (CalSettings.MusicPlayBack.PlayTimeInSeconds != 0)
       CalSettings.MusicPlayBack.PlayTimeInSeconds--;
     uint8_t TimeRemainingInMinutes = (CalSettings.MusicPlayBack.PlayTimeInSeconds/60);
     uint8_t TimeRemainingInSeconds = CalSettings.MusicPlayBack.PlayTimeInSeconds - (TimeRemainingInMinutes * 60);
     sprintf(LineText,"%02d:%02d", TimeRemainingInMinutes, TimeRemainingInSeconds);
     DIP204_txt_engine(LineText, 4, 0, strlen(LineText));
     }
		RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE);
	 }

 // STEP 3
 // CHECK FOR ALARM IRQ
 if (RTC_GetIntPending(LPC_RTC, RTC_INT_ALARM))
   {
   // CONVERT TO MILITARTY TIME TO TEST FOR ALARM
   uint8_t MilitaryHour = AlarmSetTime.Hour; 
   if (AlarmSetTime.AM_Time)
     {
     if (MilitaryHour == 12)
       MilitaryHour = 0;
     }
   else
      MilitaryHour += 12;
   // TEST FOR ALARM
   uint8_t PresentHour = (uint8_t)RTC_GetTime(LPC_RTC, RTC_TIMETYPE_HOUR);
   uint8_t PresentMinute = (uint8_t)RTC_GetTime(LPC_RTC, RTC_TIMETYPE_MINUTE);
   if ((MilitaryHour == PresentHour) && (AlarmSetTime.Minute == PresentMinute))
     {
     // SET THE ALARM EVENT ONLY IF THE ALARM EVENT HAS BEEN ENABLED
     if (CalSettings.TimeAlarm.AlarmEnable)
       {
       // CHECK IF ALARM WAKE DEVICE FROM SLEEP
       if (CalSettings.TimeAlarm.HC15C_InDeepSleep)
         {
         wakeFromSleep();
         CalSettings.TimeAlarm.AlarmEvent = TRUE;
         ctl_events_set_clear(&CalEvents, EVENT_ALARM, 0);
         }
       else
         {
         CalSettings.TimeAlarm.AlarmEvent = TRUE;
         ctl_events_set_clear(&CalEvents, EVENT_ALARM, 0);
         }
       }
     }
   
   // CLEAR IRQ
   RTC_ClearIntPending(LPC_RTC, RTC_INT_ALARM);
   }
  
 } // END OF RTC_IRQHandler




/*************************************************************************
 * Function Name: init_Clock
 * Parameters: void
 * Return: void
 *
 * Description: Init the RTC for use.  The RTC is setup to: IRQ every second, no alarm
 * IRQ, and a default time is loaded (for now - to be removed later).  
 * NOTE: The RTC IRQ is not turned on here.  It will be turned on when in clock mode.
 * STEP 1: Init the RTC, disable IRQ and set RTC IRQ priority
 * STEP 2: Reset the RTC and enable RTC to work
 * STEP 3: Set the default time value
 * STEP 4: Configure RTC IRQ and turn it on
 **************************************************************************/
void init_Clock(void)
 {
 
 // STEP 1
 RTC_Init(LPC_RTC);
 ctl_mask_isr(RTC_IRQn);
 ctl_set_priority(RTC_IRQn, RTC_IRQ_PRIORITY); 
 
 // STEP 2
 RTC_ResetClockTickCounter(LPC_RTC);
 RTC_Cmd(LPC_RTC, ENABLE);
 RTC_CalibCounterCmd(LPC_RTC, ENABLE);
 
 // STEP 3
 // RESET time is 8:59:50PM, 11-26-2011
#ifdef SET_DEFAULT_TIME_ON_CLOCK_INIT
   RTC_SetTime (LPC_RTC, RTC_TIMETYPE_SECOND, DEFAULT_SECOND);
   RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MINUTE, DEFAULT_MINUTE);
   RTC_SetTime (LPC_RTC, RTC_TIMETYPE_HOUR, DEFAULT_HOUR);
   RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MONTH, DEFAULT_MONTH);
   RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH, DEFAULT_DOM);
   RTC_SetTime (LPC_RTC, RTC_TIMETYPE_YEAR, DEFAULT_YEAR);
#endif
   
 // STEP 4
 RTC_CntIncrIntConfig(LPC_RTC, RTC_TIMETYPE_SECOND, ENABLE);
 ctl_unmask_isr(RTC_IRQn);
 
 } // END OF init_Clock




/*************************************************************************
 * Function Name: call_SWAT_Reset
 * Parameters: void
 * Return: void
 *
 * Description: Resets the stop watch to 0 and stop.  This function can only be called
 * in Clock Mode.  Actual stopwatch is updated in clock task.  Note Timer1 IRQ is effected.
 * Timer1 IRQ feeds the clock task event for the stop watch.
 * NOTE: Called by a press of key 4 in Clock Mode
 * STEP 1: Check to make sure in cal mode.  If not do nothing.
 * STEP 2: Set SWAT to 0 and stopped and enable sleep
 * STEP 3: Display SWAT as 0
 **************************************************************************/
 void call_SWAT_Reset(void)
 {
 
 uint8_t LineText[DISPLAY_COLUMN_TOTAL];
 
 // STEP 1
 if (CalSettings.CalMode != CLOCK_MODE)
   return;
 
 // STEP 2
 SWAT.Status = STOP;
 CalSettings.SleepDisable = FALSE;
 SWAT.TenMiliSecTime = 0;
 SWAT.SecTime = 0;
 SWAT.MinuteTime = 0;
 SWAT.HourTime = 0;
 
 // STEP 3
 sprintf(LineText,"SWAT: 00:00:00:00"); 
 DIP204_txt_engine(LineText,SWAT_ROW,0,strlen(LineText)); 
 DIP204_ICON_set(ICON_TIME, ICON_ON);
 
 } // END OF call_SWAT_Reset




/*************************************************************************
 * Function Name: call_SWAT_Start
 * Parameters: void
 * Return: void
 *
 * Description: Starts the stop watch.  This function can only be called
 * in Clock Mode.  Actual stopwatch is updated in clock task.  Note Timer1 IRQ is effected.
 * Timer1 IRQ feeds the clock task event for the stop watch.
 * NOTE: Cal will not goto sleep when stop watch is running
 * NOTE: Called by a press of key 6 in Clock Mode
 * STEP 1: Check to make sure in clock mode.  If not do nothing.
 * STEP 2: Set to start and blink the icon and disable sleep
 **************************************************************************/
 void call_SWAT_Start(void)
 {
 
 // STEP 1
 if (CalSettings.CalMode != CLOCK_MODE)
   return;
 
 // STEP 2
 SWAT.Status = START;
 DIP204_ICON_set(ICON_TIME, ICON_BLINK);
 CalSettings.SleepDisable = TRUE;
 
 } // END OF call_SWAT_Start




/*************************************************************************
 * Function Name: call_SWAT_Stop
 * Parameters: void
 * Return: void
 *
 * Description: Stops / Pauses the stop watch.  This function can only be called
 * in Clock Mode.  Actual stopwatch is updated in clock task.  Note Timer1 IRQ is effected.
 * Timer1 IRQ feeds the clock task event for the stop watch.
 * NOTE: Called by a press of key 5 in Clock Mode
 * STEP 1: Check to make sure in clock mode.  If not do nothing.
 * STEP 2: Set to stop - enable sleep
 **************************************************************************/
 void call_SWAT_Stop(void)
 {
 
 // STEP 1
 if (CalSettings.CalMode != CLOCK_MODE)
   return;
 
 // STEP 2
 SWAT.Status = STOP;
 DIP204_ICON_set(ICON_TIME, ICON_ON);
 CalSettings.SleepDisable = FALSE;
 
 } // END OF call_SWAT_Start




/*************************************************************************
 * Function Name: call_SetClock
 * Parameters: void
 * Return: void
 *
 * Description: Sets the EnableClockSet var that allows the clock to be set if USB
 * is connected.  The clock set is not done here, but in the clock task
 * STEP 1: Set the var if USB connected
 **************************************************************************/
 void call_SetClock(void)
 {
 
 // STEP 1
 if (VCOM_Link & CalSettings.USB_Link)
    EnableClockSet = TRUE;
 
 } // END OF call_SetClock


/*************************************************************************
 * Function Name: call_ClockModeEnd
 * Parameters: void
 * Return: void
 *
 * Description: Performs the actions necessary to end Clock Mode.  Such that the next
 * mode can start itself.  This function sets CalMode to calculator mode as a default.  
 * It is up to the next mode function to set its mode of operation.
 * STEP 1: Stop the stop watch by setting .Status member.  Stop the clock updates by disabling the
 * RTC IRQ and deschedule the clock task
 * STEP 2: Select all Keys for the next function
 * STEP 3: Prep the display for use by the next Mode
 **************************************************************************/
 void call_ClockModeEnd(void)
 {
 
 // STEP 1
 // TURN OFF RTC 1s IRQ - STOP ANY FUTURE CLOCK EVENTS
 RTC_CntIncrIntConfig(LPC_RTC, RTC_TIMETYPE_SECOND, DISABLE);
 // WAIT FOR METER EVENT TO CLEAR - AS THIS WILL BE A STABLE TIME TO SWITCH
 while (CalEvents & (EVENT_CLOCK|EVENT_SWAT))
   {
   ctl_timeout_wait(ctl_get_current_time()+5);
   } 
 SWAT.Status = STOP;
 CalSettings.SleepDisable = FALSE;

 #if defined(REMOVE_RESTORE)
    ctl_task_remove(&clock_task);
  #elif defined(SUSPEND_RUN)
    ctl_HabTaskSuspend(&clock_task);
  #endif
 
 // STEP 2
 select_All_Normal_Keys();

 // STEP 3
 DIP204_ICON_set(ICON_TIME, ICON_OFF);
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 CalSettings.CalMode = CAL_MODE; // SET AS DEFAULT
  
 } // END OF call_ClockModeEnd




/*************************************************************************
 * Function Name: call_BuildDateTimeStrings
 * Parameters: uint8_t *, uint8_t *
 * Return: void
 *
 * Description: Builds the Date and Time Strings.  The time string is built as 
 * non-military time (AM/PM) 11:34:50 AM.  The Date is built as MONTH, DATE, YEAR 
 * = JUL 6, 2012
 * STEP 1: Build the Time String
 * STEP 2: Build the Date String
 **************************************************************************/
 static void call_BuildDateTimeStrings(uint8_t *DateString, uint8_t *TimeString)
 {
 
 BOOLEAN PM_Hour = FALSE;
 uint8_t Hour;
 uint8_t LineText[DISPLAY_COLUMN_TOTAL],
         MonthText[3];
 
 // STEP 1
 // BUILD TIME STRING
 Hour = RTC_GetTime(LPC_RTC,RTC_TIMETYPE_HOUR);
 if (Hour == 0)
   Hour = 12;
 if (Hour > 12)
   {
   PM_Hour = TRUE;
   Hour -= 12;
   }
 sprintf(LineText,"TIME: %02d:%02d:%02d", Hour, 
                                          RTC_GetTime(LPC_RTC,RTC_TIMETYPE_MINUTE), 
                                          RTC_GetTime(LPC_RTC,RTC_TIMETYPE_SECOND));
 if (PM_Hour)
   strcat(LineText, " PM");
 else
   strcat(LineText, " AM");
 // LOAD TIME STRING TO PASS PARAMETER                                      
 strcpy(TimeString, LineText);
 
 // STEP 2
 // BUILD DATE STRING
 // DETERMINE MONTH AS 3 LETTERS
 switch(RTC_GetTime(LPC_RTC,RTC_TIMETYPE_MONTH))
   {
   case JAN:
     strcpy(MonthText, "JAN");
   break;
   case FEB:
     strcpy(MonthText, "FEB");
   break;
   case MAR:
     strcpy(MonthText, "MAR");
   break;
   case APR:
     strcpy(MonthText, "APR");
   break;
   case MAY:
     strcpy(MonthText, "MAY");
   break;
   case JUN:
     strcpy(MonthText, "JUN");
   break;
   case JUL:
     strcpy(MonthText, "JUL");
   break;
   case AUG:
     strcpy(MonthText, "AUG");
   break;
   case SEP:
     strcpy(MonthText, "SEP");
   break;
    case OCT:
     strcpy(MonthText, "OCT");
   break;
   case NOV:
     strcpy(MonthText, "NOV");
   break;
   default:
   case DEC:
     strcpy(MonthText, "DEC");
   break;
   }
 // FORMAT DATE
 sprintf(LineText,"DATE: %s %d, %4d", MonthText, 
                                      RTC_GetTime(LPC_RTC,RTC_TIMETYPE_DAYOFMONTH), 
                                      RTC_GetTime(LPC_RTC,RTC_TIMETYPE_YEAR));
 // LOAD TIME STRING TO PASS PARAMETER                                      
 strcpy(DateString, LineText);
 
 } // END OF call_BuildDateTimeStrings




/*************************************************************************
 * Function Name: call_AlarmMode
 * Parameters:    void 
 * Return:        void
 *
 * Description: Places the calculator in Alarm Mode.  Alarm mode is similar to clock
 * mode, but the purpose of alarm mode is to set the alarm time.  
 * STEP 1: End the present mode.  Clear display, set clock icon and set key mask as 
 * appropriate for clock mode.
 * STEP 2: Display heading, Date String
 * STEP 3: Display Time and Date, time icon, and alarm icon if set
 * STEP 4: Display alarm time - Check for bogus time (clock init)
 *************************************************************************/
 void call_AlarmMode(void)
 {
 
 uint8_t DateString[DISPLAY_COLUMN_TOTAL],
         LineText[DISPLAY_COLUMN_TOTAL];

  
 // STEP 1
 call_EndPresentMode();
 CalSettings.Mask_KeyTouchA = CalSettings.Mask_KeyTouchB = NO_KEYS_SELECTED;
 CalSettings.Mask_KeyTouchA = (MASK_ATN_KEY | MASK_KEY_LSHIFT);
 CalSettings.Mask_KeyTouchB = (MASK_KEY_2| MASK_KEY_4 | MASK_KEY_5 | MASK_KEY_6 | MASK_KEY_8 | MASK_KEY_CAL | MASK_KEY_ENTER |
                               MASK_KEY_METER |
                               MASK_KEY_FLASH |
                               MASK_KEY_MUSIC );
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 
 // STEP 2
 // BUILD TIME AND DATE STRINGSX
 sprintf(LineText,"SET ALARM:");
 DIP204_txt_engine(LineText,HEADING_ROW,0,strlen(LineText)); 
 // DISPLAY DATE AND ICON                              
 call_BuildDateTimeStrings(DateString, LineText); // NOTE TIME NOT USED
 DIP204_txt_engine(DateString,DATE_ROW,0,strlen(DateString));   
 DIP204_ICON_set(ICON_TIME, ICON_ON);
 
 // STEP 3
 // CHECK FOR HOUR, MIN, SEC
 if (LPC_RTC->AMR & 0x06)
   DIP204_ICON_set(ICON_TIME, ICON_ON);
 else
   DIP204_ICON_set(ICON_TIME, ICON_OFF);
 
 // STEP 4
 /* SET CAL MODE LAST SO THAT YOU CAN RESTORE CLOCK TASK ON FIRST TIME IN THIS FUNCTION AND WILL
  * NOT RESTORE ON SUBSEQUENT CALLS TO THIS FUNCTION IF ALREADY IN CLOCK MODE
  * NOTE: THE STOP WATCH MUST BE DISPLAYED FROM HERE AS WELL FOR THE SIMILAR REASONS
  * STOP WATCH CAN BE UPDATED FROM THE CLOCK TASKS AS WELL. */
 if (CalSettings.CalMode != ALARM_MODE)
   {
    #if defined(REMOVE_RESTORE)
      ctl_task_restore(&clock_task);
    #elif defined(SUSPEND_RUN)
      ctl_HabTaskRun(&clock_task);
    #endif 
   CalSettings.CalMode = ALARM_MODE;
   }
 // CONVERT THE TIME TO AM AND PM
 // IF NEVER SET BEFORE VALUE COULD BE BOGUS
 AlarmSetTime.Hour = (uint8_t)RTC_GetAlarmTime(LPC_RTC, RTC_TIMETYPE_HOUR);
 AlarmSetTime.Minute = (uint8_t)RTC_GetAlarmTime(LPC_RTC, RTC_TIMETYPE_MINUTE);
 // CONVERT ANY BOGUS TIME
 if (AlarmSetTime.Minute > 59)
   AlarmSetTime.Minute = 0;
 if (AlarmSetTime.Hour > 24)
   AlarmSetTime.Hour = 24;
 // CONVERT TO AM PM TIME
 if (AlarmSetTime.Hour == 0)
   AlarmSetTime.Hour = 12;
 // ADJUST MIL TIME TO AM PM TIME
 if (AlarmSetTime.Hour > 12) 
   {
   AlarmSetTime.AM_Time = FALSE;
   sprintf(LineText,"PM"); 
   AlarmSetTime.Hour -= 12;
   }
 else
   {
   AlarmSetTime.AM_Time = TRUE;
   sprintf(LineText,"AM");
   }

 DIP204_txt_engine(LineText,ALARM_TIME_ROW,ALARM_AM_POSITION,strlen(LineText));
 sprintf(LineText,"ALARM: %02d:%02d", AlarmSetTime.Hour, AlarmSetTime.Minute); 
 DIP204_txt_engine(LineText,ALARM_TIME_ROW,0,strlen(LineText));
 DIP204_cursorToXY(ALARM_TIME_ROW, ALARM_HR_POSITION); 
 DIP204_set_cursor(CURSOR_BLINK);
 CursorSet = TIME_HOUR;
 
 } // END OF call_AlarmMode




/*************************************************************************
 * Function Name: call_AlarmModeEnd
 * Parameters: void
 * Return: void
 *
 * Description: Performs the actions necessary to end Alarm Mode.  Such that the next
 * mode can start itself.  This function sets CalMode to calculator mode as a default.  
 * It is up to the next mode function to set its mode of operation.
 * STEP 1: Stop the clock updates by disabling the RTC IRQ and deschedule the clock task
 * STEP 2: Select all Keys for the next function
 * STEP 3: Prep the display for use by the next Mode
 **************************************************************************/
 void call_AlarmModeEnd(void)
 {
 
 // STEP 1
 // WAIT FOR METER EVENT TO CLEAR - AS THIS WILL BE A STABLE TIME TO SWITCH
 do
   {
   ctl_timeout_wait(ctl_get_current_time()+10);
   } while (CalEvents & (EVENT_CLOCK|EVENT_SWAT));
 //ctl_mask_isr(RTC_IRQn);
 #if defined(REMOVE_RESTORE)
    ctl_task_remove(&clock_task);
  #elif defined(SUSPEND_RUN)
    ctl_HabTaskSuspend(&clock_task);
  #endif
 
 // STEP 2
 select_All_Normal_Keys();

 // STEP 3
 DIP204_ICON_set(ICON_TIME, ICON_OFF);
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 CalSettings.CalMode = CAL_MODE;
  
 } // END OF call_AlarmModeEnd




/*************************************************************************
 * Function Name: call_IncrementAlarmTime
 * Parameters: void
 * Return: void
 *
 * Description: Increments the Alarm hour - AM / PM TIME or minute
 * NOTE: Called by a press of key 8 in Alarm Mode
 * STEP 1: Increment time parameter
 **************************************************************************/
 void call_IncrementAlarmTime()
 {
 
 uint8_t LineText[DISPLAY_COLUMN_TOTAL];
 
  // STEP 1
 switch(CursorSet)
   {
   case TIME_HOUR:
   // INCREMENT THE HOUR
   AlarmSetTime.Hour++;
   if (AlarmSetTime.Hour > 12)
     AlarmSetTime.Hour = 1;
   sprintf(LineText,"%02d", AlarmSetTime.Hour);
   DIP204_txt_engine(LineText,ALARM_TIME_ROW,ALARM_HR_POSITION,strlen(LineText));
   DIP204_cursorToXY(ALARM_TIME_ROW, ALARM_HR_POSITION);
   break;
   
   case TIME_MINUTE:
   AlarmSetTime.Minute++;
   if (AlarmSetTime.Minute > 59)
     AlarmSetTime.Minute = 0;
   sprintf(LineText,"%02d", AlarmSetTime.Minute);
   DIP204_txt_engine(LineText,ALARM_TIME_ROW,ALARM_MIN_POSITION,strlen(LineText));
   DIP204_cursorToXY(ALARM_TIME_ROW, ALARM_MIN_POSITION);
   break;
   
   case TIME_AM_PM:
   // TIME IS AM MAKE PM
   if (AlarmSetTime.AM_Time)
     {
     AlarmSetTime.AM_Time = FALSE;
     sprintf(LineText,"PM");
     DIP204_txt_engine(LineText,ALARM_TIME_ROW,ALARM_AM_POSITION,strlen(LineText));
     }
   else
     {
     // TIME IS PM MAKE AM
     AlarmSetTime.AM_Time = TRUE;
     sprintf(LineText,"AM");
     DIP204_txt_engine(LineText,ALARM_TIME_ROW,ALARM_AM_POSITION,strlen(LineText));
     }
   DIP204_cursorToXY(ALARM_TIME_ROW, ALARM_AM_POSITION);
   default:
   break;
   }
 // CURSOR MAY HAVE BEEN TURNED OFF
 DIP204_set_cursor(CURSOR_BLINK);
 
 } // END OF call_IncrementAlarmTime




/*************************************************************************
 * Function Name: call_DecrementAlarmTime
 * Parameters: void
 * Return: void
 *
 * Description: Decrements the Alarm hour - AM / PM TIME or minute
 * NOTE: Called by a press of key 2 in Alarm Mode
 * STEP 1: Decrement time parameter
 **************************************************************************/
 void call_DecrementAlarmTime()
 {
 
 uint8_t LineText[DISPLAY_COLUMN_TOTAL];
 
  // STEP 1
 switch(CursorSet)
   {
   case TIME_HOUR:
   // INCREMENT THE HOUR
   AlarmSetTime.Hour--;
   if (AlarmSetTime.Hour < 1)
     AlarmSetTime.Hour = 12;
   sprintf(LineText,"%02d", AlarmSetTime.Hour);
   DIP204_txt_engine(LineText,ALARM_TIME_ROW,ALARM_HR_POSITION,strlen(LineText));
   DIP204_cursorToXY(ALARM_TIME_ROW, ALARM_HR_POSITION);
   break;
   
   case TIME_MINUTE:
   AlarmSetTime.Minute--;
   if (AlarmSetTime.Minute < 0)
     AlarmSetTime.Minute = 59;
   sprintf(LineText,"%02d", AlarmSetTime.Minute);
   DIP204_txt_engine(LineText,ALARM_TIME_ROW,ALARM_MIN_POSITION,strlen(LineText));
   DIP204_cursorToXY(ALARM_TIME_ROW, ALARM_MIN_POSITION);
   break;
   
   case TIME_AM_PM:
   // TIME IS AM MAKE PM
   if (AlarmSetTime.AM_Time)
     {
     AlarmSetTime.AM_Time = FALSE;
     sprintf(LineText,"PM");
     DIP204_txt_engine(LineText,ALARM_TIME_ROW,ALARM_AM_POSITION,strlen(LineText));
     }
   else
     {
     // TIME IS PM MAKE AM
     AlarmSetTime.AM_Time = TRUE;
     sprintf(LineText,"AM");
     DIP204_txt_engine(LineText,ALARM_TIME_ROW,ALARM_AM_POSITION,strlen(LineText));
     }
   DIP204_cursorToXY(ALARM_TIME_ROW, ALARM_AM_POSITION);
   default:
   break;
   }
 // CURSOR MAY HAVE BEEN TURNED OFF
 DIP204_set_cursor(CURSOR_BLINK);
 } // END OF call_DecrementAlarmTime




/*************************************************************************
 * Function Name: call_MoveSetCursorRight
 * Parameters: void
 * Return: void
 *
 * Description: Moves the cursor set position Hr or Min right
 * NOTE: Called by a press of key 6 in Alarm Mode
 * STEP 1: Perform action
 **************************************************************************/
 void call_MoveSetCursorRight(void)
 {
 
 // STEP 1
 CursorSet++;
 if (CursorSet > TIME_AM_PM)
   {
   CursorSet = TIME_HOUR;
   }
 switch(CursorSet)
   {
   case TIME_HOUR:
   DIP204_cursorToXY(ALARM_TIME_ROW, ALARM_HR_POSITION);
   break;
   
   case TIME_MINUTE:
   DIP204_cursorToXY(ALARM_TIME_ROW, ALARM_MIN_POSITION);
   break;
   
   case TIME_AM_PM:
   DIP204_cursorToXY(ALARM_TIME_ROW, ALARM_AM_POSITION);
   break;
   
   default:
   break;
   }
 // CURSOR MAY HAVE BEEN TURNED OFF
 DIP204_set_cursor(CURSOR_BLINK);
 
 } // END OF call_MoveSetCursorRight




/*************************************************************************
 * Function Name: call_MoveSetCursorLeft
 * Parameters: void
 * Return: void
 *
 * Description: Moves the cursor set position Hr or Min left
 * NOTE: Called by a press of key 4 in Alarm Mode
 * STEP 1: Perform action
 **************************************************************************/
 void call_MoveSetCursorLeft(void)
 {
 
 // STEP 1
 CursorSet--;
 if (CursorSet < TIME_HOUR)
   {
   CursorSet = TIME_AM_PM;
   }
 switch(CursorSet)
   {
   case TIME_HOUR:
   DIP204_cursorToXY(ALARM_TIME_ROW, ALARM_HR_POSITION);
   break;
   
   case TIME_MINUTE:
   DIP204_cursorToXY(ALARM_TIME_ROW, ALARM_MIN_POSITION);
   break;
   
   case TIME_AM_PM:
   DIP204_cursorToXY(ALARM_TIME_ROW, ALARM_AM_POSITION);
   break;
   
   default:
   break;
   }
 // CURSOR MAY HAVE BEEN TURNED OFF
 DIP204_set_cursor(CURSOR_BLINK);
 
 } // END OF call_MoveSetCursorLeft




/*************************************************************************
 * Function Name: call_TurnOnAlarm
 * Parameters: void
 * Return: void
 *
 * Description: Sets the alarm at the given time
 * NOTE: Called by a press of ENTER key in Alarm Mode
 * STEP 1: Set the alarm - Clear all alarm time types. Set hour and minute
 * STEP 2: Show alarm icon and turn off cursor
 * STEP 3: Enable alarm time IRQ - IRQ on minute check
 **************************************************************************/
 void call_TurnOnAlarm(void)
 {
 
 // STEP 1
 // SET ALARM
 LPC_RTC->AMR = 0xFF;
 RTC_SetAlarmTime (LPC_RTC, RTC_TIMETYPE_MINUTE, AlarmSetTime.Minute);
 if (AlarmSetTime.AM_Time)
   RTC_SetAlarmTime (LPC_RTC, RTC_TIMETYPE_HOUR, AlarmSetTime.Hour);
 else
   RTC_SetAlarmTime (LPC_RTC, RTC_TIMETYPE_HOUR, (AlarmSetTime.Hour + 12));
 
 // STEP 2
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_ICON_set(ICON_RING, ICON_ON);
 
 // STEP 3
 RTC_AlarmIntConfig(LPC_RTC, RTC_TIMETYPE_MINUTE, ENABLE);
 CalSettings.TimeAlarm.AlarmEnable = TRUE;

 } // END OF call_TurnOnAlarm




/*************************************************************************
 * Function Name: call_TurnOffAlarm
 * Parameters: void
 * Return: void
 *
 * Description: Turns off alarm
 * NOTE: Called by a press of key 5 
 * NOTE: Can be called by functions when NOT in alarm mode
 * STEP 1: Disable alarm time IRQ
 * STEP 2: Turnoff alarm icon and turn off cursor ONLY if in alarm mode
 **************************************************************************/
 void call_TurnOffAlarm(void)
 {
 
 // STEP 1
 RTC_AlarmIntConfig(LPC_RTC, RTC_TIMETYPE_SECOND, DISABLE);
 CalSettings.TimeAlarm.AlarmEnable = FALSE;
 
 // STEP 2
 if (CalSettings.CalMode == ALARM_MODE)
   DIP204_set_cursor(CURSOR_BLINK);
 DIP204_ICON_set(ICON_RING, ICON_OFF);
 
 } // END OF ALARM OFF




/*************************************************************************
 * Function Name: call_ShowAlarm
 * Parameters: void
 * Return: void
 *
 * Description: Shows the user the alarm event has arrived and enable only the 
 * stop key (5) and ATN key as user inputs
 * STEP 1: Enable only stop and ATN key
 * STEP 2: Clear the display and display alarm event message.  Turn back light on
 **************************************************************************/
 static void call_ShowAlarm(void)
 {
 
 uint8_t LineText[DISPLAY_COLUMN_TOTAL];
  
 // STEP 1
 CalSettings.Mask_KeyTouchA = CalSettings.Mask_KeyTouchB = NO_KEYS_SELECTED;
 CalSettings.Mask_KeyTouchA = MASK_ATN_KEY;
 CalSettings.Mask_KeyTouchB = MASK_KEY_5;
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 
 // STEP 2
 // BUILD HEADING 
 sprintf(LineText,"  **ALARM EVENT**");
 DIP204_txt_engine(LineText,HEADING_ROW,0,strlen(LineText));
 sprintf(LineText,"    PRESS STOP");
 DIP204_txt_engine(LineText,CLOCK_SET_ROW,0,strlen(LineText));
 ctl_events_set_clear(&CalEvents, EVENT_BACK_L, 0);
 
 } // END OF call_ShowAlarm
 




                                      