/*****************************************************************
 *
 * File name:         CORE_FUNCTIONS.H
 * Description:       Project definitions and function prototypes for use with CORE_FUNCTIONS.c
 * Author:            Hab S. Collector
 * Date:              10/12/2011
 * LAST EDIT:         7/24/2012
 * Hardware:               
 * Firmware Tool:     CrossStudio for ARM
 * Notes:             This file should be written as to not be dependent 
 *                    on other includes - everything these functions need should be passed to them
*****************************************************************/ 

#ifndef _CORE_FUNCTIONS_DEFINES
#define _CORE_FUNCTIONS_DEFINES


// INCLUDES
#include "HC15C_DEFINES.H"
#include "AUDIO_TASKS.H"
#include <ctl_api.h>


// DEFINES
#define TOTAL_REGISTERS     4     // TOP OF REGISTER BOTTOM (X REGISTER = 0, Y REGISTER = 1) - ARRAY OF 4
#define MAX_DISPLAY_LENGTH  20
#define NULL_VALUE          0x00
// USED FOR NUMERIC ENTRY
#define HEX_A               65 // ASCII A
#define HEX_B               66
#define HEX_C               67
#define HEX_D               68
#define HEX_E               69
#define HEX_F               70 // ASCII F
#define DECIMAL_POINT       100
#define EXPONENT            101
#define CHANGE_SIGN         102
#define BACKSPACE           200
#define STO                 226
#define RCL                 227

// NUMBER FORMATING
#define LARGEST_FIX_DISPLAY_NUMBER 1E10               // LARGEST POSSIBLE FIX DISPLAY VALUE - GOES TO ENG IF LARGER
#define LARGEST_HEX_DISPLAY_NUMBER 2147483647         // int32_t
#define MAX_FIX_PRECISION          5
#define MIN_FIX_PRECISION          1
#define DEFAULT_FIX_PRECISION      2
#define DEFAULT_ENG_PRECISION      4

// STORAGE LOCAITONS
#define STO_RCL_LOCATIONS     100
#define EMPTY_LOCATION        0.0
#define CAL_SETTINGS_TO_READ  0xA5  // GENERIC VALUE - COULD BE ANYTHING - CHANCES OF NOT BEING WHAT I INTEND 1/256
#define CAL_MEMORY_MARKER     0x5A  // GENERIC VALUE - COULD BE ANYTHING - CHANCES OF NOT BEING WHAT I INTEND 1/256

// ENUMERATED TYPES AND STRUCTURES
typedef struct
  {
  uint8_t DisplayAs [MAX_DISPLAY_LENGTH]; // STRING REPRESENTATION OF THE NUMBER
  BOOLEAN Displayed;                      // IS THE VALUE TO BE DISPLAYED
  double NumericValue;                    // MUST BE GREATER THAN 0
  } Type_Register;

typedef struct
  {
  BOOLEAN Valid;                             // IS THE VALUE NUMERIC OR NOT
  uint8_t Description[MAX_DISPLAY_LENGTH];   // ERROR DESCRIPTION
  uint8_t Solution[MAX_DISPLAY_LENGTH];      // POSSIBLE SOLUTION
  uint8_t Result[MAX_DISPLAY_LENGTH];        // THE STRING
  uint8_t AudioErrorFileName[30];            // FILE NAME OF ASSOCIATED ERROR FILE
  uint8_t AudioPlayLevel;                    // PLAY LEVEL OF ASSOCIATED ERROR FILE
  double Value;                              // NUMERIC VALUE
  } Type_Numeric;

typedef struct
  {
  uint8_t BackLightTimeOut;
  uint8_t CalVerbose;
  uint8_t TimeToSleep;
  } Type_Setup;

typedef struct
  {
  BOOLEAN AlarmEnable;
  BOOLEAN AlarmEvent;
  BOOLEAN HC15C_InDeepSleep;
  } Type_TimeAlarm;

typedef struct
  {
  BOOLEAN Play;
  BOOLEAN Playing;
  BOOLEAN Pause;
  volatile uint32_t PlayTimeInSeconds;
  } Type_MusicPlayBack;
  

typedef struct
  {
  uint8_t CalError;                     // CAL ERROR SEE ENUM ATTENTION CODE
  uint8_t CalBase;                      // CAL BASE SEE ENUM NUMERIC BASE
  uint8_t FixPrecision;                 // FIX FORMAT PRECISION FOR BASE 10
  uint8_t EngPrecision;                 // ENG FORMAT PRECISIION FOR BASE 10
  uint8_t DisplayMode;                  // CAL DISPLAY SEE ENUM DISPLAY MODE
  uint8_t CalAngle;                     // CAL ANGLE SEE ENUM ANGLE MEASURE
  uint8_t CalMode;                      // CAL MODE SEE ENUM OPERATING_MODE
  uint16_t CalVerboseMask;              // CAL MODE FULL INTERACTIVE HELP
  BOOLEAN L_Shift;                      // LEFT SHFIT FLAG
  BOOLEAN R_Shift;                      // RIGHT SHIFT FLAG
  BOOLEAN USB_Link;                     // USB VCOM OK TO USE 
  BOOLEAN SleepDisable;                 // USED TO DISABLE SLEEP
  BOOLEAN ForceBatRead;                 // FORCE A BATTERY READ EVENT
  uint32_t Mask_KeyTouchA;              // TOUCH A KEYPAD MASKED KEYS
  uint32_t Mask_KeyTouchB;              // TOUCH A KEYPAD MASKED KEYS
  Type_Setup Setup;                     // CALCULATOR GENERAL SETTINGS
  Type_TimeAlarm TimeAlarm;             // TIME ALARM STATUS AND EVENTS
  Type_MusicPlayBack MusicPlayBack;     // MUSIC PLAY
  } Type_CalSettings;

typedef struct
  {
  double StoredValue[STO_RCL_LOCATIONS];               // 0-99 storage locations
  BOOLEAN STO_Event;
  BOOLEAN RCL_Event;
  } Type_STO_RCL;

typedef union
   {
   double DoubleValue;
   uint8_t ByteValue[sizeof(double)];
   } Union_DoubleInBytes;

enum DISPLAY_MODE
  {
  FIX,
  ENG,
  HEX
  };
enum NUMERIC_BASE
   {
   BASE_2,
   BASE_10,
   BASE_16
   };
 
enum ANGLE_MEASURE
  {
  RADS,
  DEGREES
  };
   
enum ATTENTION_CODE
  {
  NO_ERROR,
  ENTRY_ERROR,
  FORMAT_ERROR,
  MATH_ERROR
  };

enum OPERATING_MODE
  {
  CAL_MODE,
  CLOCK_MODE,
  ALARM_MODE,
  METER_MODE,
  OHMS_MODE,
  SD_LIST_MODE,
  SETUP_MODE,
  MUSIC_LIST_MODE
  };

typedef enum 
  {
  NO_SOUND,
  CORE_SOUND,
  BASIC_HELP,
  FULL_INTERACTIVE,
  } Type_CalVerbose;
  

// PROTOTYPES
void call_NumClick(uint8_t);
BOOLEAN call_IsNumericValue(const uint8_t *, uint8_t);
void call_FormatNumber(void);
void call_ShowEntryError(void);
void call_LoadCalSettings(void);
void call_StoreCalSettings(void);
void call_CalATN(void);
void call_ProcessStackDown(void);
void call_ProcessStackUp(double);
void call_PerformSTO(void);
void call_PerformRCL(void);
void call_CalMode(void);
void call_EndPresentMode(void);
void ctl_HabTaskSuspend(CTL_TASK_t *);
void ctl_HabTaskRun(CTL_TASK_t *);
void ctl_PostMessageByMemAllocate(Type_AudioQueueStruct *);

#endif