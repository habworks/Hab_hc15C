/*****************************************************************
 *
 * File name:         LIST_TASK.H
 * Description:       Project definitions and function prototypes for use with LIST_TASKS.c
 * Author:            Hab S. Collector
 * Date:              12/06/2011
 * LAST EDIT:         7/21/2012 
 * Hardware:               
 * Firmware Tool:     CrossStudio for ARM
 * Notes:             This file should be written as to not be dependent 
 *                    on other includes - everything these functions need should be passed to them
*****************************************************************/ 

#ifndef _LIST_TASKS_DEFINES
#define _LIST_TASKS_DEFINES

// INCLUDES
#include "HC15C_DEFINES.H"
#include "DIP204.H"


// DEFINES
// 0123456789
// DIRECTORY LISTING:
// 0:\FIRST FILE.TXT
// DEFINE PATH
#define DEFAULT_PATH      "0:"
#define DIR_HEADING       "DIRECTORY LISTING:"
#define MUSIC_HEADING     "MUSIC LISTING:"
#define PLAY_HEADING      "MUSIC FILE:"
#define DIR_HEAD_ROW      1
#define DISPLAY_FILE_ROW  2
#define DISPLAY_LAST_ROW  4
#define DIR_HEAD_POSITION 0
#define FILE_POSITION     2
#define MARKER_POSITION   1
#define MAX_ENTRIES       40
#define MAX_FILE_NAME_LEN 40
#define LENGTH_OF_DISPLAY 3                       // TOTAL LINES ALLOCATED FOR DISPLAY
#ifndef STRING_NULL
  #define STRING_NULL     ('\0')
#endif


// STRUCTURES
typedef struct
  {
  uint8_t DirEntry[MAX_ENTRIES][MAX_FILE_NAME_LEN];
  uint8_t UI;
  uint8_t NumberOfDirEntries;
  uint8_t TopListingDisplayed;
  } Type_SD_List; 

enum UI
  {
  NO_SCROLL,
  SCROLL_UP,
  SCROLL_DOWN
  };

typedef struct
  {
  uint8_t MusicDirEntry[MAX_ENTRIES][MAX_FILE_NAME_LEN];
  uint8_t NumberOfDirEntries;
  uint8_t  SelectedFile;
  } Type_Music_List; 
  


// PROTOTYPES
// SD LIST MODE
void SD_List_taskFn(void *);
void call_SD_ListMode(void);
void LoadSD_Dir(void);
void DisplayListOf3(void);
void call_DirScrollDown(void);
void call_DirScrollUp(void);
void call_SD_ListModeEnd(void);
// MUSIC LIST MODE
void call_MusicListMode(void);
void call_MusicListModeEnd(void);
void LoadMusicDirListing(const uint8_t *);
static void printMusicList(void);
static void checkFileName(const uint8_t *, uint8_t *);
void call_MusicScrollUp(void);
void call_MusicScrollDown(void);
void call_PlayPauseMusic(void);
void call_PlayPauseMusic(void);
void call_MusicStop(void);

#endif