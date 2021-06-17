/*****************************************************************
 *
 * File name:         MATH_FUNCTIONS.H
 * Description:       Project definitions and function prototypes for use with MATH_FUNCTIONS.c
 * Author:            Hab S. Collector
 * Date:              10/15/2011
 * LAST EDIT:         8/18/2012
 * Hardware:               
 * Firmware Tool:     CrossStudio for ARM
 * Notes:             This file should be written as to not be dependent 
 *                    on other includes - everything these functions need should be passed to them
*****************************************************************/ 

#ifndef _MATH_FUNCTIONS_DEFINES
#define _MATH_FUNCTIONS_DEFINES


// INCLUDES
#include "HC15C_DEFINES.h"


// GLOBAL VARS

// EXTERNS DECLARED

// DEFINES
#define PI                  3.141592654
// CONVERSION CONSTANTS
#define MM_TO_MILS_CONVERT  39.37000
#define MILES_TO_KM_CONVERT 1.609344
#define KG_TO_LB_CONVERT    2.204623
#define GAL_TO_L_CONVERT    3.785412
#define IN_TO_CM_CONVERT    2.540000
#define M_TO_FT_CONVERT     3.280840

// STAT
#define MAX_STAT_SIZE 50

// ENUMERATED TYPES AND STRUCTURES
typedef struct
  {
  uint8_t ErrorDescription[20];
  uint8_t ErrorSolution[20];
  } Type_MathErrorDisplay;

// STAT
typedef struct
  {
  uint8_t Count;
  double X[MAX_STAT_SIZE];
  double Sum;
  double Mean;
  double DiffSquareSum;
  double StanDev;
  } Type_Stat;

// PROTOTYPES
// CORE SUPPORT 
void call_ShowMathError(void);
void call_ChkAndDisplayDrop(double);
void call_ChkAndDisplayRaise(double);
void call_singleLineConvertDivide(double);
void call_singleLineConvertMultiply(double);
static BOOLEAN call_CheckSinSpecialCase(const double *, double *);
static BOOLEAN call_CheckCosSpecialCase(const double *, double *);
static BOOLEAN call_CheckTanSpecialCase(const double *, double *);
// SHIFT KEYS
void call_LShiftClick(void);
void call_RShiftClick(void);
// REGISTER FUNCTIONS
void call_Enter(void);
void call_Drop(void);
void call_Flush(void);
void call_ClearX(void);
void call_XtoY(void);
void call_3to2(void);
// BASIC MATH
void call_Divide(void);
void call_Multiply(void);
void call_Subtract(void);
void call_Add(void);
// UNIT CONVERSIONS
void call_MILS_to_mm(void);
void call_mm_to_MILS(void);
void call_km_to_mi(void);
void call_mi_to_km(void);
void call_kg_to_lb(void);
void call_lb_to_kg(void);
void call_l_to_gal(void);
void call_gal_to_l(void);
void call_in_to_cm(void);
void call_cm_to_in(void);
void call_mph_to_kph(void);
void call_kph_to_mph(void);
void call_mps_to_fps(void);
void call_fps_to_mps(void);
void call_m_to_ft(void);
void call_ft_to_m(void);
void call_C_to_F(void);
void call_F_to_C(void);
// PI FUNCTIONS
void call_PiClick(void);
void call_2PiClick(void);
void call_2PiXClick(void);
// FORMATING
void call_FixMode(void);
void call_EngMode(void);
void call_Base10Mode(void);
void call_Base16Mode(void);
// ANGLE MEASURE
void call_RadMode(void);
void call_DegMode(void);
// ADVANCE MATH:
void call_abs(void);
void call_10ToX(void);
void call_logX(void);
void call_eToX(void);
void call_lnX(void);
void call_sqrtX(void);
void call_Xsqr(void);
void call_YtoX(void);
void call_YtoOneOverX(void);
void call_OneOverX(void);
void call_Xfactorial(void);
void call_sinX(void);
void call_asinX(void);
void call_cosX(void);
void call_acosX(void);
void call_tanX(void);
void call_atanX(void);
// REACTANCE AND POLAR FORM
void call_jXC(void);
void call_jXL(void);
void call_RtoP(void);
void call_PtoR(void);
// STATISTICS
void call_StatClear(void);
void call_StatAdd(void);
void call_Mean(void);
void call_StanDev(void);
uint8_t call_statCalUpdate(double);
void call_DeltaPercent(void);


#endif