/*****************************************************************
 *
 * File name:       PWM_HC15C.H
 * Description:     Project definitions and function prototypes for use with PWM_HC15C.c
 * Author:          Hab S. Collector
 * Date:            7/10/2011
 * Hardware:               
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent 
 *                  on other includes - everything these functions need should be passed to them
 *
 *****************************************************************/ 

#ifndef _PWM_HC15C_DEFINES
#define _PWM_HC15C_DEFINES


// INCLUDES
#include "HC15C_DEFINES.h"
#include "lpc17xx_pwm.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_clkpwr.h"


// ENUMERATED TYPES AND STRUCTURES
enum PWM_HP15C_TYPES
     {
     PWM_BACK_LIGHT,
     PWM_AUDIO_TONE
     };

// STRUCTURES
typedef struct
  {
  uint8_t PWM_Type;         // SHOULD BE: PWM_BACK_LIGHT OR PWM_AUDIO
  uint8_t PWM_DutyCycle;    // Expressed as 0 t0 100%
  uint32_t PWM_Frequency;   // PWM REP FREQUENCY: NOTE WILL BE THE SAME FOR BOTH
  } PWM_HC15C_Type;

// DEFINES
#define BACK_LIGHT_PINSEL_FUNCTION  3
#define BACK_LIGHT_PORT             3
#define BACK_LIGHT_PIN              25
#define AUDIO_PINSEL_FUNCTION       3
#define AUDIO_PORT                  3
#define AUDIO_PIN                   26
#define BACK_LIGHT_FREQUENCY_IN_HZ  1000u

// PROTOTYPE FUNCITONS
void init_PWM(PWM_HC15C_Type PWM_HC15C_Struct);  
void PWM_PWMToTimerMode(void);
void PWM_stop(uint8_t);
void PWM_start(uint8_t);


#endif