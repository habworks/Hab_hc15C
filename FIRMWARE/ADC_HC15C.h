/*****************************************************************
 *
 * File name:       ADC_HC15C.H
 * Description:     Project definitions and function prototypes for use with ADC_HC15C.c
 * Author:          Hab S. Collector
 * Date:            7/12/2011
 * Hardware:               
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent 
 *                  on other includes - everything these functions need should be passed to them
 *
 *****************************************************************/ 

#ifndef _ADC_HC15C_DEFINES
#define _ADC_HC15C_DEFINES


// INCLUDES
#include "HC15C_DEFINES.h"
#include "CMSIS\lpc17xx_adc.h"
#include "CMSIS\lpc17xx_pinsel.h"
#include "CMSIS\lpc17xx_gpio.h"



// ENUMERATED TYPES AND STRUCTURES
enum ADC_HP15C_TYPES
     {
     ADC_BAT_VOLTAGE,
     ADC_MET_VOLTAGE,
     ADC_USB_VOLTAGE
     };

// STRUCTURES
typedef struct
  {
  uint8_t ADC_Type;          // SHOULD BE: ADC_BAT_VOLTATE TO ADC_MET_VOLTAGE
  uint8_t ADC_AvgWeight;     // 0 = NO AVERAGE
  float ADC_FrontEndDivider; // MUST BE GREATER THAN 0
  } ADC_HC15C_Type;

// DEFINES
#define ADC_FULL_COUNT    4095.0 //0xFFF
#define ADC_REFERENCE     3.00000
#define ADC_BAT_DIVIDER   0.45455 
#define ADC_10V_DIVIDER   0.29847
#define ADC_20V_DIVIDER   0.15004
#define ADC_30V_DIVIDER   0.09991

// PROTOTYPE FUNCITONS
void init_ADC_polling(void);
double ADC_getConvertedValue(ADC_HC15C_Type);



#endif