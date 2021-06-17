/*****************************************************************
 *
 * File name:       CORE_FUNCTIONS.C
 * Description:     Functions used to support calculator operations of the HC15C
 * Author:          Hab S. Collector
 * Date:            10/11/11
 * LAST EDIT:       8/17/2012
 * Hardware:        NXP LPC1769
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 
 
#include "CORE_FUNCTIONS.H"
#include "DIP204.H"
#include "CAT24C02.H"
//#include "TIMERS_HC15C.H"
#include "TOUCH_TASKS.H"
#include "CLOCK_TASKS.H"
#include "METER_TASKS.H"
#include "LIST_TASKS.H"
#include "SETUP_TASKS.H"
#include "AUDIO_TASKS.H"
#include "lpc_types.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

 
// DEFINE GLOBALS
BOOLEAN bln_LineLoaded = TRUE,
        bln_InputHasExp = FALSE;
uint8_t str_InputLine[MAX_DISPLAY_LENGTH]; 
// TYPES
Type_Register RegisterValue[TOTAL_REGISTERS];
Type_Numeric NumericValue;
Type_CalSettings CalSettings;
Type_STO_RCL STO_RCL;

// EXTERNS
extern void delayXms(uint32_t);
extern double LastX;
extern CTL_MESSAGE_QUEUE_t AudioQueue;

//extern Type_AudioQueueStruct AudioQueueStruct;
extern CTL_TASK_t audio_task;
extern CTL_MEMORY_AREA_t MemArea;
 
 
/*************************************************************************
 * Function Name: call_NumClick
 * Parameters: uint8_t
 * Return: void
 *
 * Description: This function is absolutely fundamental to how the calculator works.  
 * This function handles all the numeric and memory access. If the value is loaded
 * it creates a new line to push the stack.  If unloaded it adds to the existing
 * string that is being created.  This Works on 0-9, ., A-F, EXP, STO and RCL
 * NOTE: Loaded number: A number is said to be loaded if it on the stack (Register [0] or X register)
 * A loaded number is a valid number (as it has been qualified by the function isnum).  
 * It will have the flag bln_LineLoaded set.
 * NOTE: Unloaded number: a number that is not a loaded number.  It is in the process of being
 * entered onto the stack or operated on.  It's "value" is not numeric but rather a string array. The unloaded
 * value remains unloaded until it is Entered (Enter), or acted upon by a math function.  At that
 * time the first this the calling function does is determine if the unloaded string array can be a valid 
 * number.
 * STEP 1: Check if loaded / unloaded and take action to setup string. Check if
 * the max number of characters have already been entered if so do nothing.  Check if CHS has
 * been pressed - if so toggle the first location of the array to be "-" or nothing.
 * STEP 2: Load string according to alpha numeric digit, EXP, or decimal point that was 
 * Click-ed.  Set or clear the exponent flag if exponent added or removed (backspace). Handle
 * backspace event to remove the last char of the string (if not chars left - no string is being entered.
 * Setup for STO or RCL to receive the next two numeric digits.
 * STEP 3: Display: Clear the display, display the stack moved up - but do not store to 
 * the RegisterValue.  This is for display only at this point.
 * STEP 4: Check for STO RCL Event and take action to store X register to said memory location
 * or to recall from said memory location respectively.  If STO or RCL will have a flag set
 *************************************************************************/
 void call_NumClick(uint8_t NumberClick)
 {

 uint8_t str_StringToAdd[MAX_DISPLAY_LENGTH/4],
         str_TempString[MAX_DISPLAY_LENGTH] = "";
 uint8_t CharCount = 0;

 // STEP 1
 // CHECK IF LOADED VALUE - CHECK FOR CHS SIGN OTHERWISE CREATE A NEW
 if (bln_LineLoaded)
   {
   // CHECK IF CHS OPERATION
   if (NumberClick == CHANGE_SIGN)
     {
     RegisterValue[0].NumericValue *= -1.0;
     call_FormatNumber();
     return;
     }
   sprintf(str_InputLine, "");
   bln_LineLoaded = FALSE;
   }
 // CHECK IF MAX DISPLAY LENGHT EXCEED IF SO DO NOTHING.  IF HEX YOU HAV EOT ACCOUNT FOR 0x IN DISPLAY
 if (CalSettings.CalBase == BASE_16)
   {
   if (strlen(str_InputLine) > (MAX_DISPLAY_LENGTH - (1+2)))
     return;
   }
 else
   {
   if (strlen(str_InputLine) > (MAX_DISPLAY_LENGTH - 1))
     return;
   }

 // STEP 2
 switch(NumberClick)
   {
   // NUMERIC VALUES - NON HEX: 0-9
   case 1:
   case 2:
   case 3:
   case 4:
   case 5:
   case 6:
   case 7:
   case 8:
   case 9:
   case 0:
     sprintf(str_StringToAdd,"%d",NumberClick);
     strcat(str_InputLine, str_StringToAdd);
   break;
   
   case DECIMAL_POINT:
   // DECIMAL POINT - FRACTIONS NOT IMPLEMENTED AT THIS TIME
     sprintf(str_StringToAdd, ".");
     strcat(str_InputLine, str_StringToAdd);
   break;
   
   case EXPONENT:
   // EXPONENT SET THE EXPONENT FLAG
     if ((bln_InputHasExp) || (CalSettings.CalBase == BASE_16))
       break;
     if (strcmp(str_InputLine, "" ) == 0)
       sprintf(str_InputLine, "1E+");
     else
       {
       sprintf(str_StringToAdd, "E+");
       strcat(str_InputLine, str_StringToAdd);
       }
     bln_InputHasExp = TRUE;
   break;
   
   // ALFA NUMERIC KEYS
   case HEX_A:
   case HEX_B:
   case HEX_C:
   case HEX_D:
   case HEX_E:
   case HEX_F:
     *str_StringToAdd = (char)NumberClick;
     str_StringToAdd[1] = NULL_VALUE; // Must terminate the string
     strcat(str_InputLine, str_StringToAdd);
   break;
   
   case BACKSPACE:
     if (strcmp(str_InputLine, "") == 0)
         {
         bln_LineLoaded = TRUE;
         call_FormatNumber();
         return;
         }
     // ONLY IF LINE NOT LOADED - IF 'E' REMOVED CLEAR THE EXPONENT FLAG
     if ((!bln_LineLoaded) && (strlen(str_InputLine) > 0))
       {
       if (str_InputLine[strlen(str_InputLine) - 1] == 'E')
         bln_InputHasExp = FALSE;
       str_InputLine[strlen(str_InputLine) - 1] = NULL_VALUE; // String must be NULL terminated
       }
   break;
   
   case CHANGE_SIGN:
     // NO EXPONENT: ADD '-'. IF '-' MAKE '+' TO FIRST POSITION
     // IF EXPONENT: ADD IMMEDIATELY FOLLOWING THE 'E'
     if ((!bln_LineLoaded) && (!bln_InputHasExp))
       {
       if (str_InputLine[0] == '-')
         {
         while(str_InputLine[CharCount+1] != NULL_VALUE)
           {
           str_TempString[CharCount] = str_InputLine[CharCount+1];
           CharCount++;
           }
         str_TempString[CharCount] = NULL_VALUE;
         strcpy(str_InputLine, str_TempString);
         }
       else
         {
         sprintf(str_TempString, "-");
         strcat(str_TempString, str_InputLine);
         strcpy(str_InputLine, str_TempString);
         }
       }
     // EXPONENT: CHANGE + TO -, IF NOTHING AFTER "E" MAKE -
     if ((!bln_LineLoaded) && (bln_InputHasExp))
       {
       do
         {
         str_TempString[CharCount] = str_InputLine[CharCount];
         CharCount++;
         } while((str_InputLine[CharCount] != NULL_VALUE) && (str_InputLine[CharCount] != 'E'));
       str_TempString[CharCount] = str_InputLine[CharCount]; // Copy the 'E'
       CharCount++;
       // TEST IF NOTHING: IF SO ADD -
       if ((str_InputLine[CharCount] != '+') && (str_InputLine[CharCount] != '-'))
         {
         str_TempString[CharCount] = '-';
         CharCount++;
         while (str_InputLine[CharCount] != NULL_VALUE)
           {
           str_TempString[CharCount+1] = str_InputLine[CharCount];
           CharCount++;
           }
         strcpy(str_InputLine, str_TempString);
         break;
         }
       // TEST IF - OR +: IF SO CHANGE TO OPPOSITE SIGN
       if (str_InputLine[CharCount] == '+')
         str_TempString[CharCount] = '-';
       if (str_InputLine[CharCount] == '-')
         str_TempString[CharCount] = '+';
       CharCount++;
       while (str_InputLine[CharCount] != NULL_VALUE)
         {
         str_TempString[CharCount] = str_InputLine[CharCount];
         CharCount++;
         }
       strcpy(str_InputLine, str_TempString);
       }       
   break;   
   
   case STO:
     // SHOW DISPLAYF FOR STORE
     strcpy(str_InputLine, "STO ");
     // SET THE STORE FLAG AND MASK UNNCESSARY KEYS TO AVOID ERRORS
     STO_RCL.STO_Event = TRUE;
     CalSettings.Mask_KeyTouchA = CalSettings.Mask_KeyTouchB = NO_KEYS_SELECTED;
     CalSettings.Mask_KeyTouchB = MASK_0TO9_ONLY;
   break;
   
   case RCL:
     // SHOW DISPLAYF FOR RECALL
     strcpy(str_InputLine, "RCL ");
     // SET THE STORE FLAG AND MASK UNNCESSARY KEYS TO AVOID ERRORS
     STO_RCL.RCL_Event = TRUE;
     CalSettings.Mask_KeyTouchA = CalSettings.Mask_KeyTouchB = NO_KEYS_SELECTED;
     CalSettings.Mask_KeyTouchB = MASK_0TO9_ONLY;
   break;
   }

 // STEP 3
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 // Display registers 2, 1 and 0... Register 3 would not be in view to lines 1,2,3
 for (uint8_t LineCount = 1; LineCount < 4; LineCount++)
   {
   if (RegisterValue[3-LineCount].Displayed == TRUE)
      DIP204_txt_engine(RegisterValue[3-LineCount].DisplayAs, LineCount, 0, strlen(RegisterValue[3-LineCount].DisplayAs));
   else
     DIP204_clearLine(LineCount);
   }
 // DISPLAY THE X LINE THAT IS BEING LOADED RIGHT JUSTIFIED WITH CURSOR BLINKING AT ENTRY LOCATION
 DIP204_loadText(str_InputLine, 4);
 DIP204_cursorToXY(4,19);
 DIP204_set_cursor(CURSOR_BLINK);
            
 // STEP 4 
 // CHECK FOR STORE EVENT
 // A line length of 6 means that "STO ##" or "RCL ##" has been entered - call STO or RCL and have that sub complete the transaction
 if (STO_RCL.STO_Event)
 {
   if (strlen(str_InputLine) == 6)
   call_PerformSTO();
 }
 // CHECK FOR STORE EVENT
 // A line length of 6 means that "STO ##" or "RCL ##" has been entered - call STO or RCL and have that sub complete the transaction 
 if (STO_RCL.RCL_Event)
 {
   if (strlen(str_InputLine) == 6)
   call_PerformRCL();
 }
 
 } // END OF FUNCTION call_NumClick




/*************************************************************************
 * Function Name: call_IsNumeric
 * Parameters: const uint8_t *, uint8_t
 * Return: BOOLEAN
 *
 * Description: Tests to see if the input string (unloaded value) is a valid number.  The
 * number is evaluated within the context of the present entry system: HEX, decimal,
 * or binary.  The function can be called by the ENTER or any math function working on 
 * unloaded values.  Returns TRUE or FALSE
 * STEP 1: Check for Base 10 value.  If error occurs load Numeric structure
 * Error checking includes:
 * STEP 1: COPY AND ASSIGN POINTER
 * STEP 2: FOR DECIMAL
 * 1. If Value contains any characters other than "0123456789.E+-"
 * 2. If Value contains more than 1 "E" or more than 1 "."
 * STEP 3: FOR HEXIDECIMAL
 *************************************************************************/
 BOOLEAN call_IsNumericValue(const uint8_t StringToEval[], uint8_t NumericBase)
 {
 
 uint8_t CharE_Count = 0,
         CharDP_Count = 0,
         HexValueLenght = 0,
         Base16Power = 0;
 uint8_t StringToConvert[MAX_DISPLAY_LENGTH];
 uint8_t *StringValue;
 double HexStringValue;

 // STEP 1
 strcpy(StringToConvert,StringToEval);
 StringValue = StringToConvert;
 
 // STEP 2
 // FOR DECIMAL ENTRY
 if (NumericBase == BASE_10)
   {
   // CHECK ERRORS
   do
     {
     if (*StringValue == 'E')
       CharE_Count++;
     if (*StringValue == '.')
       CharDP_Count++;
     if ((*StringValue != '0')&&(*StringValue != '1')&&(*StringValue != '2')&&(*StringValue != '3')&&
         (*StringValue != '4')&&(*StringValue != '5')&&(*StringValue != '6')&&(*StringValue != '7')&&
         (*StringValue != '8')&&(*StringValue != '9')&&(*StringValue != '.')&&(*StringValue != 'E')&&
         (*StringValue != '-')&&(*StringValue != '+')||(CharE_Count > 1)||(CharDP_Count > 1))
       {
       NumericValue.Valid = FALSE;
       //                              01234567890123456789
       strcpy(NumericValue.Description, "Invalid Number");
       strcpy(NumericValue.Solution,    "Check your base");
       strcpy(NumericValue.AudioErrorFileName, INVALID_NUMBER_WAV);
       NumericValue.AudioPlayLevel = BASIC_HELP;
       strcpy(NumericValue.Result, StringToConvert);
       return(FALSE);
       }
     StringValue++;
     }while(*StringValue != NULL_VALUE);
   // NO ERRORS IF YOU GET THIS FAR
   NumericValue.Valid = TRUE;
   NumericValue.Value = atof(StringToConvert);
   // CHECK FOR INFINITE ERROR - INF ERROR IS NUMBER TOO LARGE
   if (isinf(NumericValue.Value) || isnan(NumericValue.Value))
     {
     //                              01234567890123456789
     strcpy(NumericValue.Description, "Value looks Inf");
     strcpy(NumericValue.Solution, "Out of range");
     strcpy(NumericValue.AudioErrorFileName, VALUE_TOO_LARGE_WAV);
     NumericValue.AudioPlayLevel = BASIC_HELP;
     return(FALSE);
     }
   bln_InputHasExp = FALSE; // On valid number must clear.
   LastX = NumericValue.Value;
   return(TRUE);
   } // BASE_10
 
 // STEP 3
 /* FOR HEXIDECIMAL ENTRY
  * NOTE: HEX ENTIRES WILL NOT HAVE EXPONENT TO BE CONCERNED WITH */
 if (NumericBase == BASE_16)
   {  
   HexStringValue = 0;
   HexValueLenght = strlen(StringToConvert);
   Base16Power = HexValueLenght - 1; // THIS IS RELATED TO AN ARRAY SO YOU COUNT FROM 0
   for(uint8_t CharCount = 0; CharCount < (HexValueLenght); CharCount++)
      {
      switch(*StringValue)
        {
        case '0':
        break;
        
        case '1':
        HexStringValue += 1 * (pow(16, Base16Power));
        break;
        
        case '2':
        HexStringValue += 2 * (pow(16, Base16Power));
        break;
        
        case '3':
        HexStringValue += 3 * (pow(16, Base16Power));
        break;
        
        case '4':
        HexStringValue += 4 * (pow(16, Base16Power));
        break;
        
        case '5':
        HexStringValue += 5 * (pow(16, Base16Power));
        break;
        
        case '6':
        HexStringValue += 6 * (pow(16, Base16Power));
        break;
        
        case '7':
        HexStringValue += 7 * (pow(16, Base16Power));
        break;
        
        case '8':
        HexStringValue += 8 * (pow(16, Base16Power));
        break;  
    
        case '9':
        HexStringValue += 9 * (pow(16, Base16Power));
        break;  
        
        case 'A':
        HexStringValue += 10 * (pow(16, Base16Power));
        break;    
      
        case 'B':
        HexStringValue += 11 * (pow(16, Base16Power));
        break;  
        
        case 'C':
        HexStringValue += 12 * (pow(16, Base16Power));
        break;    
      
        case 'D':
        HexStringValue += 13 * (pow(16, Base16Power));
        break;  
        
        case 'E':
        HexStringValue += 14 * (pow(16, Base16Power));
        break; 
   
        case 'F':
        HexStringValue += 15 * (pow(16, Base16Power));
        break; 
        
        default:
        NumericValue.Valid = FALSE;
        //                              01234567890123456789
        strcpy(NumericValue.Description, "Invalid Number");
        strcpy(NumericValue.Solution,    "Check your base");
        strcpy(NumericValue.AudioErrorFileName, INVALID_NUMBER_WAV);
        NumericValue.AudioPlayLevel = BASIC_HELP;
        strcpy(NumericValue.Result, StringToConvert);
        return(FALSE);
        break;
        }
      StringValue++;
      Base16Power--;
      }
     NumericValue.Value = HexStringValue;
     // CHECK FOR INFINITE ERROR - INF ERROR IS NUMBER TOO LARGE
     if (isinf(NumericValue.Value) || isnan(NumericValue.Value))
       {
       //                              01234567890123456789
       strcpy(NumericValue.Description, "Value looks Inf");
       strcpy(NumericValue.Solution, "Out of range");
       strcpy(NumericValue.AudioErrorFileName, VALUE_TOO_LARGE_WAV);
       NumericValue.AudioPlayLevel = BASIC_HELP;
       return(FALSE);
       }
     bln_InputHasExp = FALSE; // On valid number must clear.
     LastX = NumericValue.Value;
     return(TRUE);
     } // BASE_16

 } // END OF call_IsNumeric




/*************************************************************************
 * Function Name: call_FormatNumber
 * Parameters: NONE - works on global CalSetting struct and Resister
 * Return: NONE
 *
 * Description: Formats the number based on BASE 10 or BASE 16 modes.  Base 10
 * modes have an option of Scientific (ENG Mode) or Fix (mantissa) mode.  In fix mode
 * if the number exceeds a pre-defined max value it is automatically displayed in ENG mode.
 * In Hex mode if the number exceeds a pre-defined max value it too is displayed in ENG mode.
 * STEP 1: Clear screen
 * STEP 2: Format Display registers according to Display Base with setting precision
 * STEP 3: Display the registers according to display value status
 * STEP 4: Turn off cursor - numbers are on the stack there should be no cursor
 *************************************************************************/
 void call_FormatNumber(void)
 {
 
 Type_AudioQueueStruct AudioQueueStruct;
 
 // STEP 1
 DIP204_clearDisplay();
 
 // STEP 2
 switch(CalSettings.CalBase)
   {
   default:
   case BASE_10:
     for(uint8_t RegCount = 0; RegCount < TOTAL_REGISTERS; RegCount++)
       {
       // FIX MODE
       if (CalSettings.DisplayMode == FIX)
         {
         if ((RegisterValue[RegCount].NumericValue > LARGEST_FIX_DISPLAY_NUMBER) || (RegisterValue[RegCount].NumericValue < (-1.0 *LARGEST_FIX_DISPLAY_NUMBER)))
           {
           sprintf(RegisterValue[RegCount].DisplayAs,"%2.*E",CalSettings.EngPrecision, RegisterValue[RegCount].NumericValue);
           strcpy(AudioQueueStruct.FileName, FIX_EXCEEDED_WAV);
           AudioQueueStruct.FullInteractiveMask = FIX_EXCEEDED_MASK;
           AudioQueueStruct.PlayLevel = FULL_INTERACTIVE;
           ctl_PostMessageByMemAllocate(&AudioQueueStruct);
           }
         else
           sprintf(RegisterValue[RegCount].DisplayAs,"%#2.*f",CalSettings.FixPrecision, RegisterValue[RegCount].NumericValue);
         }
       else
         {
         // ENG MODE
         if (CalSettings.DisplayMode == ENG)
         sprintf(RegisterValue[RegCount].DisplayAs,"%2.*E",CalSettings.EngPrecision, RegisterValue[RegCount].NumericValue);
         }
       }
   break;
       
   case BASE_16:
     for(uint8_t RegCount = 0; RegCount < TOTAL_REGISTERS; RegCount++)
       {
       if (RegisterValue[RegCount].NumericValue < LARGEST_HEX_DISPLAY_NUMBER)
         sprintf(RegisterValue[RegCount].DisplayAs,"0x%X", (int32_t)RegisterValue[RegCount].NumericValue);
       else
         sprintf(RegisterValue[RegCount].DisplayAs,"%2.*E",CalSettings.EngPrecision, RegisterValue[RegCount].NumericValue);
       }
   }
 
 // STEP 3
 for (uint8_t LineCount = 1; LineCount < DISPLAY_LINE_TOTAL+1; LineCount++)
   {
   if (RegisterValue[(TOTAL_REGISTERS-LineCount)].Displayed)
     DIP204_txt_engine(RegisterValue[(TOTAL_REGISTERS-LineCount)].DisplayAs, LineCount, 0, strlen(RegisterValue[(TOTAL_REGISTERS-LineCount)].DisplayAs));
   else
     DIP204_clearLine(LineCount);
   }
 
 // STEP 4
 DIP204_set_cursor(CURSOR_OFF);

 } // END OF call_FormatNumber




/*************************************************************************
 * Function Name: call_ShowEntryError
 * Parameters: void
 * Return: void
 *
 * Description: Handles an entry error condition.  Displays error event info from
 * global structure NumericValue, sets global CalError and places the calculator 
 * key input mask to the ATN Key.  
 * STEP 1: Setup Error conditions
 * STEP 2: Display the error
 * STEP 3: Play AI help
 *************************************************************************/
 void call_ShowEntryError(void)
 {
 
 uint8_t LineText[20];
 Type_AudioQueueStruct AudioToPlay;
 
 // STEP 1
 CalSettings.CalError = ENTRY_ERROR;
 CalSettings.Mask_KeyTouchA = CalSettings.Mask_KeyTouchB = NO_KEYS_SELECTED;
 CalSettings.Mask_KeyTouchA = MASK_ATN_KEY;
 
 // STEP 2
 DIP204_clearDisplay();
 sprintf(LineText,"ENTRY ERROR:");
 DIP204_txt_engine(LineText, 1, 0, strlen(LineText));
 DIP204_txt_engine(NumericValue.Description, 2, 3, strlen(NumericValue.Description));
 DIP204_txt_engine(NumericValue.Solution, 3, 3, strlen(NumericValue.Solution));
 DIP204_ICON_set(ICON_ALERT, ICON_BLINK);
 DIP204_set_cursor(CURSOR_OFF);
 
 // STEP 3
 strcpy(AudioToPlay.FileName, NumericValue.AudioErrorFileName);
 AudioToPlay.PlayLevel = NumericValue.AudioPlayLevel;
 ctl_PostMessageByMemAllocate(&AudioToPlay);

 } // END OF call_ShowEntryError




/*************************************************************************
 * Function Name: call_LoadCalSettings
 * Parameters: void
 * Return: void
 *
 * Description: On POR this function loads all of the calculator previous settings from
 * EEPROM memory and sets the calculator in a known state.  The EEPROM is identified as 
 * having valid data by a special byte in location 00.  The Calculator is loaded according to the
 * table that follows. Note, Memory address 0-99 cannot all be filled.  After the loading of
 * the Cal Registers the memory is loaded.  A cal memory marker dictates there is memory to read.
 * Note, registers are stored as an 8 byte value.  The bytes are read into a union and converted to double
 * for use
 * TABLE OF MEMORY ADDRESS AND CONTENTS:
 * ADDR CONTENTS
 * 0    EEPROM HAS SOMETHING TO READ (A5)
 * 1    .CalBase (Lower Nibble) / .Setup.BacklightTimeOut (Upper Nibble)
 * 2    .FixPrecision (Lower Nibble) / .Setup.CalVerbose (Upper Nibble)
 * 3    .EngPrecision (Lower Nibble) / .Setup.TimeToSleep (Upper Nibble)
 * 4    .DisplayMode
 * 5    .CalAngle
 * 6    DisplayByte (for RegisterValues - Loaded LSN bit set = displayed
 * NEXT 8 BYTES = REGISTER 0 (X)
 * NEXT 8 BYTES = REGISTER 1 (Y)
 * NEXT 8 BYTES = REGISTER 2
 * NEXT 8 BYTES = REGISTER 3
 * 39   Cal Memory Marker (5A) - If present (=5A) then load this value to memory
 * NEXT Memory Location (0-99) to store the value
 * NEXT 8 BYTES = Said storage location
 * NEXT Byte Repeat by looking for Cal Memory Marker and store as before - continue until end of memory   
 * STEP 1: Init the EEPROM for use, clear storage values and set the default settings
 * STEP 2: Check if EEPROM is blank (if so, set calculator for first time use).
 * STEP 3: EEPROM has calculator data - load Cal Settings and display data according to table
 * STEP 4: EEPROM has calculator data - Load Register [0-3]
 * STEP 5: EEPROM has calculator data - Check for memory marker, if so, load double value to said location
 * STEP 6: Display information
 *************************************************************************/
 void call_LoadCalSettings(void)
 {
 
 Union_DoubleInBytes ValueToStore;
 Type_Register DefaultRegister;
 uint8_t EEPROM_Data[MAX_EEPROM_SIZE - 1];
 uint8_t Index = 0,
         StoreLocation = 0,
         DisplayByte = 0;
          
 // SET CAL DEFAULT VALUES
 delayXms(200);
 DIP204_ICON_set(ICON_WAIT, ICON_ON);
 
 // STEP 1
 // INIT I2C1 THE EEPROM I2C PORT
 init_I2C1(I2C_24C0X_FREQUENCY);
 // CLEAR AND INIT CONDITIONS
 strcpy(DefaultRegister.DisplayAs, "");
 DefaultRegister.Displayed = FALSE;
 DefaultRegister.NumericValue = 0.0;
 for (uint8_t RegCount = 0; RegCount < TOTAL_REGISTERS; RegCount++)
   {
   RegisterValue[RegCount] = DefaultRegister;
   }
 // LOAD CALCULATOR DEFAULT SETTINGS
 CalSettings.CalMode = CAL_MODE;  // ALWAYS START IN CALCULATOR MODE
 CalSettings.L_Shift = FALSE;
 CalSettings.R_Shift = FALSE;
 CalSettings.CalError = NO_ERROR;
 CalSettings.SleepDisable = FALSE;
 CalSettings.USB_Link = FALSE;
 select_All_Normal_Keys(); // ALL KEYS ARE ACTIVE IN CAL MODE
 STO_RCL.STO_Event = FALSE;
 STO_RCL.RCL_Event = FALSE;

 // STEP 2
 CAT24C0X_read(EEPROM_Data, 1, 0x00, SLAVE_ADDRESS_24C0X); 
 if (EEPROM_Data[0] != CAL_SETTINGS_TO_READ)
   {
   // FIRST TIME USE CAL SETTINGS
   CalSettings.CalBase = BASE_10;
   CalSettings.FixPrecision = 4;
   CalSettings.EngPrecision = 3;
   CalSettings.DisplayMode = FIX;
   CalSettings.CalAngle = DEGREES;
   CalSettings.Setup.BackLightTimeOut = DEFAULT_BACKLIGHT_TIMEOUT;
   CalSettings.Setup.CalVerbose = FULL_INTERACTIVE;
   CalSettings.Setup.TimeToSleep = DEFAULT_TIME_TO_SLEEP;
   // LOAD THE STACK
   // SET ALL STORE LOCATIONS TO EMPTY
   for (uint8_t StoreLocation = 0; StoreLocation < 100; StoreLocation++)
     {
     STO_RCL.StoredValue[StoreLocation] = EMPTY_LOCATION;
     }
   DIP204_ICON_set(ICON_WAIT, ICON_OFF);
   return;
   }
 
 // STEP 3
 CAT24C0X_read(EEPROM_Data, 6, 0x01, SLAVE_ADDRESS_24C0X); 
 CalSettings.CalBase = (0x0F & EEPROM_Data[0]);
 CalSettings.Setup.BackLightTimeOut = (EEPROM_Data[0] >> 4);
 CalSettings.FixPrecision = (0x0F & EEPROM_Data[1]);
 CalSettings.Setup.CalVerbose = (EEPROM_Data[1] >> 4);
 CalSettings.EngPrecision = (0x0F & EEPROM_Data[2]);
 CalSettings.Setup.TimeToSleep = (EEPROM_Data[2] >> 4);
 CalSettings.DisplayMode = EEPROM_Data[3];
 CalSettings.CalAngle = EEPROM_Data[4];
 if (CalSettings.CalAngle == RADS)
   {
   DIP204_ICON_set(ICON_TEMP, ICON_ON);
   }
 DisplayByte = EEPROM_Data[5];
 if (DisplayByte & 0x01)
   RegisterValue[0].Displayed = TRUE;
 if (DisplayByte & 0x02)
   RegisterValue[1].Displayed = TRUE;
 if (DisplayByte & 0x04)
   RegisterValue[2].Displayed = TRUE;
 if (DisplayByte & 0x08)
   RegisterValue[3].Displayed = TRUE;
 
 // STEP 4
 Index = 7;
 for (uint8_t RegCount = 0; RegCount < TOTAL_REGISTERS; RegCount++)
   {
   CAT24C0X_read(EEPROM_Data, sizeof(double), Index, SLAVE_ADDRESS_24C0X);
   for (uint8_t ByteCount = 0; ByteCount < sizeof(double); ByteCount++)
     {
     ValueToStore.ByteValue[ByteCount] = EEPROM_Data[ByteCount];
     }
   RegisterValue[RegCount].NumericValue = ValueToStore.DoubleValue;
   Index += sizeof(double);
   }
 
 // STEP 5
 CAT24C0X_read(EEPROM_Data, 1, Index , SLAVE_ADDRESS_24C0X); 
 while (EEPROM_Data[0] == CAL_MEMORY_MARKER)
   {
   CAT24C0X_read(EEPROM_Data, 1, ++Index, SLAVE_ADDRESS_24C0X); 
   StoreLocation = EEPROM_Data[0];
   CAT24C0X_read(EEPROM_Data, sizeof(double), ++Index , SLAVE_ADDRESS_24C0X);
   for (uint8_t ByteCount = 0; ByteCount < sizeof(double); ByteCount++)
     {
     ValueToStore.ByteValue[ByteCount] = EEPROM_Data[ByteCount];
     }
   STO_RCL.StoredValue[StoreLocation] = ValueToStore.DoubleValue;
   Index += sizeof(double);
   CAT24C0X_read(EEPROM_Data, 1, Index, SLAVE_ADDRESS_24C0X);
   }
 
 // STEP 6
 delayXms(500);
 DIP204_ICON_set(ICON_WAIT, ICON_OFF);
 
 } // END OF call_LoadCalSettings




/*************************************************************************
 * Function Name: call_StoreCalSettings
 * Parameters: void
 * Return: void
 *
 * Description: On Cal power off this function stores all of the calculator current settings to
 * EEPROM memory and sets the calculator in a known state.  The EEPROM is identified as 
 * having valid data by a special byte in location 00.  The EEPROM is stored according to the
 * table that follows. Note, Memory address 0-99 cannot all be stored.  After the storage of
 * the Registers the STO/RCL memory is stored.  A cal memory marker dictates there is memory present.
 * The memory marker is followed by the storage location followed by the value.  This process of
 * memory marker, storage location, and value continues until there is no more memory left for storage.
 * On a 256 memory you can store 21 memory locations.  Memory is stored from 00 to 99 if non-zero until there
 * is no more storage memory.
 * Note, registers are stored as an 8 byte value.  The bytes are read into a union and converted to double
 * for use
 * TABLE OF MEMORY ADDRESS AND CONTENTS:
 * ADDR CONTENTS
 * 0    EEPROM HAS SOMETHING TO READ (A5)
 * 1    .CalBase (Lower Nibble) / .Setup.BacklightTimeOut (Upper Nibble)
 * 2    .FixPrecision (Lower Nibble) / .Setup.CalVerbose (Upper Nibble)
 * 3    .EngPrecision (Lower Nibble) / .Setup.TimeToSleep (Upper Nibble)
 * 4    .DisplayMode
 * 5    .CalAngle
 * 6    DisplayByte (for RegisterValues - Loaded LSN bit set = displayed
 * NEXT 8 BYTES = REGISTER 0 (X)
 * NEXT 8 BYTES = REGISTER 1 (Y)
 * NEXT 8 BYTES = REGISTER 2
 * NEXT 8 BYTES = REGISTER 3
 * 39   Cal Memory Marker (5A) - If present (=5A) then load this value to memory
 * NEXT Memory Location (0-99) to store the value
 * NEXT 8 BYTES = Said storage location
 * NEXT Byte Repeat by looking for Cal Memory Marker and store as before - continue until end of memory   
 * STEP 1: Init the EEPROM for use
 * STEP 2: EEPROM has data - load byte something to read along with Cal Settings
 * STEP 3: Load the display values in LSN bit order 1 = displayed
 * STEP 4: Store the Registers
 * STEP 4: EEPROM has calculator data - Store Register [0-3]
 * STEP 5: Check memory contents and store as memory permits
 * STEP 6: Store to memory
 *************************************************************************/
 void call_StoreCalSettings(void)
 {
  
 Union_DoubleInBytes ValueToStore;
 uint8_t EEPROM_Data[0xFF];
 uint8_t Index = 0,
         DisplayByte = 0;
  
 // STEP 1
 DIP204_ICON_set(ICON_WAIT, ICON_ON);
 EEPROM_Data[Index] = 0x00;
 
 // STEP 2
 EEPROM_Data[++Index] = CAL_SETTINGS_TO_READ;  // LOCATION 00 
 EEPROM_Data[++Index] = (CalSettings.CalBase | (CalSettings.Setup.BackLightTimeOut << 4));
 EEPROM_Data[++Index] = (CalSettings.FixPrecision | (CalSettings.Setup.CalVerbose << 4));
 EEPROM_Data[++Index] = (CalSettings.EngPrecision | (CalSettings.Setup.TimeToSleep << 4));
 EEPROM_Data[++Index] = CalSettings.DisplayMode;
 EEPROM_Data[++Index] = CalSettings.CalAngle;
 
 // STEP 3
 if (RegisterValue[0].Displayed)
   DisplayByte |= 0x01;
 if (RegisterValue[1].Displayed)
   DisplayByte |= 0x02;
 if (RegisterValue[2].Displayed)
   DisplayByte |= 0x04;
 if (RegisterValue[3].Displayed)
   DisplayByte |= 0x08;
 EEPROM_Data[++Index] = DisplayByte; // LOCATION 6
 
 // STEP 4
 for (uint8_t RegCount = 0; RegCount < TOTAL_REGISTERS; RegCount++)
   {
   ValueToStore.DoubleValue = RegisterValue[RegCount].NumericValue;
   for (uint8_t ByteCount = 0; ByteCount < sizeof(double); ByteCount++)
     {
     EEPROM_Data[++Index] = ValueToStore.ByteValue[ByteCount];
     }
   }
 
 // STEP 5
 for (uint8_t StoreLocation = 0; StoreLocation < STO_RCL_LOCATIONS; StoreLocation++ )
   {
   if (STO_RCL.StoredValue[StoreLocation] != EMPTY_LOCATION)
     {
     EEPROM_Data[++Index] = CAL_MEMORY_MARKER;
     EEPROM_Data[++Index] = StoreLocation;
     ValueToStore.DoubleValue = STO_RCL.StoredValue[StoreLocation];
     for (uint8_t ByteCount = 0; ByteCount < sizeof(double); ByteCount++)
       {
       EEPROM_Data[++Index] = ValueToStore.ByteValue[ByteCount];
       }
     if ((Index + 1 + 1 + 8) > (MAX_EEPROM_SIZE - 1))
       break;
     }
   }
 
 // STEP 6
 if (CAT24C0X_write(EEPROM_Data, (Index+1), EEPROM_Data[0], SLAVE_ADDRESS_24C0X) != TRUE)
   Index = 99;
 DIP204_ICON_set(ICON_WAIT, ICON_OFF);
 
 } // END OF call_StoreCalSettings





/*************************************************************************
 * Function Name: call_CalATN
 * Parameters: void
 * Return: void
 *
 * Description: This function is called whenever a calculator error occurs.  Function
 * examines the global var CalError and preforms the corresponding action to clear the
 * error and place the calculator back into a running, good state
 * STEP 1: 
 * STEP 2: 
 *************************************************************************/
 void call_CalATN(void)
 {
 
 switch(CalSettings.CalError)
   {
   case ENTRY_ERROR:
     CalSettings.CalError = NO_ERROR;
     select_All_Normal_Keys();
     DIP204_ICON_set(ICON_ALERT, ICON_OFF);
     DIP204_clearDisplay();
     // Display registers 2, 1 and 0... Register 3 would not be in view to lines 1,2,3
     for (uint8_t LineCount = 1; LineCount < 4; LineCount++)
       {
       if (RegisterValue[3-LineCount].Displayed == TRUE)
         DIP204_txt_engine(RegisterValue[3-LineCount].DisplayAs, LineCount, 0, strlen(RegisterValue[3-LineCount].DisplayAs));
       else
        DIP204_clearLine(LineCount);
       }
     // DISPLAY THE X LINE THAT IS BEING LOADED RIGHT JUSTIFIED WITH CURSOR BLINKING AT ENTRY LOCATION
     DIP204_loadText(str_InputLine, 4);
     DIP204_cursorToXY(4,19);
     DIP204_set_cursor(CURSOR_BLINK);
   break;
    
   case MATH_ERROR:
     CalSettings.CalError = NO_ERROR;
     select_All_Normal_Keys();
     DIP204_ICON_set(ICON_ALERT, ICON_OFF);
     DIP204_clearDisplay();
     if (bln_LineLoaded)
       {
       for (uint8_t LineCount = 1; LineCount < 5; LineCount++)
         {
         if (RegisterValue[4-LineCount].Displayed == TRUE)
           DIP204_txt_engine(RegisterValue[4-LineCount].DisplayAs, LineCount, 0, strlen(RegisterValue[4-LineCount].DisplayAs));
         else
           DIP204_clearLine(LineCount);
         }
       }
     else
       {
       for (uint8_t LineCount = 1; LineCount < 4; LineCount++)
       {
       if (RegisterValue[3-LineCount].Displayed == TRUE)
         DIP204_txt_engine(RegisterValue[3-LineCount].DisplayAs, LineCount, 0, strlen(RegisterValue[3-LineCount].DisplayAs));
       else
        DIP204_clearLine(LineCount);
       }
       // DISPLAY THE X LINE THAT IS BEING LOADED RIGHT JUSTIFIED WITH CURSOR BLINKING AT ENTRY LOCATION
       DIP204_loadText(str_InputLine, 4);
       DIP204_cursorToXY(4,19);
       DIP204_set_cursor(CURSOR_BLINK);
       }
     break;
   
    
  default:
  break;
  }

 } // END OF call_CalATN




/*************************************************************************
 * Function Name: call_ProcessStackDown
 * Parameters: void
 * Return: void
 *
 * Description: After a valid math operation, operations that use 2 parameters
 * on the stack (X, Y or X and input line) will cause the stack to drop.  The 
 * stack must be dropped according to if the line was loaded or unloaded.  
 * STEP 1: Drop Stack Accordingly
 * STEP 2: Display the number
 *************************************************************************/
 void call_ProcessStackDown(void)
 {
 // STEP 1
  if (bln_LineLoaded) 
    {
    RegisterValue[1].NumericValue = RegisterValue[2].NumericValue;
    RegisterValue[1].Displayed = RegisterValue[2].Displayed;
    RegisterValue[2].NumericValue = RegisterValue[3].NumericValue;
    RegisterValue[2].Displayed = RegisterValue[3].Displayed;
    RegisterValue[3].NumericValue = 0.0;
    RegisterValue[3].Displayed = FALSE;
    }
  else
      bln_LineLoaded = TRUE;
  
  // STEP 2
  call_FormatNumber();
  
 } // END OF call_ProcessStackDown




/*************************************************************************
 * Function Name: call_ProcessStackUp
 * Parameters: void
 * Return: void
 *
 * Description: After a valid math operation, operations that use 1 parameter
 * on the stack (X, or input line) will cause the stack to stay or shift up.  The 
 * stack must be pushed up according to if the line was loaded or unloaded.  
 * STEP 1: Raise Stack Accordingly
 * STEP 2: Display the number
 *************************************************************************/
 void call_ProcessStackUp(double Ans)
 {
 // STEP 1
  if (bln_LineLoaded) 
    {
    RegisterValue[0].NumericValue = Ans;
    RegisterValue[0].Displayed = TRUE;
    }
  else
   {
   RegisterValue[3].NumericValue = RegisterValue[2].NumericValue;
   RegisterValue[3].Displayed = RegisterValue[2].Displayed;
   RegisterValue[2].NumericValue = RegisterValue[1].NumericValue;
   RegisterValue[2].Displayed = RegisterValue[1].Displayed;
   RegisterValue[1].NumericValue = RegisterValue[0].NumericValue;
   RegisterValue[1].Displayed = RegisterValue[0].Displayed;
   RegisterValue[0].NumericValue = Ans;
   RegisterValue[0].Displayed = TRUE;
   bln_LineLoaded = TRUE;
   }
  
  // STEP 2
  call_FormatNumber();
  
 } // END OF call_ProcessStackUp




/*************************************************************************
 * Function Name: call_PerformSTO
 * Parameters: void
 * Return: void
 *
 * Description: Works only on loaded values.  Stores the contents of the X register
 * to a storage location (0-99).  User must enter a 2 digit storage location - The value of
 * X register is undisturbed.  
 * STEP 1: Convert from string RCL XX the XX to a storage location value
 * STEP 2: Store the contents of register 0 in said location
 * STEP 3: Display
 *************************************************************************/
 void call_PerformSTO(void)
 {
  
 uint8_t str_StorageLocation[3];
 uint8_t StorageLocation;
 
 // STEP 1
 str_StorageLocation[0] = str_InputLine[4];
 str_StorageLocation[1] = str_InputLine[5];
 str_StorageLocation[3] = NULL_VALUE;
 StorageLocation = atoi(str_StorageLocation);
 
 // STEP 2
 STO_RCL.StoredValue[StorageLocation] = RegisterValue[0].NumericValue;
 
 // STEP 3
 STO_RCL.STO_Event = FALSE;
 select_All_Normal_Keys();
 bln_LineLoaded = TRUE;
 call_FormatNumber();
 
 } // END OF FUNCTION call_PerformSTO




/*************************************************************************
 * Function Name: call_PerformRCL
 * Parameters: void
 * Return: void
 *
 * Description: Works only on loaded values.  Stores the contents of the X register
 * to a storage location (0-99).  User must enter a 2 digit storage location - The value of
 * X register is undisturbed.  
 * STEP 1: Convert from string RCL XX the XX to a storage location value
 * STEP 2: Place the value of that storage location onto the stack
 *************************************************************************/
 void call_PerformRCL(void)
 {
  
 uint8_t str_StorageLocation[3];
 uint8_t StorageLocation;
 
 // STEP 1
 str_StorageLocation[0] = str_InputLine[4];
 str_StorageLocation[1] = str_InputLine[5];
 str_StorageLocation[3] = NULL_VALUE;
 StorageLocation = atoi(str_StorageLocation);
 
 // STEP 2
 STO_RCL.RCL_Event = FALSE;
 select_All_Normal_Keys();
 //bln_LineLoaded = TRUE;
 call_ProcessStackUp(STO_RCL.StoredValue[StorageLocation]);
 
 } // END OF FUNCTION call_PerformRCL




/*************************************************************************
 * Function Name: call_CalMode
 * Parameters: void
 * Return: void
 *
 * Description: Places the calculator in Cal Mode and ready's for use as such.
 * This function requires the previous Mode of operation End function to be called,
 * thus the calculator is placed into a state where this mode simply needs to set up
 * itself and not clear the previous state.
 * STEP 1: End the present mode
 * STEP 2: Set Cal Mode
 * STEP 3: Set the display to establish this mode
 *************************************************************************/
 void call_CalMode(void)
 {
 
 // STEP 1
 call_EndPresentMode();
 
 // STEP 2
 CalSettings.CalMode = CAL_MODE;
 
 // STEP 3
 call_FormatNumber();
 
 } // END OF call_CalStandBy




/*************************************************************************
 * Function Name: call_EndPresentMode
 * Parameters: void
 * Return: void
 *
 * Description: Places the calculator in a state where by the next mode can be called.
 * The function works by calling the EndMode function of the presently running function.
 * This function should be called before as the start of any Mode of operation.  It is still
 * necessary for the next Mode of operation to set the structure member .CalMode
 * STEP 1: 
 *************************************************************************/
 void call_EndPresentMode(void)
 {
 
 switch(CalSettings.CalMode)
   {
   case CLOCK_MODE:
     call_ClockModeEnd();
   break;
   
   case ALARM_MODE:
     call_AlarmModeEnd();
   break;
   
   case METER_MODE:
     call_VoltMeterModeEnd();
   break;
   
   case OHMS_MODE:
     call_OhmsMeterModeEnd();
   break;
   
   case SD_LIST_MODE:
    call_SD_ListModeEnd();
   break;
   
   case SETUP_MODE:
     call_SetupModeEnd();
   break;
   
   case MUSIC_LIST_MODE:
     call_MusicListModeEnd();
   break;
   
   default:
   case CAL_MODE:
   break;
   }

 } // END OF call_EndPresentMode




/*************************************************************************
 * Function Name: ctl_HabTaskSuspend
 * Parameters: CTL_TASK_t
 * Return: void
 *
 * Description: uses the CTL_TASK_t structure member to place the task in
 * suspend mode.  Use to suspend / de-schedule the operation of a task.  You can use
 * ctl_HabTaskRun to re-schedule the task.
 * STEP 1: Suspend the task
 *************************************************************************/
 void ctl_HabTaskSuspend(CTL_TASK_t *Task)
 {
   // STEP 1
   Task->state = CTL_STATE_SUSPENDED;
   
 } // END OF ctl_HabTaskSuspend
 
 
 
 
/*************************************************************************
 * Function Name: ctl_HabTaskRun
 * Parameters: CTL_TASK_t
 * Return: void
 *
 * Description: uses the CTL_TASK_t structure member to place the task in
 * suspend mode.  Use to make a task runnable after it has been suspended by 
 * ctl_HabTaskSuspend
 * STEP 1: Make the task runnable
 *************************************************************************/
 void ctl_HabTaskRun(CTL_TASK_t *Task)
 {
   // STEP 1
   Task->state = CTL_STATE_RUNNABLE;
   
 } // END OF ctl_HabTaskRun
 
 
 
 
 /*************************************************************************
 * Function Name: ctl_PostMessageByMemAllocate
 * Parameters: CTL_TASK_t
 * Return: void
 *
 * Description: For tasks that call audio_taskFn that are of a higher priority than said task
 * calls to post to the audio message queue must first allocate memory for posting.  This is 
 * necessary as in such cases posting to the message queue (the command syntax) does not 
 * immediately post the message.  The message is posted some indeterminate amount of time later.
 * The message can be forced to post by changing the priority of audio_taskFn to be higher than the 
 * calling task immediately after the message has been posted.  This causes audio_taskFn to run
 * which it would appear forces the message into the queue.  Upon completion of audio taskFn execution
 * returns to the calling task and the priority of audio_taskFn is restored.  This however is problematic as
 * audio_taskFn really needs to be (and stay) a very low priority task.  As suggested from the 
 * good folks at CrossWorks the better fix for this is to allocate memory for use with the message queue.
 * The allocated memory is declared and init in main.  Note the memory is freed within audio_taskFn
 * Allocating the memory and subsequently writing to it causes the message to post to the queue
 * immediately.
 * STEP 1: Allocate memory via a pointer to the struct and check for errors
 * STEP 2: Copy the contents that was passed to the allocated memory pointer
 * STEP 3: Post the message
 *************************************************************************/
 void ctl_PostMessageByMemAllocate(Type_AudioQueueStruct *AudioStruct)
 {
 
 // STEP 1
 Type_AudioQueueStruct *AudioToPlay = (Type_AudioQueueStruct *)ctl_memory_area_allocate(&MemArea);
 if (!AudioToPlay)
      ctl_handle_error(CTL_UNSPECIFIED_ERROR);
 
 // STEP 2
 strcpy(AudioToPlay->FileName, AudioStruct->FileName);
 AudioToPlay->PlayLevel = AudioStruct->PlayLevel;
 AudioToPlay->FullInteractiveMask = AudioStruct->FullInteractiveMask;
 
 // STEP 3
 ctl_message_queue_post(&AudioQueue, AudioToPlay, 0, 0);

 } // END OF ctl_PostMessageByMemAllocate
   