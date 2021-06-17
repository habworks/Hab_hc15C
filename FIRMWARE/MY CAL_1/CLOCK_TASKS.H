/*****************************************************************
 *
 * File name:         CLOCK_TASK.H
 * Description:       Project definitions and function prototypes for use with CLOCK_TASK.c
 * Author:            Hab S. Collector
 * Date:              11/24/2011
 * LAST EDIT:         7/12/2012
 * Hardware:               
 * Firmware Tool:     CrossStudio for ARM
 * Notes:             This file should be written as to not be dependent 
 *                    on other includes - everything these functions need should be passed to them
*****************************************************************/ 

#ifndef _CLOCK_TASKS_DEFINES
#define _CLOCK_TASKS_DEFINES

// INCLUDES
#include "HC15C_DEFINES.h"

// DEFINES
// 01234567890123456
// TIME: 00:00:00 AM
// DATE: NOV 26, 2011
// 01234567890123456
// SWAT: 00:00:00:00
// ALARM: 00:00 AM
#define SECOND_POSITION     12
#define MINUTE_POSITION     9
#define HOUR_POSITION       6
#define AM_PM_POSITION      15
#define SWAT_MIL_POSITION   15
#define SWAT_SEC_POSITION   12
#define SWAT_MIN_POSITION   9
#define SWAT_HR_POSITION    6
#define ALARM_HR_POSITION   7
#define ALARM_MIN_POSITION  10
#define ALARM_AM_POSITION   13
#define HEADING_ROW         1
#define TIME_ROW            1
#define DATE_ROW            2
#define CLOCK_SET_ROW       3
#define SWAT_ROW            4
#define ALARM_TIME_ROW      4
// RESET time is 23:59:45PM, 11-26-2011
#define DEFAULT_HOUR    23
#define DEFAULT_MINUTE  59
#define DEFAULT_SECOND  40
#define DEFAULT_MONTH   11
#define DEFAULT_DOM     26
#define DEFAULT_YEAR    2011
// RTC ABNORMAL IRQ MASK
#define RTC_OSC_FAIL    ((uint32_t)(1<<4))
#define PROMPT_WAITING_ON_UI     "*WAITING ON WIN APP*"

// STOP WATCH (SWAT) STRUCT
typedef struct
  {              
  BOOLEAN Status;            // SEE ENUM SWAT_STATUS
  uint8_t TenMiliSecTime;
  uint8_t SecTime;
  uint8_t MinuteTime;
  uint8_t HourTime;
  } Type_SWAT;

enum SWAT_STATUS
  {
  STOP,
  START
  };

enum MONTH_OF_YEAR
  {
  JAN = 1,
  FEB,
  MAR,
  APR,
  MAY,
  JUN,
  JUL,
  AUG,
  SEP,
  OCT,
  NOV,
  DEC
  };

enum ALARM_SET_PARAMETER
  {
  TIME_HOUR,
  TIME_MINUTE,
  TIME_AM_PM
  };

typedef struct
  {
  int8_t  Hour;
  int8_t  Minute;
  BOOLEAN AM_Time;
  } Type_AlarmSetTime;


// PROTOTOTYPES
void clock_taskFn(void *);
void call_ClockMode(void);
void RTC_IRQHandler(void);
void init_Clock(void);
void call_SWAT_Reset(void);
void call_SWAT_Stop(void);
void call_SWAT_Start(void);
void call_SetClock(void);
void call_ClockModeEnd(void);
static void call_BuildDateTimeStrings(uint8_t *, uint8_t *);
void call_AlarmMode(void);
void call_AlarmModeEnd(void);
void call_IncrementAlarmTime(void);
void call_DecrementAlarmTime(void);
void call_MoveSetCursorRight(void);
void call_MoveSetCursorLeft(void);
void call_TurnOnAlarm(void);
void call_TurnOffAlarm(void);
static void call_ShowAlarm(void);


#endif