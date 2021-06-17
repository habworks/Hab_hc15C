/*****************************************************************
 *
 * File name:       AUDIO_TASKS.C
 * Description:     CTL RTOS Functions used to support the playing of audio files
 * Author:          Hab S. Collector
 * Date:            2/15/12
 * LAST EDIT:       6/15/2012
 * LAST EDIT:       7/24/2012
 * Hardware:        NXP LPC1768
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 
 
// INCLUDES
#include "AUDIO_TASKS.H"
#include "CORE_FUNCTIONS.H"
#include "LED_HC15C.H"
#include "TIMERS_HC15C.H"
#include "DIP204.H"
#include "FAT_FS_INC/integer.h"
#include "FAT_FS_INC/diskio.h"
#include "FAT_FS_INC/ff.h"
#include "lpc_types.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_rtc.h"
#include <ctl_api.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// GLOBALS
volatile Type_CircularBuffer CircularBufferLeft, CircularBufferRight;
volatile uint8_t AudioChannel;
volatile BOOLEAN PlayLeftChannel = TRUE;
volatile uint8_t Last_LED_BarGraphState = 0;


// EXTERNS
extern CTL_EVENT_SET_t CalEvents;
extern CTL_MESSAGE_QUEUE_t MsgQueue;
extern Type_CalSettings CalSettings;
extern FATFS fs[1];
extern FRESULT FF_Result;
extern FILINFO FF_Status;
extern DIR Directory;
extern CTL_MESSAGE_QUEUE_t AudioQueue;
extern Type_AudioQueueStruct AudioQueueStruct;
extern CTL_MEMORY_AREA_t MemArea;

/*************************************************************************
 * Function Name: audio_taskFn
 * Parameters:    void *
 * Return:        void
 *
 * Description: RTOS CTL task to manage the playing of audio files from a queue list.  
 * The task is feed from a queue which holds a struct to the file to be played.  The
 * struct is passed by pointer to call_play16Bit_WAVE function for the actual playing of the file.
 * NOTE: This task can be called from any of the major tasks, so it has no pre-conditions
 * STEP 1: Check message queue for struct meassage and assign message to struct pointer
 * STEP 2: Play the file
 * STEP 3: Free any allocated memory
 *************************************************************************/
 void audio_taskFn(void *p)
 {
 
 void *Msg;
 Type_AudioQueueStruct *AudioToPlay;
 
 while (1)
   {
   // STEP 1
   ctl_message_queue_receive(&AudioQueue, &Msg, CTL_TIMEOUT_NONE, 0);
   AudioToPlay = (Type_AudioQueueStruct *)Msg;
   
   // STEP 2
   call_play16Bit_WAVE(AudioToPlay);
   
   // STEP 3
   ctl_memory_area_free(&MemArea, (unsigned *)AudioToPlay);
   }
   
 } // END OF audio_taskFn




/*************************************************************************
 * Function Name: call_play16Bit_WAVE
 * Parameters:    Type_AudioQueueStruct *
 * Return:        BOOLEAN
 *
 * Description: This function accepts a pointer to Type_AudioQueueStruct.  That
 * struct contains the name of the 16bit .WAV file that is to be played.
 * The function does not handle a BitPerSample other than 16 which is the most common.
 * It can however accept 1 or 2 channels and varying Bytes Per Second.  It returns
 * True or False upon completion.  The file compares the play level of the passed struct
 * to determine if the present Cal Play (verbose) settings will allow it to be played.  
 * If the verbose level allows the file to play, the function opens and reads the file from the SD 
 * card, so it incorporates FAT FS features for reading a file.  The file is read, 
 * parsed for its parameters, the Play Back Rate is calculated, and the associated timer
 * IRQ (TIMER2_IRQHandler) is set to match the Play Back Rate.  Two circular buffers are created (left / 
 * right {if necessary}).  The circular (L/R) buffers are loaded from the file open Buffer....
 * then the data stream is extracted (R/L) and scaled to the DAC for audio out.
 * NOTE: File must be a .WAV 16bit PCM
 * NOTE: The file play data is loaded to the DAC (Audio) via the Timer IRQ
 * NOTE: This function has been modified to work with Music List Mode.  Where ever you see a test for 
 * music list mode is where there lies a "hook"
 * STEP 1: Verify file can play at present Cal Verbose settings.  Use of LFN if set, Other start stuff
 * STEP 2: Build the file name with path, mount the drive and open the file for reading
 * STEP 3: If this is the first read of the file (the file is read in buffer size chunks)
 * Load the various file parameters (BytesPerSec, Size, Number of Channels.  Calculate the
 * playback rate, and set the timer IRQ for at the playback rate.  
 * STEP 4: Adjust the pointer to the first data (if first read) or the next data if subsequent
 * read.
 * STEP 5: For the left channel (there will always be a left channel) Read the low and 
 * high byte to build the 16 bit word to load to the circular buffer.  Increment the bytes read accordingly
 * repeat until the all bytes are read (from the file chunk).  Note, if the circular buffer is
 * full wait for it to become empty.  The circular buffer is emptied by the timer IRQ extracting the
 * bytes and driving the DAC
 * STEP 6: For the right channel (there is not always a left channel) repeat step 5
 * STEP 7: Check for end (all audio samples in the file have been read).  If so, close the
 * file, free memory, and DAC and level meter house keeping
 *************************************************************************/
 BOOLEAN call_play16Bit_WAVE(Type_AudioQueueStruct *AudioToPlay)
 {
 
 FIL FileStream;
 UINT BytesRead;
 BYTE Buffer[IN_COMMING_BUFFER_SIZE];
 BOOLEAN FileVerified = FALSE,
         FirstRead = TRUE;
 uint8_t PathName[40], 
         FileType[5],
         RiffType[5];
 uint8_t *ptr_ToRiffOffset,
         *ptr_ToBufferData;
 uint16_t DAC_Value;
 uint32_t AudioPlayBackRate;
 Union_AudioValue AudioValue;
 Union_DataChunkSize DataChunkSize;
 Union_BytesPerSecond BytesPerSecond;
 uint8_t LineText[DISPLAY_COLUMN_TOTAL];         
         

 // STEP 1
 // CHECK TO MAKE SURE THE CAL SETTINGS WILL ALLOW THIS SOUND TO PLAY
 if (AudioToPlay->PlayLevel > CalSettings.Setup.CalVerbose)
   return(FALSE);
 // IF FULL INTERACTIVE ONLY PLAY AUDIO ONCE
 if (AudioToPlay->PlayLevel == FULL_INTERACTIVE)
   {
   if (AudioToPlay->FullInteractiveMask & CalSettings.CalVerboseMask)
     return(FALSE);
   else
     CalSettings.CalVerboseMask |= AudioToPlay->FullInteractiveMask;
   }
// CHECK FOR LONG FILE NAME USAGE 
#if _USE_LFN
 static char lfn[_MAX_LFN + 1];
 FF_Status.lfname = lfn;
 FF_Status.lfsize = sizeof(lfn);
#endif
CalSettings.MusicPlayBack.Playing = FALSE;

 // STEP 2
 // BUILD THE FILE NAME WITH PATH
 // ALLOWS SYSTEM AUDIO FILES TO PLAY AND MUSIC FILES TO PLAY WHEN IN MUSIC LIST MODE - FILTER ON CLICK
 if ((CalSettings.CalMode == MUSIC_LIST_MODE) && (strcmp(AudioToPlay->FileName, BUTTON_CLICK_WAV)))
   {
   strcpy(PathName, HC15C_MUSIC_PATH);
   strcat(PathName, "\\");
   }
 else
   {
   strcpy(PathName, HC15C_AUDIO_PATH); 
   strcat(PathName, "\\");
   }
 strcat(PathName, AudioToPlay->FileName);
 
 // MOUNT THE DRIVE AND OPEN
 //f_mount(0, &fs[0]);
 FF_Result = f_open(&FileStream, PathName, FA_OPEN_EXISTING | FA_READ);
 if (FF_Result != FR_OK)
   {
   //f_mount(0, NULL);
   return(FALSE);
   }
 // POWER UP AUDIO
 GPIO_SetValue(PORT0, PWR_AUDIO);
 // READ THE FILE STREAM AND LOAD THE BUFFER
 while(TRUE)
   {
   // READ A CHUNK OF THE WAV FILE
   FF_Result = f_read(&FileStream, Buffer, sizeof(Buffer), &BytesRead);  
   if ((FF_Result != FR_OK) || BytesRead == 0) 
     break; 
   
   // STEP 3
   // IS THIS A FRIST READ
   if (!FileVerified)
     {
     // VERIFY THE FILE TYPE AS VALID - GET FILE TYPE AND RIFF TYPE
     strncpy(FileType, Buffer, 4);
     FileType[4] = STRING_NULL;  // TERMINATE THE STRING
     ptr_ToRiffOffset = Buffer + RIFF_TYPE_OFFSET;
     strncpy(RiffType, ptr_ToRiffOffset, 4);
     RiffType[4] = STRING_NULL;  // TERMINATE THE STRING
     uint8_t StringCompareTotal = strcmp(RIFF_FILE_TYPE, FileType) + strcmp(WAVE_RIFF_TYPE, RiffType);
     if ((StringCompareTotal !=0) || (Buffer[FORMAT_SIZE_OFFSET] != 16) && (Buffer[BIT_PER_SAMPLE_OFFSET] != 16))
       {
       f_close(&FileStream);
       //f_mount(0, NULL);
       return(FALSE);
       }
     else
       {
       FileVerified = TRUE;
       // LOAD BYTE PER SECOND RATE 
       BytesPerSecond.ByteValue[0] = Buffer[BYTE_RATE_OFFSET];
       BytesPerSecond.ByteValue[1] = Buffer[BYTE_RATE_OFFSET + 1];
       BytesPerSecond.ByteValue[2] = Buffer[BYTE_RATE_OFFSET + 2];
       BytesPerSecond.ByteValue[3] = Buffer[BYTE_RATE_OFFSET + 3];
       // LOAD DATA CHUNCK SIZE
       DataChunkSize.ByteValue[0] = Buffer[DATA_SIZE_OFFSET];
       DataChunkSize.ByteValue[1] = Buffer[DATA_SIZE_OFFSET + 1];
       DataChunkSize.ByteValue[2] = Buffer[DATA_SIZE_OFFSET + 2];
       DataChunkSize.ByteValue[3] = Buffer[DATA_SIZE_OFFSET + 3];
       // INIT DAC AND SET TO MID RANGE
       init_DAC();
       LPC_DAC->DACR = (0x200 << 6);
       // SET NUMBER OF CHANNELS AND CREATE CIRCULAR BUFFERS - THERE IS ALWAYS A LEFT CHANNEL
       AudioChannel = Buffer[CHANNEL_NUMBER_OFFSET];
       init_CB(&CircularBufferLeft, CIRCULAR_BUFFER_SIZE);
       if (AudioChannel == STEREO)
         init_CB(&CircularBufferRight, CIRCULAR_BUFFER_SIZE);
       // SET THE TIMER AT THE AUDIO PLAY BACK RATE
       uint32_t PClockHz = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_TIMER2);
       AudioPlayBackRate = (Buffer[CHANNEL_NUMBER_OFFSET] * BytesPerSecond.Int32Value)/(Buffer[CHANNEL_NUMBER_OFFSET] * 2); // PLAYBACK RATE IN BYTES PER SECOND FOR THE NUMBER OF CHANNELS
       AudioPlayBackRate = (PClockHz / AudioPlayBackRate); // PLAYBACK RATE IN SECONDS AS AN INTERVAL OF THE PCLK PERIOD 
       init_HC15C_OnTimerCounter2(AudioPlayBackRate);
       // ADJUST BYTES READ FOR THIS FIRST PASS
       BytesRead -= DATA_OFFSET;
       // ALLOWS SYSTEM AUDIO FILES TO PLAY AND MUSIC FILES TO PLAY WHEN IN MUSIC LIST MODE - FILTER ON CLICK
       // SHOW TIME TO COUNT DOWN IF THIS IS A VALID MUSIC FILE
       if ((CalSettings.CalMode == MUSIC_LIST_MODE) && (strcmp(AudioToPlay->FileName, BUTTON_CLICK_WAV)))
         {
         CalSettings.MusicPlayBack.PlayTimeInSeconds = (uint32_t)((AudioChannel * DataChunkSize.Int32Value) / BytesPerSecond.Int32Value);
         uint8_t TimeRemainingInMinutes = (CalSettings.MusicPlayBack.PlayTimeInSeconds/60);
         uint8_t TimeRemainingInSeconds = CalSettings.MusicPlayBack.PlayTimeInSeconds - (TimeRemainingInMinutes * 60);
         sprintf(LineText,"%02d:%02d", TimeRemainingInMinutes, TimeRemainingInSeconds);
         DIP204_txt_engine(LineText, 4, 0, strlen(LineText));
         // SET CLOCK TASK TO RUN. USES RTC IRQ ENABLED TO IRQ EVERY SECOND
         RTC_CntIncrIntConfig(LPC_RTC, RTC_TIMETYPE_SECOND, ENABLE);
         CalSettings.MusicPlayBack.Playing = TRUE;
         }
       }
     }
   
   // STEP 4
   // SET POINTER TO DATA LOCATION THE FIRST TIME - THE POINTER WILL BE UPDATED FORWARD OF THIS
   if (FirstRead)
     ptr_ToBufferData = Buffer + DATA_OFFSET;
   else
     ptr_ToBufferData = Buffer;
   
   // STEP 5
   // LEFT AUDIO: BYTES ARE READ AS 16BIT (2 BYTES) IN LITTLE ENDIAN
   do
     {
     AudioValue.ByteValue[LOW_BYTE] = *ptr_ToBufferData;
     ptr_ToBufferData++;
     DataChunkSize.Int32Value--;
     BytesRead--;
     AudioValue.ByteValue[HIGH_BYTE] = *ptr_ToBufferData;
     ptr_ToBufferData++;
     DataChunkSize.Int32Value--;
     BytesRead--;
     // LOAD THE CIRCULAR BUFFER - WAIT IF FULL TO CONTINUE LOADING
     if ((AudioChannel == LEFT) || (AudioChannel == STEREO))
       {
       DAC_Value = call_S16Bit_To_10Bit(AudioValue.Signed16Bit_Value);
       while(isFull_CB(&CircularBufferLeft));
       write_CB(&CircularBufferLeft, &DAC_Value);
       }
      
     // STEP 6
     // RIGHT AUDIO: BYTES ARE READ AS 16BIT (2 BYTES) IN LITTLE ENDIAN
     if (AudioChannel == STEREO)
       {
       AudioValue.ByteValue[LOW_BYTE] = *ptr_ToBufferData;
       ptr_ToBufferData++;
       DataChunkSize.Int32Value--;
       BytesRead--;
       AudioValue.ByteValue[HIGH_BYTE] = *ptr_ToBufferData;
       ptr_ToBufferData++;
       DataChunkSize.Int32Value--;
       BytesRead--;
       // LOAD THE CIRCULAR BUFFER - WAIT IF FULL TO CONTINUE LOADING
       DAC_Value = call_S16Bit_To_10Bit(AudioValue.Signed16Bit_Value);
       while(isFull_CB(&CircularBufferRight));
       write_CB(&CircularBufferRight, &DAC_Value);
       } 
     } while ((DataChunkSize.Int32Value !=0) && (BytesRead !=0));
   
   // STEP 7
   FirstRead = FALSE;
   // IF MUSIC LIST MODE: CHECK FOR STOP OR PAUSE
   if (CalSettings.CalMode == MUSIC_LIST_MODE)
     {
     if (!CalSettings.MusicPlayBack.Play)
       DataChunkSize.Int32Value = 0;  // THIS WILL FORCE STOP PLAY CONDITION
     // CHECK FOR PAUSE - IF SO LED BAR GRAPH OFF AND WAIT TO PROCCEED
     while ((CalSettings.MusicPlayBack.Play) && (CalSettings.MusicPlayBack.Pause))
       {
       switchLED(LED_OFF, (LED_BAR0|LED_BAR1|LED_BAR2|LED_BAR3|LED_BAR4|LED_BAR5));
       ctl_timeout_wait(ctl_get_current_time()+5);
       }
     }
   // CHECK FOR END CONDITION IF MODE OTHER THAN MUSIC_LIST_MODE
   if (DataChunkSize.Int32Value == 0)
     {
     f_close(&FileStream);
     //f_mount(0, NULL);
     // WAIT FOR THE BUFFERS TO BE EMPTY: FREE MEMORY AND DISABLE TIMER IRQ
     while(!isEmpty_CB(&CircularBufferLeft));
     if (AudioChannel == STEREO)
       while(!isEmpty_CB(&CircularBufferRight));
     free_CB(&CircularBufferLeft);
     if (AudioChannel == STEREO)
       free_CB(&CircularBufferRight);
     TIM_Cmd(LPC_TIM2, DISABLE);
     PlayLeftChannel = TRUE;
     // POWER DOWN AUDIO
     GPIO_ClearValue(PORT0, PWR_AUDIO);
     // LED BAR GRAPH OFF
     switchLED(LED_OFF, (LED_BAR0|LED_BAR1|LED_BAR2|LED_BAR3|LED_BAR4|LED_BAR5));
     Last_LED_BarGraphState = 0;
     // FOR MUSIC MODE TURN OFF AUDIO PLAYING
     if (CalSettings.CalMode == MUSIC_LIST_MODE)
       {
       CalSettings.MusicPlayBack.Playing = FALSE;
       RTC_CntIncrIntConfig(LPC_RTC, RTC_TIMETYPE_SECOND, DISABLE);
       }
     break;
     }
   }// END OF WHILE

 // IF YOU GET TO THIS - THE FILE FINISHED PLAY
 return(TRUE);
 
 } // END OF call_play16Bit_WAVE
 
 
 
 
/*************************************************************************
 * Function Name: call_S16Bit_To_10Bit
 * Parameters: int16_t
 * Return: uint16_t
 *
 * Description: Accepts 16bit Signed PCM Audio data.  Scales and converts that
 * data to a 10bit unsigned value.  The high end is represented by 1024 the mid
 * point by 512 and the low end (the negative) as 0.  This way the audio is centered
 * about the mid point of the DAC output.  The returned value is 10 bits in a uint16_t
 * NOTE: This is a 10Bit DAC
 * STEP 1: Convert
 *************************************************************************/
 uint16_t call_S16Bit_To_10Bit(int16_t Signed16BitValue)
 {
 
 uint16_t ConvertedValue;

 // STEP 1
 ConvertedValue = (uint16_t)((Signed16BitValue + 32768)/64);
 return(ConvertedValue);
 
 } // END OF call_S16Bit_To_10Bit




/*************************************************************************
 * Function Name: audioLED_BarGraph
 * Parameters: uint16_t
 * Return: void
 *
 * Description: This function uses the LEDs to indicate an Audio Graph of
 * the audio output level.  The output LEDs are driven to a dB level as described
 * in the .h file.  Logs are used since hearing is in logs.  The code is written
 * to be fast and only updates the LED graph upon change.  The function accepts
 * the converted 10Bit Audio (see function call_S16Bit_To_10Bit), converts it back to
 * signed 16 bit.  It channels on the absolute value.
 * NOTE: This is a 10Bit DAC
 * STEP 1: Convert
 * STEP 2: Only deal in absolutes
 * STEP 3: Drive LED bar graph
 *************************************************************************/
void audioLED_BarGraph(uint16_t ScaledAudio)
{
  int16_t Signed16BitValue;
  
  // STEP 1
  // CONVERT
  Signed16BitValue = (int16_t)(64*ScaledAudio - 32768);

  // STEP 2
  // ABSOLUTE VALUE
  if (Signed16BitValue < 0)
    Signed16BitValue *= -1;

  // STEP 3
  // DISPLAY LED BAR GRAPH
  if ((Signed16BitValue < LEVEL_30DB) && (Last_LED_BarGraphState != 1))
    {
     switchLED(LED_OFF, (LED_BAR0|LED_BAR1|LED_BAR2|LED_BAR3|LED_BAR4|LED_BAR5));
     Last_LED_BarGraphState = 1;
    }
  if ((Signed16BitValue >= LEVEL_30DB) && (Signed16BitValue < LEVEL_26DB) && (Last_LED_BarGraphState !=2))
    {
     switchLED(LED_ON, LED_BAR0);
     switchLED(LED_OFF, (LED_BAR1|LED_BAR2|LED_BAR3|LED_BAR4|LED_BAR5));
     Last_LED_BarGraphState = 2;
    }
  if ((Signed16BitValue >= LEVEL_26DB) && (Signed16BitValue < LEVEL_22DB) && (Last_LED_BarGraphState !=3))
    {
     switchLED(LED_ON, (LED_BAR0|LED_BAR1));
     switchLED(LED_OFF, (LED_BAR2|LED_BAR3|LED_BAR4|LED_BAR5));
     Last_LED_BarGraphState = 3;
    }
  if ((Signed16BitValue >= LEVEL_22DB) && (Signed16BitValue < LEVEL_18DB) && (Last_LED_BarGraphState != 4))
    {
     switchLED(LED_ON, (LED_BAR0|LED_BAR1|LED_BAR2));
     switchLED(LED_OFF, (LED_BAR3|LED_BAR4|LED_BAR5));
     Last_LED_BarGraphState = 4;
    }
  if ((Signed16BitValue >= LEVEL_18DB) && (Signed16BitValue < LEVEL_14DB) && (Last_LED_BarGraphState != 5))
    {
     switchLED(LED_ON, (LED_BAR0|LED_BAR1|LED_BAR2|LED_BAR3));
     switchLED(LED_OFF, (LED_BAR4|LED_BAR5));
     Last_LED_BarGraphState = 5;
    }
  if ((Signed16BitValue >= LEVEL_14DB) && (Signed16BitValue < LEVEL_10DB) && (Last_LED_BarGraphState != 6))
    {
     switchLED(LED_ON, (LED_BAR0|LED_BAR1|LED_BAR2|LED_BAR3|LED_BAR4));
     switchLED(LED_OFF, LED_BAR5);
     Last_LED_BarGraphState = 6;
    }  
  if ((Signed16BitValue >= LEVEL_10DB) && (Last_LED_BarGraphState != 7))
    {
     switchLED(LED_ON, (LED_BAR0|LED_BAR1|LED_BAR2|LED_BAR3|LED_BAR4|LED_BAR5));
     Last_LED_BarGraphState = 7;
    }

} // END OF audioLED_BarGraph




/***********************CIRCULAR BUFFER FUNCTIONS************************* 
/*************************************************************************
 * Function Name: init_CB
 * Parameters: Type_CircularBuffer *, uint16_t
 * Return: void
 *
 * Description: Init of Circular Buffer - Sets start and end to 0 (same location)
 * and allocates memory for use.  Accepts a pointer to the Circular Buffer type and 
 * the size of the circular buffer to create.
 * STEP 1: Set start and end of buffer to 0 - set size 
 * STEP 2: Allocate memory
 *************************************************************************/
void init_CB(Type_CircularBuffer *CircularBuffer, uint16_t Size) 
{
  // STEP 1
  CircularBuffer->Size  = Size + 1; // INCLUDES AND EMPTY LOCATION
  CircularBuffer->Start = 0;
  CircularBuffer->End   = 0;
  
  // STEP 2
  CircularBuffer->Elems = (uint16_t *)calloc(CircularBuffer->Size, sizeof(int16_t));
   
} // END OF init_CB

 
 
 
 /*************************************************************************
 * Function Name: free_CB
 * Parameters: Type_CircularBuffer *
 * Return: void
 *
 * Description: The Circular Buffer allocates memory as needed.  When you are
 * done with the Circular Buffer the memory can be returned to the stack.
 * STEP 1: Free the Circular Buffer memory
 *************************************************************************/
void free_CB(Type_CircularBuffer *CircularBuffer) 
{
   
  // STEP 1
  // OK IF NULL
  free(CircularBuffer->Elems);
 
} // END OF free_CB
 
 
 
 
/*************************************************************************
 * Function Name: isFull_CB
 * Parameters: Type_CircularBuffer *
 * Return: uint8_t
 *
 * Description: Checks if the Circular Buffer is full.  Accepts a pointer to 
 * the Circular Buffer returns true if full, else returns false
 * STEP 1: Check if full
 *************************************************************************/
uint8_t isFull_CB(Type_CircularBuffer *CircularBuffer) 
{
  
  // STEP 1
  return((CircularBuffer->End + 1) % CircularBuffer->Size == CircularBuffer->Start); 

} // END OF isFull_CB




/*************************************************************************
 * Function Name: isEmpty_CB
 * Parameters: Type_CircularBuffer *
 * Return: uint8_t
 *
 * Description: Checks if the Circular Buffer is empty.  Accepts a pointer to 
 * the Circular Buffer returns true if empty, else returns false
 * STEP 1: Check if empty
 *************************************************************************/
uint8_t isEmpty_CB(Type_CircularBuffer *CircularBuffer) 
{
  // STEP 1
  return(CircularBuffer->End == CircularBuffer->Start); 
  
} // END OF isEmpty_CB




/*************************************************************************
 * Function Name: write_CB
 * Parameters: Type_CircularBuffer *, uint16 *
 * Return: void
 *
 * Description: Writes a single element to the Circular Buffer.  The data is
 * written to the end (oldest) element in the Circular Buffer array.  It is overwritten
 * if the buffer is full. 
 * NOTE: Application can choose to avoid over writing the buffer by checking first
 * to see if it is full isFull_CB.
 * NOTE: Element and Circular Buffer elems type must be the same type
 * STEP 1: Store element
 * STEP 2: Advance the end storage location
 *************************************************************************/
void write_CB(Type_CircularBuffer *CircularBuffer, uint16_t *Element)
{
  
  // STEP 1
  CircularBuffer->Elems[CircularBuffer->End] = *Element;
  
  // STEP 2
  CircularBuffer->End = (CircularBuffer->End + 1) % CircularBuffer->Size;
  if (CircularBuffer->End == CircularBuffer->Start)
    CircularBuffer->Start = (CircularBuffer->Start + 1) % CircularBuffer->Size; /* full, overwrite */

} // END OF write_CB




/*************************************************************************
 * Function Name: read_CB
 * Parameters: Type_CircularBuffer *, uint16_t *
 * Return: void
 *
 * Description: Reads a single element from the Circular Buffer.  The data is passed by reference
 * to the var Element.  The data is read from the start location.  After reading the
 * start location is advanced to the next location.
 * NOTE: Application must first ensure the Circular Buffer is not empty before reading isEmpty_CB
 * NOTE: Element and Circular Buffer elems type must be the same type
 * to see if it is full isFull_CB.
 * STEP 1: Recall element
 * STEP 2: Advance the start storage location
 *************************************************************************/
void read_CB(Type_CircularBuffer *Type_CircularBuffer, uint16_t *Element) 
{

  // STEP 1
  *Element = Type_CircularBuffer->Elems[Type_CircularBuffer->Start];
  
  // STEP 2
  Type_CircularBuffer->Start = (Type_CircularBuffer->Start + 1) % Type_CircularBuffer->Size;

} // END OF read_CB
 
 

/***********************DAC INIT SUPPORT FUNCTION************************* 
/*************************************************************************
 * Function Name: DACInit
 * Parameters: void
 * Return: void
 *
 * Description: Init of DAC output.  Set the pin for use as a DAC.  Sets the
 * DAC for use as no DMA.
 * STEP 1: Set the pin for use as a DAC
 * STEP 2: Sets the DAC as no DMA
 *************************************************************************/
 void init_DAC(void)
 {
   
  PINSEL_CFG_Type PINSEL_PinCfgStruct;
 
  // STEP 1
  // FROM TABLE 8.5 USER MANUAL LPC17XX - NOTE: P0.23 IS ADC_BAT
  PINSEL_PinCfgStruct.Funcnum = 2;
  PINSEL_PinCfgStruct.Pinnum = 26;
  PINSEL_PinCfgStruct.Portnum = PORT0;
  PINSEL_ConfigPin(&PINSEL_PinCfgStruct);
  
  // STEP 2
  // SET TO NO DMA
  LPC_DAC->DACCNTVAL = 0;
  LPC_DAC->DACCTRL = 0;
  return;
  
 } // END OF DACInit




