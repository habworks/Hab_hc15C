/*****************************************************************
 *
 * File name:       TOUCH_TASKS.C
 * Description:     CTL RTOS Functions used to support touch related functions of the HC15C
 * Author:          Hab S. Collector
 * Date:            1/21/2012
 * LAST EDIT:       8/01/2012 
 * Hardware:        NXP LPC1769
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 

#include <ctl_api.h>
#include <stdio.h>
#include <string.h>
#include "lpc_types.h"
#include "TOUCH_TASKS.H"
#include "CLOCK_TASKS.H"
#include "METER_TASKS.H"
#include "LIST_TASKS.H"
#include "AUDIO_TASKS.H"
#include "SETUP_TASKS.H"
#include "START_N_SLEEP_TASKS.H"
#include "DIP204.H"
#include "CORE_FUNCTIONS.H"
#include "MATH_FUNCTIONS.H"
#include "AUDIO_TASKS.H"
#include "USB_LINK.H"


// GLOBAL VARIABLES

 
// EXTERN VARS
extern CTL_EVENT_SET_t CalEvents;
extern CTL_MESSAGE_QUEUE_t MsgQueue, AudioQueue;
extern volatile uint8_t BackLightTimer;
extern Type_CalSettings CalSettings;
extern BOOLEAN bln_LineLoaded;
extern CTL_MEMORY_AREA_t MemArea;
extern CTL_TASK_t audio_task;


/*************************************************************************
 * Function Name: click_taskFn
 * Parameters:    void *
 * Return:        void
 *
 * Description: RTOS CTL task to process a key touch event.  In many aspects this in the
 * heart and soul of the calculator because just about everything is controlled by a key press / click.
 * This function will filter the keypress (calculator mode dependent) and launch a specific action
 * based on the key pressed.  As almost everything the cal does is based on a keypress event
 * this task should be the highest running priority.
 * STEP 1: Get a message string (KeyPress event) from the message queue and filter the event
 * ie check to see if the key is enabled.  There are two key sets to consider A and B
 * STEP 2: Call the specific key event and clear the event.  The bit MASK_B_TOUCH_DATA
 * is used to establish if the event is a A or B channel press.
 *************************************************************************/
void click_taskFn(void *p)
{
 
 void *Msg;
 uint32_t MsgValue;
 
 while (1)
   {
   // STEP 1
   ctl_message_queue_receive(&MsgQueue, &Msg, CTL_TIMEOUT_NONE, 0);
   MsgValue = (uint32_t)Msg;  // Msg must be typed cast because it was defined as void type
   // CHECK IF KEY ENABLED
   if(!filter_KeyPressEvent(&MsgValue))
     {
     continue;
     } 
   
   // STEP 2
   if (MsgValue & MASK_B_TOUCH_DATA)
      select_key_functionB(MsgValue & 0x7FFFFFFF);
   else
     select_key_functionA(MsgValue);
   
   }
 
} // END OF click_taskFn




/*************************************************************************
 * Function Name: filter_KeyPressEvent
 * Parameters:    uint32_t *
 * Return:        BOOLEAN
 *
 * Description: Performs a filter of the key that was passed to see if it is presently
 * being filtered (screened) by the program.  The program needs to ignore certain keys depending on the Mode
 * it is in.  Since there are 2 cap touch controllers they are masked separately.  The value of the
 * KeyPressMsg is modified via its pointer reference.  If a valid key is pressed return true and set
 * the back light event (back light event will re-set back light to full brightness) and play the key press sound.
 * NOTE: This function is passed both CH A and B Keys - It can tell the difference by the CH B mask being set
 * STEP 1: Create a pointer
 * STEP 2: Check if more than 1 bit of the first 24 (24 total channels) was set - This is not a valid
 * key press event so return false
 * STEP 3: Perform the key mask filter
 * STEP 4: If any bit is set (excluding the B Mask for channel B - MSb), a key was pressed: reset the backlight timer
 * play click and return TRUE.  If no bits set return false 
 *************************************************************************/
 BOOLEAN filter_KeyPressEvent(uint32_t *KeyPressMsg)
 {

 uint32_t *KeyPress_ptr;
 uint32_t ChkBitPattern;
 uint8_t BitCount = 0;
 Type_AudioQueueStruct AudioQueueStruct;

  // STEP 1
 KeyPress_ptr = KeyPressMsg;
 
 // STEP 2 
 // CHECK IF MORE THAN ONE BIT SET - NOT A VALID KEY PRESS SO RETURN
 ChkBitPattern = *KeyPress_ptr;
 for (int Bit=0; Bit<24; Bit++)
    {
    if (ChkBitPattern & 0x00000001)
      {
      BitCount++;
      if (BitCount > 1)
        return(FALSE);
      }
    ChkBitPattern = ChkBitPattern >> 1;
    }

 // STEP 3
 // FILTER THE KEY PRESS BASED ON A OR B KEY
 if (*KeyPress_ptr & MASK_B_TOUCH_DATA)
   *KeyPress_ptr &= (CalSettings.Mask_KeyTouchB | MASK_B_TOUCH_DATA); // Preserve B channel by or with MASK_B_TOUCH_DATA
 else
   *KeyPress_ptr &= CalSettings.Mask_KeyTouchA;
   
 // STEP 4
 // FOR VALID KEYS PLAY THE AUDIO CLICK AND RESET THE BACKLIGHT TIMER BY CALLING THE BACK LIGHT EVENT
 if (*KeyPress_ptr & ~MASK_B_TOUCH_DATA)
   {
   ctl_events_set_clear(&CalEvents, EVENT_BACK_L, 0);
   strcpy(AudioQueueStruct.FileName, BUTTON_CLICK_WAV);
   AudioQueueStruct.PlayLevel = CORE_SOUND;
   ctl_PostMessageByMemAllocate(&AudioQueueStruct);
   return(TRUE);
   }
 else
   return(FALSE);

 } // END OF filter_KeyPressEvent




/*************************************************************************
 * Function Name: select_key_functionA
 * Parameters:    uint32_t 
 * Return:        void
 *
 * Description: Selects a function based on the combination of key press events.  
 * By combination it is meant that a shift key may be pressed hence giving that unique
 * key its alternate meaning.  Beyond the actual key pressed and shift key the function
 * is furthered defined by the cal mode of operation.  Unless decoded by the case, the 
 * Cal Mode is the mode of operation.   
 * STEP 1: call function based on keypress event on A
 *************************************************************************/
void select_key_functionA(uint32_t ACAP_TouchKey)
{
 
 // STEP 1
 switch(ACAP_TouchKey)
  {
  // CS37 Key_ATN
  // ALARM MODE: TURN OFF ALARM
  // GO TO SLEEP
  case ((uint32_t)(1<<0)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.TimeAlarm.AlarmEvent)
    {
    call_TurnOffAlarm();
    break;
    }
  if (CalSettings.L_Shift)
    {
    ctl_events_set_clear(&CalEvents, EVENT_D_SLEEP, 0);
    call_LShiftClick();
    break;
    }
  call_CalATN();
  break;
  
  // CS27 Key_tanx, SHIFT L: ATAN
  case ((uint32_t)(1<<1)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_atanX();
    call_LShiftClick();
    break;
    }
  call_tanX();
  break;
  
  // CS26 Key_cosX, SHIFT L: ACOS
  case ((uint32_t)(1<<2)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_acosX();
    call_LShiftClick();
    break;
    }
  call_cosX();
  break;
  
  // CS14 Key_eToX, SHIFT L: LN(x)
  case ((uint32_t)(1<<3)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_lnX();
    call_LShiftClick();
    break;
    }
  call_eToX();
  break;
  
  // CS15 Key_Xsqr, SHIFT L: SQRT(x)
  case ((uint32_t)(1<<4)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_sqrtX();
    call_LShiftClick();
    break;
    }
  call_Xsqr();
  break;
  
  // CS1 Key_A, SHIFT R: DEG
  case ((uint32_t)(1<<5)):
  if (CalSettings.L_Shift)  // L_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.R_Shift)
    {
    call_DegMode();
    call_RShiftClick();
    break;
    }
  call_NumClick(HEX_A);
  break;
  
  // CS2 Key_B, SHIFT R: RAD
  case ((uint32_t)(1<<6)):
  if (CalSettings.L_Shift)  // L_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.R_Shift)
    {
    call_RadMode();
    call_RShiftClick();
    break;
    }
  call_NumClick(HEX_B);
  break;
  
  // CS3 Key_C, SHIFT R: FIX
  case ((uint32_t)(1<<7)):
  if (CalSettings.L_Shift)  // L_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.R_Shift)
    {
    call_FixMode();
    call_RShiftClick();
    break;
    }
  call_NumClick(HEX_C);
  break;
  
  // CS17 Key_1overX, SHIFT L: !X
  case ((uint32_t)(1<<8)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_Xfactorial();
    call_LShiftClick();
    break;
    }
  call_OneOverX();
  break;
  
  // CS18 KEY_XTOY, SHIFT L: 3<>2
  case ((uint32_t)(1<<9)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_3to2();
    call_LShiftClick();
    break;
    }
  call_XtoY();
  break;
  
  // CS30 Backspace, SHIFT_L CLRx
  case ((uint32_t)(1<<10)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    {
    call_ClearX();
    call_LShiftClick();
    break;
    }
  call_NumClick(BACKSPACE);
  break;

  // CS29 Key_PtoR, SHIFT L: RtoP
  case ((uint32_t)(1<<11)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_RtoP();
    call_LShiftClick();
    break;
    }
    call_PtoR();
  break;
  
  // CS28 Key_jXC, SHIFT L: jXL
  case ((uint32_t)(1<<12)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_jXL();
    call_LShiftClick();
    }
    call_jXC();
  break;
  
  // CS42 Key_Drop SHIFT L: FLUSH
  case ((uint32_t)(1<<13)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_Flush();
    call_LShiftClick();
    }
    call_Drop();
  break;
  
  // CS39 Key_RShift
  case ((uint32_t)(1<<14)):
  call_RShiftClick();
  break;
  
  // CS38 Key_LShift
  case ((uint32_t)(1<<15)):
  call_LShiftClick();
  break;
  
  // CS25 Key_sinX, SHIFT L: ASIN
  case ((uint32_t)(1<<16)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_asinX();
    call_LShiftClick();
    break;
    }
  call_sinX();
  break;
  
  // CS13 Key_10toX, SHIFT L: LOG(x)
  case ((uint32_t)(1<<17)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_logX();
    call_LShiftClick();
    break;
    }
  call_10ToX();
  break;
  
  // CS4 Key_D, SHIFT R: ENG
  case ((uint32_t)(1<<18)):
  if (CalSettings.L_Shift)  // L_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.R_Shift)
    {
    call_EngMode();
    call_RShiftClick();
    break;
    }
  call_NumClick(HEX_D);
  break;
  
  // CS5 Key_E, SHIFT R: DEC
  case ((uint32_t)(1<<19)):
  if (CalSettings.L_Shift)  // L_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.R_Shift)
    {
    call_Base10Mode();
    call_RShiftClick();
    break;
    }
  call_NumClick(HEX_E);
  break;
  
  // CS6 Key_F, SHIFT R: HEX
  case ((uint32_t)(1<<20)):
  if (CalSettings.L_Shift)  // L_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.R_Shift)
    {
    call_Base16Mode();
    call_RShiftClick();
    break;
    }
  call_NumClick(HEX_F);
  break;
  
  // CS16 Key_yToX, SHIFT L: y^(1/x)
  case ((uint32_t)(1<<21)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;  
    if (CalSettings.L_Shift)
    {
    call_YtoOneOverX();
    call_LShiftClick();
    break;
    }
  call_YtoX();
  break;
  break;
  
  // CS41 Key_RCL, SHIFT L: STO
  case ((uint32_t)(1<<22)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    if (bln_LineLoaded)
      call_NumClick(STO);
    call_LShiftClick();
    break;
    }
  if (bln_LineLoaded)
    call_NumClick(RCL);
  break;
  
  // CS40 Key_Sigma+, SHIFT L: Clear Stat sums
  case ((uint32_t)(1<<23)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_StatClear();
    call_LShiftClick();
    break;
    }
  call_StatAdd();
  break;
  
  default:
  break;
  }

} // END OF select_key_functionA
        


/*************************************************************************
 * Function Name: select_key_functionB
 * Parameters:    uint32_t 
 * Return:        void
 *
 * Description: Selects a function based on the combination of key press events.  
 * By combination it is meant that a shift key may be pressed hence giving that unique
 * key its alternate meaning.  Beyond the actual key pressed and shift key the function
 * is furthered defined by the cal mode of operation.  Unless decoded by the case, the 
 * Cal Mode is the mode of operation.   
 * STEP 1: call function based on keypress event on B
 *************************************************************************/
void select_key_functionB(uint32_t BCAP_TouchKey)
{

// STEP 1
switch(BCAP_TouchKey)
  {
  // CS34 Key_3, SHIFT L: lb to kg, SHIFT R: kg to lb
  // SETUP MODE: CHANGE TIME TO DEEP SLEEP (DREAM)
  case ((uint32_t)(1<<0)):
  if (CalSettings.CalMode == SETUP_MODE)
    {
    call_TimeToSleepSetup();
    break;
    }
  if (CalSettings.L_Shift)
    {
    call_lb_to_kg();
    call_LShiftClick();
    break;
    }
  if (CalSettings.R_Shift)
    {
    call_kg_to_lb();
    call_RShiftClick();
    break;
    }
  call_NumClick(3);
  break;
  
  // CS35 Key_Subtract, SHIFT L: gal to l, SHIFT R: l to gal
  case ((uint32_t)(1<<1)):
  if (CalSettings.L_Shift)
    {
    call_gal_to_l();
    call_LShiftClick();
    break;
    }
  if (CalSettings.R_Shift)
    {
    call_l_to_gal();
    call_RShiftClick();
    break;
    }
  call_Subtract();
  break;
  
  // CS47 Key_Add, SHIFT L: in to cm, SHIFT R: cm to in
  case ((uint32_t)(1<<2)):
  if (CalSettings.L_Shift)
    {
    call_in_to_cm();
    call_LShiftClick();
    break;
    }
  if (CalSettings.R_Shift)
    {
    call_cm_to_in();
    call_RShiftClick();
    break;
    }
  call_Add();
  break;
  
  // CS44 Key_0, SHIFT L: mils to mm, SHIFT R: mm to mils
  case ((uint32_t)(1<<3)):
  if (CalSettings.L_Shift)
    {
    call_MILS_to_mm();
    call_LShiftClick();
    break;
    }
  if (CalSettings.R_Shift)
    {
    call_mm_to_MILS();
    call_RShiftClick();
    break;
    }
  call_NumClick(0);
  break;
  
  // CS32 Key1, SHIFT L: ft to m, SHIFT R: m to ft
  // SETUP MODE: CHANGE BACK LIGHT TIMETOUT
  case ((uint32_t)(1<<4)):
  if (CalSettings.CalMode == SETUP_MODE)
    {
    call_BL_TimeOutSetup();
    break;
    }
  if (CalSettings.L_Shift)
    {
    call_ft_to_m();
    call_LShiftClick();
    break;
    }
  if (CalSettings.R_Shift)
    {
    call_m_to_ft();
    call_RShiftClick();
    break;
    }
  call_NumClick(1);
  break;
  
  // CS43 Key_Enter
  // ALARM MODE: TURN ON ALARM
  case ((uint32_t)(1<<5)):
  if (CalSettings.CalMode == ALARM_MODE)
    {
    call_TurnOnAlarm();
    break;
    }
  if (CalSettings.CalMode == MUSIC_LIST_MODE)
    {
    call_PlayPauseMusic();
    break;
    }
  call_Enter();
  break;
  
  // CS19 Key_Exponent, SHIFT L: Percent Difference
  case ((uint32_t)(1<<6)):
  if (CalSettings.L_Shift)
    {
    call_DeltaPercent();
    call_LShiftClick();
    break;
    }
  call_NumClick(EXPONENT);
  break;
  
  // CS20 Key_4
  // ALARM MODE: MOVE TIME CURSOR SET POSITION LEFT
  // CLOCK MODE: STOP THE STOPWATCH
  // VOLTE METER MODE: RESET THE MAX HIGH VALUE
  case ((uint32_t)(1<<7)):
  if ((CalSettings.R_Shift)||(CalSettings.L_Shift))  // R_Shift OR L_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.CalMode == ALARM_MODE)
    {
    call_MoveSetCursorLeft();
    break;
    }
  if (CalSettings.CalMode == CLOCK_MODE)
    {
    call_SWAT_Reset();
    break;
    }
  if (CalSettings.CalMode == METER_MODE)
    {
    call_VoltMeterMaxReset();
    break;
    }
  call_NumClick(4);
  break;
  
  // CS31 USB LINK, SHIFT L: USB EJECT
  case ((uint32_t)(1<<8)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_USB_VCOM_UnLink();
    call_LShiftClick();
    break;
    }
  call_USB_VCOM_Link();
  break;
  
  // CS36 VOLT METER MODE, SHIFT L: OHM METER MODE
  case ((uint32_t)(1<<9)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_OhmsMeterMode();
    call_LShiftClick();
    break;
    }
  call_VoltMeterMode();
  break;
  
  // CS48 MUSIC MODE, SHIFT L: DIR LIST MODE
  case ((uint32_t)(1<<10)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_SD_ListMode();
    call_LShiftClick();
    break;
    }
   call_MusicListMode();
  break;
  
  // CS9 Key_8
  // ALARM MODE: INCREMENT TIME PARAMETER UP
  // OHM METER MODE: INCREMENT CONTINUNITY MATCH VALUE UP
  // DIR LIST MODE: SCROLL DISPLAY UP
  // MUSIC LIST MODE: SCROLL DISPLAY UP
  case ((uint32_t)(1<<11)):
  if ((CalSettings.R_Shift)||(CalSettings.L_Shift))  // R_Shift OR L_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.CalMode == ALARM_MODE)
    {
    call_IncrementAlarmTime();
    break;
    }
  if (CalSettings.CalMode == OHMS_MODE)
    {
    call_OhmsLimitUp();
    break;
    }
  if (CalSettings.CalMode == SD_LIST_MODE)
    {
    call_DirScrollUp();
    break;
    }
  if (CalSettings.CalMode == MUSIC_LIST_MODE)
    {
    call_MusicScrollUp();
    break;
    }
  call_NumClick(8);
  break;
  
  // CS10 Key_9, SHIFT L: F to C, SHIFT R: C to F
  case ((uint32_t)(1<<12)):
  if (CalSettings.L_Shift)
    {
    call_F_to_C();
    call_LShiftClick();
    break;
    }
  if (CalSettings.R_Shift)
    {
    call_C_to_F();
    call_RShiftClick();
    break;
    }
  call_NumClick(9);
  break;
  
  // CS11 Key_Divide, SHIFT L: mph to kph, SHIFT R: kph to mph
  case ((uint32_t)(1<<13)):
  if (CalSettings.L_Shift)
    {
    call_mph_to_kph();
    call_LShiftClick();
    break;
    }
  if (CalSettings.R_Shift)
    {
    call_kph_to_mph();
    call_RShiftClick();
    break;
    }
  call_Divide();
  break;
  
  // CS23 Key_Multiply, SHIFT L: fps to mps, SHIFT R: mps to fps
  case ((uint32_t)(1<<14)):
  if (CalSettings.L_Shift)
    {
    call_fps_to_mps();
    call_LShiftClick();
    break;
    }
  if (CalSettings.R_Shift)
    {
    call_mps_to_fps();
    call_RShiftClick();
    break;
    }
  call_Multiply();
  break;
  
  // CS33 Key_2
  // ALARM MODE: DECREMENT ALARM TIME PARAMETER
  // CLOCK MODE: ENABLE WIN APP CLOCK SET BUTTON
  // OHM METER MODE: DECREMENT CONTINUNITY SET RESISTANCE
  // SD LIST MODE: SCROLL DOWN SD LIST
  // SETUP MODE: CHANGE CAL VERBSOE SETTING
  // MUSIC LIST MODE: SCROLL DOWN MUSIC LIST
  case ((uint32_t)(1<<15)):
  if ((CalSettings.R_Shift)||(CalSettings.L_Shift))  // R_Shift OR L_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.CalMode == ALARM_MODE)
    {
    call_DecrementAlarmTime();
    break;
    }
  if (CalSettings.CalMode == CLOCK_MODE)
    {
    call_SetClock();
    break;
    }
  if (CalSettings.CalMode == OHMS_MODE)
    {
    call_OhmsLimitDown();
    break;
    }
  if (CalSettings.CalMode == SD_LIST_MODE)
    {
    call_DirScrollDown();
    break;
    }
  if (CalSettings.CalMode == SETUP_MODE)
    {
    call_CalVerboseSetup();
    break;
    }
  if (CalSettings.CalMode == MUSIC_LIST_MODE)
    {
    call_MusicScrollDown();
    break;
    }
  call_NumClick(2);
  break;
  
  // CS46 Key_PI, SHIFT L: 2pi, SHIFT R: 2piX
  case ((uint32_t)(1<<16)):
  if (CalSettings.L_Shift)
    {
    call_2PiClick();
    call_LShiftClick();
    break;
    }
  if (CalSettings.R_Shift)
    {
    call_2PiXClick();
    call_RShiftClick();
    break;
    }
  call_PiClick();
  break;
  
  // CS45 Key_. SHIFT L: mean, SHIFT R: standard deviation
  case ((uint32_t)(1<<17)):
  if (CalSettings.L_Shift)
    {
    call_Mean();
    call_LShiftClick();
    break;
    }
  if (CalSettings.R_Shift)
    {
    call_StanDev();
    call_RShiftClick();
    break;
    }
  call_NumClick(DECIMAL_POINT);
  break;
  
  // CS7 Key_CHS, SHIFT L: absolute value
  case ((uint32_t)(1<<18)):
  if (CalSettings.R_Shift)
    {
    call_abs();
    call_RShiftClick();
    break;
    }
  call_NumClick(CHANGE_SIGN);
  break;
  
  // CS8 Key_7, SHIFT L: mi to km, SHIFT R: km to mi
  case ((uint32_t)(1<<19)):
  if (CalSettings.L_Shift)
    {
    call_mi_to_km();
    call_LShiftClick();
    break;
    }
  if (CalSettings.R_Shift)
    {
    call_km_to_mi();
    call_RShiftClick();
    break;
    }
  call_NumClick(7);
  break;
  
  // CS12 CALCULATOR MODE, SHIFT L: SETUP MODE
  case ((uint32_t)(1<<20)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_SetupMode();
    call_LShiftClick();
    break;
    } 
  call_CalMode();
  break;
  
  //CS24 CLOCK MODE, SHIFT L: ALARM MODE
  case ((uint32_t)(1<<21)):
  if (CalSettings.R_Shift)  // R_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.L_Shift)
    {
    call_AlarmMode();
    call_LShiftClick();
    break;
    }
  call_ClockMode();
  break;
  
  // CS21 Key_5
  // CLOCK MODE: STOP STOP WATCH
  // ALARM MODE: TURN OFF ALARM
  // MUSIC LIST MODE: ENABLE STOP OF PLAY AUDIO
  case ((uint32_t)(1<<22)): 
  if ((CalSettings.R_Shift)||(CalSettings.L_Shift))  // R_Shift OR L_Shift NOT DEFINED FOR THIS KEY
    break;
  if ((CalSettings.CalMode == CLOCK_MODE) && (!CalSettings.TimeAlarm.AlarmEvent))
    {
    call_SWAT_Stop();
    break;
    }
  if ((CalSettings.CalMode == ALARM_MODE) || (CalSettings.TimeAlarm.AlarmEvent))
    {
    call_TurnOffAlarm();
    break;
    }
  if (CalSettings.CalMode == MUSIC_LIST_MODE)
    {
    call_MusicStop();
    break;
    }
  call_NumClick(5);
  break;
  
  // CS22 Key_6
  // ALARM MODE: MOVE TIME CURSOR SET POSITION RIGHT
  // CLOCK MODE: START / CONTINUE STOP WATCH
  // MUSIC LIST MODE: PLAY SELECTED AUDIO
  case ((uint32_t)(1<<23)):
  if ((CalSettings.R_Shift)||(CalSettings.L_Shift))  // R_Shift OR L_Shift NOT DEFINED FOR THIS KEY
    break;
  if (CalSettings.CalMode == ALARM_MODE)
    {
    call_MoveSetCursorRight();
    break;
    }
  if (CalSettings.CalMode == CLOCK_MODE)
    {
    call_SWAT_Start();
    break;
    }
  if (CalSettings.CalMode == MUSIC_LIST_MODE)
    {
    call_PlayPauseMusic();
    break;
    }
  call_NumClick(6);
  break;
  
  default:
  break;
  }

} // END OF select_key_functionB




/*************************************************************************
 * Function Name: select_All_Normal_Keys
 * Parameters:    void 
 * Return:        void
 *
 * Description: Selects all keys for Touch A inputs, and all keys minus the MASK_KEY_ALINK
 * for the Touch B inputs.  Values are loaded to the CalSettings Mask Key members for
   A and B 
 * NOTE: The Key MASK_KEY_ALINK calls the VCOM USB select.  This should ONLY be active
 * when in Meter Mode.  If active anywhere else the USB connect or disconnect will cause
 * crash
 * STEP 1: Set the struct member
 *************************************************************************/
void select_All_Normal_Keys(void)
{
  
  // STEP 1
  CalSettings.Mask_KeyTouchA = CalSettings.Mask_KeyTouchB = ALL_KEYS_SELECTED;
  CalSettings.Mask_KeyTouchB &= ~MASK_KEY_ALINK;
  
} // END OF select_All_Normal_Keys