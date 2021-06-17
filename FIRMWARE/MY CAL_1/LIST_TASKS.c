/*****************************************************************
 *
 * File name:       LIST_TASKS.C
 * Description:     CTL RTOS Functions used to support SD Directory LIST 
                    related functions of the HC15C
 * Author:          Hab S. Collector`
 * Date:            12/06/11
 * LAST EDIT:       7/21/2012 
 * Hardware:        NXP LPC1769
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent on other includes.
 *                  everything these functions need should be passed to them.  
 *                  It will be necessary to consult the reference documents and associated schematics to understand
 *                  the operations of this firmware.
 *****************************************************************/ 

#include "LIST_TASKS.H"
#include "TOUCH_TASKS.H"
#include "AUDIO_TASKS.H"
#include "DIP204.H"
#include "CORE_FUNCTIONS.H"
#include "FAT_FS_INC/integer.h"
#include "FAT_FS_INC/diskio.h"
#include "FAT_FS_INC/ff.h"
#include "lpc_types.h"
#include "lpc17xx_rtc.h"
#include <string.h>
#include <stdio.h>
#include <ctl.h>


// GLOBALS
Type_SD_List SD_List;
Type_Music_List Music_List;


// EXTERNS
extern void delayXms(uint32_t);
extern CTL_EVENT_SET_t CalEvents;
extern CTL_TASK_t SDlist_task;
extern Type_CalSettings CalSettings;
extern FATFS fs[1];
extern FRESULT FF_Result;
extern FILINFO FF_Status;
extern DIR Directory;



/*************************************************************************
 * Function Name: SD_List_taskFn
 * Parameters:    void *
 * Return:        void
 *
 * Description: RTOS CTL task to manage the SD directory list.  The task list the
 * directory of the SD card drive 0 (only).  If the number of directory listings are
 * greater than 3, it allows the user to scroll up and down the directory and displays
 * the up and down arrow accordingly. 
 * NOTE: This task must not be called unless the SD List Mode is first called 
 * NOTE: The events are set up the UI and the mode functions
 * STEP 1: Made runnable by the SD List Mode.  This event loads the directory and displays
 * the list from the first entry.
 * STEP 2: UI wants to scroll down - determine if possible and if so scroll down
 * STEP 3: UI wants to scroll up - determine if possible and if so scroll up
 * STEP 4: Service music list events
 *************************************************************************/
 void SD_List_taskFn(void *p)
 {
 
 while (1)
   {
   ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS, &CalEvents, (EVENT_SD_LIST|EVENT_SD_UI|EVENT_MUSIC_LIST), CTL_TIMEOUT_NONE, 0);
   
   // STEP 1
   if (CalEvents & EVENT_SD_LIST)
     {
     LoadSD_Dir();
     SD_List.TopListingDisplayed = 0;
     DisplayListOf3();
     ctl_events_set_clear(&CalEvents, 0, EVENT_SD_LIST);
     } // END OF EVENT_SD_LIST
   
   // STEP 2
   // UI TO SCROLL DOWN
   if (CalEvents & EVENT_SD_UI)
     {
     if (SD_List.UI == SCROLL_DOWN)
       {
       if (SD_List.TopListingDisplayed < (SD_List.NumberOfDirEntries - 1))
         {
         SD_List.TopListingDisplayed++;
         DisplayListOf3();
         }
       }
     // STEP 3
     // UI TO SCROLL UP
     if (SD_List.UI == SCROLL_UP)
       {
       if (SD_List.TopListingDisplayed > 0)
         {
         SD_List.TopListingDisplayed--;
         DisplayListOf3();
         }
       }
     SD_List.UI = NO_SCROLL;
     ctl_events_set_clear(&CalEvents, 0, EVENT_SD_UI);
     } // END OF EVENT_SD_UI
   
   // STEP 4
   // MUSIC LIST EVENTS
   if (CalEvents & EVENT_MUSIC_LIST)
     {
     printMusicList();
     ctl_events_set_clear(&CalEvents, 0, EVENT_MUSIC_LIST); 
     } // END OF EVENT_MUSIC_LIST
      
   } // END OF WHILE
   
 } // END OF SD_List_taskFn




/*************************************************************************
 * Function Name: call_SD_ListMode
 * Parameters:    void
 * Return:        void
 *
 * Description: Places the calculator in SD List mode.  List Mode displays the contents SD Drive
 * of the directory.  The user can use the up and down scroll bars to move through the directory.
 * SD List Mode is one of the fundamental modes of operation.  
 * STEP 1: End the present mode, set up keys to use
 * STEP 2: Ready the display
 * STEP 3: Set for SD List Mode
 *************************************************************************/
 void call_SD_ListMode(void)
 {

 // STEP 1
 call_EndPresentMode();
 CalSettings.Mask_KeyTouchA = CalSettings.Mask_KeyTouchB = NO_KEYS_SELECTED;
 CalSettings.Mask_KeyTouchA = (MASK_ATN_KEY | MASK_KEY_LSHIFT);
 CalSettings.Mask_KeyTouchB = (MASK_KEY_CAL | MASK_KEY_METER | MASK_KEY_FLASH | MASK_KEY_MUSIC | // MODES OF OPERATION
                               MASK_KEY_UP |     // SCROLL LIST UP
                               MASK_KEY_DOWN);   // SCROLL LIST DOWN

 // STEP 2
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 DIP204_txt_engine(DIR_HEADING, DIR_HEAD_ROW, DIR_HEAD_POSITION, strlen(DIR_HEADING));
 
 // STEP 3
 /* SET SD LIST MODE LAST SO THAT YOU CAN RESTORE SD LIST TASK ON FIRST TIME IN THIS FUNCTION AND WILL
  * NOT RESTORE ON SUBSEQUENT CALLS TO THIS FUNCTION IF ALREADY IN SD LIST MODE. */
 if (CalSettings.CalMode != SD_LIST_MODE)
   {
    #if defined(REMOVE_RESTORE)
      ctl_task_restore(&SDlist_task);
    #elif defined(SUSPEND_RUN)
      ctl_HabTaskRun(&SDlist_task);
    #endif
   }
 // DISPLAY ICON
 DIP204_ICON_set(ICON_INFO, ICON_ON);
 SD_List.UI = NO_SCROLL;
 SD_List.NumberOfDirEntries = 0;
 CalSettings.CalMode = SD_LIST_MODE;
 // LOAD THE LIST ARRAY DIRECTORY AND SET THE FIRST ITEM TO BE DISPLAYED AS THE FIRST ITEM IN THE ARRAY
 ctl_events_set_clear(&CalEvents, EVENT_SD_LIST, 0);  
   
 } // END OF call_SD_ListMode




/*************************************************************************
 * Function Name: call_DirScrollDown
 * Parameters: void
 * Return: void
 *
 * Description: Sets up the SD Dir to scroll down by setting the UI function and
 * calling the SD task.
 * STEP 1: Setup for event
 **************************************************************************/
void call_DirScrollDown(void)
{
 
 // STEP 1
 SD_List.UI = SCROLL_DOWN;
 ctl_events_set_clear(&CalEvents, EVENT_SD_UI, 0);
 
} // END OF call_DirScrollDown




/*************************************************************************
 * Function Name: call_DirScrollUp
 * Parameters: void
 * Return: void
 *
 * Description: Sets up the SD Dir to scroll up by setting the UI function and
 * calling the SD task.
 * STEP 1: Setup for event
 **************************************************************************/
void call_DirScrollUp(void)
{
 
 // STEP 1
 SD_List.UI = SCROLL_UP;
 ctl_events_set_clear(&CalEvents, EVENT_SD_UI, 0);
 
} // END OF call_DirScrollUp




/******************************************************************************
 * Function Name: DisplayListOf3
 * Parameters: uint8_t
 * Return: void
 *
 * Description: Prints 3 files to screen starting from the TopListingDisplayed Entry.
 * Three files because it is a 4 line screen with the top line used as a header.
 * NOTE: Function requires the LoadSD_Dir to have been previously called.
 * STEP 1: Clear the screen and Print the directory heading
 * STEP 2: Print 3 files or if less the total files remaining
 * STEP 3: Update the scroll bars 
 ******************************************************************************/
 void DisplayListOf3(void)
 {
 
 uint8_t FileListing;
 
 // STEP 1
 DIP204_clearDisplay();
 DIP204_txt_engine(DIR_HEADING, DIR_HEAD_ROW, DIR_HEAD_POSITION, strlen(DIR_HEADING));
 
 // STEP 2
 // PRINT THE FILES
 for (uint8_t Count = 0; Count < 3; Count++)
   {
   FileListing = Count + SD_List.TopListingDisplayed;
   if (FileListing > SD_List.NumberOfDirEntries)
     break;
   else
     DIP204_txt_engine(SD_List.DirEntry[FileListing], (Count + 2), FILE_POSITION, strlen(SD_List.DirEntry[FileListing]));
   }
   
 // STEP 3
 // COULD YOU SCROLL UP
 if (SD_List.TopListingDisplayed > 0)
   DIP204_ICON_set(ICON_UP_ARROW, ICON_ON);
 else
   DIP204_ICON_set(ICON_UP_ARROW, ICON_OFF);
 // COULD YOU SCROLL DOWN
 if ((SD_List.TopListingDisplayed + 2) < (SD_List.NumberOfDirEntries - 1))
   DIP204_ICON_set(ICON_DOWN_ARROW, ICON_ON);
 else
   DIP204_ICON_set(ICON_DOWN_ARROW, ICON_OFF); 
 
 } // END OF DisplayListOf3




/*************************************************************************
 * Function Name: call_SD_ListModeEnd
 * Parameters: void
 * Return: void
 *
 * Description: Performs the actions necessary to end SD_List Mode.  Such that the next
 * mode can start itself.  This function does not set the calculator mode.  It is up to the 
 * the next mode function to set the mode of operation.
 * STEP 1: End the meter task
 * STEP 2: Select all Keys for the next function
 * STEP 3: Prep the display for use by the next Mode
 **************************************************************************/
 void call_SD_ListModeEnd(void)
 {
 ctl_unmask_isr(RTC_IRQn);
 // STEP 1
 #if defined(REMOVE_RESTORE)
    ctl_task_remove(&SDlist_task);
  #elif defined(SUSPEND_RUN)
    ctl_HabTaskSuspend(&SDlist_task);
  #endif
 
 // STEP 2
 select_All_Normal_Keys();

 // STEP 3
 DIP204_ICON_set(ICON_INFO, ICON_OFF);
 DIP204_ICON_set(ICON_UP_ARROW, ICON_OFF);
 DIP204_ICON_set(ICON_DOWN_ARROW, ICON_OFF); 
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
  
 } // END OF call_SD_ListModeEnd




static FRESULT scan_files (char* path)
{
 // IF YOU WANT TO LIST WHAT IS INSIDE THE SUB DIR...
}
/******************************************************************************
 * Function Name: LoadSD_Dir
 * Parameters: void
 * Return: void
 *
 * Description: Load file names and directories of the root directory of the SD card.
 * Place those names into the SD structure.  If directory bound by "<" and ">".
 * This function also provides a count of the total files loaded (which cannot exceed
 * MAX_ENTRIES   
 * NOTE: The buffer does not sort the directories or file names in any order.  They are loaded
 * in the same manner they are read
 * STEP 1: Clear display and perform FAT FS house keeping on long file names
 * STEP 2: Open the root (0) Dir - If OK on open proceed
 * STEP 3: Read the directory until empty  
 * STEP 4: Check if dir read is a file or a sub dir.  Display sub dir within <Dir Name>
 * Also check if name is too long to be displayed on screen.  If so end file name as
 * ... and dir name as ..>
 * STEP 5: Load to the array
 ******************************************************************************/
void LoadSD_Dir(void)
{
  uint8_t LineText[MAX_FILE_NAME_LEN];
  uint8_t Path[40];
  strcpy(Path, DEFAULT_PATH);
  uint8_t StrLength, 
          DirLineListing = 0;
  char *Fn;

  // STEP 1
  // SETUP DISPLAY, MOUNT DRIVE, CHECK FOR LONG FILE NAMES
  DIP204_clearDisplay();
  DIP204_txt_engine("DIRECTORY LISTING:", 1, 0, strlen("DIRECTORY LISTING:"));
  //f_mount(0, &fs[0]); 
#if _USE_LFN
  static char lfn[_MAX_LFN + 1];
  FF_Status.lfname = lfn;
  FF_Status.lfsize = sizeof(lfn);
#endif
    
  // STEP 2
  // OPEN ROOT AND VALIDATE
  FF_Result = f_opendir(&Directory, Path);                       
  if (FF_Result == FR_OK) 
  {
    // STEP 3
    // READ ROOT - CHECK IF EMPTY TO EXIT
    for (;;) 
    {
      FF_Result = f_readdir(&Directory, &FF_Status);             // READ DIR ITEM
      if (FF_Result != FR_OK || FF_Status.fname[0] == 0) break;  // BREAK ON ERROR OR END OF DIR LIST
      if (FF_Status.fname[0] == '.') continue;                   // IGNOR . ENTRIES
#if _USE_LFN
      Fn = *FF_Status.lfname ? FF_Status.lfname : FF_Status.fname;
#else
      Fn = FF_Status.fname;
#endif
        
      // STEP 4
      // CHECK IF FILE OR DIR AND LOAD TO BUFFER ACCORDINGLY
      if (FF_Status.fattrib & AM_DIR) 
      {   
        // IT IS DIR
        sprintf(LineText, "<%s>", Fn);
        StrLength = strlen(LineText);
        if (StrLength > 0)
          LineText[StrLength] = STRING_NULL;
        /* TRUNCATE TO FIT DISPLAY WIDITH...
         * THE DISPLAY IS 20 CHARS LONG, BUT FILE IS OFFSET BY 2.  HENCE YOU
         * CAN DISPLAY A MAX OF 18 CHARS.  IF LONGER THAN 18 CHARS PLACE ...
         * AT THE VERY END OF FILE NAME TO SIGNIFY IT IS LONGER */
        StrLength = strlen(LineText);
        if (StrLength > (DISPLAY_COLUMN_TOTAL - FILE_POSITION))
          {
          LineText[15] = '.';
          LineText[16] = '.';
          LineText[17] = '>';
          LineText[18] = STRING_NULL;
          }
        strcpy(SD_List.DirEntry[DirLineListing], LineText);
        DirLineListing++;
        /* IF IN THE FUTURE YOU WANT TO READ THE SUB DIRS PUT THAT CODE HERE
         * FF_Result = scan_files(Path);
         * if (FF_Result != FR_OK) break; */
      } 
      else 
      {   
        // IT IS FILE
        sprintf(LineText, "%s", Fn); // MAY WANT TO PRINT PATH %s
        // PLACE NULL STRING AT END OF FILE NAME
        StrLength = strlen(LineText);
        if (StrLength > 0)
          LineText[StrLength] = STRING_NULL;
        /* TRUNCATE TO FIT DISPLAY WIDITH...
         * THE DISPLAY IS 20 CHARS LONG, BUT FILE IS OFFSET BY 2.  HENCE YOU
         * CAN DISPLAY A MAX OF 18 CHARS.  IF LONGER THAN 18 CHARS PLACE ...
         * AT THE VERY END OF FILE NAME TO SIGNIFY IT IS LONGER */
        StrLength = strlen(LineText);
        if (StrLength > (DISPLAY_COLUMN_TOTAL - FILE_POSITION))
          {
          LineText[15] = '.';
          LineText[16] = '.';
          LineText[17] = '.';
          LineText[18] = STRING_NULL;
          }
        // STEP 5: 
        // LOAD THE SD DIR ARRAY
        strcpy(SD_List.DirEntry[DirLineListing], LineText);
        DirLineListing++;
      }
    }
    // LOAD ARRAY VALUE TOTAL
    SD_List.NumberOfDirEntries = DirLineListing;
  }
  
  //f_mount(0, NULL);
  
} // END OF LoadSD_Dir




/*************************************************************************
 * Function Name: call_MusicListMode
 * Parameters:    void
 * Return:        void
 *
 * Description: Places the calculator in Music List mode.  Music List Mode displays the contents SD Drive
 * Music Directory for .WAV files.  The user can use the up and down scroll keys to move through the directory.
 * Music List Mode is one of the fundamental modes of operation.  This function does not play the
 * music.  Rather it works with the task SD_List_taskFn to display, scroll through, enable play of music.
 * The actual playing of the music files is accomplished via the task audio_taskFn
 * STEP 1: End the present mode, set up keys to use
 * STEP 2: Ready the display
 * STEP 3: Set for SD List Mode
 * STEP 4: Load the Music Directory to the Music structure 
 *************************************************************************/
 void call_MusicListMode(void)
 {

 // STEP 1
 call_EndPresentMode();
 CalSettings.Mask_KeyTouchA = CalSettings.Mask_KeyTouchB = NO_KEYS_SELECTED;
 CalSettings.Mask_KeyTouchA = (MASK_ATN_KEY | MASK_KEY_LSHIFT);
 CalSettings.Mask_KeyTouchB = (MASK_KEY_CAL | MASK_KEY_METER | MASK_KEY_FLASH | MASK_KEY_MUSIC |
                               MASK_KEY_UP |     // SCROLL LIST UP
                               MASK_KEY_DOWN |   // SCROLL LIST DOWN
                               MASK_KEY_STOP |   // SCROLL LIST STOP PLAYING
                               MASK_KEY_ENTER |  // PLAY PAUSE MUSIC
                               MASK_KEY_START);  // PLAY PAUSE MUSIC

 // STEP 2
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 DIP204_txt_engine(MUSIC_HEADING, DIR_HEAD_ROW, DIR_HEAD_POSITION, strlen(MUSIC_HEADING));
 
 // STEP 3
 /* SET MUSIC LIST MODE LAST SO THAT YOU CAN RESTORE MUSIC LIST TASK ON FIRST TIME IN THIS FUNCTION AND WILL
  * NOT RESTORE ON SUBSEQUENT CALLS TO THIS FUNCTION IF ALREADY IN MUSIC LIST MODE. */
 if (CalSettings.CalMode != MUSIC_LIST_MODE)
   {
    #if defined(REMOVE_RESTORE)
      ctl_task_restore(&SDlist_task);
    #elif defined(SUSPEND_RUN)
      ctl_HabTaskRun(&SDlist_task);
    #endif 
   }
 // DISPLAY ICON
 DIP204_ICON_set(ICON_INFO, ICON_ON);
 Music_List.NumberOfDirEntries = 0;
 Music_List.SelectedFile = 0;
 CalSettings.CalMode = MUSIC_LIST_MODE;
 
 // STEP 4
 // LOAD THE LIST ARRAY DIRECTORY AND SET THE FIRST ITEM TO BE DISPLAYED AS THE FIRST ITEM IN THE ARRAY
 LoadMusicDirListing(HC15C_MUSIC_PATH);
 ctl_events_set_clear(&CalEvents, EVENT_MUSIC_LIST, 0);   
 
 } // END OF call_MusicListMode




/*************************************************************************
 * Function Name: call_MusicListModeEnd
 * Parameters: void
 * Return: void
 *
 * Description: Performs the actions necessary to end Music_List Mode such that the next
 * mode can start itself.  This function does not set the calculator mode.  It is up to the 
 * the next mode function to set the mode of operation.
 * STEP 1: End the meter task
 * STEP 2: Select all Keys for the next function
 * STEP 3: Prep the display for use by the next Mode
 **************************************************************************/
 void call_MusicListModeEnd(void)
 {
 ctl_unmask_isr(RTC_IRQn);
 // STEP 1
 #if defined(REMOVE_RESTORE)
    ctl_task_remove(&SDlist_task);
  #elif defined(SUSPEND_RUN)
    ctl_HabTaskSuspend(&SDlist_task);
  #endif
 
 // STEP 2
 select_All_Normal_Keys();

 // STEP 3
 DIP204_ICON_set(ICON_INFO, ICON_OFF);
 DIP204_ICON_set(ICON_UP_ARROW, ICON_OFF);
 DIP204_ICON_set(ICON_DOWN_ARROW, ICON_OFF); 
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
  
 } // END OF call_MusicListModeEnd




/******************************************************************************
 * Function Name: LoadMusicDirListing
 * Parameters: uint8_t *
 * Return: void
 *
 * Description: load file names of the MusicPath which is passed by reference to the music structure
 * This function is also provides a count of the total files loaded (which cannot exceed
 * MAX_ENTRIES  
 * NOTE: Only file names are loaded.  File names can be a max of MAX_FILE_NAME_LEN long
 * however this function does not limit as MAX_FILE_NAME_LEN (40) is thought to be long enough.
 * STEP 1: FAT FS HOUSE KEEPING FOR LONG FILE NAME
 * STEP 2: Open directory and read contents
 * STEP 3: If read file (only) add to music list and increment music list directory count
 **************************************************************************/  
 void LoadMusicDirListing(const uint8_t *MusicPath)
 {
 
 char *Fn;
 uint8_t LineText[MAX_FILE_NAME_LEN];
 uint8_t DirLineListing = 0,
         StrLength; 
 
 // STEP 1
#if _USE_LFN
  static char lfn[_MAX_LFN + 1];
  FF_Status.lfname = lfn;
  FF_Status.lfsize = sizeof(lfn);
#endif
    
  // STEP 2
  // OPEN DIRECTORY AND VALIDATE
  FF_Result = f_opendir(&Directory, MusicPath);                       
  if (FF_Result == FR_OK) 
  {
    for (;;) 
    {
      FF_Result = f_readdir(&Directory, &FF_Status);             // READ DIR ITEM
      // CHECK FOR END OF READ
      if ((FF_Result != FR_OK) || (FF_Status.fname[0] == 0) || (DirLineListing == MAX_ENTRIES))
        break;                                                   // BREAK ON ERROR OR END OF DIR LIST
      if (FF_Status.fname[0] == '.') continue;                   // IGNOR . ENTRIES
#if _USE_LFN
      Fn = *FF_Status.lfname ? FF_Status.lfname : FF_Status.fname;
#else
      Fn = FF_Status.fname;
#endif
       
      // STEP 3
      // CHECK IF FILE DO NOTHING IF DIR
      if (!(FF_Status.fattrib & AM_DIR)) 
      {     
        sprintf(LineText, "%s", Fn); // MAY WANT TO PRINT PATH %s
        // PLACE NULL STRING AT END OF FILE NAME
        StrLength = strlen(LineText);
        if (StrLength > 0)
          LineText[StrLength] = STRING_NULL;
        // LOAD THE MUSIC DIR ARRAY
        strcpy(Music_List.MusicDirEntry[DirLineListing], LineText);
        DirLineListing++;
      }
    }
    /* LOAD ARRAY VALUE TOTAL
     * NOTE MEMBER NumberOfDirEntries IS A TOTAL COUNT, NOT AN INDEX LIKE SelectedFile (WHICH
     * STARTS AT 0.  THEREFORE AT ITS MAX SelectedFile WILL ALLWAYS BE 1 LESS THAN NumberOfDirEntries */
    Music_List.NumberOfDirEntries = DirLineListing;
  }
 
 } // END OF LoadMusicDirListing




/******************************************************************************
 * Function Name: printMusicList
 * Parameters: uint8_t *
 * Return: void
 *
 * Description: Scrolls the Music Display, while placing a marker by the selected file.
 * There are 2 basic conditions for this logic.  First, if the selected file is less than 
 * the total lines of the display.  If so, you can print that many lines starting at the
 * first location, just add the selected file marker to the selected file.  Second, if the
 * selected file is greater than the total lines of the display.  If so, the selected file
 * is printed at the bottom of the display (with marker) and the files above it are printed
 * above it.
 * NOTE: It is up to the music list scroll up and down function to make sure the selected file
 * is within bounds of the number of listings
 * STEP 1: Clear the file print lines
 * STEP 2: Check for condition 1
 * STEP 3: Check for condition 2
 * STEP 4: Set the scroll icons (up and down) accordingly
 **************************************************************************/  
 static void printMusicList(void)
 {
 
 uint8_t LineText[DISPLAY_COLUMN_TOTAL];
 uint8_t ValidFileName[DISPLAY_COLUMN_TOTAL];
 
 // STEP 1
 // CLEAR THE EXSISTING LIST DISPLAY LINES - LINE 1 IS HEADING
 DIP204_clearLine(2);
 DIP204_clearLine(3);
 DIP204_clearLine(4);
 
 // STEP 2
 // CHECK IF SELECTED FILE IS WITHIN THE RANGE OF THE DISPLAY LENGHT IF SO JUST DISPLAY THE FIRST 3 IF POSSIBLE
 if (Music_List.SelectedFile < LENGTH_OF_DISPLAY)
   {
   for (uint8_t Count = 0; Count < LENGTH_OF_DISPLAY; Count++)
       {
       if (Count > (Music_List.NumberOfDirEntries - 1))
          return;
       checkFileName(Music_List.MusicDirEntry[Count], ValidFileName);
       // IF SELECTED FILE PLACE MARKER
       if (Count == Music_List.SelectedFile)
         {
         strcpy(LineText,"*");
         strcat(LineText, ValidFileName);
         DIP204_txt_engine(LineText, (Count + 2), MARKER_POSITION, strlen(LineText));
         }
       else
         {
         strcpy(LineText, ValidFileName);
         DIP204_txt_engine(LineText, (Count + 2), FILE_POSITION, strlen(LineText));
         }
       }
   }
 
 // STEP 3
 // CHECK IF SELECTED FILE IS GRERATER THAN DISPLAY LENGHT.  IF SO IT WILL DISPLAY AT THE BOTTOM OF THE DISPLAY
 if (Music_List.SelectedFile >= (LENGTH_OF_DISPLAY))
   {
   for (uint8_t Count = 0; Count < LENGTH_OF_DISPLAY; Count++)
     {
     checkFileName(Music_List.MusicDirEntry[(Music_List.SelectedFile - Count)], ValidFileName);
     // IF SELECTED FILE PLACE MARKER
     if (Count == 0)
       {
       strcpy(LineText,"*");
       strcat(LineText, ValidFileName);
       DIP204_txt_engine(LineText, DISPLAY_LAST_ROW, MARKER_POSITION, strlen(LineText));
       }
     else
       {
       strcpy(LineText, ValidFileName);
       DIP204_txt_engine(LineText, (DISPLAY_LAST_ROW - Count), FILE_POSITION, strlen(LineText));
       }
     }
   }
 
 // STEP 4
 // SET THE ICON SCROLL CURSORS
 // SELECTED AT TOP OF LIST
 if (Music_List.SelectedFile == 0)
   {
   DIP204_ICON_set(ICON_DOWN_ARROW, ICON_ON);
   DIP204_ICON_set(ICON_UP_ARROW, ICON_OFF);
   }
 // SELECTED SOMEWHERE IN THE MIDDLE OF LIST
 if ((Music_List.SelectedFile > 0) && (Music_List.SelectedFile < (Music_List.NumberOfDirEntries - 1)))
   {
   DIP204_ICON_set(ICON_UP_ARROW, ICON_ON);
   DIP204_ICON_set(ICON_DOWN_ARROW, ICON_ON);
   }
 // SELECTED AT END OF LIST
 if (Music_List.SelectedFile == (Music_List.NumberOfDirEntries - 1))
   {
   DIP204_ICON_set(ICON_UP_ARROW, ICON_ON);
   DIP204_ICON_set(ICON_DOWN_ARROW, ICON_OFF);
   }
     
 } // END OF printMusicList
   
   
   
   
/******************************************************************************
 * Function Name: checkFileName
 * Parameters: uint8_t *, uint8_t *
 * Return: void
 *
 * Description: The parameter FileNameToCheck is stripped of its extension.  The
 * file name is also checked to see if the remaining length of the file can fit
 * into the display.  The max lenght is DISPLAY_COLUMN_TOTAL - FILE_POSITION.  If too
 * long the name is truncated and formated such that the new name looks like "FileName..."
 * The ValidFileName is returned by reference
 * STEP 1: Strip the file name of its extension
 * STEP 2: Check if too long for display, if so truncate name to fit ending with ...
 * STEP 3: Load the valid name
 **************************************************************************/     
 static void checkFileName(const uint8_t *FileNameToCheck, uint8_t *ValidName)
 {
 
 uint8_t StrLength,
         FileExtensionPosition = 0;
 uint8_t ModifiedName[MAX_FILE_NAME_LEN];
 uint8_t *ptr_FileNameToCheck;
 
 // STEP 1
 // STRIP THE FILE EXTENSION
 strcpy(ModifiedName, FileNameToCheck);
 ptr_FileNameToCheck = ModifiedName;
 for (uint8_t DummyCount = 0; DummyCount < DISPLAY_COLUMN_TOTAL; DummyCount++)
   {
   if (*ptr_FileNameToCheck == '.')
     {
     strncpy(ModifiedName, FileNameToCheck, (FileExtensionPosition + 1));
     uint8_t *TerminateStringLocation = ModifiedName + FileExtensionPosition;
     *TerminateStringLocation = STRING_NULL;
     break;
     }
   *ptr_FileNameToCheck++;
   FileExtensionPosition++;
   }
 
 // STEP 2
 // CHECK THE LENGHT - IF OVER FILL IN WITH ...
 // SET POINTER TO WHERE ... WOULD GO
 ptr_FileNameToCheck = ModifiedName;
 ptr_FileNameToCheck += (DISPLAY_COLUMN_TOTAL - FILE_POSITION) - 3; // DOTS = 3
 StrLength = strlen(FileNameToCheck);
 if (StrLength > (DISPLAY_COLUMN_TOTAL - FILE_POSITION))
   {
   *ptr_FileNameToCheck = '.';
   ptr_FileNameToCheck++;
   *ptr_FileNameToCheck = '.';
   ptr_FileNameToCheck++;
   *ptr_FileNameToCheck = '.';
   ptr_FileNameToCheck++;
   *ptr_FileNameToCheck = STRING_NULL;
   } 
 
  // STEP 3
  strcpy(ValidName, ModifiedName);
  return;
 
 } // END OF checkFileName




/*************************************************************************
 * Function Name: call_MusicScrollUp
 * Parameters: void
 * Return: void
 *
 * Description: Sets up the Music Dir to scroll up by incrementing the selected file
 * and calling the task to display the music files.
 * STEP 1: Increment to next selected file and set event task
 **************************************************************************/
 void call_MusicScrollUp(void)
 {
 
 // STEP 1
 if (Music_List.SelectedFile != 0)
   Music_List.SelectedFile--;
 ctl_events_set_clear(&CalEvents, EVENT_MUSIC_LIST, 0);
 
 } // END OF call_MusicScrollUp




/*************************************************************************
 * Function Name: call_MusicScrollDown
 * Parameters: void
 * Return: void
 *
 * Description: Sets up the Music Dir to scroll down by decrementing the selected file
 * and calling the task to display the music files.
 * STEP 1: Decrement to next selected file and set event task
 **************************************************************************/
 void call_MusicScrollDown(void)
 {
 
 // STEP 1
 Music_List.SelectedFile++;
 if (Music_List.SelectedFile > (Music_List.NumberOfDirEntries - 1))
   Music_List.SelectedFile = Music_List.NumberOfDirEntries - 1;   
 ctl_events_set_clear(&CalEvents, EVENT_MUSIC_LIST, 0);
 
 } // END OF call_MusicScrollDown




/*************************************************************************
 * Function Name: call_PlayPauseMusic
 * Parameters: void
 * Return: void
 *
 * Description: Sets the condition to play or pause the selected file.
 * NOTE: The actual playing of music is done by the task audio_taskFn.  
 * NOTE: This function can only be entered when in Music List mode
 * NOTE: CalMode.MusicPlayBack.Playing is set by function call_play16Bit_WAVE
 * STEP 1: Check for play pause if already playing
 * STEP 2: Disable scrolling and setup screen and blink the file name before play
 * STEP 3: Play audio
 **************************************************************************/
 void call_PlayPauseMusic(void)
 {
 
 Type_AudioQueueStruct AudioQueueStruct;
 uint8_t DisplayFileName[DISPLAY_COLUMN_TOTAL];
 
 // STEP 1
 // CHECK IF ALREADY PLAYING - IF SO TOGGLE PLAY PAUSE
 if (CalSettings.MusicPlayBack.Playing)
   {
   if (CalSettings.MusicPlayBack.Pause)
     CalSettings.MusicPlayBack.Pause = FALSE;
   else
     CalSettings.MusicPlayBack.Pause = TRUE;
   return;
   }
 
 // STEP 2
 // DISABLE SCROLLING - PLAY AND STOP STILL ENABLED
 CalSettings.Mask_KeyTouchB &= ~(MASK_KEY_UP | MASK_KEY_DOWN);
 // SETUP DISPLAY
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 DIP204_ICON_set(ICON_UP_ARROW, ICON_OFF);
 DIP204_ICON_set(ICON_DOWN_ARROW, ICON_OFF);
 DIP204_txt_engine(PLAY_HEADING, DIR_HEAD_ROW, DIR_HEAD_POSITION, strlen(PLAY_HEADING));
 checkFileName(Music_List.MusicDirEntry[Music_List.SelectedFile], DisplayFileName);
 // BLINK FILE NAME
 for (uint8_t Count = 0; Count < 3; Count++)
   {
    DIP204_txt_engine(DisplayFileName, DISPLAY_FILE_ROW, FILE_POSITION, strlen(DisplayFileName));
    ctl_timeout_wait(ctl_get_current_time()+300);
    DIP204_clearLine(DISPLAY_FILE_ROW);
    ctl_timeout_wait(ctl_get_current_time()+300);
   }
 DIP204_txt_engine(DisplayFileName, DISPLAY_FILE_ROW, FILE_POSITION, strlen(DisplayFileName));
 
 // STEP 3
 // PLAY AUDIO: SET AUDIO PLAY CONDITIONS ADD TO AUDIO QUEUE
 strcpy(AudioQueueStruct.FileName, Music_List.MusicDirEntry[Music_List.SelectedFile]);
 AudioQueueStruct.PlayLevel = CORE_SOUND;
 CalSettings.MusicPlayBack.Play = TRUE;
 CalSettings.MusicPlayBack.Pause = FALSE;
 ctl_PostMessageByMemAllocate(&AudioQueueStruct);
 
 } // END OF call_PlayPauseMusic




/*************************************************************************
 * Function Name: call_MusicStop
 * Parameters: void
 * Return: void
 *
 * Description: Sets the conditions to stop the playing of audio
 * STEP 1: Set condition to stop audio
 * STEP 2: Enable scrolling keys and show display
 **************************************************************************/
 void call_MusicStop(void)
 {
  
 // STEP 1
 CalSettings.MusicPlayBack.Play = FALSE;

 // STEP 2
 // ENABLE SCROLLING KEYS
 CalSettings.Mask_KeyTouchB |= (MASK_KEY_UP | MASK_KEY_DOWN);
 // SET DISPLAY FOR MUSIC LIST
 DIP204_set_cursor(CURSOR_OFF);
 DIP204_clearDisplay();
 DIP204_txt_engine(MUSIC_HEADING, DIR_HEAD_ROW, DIR_HEAD_POSITION, strlen(MUSIC_HEADING));
 printMusicList();
 
 } // END OF call_MusicStop


