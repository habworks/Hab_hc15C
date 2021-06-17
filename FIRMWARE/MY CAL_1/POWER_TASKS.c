/*****************************************************************
 *
 * File name:       POWER_TASKS.C
 * Description:     CTL RTOS Functions used to support power related functions of the HC15C
 * Author:          Hab S. Collector
 * Date:            9/25/11
 * LAST EDIT:       7/04/2012
 * Hardware:        NXP LPC1769
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 

#include <ctl_api.h>
#include "POWER_TASKS.H"
#include "START_N_SLEEP_TASKS.H"
#include "DIP204.H"
#include "ADC_HC15C.H"
#include "PWM_HC15C.H"
#include "CORE_FUNCTIONS.H"

// EXTERNS
extern CTL_EVENT_SET_t CalEvents;
extern volatile uint8_t BackLightTimer;
extern Type_CalSettings CalSettings;



/*************************************************************************
 * Function Name: batQ_taskFn
 * Parameters: void *
 * Return: void
 *
 * Description: RTOS task to check and display the status of the battery charge.  
 * This event is set in the timer IRQ every second.  The Battery ICON is only updated
 * if there was a change in status from the last call - no hysteresis implemented at this time
 * The other function of this task is to update the backlight display.  The counter BackLightTimer
 * is incremented every second in the same IRQ.  The back light is dimmed according to the count.
 * The event back light sets the back light to full brightness and resets the back light counter to 0
 * STEP 1: Read Battery Voltage and USB (J3) voltage.  If connected to USB always display a full charge and
 * display ICON of computer connect - this is the USB Link
 * STEP 2: Update the Battery Charge Display ICON if necessary
 * STEP 3: Update the display back light
 * STEP 4: Handle Back Light Event
 **************************************************************************/
void batQ_taskFn(void *p)
{
  
  ADC_HC15C_Type ADC_HC15C_Struct;
  PWM_HC15C_Type PWM_HC15C_Struct;
  static float LastSetting = 999.999;  // DummyValue
  static double LastValidBatVoltageReading;
  double MeasuredBatVoltage,
         MeasuredUSB_Voltage;
  uint8_t TimeTo_70Percent,
          TimeTo_30Percent,
          TimeTo_00Percent;
          
  
  while(1)
    {
    ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS, &CalEvents, (EVENT_BAT_Q|EVENT_BACK_L), CTL_TIMEOUT_NONE, 0);

    // EVENT BAT_Q:
    // UPDATE THE BAT ICON AND UPDATE THE DISPLAY BACKLIGHT
    if (CalEvents & EVENT_BAT_Q)
      {
      // STEP 1
      // READ AND CHECK IF CONNECTED TO USB FOR PWR
      ADC_HC15C_Struct.ADC_AvgWeight = ADC_AVG_WEIGHT;
      ADC_HC15C_Struct.ADC_msTimeBetweenSamples = ADC_SAMPLE_RATE;
      ADC_HC15C_Struct.ADC_Type = ADC_USB_VOLTAGE;
      ADC_HC15C_Struct.ADC_FrontEndDivider = ADC_VUSB_DIVIDER;
      MeasuredUSB_Voltage = ADC_getConvertedValue(ADC_HC15C_Struct);
      if (MeasuredUSB_Voltage > MIN_USB_VOLTAGE)
        {
        CalSettings.USB_Link = TRUE;
        MeasuredBatVoltage = Q_100_PERCENT; 
        DIP204_ICON_set(ICON_CALCULATOR, ICON_ON);
        }
      else
        {
        // NOT CONNECTED TO USB FOR PWR - MEASURE BATTERY VOLTAGE IF BACKLIGHT TIMER IS OFF
        CalSettings.USB_Link = FALSE;
        DIP204_ICON_set(ICON_CALCULATOR, ICON_OFF);
        ADC_HC15C_Struct.ADC_Type = ADC_BAT_VOLTAGE;
        ADC_HC15C_Struct.ADC_FrontEndDivider = ADC_BAT_DIVIDER;
        /* ONLY MEASURE IF BACKLIGHT IS OFF OR IF FIRST TIME READING BATTERY (SO SOMETHING CAN BE DISPLAYED)
         * HIGHER LOADER CURRENT INCREASES THE BAT'S INTERNAL RESISTANCE RESULTING IN LESSER BAT SOURCE VOLTAGE
         * MEASURE BAT AT LOWEST CURRENT DRAIN, WHICH ALSO HAPPENS TO BE THE NORMAL STATE */
        if ((CalSettings.ForceBatRead) ||(BackLightTimer >= TimeTo_00Percent))
          {
          MeasuredBatVoltage = ADC_getConvertedValue(ADC_HC15C_Struct);
          LastValidBatVoltageReading = MeasuredBatVoltage;
          CalSettings.ForceBatRead = FALSE;
          }
        else
          {
          MeasuredBatVoltage = LastValidBatVoltageReading;
          }
        }
      
      // STEP 2
      // LESS THAN 10% - SHOW WARNING
      if(MeasuredBatVoltage < Q_10_PERCENT)
        {
        if (LastSetting != 0)
          {
           DIP204_ICON_set(ICON_BATTERY, ICON_BAT_EWARN);
           LastSetting = 0;
          }
        }
      // BATTERY AT EMPTY
      if((MeasuredBatVoltage >= Q_10_PERCENT) && (MeasuredBatVoltage < Q_25_PERCENT))
        {
        if (LastSetting != Q_10_PERCENT)
          {
           DIP204_ICON_set(ICON_BATTERY, ICON_BAT_EMPTY);
           LastSetting = Q_10_PERCENT;
          }
        }
      // BATTERY AT 25%
      if((MeasuredBatVoltage >= Q_25_PERCENT) && (MeasuredBatVoltage < Q_50_PERCENT))
        {
        if (LastSetting != Q_25_PERCENT)
          {
           DIP204_ICON_set(ICON_BATTERY, ICON_BAT_1QTR);
           LastSetting = Q_25_PERCENT;
          }
        }
      // BATTERY AT 50%
      if((MeasuredBatVoltage >= Q_50_PERCENT) && (MeasuredBatVoltage < Q_75_PERCENT))
        {
        if (LastSetting != Q_50_PERCENT)
          {
           DIP204_ICON_set(ICON_BATTERY, ICON_BAT_2QTR);
           LastSetting = Q_50_PERCENT;
          }
        }
      // BATTERY AT 75%
      if((MeasuredBatVoltage >= Q_75_PERCENT) && (MeasuredBatVoltage < Q_100_PERCENT))
        {
        if (LastSetting != Q_75_PERCENT)
          {
           DIP204_ICON_set(ICON_BATTERY, ICON_BAT_3QTR);
           LastSetting = Q_75_PERCENT;
          }
        }
      // BATTERY AT 100%
      if(MeasuredBatVoltage >= Q_100_PERCENT)
        {
        if (LastSetting != Q_100_PERCENT)
          {
           DIP204_ICON_set(ICON_BATTERY, ICON_BAT_FULL);
           LastSetting = Q_100_PERCENT;
          }
        }
      
      // STEP 3
      // UPDATE THE BACKLIGHT - BackLightTimeOut IS IN 10'S OF SECONDS
      PWM_HC15C_Struct.PWM_Frequency = BACK_LIGHT_FREQUENCY_IN_HZ;
      PWM_HC15C_Struct.PWM_Type = PWM_BACK_LIGHT;
      TimeTo_70Percent = (uint8_t)(0.25 * (10 * CalSettings.Setup.BackLightTimeOut));
      TimeTo_30Percent = (uint8_t)(0.50 * (10 * CalSettings.Setup.BackLightTimeOut));
      TimeTo_00Percent = 10 * CalSettings.Setup.BackLightTimeOut;
      // TAKE ACTION BASED ON SPECIFIC TIME EVENTS, BACK LIGHT REDUCTION IN STAGES OR SLEEP
      if ((BackLightTimer == TimeTo_70Percent) && (CalSettings.Setup.BackLightTimeOut != 0))
        {
        PWM_HC15C_Struct.PWM_DutyCycle = 70;
        init_PWM(PWM_HC15C_Struct);
        }      
      if ((BackLightTimer == TimeTo_30Percent) && (CalSettings.Setup.BackLightTimeOut != 0))
        {
        PWM_HC15C_Struct.PWM_DutyCycle = 30;
        init_PWM(PWM_HC15C_Struct);
        }     
      if ((BackLightTimer == TimeTo_00Percent) || (CalSettings.Setup.BackLightTimeOut == 0))
        {
        PWM_stop(PWM_BACK_LIGHT);
        } 
      if (BackLightTimer >= CalSettings.Setup.TimeToSleep * 10)
        ctl_events_set_clear(&CalEvents, EVENT_D_SLEEP, 0);

      ctl_events_set_clear(&CalEvents, 0, EVENT_BAT_Q);
    } // END OF EVNET_BAT_Q
    
    // STEP 4
    // BACK LIGHT EVENT
    if ((CalEvents & EVENT_BACK_L) && (CalSettings.Setup.BackLightTimeOut != 0))
      {
      BackLightTimer = 0; // BECASUE IT IS SET TO 0 HERE - IT CAN NEVER BE 0 ABOVE - AS IT IS INCREMENTED ON NEXT ISR TO BE 1
      PWM_HC15C_Struct.PWM_Frequency = BACK_LIGHT_FREQUENCY_IN_HZ;
      PWM_HC15C_Struct.PWM_DutyCycle = 90;
      PWM_HC15C_Struct.PWM_Type = PWM_BACK_LIGHT;      
      init_PWM(PWM_HC15C_Struct);
      ctl_events_set_clear(&CalEvents, 0, EVENT_BACK_L);
      } // END OF EVENT_BACK_L
    } // END OF WHILE
  
} // END OF FUNCTION batQ_taskFn