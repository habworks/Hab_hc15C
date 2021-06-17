/*****************************************************************
 *
 * File name:         TIMER_HC15C.C
 * Description:       Functions used to support LED operation on the HC 15C
 * Author:            Hab S. Collector
 * Date:              7/4/11
 * Hardware:          NXP LPC1769
 * Firmware Tool:     CrossStudio for ARM
 * Notes:             This file should be written as to not be dependent on other includes.
 *                    everything these functions need should be passed to them.  
 *                    It will be necessary to consult the reference documents and associated schematics to understand
 *                    the operations of this firmware.
 *****************************************************************/ 

// INCLUDES
#include "TIMER_HC15C.H"


// GLOBAL VARS
uint32_t volatile msCounter;
uint16_t volatile TimerCounterA_100us;
uint16_t volatile TimerCounterB_100us;
// TYPE DEFS
static CTL_ISR_FN_t ISR_TimerCounter0_Var;
TIM_TIMERCFG_Type TIM_ConfigStruct;
TIM_MATCHCFG_Type TIM_MatchConfigStruct;




// EXTERNS DECLARED


// PRIVATE FUNCTIONS USED ONLY HERE
//void INT_Handle_TimerCounter0 (void);


/*************************************************************************
 * Function Name: initTimerCounter0
 * Parameters: uint8_t LED_Status, uint32_t LEDs_ToChange
 * Return: void
 *
 * Description: Init of Timer to IRQ at stated frequency
 * 
 * STEP 1: Switch based on LED status
 * STEP 2: Perform Status operation as a case.  Note setting the GPIO port bit turn off the LED.
 **************************************************************************/
void ISR_TimerCounter0(void)
 {
 
 static BOOLEAN OnOffState = TRUE;
 
 TimerCounterA_100us++;
 TimerCounterB_100us++;
 
 // EVERY MILL-SECOND
 if(TimerCounterA_100us > 9)
   {
   msCounter++;
   TimerCounterA_100us = 0;
   }
 
 // EVERY 500 MILL-SECONDS
 if (TimerCounterB_100us > 4999)
  {
  if (OnOffState == TRUE)
    {
    switchLED(LED_ON, (LED_DRIVE));
    OnOffState = FALSE;
    }
  else
    {
    switchLED(LED_OFF, (LED_USB|LED_DRIVE));
    OnOffState = TRUE;
    }
  TimerCounterB_100us = 0;
  }

 }

void delayXms(uint32_t DelayIn_ms)
  {
  msCounter = 0;
  while (msCounter < DelayIn_ms);
  }




void TIMER0_IRQHandler(void)
  {
  ctl_enter_isr();
  ISR_TimerCounter0();
  TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
  ctl_exit_isr();
  }


void ctl_HC15C_OnTimerCounter0(CTL_ISR_FN_t buttonFn)
{
    
  //ISR_TimerCounter0_Var = buttonFn;

  TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
  TIM_ConfigStruct.PrescaleValue = 1;
  TIM_ConfigStructInit(TIM_TIMER_MODE, &TIM_ConfigStruct);
  // PRESCALE TIME = PCLK (IN MHZ) x PrescaleValue x PCLK Period = TIME IN MICRO-SECONDS
  TIM_ConfigStruct.PrescaleValue = 1;

  // use channel 0, MR0
  TIM_MatchConfigStruct.MatchChannel = 0;
  // Enable interrupt when MR0 matches the value in TC register
  TIM_MatchConfigStruct.IntOnMatch   = TRUE;
  //Enable reset on MR0: TIMER will reset if MR0 matches it
  TIM_MatchConfigStruct.ResetOnMatch = TRUE;
  //Stop on MR0 if MR0 matches it
  TIM_MatchConfigStruct.StopOnMatch  = FALSE;
  //Toggle MR0.0 pin if MR0 matches it
  TIM_MatchConfigStruct.ExtMatchOutputType =TIM_EXTMATCH_NOTHING;
  // PRESCALE TIME (IN MICRO-SECONDS x MatchValue = Counter Time
  TIM_MatchConfigStruct.MatchValue   = 100;

  // Set configuration for Tim_config and Tim_MatchConfig
  TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TIM_ConfigStruct);
  TIM_ConfigMatch(LPC_TIM0,&TIM_MatchConfigStruct);
  //TIM_Init(LPC_TIM0, TIM_COUNTER_RISING_MODE, &TIM_ConfigStruct);

  // To start timer 0
  TIM_Cmd(LPC_TIM0,ENABLE);
  
  ctl_set_priority(TIMER0_IRQn, 1);
  ctl_unmask_isr(TIMER0_IRQn);
}

