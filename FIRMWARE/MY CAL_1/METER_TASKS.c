/*****************************************************************
 *
 * File name:       METER_TASKS.C
 * Description:     CTL RTOS Functions used to support Meter (voltage read and continuity)
                    related functions of the HC15C
 * Author:          Hab S. Collector
 * Date:            11/27/11
 * LAST EDIT:       7/16/2012 
 * Hardware:        NXP LPC1769
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 

#include "METER_TASKS.H"
#include "DIP204.H"
#include "ADC_HC15C.H"
#include "LED_HC15C.H"
#include "PWM_HC15C.H"
#include "CORE_FUNCTIONS.H"
#include "TOUCH_TASKS.H"
#include "HC15C_PROTOCOL.H"
#include "USB_LINK.H"
#include "lpc17xx_gpio.h"
#include <ctl_api.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// GLOBALS
Type_Meter Meter;
Type_OHMS OHMS;
BOOLEAN ContinunityTone = FALSE;


// EXTERNS
extern CTL_EVENT_SET_t CalEvents;
extern CTL_TASK_t meter_task;
extern Type_CalSettings CalSettings;
extern BOOLEAN VCOM_Link;


/*************************************************************************
 * Function Name: meter_taskFn
 * Parameters:    void *
 * Return:        void
 *
 * Description: RTOS CTL task to manage the meter tasks.  The meter tasks are
 * an update of the present voltage (when in Meter Mode) and an update of the 
 * continuity OHMs when in OHMS mode.  The Volt Meter mode is self ranging across 3
 * ranges 0-10, 10-20 and 20-30.  Volt Meter mode displays the present measure and the max
 * measure.  The Max measure can be reset by pressing the back button.
 * OHMs mode determines if the measured resistance is below the stated (displayed) limit.
 * If so, a sound is made.  The display reflects short or open and the present measure.
 * Due to the way resistance is measured a very small change in the ADC measure (about 1%)
 * results in a very large change in resistance measured.  Because of this the measured
 * resistance is compared against the last resistance to be within a given tolerance to be
 * acceptable.  
 * STEP 1: Volt Meter Event {Steps 2...4}
 * STEP 2: Make ADC measure and display
 * STEP 3: Update the Max V measure
 * STEP 4: Update the auto range based on the input voltage and take action if necessary
 * to setup ADC ratio and hardware divider
 * STEP 5: OHMS Meter Event {Steps 6..8}
 * STEP 6: Calculate the Measured resistance from the ADC read
 * STEP 7: Only accept that reading to continue if the reading is less than X% of the last reading.
 * if not get another measure.
 * STEP 8: Determine if the measured resistance is greater than the set limit resistance.  If so,
 * beep, blink and display continuity.  If not, show as open.
 * STEP 9: Check for UI increment down of ohms set.  Decrement the set point
 * accordingly and update the display with the new set point value
 * STEP 10: Check for UI increment up of ohms set.  Increment the set point
 * accordingly and update the display with the new set point value
 *************************************************************************/
 void meter_taskFn(void *p)
 {
 
 uint8_t LineText[DISPLAY_COLUMN_TOTAL];
 static double LastResistance = 0.0;
 double MeasuredVoltage, 
        MeasuredResistance,
        I1, I2, Vb;
 
 while (1)
   {
   ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS, &CalEvents, (EVENT_METER|EVENT_OHMS|EVENT_OHMS_UI), CTL_TIMEOUT_NONE, 0);
  
   // STEP 1
   // VOLTMETER MODE
   if (CalEvents & EVENT_METER)
     {
     // STEP 2
     MeasuredVoltage = ADC_getConvertedValue(Meter.ADC_HC15C);
     DIP204_txt_engine(CLEAR_READING, METER_ROW, METER_POSITION, strlen(CLEAR_READING));
     // WRITE TO DISPLAY
     sprintf(LineText,"%2.3fV", MeasuredVoltage);
     DIP204_txt_engine(LineText, METER_ROW, METER_POSITION, strlen(LineText));
     // IF VCOM LINK WRITE OHM READING TO USB PORT
     if (VCOM_Link & CalSettings.USB_Link)
       {
       call_WriteToUSB_Meter(MeasuredVoltage, TYPE_VOLT);
       }  
     // STEP 3
     if (MeasuredVoltage > Meter.MaxV)
       {
       Meter.MaxV = MeasuredVoltage;
       DIP204_txt_engine(CLEAR_READING, MAX_ROW, MAX_POSITION, strlen(CLEAR_READING));
       sprintf(LineText,"%2.3fV", Meter.MaxV);
       DIP204_txt_engine(LineText, MAX_ROW, MAX_POSITION, strlen(LineText));
       }
     // STEP3
     // CHECK FOR 10V RANGE
     if (MeasuredVoltage <= RANGE_10V_TH)
       {
       if (Meter.LastRange != RANGE_10V_TH)
         {
         GPIO_SetValue(PORT1, (RANGE_10V|SEL_V_C));
         GPIO_ClearValue(PORT1, (RANGE_20V|RANGE_30V));
         Meter.ADC_HC15C.ADC_FrontEndDivider = ADC_10V_DIVIDER;
         Meter.LastRange = RANGE_10V_TH;
         DIP204_ICON_set(ICON_ALERT, ICON_OFF);
         DIP204_txt_engine(CLEAR_READING, RANGE_ROW, RANGE_POSITION, strlen(CLEAR_READING));
         DIP204_txt_engine(TXT_RNG_00_10, RANGE_ROW, RANGE_POSITION, strlen(TXT_RNG_00_10));
         }
       }
     // CHECK FOR 20V RANGE
     if ((MeasuredVoltage > RANGE_10V_TH) && (MeasuredVoltage < RANGE_20V_TH))
       {
       if (Meter.LastRange != RANGE_20V_TH)
         {
         GPIO_SetValue(PORT1, (RANGE_20V|SEL_V_C));
         GPIO_ClearValue(PORT1, (RANGE_10V|RANGE_30V));
         Meter.ADC_HC15C.ADC_FrontEndDivider = ADC_20V_DIVIDER;
         Meter.LastRange = RANGE_20V_TH;
         DIP204_ICON_set(ICON_ALERT, ICON_OFF);
         DIP204_txt_engine(CLEAR_READING, RANGE_ROW, RANGE_POSITION, strlen(CLEAR_READING));
         DIP204_txt_engine(TXT_RNG_10_20, RANGE_ROW, RANGE_POSITION, strlen(TXT_RNG_10_20));
         }
       }
     // CHECK FOR 30V RANGE
     if (MeasuredVoltage >= RANGE_20V_TH)
       {
       if (Meter.LastRange != RANGE_30V_TH)
         {
         GPIO_SetValue(PORT1, (RANGE_30V|SEL_V_C));
         GPIO_ClearValue(PORT1, (RANGE_10V|RANGE_20V));
         Meter.ADC_HC15C.ADC_FrontEndDivider = ADC_30V_DIVIDER;
         Meter.LastRange = RANGE_30V_TH;
         DIP204_ICON_set(ICON_ALERT, ICON_BLINK);
         DIP204_txt_engine(CLEAR_READING, RANGE_ROW, RANGE_POSITION, strlen(CLEAR_READING));
         DIP204_txt_engine(TXT_RNG_20_30, RANGE_ROW, RANGE_POSITION, strlen(TXT_RNG_20_30));
         }
       }
     // CLEAR THE VOLT METER EVENT    
     ctl_events_set_clear(&CalEvents, 0, EVENT_METER);
     } // END OF METER EVENT
   
   /* STEP 5
    * OHM METER MODE MODE */
   if (CalEvents & EVENT_OHMS)
     {
     // STEP 6
     MeasuredResistance = ADC_getConvertedValue(OHMS.ADC_HC15C);
     /* CALCULATE THE RESISTANCE BEING MEASURE WHICH WILL BE STORED IN Measured Value
      * FROM NOTE ON 11/30/11 */
     I2 = MeasuredResistance / RD;
     Vb = I2 * (RC + RD);
     I1 = (Vb + (RA * I2)) / RA;
     // THIS IS MEASURED RESISTANCE 
     MeasuredResistance = (ADC_REFERENCE - Vb) / I1;
     // STEP 7
     if ( (100.0*fabs(MeasuredResistance-LastResistance)/MeasuredResistance) > READ_TOL_ERROR)
       {
       LastResistance = MeasuredResistance;
       }
     else
       {
       // STEP 8
       DIP204_txt_engine(CLEAR_READING, RX_ROW, RX_POSITION, strlen(CLEAR_READING));
       if (MeasuredResistance > MAX_OHM_READ)
         {
         DIP204_txt_engine(TXT_OPEN_MEASURE, RX_ROW, RX_POSITION, strlen(TXT_OPEN_MEASURE));
         DIP204_txt_engine(TXT_OPEN, STATUS_ROW, STATUS_POSITION, strlen(TXT_OPEN));
         // IF VCOM LINK WRITE OHM READING TO USB PORT
         if (VCOM_Link & CalSettings.USB_Link)
           {
           call_WriteToUSB_Meter(OPEN_CIRCUIT_READING, TYPE_OHMS);
           }
         }
       else
         {
         // DISPLAY THE OHMS READING
         sprintf(LineText,"%d", (uint16_t)MeasuredResistance);
         DIP204_txt_engine(LineText, RX_ROW, RX_POSITION, strlen(LineText));
         // IF VCOM LINK WRITE OHM READING TO USB PORT
         if (VCOM_Link & CalSettings.USB_Link)
           {
           call_WriteToUSB_Meter(MeasuredResistance, TYPE_OHMS);
           }
          if (MeasuredResistance < OHMS.Limit)
            {
            DIP204_txt_engine(TXT_SHORT, STATUS_ROW, STATUS_POSITION, strlen(TXT_SHORT));
            ContinunityTone = TRUE;
            }
          else
            {
            DIP204_txt_engine(TXT_OPEN, STATUS_ROW, STATUS_POSITION, strlen(TXT_OPEN));
            ContinunityTone = FALSE;
            }
         }
       }
     ctl_events_set_clear(&CalEvents, 0, EVENT_OHMS);
     } // END OF EVENT_OHMS
   
   // OHMS UI EVENT
   // STEP 9
   if (CalEvents & EVENT_OHMS_UI)
     {
     if (OHMS.UI == INCREMENT_DOWN)
       {
       OHMS.Limit -= LIMIT_INCREMENT;
       if (OHMS.Limit <= MIN_OHM_LIMIT)
         {
         OHMS.Limit = MIN_OHM_LIMIT;
         }
       }
     // STEP 10
     if (OHMS.UI == INCREMENT_UP)
       {
       OHMS.Limit += LIMIT_INCREMENT;
       if (OHMS.Limit > MAX_OHM_LIMIT)
         {
         OHMS.Limit = MAX_OHM_LIMIT;
         }
       }
     // STEP 10
     DIP204_txt_engine(CLEAR_READING, LIMIT_ROW, LIMIT_POSITION, strlen(CLEAR_READING));
     sprintf(LineText, "%d OHMS", (uint16_t)OHMS.Limit);
     DIP204_txt_engine(LineText, LIMIT_ROW, LIMIT_POSITION, strlen(LineText));
       
     ctl_events_set_clear(&CalEvents, 0, EVENT_OHMS_UI);
     } // END OF EVENT_OHMS_UI
   }// END OF WHILE     
   
 } // END OF meter_taskFn
 
 
 
 
 /*************************************************************************
 * Function Name: call_VoltMeterMode
 * Parameters:    void
 * Return:        void
 *
 * Description: Places the calculator in Volt Meter mode.  Volt Meter Mode continuously displays the analog
 * voltage of the METER input.  Meter mode is self ranging and works with a timer1 IRQ and the meter task
 * to produce the desired effect.  The timer is used as the acquire interval.
 * Meter Mode is one of the fundamental modes of operation.  Meter mode is run from a task
 * which is (restored) by this function call.  Note the meter is not updated in this function.
 * It is updated in the meter task (event set by timer1 irq).
 * STEP 1: End the present mode, set up keys to use
 * STEP 2: Ready the display
 * STEP 3: Set meter mode vars and parameters
 *************************************************************************/
 void call_VoltMeterMode(void)
 {
 
 uint8_t LineText[DISPLAY_COLUMN_TOTAL];
 // STEP 1
 call_EndPresentMode();
 CalSettings.Mask_KeyTouchA = CalSettings.Mask_KeyTouchB = NO_KEYS_SELECTED;
 CalSettings.Mask_KeyTouchA = (MASK_ATN_KEY | MASK_KEY_LSHIFT);
 CalSettings.Mask_KeyTouchB = (MASK_VMAX_RST | MASK_KEY_CAL | 
                                               MASK_KEY_METER |
                                               MASK_KEY_ALINK |
                                               MASK_KEY_FLASH |
                                               MASK_KEY_MUSIC );

 // STEP 2
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 strcpy(LineText, "VMAX: 0.000");
 DIP204_txt_engine(LineText,MAX_ROW,0,strlen(LineText));
 strcpy(LineText, TXT_RNG_00_10);
 DIP204_txt_engine(LineText,RANGE_ROW,0,strlen(LineText));
 
 // STEP 3
 /* SET METER MODE LAST SO THAT YOU CAN RESTORE METER TASK ON FIRST TIME IN THIS FUNCTION AND WILL
  * NOT RESTORE ON SUBSEQUENT CALLS TO THIS FUNCTION IF ALREADY IN METER MODE.
  * NOTE: THE  */
 if (CalSettings.CalMode != METER_MODE)
   {
    #if defined(REMOVE_RESTORE)
      ctl_task_restore(&meter_task);
    #elif defined(SUSPEND_RUN)
      ctl_HabTaskRun(&meter_task);
    #endif    
   // SETUP THE ADC STRUCT VALUES TO DEFAULT 
   Meter.ADC_HC15C.ADC_AvgWeight = METER_SAMPLE_AVG;
   Meter.ADC_HC15C.ADC_msTimeBetweenSamples = METER_SAMPLE_AVG_TIME;
   Meter.ADC_HC15C.ADC_Type = ADC_MET_VOLTAGE;
   Meter.ADC_HC15C.ADC_FrontEndDivider = ADC_30V_DIVIDER;
   // SET THE METER TO THE HIGHEST RANGE (SAFEEST MODE) - MAKE BEFORE BREAK
   GPIO_SetValue(PORT1, (RANGE_30V|SEL_V_C));
   GPIO_ClearValue(PORT1, (RANGE_10V|RANGE_20V));
   // SETUP THE METER STRUCT, AND DISPLAY
   Meter.MaxV = 0.0;
   Meter.LastRange = 0.0;
   Meter.Status = START_MEASURE;
   DIP204_ICON_set(ICON_INFO, ICON_ON);
   CalSettings.CalMode = METER_MODE;
   }  
   
 } // END OF call_VoltMeterMode
 
 
 
 
 /*************************************************************************
 * Function Name: call_VoltMeterMaxReset
 * Parameters: void
 * Return: void
 *
 * Description: Resets the Max Voltage to 0.  This function can only be called
 * in Meter mode.  Note Timer1 IRQ is effected.  Timer1 IRQ feeds the meter event.
 * STEP 1: Check to make sure in Meter mode.  If not do nothing.
 * STEP 2: Reset max V
 **************************************************************************/
 void call_VoltMeterMaxReset(void)
 {
 
 // STEP 1
 if (CalSettings.CalMode != METER_MODE)
   return;
 
 // STEP 2
 Meter.MaxV = 0.0;
 
 } // END OF call_VoltMeterMaxReset.




/*************************************************************************
 * Function Name: call_VoltMeterModeEnd
 * Parameters: void
 * Return: void
 *
 * Description: Performs the actions necessary to end Meter Mode.  Such that the next
 * mode can start itself.  This function sets CalMode to calculator mode as a default.  
 * It is up to the next mode function to set its mode of operation.
 * STEP 1: End the meter task
 * STEP 2: Select all Keys for the next function
 * STEP 3: Prep the display for use by the next Mode
 **************************************************************************/
 void call_VoltMeterModeEnd(void)
 {
 
 // STEP 1
 // STOP ANY FUTURE VOLT METER EVENTS
 Meter.Status = STOP_MEASURE;
 // WAIT FOR METER EVENT TO CLEAR - AS THIS WILL BE A STABLE TIME TO SWITCH
 while (CalEvents & EVENT_METER)
   {
   ctl_timeout_wait(ctl_get_current_time()+1);
   } 
 #if defined(REMOVE_RESTORE)
    ctl_task_remove(&meter_task);
 #elif defined(SUSPEND_RUN)
    ctl_HabTaskSuspend(&meter_task);
 #endif
 
 // STEP 2
 select_All_Normal_Keys();

 // STEP 3
 DIP204_ICON_set(ICON_INFO, ICON_OFF);
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 CalSettings.CalMode = CAL_MODE; // AS A DEFAULT
  
 } // END OF call_VoltMeterModeEnd




/*************************************************************************
 * Function Name: call_OHMS_Mode
 * Parameters: void
 * Return: void
 *
 * Description: Places the calculator in OHMS mode.  OHMS Mode continuously displays the resistance
 * as measured at the METER input terminal.  OMHS mode works with timer1 IRQ and the meter task
 * to produce the desired effect.
 * OHMs Mode is one of the fundamental modes of operation.  OHMS mode is run from a task
 * which is restored by this function call.  Note the meter is not updated in this function.
 * It is updated in the meter task (event set by timer1 irq).
 * STEP 1: End the present mode, set up keys to use
 * STEP 2: Ready the display for use
 * STEP 3: Set the ohms mode vars and parameters
 **************************************************************************/
 void call_OhmsMeterMode(void)
 {
 
 uint8_t LineText[DISPLAY_COLUMN_TOTAL];
 
 // STEP 1
 call_EndPresentMode();
 CalSettings.Mask_KeyTouchA = CalSettings.Mask_KeyTouchB = NO_KEYS_SELECTED;
 CalSettings.Mask_KeyTouchA = (MASK_ATN_KEY | MASK_KEY_LSHIFT);
 CalSettings.Mask_KeyTouchB = (MASK_KEY_UP | MASK_KEY_DOWN |  MASK_KEY_CAL   | 
                                                              MASK_KEY_METER |
                                                              MASK_KEY_ALINK |
                                                              MASK_KEY_FLASH |
                                                              MASK_KEY_MUSIC );

 // STEP 2
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 strcpy(LineText, "SET LIMIT: 25 OHMS");
 DIP204_txt_engine(LineText,LIMIT_ROW,0,strlen(LineText));
 strcpy(LineText, "RX: XX OMHS");
 DIP204_txt_engine(LineText,RX_ROW,0,strlen(LineText));
 DIP204_txt_engine(TXT_OPEN,STATUS_ROW,STATUS_POSITION,strlen(TXT_OPEN));
 
 // STEP 3
 /* SET METER MODE LAST SO THAT YOU CAN RESTORE METER TASK ON FIRST TIME IN THIS FUNCTION AND WILL
  * NOT RESTORE ON SUBSEQUENT CALLS TO THIS FUNCTION IF ALREADY IN METER MODE.
  * NOTE: THE  */
 if (CalSettings.CalMode != OHMS_MODE)
   {
    #if defined(REMOVE_RESTORE)
      ctl_task_restore(&meter_task);
    #elif defined(SUSPEND_RUN)
      ctl_HabTaskRun(&meter_task);
    #endif 
   // SETUP THE ADC STRUCT VALUES
   OHMS.ADC_HC15C.ADC_AvgWeight = OHMS_SAMPLE_AVG;
   OHMS.ADC_HC15C.ADC_msTimeBetweenSamples = OHMS_SAMPLE_AVG_TIME;
   OHMS.ADC_HC15C.ADC_Type = ADC_MET_VOLTAGE;
   OHMS.ADC_HC15C.ADC_FrontEndDivider = ADC_OHMS_DIVIDER;
   // SET THE METER TO MEASURE OHMS - MAKE BEFORE BREAK
   GPIO_ClearValue(PORT1, (RANGE_10V|RANGE_20V|SEL_V_C));
   GPIO_SetValue(PORT1, RANGE_30V);
   // SETUP THE OHMS STRUCT AND DISPLAY
   OHMS.Status = START_MEASURE;
   OHMS.Limit = LIMIT_INCREMENT;
   // SHOW CAL DISPLAY
   CalSettings.CalMode = OHMS_MODE;
   }
   
 } // END OF call_OhmsMeterMode
 
 
 
 
/*************************************************************************
 * Function Name: call_LimitUp
 * Parameters: void
 * Return: void
 *
 * Description: Increments the limit up by the limit increment amount until the max
 * limit is reached, then increment up no more.  This function is performed in the
 * meter task - The variable is and event are set here.
 * STEP 1: Setup for event
 **************************************************************************/
void call_OhmsLimitUp(void)
{
 
 OHMS.UI = INCREMENT_UP;
 ctl_events_set_clear(&CalEvents, EVENT_OHMS_UI, 0);

} // END OF call_OHMS_LimitUp




/*************************************************************************
 * Function Name: call_LimitDown
 * Parameters: void
 * Return: void
 *
 * Description: Decrements the limit down by the limit increment amount until the max
 * limit is reached, then decrement down no more.  This function is performed in the
 * meter task - The variable is and event are set here.
 * STEP 1: Setup for event
 **************************************************************************/
void call_OhmsLimitDown(void)
{
 
 OHMS.UI = INCREMENT_DOWN;
 ctl_events_set_clear(&CalEvents, EVENT_OHMS_UI, 0);
 
} // END OF call_OHMS_LimitDown




/*************************************************************************
 * Function Name: call_OhmsMeterModeEnd
 * Parameters: void
 * Return: void
 *
 * Description: Performs the actions necessary to end OHMS Mode such that the next
 * mode can start itself.  This function sets CalMode to calculator mode as a default.  
 * It is up to the next mode function to set its mode of operation.
 * STEP 1: End the Ohm meter task: by stopping the IRQ event and turning off beep tone
 * STEP 2: Select all Keys for the next function and set Meter to Measure voltage (safe mode)
 * STEP 3: Prep the display for use by the next Mode
 **************************************************************************/
 void call_OhmsMeterModeEnd(void)
 {
 
 // STEP 1
 // STOP ANY FUTURE OHMS METER EVENT
 OHMS.Status = STOP_MEASURE;
 ContinunityTone = FALSE;
 // WAIT FOR METER EVENT TO CLEAR - AS THIS WILL BE A STABLE TIME TO SWITCH
 while (CalEvents & (EVENT_OHMS|EVENT_OHMS_UI))
   {
   ctl_timeout_wait(ctl_get_current_time()+1);
   } 
 #if defined(REMOVE_RESTORE)
    ctl_task_remove(&meter_task);
 #elif defined(SUSPEND_RUN)
    ctl_HabTaskSuspend(&meter_task);
 #endif
 
 // STEP 2
 select_All_Normal_Keys();
 // SET THE METER TO THE HIGHEST RANGE (SAFEEST MODE) - MAKE BEFORE BREAK
 GPIO_SetValue(PORT1, (RANGE_30V|SEL_V_C));
 GPIO_ClearValue(PORT1, (RANGE_10V|RANGE_20V));

 // STEP 3
 // CLEAR THE DISPLAY
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 // SET TO DEFAULT MODE
 CalSettings.CalMode = CAL_MODE;
  
 } // END OF call_OhmsMeterModeEnd