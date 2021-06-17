/*****************************************************************
 *
 * File name:       ADC_HC15C.H
 * Description:     Project definitions and function prototypes for use with ADC_HC15C.c
 * Author:          Hab S. Collector
 * Date:            11/12/2011
 * LAST EDIT:       7/04/2012
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


// DEFINES
#define ADC_FULL_COUNT    4095.0
#define ADC_REFERENCE     3.0000

// USE FOR METER FRONT END DIVIDER AND BAT VOLTAGE
/* FROM LOG BOOK NOTES (HAB'S NEXT CAL) ON HOW TO CALCULATE RESISTANCE (OHMS MODE)
 * NOTES ON 11/30/11 */
#define R12               (10.0E3)
#define R13               (10.0E3)
#define R31               (100.0)    
#define R34               (300.0E3)
#define R35               (10.0E3)
#define R37               (147.0E3)
#define R39               (100.0E3)
#define R41               (10990.0)
#define R42               (58900.0)
#define R47               (30240.0)
#define R48               (12000.0)
#define R49               (7521.0)
#define R52               (357.0)
#define R53               (270.0)
#define RA                (R34 + R35)
#define RC                R31
#define RD                (R49 + R53)

// ADC DIVIDER VALUES
#define ADC_10V_DIVIDER   (R47/(R41+R42+R47))
#define ADC_20V_DIVIDER   ((R52 + R48)/(R52+R48+R41+R42))
#define ADC_30V_DIVIDER   ((R53 + R49)/(R53+R49+R41+R42))
#define ADC_OHMS_DIVIDER  1.0
#define ADC_VUSB_DIVIDER  (R13/(R12+R13))
#define ADC_BAT_DIVIDER   (R39/(R37+R39))

// ADC PARAMETERS CONVERSION RATE
#define ADC_CONVERSION_RATE 100000
#define ADC_AVG_WEIGHT      10
#define ADC_SAMPLE_RATE     3

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
  uint8_t ADC_Type;                 // SHOULD BE: ADC_BAT_VOLTATE TO ADC_MET_VOLTAGE
  uint8_t ADC_AvgWeight;            // 0 = NO AVERAGE
  uint8_t ADC_msTimeBetweenSamples; // Time between samples in the average in ms
  double ADC_FrontEndDivider;       // MUST BE GREATER THAN 0
  } ADC_HC15C_Type;


// PROTOTYPE FUNCITONS
void init_ADC_polling(void);
double ADC_getConvertedValue(ADC_HC15C_Type);


#endif