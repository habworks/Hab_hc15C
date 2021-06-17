/*****************************************************************
 *
 * File name:         METER_TASK.H
 * Description:       Project definitions and function prototypes for use with METER_TASKS.c
 * Author:            Hab S. Collector
 * Date:              11/27/2011
 * LAST EDIT:         7/06/2012
 * Hardware:               
 * Firmware Tool:     CrossStudio for ARM
 * Notes:             This file should be written as to not be dependent 
 *                    on other includes - everything these functions need should be passed to them
*****************************************************************/ 

#ifndef _METER_TASK_DEFINES
#define _METER_TASK_DEFINES


// INCLUDES
#include "HC15C_DEFINES.h"
#include "PWM_HC15C.H"
#include "ADC_HC15C.H"


// DEFINES FOR METER MODE
// 01234567890
// VMAX: 0.000
// RANGE 00-10V
#define MAX_ROW           1
#define RANGE_ROW         2
#define METER_ROW         4
#define MAX_POSITION      6
#define RANGE_POSITION    0
#define METER_POSITION    7
#define METER_SAMPLE_AVG  32
#define METER_SAMPLE_AVG_TIME 3 // TIME IN ms
#define OHMS_SAMPLE_AVG   32
#define OHMS_SAMPLE_AVG_TIME  3 // TIME IN mS
#define RANGE_10V_TH      9.9   // THRESHOLD FOR 10V TO 20V RANGE
#define RANGE_20V_TH      19.9  // THRESHOLD FOR 20V TO 30V RANGE
#define RANGE_30V_TH      29.0  // THRESHOLD FOR 30V RANGE
// 012345
// 29.999
#define CLEAR_READING    "        "
#define TXT_RNG_00_10    "RANGE 00-10V"
#define TXT_RNG_10_20    "RANGE 10-20V"
#define TXT_RNG_20_30    "RANGE 20-30V"


// DEFINES FOR OHMS MODE
// 012345678901234567
// SET LIMIT: 25 OHMS
// RX: XX OMHS
//
// **SHORT**
#define LIMIT_ROW         1
#define RX_ROW            2
#define STATUS_ROW        4
#define LIMIT_POSITION    11
#define RX_POSITION       4
#define STATUS_POSITION   5
#define LIMIT_INCREMENT   25.0
#define MAX_OHM_LIMIT     1000.0
#define MIN_OHM_LIMIT     25.0
#define TXT_SHORT         "***CONT***"
#define TXT_OPEN          "***OPEN***"
#define TXT_OPEN_MEASURE  "OL"
#define OPEN_CIRCUIT_READING -999           // USED WITH HC15C APP TO DENOTE AN OPEN CIRCUIT

#define READ_TOL_ERROR    1.0               // PERCENT ERROR BASED ON READ TO READ ACCURACY AS BEING ACCEPTABLE
#define MAX_OHM_READ      5000              // A MEASURMENT GREATER THAN THIS IS CONSIDER OL

// STRUCTS FOR METER MODE
typedef struct
  {
  BOOLEAN Status;
  double MaxV;
  double LastRange;
  ADC_HC15C_Type ADC_HC15C;
  } Type_Meter;

// STRUCTS FOR OHMS MODE
typedef struct
  {
  BOOLEAN Status;
  double Limit;
  uint8_t UI;
  ADC_HC15C_Type ADC_HC15C;
  } Type_OHMS;

enum METER_STATUS
  {
  STOP_MEASURE,
  START_MEASURE
  };

enum OHMS_UI
  {
  INCREMENT_UP,
  INCREMENT_DOWN
  };

// PROTOTYPE FUNCITONS
// VOLT METER MODE
void meter_taskFn(void *p);
void call_VoltMeterMode(void);
void call_VoltMeterMaxReset(void);
void call_VoltMeterModeEnd(void);
// OHMS METER MODE
void call_OhmsMeterMode(void);
void call_OhmsLimitUp(void);
void call_OhmsLimitDown(void);
void call_OhmsMeterModeEnd(void);

#endif