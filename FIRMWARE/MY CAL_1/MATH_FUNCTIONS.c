/*****************************************************************
 *
 * File name:       MATH_FUNCTIONS.C
 * Description:     Functions MATH RELATED operations used to support calculator operations of the HC15C
 * Author:          Hab S. Collector
 * Date:            12/15/11
 * LAST EDIT:       8/18/2012
 * Hardware:        NXP LPC1769
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 
 
#include "MATH_FUNCTIONS.H"
#include "CORE_FUNCTIONS.H"
#include "TOUCH_TASKS.H"
#include "DIP204.H"
#include "lpc_types.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

// GLOBAL VARS
double LastX;
// TYPES
Type_MathErrorDisplay MathError;
Type_Stat Stat;

// EXTERN VARS
extern uint8_t str_InputLine[MAX_DISPLAY_LENGTH];
extern Type_Register RegisterValue[TOTAL_REGISTERS];
extern Type_Numeric NumericValue;
extern Type_CalSettings CalSettings;
extern BOOLEAN bln_LineLoaded;





/*************************************************************************
 * Function Name: call_ShowMathError
 * Parameters: void
 * Return: void
 *
 * Description: Handles a math error condition.  Displays error event info from
 * global structure NumericValue, sets global CalError and places the calculator 
 * key input mask to the ATN Key
 * STEP 1: Setup Error conditions
 * STEP 2: Display the error
 * STEP 3: Play AI help
 *************************************************************************/
 void call_ShowMathError(void)
 {
 
 uint8_t LineText[20];
 Type_AudioQueueStruct AudioQueueStruct;
 
 // STEP 1
 CalSettings.CalError = MATH_ERROR;
 CalSettings.Mask_KeyTouchA = CalSettings.Mask_KeyTouchB = NO_KEYS_SELECTED;
 CalSettings.Mask_KeyTouchA = MASK_ATN_KEY;
 
 // STEP 2
 DIP204_clearDisplay();
 sprintf(LineText,"MATH ERROR:");
 DIP204_txt_engine(LineText, 1, 0, strlen(LineText));
 DIP204_txt_engine(MathError.ErrorDescription, 2, 3, strlen(MathError.ErrorDescription));
 DIP204_txt_engine(MathError.ErrorSolution, 3, 3, strlen(MathError.ErrorSolution));
 DIP204_ICON_set(ICON_ALERT, ICON_BLINK);
 DIP204_set_cursor(CURSOR_OFF);
 
 // STEP 3
 strcpy(AudioQueueStruct.FileName, NumericValue.AudioErrorFileName);
 AudioQueueStruct.PlayLevel = NumericValue.AudioPlayLevel;
 ctl_PostMessageByMemAllocate(&AudioQueueStruct);
 
 } // END OF call_ShowMathError




/*************************************************************************
 * Function Name: call_ChkAndDisplayDrop
 * Parameters: double
 * Return: void
 *
 * Description: Checks for math result error on math functions that involve two stack
   variables - X and Y or input line and X.  Handles a math error condition by 
   Displaying error event info from global structure MathError.  If no error, sets 
   the stack to be loaded Register 0 will always have the result.  
 * STEP 1: Check for Error - If error show error and exit
 * STEP 2: If no error update Reg 0 and update the stack
 *************************************************************************/
 void call_ChkAndDisplayDrop(double Ans)
 {
  
 // STEP 1 
 if (isinf(Ans) || isnan(Ans))
   {
    strcpy(MathError.ErrorDescription,"Invalid result");
    strcpy(MathError.ErrorSolution,"Unsupported math");
    strcpy(NumericValue.AudioErrorFileName, I_AND_I_ERROR_WAV);
    NumericValue.AudioPlayLevel = BASIC_HELP;
    call_ShowMathError();
    return;
   }
 
 // STEP 2
 RegisterValue[0].NumericValue = Ans;
 call_ProcessStackDown(); 
   
 } // END OF call_ChkAndDisplayDrop




/*************************************************************************
 * Function Name: call_ChkAndDisplayRaise
 * Parameters: double
 * Return: void
 *
 * Description: Checks for math result error on math functions that involve a single 
   variable - X or input line.  Handles a math error condition by 
   Displaying error event info from global structure MathError.  If no error, sets 
   the stack to be loaded Register 0 will always have the result.  
 * STEP 1: Check for Error - If error show error and exit
 * STEP 2: If no error update Reg 0 and update the stack
 *************************************************************************/
 void call_ChkAndDisplayRaise(double Ans)
 {
  
 // STEP 1 
 if (isinf(Ans) || isnan(Ans))
   {
    strcpy(MathError.ErrorDescription,"Invalid result");
    strcpy(MathError.ErrorSolution,"Unsupported math");
    strcpy(NumericValue.AudioErrorFileName, I_AND_I_ERROR_WAV);
    NumericValue.AudioPlayLevel = BASIC_HELP;
    call_ShowMathError();
    return;
   }
 
 // STEP 2
 call_ProcessStackUp(Ans); 
   
 } // END OF call_ChkAndDisplayRaise




/*************************************************************************
 * Function Name: call_singleLineConvertMultiply
 * Parameters: double
 * Return: void
 *
 * Description: Many conversions operate on a single variable.  That is a defined
 * converted value operates on the X or line input.  The function is passed the conversion
 * and multiply to convert the value
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_singleLineConvertMultiply(double MultiplyConversion)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = NumericValue.Value * MultiplyConversion;
   }
 else
   TempAns = RegisterValue[0].NumericValue * MultiplyConversion;
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_singleLineConvertMultiply




/*************************************************************************
 * Function Name: call_singleLineConvertDivide
 * Parameters: double
 * Return: void
 *
 * Description: Many conversions operate on a single variable.  That is a defined
 * converted value operates on the X or line input.  The function is passed the conversion
 * and divided to convert the value
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_singleLineConvertDivide(double DivideConversion)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = NumericValue.Value / DivideConversion;
   }
 else
   TempAns = RegisterValue[0].NumericValue / DivideConversion;
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_singleLineConvertDivide
 



/*************************************************************************
 * Function Name: call_LShiftClick
 * Parameters: void
 * Return: void
 *
 * Description: Use to access the Left Shift Key Functions.  Sets a flag and sets an ICON.
 * The shift key can also toggle itself.  If the R Shift Key is on - turn off
 * STEP 1: Toggle the ShiftKey
 *************************************************************************/
 void call_LShiftClick(void)
 {
 
 // STEP 1
 if (!CalSettings.L_Shift)
   {
   CalSettings.L_Shift = TRUE;
   CalSettings.R_Shift = FALSE;
   DIP204_ICON_set(ICON_LEFT_ARROW, ICON_ON);
   DIP204_ICON_set(ICON_RIGHT_ARROW, ICON_OFF);
   }
 else
   {
   CalSettings.L_Shift = FALSE;
   DIP204_ICON_set(ICON_LEFT_ARROW, ICON_OFF);
   }
 
 } // END OF call_LShiftClick




/*************************************************************************
 * Function Name: call_RShiftClick
 * Parameters: void
 * Return: void
 *
 * Description: Use to access the Right Shift Key Functions.  Sets a flag and sets an ICON.
 * The shift key can also toggle itself.  If the L Shift Key is on - turn off
 * STEP 1: Toggle the ShiftKey
 *************************************************************************/
 void call_RShiftClick(void)
 {
 
 // STEP 1
 if (!CalSettings.R_Shift)
   {
   CalSettings.R_Shift = TRUE;
   CalSettings.L_Shift = FALSE;
   DIP204_ICON_set(ICON_RIGHT_ARROW, ICON_ON);
   DIP204_ICON_set(ICON_LEFT_ARROW, ICON_OFF);
   }
 else
   {
   CalSettings.R_Shift = FALSE;
   DIP204_ICON_set(ICON_RIGHT_ARROW, ICON_OFF);
   }
 
 } // END OF call_RShiftClick




/*************************************************************************
 * Function Name: call_Enter
 * Parameters: void
 * Return: void
 *
 * Description: This function is the key to RPN.  The Enter key loads an unloaded number unto
 * the stack, pushing the stack upward (0 to 1, 1 to 2, 2 to 3, 3 to 4).  If there is no unloaded
 * number then the contents of 0 is copied and the stack is moved upward with the value of register
 * 4 being lost.  In the case of last X, the previous X value (register 0) is pushed onto the stack
 * pushing the stack upward.  Once again the contents of register 4 is lost
 * STEP 1: If number loaded (then the number is good) - copy to 0 register (X) and push up the stack
 * STEP 2: If number not loaded then: check if number valid.  If not valid display error and exit.
 * If valid load unto the stack
 *************************************************************************/
void call_Enter(void)
 {
 
 // STEP 1
 if (bln_LineLoaded)
   {
   RegisterValue[3].NumericValue = RegisterValue[2].NumericValue;
   RegisterValue[3].Displayed = RegisterValue[2].Displayed;
   RegisterValue[2].NumericValue = RegisterValue[1].NumericValue;
   RegisterValue[2].Displayed = RegisterValue[1].Displayed;
   RegisterValue[1].NumericValue = RegisterValue[0].NumericValue;
   RegisterValue[1].Displayed = RegisterValue[0].Displayed;
   LastX = RegisterValue[0].NumericValue;
   call_FormatNumber();
   }
 else
   {
   // STEP 2
   // DETERMINE IF NUMBER VALID - IF SO LOAD STACK IF NOT SHOW ENTRY ERROR AND EXIT
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   else
     {
     RegisterValue[3].NumericValue = RegisterValue[2].NumericValue;
     RegisterValue[3].Displayed = RegisterValue[2].Displayed;
     RegisterValue[2].NumericValue = RegisterValue[1].NumericValue;
     RegisterValue[2].Displayed = RegisterValue[1].Displayed;
     RegisterValue[1].NumericValue = RegisterValue[0].NumericValue;
     RegisterValue[1].Displayed = RegisterValue[0].Displayed;
     RegisterValue[0].NumericValue = NumericValue.Value;
     RegisterValue[0].Displayed = TRUE;
     LastX = RegisterValue[0].NumericValue;
     bln_LineLoaded = TRUE;
     call_FormatNumber();
     }
   }
 
 } // END OF FUNCTION call_Enter




/*************************************************************************
 * Function Name: call_Drop
 * Parameters: void
 * Return: void
 *
 * Description: Drops the stack - if not loaded drops the entry value
 * STEP 1: 
 *************************************************************************/
void call_Drop(void)
 {
 // STEP 1
 if(bln_LineLoaded)
   {
   RegisterValue[0].NumericValue = RegisterValue[1].NumericValue;
   RegisterValue[0].Displayed = RegisterValue[1].Displayed;
   RegisterValue[1].NumericValue = RegisterValue[2].NumericValue;
   RegisterValue[1].Displayed = RegisterValue[2].Displayed;
   RegisterValue[2].NumericValue = RegisterValue[3].NumericValue;
   RegisterValue[2].Displayed = RegisterValue[3].Displayed;
   RegisterValue[3].NumericValue = 0.0;
   RegisterValue[3].Displayed = FALSE;
   }
 bln_LineLoaded = TRUE;
 call_FormatNumber();
 
 } // END OF call_Drop




/*************************************************************************
 * Function Name: call_Flush
 * Parameters: void
 * Return: void
 *
 * Description: Sets All stack register to zero and non-displayed
 * STEP 1: Flush the stack
 * STEP 2: Display
 *************************************************************************/
 void call_Flush(void)
 {
 // STEP 1
 for (uint8_t RegCount = 0; RegCount < TOTAL_REGISTERS; RegCount++)
   {
   strcpy(RegisterValue[RegCount].DisplayAs,"");
   RegisterValue[RegCount].Displayed = FALSE;
   RegisterValue[RegCount].NumericValue = 0.0;
   }
 
 // STEP 2
 call_FormatNumber();  
   
 } // END OF call_Flush
 
 
 
 
 /*************************************************************************
 * Function Name: call_clearX
 * Parameters: void
 * Return: void
 *
 * Description: Clears the X register to 0 if loaded.  If not loaded just clears
 * STEP 1: Clear
 *************************************************************************/
 void call_ClearX(void)
 {
 // STEP 1
 if(bln_LineLoaded)
   {
   RegisterValue[0].NumericValue = 0;
   RegisterValue[0].Displayed = TRUE;
   call_FormatNumber();
   }
 else
   {
   bln_LineLoaded = TRUE;
   call_FormatNumber();
   }
   
 } // END OF call_clearX
 
 
 
 
/*************************************************************************
 * Function Name: call_Divide
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Show result (includes dropping stack)
 *************************************************************************/
void call_Divide(void)
 {
 
 double TempAns;

 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // CHECK FOR DIVIDE BY ZERO.
   if (NumericValue.Value == 0)
     {
     strcpy(MathError.ErrorDescription,"Divide By 0");
     strcpy(MathError.ErrorSolution,"Causes infinity");
     strcpy(NumericValue.AudioErrorFileName, I_AND_I_ERROR_WAV);
     NumericValue.AudioPlayLevel = BASIC_HELP;
     call_ShowMathError();
     return;
     }
   // MATH
   TempAns = RegisterValue[0].NumericValue / NumericValue.Value;
   }
 else
   {
   // CHECK FOR DIVIDE BY ZERO
   if (RegisterValue[0].NumericValue == 0)
     {
     strcpy(MathError.ErrorDescription,"Divide By 0");
     strcpy(MathError.ErrorSolution,"Causes infinity");
     strcpy(NumericValue.AudioErrorFileName, I_AND_I_ERROR_WAV);
     NumericValue.AudioPlayLevel = BASIC_HELP;
     call_ShowMathError();
     return;
     }
   TempAns = RegisterValue[1].NumericValue / RegisterValue[0].NumericValue;
   }
 
 // STEP 2
 call_ChkAndDisplayDrop(TempAns);
 
 } // END OF call_Divide




/*************************************************************************
 * Function Name: call_Multiply
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_Multiply(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = RegisterValue[0].NumericValue * NumericValue.Value;
   }
 else
   TempAns = RegisterValue[1].NumericValue * RegisterValue[0].NumericValue;
 
 // STEP 2
 call_ChkAndDisplayDrop(TempAns);
 
 } // END OF call_Multiply




/*************************************************************************
 * Function Name: call_Subtract
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_Subtract(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = RegisterValue[0].NumericValue - NumericValue.Value;
   }
 else
   TempAns = RegisterValue[1].NumericValue - RegisterValue[0].NumericValue;
 
 // STEP 2
 call_ChkAndDisplayDrop(TempAns);
 
 } // END OF call_Subtract




/*************************************************************************
 * Function Name: call_Add
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_Add(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = RegisterValue[0].NumericValue + NumericValue.Value;
   }
 else
   TempAns = RegisterValue[1].NumericValue + RegisterValue[0].NumericValue;
 
 // STEP 2
 call_ChkAndDisplayDrop(TempAns);
 
 } // END OF call_Add




/*************************************************************************
 * Function Name: call_mm_to_MILS
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_mm_to_MILS(void)
 {
 
 call_singleLineConvertMultiply(MM_TO_MILS_CONVERT);
 
 } // END OF call_mm_to_MILS




/*************************************************************************
 * Function Name: call_mm_to_MILS
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_MILS_to_mm(void)
 {
 
 call_singleLineConvertDivide(MM_TO_MILS_CONVERT);
 
 } // END OF call_MILS_to_mm




/*************************************************************************
 * Function Name: call_km_to_mi
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_km_to_mi(void)
 {
 
 call_singleLineConvertDivide(MILES_TO_KM_CONVERT);
 
 } // END OF call_km_to_mi




/*************************************************************************
 * Function Name: call_mi_to_km
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_mi_to_km(void)
 {
 
 call_singleLineConvertMultiply(MILES_TO_KM_CONVERT);
 
 } // END OF call_mi_to_km




/*************************************************************************
 * Function Name: call_kg_to_lb
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_kg_to_lb(void)
 {
 
 call_singleLineConvertMultiply(KG_TO_LB_CONVERT);
 
 } // END OF call_kg_to_lb




/*************************************************************************
 * Function Name: call_lb_to_kg
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_lb_to_kg(void)
 {
 
 call_singleLineConvertDivide(KG_TO_LB_CONVERT);
 
 } // END OF call_lb_to_kg




/*************************************************************************
 * Function Name: call_gal_to_l
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_gal_to_l(void)
 {
 
 call_singleLineConvertMultiply(GAL_TO_L_CONVERT);
 
 } // END OF call_gal_to_l




/*************************************************************************
 * Function Name: call_l_to_gal
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_l_to_gal(void)
 {
 
 call_singleLineConvertDivide(GAL_TO_L_CONVERT);
 
 } // END OF call_l_to_gal




/*************************************************************************
 * Function Name: call_in_to_cm
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_in_to_cm(void)
 {
 
 call_singleLineConvertMultiply(IN_TO_CM_CONVERT);
 
 } // END OF call_in_to_cm




/*************************************************************************
 * Function Name: call_cm_to_in
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_cm_to_in(void)
 {
 
 call_singleLineConvertDivide(IN_TO_CM_CONVERT);
 
 } // END OF call_cm_to_in




/*************************************************************************
 * Function Name: call_mph_to_kph
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_mph_to_kph(void)
 {
 
 call_singleLineConvertMultiply(MILES_TO_KM_CONVERT);
 
 } // END OF call_mph_to_kph




/*************************************************************************
 * Function Name: call_kph_to_mph
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_kph_to_mph(void)
 {
 
 call_singleLineConvertDivide(MILES_TO_KM_CONVERT);
 
 } // END OF call_kph_to_mph




/*************************************************************************
 * Function Name: call_mps_to_fps
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_mps_to_fps(void)
 {
 
 call_singleLineConvertMultiply(M_TO_FT_CONVERT);
 
 } // END OF call_mps_to_fps




/*************************************************************************
 * Function Name: call_fps_to_mps
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_fps_to_mps(void)
 {
 
 call_singleLineConvertDivide(M_TO_FT_CONVERT);
 
 } // END OF call_fps_to_mps




/*************************************************************************
 * Function Name: call_m_to_ft
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_m_to_ft(void)
 {
 
 call_singleLineConvertMultiply(M_TO_FT_CONVERT);
 
 } // END OF call_m_to_ft




/*************************************************************************
 * Function Name: call_ft_to_m
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_ft_to_m(void)
 {
 
 call_singleLineConvertDivide(M_TO_FT_CONVERT);
 
 } // END OF call_ft_to_m




/*************************************************************************
 * Function Name: call_F_to_C
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_F_to_C(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = (NumericValue.Value - 32.0) / 1.8;
   }
 else
   TempAns = (RegisterValue[0].NumericValue - 32.0) / 1.8;
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_F_to_C




/*************************************************************************
 * Function Name: call_C_to_F
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 *************************************************************************/
 void call_C_to_F(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = (NumericValue.Value * 1.8) + 32.0;
   }
 else
   TempAns = (RegisterValue[0].NumericValue * 1.8) + 32.0;
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_C_to_F




/*************************************************************************
 * Function Name: call_PiClick
 * Parameters: void
 * Return: void
 *
 * Description: Places the value PI onto the stack. If the value is loaded PI
 * is placed in X and the stack is pushed up.  If unloaded PI replaces the unloaded
 * value and pushes onto the stack
 * STEP 1: If unloaded or loaded push value onto stack as described above
 * STEP 2: Display the stack
 *************************************************************************/
 void call_PiClick(void)
 {
 
 // STEP 1
 if (!bln_LineLoaded)
    call_ChkAndDisplayRaise(PI);
 else
   {
   RegisterValue[3].NumericValue = RegisterValue[2].NumericValue;
   RegisterValue[3].Displayed = RegisterValue[2].Displayed;
   RegisterValue[2].NumericValue = RegisterValue[1].NumericValue;
   RegisterValue[2].Displayed = RegisterValue[1].Displayed;
   RegisterValue[1].NumericValue = RegisterValue[0].NumericValue;
   RegisterValue[1].Displayed = RegisterValue[0].Displayed;
   RegisterValue[0].NumericValue = PI;
   RegisterValue[0].Displayed = TRUE;
   }
 
 // STEP 2
 call_FormatNumber();
 
 } // END OF call_PiClick




/*************************************************************************
 * Function Name: call_2PiClick
 * Parameters: void
 * Return: void
 *
 * Description: Places the value 2PI onto the stack. If the value is loaded PI
 * is placed in X and the stack is pushed up.  If unloaded PI replaces the unloaded
 * value and pushes onto the stack
 * STEP 1: If unloaded or loaded push value onto stack as described above
 * STEP 2: Display the stack
 *************************************************************************/
 void call_2PiClick(void)
 {
 
 // STEP 1
 if (!bln_LineLoaded)
    call_ChkAndDisplayRaise(2.0 * PI);
 else
   {
   RegisterValue[3].NumericValue = RegisterValue[2].NumericValue;
   RegisterValue[3].Displayed = RegisterValue[2].Displayed;
   RegisterValue[2].NumericValue = RegisterValue[1].NumericValue;
   RegisterValue[2].Displayed = RegisterValue[1].Displayed;
   RegisterValue[1].NumericValue = RegisterValue[0].NumericValue;
   RegisterValue[1].Displayed = RegisterValue[0].Displayed;
   RegisterValue[0].NumericValue = (2.0 * PI);
   RegisterValue[0].Displayed = TRUE;
   }
 
 // STEP 2
 call_FormatNumber();
 
 } // END OF call_2PiClick




/*************************************************************************
 * Function Name: call_2PiXClick
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the register is pushed up.  If loaded the function is performed on the
 * X - the stack is not pushed.
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 *************************************************************************/
 void call_2PiXClick(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = 2.0 * PI * NumericValue.Value;
   call_ProcessStackUp(TempAns);
   }
 else
   {
   RegisterValue[0].NumericValue = 2.0 * PI * RegisterValue[0].NumericValue;
   call_FormatNumber();
   }
 
 } // END OF call_2PiXClick




/*************************************************************************
 * Function Name: call_FixMode
 * Parameters: void
 * Return: void
 *
 * Description: Places the calculator in fix mode with a precision of 1 to 5
 * based on the input string or X register.  If X register is not in the valid range
 * then a default fix value is loaded.  Unloaded numbers will be dropped.
 * STEP 1: Set the display value in accordance with the input line or X register - 
 * also determine if value is valid, if not use default.
 * STEP 2: Display
 *************************************************************************/
 void call_FixMode(void)
 {
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // CHECK IF VALUE LEGAL
   if (((uint8_t)NumericValue.Value <= MAX_FIX_PRECISION) && ((uint8_t)NumericValue.Value > MIN_FIX_PRECISION))
     CalSettings.FixPrecision = (uint8_t)NumericValue.Value;
   else
     CalSettings.FixPrecision = DEFAULT_FIX_PRECISION;
   }
 else
   {
   if (((uint8_t)RegisterValue[0].NumericValue <= MAX_FIX_PRECISION) && ((uint8_t)RegisterValue[0].NumericValue >= MIN_FIX_PRECISION))
     CalSettings.FixPrecision = (uint8_t)RegisterValue[0].NumericValue;
   else
     CalSettings.FixPrecision = DEFAULT_FIX_PRECISION;
   }
 
 // STEP 2
 CalSettings.CalBase = BASE_10;
 CalSettings.DisplayMode = FIX;
 call_FormatNumber();
 
 } // END OF call_FixMode




/*************************************************************************
 * Function Name: call_EngMode
 * Parameters: void
 * Return: void
 *
 * Description: Places the calculator in fix mode with a precision of 1 to 5
 * based on the input string or X register.  If X register is not in the valid range
 * then a default fix value is loaded.  Unloaded numbers will be dropped.
 * STEP 1: Set the display value in accordance with the input line or X register - 
 * also determine if value is valid, if not use default.
 * STEP 2: Display
 *************************************************************************/
 void call_EngMode(void)
 {
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // CHECK IF VALUE LEGAL
   if (((uint8_t)NumericValue.Value <= MAX_FIX_PRECISION) && ((uint8_t)NumericValue.Value > MIN_FIX_PRECISION))
     CalSettings.EngPrecision = (uint8_t)NumericValue.Value;
   else
     CalSettings.EngPrecision = DEFAULT_ENG_PRECISION;
   }
 else
   {
   if (((uint8_t)RegisterValue[0].NumericValue <= MAX_FIX_PRECISION) && ((uint8_t)RegisterValue[0].NumericValue >= MIN_FIX_PRECISION))
     CalSettings.EngPrecision = (uint8_t)RegisterValue[0].NumericValue;
   else
     CalSettings.EngPrecision = DEFAULT_ENG_PRECISION;
   }
 
 // STEP 2
 CalSettings.CalBase = BASE_10;
 CalSettings.DisplayMode = ENG;
 call_FormatNumber();
 
 } // END OF call_EngMode




/*************************************************************************
 * Function Name: call_Base10Mode
 * Parameters: void
 * Return: void
 *
 * Description: Places the calculator in Base10 mode.  Has not effect on loaded
 * numbers.  Unloaded numbers will be dropped from the stack.
 * STEP 1: Sets base 10 mode
 * STEP 2: Display
 *************************************************************************/
 void call_Base10Mode(void)
 {
 
 Type_AudioQueueStruct AudioQueueStruct;
 
 // STEP 1
 CalSettings.CalBase = BASE_10;
 // STEP 2
 call_FormatNumber();
 strcpy(AudioQueueStruct.FileName, DEC_INPUT_WAV);
 AudioQueueStruct.FullInteractiveMask = DEC_INPUT_MASK;
 AudioQueueStruct.PlayLevel = FULL_INTERACTIVE;
 ctl_PostMessageByMemAllocate(&AudioQueueStruct);
 
 } // END OF call_Base10Mode




/*************************************************************************
 * Function Name: call_Base16Mode
 * Parameters: void
 * Return: void
 *
 * Description: Places the calculator in Base16 mode.  Has not effect on loaded
 * numbers.  Unloaded numbers will be dropped from the stack.
 * STEP 1: Sets base 16 mode
 * STEP 2: Display
 *************************************************************************/
 void call_Base16Mode(void)
 {
 
 Type_AudioQueueStruct AudioQueueStruct;
 
 // STEP 1
 CalSettings.CalBase = BASE_16;
 // STEP 2
 call_FormatNumber();
 strcpy(AudioQueueStruct.FileName, HEX_INPUT_WAV);
 AudioQueueStruct.FullInteractiveMask = HEX_INPUT_MASK;
 AudioQueueStruct.PlayLevel = FULL_INTERACTIVE;
 ctl_PostMessageByMemAllocate(&AudioQueueStruct);
 
 } // END OF call_Base16Mode




/*************************************************************************
 * Function Name: call_abs
 * Parameters: void
 * Return: void
 *
 * Description: Performs abs value on loaded X register only.  Unloaded values
 * are discarded.
 * STEP 1: ABS
 * STEP 2: Display
 *************************************************************************/
 void call_abs(void)
 {
 
 // STEP 1
 if (RegisterValue[0].NumericValue < 0)
   {
   RegisterValue[0].NumericValue *= -1.0;
   }

 // STEP 2
 call_FormatNumber();
 
 } // END OF call_abs




/*************************************************************************
 * Function Name: call_XtoY
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 *************************************************************************/
 void call_XtoY(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = RegisterValue[0].NumericValue;
   RegisterValue[0].NumericValue = NumericValue.Value;
   call_ProcessStackUp(TempAns);
   }
 else
   {
   TempAns = RegisterValue[1].NumericValue;
   RegisterValue[1].NumericValue = RegisterValue[0].NumericValue;
   RegisterValue[0].NumericValue = TempAns;
   call_FormatNumber();
   }
 
 } // END OF call_XtoY




/*************************************************************************
 * Function Name: call_3to2
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on loaded numbers.  Swap Reg 3 with Reg 2.
 * STEP 1: Perform on unloded
 *************************************************************************/
 void call_3to2(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (bln_LineLoaded)
   {
   TempAns = RegisterValue[3].NumericValue;
   RegisterValue[3].NumericValue = RegisterValue[2].NumericValue;
   RegisterValue[2].NumericValue = TempAns;
   call_FormatNumber();
   }

 } // END OF call_3to2




/*************************************************************************
 * Function Name: call_10ToX
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_10ToX(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = pow(10,NumericValue.Value);
   }
 else
   TempAns = pow(10,RegisterValue[0].NumericValue);
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_10ToX




/*************************************************************************
 * Function Name: call_logX
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_logX(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = log10(NumericValue.Value);
   }
 else
   TempAns = log10(RegisterValue[0].NumericValue);
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_logX




/*************************************************************************
 * Function Name: call_eToX
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_eToX(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = exp(NumericValue.Value);
   }
 else
   TempAns = exp(RegisterValue[0].NumericValue);
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_eToX




/*************************************************************************
 * Function Name: call_lnX
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_lnX(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = log(NumericValue.Value);
   }
 else
   TempAns = log(RegisterValue[0].NumericValue);
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_lnX


 

/*************************************************************************
 * Function Name: call_Xsqr
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_Xsqr(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = pow(NumericValue.Value, 2);
   }
 else
   TempAns = pow(RegisterValue[0].NumericValue, 2);
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_Xsqr



/*************************************************************************
 * Function Name: call_sqrtX
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_sqrtX(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = sqrt(NumericValue.Value);
   }
 else
   TempAns = sqrt(RegisterValue[0].NumericValue);
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_sqrtX




/*************************************************************************
 * Function Name: call_YtoX
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_YtoX(void)
 {
 
 double TempAns;
 
// STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = pow(RegisterValue[0].NumericValue, NumericValue.Value);
   }
 else
   TempAns = pow(RegisterValue[1].NumericValue, RegisterValue[0].NumericValue);
 
 // STEP 2
 call_ChkAndDisplayDrop(TempAns);
 
 } // END OF call_YtoX



/*************************************************************************
 * Function Name: call_YtoOneOverX
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_YtoOneOverX(void)
 {
 
 double TempAns;
 
// STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = pow(RegisterValue[0].NumericValue, (1.0/NumericValue.Value));
   }
 else
   TempAns = pow(RegisterValue[1].NumericValue, (1.0/RegisterValue[0].NumericValue));
 
 // STEP 2
 call_ChkAndDisplayDrop(TempAns);
 
 } // END OF call_YtoOneOverX




/*************************************************************************
 * Function Name: call_OneOverX
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_OneOverX(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = 1.0/NumericValue.Value;
   }
 else
   TempAns = 1.0/RegisterValue[0].NumericValue;
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_OneOverX



/*************************************************************************
 * Function Name: call_Xfactorial
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_Xfactorial(void)
 {
 
 double TempAns = 1.0;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH CHECK FOR ERRORS: NEGATIVE
   if (NumericValue.Value < 0)
     {
     strcpy(MathError.ErrorDescription, "Negative X!");
     strcpy(MathError.ErrorSolution, "Undefined X!");
     call_ShowMathError();
     return;
     }
   // MATH CHECK FOR ERRORS: NON INTEGER
   if ((NumericValue.Value - (int32_t)NumericValue.Value) != 0.0)
     {
     strcpy(MathError.ErrorDescription, "Improper number");
     strcpy(MathError.ErrorSolution, "Only INT32 valid");
     call_ShowMathError();
     return;
     }
   // MATH
   if (NumericValue.Value == 0.0)
     TempAns = 1.0;
   while (NumericValue.Value > 1)
     {
       TempAns *= NumericValue.Value;
       NumericValue.Value--;
     }
   }
 else // Loaded number
   {
   // MATH CHECK FOR ERRORS: NEGATIVE
   if (RegisterValue[0].NumericValue < 0)
     {
     strcpy(MathError.ErrorDescription, "Negative X!");
     strcpy(MathError.ErrorSolution, "Undefined X!");
     call_ShowMathError();
     return;
     }
   // MATH CHECK FOR ERRORS: NON INTEGER
   if ((RegisterValue[0].NumericValue - (int32_t)RegisterValue[0].NumericValue) != 0.0)
     {
     strcpy(MathError.ErrorDescription, "Improper number");
     strcpy(MathError.ErrorSolution, "Only INT32 valid");
     call_ShowMathError();
     return;
     }
   // FACTORIAL MATH
   if (RegisterValue[0].NumericValue == 0.0)
     TempAns = 1.0;
   while (RegisterValue[0].NumericValue > 1)
     {
       TempAns *= RegisterValue[0].NumericValue;
       RegisterValue[0].NumericValue--;
     }
   }
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_Xfactorial




/*************************************************************************
 * Function Name: call_RadMode
 * Parameters: void
 * Return: void
 *
 * Description: Places the calculator in Rad mode.  Has no effect on loaded or unloaded
 * numbers.
 * STEP 1: Set cal settings structure
 * STEP 2: Set Icon (temp ie radians ie heat)
 *************************************************************************/
 void call_RadMode(void)
 {
   Type_AudioQueueStruct AudioQueueStruct;
   
   // STEP 1
   CalSettings.CalAngle = RADS;
   
   // STEP 2
   DIP204_ICON_set(ICON_TEMP, ICON_ON);
   strcpy(AudioQueueStruct.FileName, ANGULAR_RAD_WAV);
   AudioQueueStruct.FullInteractiveMask = ANGULAR_RAD_MASK;
   AudioQueueStruct.PlayLevel = FULL_INTERACTIVE;
   ctl_PostMessageByMemAllocate(&AudioQueueStruct);
 
 } // END OF call_RadMode




/*************************************************************************
 * Function Name: call_RadMode
 * Parameters: void
 * Return: void
 *
 * Description: Places the calculator in Rad mode.  Has no effect on loaded or unloaded
 * numbers.
 * STEP 1: Set cal settings structure
 * STEP 2: Set Icon (temp ie radians ie heat)
 *************************************************************************/
 void call_DegMode(void)
 {
   
   Type_AudioQueueStruct AudioQueueStruct;
   
   // STEP 1
   CalSettings.CalAngle = DEGREES;
   
   // STEP 2
   DIP204_ICON_set(ICON_TEMP, ICON_OFF);
   strcpy(AudioQueueStruct.FileName, ANGULAR_DEGREE_WAV);
   AudioQueueStruct.FullInteractiveMask = ANGULAR_DEGREE_MASK;
   AudioQueueStruct.PlayLevel = FULL_INTERACTIVE;
   ctl_PostMessageByMemAllocate(&AudioQueueStruct);
 
 } // END OF call_RadMode




/*************************************************************************
 * Function Name: call_sinX
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: Check for special case
 * STEP 2: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 3: Check and Display the answer
 *************************************************************************/
 void call_sinX(void)
 {
 
 double TempAns,
        AnsShouldBe;
 
 // STEP 2
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // CHECK SPECIAL CASE
   if (call_CheckSinSpecialCase(&NumericValue.Value, &AnsShouldBe))
     TempAns = AnsShouldBe;
   // MATH
   else
     {
     if (CalSettings.CalAngle == DEGREES)
       TempAns = sin(NumericValue.Value*PI/180.0);
     else
       TempAns = sin(NumericValue.Value);
     }
   }
 else
   {
   // CHECK SPECIAL CASE
   if (call_CheckSinSpecialCase(&RegisterValue[0].NumericValue, &AnsShouldBe))
     TempAns = AnsShouldBe;
   // MATH
   else
     {
     if (CalSettings.CalAngle == DEGREES)
       TempAns = sin(RegisterValue[0].NumericValue*PI/180.0);
     else
       TempAns = sin(RegisterValue[0].NumericValue);
     }
   }
 
 // STEP 3
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_sinX




/*************************************************************************
 * Function Name: call_CheckSinSpecialCase
 * Parameters: const double *, double *
 * Return: BOOLEAN
 *
 * Description: Checks for trig sin special cases of 0, and 180 degrees (and factors).  These
 * special cases are not covered due to computer estimations.  The funciton is passed
 * the value to check and a pointer in which the correct answer (if a special case) is to
 * be loaded
 * STEP 1: Work in degree mode
 * STEP 2: Check for the error condition
 *************************************************************************/
 static BOOLEAN call_CheckSinSpecialCase(const double *Value, double *Answer)
 {
 
 double ChkValue = *Value;
 
 // STEP 1
 // CONVERT TO DEGREES
 if (CalSettings.CalAngle != DEGREES)
   {
   ChkValue = ChkValue * 180.0/PI;
   }
 
 // STEP 2
 // CHECK IF A MULTIPLE OF 0 OR 180 - WORK IN ABSOLUTES
 if (ChkValue < 0)
   ChkValue *= -1.0;
 if ((ChkValue - (uint32_t)ChkValue) == 0)
   {
   if ((((uint32_t)ChkValue % 180) == 0) || ChkValue == 0)
     {
     *Answer = 0;
     return(TRUE);
     }
   }
 
 return(FALSE);
 } // END OF call_CheckSinSpecialCase
 




/*************************************************************************
 * Function Name: call_cosX
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_cosX(void)
 {
 
 double TempAns,
        AnsShouldBe;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // CHECK SPECIAL CASE
   if (call_CheckCosSpecialCase(&NumericValue.Value, &AnsShouldBe))
     TempAns = AnsShouldBe;
   // MATH
   else
     {
     if (CalSettings.CalAngle == DEGREES)
       TempAns = cos(NumericValue.Value*PI/180.0);
     else
       TempAns = cos(NumericValue.Value);
     }
   }
 else
   {
   // CHECK SPECIAL CASE
   if (call_CheckCosSpecialCase(&RegisterValue[0].NumericValue, &AnsShouldBe))
     TempAns = AnsShouldBe;
   else
     {
     // MATH
     if (CalSettings.CalAngle == DEGREES)
       TempAns = cos(RegisterValue[0].NumericValue*PI/180.0);
     else
       TempAns = cos(RegisterValue[0].NumericValue);
     }
   }
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_cosX




/*************************************************************************
 * Function Name: call_CheckCosSpecialCase
 * Parameters: const double *, double *
 * Return: BOOLEAN
 *
 * Description: Checks for trig cos special cases of 90 degrees (and factors).  These
 * special cases are not covered due to computer estimations.  The funciton is passed
 * the value to check and a pointer in which the correct answer (if a special case) is to
 * be loaded
 * STEP 1: Work in degree mode
 * STEP 2: Check for the error condition
 *************************************************************************/
 static BOOLEAN call_CheckCosSpecialCase(const double *Value, double *Answer)
 {
 
 double ChkValue = *Value;
 
 // STEP 1
 // CONVERT TO DEGREES
 if (CalSettings.CalAngle != DEGREES)
   {
   ChkValue = ChkValue * 180.0/PI;
   }
 
 // STEP 2
 // CHECK IF 90 DEGREE OR REPEATED BY 180
 // WORK IN ABSOLUTES
 if (ChkValue < 0)
   ChkValue *= -1.0;
 // LESS THAN 90
 if (ChkValue < 90)
   return(FALSE);
 // EQAUL TO 90
 if (ChkValue == 90)
   {
   *Answer = 0;
   return(TRUE);
   }
 // GREATER THAN 90
 if (ChkValue > 90)
   {
   ChkValue -= 90;
   if (((uint32_t)ChkValue % 180) == 0)
     {
     *Answer = 0;
     return(TRUE);
     }
   }
 
 return(FALSE);
 } // END OF call_CheckCosSpecialCase




/*************************************************************************
 * Function Name: call_tanX
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_tanX(void)
 {
 
 double TempAns,
        AnsShouldBe;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // CHECK SPECIAL CASE
   if (call_CheckTanSpecialCase(&NumericValue.Value, &AnsShouldBe))
     TempAns = AnsShouldBe;
   else
     {
     // MATH
     if (CalSettings.CalAngle == DEGREES)
       TempAns = tan(NumericValue.Value*PI/180.0);
     else
       TempAns = tan(NumericValue.Value);
     }
   }
 else
   {
   if (call_CheckTanSpecialCase(&RegisterValue[0].NumericValue, &AnsShouldBe))
     TempAns = AnsShouldBe;
   else
     {
     // MATH
     if (CalSettings.CalAngle == DEGREES)
       TempAns = tan(RegisterValue[0].NumericValue*PI/180.0);
     else
       TempAns = tan(RegisterValue[0].NumericValue);
     }
   }
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_tanX




/*************************************************************************
 * Function Name: call_CheckTanSpecialCase
 * Parameters: const double *, double *
 * Return: BOOLEAN
 *
 * Description: Checks for trig tan special cases of 0, 90, 180 degrees (and factors).  These
 * special cases are not covered due to computer estimations.  The function is passed
 * the value to check and a pointer in which the correct answer (if a special case) is to
 * be loaded.  tanX = sinX/CosX
 * STEP 1: Check for cosX special case (0) if so return infinity
 * STEP 2: Check for sinX special case (0) if so return 0
 *************************************************************************/
 static BOOLEAN call_CheckTanSpecialCase(const double *Value, double *Answer)
 {
 
 double Result;
 
 /* TAN = sinX/cosX 
  * CHECK FOR sinX = 0
  * CHECK FOR cosX = 0 */
  // STEP 1
 if (call_CheckCosSpecialCase(Value, &Result))
   {
   *Answer = sqrt(-1); // FORCE AN INFINTE - UNREAL ERROR
    return(TRUE);
   }
  // STEP 2
 if (call_CheckSinSpecialCase(Value, &Result))
   {
    *Answer = Result; // RESULT IS 0
    return(TRUE);
   }
  
 return(FALSE);

 } // END OF call_CheckTanSpecialCase




/*************************************************************************
 * Function Name: call_asinX
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_asinX(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   if (CalSettings.CalAngle == DEGREES)
     TempAns = (180.0/PI) * asin(NumericValue.Value);
   else
     TempAns = asin(NumericValue.Value);
   }
 else
   {
   if (CalSettings.CalAngle == DEGREES)
     TempAns = (180.0/PI) * asin(RegisterValue[0].NumericValue);
   else
     TempAns = asin(RegisterValue[0].NumericValue);
   }
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_asinX




/*************************************************************************
 * Function Name: call_acosX
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_acosX(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   if (CalSettings.CalAngle == DEGREES)
     TempAns = (180.0/PI) * acos(NumericValue.Value);
   else
     TempAns = acos(NumericValue.Value);
   }
 else
   {
   if (CalSettings.CalAngle == DEGREES)
     TempAns = (180.0/PI) * acos(RegisterValue[0].NumericValue);
   else
     TempAns = acos(RegisterValue[0].NumericValue);
   }
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_acosX




/*************************************************************************
 * Function Name: call_atanX
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. Operates
 * only on a single variable X register (loaded) or line input (unloaded).  The result is
 * stored in Reg X
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_atanX(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   if (CalSettings.CalAngle == DEGREES)
     TempAns = (180.0/PI) * atan(NumericValue.Value);
   else
     TempAns = atan(NumericValue.Value);
   }
 else
   {
   if (CalSettings.CalAngle == DEGREES)
     TempAns = (180.0/PI) * atan(RegisterValue[0].NumericValue);
   else
     TempAns = atan(RegisterValue[0].NumericValue);
   }
 
 // STEP 2
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_atanX



/*************************************************************************
 * Function Name: call_jXC
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_jXC(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = 1.0 / (2.0 * PI * RegisterValue[0].NumericValue * NumericValue.Value);
   }
 else
   TempAns = 1.0 / (2.0* PI* RegisterValue[1].NumericValue + RegisterValue[0].NumericValue);
 
 // STEP 2
 call_ChkAndDisplayDrop(TempAns);
 
 } // END OF call_jXC




/*************************************************************************
 * Function Name: call_jXL
 * Parameters: void
 * Return: void
 *
 * Description: Perform the stated math function on loaded and unloaded numbers. In the case
 * of unloaded numbers the input string is first checked to see if valid.  If so, the operation
 * is performed on the input string and the X (REG0).  If loaded the function is performed on the
 * X and Y register (REG0 and REG1).
 * STEP 1: If unloaded check if valid number and check for pre-existing error conditions
 * STEP 2: Check and Display the answer
 *************************************************************************/
 void call_jXL(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = (2.0 * PI * RegisterValue[0].NumericValue * NumericValue.Value);
   }
 else
   TempAns = (2.0* PI* RegisterValue[1].NumericValue + RegisterValue[0].NumericValue);
 
 // STEP 2
 call_ChkAndDisplayDrop(TempAns);
 
 } // END OF call_jXC




/*************************************************************************
 * Function Name: call_PtoR
 * Parameters: void
 * Return: void
 *
 * Description: Calculates the Rectangular coordinates from the given Polar form. 
 * Works on loaded and unloaded numbers.  For unloaded numbers the Angle is in X,
 * while the Resultant is the line input.  For loaded the Angle is Y and the Resultant
 * is X 
 * STEP 1: Create Angle multiplier for dealing with Cal Angle Mode
 * STEP 2: Perform the math
 *************************************************************************/
 void call_PtoR(void)
 {

 double Angle, R;
 Type_AudioQueueStruct AudioQueueStruct;
 
 // STEP 1
 if (CalSettings.CalAngle == DEGREES)
   Angle = PI / 180.0;
 else
   Angle = 1.0;
 
 // STEP 2
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   R = NumericValue.Value;
   Angle *= RegisterValue[0].NumericValue;
   RegisterValue[0].NumericValue = sqrt(pow(R,2)/(1.0 + (1.0/pow(tan(Angle),2))));
   R = sqrt(pow(R,2) - pow(RegisterValue[0].NumericValue,2));
   call_ChkAndDisplayRaise(R);
   }
 else
   {
   R = RegisterValue[0].NumericValue;
   Angle *= RegisterValue[1].NumericValue;
   RegisterValue[1].NumericValue = sqrt(pow(R,2)/(1.0 + (1.0/pow(tan(Angle),2))));
   RegisterValue[0].NumericValue = sqrt(pow(R,2) - pow(RegisterValue[1].NumericValue,2));
   call_FormatNumber();
   }
   strcpy(AudioQueueStruct.FileName, VERIFY_ANG_MEASURE_WAV);
   AudioQueueStruct.FullInteractiveMask = VERIFY_ANG_MEASURE_MASK;
   AudioQueueStruct.PlayLevel = FULL_INTERACTIVE;
   ctl_PostMessageByMemAllocate(&AudioQueueStruct);

 } // END OF call_PtoR




/*************************************************************************
 * Function Name: call_RtoP
 * Parameters: void
 * Return: void
 *
 * Description: Calculates the Polar coordinates from the given Rectangular form. 
 * Works on loaded and unloaded numbers.  For unloaded numbers the imaginary is in X,
 * while the real is the line input.  For loaded the imaginary is Y and the real
 * is X 
 * STEP 1: Create Angle multiplier for dealing with Cal Angle Mode
 * STEP 2: Perform the math
 *************************************************************************/
 void call_RtoP(void)
 {

 double Angle, Imaginary;
 Type_AudioQueueStruct AudioQueueStruct;
 
 // STEP 1
 if (CalSettings.CalAngle == DEGREES)
   Angle = 180.0/PI;
 else
   Angle = 1.0;
 
 // STEP 2
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   Imaginary = RegisterValue[0].NumericValue;
   RegisterValue[0].NumericValue = Angle * atan(Imaginary/NumericValue.Value);
   NumericValue.Value = sqrt(pow(NumericValue.Value,2) + pow(Imaginary,2));
   call_ChkAndDisplayRaise(NumericValue.Value);
   }
 else
   {
   Imaginary = RegisterValue[1].NumericValue;
   RegisterValue[1].NumericValue = Angle * atan(Imaginary/RegisterValue[0].NumericValue);
   RegisterValue[0].NumericValue = sqrt(pow(RegisterValue[0].NumericValue,2) + pow(Imaginary,2));
   call_FormatNumber();
   }
   strcpy(AudioQueueStruct.FileName, VERIFY_ANG_MEASURE_WAV);
   AudioQueueStruct.FullInteractiveMask = VERIFY_ANG_MEASURE_MASK;
   AudioQueueStruct.PlayLevel = FULL_INTERACTIVE;
   ctl_PostMessageByMemAllocate(&AudioQueueStruct);
 
 } // END OF call_RtoP




/*************************************************************************
 * Function Name: call_StatClear
 * Parameters: void
 * Return: void
 *
 * Description: Operates on the global struct Stat to clear its members to zero.
 * This function has no effect on the unloaded or unloaded values
 * STEP 1: Clear the strct members to zero
 * STEP 2: Turn off the stat storage icon
 *************************************************************************/
 void call_StatClear(void)
 {
 
 Stat.Count = 0;
 Stat.Sum = 0.0;
 Stat.Mean = 0.0;
 Stat.DiffSquareSum = 0.0;
 Stat.StanDev = 0.0;
 DIP204_ICON_set(ICON_MAIL, ICON_OFF);
 
 } // END OF call_StatClear




/*************************************************************************
 * Function Name: call_statAdd
 * Parameters: void
 * Return: void
 *
 * Description: Works on loaded and unloaded values.  Takes the value and adds
 * to the Stat computation for mean and standard deviation.  The Count, Sum, DiffSquareSum
 * and mean are all updated.  The count is loaded to the X register and that is displayed.
 * STEP 1: Process the stat by calling associated function
 * STEP 2: Display and set stat store icon
 *************************************************************************/
 void call_StatAdd(void)
 {
 
 double TempAns;
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // MATH
   TempAns = call_statCalUpdate(NumericValue.Value);
   }
 else
   TempAns = call_statCalUpdate(RegisterValue[0].NumericValue);
 
 // STEP 2
 DIP204_ICON_set(ICON_MAIL, ICON_ON);
 call_ChkAndDisplayRaise(TempAns);
 
 } // END OF call_statAdd

 
 
 
/*************************************************************************
 * Function Name: call_statCalUpdate
 * Parameters: double
 * Return: uint8_t
 *
 * Description: Updates the Count, Sum, and Mean.  Returns the 
 * present count.
 * STEP 1: 
 *************************************************************************/
 uint8_t call_statCalUpdate(double Xi)
 {
   
 Stat.X[Stat.Count] = Xi;
 Stat.Count++;
 Stat.Sum += Xi;
 Stat.Mean = Stat.Sum/Stat.Count;
 return(Stat.Count);
 
 } // END OF call_statCalUpdate




/*************************************************************************
 * Function Name: call_Mean
 * Parameters: void
 * Return: void
 *
 * Description: Returns the stat mean from structure to the loaded X register
 * STEP 1: 
 *************************************************************************/
 void call_Mean(void)
 {
 
 RegisterValue[0].NumericValue = Stat.Mean;
 call_FormatNumber();
 
 } // END OF call_Mean




/*************************************************************************
 * Function Name: call_StanDev
 * Parameters: void
 * Return: void
 *
 * Description: Returns the stat standard deviation on x from stat structure to 
 * the loaded X register.
 * STEP 1: Calculate SDx
 * STEP 2: Display
 *************************************************************************/
 void call_StanDev(void)
 {
 
 // STEP 1
 Stat.DiffSquareSum = 0.0;
 for (uint8_t Count = 0; Count < Stat.Count; Count++)
   {
   Stat.DiffSquareSum += pow((Stat.X[Count] - Stat.Mean),2);
   }
 RegisterValue[0].NumericValue = sqrt((1.0 / Stat.Count) * Stat.DiffSquareSum);
 
 // STEP 2
 call_FormatNumber();
 
 } // END OF call_StanDev




/*************************************************************************
 * Function Name: call_DeltaPercent
 * Parameters: void
 * Return: void
 *
 * Description: Calculates percent difference of x on y (x-y/y)*100. 
 * Works on loaded and unloaded numbers.  For unloaded numbers the y value is in RegX,
 * while the x is the line input.  For loaded the x and y are in their respective X and Y
 * registers. 
 * STEP 1: In unloaded or loaded case - step 1 check for errors: is valid number (unloaded only)
 * and divide by zero in both cases
 * STEP 2: Perform the math
 *************************************************************************/
 void call_DeltaPercent(void)
 {
 
 // STEP 1
 if (!bln_LineLoaded)
   {
   if (call_IsNumericValue(str_InputLine, CalSettings.CalBase) == FALSE)
     {
     call_ShowEntryError();
     return;
     }
   // CHECK FOR DIVIDE BY ZERO.
   if (RegisterValue[0].NumericValue == 0)
     {
     strcpy(MathError.ErrorDescription,"Divide By 0");
     strcpy(MathError.ErrorSolution,"Causes infinity");
     call_ShowMathError();
     return;
     }
   // STEP 2
   // MATH
   NumericValue.Value = 100.0 * (NumericValue.Value - RegisterValue[0].NumericValue)/RegisterValue[0].NumericValue;
   call_ChkAndDisplayRaise(NumericValue.Value);
   }
 else
   {
   // CHECK FOR DIVIDE BY ZERO
   if (RegisterValue[1].NumericValue == 0)
     {
     strcpy(MathError.ErrorDescription,"Divide By 0");
     strcpy(MathError.ErrorSolution,"Causes infinity");
     call_ShowMathError();
     return;
     }
   // MATH
   RegisterValue[0].NumericValue = 100.0 * (RegisterValue[0].NumericValue - RegisterValue[1].NumericValue) / RegisterValue[1].NumericValue;
   call_FormatNumber();
   }
 
 } // END OF call_DeltaPercent


