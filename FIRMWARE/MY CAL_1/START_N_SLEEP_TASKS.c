/*****************************************************************
 *
 * File name:       START_N_SLEEP.C
 * Description:     CTL RTOS Functions used to support start and sleep related functions of the HC15C
 * Author:          Hab S. Collector
 * Date:            1/23/12
 * LAST EDIT:       8/20/2012
 * Hardware:        NXP LPC1768
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 
 
// INCLUDES
#include <ctl_api.h>
#include <string.h>
#include "lpc17xx.h"
#include "lpc_types.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_rtc.h"
#include "START_N_SLEEP_TASKS.H"
#include "CLOCK_TASKS.H"
#include "AUDIO_TASKS.H"
#include "ADC_HC15C.H"
#include "LED_HC15C.H"
#include "STMPE24M31_HC15C.H"
#include "TIMERS_HC15C.H"
#include "PWM_HC15C.H"
#include "CORE_FUNCTIONS.H"
#include "DIP204.H"

#include "FAT_FS_INC/ff.h"

// GLOBALS
  

// EXTERNS
extern CTL_EVENT_SET_t CalEvents;
extern CTL_TASK_t init_task;
extern CTL_MESSAGE_QUEUE_t AudioQueue;
extern Type_CalSettings CalSettings;
extern FATFS fs[1];
extern volatile uint8_t BackLightTimer;
 
/*************************************************************************
 * Function Name: init_taskFn
 * Parameters: void *
 * Return: void
 *
 * Description: RTOS task to init system peripherials and place the calculator in
 * the startup configuration.  The task waits on the init event being set.  This task
 * is call on POR and on wake from sleep to put the calculator in the start up state
 * INIT EVENT:
 * STEP 1: Launch timers and system level Inits.  Load cal settings and Ramp up display backlight.
 * STEP 2: Init the Touch Devices - this is fairly elaborate and may have initial failures
 * so a restart is necessary.
 * STEP 3: Play welcome and put device in Calculator Mode
 * STEP 4: Turn off power to unused hardware
 * STEP 5: Clear init task
 * DEEP SLEEP EVENT:
 * STEP 6: Process a go to deep sleep
 **************************************************************************/
void init_taskFn(void *p)
{
  PWM_HC15C_Type PWM_HC15C_Struct;
  static BOOLEAN PowerOnReset = TRUE;
  
  while(1)
    {
    ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS, &CalEvents, (EVENT_INIT|EVENT_D_SLEEP), CTL_TIMEOUT_NONE, 0);    
    
    if (CalEvents & EVENT_INIT)
      {
      // STEP 1
      // LAUNCH SYS LEVEL INITS - THE ORDER IS IMPORTANT
      // INIT BACK LIGHT AND AUDIO PWM BEFORE USE - DO FIRST BECAUSE Q6 GATE NOT PULLED DOWN
      PWM_HC15C_Struct.PWM_Frequency = BACK_LIGHT_FREQUENCY_IN_HZ;
      PWM_HC15C_Struct.PWM_DutyCycle = 0;
      PWM_HC15C_Struct.PWM_Type = PWM_BACK_LIGHT;
      init_PWM(PWM_HC15C_Struct);
      // TIMERS NEEED FOR EVENTS THAT FOLLOW
      init_HC15C_OnTimerCounter0(100);
      init_HC15C_OnTimerCounter1(100000);
      init_LED();
      init_ADC_polling();
      init_I2C0(I2C_STMPE24M31_FREQUENCY);
      init_Clock();
      init_DIP204();

      call_LoadCalSettings();
      // TAKE BAT READ MEASUREMET - BACK LIGHT IS OFF
      CalSettings.ForceBatRead = TRUE;
      ctl_events_set_clear(&CalEvents, EVENT_BAT_Q, 0);
      CalSettings.ForceBatRead = TRUE;
      ctl_events_set_clear(&CalEvents, EVENT_BAT_Q, 0);
      // OK NOW TURN ON BACK LIGHT TO 90%
      if (PowerOnReset)
        {
        PWM_HC15C_Struct.PWM_Type = PWM_BACK_LIGHT;
        for (uint8_t PWM_DutyCycle = 0; PWM_DutyCycle < 91; PWM_DutyCycle++)
          {
          PWM_HC15C_Struct.PWM_DutyCycle = PWM_DutyCycle;
          init_PWM(PWM_HC15C_Struct);
          delayXms(10);
          }
        }
     
      // STEP 2
      // INIT TOUCH 
      if (PowerOnReset)      
        {        
        init_STMPE24M31_EINT(); 
        // INIT DEVICE A
        do
          {
          STMPE24M31_reset(STMPE24M31_ACAP);
          delayXms(10); // ALLOW START UP IRQ TO CLEAR OR THERE WILL BE BUS CONTENTION IF YOU TRY TO ACCESS A SECOND
          } while (STMPE24M31_init(STMPE24M31_ACAP) != TRUE);
        delayXms(200);
        // INIT DEVICE B
        do
          {
          STMPE24M31_reset(STMPE24M31_BCAP);
          delayXms(10);
          } while (STMPE24M31_init(STMPE24M31_BCAP) != TRUE);
        delayXms(200); 
        }      
     
      // STEP 3
      // PLAY WELCOME
      if (PowerOnReset)
        {
        f_mount(0, &fs[0]);
        Type_AudioQueueStruct AudioQueueStruct;
        strcpy(AudioQueueStruct.FileName, WELCOME_WAV);
        AudioQueueStruct.PlayLevel = CORE_SOUND;
        ctl_message_queue_post(&AudioQueue, &AudioQueueStruct, CTL_TIMEOUT_NONE, 0);
        }
      call_CalMode();
      BackLightTimer = 0;

       
      /* STEP 4
       * PWR SAVE
       * TURN OFF UART0 AND UART1 */
      LPC_SC->PCONP &= ~(PCUART0|PCUART1);
      // SET THE AUDIO OUT TO LOW
      GPIO_SetDir(PORT0, PWR_AUDIO, 1);
      GPIO_ClearValue(PORT0, AOUT);
      GPIO_SetDir(PORT0, AOUT, 1);
      GPIO_ClearValue(PORT0, AOUT);

      // STEP 5
      ctl_events_set_clear(&CalEvents, 0, EVENT_INIT);
#if defined(REMOVE_RESTORE)
      ctl_task_remove(&init_task);
#elif defined(SUSPEND_RUN)
      ctl_HabTaskSuspend(&init_task);
#endif
        PowerOnReset = FALSE;
      } // END OF EVENT_INIT
    
    // STEP 6
    // DEEP SLEEP EVENT
    if (CalEvents & EVENT_D_SLEEP)
      {
      ctl_events_set_clear(&CalEvents, 0, EVENT_D_SLEEP);
      while (CalEvents & EVENT_D_SLEEP)
        {
        ctl_timeout_wait(ctl_get_current_time()+10);
        }
      goToSleep();
      } // END OF EVENT_D_SLEEP    
    
    } // END OF WHILE
  
} // END OF FUNCTION init_taskFn




/*************************************************************************
 * Function Name: init_WakeFromSleep_EINT
 * Parameters:    void
 * Return:        void
 *
 * Description: Sets up the External IRQ (EINT0) for use as a wake from sleep, deep 
 * sleep or power down.  This function configures the pin settings for the IRQ and enables
 * the IRQ.
 * STEP 1: Define pin assignments for External IRQ - EINT0
 * STEP 2: Set EINT0 as: edge, active low
 * STEP 3: Set priority and enable
 *************************************************************************/
 void init_WakeFromSleep_EINT(void)
 {
  
  // TYPEDEF
  PINSEL_CFG_Type PinCfgStruct;
  EXTI_InitTypeDef EXTICfgstruct;
  
  // STEP 1
  // FROM TABLE 8.5 USER MANUAL LPC17XX
  PinCfgStruct.OpenDrain = 0;
  PinCfgStruct.Pinmode = 0;
  PinCfgStruct.Funcnum = 1;
  PinCfgStruct.Pinnum = 10;
  PinCfgStruct.Portnum = PORT2;
  PINSEL_ConfigPin(&PinCfgStruct);
  
  // STEP 2
  EXTICfgstruct.EXTI_Line = EXTI_EINT0;
  EXTICfgstruct.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
  EXTICfgstruct.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
  EXTI_Config(&EXTICfgstruct);
  
  // STEP 3
  ctl_set_priority(EINT0_IRQn, EINT0_IRQ_PRIORITY);
  ctl_unmask_isr(EINT0_IRQn);
   
 } // END OF FUNCTION init_WakeFromSleep_EINT




/*************************************************************************
 * Function Name: EINT0_IRQHandler
 * Parameters:    void
 * Return:        void
 *
 * Description: IRQ Handler for EINT0.  The purpose of this IRQ is to wake the
 * processor from sleeping.
 * STEP 1: Clear the IRQ condition and mask the IRQ to prevent bounce re-occur
 * STEP 2: Perform actions to wake from sleep
 *************************************************************************/
 void EINT0_IRQHandler(void)
 {
 
  // STEP 1
  EXTI_ClearEXTIFlag(EXTI_EINT0);
  // UNMASK IRQ TO PREVENT SWITCH BOUNCE
  ctl_mask_isr(EINT0_IRQn);
  
  // STEP 2
  if (LPC_SC->PCON & (1<<9))  
    {
    LPC_SC->PCON |= ((1<<3));
    SCB->SCR = 0x00;
    wakeFromSleep();
    } 

 } // END OF FUNCTION EINT0_IRQHandler
 
 
 
 
 /*************************************************************************
 * Function Name: wakeFromSleep
 * Parameters:    void
 * Return:        void
 *
 * Description: The actions necessary to take when the calculator wakes from sleep.
 * The calculator is always awaken from sleep and placed into Cal Mode regardless
 * as to what mode it was in before it went to sleep.
 * STEP 1: Restore PLLs and core system functions.  Clear CalSettings InDeepSleep
 * STEP 2: Restore the init task
 * STEP 3: Set event for the init task - init task will run differently if not POR (see init task)
 *************************************************************************/
 void wakeFromSleep(void)
 {
  
  // STEP 1
  // RESTORE WAKEUP   
  SystemInit();
  CalSettings.TimeAlarm.HC15C_InDeepSleep = FALSE;
  //LPC_SC->PCON |= ((1<<9)); // CLEAR DEEP SLEEP AND POWER DOWN IF SET

  // STEP 2
#if defined(REMOVE_RESTORE)
    ctl_task_restore(&init_task);
#elif defined(SUSPEND_RUN)
    ctl_HabTaskRun(&init_task);
#endif
  
  // STEP 3
  ctl_events_set_clear(&CalEvents, EVENT_INIT, 0);
 
 } // END OF FUNCTION wakeFromSleep
 
 
 
 
  /*************************************************************************
 * Function Name: goToSleep
 * Parameters:    void
 * Return:        void
 *
 * Description: The actions necessary to place the processor in a type of sleep mode.
 * The processor stays in sleep mode until waken by EINT0, Touch A and B or RTC alarm.
 * NOTE: Do not sleep if connected to a USB Port (CalSettings.USB_Link) - HC15C is being
 * powered by the PC and not from its own batteries
 * NOTE: Deep Sleep and Power Down have an associated errata condition.  The recommended fix
 * is implemented here
 * STEP 1: Check if sleep disable - if so return.  
 * STEP 2: Store the Calculator Settings.  Save power by placing cal support devices in lowest power settings.  
 * Turn off Audio Amp. 
 * STEP 3: Disable all possible sources of external wake up except what is intended - the
 * wake from sleep external irq, touch irq and RTC alarm to wake from sleep mode.  Enable external wake
 * STEP 4: Sleep based on defined condition 
 *************************************************************************/
 void goToSleep(void)
 {
 
 //uint8_t RegisterAndValue[2] = {STMPE24M31_SYSCON1_REG, SYSCON1_HIBERNATE};
 
  // STEP 1
  // STORE SETTINGS
  if (CalSettings.SleepDisable | CalSettings.USB_Link)
    return;
  
  // STEP 2
  // SAVE PRESENT SETTINGS
  call_StoreCalSettings();
  // SET CONDITIONS FOR CIRCUIT LOW POWER MODE
  // POWER DOWN AUDIO
  GPIO_ClearValue(PORT0, PWR_AUDIO);
  // TURN OFF BACKLIGHT
  PWM_stop(PWM_BACK_LIGHT);
  //TURN OFF DISPLAY
  DIP204_DisplayOff();
  
  // STEP 3
  // DISABLE ALL POSSIBLE SOURCES OF WAKE UP EXCEPT WHAT YOU INTEND: WAKE FROM SLEEP EINT
  RTC_CntIncrIntConfig(LPC_RTC, RTC_TIMETYPE_SECOND, DISABLE);
  // TOUCH DEVICES TO HIBERNATE TO CONSERVE POWER
  //STMPE24M31_write(RegisterAndValue, sizeof(RegisterAndValue), SLAVE_ADDRESS_STMPE24M31_ACAP);
  //STMPE24M31_write(RegisterAndValue, sizeof(RegisterAndValue), SLAVE_ADDRESS_STMPE24M31_BCAP); 
  init_WakeFromSleep_EINT();

  // STEP 4
  // READY THE EXTERNAL NMI TO WAKE FROM SLEEP
#if defined(LIGHT_SLEEP)
  CLKPWR_Sleep();
#elif (defined(DEEP_SLEEP) || defined(POWER_DOWN))   
  /* ERRATA FIX: IF PLL0 IS ENABLED AND CONNECTED BEFORE DEEP SLEEP OR PWR DOWN MODES
   * IT WILL REMAIN CONNECTED AFTER THE CHIP ENTERS SAID SLEEP MODE.  THE RESULT OF 
   * WHICH WILL BE EXCESSIVE POWER CONSUMPTION NOT INDICATIVE OF THE SLEEP MODE YOU ARE IN.
   * DISABLE IRQ'S TO PREVENT PLLO REGISTER FROM NOT LOADING DUE TO IRQ INTERRUPUTING THE WRITE SEQUENCE
   * SOLUTION: DISABLE AND DISCONNECT PLL0 BEFORE SLEEP */
  ctl_global_interrupts_disable();
  LPC_SC->PLL0CON &= ~(1<<1); // DISCONNECT PLLO
  LPC_SC->PLL0FEED = 0xAA; // THIS LINE AND NEXT IS REQUIRED TO LOAD VALUE TO PLL0 REGISTER
  LPC_SC->PLL0FEED = 0x55;
  while ((LPC_SC->PLL0STAT & (1<<25)) != 0x00); /* Wait for main PLL (PLL0) to disconnect */
  LPC_SC->PLL0CON &= ~(1<<0); // TURN OFF PLL
  LPC_SC->PLL0FEED = 0xAA; // THIS LINE AND NEXT IS REQUIRED TO LOAD VALUE TO PLL0 REGISTER
  LPC_SC->PLL0FEED = 0x55;
  while ((LPC_SC->PLL0STAT & (1<<24)) != 0x00); /* Wait for main PLL (PLL0) to shut down */
  ctl_global_interrupts_enable();
  // NOW ISSUE DEEP SLEEP OR POWER DOWN    
  CalSettings.TimeAlarm.HC15C_InDeepSleep = TRUE;
#if defined(DEEP_SLEEP)
  CLKPWR_DeepSleep();
#endif
#if defined(POWER_DOWN)
  CLKPWR_PowerDown();
#endif
      
#elif defined(DEEP_POWER_DOWN)
  CLKPWR_DeepPowerDown();
#endif
      
 // NOTE: WAKE FROM SLEEP IRQ MUST RESTORE THE CLOCKS AND PLL
 
 } // END OF FUNCTION goToSleep
