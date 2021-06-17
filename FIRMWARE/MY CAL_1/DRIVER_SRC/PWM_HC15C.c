/******************************************************************************
 *
 * File name:         PWM_HC15C.C
 * Description:       Functions used to support PWM operation on the hc 15C
 * Author:            Hab S. Collector
 * Date:              7/10/11
 * Last Edit:         8/03/12
 * Hardware:          NXP LPC1769
 * Firmware Tool:     CrossStudio for ARM
 * Notes:             This is the IP of Hab Collector.  You are welcomed to use this work free
 * of charge.  However, within the file's header you are requested to give credit to the
 * the author even if you make changes from its original form.  If the information provided 
 * was of assistance to you, then do the right thing.
 *
 *****************************************************************************/ 

// INCLUDES
#include "PWM_HC15C.H"

// GLOBAL VARS

// EXTERNS


/*************************************************************************
 * Function Name: init_PWM
 * Parameters: PWM_HC15C_Type *
 * Return: void
 *
 * Description: Init PWM for HC15C back light and PWM audio.  Requires the function
   PWM_PWMToTimerMode to be ran first.
 * STEP 1: Set up pin functions for PWM output
 * STEP 2: Compute Timer0 Count value based on frequency (structure member) and set Timer 0
 *         Calculate the duty cycle count and set the corresponding match timer 
 * STEP 3: Configure the match register for single edge - 
 * STEP 4: Enable the PWM output channel
 * NOTE: THE FREQUENY OF ALL THE PWM CHANNEL OUTPUS WILL BE THE VALUE OF THE LAST FREQUENCY SET
 *************************************************************************/
 void init_PWM(PWM_HC15C_Type PWM_HC15C_Struct)
 {
  
  // DECLARE TYPES FOR USE
  uint32_t PWM_TimerCounter0, DutyCycleCounter;
  uint8_t PWM_Channel;
  
  // DEFINE TYPES FOR USEAGE
  PWM_MATCHCFG_Type PWMMatchCfgDatStruct;
  PINSEL_CFG_Type PINSEL_PinCfgStruct;
  
  // STEP 1
  // FROM TABLE 8.5 USER MANUAL LPC17XX
  // SET PIN PARAMETERS BASED ON PWM TYPE
  switch (PWM_HC15C_Struct.PWM_Type)
   {
   case PWM_BACK_LIGHT:
   // P3.25 = PWM1.2
   PINSEL_PinCfgStruct.Funcnum = BACK_LIGHT_PINSEL_FUNCTION;
   PINSEL_PinCfgStruct.OpenDrain = 0;
   PINSEL_PinCfgStruct.Pinmode = 0;
   PINSEL_PinCfgStruct.Portnum = BACK_LIGHT_PORT;
   PINSEL_PinCfgStruct.Pinnum = BACK_LIGHT_PIN;
   PWM_Channel = 2;
   break;
   
   default:
   case PWM_AUDIO_TONE:
   // P3.26 = PWM1.3
   PINSEL_PinCfgStruct.Funcnum = AUDIO_PINSEL_FUNCTION;
   PINSEL_PinCfgStruct.OpenDrain = 0;
   PINSEL_PinCfgStruct.Pinmode = 0;
   PINSEL_PinCfgStruct.Portnum = AUDIO_PORT;
   PINSEL_PinCfgStruct.Pinnum = AUDIO_PIN;
   PWM_Channel = 3;
   break;
   }
  PINSEL_ConfigPin(&PINSEL_PinCfgStruct);
        
  // STEP 2
	/* CALCULATE PWM FREQUENCY COUNT BASED ON PCLK FOR THE PWM MODULE
   * VALUE IS DERIVED FROM THE DESIRED FREQUENCY */
  PWM_TimerCounter0 = (uint32_t) ((CLKPWR_GetPCLK(CLKPWR_PCLKSEL_TIMER0)/PWM_HC15C_Struct.PWM_Frequency) + ROUNDING);
	PWM_MatchUpdate(LPC_PWM1, 0, PWM_TimerCounter0, PWM_MATCH_UPDATE_NOW);
	/* PWM Timer/Counter will be reset when channel 0 matching
	 * no interrupt when match
	 * no stop when match */
	PWMMatchCfgDatStruct.IntOnMatch = DISABLE;
	PWMMatchCfgDatStruct.MatchChannel = 0;
	PWMMatchCfgDatStruct.ResetOnMatch = ENABLE;
	PWMMatchCfgDatStruct.StopOnMatch = DISABLE;
	PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDatStruct);
   
  // STEP 3
  /* CONFIGURE AS:
   * - SINGLE EDGE
   * - PWM DUTY
   * - PWM DUTY IS DETERMINED BY THE MATCH ON CH 0 TO THE MATCH OF THAT CHANNEL
   * EXAMPLE: PWM DUTY ON PWM CHANNEL 1 IS DETERMINED BY THE MATCH ON CH0 TO THE MATCH ON CHANNEL 1 
	 * NOTE: PWM CH 1 IS IN SINGLE MODE AS DEFAULT STATE AND CANNOT BE CHANGED TO DOUBLE EDGE */
		PWM_ChannelConfig(LPC_PWM1, PWM_Channel, PWM_CHANNEL_SINGLE_EDGE);
  // CALCULATE DUTY CYCLE COUNT VALUE
  DutyCycleCounter = (uint32_t) ((PWM_TimerCounter0 * (PWM_HC15C_Struct.PWM_DutyCycle/100.0)) + ROUNDING);
  // LOAD THE MATCH VALUE
	PWM_MatchUpdate(LPC_PWM1, PWM_Channel, DutyCycleCounter, PWM_MATCH_UPDATE_NOW);
  // CONFIGURE THE MATCH OPTIONS
  PWMMatchCfgDatStruct.IntOnMatch = DISABLE;
  PWMMatchCfgDatStruct.MatchChannel = PWM_Channel;
  PWMMatchCfgDatStruct.ResetOnMatch = DISABLE;
  PWMMatchCfgDatStruct.StopOnMatch = DISABLE;
  PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDatStruct);
  
  // STEP 4
  /* NOTE MUST BE CODED IN THIS ORDER TO WORK:
   * ENABLE PWM CHANNEL OUTPUT */ 
  PWM_ChannelCmd(LPC_PWM1, PWM_Channel, ENABLE);
  // RESET AND START COUNTER
 	PWM_ResetCounter(LPC_PWM1);
	PWM_CounterCmd(LPC_PWM1, ENABLE);
  // START NOW
//  PWM_Cmd(LPC_PWM1, ENABLE);

  // TEST STOP
//  PWM_ChannelCmd(LPC_PWM1, PWM_Channel, DISABLE);
  // TEST START
//  PWM_ChannelCmd(LPC_PWM1, PWM_Channel, ENABLE);

 } // END OF FUNCTION init_PWM




/*************************************************************************
 * Function Name: PWM_PWMToTimerMode
 * Parameters: void
 * Return: void
 * Description: Configure the PWM to timer mode.  Must be called before PWM_init
 * can be called.  
 * STEP 1: Configure PWM in timer mode
 *************************************************************************/
 void PWM_PWMToTimerMode(void)
 {
  // DECLARE TYPES FOR USE
  PWM_TIMERCFG_Type PWMCfgDatStruct;
  
  // STEP 1
  /* CONFIGURE PWM IN TIMER MODE
   * PWM prescale value = 1 (absolute value - tick value) */
  PWMCfgDatStruct.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL;
  PWMCfgDatStruct.PrescaleValue = 1;
  PWM_Init(LPC_PWM1, PWM_MODE_TIMER, (void *) &PWMCfgDatStruct);
   
 } // END OF FUNCTION PWM_PWMToTimerMode
 
 
 
 
 /*************************************************************************
 * Function Name: PWM_stop
 * Parameters: uint8_t
 * Return: void
 *
 * Description: Accepts PWM_BACK_LIGHT -OR- PWM_AUDIO TONE.  Stops the PWM such
 * that the output will be low. 
 * STEP 1: Wait on Timer Counter to be greater than the selected PWM channel 
 * STEP 2: Disable the selected channel 
 * NOTE: Requires that the functions PWM_PWMToTimerMode, init_PWM, and PWM_start
 * be first called in that order
 *************************************************************************/
 void PWM_stop(uint8_t PWM_HP15C_Type)
 {
 
 // DECLARE TYPES FOR USE
 uint8_t PWM_Channel;
 
 // STEP 1
 switch (PWM_HP15C_Type)
   {
   case PWM_AUDIO_TONE:
   while (LPC_PWM1->TC <= LPC_PWM1->MR3);
   PWM_Channel = 3;
   break;
   
   default:
   case PWM_BACK_LIGHT:
   PWM_Channel = 2;
   while (LPC_PWM1->TC <= LPC_PWM1->MR2);
   break;
   }
  
 // STEP 2 
 PWM_ChannelCmd(LPC_PWM1, PWM_Channel, DISABLE);
 PWM_ResetCounter(LPC_PWM1);
 
 } // END OF FUNCTION PWM_stop
 
 
 
 
 /*************************************************************************
 * Function Name: PWM_start
 * Parameters: uint8_t
 * Return: void
 *
 * Description: Accepts PWM_BACK_LIGHT -OR- PWM_AUDIO TONE.  Stops the PWM such
 * that the output will be low.
 * NOTE: Requires that the functions PWM_PWMToTimerMode, and init_PWM be first called 
 * in that order
 * STEP 1: Start the selected PWM channel
 *************************************************************************/
 void PWM_start(uint8_t PWM_HP15C_Type)
 {
   
 // STEP 1
 switch (PWM_HP15C_Type)
   {
   case PWM_AUDIO_TONE:
   PWM_ChannelCmd(LPC_PWM1, 3, ENABLE);
   break;
   
   default:
   case PWM_BACK_LIGHT:
   PWM_ChannelCmd(LPC_PWM1, 2, ENABLE);
   break;
   }
 
 } // END OF FUNCTIOIN PWM_start
   
