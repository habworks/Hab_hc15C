/*****************************************************************
 *
 * File name:       TIMERS_HC15C.C
 * Description:     Functions used to support timer operations on the HC15C
 * Author:          Hab S. Collector
 * Date:            9/22/11
 * LAST EDIT:       8/03/2012
 * Hardware:        NXP LPC1769
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 
 
 #include <ctl_api.h>
 #include "TIMERS_HC15C.H"
 #include "LED_HC15C.H"
 #include "CLOCK_TASKS.H"
 #include "METER_TASKS.H"
 #include "AUDIO_TASKS.H"
 #include "CORE_FUNCTIONS.H"
 #include "lpc17xx_timer.h"
 #include "lpc17xx_gpio.h"
 #include "FAT_FS_INC/diskio.h"
 
 
 // GLOBAL VARS
 volatile uint8_t BackLightTimer = 0;
 volatile uint16_t TimerCounterA_100us;
 volatile uint16_t TimerCounterB_100us;
 volatile uint16_t TimerCounterC_100us;
 volatile uint32_t msCounter;


 // EXTERNS
 extern CTL_EVENT_SET_t CalEvents;
 extern Type_CalSettings CalSettings;
 extern Type_SWAT SWAT;
 extern Type_Meter Meter;
 extern Type_OHMS OHMS;
 extern CTL_MUTEX_t DelayMutex;
 extern volatile uint8_t AudioChannel;
 extern volatile BOOLEAN PlayLeftChannel;
 extern Type_CircularBuffer CircularBufferLeft, CircularBufferRight;
 extern BOOLEAN ContinunityTone;
 
 
 /*************************************************************************
 * Function Name: delayXms
 * Parameters:    uint32_t
 * Return:        void
 *
 * Description: For CTL RTOS based applications.  Delays program execution by the
 * stated number of mili-seconds.  The time interval of the IRQ in ms
 * NOTE: Multiple calling tasks - must be thread safe
 * STEP 1: If desired delay less than max possible delay, then delay, if not no delay
 *************************************************************************/
 void delayXms(uint32_t DelayIn_ms)
  {
 
  ctl_mutex_lock(&DelayMutex, CTL_TIMEOUT_NONE, 0); 
 
  // STEP 1
  TimerCounterA_100us = 0;
  msCounter = 0;
  while (msCounter < DelayIn_ms);
  
  ctl_mutex_unlock(&DelayMutex); 
  
  } // END OF delayXms



/*************************************************************************
 * Function Name: SysTimerCallFromISR
 * Parameters:    void
 * Return:        void
 *
 * Description: For CTL RTOS based applications.  This function is called whenever
 * the SYSTick Timer IRQ occurs.  Stated differently it is called by the SYS Tick ISR.
 * As it is included in the SYS Tick ISR it should be non-blocking, as short as possible 
 * and should only be used if 100% totally unavoidable.  Leave this for the CTL functions.
 * time interval of the IRQ in ms
 * STEP 1: 
 *************************************************************************/
 void SysTimerCallFromISR(void)
 {
 
 ctl_increment_tick_from_isr();

 } // END OF FUNCTION SysTimerCallFromISR
  
  

/*************************************************************************
 * Function Name: init_HC15C_OnTimerCounter0
 * Parameters: void
 * Return: void
 *
 * Description: Init of Timer to IRQ every X number of micro-seconds based on 
 * the parameter that is passed.  Use of the MR0, timer counter peripherial 
 * STEP 1: Set up counter prescale Value
 * STEP 2: Set up MR0 channel to use
 * STEP 3: Set configuration and enable IRQ
 * STEP 4: Set IRQ priority in NVIC
 **************************************************************************/
void init_HC15C_OnTimerCounter0(uint32_t TimeInMicoSeconds)
{
  
  TIM_TIMERCFG_Type TIM_ConfigStruct;
  TIM_MATCHCFG_Type TIM_MatchConfigStruct;
  
  // STEP 1
  TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
  TIM_ConfigStruct.PrescaleValue = 1;
  TIM_ConfigStructInit(TIM_TIMER_MODE, &TIM_ConfigStruct);
  // PRESCALE TIME = PCLK (IN MHZ) x PrescaleValue x PCLK Period = TIME IN MICRO-SECONDS
  TIM_ConfigStruct.PrescaleValue = 1;

  // STEP 2
  TIM_MatchConfigStruct.MatchChannel = 0;
  // Enable interrupt when MR0 matches the value in TC register
  TIM_MatchConfigStruct.IntOnMatch = TRUE;
  //Enable reset on MR0: TIMER will reset if MR0 matches it
  TIM_MatchConfigStruct.ResetOnMatch = TRUE;
  //Stop on MR0 if MR0 matches it
  TIM_MatchConfigStruct.StopOnMatch = FALSE;
  //Toggle MR0.0 pin if MR0 matches it
  TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
  // PRESCALE TIME (IN MICRO-SECONDS x MatchValue = Counter Time
  TIM_MatchConfigStruct.MatchValue = TimeInMicoSeconds;

  // STEP 3
  TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TIM_ConfigStruct);
  TIM_ConfigMatch(LPC_TIM0, &TIM_MatchConfigStruct);
  TIM_Cmd(LPC_TIM0, ENABLE);
  
  // STEP 4
  ctl_set_priority(TIMER0_IRQn, TIMER0_IRQ_PRIORITY);
  ctl_unmask_isr(TIMER0_IRQn);
  
} // END OF FUNCTIOIN init_HC15C_OnTimerCounter0




/*************************************************************************
 * Function Name: TIMER0_IRQHandler
 * Parameters: void
 * Return: void
 *
 * Description: IRQ Handler for Timer0.  
 * Used to count milli-seconds: for the delayXms function and for the 50% duty square 
 * wave of the continunity beep.  
 * Used to create 1 sec period: The 1s period is used as the backlight timer and to set an event for
 * the battery power monitor task.  
 * Used to create 10ms interval for use with the FAT FS
 * NOTE: In order to use this IRQ the function init_HC15C_OnTimerCounter0
 * must be previously called and set to an IRQ of 100us.  
 * STEP 1: Increment the 100us Timers
 * STEP 2: Increment the ms Counter - Create Audio Beep if Continunity Tone set
 * STEP 3: Increment the 500ms Counter 
 * STEP 4: Increment the 1s Counter: Take care of back light
 * STEP 5: Increment the 10ms Counter
 * STEP 6: Clear the IRQ
 **************************************************************************/
void TIMER0_IRQHandler(void)
 {
 
 static uint8_t TimerFatFS_DiskIO;
 static BOOLEAN ToggleTone = 0;
 
 // STEP 1
 TimerCounterA_100us++;
 TimerCounterB_100us++;
 TimerCounterC_100us++;
 
 // STEP 2
 // EVERY 1 MILI-SECOND COUNTER
 // USED IN FUNCTION delayXms
 if(TimerCounterA_100us > 9)
   {
   msCounter++;
   TimerCounterA_100us = 0;
   
    // USED IN OHM METER CONTINUNITY TO PRODUCE 2KHz TONE (T = 1ms + 1ms)
   if (ContinunityTone)
     {
     if (ToggleTone)
       {
       // ARBITARY VALUE FOR AUDIO HIGH OUTPUT - COULD BE ALMOST ANY VALUE: 0x64 < VALUE < 0x3FF
       uint16_t SetToneHigh = 0x0FF;
       // NOTE: MUST TURN ON AUDIO AS ANY PLAY OF AUDIO WILL TURN OFF AUDIO PWR UPON COMPLETION
       GPIO_SetValue(PORT0, PWR_AUDIO);
       LPC_DAC->DACR = (SetToneHigh << 6); // SET AUDIO OUT HIGH FOR NEXT 1ms
       }
     else
       {
       LPC_DAC->DACR = 0; // SET AUDIO OUT LOW FOR NEXT 1ms
       }
     ToggleTone = ~ToggleTone;
     }
   }
 
 // STEP 3
 // EVERY 500 MILL-SECONDS
 // USE IN LED BLINK EVENT
 if (TimerCounterB_100us > 4999)
  {
  // FOR FUTURE USE - IF YOU NEED A 500ms EVENT
  TimerCounterB_100us = 0;
  }
 
 // STEP 4
 // EVERY SECOND 
 // USED IN BACKLIGHT EVENT AND AUDIO PLAY BACK COUNT TIME COUNT DOWN
 if (TimerCounterC_100us > 9999)
   {
   // PREVENT OVERFLOW 
   if (BackLightTimer < 0xFF)
      BackLightTimer++;
   ctl_events_set_clear(&CalEvents, EVENT_BAT_Q, 0);
   TimerCounterC_100us = 0;
   }
 
 // STEP 5
 // EVERY 10ms 100Hz
 // USE FOR FAT FS SD INTERFACE
 if (TimerFatFS_DiskIO > 99)
   {
   TimerFatFS_DiskIO = 0;
   disk_timerproc();
   }
 
 // STEP 6
 TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);

 } // END OF FUNCTION TIMER0_IRQHandler




/*************************************************************************
 * Function Name: init_HC15C_OnTimerCounter1
 * Parameters: void
 * Return: void
 *
 * Description: Init of Timer to IRQ every X number of micro-seconds based on 
 * the parameter that is passed.  Use of the MR0, timer counter peripherial 
 * STEP 1: Set up counter prescale Value
 * STEP 2: Set up MR0 channel to use
 * STEP 3: Set configuration and enable IRQ
 * STEP 4: Set IRQ priority in NVIC
 **************************************************************************/
void init_HC15C_OnTimerCounter1(uint32_t TimeInMicoSeconds)
{
  
  TIM_TIMERCFG_Type TIM_ConfigStruct;
  TIM_MATCHCFG_Type TIM_MatchConfigStruct;
  
  // STEP 1
  TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
  TIM_ConfigStruct.PrescaleValue = 1;
  TIM_ConfigStructInit(TIM_TIMER_MODE, &TIM_ConfigStruct);
  // PRESCALE TIME = PCLK (IN MHZ) x PrescaleValue x PCLK Period = TIME IN MICRO-SECONDS
  TIM_ConfigStruct.PrescaleValue = 1;

  // STEP 2
  TIM_MatchConfigStruct.MatchChannel = 0;
  // Enable interrupt when MR0 matches the value in TC register
  TIM_MatchConfigStruct.IntOnMatch = TRUE;
  // Enable reset on MR0: TIMER will reset if MR0 matches it
  TIM_MatchConfigStruct.ResetOnMatch = TRUE;
  // Stop on MR0 if MR0 matches it
  TIM_MatchConfigStruct.StopOnMatch = FALSE;
  //Toggle MR0.0 pin if MR0 matches it
  TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
  // PRESCALE TIME (IN MICRO-SECONDS x MatchValue = Counter Time
  TIM_MatchConfigStruct.MatchValue = TimeInMicoSeconds;

  // STEP 3
  TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &TIM_ConfigStruct);
  TIM_ConfigMatch(LPC_TIM1, &TIM_MatchConfigStruct);
  TIM_Cmd(LPC_TIM1, ENABLE);
  
  // STEP 4
  ctl_set_priority(TIMER1_IRQn, TIMER1_IRQ_PRIORITY);
  ctl_unmask_isr(TIMER1_IRQn);
  
} // END OF FUNCTIOIN init_HC15C_OnTimerCounter1




/*************************************************************************
 * Function Name: TIMER1_IRQHandler
 * Parameters: void
 * Return: void
 *
 * Description: IRQ Handler for Timer1. This timer IRQ handler sets the events
 * for the StopWatch, VoltMeter, OhmMeter, and USB link. Timer is set to expire in 
 * accordance with the passed value to init_HC15C_OnTimerCounter1.  This is set to be
 * a 100ms timer event
 * STEP 1: Set the stop watch (SWAT) event
 * STEP 2: Set the Meter event (measure voltage at terminal)
 * STEP 3: Set the OHMs event (measure ohms at terminal)
 * STEP 5: Clear the Timer IRQ
 **************************************************************************/
void TIMER1_IRQHandler(void)
 {
 
 // STEP 1
 if ((CalSettings.CalMode == CLOCK_MODE) && (SWAT.Status == START))
   {
   ctl_events_set_clear(&CalEvents, EVENT_SWAT, 0);
   }
 
 // STEP 2
 if ((CalSettings.CalMode == METER_MODE) && (Meter.Status == START_MEASURE))
   {
   ctl_events_set_clear(&CalEvents, EVENT_METER, 0);
   }
 
  // STEP 3
 if ((CalSettings.CalMode == OHMS_MODE) && (OHMS.Status == START_MEASURE))
   {
   ctl_events_set_clear(&CalEvents, EVENT_OHMS, 0);
   }
   
 // STEP 4
 TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
  
 } // END OF FUNCTION TIMER1_IRQHandler




/*************************************************************************
 * Function Name: init_HC15C_OnTimerCounter2
 * Parameters: void
 * Return: void
 *
 * Description: Init of Timer to IRQ every X number of PClk period intervals based on 
 * the parameter that is passed.  Use of the MR0, timer counter peripherial 
 * STEP 1: Set up counter prescale Value
 * STEP 2: Set up MR0 channel to use
 * STEP 3: Set configuration and enable IRQ
 * STEP 4: Set IRQ priority in NVIC
 **************************************************************************/
void init_HC15C_OnTimerCounter2(uint32_t TimeInMicoSeconds)
{
  
  TIM_TIMERCFG_Type TIM_ConfigStruct;
  TIM_MATCHCFG_Type TIM_MatchConfigStruct;
  
  // STEP 1
  TIM_ConfigStructInit(TIM_TIMER_MODE, &TIM_ConfigStruct);
  TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_TICKVAL; // change here
  TIM_ConfigStruct.PrescaleValue = 1;
  TIM_ConfigStruct.Reserved[0] =  TIM_ConfigStruct.Reserved[1] =  TIM_ConfigStruct.Reserved[2] = 0;
  
  // STEP 2
  TIM_MatchConfigStruct.MatchChannel = 0;
  // Enable interrupt when MR0 matches the value in TC register
  TIM_MatchConfigStruct.IntOnMatch = TRUE;
  // Enable reset on MR0: TIMER will reset if MR0 matches it
  TIM_MatchConfigStruct.ResetOnMatch = TRUE;
  // Stop on MR0 if MR0 matches i
  TIM_MatchConfigStruct.StopOnMatch = FALSE;
  //Toggle MR0.0 pin if MR0 matches it
  TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
  // PRESCALE TIME (IN MICRO-SECONDS x MatchValue = Counter Time
  TIM_MatchConfigStruct.MatchValue = TimeInMicoSeconds;

  // STEP 3
  TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &TIM_ConfigStruct);
  TIM_ConfigMatch(LPC_TIM2, &TIM_MatchConfigStruct);
  TIM_Cmd(LPC_TIM2, ENABLE);
  
  // STEP 4
  ctl_set_priority(TIMER2_IRQn, TIMER2_IRQ_PRIORITY);
  ctl_unmask_isr(TIMER2_IRQn);
  
} // END OF FUNCTIOIN init_HC15C_OnTimerCounter2




/*************************************************************************
 * Function Name: TIMER2_IRQHandler
 * Parameters: void
 * Return: void
 *
 * Description: ISR Handler for Timer2. This is the Timer for the Audio play back.
 * It works with the function call_play16Bit_WAVE.  The ISR reads the value from one
 * of two circular buffers (L/R) and outputs PCM Audio to the (singular) DAC.  The ISR is enabled,
 * its timer set and disabled by call_play16Bit_WAVE.  PlayLeftChannel is set true by
 * the end of call_play16Bit_WAVE such that this IRQ will always be in-sync to play the
 * left channel first.
 * NOTE: If stereo, the channels alternate between left and right to the single DAC
 * STEP 1: Output Data from the circular buffer if it is not empty for the corresponding
 * channel.  If this is STEREO alternate between left and right channels.  Load the value
 * the audioLED_BarGraph and drive the output DAC.  
 * STEP 2: Alternate between left and right channels
 * STEP 3: Clear the IRQ
 **************************************************************************/
void TIMER2_IRQHandler(void)
 {
 
 uint16_t PlayValue;

 // STEP 1
 // READ VALUE FROM ONE OF CIRCULAR BUFFERS AND OUTPUT TO AUDIO
 if ((!isEmpty_CB(&CircularBufferLeft)) && (PlayLeftChannel))
   {
   read_CB(&CircularBufferLeft, &PlayValue);
   PlayValue &= 0x03FF; // THE DAC VALUE IS 10BIT - JUST BEING SAFE
   audioLED_BarGraph(PlayValue);
   LPC_DAC->DACR = (PlayValue << 6);
   }
 if ((!isEmpty_CB(&CircularBufferRight)) && (AudioChannel == STEREO) && (!PlayLeftChannel))
   {
   read_CB(&CircularBufferRight, &PlayValue);
   PlayValue &= 0x03FF; // THE DAC VALUE IS 10BIT - JUST BEING SAFE
   audioLED_BarGraph(PlayValue);
   LPC_DAC->DACR = (PlayValue << 6); // FOR TRUE STERO WOULD OUTPUT TO 2ND AUDIO DAC
   }

 // STEP 2
 // ALTERNATE AUDIO CHANNEL IF IN STERO
 if (AudioChannel == STEREO)
   {
   if (PlayLeftChannel)
     PlayLeftChannel = FALSE;
   else
     PlayLeftChannel = TRUE;
   }
 
 // STEP 3
 TIM_ClearIntPending(LPC_TIM2, TIM_MR0_INT);
  
 } // END OF FUNCTION TIMER2_IRQHandler


